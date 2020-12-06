#include "sys_control.h"
#include "argp.h"
#include "stdlib.h"

static char doc[] = 
	"A program to configure Dylan's hydropnics system. It's lit.\n";

/*Description of the arguments accepted*/
static char args_doc[] = "";

/* the options */
static struct argp_option options[] = {
	{"ltstart", 'a', "TIME1", 0, "time when lights turn on"}, 
	{"ltstop", 'b', "TIME2", 0, "time when lights turn off"},
	{"ltlocal", 'c', 0, 0, "time of day"},
	{"lintensity", 'd', "INTENSITY", 0, "light intensity (0 - 99)"},
	{"ptstart", 'e', "TIME4", 0, "time when lights turn on"}, 
	{"ptstop", 'f', "TIME5", 0, "time when lights turn off"},
	{"ptpump", 'g', "TIME6", 0, "period of time that the pump floods the tray"},
	{"ptcycle", 'h', "TIME7", 0, "time between flooding events"},
	{"ptlocal", 'i', 0, 0, "time of day"},
	{0}	
};

struct arguments {
	//char *args[2];
	uint8_t lintensity;
	struct tm ltstart, ltstop, ltlocal,  ptstart, ptstop, ptpump, ptcycle, ptlocal;
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
	struct arguments *arguments = state->input;
	int i;
	time_t t = time(NULL);
	struct tm *local_tm = localtime(&t);

	switch (key) {
		case 'a':
			arguments->ltstart.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ltstart.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ltstart.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'b':
			arguments->ltstop.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ltstop.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ltstop.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'c':
			
			arguments->ltlocal.tm_hour = local_tm->tm_hour;
			arguments->ltlocal.tm_min = local_tm->tm_min;
			arguments->ltlocal.tm_sec = local_tm->tm_sec;
			break;
		case 'd':
			arguments->lintensity = 10*(arg[0] - '0') + (arg[1] - '0');
			break;
		case 'e':
			arguments->ptstart.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ptstart.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ptstart.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'f':
			arguments->ptstop.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ptstop.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ptstop.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'g':
			arguments->ptpump.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ptpump.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ptpump.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'h':
			arguments->ptcycle.tm_hour = 10*(arg[0] - '0') + (arg[1] - '0');
			arguments->ptcycle.tm_min = 10*(arg[3] - '0') + (arg[4] - '0');
			arguments->ptcycle.tm_sec = 10*(arg[6] - '0') + (arg[7] - '0');
			break;
		case 'i':
			arguments->ptlocal.tm_hour = local_tm->tm_hour;
			arguments->ptlocal.tm_min = local_tm->tm_min;
			arguments->ptlocal.tm_sec = local_tm->tm_sec;
			break;
		case ARGP_KEY_ARG:
			if (state->arg_num >= 0)
				/*Too many arguments*/
				argp_usage(state);	
			break;
		default:
			return ARGP_ERR_UNKNOWN;
	}		
	return 0;
}

void set_default_arg_vals(struct arguments *arguments) {	
	arguments->ltstart.tm_hour = ARG_VAL_IGNORE;
	arguments->ltstart.tm_min = ARG_VAL_IGNORE;
	arguments->ltstart.tm_sec = ARG_VAL_IGNORE;	
	arguments->ltstop.tm_hour = ARG_VAL_IGNORE;
	arguments->ltstop.tm_min = ARG_VAL_IGNORE;
	arguments->ltstop.tm_sec = ARG_VAL_IGNORE;	
	arguments->ltlocal.tm_hour = ARG_VAL_IGNORE;
	arguments->ltlocal.tm_min = ARG_VAL_IGNORE;
	arguments->ltlocal.tm_sec = ARG_VAL_IGNORE;
	arguments->lintensity = ARG_VAL_IGNORE;

	arguments->ptstart.tm_hour = ARG_VAL_IGNORE;
	arguments->ptstart.tm_min = ARG_VAL_IGNORE;
	arguments->ptstart.tm_sec = ARG_VAL_IGNORE;	
	arguments->ptstop.tm_hour = ARG_VAL_IGNORE;
	arguments->ptstop.tm_min = ARG_VAL_IGNORE;
	arguments->ptstop.tm_sec = ARG_VAL_IGNORE;	
	arguments->ptpump.tm_hour = ARG_VAL_IGNORE;
	arguments->ptpump.tm_min = ARG_VAL_IGNORE;
	arguments->ptpump.tm_sec = ARG_VAL_IGNORE;	
	arguments->ptcycle.tm_hour = ARG_VAL_IGNORE;
	arguments->ptcycle.tm_min = ARG_VAL_IGNORE;
	arguments->ptcycle.tm_sec = ARG_VAL_IGNORE;	
	arguments->ptlocal.tm_hour = ARG_VAL_IGNORE;
	arguments->ptlocal.tm_min = ARG_VAL_IGNORE;
	arguments->ptlocal.tm_sec = ARG_VAL_IGNORE;
}

