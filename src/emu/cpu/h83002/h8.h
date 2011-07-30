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

////////////////////////////
// INTERNAL I/O REGISTERS //
////////////////////////////

#define H8S_IO_OFFS(addr) ((addr) - 0xFFFE40) /* convert address to offset */
#define H8S_IO_ADDR(offs) ((offs) + 0xFFFE40) /* convert offset to address */

#define H8S_IO(xxxx) ((xxxx) - 0xFE40)

// port
#define H8S_IO_P1DDR  H8S_IO(0xFEB0)
#define H8S_IO_P2DDR  H8S_IO(0xFEB1)
#define H8S_IO_P3DDR  H8S_IO(0xFEB2)
#define H8S_IO_P5DDR  H8S_IO(0xFEB4)
#define H8S_IO_PADDR  H8S_IO(0xFEB9)
#define H8S_IO_PBDDR  H8S_IO(0xFEBA)
#define H8S_IO_PCDDR  H8S_IO(0xFEBB)
#define H8S_IO_PDDDR  H8S_IO(0xFEBC)
#define H8S_IO_PEDDR  H8S_IO(0xFEBD)
#define H8S_IO_PFDDR  H8S_IO(0xFEBE)
#define H8S_IO_PGDDR  H8S_IO(0xFEBF)
// interrupt controller
#define H8S_IO_ICRA   H8S_IO(0xFEC0)
#define H8S_IO_ICRB   H8S_IO(0xFEC1)
#define H8S_IO_ICRC   H8S_IO(0xFEC2)
// dtc (data transfer controller)
#define H8S_IO_DTCEA  H8S_IO(0xFF30)
#define H8S_IO_DTCEB  H8S_IO(0xFF31)
#define H8S_IO_DTCEC  H8S_IO(0xFF32)
#define H8S_IO_DTCED  H8S_IO(0xFF33)
#define H8S_IO_DTCEE  H8S_IO(0xFF34)
#define H8S_IO_DTCEF  H8S_IO(0xFF35)
// port
#define H8S_IO_PORT1  H8S_IO(0xFF50)
#define H8S_IO_PORT2  H8S_IO(0xFF51)
#define H8S_IO_PORT3  H8S_IO(0xFF52)
#define H8S_IO_PORT4  H8S_IO(0xFF53)
#define H8S_IO_PORT5  H8S_IO(0xFF54)
#define H8S_IO_PORTA  H8S_IO(0xFF59)
#define H8S_IO_PORTB  H8S_IO(0xFF5A)
#define H8S_IO_PORTC  H8S_IO(0xFF5B)
#define H8S_IO_PORTD  H8S_IO(0xFF5C)
#define H8S_IO_PORTE  H8S_IO(0xFF5D)
#define H8S_IO_PORTF  H8S_IO(0xFF5E)
#define H8S_IO_PORTG  H8S_IO(0xFF5F)
#define H8S_IO_P1DR   H8S_IO(0xFF60)
#define H8S_IO_P2DR   H8S_IO(0xFF61)
#define H8S_IO_P3DR   H8S_IO(0xFF62)
#define H8S_IO_P4DR   H8S_IO(0xFF63)
#define H8S_IO_P5DR   H8S_IO(0xFF64)
#define H8S_IO_PADR   H8S_IO(0xFF69)
#define H8S_IO_PBDR   H8S_IO(0xFF6A)
#define H8S_IO_PCDR   H8S_IO(0xFF6B)
#define H8S_IO_PDDR   H8S_IO(0xFF6C)
#define H8S_IO_PEDR   H8S_IO(0xFF6D)
#define H8S_IO_PFDR   H8S_IO(0xFF6E)
#define H8S_IO_PGDR   H8S_IO(0xFF6F)
// SCI0
#define H8S_IO_SMR0   H8S_IO(0xFF78)
#define H8S_IO_BRR0   H8S_IO(0xFF79)
#define H8S_IO_SCR0   H8S_IO(0xFF7A)
#define H8S_IO_TDR0   H8S_IO(0xFF7B)
#define H8S_IO_SSR0   H8S_IO(0xFF7C)
#define H8S_IO_RDR0   H8S_IO(0xFF7D)
#define H8S_IO_SCMR0  H8S_IO(0xFF7E)
// SCI1
#define H8S_IO_SMR1   H8S_IO(0xFF80)
#define H8S_IO_BRR1   H8S_IO(0xFF81)
#define H8S_IO_SCR1   H8S_IO(0xFF82)
#define H8S_IO_TDR1   H8S_IO(0xFF83)
#define H8S_IO_SSR1   H8S_IO(0xFF84)
#define H8S_IO_RDR1   H8S_IO(0xFF85)
#define H8S_IO_SCMR1  H8S_IO(0xFF86)
// SCI2
#define H8S_IO_SMR2   H8S_IO(0xFF88)
#define H8S_IO_BRR2   H8S_IO(0xFF89)
#define H8S_IO_SCR2   H8S_IO(0xFF8A)
#define H8S_IO_TDR2   H8S_IO(0xFF8B)
#define H8S_IO_SSR2   H8S_IO(0xFF8C)
#define H8S_IO_RDR2   H8S_IO(0xFF8D)
#define H8S_IO_SCMR2  H8S_IO(0xFF8E)
// 8-bit timer (channel 0 & 1)
#define H8S_IO_TCR0   H8S_IO(0xFFB0)
#define H8S_IO_TCR1   H8S_IO(0xFFB1)
#define H8S_IO_TCSR0  H8S_IO(0xFFB2)
#define H8S_IO_TCSR1  H8S_IO(0xFFB3)
#define H8S_IO_TCORA0 H8S_IO(0xFFB4)
#define H8S_IO_TCORA1 H8S_IO(0xFFB5)
#define H8S_IO_TCORB0 H8S_IO(0xFFB6)
#define H8S_IO_TCORB1 H8S_IO(0xFFB7)
#define H8S_IO_TCNT0  H8S_IO(0xFFB8)
#define H8S_IO_TCNT1  H8S_IO(0xFFB9)
// TPU
#define H8S_IO_TSTR   H8S_IO(0xFFC0)
#define H8S_IO_TSYR   H8S_IO(0xFFC1)
// TPU0
#define H8S_IO_TTCR0  H8S_IO(0xFFD0)
#define H8S_IO_TMDR0  H8S_IO(0xFFD1)
#define H8S_IO_TIOR0H H8S_IO(0xFFD2)
#define H8S_IO_TIOR0L H8S_IO(0xFFD3)
#define H8S_IO_TIER0  H8S_IO(0xFFD4)
#define H8S_IO_TSR0   H8S_IO(0xFFD5)
#define H8S_IO_TTCNT0 H8S_IO(0xFFD6)
#define H8S_IO_TGR0A  H8S_IO(0xFFD8)
#define H8S_IO_TGR0B  H8S_IO(0xFFDA)
#define H8S_IO_TGR0C  H8S_IO(0xFFDC)
#define H8S_IO_TGR0D  H8S_IO(0xFFDE)
// TPU1
#define H8S_IO_TTCR1  H8S_IO(0xFFE0)
#define H8S_IO_TMDR1  H8S_IO(0xFFE1)
#define H8S_IO_TIOR1  H8S_IO(0xFFE2)
#define H8S_IO_TIER1  H8S_IO(0xFFE4)
#define H8S_IO_TSR1   H8S_IO(0xFFE5)
#define H8S_IO_TTCNT1 H8S_IO(0xFFE6)
#define H8S_IO_TGR1A  H8S_IO(0xFFE8)
#define H8S_IO_TGR1B  H8S_IO(0xFFEA)
// TPU5
#define H8S_IO_TTCR5  H8S_IO(0xFEA0)
#define H8S_IO_TMDR5  H8S_IO(0xFEA1)
#define H8S_IO_TIOR5  H8S_IO(0xFEA2)
#define H8S_IO_TIER5  H8S_IO(0xFEA4)
#define H8S_IO_TSR5   H8S_IO(0xFEA5)
#define H8S_IO_TTCNT5 H8S_IO(0xFEA6)
#define H8S_IO_TGR5A  H8S_IO(0xFEA8)
#define H8S_IO_TGR5B  H8S_IO(0xFEAA)
// DMA
#define H8S_IO_MAR1AH  H8S_IO(0xFEF0)
#define H8S_IO_MAR1AL  H8S_IO(0xFEF2)
#define H8S_IO_ETCR1A  H8S_IO(0xFEF6)
#define H8S_IO_MAR1BH  H8S_IO(0xFEF8)
#define H8S_IO_MAR1BL  H8S_IO(0xFEFA)
#define H8S_IO_ETCR1B  H8S_IO(0xFEFE)
#define H8S_IO_DMACR0A H8S_IO(0xFF02)
#define H8S_IO_DMACR0B H8S_IO(0xFF03)
#define H8S_IO_DMACR1A H8S_IO(0xFF04)
#define H8S_IO_DMACR1B H8S_IO(0xFF05)
#define H8S_IO_DMABCRH H8S_IO(0xFF06)
#define H8S_IO_DMABCRL H8S_IO(0xFF07)

