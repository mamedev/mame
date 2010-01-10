/***************************************************************************

 h83002.h : Public constants and function defs for the H8/3002 emulator.

****************************************************************************/

#pragma once

#ifndef __H83002_H__
#define __H83002_H__


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

// external input lines
enum
{
	H8_IRQ0 = 0,
	H8_IRQ1,
	H8_IRQ2,
	H8_IRQ3,
	H8_IRQ4,
	H8_IRQ5,
	H8_IRQ6,	// IRQs 6+ only available on 8-bit H8/3xx
	H8_IRQ7,
	H8_NMI,

	H8_METRO_TIMER_HACK,	// as described.  this needs to be fixed.

	H8_SCI_0_RX,	// incoming character on SCI 0
	H8_SCI_1_RX,	// incoming character on SCI 1
};

// I/O ports
enum
{
	// digital I/O ports
	// ports 4-B are valid on 16-bit H8/3xx, ports 1-9 on 8-bit H8/3xx
	H8_PORT_1 = 0,	// 0
	H8_PORT_2,	// 1
	H8_PORT_3,	// 2
	H8_PORT_4,	// 3
	H8_PORT_5,	// 4
	H8_PORT_6,	// 5
	H8_PORT_7,	// 6
	H8_PORT_8,	// 7
	H8_PORT_9,	// 8
	H8_PORT_A,	// 9
	H8_PORT_B,	// A

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
	H8_SERIAL_0 = 0x20,
	H8_SERIAL_1,
};

CPU_GET_INFO( h8_3002 );
CPU_GET_INFO( h8_3007 );
CPU_GET_INFO( h8_3044 );
CPU_GET_INFO( h8_3334 );

#define CPU_H83002 CPU_GET_INFO_NAME( h8_3002 )
#define CPU_H83007 CPU_GET_INFO_NAME( h8_3007 )
#define CPU_H83044 CPU_GET_INFO_NAME( h8_3044 )
#define CPU_H83334 CPU_GET_INFO_NAME( h8_3334 )


#endif /* __H83002_H__ */
