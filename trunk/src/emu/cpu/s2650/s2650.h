#pragma once

#ifndef __S2650_H__
#define __S2650_H__


enum
{
	S2650_PC=1, S2650_PS, S2650_R0, S2650_R1, S2650_R2, S2650_R3,
	S2650_R1A, S2650_R2A, S2650_R3A,
	S2650_HALT, S2650_SI, S2650_FO
};

/* fake I/O space ports */
enum
{
	S2650_EXT_PORT      = 0x00ff,   /* M/~IO=0 D/~C=x E/~NE=1 */
	S2650_CTRL_PORT     = 0x0100,   /* M/~IO=0 D/~C=0 E/~NE=0 */
	S2650_DATA_PORT     = 0x0101,   /* M/~IO=0 D/~C=1 E/~NE=0 */
	S2650_SENSE_PORT    = 0x0102,   /* Fake Sense Line */
	S2650_FO_PORT       = 0x0103    /* Fake FO Line */
};

DECLARE_LEGACY_CPU_DEVICE(S2650, s2650);

extern CPU_DISASSEMBLE( s2650 );

#endif /* __S2650_H__ */
