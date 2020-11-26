////////////////////////////////////////////////////////////////////////////////
//
// File: competition.c
//
// Description:
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//                  18 Apr 2006 - Bugs fixed in RegisterRobot().
//                  19 Jul 2006 - Bug fixed in SetStatusMessage().
//                  19 Jul 2006 - Major rework to change robot data from array
//                                to linked list.  LL docs found at:
//                                http://cmp.felk.cvut.cz/~matas/surrey/
//                                                             software/LL.html
//                  20 Jul 2006 - Added gamestate GS_SETUP and added checks in
//                                RegisterRobot() and AddSensor() to ensure
//                                they are called only during game setup.
//                                Note that through a couple of tricks it is
//                                still possible for a robot to add another
//                                robot during its own setup, but it's not
//                                worth coding the code to stop it as this
//                                cheat would be plainly evident.
//                  23 Jul 2006 - Changed the way weapons function to have them
//                                exist as systems in the robot before firing,
//                                but create a weapon in a linked list after
//                                firing.  This greatly simplifies iterating
//                                through weapons in other files.  Changes here
//                                include some in InitCompetition() and
//                                Fight(), but more in FireWeapon(), and
//                                EndCompetition().  As part of these changes,
//                                weapons were also standardized.  For example,
//                                lasers now have a splash damage that is
//                                treated the same as missiles, but it happens
//                                to be for 0 range and 0 damage.
//                   1 Aug 2006 - Added a range of new functions:
//                                   - SendMessage()
//                                   - GetMessage()
//                                   - HasNullCharacter()
//                                   - LookupRobotByName()
//                                Also changed all "internal" functions to
//                                static to protect newer functions such as
//                                LookupRobotByName(), which could have easily
//                                broken the internal game "secrets".
//                   6 Sep 2006 - Changed RegisterRobot() to allow for custom
//                                robot graphics and start locations.
//                              - Changed ChooseRandomLocation() so that robots
//                                are placed anywhere on the screen but keep at
//                                least MIN_RANDOM_DIST cm from each other.
//                  10 Apr 2007 - New functions TurboBoost() and IsTurboOn().
//                                New sound in global sound file array.
//
// TODO: 1) Add a detector for an infinite loop on a robot's turn.
//
//
////////////////////////////////////////////////////////////////////////////////
#include <stdio.h>                       //For sprintf(), srand, and rand
#include <time.h>                        //For system time.
#include <string.h>
#include <math.h>
#include "competition.h"
#include "physics.h"
#include "graphics.h"
#include "particles.h"

static GAME theGame;
static t_LL robotList;        //Holds list of all robots.
static t_LL deadRobotList;    //Holds list of destroyed robots.
static ROBOT *curRobot = NULL;   //Used when robots are giving orders.
static t_LL weaponList;       //Weapons that have been fired.

volatile int calcCounter = 0;
volatile int calcsCompleted = 0, cps = 0;
volatile int frameCounter = 0, fps = 0;

static char *soundFileNames[NUM_SOUNDS] = { "..\\resources\\sounds\\laser.wav",
		"..\\resources\\sounds\\laserhit.wav",
		"..\\resources\\sounds\\missile.wav",
		"..\\resources\\sounds\\missilehit.wav",
		"..\\resources\\sounds\\fight.wav",
		"..\\resources\\sounds\\robotscollide.wav",
		"..\\resources\\sounds\\robotexplode.wav",
		"..\\resources\\sounds\\turboboost.wav" };

//Internal Helper Prototypes.  These are static
//to protect them from being called by a robot.
static int HasNullCharacter(char *string, int numChars);
static ROBOT *LookupRobotByName(char *name);
static int BoundAngle(int angle);
static void ChooseRandomLocation(ROBOT *robot);
static void PlaySounds(GAME *game);

////////////////////////////////////////////////////////////////////////////////
//
// Name: AddCalc
//
// Description: This is the timer interrupt handler for calculation counts.
//              It should be called CALCS_PER_SEC times each second.  All that
//              is performed is a record of the number of calculations to
//              perform the next time a game loop runs.  This way the robots
//              will move at the same speed irrespective of the refresh rate
//              of the monitor.
//
//              Because timers are actually implemented as threads, the
//              following rules must be followed for proper Allegro timer ops:
//                - Globals accessed here must be declared volatile AND must be
//                  locked in the initialization code.
//                - The function itself must be locked in the initialization.
//                - Function must end with the macro END_OF_FUNCTION(AddCalc)
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void AddCalc(void) {
	calcCounter++;
}
END_OF_FUNCTION(AddCalc)

////////////////////////////////////////////////////////////////////////////////
//
// Name: SecondTimer
//
// Description: This is the timer interrupt handler that increments once per
//              second.  This is used to calculate the frames per second.
//              and calculations performed per second.  This function and the
//              variables frameCounter, fps, calcsCompleted, and cps are
//              subject to all the same restrictions described in AddCalc().
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void FPSTimer(void) {
	fps = frameCounter;
	frameCounter = 0;
	cps = calcsCompleted;
	calcsCompleted = 0;
}
END_OF_FUNCTION(SecondTimer)

