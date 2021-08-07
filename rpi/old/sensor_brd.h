#ifndef SENSOR_BRD_H
#define SENSOR_BRD_H

#include <stdint.h>
#include <stdio.h>

#define PH_SENSOR 0
#define EC_SENSOR 1
#define WATER_LVL_SENSOR 2
#define TC_SENSOR1 3
#define TC_SENSOR2 4
#define TC_SENSOR3 5
#define TC_SENSOR4 6
#define CJ_SENSOR1 7
#define CJ_SENSOR2 8
#define CJ_SENSOR3 9
#define CJ_SENSOR4 10
#define HUM_SENSOR1 11
#define HUM_SENSOR2 12
#define HUM_SENSOR3 13
#define HUM_SENSOR4 14
#define TN_SENSOR1 15
#define TN_SENSOR2 16
#define TN_SENSOR3 17
#define TN_SENSOR4 18
#define LIGHT_SENSOR1 19
#define LIGHT_SENSOR2 20
#define LIGHT_SENSOR3 21
#define LIGHT_SENSOR4 22
#define MOIS_SENSOR1 23
#define MOIS_SENSOR2 24
#define MOIS_SENSOR3 25
#define MOIS_SENSOR4 26

#define PH_VCC 3.334
#define PH_VREF 1.978

#define EC_VCC 3.334
#define EC_HIGH_KVAL 1.122
#define EC_LOW_KVAL 1.078

/* (This typedef is probably unnecesary)
typedef enum sensor {ph = 0, ec = 1, wls = 2, tc1 = 3, tc2 = 4,
	tc3 = 5, tc4 = 6, cj1 = 7, cj2 = 8, cj3 = 9,
	cj4 = 10, hum1 = 11, hum2 = 12, hum3 = 13, hum4 = 14,
	tn1 = 15, tn2 = 16, tn3 = 17, tn4 = 18, light1 = 19,
	light2 = 20, light3 = 21, light4 = 22, mois1 = 23, mois2 = 24,
	mois3 = 25, mois4 = 26} sensor_t;
*/

// this function converts raw sensor result from mcu (rx data) to real final physical units
float raw_twater_to_float(uint8_t raw_data[4]);
float raw_to_float(uint8_t raw_data[4], int sensor_type, float temperature);

#endif /* SENSOR_BRD_H */
