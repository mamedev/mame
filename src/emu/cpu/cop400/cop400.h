/***************************************************************************

    cop400.h

    National Semiconductor COP400 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

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

extern void cop401_get_info(UINT32 state, cpuinfo *info);
extern void cop410_get_info(UINT32 state, cpuinfo *info);
extern void cop411_get_info(UINT32 state, cpuinfo *info);

extern void cop402_get_info(UINT32 state, cpuinfo *info);
extern void cop420_get_info(UINT32 state, cpuinfo *info);
extern void cop421_get_info(UINT32 state, cpuinfo *info);
extern void cop422_get_info(UINT32 state, cpuinfo *info);

extern void cop404_get_info(UINT32 state, cpuinfo *info);
extern void cop444_get_info(UINT32 state, cpuinfo *info);
extern void cop445_get_info(UINT32 state, cpuinfo *info);

#ifdef ENABLE_DEBUGGER
offs_t cop410_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t cop420_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif /* ENABLE_DEBUGGER */

#endif  /* __COP400__ */