////////////////////////////////////////////////////////////////////////////////
//
// Function: Fight
//
// Description: This is heart of the Robot Wars competition.  It installs the
//              required timers (including locking the related variables) and
//              then enters the main game loop.  In that loop, the robots are
//              updated based on the number of remaining calculations to
//              perform.  Only once calcCounter has been reduced to 0 is the
//              scene drawn.
//
// Note: While not a bug per se, if run on a crappy system, or with too many
//       robots, this system can result in no scenes being rendered for periods
//       of time, or even the duration of the competition.  I need to add a
//       sanity check that will still render at least 1 frames/s even if the
//       overall speed of the simulation drops.
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void Fight(void) {
	int keyPress, calcsUntilOrders = 0;
	char systemMessage[255] = "";
	ROBOT *robot;

	LOCK_VARIABLE(calcCounter);LOCK_VARIABLE(calcsCompleted);LOCK_VARIABLE(cps);LOCK_VARIABLE(frameCounter);LOCK_FUNCTION(fps);LOCK_FUNCTION(AddCalc);

	if (install_int_ex(AddCalc, BPS_TO_TIMER(CALCS_PER_SEC)) != 0)
		AbortOnError(
				"Fight() failed to create calcs timer.\nProgram will exit.");

	if (install_int_ex(FPSTimer, SECS_TO_TIMER(1)) != 0)
		AbortOnError("Fight() failed to create FPS timer.\nProgram will exit.");

	while (!key[KEY_SPACE])
		;  //Wait for screen to initialize before starting.

	theGame.state = GS_FIGHTING;

	while (!key[KEY_ESC]) {

		if (keypressed())                          //Check for relevant keys.
		{
			keyPress = readkey() & 0xff;
			if (keyPress == 'r' || keyPress == 'R') //User wants re-randomized loc's.
				ForeachLL_M(robotList, robot)
				{
					ChooseRandomLocation(robot);
					robot->heading = GetRandomNumber(360);
				}
		}

		while (calcCounter) {
			calcCounter--;
			calcsUntilOrders++;

			UpdateEnergySystems(robotList);           //Do first so we know what
													  //systems are powered.
			MoveRobots(robotList);
			DrawRobotBitmaps(robotList);
			CheckRobotCollisions(&theGame, robotList);

			MoveWeapons(weaponList);
			CheckWeaponCollisions(&theGame, robotList, weaponList);

			//Now that collisions are
			ApplyDamage(&theGame, robotList, deadRobotList); //done and weapons have
															 //hit, we apply damage.

			DrawSensorBitmaps(robotList);          //Need to draw BEFORE data is
												   //updated because bimaps are
												   //used in collision detection.
			UpdateSensorData(robotList);

			UpdateParticles();

			calcsCompleted++;

			if (calcsUntilOrders == ORDER_FREQ) {
				calcsUntilOrders = 0;
				ForeachLL_M(robotList, curRobot)
					curRobot->ActionsFunction(TURN_TIME);
			}

			if (theGame.useSounds)
				PlaySounds(&theGame);

		}

		sprintf(systemMessage, "FPS: %d  CPS: %d", fps, cps);
		RenderScene(robotList, deadRobotList, weaponList, systemMessage);
		frameCounter++;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: InitCompetition
//
// Description: This function sets up the robot list, the Allegro library,
//              sets up and loads in the sounds, and calls the function to
//              initialize the graphics.
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void InitCompetition() {
	int i;

	theGame.state = GS_SETUP;    //Record game state.
	robotList = ConsLL();        //Create the robot linked list.
	deadRobotList = ConsLL();    //Create the destroyed robots linked list.
	weaponList = ConsLL();

	srand((unsigned) time(NULL)); //Randomize random numbers with time.
	allegro_init();              //Initialize the Allegro library and
	install_timer();             //specifically add the timer, keyboard,
	install_keyboard();          //and mouse functionality.
	install_mouse();

	if (!install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL)) {
		theGame.useSounds = 1;
		for (i = 0; i < NUM_SOUNDS; i++)   //Set all sounds to NULL so they will
			theGame.sounds[i] = NULL;    //be properly freed in EndCompetition()

		for (i = 0; i < NUM_SOUNDS; i++) {
			theGame.playSound[i] = 0;
			theGame.sounds[i] = load_wav(soundFileNames[i]);
			if (theGame.sounds[i] == NULL) {
				theGame.useSounds = 0;
				char message [40];
                sprintf(message, "Sounds failed to load %d", i);
				AbortOnError(message);
				break;
			}
		}
	} else
		theGame.useSounds = 0;

	InitGraphics();              //Load graphics particular to the robot
}                              //competition.

////////////////////////////////////////////////////////////////////////////////
//
// Function: RegisterRobot
//
// Description: This function adds robots to the game.  Note that the random
//              starting locations are highly dependent on a maximum number
//              of robots of 4.  If more are desired, this function will
//              have to be adjusted.
//
// Bug Fixes: 18 Apr 2006 - Found a bug that caused all calls to
//                          SetSystemChargePriorities() to affect the 0th
//                          robot when called from the setup function.
//                          curRobot is now changed before the call to the
//                          robot configuration function.  This also means that
//                          all robot functions can be called during
//                          configuration.
//                        - The call the the configuration function also moved
//                          to the end of this function so that any changes the
//                          player makes to how the robot functions will not be
//                          overwritten with defaults.
//
// Change History: 23 Juy 2006 - Now that weapons are treated the same, there
//                               is a much larger area as weapons are
//                               initialized on the robot.  Some of this data
//                               (eg: impactSound) isn't even used as part of
//                               the robot, but is simply copied over to the
//                               weapon when it is fired in FireWeapon() for
//                               later use.
//                  6 Sep 2006 - Major change as users can now specify unique
//                               graphics for their robot, as well as a
//                               starting location.
//                 10 Apr 2007 - Added Turbo boost functions to turn on/check
//                               as well as required constants and sound
//                               support.
//
// Parameters: char *robotName - The string name of the robot.
//             ROBOTCOLORS color - The color of the robot in the game.
//             robotActions - A pointer to a function that accepts an int
//                            to be called by the game every time the robot's
//                            logic is to be executed.
//             configureRobot - A pointer to a function so the player can
//                              configure the robot before the game begins.
//             customImage - A string giving the path/file name of a bitmap to
//                           use as the image for this robot.  No size checking
//                           is performed, so use with caution.
//             x, y        - Starting location and heading for the robot.  If
//                           any of these values are <0 then a random location
//                           will be selected.  Values in cm.
//             heading     - Starting heading of the robot.  If <0 then a random
//                           heading will be chosen.  Value in degrees.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void RegisterRobot(char *robotName, ROBOTCOLORS color,
		void (*robotActions)(int), void (*configureRobot)(void),
		char *customImage, int x, int y, float heading) {
	int i, robotNumber;
	ROBOT newRobot;

	if (theGame.state != GS_SETUP)
		AbortOnError(
				"Call made to RegisterRobot() when game is not in setup state."
						"\nProgram will end.");

	robotNumber = SizeLL(robotList);
	if (robotNumber == MAX_ROBOTS)
		AbortOnError("RegisterRobot() Attempted to add too many robots.\n"
				"Program will end.");

	newRobot.ActionsFunction = robotActions;

	switch (color) {
	case ROBOT_RED:
		newRobot.color = makecol(255, 0, 0);
		break;
	case ROBOT_GREEN:
		newRobot.color = makecol(0, 255, 0);
		break;
	case ROBOT_BLUE:
		newRobot.color = makecol(0, 0, 255);
		break;
	case ROBOT_YELLOW:
		newRobot.color = makecol(255, 255, 0);
		break;
	case ROBOT_PURPLE:
		newRobot.color = makecol(255, 0, 240);
		break;
	case ROBOT_TURQUOISE:
		newRobot.color = makecol(0, 255, 255);
		break;
	case ROBOT_WHITE:
	default:
		newRobot.color = makecol(255, 255, 255);
	}
	if (HasNullCharacter(robotName, MAX_NAME_LEN)) {
		newRobot.name = malloc(sizeof(char) * strlen(robotName));
		strcpy(newRobot.name, robotName);
		newRobot.number = robotNumber;
	} else
		AbortOnError("RegisterRobot() was passed a robot name exceeding "
				"MAX_NAME_LEN.\nProgram will end.");

	//Set starting speed values
	newRobot.leftTreadSpeed = 0;
	newRobot.rightTreadSpeed = 0;
	newRobot.impulseHeading = 0;
	newRobot.impulseSpeed = 0;
	newRobot.turboTime = 0;

	//Configure the sensors.
	newRobot.bumped = BUMP_NONE;
	for (i = 0; i < MAX_SENSORS; i++) {
		newRobot.sensorArray[i].type = SENSOR_NONE;
		newRobot.sensorArray[i].image = NULL;
	}

	//Configure the laser.
	newRobot.weaponArray[LASER_PORT].type = WEAPON_LASER;
	newRobot.weaponArray[LASER_PORT].maxAngle = LASER_MAX_ANGLE;
	newRobot.weaponArray[LASER_PORT].minEnergy = MIN_LASER_ENERGY;
	newRobot.weaponArray[LASER_PORT].maxEnergy = MAX_LASER_ENERGY;
	newRobot.weaponArray[LASER_PORT].bonusEnergy = LASER_ENERGY_BONUS;
	newRobot.weaponArray[LASER_PORT].splashRange = LASER_SPLASH_RANGE;
	newRobot.weaponArray[LASER_PORT].splashDamage = LASER_SPLASH_DAMAGE;
	newRobot.weaponArray[LASER_PORT].speed = LASER_SPEED;
	newRobot.weaponArray[LASER_PORT].bumpValue = BUMP_LASER;
	newRobot.weaponArray[LASER_PORT].firingSound = SND_LASER_FIRE;
	newRobot.weaponArray[LASER_PORT].impactSound = SND_LASER_HIT;

	//Configure the missiles.
	newRobot.weaponArray[MISSILE_PORT].type = WEAPON_MISSILE;
	newRobot.weaponArray[MISSILE_PORT].maxAngle = MISSILE_MAX_ANGLE;
	newRobot.weaponArray[MISSILE_PORT].minEnergy = MIN_MISSILE_ENERGY;
	newRobot.weaponArray[MISSILE_PORT].maxEnergy = MAX_MISSILE_ENERGY;
	newRobot.weaponArray[MISSILE_PORT].bonusEnergy = MISSILE_ENERGY_BONUS;
	newRobot.weaponArray[MISSILE_PORT].splashRange = MISSILE_SPLASH_RANGE;
	newRobot.weaponArray[MISSILE_PORT].splashDamage = MISSILE_SPLASH_DAMAGE;
	newRobot.weaponArray[MISSILE_PORT].speed = MISSILE_SPEED;
	newRobot.weaponArray[MISSILE_PORT].bumpValue = BUMP_MISSILE;
	newRobot.weaponArray[MISSILE_PORT].firingSound = SND_MISSILE_FIRE;
	newRobot.weaponArray[MISSILE_PORT].impactSound = SND_MISSILE_HIT;

	for (i = 0; i < MAX_WEAPONS; i++) {
		newRobot.weaponArray[i].chargeRate = 0;
		newRobot.weaponArray[i].chargeEnergy = 0;
	}

	//Set starting energy priorites in same order as SYSTEMS declaration.
	for (i = 0; i < NUM_ENERGY_SYSTEMS; i++)
		newRobot.energyPriorities[i] = (SYSTEM) i;

	//Set initial energy values.  Weapons were done in the weapons section.
	newRobot.shields = START_SHIELD_ENERGY;
	newRobot.shieldChargeRate = 0;

	//Set structure-related values
	newRobot.generatorStructure = MAX_GENERATOR_STRUCTURE;

	//Robot has no damage to be applied at start.
	newRobot.damageBank = 0;

	//Load the robot's graphic, if required.  There is no error checking here as
	//if newRobot.graphic is NULL, the graphics routines will simply use the
	//default robot graphic to render the robot.
	newRobot.graphic = NULL;
	if (customImage != NULL)
		newRobot.graphic = load_bitmap(customImage, NULL);

	//Create the robot's bitmap.
	newRobot.image = create_bitmap(SHIELD_BMP_SZ, SHIELD_BMP_SZ);
	if (newRobot.image == NULL)
		AbortOnError("RegisterRobot() failed to create robot image.\n"
				"Program will end.");

	strcpy(newRobot.statusMessage, ""); //No status message at start.

	newRobot.mailBox = ConsLL();        //Create the robot's mailbox.

	InsLastLL(robotList, newRobot);     //Put robot on end of the list.
	curRobot = LastElmLL(robotList);   //Record current robot for configuration.

	//Set robot starting location.  This is done only after insertion into the
	//list because ChooseRandomLocation() requires the list to have robots in it.
	if (x < 0 || y < 0)
		ChooseRandomLocation(curRobot);
	else {
		curRobot->x = x;
		curRobot->y = y;
	}
	if (heading < 0)
		curRobot->heading = GetRandomNumber(360);
	else
		curRobot->heading = heading;

	configureRobot();                   //Call user's configuration function.
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: AddSensor
//
// Description: This adds a sensor to the robot.  The provided port must be
//              an empty port on the robot, and must be in the valid port range,
//              width range, and distance range.
//
// Parameters: int port - The port to put the sensor on.
//             SENSORTYPE type - The type of sensor to add.
//             int angle - The angle the sensor points.  This may have different
//                         meanings for different sensors.
//             int range - The distance to which the sensor can "look".
//
// Returns: int - 0 if sensor install fails.
//                1 if successful.
//
////////////////////////////////////////////////////////////////////////////////
int AddSensor(int port, SENSORTYPE type, int angle, int width, int range) {
	if (theGame.state != GS_SETUP)
		AbortOnError(
				"Call made to AddSensor() when game is not in setup state.\n"
						"Program will end.");

	if (port < 0 || port > MAX_SENSORS - 1)
		return 0;

	if (curRobot->sensorArray[port].type != SENSOR_NONE)
		return 0;

	angle = BoundAngle(angle);

	if (type == SENSOR_RADAR) {
		if (width < MIN_RADAR_ARC || width > MAX_RADAR_ARC)
			return 0;

		if (range < RADAR_MIN_RANGE || range > RADAR_MAX_RANGE)
			return 0;

		curRobot->sensorArray[port].image = create_bitmap(RADAR_IMAGE_PX,
		RADAR_IMAGE_PX);
		if (curRobot->sensorArray[port].image == NULL)
			AbortOnError("AddSensor() failed to create a radar bitmap.\n"
					"Program will end.");
	} else if (type == SENSOR_RANGE) {
		curRobot->sensorArray[port].image = NULL;
		range = RANGE_MAX_RANGE;
	} else
		return 0;

	curRobot->sensorArray[port].type = type;
	curRobot->sensorArray[port].angle = angle;
	curRobot->sensorArray[port].width = width;
	curRobot->sensorArray[port].range = range;
	curRobot->sensorArray[port].on = 1;
	curRobot->sensorArray[port].powered = 0;
	curRobot->sensorArray[port].data = -1;

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: EndCompetition
//
// Description: This function cleans up our allocated resources such as the
//              linked lists, images, sounds, etc... as well as shutting down
//              the Allegro graphics library.
//
// Parameters:  None.
//
// Change History: 19-23 July 2006 - When the linked list rework was done, this
//                                   function changed to perform the cleanup
//                                   of a) allocated memory for each element
//                                   in a linked list, b) each element of in
//                                   that list and finally c) the list.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void EndCompetition() {
	int i;
	ROBOT *tempRobot, *nextRobot;
	WEAPON *tempWeapon, *nextWeapon;

	//Move the dead robots back into the main list for deletion.
	//deadRobotList should be empty after this loop has finished.
	while (!IsEmptyLL(deadRobotList))
		LinkAftLL(LastElmLL(robotList), UnlinkLL(FirstElmLL(deadRobotList)));

	//Now iterate through each robot, cleaning up allocated memory
	//and, when all memory is cleaned up, remove each one from the list.
	SafeForeachLL_M(robotList, tempRobot, nextRobot)
	//Has to be "safe" as
	{                                                    //list elements will be
		nextRobot = NextElmLL(tempRobot);                   //deleted during the
		for (i = 0; i < MAX_SENSORS; i++)                       //iteration.
			if (tempRobot->sensorArray[i].image != NULL)
				destroy_bitmap(tempRobot->sensorArray[i].image); //Free sensor images.

		if (tempRobot->graphic != NULL)
			destroy_bitmap(tempRobot->graphic);        //Free custom images.
		destroy_bitmap(tempRobot->image);            //Free robot bitmaps.
		free(tempRobot->name);                       //Free robot name strings.

		DestLL(tempRobot->mailBox);                  //Remove all messages.

		DelElmLL(tempRobot);                   //Delete the robot from the list.
	}

	//Now clean up the weapon list, deleting each weapon.
	SafeForeachLL_M(weaponList, tempWeapon, nextWeapon)
	{
		nextWeapon = NextElmLL(tempWeapon);
		if (tempWeapon->image != NULL)
			destroy_bitmap(tempWeapon->image);           //Free weapon images.
		DelElmLL(tempWeapon);                          //Delete the weapon.
	}

	if (IsEmptyLL(robotList) && IsEmptyLL(deadRobotList)
			&& IsEmptyLL(weaponList)) {
		DestLL(robotList);
		DestLL(deadRobotList);
		DestLL(weaponList);
	} else {
		DestLL(robotList);
		DestLL(deadRobotList);
		DestLL(weaponList);
		AbortOnError(
				"EndCompetition() discovered one of the main linked lists was "
						"not empty before deletion.\n"
						"Program will now end.");
	}

	//Delete the sounds.
	for (i = 0; i < NUM_SOUNDS; i++)
		if (theGame.sounds[i] != NULL)
			destroy_sample(theGame.sounds[i]);

	clear_keybuf();                //Clean up Allegro and our graphics.
	DeInitGraphics();

}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SetMotorSpeeds
//
// Description: This function sets the robots tread speeds.  Speeds are a % of
//              maximum, from 0 to 100, -ve to +ve.
//
// Parameters: int leftSpd - Left tread speed.
//             int rightSpd - Right tread speed.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void SetMotorSpeeds(int leftSpd, int rightSpd) {
	if (leftSpd < -100)
		leftSpd = -100;
	else if (leftSpd > 100)
		leftSpd = 100;

	if (rightSpd < -100)
		rightSpd = -100;
	else if (rightSpd > 100)
		rightSpd = 100;

	curRobot->leftTreadSpeed = leftSpd;
	curRobot->rightTreadSpeed = rightSpd;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: TurboBoost
//
// Description: This function fires the turbo boost, if energy allows.
//              The new firing "wastes" any time remaining in the previous turbo
//              boost, if there was any.
//
// Parameters: None.
//
// Returns: int - 1 if Turbo was turned on, 0 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
int TurboBoost() {
	if (curRobot->shields > TURBOBOOST_COST) {
		curRobot->shields -= TURBOBOOST_COST;
		curRobot->turboTime = TURBOBOOST_TIME * CALCS_PER_SEC;
		theGame.playSound[SND_TURBOBOOST] = 1;
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: IsTurboOn
//
// Description: This function check if a turbo boost is currently firing.
//
// Parameters: None.
//
// Returns: int - 1 if Turbo is firing, 0 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
int IsTurboOn() {
	if (curRobot->turboTime)
		return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetGPSInfo
//
// Description: This function fills out a GPS_INFO structure for the requesting
//              robot.  Energy for the GPS use is dedected from the robot's
//              shield capacitor.  If there in not sufficient energy, the
//              request fails.
//
// Note: The heading is the same as that stored by the robot, so it is in
//       math coordinates, with 0 being to the right and increasing counter-
//       clockwise.
//
// Parameters: GPS_INFO *gpsData - Pointer to the structure to fill out.
//
// Returns: int - 1 if the request succeeds, 0 if it fails.
//
////////////////////////////////////////////////////////////////////////////////
int GetGPSInfo(GPS_INFO *gpsData) {
	if (curRobot->shields > GPS_COST) {
		curRobot->shields -= GPS_COST;
		gpsData->x = curRobot->x;
		gpsData->y = curRobot->y;
		gpsData->heading = curRobot->heading;
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetSensorData
//
// Description: This function returns a sensor's data, if the sensor is on
//              and powered.
//
// Parameters: int port - The sensor port to read.
//
// Returns: int - The sensor's data.  If the port is invalid, or the sensor is
//                off or unpowered this returns -1.
//
////////////////////////////////////////////////////////////////////////////////
int GetSensorData(int port) {
	if (port >= 0 && port < MAX_SENSORS)
		if (curRobot->sensorArray[port].type != SENSOR_NONE)
			if (curRobot->sensorArray[port].on
					&& curRobot->sensorArray[port].powered)
				return curRobot->sensorArray[port].data;

	return -1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SetSensorStatus
//
// Description: This function allows the user to turn a robot's sensor on/off.
//
// Parameters: int port - The sensor port to change.
//             int status - 0 to turn off, non-zero to turn on.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void SetSensorStatus(int port, int status) {
	if (status != 0)
		status = 1;
	if (port >= 0 && port < MAX_SENSORS)
		curRobot->sensorArray[port].on = status;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: FireWeapon
//
// Description: This function fires the given weapon, if it is sufficiently
//              powered and it meets firing arc requirements.
//
// Parameters: WEAPONTYPE type - The weapon to fire.
//
// Returns: int - 0 if weapon firing fails, 1 if successful.
//
////////////////////////////////////////////////////////////////////////////////
int FireWeapon(WEAPONTYPE type, int heading) {
	WEAPON_SYSTEM *weaponSys;
	WEAPON weapon;
	int imageX, imageY;

	heading = heading % 360;     //Change heading so it is abs(heading)<360.
	//Figure out which weapon is being examined and set the bitmap constants.
	//Note: should the image vals be part of the weapon system for uniformity?
	switch (type) {
	case WEAPON_LASER:
		weaponSys = &curRobot->weaponArray[LASER_PORT];
		imageX = LASER_BMP_SZ;
		imageY = LASER_BMP_SZ;
		break;
	case WEAPON_MISSILE:
		weaponSys = &curRobot->weaponArray[MISSILE_PORT];
		imageX = MISSILE_BMP_SZ;
		imageY = MISSILE_BMP_SZ;
		break;
	default:
		return 0;
	}

	//Check aiming and energy conditions.
	if (heading < -weaponSys->maxAngle || heading > weaponSys->maxAngle)
		return 0;
	if (weaponSys->chargeEnergy < weaponSys->minEnergy) {
		weaponSys->chargeEnergy = 0;     //Trying to fire without enough energy
		return 0;                      //wastes the charged energy.
	}

	//Fill out remaining information and create the bitmap.
	weapon.type = weaponSys->type;
	weapon.owner = curRobot;
	weapon.x = curRobot->x;
	weapon.y = curRobot->y;
	weapon.heading = BoundAngle(curRobot->heading - heading);
	weapon.speed = weaponSys->speed;
	weapon.energy = weaponSys->chargeEnergy
			+ weaponSys->chargeEnergy * weaponSys->bonusEnergy;
	weapon.splashRange = weaponSys->splashRange;
	weapon.splashDamage = weaponSys->splashDamage;
	weapon.bumpValue = weaponSys->bumpValue;
	weapon.impactSound = weaponSys->impactSound;
	weapon.image = create_bitmap(imageX, imageY);

	weaponSys->chargeEnergy = 0;            //Weapon fired: reset charge energy.

	InsLastLL(weaponList, weapon);                //Put the weapon in the list.
	DrawWeaponBitmap(LastElmLL(weaponList));     //Pass the weapon for bmp draw.
	theGame.playSound[weaponSys->firingSound] = 1; //Play the weapon firing sound.

	return 1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetSystemEnergy
//
// Description: This function returns the current amount of energy in the
//              indicated system.
//
// Parameters: SYSTEM type - The system to check the power on.
//
// Returns: float - The power in the requested system.  -1 if an invalid
//                  system was specified.
//
////////////////////////////////////////////////////////////////////////////////
float GetSystemEnergy(SYSTEM type) {
	switch (type) {
	case SYSTEM_SHIELDS:
		return curRobot->shields;
	case SYSTEM_LASERS:
		return (curRobot->weaponArray[LASER_PORT].chargeEnergy);
	case SYSTEM_MISSILES:
		return (curRobot->weaponArray[MISSILE_PORT].chargeEnergy);
	default:
		return -1;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SetSystemChargeRate
//
// Description: This function sets the requested rate for a system to charge.
//
// Parameters: SYSTEM type - The system to set the charge rate for.
//             int rate - The rate in units/min at which the system
//             should charge.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void SetSystemChargeRate(SYSTEM type, int rate) {
	if (rate < 0)
		rate = 0;

	//Since sensor "rates" can't be set, just check for shields, lasers, and
	//missiles if they are within the correct charging bounds.  If not, bound
	//them and set the rate.
	switch (type) {
	case SYSTEM_SHIELDS:
		if (rate > MAX_SHIELD_CHARGE_RATE)
			rate = MAX_SHIELD_CHARGE_RATE;
		curRobot->shieldChargeRate = rate;
		return;
	case SYSTEM_LASERS:
		if (rate > MAX_LASER_CHARGE_RATE)
			rate = MAX_LASER_CHARGE_RATE;
		curRobot->weaponArray[LASER_PORT].chargeRate = rate;
		return;
	case SYSTEM_MISSILES:
		if (rate > MAX_MISSILE_CHARGE_RATE)
			rate = MAX_MISSILE_CHARGE_RATE;
		curRobot->weaponArray[MISSILE_PORT].chargeRate = rate;
		return;
	default:
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SetSystemChargePriorities
//
// Description: This function records the robot's charging priorities.
//
// Parameters: int priorities - This is an array where each system must appear
//                              once and only once or the function will not
//                              change the priorities.  For example:
//
//                  priorities[0]=SYSTEM_MISSILES;
//                  priorities[1]=SYSTEM_SHIELDS;
//                  priorities[2]=SYSTEM_SENSORS;
//                  priorities[3]=SYSTEM_LASERS;
//
// Returns: int - 1 if change was successful,
//                0 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
int SetSystemChargePriorites(SYSTEM priorities[NUM_ENERGY_SYSTEMS]) {
	int i, j;

	for (i = 0; i < NUM_ENERGY_SYSTEMS; i++) {
		if (priorities[i] < 0 || priorities[i] > NUM_ENERGY_SYSTEMS - 1) //Check if valid
			return 0;                                            //SYSTEM value.

		for (j = i + 1; j < NUM_ENERGY_SYSTEMS; j++)   //Check for duplicates in
			if (priorities[i] == priorities[j])          //the priority list.
				return 0;
	}

	for (i = 0; i < NUM_ENERGY_SYSTEMS; i++)                  //Write priorities
		curRobot->energyPriorities[i] = priorities[i];          //into the list.
	return 1;

}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetBumpInfo
//
// Description: This function reports the bump information since the bump
//              status was last checked.  Bump info is cleared after checking.
//
// Parameters: None.
//
// Returns: int - A number representing what has been bumped.
//
////////////////////////////////////////////////////////////////////////////////
int GetBumpInfo(void) {
	int tempBumpInfo = curRobot->bumped;
	curRobot->bumped = BUMP_NONE;
	return tempBumpInfo;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetGeneratorStructure
//
// Description: This function returns the remaining structure of the generator.
//
// Parameters: None.
//
// Returns: int - The remaining generator structure.
//
////////////////////////////////////////////////////////////////////////////////
int GetGeneratorStructure(void) {
	return curRobot->generatorStructure;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetGeneratorOutput
//
// Description: This function returns the remaining output of the generator
//              in energy units/min.
//
// Parameters: None.
//
// Returns: int - The output of the generator.
//
////////////////////////////////////////////////////////////////////////////////
int GetGeneratorOutput(void) {
	return curRobot->generatorStructure * GENERATOR_CAPACITY
			/ MAX_GENERATOR_STRUCTURE;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SetStatusMessage
//
// Description: This function allows the user to set the status message of the
//              robot.
//
// Parameters: char *message - String to be recorded.  Max length is
//                             STATUS_MSG_LEN (includes null char).  Messages
//                             that exceed this length will cause program
//                             termination.
//
// Bug Fixes: 19 July 2006 - Found that hasNull was initialized to 1, so calls
//                           would never fail, no matter how long the message.
//                           Additionally, the loop looking for the null char
//                           was only checking the 0th character of the string
//                           ie: message instead of message[i].
//                           Update: August 1 2006 - This part of the function
//                                   has been moved to its own function called
//                                   HasNullCharacter()
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void SetStatusMessage(char *message) {

	if (!HasNullCharacter(message, STATUS_MSG_LEN)) {
		char errorMessage[100 + MAX_NAME_LEN];        //Only create if required.
		sprintf(errorMessage, "%s did not have a properly formatted string "
				"in its call to SetStatusMessage().\n"
				"Program will end.", curRobot->name);
		AbortOnError(errorMessage);
	} else
		strcpy(curRobot->statusMessage, message);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: SendMessage
//
// Description: This function allows a robot to send a message to another robot
//              if they know its name.
//
// Parameters: char *robotName - The name of the destination robot.
//             INT_32 data - INT_32 is a platform specific type defined in
//                           competition.h and is a 32 bit integer.  This is the
//                           data being sent.
//
// Returns: int - 1 if message was successfully delivered.
//                0 if robot was not found (name wrong or robot dead).
//
////////////////////////////////////////////////////////////////////////////////
int SendMessage(char *robotName, INT_32 data) {
	ROBOT *addressee;
	if (!HasNullCharacter(robotName, MAX_NAME_LEN)) {
		char errorMessage[100 + MAX_NAME_LEN];        //Only create if required.
		sprintf(errorMessage,
				"%s did not have a properly formatted name string "
						"in its call to SendMessage().\n"
						"Program will end.", curRobot->name);
		AbortOnError(errorMessage);
	}

	if ((addressee = LookupRobotByName(robotName)) != NULL) {
		InsLastLL(addressee->mailBox, data);
		return 1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetMessage
//
// Description: This function reads a message from a robot's mailbox.  When the
//              message is read, it is deleted from the mailbox.
//
// Parameters: INT_32 *data - A pointer to a 32-bit integer that will be filled
//                            out with the message from the mailbox.
//
// Returns: int - 0 if the mailbox was empty and no data was returned.
//                1 if the mailbox contained a message which was returned.
//
////////////////////////////////////////////////////////////////////////////////
int GetMessage(INT_32 *data) {
	if (!IsEmptyLL(curRobot->mailBox))                //There are messages...
			{
		*data = *(int*) FirstElmLL(curRobot->mailBox);    //Get the data out.
		DelElmLL(FirstElmLL(curRobot->mailBox));       //Delete the message.
		return 1;
	}
	return 0;                                        //No messages.
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: AbortOnError
//
// Description: This function reports an error to the user and exits
//              the program.
//
// Parameters: char *message - The error message to show the user.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void AbortOnError(char *message) {
	allegro_message(message);
	exit(-1);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: GetRandomNumber
//
// Description: This function returns a random integer in the range
//              0<=x<upperBound.
//
// Parameters: int upperBound - The upper bound of numbers.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
float GetRandomNumber(int upperBound) {
	return upperBound * rand() / (RAND_MAX + 1.0);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: HasNullCharacter
//
// Description: This function checks if a string contains a null character
//              within a specified length.
//
// Parameters: char *string - The string to test
//             int   numChars - The maximum length of the string.  If properly
//                              formatted, a null character will exist within
//                              the string.
//
// Returns: int - 1 if the string contains a null character
//                0 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
static int HasNullCharacter(char *string, int numChars) {
	int i;

	for (i = 0; i < numChars - 1; i++)
		if (string[i] == '\0')
			return 1;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: LookupRobotByName
//
// Description: This function finds the pointer of another robot in the linked
//              list by using the name of the robot.
//
// Notes: This function simply iterates through the linked list looking for the
//        named robot.  If there is ever a significant number of robots in the
//        competition (and I don't know what that is, but I suspect over 100 or
//        so) this would no longer suffice.  Instead, a hash would likely have
//        to be created to quickly find the robot in question.
//
// Parameters: char *name - The name of the robot to find.
//
// Returns: ROBOT* - Pointer to the robot.  If the robot was not found in the
//                   list (because the name is bad or the robot is dead) NULL
//                   is returned.
//
////////////////////////////////////////////////////////////////////////////////
static ROBOT *LookupRobotByName(char *name) {
	ROBOT *theRobot;

	ForeachLL_M(robotList, theRobot)
	{
		if (!strcmp(theRobot->name, name))
			return theRobot;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: BoundAngle
//
// Description: This is a short utility function to ensure an angle is within
//              a range of 0<=angle<360
//
// Parameters: int angle - The angle to be bounded.
//
// Returns: int - The bounded angle.
//
////////////////////////////////////////////////////////////////////////////////
static int BoundAngle(int angle) {
	while (angle > 360)
		angle -= 360;
	while (angle < 360)
		angle += 360;
	return angle;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: ChooseRandomLocation
//
// Description: This function chooses a random x/y location for a robot.  The
//              location will be at least MIN_RANDOM_DIST cm away from each
//              other robot The function will make 1000 attempts to find a non-
//              colliding location, and then give up.
//
// Parameters: ROBOT *robot - The robot to move.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
static void ChooseRandomLocation(ROBOT *robot) {
	int dx, dy, success, attempts = 1000;
	double dist;
	ROBOT *tmpRob;                         //Robot pointer iterator.

	while (attempts--) {
		//Choose a random location within the arena.
		robot->x = GetRandomNumber(
		ARENA_WIDTH_CM - SHIELD_RAD_CM * 2) + SHIELD_RAD_CM;
		robot->y = GetRandomNumber(
		ARENA_HEIGHT_CM - SHIELD_RAD_CM * 2) + SHIELD_RAD_CM;

		if (SizeLL(robotList) == 1) //No need to check for collisions if this is
			return;                   //the only robot.

		//Iterate through each other robot.  If the distance is less than that
		//allowed, record a failure and break to start next attempt.
		success = 1;
		for (tmpRob = FirstElmLL(robotList); IsElmLL(tmpRob); tmpRob =
				NextElmLL(tmpRob)) {
			if (tmpRob != robot)           //Don't check if too close to itself.
					{
				dx = robot->x - tmpRob->x;
				dy = robot->y - tmpRob->y;
				dist = sqrt(pow(dx, 2) + pow(dy, 2));
				if (dist < (SHIELD_RAD_CM * 2 + MIN_RANDOM_DIST)) {
					success = 0;
					break;
				}
			}
		}
		if (success)
			return;
	}

	AbortOnError("ChooseRandomLocation() could not find a place for this robot "
			"that did not collide with another robot.\n"
			"Program will end.");
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: PlaySounds
//
// Description: This function plays sounds based on the requests in the game's
//              state.  Once fulfilled, these requests are turned off.
//
// Parameters: GAME *game - Pointer to the game's state.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
static void PlaySounds(GAME *game) {
	int i;
	for (i = 0; i < NUM_SOUNDS; i++)
		if (game->playSound[i]) {
			play_sample(game->sounds[i], 255, 128, 1000, 0);
			game->playSound[i] = 0;
		}
}
