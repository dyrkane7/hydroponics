#include <time.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <wiringPi.h>
#include "nrf_driver_rpi/nrf_comm_rpi.h"
//#include "sensor_brd.h"

#define SENSOR_CTRL_ADDR_I 2
#define LIGHTING_CTRL_ADDR_I 1
#define PUMP_CTRL_ADDR_I 0
#define RF_CH 5
#define SPI_FREQ 500000
#define MAX_TX_MSG_LEN 8
#define MAX_RX_MSG_LEN 6

#define RADIO_RX_TIMEOUT -2
#define RADIO_TX_TIMEOUT -1
#define RADIO_NO_ERROR 0

#define PH_VCC 3.334
#define PH_VREF 1.978

#define EC_VCC 3.334
#define EC_HIGH_KVAL 1.122
#define EC_LOW_KVAL 1.078

#define ARG_VAL_IGNORE 100 // value for system paramaters that should not be updated

enum cmd_types {write_cmd = 0, read_cmd = 1};

// pump params: 
// t_start, t_stop, t_pump, t_cycle, t_now, temperature
// lighting params: 
// t_start, t_stop, t_now, intensity, temperature
// sensor brd params:
// wls - water level [in], tc# - thermocouple [C], cj# - thermocouple cold junction [C]
// hum# - humidity [%RH], tn# - node temperature [C], light# - light [lux] 
// mois# - soil moisture [arbitrary units]  
enum param_types {t_start = 0, t_stop = 1, t_pump = 2, t_cycle = 3, t_local = 4, intensity = 5, temperature = 6,
		ph = 7, ec = 8, wls = 9, tc1 = 10, tc2 = 11, tc3 = 12, tc4 = 13, cj1 = 14, cj2 = 15, 
		cj3 = 16, cj4 = 17, hum1 = 18, hum2 = 19, hum3 = 20, hum4 = 21, tn1 = 22, tn2 = 23, 
		tn3 = 24, tn4 = 25, light1 = 26, light2 = 27, light3 = 28, light4 = 29, mois1 = 30, 
		mois2 = 31, mois3 = 32, mois4 = 33};

enum nodes {pump = 0, lights = 1, sensors = 2};

typedef struct radio_config {
	uint8_t rf_ch;
	uint8_t rx_addr[5];
	uint8_t tx_addr[5];
	interface_t interface;
} radio_config_t;

typedef struct radio_pl {
	enum param_types param;
	enum cmd_types cmd;
	enum nodes node;
	struct tm p_local_tm, p_start_tm, p_stop_tm, p_pump_tm, p_cycle_tm;
	int intensity, temperature;
	float ph, ec, wls, tc1, tc2, tc3, tc4, cj1, cj2, cj3, cj4, hum1, hum2, hum3, hum4;
	float tn1, tn2, tn3, tn4, light1, light2, light3, light4, mois1, mois2, mois3, mois4; 
} radio_pl_t;

void RADIO_init(int fd, radio_config_t radio_config);
int RADIO_tx(int fd, radio_config_t config, radio_pl_t pl);  
int RADIO_rx(int fd, radio_config_t config, radio_pl_t *pl);
int RADIO_config_node(int fd, radio_config_t config, radio_pl_t rx_pl, radio_pl_t tx_pl);
void RADIO_read_param(int fd, radio_config_t config, radio_pl_t *rx_pl, radio_pl_t tx_pl);
void RADIO_write_param(int fd, radio_config_t config, radio_pl_t tx_pl);
void RADIO_write_and_verify_param(int fd, radio_config_t config, radio_pl_t rx_pl, radio_pl_t tx_pl);

void convert_tm_to_asc(struct tm p_tm, uint8_t ascii_bytes[6]);
void convert_dec_to_asc(int dec_val, uint8_t ascii_bytes[6]);
void pl_set_param(uint8_t ascii_bytes[6], struct radio_pl *pl);
int pl_compare_param(radio_pl_t rx_pl, radio_pl_t tx_pl);
float pl_read_param(enum param_types param, struct radio_pl*  pl);

float raw_twater_to_float(uint8_t raw_data[4]);
float raw_to_float(uint8_t raw_data[4], enum param_types sensor_type, float temperature);


