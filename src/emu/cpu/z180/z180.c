/*****************************************************************************
 *
 *   z180.c
 *   Portable Z180 emulator V0.3
 *
 *   Copyright Juergen Buchmueller, all rights reserved.
 *   You can contact me at juergen@mame.net or pullmoll@stop1984.com
 *
 *   - This source code is released as freeware for non-commercial purposes
 *     as part of the M.A.M.E. (Multiple Arcade Machine Emulator) project.
 *     The licensing terms of MAME apply to this piece of code for the MAME
 *     project and derviative works, as defined by the MAME license. You
 *     may opt to make modifications, improvements or derivative works under
 *     that same conditions, and the MAME project may opt to keep
 *     modifications, improvements or derivatives under their terms exclusively.
 *
 *   - Alternatively you can choose to apply the terms of the "GPL" (see
 *     below) to this - and only this - piece of code or your derivative works.
 *     Note that in no case your choice can have any impact on any other
 *     source code of the MAME project, or binary, or executable, be it closely
 *     or losely related to this piece of code.
 *
 *  -  At your choice you are also free to remove either licensing terms from
 *     this file and continue to use it under only one of the two licenses. Do this
 *     if you think that licenses are not compatible (enough) for you, or if you
 *     consider either license 'too restrictive' or 'too free'.
 *
 *  -  GPL (GNU General Public License)
 *     This program is free software; you can redistribute it and/or
 *     modify it under the terms of the GNU General Public License
 *     as published by the Free Software Foundation; either version 2
 *     of the License, or (at your option) any later version.
 *
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with this program; if not, write to the Free Software
 *     Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 *
 *****************************************************************************/

/*****************************************************************************

Z180 Info:

Known clock speeds (from ZiLOG): 6, 8, 10, 20 & 33MHz

ZiLOG Z180 codes:

  Speed: 10 = 10MHZ
         20 = 20MHz
         33 = 33MHz
Package: P = 60-Pin Plastic DIP
         V = 68-Pin PLCC
         F = 80-Pin QFP
   Temp: S = 0C to +70C
         E = -40C to +85C

Environmanetal Flow: C = Plastic Standard


Example from Ms.Pac-Man/Galaga - 20 year Reunion hardare (see src/mame/drivers/20pacgal.c):

   CPU is Z8S18020VSC = Z180, 20MHz, 68-Pin PLCC, 0C to +70C, Plastic Standard


Other CPUs that use a compatible Z180 core:

Hitachi HD647180 series:
  Available in QFP80, PLCC84 & DIP90 packages (the QFP80 is not pinout compatible)
  The HD647180 also has an internal ROM

 *****************************************************************************/

#include "debugger.h"
#include "driver.h"
#include "z180.h"
#include "cpu/z80/z80daisy.h"

#define VERBOSE 0

#define LOG(x)	do { if (VERBOSE) logerror x; } while (0)

/* execute main opcodes inside a big switch statement */
#ifndef BIG_SWITCH
#define BIG_SWITCH			0
#endif

/* use big flag arrays (4*64K) for ADD/ADC/SUB/SBC/CP results */
#define BIG_FLAGS_ARRAY 	1

static void set_irq_line(int irqline, int state);

/****************************************************************************/
/* The Z180 registers. HALT is set to 1 when the CPU is halted, the refresh */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/
typedef struct {
	PAIR	PREPC,PC,SP,AF,BC,DE,HL,IX,IY;
	PAIR	AF2,BC2,DE2,HL2;
	UINT8	R,R2,IFF1,IFF2,HALT,IM,I;
	UINT8	tmdr_latch; 		/* flag latched TMDR0H, TMDR1H values */
	UINT8	read_tcr_tmdr[2];	/* flag to indicate that TCR or TMDR was read */
	UINT32	iol;				/* I/O line status bits */
	UINT8	io[64]; 			/* 64 internal 8 bit registers */
	offs_t	mmu[16];			/* MMU address translation */
	UINT8	tmdrh[2];			/* latched TMDR0H and TMDR1H values */
	UINT16	tmdr_value[2];		/* TMDR values used byt PRT0 and PRT1 as down counter */
	UINT8	tif[2];				/* TIF0 and TIF1 values */
	UINT8	nmi_state;			/* nmi line state */
	UINT8	nmi_pending;		/* nmi pending */
	UINT8	irq_state[3];		/* irq line states (INT0,INT1,INT2) */
	UINT8	after_EI;			/* are we in the EI shadow? */
	z80_daisy_state *daisy;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *iospace;
}	Z180_Regs;

#define CF	0x01
#define NF	0x02
#define PF	0x04
#define VF	PF
#define XF	0x08
#define HF	0x10
#define YF	0x20
#define ZF	0x40
#define SF	0x80

/* I/O line status flags */
#define Z180_CKA0	  0x00000001  /* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
#define Z180_CKA1	  0x00000002  /* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
#define Z180_CKS	  0x00000004  /* I/O serial clock (active high) */
#define Z180_CTS0	  0x00000100  /* I   clear to send 0 (active low) */
#define Z180_CTS1	  0x00000200  /* I   clear to send 1 (active low) or RXS (mux) */
#define Z180_DCD0	  0x00000400  /* I   data carrier detect (active low) */
#define Z180_DREQ0	  0x00000800  /* I   data request DMA ch 0 (active low) or CKA0 (mux) */
#define Z180_DREQ1	  0x00001000  /* I   data request DMA ch 1 (active low) */
#define Z180_RXA0	  0x00002000  /* I   asynchronous receive data 0 (active high) */
#define Z180_RXA1	  0x00004000  /* I   asynchronous receive data 1 (active high) */
#define Z180_RXS	  0x00008000  /* I   clocked serial receive data (active high) or CTS1 (mux) */
#define Z180_RTS0	  0x00010000  /*   O request to send (active low) */
#define Z180_TEND0	  0x00020000  /*   O transfer end 0 (active low) or CKA1 (mux) */
#define Z180_TEND1	  0x00040000  /*   O transfer end 1 (active low) */
#define Z180_A18_TOUT 0x00080000  /*   O transfer out (PRT channel, active low) or A18 (mux) */
#define Z180_TXA0	  0x00100000  /*   O asynchronous transmit data 0 (active high) */
#define Z180_TXA1	  0x00200000  /*   O asynchronous transmit data 1 (active high) */
#define Z180_TXS	  0x00400000  /*   O clocked serial transmit data (active high) */

/*
 * Prevent warnings on NetBSD.  All identifiers beginning with an underscore
 * followed by an uppercase letter are reserved by the C standard (ISO/IEC
 * 9899:1999, 7.1.3) to be used by the implementation.  It'd be best to rename
 * all such instances, but this is less intrusive and error-prone.
 */
#undef _B
#undef _C
#undef _L

#define _PPC	Z180.PREPC.d	/* previous program counter */

#define _PCD	Z180.PC.d
#define _PC 	Z180.PC.w.l

#define _SPD	Z180.SP.d
#define _SP 	Z180.SP.w.l

#define _AFD	Z180.AF.d
#define _AF 	Z180.AF.w.l
#define _A		Z180.AF.b.h
#define _F		Z180.AF.b.l

#define _BCD	Z180.BC.d
#define _BC 	Z180.BC.w.l
#define _B		Z180.BC.b.h
#define _C		Z180.BC.b.l

#define _DED	Z180.DE.d
#define _DE 	Z180.DE.w.l
#define _D		Z180.DE.b.h
#define _E		Z180.DE.b.l

#define _HLD	Z180.HL.d
#define _HL 	Z180.HL.w.l
#define _H		Z180.HL.b.h
#define _L		Z180.HL.b.l

#define _IXD	Z180.IX.d
#define _IX 	Z180.IX.w.l
#define _HX 	Z180.IX.b.h
#define _LX 	Z180.IX.b.l

#define _IYD	Z180.IY.d
#define _IY 	Z180.IY.w.l
#define _HY 	Z180.IY.b.h
#define _LY 	Z180.IY.b.l

#define _I		Z180.I
#define _R		Z180.R
#define _R2 	Z180.R2
#define _IM 	Z180.IM
#define _IFF1	Z180.IFF1
#define _IFF2	Z180.IFF2
#define _HALT	Z180.HALT

#define IO(n)		Z180.io[(n)-Z180_CNTLA0]
#define IO_CNTLA0	IO(Z180_CNTLA0)
#define IO_CNTLA1	IO(Z180_CNTLA1)
#define IO_CNTLB0	IO(Z180_CNTLB0)
#define IO_CNTLB1	IO(Z180_CNTLB1)
#define IO_STAT0	IO(Z180_STAT0)
#define IO_STAT1	IO(Z180_STAT1)
#define IO_TDR0 	IO(Z180_TDR0)
#define IO_TDR1 	IO(Z180_TDR1)
#define IO_RDR0 	IO(Z180_RDR0)
#define IO_RDR1 	IO(Z180_RDR1)
#define IO_CNTR 	IO(Z180_CNTR)
#define IO_TRDR 	IO(Z180_TRDR)
#define IO_TMDR0L	IO(Z180_TMDR0L)
#define IO_TMDR0H	IO(Z180_TMDR0H)
#define IO_RLDR0L	IO(Z180_RLDR0L)
#define IO_RLDR0H	IO(Z180_RLDR0H)
#define IO_TCR		IO(Z180_TCR)
#define IO_IO11 	IO(Z180_IO11)
#define IO_ASEXT0	IO(Z180_ASEXT0)
#define IO_ASEXT1	IO(Z180_ASEXT1)
#define IO_TMDR1L	IO(Z180_TMDR1L)
#define IO_TMDR1H	IO(Z180_TMDR1H)
#define IO_RLDR1L	IO(Z180_RLDR1L)
#define IO_RLDR1H	IO(Z180_RLDR1H)
#define IO_FRC		IO(Z180_FRC)
#define IO_IO19 	IO(Z180_IO19)
#define IO_ASTC0L	IO(Z180_ASTC0L)
#define IO_ASTC0H	IO(Z180_ASTC0H)
#define IO_ASTC1L	IO(Z180_ASTC1L)
#define IO_ASTC1H	IO(Z180_ASTC1H)
#define IO_CMR		IO(Z180_CMR)
#define IO_CCR		IO(Z180_CCR)
#define IO_SAR0L	IO(Z180_SAR0L)
#define IO_SAR0H	IO(Z180_SAR0H)
#define IO_SAR0B	IO(Z180_SAR0B)
#define IO_DAR0L	IO(Z180_DAR0L)
#define IO_DAR0H	IO(Z180_DAR0H)
#define IO_DAR0B	IO(Z180_DAR0B)
#define IO_BCR0L	IO(Z180_BCR0L)
#define IO_BCR0H	IO(Z180_BCR0H)
#define IO_MAR1L	IO(Z180_MAR1L)
#define IO_MAR1H	IO(Z180_MAR1H)
#define IO_MAR1B	IO(Z180_MAR1B)
#define IO_IAR1L	IO(Z180_IAR1L)
#define IO_IAR1H	IO(Z180_IAR1H)
#define IO_IAR1B	IO(Z180_IAR1B)
#define IO_BCR1L	IO(Z180_BCR1L)
#define IO_BCR1H	IO(Z180_BCR1H)
#define IO_DSTAT	IO(Z180_DSTAT)
#define IO_DMODE	IO(Z180_DMODE)
#define IO_DCNTL	IO(Z180_DCNTL)
#define IO_IL		IO(Z180_IL)
#define IO_ITC		IO(Z180_ITC)
#define IO_IO35 	IO(Z180_IO35)
#define IO_RCR		IO(Z180_RCR)
#define IO_IO37 	IO(Z180_IO37)
#define IO_CBR		IO(Z180_CBR)
#define IO_BBR		IO(Z180_BBR)
#define IO_CBAR 	IO(Z180_CBAR)
#define IO_IO3B 	IO(Z180_IO3B)
#define IO_IO3C 	IO(Z180_IO3C)
#define IO_IO3D 	IO(Z180_IO3D)
#define IO_OMCR 	IO(Z180_OMCR)
#define IO_IOCR 	IO(Z180_IOCR)

/* 00 ASCI control register A ch 0 */
#define Z180_CNTLA0_MPE 		0x80
#define Z180_CNTLA0_RE			0x40
#define Z180_CNTLA0_TE			0x20
#define Z180_CNTLA0_RTS0		0x10
#define Z180_CNTLA0_MPBR_EFR	0x08
#define Z180_CNTLA0_MODE_DATA	0x04
#define Z180_CNTLA0_MODE_PARITY 0x02
#define Z180_CNTLA0_MODE_STOPB	0x01

#define Z180_CNTLA0_RESET		0x10
#define Z180_CNTLA0_RMASK		0xff
#define Z180_CNTLA0_WMASK		0xff

/* 01 ASCI control register A ch 1 */
#define Z180_CNTLA1_MPE 		0x80
#define Z180_CNTLA1_RE			0x40
#define Z180_CNTLA1_TE			0x20
#define Z180_CNTLA1_CKA1D		0x10
#define Z180_CNTLA1_MPBR_EFR	0x08
#define Z180_CNTLA1_MODE		0x07

