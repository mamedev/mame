#ifndef _TMS0980_H_
#define _TMS0980_H_

#include "cpuintrf.h"

/* Registers */
enum {
	TMS0980_PC=1, TMS0980_SR, TMS0980_PA, TMS0980_PB,
	TMS0980_A, TMS0980_X, TMS0980_Y, TMS0980_STATUS
};


typedef struct _tms0980_config tms0980_config;
struct _tms0980_config {
	read8_device_func	read_k;
	write8_device_func	write_o;
	write16_device_func	write_r;
};


#define CPU_TMS0980 CPU_GET_INFO_NAME( tms0980 )

extern CPU_GET_INFO( tms0980 );
extern CPU_DISASSEMBLE( tms0980 );

#endif /* _TMS0980_H_ */

