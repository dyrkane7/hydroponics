#include "sys_control.h"

void RADIO_init(int fd, radio_config_t radio_config) {
        uint8_t tx[1] = {0xff};
        uint8_t rx[1] = {0x00};

        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx,
                .rx_buf = (unsigned long)rx,
                .len = 1,
                .speed_hz = SPI_FREQ,
                .bits_per_word = 8,
                .cs_change = 0,
                .delay_usecs = 10,
        };
	
	NRF_reset(&tr, fd, radio_config.interface);
	NRF_en_rx_pipes(&tr, fd, 0x01);
	NRF_set_tx_addr(&tr, fd, radio_config.tx_addr);
	NRF_set_rx_addr(&tr, fd, radio_config.rx_addr, 0);
	NRF_set_rf_ch(&tr, fd, radio_config.rf_ch);
        NRF_config_pw(&tr, fd, 0, 4); // payload width doesn't matter b/c dynamic pw is enabled

        NRF_clear_tx_ds(&tr, fd);
        NRF_clear_rx_dr(&tr, fd);
        NRF_clear_max_rt(&tr, fd);
        NRF_config_auto_retr(&tr, fd, 4000, 15);
        NRF_config_dpl(&tr, fd, ENABLE);

        NRF_config_rx_dr_int(&tr, fd, ENABLE);// enable rx data ready interrupt on IRQ pin
        NRF_config_tx_ds_int(&tr, fd, ENABLE);// enable tx data sent interrupt on IRQ pin

        printf("config reg: %.2X\n", NRF_read_register(&tr, fd, CONFIG_REG_ADDR));
        printf("status reg: %.2X\n", NRF_read_register(&tr, fd, STATUS_REG_ADDR));
        printf("feature_reg_data: %.2X\n", NRF_read_register(&tr, fd, FEATURE_REG_ADDR));
        printf("fifo_stat_reg_data: %.2X\n", NRF_read_register(&tr, fd, FIFO_STATUS_REG_ADDR));
        printf("rx p0 pw: %.2X\n", NRF_read_register(&tr, fd, RX_P0_PW_REG_ADDR));
        printf("tx p0 addr: %.2X\n", NRF_read_register(&tr, fd, TX_PIPE_ADDR_REG_ADDR));
        printf("rx p0 addr: %.2X\n", NRF_read_register(&tr, fd, RX_P0_ADDR_REG_ADDR));
        printf("auto retr byte: %.2X\n", NRF_read_register(&tr, fd, SETUP_RETR_ADDR));
        printf("dynpd reg: %.2X\n", NRF_read_register(&tr, fd, DYNPD_REG_ADDR));
        printf("en rx pipes reg: %.2X\n\n", NRF_read_register(&tr, fd, EN_RX_PIPE_REG_ADDR));
        printf("rf ch: %.2X\n\n", NRF_read_register(&tr, fd, RF_CH_REG_ADDR));	
}