#define Z180_CNTLA1_RESET		0x10
#define Z180_CNTLA1_RMASK		0xff
#define Z180_CNTLA1_WMASK		0xff

/* 02 ASCI control register B ch 0 */
#define Z180_CNTLB0_MPBT		0x80
#define Z180_CNTLB0_MP			0x40
#define Z180_CNTLB0_CTS_PS		0x20
#define Z180_CNTLB0_PEO 		0x10
#define Z180_CNTLB0_DR			0x08
#define Z180_CNTLB0_SS			0x07

#define Z180_CNTLB0_RESET		0x07
#define Z180_CNTLB0_RMASK		0xff
#define Z180_CNTLB0_WMASK		0xff

/* 03 ASCI control register B ch 1 */
#define Z180_CNTLB1_MPBT		0x80
#define Z180_CNTLB1_MP			0x40
#define Z180_CNTLB1_CTS_PS		0x20
#define Z180_CNTLB1_PEO 		0x10
#define Z180_CNTLB1_DR			0x08
#define Z180_CNTLB1_SS			0x07

#define Z180_CNTLB1_RESET		0x07
#define Z180_CNTLB1_RMASK		0xff
#define Z180_CNTLB1_WMASK		0xff

/* 04 ASCI status register 0 */
#define Z180_STAT0_RDRF 		0x80
#define Z180_STAT0_OVRN 		0x40
#define Z180_STAT0_PE			0x20
#define Z180_STAT0_FE			0x10
#define Z180_STAT0_RIE			0x08
#define Z180_STAT0_DCD0 		0x04
#define Z180_STAT0_TDRE 		0x02
#define Z180_STAT0_TIE			0x01

#define Z180_STAT0_RESET		0x00
#define Z180_STAT0_RMASK		0xff
#define Z180_STAT0_WMASK		0x09

/* 05 ASCI status register 1 */
#define Z180_STAT1_RDRF 		0x80
#define Z180_STAT1_OVRN 		0x40
#define Z180_STAT1_PE			0x20
#define Z180_STAT1_FE			0x10
#define Z180_STAT1_RIE			0x08
#define Z180_STAT1_CTS1E		0x04
#define Z180_STAT1_TDRE 		0x02
#define Z180_STAT1_TIE			0x01

#define Z180_STAT1_RESET		0x00
#define Z180_STAT1_RMASK		0xff
#define Z180_STAT1_WMASK		0x0d

/* 06 ASCI transmit data register 0 */
#define Z180_TDR0_TDR			0xff

#define Z180_TDR0_RESET 		0x00
#define Z180_TDR0_RMASK 		0xff
#define Z180_TDR0_WMASK 		0xff

/* 07 ASCI transmit data register 1 */
#define Z180_TDR1_TDR			0xff

#define Z180_TDR1_RESET 		0x00
#define Z180_TDR1_RMASK 		0xff
#define Z180_TDR1_WMASK 		0xff

/* 08 ASCI receive register 0 */
#define Z180_RDR0_RDR			0xff

#define Z180_RDR0_RESET 		0x00
#define Z180_RDR0_RMASK 		0xff
#define Z180_RDR0_WMASK 		0xff

/* 09 ASCI receive register 1 */
#define Z180_RDR1_RDR			0xff

#define Z180_RDR1_RESET 		0x00
#define Z180_RDR1_RMASK 		0xff
#define Z180_RDR1_WMASK 		0xff

/* 0a CSI/O control/status register */
#define Z180_CNTR_EF			0x80
#define Z180_CNTR_EIE			0x40
#define Z180_CNTR_RE			0x20
#define Z180_CNTR_TE			0x10
#define Z180_CNTR_SS			0x07

#define Z180_CNTR_RESET 		0x07
#define Z180_CNTR_RMASK 		0xff
#define Z180_CNTR_WMASK 		0x7f

/* 0b CSI/O transmit/receive register */
#define Z180_TRDR_RESET 		0x00
#define Z180_TRDR_RMASK 		0xff
#define Z180_TRDR_WMASK 		0xff

/* 0c TIMER data register ch 0 L */
#define Z180_TMDR0L_RESET		0x00
#define Z180_TMDR0L_RMASK		0xff
#define Z180_TMDR0L_WMASK		0xff

/* 0d TIMER data register ch 0 H */
#define Z180_TMDR0H_RESET		0x00
#define Z180_TMDR0H_RMASK		0xff
#define Z180_TMDR0H_WMASK		0xff

/* 0e TIMER reload register ch 0 L */
#define Z180_RLDR0L_RESET		0xff
#define Z180_RLDR0L_RMASK		0xff
#define Z180_RLDR0L_WMASK		0xff

/* 0f TIMER reload register ch 0 H */
#define Z180_RLDR0H_RESET		0xff
#define Z180_RLDR0H_RMASK		0xff
#define Z180_RLDR0H_WMASK		0xff

/* 10 TIMER control register */
#define Z180_TCR_TIF1			0x80
#define Z180_TCR_TIF0			0x40
#define Z180_TCR_TIE1			0x20
#define Z180_TCR_TIE0			0x10
#define Z180_TCR_TOC1			0x08
#define Z180_TCR_TOC0			0x04
#define Z180_TCR_TDE1			0x02
#define Z180_TCR_TDE0			0x01

#define Z180_TCR_RESET			0x00
#define Z180_TCR_RMASK			0xff
#define Z180_TCR_WMASK			0x3f

/* 11 reserved */
#define Z180_IO11_RESET 		0x00
#define Z180_IO11_RMASK 		0xff
#define Z180_IO11_WMASK 		0xff

/* 12 (Z8S180/Z8L180) ASCI extension control register 0 */
#define Z180_ASEXT0_RDRF		0x80
#define Z180_ASEXT0_DCD0		0x40
#define Z180_ASEXT0_CTS0		0x20
#define Z180_ASEXT0_X1_BIT_CLK0 0x10
#define Z180_ASEXT0_BRG0_MODE	0x08
#define Z180_ASEXT0_BRK_EN		0x04
#define Z180_ASEXT0_BRK_DET 	0x02
#define Z180_ASEXT0_BRK_SEND	0x01

#define Z180_ASEXT0_RESET		0x00
#define Z180_ASEXT0_RMASK		0xff
#define Z180_ASEXT0_WMASK		0xfd

/* 13 (Z8S180/Z8L180) ASCI extension control register 0 */
#define Z180_ASEXT1_RDRF		0x80
#define Z180_ASEXT1_X1_BIT_CLK1 0x10
#define Z180_ASEXT1_BRG1_MODE	0x08
#define Z180_ASEXT1_BRK_EN		0x04
#define Z180_ASEXT1_BRK_DET 	0x02
#define Z180_ASEXT1_BRK_SEND	0x01

#define Z180_ASEXT1_RESET		0x00
#define Z180_ASEXT1_RMASK		0xff
#define Z180_ASEXT1_WMASK		0xfd


/* 14 TIMER data register ch 1 L */
#define Z180_TMDR1L_RESET		0x00
#define Z180_TMDR1L_RMASK		0xff
#define Z180_TMDR1L_WMASK		0xff

/* 15 TIMER data register ch 1 H */
#define Z180_TMDR1H_RESET		0x00
#define Z180_TMDR1H_RMASK		0xff
#define Z180_TMDR1H_WMASK		0xff

/* 16 TIMER reload register ch 1 L */
#define Z180_RLDR1L_RESET		0x00
#define Z180_RLDR1L_RMASK		0xff
#define Z180_RLDR1L_WMASK		0xff

/* 17 TIMER reload register ch 1 H */
#define Z180_RLDR1H_RESET		0x00
#define Z180_RLDR1H_RMASK		0xff
#define Z180_RLDR1H_WMASK		0xff

/* 18 free running counter */
#define Z180_FRC_RESET			0x00
#define Z180_FRC_RMASK			0xff
#define Z180_FRC_WMASK			0xff

/* 19 reserved */
#define Z180_IO19_RESET 		0x00
#define Z180_IO19_RMASK 		0xff
#define Z180_IO19_WMASK 		0xff

/* 1a ASCI time constant ch 0 L */
#define Z180_ASTC0L_RESET		0x00
#define Z180_ASTC0L_RMASK		0xff
#define Z180_ASTC0L_WMASK		0xff

/* 1b ASCI time constant ch 0 H */
#define Z180_ASTC0H_RESET		0x00
#define Z180_ASTC0H_RMASK		0xff
#define Z180_ASTC0H_WMASK		0xff

/* 1c ASCI time constant ch 1 L */
#define Z180_ASTC1L_RESET		0x00
#define Z180_ASTC1L_RMASK		0xff
#define Z180_ASTC1L_WMASK		0xff

/* 1d ASCI time constant ch 1 H */
#define Z180_ASTC1H_RESET		0x00
#define Z180_ASTC1H_RMASK		0xff
#define Z180_ASTC1H_WMASK		0xff

/* 1e clock multiplier */
#define Z180_CMR_X2 			0x80

#define Z180_CMR_RESET			0x7f
#define Z180_CMR_RMASK			0x80
#define Z180_CMR_WMASK			0x80

/* 1f chip control register */
#define Z180_CCR_CLOCK_DIVIDE	0x80
#define Z180_CCR_STDBY_IDLE1	0x40
#define Z180_CCR_BREXT			0x20
#define Z180_CCR_LNPHI			0x10
#define Z180_CCR_STDBY_IDLE0	0x08
#define Z180_CCR_LNIO			0x04
#define Z180_CCR_LNCPU_CTL		0x02
#define Z180_CCR_LNAD_DATA		0x01

#define Z180_CCR_RESET			0x00
#define Z180_CCR_RMASK			0xff
#define Z180_CCR_WMASK			0xff

/* 20 DMA source address register ch 0 L */
#define Z180_SAR0L_SAR			0xff

#define Z180_SAR0L_RESET		0x00
#define Z180_SAR0L_RMASK		0xff
#define Z180_SAR0L_WMASK		0xff

/* 21 DMA source address register ch 0 H */
#define Z180_SAR0H_SAR			0xff

#define Z180_SAR0H_RESET		0x00
#define Z180_SAR0H_RMASK		0xff
#define Z180_SAR0H_WMASK		0xff

/* 22 DMA source address register ch 0 B */
#define Z180_SAR0B_SAR			0x0f

#define Z180_SAR0B_RESET		0x00
#define Z180_SAR0B_RMASK		0x0f
#define Z180_SAR0B_WMASK		0x0f

/* 23 DMA destination address register ch 0 L */
#define Z180_DAR0L_DAR			0xff

#define Z180_DAR0L_RESET		0x00
#define Z180_DAR0L_RMASK		0xff
#define Z180_DAR0L_WMASK		0xff

/* 24 DMA destination address register ch 0 H */
#define Z180_DAR0H_DAR			0xff

#define Z180_DAR0H_RESET		0x00
#define Z180_DAR0H_RMASK		0xff
#define Z180_DAR0H_WMASK		0xff

/* 25 DMA destination address register ch 0 B */
#define Z180_DAR0B_DAR			0x00

#define Z180_DAR0B_RESET		0x00
#define Z180_DAR0B_RMASK		0x0f
#define Z180_DAR0B_WMASK		0x0f

/* 26 DMA byte count register ch 0 L */
#define Z180_BCR0L_BCR			0xff

#define Z180_BCR0L_RESET		0x00
#define Z180_BCR0L_RMASK		0xff
#define Z180_BCR0L_WMASK		0xff

/* 27 DMA byte count register ch 0 H */
#define Z180_BCR0H_BCR			0xff

#define Z180_BCR0H_RESET		0x00
#define Z180_BCR0H_RMASK		0xff
#define Z180_BCR0H_WMASK		0xff

/* 28 DMA memory address register ch 1 L */
#define Z180_MAR1L_MAR			0xff

#define Z180_MAR1L_RESET		0x00
#define Z180_MAR1L_RMASK		0xff
#define Z180_MAR1L_WMASK		0xff

/* 29 DMA memory address register ch 1 H */
#define Z180_MAR1H_MAR			0xff

#define Z180_MAR1H_RESET		0x00
#define Z180_MAR1H_RMASK		0xff
#define Z180_MAR1H_WMASK		0xff

/* 2a DMA memory address register ch 1 B */
#define Z180_MAR1B_MAR			0x0f

#define Z180_MAR1B_RESET		0x00
#define Z180_MAR1B_RMASK		0x0f
#define Z180_MAR1B_WMASK		0x0f

/* 2b DMA I/O address register ch 1 L */
#define Z180_IAR1L_IAR			0xff

#define Z180_IAR1L_RESET		0x00
#define Z180_IAR1L_RMASK		0xff
#define Z180_IAR1L_WMASK		0xff

/* 2c DMA I/O address register ch 1 H */
#define Z180_IAR1H_IAR			0xff

#define Z180_IAR1H_RESET		0x00
#define Z180_IAR1H_RMASK		0xff
#define Z180_IAR1H_WMASK		0xff

/* 2d (Z8S180/Z8L180) DMA I/O address register ch 1 B */
#define Z180_IAR1B_IAR			0x0f

#define Z180_IAR1B_RESET		0x00
#define Z180_IAR1B_RMASK		0x0f
#define Z180_IAR1B_WMASK		0x0f

