
#include "..\src\competition.h"
#include "Juan.h"

//Function Declarations
void juan_setup();
void juan_actions();
int case_select();
void case_execute(int juan_case);

void juan_setup(){
    AddSensor(0,SENSOR_RADAR,45,50,125);
    AddSensor(1, SENSOR_RADAR,85,50,125);
    //AddSensor
}

void juan_actions() {

    SetSensorStatus(0, 1);
    SetSensorStatus(1,1);

    int case_val = 0;

    case_val = case_select();

    case_execute(case_val);



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