int RADIO_tx(int fd, radio_config_t config, radio_pl_t pl) {
	int i;
	unsigned long int t0, t_loop;
	unsigned long int tx_timeout_ms = 1000;
        uint8_t tx_pl[MAX_TX_MSG_LEN];
	uint8_t rx_pl[MAX_RX_MSG_LEN];
	uint8_t pl_param_val[6];
		
	struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx_pl,
                .rx_buf = (unsigned long)rx_pl,
                .len = 1,
                .speed_hz = SPI_FREQ,
                .bits_per_word = 8,
                .cs_change = 0,
                .delay_usecs = 10,
        };
	
	if (pl.cmd == read_cmd) {
		//printf("read cmd\n");
		tx_pl[0] = 'R';
		switch (pl.param) {
			case t_start:
				tx_pl[1] = 'S';
				break;
			case t_stop:
				tx_pl[1] = 'E';
				break;
			case t_pump:
				tx_pl[1] = 'P';
				break;
			case t_cycle:
				tx_pl[1] = 'C';
				break;
			case t_local:
				tx_pl[1] = 'N';
				break;
			case intensity:
				tx_pl[1] = 'I';
				break;
			case temperature:
				tx_pl[1] = 'T';
				break;
			case ph:
				tx_pl[1] = 'P';
				tx_pl[2] = '1';
				break;
			case ec:
				tx_pl[1] = 'E';
				tx_pl[2] = '1';
				break;
			case wls:
				tx_pl[1] = 'W';
				tx_pl[2] = '1';
				break;
			case tc1:
				tx_pl[1] = 'T';
				tx_pl[2] = '1';
				break;
			case tc2:
				tx_pl[1] = 'T';
				tx_pl[2] = '2';
				break;
			case tc3:
				tx_pl[1] = 'T';
				tx_pl[2] = '3';
				break;
			case tc4:
				tx_pl[1] = 'T';
				tx_pl[2] = '4';
				break;
			case cj1:
				tx_pl[1] = 'C';
				tx_pl[2] = '1';
				break;
			case cj2:
				tx_pl[1] = 'C';
				tx_pl[2] = '2';
				break;
			case cj3:
				tx_pl[1] = 'C';
				tx_pl[2] = '3';
				break;
			case cj4:
				tx_pl[1] = 'C';
				tx_pl[2] = '4';
				break;		
			case hum1:
				tx_pl[1] = 'H';
				tx_pl[2] = '1';
				break;
			case hum2:
				tx_pl[1] = 'H';
				tx_pl[2] = '2';
				break;
			case hum3:
				tx_pl[1] = 'H';
				tx_pl[2] = '3';
				break;
			case hum4:
				tx_pl[1] = 'H';
				tx_pl[2] = '4';
				break;
			case tn1:
				tx_pl[1] = 't';
				tx_pl[2] = '1';
				break;
			case tn2:
				tx_pl[1] = 't';
				tx_pl[2] = '2';
				break;
			case tn3:
				tx_pl[1] = 't';
				tx_pl[2] = '3';
				break;
			case tn4:
				tx_pl[1] = 't';
				tx_pl[2] = '4';
				break;
			case light1:
				tx_pl[1] = 'L';
				tx_pl[2] = '1';
				break;
			case light2:
				tx_pl[1] = 'L';
				tx_pl[2] = '2';
				break;
			case light3:
				tx_pl[1] = 'L';
				tx_pl[2] = '3';
				break;
			case light4:
				tx_pl[1] = 'L';
				tx_pl[2] = '4';
				break;
			case mois1:
				tx_pl[1] = 'M';
				tx_pl[2] = '1';
				break;
			case mois2:
				tx_pl[1] = 'M';
				tx_pl[2] = '2';
				break;
			case mois3:
				tx_pl[1] = 'M';
				tx_pl[2] = '3';
				break;
			case mois4:
				tx_pl[1] = 'M';
				tx_pl[2] = '4';
				break;
		}

	} else if (pl.cmd == write_cmd) {
		printf("write cmd\n");
		tx_pl[0] = 'W';
		switch (pl.param) {
			case t_start:
				convert_tm_to_asc(pl.p_start_tm, pl_param_val);		
				tx_pl[1] = 'S';
				break;
			case t_stop:
				convert_tm_to_asc(pl.p_stop_tm, pl_param_val);		
				tx_pl[1] = 'E';
				break;
			case t_pump:
				convert_tm_to_asc(pl.p_pump_tm, pl_param_val);		
				tx_pl[1] = 'P';
				break;
			case t_cycle:
				convert_tm_to_asc(pl.p_cycle_tm, pl_param_val);		
				tx_pl[1] = 'C';
				break;
			case t_local:
				convert_tm_to_asc(pl.p_local_tm, pl_param_val);		
				tx_pl[1] = 'N';
				break;
			case intensity:
				convert_dec_to_asc(pl.intensity, pl_param_val);
				tx_pl[1] = 'I';
				break;
			case temperature:
				convert_dec_to_asc(pl.temperature, pl_param_val);
				tx_pl[1] = 'T';
				break;
		}	
		for (i = 0; i < 6; i++) {
			tx_pl[i + 2] = pl_param_val[i];
		}
	}
	for (i = 0; i < 5; i++) {
		config.rx_addr[i] = pipe_addr[pl.node][i];
		config.tx_addr[i] = pipe_addr[pl.node][i];
	}	
	NRF_set_tx_addr(&tr, fd, config.tx_addr);
        NRF_set_rx_addr(&tr, fd, config.rx_addr, 0);

        if (tx_pl[0] == 'W') {
                NRF_write_payload(&tr, fd, tx_pl, 8);
        } else if (tx_pl[0] == 'R') {
        	if (pl.node != sensors) {
			NRF_write_payload(&tr, fd, tx_pl, 2);
		} else if (pl.node == sensors) {
			//printf("writing to sensor node\n");
			NRF_write_payload(&tr, fd, tx_pl, 3);
		}
        } else {
		printf("Error: tx_pl[0] must be 'W' or 'R'\n");
	}	
	
        NRF_clear_prim_rx(&tr, fd);
        delayMicroseconds(100); //added 5/8/21 1pm
	config.interface.ce_set(1);
        //printf("waiting for data to send\n");
 	t0 = millis();
	t_loop = 0;
        while (!(TX_DS_BIT & NRF_read_status_reg(&tr, fd)) && (t_loop < tx_timeout_ms)) {
		t_loop = (unsigned long)millis() - t0;
		if (MAX_RT_BIT & NRF_read_status_reg(&tr, fd)) {
			printf("max_rt set\n");
			config.interface.ce_set(0);
			NRF_clear_max_rt(&tr, fd);	
			config.interface.ce_set(1);
			printf("t_loop: %u\n", t_loop);
			//printf("max_rt: %u\n", MAX_RT_BIT & NRF_read_status_reg(&tr, fd));
		}
		//printf("+");
		
        }
        
        config.interface.ce_set(0);
	NRF_clear_max_rt(&tr, fd);
	if (t_loop >= tx_timeout_ms) {
		printf("t_loop: %u", t_loop);
		NRF_flush_tx(&tr, fd);
		NRF_set_prim_rx(&tr, fd);
		printf("tx timeout\n");
		return RADIO_TX_TIMEOUT;
	} else 
		NRF_flush_tx(&tr, fd);{
		NRF_clear_tx_ds(&tr, fd);
		//printf("data sent\n");
		return RADIO_NO_ERROR;
	} 				
}

