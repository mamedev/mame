#ifndef __SCMP_H__
#define __SCMP_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	SCMP_PC, SCMP_P1, SCMP_P2, SCMP_P3, SCMP_AC, SCMP_ER, SCMP_SR,
	SCMP_GENPC = REG_GENPC,
	SCMP_GENSP = REG_GENSP,
	SCMP_GENPCBASE = REG_GENPCBASE
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
typedef struct _scmp_config scmp_config;
struct _scmp_config
{
	devcb_write8		flag_out_func;
	devcb_write_line	sout_func;
	devcb_read_line		sin_func;
	devcb_read_line		sensea_func;
	devcb_read_line		senseb_func;
	devcb_write_line	halt_func;
};
#define SCMP_CONFIG(name) const scmp_config (name) =

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

CPU_GET_INFO( scmp );
#define CPU_SCMP CPU_GET_INFO_NAME( scmp )

CPU_DISASSEMBLE( scmp );

CPU_GET_INFO( ins8060 );
#define CPU_INS8060 CPU_GET_INFO_NAME( ins8060 )

#endif
