////////////////////////////////////////////////////////////////////////////////
//
// File: graphics.c
//
// Description: This file contains graphics-related functions for the
//              robot competition.
//
// Author: Capt. Mike LeSauvage
//
// Revision History: 6 Mar 2006 - Created
//                  19 Jul 2006 - Major rework to change robot data from array
//                                to linked list.  LL docs found at:
//                                http://cmp.felk.cvut.cz/~matas/surrey/
//                                                             software/LL.html
//                   6 Sep 2006 - Changed DrawRobotBitmaps() to automatically
//                                center each robot based on the shield bitmap
//                                size, and the size of the robot's image.
//
////////////////////////////////////////////////////////////////////////////////
#include <math.h>                 //For cos, sin
#include "graphics.h"
#include "particles.h"

static BITMAP *fullScreen;    //Bitmap for full screen (arena/border/displays).
static BITMAP *arena;         //Bitmap of just the arena (sub-bmp of fullScreen)
static BITMAP *shieldPic;     //Used for assembling shields and robot.
static BITMAP *robotImg;      //Static pic of the robot.
static BITMAP *missileImg;    //Static pic of a missile.
static BITMAP *laserImg;      //Used for temp drawing.
static BITMAP *backgroundImg; //Background interface image.
static BITMAP *sensorPic;     //This image is used to draw the "large" sensor
//bitmap before it is cropped onto the individual
//sensor's bitmap.

//Internal helper prototypes
void DrawText(ROBOT *robot, BITMAP *text, int destroyed);

