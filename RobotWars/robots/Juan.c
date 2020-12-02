
#include "..\src\competition.h"
#include "Juan.h"

//Function Declarations
void juan_setup();
void juan_actions();
void juan_hide();
void juan_find();
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
    sprintf(statusMessage,
            "Dios Mio! Hiding!");
    SetStatusMessage(statusMessage);

    SetSensorStatus(0, 0);
    SetSensorStatus(1,0);
    SetSensorStatus(2, 1);
    SetSensorStatus(3,0);

    int front_d, back_d;
    int left_r, right_r;

    SYSTEM juan_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SENSORS, SYSTEM_SHIELDS,
                                               SYSTEM_LASERS, SYSTEM_MISSILES };
    SetSystemChargePriorites(juan_prios);


    if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 200);
    } else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
        SetSystemChargeRate(SYSTEM_LASERS, 0);
    }

    if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
        if (GetSystemEnergy(SYSTEM_LASERS) <= 50) {
            SetSystemChargeRate(SYSTEM_MISSILES, 600);
        } else if (GetSystemEnergy(SYSTEM_LASERS) >=50) {
            SetSystemChargeRate(SYSTEM_MISSILES, 300);
        }
    } else if (GetSystemEnergy(SYSTEM_MISSILES) >= MAX_MISSILE_ENERGY) {
        SetSystemChargeRate(SYSTEM_MISSILES, 0);
    }

    SetMotorSpeeds(100, 100);
    SetSystemChargeRate(SYSTEM_SHIELDS, 1000);

    front_d=GetSensorData(2);
    back_d =GetSensorData(3);
    left_r=GetSensorData(0);
    right_r=GetSensorData(1);

    if(front_d<50){

        SetMotorSpeeds(-100, 100);
    }

    if ((GetSystemEnergy(SYSTEM_SHIELDS) > 380)&&((front_d<60)||((GetBumpInfo() == 0x04)||(GetBumpInfo() == 0x08)))){
        if (IsTurboOn() == 0) {
            TurboBoost();
        } else if (IsTurboOn() == 1) {
            SetMotorSpeeds(-100, -100);
        }
    }

}

void juan_find(){
    sprintf(statusMessage,
            "On the prowl!");
    SetStatusMessage(statusMessage);

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
    //combat function
    if(lock>0){
        juan_fight(radar_top,radar_bottom,lock);
    }

    if((front_d<80)&&(radar_bottom==0)&&(radar_top==0)){

        GPS_INFO gpsData;

        if(GetSystemEnergy(SYSTEM_SHIELDS) > 400) {
            GetGPSInfo(&gpsData);

            sprintf(statusMessage,
                    "x: %f\n y: %f\n heading: %f\n", gpsData.x, gpsData.y, gpsData.heading);
            SetStatusMessage(statusMessage);

            if (((gpsData.x < 40) || (gpsData.x > 300)) && ((gpsData.y < 40) || (gpsData.y > 300))) { //robot in corner
                sprintf(statusMessage,
                        "Corner Detected! \nHeading: %f", gpsData.heading);
                SetStatusMessage(statusMessage);

                if ((gpsData.x < 40) && ((gpsData.y < 40))) { //bottom left
                    sprintf(statusMessage,
                            "Bottom Left Corner!\n Heading: %f", gpsData.heading);
                    SetStatusMessage(statusMessage);
                    if (abs(gpsData.heading - 45) > 5) {
                        SetMotorSpeeds(100, -100);
                    } else {
                        SetMotorSpeeds(100, 100);
                    }

                }

                if ((gpsData.x < 40) && ((gpsData.y > 300))) { //top left
                    sprintf(statusMessage,
                            "Top Left Corner! \n Heading: %f", gpsData.heading);
                    SetStatusMessage(statusMessage);

                    if (abs(gpsData.heading - 315) < 5) {
                        SetMotorSpeeds(100, -100);
                    } else {
                        SetMotorSpeeds(100, 100);
                    }

                }

                if ((gpsData.x > 300) && ((gpsData.y > 300))) { //top right
                    sprintf(statusMessage,
                            "Top Right Corner!\n Heading: %f", gpsData.heading);
                    SetStatusMessage(statusMessage);
                    if (abs(gpsData.heading - 225) > 5) {
                        SetMotorSpeeds(100, -100);
                    } else {
                        SetMotorSpeeds(100, 100);
                    }

                }

                if ((gpsData.x > 300) && ((gpsData.y < 40))) { //bottom right
                    sprintf(statusMessage,
                            "Bottom Right Corner!\n Heading: %f", gpsData.heading);
                    SetStatusMessage(statusMessage);
                    if (abs(gpsData.heading - 135) > 5) {
                        SetMotorSpeeds(50, -50);
                    } else {
                        SetMotorSpeeds(100, 100);
                    }

                }

            }
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