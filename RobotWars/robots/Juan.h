/*
 * Header file for Juan, the fiery luchador robot. Juan is equipped
 * with a variety of sensors, weapons, defenses and combat
 * modes to deal with his enemigos.
 *
 * Author   : Brooks and Lopez Espinosa
 * Version  : 12/7/2020
 */
#ifndef ROBOTWARS_JUAN_H
#define ROBOTWARS_JUAN_H
/*
 * Sensor allocation for Juan.
 * He has 2 radars on his right side, and one
 * range sensor on his front.
 *
 */
void juan_setup();
/*
 * Turns on essential sensors. Begins process of
 * case selection and execution.
 */
void juan_actions();

#endif //ROBOTWARS_JUAN_H
