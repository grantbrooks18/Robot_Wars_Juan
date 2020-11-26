////////////////////////////////////////////////////////////////////////////////
//
// File: competition.h
//
// Description:
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//                   1 Aug 2006 - Many revisions over the last month have
//                                been made, but all comments were put in
//                                competition.c.  Of note is the standardization
//                                of weapon constants, the creation of a new
//                                weapon structure (for fired weapons), changes
//                                to the old weapon structure for a weapon on a
//                                robot (now called WEAPON_SYSTEM) and the
//                                addition of the platform-specific constants.
//                   6 Sep 2006 - Removed robot centering constants.  Robots are
//                                now centered in their shield bitmap
//                                dynamically based on robot image sizes.
//                                Note this expects robot images to be an ODD
//                                number of pixels in each dimension to appear
//                                centered!
//                              - New constant MIN_RANDOM_DIST added to enhance
//                                random start locations.
//                  10 Apr 2007 - New constants in speed section related to
//                                turbo boost feature. This also changed the
//                                sounds constants.
//                                Changed constant BUMP_LASER so it would be
//                                felt by the bump sensor.
//
////////////////////////////////////////////////////////////////////////////////
#ifndef COMPETITION_HEADER            //Protect competition header with
#define COMPETITION_HEADER 1          //preprocessor directives as it is
//included in multiple files.
#include <allegro.h>
#include "ll.h"                      //Linked list library.
//Platform-Specific Constants
#define INT_32             int       //When we need a 32-bit int specifically.
//Math values
#define PI                 3.1415926535897932
#define DEG_PER_RAD        1.7453292519943295E-2   //Equal to: 2*PI/360
#define RAD_PER_DEG        5.7295779513082320E1    //Equal to: 360/2*PI
//Game Settings
#define MAX_ROBOTS         4
#define CALCS_PER_SEC     60       //Number of calculations/s.
#define CALCS_PER_MIN   3600       //For use in energy charging (units are/min)
#define ORDER_FREQ        15       //How often player order func's are called
//in CALCS.
#define TURN_TIME        250       //Time between player order function calls.
//Derived from CALCS_PER_SEC and ORDER_FREQ
#define ARENA_WIDTH_CM   375
#define ARENA_HEIGHT_CM  375
#define STATUS_MSG_LEN   150
#define MIN_RANDOM_DIST    3       //When using random locations, min distance
//between robots, in cm.
//#define SHOW_COLLISIONS  1       //If defined, draw radar<->robot collisions.
#define SHOW_PARTICLES     1       //If defined, draw particle effects.
//Weapon Settings
#define MAX_WEAPONS        2

