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
		}

	} else if (pl.cmd == write_cmd) {
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
                NRF_write_payload(&tr, fd, tx_pl, 2);
        }	
	
        NRF_clear_prim_rx(&tr, fd);
        config.interface.ce_set(1);
        printf("waiting for data to send\n");
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
		printf("data sent\n");
		return RADIO_NO_ERROR;
	} 				
}

int RADIO_rx(int fd, radio_config_t config, radio_pl_t *pl) {
        int i, pw;
        unsigned int t0, t_loop;
        unsigned int rx_timeout_ms = 1000;
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
	printf("waiting for rx data...\n");
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
	}	
}

