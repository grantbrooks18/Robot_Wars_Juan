////////////////////////////////////////////////////////////////////////////////
//
// File: physics.h
//
// Description:
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 2 Apr 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////
#include "competition.h"

void MoveRobots(t_LL listOfRobots);
void CheckRobotCollisions(GAME *game, t_LL listOfRobots);
void MoveWeapons(t_LL listOfWeapons);
void CheckWeaponCollisions(GAME *game, t_LL listOfRobots, t_LL listOfWeapons);
void UpdateSensorData(t_LL listOfRobots);
void UpdateEnergySystems(t_LL listOfRobots);
void CheckPixel(BITMAP *bmp, int x, int y, int data);
int ImagesCollide(BITMAP *imgA, int xA, int yA, BITMAP *imgB, int xB, int yB);
void ApplyDamage(GAME *game, t_LL listOfRobots, t_LL listOfDeadRobots);
