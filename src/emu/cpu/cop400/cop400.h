/***************************************************************************

    cop400.h

    National Semiconductor COPS Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __COP400__
#define __COP400__

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

enum
{
	COP400_PC = 1,
	COP400_A,
	COP400_B,
	COP400_C,
	COP400_G,
	COP400_H,
	COP400_R,
	COP400_EN,
	COP400_Q,
    COP400_SA,
    COP400_SB,
    COP400_SC,
    COP400_SIO,
    COP400_SKL,
    COP400_T
};

typedef enum _cop400_cki_bond cop400_cki_bond;
enum _cop400_cki_bond {
	COP400_CKI_DIVISOR_4 = 4,
	COP400_CKI_DIVISOR_8 = 8,
	COP400_CKI_DIVISOR_16 = 16,
	COP400_CKI_DIVISOR_32 = 32
};

typedef enum _cop400_cko_bond cop400_cko_bond;
enum _cop400_cko_bond {
	COP400_CKO_OSCILLATOR_OUTPUT = 0,
	COP400_CKO_RAM_POWER_SUPPLY,
	COP400_CKO_HALT_IO_PORT,
	COP400_CKO_SYNC_INPUT,
	COP400_CKO_GENERAL_PURPOSE_INPUT
};

typedef enum _cop400_microbus cop400_microbus;
enum _cop400_microbus {
	COP400_MICROBUS_DISABLED = 0,
	COP400_MICROBUS_ENABLED
};

/* interface */
typedef struct _cop400_interface cop400_interface;
struct _cop400_interface
{
	cop400_cki_bond		cki;			/* CKI bonding option */
	cop400_cko_bond		cko;			/* CKO bonding option */
	cop400_microbus		microbus;		/* microbus option */
};
#define COP400_INTERFACE(name) const cop400_interface (name) =

/*

    Prefix      Temperature Range

    COP4xx      0C to 70C
    COP3xx      -40C to +85C
    COP2xx      -55C to +125C

*/

/* COP 41x */
extern CPU_GET_INFO( cop401 );
extern CPU_GET_INFO( cop410 );
extern CPU_GET_INFO( cop411 );

/* COP 42x */
extern CPU_GET_INFO( cop402 );
extern CPU_GET_INFO( cop420 );
extern CPU_GET_INFO( cop421 );
extern CPU_GET_INFO( cop422 );

/* COP 44x */
extern CPU_GET_INFO( cop404 );
extern CPU_GET_INFO( cop424 );
extern CPU_GET_INFO( cop425 );
extern CPU_GET_INFO( cop426 );
extern CPU_GET_INFO( cop444 );
extern CPU_GET_INFO( cop445 );

CPU_DISASSEMBLE( cop410 );
CPU_DISASSEMBLE( cop420 );
CPU_DISASSEMBLE( cop444 );

#endif  /* __COP400__ */