#define LASER_PORT         1
#define LASER_BMP_SZ      11
#define LASER_SPEED      200       //Speed in cm/s.  Clears screen in 1.8s.
#define LASER_MAX_ANGLE  360       //Lasers can shoot at any angle.
#define LASER_SPLASH_RANGE       0 //Currently, lasers have no splash damage.
#define LASER_SPLASH_DAMAGE      0 //     "        "     "   "    "     "
#define MISSILE_PORT       0
#define MISSILE_BMP_SZ    17
#define MISSILE_SPEED     50       //Speed in cm/s. Max time to clear sreen 7.5s
#define MISSILE_MAX_ANGLE 90
#define MISSILE_X_OFFSET   4       //Missile offset from the "static" image
#define MISSILE_Y_OFFSET   2       //onto the larger bitmap for each WEAPON.
#define MISSILE_SPLASH_RANGE    50 //If robot is near an explosion it gets hurt.
#define MISSILE_SPLASH_DAMAGE   50 //This is how much it gets hurt.
//Sensor Settings
#define MAX_SENSORS        4
#define MIN_RADAR_ARC     10
#define MAX_RADAR_ARC     45
#define RADAR_MAX_RANGE  100       //For shield size, plus three robot shield
//diameters, +20 =16 + 32 + 32 + 20 (CM)
#define RADAR_MIN_RANGE   32       //Must extend out at least 1/2 shield radius
//PAST the shields=16 + 16
#define RADAR_WRKIMG_PX  403       //=Radar range*2(both directions)
// *2(pixels/cm) +1 for centering + 2 to be
// larger than any radar arc.
#define RADAR_IMAGE_PX   200       //This is the image cropped from the working
//image. It is associated with the sensor.
#define RANGE_MAX_RANGE  125       //Range sensor max range.
//Free Sensor Settings             //"Free" meaning they come pre-installed.
#define GPS_COST           2       //Energy cost from shield caps for GPS test.
#define BUMP_NONE       0x00       //Reset value for after bump check.
#define BUMP_WALL       0x01       //Results of bumping into a wall,
#define BUMP_ROBOT      0x02       //robot,
#define BUMP_MISSILE    0x04       //missile,
#define BUMP_LASER      0x08       //or laser.
//Screen Drawing Constants
#define PX_PER_CM          2
#define SCREEN_WIDTH    1024
#define SCREEN_HEIGHT    768
#define COLOR_DEPTH       16       //Use 16 bit color.  32 bits render too slow
//when using transparent bitmaps (radar!)
#define ARENA_WIDTH_PX   750
#define ARENA_HEIGHT_PX  750
#define WALL_HIT_CORRECT   0.5     //"Magic" wall collision detection number.
#define BORDER_WIDTH       9       //Border size between screen elements, in px.
#define SHIELD_RAD_CM     16       //Shield range from robot center.
#define SHIELD_RAD_PX     32       //The above #, multiplied by PX_PER_CM
#define SHIELD_BMP_SZ     65       //Shield bmp size is diameter+1 to allow for
//easy centering on the "odd" pixel.
#define SHIELD_THICK_PX    5       //Shield thickness in pixels.
#define STATUS_CHAR_COLS  30       //Number of default font characters in the
//status area.
#define STATUS_CHAR_ROWS   4       //Number of lines in custom message area.
#define MAX_NAME_LEN      20       //Longest robot name.
//Motion Constants
#define MAX_SPEED         11.76    //Speed in cm/s.
#define TURBOBOOST_TIME    3       //Turbo boost time in seconds.
#define TURBOBOOST_COST  100       //One-time cost to fire turbo boost.
#define TURBOBOOST_SPEED 200       //Percentage speed added to each tread.
#define SHIELD_CROSS_SPD  45.0     //Speed imparted when shields collide.
#define FRIC_SLOW_RATE    30.0     //Impulse speed drop per second.
#define TREAD_DISTANCE    15.5     //Distance between treads in cm.
//System Energy Constants
#define NUM_ENERGY_SYSTEMS          4
#define GENERATOR_CAPACITY       1400    //All at max charge is drain of 1750!
#define MAX_SHIELD_ENERGY        1000
#define START_SHIELD_ENERGY       600
#define MAX_SHIELD_CHARGE_RATE    600
#define MAX_MISSILE_CHARGE_RATE   600
#define MAX_MISSILE_ENERGY        100    //Max firing rate of 4/min, every 15s.
#define MIN_MISSILE_ENERGY        100
#define MISSILE_ENERGY_BONUS        0.5  //Energy multiplier on firing.
#define MAX_LASER_CHARGE_RATE     500
#define MAX_LASER_ENERGY           50    //Can fire 10x/min at full power.
#define MIN_LASER_ENERGY           20    //Max fire rate is 25/min->every 2.4s
#define LASER_ENERGY_BONUS          0    //Energy multiplier on firing.
#define RANGE_SENSOR_ENERGY_COST   50
#define RADAR_SENSOR_ENERGY_COST   75

//System Damage Constants
#define SHIELDS_LEAK_THRESHOLD    600  //Below 600 points, shields leak damage.
#define MAX_GENERATOR_STRUCTURE   500
#define SHIELD_CROSS_DAMAGE       200  //Damage caused when shields collide.
//Drawing Resources
#define IMG_UI      "..\\resources\\images\\background.bmp"
#define IMG_ROBOT   "..\\resources\\images\\basic_robot.bmp"
#define IMG_MISSILE "..\\resources\\images\\missile.bmp"
#define COLOR_TRANS  makecol(255, 0, 255)
#define COLOR_ARENA  makecol(127,127,127)

//Sound Resources
#define NUM_SOUNDS        8
#define SND_LASER_FIRE    0
#define SND_LASER_HIT     1
#define SND_MISSILE_FIRE  2
#define SND_MISSILE_HIT   3
#define SND_FIGHT         4
#define SND_ROBOTS_HIT    5
#define SND_ROBOT_EXPLODE 6
#define SND_TURBOBOOST    7
//Note that an array of all the sound file names is also required
//and is found as a global in competition.c

typedef enum {
	ROBOT_RED,
	ROBOT_GREEN,
	ROBOT_BLUE,
	ROBOT_YELLOW,
	ROBOT_PURPLE,
	ROBOT_TURQUOISE,
	ROBOT_WHITE
} ROBOTCOLORS;

typedef enum {
	SENSOR_RADAR, SENSOR_RANGE, SENSOR_NONE
} SENSORTYPE;

typedef enum {
	WEAPON_MISSILE, WEAPON_LASER, WEAPON_NONE
} WEAPONTYPE;