////////////////////////////////////////////////////////////////////////////////
//
// Function: RenderScene
//
// Description: This function draws the current state of the robot competition.
//              Note that the arena is simply a sub-bitmap of the full screen.
//              This means it is quite easy to accidentally draw on the arena...
//              it won't be clipped, so the boundaries have to be right!
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//             t_LL listOfDeadRobots - The linked list of dead robots.
//             message - A message to be printed in the system area.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void RenderScene(t_LL listOfRobots, t_LL listOfDeadRobots, t_LL listOfWeapons,
		char *message) {
	int j, drawX, drawY, drawX2, drawY2, range;
	float angle, radians;
	ROBOT *robot;
	WEAPON *weapon;

	//Draw the background image.  Drawing just the needed portions picks up
	//5 fps on my home PC.  Should replace these hard-coded values!
	//Draws top bar, left bar, bottom bar, middle vertical bar, right bar.
	blit(backgroundImg, fullScreen, 0, 0, 0, 0, SCREEN_WIDTH, BORDER_WIDTH);
	blit(backgroundImg, fullScreen, 0, 0, 0, 0, BORDER_WIDTH, SCREEN_HEIGHT);
	blit(backgroundImg, fullScreen, 0, SCREEN_HEIGHT - BORDER_WIDTH, 0,
	SCREEN_HEIGHT - BORDER_WIDTH, SCREEN_WIDTH, BORDER_WIDTH);
	blit(backgroundImg, fullScreen, 759, BORDER_WIDTH, 759, BORDER_WIDTH,
	BORDER_WIDTH, SCREEN_HEIGHT - 2 * BORDER_WIDTH);
	blit(backgroundImg, fullScreen, SCREEN_WIDTH - BORDER_WIDTH, BORDER_WIDTH,
	SCREEN_WIDTH - BORDER_WIDTH, BORDER_WIDTH,
	BORDER_WIDTH, SCREEN_HEIGHT - 2 * BORDER_WIDTH);

	rectfill(fullScreen, 759 + BORDER_WIDTH, BORDER_WIDTH,     //Blackout status
			SCREEN_WIDTH - BORDER_WIDTH - 1,         //area.
			SCREEN_HEIGHT - BORDER_WIDTH - 1, 0);

	clear_to_color(arena, COLOR_ARENA);             //Clear arena bitmap.

	//Draw the sensor graphics
	ForeachLL_M(listOfRobots, robot)
	{
		for (j = 0; j < MAX_SENSORS; j++)
			if (robot->sensorArray[j].on && robot->sensorArray[j].powered)
				switch (robot->sensorArray[j].type) {
				case SENSOR_RADAR:
					drawX = robot->x * PX_PER_CM - robot->sensorArray[j].drawX;
					drawY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM
							- robot->sensorArray[j].drawY;
					draw_trans_sprite(arena, robot->sensorArray[j].image, drawX,
							drawY);
					break;
				case SENSOR_RANGE:
					angle = -robot->sensorArray[j].angle + robot->heading;
					radians = angle * DEG_PER_RAD;
					drawX = robot->x * PX_PER_CM;
					drawY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM;
					range = robot->sensorArray[j].data;
					if (range == -1)
						range = 80;
					drawX2 = drawX + cos(radians) * range * PX_PER_CM;
					drawY2 = drawY - sin(radians) * range * PX_PER_CM;
					line(arena, drawX, drawY, drawX2, drawY2, robot->color);
					circlefill(arena, drawX2, drawY2, 2, robot->color);
					break;
				case SENSOR_NONE:
				default:
					break;
				}
	}

	//Draw the robots and their related status information.
	ForeachLL_M(listOfRobots, robot)
	{
		//Draw the robot in its shield.
		drawX = robot->x * PX_PER_CM - SHIELD_BMP_SZ / 2;
		drawY = ARENA_HEIGHT_PX - robot->y * PX_PER_CM - SHIELD_BMP_SZ / 2;
		draw_sprite(arena, robot->image, drawX, drawY);
	}

	//Draw robot information in the status area.
	ForeachLL_M(listOfRobots, robot)
		DrawText(robot, fullScreen, 0);
	ForeachLL_M(listOfDeadRobots, robot)
		DrawText(robot, fullScreen, 1);

	//Draw the weapons
	ForeachLL_M(listOfWeapons, weapon)
	{
		switch (weapon->type) {
		case WEAPON_LASER:
			drawX = weapon->x * PX_PER_CM - LASER_BMP_SZ / 2;
			drawY = ARENA_HEIGHT_PX - weapon->y * PX_PER_CM - LASER_BMP_SZ / 2;
			break;
		case WEAPON_MISSILE:
			drawX = weapon->x * PX_PER_CM - MISSILE_BMP_SZ / 2;
			drawY = ARENA_HEIGHT_PX - weapon->y * PX_PER_CM
					- MISSILE_BMP_SZ / 2;
			break;
		default:
			break;
		}
		draw_sprite(arena, weapon->image, drawX, drawY);
	}

	DrawParticles(arena);

	//Draw the system message, if any.
	textprintf_ex(fullScreen, font, 770, 750, makecol(255, 255, 255), -1,
			message);

	//Draw the fullscreen bitmap onto video memory.
	acquire_screen();
	blit(fullScreen, screen, 0, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	release_screen();
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DrawSensorBitmaps
//
// Description: For sensors that have a corresponding image, this function
//              draws that image.
//              Images are drawn as they will appear on the screen.  This
//              means that y values are inverted because y increases from top
//              to bottom on a bitmap.
//              Angles are correct as sensor angles are given relative to the
//              robot's heading, but width values must be subtracted as
//              angles increase counterclockwise in "math" headings vs "compass"
//              headings.
//
// Note: There is the opportunity to optimize this function by drawing the
//       sensor bitmap only when it is on and powered (as is the case with the
//       RenderScene and UpdateSensorData functions.)  However, when done this
//       way the robot may turn its radar on AFTER this function is called,
//       which results in the older sensor graphic appearing on the screen for
//       one frame.
//
// Assumption: Currently, this function assumes that the max radar width
//             is 45 deg.  The assumption solves some problems related to
//             fitting the radar into a 200x200 pixel box.  The sensor graphic
//             would not fit in this box if much larger than 45deg.  Also,
//             the 45deg has been used in handing the special case of the
//             radar pointing left/right or up/down (see in-line docs).
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DrawSensorBitmaps(t_LL listOfRobots) {
	double startAngle, endAngle, oppositeAngle, radians, radiansL1, radiansL2;
	int j, x, y, x1, x2, y1, y2, range;
	int xMin, yMin, left, top;
	ROBOT *robot;

	ForeachLL_M(listOfRobots, robot)
	{
		for (j = 0; j < MAX_SENSORS; j++)
			switch (robot->sensorArray[j].type) {
			case SENSOR_RADAR:
				//The algorithm for drawing the radar bitmap is:
				//1: Clear the bitmap to transparent.
				//2: Draw a colored circle to the radar range.
				//3: Draw lines in transparency bounding the arc on either side
				//4: Directly opposite to the arc, 10 pixels out, fill the remainder
				//   of the circle with the transparent color.
				//5: Now the job is to copy just the sensor graphic to the bitmap
				//   associated with the radar.
				//6: Find the three x,y points that define the arc and find the min
				//   and max each of x/y.
				//7: Min/max doesn't work if the arc is pointing with its curve left/
				//   right or up/down because the curve is further away than any of
				//   the defining arc points. To compensate, do a test to see if the
				//   round part of the arc is left/right or up/down, and if so,
				//   bound the box so the arc fits.
				//8: Pick x/y points for the final image for where the sensor should
				//   be drawn.  Sadly, although there is math behind it, due to
				//   rounding of odd pixels, the value had to be adjusted with +1
				//   with no proof...just empirical tests.
				clear_to_color(sensorPic, COLOR_TRANS);                      //1

				range = robot->sensorArray[j].range * PX_PER_CM;

				startAngle = -robot->sensorArray[j].angle + robot->heading;
				endAngle = startAngle - robot->sensorArray[j].width;
				oppositeAngle = startAngle - robot->sensorArray[j].width / 2
						+ 180;
				circlefill(sensorPic, sensorPic->w / 2, sensorPic->h / 2,    //2
				range, robot->color);
				radiansL1 = startAngle * DEG_PER_RAD;
				x1 = RADAR_WRKIMG_PX / 2 * cos(radiansL1);
				y1 = RADAR_WRKIMG_PX / 2 * sin(radiansL1);
				line(sensorPic, sensorPic->w / 2, sensorPic->h / 2,         //3a
				sensorPic->w / 2 + x1, sensorPic->h / 2 - y1, COLOR_TRANS);
				radiansL2 = endAngle * DEG_PER_RAD;
				x2 = RADAR_WRKIMG_PX / 2 * cos(radiansL2);
				y2 = RADAR_WRKIMG_PX / 2 * sin(radiansL2);
				line(sensorPic, sensorPic->w / 2, sensorPic->h / 2,         //3b
				sensorPic->w / 2 + x2, sensorPic->h / 2 - y2, COLOR_TRANS);
				radians = oppositeAngle * DEG_PER_RAD;
				x = 10 * cos(radians);
				y = 10 * sin(radians);
				floodfill(sensorPic, sensorPic->w / 2 + x,                   //4
				sensorPic->h / 2 - y, COLOR_TRANS);

				//OK, now the *big* bitmap is drawn.  Now we just have to take the
				//section we need and copy it to the radar sensor's bitmap.        //5
				x1 = RADAR_WRKIMG_PX / 2 + range * cos(radiansL1);           //6
				y1 = RADAR_WRKIMG_PX / 2 - range * sin(radiansL1);
				x2 = RADAR_WRKIMG_PX / 2 + range * cos(radiansL2);
				y2 = RADAR_WRKIMG_PX / 2 - range * sin(radiansL2);

				xMin = sensorPic->w / 2;                                     //6
				if (x1 < xMin)
					xMin = x1;
				if (x2 < xMin)
					xMin = x2;

				yMin = sensorPic->h / 2;                                     //6
				if (y1 < yMin)
					yMin = y1;
				if (y2 < yMin)
					yMin = y2;

				while (startAngle < 0)                    //Due to earlier calc,
					startAngle += 360;                //startAngle could be -ve.

				if (startAngle > 180 && startAngle < 225)                    //7
					left = 1;
				else
					left = xMin + 1;

				if (startAngle > 90 && startAngle < 135)                     //7
					top = 1;
				else
					top = yMin + 1;

				blit(sensorPic, robot->sensorArray[j].image, left, top, 0, 0, //7
						RADAR_IMAGE_PX, RADAR_IMAGE_PX);

				robot->sensorArray[j].drawX = RADAR_IMAGE_PX - left + 1;
				;                //8
				robot->sensorArray[j].drawY = RADAR_IMAGE_PX - top + 1;
				break;
			case SENSOR_RANGE:
				break;
			case SENSOR_NONE:
			default:
				break;
			}
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DrawRobotBitmaps
//
// Description: This function draws the shield outline and robot on the robot's
//              image bitmap.  A temporary image is used to draw the shield
//              and robot before the final product is drawn, rotated, onto the
//              robot's bitmap.  The robot bitmap is drawn centered onto
//              the shield image, which is then drawn centered on the robot's
//              image (the one in its structure).
//
// Parameters: t_LL listOfRobots - The linked list of robots.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DrawRobotBitmaps(t_LL listOfRobots) {
	int j, r, g, b;
	int shieldCenterX, shieldCenterY, robotOffsetX, robotOffsetY;
	float drawAngle, modifier;
	ROBOT *robot;
	BITMAP *robotGraphic;

	shieldCenterX = (shieldPic->w - 1) / 2; //All robot's shields are same size,
	shieldCenterY = (shieldPic->h - 1) / 2;    //so calculate center point here.

	ForeachLL_M(listOfRobots, robot)
	{
		clear_to_color(robot->image, COLOR_TRANS);
		clear_to_color(shieldPic, COLOR_TRANS);

		drawAngle = 90 - robot->heading; //Convert "math" angle to screen "heading"
		if (drawAngle < 0)                //angle as rotate_sprite is in compass
			drawAngle = drawAngle + 360;        //coords and make positive.
		drawAngle = drawAngle * 255 / 360; //Convert from 0->255 for rotate_sprite.

		r = getr(robot->color);
		g = getg(robot->color);
		b = getb(robot->color);

		modifier = 0.15 * robot->shields / MAX_SHIELD_ENERGY;
		for (j = 0; j < SHIELD_THICK_PX; j++)
			circlefill(shieldPic, shieldPic->w / 2, shieldPic->h / 2,
			SHIELD_RAD_PX - j,
					makecol(r * .4 + r * j * modifier,
							g * .4 + g * j * modifier,
							b * .4 + b * j * modifier));
		circlefill(shieldPic, shieldPic->w / 2, shieldPic->h / 2,
		SHIELD_RAD_PX - SHIELD_THICK_PX, COLOR_TRANS);

		if (robot->graphic != NULL)
			robotGraphic = robot->graphic;
		else
			robotGraphic = robotImg;
		robotOffsetX = shieldCenterX - (robotGraphic->w - 1) / 2; //Calculate where to
		robotOffsetY = shieldCenterY - (robotGraphic->h - 1) / 2; //draw robot bitmap.
		draw_sprite(shieldPic, robotGraphic, robotOffsetX, robotOffsetY);
		rotate_sprite(robot->image, shieldPic, 0, 0, ftofix(drawAngle));
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DrawWeaponBitmaps
//
// Description: This function draws the image of the weapon for the direction
//              it will be travelling.
//
// Parameters: WEAPON *weapon - A pointer to the weapon being drawn.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DrawWeaponBitmap(WEAPON *weapon) {
	float drawAngle;

	drawAngle = 90 - weapon->heading; //Convert "math" angle to screen "heading"
	if (drawAngle < 0)                   //angle as rotate_sprite is in compass
		drawAngle = drawAngle + 360;        //coords and make positive.
	drawAngle = drawAngle * 255 / 360;  //Convert from 0->255 for rotate_sprite.

	switch (weapon->type) {
	case WEAPON_LASER:
		clear_to_color(laserImg, COLOR_TRANS);
		clear_to_color(weapon->image, COLOR_TRANS);
		rect(laserImg, 4, 2, 6, 8, makecol(255, 0, 0));
		line(laserImg, 5, 1, 5, 9, makecol(255, 0, 0));
		rotate_sprite(weapon->image, laserImg, 0, 0, ftofix(drawAngle));
		break;
	case WEAPON_MISSILE:
		clear_to_color(weapon->image, COLOR_TRANS);
		rotate_sprite(weapon->image, missileImg, MISSILE_X_OFFSET,
		MISSILE_Y_OFFSET, ftofix(drawAngle));
		break;
	default:
		break;
	}
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: ClearRobotGraphics
//
// Description: This function erases the robot bitmap and all sensor bitmaps
//              to the transparent color.
//
// Parameters: ROBOT *robot - A pointer to the robot to clear.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void ClearRobotGraphics(ROBOT *robot) {
	int i;
	clear_to_color(robot->image, COLOR_TRANS);
	for (i = 0; i < MAX_SENSORS; i++)
		if (robot->sensorArray[i].image != NULL)
			clear_to_color(robot->sensorArray[i].image, COLOR_TRANS);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: InitGraphics
//
// Description:
//
// Parameters:
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void InitGraphics() {
	//Set up the for full screen graphics.  Give warning on error and exit.
	int res;
	set_color_depth(COLOR_DEPTH);
	//res = set_gfx_mode(GFX_AUTODETECT_FULLSCREEN, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	res = set_gfx_mode(GFX_AUTODETECT_WINDOWED, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);
	
	if (res != 0)
		AbortOnError("InitGraphics() failed to set graphics mode.\n"
				"Program will exit.");

	set_trans_blender(0, 0, 0, 60);   //Set transparency level for drawing
									  //transparent sprites.

	//Perform loading of specific resources to the competition.
	fullScreen = create_bitmap(SCREEN_WIDTH, SCREEN_HEIGHT);
	arena = create_sub_bitmap(fullScreen, 9, 9, ARENA_WIDTH_PX,
	ARENA_HEIGHT_PX);
	shieldPic = create_bitmap(SHIELD_BMP_SZ, SHIELD_BMP_SZ);
	sensorPic = create_bitmap(RADAR_WRKIMG_PX, RADAR_WRKIMG_PX);
	robotImg = load_bitmap(IMG_ROBOT, NULL);
	missileImg = load_bitmap(IMG_MISSILE, NULL);
	laserImg = create_bitmap(LASER_BMP_SZ, LASER_BMP_SZ);
	backgroundImg = load_bitmap(IMG_UI, NULL);
	if (fullScreen == NULL || arena == NULL || shieldPic == NULL
			|| sensorPic == NULL || robotImg == NULL || missileImg == NULL
			|| laserImg == NULL || backgroundImg == NULL)
		AbortOnError(
				"InitGraphics() failed to create/load required resources.\n"
						"Program will exit.");
	InitParticleSystem(COLOR_ARENA);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DeInitGraphics
//
// Description:
//
// Parameters:
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DeInitGraphics() {
	DeleteAllParticles();            //If any particles exist, free them!
	destroy_bitmap(arena);         //Delete arena bitmap before fullScreen as it
	destroy_bitmap(fullScreen);      //is really a sub-bitmap of fullScreen.
	destroy_bitmap(robotImg);
	destroy_bitmap(missileImg);
	destroy_bitmap(laserImg);
	destroy_bitmap(shieldPic);
	destroy_bitmap(sensorPic);
	destroy_bitmap(backgroundImg);
}

////////////////////////////////////////////////////////////////////////////////
//
// Function: DrawText
//
// Description: This internal helper is used for drawing the information about
//              each robot in the status area.  Sadly, it is a poorly-documented
//              mess as it was quickly implemented before the 2007 competition.
//              However, what it is doing is simply converting the generator,
//              shield, missile, and laser systems to bar graphs that fade
//              from green to red.
//
// Parameters: ROBOT *robot - The robot to draw information about.
//             BITMAP *text - The statues area of the screen to drawn on.
//             int destroyed - 0 if robot is destroyed, 1 if still fighting.
//
// Returns: Nothing.
//
////////////////////////////////////////////////////////////////////////////////
void DrawText(ROBOT *robot, BITMAP *text, int destroyed) {
	int i, j, k, count = 0, offset = 180;
	int tx = 850, ty, bx = 855, by, bw = 150, bh = 10, white; //text and bar values.
	char msg[STATUS_CHAR_COLS + 1];                  //Extra for null character.

	white = makecol(255, 255, 255);

	i = robot->number;
	textprintf_ex(text, font, 770, 20 + offset * i, robot->color, -1, "%s",
			robot->name);

	if (destroyed)
		textprintf_ex(text, font, 800, 40 + offset * i, robot->color, -1,
				"DESTROYED!!!");
	else {
		//Output Generator Status
		ty = 45;
		by = 43;
		rect(text, bx - 1, by - 1 + offset * i, bx + bw,
				by + 1 + bh + offset * i, white);
		for (j = 0; j < bw; j++) {
			if (robot->generatorStructure / (float) MAX_GENERATOR_STRUCTURE
					<= j / (float) bw)
				break;
			line(text, bx + j, by + offset * i, bx + j, by + bh + offset * i,
					makecol(255 * ((bw - j) / (float) bw),
							255 * (j / (float) bw), 0));
		}
		textprintf_right_ex(text, font, tx, ty + offset * i, robot->color, -1,
				"Generator:");
		textprintf_ex(text, font, tx, ty + offset * i, white, -1, " %d",
				robot->generatorStructure);

		//Output Shield Status
		ty = 65;
		by = 63;
		rect(text, bx - 1, by - 1 + offset * i, bx + bw,
				by + 1 + bh + offset * i, white);
		for (j = 0; j < bw; j++) {
			if (robot->shields / (float) MAX_SHIELD_ENERGY <= j / (float) bw)
				break;
			line(text, bx + j, by + offset * i, bx + j, by + bh + offset * i,
					makecol(255 * ((bw - j) / (float) bw),
							255 * (j / (float) bw), 0));
		}
		textprintf_right_ex(text, font, tx, ty + offset * i, robot->color, -1,
				"Shields:");
		textprintf_ex(text, font, tx, ty + offset * i, white, -1, " %.2f",
				robot->shields);

		//Output Missile Status
		ty = 85;
		by = 83;
		rect(text, bx - 1, by - 1 + offset * i, bx + bw,
				by + 1 + bh + offset * i, white);
		for (j = 0; j < bw; j++) {
			if (robot->weaponArray[MISSILE_PORT].chargeEnergy
					/ (float) MAX_MISSILE_ENERGY <= j / (float) bw)
				break;
			line(text, bx + j, by + offset * i, bx + j, by + bh + offset * i,
					makecol(255 * ((bw - j) / (float) bw),
							255 * (j / (float) bw), 0));
		}
		textprintf_right_ex(text, font, tx, ty + offset * i, robot->color, -1,
				"Missiles:");
		textprintf_ex(text, font, tx, ty + offset * i, white, -1, " %.2f",
				robot->weaponArray[MISSILE_PORT].chargeEnergy);

		//Output Laser Status
		ty = 105;
		by = 103;
		rect(text, bx - 1, by - 1 + offset * i, bx + bw,
				by + 1 + bh + offset * i, white);
		for (j = 0; j < bw; j++) {
			if (robot->weaponArray[LASER_PORT].chargeEnergy
					/ (float) MAX_LASER_ENERGY <= j / (float) bw)
				break;
			line(text, bx + j, by + offset * i, bx + j, by + bh + offset * i,
					makecol(255 * ((bw - j) / (float) bw),
							255 * (j / (float) bw), 0));
		}
		textprintf_right_ex(text, font, tx, ty + offset * i, robot->color, -1,
				"Lasers:");
		textprintf_ex(text, font, tx, ty + offset * i, white, -1, " %.2f",
				robot->weaponArray[LASER_PORT].chargeEnergy);

		//Output user's text message.
		for (j = 0; j < STATUS_CHAR_ROWS; j++) {
			for (k = 0; k < STATUS_CHAR_COLS; k++) {
				msg[k] = robot->statusMessage[count++];
				msg[k + 1] = '*';
				if (msg[k] == '\0' || msg[k] == '\n')
					break;
			}
			if (msg[k] == '\0') {
				textprintf_ex(text, font, 770, 135 + offset * i + 10 * j,
						robot->color, -1, msg);
				break;
			} else if (msg[k] == '\n')
				msg[k] = '\0';
			else
				msg[k] = '\0';
			textprintf_ex(text, font, 770, 135 + offset * i + 10 * j,
					robot->color, -1, msg);
		}
	}
	fastline(text, 770, 30 + offset * i, 970, 30 + offset * i, robot->color);
}

//void DrawText(ROBOT *robot, BITMAP *text, int destroyed)
//{
//  int i, j, k, count=0, offset=180;
//  char msg[STATUS_CHAR_COLS+1];            //Extra for null character.
//
//  i=robot->number;
//  textprintf_ex(text, font, 770, 20+offset*i, robot->color, -1, "%s", robot->name);
//
//  if(destroyed)
//    textprintf_ex(text, font, 800, 40+offset*i, robot->color, -1, "DESTROYED!!!");
//  else
//  {
//    textprintf_ex(text, font, 770, 35+offset*i, robot->color, -1, "Speeds: %d, %d", robot->leftTreadSpeed, robot->rightTreadSpeed);
//    textprintf_ex(text, font, 770, 45+offset*i, robot->color, -1, "Shield Capacitor: %.2f", robot->shields);
//    textprintf_ex(text, font, 770, 55+offset*i, robot->color, -1, "Shield Recharge Rate: %d", robot->shieldChargeRate);
//    textprintf_ex(text, font, 770, 65+offset*i, robot->color, -1, "Missile Energy: %.2f", robot->weaponArray[MISSILE_PORT].chargeEnergy);
//    textprintf_ex(text, font, 770, 75+offset*i, robot->color, -1, "Missile Recharge Rate: %d", robot->weaponArray[MISSILE_PORT].chargeRate);
//    textprintf_ex(text, font, 770, 85+offset*i, robot->color, -1, "Laser Energy: %.2f", robot->weaponArray[LASER_PORT].chargeEnergy);
//    textprintf_ex(text, font, 770, 95+offset*i, robot->color, -1, "Laser Recharge Rate: %d", robot->weaponArray[LASER_PORT].chargeRate);
//    textprintf_ex(text, font, 770, 105+offset*i, robot->color, -1, "Bump Status: %d", robot->bumped);
//    textprintf_ex(text, font, 770, 115+offset*i, robot->color, -1, "Generator Structure: %d", robot->generatorStructure);
//
//    for(j=0; j<STATUS_CHAR_ROWS; j++)
//    {
//      for(k=0; k<STATUS_CHAR_COLS; k++)
//      {
//        msg[k]=robot->statusMessage[count++];
//        msg[k+1]='*';
//        if(msg[k]=='\0' || msg[k]=='\n')
//          break;
//      }
//      if(msg[k] == '\0')
//      {
//        textprintf_ex(text, font, 770, 135+offset*i+10*j, robot->color, -1, msg);
//        break;
//      }
//      else if(msg[k] == '\n')
//        msg[k]='\0';
//      else
//        msg[k]='\0';
//      textprintf_ex(text, font, 770, 135+offset*i+10*j, robot->color, -1, msg);
//    }
//  }
//    fastline(text, 770, 30+offset*i, 970, 30+offset*i, robot->color);
//}