int RADIO_rx(int fd, radio_config_t config, radio_pl_t *pl) {
        int i, pw;
        unsigned int t0, t_loop;
        unsigned int rx_timeout_ms = 8000;
        uint8_t tx_pl[MAX_TX_MSG_LEN];
        uint8_t rx_pl[MAX_RX_MSG_LEN] = {0,0,0,0,0,0};
        uint8_t pl_param_val[6];

        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx_pl,
                .rx_buf = (unsigned long)rx_pl,
                .len = 1,
                .speed_hz = SPI_FREQ,
                .bits_per_word = 8,
                .cs_change = 0,
                .delay_usecs = 10,
        };
	
	NRF_set_prim_rx(&tr, fd);
	config.interface.ce_set(1);
	//printf("waiting for rx data...\n");
	t0 = millis();
	t_loop = 0;
        while (!(RX_DR_BIT & NRF_read_status_reg(&tr, fd)) && (t_loop < rx_timeout_ms)) {
		t_loop = millis() - t0;
        	//printf("+");
        }
        config.interface.ce_set(0);
	if (t_loop < rx_timeout_ms) {
        	NRF_clear_rx_dr(&tr, fd);
		pw = NRF_read_top_rx_pw(&tr, fd);
        	//printf("pw: %u\n", pw);
		NRF_read_payload(&tr, fd, rx_pl, pw);

        	printf("rx data: ");
        	for (i = 0; i < pw; i++) {
        		printf("%.2X,", rx_pl[i]);
        	}
        	printf("\n");
		NRF_flush_rx(&tr, fd);

		pl_set_param(rx_pl, pl); 

		return RADIO_NO_ERROR;
	} else {
		NRF_flush_rx(&tr, fd);
		printf("rx timeout\n");	
		return RADIO_RX_TIMEOUT;
	}
}

