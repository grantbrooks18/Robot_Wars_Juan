/*
 * File for Juan, the fiery luchador robot. Juan is equipped
 * with a variety of sensors, weapons, defenses and combat
 * modes to deal with his enemigos.
 *
 * Author   : Brooks and Lopez Espinosa
 * Version  : 12/7/2020
 */
#include "..\src\competition.h"
#include "Juan.h"

#include<math.h>
#define pi acos(-1.0)

//Function Declarations
void juan_setup();
void juan_actions();
void juan_hide();
void juan_find();
int juan_obstacle(int *obstacle);
void juan_fight(int radar_top, int radar_bottom, int lock);
int case_select();
void case_execute(int juan_case);

/*
 * Sensor allocation for Juan.
 * He has 2 radars on his right side, and one
 * range sensor on his front.
 *
 */
void juan_setup(){

    AddSensor(0,SENSOR_RADAR,40,40,RADAR_MAX_RANGE);
    AddSensor(1, SENSOR_RADAR,90,40,RADAR_MAX_RANGE);
    AddSensor(2,SENSOR_RANGE, 0, 0,RANGE_MAX_RANGE);
}

/*
 * Turns on essential sensors. Begins process of
 * case selection and execution.
 */
void juan_actions() {

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);

    int case_val;
    case_val = case_select();

    case_execute(case_val);

}
/*
 * Based on shield reading, selects a
 * case for juan to execute.
 */
int case_select(){
    /* Case 0: Hide
     * Case 1: Search
     * Case 2: Combat
     */
    if(GetSystemEnergy(SYSTEM_SHIELDS) < 400){
        return 0;
    }
    if((GetSensorData(1) + GetSensorData(2)) == 2){
        return 2;
    }
    else{
        return 1;
    }
}
/*
 * Based on case selection, selects and executes a
 * given case.
 *
 * Receives: juan_case, int that determines which
 * case Juan is in.
 */
void case_execute(int juan_case){

    switch(juan_case){
        case 0:
            juan_hide();
        break;

        case 1:
            juan_find();
        break;

        case 2:
            juan_fight(1,1,1);
        break;
    }
}
/*
 * Juan hides, avoiding enemies and recharging his shield and weapons.
 * If hit and with sufficient shield, will turbo out of danger. Triggered
 * when shield is low.
 *
 */
void juan_hide(){

    SetSensorStatus(0, 0);
    SetSensorStatus(1,0);
    SetSensorStatus(2, 1);

    int front_d;

    SYSTEM juan_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SENSORS, SYSTEM_SHIELDS,
                                               SYSTEM_LASERS, SYSTEM_MISSILES };
    SetSystemChargePriorites(juan_prios);

    if (GetSystemEnergy(SYSTEM_SHIELDS) < MAX_SHIELD_ENERGY) {
        SetSystemChargeRate(SYSTEM_SHIELDS,MAX_SHIELD_CHARGE_RATE);
        SetSystemChargeRate(SYSTEM_MISSILES,(GetGeneratorOutput()-MAX_SHIELD_CHARGE_RATE)/2);
        SetSystemChargeRate(SYSTEM_LASERS,(GetGeneratorOutput()-MAX_SHIELD_CHARGE_RATE)/2);
    } else if (GetSystemEnergy(SYSTEM_SHIELDS) >= MAX_SHIELD_ENERGY) {

        SetSystemChargeRate(SYSTEM_SHIELDS, 0);
            if((GetSystemEnergy(SYSTEM_LASERS)!=MAX_LASER_ENERGY)&&GetSystemEnergy(SYSTEM_MISSILES)!=MAX_MISSILE_ENERGY){
            if (GetSystemEnergy(SYSTEM_LASERS) == MAX_LASER_ENERGY) {
                SetSystemChargeRate(SYSTEM_MISSILES, GetGeneratorOutput());
                SetSystemChargeRate(SYSTEM_LASERS, 0);
            }

            if (GetSystemEnergy(SYSTEM_MISSILES) == MAX_MISSILE_ENERGY) {
                SetSystemChargeRate(SYSTEM_MISSILES, 0);
                SetSystemChargeRate(SYSTEM_LASERS, GetGeneratorOutput());
            }
        }
    }

    SetMotorSpeeds(100, 100);

    front_d=GetSensorData(2);

    if(front_d<50){

        SetMotorSpeeds(-100, 100);
    }

    if ((GetSystemEnergy(SYSTEM_SHIELDS) > 350)&&((GetBumpInfo() == 0x04)||(GetBumpInfo() == 0x08))){
        if (IsTurboOn() == 0) {
            TurboBoost();
        } else if (IsTurboOn() == 1) {
            SetMotorSpeeds(100, 100);
        }
    }

}
/*
 * Juan tries to find enemies. Roams around at random,
 * trying to detect enemies in radar. Will also
 * detect obstacles in fornt, and determine if they are enemies or walls
 * using obstacle. If trapped in corner or by wall, will use GPS and
 * trig to drive towards middle of stage. If an enemy is found, will try
 * to lock on and engage with juan_fight().
 */
