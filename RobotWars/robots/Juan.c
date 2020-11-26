
#include "..\src\competition.h"
#include "Juan.h"

//Function Declarations
void juan_setup();
void juan_actions();
int mode_select();
int case_select();
void case_execute(int juan_case);

void juan_setup(){
    AddSensor(0,SENSOR_RADAR,45,50,RADAR_MAX_RANGE);
    AddSensor(1, SENSOR_RADAR,85,50,RADAR_MAX_RANGE);
    AddSensor(2,SENSOR_RANGE, 0, 0,RANGE_MAX_RANGE);
    AddSensor(3, SENSOR_RANGE, 180,0,RANGE_MAX_RANGE);
}

void juan_actions() {

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);

    int mode_val = 0;
    mode_val = mode_select();

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


    if(GetSystemEnergy(SYSTEM_SHIELDS) < 400){
        return 0;
    }

    if((GetSensorData(1) + GetSensorData(2)) == 0){
        return 1;
    }

    if((GetSensorData(1) + GetSensorData(2)) == 1){
        return 2;
    }

    if((GetSensorData(1) + GetSensorData(2)) == 2){
        return 3;
    }
}

void case_execute(int juan_case){

    switch(juan_case){
        case 0:

        break;

        case 1:

        break;

        case 2:

        break;

        case 3:

        break;

        case 4:

        break;

    }

}