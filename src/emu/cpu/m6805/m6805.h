/*** m6805: Portable 6805 emulator ******************************************/

#pragma once

#ifndef __M6805_H__
#define __M6805_H__


enum { M6805_PC=1, M6805_S, M6805_CC, M6805_A, M6805_X, M6805_IRQ_STATE };

#define M6805_IRQ_LINE		0

extern CPU_GET_INFO( m6805 );
#define CPU_M6805 CPU_GET_INFO_NAME( m6805 )

/****************************************************************************
 * 68705 section
 ****************************************************************************/
#define M68705_A					M6805_A
#define M68705_PC					M6805_PC
#define M68705_S					M6805_S
#define M68705_X					M6805_X
#define M68705_CC					M6805_CC
#define M68705_IRQ_STATE			M6805_IRQ_STATE

#define M68705_INT_MASK				0x03
#define M68705_IRQ_LINE				M6805_IRQ_LINE
#define M68705_INT_TIMER			0x01

extern CPU_GET_INFO( m68705 );
#define CPU_M68705 CPU_GET_INFO_NAME( m68705 )

/****************************************************************************
 * HD63705 section
 ****************************************************************************/
#define HD63705_A					M6805_A
#define HD63705_PC					M6805_PC
#define HD63705_S					M6805_S
#define HD63705_X					M6805_X
#define HD63705_CC					M6805_CC
#define HD63705_NMI_STATE			M6805_IRQ_STATE
#define HD63705_IRQ1_STATE			M6805_IRQ_STATE+1
#define HD63705_IRQ2_STATE			M6805_IRQ_STATE+2
#define HD63705_ADCONV_STATE		M6805_IRQ_STATE+3

#define HD63705_INT_MASK			0x1ff

#define HD63705_INT_IRQ1			0x00
#define HD63705_INT_IRQ2			0x01
#define	HD63705_INT_TIMER1			0x02
#define	HD63705_INT_TIMER2			0x03
#define	HD63705_INT_TIMER3			0x04
#define	HD63705_INT_PCI				0x05
#define	HD63705_INT_SCI				0x06
#define	HD63705_INT_ADCONV			0x07
#define HD63705_INT_NMI				0x08

extern CPU_GET_INFO( hd63705 );
#define CPU_HD63705 CPU_GET_INFO_NAME( hd63705 )

CPU_DISASSEMBLE( m6805 );

#endif /* __M6805_H__ */
