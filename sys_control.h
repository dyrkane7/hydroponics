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

#define LIGHTING_CTRL_ADDR_I 1
#define PUMP_CTRL_ADDR_I 0
#define RF_CH 2
#define SPI_FREQ 500000
#define MAX_TX_MSG_LEN 8
#define MAX_RX_MSG_LEN 6

#define RADIO_RX_TIMEOUT -2
#define RADIO_TX_TIMEOUT -1
#define RADIO_NO_ERROR 0

#define ARG_VAL_IGNORE 100 // value for system paramaters that should not be updated

enum cmd_types {write_cmd = 0, read_cmd = 1};
enum param_types {t_start = 0, t_stop = 1, t_pump = 2, t_cycle = 3, t_local = 4, intensity = 5, temperature = 6};
enum nodes {pump = 0, lights = 1};

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