///////////
// PORTS //
///////////

#define H8S_P1_TIOCB2 0x80
#define H8S_P1_TIOCA2 0x40
#define H8S_P1_TIOCB1 0x20
#define H8S_P1_TIOCA1 0x10
#define H8S_P1_TIOCD0 0x08
#define H8S_P1_TIOCC0 0x04
#define H8S_P1_TIOCB0 0x02
#define H8S_P1_TIOCA0 0x01

#define H8S_P3_SCK1 0x20
#define H8S_P3_SCK0 0x10
#define H8S_P3_RXD1 0x08
#define H8S_P3_RXD0 0x04
#define H8S_P3_TXD1 0x02
#define H8S_P3_TXD0 0x01

#define H8S_P5_SCK2 0x04
#define H8S_P5_RXD2 0x02
#define H8S_P5_TXD2 0x01

#define H8S_PF_PF2 0x04
#define H8S_PF_PF1 0x02
#define H8S_PF_PF0 0x01

///////////////
// SCI 0/1/2 //
///////////////

#define H8S_SSR_TDRE 0x80 /* transmit data register empty */
#define H8S_SSR_RDRF 0x40 /* receive data register full */
#define H8S_SSR_ORER 0x20 /* overrun error */
#define H8S_SSR_FER  0x10 /* framing error */
#define H8S_SSR_PER  0x08 /* parity error */
#define H8S_SSR_TEND 0x04 /* transmit end */
#define H8S_SSR_MPB  0x02 /* multiprocessor bit */
#define H8S_SSR_MPBT 0x01 /* multiprocessor bit transfer */

