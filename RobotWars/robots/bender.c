////////////////////////////////////////////////////////////////////////////////
//
// File: bender.c
//
// Description: This is an example file demonstrating how a robot can be
//              written for the competition.  The most important function in
//              this example is BenderActions() as it is the function registered
//              in main() and will be called by the competition engine to allow
//              you to have your robot take actions.  Almost as important is
//              PimpOutBender() which is also a callback function used to define
//              what sensors you are using.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////

#include "..\src\competition.h"
#include "bender.h"

////////////////////////////////////////////////////////////////////////////////
//
//  Function: BenderActions
//
//  Description: This function contains the actions that the Bender robot
//               will perform during the competition.
//
//  Parameters: int timePassed - The number of milliseconds since this function
//                               was last called.
//
//  Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void BenderActions(int timePassed) {
	//Add your robot actions here.  Remember, since this isn't your own main()
	//that you'll need to make any variables that need to have a memory
	//static so they'll preserve their value from call to call.
	//I TOLD YOU there was a use for it!
	int frontRange, backRange;
	static int numViews = 0, laserHits = 0;
	char statusMessage[STATUS_MSG_LEN];
	SYSTEM prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_MISSILES, SYSTEM_SENSORS,
			SYSTEM_SHIELDS, SYSTEM_LASERS };
	SetSystemChargePriorites(prios);

	SetSystemChargeRate(SYSTEM_SHIELDS, 200);
	SetSystemChargeRate(SYSTEM_MISSILES, 600);

	if (GetSensorData(1) > 0)
		SetMotorSpeeds(25, 25);
	else if (GetSensorData(0) > 0) {
		if (!IsTurboOn())
			TurboBoost();
		SetMotorSpeeds(75, 75);
	} else {
		frontRange = GetSensorData(2);
		backRange = GetSensorData(3);
		if (frontRange < 80 && backRange < 80)
			SetMotorSpeeds(60, 100);
		else if (frontRange > backRange)
			SetMotorSpeeds(100, 70);
		else if (frontRange < backRange)
			SetMotorSpeeds(70, 100);
		else
			SetMotorSpeeds(100, 100);
	}
	if (GetSystemEnergy(SYSTEM_MISSILES) == MIN_MISSILE_ENERGY)
		FireWeapon(WEAPON_MISSILE, 0);

	if (GetBumpInfo() & BUMP_LASER)
		laserHits++;

	sprintf(statusMessage,
			"You just proved robot\nadvertising works! %d views!\n"
					"\nTimes hit by laser: %d", numViews++, laserHits);
	SetStatusMessage(statusMessage);
}

////////////////////////////////////////////////////////////////////////////////
//
//  Function: PimpOutBender
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
void PimpOutBender(void) {
	AddSensor(0, SENSOR_RADAR, -10, 20, 100);
	AddSensor(1, SENSOR_RADAR, -20, 40, 60);
	AddSensor(2, SENSOR_RANGE, 40, 0, 0);
	AddSensor(3, SENSOR_RANGE, 140, 0, 0);
}
