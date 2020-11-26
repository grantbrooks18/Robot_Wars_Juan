////////////////////////////////////////////////////////////////////////////////
//
// File: particles.h
//
// Description: This is the header file for the particle engine.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 2 April 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////
#include "competition.h"

typedef struct PARTICLE_TAG {
	float x;
	float y;
	int r;
	int g;
	int b;
	int color;
	float heading;
	float speed;
	float timeToLive;              //Holds remaining life of particle.
	float originalTTL;            //Used to calculate color reduction over time.
//  struct PARTICLE_TAG *next;
} PARTICLE;

void InitParticleSystem(int color);
void AddParticle(int x, int y, int color, float heading, float speed,
		float timeToLive);
void UpdateParticles();
void DrawParticles(BITMAP *bmp);
void DeleteAllParticles(void);
void UpdateParticles(void);
