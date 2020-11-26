////////////////////////////////////////////////////////////////////////////////
//
// File: bender.c
//
// Description: This is an example file demonstrating how a robot can be
//              written for the competition.  The most important function in
//              this example is MaxWillCrushYou() as it is the function
//              registered in main() and will be called by the competition
//              engine to allow you to have your robot take actions.  Almost
//              as important is EquipMax() which is also a callback function
//              used to define what sensors you are using.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 11 Mar 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "..\src\competition.h"
#include "maximilian.h"

////////////////////////////////////////////////////////////////////////////////
//
//  Function: MaxWillCrushYou
//
//  Description: This function contains the actions that Maximilian
//               will perform during the competition.
//
//  Parameters: int timePassed - The number of milliseconds since this function
//                               was last called.
//
//  Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void MaxWillCrushYou(int timePassed) {
	//Add your robot actions here.  Remember, since this isn't your own main()
	//that you'll need to make any variables that need to have a memory
	//static so they'll preserve their value from call to call.
	//I TOLD YOU there was a use for it!

	GPS_INFO gpsData;
	static int target = 0;
	int targetX = 0;
	int targetY = 0;
	float dx, dy, dist, angle;
	char statusMessage[STATUS_MSG_LEN];

	SetSystemChargeRate(SYSTEM_SHIELDS, 400);
	SetSystemChargeRate(SYSTEM_LASERS, 500);

	if (GetSensorData(0) && GetSensorData(1)) {
		SetMotorSpeeds(0, 0);
		SetStatusMessage("Firing!");
		if (GetSystemEnergy(SYSTEM_LASERS) >= MIN_LASER_ENERGY)
			FireWeapon(WEAPON_LASER, 0);
	} else if (GetSensorData(0)) {
		SetMotorSpeeds(-20, 20);
		SetStatusMessage("Aiming!");
	} else if (GetSensorData(1)) {
		SetMotorSpeeds(20, -20);
		SetStatusMessage("Aiming!");
	} else {
		GetGPSInfo(&gpsData);
		switch (target) {
		case 0:
			targetX = 30, targetY = 30;
			break;
		case 1:
			targetX = 345, targetY = 345;
			break;
		case 2:
			targetX = 30, targetY = 345;
			break;
		case 3:
			targetX = 345, targetY = 30;
			break;

		}
		dx = gpsData.x - targetX;
		dy = gpsData.y - targetY;
		dist = sqrt(pow(dx, 2) + pow(dy, 2));
		if (dist < 10) {
			target += 1 % 4;
		}
		angle = asin(dy / dist);
		angle *= RAD_PER_DEG;
		if (dy < 0) {
			if (dx < 0)
				angle = -angle;
			else
				angle = 180 + angle;
		} else if (dx > 0)
			angle = angle + 180;
		else
			angle = 360 - angle;

		if (abs(gpsData.heading - angle) < 2.5)
			SetMotorSpeeds(100, 100);
		else if (gpsData.heading - angle > 0)
			SetMotorSpeeds(100, 60);
		else
			SetMotorSpeeds(60, 100);

		sprintf(statusMessage,
				"Target: %d\nDesired Angle: %5.2f\nActual Angle: %5.2f\nDistance: %5.2f\n",
				target, angle, gpsData.heading, dist);
		SetStatusMessage(statusMessage);
	}

}

////////////////////////////////////////////////////////////////////////////////
//
//  Function: EquipMax
//
//  Description: This is an example function for the robot callback
//               configuration function.  The ONLY lab functions you should call
//               here are AddSensor() and SetSystemChargePriorities().
//
//  Parameters: None.
//
//  Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void EquipMax(void) {
	AddSensor(0, SENSOR_RADAR, -15, 12, 100);
	AddSensor(1, SENSOR_RADAR, 3, 12, 100);
}
