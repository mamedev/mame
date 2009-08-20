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
	/* O-output PLA configuration */
	struct {
		UINT8	value;
		UINT16	output;
	}	o_pla[20];
	read8_device_func	read_k;
	write16_device_func	write_o;		/* tms1270 has 10 O-outputs */
	write16_device_func	write_r;
};


/* 9-bit family */
#define CPU_TMS0980 CPU_GET_INFO_NAME( tms0980 )

extern CPU_GET_INFO( tms0980 );
extern CPU_DISASSEMBLE( tms0980 );

/* 8-bit family */
#define CPU_TMS1000 CPU_GET_INFO_NAME( tms1000 )
#define CPU_TMS1070 CPU_GET_INFO_NAME( tms1070 )
#define CPU_TMS1100 CPU_GET_INFO_NAME( tms1100 )
#define CPU_TMS1200 CPU_GET_INFO_NAME( tms1200 )
#define CPU_TMS1270 CPU_GET_INFO_NAME( tms1270 )
#define CPU_TMS1300 CPU_GET_INFO_NAME( tms1300 )

extern CPU_GET_INFO( tms1000 );
extern CPU_GET_INFO( tms1070 );
extern CPU_GET_INFO( tms1100 );
extern CPU_GET_INFO( tms1200 );
extern CPU_GET_INFO( tms1270 );
extern CPU_GET_INFO( tms1300 );

extern CPU_DISASSEMBLE( tms1000 );
extern CPU_DISASSEMBLE( tms1100 );

#endif /* _TMS0980_H_ */