int RADIO_config_node(int fd, radio_config_t config, radio_pl_t rx_pl, radio_pl_t tx_pl) {
	//radio_pl_t rx_pl, tx_pl;
	uint8_t tx[32] = {0};
	uint8_t rx[32] = {0};	
	int i;
	int rx_result = -1;

        struct spi_ioc_transfer tr = {
                .tx_buf = (unsigned long)tx,
                .rx_buf = (unsigned long)rx,
                .len = 1,
                .speed_hz = SPI_FREQ,
                .bits_per_word = 8,
                .cs_change = 0,
                .delay_usecs = 10,
        };

	if (tx_pl.node == pump) {
		for (i = 0; i < 5; i++) {
			config.rx_addr[i] = pipe_addr[0][i];
			config.tx_addr[i] = pipe_addr[0][i];
		}
	} else if (tx_pl.node == lights) {
		for (i = 0; i < 5; i++) {
			config.rx_addr[i] = pipe_addr[1][i];
			config.tx_addr[i] = pipe_addr[1][i];
		}
	} else if (tx_pl.node == sensors) {
		for (i = 0; i < 5; i++) {
			config.rx_addr[i] = pipe_addr[2][i];
			config.tx_addr[i] = pipe_addr[2][i];
		}
	}
	rx_pl.cmd = read_cmd; // always a read command
	
	
	printf("t_start\n");
	rx_pl.param = t_start;
	tx_pl.param = t_start;
	do {
		RADIO_write_param(fd, config, tx_pl);
		RADIO_read_param(fd, config, &rx_pl, tx_pl);
	} while (1 != pl_compare_param(rx_pl, tx_pl));

	printf("t_stop\n");
	rx_pl.param = t_stop;
	tx_pl.param = t_stop;
	do {
		RADIO_write_param(fd, config, tx_pl);
		RADIO_read_param(fd, config, &rx_pl, tx_pl);
	} while (1 != pl_compare_param(rx_pl, tx_pl));
	
	printf("t_local\n");
	rx_pl.param = t_local;
	tx_pl.param = t_local;
	do {
		RADIO_write_param(fd, config, tx_pl);
		RADIO_read_param(fd, config, &rx_pl, tx_pl);
	} while (1 != pl_compare_param(rx_pl, tx_pl));
	
	if (tx_pl.node == lights) {
		printf("intensity\n");
		rx_pl.param = intensity;
		tx_pl.param = intensity;
		do {
			RADIO_write_param(fd, config, tx_pl);
			RADIO_read_param(fd, config, &rx_pl, tx_pl);
		} while (1 != pl_compare_param(rx_pl, tx_pl));
	} else if (tx_pl.node == pump) {
		printf("t_pump\n");
		rx_pl.param = t_pump;
		tx_pl.param = t_pump;
		do {
			RADIO_write_param(fd, config, tx_pl);
			RADIO_read_param(fd, config, &rx_pl, tx_pl);
		} while (1 != pl_compare_param(rx_pl, tx_pl));
		
		printf("t_cycle\n");
		rx_pl.param = t_cycle;
		tx_pl.param = t_cycle;
		do {
			RADIO_write_param(fd, config, tx_pl);
			RADIO_read_param(fd, config, &rx_pl, tx_pl);
		} while (1 != pl_compare_param(rx_pl, tx_pl));
	}	
}

void RADIO_read_param(int fd, radio_config_t config, radio_pl_t *rx_pl, radio_pl_t tx_pl) {
	tx_pl.cmd = read_cmd;
	do {
		while (RADIO_NO_ERROR != RADIO_tx(fd, config, tx_pl)) {delay(1);}
	} while (RADIO_NO_ERROR != RADIO_rx(fd, config, rx_pl));
}

