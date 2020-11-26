/*
 * Teemo.h
 *
 *  Created on: Nov 20, 2014
 *      Author: s26831
 */

#ifndef TEEMO_H_
#define TEEMO_H_

void TeemoActions(int timePassed);
void TeemosEquipement(void);
void Shield_Sensor_Charge(void);
void Mode_Chasse(void);
void Mode_Combat(void);
void Mode_Escape(void);
void Mode_Search(void);
void Case_Select(int teemo_case);
void Teemo_Attack(void);
int Teemo_Targetting(void);
int Teemo_Case(int teemo_case);



#endif /* TEEMO_H_ */
