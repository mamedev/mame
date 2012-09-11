#ifndef __I8085_H__
#define __I8085_H__


/***************************************************************************
    CONSTANTS
***************************************************************************/

enum
{
	I8085_PC, I8085_SP, I8085_AF, I8085_BC, I8085_DE, I8085_HL,
	I8085_A, I8085_B, I8085_C, I8085_D, I8085_E, I8085_F, I8085_H, I8085_L,
	I8085_STATUS, I8085_SOD, I8085_SID, I8085_INTE,
	I8085_HALT, I8085_IM,

	I8085_GENPC = STATE_GENPC,
	I8085_GENSP = STATE_GENSP,
	I8085_GENPCBASE = STATE_GENPCBASE
};

#define I8085_INTR_LINE     0
#define I8085_RST55_LINE	1
#define I8085_RST65_LINE	2
#define I8085_RST75_LINE	3

#define I8085_STATUS_INTA	0x01
#define I8085_STATUS_WO		0x02
#define I8085_STATUS_STACK	0x04
#define I8085_STATUS_HLTA	0x08
#define I8085_STATUS_OUT	0x10
#define I8085_STATUS_M1		0x20
#define I8085_STATUS_INP	0x40
#define I8085_STATUS_MEMR	0x80

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _i8085_config i8085_config;
struct _i8085_config
{
	devcb_write8		out_status_func;	/* STATUS changed callback */
	devcb_write_line	out_inte_func;		/* INTE changed callback */
	devcb_read_line		in_sid_func;		/* SID changed callback (8085A only) */
	devcb_write_line	out_sod_func;		/* SOD changed callback (8085A only) */
};
#define I8085_CONFIG(name) const i8085_config (name) =

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

DECLARE_LEGACY_CPU_DEVICE(I8080, i8080);
DECLARE_LEGACY_CPU_DEVICE(I8080A, i8080a);
DECLARE_LEGACY_CPU_DEVICE(I8085A, i8085);

CPU_DISASSEMBLE( i8085 );

#define i8085_set_sid(cpu, sid)		(cpu)->state().set_state_int(I8085_SID, sid)

#endif
