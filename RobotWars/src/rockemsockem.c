////////////////////////////////////////////////////////////////////////////////
//
// File: rockemsockem.c
//
// Description: This is the main game file for Robot Wars.  It sets up the
//              environment, registers the robots, and runs the game.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//
////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include "competition.h"
#include "..\robots\bender.h"                   //0
#include "..\robots\maximilian.h"               //1
#include "..\robots\6R4V3 D1663R.h"             //2
#include "..\robots\capsule1337.h"              //3
#include "..\robots\JR.h"               		//4
#include "..\robots\Lopez.h"               		//5
#include "..\robots\Teemo.h"					//6
#include "..\robots\Juan.h"


// Bender - Testbot with source code			ROBOT 0
void BENDERBOT(ROBOTCOLORS color) {
	RegisterRobot("Bender", color, &BenderActions, &PimpOutBender,
			"resources\\images\\basic_robot.bmp", -1, -1, -1);
}
// Maximillian - Testbot with source code		ROBOT 1
void MAXMILLIAN(ROBOTCOLORS color) {
	RegisterRobot("Maximillian", color, &MaxWillCrushYou, &EquipMax,
			"resources\\images\\artillery_small.bmp", -1, -1, -1);
}
// Teemo - Testbot with source code				ROBOT 2
void TEEMO(ROBOTCOLORS color) {
	RegisterRobot("Teemo", color, &TeemoActions, &TeemosEquipement,
			"resources\\images\\teemo.bmp", -1, -1, -1);
}

//----------------------------------------- Pre-Compilled Robots -----------------

 // Grave Digger - Testbot with object code 	ROBOT 3
 void GRAVEDIGGER(ROBOTCOLORS color) {
 RegisterRobot("6R4V3 DI663R", color, &GraveDiggerActions,
 &GraveDiggerSkillz, "resources\\images\\grave_digger.bmp", -1, -1,
 -1);
 }


 // Capsule1337 - Testbot with object code		ROBOT 4
 void CAPSULE(ROBOTCOLORS color) {
 RegisterRobot("capsule1337", color, &capsule1337_AI, &capsule1337_SETUP,
 "resources\\images\\capsule1337.bmp", -1, -1, -1);
 }


// JR - Testbot with object code				ROBOT 5
void JR(ROBOTCOLORS color) {
RegisterRobot("JR", color, &JRActions, &KitOut,
"resources\\images\\Coke.bmp", -1, -1, -1);
}


// Lopez - Testbot with object code			ROBOT 6
void LOPEZ(ROBOTCOLORS color) {
RegisterRobot("Lopez", color, &LopezActions, &PimpOutLopez,
"resources\\images\\benny_lek.bmp", -1, -1, -1);
}

void JUAN(ROBOTCOLORS color) {
    RegisterRobot("Juan", color, &juan_actions, &juan_setup,
                  "resources\\images\\benny_lek.bmp", -1, -1, -1);
}

//----------------------------------------------------------------------------

typedef void (*fpRegister)(ROBOTCOLORS);

//typedef void (*)(ROBOTCOLORS) fpREG;                  
fpRegister fpREG[] = { &BENDERBOT, &MAXMILLIAN, &TEEMO
		,&GRAVEDIGGER, &CAPSULE, &JR, &LOPEZ, &JUAN 	// Pre-compilled robots
		};

int registeredRobots[4];
int numInCompetition = 0;

void ProcessCommandLine(int argc, char **argv);
//   ROBOT_RED, ROBOT_GREEN, ROBOT_BLUE, ROBOT_YELLOW,
// ROBOT_PURPLE, ROBOT_TURQUOISE, ROBOT_WHITE

int main(int argc, char *argv[]) {
	ROBOTCOLORS colours[4] = { ROBOT_RED, ROBOT_GREEN, ROBOT_YELLOW,
			ROBOT_PURPLE };
	int cnt;

	ProcessCommandLine(argc, argv);
	InitCompetition();

	for (cnt = 0; cnt < numInCompetition; cnt++) {
		(*fpREG[registeredRobots[cnt]])(colours[cnt]);
	}

	Fight();
	EndCompetition();

	return EXIT_SUCCESS;
}
END_OF_MAIN()          //Macro required for Allegro graphics library in Windows.

void ProcessCommandLine(int argc, char *argv[]) {
	int cnt;

	if (argc <= 1) {
		AbortOnError("No Robot Registers on command line\nProgram will exit.");
	}

	if (argc > 5) {
		AbortOnError("More then 4 robots registered\nProgram will exit.");
	}

	for (cnt = 1; cnt < argc; cnt++) {
		sscanf(argv[cnt], "%d", &registeredRobots[cnt - 1]);
		numInCompetition++;
	}
}
