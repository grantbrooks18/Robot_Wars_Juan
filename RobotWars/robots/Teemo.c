/*
 * Teemo.c
 *
 *  Created on: Nov 20, 2014
 *      Author: s26831
 */

#include "..\src\competition.h"
#include "Teemo.h"


/*
 * Ici on trouve les actions principales
 * de mon robot Teemo!
 */
void TeemoActions(int timePassed) {

	static int teemo_case = 0;

	Shield_Sensor_Charge();

	teemo_case = Teemo_Case(teemo_case);

	Case_Select(teemo_case);

}
/*
 * Liste de condition pour le choix du
 * mode dans lequel Teemo se retrouvera
 *
 * ENTREE: Le cas precedent du robot
 *
 * SORTI: Le cas prochain du robot
 */
int Teemo_Case(int teemo_case) {

	//Return 1 Mode combat
	//Return 2 Mode search
	//Return 3 Mode escape
	//Return 4 Mode chasse

	if (GetSensorData(0) == 1 && GetSensorData(1) == 1) {
		if (GetBumpInfo() == 0x04 || GetBumpInfo() == 0x08) {
			return 3;
		} else {
			return 1;
		}
	} else if ((GetSensorData(2) > 100 && GetSensorData(2) < 125)
			|| Teemo_Targetting() == 2) {
		if (GetSensorData(0) == 1 || GetSensorData(1) == 1) {
			return 4;
		} else if (GetSensorData(0) == 0 && GetSensorData(1) == 0) {
			return 2;
		}
	} else if ((GetSensorData(3) > 100 && GetSensorData(3) < 125)
			|| Teemo_Targetting() == 2) {
		if (GetSensorData(0) == 1 || GetSensorData(1) == 1) {
			return 4;
		} else if (GetSensorData(0) == 0 && GetSensorData(1) == 0) {
			return 2;
		}
	} else if (Teemo_Targetting() == 0) {
		return 2;
	} else if (GetSystemEnergy(SYSTEM_SHIELDS) < 400) {
		return 3;
	}

}

/*
 * Applique le mode dans lequel le robot doit se trouver
 *
 * ENTREE: Le cas dans lequel le robot doit se trouver
 */
void Case_Select(int teemo_case) {

	switch (teemo_case) {
	case 1:
		SetStatusMessage("Mode Combat");
		Mode_Combat();
		break;
	case 2:
		SetStatusMessage("Mode Search");
		Mode_Search();
		break;
	case 3:
		SetStatusMessage("Mode Escape");
		Mode_Escape();
		break;
	case 4:
		SetStatusMessage("Mode Chasse");
		Mode_Chasse();
		break;
	}

}

/*
 *Systeme de combat du robot
 */
void Teemo_Attack() {

	if (Teemo_Targetting() == 1) {
		if (GetSystemEnergy(SYSTEM_LASERS) > 25) {
			FireWeapon(WEAPON_LASER, 0);
		}

		if (GetSensorData(2) <= 40 && GetSensorData(3) <= 40) {

		} else if (GetSensorData(2) > 40 && GetSensorData(3) > 40) {
			if (GetSystemEnergy(SYSTEM_MISSILES) == 100) {
				FireWeapon(WEAPON_MISSILE, 0);
			}
		}
	}

}

/*
 * Change l'etat dans lequel se trouve le bouclier
 * du robot ainsi qu'il allume les senseur.
 */
void Shield_Sensor_Charge() {

	//Shield Charge
	if (GetGeneratorStructure() >= 500) {
		if (GetSystemEnergy(SYSTEM_SHIELDS) < 800) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 600);
		} else if (GetSystemEnergy(SYSTEM_SHIELDS) < 1000) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 400);
		} else if (GetSystemEnergy(SYSTEM_SHIELDS) >= 1000) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 0);
		}
	} else if (GetGeneratorStructure() < 400) {
		if (GetSystemEnergy(SYSTEM_SHIELDS) < 800) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 500);
		} else if (GetSystemEnergy(SYSTEM_SHIELDS) < 1000) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 300);
		} else if (GetSystemEnergy(SYSTEM_SHIELDS) >= 1000) {
			SetSystemChargeRate(SYSTEM_SHIELDS, 0);
		}
	}

	//Sensors activation

	SetSensorStatus(0, 1);
	SetSensorStatus(1, 1);
	SetSensorStatus(2, 1);
	SetSensorStatus(3, 1);

}

/*
 * Le systeme de vise du robot
 *
 * SORTIE: Un int qui indique si il y a un enemie
 * 		   et ou il est.
 */
int Teemo_Targetting() {

	//Return 0 si aucun ennemie
	//Return 1 si ennemie locked
	//Return 2 si ennemie proche

	if (GetSensorData(0) == 1 && GetSensorData(1) == 1) {	//Tire sur la target
		return 1;
	} else if (GetSensorData(2) < GetSensorData(3)) {	//Si
		if (GetSensorData(0) == 1 && GetSensorData(1) == 0) {
			SetMotorSpeeds(15, 45);
			return 2;
		} else if (GetSensorData(0) == 0 && GetSensorData(1) == 0) {
			SetMotorSpeeds(70, 100);
			return 0;
		}
	} else if (GetSensorData(2) > GetSensorData(3)) {
		if (GetSensorData(0) == 0 && GetSensorData(1) == 1) {
			SetMotorSpeeds(45, 15);
			return 2;
		} else if (GetSensorData(0) == 0 && GetSensorData(1) == 0) {
			SetMotorSpeeds(100, 70);
			return 0;
		}
	} else if (GetSensorData(0) == 0 && GetSensorData(1) == 0) {
		return 0;
	}

}

