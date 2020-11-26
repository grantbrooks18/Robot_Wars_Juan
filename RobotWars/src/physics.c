////////////////////////////////////////////////////////////////////////////////
//
// File: physics.c
//
// Description:
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 2 Apr 2006 - Created
//                  19 July 2006 - Major rework to change robot data from array
//                                 to linked list.  LL docs found at:
//                                 http://cmp.felk.cvut.cz/~matas/surrey/
//                                                             software/LL.html
//                  23 July 2006 - Changed the way weapons function to have them
//                                 exist as systems in the robot before firing,
//                                 but create a weapon in a linked list after
//                                 firing.  This changed MoveWeapons() as it now
//                                 receives only the list of weapons, as well as
//                                 CheckWeaponCollisions() as it now receives
//                                 the list of weapons in addition to the list
//                                 of robots.  The primary benefit of this
//                                 change is that it is now much easier to
//                                 iterate through weapons.  In particular, this
//                                 change greatly simplified the readability
//                                 and ease of modification of
//                                 CheckWeaponCollisions()
//                 10 April 2007 - Changed MoveRobots() to account for addition
//                                 of TurboBoost.
//
////////////////////////////////////////////////////////////////////////////////
#include <math.h>               //For cos, sin
#include <stdlib.h>             //For abs()
#include "physics.h"
#include "particles.h"
#include "graphics.h"

//Internal helper prototypes
void CreateWeaponParticleBurst(WEAPONTYPE type, int x, int y);
//void CreateMissileParticleBurst(int x, int y);
void CreateRobotsCollideParticleBurst(int x, int y);
void CreateRobotExplodeParticleBurst(int x, int y);

