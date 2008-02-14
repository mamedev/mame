/***************************************************************************

 h83002.h : Public constants and function defs for the H8/3002 emulator.

****************************************************************************/

#ifndef _H83002_H_
#define _H83002_H_

#include "cpuintrf.h"

#define IFLAG  0x80
#define UIFLAG 0x40
#define HFLAG  0x20
#define UFLAG  0x10
#define NFLAG  0x08
#define ZFLAG  0x04
#define VFLAG  0x02
#define CFLAG  0x01

enum
{
	H8_E0 = 1,
	H8_E1,
	H8_E2,
	H8_E3,
	H8_E4,
	H8_E5,
	H8_E6,
	H8_E7,

	H8_PC,
	H8_CCR
};

// external interrupt lines
enum
{
	H8_IRQ0 = 0,
	H8_IRQ1,
	H8_IRQ2,
	H8_IRQ3,
	H8_IRQ4,
	H8_IRQ5
};

// I/O ports
enum
{
	// digital I/O ports
	H8_PORT4 = 0,	// 0
	H8_PORT6,	// 1
	H8_PORT7,	// 2
	H8_PORT8,	// 3
	H8_PORT9,	// 4
	H8_PORTA,	// 5
	H8_PORTB,	// 6

	// analog inputs
	H8_ADC_0_H = 0x10,
	H8_ADC_0_L,
	H8_ADC_1_H,
	H8_ADC_1_L,
	H8_ADC_2_H,
	H8_ADC_2_L,
	H8_ADC_3_H,
	H8_ADC_3_L,

	// serial ports
	H8_SERIAL_A = 0x20,
	H8_SERIAL_B
};

void h8_3002_get_info(UINT32 state, cpuinfo *info);

void h8_3002_InterruptRequest(UINT8 source);

#endif