#define H8S_SCR_TIE  0x80 /* transmit interrupt enable */
#define H8S_SCR_RIE  0x40 /* receive interrupt enable */
#define H8S_SCR_TE   0x20 /* transmit enable */
#define H8S_SCR_RE   0x10 /* receive enable */
#define H8S_SCR_MPIE 0x08 /* multiprocessor interrupt enable */
#define H8S_SCR_TEIE 0x04 /* transmit end interrupt enable */
#define H8S_SCR_CKE1 0x02 /* clock enable 1 */
#define H8S_SCR_CKE0 0x01 /* clock enable 0 */

////////////////
// INTERRUPTS //
////////////////

#define H8S_INT_NMI     0x07
#define H8S_INT_IRQ0    0x10
#define H8S_INT_IRQ1    0x11
#define H8S_INT_IRQ2    0x12
#define H8S_INT_IRQ3    0x13
#define H8S_INT_IRQ4    0x14
#define H8S_INT_IRQ5    0x15
#define H8S_INT_IRQ6    0x16
#define H8S_INT_IRQ7    0x17
#define H8S_INT_SWDTEND 0x18
#define H8S_INT_WOVI    0x19
#define H8S_INT_ADI     0x1C
#define H8S_INT_TGI0A   0x20
#define H8S_INT_TGI0B   0x21
#define H8S_INT_TGI0C   0x22
#define H8S_INT_TGI0D   0x23
#define H8S_INT_TGI0V   0x24
#define H8S_INT_TGI1A   0x28
#define H8S_INT_TGI1B   0x29
#define H8S_INT_TGI1V   0x2A
#define H8S_INT_TGI1U   0x2B
#define H8S_INT_TGI2A   0x2C
#define H8S_INT_TGI2B   0x2D
#define H8S_INT_TGI2V   0x2E
#define H8S_INT_TGI2U   0x2F
#define H8S_INT_CMIA0   0x40
#define H8S_INT_CMIB0   0x41
#define H8S_INT_OVI0    0x42
#define H8S_INT_CMIA1   0x44
#define H8S_INT_CMIB1   0x45
#define H8S_INT_OVI1    0x46
#define H8S_INT_ERI0    0x50
#define H8S_INT_RXI0    0x51
#define H8S_INT_TXI0    0x52
#define H8S_INT_TEI0    0x53
#define H8S_INT_ERI1    0x54
#define H8S_INT_RXI1    0x55
#define H8S_INT_TXI1    0x56
#define H8S_INT_TEI1    0x57
#define H8S_INT_ERI2    0x58
#define H8S_INT_RXI2    0x59
#define H8S_INT_TXI2    0x5A
#define H8S_INT_TEI2    0x5B

DECLARE_LEGACY_CPU_DEVICE(H83002, h8_3002);
DECLARE_LEGACY_CPU_DEVICE(H83007, h8_3007);
DECLARE_LEGACY_CPU_DEVICE(H83044, h8_3044);
DECLARE_LEGACY_CPU_DEVICE(H83334, h8_3334);

DECLARE_LEGACY_CPU_DEVICE(H8S2241, h8s_2241);
DECLARE_LEGACY_CPU_DEVICE(H8S2246, h8s_2246);
DECLARE_LEGACY_CPU_DEVICE(H8S2323, h8s_2323);

#endif /* __H83002_H__ */