void RADIO_write_param(int fd, radio_config_t config, radio_pl_t tx_pl) {
	tx_pl.cmd = write_cmd;
	while (RADIO_NO_ERROR != RADIO_tx(fd, config, tx_pl)) {delay(1);}
}

void RADIO_write_and_verify_param(int fd, radio_config_t config, radio_pl_t rx_pl, radio_pl_t tx_pl) {
		do {
			RADIO_write_param(fd, config, tx_pl);
			RADIO_read_param(fd, config, &rx_pl, tx_pl);
		} while (1 != pl_compare_param(rx_pl, tx_pl));	
}

void convert_tm_to_asc(struct tm p_tm, uint8_t ascii_bytes[6]) {
	
	ascii_bytes[0] = (p_tm.tm_sec % 10) + 48;
	ascii_bytes[1] = (p_tm.tm_sec / 10) + 48;
	ascii_bytes[2] = (p_tm.tm_min % 10) + 48;
	ascii_bytes[3] = (p_tm.tm_min / 10) + 48;
	ascii_bytes[4] = (p_tm.tm_hour % 10) + 48;
	ascii_bytes[5] = (p_tm.tm_hour / 10) + 48;
	
}

void convert_dec_to_asc(int dec_val, uint8_t ascii_bytes[6]) {
	ascii_bytes[0] = 48 + (dec_val % 10);
	ascii_bytes[1] = 48 + (dec_val % 100) / 10;
	ascii_bytes[2] = 48 + (dec_val % 1000) / 100;
	ascii_bytes[3] = 48 + (dec_val % 10000) / 1000;
	ascii_bytes[4] = 48 + (dec_val % 100000) / 10000;
	ascii_bytes[5] = 48 + (dec_val % 1000000) / 100000;	
} 

int pl_compare_param(radio_pl_t rx_pl, radio_pl_t tx_pl) {
	if (rx_pl.param != tx_pl.param) {
		printf("ERROR: rx_pl.param is not the same is tx_pl.param\n");
		return -1;
	}
	switch (tx_pl.param) {
		case t_start:
			if (tx_pl.p_start_tm.tm_sec == rx_pl.p_start_tm.tm_sec &&
				tx_pl.p_start_tm.tm_min == rx_pl.p_start_tm.tm_min &&
					tx_pl.p_start_tm.tm_hour == rx_pl.p_start_tm.tm_hour) {
				//printf("tx sec: %u, rx sec: %u\n", tx_pl.p_start_tm.tm_sec, rx_pl.p_start_tm.tm_sec);
				return 1;
			} else {
				//printf("tx sec: %u, rx sec: %u\n", tx_pl.p_start_tm.tm_min, rx_pl.p_start_tm.tm_min);
				return 0;
			}
		case t_stop:
			if (tx_pl.p_stop_tm.tm_sec == rx_pl.p_stop_tm.tm_sec &&
				tx_pl.p_stop_tm.tm_min == rx_pl.p_stop_tm.tm_min &&
					tx_pl.p_stop_tm.tm_hour == rx_pl.p_stop_tm.tm_hour) {
				return 1;
			} else {
				return 0;
			}
		case t_pump:
			if (tx_pl.p_pump_tm.tm_sec == rx_pl.p_pump_tm.tm_sec &&
				tx_pl.p_pump_tm.tm_min == rx_pl.p_pump_tm.tm_min &&
					tx_pl.p_pump_tm.tm_hour == rx_pl.p_pump_tm.tm_hour) {
				return 1;
			} else {
				return 0;
			}
		case t_cycle:
			if (tx_pl.p_cycle_tm.tm_sec == rx_pl.p_cycle_tm.tm_sec &&
				tx_pl.p_cycle_tm.tm_min == rx_pl.p_cycle_tm.tm_min &&
					tx_pl.p_cycle_tm.tm_hour == rx_pl.p_cycle_tm.tm_hour) {
				return 1;
			} else {
				return 0;
			}
		case t_local:
			if (tx_pl.p_local_tm.tm_sec == rx_pl.p_local_tm.tm_sec &&
				tx_pl.p_local_tm.tm_min == rx_pl.p_local_tm.tm_min &&
					tx_pl.p_local_tm.tm_hour == rx_pl.p_local_tm.tm_hour) {
				return 1;
			} else {
				return 0;
			}
		case intensity:
			if (tx_pl.intensity == rx_pl.intensity) {
				return 1;
			} else {
				return 0;
			}
	}	
	printf("ERROR: invalid param value\n");	
	return -1;
}