/* 2e DMA byte count register ch 1 L */
#define Z180_BCR1L_BCR			0xff

#define Z180_BCR1L_RESET		0x00
#define Z180_BCR1L_RMASK		0xff
#define Z180_BCR1L_WMASK		0xff

/* 2f DMA byte count register ch 1 H */
#define Z180_BCR1H_BCR			0xff

#define Z180_BCR1H_RESET		0x00
#define Z180_BCR1H_RMASK		0xff
#define Z180_BCR1H_WMASK		0xff

/* 30 DMA status register */
#define Z180_DSTAT_DE1			0x80	/* DMA enable ch 1 */
#define Z180_DSTAT_DE0			0x40	/* DMA enable ch 0 */
#define Z180_DSTAT_DWE1 		0x20	/* DMA write enable ch 0 (active low) */
#define Z180_DSTAT_DWE0 		0x10	/* DMA write enable ch 1 (active low) */
#define Z180_DSTAT_DIE1 		0x08	/* DMA IRQ enable ch 1 */
#define Z180_DSTAT_DIE0 		0x04	/* DMA IRQ enable ch 0 */
#define Z180_DSTAT_DME			0x01	/* DMA enable (read only) */

#define Z180_DSTAT_RESET		0x30
#define Z180_DSTAT_RMASK		0xfd
#define Z180_DSTAT_WMASK		0xcc

/* 31 DMA mode register */
#define Z180_DMODE_DM			0x30
#define Z180_DMODE_SM			0x0c
#define Z180_DMODE_MMOD 		0x04

#define Z180_DMODE_RESET		0x00
#define Z180_DMODE_RMASK		0x3e
#define Z180_DMODE_WMASK		0x3e

/* 32 DMA/WAIT control register */
#define Z180_DCNTL_MWI1 		0x80
#define Z180_DCNTL_MWI0 		0x40
#define Z180_DCNTL_IWI1 		0x20
#define Z180_DCNTL_IWI0 		0x10
#define Z180_DCNTL_DMS1 		0x08
#define Z180_DCNTL_DMS0 		0x04
#define Z180_DCNTL_DIM1 		0x02
#define Z180_DCNTL_DIM0 		0x01

#define Z180_DCNTL_RESET		0x00
#define Z180_DCNTL_RMASK		0xff
#define Z180_DCNTL_WMASK		0xff

/* 33 INT vector low register */
#define Z180_IL_IL				0xe0

#define Z180_IL_RESET			0x00
#define Z180_IL_RMASK			0xe0
#define Z180_IL_WMASK			0xe0

/* 34 INT/TRAP control register */
#define Z180_ITC_TRAP			0x80
#define Z180_ITC_UFO			0x40
#define Z180_ITC_ITE2			0x04
#define Z180_ITC_ITE1			0x02
#define Z180_ITC_ITE0			0x01

#define Z180_ITC_RESET			0x01
#define Z180_ITC_RMASK			0xc7
#define Z180_ITC_WMASK			0x87

/* 35 reserved */
#define Z180_IO35_RESET 		0x00
#define Z180_IO35_RMASK 		0xff
#define Z180_IO35_WMASK 		0xff

/* 36 refresh control register */
#define Z180_RCR_REFE			0x80
#define Z180_RCR_REFW			0x80
#define Z180_RCR_CYC			0x03

#define Z180_RCR_RESET			0xc0
#define Z180_RCR_RMASK			0xc3
#define Z180_RCR_WMASK			0xc3

/* 37 reserved */
#define Z180_IO37_RESET 		0x00
#define Z180_IO37_RMASK 		0xff
#define Z180_IO37_WMASK 		0xff

/* 38 MMU common base register */
#define Z180_CBR_CB 			0xff

#define Z180_CBR_RESET			0x00
#define Z180_CBR_RMASK			0xff
#define Z180_CBR_WMASK			0xff

/* 39 MMU bank base register */
#define Z180_BBR_BB 			0xff

#define Z180_BBR_RESET			0x00
#define Z180_BBR_RMASK			0xff
#define Z180_BBR_WMASK			0xff

/* 3a MMU common/bank area register */
#define Z180_CBAR_CA			0xf0
#define Z180_CBAR_BA			0x0f

#define Z180_CBAR_RESET 		0xf0
#define Z180_CBAR_RMASK 		0xff
#define Z180_CBAR_WMASK 		0xff

/* 3b reserved */
#define Z180_IO3B_RESET 		0x00
#define Z180_IO3B_RMASK 		0xff
#define Z180_IO3B_WMASK 		0xff

/* 3c reserved */
#define Z180_IO3C_RESET 		0x00
#define Z180_IO3C_RMASK 		0xff
#define Z180_IO3C_WMASK 		0xff

/* 3d reserved */
#define Z180_IO3D_RESET 		0x00
#define Z180_IO3D_RMASK 		0xff
#define Z180_IO3D_WMASK 		0xff

/* 3e operation mode control register */
#define Z180_OMCR_RESET 		0x00
#define Z180_OMCR_RMASK 		0xff
#define Z180_OMCR_WMASK 		0xff

/* 3f I/O control register */
#define Z180_IOCR_RESET 		0x00
#define Z180_IOCR_RMASK 		0xff
#define Z180_IOCR_WMASK 		0xff

static int z180_icount;
static Z180_Regs Z180;
static UINT32 EA;

static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

#if BIG_FLAGS_ARRAY
static UINT8 *SZHVC_add;
static UINT8 *SZHVC_sub;
#endif

/****************************************************************************
 * Burn an odd amount of cycles, that is instructions taking something
 * different from 4 T-states per opcode (and R increment)
 ****************************************************************************/
INLINE void BURNODD(int cycles, int opcodes, int cyclesum)
{
	if( cycles > 0 )
	{
		_R += (cycles / cyclesum) * opcodes;
		z180_icount -= (cycles / cyclesum) * cyclesum;
	}
}

static UINT8 z180_readcontrol(offs_t port);
static void z180_writecontrol(offs_t port, UINT8 data);
static void z180_dma0(void);
static void z180_dma1(void);
static CPU_BURN( z180 );
static CPU_SET_INFO( z180 );

#include "z180daa.h"
#include "z180ops.h"
#include "z180tbl.h"

#include "z180cb.c"
#include "z180xy.c"
#include "z180dd.c"
#include "z180fd.c"
#include "z180ed.c"
#include "z180op.c"

