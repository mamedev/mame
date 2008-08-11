/***************************************************************************

    cop400.h

    National Semiconductor COPS Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#pragma once

#ifndef __COP400__
#define __COP400__

#define COP400_PORT_L	0x100
#define COP400_PORT_G	0x101
#define COP400_PORT_D	0x102
#define	COP400_PORT_IN	0x103
#define	COP400_PORT_SK	0x104
#define	COP400_PORT_SIO	0x105
#define	COP400_PORT_CKO	0x106

enum {
	COP400_PC=1,
	COP400_A,
	COP400_B,
	COP400_C,
	COP400_G,
	COP400_EN,
	COP400_Q,
    COP400_SA,
    COP400_SB,
    COP400_SC,
    COP400_SIO,
    COP400_SKL
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
extern void cop401_get_info(UINT32 state, cpuinfo *info);
extern void cop410_get_info(UINT32 state, cpuinfo *info);
extern void cop411_get_info(UINT32 state, cpuinfo *info);

/* COP 42x */
extern void cop402_get_info(UINT32 state, cpuinfo *info);
extern void cop420_get_info(UINT32 state, cpuinfo *info);
extern void cop421_get_info(UINT32 state, cpuinfo *info);
extern void cop422_get_info(UINT32 state, cpuinfo *info);

/* COP 44x */
extern void cop404_get_info(UINT32 state, cpuinfo *info);
extern void cop424_get_info(UINT32 state, cpuinfo *info);
extern void cop425_get_info(UINT32 state, cpuinfo *info);
extern void cop426_get_info(UINT32 state, cpuinfo *info);
extern void cop444_get_info(UINT32 state, cpuinfo *info);
extern void cop445_get_info(UINT32 state, cpuinfo *info);

offs_t cop410_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t cop420_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);

#endif  /* __COP400__ */
