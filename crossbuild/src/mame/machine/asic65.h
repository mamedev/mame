/*************************************
 *
 *  Implementation of ASIC65
 *
 *************************************/

#define ASIC65_STANDARD		0
#define ASIC65_STEELTAL		1
#define ASIC65_GUARDIANS	2
#define ASIC65_ROMBASED		3

void asic65_config(int asictype);
void asic65_reset(int state);
WRITE16_HANDLER( asic65_data_w );
READ16_HANDLER( asic65_r );
READ16_HANDLER( asic65_io_r );

MACHINE_DRIVER_EXTERN( asic65 );
