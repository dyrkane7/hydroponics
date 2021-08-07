#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include "nrf_driver_rpi/nrf_comm_rpi.h"
#include "sys_control.h"
//#include <ncurses.h>

void nrf_ce_set(int state);
static const char *device = "/dev/spidev0.0";

char *param_strings[] = {"PH", "EC[uS/cm]", "Water Level[in]", "Thermocouple #1[C]", 
	"Thermocouple #2[C]", "Thermocouple #3[C]", "Thermocouple #4[C]", "TC Cold Junction #1[C]",
	"TC Cold Junction #2[C]", "TC Cold Junction #3[C]", "TC Cold Junction #4[C]", "Probe #1 Humidity",
	"Probe #2 Humidity", "Probe #3 Humidity", "Probe #4 Humidity", "Probe #1 Temperature[C]", 
	"Probe #2 Temperature[C]", "Probe #3 Temperature[C]", "Probe #4 Temperature[C]",
	"Probe #1 Light Level[lux]", "Probe #2 Light Level[lux]", "Probe #3 Light Level[lux]", 
	"Probe #4 Light Level[lux]", "Probe #1 Moisture Level", "Probe #2 Moisture Level",
	"Probe #3 Moisture Level", "Probe #4 Moisture Level"};
 
const int smpl_interval_s = 60;
//enum param_types params_list[] = {ph, ec, wls, tc1, cj1, hum1, tn1, light1, mois1};
enum param_types params_list[] = {tc1, cj1, wls, hum1, tn1, ec, light1, ph, mois1};
//enum param_types params_list[] = {tc1, cj1, hum1, tn1, light1, mois1};
FILE *datalog_f;

int main(int argc, char *argv[]) {

	int i;
	int fd; // file descriptor for SPI interface
	interface_t interface;
	uint8_t tx_buf[MAX_TX_MSG_LEN];
	uint8_t rx_buf[MAX_RX_MSG_LEN];
	interface.ce_set = nrf_ce_set;

	radio_config_t radio_config;
	radio_config.interface = interface;
	radio_config.rf_ch = RF_CH;
	for (i = 0; i < 5; i++) { // 5 channels 
		radio_config.rx_addr[i] = pipe_addr[SENSOR_CTRL_ADDR_I][i];
		radio_config.tx_addr[i] = pipe_addr[SENSOR_CTRL_ADDR_I][i];
	}
	if (wiringPiSetup() == -1) {
		printf("WiringPiSetup error\n");
		return 1;
	}
	pinMode(0, OUTPUT);

	fd = open(device, O_RDWR);
	if (fd < 0) {
		printf("can't open spi interface\n");
	}

	struct spi_ioc_transfer mesg = {
		.tx_buf = (unsigned long)tx_buf,
		.rx_buf = (unsigned long)rx_buf,
		.len = 1,
		.speed_hz = SPI_FREQ,
		.bits_per_word = 8,
		.cs_change = 0,
		.delay_usecs = 10,
	};

	int pw;
	radio_pl_t rx_pl, tx_pl;
	
	rx_pl.node = sensors;
	
	tx_pl.node = sensors;
	tx_pl.cmd = read_cmd;

	char datalog_fp[80];
	time_t t = time(NULL);
	struct tm *cur_tm = localtime(&t);
	
	sprintf(datalog_fp, "hydro_datalog_%02u%02u%02u_%02u%02u%02u.csv", 
		cur_tm->tm_mon+1, cur_tm->tm_mday, cur_tm->tm_year%100, 
		cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec
	);
	printf("fp contents: %s\n", datalog_fp);
	datalog_f = fopen(datalog_fp, "w+");
	fprintf(datalog_f, "Date,Time,");
	for (i = 0; i < (sizeof(params_list) / sizeof(params_list[0])); i++) {	
		fprintf(datalog_f, "%s,", param_strings[params_list[i]-7]); 	
	}
	fprintf(datalog_f, "\n");	
	fclose(datalog_f);

	RADIO_init(fd, radio_config);
	printf("Radio initialized. Press any key to start logging...\n");
	getchar();

	unsigned int t0, t1;
	char time_str[10];
	char date_str[10];
	while (1) {
		t0 = millis();
		tx_pl.param = tc4; // read water temperature
		rx_pl.param = tc4;
		RADIO_read_param(fd, radio_config, &rx_pl, tx_pl);
		datalog_f = fopen(datalog_fp, "a");
		t = time(NULL);
		cur_tm = localtime(&t);
		sprintf(time_str, "%02u:%02u:%02u", cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec); 		
		printf("%s\n\n", time_str);
		sprintf(date_str, "%02u/%02u/%02u", cur_tm->tm_mon+1, cur_tm->tm_mday, cur_tm->tm_year%100);
		fprintf(datalog_f, "%s,", date_str);  
		fprintf(datalog_f, "%s,", time_str);  	
		for (i = 0; i < (sizeof(params_list) / sizeof(params_list[0])); i++) {
			tx_pl.param = params_list[i];
			rx_pl.param = params_list[i];
			RADIO_read_param(fd, radio_config, &rx_pl, tx_pl);
			fprintf(datalog_f, "%.2f,", pl_read_param(params_list[i], &rx_pl));
			printf("\n");
			//sleep(0.1); // change this later
		}
		fprintf(datalog_f, "\n");	
		fclose(datalog_f);
		while (millis() - t0 < smpl_interval_s*1000) {
			sleep(1);
		}		
	}
	return 0;
}

void nrf_ce_set(int state) {
	digitalWrite(0, state);
}