void pl_set_param(uint8_t ascii_bytes[6], struct radio_pl *pl) {
	switch (pl->param) {
		case t_start:
			pl->p_start_tm.tm_sec = 10 * (ascii_bytes[1] - '0') + (ascii_bytes[0] - '0');
			pl->p_start_tm.tm_min = 10 * (ascii_bytes[3] - '0') + (ascii_bytes[2] - '0');
			pl->p_start_tm.tm_hour = 10 * (ascii_bytes[5] - '0') + (ascii_bytes[4] - '0');
			printf("start_tm sec: %u\n", pl->p_start_tm.tm_sec);
			printf("start_tm min: %u\n", pl->p_start_tm.tm_min);
			printf("start_tm hour: %u\n", pl->p_start_tm.tm_hour);
			break;
		case t_stop:
			pl->p_stop_tm.tm_sec = 10 * (ascii_bytes[1] - '0') + (ascii_bytes[0] - '0');
			pl->p_stop_tm.tm_min = 10 * (ascii_bytes[3] - '0') + (ascii_bytes[2] - '0');
			pl->p_stop_tm.tm_hour = 10 * (ascii_bytes[5] - '0') + (ascii_bytes[4] - '0');
			printf("stop_tm sec: %u\n", pl->p_stop_tm.tm_sec);
			printf("stop_tm min: %u\n", pl->p_stop_tm.tm_min);
			printf("stop_tm hour: %u\n", pl->p_stop_tm.tm_hour);
			break;
		case t_cycle:
			pl->p_cycle_tm.tm_sec = 10 * (ascii_bytes[1] - '0') + (ascii_bytes[0] - '0');
			pl->p_cycle_tm.tm_min = 10 * (ascii_bytes[3] - '0') + (ascii_bytes[2] - '0');
			pl->p_cycle_tm.tm_hour = 10 * (ascii_bytes[5] - '0') + (ascii_bytes[4] - '0');
			printf("cycle_tm sec: %u\n", pl->p_cycle_tm.tm_sec);
			printf("cycle_tm min: %u\n", pl->p_cycle_tm.tm_min);
			printf("cycle_tm hour: %u\n", pl->p_cycle_tm.tm_hour);
			break;
		case t_pump:
			pl->p_pump_tm.tm_sec = 10 * (ascii_bytes[1] - '0') + (ascii_bytes[0] - '0');
			pl->p_pump_tm.tm_min = 10 * (ascii_bytes[3] - '0') + (ascii_bytes[2] - '0');
			pl->p_pump_tm.tm_hour = 10 * (ascii_bytes[5] - '0') + (ascii_bytes[4] - '0');
			printf("pump_tm sec: %u\n", pl->p_pump_tm.tm_sec);
			printf("pump_tm min: %u\n", pl->p_pump_tm.tm_min);
			printf("pump_tm hour: %u\n", pl->p_pump_tm.tm_hour);
			break;
		case t_local:
			pl->p_local_tm.tm_sec = 10 * (ascii_bytes[1] - '0') + (ascii_bytes[0] - '0');
			pl->p_local_tm.tm_min = 10 * (ascii_bytes[3] - '0') + (ascii_bytes[2] - '0');
			pl->p_local_tm.tm_hour = 10 * (ascii_bytes[5] - '0') + (ascii_bytes[4] - '0');
			printf("local_tm sec: %u\n", pl->p_local_tm.tm_sec);
			printf("local_tm min: %u\n", pl->p_local_tm.tm_min);
			printf("local_tm hour: %u\n", pl->p_local_tm.tm_hour);
			break;
		case intensity:
			pl->intensity = 0;
			pl->intensity += (ascii_bytes[0] - '0'); // ones place
			pl->intensity += 10 * (ascii_bytes[1] - '0'); // tens place
			pl->intensity += 100 * (ascii_bytes[2] - '0'); // hundreds place
			pl->intensity += 1000 * (ascii_bytes[3] - '0'); // thousands place
			printf("intensity: %u\n", pl->intensity);
			break;
		case temperature:
			pl->temperature = 0;
			pl->temperature += (ascii_bytes[0] - '0'); // ones place
			pl->temperature += 10 * (ascii_bytes[1] - '0'); // tens place
			pl->temperature += 100 * (ascii_bytes[2] - '0'); // hundreds place
			pl->temperature += 1000 * (ascii_bytes[3] - '0'); // thousands place
			printf("temperature: %u\n", pl->temperature);
			break;
		case ph: // assumes water temperature sensor is TC#4
			pl->ph = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("ph: %f\n", pl->ph);
			break;   
		case ec: // assumes water temperature sensor is TC#4
			pl->ec = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("ec: %f\n", pl->ec);
			break;   
		case wls: 
			pl->wls = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("wls: %f\n", pl->wls);
			break;   
		case tc1: 
			pl->tc1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tc1: %f\n", pl->tc1);
			break;   
		case tc2: 
			pl->tc2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tc2: %f\n", pl->tc2);
			break;   
		case tc3: 
			pl->tc3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tc3: %f\n", pl->tc3);
			break;   
		case tc4: 
			pl->tc4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tc4: %f\n", pl->tc4);
			break;   
		case cj1: 
			pl->cj1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("cj1: %f\n", pl->cj1);
			break;   
		case cj2: 
			pl->cj2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("cj2: %f\n", pl->cj2);
			break;   
		case cj3: 
			pl->cj3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("cj3: %f\n", pl->cj3);
			break;   
		case cj4: 
			pl->cj4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("cj4: %f\n", pl->cj4);
			break;   
		case hum1: 
			pl->hum1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("hum1: %f\n", pl->hum1);
			break;   
		case hum2: 
			pl->hum2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("hum2: %f\n", pl->hum2);
			break;   
		case hum3: 
			pl->hum3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("hum3: %f\n", pl->hum3);
			break;   
		case hum4: 
			pl->hum4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("hum4: %f\n", pl->hum4);
			break;   
		case tn1: 
			pl->tn1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tn1: %f\n", pl->tn1);
			break;   
		case tn2: 
			pl->tn2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tn2: %f\n", pl->tn2);
			break;   
		case tn3: 
			pl->tn3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tn3: %f\n", pl->tn3);
			break;   
		case tn4: 
			pl->tn4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("tn4: %f\n", pl->tn4);
			break;   
		case light1: 
			pl->light1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("light1: %f\n", pl->light1);
			break;   
		case light2: 
			pl->light2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("light2: %f\n", pl->light2);
			break;   
		case light3: 
			pl->light3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("light3: %f\n", pl->light3);
			break;   
		case light4: 
			pl->light4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("light4: %f\n", pl->light4);
			break;   
		case mois1: 
			pl->mois1 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("mois1: %f\n", pl->mois1);
			break;   
		case mois2: 
			pl->mois2 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("mois2: %f\n", pl->mois2);
			break;   
		case mois3: 
			pl->mois3 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("mois3: %f\n", pl->mois3);
			break;   
		case mois4: 
			pl->mois4 = raw_to_float(ascii_bytes, pl->param, pl->tc4);
			printf("mois4: %f\n", pl->mois4);
			break;   
	}	
}