void nrf_ce_set(int state) {
	digitalWrite(0, state);
}

static struct argp argp = {options, parse_opt, args_doc, doc};
static const char *device = "/dev/spidev0.0";

int main(int argc, char *argv[]) {

	int i;
	int fd;
        uint8_t tx_payload[MAX_TX_MSG_LEN];
        uint8_t rx_payload[MAX_RX_MSG_LEN];
	interface_t interface;
	interface.ce_set = nrf_ce_set;
	struct arguments arguments;
	radio_pl_t rx_pl, tx_pl;

	radio_config_t config;
	config.interface = interface;
	config.rf_ch = RF_CH;

        for (i = 0; i < 5; i++) {
                config.rx_addr[i] = pipe_addr[PUMP_CTRL_ADDR_I][i];
                config.tx_addr[i] = pipe_addr[PUMP_CTRL_ADDR_I][i];
        }

        if (wiringPiSetup() == -1) {
                printf("WiringPiSetup error.\n");
                return 1;
        }

        pinMode(0, OUTPUT);

        fd = open(device, O_RDWR);
        if (fd < 0) {
                printf("can't open device\n");
        }

        struct spi_ioc_transfer mesg = {
                .tx_buf = (unsigned long)tx_payload,
                .rx_buf = (unsigned long)rx_payload,
                .len = 1,
                .speed_hz = SPI_FREQ,
                .bits_per_word = 8,
                .cs_change = 0,
                .delay_usecs = 10,
        };

	RADIO_init(fd, config); 
	set_default_arg_vals(&arguments);
	argp_parse(&argp, argc, argv, 0, 0, &arguments);
	
	printf("ltstart: %02u:%02u:%02u, ltstop: %02u:%02u:%02u, ltlocal: %02u:%02u:%02u, lintensity: %u\n", 
		arguments.ltstart.tm_hour, arguments.ltstart.tm_min, arguments.ltstart.tm_sec,
		arguments.ltstop.tm_hour, arguments.ltstop.tm_min, arguments.ltstop.tm_sec,
		arguments.ltlocal.tm_hour, arguments.ltlocal.tm_min, arguments.ltlocal.tm_sec,
		arguments.lintensity
	);

	printf("ptstart: %02u:%02u:%02u, ptstop: %02u:%02u:%02u, ptpump: %02u:%02u:%02u, plcycle: %02u:%02u:%02u, ptlocal: %02u:%02u:%02u\n", 
		arguments.ptstart.tm_hour, arguments.ptstart.tm_min, arguments.ptstart.tm_sec,
		arguments.ptstop.tm_hour, arguments.ptstop.tm_min, arguments.ptstop.tm_sec,
		arguments.ptpump.tm_hour, arguments.ptpump.tm_min, arguments.ptpump.tm_sec,
		arguments.ptcycle.tm_hour, arguments.ptcycle.tm_min, arguments.ptcycle.tm_sec,
		arguments.ptlocal.tm_hour, arguments.ptlocal.tm_min, arguments.ptlocal.tm_sec
	);

	if (arguments.ltstart.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ltstart~~~\n");
		tx_pl.p_start_tm.tm_sec = arguments.ltstart.tm_sec;
		tx_pl.p_start_tm.tm_min = arguments.ltstart.tm_min;
		tx_pl.p_start_tm.tm_hour = arguments.ltstart.tm_hour;
		tx_pl.param = t_start;
		rx_pl.param = t_start;	
		tx_pl.node = lights;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ltstop.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ltstop~~~\n");
		tx_pl.p_stop_tm.tm_sec = arguments.ltstop.tm_sec;
		tx_pl.p_stop_tm.tm_min = arguments.ltstop.tm_min;
		tx_pl.p_stop_tm.tm_hour = arguments.ltstop.tm_hour;
		tx_pl.param = t_stop;
		rx_pl.param = t_stop;
		tx_pl.node = lights;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ltlocal.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ltlocal~~~\n");
		tx_pl.p_local_tm.tm_sec = arguments.ltlocal.tm_sec;
		tx_pl.p_local_tm.tm_min = arguments.ltlocal.tm_min;
		tx_pl.p_local_tm.tm_hour = arguments.ltlocal.tm_hour;
		tx_pl.param = t_local;
		rx_pl.param = t_local;
		tx_pl.node = lights;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.lintensity != ARG_VAL_IGNORE) {
		printf("~~~updating lintensity~~~\n");
		tx_pl.intensity = arguments.lintensity;
		rx_pl.param = intensity;
		tx_pl.param = intensity;
		tx_pl.node = lights;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ptstart.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ptstart~~~\n");
		tx_pl.p_start_tm.tm_sec = arguments.ptstart.tm_sec;
		tx_pl.p_start_tm.tm_min = arguments.ptstart.tm_min;
		tx_pl.p_start_tm.tm_hour = arguments.ptstart.tm_hour;
		tx_pl.param = t_start;
		rx_pl.param = t_start;
		tx_pl.node = pump;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ptstop.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ptstop~~~\n");
		tx_pl.p_stop_tm.tm_sec = arguments.ptstop.tm_sec;
		tx_pl.p_stop_tm.tm_min = arguments.ptstop.tm_min;
		tx_pl.p_stop_tm.tm_hour = arguments.ptstop.tm_hour;
		tx_pl.param = t_stop;
		rx_pl.param = t_stop;
		tx_pl.node = pump;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ptpump.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ptpump~~~\n");
		tx_pl.p_pump_tm.tm_sec = arguments.ptpump.tm_sec;
		tx_pl.p_pump_tm.tm_min = arguments.ptpump.tm_min;
		tx_pl.p_pump_tm.tm_hour = arguments.ptpump.tm_hour;
		tx_pl.param = t_pump;
		rx_pl.param = t_pump;
		tx_pl.node = pump;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ptcycle.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ptcycle~~~\n");
		tx_pl.p_cycle_tm.tm_sec = arguments.ptcycle.tm_sec;
		tx_pl.p_cycle_tm.tm_min = arguments.ptcycle.tm_min;
		tx_pl.p_cycle_tm.tm_hour = arguments.ptcycle.tm_hour;
		tx_pl.param = t_cycle;
		rx_pl.param = t_cycle;
		tx_pl.node = pump;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	
	if (arguments.ptlocal.tm_hour != ARG_VAL_IGNORE) {
		printf("~~~updating ptlocal~~~\n");
		tx_pl.p_local_tm.tm_sec = arguments.ptlocal.tm_sec;
		tx_pl.p_local_tm.tm_min = arguments.ptlocal.tm_min;
		tx_pl.p_local_tm.tm_hour = arguments.ptlocal.tm_hour;
		tx_pl.param = t_local;
		rx_pl.param = t_local;
		tx_pl.node = pump;
		RADIO_write_and_verify_param(fd, config, rx_pl, tx_pl);				
	}	

	return 1;
}
