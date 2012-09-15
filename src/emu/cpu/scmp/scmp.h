#ifndef __SCMP_H__
#define __SCMP_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	SCMP_PC, SCMP_P1, SCMP_P2, SCMP_P3, SCMP_AC, SCMP_ER, SCMP_SR,
	SCMP_GENPC = STATE_GENPC,
	SCMP_GENSP = STATE_GENSP,
	SCMP_GENPCBASE = STATE_GENPCBASE
};

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/
struct scmp_config
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

DECLARE_LEGACY_CPU_DEVICE(SCMP, scmp);

CPU_DISASSEMBLE( scmp );

DECLARE_LEGACY_CPU_DEVICE(INS8060, ins8060);

#endif