float pl_read_param(enum param_types param, struct radio_pl*  pl) {
	float result = 0;
	switch (param) {	
		case ph: // assumes water temperature sensor is TC#4
			result = pl->ph;
			break;   
		case ec: // assumes water temperature sensor is TC#4
			result = pl->ec;
			break;   
		case wls: 
			result = pl->wls;
			break;   
		case tc1: 
			result = pl->tc1;
			break;   
		case tc2: 
			result = pl->tc2;
			break;   
		case tc3: 
			result = pl->tc3;
			break;   
		case tc4: 
			result = pl->tc4;
			break;   
		case cj1: 
			result = pl->cj1;
			break;   
		case cj2: 
			result = pl->cj2;
			break;   
		case cj3: 
			result = pl->cj3;
			break;   
		case cj4: 
			result = pl->cj4;
			break;   
		case hum1: 
			result = pl->hum1;
			break;   
		case hum2: 
			result = pl->hum2;
			break;   
		case hum3: 
			result = pl->hum3;
			break;   
		case hum4: 
			result = pl->hum4;
			break;   
		case tn1: 
			result = pl->tn1;
			break;   
		case tn2: 
			result = pl->tn2;
			break;   
		case tn3: 
			result = pl->tn3;
			break;   
		case tn4: 
			result = pl->tn4;
			break;   
		case light1: 
			result = pl->light1;
			break;   
		case light2: 
			result = pl->light2;
			break;   
		case light3: 
			result = pl->light3;
			break;   
		case light4: 
			result = pl->light4;
			break;   
		case mois1: 
			result = pl->mois1;
			break;   
		case mois2: 
			result = pl->mois2;
			break;   
		case mois3: 
			result = pl->mois3;
			break;   
		case mois4: 
			result = pl->mois4;
			break;   
	}	
	return result;
}