typedef enum {
	SYSTEM_SHIELDS, SYSTEM_SENSORS, SYSTEM_LASERS, SYSTEM_MISSILES
} SYSTEM;

typedef enum {
	GS_SETUP, GS_INTRO, GS_FIGHTING, GS_OVER
} GAMESTATE;

typedef struct {
	SENSORTYPE type;
	int angle;
	int width;
	int range;
	int on;             //User can set on/off status of sensors to save power.
	int powered;        //If not enough power available, this will be 0.
	int data;
	int drawX;          //Required for sensor radar bitmaps as their draw point
	int drawY;        //within the image constantly fluctuates based on heading.
	BITMAP *image;      //For sensors that cast a "sensor shadow".
} SENSOR;

typedef struct          //This represents a weapon system on the robot.
{
	WEAPONTYPE type;
	int maxAngle;      //Maximum angle weapon can be aimed at.
	int minEnergy;     //Minimum energy required for firing.
	int maxEnergy;     //Maximum energy weapon can hold.
	float bonusEnergy;   //Bonus energy on firing: a percentage of fired energy.
	int splashRange;   //Splash damage range.
	int splashDamage;  //Splash damage.
	float speed;         //Speed of weapon when fired.
	int chargeRate;    //Rate of energy requested for charging in units/min.
	float chargeEnergy;  //Energy being built up for firing.
	int bumpValue;     //A bump value to be recored in the bump sensor.
	int firingSound;   //Index into the sounds for firing.
	int impactSound;   //Index into the sounds for impact.
} WEAPON_SYSTEM;

typedef struct {
	void (*ActionsFunction)(int);            //The robot's turn function.
	char *name;
	int number;                   //Used for sorting text in status area.
	int color;                               //Robot's clr in RGB format.
	int energyPriorities[NUM_ENERGY_SYSTEMS];       //Determines egy allocation.
	float x;                                   //Robot's x location.
	float y;                                   //Robot's y location.
	float heading;          //Robot's heading. Uses standard math co-ords.
	int leftTreadSpeed;
	int rightTreadSpeed;
	int turboTime;        //Turbo boost time remaining.
	float impulseHeading;   //Heading and speed imparted on the robot
	float impulseSpeed;     //from an external source ie: explosion
	int bumped;           //Did robot run into wall or another robot?
	float shields;
	int shieldChargeRate;
	int generatorStructure;
	float damageBank;                     //Records damage to be applied.
	char statusMessage[STATUS_MSG_LEN];  //Robot message to be printed.
	SENSOR sensorArray[MAX_SENSORS];       //Array of pointers to sensors.
	WEAPON_SYSTEM weaponArray[MAX_WEAPONS];       //An array of weapons.
	t_LL mailBox;                        //Incoming messages.
	BITMAP *graphic;                       //Graphic of just the robot.
	BITMAP *image;                         //Robot image with shield.
} ROBOT;

typedef struct           //A WEAPON is a weapon system that has been fired.
{
	WEAPONTYPE type;
	ROBOT *owner;     //Ensures weapons don't explode on their owners!
	float x;
	float y;
	float heading;
	float speed;
	float energy;
	int splashRange;
	int splashDamage;
	int bumpValue;
	int impactSound;
	BITMAP *image;
} WEAPON;

typedef struct {
	GAMESTATE state;              //State of the game.
	int useSounds;
	SAMPLE *sounds[NUM_SOUNDS];
	int playSound[NUM_SOUNDS];
} GAME;

typedef struct {
	float x;
	float y;
	float heading;
} GPS_INFO;

//Interfaces to set up the game.
void Fight();
void InitCompetition();
void RegisterRobot(char *robotName, ROBOTCOLORS color,
		void (*robotActions)(int), void (*configureRobot)(void),
		char *customImage, int x, int y, float heading);
int AddSensor(int port, SENSORTYPE type, int angle, int width, int range);
void EndCompetition();

//Interfaces to control the robot.
void SetMotorSpeeds(int motorASpd, int motorBSpd);
int TurboBoost(void);
int IsTurboOn(void);
int GetGPSInfo(GPS_INFO *gpsData);
int GetSensorData(int port);
void SetSensorStatus(int port, int status);
int FireWeapon(WEAPONTYPE type, int heading);
float GetSystemEnergy(SYSTEM type);
void SetSystemChargeRate(SYSTEM type, int rate);
int SetSystemChargePriorites(SYSTEM priorities[NUM_ENERGY_SYSTEMS]);
int GetBumpInfo(void);
int GetGeneratorStructure(void);
int GetGeneratorOutput(void);
void SetStatusMessage(char *message);

//Other useful interfaces.
void AbortOnError(char *message);
float GetRandomNumber(int upperBound);

#endif                                           //End header file "protection".