void juan_find(){

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);
    SetSensorStatus(2, 1);
    SetSensorStatus(3,0);

    if(GetSystemEnergy(SYSTEM_SHIELDS)<MAX_SHIELD_ENERGY){
        SetSystemChargeRate(SYSTEM_SHIELDS, GetGeneratorOutput()*0.5);
        SetSystemChargeRate(SYSTEM_MISSILES, GetGeneratorOutput()*0.4);
        SetSystemChargeRate(SYSTEM_LASERS, GetGeneratorOutput()*0.6);
    } else {SetSystemChargeRate(SYSTEM_SHIELDS, 0);}

    if(GetSystemEnergy(SYSTEM_SHIELDS)>=MAX_SHIELD_ENERGY) {
        if(GetSystemEnergy(SYSTEM_LASERS)==MAX_LASER_ENERGY) {
            SetSystemChargeRate(SYSTEM_MISSILES, GetGeneratorOutput());
            SetSystemChargeRate(SYSTEM_LASERS, 0);
        }

        if(GetSystemEnergy(SYSTEM_MISSILES)==MAX_MISSILE_ENERGY){
            SetSystemChargeRate(SYSTEM_MISSILES, 0);
            SetSystemChargeRate(SYSTEM_LASERS, GetGeneratorOutput());
        }

    }
    SetMotorSpeeds(100, 100);

    int radar_top=GetSensorData(0);
    int radar_bottom=GetSensorData(1);
    int front_d=GetSensorData(2);
    int lock=0;

    if(radar_top==1){
        SetMotorSpeeds(70, 100 );
        lock++;
    }

    if(radar_bottom==1){
        SetMotorSpeeds(100, 70);
        lock++;
    }

    if(lock>0){
        juan_fight(radar_top,radar_bottom,lock);
    }

    int obstacle;

    if(front_d<40){
        obstacle=1;
    }else{
        obstacle=0;
    }

    juan_obstacle(&obstacle);

}
/*
 * Juan has found an obstacle in front of him. He will try to
 * use his radar to determine if it is an enemy, and if not,
 * use the GPS to find heading that will bring him to center stage.
 *
 * obstacle: 0 if no obstacle, 1 if obstacle.
 */
int juan_obstacle(int *obstacle){

    int radar_top=GetSensorData(0);
    int radar_bottom=GetSensorData(1);
    float front_d=GetSensorData(2);

    float new_heading;

    if((radar_bottom==0)&&(radar_top==0)){

        float shield =GetSystemEnergy(SYSTEM_SHIELDS); //helps maintain shield
        int rounded = (shield+5)/10;                        //when collecting gps
        rounded = rounded*10;

        if(GetSystemEnergy(SYSTEM_SHIELDS)>(rounded)&&(rounded>500)) {

            GPS_INFO gpsData;

            if(obstacle==1) {
                GetGPSInfo(&gpsData);
            }else{
                gpsData.x=GetRandomNumber(375); //randomized searching
                gpsData.y=GetRandomNumber(375); //if there's no obstacle
                gpsData.heading=GetRandomNumber(360);
            }

            if ((gpsData.y < 40) || (gpsData.y > 335) || (gpsData.x < 40) || (gpsData.x > 335)) {
                gpsData.y = gpsData.y - 187.5; //convert to new coord system:
                gpsData.x = gpsData.x - 187.5; //origin @(187.5,187.5) of arena

                new_heading = (atan((gpsData.y) / (gpsData.x))) * (180 / pi); //takes xy coord, turns into angle to centre of arena

                if ((gpsData.x > 0) && (gpsData.y > 0)) { //quad 1
                    new_heading = new_heading + 180;
                }

                if ((gpsData.x < 0) && (gpsData.y > 0)) { //quad 2
                    new_heading = new_heading + 360;
                }

                if ((gpsData.x < 0) && (gpsData.y < 0)) { //quad 3
                    //good angle
                }

                if ((gpsData.x > 0) && (gpsData.y < 0)) { //quad 4
                    new_heading = new_heading + 180;
                }

                if (abs(gpsData.heading - new_heading) > 10) {
                    SetMotorSpeeds(-100, 100);
                    *obstacle = 1;
                } else {
                    SetMotorSpeeds(100, 100);
                    *obstacle = 0;
                    return 0;
                }
            }

        }else if(front_d<RADAR_MAX_RANGE){
            SetMotorSpeeds(-100, 100);
        }
    }
}
/*
 * Juan has found an ennemy. If locked and weapon systems are ready,
 * will fire upon ennemy and strafe around them.
 *
 * radar_top: top radar. 0 for no enemy, 1 for yes.
 * radar_bottom: bottom radar. 0 for no enemy, 1 for yes.
 * lock: 1 for locked on, 0 for not.
 */
void juan_fight(int radar_top, int radar_bottom, int lock) {
    SetSensorStatus(2, 0);
    if((radar_bottom==1)&&(radar_top==1)){
        if(lock>0){

            if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
                FireWeapon(WEAPON_LASER, 80);
            }

            if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
                FireWeapon(WEAPON_MISSILE, 85);
            }
        }

        SetMotorSpeeds(100, 50);
    }
}