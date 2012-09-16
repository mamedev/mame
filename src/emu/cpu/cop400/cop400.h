/***************************************************************************

    cop400.h

    National Semiconductor COPS Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __COP400__
#define __COP400__

/***************************************************************************
    CONSTANTS
***************************************************************************/

/* register access indexes */
enum
{
	COP400_PC,
    COP400_SA,
    COP400_SB,
    COP400_SC,
	COP400_N,
	COP400_A,
	COP400_B,
	COP400_C,
	COP400_G,
	COP400_H,
	COP400_Q,
	COP400_R,
	COP400_EN,
    COP400_SIO,
    COP400_SKL,
    COP400_T,
	COP400_GENPC = STATE_GENPC,
	COP400_GENPCBASE = STATE_GENPCBASE,
	COP400_GENSP = STATE_GENSP
};

/* special I/O space ports */
enum
{
	COP400_PORT_L = 0x100,
	COP400_PORT_G,
	COP400_PORT_D,
	COP400_PORT_H,
	COP400_PORT_R,
	COP400_PORT_IN,
	COP400_PORT_SK,
	COP400_PORT_SIO,
	COP400_PORT_CKO
};

/* input lines */
enum
{
	/* COP420 */
	COP400_IN0 = 0,
	COP400_IN1,
	COP400_IN2,
	COP400_IN3,

	/* COP404 */
	COP400_MB,
	COP400_DUAL,
	COP400_SEL10,
	COP400_SEL20
};

/* CKI bonding options */
enum cop400_cki_bond {
	COP400_CKI_DIVISOR_4 = 4,
	COP400_CKI_DIVISOR_8 = 8,
	COP400_CKI_DIVISOR_16 = 16,
	COP400_CKI_DIVISOR_32 = 32
};

/* CKO bonding options */
enum cop400_cko_bond {
	COP400_CKO_OSCILLATOR_OUTPUT = 0,
	COP400_CKO_RAM_POWER_SUPPLY,
	COP400_CKO_HALT_IO_PORT,
	COP400_CKO_SYNC_INPUT,
	COP400_CKO_GENERAL_PURPOSE_INPUT
};

/* microbus bonding options */
enum cop400_microbus {
	COP400_MICROBUS_DISABLED = 0,
	COP400_MICROBUS_ENABLED
};

/* interface */
struct cop400_interface
{
	cop400_cki_bond		cki;			/* CKI bonding option */
	cop400_cko_bond		cko;			/* CKO bonding option */
	cop400_microbus		microbus;		/* microbus option */
};
#define COP400_INTERFACE(name) const cop400_interface (name) =

/***************************************************************************
    MACROS
***************************************************************************/

#define CPU_COP401 CPU_GET_INFO_NAME( cop401 )
#define CPU_COP410 CPU_GET_INFO_NAME( cop410 )
#define CPU_COP411 CPU_GET_INFO_NAME( cop411 )

#define CPU_COP402 CPU_GET_INFO_NAME( cop402 )
#define CPU_COP420 CPU_GET_INFO_NAME( cop420 )
#define CPU_COP421 CPU_GET_INFO_NAME( cop421 )
#define CPU_COP422 CPU_GET_INFO_NAME( cop422 )

#define CPU_COP404 CPU_GET_INFO_NAME( cop404 )
#define CPU_COP424 CPU_GET_INFO_NAME( cop424 )
#define CPU_COP425 CPU_GET_INFO_NAME( cop425 )
#define CPU_COP426 CPU_GET_INFO_NAME( cop426 )
#define CPU_COP444 CPU_GET_INFO_NAME( cop444 )
#define CPU_COP445 CPU_GET_INFO_NAME( cop445 )

/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/* COP410 family */
DECLARE_LEGACY_CPU_DEVICE(COP401, cop401);
DECLARE_LEGACY_CPU_DEVICE(COP410, cop410);
DECLARE_LEGACY_CPU_DEVICE(COP411, cop411);

/* COP420 family */
DECLARE_LEGACY_CPU_DEVICE(COP402, cop402);
DECLARE_LEGACY_CPU_DEVICE(COP420, cop420);
DECLARE_LEGACY_CPU_DEVICE(COP421, cop421);
DECLARE_LEGACY_CPU_DEVICE(COP422, cop422);

/* COP444 family */
DECLARE_LEGACY_CPU_DEVICE(COP404, cop404);
DECLARE_LEGACY_CPU_DEVICE(COP424, cop424);
DECLARE_LEGACY_CPU_DEVICE(COP425, cop425);
DECLARE_LEGACY_CPU_DEVICE(COP426, cop426);
DECLARE_LEGACY_CPU_DEVICE(COP444, cop444);
DECLARE_LEGACY_CPU_DEVICE(COP445, cop445);

/* disassemblers */
extern CPU_DISASSEMBLE( cop410 );
extern CPU_DISASSEMBLE( cop420 );
extern CPU_DISASSEMBLE( cop444 );

#endif  /* __COP400__ */
