/*************************************
 *
 *  Implementation of ASIC65
 *
 *************************************/

#define ASIC65_STANDARD     0
#define ASIC65_STEELTAL     1
#define ASIC65_GUARDIANS    2
#define ASIC65_ROMBASED     3

void asic65_config(running_machine &machine, int asictype);
void asic65_reset(running_machine &machine, int state);
DECLARE_WRITE16_HANDLER( asic65_data_w );
DECLARE_READ16_HANDLER( asic65_r );
DECLARE_READ16_HANDLER( asic65_io_r );

MACHINE_CONFIG_EXTERN( asic65 );
