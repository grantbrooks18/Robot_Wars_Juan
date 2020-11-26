////////////////////////////////////////////////////////////////////////////////
//
// File: particles.c
//
// Description: This file contains the implementation of the particle engine.
//              The engine must be initialized with InitParticleSystem() to
//              ensure particles fade away to the correct background color
//              and to set up the linked list.
//
//              By better using the InitParticleSystem() function, this module
//              could sever ties with the competition framework. For example,
//              setting an amount of time to pass on each call.  Some constants
//              such as DEG_PER_RAD would also need to be defined.
//
//              Note: Once initialized with InitParticleSystem(),
//                    DestroyAllParticles() must be called to clean up the list.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 4  April 2006 - Created
//                   14 July 2006  - All functions adjusted to add support for
//                                   the linked list library by George Matas.
//                                   My older linked list code removed.
//
////////////////////////////////////////////////////////////////////////////////
#include <math.h>
#include "particles.h"
#include "ll.h"

//Global Variables.
static t_LL particleList;                   //The list of particles.
static int fadeR = 0, fadeG = 0, fadeB = 0;       //Background fade color.

////////////////////////////////////////////////////////////////////////////////
//
// Function: InitParticleSystem
//
// Description: This function records the red, green, and blue values to which
//              particles should be faded over their lifetime.
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void InitParticleSystem(int color) {
	particleList = ConsLL();            //Initialize the list of particles.
	fadeR = getr(color);
	fadeG = getg(color);
	fadeB = getb(color);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: AddParticle
//
// Description: This function adds a particle to the head of the particle list.
//
// Parameters:  int x, y - Location of the particle.
//              int color - Initial color of the particle.
//              float heading - Direction particle is travelling.
//              float speed - Speed of the particle.
//              float timeToLive - Time (in seconds) over which to fade the
//                                 particle away and then remove it.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void AddParticle(int x, int y, int color, float heading, float speed,
		float timeToLive) {
	PARTICLE newParticle;                //Temporary particle to add to list.

	newParticle.x = x;
	newParticle.y = y;
	newParticle.color = color;
	newParticle.r = getr(color);
	newParticle.g = getg(color);
	newParticle.b = getb(color);
	newParticle.heading = heading;
	newParticle.speed = speed;
	newParticle.timeToLive = timeToLive;
	newParticle.originalTTL = timeToLive;

	InsLastLL(particleList, newParticle);     //Put particle at end of the list.

}

////////////////////////////////////////////////////////////////////////////////
//
// Function: UpdateParticles
//
// Description: This function moves the particles, fades them out, and
//              deletes them if their time has expired.
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void UpdateParticles() {
	PARTICLE *tempParticle, *nextParticle;   //Need two temporary particles when
											 //elements may be deleted from the
											 //list while iterating through it.
	double dist, radians, fadePct, addPct;

	SafeForeachLL_M(particleList, tempParticle, nextParticle)
	//An iterator macro
	{                                                       //See ll.c for info.
		nextParticle = NextElmLL(tempParticle);     //Set up nextParticle.  This
		if (tempParticle->timeToLive < 0)      //allows deletion of tempParticle
			DelElmLL(tempParticle);                  //if required.
		else {
			radians = tempParticle->heading * DEG_PER_RAD;      //Move particle.
			dist = tempParticle->speed / CALCS_PER_SEC;
			tempParticle->x += dist * cos(radians);
			tempParticle->y += dist * sin(radians);
			fadePct = tempParticle->timeToLive / tempParticle->originalTTL; //Fade
			addPct = 1 - fadePct;                                     //particle
			tempParticle->color = makecol(                          //out to the
					tempParticle->r * fadePct + fadeR * addPct,  //background
					tempParticle->g * fadePct + fadeG * addPct,  //color.
					tempParticle->b * fadePct + fadeB * addPct);

			tempParticle->timeToLive -= 1.0 / CALCS_PER_SEC; //Update time to live.
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DrawParticles
//
// Description: This function draws particles on the given bitmap.  The constant
//              PX_PER_CM is required to calculate the pixel position vs. the
//              cm position.
//
// Parameters: BITMAP *bmp - The bitmap to draw the particles on.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DrawParticles(BITMAP *bmp) {
	int drawX, drawY;
	PARTICLE *tempParticle;                   //Particle use for list iteration.

	ForeachLL_M(particleList, tempParticle)
	//Iteration macro.
	{
		drawX = tempParticle->x * PX_PER_CM;
		drawY = ARENA_HEIGHT_PX - tempParticle->y * PX_PER_CM;
		putpixel(bmp, drawX, drawY, tempParticle->color);
	}

}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DeleteAllParticles
//
// Description: This function deletes all particles from the linked list.
//
// Parameters: None.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DeleteAllParticles(void) {
	DestLL(particleList);
}