float raw_twater_to_float(uint8_t raw_data[4]) {
        float temperature = 0;
        uint16_t temp_var;

        temp_var = ((uint16_t)raw_data[1] << 8) + raw_data[0];
        return ((float)temp_var / 4);
}

float raw_to_float(uint8_t raw_data[4], enum param_types sensor_type, float temperature) {
        float final_value = 0;
        float ph_slope, ph_v_raw;
        float ec_v_raw;
        uint16_t temp_var1;
        float temp_var2, temp_var3;
        uint16_t light_scale_factor;

        switch (sensor_type) {
                case ph:
                        temp_var1 = ((uint16_t)raw_data[0] << 8) + raw_data[1];
                        ph_v_raw = ((float)temp_var1 / 32768) * PH_VCC;
                        ph_slope = 0.1884*temperature + 54.2;
                        final_value = 7 - 1000*((ph_v_raw - PH_VREF)/ph_slope);
                        break;
                case ec:
                        temp_var1 = ((uint16_t)raw_data[0] << 8) + raw_data[1];
                        ec_v_raw = ((float)temp_var1 / 32768) * EC_VCC;
                        temp_var2 = (ec_v_raw / 820) / 0.0002;
                        if (temp_var2 < 2.5) {
                                temp_var3 = temp_var2 * EC_LOW_KVAL;
                        } else {
                                temp_var3 = temp_var2 * EC_HIGH_KVAL;
                        }
                        final_value = temp_var3 / (1 + 0.0185*(temperature - 25));
                        break;
                case wls:
                        final_value = raw_data[0] * 0.6;
                        break;
                case tc1:
                case tc2:
                case tc3:
                case tc4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        final_value = (float)temp_var1 / 4;
                        break;
                case cj1:
                case cj2:
                case cj3:
                case cj4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        final_value = (float)temp_var1 / 16;
                        break;
                case hum1:
                case hum2:
                case hum3:
                case hum4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        final_value = 100 * ((float)temp_var1 / 65536);
                        break;
                case tn1:
                case tn2:
                case tn3:
                case tn4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        final_value = -45 + 175 * ((float)temp_var1 / 65536);
                        break;
                case light1:
                case light2:
                case light3:
                case light4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        light_scale_factor = ((uint16_t)raw_data[3] << 8) + raw_data[2];
                        final_value = temp_var1 * 1.8432 / light_scale_factor;
                        break;
                case mois1:
                case mois2:
                case mois3:
                case mois4:
                        temp_var1 = ((uint16_t)raw_data[1] << 8) + raw_data[0];
                        final_value = (float)temp_var1;
                        break;
        }
        return final_value;
}