/*
 * Mode de poursuite du robot
 */
void Mode_Chasse() {

	//Assigne les valeurs de recharges des armes

	SYSTEM teemo_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SHIELDS, SYSTEM_LASERS,
			SYSTEM_SENSORS, SYSTEM_MISSILES };
	SetSystemChargePriorites(teemo_prios);

	if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 300);
	} else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 0);
	}

	if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 150);
	} else if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 0);
	}
	//Behavior

	if (Teemo_Targetting() == 0 && GetSensorData(2) < 125) {
		SetMotorSpeeds(50, 100);
	} else if (Teemo_Targetting() == 0 && GetSensorData(3) < 125) {
		SetMotorSpeeds(100, 50);
	}

}

void Mode_Combat() {

	//Assigne les valeurs de recharges des armes

	SYSTEM teemo_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SHIELDS, SYSTEM_MISSILES,
			SYSTEM_SENSORS, SYSTEM_LASERS };
	SetSystemChargePriorites(teemo_prios);

	if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 350);
	} else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 0);
	}

	if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 450);
	} else if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 0);
	}
	//Behavior

	Teemo_Attack();
}

void Mode_Escape() {

	SYSTEM teemo_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SHIELDS, SYSTEM_SENSORS,
			SYSTEM_LASERS, SYSTEM_MISSILES };
	SetSystemChargePriorites(teemo_prios);

	if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 450);
	} else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 0);
	}

	if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
		if (GetSystemEnergy(SYSTEM_LASERS) == 50) {
			SetSystemChargeRate(SYSTEM_MISSILES, 600);
		} else if (GetSystemEnergy(SYSTEM_LASERS) != 50) {
			SetSystemChargeRate(SYSTEM_MISSILES, 350);
		}
		SetSystemChargeRate(SYSTEM_MISSILES, 250);
	} else if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 0);
	}
	//Behavior

	/*
	 * Comme le boost descend le bouclier,
	 * le boost ne s'activera pas si le
	 * bouclier est trop bas.
	 */
	if (GetSystemEnergy(SYSTEM_SHIELDS) > 600) {
		if (IsTurboOn() == 0) {
			TurboBoost();
		} else if (IsTurboOn() == 1) {
			SetMotorSpeeds(-100, -100);
		}
	}

	if (GetBumpInfo() == 0x01 || GetBumpInfo() == 0x02) {
		SetMotorSpeeds(50, 100);
	}
}

/*
 * Mode de recherche de cible du robot
 */
void Mode_Search() {

	float randomSearch = GetRandomNumber(4);

	//Assigne les valeurs de recharges des armes

	SYSTEM teemo_prios[NUM_ENERGY_SYSTEMS] = { SYSTEM_SHIELDS, SYSTEM_SENSORS,
			SYSTEM_LASERS, SYSTEM_MISSILES };
	SetSystemChargePriorites(teemo_prios);

	if (GetSystemEnergy(SYSTEM_LASERS) < 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 450);
	} else if (GetSystemEnergy(SYSTEM_LASERS) >= 50) {
		SetSystemChargeRate(SYSTEM_LASERS, 0);
	}

	if (GetSystemEnergy(SYSTEM_MISSILES) < 100) {
		if (GetSystemEnergy(SYSTEM_LASERS) == 50) {
			SetSystemChargeRate(SYSTEM_MISSILES, 600);
		}
	} else if (GetSystemEnergy(SYSTEM_LASERS) != 50) {
		SetSystemChargeRate(SYSTEM_MISSILES, 350);
	} else if (GetSystemEnergy(SYSTEM_MISSILES) >= 100) {
		SetSystemChargeRate(SYSTEM_MISSILES, 0);
	}

//Behavior

	/*
	 * Comme les robot peuvent se trouver n'importe ou,
	 * le robot cherche aleatoirement pour les trouver
	 */

	if (GetSensorData(2) <= 40 && GetSensorData(3) <= 40) {
		SetMotorSpeeds(-100, 100);
	} else if (GetSensorData(2) < GetSensorData(3)) {
		SetMotorSpeeds(100, 80);
	} else if (GetSensorData(2) > GetSensorData(3)) {
		SetMotorSpeeds(80, 100);
	} else if (GetSensorData(2) == GetSensorData(3)) {
		if (randomSearch <= 1.0) {
			SetMotorSpeeds(100, 100);
		} else if (randomSearch <= 2.0) {
			SetMotorSpeeds(15, 100);
		} else if (randomSearch <= 3.0) {
			SetMotorSpeeds(100, 15);
		}
	}

}

/*
 * Initiallisation des sensor du robot
 */
void TeemosEquipement(void) {

	AddSensor(0, SENSOR_RADAR, 330, 30, 100);
	AddSensor(1, SENSOR_RADAR, 0, 30, 100);
	AddSensor(2, SENSOR_RANGE, 350, 0, 100);
	AddSensor(3, SENSOR_RANGE, 10, 0, 100);
}
