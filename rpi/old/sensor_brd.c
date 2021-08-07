#include "sensor_brd.h"

float raw_twater_to_float(uint8_t raw_data[4]) {
	float temperature = 0;
	uint16_t temp_var;

	temp_var = ((uint16_t)raw_data[1] << 8) + raw_data[0];
	return ((float)temp_var / 4);
}

float raw_to_float(uint8_t raw_data[4], int sensor_type, float temperature) {
	float final_value = 0;	
	float ph_slope, ph_v_raw;
	float ec_v_raw;
	uint16_t temp_var1;
	float temp_var2, temp_var3;
	uint16_t light_scale_factor;

	switch (sensor_type) {
		case PH_SENSOR:
			temp_var1 = ((uint16_t)raw_data[0] << 8) + raw_data[1];
			ph_v_raw = ((float)temp_var1 / 32768) * PH_VCC;					
			ph_slope = 0.1884*temperature + 54.2;
			final_value = 7 - 1000*((ph_v_raw - PH_VREF)/ph_slope);
			break;
		case EC_SENSOR:
			temp_var1 = ((uint16_t)raw_data[0] << 8) + raw_data[1];
			ec_v_raw = ((float)temp_var1 / 32768) * EC_VCC;
			temp_var2 = (ph_v_raw / 820) / 0.0002;
			if (temp_var2 < 2.5) {
				temp_var3 = temp_var2 * EC_LOW_KVAL;
			} else {
				temp_var3 = temp_var2 * EC_HIGH_KVAL;
			}
			final_value = temp_var3 / (1 + 0.0185*(temperature - 25));
			break;
		case WATER_LVL_SENSOR:
			final_value = raw_data[0] * 0.6;
			break;
		case TC_SENSOR1:
		case TC_SENSOR2:
		case TC_SENSOR3:
		case TC_SENSOR4:
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			final_value = (float)temp_var1 / 4;
			break;
		case CJ_SENSOR1:
		case CJ_SENSOR2:
		case CJ_SENSOR3:
		case CJ_SENSOR4:
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			final_value = (float)temp_var1 / 16;
			break;	
		case HUM_SENSOR1:
		case HUM_SENSOR2:
		case HUM_SENSOR3:
		case HUM_SENSOR4:
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			final_value = 100 * ((float)temp_var1 / 65536);
			break;	
		case TN_SENSOR1:
		case TN_SENSOR2:
		case TN_SENSOR3:
		case TN_SENSOR4:
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			final_value = -45 + 175 * ((float)temp_var1 / 65536);
			break;	
		case LIGHT_SENSOR1:			
		case LIGHT_SENSOR2:			
		case LIGHT_SENSOR3:			
		case LIGHT_SENSOR4:			
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			light_scale_factor = ((uint16_t)raw_data[3] << 8) + raw_data[2];
			final_value = temp_var1 * 1.8432 / light_scale_factor;
			break;
		case MOIS_SENSOR1:						
		case MOIS_SENSOR2:						
		case MOIS_SENSOR3:						
		case MOIS_SENSOR4:
			temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
			final_value = (float)temp_var1;	
			break;						
	}
	return final_value;
}
