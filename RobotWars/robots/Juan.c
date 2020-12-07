
#include "..\src\competition.h"
#include "Juan.h"

#include<math.h>
#define pi acos(-1.0)

//Function Declarations
void juan_setup();
void juan_actions();
void juan_hide();
void juan_find();
void juan_obstacle(int *obstacle);
void juan_chase();
void juan_fight(int radar_top, int radar_bottom, int lock);
int mode_select();
int case_select();
void case_execute(int juan_case);
char statusMessage[50];

void juan_setup(){

    AddSensor(0,SENSOR_RADAR,40,40,RADAR_MAX_RANGE);
    AddSensor(1, SENSOR_RADAR,90,40,RADAR_MAX_RANGE);
    AddSensor(2,SENSOR_RANGE, 0, 0,RANGE_MAX_RANGE);
    AddSensor(3, SENSOR_RANGE, 180,0,RANGE_MAX_RANGE);

}

void juan_actions() {

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);

    int case_val = 0;
    case_val = case_select();

    case_execute(case_val);

}

int mode_select(){
    if(GetGeneratorStructure() < 250){
        return 1;
    } else{
        return 0;
    }
}

int case_select(){
    /* Case 0: Hide
     * Case 1: Search
     * Case 2: Chase
     * Case 3: Combat
     */
    int mode_val=mode_select();

    if((GetSystemEnergy(SYSTEM_SHIELDS) < 400) && (mode_val == 0)){
        return 0;
    }
    if((GetSystemEnergy(SYSTEM_SHIELDS) > 400) && (mode_val == 0)){ //testing search
        return 1;
    }
    if(((GetSensorData(1) + GetSensorData(2)) == 0) && (mode_val == 0)){
        return 1;
    }
    if(((GetSensorData(1) + GetSensorData(2)) == 1) && (mode_val == 0)){
        return 2;
    }
    if(((GetSensorData(1) + GetSensorData(2)) == 2) && (mode_val == 0)){
        return 3;
    }
}

void case_execute(int juan_case){

    switch(juan_case){
        case 0:
            juan_hide();
        break;

        case 1:
            juan_find();
        break;

        case 2:
            //juan_chase();
        break;

        case 3:
            //juan_fight();
        break;

        case 4:

        break;

    }

}

void juan_hide(){

    SetSensorStatus(0, 1);
    SetSensorStatus(1,0);
    SetSensorStatus(2, 1);
    SetSensorStatus(3,0);

    int front_d;
    int radar_top;

    radar_top=GetSensorData(0);


    SYSTEM juan_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SENSORS, SYSTEM_SHIELDS,
                                               SYSTEM_LASERS, SYSTEM_MISSILES };
    SetSystemChargePriorites(juan_prios);


    if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 200);
    } else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 0);
    }

    if (GetSystemEnergy(SYSTEM_SHIELDS) < MAX_SHIELD_ENERGY) {
        SetSystemChargeRate(SYSTEM_SHIELDS,MAX_SHIELD_CHARGE_RATE);
    } else if (GetSystemEnergy(SYSTEM_SHIELDS) >= MAX_SHIELD_ENERGY) {
        SetSystemChargeRate(SYSTEM_SHIELDS, 0);
        if (GetSystemEnergy(SYSTEM_MISSILES) < MAX_MISSILE_ENERGY) {
            if (GetSystemEnergy(SYSTEM_LASERS) <= MAX_LASER_ENERGY) {
                SetSystemChargeRate(SYSTEM_LASERS, GetGeneratorOutput()*0.5);
            } else if (GetSystemEnergy(SYSTEM_LASERS) == MAX_LASER_ENERGY) {
                SetSystemChargeRate(SYSTEM_LASERS, 0);
            }
            if (GetSystemEnergy(SYSTEM_MISSILES) <= MAX_MISSILE_ENERGY) {
                SetSystemChargeRate(SYSTEM_MISSILES, GetGeneratorOutput()*0.5);
            } else if (GetSystemEnergy(SYSTEM_MISSILES) == MAX_MISSILE_ENERGY) {
                SetSystemChargeRate(SYSTEM_MISSILES, 0);
            }
        }
    }

    SetMotorSpeeds(100, 100);
    SetSystemChargeRate(SYSTEM_SHIELDS, 1000);

    front_d=GetSensorData(2);

    if(front_d<50){

        SetMotorSpeeds(-100, 100);
    }

    if ((GetSystemEnergy(SYSTEM_SHIELDS) > 350)&&((radar_top==1)||((GetBumpInfo() == 0x04)||(GetBumpInfo() == 0x08)))){
        if (IsTurboOn() == 0) {
            TurboBoost();
        } else if (IsTurboOn() == 1) {
            SetMotorSpeeds(100, 100);
        }
    }

}

void juan_find(){

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);
    SetSensorStatus(2, 1);
    SetSensorStatus(3,0);

    if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 500);
    } else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 0);
    }
    if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
        if (GetSystemEnergy(SYSTEM_LASERS) <= 50) {
            SetSystemChargeRate(SYSTEM_MISSILES, 600);
        }
    } else if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
        SetSystemChargeRate(SYSTEM_MISSILES, 0);
    }
    if(GetSystemEnergy(SYSTEM_SHIELDS)<MAX_SHIELD_ENERGY){
        SetSystemChargeRate(SYSTEM_SHIELDS, 500);
    } else {SetSystemChargeRate(SYSTEM_SHIELDS, 0);}
    if(GetSystemEnergy(SYSTEM_SHIELDS)==MAX_SHIELD_ENERGY) {
        if(GetSystemEnergy(SYSTEM_LASERS)==MAX_LASER_ENERGY) {
            SetSystemChargeRate(SYSTEM_MISSILES, GetGeneratorOutput());
            SetSystemChargeRate(SYSTEM_LASERS, 0);
        }else
            if(GetSystemEnergy(SYSTEM_MISSILES)==MAX_MISSILE_ENERGY)
        SetSystemChargeRate(SYSTEM_MISSILES, 0);
        SetSystemChargeRate(SYSTEM_LASERS, GetGeneratorOutput());
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

void juan_obstacle(int *obstacle){

    int radar_top=GetSensorData(0);
    int radar_bottom=GetSensorData(1);
    float front_d=GetSensorData(2);
    float new_heading;

    if((radar_bottom==0)&&(radar_top==0)){

        if(GetSystemEnergy(SYSTEM_SHIELDS)>(800)) { //fix 900 - currently kills shield to 900

            GPS_INFO gpsData;
            GetGPSInfo(&gpsData);

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

                sprintf(statusMessage,
                        "New heading: %f. \nCurrent: %f\nObstacle: %i ", new_heading, gpsData.heading, *obstacle);
                SetStatusMessage(statusMessage);

                if (abs(gpsData.heading - new_heading) > 10) {
                    SetMotorSpeeds(-100, 100);
                    *obstacle = 1;
                } else {
                    *obstacle = 0;
                }
            }
        }else if(front_d<RADAR_MAX_RANGE){
            SetMotorSpeeds(-100, 100);
        }
    }
}

void juan_fight(int radar_top, int radar_bottom, int lock) {
    SetSensorStatus(2, 0);
    if((radar_bottom==1)&&(radar_top==1)){
        if(lock>0){

            if (GetSystemEnergy(SYSTEM_LASERS) >= 25) {
                FireWeapon(WEAPON_LASER, 80);
            }

            if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
                FireWeapon(WEAPON_MISSILE, 80);
            }
        }

        SetMotorSpeeds(100, 50);
    }
}