////////////////////////////////////////////////////////////////////////////////
//
// Function: MoveRobots
//
// Description: This function performs the physics of motion on the robots.
//              It should be called CALCS_PER_SEC times each second to ensure
//              that the distances travelled reflect the actual amount of time
//              passed.
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void MoveRobots(t_LL listOfRobots) {
	double rotAngle, startAngle, lTreadDist, rTreadDist, x, y, innerRad, midRad,
			u, v, radians, dist;
	int lTreadSpeed, rTreadSpeed;
	ROBOT *robot;                                               //List iterator.

	//Peform motion based on "sliding" movement not related to treads.

	ForeachLL_M(listOfRobots, robot)
	//Here, robot is a pointer
	{                                                 //that uses the Foreach
		if (robot->impulseSpeed != 0)                 //macro to iterate through
				{                                     //all robots in the linked
			radians = robot->impulseHeading * DEG_PER_RAD; //list.  The loop ends after
			dist = robot->impulseSpeed / CALCS_PER_SEC;    //Each robot has been
			robot->x += dist * cos(radians);                  //examined.
			robot->y += dist * sin(radians);
			robot->impulseSpeed -= FRIC_SLOW_RATE / CALCS_PER_SEC;
			if (robot->impulseSpeed < 0)
				robot->impulseSpeed = 0;
		}
	}

	ForeachLL_M(listOfRobots, robot)
	{
		//Start by extracting speed of each tread and calculate its distance
		//travelled for use in later calculations.
		if (robot->turboTime)                                  //Add extra speed
		{                                                     //and particles if
			lTreadSpeed = robot->leftTreadSpeed + TURBOBOOST_SPEED; //turbo boost is
			rTreadSpeed = robot->rightTreadSpeed + TURBOBOOST_SPEED;  //firing.
			robot->turboTime--;
#ifdef SHOW_PARTICLES
			{
				int boostClr, i;
				boostClr = makecol(255, 80, 80);
				for (i = 0; i < 5; i++)
					AddParticle(robot->x, robot->y, boostClr,
							GetRandomNumber(360), GetRandomNumber(1), 4);
			}
#endif
		} else {                                                //No turbo, just
			lTreadSpeed = robot->leftTreadSpeed;                //regular speed.
			rTreadSpeed = robot->rightTreadSpeed;
		}
		lTreadDist = MAX_SPEED * lTreadSpeed / (100.0 * CALCS_PER_SEC);
		rTreadDist = MAX_SPEED * rTreadSpeed / (100.0 * CALCS_PER_SEC);

		//When calculating treaded motion there are 3 main cases to consider:
		//   - Treads equal speed
		//   - Left or right tread stopped
		//   - Both treads moving
		//Of the above, equal speed is a unique case which is handled on its own.
		//The other two cases first calculate their starting angle (from the center
		//of the circle they're following), the number of degrees of rotation
		//around the circle, and the radius from that circle center to the mid-point
		//between the two treads to be used in the shared code.
		//--------------------------------------------------------------------------

		if (lTreadSpeed == rTreadSpeed)             //Special case: tread speeds
				{                                  //equal. Use sin/cos on basic
			radians = robot->heading * DEG_PER_RAD;    //right-angle triangle to
			robot->x = robot->x + lTreadDist * cos(radians);   //get new x/y
			robot->y = robot->y + lTreadDist * sin(radians);   //locations.
		} else {
			if (rTreadSpeed == 0)           //Right tread stopped. Robot rotates
					{                          //around right tread so midRad is
				midRad = TREAD_DISTANCE / 2; //half the dist between the treads.
				rotAngle = -lTreadDist * 360.0 / //rotAngle is based on arc dist
						(2 * PI * TREAD_DISTANCE); //followed on the circle for:
												   //    deg/360  =  arcdist/2*PI*r
				startAngle = robot->heading + 90; //Turning right: add 90 to get angle
			}                                 //from circle center to the robot.
			else if (lTreadSpeed == 0) {                  //This section same as
				midRad = TREAD_DISTANCE / 2;                //rTreadSpeed=0, but
				rotAngle = rTreadDist * 360.0 / (2 * PI * TREAD_DISTANCE); //turning to the left.
				startAngle = robot->heading + 270;
			} else                                       //Both treads moving...
			{
				if (abs(lTreadSpeed) > abs(rTreadSpeed)) //Left faster than right.
						{                              //Get the radius from the
					innerRad = rTreadDist * TREAD_DISTANCE / //circle center to inner
							(lTreadDist - rTreadDist); //tread and use it to get
					midRad = innerRad + TREAD_DISTANCE / 2; //distance to mid-tread.
					rotAngle = -rTreadDist * 360.0 / (2 * PI * innerRad); //Find the angle moved
																		  //around the circle.
					startAngle = robot->heading + 90; //Transform heading to angle
													  //from circle center to robot.
				} else                                  //Same as previous case,
				{                                     //but for turning to left.
					innerRad = lTreadDist * TREAD_DISTANCE
							/ (rTreadDist - lTreadDist);
					midRad = innerRad + TREAD_DISTANCE / 2;
					rotAngle = lTreadDist * 360.0 / (2 * PI * innerRad);
					startAngle = robot->heading + 270;

				}
			}
			radians = startAngle * DEG_PER_RAD; //Use the starting angle from the
			x = midRad * cos(radians);           //circle center and distance to
			y = midRad * sin(radians);       //the robot to calculate a starting
			radians = rotAngle * DEG_PER_RAD; //x and y location.  Using that, plus
			u = x * cos(radians) - y * sin(radians); //the rotation angle, find the new
			v = y * cos(radians) + x * sin(radians); //location (u & v).  This calculation
			robot->x += u - x;              //was performed as if the origin was
			robot->y += v - y;                //at (0,0), so find the difference
			robot->heading += rotAngle;      //between the two points and update
											 //the robot location.

			if (robot->heading >= 360)            //Fix robot heading to keep
				robot->heading -= 360;           //it below 360deg and positive.
			else if (robot->heading < 0)
				robot->heading += 360;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: CheckRobotCollisions
//
// Description:
//              Note the constant: WALL_HIT_CORRECT.  This is a "magic"
//              correction value to bring the robots exactly to the arena's
//              edge on a pixel-by-pixel basis.  The value comes from the fact
//              that the robot/shield bitmap is an odd number to draw nicely
//              around a center point, and is adjusted (I think) by the pixels
//              per centimeter.  It *may* be as simple as 1/PX_PER_CM, but some
//              tests would be required to prove this.
//              The value is added to the y check because the drawing is
//              inverted in the y direction on the bitmap.
//
// Parameters: t_LL listOfRobots - The linked list of robots.

//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void CheckRobotCollisions(GAME *game, t_LL listOfRobots) {
	int hitWall;
	double x, y, dist, angle;
	ROBOT *robot, *robot2;

	//First, ensure all robots are in the arena boundaries:
	ForeachLL_M(listOfRobots, robot)
	{
		hitWall = 0;
		if (robot->x < SHIELD_RAD_CM) {
			robot->x = SHIELD_RAD_CM;
			hitWall = 1;
		} else if (robot->x > ARENA_WIDTH_CM - SHIELD_RAD_CM - WALL_HIT_CORRECT) {
			robot->x = ARENA_WIDTH_CM - SHIELD_RAD_CM - WALL_HIT_CORRECT;
			hitWall = 1;
		}

		if (robot->y < SHIELD_RAD_CM + WALL_HIT_CORRECT) {
			robot->y = SHIELD_RAD_CM + WALL_HIT_CORRECT;
			hitWall = 1;
		} else if (robot->y > ARENA_WIDTH_CM - SHIELD_RAD_CM) {
			robot->y = ARENA_WIDTH_CM - SHIELD_RAD_CM;
			hitWall = 1;
		}
		if (hitWall)                                     //If robot hit the wall
			robot->bumped = robot->bumped | BUMP_WALL;  //then set its bump val.
	}

	//Next, check for collisions between robots.  Note the following dual loop
	//structure always has the second loop start one element further into the
	//list than where the outer loop is, so no two robots are tested twice.
	//IsElmLL() checks to see if that postion in the list is actually a robot.
	for (robot = FirstElmLL(listOfRobots); IsElmLL(robot);
			robot = NextElmLL(robot))
		for (robot2 = NextElmLL(robot); IsElmLL(robot2);
				robot2 = NextElmLL(robot2)) {
			x = robot->x - robot2->x;
			y = robot->y - robot2->y;
			dist = sqrt(pow(x, 2) + pow(y, 2));
			if (dist == 0)
				AbortOnError(
						"CheckRobotCollisions() was about to divide by zero.\n"
								"Program will end.");
			if (dist < SHIELD_RAD_CM * 2) {
				game->playSound[SND_ROBOTS_HIT] = 1;
				CreateRobotsCollideParticleBurst(x / 2 + robot2->x,
						y / 2 + robot2->y);
				robot->impulseSpeed = SHIELD_CROSS_SPD;
				robot2->impulseSpeed = SHIELD_CROSS_SPD;
				if (y >= 0)                             //Get angle from j to i.
					angle = acos(x / dist);             //COS requires no change
				else
					//from 0 to 180 deg, but use
					angle = 2 * PI - acos(x / dist);    //360-angle for 180-360.
				angle *= RAD_PER_DEG;
				robot->impulseHeading = angle;
				robot2->impulseHeading = angle + 180;
				robot->damageBank += SHIELD_CROSS_DAMAGE;   //Record damage from
				robot2->damageBank += SHIELD_CROSS_DAMAGE;   //crossing shields.
				robot->bumped = robot->bumped | BUMP_ROBOT;    //Record bump for
				robot2->bumped = robot2->bumped | BUMP_ROBOT;    //bump sensors.
			}
		}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: MoveWeapons
//
// Description: This function performs the physics of motion of the weapons.
//              It should be called CALCS_PER_SEC times each second to ensure
//              that the distances travelled reflect the actual amount of time
//              passed.
//              Currenly, this function is pretty simple as the weapons only
//              travel ballistically.
//
// Parameters: t_LL listOfWeapons - The linked list of weapons.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void MoveWeapons(t_LL listOfWeapons) {
	int particleCount;
	double dist, radians;
	WEAPON *weapon;

	ForeachLL_M(listOfWeapons, weapon)
	{
		radians = weapon->heading * DEG_PER_RAD;
		dist = weapon->speed / CALCS_PER_SEC;
		weapon->x += dist * cos(radians);
		weapon->y += dist * sin(radians);
#ifdef SHOW_PARTICLES
		if (weapon->type == WEAPON_MISSILE) {
			for (particleCount = 0; particleCount < 3; particleCount++)
				AddParticle(weapon->x, weapon->y, makecol(200, 200, 200),
						weapon->heading + 150 + GetRandomNumber(60),
						2 + GetRandomNumber(3), 5);

		}
#endif
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: CheckWeaponCollisions
//
// Description: This function checks to see if any weapons have hit a robot
//              or an arena boundary.  If so, the weapon is removed and damage
//              is applied.  This function used to be grossly complicated, but
//              with the change of weapons being maintained in their own list,
//              it is much easier to understand.
//              The implementation of this function follows this loop sequence:
//
//              Loop through all weapons (*weapon)
//                Loop through all robots (*robot)
//                  See if weapon collides with a robot (that didn't fire it!)
//                  If so, apply damage to robot bank, record bump value.
//                  Record which robot was hit (*hitRobot)
//                Check to see if the weapon hit a wall.  If so, set hitWall=1.
//                If the weapon hit a robot or wall...
//                  Play the collision sound.
//                  Draw the weapon's particle burst.
//                  Loop through each robot (*robot2)
//                    Apply splash damage from weapon to each robot2 in range,
//                    except for hitRobot as it shouldn't receive splash damage.
//                  Delete the weapon, freeing its bitmap image first.
//
// Parameters: GAME *game - Info on game. Used to record sound play requests.
//             t_LL listOfRobots - The linked list of robots.
//             t_LL listOfWeapons - The linked list of weapons.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void CheckWeaponCollisions(GAME *game, t_LL listOfRobots, t_LL listOfWeapons) {
	int robX, robY, weaponX, weaponY;
	int hitWall;
	float dx, dy, dist;
	ROBOT *robot, *robot2, *hitRobot;
	WEAPON *weapon, *nextWeapon;

	SafeForeachLL_M(listOfWeapons, weapon, nextWeapon)
	//Use "safe" loop as weapon
	{                                                //may be deleted during the
		nextWeapon = NextElmLL(weapon);                    //loop if it impacts.
		hitRobot = NULL;
		hitWall = 0;

		//Get weapon coordinates, in pixels.
		weaponX = weapon->x * PX_PER_CM - weapon->image->w / 2;
		weaponY = ARENA_HEIGHT_PX - weapon->y * PX_PER_CM
				- weapon->image->h / 2;

		ForeachLL_M(listOfRobots, robot)
		//Loop through each robot.
		{
			if (weapon->owner != robot)                  //Don't impact of robot
					{                                   //that fired the weapon.
				//Get robot coordinates, in pixels.
				robX = robot->x * PX_PER_CM - SHIELD_BMP_SZ / 2;
				robY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM
						- SHIELD_BMP_SZ / 2;

				if (ImagesCollide(robot->image, robX, robY, //See if weapon/robot
						weapon->image, weaponX, weaponY)) //images intersect.
						{
					hitRobot = robot;                              //If so, hit!
					robot->damageBank += weapon->energy;        //Record who was
					robot->bumped = robot->bumped | weapon->bumpValue; //hit, amount of
					break;                                     //damage, and set
				}                                            //bump value in the
			}                                                     //robot's bump
		}                                                        //sensor.

		//Check to see if weapon has hit a wall.
		if (weaponX < 0 || weaponX + weapon->image->w - 1 > ARENA_WIDTH_PX - 1
				|| weaponY < 0
				|| weaponY + weapon->image->h - 1 > ARENA_HEIGHT_PX - 1)
			hitWall = 1;

		if (hitRobot != NULL || hitWall)       //Weapon hit a robot or the wall,
				{                                     //so play the impact sound
			game->playSound[weapon->impactSound] = 1; //and draw the particle burst.
			CreateWeaponParticleBurst(weapon->type, weapon->x, weapon->y);

			ForeachLL_M(listOfRobots, robot2)
			//Since weapon impacted, check
			{                                    //all robots for splash damage.
				if (robot2 == hitRobot)        //But if the weapon exploded on a
					continue;                          //robot, don't splash it!
				dx = robot2->x - weapon->x;
				dy = robot2->y - weapon->y;
				dist = sqrt(pow(dx, 2) + pow(dy, 2));
				if (dist < weapon->splashRange)               //If in range, add
					robot2->damageBank += weapon->splashDamage; //splash damage to bank.
			}

			if (weapon->image != NULL)                  //Delete weapon.
				destroy_bitmap(weapon->image);        //Free weapon image first!
			DelElmLL(weapon);                         //Remove it from the list.
			break;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: UpdateSensorData
//
// Description: This function updates sensor information for sensors that change
//              based on the situation around them.
//              RADAR sensor data is 1 if a qualifying object is found in the
//                radar, and 0 otherwise.
//              RANGE sensor data is 0 if no qualifying object is found within
//                the range of the range sensor.  Otherwise, it is the range
//                to the detected object.
//
//              In either case, the data is set to -1 if the sensor is off
//              or unpowered.
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void UpdateSensorData(t_LL listOfRobots) {
	int i, robX, robY, sensX, sensY;
	double angle, radians;
	ROBOT *robot, *robot2;
	int collidedData[5];             //Used to pass data back and forth with
									 //the callback function of do_line(). See the
									 //description of CheckPixel() for more info.

	collidedData[4] = (int) listOfRobots;  //Write the address of the list
										   //into collidedData for CheckPixel.

	ForeachLL_M(listOfRobots, robot)
		for (i = 0; i < MAX_SENSORS; i++) {
			if (!robot->sensorArray[i].on || !robot->sensorArray[i].powered) {
				robot->sensorArray[i].data = -1;           //If sensor is off or
				continue;                              //unpowered, don't bother
			}                                           //updating its data.
			switch (robot->sensorArray[i].type) {
			case SENSOR_RADAR:                      //For radar sensors, iterate
				robot->sensorArray[i].data = 0;   //through each other robot and
				ForeachLL_M(listOfRobots, robot2)
					//see if the sensor bitmap
					if (robot != robot2)           //collides with those robots.
							{
						//This section calculates the top left location of the robot and
						//sensor bitmaps in arena co-ordinates.  This info is passed to
						//the function ImagesCollide to see if those images have any
						//non-transparent pixels which overlap.
						robX = robot2->x * PX_PER_CM - SHIELD_BMP_SZ / 2;
						robY = ARENA_HEIGHT_PX - robot2->y * PX_PER_CM
								- SHIELD_BMP_SZ / 2;
						sensX = robot->x * PX_PER_CM
								- robot->sensorArray[i].drawX;
						sensY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM
								- robot->sensorArray[i].drawY;
						if (ImagesCollide(robot2->image, robX, robY,
								robot->sensorArray[i].image, sensX, sensY))
							robot->sensorArray[i].data = 1;
					}
				break;
			case SENSOR_RANGE:
				//The range sensor returns the distance to the first encountered
				//object (wall or robot) based on the following algorithm:
				// -Find the start point of the line (1) and the end point (2)
				//  based on the angle of the sensor and its detection range.
				// -Call do_line() with the line calculated in (1) and (2) which
				//  will use the callback function CheckPixel() for each point on the
				//  line(3).
				// -CheckPixel() will fill the array collidedData with information
				//  specifying if there was a collision, and if so, at what x and y
				//  co-ordinates.  Use this information (4) to determine the data
				//  in the range sensor.  Record max range (5) if no collision found.
				angle = -robot->sensorArray[i].angle + robot->heading;
				radians = angle * DEG_PER_RAD;
				robX = robot->x * PX_PER_CM;                                 //1
				robY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM;
				sensX =
						robX
								+ cos(radians) * robot->sensorArray[i].range
										* PX_PER_CM;     //2
				sensY =
						robY
								- sin(radians) * robot->sensorArray[i].range
										* PX_PER_CM;
				collidedData[0] = 0;
				collidedData[1] = (int) robot;
				do_line(NULL, robX, robY, sensX, sensY,                      //3
						(int) collidedData, CheckPixel);
				if (collidedData[0] != 0)                                    //4
					robot->sensorArray[i].data = sqrt(
							pow(robX - collidedData[2], 2)
									+ pow(robY - collidedData[3], 2))/PX_PER_CM;
				else
					robot->sensorArray[i].data = robot->sensorArray[i].range; //5
				break;
			case SENSOR_NONE:
			default:
				break;
			}
		}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: UpdateEnergySystems
//
// Description: This function farms out energy to the systems on the robot.
//              It does this by calculating the pool of energy available each
//              minute, and the examining the request of each system in the
//              order determined by robots[].energyPriorities.
//
//              If the sensors are not allocated sufficient energy to run, they
//              are fed energy in their creation order.
//
//              The general algorithm (for each robot) is:
//                1: Get the total energy available from the generator.
//                2: Iterate through each energy system in the order set by
//                   the robot's energy priorities
//                3: If that system cannot accept more energy, skip it!
//                4: Calculate the energy actually used by that system.
//                   that system will use.
//                5: For systems that accumulate energy, calculate the energy
//                   used in this time frame (energyUsed/CALCS_PER_SEC).  For
//                   systems that use a fixed rate, mark them as powered or
//                   unpowered depending on the energy left in energyPool.
//                6: Cap the energy in the system if more than max.
//                7: Remove the energy from the energy pool.
//                8: If there is no more energy in the pool, stop checking
//                   energy systems.
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void UpdateEnergySystems(t_LL listOfRobots) {
	int i, j;
	float energyPool, energyUsed;
	ROBOT *robot;

	ForeachLL_M(listOfRobots, robot)
	{
		energyPool = robot->generatorStructure * GENERATOR_CAPACITY /       //1
				MAX_GENERATOR_STRUCTURE;
		for (i = 0; i < NUM_ENERGY_SYSTEMS; i++)                             //2
				{
			energyUsed = 0;

			switch (robot->energyPriorities[i]) {
			case SYSTEM_SHIELDS:
				if (robot->shields >= MAX_SHIELD_ENERGY)                     //3
					break;
				if (energyPool >= robot->shieldChargeRate)                   //4
						{                                                     //
					energyUsed = robot->shieldChargeRate;                     //
				}                                                             //
				else
					//3
					energyUsed = energyPool;                                 //4
				robot->shields += energyUsed / CALCS_PER_MIN;                //5
				if (robot->shields > MAX_SHIELD_ENERGY)                      //6
					robot->shields = MAX_SHIELD_ENERGY;
				break;
			case SYSTEM_SENSORS:
				for (j = 0; j < MAX_SENSORS; j++) {
					if (robot->sensorArray[j].on) {
						switch (robot->sensorArray[j].type) {
						case SENSOR_RADAR:
							energyUsed += RADAR_SENSOR_ENERGY_COST;
							if (energyUsed > energyPool)
								robot->sensorArray[j].powered = 0;
							else
								robot->sensorArray[j].powered = 1;
							break;
						case SENSOR_RANGE:
							energyUsed += RANGE_SENSOR_ENERGY_COST;
							if (energyUsed > energyPool)
								robot->sensorArray[j].powered = 0;
							else
								robot->sensorArray[j].powered = 1;
							break;
						default:
							break;
						}
					}
				}
				break;
			case SYSTEM_LASERS:
				if (robot->weaponArray[LASER_PORT].chargeEnergy
						>= MAX_LASER_ENERGY)
					break;
				if (energyPool >= robot->weaponArray[LASER_PORT].chargeRate)
					energyUsed = robot->weaponArray[LASER_PORT].chargeRate;
				else
					energyUsed = energyPool;
				robot->weaponArray[LASER_PORT].chargeEnergy += energyUsed
						/ CALCS_PER_MIN;
				if (robot->weaponArray[LASER_PORT].chargeEnergy
						> MAX_LASER_ENERGY)
					robot->weaponArray[LASER_PORT].chargeEnergy =
					MAX_LASER_ENERGY;
				break;
			case SYSTEM_MISSILES:                                            //3
				if (robot->weaponArray[MISSILE_PORT].chargeEnergy
						>= MAX_MISSILE_ENERGY)
					break;
				if (energyPool >= robot->weaponArray[MISSILE_PORT].chargeRate) //4
					energyUsed = robot->weaponArray[MISSILE_PORT].chargeRate; //
				else
					//
					energyUsed = energyPool;                                 //4
				robot->weaponArray[MISSILE_PORT].chargeEnergy += energyUsed
						/ CALCS_PER_MIN; //5
				if (robot->weaponArray[MISSILE_PORT].chargeEnergy
						> MAX_MISSILE_ENERGY)
					robot->weaponArray[MISSILE_PORT].chargeEnergy =
					MAX_MISSILE_ENERGY; //6
				break;
			default:
				break;
			}
			energyPool -= energyUsed;  //Subtract energy used.               //7
			if (energyPool < 0)                 //No energy left!
				break;                                                       //8
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: CheckPixel
//
// Description: This function has a lot of idiosyncrasies, so read closely.
//              The purpose of this function is to determine the length of a
//              line before it collides with a robot or the arena boundaries.
//              Specifically, CheckPixel() is used as a callback function for
//              the Allegro function do_line().  do_line() calculates all the
//              points along a line from point (x1, y1) to (x2, y2), calling the
//              supplied function, in this case: CheckPixel(), for each one.
//              CheckPixel() checks to see if that pixel collides with a robot
//              other than the "current" robot (see data parameter for more
//              info) or exceeds the bounds of the arena, and if so, records
//              the collision and its location.
//
//              Note 1: In many ways, this function can be considered an
//                      extension of UpdateSensorData() in that the two are
//                      somewhat coupled...sharing the information on what
//                      array of data is expected.
//
//              Note 2: The callback function is allowed to be passed a bitmap
//                      to allow drawing of the line, but since we're only
//                      calculating collisions here we ignore it.
//
//              Note 3: The callback function can also be passed an int to
//                      be used as information relevant to the drawing.
//                      CheckPixel() expects that this int is actually a pointer
//                      to an array of integers as described in the parameters
//                      below.
//
//              Note 4: Why check if a collision has happened yet?  This fn
//                      will be called for every point to be drawn on the line
//                      no matter if a collision is detected or not.  Since
//                      there is no way to interrupt the process, we simply
//                      skip all the detection/recording once the first
//                      collision occurs; this will be the closest object to
//                      the robot.
//
// Parameters: BITMAP *bmp - A pointer to a bitmap to draw the line on, but
//                           since this function is only checking for
//                           collisions, this parameter is ignored.
//             int x, y - The point that is being drawn.
//             int data - This parameter is actually a pointer to integers and
//                        must be cast as such.  It points to a 5-element array
//                        containing the following information:
//                        data[0]-0 if no collision has yet occurred for this
//                                line, 1 otherwise.  This function fills this
//                                element out as return information.
//                        data[1]-Represents the robot that "owns" the line.
//                                Collisions should not be checked against
//                                this robot.  This is a pointer to a robot
//                                (*ROBOT)
//                        data[2]-The x position of the collision, if any.
//                        data[3]-The y position of the collision, if any.
//                        data[4]-A pointer to the list of robots.  Must be
//                                cast back to (t_LL *).
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void CheckPixel(BITMAP *bmp, int x, int y, int data) {
	int robX, robY, collision = 0;
	int transClr = COLOR_TRANS;
	int *collisionData = (int*) data;
	ROBOT *robot;
	ROBOT *currentRobot = (ROBOT *) collisionData[1];
	t_LL listOfRobots = (t_LL) collisionData[4];

	if (collisionData[0] == 0)                  //See if collision for this line
			{                                             //has happened yet.

		if (x < 0 || y < 0 || x > ARENA_WIDTH_PX - 1 || y > ARENA_HEIGHT_PX - 1) //See if point is
			collision = 1;                                      //outside arena.
		else {
			ForeachLL_M(listOfRobots, robot)
				//Iterate through all other robots
				if (robot != currentRobot)  //and see if the pixel collides with
						{                     //a non-transparent pixel in them.
					robX = robot->x * PX_PER_CM - SHIELD_BMP_SZ / 2;
					robY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM
							- SHIELD_BMP_SZ / 2;
					if (x
							>= robX&& x<robX+SHIELD_BMP_SZ && y>=robY && y<robY+SHIELD_BMP_SZ)
						if (getpixel(robot->image, x - robX, y - robY)
								!= transClr)
							collision = 1;
				}
		}

		if (collision) {
			collisionData[0] = 1;       //Record collision as having occurred.
			collisionData[2] = x;       //Record x position information.
			collisionData[3] = y;       //Record y position information.
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: ImagesCollide
//
// Description: This function performs pixel-perfect collision detection on two
//              images given the x,y co-ordinates of the top left pixel of each
//              bitmap.
//              The function is fairly optimized in that it first checks to see
//              if the bounding boxes overlap over at all.  If they don't, it
//              returns immediately.  It then traverses only across the
//              rows/cols of the smaller image and will pass on any row/col
//              that is not bounded by the larger image.
//              If the preprocessor directive SHOW_COLLISIONS is not set, this
//              function returns immediately upon detecting a collision.
//              If it is set, collided pixels are colored white, and the
//              function returns after the images have been completely checked.
//
// Parameters: BITMAP *imgA   - The first image
//             int xA, int yA - The top left pixel of the first image.
//             BITMAP *imgB   - The second image.
//             int xB, int yB - The top left pixel of the second image.
//
// Returns: int - 0 if there is no collision.
//              - 1 if the images collide.
//
////////////////////////////////////////////////////////////////////////////////
int ImagesCollide(BITMAP *imgA, int xA, int yA, BITMAP *imgB, int xB, int yB) {
	int areaA, areaB;
	int xA2, yA2, xB2, yB2;
	int smallX1, smallX2, smallY1, smallY2;
	int bigX1, bigX2, bigY1, bigY2;
	int col, row;
	BITMAP *small, *big;
	int transClr = COLOR_TRANS;
	int collided = 0;

	xA2 = xA + imgA->w - 1;                      //Get co-ordinates of right and
	yA2 = yA + imgA->h - 1;                              //bottom image points.
	xB2 = xB + imgB->w - 1;
	yB2 = yB + imgB->h - 1;
	if (xA2 < xB || xB2 < xA || yA2 < yB || yB2 < yA) //Compare bounding boxes of the
		return 0;                                //image. If no overlap, return!

	areaA = imgA->w * imgA->h;                       //Find the smaller image.
	areaB = imgB->w * imgB->h;

	if (areaA < areaB)                               //Get x/y locations for the
			{                                    //small and big boxes depending
		small = imgA;                              //on if image A was bigger or
		big = imgB;                                    //smaller than image B.
		smallX1 = xA;
		smallX2 = xA2;
		smallY1 = yA;
		smallY2 = yA2;
		bigX1 = xB;
		bigX2 = xB2;
		bigY1 = yB;
		bigY2 = yB2;
	} else {
		small = imgB;
		big = imgA;
		smallX1 = xB;
		smallX2 = xB2;
		smallY1 = yB;
		smallY2 = yB2;
		bigX1 = xA;
		bigX2 = xA2;
		bigY1 = yA;
		bigY2 = yA2;
	}

	for (col = smallX1; col <= smallX2; col++)    //This section performs a col-
			{                                       //row comparison between the
		if (col < bigX1 || col > bigX2)            //two images.  Rows/cols that
			continue;                              //do not overlap are skipped.
		for (row = smallY1; row <= smallY2; row++) //When coincidence occurs, each
				{                                //pixel in the same row between
			if (row < bigY1 || row > bigY2)      //the two images is compared to
				continue;                         //find non-transparent pixels.

			if (getpixel(small, col - smallX1, row - smallY1) != transClr
					&& getpixel(big, col - bigX1, row - bigY1) != transClr) {
#ifdef SHOW_COLLISIONS
				putpixel(small, col-smallX1, row-smallY1, makecol(255, 255, 255));
				putpixel(big, col-bigX1, row-bigY1, makecol(255, 255, 255));
				collided=1;
#else
				return 1;
#endif
			}
		}
	}
	return collided;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: ApplyDamage
//
// Description: This function applies the damage stored in each robot's "Damage
//              bank" to that robot based on the amount of shields on the robot
//              and SHIELDS_LEAK_THRESHOLD.
//
// Parameters: GAME *game - A pointer to game data for sound requests.
//             t_LL listOfRobots - The linked list of robots.
//             t_LL listOfDeadRobots - The linked list of dead robots, where
//                                     robots will be placed if they explode.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void ApplyDamage(GAME *game, t_LL listOfRobots, t_LL listOfDeadRobots) {
	float leakRatio, damage, internalDamage;
	int i;
	ROBOT *robot, *nextRobot;

	SafeForeachLL_M(listOfRobots, robot, nextRobot)
	{
		nextRobot = NextElmLL(robot);

		damage = robot->damageBank;
		robot->damageBank = 0;

		if (robot->shields > SHIELDS_LEAK_THRESHOLD)
			leakRatio = 0;
		else
			leakRatio = 1 - robot->shields / SHIELDS_LEAK_THRESHOLD; //Find ratio leaking.

		internalDamage = damage * leakRatio;          //Get internal damage that
													  //leaked through, if any.
		damage = damage - internalDamage;             //Reduce damage by leaked.

		if (damage > robot->shields)                  //If damage overwhelms the
				{                                       //shields, reduce damage
			damage -= robot->shields;                 //by remaining shields and
			robot->shields = 0;                           //set shields to zero.
		} else                                      //Otherwise, just reduce the
		{                                            //shields by the damage and
			robot->shields -= damage;                      //remove non-internal
			damage = 0;                               //hits as shields absorbed
		}                                                 //them all.

		internalDamage += damage;                          //Add any damage that
		robot->generatorStructure -= internalDamage;  //shields couldn't hold to
													  //internals and apply.

		if (robot->generatorStructure <= 0)                //Robot destroyed!
				{
			game->playSound[SND_ROBOT_EXPLODE] = 1;                //Play sound.
			CreateRobotExplodeParticleBurst(robot->x, robot->y); //Draw explosion.
			ClearRobotGraphics(robot);        //Clear graphics so robot won't be
											  //"noticed" in collision detection.
			for (i = 0; i < MAX_SENSORS; i++) { //Save some cycles by turning off
				robot->sensorArray[i].on = 0;           //all the sensors.
				robot->sensorArray[i].powered = 0;
			}

			//This line of code unlinks the now-dead robot from the main list and
			//appends it to the list of dead robots.
			LinkAftLL(LastElmLL(listOfDeadRobots), UnlinkLL(robot));
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: Create Weapon/Robot/Explode ParticleBurst
//
// Description: These next three functions are hard-coded helper functions to
//              create neat particle bursts for lasers, missiles, and robot
//              explosions.  Preprocessor directives have been used to skip
//              any effects if particles are not to be drawn.
//
// Parameters: int x, y: location of the event.
//
// Returns: Nothing
//
////////////////////////////////////////////////////////////////////////////////
void CreateWeaponParticleBurst(WEAPONTYPE type, int x, int y) {
#ifdef SHOW_PARTICLES
	int particleCount;

	switch (type) {
	case WEAPON_LASER:
		for (particleCount = 0; particleCount < 600; particleCount++)
			AddParticle(x, y,
					makecol(156 + GetRandomNumber(100), GetRandomNumber(30),
							GetRandomNumber(30)), GetRandomNumber(360),
					5 + GetRandomNumber(60), .25);
		break;
	case WEAPON_MISSILE:
		for (particleCount = 0; particleCount < 1500; particleCount++)
			AddParticle(x, y,
					makecol(156 + GetRandomNumber(100), GetRandomNumber(30),
							GetRandomNumber(30)), GetRandomNumber(360),
					GetRandomNumber(40), 2);
		break;
	default:
		AbortOnError("CreateWeaponParticleBurst() was sent a non-weapon type"
				"for particle creation.\nProgram will end.");
	}
#endif
}

void CreateRobotsCollideParticleBurst(int x, int y) {
#ifdef SHOW_PARTICLES
	int particleCount;
	for (particleCount = 0; particleCount < 1000; particleCount++)
		AddParticle(x, y,
				makecol(GetRandomNumber(256), GetRandomNumber(256),
						GetRandomNumber(256)), GetRandomNumber(360),
				GetRandomNumber(40), 1.5);
#endif
}

void CreateRobotExplodeParticleBurst(int x, int y) {
#ifdef SHOW_PARTICLES
	int particleCount;
	for (particleCount = 0; particleCount < 5000; particleCount++)
		AddParticle(x, y,
				makecol(GetRandomNumber(256), GetRandomNumber(256),
						GetRandomNumber(256)), GetRandomNumber(360),
				GetRandomNumber(60), 5);
#endif
}
