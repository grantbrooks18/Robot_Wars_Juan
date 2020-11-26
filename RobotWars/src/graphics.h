////////////////////////////////////////////////////////////////////////////////
//
// File: graphics.h
//
// Description: This file contains graphics-related functions for the
//              robot competition.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////
#include "competition.h"
#include "ll.h"

void InitGraphics();
void DeInitGraphics();
void RenderScene(t_LL listOfRobots, t_LL listOfDeadRobots, t_LL listOfWeapons,
		char *message);
void DrawSensorBitmaps(t_LL listOfRobots);
void DrawRobotBitmaps(t_LL listOfRobots);
void DrawWeaponBitmap(WEAPON *weapon);
void ClearRobotGraphics(ROBOT *robot);