static UINT8 z180_readcontrol(offs_t port)
{
	/* normal external readport */
	UINT8 data = memory_read_byte_8le(Z180.iospace, port);

	/* remap internal I/O registers */
	if((port & (IO_IOCR & 0xc0)) == (IO_IOCR & 0xc0))
		port = port - (IO_IOCR & 0xc0);

	/* but ignore the data and read the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		data = IO_CNTLA0 & Z180_CNTLA0_RMASK;
		LOG(("Z180 '%s' CNTLA0 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CNTLA1:
		data = IO_CNTLA1 & Z180_CNTLA1_RMASK;
		LOG(("Z180 '%s' CNTLA1 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CNTLB0:
		data = IO_CNTLB0 & Z180_CNTLB0_RMASK;
		LOG(("Z180 '%s' CNTLB0 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CNTLB1:
		data = IO_CNTLB1 & Z180_CNTLB1_RMASK;
		LOG(("Z180 '%s' CNTLB1 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_STAT0:
		data = IO_STAT0 & Z180_STAT0_RMASK;
data |= 0x02; // kludge for 20pacgal
		LOG(("Z180 '%s' STAT0  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_STAT1:
		data = IO_STAT1 & Z180_STAT1_RMASK;
		LOG(("Z180 '%s' STAT1  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TDR0:
		data = IO_TDR0 & Z180_TDR0_RMASK;
		LOG(("Z180 '%s' TDR0   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TDR1:
		data = IO_TDR1 & Z180_TDR1_RMASK;
		LOG(("Z180 '%s' TDR1   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RDR0:
		data = IO_RDR0 & Z180_RDR0_RMASK;
		LOG(("Z180 '%s' RDR0   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RDR1:
		data = IO_RDR1 & Z180_RDR1_RMASK;
		LOG(("Z180 '%s' RDR1   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CNTR:
		data = IO_CNTR & Z180_CNTR_RMASK;
		LOG(("Z180 '%s' CNTR   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TRDR:
		data = IO_TRDR & Z180_TRDR_RMASK;
		LOG(("Z180 '%s' TRDR   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TMDR0L:
		data = Z180.tmdr_value[0] & Z180_TMDR0L_RMASK;
		LOG(("Z180 '%s' TMDR0L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((IO_TCR & Z180_TCR_TDE0) == 0)
		{
			Z180.tmdr_latch |= 1;
			Z180.tmdrh[0] = (Z180.tmdr_value[0] & 0xff00) >> 8;
		}

		if(Z180.read_tcr_tmdr[0])
		{
			Z180.tif[0] = 0; // reset TIF0
			Z180.read_tcr_tmdr[0] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[0] = 1;
		}
		break;

	case Z180_TMDR0H:
		/* read latched value? */
		if (Z180.tmdr_latch & 1)
		{
			Z180.tmdr_latch &= ~1;
			data = Z180.tmdrh[0];
		}
		else
		{
			data = (Z180.tmdr_value[0] & 0xff00) >> 8;
		}

		if(Z180.read_tcr_tmdr[0])
		{
			Z180.tif[0] = 0; // reset TIF0
			Z180.read_tcr_tmdr[0] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[0] = 1;
		}
		LOG(("Z180 '%s' TMDR0H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RLDR0L:
		data = IO_RLDR0L & Z180_RLDR0L_RMASK;
		LOG(("Z180 '%s' RLDR0L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RLDR0H:
		data = IO_RLDR0H & Z180_RLDR0H_RMASK;
		LOG(("Z180 '%s' RLDR0H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TCR:
		data = (IO_TCR & Z180_TCR_RMASK) | (Z180.tif[0] << 6) | (Z180.tif[1] << 7);

		if(Z180.read_tcr_tmdr[0])
		{
			Z180.tif[0] = 0; // reset TIF0
			Z180.read_tcr_tmdr[0] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[0] = 1;
		}

		if(Z180.read_tcr_tmdr[1])
		{
			Z180.tif[1] = 0; // reset TIF1
			Z180.read_tcr_tmdr[1] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[1] = 1;
		}

		LOG(("Z180 '%s' TCR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO11:
		data = IO_IO11 & Z180_IO11_RMASK;
		LOG(("Z180 '%s' IO11   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASEXT0:
		data = IO_ASEXT0 & Z180_ASEXT0_RMASK;
		LOG(("Z180 '%s' ASEXT0 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASEXT1:
		data = IO_ASEXT1 & Z180_ASEXT1_RMASK;
		LOG(("Z180 '%s' ASEXT1 rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_TMDR1L:
		data = Z180.tmdr_value[1] & Z180_TMDR1L_RMASK;
		LOG(("Z180 '%s' TMDR1L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((IO_TCR & Z180_TCR_TDE1) == 0)
		{
			Z180.tmdr_latch |= 2;
			Z180.tmdrh[1] = (Z180.tmdr_value[1] & 0xff00) >> 8;
		}

		if(Z180.read_tcr_tmdr[1])
		{
			Z180.tif[1] = 0; // reset TIF1
			Z180.read_tcr_tmdr[1] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[1] = 1;
		}
		break;

	case Z180_TMDR1H:
		/* read latched value? */
		if (Z180.tmdr_latch & 2)
		{
			Z180.tmdr_latch &= ~2;
			data = Z180.tmdrh[1];
		}
		else
		{
			data = (Z180.tmdr_value[1] & 0xff00) >> 8;
		}

		if(Z180.read_tcr_tmdr[1])
		{
			Z180.tif[1] = 0; // reset TIF1
			Z180.read_tcr_tmdr[1] = 0;
		}
		else
		{
			Z180.read_tcr_tmdr[1] = 1;
		}
		LOG(("Z180 '%s' TMDR1H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RLDR1L:
		data = IO_RLDR1L & Z180_RLDR1L_RMASK;
		LOG(("Z180 '%s' RLDR1L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RLDR1H:
		data = IO_RLDR1H & Z180_RLDR1H_RMASK;
		LOG(("Z180 '%s' RLDR1H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_FRC:
		data = IO_FRC & Z180_FRC_RMASK;
		LOG(("Z180 '%s' FRC    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO19:
		data = IO_IO19 & Z180_IO19_RMASK;
		LOG(("Z180 '%s' IO19   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASTC0L:
		data = IO_ASTC0L & Z180_ASTC0L_RMASK;
		LOG(("Z180 '%s' ASTC0L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASTC0H:
		data = IO_ASTC0H & Z180_ASTC0H_RMASK;
		LOG(("Z180 '%s' ASTC0H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASTC1L:
		data = IO_ASTC1L & Z180_ASTC1L_RMASK;
		LOG(("Z180 '%s' ASTC1L rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ASTC1H:
		data = IO_ASTC1H & Z180_ASTC1H_RMASK;
		LOG(("Z180 '%s' ASTC1H rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CMR:
		data = IO_CMR & Z180_CMR_RMASK;
		LOG(("Z180 '%s' CMR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CCR:
		data = IO_CCR & Z180_CCR_RMASK;
		LOG(("Z180 '%s' CCR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_SAR0L:
		data = IO_SAR0L & Z180_SAR0L_RMASK;
		LOG(("Z180 '%s' SAR0L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_SAR0H:
		data = IO_SAR0H & Z180_SAR0H_RMASK;
		LOG(("Z180 '%s' SAR0H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_SAR0B:
		data = IO_SAR0B & Z180_SAR0B_RMASK;
		LOG(("Z180 '%s' SAR0B  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DAR0L:
		data = IO_DAR0L & Z180_DAR0L_RMASK;
		LOG(("Z180 '%s' DAR0L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DAR0H:
		data = IO_DAR0H & Z180_DAR0H_RMASK;
		LOG(("Z180 '%s' DAR0H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DAR0B:
		data = IO_DAR0B & Z180_DAR0B_RMASK;
		LOG(("Z180 '%s' DAR0B  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_BCR0L:
		data = IO_BCR0L & Z180_BCR0L_RMASK;
		LOG(("Z180 '%s' BCR0L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_BCR0H:
		data = IO_BCR0H & Z180_BCR0H_RMASK;
		LOG(("Z180 '%s' BCR0H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_MAR1L:
		data = IO_MAR1L & Z180_MAR1L_RMASK;
		LOG(("Z180 '%s' MAR1L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_MAR1H:
		data = IO_MAR1H & Z180_MAR1H_RMASK;
		LOG(("Z180 '%s' MAR1H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_MAR1B:
		data = IO_MAR1B & Z180_MAR1B_RMASK;
		LOG(("Z180 '%s' MAR1B  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IAR1L:
		data = IO_IAR1L & Z180_IAR1L_RMASK;
		LOG(("Z180 '%s' IAR1L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IAR1H:
		data = IO_IAR1H & Z180_IAR1H_RMASK;
		LOG(("Z180 '%s' IAR1H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IAR1B:
		data = IO_IAR1B & Z180_IAR1B_RMASK;
		LOG(("Z180 '%s' IAR1B  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_BCR1L:
		data = IO_BCR1L & Z180_BCR1L_RMASK;
		LOG(("Z180 '%s' BCR1L  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_BCR1H:
		data = IO_BCR1H & Z180_BCR1H_RMASK;
		LOG(("Z180 '%s' BCR1H  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DSTAT:
		data = IO_DSTAT & Z180_DSTAT_RMASK;
		LOG(("Z180 '%s' DSTAT  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DMODE:
		data = IO_DMODE & Z180_DMODE_RMASK;
		LOG(("Z180 '%s' DMODE  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_DCNTL:
		data = IO_DCNTL & Z180_DCNTL_RMASK;
		LOG(("Z180 '%s' DCNTL  rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IL:
		data = IO_IL & Z180_IL_RMASK;
		LOG(("Z180 '%s' IL     rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_ITC:
		data = IO_ITC & Z180_ITC_RMASK;
		LOG(("Z180 '%s' ITC    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO35:
		data = IO_IO35 & Z180_IO35_RMASK;
		LOG(("Z180 '%s' IO35   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_RCR:
		data = IO_RCR & Z180_RCR_RMASK;
		LOG(("Z180 '%s' RCR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO37:
		data = IO_IO37 & Z180_IO37_RMASK;
		LOG(("Z180 '%s' IO37   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CBR:
		data = IO_CBR & Z180_CBR_RMASK;
		LOG(("Z180 '%s' CBR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_BBR:
		data = IO_BBR & Z180_BBR_RMASK;
		LOG(("Z180 '%s' BBR    rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_CBAR:
		data = IO_CBAR & Z180_CBAR_RMASK;
		LOG(("Z180 '%s' CBAR   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO3B:
		data = IO_IO3B & Z180_IO3B_RMASK;
		LOG(("Z180 '%s' IO3B   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO3C:
		data = IO_IO3C & Z180_IO3C_RMASK;
		LOG(("Z180 '%s' IO3C   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IO3D:
		data = IO_IO3D & Z180_IO3D_RMASK;
		LOG(("Z180 '%s' IO3D   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_OMCR:
		data = IO_OMCR & Z180_OMCR_RMASK;
		LOG(("Z180 '%s' OMCR   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;

	case Z180_IOCR:
		data = IO_IOCR & Z180_IOCR_RMASK;
		LOG(("Z180 '%s' IOCR   rd $%02x ($%02x)\n", Z180.device->tag, data, Z180.io[port & 0x3f]));
		break;
	}

	return data;
}

static void z180_writecontrol(offs_t port, UINT8 data)
{
	/* normal external write port */
	memory_write_byte_8le(Z180.iospace, port, data);

	/* remap internal I/O registers */
	if((port & (IO_IOCR & 0xc0)) == (IO_IOCR & 0xc0))
		port = port - (IO_IOCR & 0xc0);

	/* store the data in the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		LOG(("Z180 '%s' CNTLA0 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CNTLA0_WMASK));
		IO_CNTLA0 = (IO_CNTLA0 & ~Z180_CNTLA0_WMASK) | (data & Z180_CNTLA0_WMASK);
		break;

	case Z180_CNTLA1:
		LOG(("Z180 '%s' CNTLA1 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CNTLA1_WMASK));
		IO_CNTLA1 = (IO_CNTLA1 & ~Z180_CNTLA1_WMASK) | (data & Z180_CNTLA1_WMASK);
		break;

	case Z180_CNTLB0:
		LOG(("Z180 '%s' CNTLB0 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CNTLB0_WMASK));
		IO_CNTLB0 = (IO_CNTLB0 & ~Z180_CNTLB0_WMASK) | (data & Z180_CNTLB0_WMASK);
		break;

	case Z180_CNTLB1:
		LOG(("Z180 '%s' CNTLB1 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CNTLB1_WMASK));
		IO_CNTLB1 = (IO_CNTLB1 & ~Z180_CNTLB1_WMASK) | (data & Z180_CNTLB1_WMASK);
		break;

	case Z180_STAT0:
		LOG(("Z180 '%s' STAT0  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_STAT0_WMASK));
		IO_STAT0 = (IO_STAT0 & ~Z180_STAT0_WMASK) | (data & Z180_STAT0_WMASK);
		break;

	case Z180_STAT1:
		LOG(("Z180 '%s' STAT1  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_STAT1_WMASK));
		IO_STAT1 = (IO_STAT1 & ~Z180_STAT1_WMASK) | (data & Z180_STAT1_WMASK);
		break;

	case Z180_TDR0:
		LOG(("Z180 '%s' TDR0   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TDR0_WMASK));
		IO_TDR0 = (IO_TDR0 & ~Z180_TDR0_WMASK) | (data & Z180_TDR0_WMASK);
		break;

	case Z180_TDR1:
		LOG(("Z180 '%s' TDR1   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TDR1_WMASK));
		IO_TDR1 = (IO_TDR1 & ~Z180_TDR1_WMASK) | (data & Z180_TDR1_WMASK);
		break;

	case Z180_RDR0:
		LOG(("Z180 '%s' RDR0   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RDR0_WMASK));
		IO_RDR0 = (IO_RDR0 & ~Z180_RDR0_WMASK) | (data & Z180_RDR0_WMASK);
		break;

	case Z180_RDR1:
		LOG(("Z180 '%s' RDR1   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RDR1_WMASK));
		IO_RDR1 = (IO_RDR1 & ~Z180_RDR1_WMASK) | (data & Z180_RDR1_WMASK);
		break;

	case Z180_CNTR:
		LOG(("Z180 '%s' CNTR   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CNTR_WMASK));
		IO_CNTR = (IO_CNTR & ~Z180_CNTR_WMASK) | (data & Z180_CNTR_WMASK);
		break;

	case Z180_TRDR:
		LOG(("Z180 '%s' TRDR   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TRDR_WMASK));
		IO_TRDR = (IO_TRDR & ~Z180_TRDR_WMASK) | (data & Z180_TRDR_WMASK);
		break;

	case Z180_TMDR0L:
		LOG(("Z180 '%s' TMDR0L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TMDR0L_WMASK));
		IO_TMDR0L = data & Z180_TMDR0L_WMASK;
		Z180.tmdr_value[0] = (Z180.tmdr_value[0] & 0xff00) | IO_TMDR0L;
		break;

	case Z180_TMDR0H:
		LOG(("Z180 '%s' TMDR0H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TMDR0H_WMASK));
		IO_TMDR0H = data & Z180_TMDR0H_WMASK;
		Z180.tmdr_value[0] = (Z180.tmdr_value[0] & 0x00ff) | (IO_TMDR0H << 8);
		break;

	case Z180_RLDR0L:
		LOG(("Z180 '%s' RLDR0L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RLDR0L_WMASK));
		IO_RLDR0L = (IO_RLDR0L & ~Z180_RLDR0L_WMASK) | (data & Z180_RLDR0L_WMASK);
		break;

	case Z180_RLDR0H:
		LOG(("Z180 '%s' RLDR0H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RLDR0H_WMASK));
		IO_RLDR0H = (IO_RLDR0H & ~Z180_RLDR0H_WMASK) | (data & Z180_RLDR0H_WMASK);
		break;

	case Z180_TCR:
		LOG(("Z180 '%s' TCR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TCR_WMASK));
		IO_TCR = (IO_TCR & ~Z180_TCR_WMASK) | (data & Z180_TCR_WMASK);
		break;

	case Z180_IO11:
		LOG(("Z180 '%s' IO11   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO11_WMASK));
		IO_IO11 = (IO_IO11 & ~Z180_IO11_WMASK) | (data & Z180_IO11_WMASK);
		break;

	case Z180_ASEXT0:
		LOG(("Z180 '%s' ASEXT0 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASEXT0_WMASK));
		IO_ASEXT0 = (IO_ASEXT0 & ~Z180_ASEXT0_WMASK) | (data & Z180_ASEXT0_WMASK);
		break;

	case Z180_ASEXT1:
		LOG(("Z180 '%s' ASEXT1 wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASEXT1_WMASK));
		IO_ASEXT1 = (IO_ASEXT1 & ~Z180_ASEXT1_WMASK) | (data & Z180_ASEXT1_WMASK);
		break;

	case Z180_TMDR1L:
		LOG(("Z180 '%s' TMDR1L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TMDR1L_WMASK));
		IO_TMDR1L = data & Z180_TMDR1L_WMASK;
		Z180.tmdr_value[1] = (Z180.tmdr_value[1] & 0xff00) | IO_TMDR1L;
		break;

	case Z180_TMDR1H:
		LOG(("Z180 '%s' TMDR1H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_TMDR1H_WMASK));
		IO_TMDR1H = data & Z180_TMDR1H_WMASK;
		Z180.tmdr_value[1] = (Z180.tmdr_value[1] & 0x00ff) | IO_TMDR1H;
		break;

	case Z180_RLDR1L:
		LOG(("Z180 '%s' RLDR1L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RLDR1L_WMASK));
		IO_RLDR1L = (IO_RLDR1L & ~Z180_RLDR1L_WMASK) | (data & Z180_RLDR1L_WMASK);
		break;

	case Z180_RLDR1H:
		LOG(("Z180 '%s' RLDR1H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RLDR1H_WMASK));
		IO_RLDR1H = (IO_RLDR1H & ~Z180_RLDR1H_WMASK) | (data & Z180_RLDR1H_WMASK);
		break;

	case Z180_FRC:
		LOG(("Z180 '%s' FRC    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_FRC_WMASK));
		IO_FRC = (IO_FRC & ~Z180_FRC_WMASK) | (data & Z180_FRC_WMASK);
		break;

	case Z180_IO19:
		LOG(("Z180 '%s' IO19   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO19_WMASK));
		IO_IO19 = (IO_IO19 & ~Z180_IO19_WMASK) | (data & Z180_IO19_WMASK);
		break;

	case Z180_ASTC0L:
		LOG(("Z180 '%s' ASTC0L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASTC0L_WMASK));
		IO_ASTC0L = (IO_ASTC0L & ~Z180_ASTC0L_WMASK) | (data & Z180_ASTC0L_WMASK);
		break;

	case Z180_ASTC0H:
		LOG(("Z180 '%s' ASTC0H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASTC0H_WMASK));
		IO_ASTC0H = (IO_ASTC0H & ~Z180_ASTC0H_WMASK) | (data & Z180_ASTC0H_WMASK);
		break;

	case Z180_ASTC1L:
		LOG(("Z180 '%s' ASTC1L wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASTC1L_WMASK));
		IO_ASTC1L = (IO_ASTC1L & ~Z180_ASTC1L_WMASK) | (data & Z180_ASTC1L_WMASK);
		break;

	case Z180_ASTC1H:
		LOG(("Z180 '%s' ASTC1H wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ASTC1H_WMASK));
		IO_ASTC1H = (IO_ASTC1H & ~Z180_ASTC1H_WMASK) | (data & Z180_ASTC1H_WMASK);
		break;

	case Z180_CMR:
		LOG(("Z180 '%s' CMR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CMR_WMASK));
		IO_CMR = (IO_CMR & ~Z180_CMR_WMASK) | (data & Z180_CMR_WMASK);
		break;

	case Z180_CCR:
		LOG(("Z180 '%s' CCR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CCR_WMASK));
		IO_CCR = (IO_CCR & ~Z180_CCR_WMASK) | (data & Z180_CCR_WMASK);
		break;

	case Z180_SAR0L:
		LOG(("Z180 '%s' SAR0L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_SAR0L_WMASK));
		IO_SAR0L = (IO_SAR0L & ~Z180_SAR0L_WMASK) | (data & Z180_SAR0L_WMASK);
		break;

	case Z180_SAR0H:
		LOG(("Z180 '%s' SAR0H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_SAR0H_WMASK));
		IO_SAR0H = (IO_SAR0H & ~Z180_SAR0H_WMASK) | (data & Z180_SAR0H_WMASK);
		break;

	case Z180_SAR0B:
		LOG(("Z180 '%s' SAR0B  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_SAR0B_WMASK));
		IO_SAR0B = (IO_SAR0B & ~Z180_SAR0B_WMASK) | (data & Z180_SAR0B_WMASK);
		break;

	case Z180_DAR0L:
		LOG(("Z180 '%s' DAR0L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DAR0L_WMASK));
		IO_DAR0L = (IO_DAR0L & ~Z180_DAR0L_WMASK) | (data & Z180_DAR0L_WMASK);
		break;

	case Z180_DAR0H:
		LOG(("Z180 '%s' DAR0H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DAR0H_WMASK));
		IO_DAR0H = (IO_DAR0H & ~Z180_DAR0H_WMASK) | (data & Z180_DAR0H_WMASK);
		break;

	case Z180_DAR0B:
		LOG(("Z180 '%s' DAR0B  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DAR0B_WMASK));
		IO_DAR0B = (IO_DAR0B & ~Z180_DAR0B_WMASK) | (data & Z180_DAR0B_WMASK);
		break;

	case Z180_BCR0L:
		LOG(("Z180 '%s' BCR0L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_BCR0L_WMASK));
		IO_BCR0L = (IO_BCR0L & ~Z180_BCR0L_WMASK) | (data & Z180_BCR0L_WMASK);
		break;

	case Z180_BCR0H:
		LOG(("Z180 '%s' BCR0H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_BCR0H_WMASK));
		IO_BCR0H = (IO_BCR0H & ~Z180_BCR0H_WMASK) | (data & Z180_BCR0H_WMASK);
		break;

	case Z180_MAR1L:
		LOG(("Z180 '%s' MAR1L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_MAR1L_WMASK));
		IO_MAR1L = (IO_MAR1L & ~Z180_MAR1L_WMASK) | (data & Z180_MAR1L_WMASK);
		break;

	case Z180_MAR1H:
		LOG(("Z180 '%s' MAR1H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_MAR1H_WMASK));
		IO_MAR1H = (IO_MAR1H & ~Z180_MAR1H_WMASK) | (data & Z180_MAR1H_WMASK);
		break;

	case Z180_MAR1B:
		LOG(("Z180 '%s' MAR1B  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_MAR1B_WMASK));
		IO_MAR1B = (IO_MAR1B & ~Z180_MAR1B_WMASK) | (data & Z180_MAR1B_WMASK);
		break;

	case Z180_IAR1L:
		LOG(("Z180 '%s' IAR1L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IAR1L_WMASK));
		IO_IAR1L = (IO_IAR1L & ~Z180_IAR1L_WMASK) | (data & Z180_IAR1L_WMASK);
		break;

	case Z180_IAR1H:
		LOG(("Z180 '%s' IAR1H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IAR1H_WMASK));
		IO_IAR1H = (IO_IAR1H & ~Z180_IAR1H_WMASK) | (data & Z180_IAR1H_WMASK);
		break;

	case Z180_IAR1B:
		LOG(("Z180 '%s' IAR1B  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IAR1B_WMASK));
		IO_IAR1B = (IO_IAR1B & ~Z180_IAR1B_WMASK) | (data & Z180_IAR1B_WMASK);
		break;

	case Z180_BCR1L:
		LOG(("Z180 '%s' BCR1L  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_BCR1L_WMASK));
		IO_BCR1L = (IO_BCR1L & ~Z180_BCR1L_WMASK) | (data & Z180_BCR1L_WMASK);
		break;

	case Z180_BCR1H:
		LOG(("Z180 '%s' BCR1H  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_BCR1H_WMASK));
		IO_BCR1H = (IO_BCR1H & ~Z180_BCR1H_WMASK) | (data & Z180_BCR1H_WMASK);
		break;

	case Z180_DSTAT:
		LOG(("Z180 '%s' DSTAT  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DSTAT_WMASK));
		IO_DSTAT = (IO_DSTAT & ~Z180_DSTAT_WMASK) | (data & Z180_DSTAT_WMASK);
		if ((data & (Z180_DSTAT_DE1 | Z180_DSTAT_DWE1)) == Z180_DSTAT_DE1)
			IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		if ((data & (Z180_DSTAT_DE0 | Z180_DSTAT_DWE0)) == Z180_DSTAT_DE0)
			IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		break;

	case Z180_DMODE:
		LOG(("Z180 '%s' DMODE  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DMODE_WMASK));
		IO_DMODE = (IO_DMODE & ~Z180_DMODE_WMASK) | (data & Z180_DMODE_WMASK);
		break;

	case Z180_DCNTL:
		LOG(("Z180 '%s' DCNTL  wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_DCNTL_WMASK));
		IO_DCNTL = (IO_DCNTL & ~Z180_DCNTL_WMASK) | (data & Z180_DCNTL_WMASK);
		break;

	case Z180_IL:
		LOG(("Z180 '%s' IL     wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IL_WMASK));
		IO_IL = (IO_IL & ~Z180_IL_WMASK) | (data & Z180_IL_WMASK);
		break;

	case Z180_ITC:
		LOG(("Z180 '%s' ITC    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_ITC_WMASK));
		IO_ITC = (IO_ITC & ~Z180_ITC_WMASK) | (data & Z180_ITC_WMASK);
		break;

	case Z180_IO35:
		LOG(("Z180 '%s' IO35   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO35_WMASK));
		IO_IO35 = (IO_IO35 & ~Z180_IO35_WMASK) | (data & Z180_IO35_WMASK);
		break;

	case Z180_RCR:
		LOG(("Z180 '%s' RCR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_RCR_WMASK));
		IO_RCR = (IO_RCR & ~Z180_RCR_WMASK) | (data & Z180_RCR_WMASK);
		break;

	case Z180_IO37:
		LOG(("Z180 '%s' IO37   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO37_WMASK));
		IO_IO37 = (IO_IO37 & ~Z180_IO37_WMASK) | (data & Z180_IO37_WMASK);
		break;

	case Z180_CBR:
		LOG(("Z180 '%s' CBR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CBR_WMASK));
		IO_CBR = (IO_CBR & ~Z180_CBR_WMASK) | (data & Z180_CBR_WMASK);
		z180_mmu();
		break;

	case Z180_BBR:
		LOG(("Z180 '%s' BBR    wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_BBR_WMASK));
		IO_BBR = (IO_BBR & ~Z180_BBR_WMASK) | (data & Z180_BBR_WMASK);
		z180_mmu();
		break;

	case Z180_CBAR:
		LOG(("Z180 '%s' CBAR   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_CBAR_WMASK));
		IO_CBAR = (IO_CBAR & ~Z180_CBAR_WMASK) | (data & Z180_CBAR_WMASK);
		z180_mmu();
		break;

	case Z180_IO3B:
		LOG(("Z180 '%s' IO3B   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO3B_WMASK));
		IO_IO3B = (IO_IO3B & ~Z180_IO3B_WMASK) | (data & Z180_IO3B_WMASK);
		break;

	case Z180_IO3C:
		LOG(("Z180 '%s' IO3C   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO3C_WMASK));
		IO_IO3C = (IO_IO3C & ~Z180_IO3C_WMASK) | (data & Z180_IO3C_WMASK);
		break;

	case Z180_IO3D:
		LOG(("Z180 '%s' IO3D   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IO3D_WMASK));
		IO_IO3D = (IO_IO3D & ~Z180_IO3D_WMASK) | (data & Z180_IO3D_WMASK);
		break;

	case Z180_OMCR:
		LOG(("Z180 '%s' OMCR   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_OMCR_WMASK));
		IO_OMCR = (IO_OMCR & ~Z180_OMCR_WMASK) | (data & Z180_OMCR_WMASK);
		break;

	case Z180_IOCR:
		LOG(("Z180 '%s' IOCR   wr $%02x ($%02x)\n", Z180.device->tag, data,  data & Z180_IOCR_WMASK));
		IO_IOCR = (IO_IOCR & ~Z180_IOCR_WMASK) | (data & Z180_IOCR_WMASK);
		break;
	}
}

static void z180_dma0(void)
{
	offs_t sar0 = 65536 * IO_SAR0B + 256 * IO_SAR0H + IO_SAR0L;
	offs_t dar0 = 65536 * IO_DAR0B + 256 * IO_DAR0H + IO_DAR0L;
	int bcr0 = 256 * IO_BCR0H + IO_BCR0L;
	int count = (IO_DMODE & Z180_DMODE_MMOD) ? bcr0 : 1;

	if (bcr0 == 0)
	{
		IO_DSTAT &= ~Z180_DSTAT_DE0;
		return;
	}

	while (count-- > 0)
	{
		/* last transfer happening now? */
		if (bcr0 == 1)
		{
			Z180.iol |= Z180_TEND0;
		}
		switch( IO_DMODE & (Z180_DMODE_SM | Z180_DMODE_DM) )
		{
		case 0x00:	/* memory SAR0+1 to memory DAR0+1 */
			memory_write_byte_8le(Z180.program, dar0++, memory_read_byte_8le(Z180.program, sar0++));
			break;
		case 0x04:	/* memory SAR0-1 to memory DAR0+1 */
			memory_write_byte_8le(Z180.program, dar0++, memory_read_byte_8le(Z180.program, sar0--));
			break;
		case 0x08:	/* memory SAR0 fixed to memory DAR0+1 */
			memory_write_byte_8le(Z180.program, dar0++, memory_read_byte_8le(Z180.program, sar0));
			break;
		case 0x0c:	/* I/O SAR0 fixed to memory DAR0+1 */
			if (Z180.iol & Z180_DREQ0)
			{
				memory_write_byte_8le(Z180.program, dar0++, IN(sar0));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					Z180.iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x10:	/* memory SAR0+1 to memory DAR0-1 */
			memory_write_byte_8le(Z180.program, dar0--, memory_read_byte_8le(Z180.program, sar0++));
			break;
		case 0x14:	/* memory SAR0-1 to memory DAR0-1 */
			memory_write_byte_8le(Z180.program, dar0--, memory_read_byte_8le(Z180.program, sar0--));
			break;
		case 0x18:	/* memory SAR0 fixed to memory DAR0-1 */
			memory_write_byte_8le(Z180.program, dar0--, memory_read_byte_8le(Z180.program, sar0));
			break;
		case 0x1c:	/* I/O SAR0 fixed to memory DAR0-1 */
			if (Z180.iol & Z180_DREQ0)
            {
				memory_write_byte_8le(Z180.program, dar0--, IN(sar0));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					Z180.iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x20:	/* memory SAR0+1 to memory DAR0 fixed */
			memory_write_byte_8le(Z180.program, dar0, memory_read_byte_8le(Z180.program, sar0++));
			break;
		case 0x24:	/* memory SAR0-1 to memory DAR0 fixed */
			memory_write_byte_8le(Z180.program, dar0, memory_read_byte_8le(Z180.program, sar0--));
			break;
		case 0x28:	/* reserved */
			break;
		case 0x2c:	/* reserved */
			break;
		case 0x30:	/* memory SAR0+1 to I/O DAR0 fixed */
			if (Z180.iol & Z180_DREQ0)
            {
				OUT(dar0, memory_read_byte_8le(Z180.program, sar0++));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					Z180.iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x34:	/* memory SAR0-1 to I/O DAR0 fixed */
			if (Z180.iol & Z180_DREQ0)
            {
				OUT(dar0, memory_read_byte_8le(Z180.program, sar0--));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					Z180.iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x38:	/* reserved */
			break;
		case 0x3c:	/* reserved */
			break;
		}
		bcr0--;
		count--;
		if ((z180_icount -= 6) < 0)
			break;
	}

	IO_SAR0L = sar0;
	IO_SAR0H = sar0 >> 8;
	IO_SAR0B = sar0 >> 16;
	IO_DAR0L = dar0;
	IO_DAR0H = dar0 >> 8;
	IO_DAR0B = dar0 >> 16;
	IO_BCR0L = bcr0;
	IO_BCR0H = bcr0 >> 8;

	/* DMA terminal count? */
	if (bcr0 == 0)
	{
		Z180.iol &= ~Z180_TEND0;
		IO_DSTAT &= ~Z180_DSTAT_DE0;
		/* terminal count interrupt enabled? */
		if (IO_DSTAT & Z180_DSTAT_DIE0 && _IFF1)
			take_interrupt(Z180_INT_DMA0);
	}
}

static void z180_dma1(void)
{
	offs_t mar1 = 65536 * IO_MAR1B + 256 * IO_MAR1H + IO_MAR1L;
	offs_t iar1 = 256 * IO_IAR1H + IO_IAR1L;
	int bcr1 = 256 * IO_BCR1H + IO_BCR1L;

	if ((Z180.iol & Z180_DREQ1) == 0)
		return;

	/* counter is zero? */
	if (bcr1 == 0)
	{
		IO_DSTAT &= ~Z180_DSTAT_DE1;
		return;
	}

	/* last transfer happening now? */
	if (bcr1 == 1)
	{
		Z180.iol |= Z180_TEND1;
	}

	switch (IO_DCNTL & (Z180_DCNTL_DIM1 | Z180_DCNTL_DIM0))
	{
	case 0x00:	/* memory MAR1+1 to I/O IAR1 fixed */
		memory_write_byte_8le(Z180.iospace, iar1, memory_read_byte_8le(Z180.program, mar1++));
		break;
	case 0x01:	/* memory MAR1-1 to I/O IAR1 fixed */
		memory_write_byte_8le(Z180.iospace, iar1, memory_read_byte_8le(Z180.program, mar1--));
		break;
	case 0x02:	/* I/O IAR1 fixed to memory MAR1+1 */
		memory_write_byte_8le(Z180.program, mar1++, memory_read_byte_8le(Z180.iospace, iar1));
		break;
	case 0x03:	/* I/O IAR1 fixed to memory MAR1-1 */
		memory_write_byte_8le(Z180.program, mar1--, memory_read_byte_8le(Z180.iospace, iar1));
		break;
	}

	/* edge sensitive DREQ1 ? */
	if (IO_DCNTL & Z180_DCNTL_DIM1)
		Z180.iol &= ~Z180_DREQ1;

	IO_MAR1L = mar1;
	IO_MAR1H = mar1 >> 8;
	IO_MAR1B = mar1 >> 16;
	IO_BCR1L = bcr1;
	IO_BCR1H = bcr1 >> 8;

	/* DMA terminal count? */
	if (bcr1 == 0)
	{
		Z180.iol &= ~Z180_TEND1;
		IO_DSTAT &= ~Z180_DSTAT_DE1;
		if (IO_DSTAT & Z180_DSTAT_DIE1 && _IFF1)
			take_interrupt(Z180_INT_DMA1);
	}

	/* six cycles per transfer (minimum) */
	z180_icount -= 6;
}

static void z180_write_iolines(UINT32 data)
{
	UINT32 changes = Z180.iol ^ data;

    /* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
	if (changes & Z180_CKA0)
	{
		LOG(("Z180 '%s' CKA0   %d\n", Z180.device->tag, data & Z180_CKA0 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_CKA0) | (data & Z180_CKA0);
    }

    /* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
	if (changes & Z180_CKA1)
	{
		LOG(("Z180 '%s' CKA1   %d\n", Z180.device->tag, data & Z180_CKA1 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_CKA1) | (data & Z180_CKA1);
    }

    /* I/O serial clock (active high) */
	if (changes & Z180_CKS)
	{
		LOG(("Z180 '%s' CKS    %d\n", Z180.device->tag, data & Z180_CKS ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_CKS) | (data & Z180_CKS);
    }

    /* I   clear to send 0 (active low) */
	if (changes & Z180_CTS0)
	{
		LOG(("Z180 '%s' CTS0   %d\n", Z180.device->tag, data & Z180_CTS0 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_CTS0) | (data & Z180_CTS0);
    }

    /* I   clear to send 1 (active low) or RXS (mux) */
	if (changes & Z180_CTS1)
	{
		LOG(("Z180 '%s' CTS1   %d\n", Z180.device->tag, data & Z180_CTS1 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_CTS1) | (data & Z180_CTS1);
    }

    /* I   data carrier detect (active low) */
	if (changes & Z180_DCD0)
	{
		LOG(("Z180 '%s' DCD0   %d\n", Z180.device->tag, data & Z180_DCD0 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_DCD0) | (data & Z180_DCD0);
    }

    /* I   data request DMA ch 0 (active low) or CKA0 (mux) */
	if (changes & Z180_DREQ0)
	{
		LOG(("Z180 '%s' DREQ0  %d\n", Z180.device->tag, data & Z180_DREQ0 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_DREQ0) | (data & Z180_DREQ0);
    }

    /* I   data request DMA ch 1 (active low) */
	if (changes & Z180_DREQ1)
	{
		LOG(("Z180 '%s' DREQ1  %d\n", Z180.device->tag, data & Z180_DREQ1 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_DREQ1) | (data & Z180_DREQ1);
    }

    /* I   asynchronous receive data 0 (active high) */
	if (changes & Z180_RXA0)
	{
		LOG(("Z180 '%s' RXA0   %d\n", Z180.device->tag, data & Z180_RXA0 ? 1 : 0));
        Z180.iol = (Z180.iol & ~Z180_RXA0) | (data & Z180_RXA0);
    }

    /* I   asynchronous receive data 1 (active high) */
	if (changes & Z180_RXA1)
	{
		LOG(("Z180 '%s' RXA1   %d\n", Z180.device->tag, data & Z180_RXA1 ? 1 : 0));
		Z180.iol = (Z180.iol & ~Z180_RXA1) | (data & Z180_RXA1);
    }

    /* I   clocked serial receive data (active high) or CTS1 (mux) */
	if (changes & Z180_RXS)
	{
		LOG(("Z180 '%s' RXS    %d\n", Z180.device->tag, data & Z180_RXS ? 1 : 0));
        Z180.iol = (Z180.iol & ~Z180_RXS) | (data & Z180_RXS);
    }

    /*   O request to send (active low) */
	if (changes & Z180_RTS0)
	{
		LOG(("Z180 '%s' RTS0   won't change output\n", Z180.device->tag));
    }

    /*   O transfer end 0 (active low) or CKA1 (mux) */
	if (changes & Z180_TEND0)
	{
		LOG(("Z180 '%s' TEND0  won't change output\n", Z180.device->tag));
    }

    /*   O transfer end 1 (active low) */
	if (changes & Z180_TEND1)
	{
		LOG(("Z180 '%s' TEND1  won't change output\n", Z180.device->tag));
    }

    /*   O transfer out (PRT channel, active low) or A18 (mux) */
	if (changes & Z180_A18_TOUT)
	{
		LOG(("Z180 '%s' TOUT   won't change output\n", Z180.device->tag));
    }

    /*   O asynchronous transmit data 0 (active high) */
	if (changes & Z180_TXA0)
	{
		LOG(("Z180 '%s' TXA0   won't change output\n", Z180.device->tag));
    }

    /*   O asynchronous transmit data 1 (active high) */
	if (changes & Z180_TXA1)
	{
		LOG(("Z180 '%s' TXA1   won't change output\n", Z180.device->tag));
    }

    /*   O clocked serial transmit data (active high) */
	if (changes & Z180_TXS)
	{
		LOG(("Z180 '%s' TXS    won't change output\n", Z180.device->tag));
    }
}


static CPU_INIT( z180 )
{
	Z180.daisy = NULL;
	if (device->static_config)
		Z180.daisy = z80daisy_init(device, device->static_config);
	Z180.irq_callback = irqcallback;
	Z180.device = device;
	Z180.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	Z180.iospace = memory_find_address_space(device, ADDRESS_SPACE_IO);

	state_save_register_item("z180", device->tag, 0, Z180.AF.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.BC.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.DE.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.HL.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.IX.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.IY.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.PC.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.SP.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.AF2.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.BC2.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.DE2.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.HL2.w.l);
	state_save_register_item("z180", device->tag, 0, Z180.R);
	state_save_register_item("z180", device->tag, 0, Z180.R2);
	state_save_register_item("z180", device->tag, 0, Z180.IFF1);
	state_save_register_item("z180", device->tag, 0, Z180.IFF2);
	state_save_register_item("z180", device->tag, 0, Z180.HALT);
	state_save_register_item("z180", device->tag, 0, Z180.IM);
	state_save_register_item("z180", device->tag, 0, Z180.I);
	state_save_register_item("z180", device->tag, 0, Z180.nmi_state);
	state_save_register_item("z180", device->tag, 0, Z180.nmi_pending);
	state_save_register_item("z180", device->tag, 0, Z180.irq_state[0]);
	state_save_register_item("z180", device->tag, 0, Z180.irq_state[1]);
	state_save_register_item("z180", device->tag, 0, Z180.irq_state[2]);
	state_save_register_item("z180", device->tag, 0, Z180.after_EI);
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( z180 )
{
	z80_daisy_state *save_daisy;
	cpu_irq_callback save_irqcallback;
	int i, p;
#if BIG_FLAGS_ARRAY
	int oldval, newval, val;
	UINT8 *padd, *padc, *psub, *psbc;
	/* allocate big flag arrays once */
	SZHVC_add = (UINT8 *)auto_malloc(2*256*256);
	SZHVC_sub = (UINT8 *)auto_malloc(2*256*256);
	padd = &SZHVC_add[	0*256];
	padc = &SZHVC_add[256*256];
	psub = &SZHVC_sub[	0*256];
	psbc = &SZHVC_sub[256*256];
	for (oldval = 0; oldval < 256; oldval++)
	{
		for (newval = 0; newval < 256; newval++)
		{
			/* add or adc w/o carry set */
			val = newval - oldval;
			*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padd |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */

			if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
			if( newval < oldval ) *padd |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
			padd++;

			/* adc with carry set */
			val = newval - oldval - 1;
			*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
			if( newval <= oldval ) *padc |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
			padc++;

			/* cp, sub or sbc w/o carry set */
			val = oldval - newval;
			*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psub |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
			if( newval > oldval ) *psub |= CF;
			if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
			psub++;

			/* sbc with carry set */
			val = oldval - newval - 1;
			*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psbc |= (newval & (YF | XF));	/* undocumented flag bits 5+3 */
			if( (newval & 0x0f) >= (oldval & 0x0f) ) *psbc |= HF;
			if( newval >= oldval ) *psbc |= CF;
			if( (val^oldval) & (oldval^newval) & 0x80 ) *psbc |= VF;
			psbc++;
		}
	}
#endif
	for (i = 0; i < 256; i++)
	{
		p = 0;
		if( i&0x01 ) ++p;
		if( i&0x02 ) ++p;
		if( i&0x04 ) ++p;
		if( i&0x08 ) ++p;
		if( i&0x10 ) ++p;
		if( i&0x20 ) ++p;
		if( i&0x40 ) ++p;
		if( i&0x80 ) ++p;
		SZ[i] = i ? i & SF : ZF;
		SZ[i] |= (i & (YF | XF));		/* undocumented flag bits 5+3 */
		SZ_BIT[i] = i ? i & SF : ZF | PF;
		SZ_BIT[i] |= (i & (YF | XF));	/* undocumented flag bits 5+3 */
		SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
		SZHV_inc[i] = SZ[i];
		if( i == 0x80 ) SZHV_inc[i] |= VF;
		if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
		SZHV_dec[i] = SZ[i] | NF;
		if( i == 0x7f ) SZHV_dec[i] |= VF;
		if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
	}

	save_daisy = Z180.daisy;
	save_irqcallback = Z180.irq_callback;
	memset(&Z180, 0, sizeof(Z180));
	Z180.daisy = save_daisy;
	Z180.irq_callback = save_irqcallback;
	Z180.device = device;
	Z180.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	Z180.iospace = memory_find_address_space(device, ADDRESS_SPACE_IO);
	_IX = _IY = 0xffff; /* IX and IY are FFFF after a reset! */
	_F = ZF;			/* Zero flag is set */
	Z180.nmi_state = CLEAR_LINE;
	Z180.nmi_pending = 0;
	Z180.irq_state[0] = CLEAR_LINE;
	Z180.irq_state[1] = CLEAR_LINE;
	Z180.irq_state[2] = CLEAR_LINE;
	Z180.after_EI = 0;
	Z180.tif[0] = 0;
	Z180.tif[1] = 0;
	Z180.read_tcr_tmdr[0] = 0;
	Z180.read_tcr_tmdr[1] = 0;
	Z180.tmdr_value[0] = 0xffff;
	Z180.tmdr_value[1] = 0xffff;
	Z180.device = device;

	/* reset io registers */
	IO_CNTLA0  = Z180_CNTLA0_RESET;
	IO_CNTLA1  = Z180_CNTLA1_RESET;
	IO_CNTLB0  = Z180_CNTLB0_RESET;
	IO_CNTLB1  = Z180_CNTLB1_RESET;
	IO_STAT0   = Z180_STAT0_RESET;
	IO_STAT1   = Z180_STAT1_RESET;
	IO_TDR0    = Z180_TDR0_RESET;
	IO_TDR1    = Z180_TDR1_RESET;
	IO_RDR0    = Z180_RDR0_RESET;
	IO_RDR1    = Z180_RDR1_RESET;
	IO_CNTR    = Z180_CNTR_RESET;
	IO_TRDR    = Z180_TRDR_RESET;
	IO_TMDR0L  = Z180_TMDR0L_RESET;
	IO_TMDR0H  = Z180_TMDR0H_RESET;
	IO_RLDR0L  = Z180_RLDR0L_RESET;
	IO_RLDR0H  = Z180_RLDR0H_RESET;
	IO_TCR	   = Z180_TCR_RESET;
	IO_IO11    = Z180_IO11_RESET;
	IO_ASEXT0  = Z180_ASEXT0_RESET;
	IO_ASEXT1  = Z180_ASEXT1_RESET;
	IO_TMDR1L  = Z180_TMDR1L_RESET;
	IO_TMDR1H  = Z180_TMDR1H_RESET;
	IO_RLDR1L  = Z180_RLDR1L_RESET;
	IO_RLDR1H  = Z180_RLDR1H_RESET;
	IO_FRC	   = Z180_FRC_RESET;
	IO_IO19    = Z180_IO19_RESET;
	IO_ASTC0L  = Z180_ASTC0L_RESET;
	IO_ASTC0H  = Z180_ASTC0H_RESET;
	IO_ASTC1L  = Z180_ASTC1L_RESET;
	IO_ASTC1H  = Z180_ASTC1H_RESET;
	IO_CMR	   = Z180_CMR_RESET;
	IO_CCR	   = Z180_CCR_RESET;
	IO_SAR0L   = Z180_SAR0L_RESET;
	IO_SAR0H   = Z180_SAR0H_RESET;
	IO_SAR0B   = Z180_SAR0B_RESET;
	IO_DAR0L   = Z180_DAR0L_RESET;
	IO_DAR0H   = Z180_DAR0H_RESET;
	IO_DAR0B   = Z180_DAR0B_RESET;
	IO_BCR0L   = Z180_BCR0L_RESET;
	IO_BCR0H   = Z180_BCR0H_RESET;
	IO_MAR1L   = Z180_MAR1L_RESET;
	IO_MAR1H   = Z180_MAR1H_RESET;
	IO_MAR1B   = Z180_MAR1B_RESET;
	IO_IAR1L   = Z180_IAR1L_RESET;
	IO_IAR1H   = Z180_IAR1H_RESET;
	IO_IAR1B   = Z180_IAR1B_RESET;
	IO_BCR1L   = Z180_BCR1L_RESET;
	IO_BCR1H   = Z180_BCR1H_RESET;
	IO_DSTAT   = Z180_DSTAT_RESET;
	IO_DMODE   = Z180_DMODE_RESET;
	IO_DCNTL   = Z180_DCNTL_RESET;
	IO_IL	   = Z180_IL_RESET;
	IO_ITC	   = Z180_ITC_RESET;
	IO_IO35    = Z180_IO35_RESET;
	IO_RCR	   = Z180_RCR_RESET;
	IO_IO37    = Z180_IO37_RESET;
	IO_CBR	   = Z180_CBR_RESET;
	IO_BBR	   = Z180_BBR_RESET;
	IO_CBAR    = Z180_CBAR_RESET;
	IO_IO3B    = Z180_IO3B_RESET;
	IO_IO3C    = Z180_IO3C_RESET;
	IO_IO3D    = Z180_IO3D_RESET;
	IO_OMCR    = Z180_OMCR_RESET;
	IO_IOCR    = Z180_IOCR_RESET;

	if (Z180.daisy)
		z80daisy_reset(Z180.daisy);
	z180_mmu();
}

/* Handle PRT timers, decreasing them after 20 clocks and returning the new icount base that needs to be used for the next check */
static int handle_timers(int current_icount, int previous_icount)
{
	int diff = previous_icount - current_icount;
	int new_icount_base;

	if(diff >= 20)
	{
		/* Programmable Reload Timer 0 */
		if(IO_TCR & Z180_TCR_TDE0)
		{
			Z180.tmdr_value[0]--;
			if(Z180.tmdr_value[0] == 0)
			{
				Z180.tmdr_value[0] = IO_RLDR0L | (IO_RLDR0H << 8);
				Z180.tif[0] = 1;
			}
		}

		/* Programmable Reload Timer 1 */
		if(IO_TCR & Z180_TCR_TDE1)
		{
			Z180.tmdr_value[1]--;
			if(Z180.tmdr_value[1] == 0)
			{
				Z180.tmdr_value[1] = IO_RLDR1L | (IO_RLDR1H << 8);
				Z180.tif[1] = 1;
			}
		}

		if((IO_TCR & Z180_TCR_TIE0) && Z180.tif[0])
		{
			// check if we can take the interrupt
			if(_IFF1 && !Z180.after_EI)
			{
				take_interrupt(Z180_INT_PRT0);
			}
		}

		if((IO_TCR & Z180_TCR_TIE1) && Z180.tif[1])
		{
			// check if we can take the interrupt
			if(_IFF1 && !Z180.after_EI)
			{
				take_interrupt(Z180_INT_PRT1);
			}
		}

		new_icount_base = current_icount + (diff - 20);
	}
	else
	{
		new_icount_base = previous_icount;
	}

	return new_icount_base;
}

static void check_interrupts(void)
{
	int i;
	for(i = 0; i <= 2; i++)
	{
		/* check for IRQs before each instruction */
		if(Z180.irq_state[i] != CLEAR_LINE && _IFF1 && !Z180.after_EI)
		{
			take_interrupt(Z180_INT0 + i);
		}
	}
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
static CPU_EXECUTE( z180 )
{
	int old_icount = cycles;
	z180_icount = cycles;

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (Z180.nmi_pending)
	{
		LOG(("Z180 '%s' take NMI\n", Z180.device->tag));
		_PPC = -1;			/* there isn't a valid previous program counter */
		LEAVE_HALT; 		/* Check if processor was halted */

		/* disable DMA transfers!! */
		IO_DSTAT &= ~Z180_DSTAT_DME;

		_IFF2 = _IFF1;
		_IFF1 = 0;
		PUSH( PC );
		_PCD = 0x0066;
		z180_icount -= 11;
		Z180.nmi_pending = 0;
	}

again:
    /* check if any DMA transfer is running */
	if ((IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
	{
		/* check if DMA channel 0 is running and also is in burst mode */
		if ((IO_DSTAT & Z180_DSTAT_DE0) == Z180_DSTAT_DE0 &&
			(IO_DMODE & Z180_DMODE_MMOD) == Z180_DMODE_MMOD)
		{
			debugger_instruction_hook(device, _PCD);

			z180_dma0();
			old_icount = handle_timers(z180_icount, old_icount);
		}
		else
		{
			do
			{
				check_interrupts();
				Z180.after_EI = 0;

				_PPC = _PCD;
				debugger_instruction_hook(device, _PCD);
				_R++;

				EXEC_INLINE(op,ROP());
				old_icount = handle_timers(z180_icount, old_icount);

				z180_dma0();
				old_icount = handle_timers(z180_icount, old_icount);

				z180_dma1();
				old_icount = handle_timers(z180_icount, old_icount);

				/* If DMA is done break out to the faster loop */
				if ((IO_DSTAT & Z180_DSTAT_DME) != Z180_DSTAT_DME)
					break;
			} while( z180_icount > 0 );
		}
    }

    if (z180_icount > 0)
    {
        do
		{
			check_interrupts();
			Z180.after_EI = 0;

			_PPC = _PCD;
			debugger_instruction_hook(device, _PCD);
			_R++;
			EXEC_INLINE(op,ROP());
			old_icount = handle_timers(z180_icount, old_icount);

			/* If DMA is started go to check the mode */
			if ((IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
				goto again;
        } while( z180_icount > 0 );
	}

	return cycles - z180_icount;
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
static CPU_BURN( z180 )
{
	if( cycles > 0 )
	{
		/* NOP takes 3 cycles per instruction */
		int n = (cycles + 2) / 3;
		_R += n;
		z180_icount -= 3 * n;
	}
}

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
static CPU_GET_CONTEXT( z180 )
{
	if( dst )
		*(Z180_Regs*)dst = Z180;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static CPU_SET_CONTEXT( z180 )
{
	if( src )
		Z180 = *(Z180_Regs*)src;
}

#ifdef UNUSED_FUNCTION
READ8_HANDLER( z180_internal_r )
{
	return Z180.io[offset & 0x3f];
}

WRITE8_HANDLER( z180_internal_w )
{
	cpuinfo info;
	info.i = data;
	z180_set_info( CPUINFO_INT_REGISTER + Z180_CNTLA0 + (offset & 0x3f), &info );
}
#endif

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* mark an NMI pending on the rising edge */
		if (Z180.nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			Z180.nmi_pending = 1;
		Z180.nmi_state = state;
	}
	else
	{
		LOG(("Z180 '%s' set_irq_line %d = %d\n",Z180.device->tag , irqline,state));

		/* update the IRQ state */
		Z180.irq_state[irqline] = state;
		if (Z180.daisy)
			Z180.irq_state[0] = z80daisy_update_irq_state(Z180.daisy);

		/* the main execute loop will take the interrupt */
	}
}

/* logical to physical address translation */
static CPU_TRANSLATE( z180 )
{
	if (space == ADDRESS_SPACE_PROGRAM)
	{
		*address = MMU_REMAP_ADDR(*address);
	}
	return TRUE;
}

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( z180 )
{
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT0:		set_irq_line(Z180_INT0, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT1:		set_irq_line(Z180_INT1, info->i);		break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT2:		set_irq_line(Z180_INT2, info->i);		break;

		case CPUINFO_INT_PC:							_PC = info->i; 							break;
		case CPUINFO_INT_REGISTER + Z180_PC:			Z180.PC.w.l = info->i;					break;
		case CPUINFO_INT_SP:							_SP = info->i;							break;
		case CPUINFO_INT_REGISTER + Z180_SP:			Z180.SP.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_AF:			Z180.AF.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_BC:			Z180.BC.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_DE:			Z180.DE.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_HL:			Z180.HL.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_IX:			Z180.IX.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_IY:			Z180.IY.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_R:				Z180.R = info->i; Z180.R2 = info->i & 0x80;	break;
		case CPUINFO_INT_REGISTER + Z180_I:				Z180.I = info->i;						break;
		case CPUINFO_INT_REGISTER + Z180_AF2:			Z180.AF2.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_BC2:			Z180.BC2.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_DE2:			Z180.DE2.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_HL2:			Z180.HL2.w.l = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_IM:			Z180.IM = info->i;						break;
		case CPUINFO_INT_REGISTER + Z180_IFF1:			Z180.IFF1 = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_IFF2:			Z180.IFF2 = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_HALT:			Z180.HALT = info->i;					break;
		case CPUINFO_INT_REGISTER + Z180_CNTLA0:		Z180.io[0x00] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLA1:		Z180.io[0x01] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLB0:		Z180.io[0x02] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLB1:		Z180.io[0x03] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_STAT0:			Z180.io[0x04] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_STAT1:			Z180.io[0x05] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TDR0:			Z180.io[0x06] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TDR1:			Z180.io[0x07] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RDR0:			Z180.io[0x08] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RDR1:			Z180.io[0x09] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CNTR:			Z180.io[0x0a] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TRDR:			Z180.io[0x0b] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR0L:		Z180.io[0x0c] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR0H:		Z180.io[0x0d] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR0L:		Z180.io[0x0e] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR0H:		Z180.io[0x0f] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TCR:			Z180.io[0x10] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO11:			Z180.io[0x11] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASEXT0:		Z180.io[0x12] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASEXT1:		Z180.io[0x13] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR1L:		Z180.io[0x14] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR1H:		Z180.io[0x15] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR1L:		Z180.io[0x16] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR1H:		Z180.io[0x17] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_FRC:			Z180.io[0x18] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO19:			Z180.io[0x19] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC0L:		Z180.io[0x1a] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC0H:		Z180.io[0x1b] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC1L:		Z180.io[0x1c] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC1H:		Z180.io[0x1d] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CMR:			Z180.io[0x1e] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CCR:			Z180.io[0x1f] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0L:			Z180.io[0x20] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0H:			Z180.io[0x21] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0B:			Z180.io[0x22] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0L:			Z180.io[0x23] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0H:			Z180.io[0x24] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0B:			Z180.io[0x25] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_BCR0L:			Z180.io[0x26] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_BCR0H:			Z180.io[0x27] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1L:			Z180.io[0x28] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1H:			Z180.io[0x29] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1B:			Z180.io[0x2a] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1L:			Z180.io[0x2b] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1H:			Z180.io[0x2c] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1B:			Z180.io[0x2d] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_BCR1L:			Z180.io[0x2e] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_BCR1H:			Z180.io[0x2f] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DSTAT:			Z180.io[0x30] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DMODE:			Z180.io[0x31] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_DCNTL:			Z180.io[0x32] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IL:			Z180.io[0x33] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_ITC:			Z180.io[0x34] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO35:			Z180.io[0x35] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_RCR:			Z180.io[0x36] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO37:			Z180.io[0x37] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_CBR:			Z180.io[0x38] = info->i; z180_mmu();	break;
		case CPUINFO_INT_REGISTER + Z180_BBR:			Z180.io[0x39] = info->i; z180_mmu();	break;
		case CPUINFO_INT_REGISTER + Z180_CBAR:			Z180.io[0x3a] = info->i; z180_mmu();	break;
		case CPUINFO_INT_REGISTER + Z180_IO3B:			Z180.io[0x3b] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO3C:			Z180.io[0x3c] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IO3D:			Z180.io[0x3d] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_OMCR:			Z180.io[0x3e] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IOCR:			Z180.io[0x3f] = info->i;				break;
		case CPUINFO_INT_REGISTER + Z180_IOLINES:		z180_write_iolines(info->i);			break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_op: cc[Z180_TABLE_op] = info->p;			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_cb: cc[Z180_TABLE_cb] = info->p;			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ed: cc[Z180_TABLE_ed] = info->p;			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xy: cc[Z180_TABLE_xy] = info->p;			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xycb: cc[Z180_TABLE_xycb] = info->p;		break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ex: cc[Z180_TABLE_ex] = info->p;			break;
	}
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( z180 )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(Z180);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 20;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 16;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = Z180.nmi_state;				break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT0:		info->i = Z180.irq_state[0];			break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT1:		info->i = Z180.irq_state[1];			break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT2:		info->i = Z180.irq_state[2];			break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = Z180.PREPC.w.l;				break;

		case CPUINFO_INT_PC:							info->i = _PCD;							break;
		case CPUINFO_INT_REGISTER + Z180_PC:			info->i = Z180.PC.w.l;					break;
		case CPUINFO_INT_SP:							info->i = _SPD;							break;
		case CPUINFO_INT_REGISTER + Z180_SP:			info->i = Z180.SP.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_AF:			info->i = Z180.AF.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_BC:			info->i = Z180.BC.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_DE:			info->i = Z180.DE.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_HL:			info->i = Z180.HL.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_IX:			info->i = Z180.IX.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_IY:			info->i = Z180.IY.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_R:				info->i = (Z180.R & 0x7f) | (Z180.R2 & 0x80); break;
		case CPUINFO_INT_REGISTER + Z180_I:				info->i = Z180.I;						break;
		case CPUINFO_INT_REGISTER + Z180_AF2:			info->i = Z180.AF2.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_BC2:			info->i = Z180.BC2.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_DE2:			info->i = Z180.DE2.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_HL2:			info->i = Z180.HL2.w.l;					break;
		case CPUINFO_INT_REGISTER + Z180_IM:			info->i = Z180.IM;						break;
		case CPUINFO_INT_REGISTER + Z180_IFF1:			info->i = Z180.IFF1;					break;
		case CPUINFO_INT_REGISTER + Z180_IFF2:			info->i = Z180.IFF2;					break;
		case CPUINFO_INT_REGISTER + Z180_HALT:			info->i = Z180.HALT;					break;
		case CPUINFO_INT_REGISTER + Z180_CNTLA0:		info->i = Z180.io[0x00];				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLA1:		info->i = Z180.io[0x01];				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLB0:		info->i = Z180.io[0x02];				break;
		case CPUINFO_INT_REGISTER + Z180_CNTLB1:		info->i = Z180.io[0x03];				break;
		case CPUINFO_INT_REGISTER + Z180_STAT0:			info->i = Z180.io[0x04];				break;
		case CPUINFO_INT_REGISTER + Z180_STAT1:			info->i = Z180.io[0x05];				break;
		case CPUINFO_INT_REGISTER + Z180_TDR0:			info->i = Z180.io[0x06];				break;
		case CPUINFO_INT_REGISTER + Z180_TDR1:			info->i = Z180.io[0x07];				break;
		case CPUINFO_INT_REGISTER + Z180_RDR0:			info->i = Z180.io[0x08];				break;
		case CPUINFO_INT_REGISTER + Z180_RDR1:			info->i = Z180.io[0x09];				break;
		case CPUINFO_INT_REGISTER + Z180_CNTR:			info->i = Z180.io[0x0a];				break;
		case CPUINFO_INT_REGISTER + Z180_TRDR:			info->i = Z180.io[0x0b];				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR0L:		info->i = Z180.io[0x0c];				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR0H:		info->i = Z180.io[0x0d];				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR0L:		info->i = Z180.io[0x0e];				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR0H:		info->i = Z180.io[0x0f];				break;
		case CPUINFO_INT_REGISTER + Z180_TCR:			info->i = Z180.io[0x10];				break;
		case CPUINFO_INT_REGISTER + Z180_IO11:			info->i = Z180.io[0x11];				break;
		case CPUINFO_INT_REGISTER + Z180_ASEXT0:		info->i = Z180.io[0x12];				break;
		case CPUINFO_INT_REGISTER + Z180_ASEXT1:		info->i = Z180.io[0x13];				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR1L:		info->i = Z180.io[0x14];				break;
		case CPUINFO_INT_REGISTER + Z180_TMDR1H:		info->i = Z180.io[0x15];				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR1L:		info->i = Z180.io[0x16];				break;
		case CPUINFO_INT_REGISTER + Z180_RLDR1H:		info->i = Z180.io[0x17];				break;
		case CPUINFO_INT_REGISTER + Z180_FRC:			info->i = Z180.io[0x18];				break;
		case CPUINFO_INT_REGISTER + Z180_IO19:			info->i = Z180.io[0x19];				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC0L:		info->i = Z180.io[0x1a];				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC0H:		info->i = Z180.io[0x1b];				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC1L:		info->i = Z180.io[0x1c];				break;
		case CPUINFO_INT_REGISTER + Z180_ASTC1H:		info->i = Z180.io[0x1d];				break;
		case CPUINFO_INT_REGISTER + Z180_CMR:			info->i = Z180.io[0x1e];				break;
		case CPUINFO_INT_REGISTER + Z180_CCR:			info->i = Z180.io[0x1f];				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0L:			info->i = Z180.io[0x20];				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0H:			info->i = Z180.io[0x21];				break;
		case CPUINFO_INT_REGISTER + Z180_SAR0B:			info->i = Z180.io[0x22];				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0L:			info->i = Z180.io[0x23];				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0H:			info->i = Z180.io[0x24];				break;
		case CPUINFO_INT_REGISTER + Z180_DAR0B:			info->i = Z180.io[0x25];				break;
		case CPUINFO_INT_REGISTER + Z180_BCR0L:			info->i = Z180.io[0x26];				break;
		case CPUINFO_INT_REGISTER + Z180_BCR0H:			info->i = Z180.io[0x27];				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1L:			info->i = Z180.io[0x28];				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1H:			info->i = Z180.io[0x29];				break;
		case CPUINFO_INT_REGISTER + Z180_MAR1B:			info->i = Z180.io[0x2a];				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1L:			info->i = Z180.io[0x2b];				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1H:			info->i = Z180.io[0x2c];				break;
		case CPUINFO_INT_REGISTER + Z180_IAR1B:			info->i = Z180.io[0x2d];				break;
		case CPUINFO_INT_REGISTER + Z180_BCR1L:			info->i = Z180.io[0x2e];				break;
		case CPUINFO_INT_REGISTER + Z180_BCR1H:			info->i = Z180.io[0x2f];				break;
		case CPUINFO_INT_REGISTER + Z180_DSTAT:			info->i = Z180.io[0x30];				break;
		case CPUINFO_INT_REGISTER + Z180_DMODE:			info->i = Z180.io[0x31];				break;
		case CPUINFO_INT_REGISTER + Z180_DCNTL:			info->i = Z180.io[0x32];				break;
		case CPUINFO_INT_REGISTER + Z180_IL:			info->i = Z180.io[0x33];				break;
		case CPUINFO_INT_REGISTER + Z180_ITC:			info->i = Z180.io[0x34];				break;
		case CPUINFO_INT_REGISTER + Z180_IO35:			info->i = Z180.io[0x35];				break;
		case CPUINFO_INT_REGISTER + Z180_RCR:			info->i = Z180.io[0x36];				break;
		case CPUINFO_INT_REGISTER + Z180_IO37:			info->i = Z180.io[0x37];				break;
		case CPUINFO_INT_REGISTER + Z180_CBR:			info->i = Z180.io[0x38];				break;
		case CPUINFO_INT_REGISTER + Z180_BBR:			info->i = Z180.io[0x39];				break;
		case CPUINFO_INT_REGISTER + Z180_CBAR:			info->i = Z180.io[0x3a];				break;
		case CPUINFO_INT_REGISTER + Z180_IO3B:			info->i = Z180.io[0x3b];				break;
		case CPUINFO_INT_REGISTER + Z180_IO3C:			info->i = Z180.io[0x3c];				break;
		case CPUINFO_INT_REGISTER + Z180_IO3D:			info->i = Z180.io[0x3d];				break;
		case CPUINFO_INT_REGISTER + Z180_OMCR:			info->i = Z180.io[0x3e];				break;
		case CPUINFO_INT_REGISTER + Z180_IOCR:			info->i = Z180.io[0x3f];				break;
		case CPUINFO_INT_REGISTER + Z180_IOLINES:		info->i = Z180.iol;						break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(z180);			break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(z180);	break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(z180);	break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(z180);					break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(z180);				break;
		case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(z180);			break;
		case CPUINFO_PTR_BURN:							info->burn = CPU_BURN_NAME(z180);					break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(z180);			break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &z180_icount;			break;
		case CPUINFO_PTR_TRANSLATE:						info->translate = CPU_TRANSLATE_NAME(z180);		break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_op: info->p = (void *)cc[Z180_TABLE_op];			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_cb: info->p = (void *)cc[Z180_TABLE_cb];			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ed: info->p = (void *)cc[Z180_TABLE_ed];			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xy: info->p = (void *)cc[Z180_TABLE_xy];			break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xycb: info->p = (void *)cc[Z180_TABLE_xycb];		break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ex: info->p = (void *)cc[Z180_TABLE_ex];			break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "Z180");				break;
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "Zilog Z8x180");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.4");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				Z180.AF.b.l & 0x80 ? 'S':'.',
				Z180.AF.b.l & 0x40 ? 'Z':'.',
				Z180.AF.b.l & 0x20 ? '5':'.',
				Z180.AF.b.l & 0x10 ? 'H':'.',
				Z180.AF.b.l & 0x08 ? '3':'.',
				Z180.AF.b.l & 0x04 ? 'P':'.',
				Z180.AF.b.l & 0x02 ? 'N':'.',
				Z180.AF.b.l & 0x01 ? 'C':'.');
			break;

		case CPUINFO_STR_REGISTER + Z180_PC:			sprintf(info->s, "PC:%04X", Z180.PC.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_SP:			sprintf(info->s, "SP:%04X", Z180.SP.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_AF:			sprintf(info->s, "AF:%04X", Z180.AF.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_BC:			sprintf(info->s, "BC:%04X", Z180.BC.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_DE:			sprintf(info->s, "DE:%04X", Z180.DE.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_HL:			sprintf(info->s, "HL:%04X", Z180.HL.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_IX:			sprintf(info->s, "IX:%04X", Z180.IX.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_IY:			sprintf(info->s, "IY:%04X", Z180.IY.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_R: 			sprintf(info->s, "R   :%02X", (Z180.R & 0x7f) | (Z180.R2 & 0x80)); break;
		case CPUINFO_STR_REGISTER + Z180_I: 			sprintf(info->s, "I   :%02X", Z180.I); break;
		case CPUINFO_STR_REGISTER + Z180_IL:			sprintf(info->s, "IL  :%02X", Z180.io[Z180_IL-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_AF2:			sprintf(info->s, "AF2:%04X", Z180.AF2.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_BC2:			sprintf(info->s, "BC2:%04X", Z180.BC2.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_DE2:			sprintf(info->s, "DE2:%04X", Z180.DE2.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_HL2:			sprintf(info->s, "HL2:%04X", Z180.HL2.w.l); break;
		case CPUINFO_STR_REGISTER + Z180_IM:			sprintf(info->s, "IM  :%X", Z180.IM); break;
		case CPUINFO_STR_REGISTER + Z180_IFF1:			sprintf(info->s, "IFF1:%X", Z180.IFF1); break;
		case CPUINFO_STR_REGISTER + Z180_IFF2:			sprintf(info->s, "IFF2:%X", Z180.IFF2); break;
		case CPUINFO_STR_REGISTER + Z180_HALT:			sprintf(info->s, "HALT:%X", Z180.HALT); break;
		case CPUINFO_STR_REGISTER + Z180_CCR: 			sprintf(info->s, "CCR :%02X", Z180.io[Z180_CCR-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_ITC: 			sprintf(info->s, "ITC :%02X", Z180.io[Z180_ITC-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_CBR: 			sprintf(info->s, "CBR :%02X", Z180.io[Z180_CBR-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_BBR: 			sprintf(info->s, "BBR :%02X", Z180.io[Z180_BBR-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_CBAR:			sprintf(info->s, "CBAR:%02X", Z180.io[Z180_CBAR-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_OMCR:			sprintf(info->s, "OMCR:%02X", Z180.io[Z180_OMCR-Z180_CNTLA0]); break;
		case CPUINFO_STR_REGISTER + Z180_IOCR:			sprintf(info->s, "IOCR:%02X", Z180.io[Z180_IOCR-Z180_CNTLA0]); break;
	}
}
