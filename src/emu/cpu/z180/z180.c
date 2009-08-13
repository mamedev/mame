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

/****************************************************************************/
/* The Z180 registers. HALT is set to 1 when the CPU is halted, the refresh */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/
typedef struct _z180_state z180_state;
struct _z180_state
{
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
	UINT32	ea;
	z80_daisy_state *daisy;
	cpu_irq_callback irq_callback;
	const device_config *device;
	const address_space *program;
	const address_space *iospace;
	cpu_state_table state;
	UINT8	rtemp;
	UINT32	ioltemp;
	int icount;
};

INLINE z180_state *get_safe_token(const device_config *device)
{
	assert(device != NULL);
	assert(device->token != NULL);
	assert(device->type == CPU);
	assert(cpu_get_type(device) == CPU_Z180);
	return (z180_state *)device->token;
}

static void set_irq_line(z180_state *cpustate, int irqline, int state);

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

#define _PPC	PREPC.d	/* previous program counter */

#define _PCD	PC.d
#define _PC 	PC.w.l

#define _SPD	SP.d
#define _SP 	SP.w.l

#define _AFD	AF.d
#define _AF 	AF.w.l
#define _A		AF.b.h
#define _F		AF.b.l

#define _BCD	BC.d
#define _BC 	BC.w.l
#define _B		BC.b.h
#define _C		BC.b.l

#define _DED	DE.d
#define _DE 	DE.w.l
#define _D		DE.b.h
#define _E		DE.b.l

#define _HLD	HL.d
#define _HL 	HL.w.l
#define _H		HL.b.h
#define _L		HL.b.l

#define _IXD	IX.d
#define _IX 	IX.w.l
#define _HX 	IX.b.h
#define _LX 	IX.b.l

#define _IYD	IY.d
#define _IY 	IY.w.l
#define _HY 	IY.b.h
#define _LY 	IY.b.l

#define IO(n)		io[(n)-Z180_CNTLA0]
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


/***************************************************************************
    CPU STATE DESCRIPTION
***************************************************************************/

#define Z180_STATE_ENTRY(_name, _format, _member, _datamask, _flags) \
	CPU_STATE_ENTRY(Z180_##_name, #_name, _format, z180_state, _member, _datamask, ~0, _flags)

#define Z180_STATE_IO_ENTRY(_name, _flags) \
	CPU_STATE_ENTRY(Z180_##_name, #_name, "%02X", z180_state, IO_##_name, 0xff, ~0, _flags)


static const cpu_state_entry state_array[] =
{
	Z180_STATE_ENTRY(PC,  "%04X", PC.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(GENPC, "%04X", _PCD, 0xffff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(GENPCBASE, "%04X", PREPC.w.l, 0xffff, CPUSTATE_NOSHOW)

	Z180_STATE_ENTRY(SP,  "%04X", _SPD, 0xffff, 0)
	Z180_STATE_ENTRY(GENSP, "%04X", SP.w.l, 0xffff, CPUSTATE_NOSHOW)

	Z180_STATE_ENTRY(A, "%02X", _A, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(B, "%02X", _B, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(C, "%02X", _C, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(D, "%02X", _D, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(E, "%02X", _E, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(H, "%02X", _H, 0xff, CPUSTATE_NOSHOW)
	Z180_STATE_ENTRY(L, "%02X", _L, 0xff, CPUSTATE_NOSHOW)

	Z180_STATE_ENTRY(AF, "%04X", AF.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(BC, "%04X", BC.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(DE, "%04X", DE.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(HL, "%04X", HL.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(IX, "%04X", IX.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(IY, "%04X", IY.w.l, 0xffff, 0)

	Z180_STATE_ENTRY(AF2, "%04X", AF2.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(BC2, "%04X", BC2.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(DE2, "%04X", DE2.w.l, 0xffff, 0)
	Z180_STATE_ENTRY(HL2, "%04X", HL2.w.l, 0xffff, 0)

	Z180_STATE_ENTRY(R,      "%02X", rtemp, 0xff, CPUSTATE_EXPORT | CPUSTATE_IMPORT)
	Z180_STATE_ENTRY(I,      "%02X", I, 0xff, 0)
	Z180_STATE_ENTRY(IM,     "%1u", IM, 0x3, 0)
	Z180_STATE_ENTRY(IFF1,   "%1u", IFF1, 0x1, 0)
	Z180_STATE_ENTRY(IFF2,   "%1u", IFF2, 0x1, 0)
	Z180_STATE_ENTRY(HALT,   "%1u", HALT, 0x1, 0)

	Z180_STATE_ENTRY(IOLINES, "%06X", ioltemp, 0xffffff, CPUSTATE_IMPORT)

	Z180_STATE_IO_ENTRY(CNTLA0, 0)
	Z180_STATE_IO_ENTRY(CNTLA1, 0)
	Z180_STATE_IO_ENTRY(CNTLB0, 0)
	Z180_STATE_IO_ENTRY(CNTLB1, 0)
	Z180_STATE_IO_ENTRY(STAT0, 0)
	Z180_STATE_IO_ENTRY(STAT1, 0)
	Z180_STATE_IO_ENTRY(TDR0, 0)
	Z180_STATE_IO_ENTRY(TDR1, 0)
	Z180_STATE_IO_ENTRY(RDR0, 0)
	Z180_STATE_IO_ENTRY(RDR1, 0)
	Z180_STATE_IO_ENTRY(CNTR, 0)
	Z180_STATE_IO_ENTRY(TRDR, 0)
	Z180_STATE_IO_ENTRY(TMDR0L, 0)
	Z180_STATE_IO_ENTRY(TMDR0H, 0)
	Z180_STATE_IO_ENTRY(RLDR0L, 0)
	Z180_STATE_IO_ENTRY(RLDR0H, 0)
	Z180_STATE_IO_ENTRY(TCR, 0)
	Z180_STATE_IO_ENTRY(IO11, 0)
	Z180_STATE_IO_ENTRY(ASEXT0, 0)
	Z180_STATE_IO_ENTRY(ASEXT1, 0)
	Z180_STATE_IO_ENTRY(TMDR1L, 0)
	Z180_STATE_IO_ENTRY(TMDR1H, 0)
	Z180_STATE_IO_ENTRY(RLDR1L, 0)
	Z180_STATE_IO_ENTRY(RLDR1H, 0)
	Z180_STATE_IO_ENTRY(FRC, 0)
	Z180_STATE_IO_ENTRY(IO19, 0)
	Z180_STATE_IO_ENTRY(ASTC0L, 0)
	Z180_STATE_IO_ENTRY(ASTC0H, 0)
	Z180_STATE_IO_ENTRY(ASTC1L, 0)
	Z180_STATE_IO_ENTRY(ASTC1H, 0)
	Z180_STATE_IO_ENTRY(CMR, 0)
	Z180_STATE_IO_ENTRY(CCR, 0)
	Z180_STATE_IO_ENTRY(SAR0L, 0)
	Z180_STATE_IO_ENTRY(SAR0H, 0)
	Z180_STATE_IO_ENTRY(SAR0B, 0)
	Z180_STATE_IO_ENTRY(DAR0L, 0)
	Z180_STATE_IO_ENTRY(DAR0H, 0)
	Z180_STATE_IO_ENTRY(DAR0B, 0)
	Z180_STATE_IO_ENTRY(BCR0L, 0)
	Z180_STATE_IO_ENTRY(BCR0H, 0)
	Z180_STATE_IO_ENTRY(MAR1L, 0)
	Z180_STATE_IO_ENTRY(MAR1H, 0)
	Z180_STATE_IO_ENTRY(MAR1B, 0)
	Z180_STATE_IO_ENTRY(IAR1L, 0)
	Z180_STATE_IO_ENTRY(IAR1H, 0)
	Z180_STATE_IO_ENTRY(IAR1B, 0)
	Z180_STATE_IO_ENTRY(BCR1L, 0)
	Z180_STATE_IO_ENTRY(BCR1H, 0)
	Z180_STATE_IO_ENTRY(DSTAT, 0)
	Z180_STATE_IO_ENTRY(DMODE, 0)
	Z180_STATE_IO_ENTRY(DCNTL, 0)
	Z180_STATE_IO_ENTRY(IL, 0)
	Z180_STATE_IO_ENTRY(ITC, 0)
	Z180_STATE_IO_ENTRY(IO35, 0)
	Z180_STATE_IO_ENTRY(RCR, 0)
	Z180_STATE_IO_ENTRY(IO37, 0)
	Z180_STATE_IO_ENTRY(CBR, CPUSTATE_IMPORT)
	Z180_STATE_IO_ENTRY(BBR, CPUSTATE_IMPORT)
	Z180_STATE_IO_ENTRY(CBAR, CPUSTATE_IMPORT)
	Z180_STATE_IO_ENTRY(IO3B, 0)
	Z180_STATE_IO_ENTRY(IO3C, 0)
	Z180_STATE_IO_ENTRY(IO3D, 0)
	Z180_STATE_IO_ENTRY(OMCR, 0)
	Z180_STATE_IO_ENTRY(IOCR, 0)
};

static const cpu_state_table state_table_template =
{
	NULL,						/* pointer to the base of state (offsets are relative to this) */
	0,							/* subtype this table refers to */
	ARRAY_LENGTH(state_array),	/* number of entries */
	state_array					/* array of entries */
};



static UINT8 SZ[256];		/* zero and sign flags */
static UINT8 SZ_BIT[256];	/* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];		/* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static UINT8 *SZHVC_add;
static UINT8 *SZHVC_sub;

static UINT8 z180_readcontrol(z180_state *cpustate, offs_t port);
static void z180_writecontrol(z180_state *cpustate, offs_t port, UINT8 data);
static void z180_dma0(z180_state *cpustate);
static void z180_dma1(z180_state *cpustate);
static CPU_BURN( z180 );
static CPU_SET_INFO( z180 );

#include "z180ops.h"
#include "z180tbl.h"

#include "z180cb.c"
#include "z180xy.c"
#include "z180dd.c"
#include "z180fd.c"
#include "z180ed.c"
#include "z180op.c"

static UINT8 z180_readcontrol(z180_state *cpustate, offs_t port)
{
	/* normal external readport */
	UINT8 data = memory_read_byte_8le(cpustate->iospace, port);

	/* remap internal I/O registers */
	if((port & (cpustate->IO_IOCR & 0xc0)) == (cpustate->IO_IOCR & 0xc0))
		port = port - (cpustate->IO_IOCR & 0xc0);

	/* but ignore the data and read the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		data = cpustate->IO_CNTLA0 & Z180_CNTLA0_RMASK;
		LOG(("Z180 '%s' CNTLA0 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CNTLA1:
		data = cpustate->IO_CNTLA1 & Z180_CNTLA1_RMASK;
		LOG(("Z180 '%s' CNTLA1 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CNTLB0:
		data = cpustate->IO_CNTLB0 & Z180_CNTLB0_RMASK;
		LOG(("Z180 '%s' CNTLB0 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CNTLB1:
		data = cpustate->IO_CNTLB1 & Z180_CNTLB1_RMASK;
		LOG(("Z180 '%s' CNTLB1 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_STAT0:
		data = cpustate->IO_STAT0 & Z180_STAT0_RMASK;
data |= 0x02; // kludge for 20pacgal
		LOG(("Z180 '%s' STAT0  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_STAT1:
		data = cpustate->IO_STAT1 & Z180_STAT1_RMASK;
		LOG(("Z180 '%s' STAT1  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TDR0:
		data = cpustate->IO_TDR0 & Z180_TDR0_RMASK;
		LOG(("Z180 '%s' TDR0   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TDR1:
		data = cpustate->IO_TDR1 & Z180_TDR1_RMASK;
		LOG(("Z180 '%s' TDR1   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RDR0:
		data = cpustate->IO_RDR0 & Z180_RDR0_RMASK;
		LOG(("Z180 '%s' RDR0   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RDR1:
		data = cpustate->IO_RDR1 & Z180_RDR1_RMASK;
		LOG(("Z180 '%s' RDR1   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CNTR:
		data = cpustate->IO_CNTR & Z180_CNTR_RMASK;
		LOG(("Z180 '%s' CNTR   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TRDR:
		data = cpustate->IO_TRDR & Z180_TRDR_RMASK;
		LOG(("Z180 '%s' TRDR   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TMDR0L:
		data = cpustate->tmdr_value[0] & Z180_TMDR0L_RMASK;
		LOG(("Z180 '%s' TMDR0L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((cpustate->IO_TCR & Z180_TCR_TDE0) == 0)
		{
			cpustate->tmdr_latch |= 1;
			cpustate->tmdrh[0] = (cpustate->tmdr_value[0] & 0xff00) >> 8;
		}

		if(cpustate->read_tcr_tmdr[0])
		{
			cpustate->tif[0] = 0; // reset TIF0
			cpustate->read_tcr_tmdr[0] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[0] = 1;
		}
		break;

	case Z180_TMDR0H:
		/* read latched value? */
		if (cpustate->tmdr_latch & 1)
		{
			cpustate->tmdr_latch &= ~1;
			data = cpustate->tmdrh[0];
		}
		else
		{
			data = (cpustate->tmdr_value[0] & 0xff00) >> 8;
		}

		if(cpustate->read_tcr_tmdr[0])
		{
			cpustate->tif[0] = 0; // reset TIF0
			cpustate->read_tcr_tmdr[0] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[0] = 1;
		}
		LOG(("Z180 '%s' TMDR0H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RLDR0L:
		data = cpustate->IO_RLDR0L & Z180_RLDR0L_RMASK;
		LOG(("Z180 '%s' RLDR0L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RLDR0H:
		data = cpustate->IO_RLDR0H & Z180_RLDR0H_RMASK;
		LOG(("Z180 '%s' RLDR0H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TCR:
		data = (cpustate->IO_TCR & Z180_TCR_RMASK) | (cpustate->tif[0] << 6) | (cpustate->tif[1] << 7);

		if(cpustate->read_tcr_tmdr[0])
		{
			cpustate->tif[0] = 0; // reset TIF0
			cpustate->read_tcr_tmdr[0] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[0] = 1;
		}

		if(cpustate->read_tcr_tmdr[1])
		{
			cpustate->tif[1] = 0; // reset TIF1
			cpustate->read_tcr_tmdr[1] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[1] = 1;
		}

		LOG(("Z180 '%s' TCR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO11:
		data = cpustate->IO_IO11 & Z180_IO11_RMASK;
		LOG(("Z180 '%s' IO11   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASEXT0:
		data = cpustate->IO_ASEXT0 & Z180_ASEXT0_RMASK;
		LOG(("Z180 '%s' ASEXT0 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASEXT1:
		data = cpustate->IO_ASEXT1 & Z180_ASEXT1_RMASK;
		LOG(("Z180 '%s' ASEXT1 rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_TMDR1L:
		data = cpustate->tmdr_value[1] & Z180_TMDR1L_RMASK;
		LOG(("Z180 '%s' TMDR1L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((cpustate->IO_TCR & Z180_TCR_TDE1) == 0)
		{
			cpustate->tmdr_latch |= 2;
			cpustate->tmdrh[1] = (cpustate->tmdr_value[1] & 0xff00) >> 8;
		}

		if(cpustate->read_tcr_tmdr[1])
		{
			cpustate->tif[1] = 0; // reset TIF1
			cpustate->read_tcr_tmdr[1] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[1] = 1;
		}
		break;

	case Z180_TMDR1H:
		/* read latched value? */
		if (cpustate->tmdr_latch & 2)
		{
			cpustate->tmdr_latch &= ~2;
			data = cpustate->tmdrh[1];
		}
		else
		{
			data = (cpustate->tmdr_value[1] & 0xff00) >> 8;
		}

		if(cpustate->read_tcr_tmdr[1])
		{
			cpustate->tif[1] = 0; // reset TIF1
			cpustate->read_tcr_tmdr[1] = 0;
		}
		else
		{
			cpustate->read_tcr_tmdr[1] = 1;
		}
		LOG(("Z180 '%s' TMDR1H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RLDR1L:
		data = cpustate->IO_RLDR1L & Z180_RLDR1L_RMASK;
		LOG(("Z180 '%s' RLDR1L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RLDR1H:
		data = cpustate->IO_RLDR1H & Z180_RLDR1H_RMASK;
		LOG(("Z180 '%s' RLDR1H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_FRC:
		data = cpustate->IO_FRC & Z180_FRC_RMASK;
		LOG(("Z180 '%s' FRC    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO19:
		data = cpustate->IO_IO19 & Z180_IO19_RMASK;
		LOG(("Z180 '%s' IO19   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASTC0L:
		data = cpustate->IO_ASTC0L & Z180_ASTC0L_RMASK;
		LOG(("Z180 '%s' ASTC0L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASTC0H:
		data = cpustate->IO_ASTC0H & Z180_ASTC0H_RMASK;
		LOG(("Z180 '%s' ASTC0H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASTC1L:
		data = cpustate->IO_ASTC1L & Z180_ASTC1L_RMASK;
		LOG(("Z180 '%s' ASTC1L rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ASTC1H:
		data = cpustate->IO_ASTC1H & Z180_ASTC1H_RMASK;
		LOG(("Z180 '%s' ASTC1H rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CMR:
		data = cpustate->IO_CMR & Z180_CMR_RMASK;
		LOG(("Z180 '%s' CMR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CCR:
		data = cpustate->IO_CCR & Z180_CCR_RMASK;
		LOG(("Z180 '%s' CCR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_SAR0L:
		data = cpustate->IO_SAR0L & Z180_SAR0L_RMASK;
		LOG(("Z180 '%s' SAR0L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_SAR0H:
		data = cpustate->IO_SAR0H & Z180_SAR0H_RMASK;
		LOG(("Z180 '%s' SAR0H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_SAR0B:
		data = cpustate->IO_SAR0B & Z180_SAR0B_RMASK;
		LOG(("Z180 '%s' SAR0B  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DAR0L:
		data = cpustate->IO_DAR0L & Z180_DAR0L_RMASK;
		LOG(("Z180 '%s' DAR0L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DAR0H:
		data = cpustate->IO_DAR0H & Z180_DAR0H_RMASK;
		LOG(("Z180 '%s' DAR0H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DAR0B:
		data = cpustate->IO_DAR0B & Z180_DAR0B_RMASK;
		LOG(("Z180 '%s' DAR0B  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_BCR0L:
		data = cpustate->IO_BCR0L & Z180_BCR0L_RMASK;
		LOG(("Z180 '%s' BCR0L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_BCR0H:
		data = cpustate->IO_BCR0H & Z180_BCR0H_RMASK;
		LOG(("Z180 '%s' BCR0H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_MAR1L:
		data = cpustate->IO_MAR1L & Z180_MAR1L_RMASK;
		LOG(("Z180 '%s' MAR1L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_MAR1H:
		data = cpustate->IO_MAR1H & Z180_MAR1H_RMASK;
		LOG(("Z180 '%s' MAR1H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_MAR1B:
		data = cpustate->IO_MAR1B & Z180_MAR1B_RMASK;
		LOG(("Z180 '%s' MAR1B  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IAR1L:
		data = cpustate->IO_IAR1L & Z180_IAR1L_RMASK;
		LOG(("Z180 '%s' IAR1L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IAR1H:
		data = cpustate->IO_IAR1H & Z180_IAR1H_RMASK;
		LOG(("Z180 '%s' IAR1H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IAR1B:
		data = cpustate->IO_IAR1B & Z180_IAR1B_RMASK;
		LOG(("Z180 '%s' IAR1B  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_BCR1L:
		data = cpustate->IO_BCR1L & Z180_BCR1L_RMASK;
		LOG(("Z180 '%s' BCR1L  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_BCR1H:
		data = cpustate->IO_BCR1H & Z180_BCR1H_RMASK;
		LOG(("Z180 '%s' BCR1H  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DSTAT:
		data = cpustate->IO_DSTAT & Z180_DSTAT_RMASK;
		LOG(("Z180 '%s' DSTAT  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DMODE:
		data = cpustate->IO_DMODE & Z180_DMODE_RMASK;
		LOG(("Z180 '%s' DMODE  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_DCNTL:
		data = cpustate->IO_DCNTL & Z180_DCNTL_RMASK;
		LOG(("Z180 '%s' DCNTL  rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IL:
		data = cpustate->IO_IL & Z180_IL_RMASK;
		LOG(("Z180 '%s' IL     rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_ITC:
		data = cpustate->IO_ITC & Z180_ITC_RMASK;
		LOG(("Z180 '%s' ITC    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO35:
		data = cpustate->IO_IO35 & Z180_IO35_RMASK;
		LOG(("Z180 '%s' IO35   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_RCR:
		data = cpustate->IO_RCR & Z180_RCR_RMASK;
		LOG(("Z180 '%s' RCR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO37:
		data = cpustate->IO_IO37 & Z180_IO37_RMASK;
		LOG(("Z180 '%s' IO37   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CBR:
		data = cpustate->IO_CBR & Z180_CBR_RMASK;
		LOG(("Z180 '%s' CBR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_BBR:
		data = cpustate->IO_BBR & Z180_BBR_RMASK;
		LOG(("Z180 '%s' BBR    rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_CBAR:
		data = cpustate->IO_CBAR & Z180_CBAR_RMASK;
		LOG(("Z180 '%s' CBAR   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO3B:
		data = cpustate->IO_IO3B & Z180_IO3B_RMASK;
		LOG(("Z180 '%s' IO3B   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO3C:
		data = cpustate->IO_IO3C & Z180_IO3C_RMASK;
		LOG(("Z180 '%s' IO3C   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IO3D:
		data = cpustate->IO_IO3D & Z180_IO3D_RMASK;
		LOG(("Z180 '%s' IO3D   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_OMCR:
		data = cpustate->IO_OMCR & Z180_OMCR_RMASK;
		LOG(("Z180 '%s' OMCR   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;

	case Z180_IOCR:
		data = cpustate->IO_IOCR & Z180_IOCR_RMASK;
		LOG(("Z180 '%s' IOCR   rd $%02x ($%02x)\n", cpustate->device->tag, data, cpustate->io[port & 0x3f]));
		break;
	}

	return data;
}

static void z180_writecontrol(z180_state *cpustate, offs_t port, UINT8 data)
{
	/* normal external write port */
	memory_write_byte_8le(cpustate->iospace, port, data);

	/* remap internal I/O registers */
	if((port & (cpustate->IO_IOCR & 0xc0)) == (cpustate->IO_IOCR & 0xc0))
		port = port - (cpustate->IO_IOCR & 0xc0);

	/* store the data in the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		LOG(("Z180 '%s' CNTLA0 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CNTLA0_WMASK));
		cpustate->IO_CNTLA0 = (cpustate->IO_CNTLA0 & ~Z180_CNTLA0_WMASK) | (data & Z180_CNTLA0_WMASK);
		break;

	case Z180_CNTLA1:
		LOG(("Z180 '%s' CNTLA1 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CNTLA1_WMASK));
		cpustate->IO_CNTLA1 = (cpustate->IO_CNTLA1 & ~Z180_CNTLA1_WMASK) | (data & Z180_CNTLA1_WMASK);
		break;

	case Z180_CNTLB0:
		LOG(("Z180 '%s' CNTLB0 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CNTLB0_WMASK));
		cpustate->IO_CNTLB0 = (cpustate->IO_CNTLB0 & ~Z180_CNTLB0_WMASK) | (data & Z180_CNTLB0_WMASK);
		break;

	case Z180_CNTLB1:
		LOG(("Z180 '%s' CNTLB1 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CNTLB1_WMASK));
		cpustate->IO_CNTLB1 = (cpustate->IO_CNTLB1 & ~Z180_CNTLB1_WMASK) | (data & Z180_CNTLB1_WMASK);
		break;

	case Z180_STAT0:
		LOG(("Z180 '%s' STAT0  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_STAT0_WMASK));
		cpustate->IO_STAT0 = (cpustate->IO_STAT0 & ~Z180_STAT0_WMASK) | (data & Z180_STAT0_WMASK);
		break;

	case Z180_STAT1:
		LOG(("Z180 '%s' STAT1  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_STAT1_WMASK));
		cpustate->IO_STAT1 = (cpustate->IO_STAT1 & ~Z180_STAT1_WMASK) | (data & Z180_STAT1_WMASK);
		break;

	case Z180_TDR0:
		LOG(("Z180 '%s' TDR0   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TDR0_WMASK));
		cpustate->IO_TDR0 = (cpustate->IO_TDR0 & ~Z180_TDR0_WMASK) | (data & Z180_TDR0_WMASK);
		break;

	case Z180_TDR1:
		LOG(("Z180 '%s' TDR1   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TDR1_WMASK));
		cpustate->IO_TDR1 = (cpustate->IO_TDR1 & ~Z180_TDR1_WMASK) | (data & Z180_TDR1_WMASK);
		break;

	case Z180_RDR0:
		LOG(("Z180 '%s' RDR0   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RDR0_WMASK));
		cpustate->IO_RDR0 = (cpustate->IO_RDR0 & ~Z180_RDR0_WMASK) | (data & Z180_RDR0_WMASK);
		break;

	case Z180_RDR1:
		LOG(("Z180 '%s' RDR1   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RDR1_WMASK));
		cpustate->IO_RDR1 = (cpustate->IO_RDR1 & ~Z180_RDR1_WMASK) | (data & Z180_RDR1_WMASK);
		break;

	case Z180_CNTR:
		LOG(("Z180 '%s' CNTR   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CNTR_WMASK));
		cpustate->IO_CNTR = (cpustate->IO_CNTR & ~Z180_CNTR_WMASK) | (data & Z180_CNTR_WMASK);
		break;

	case Z180_TRDR:
		LOG(("Z180 '%s' TRDR   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TRDR_WMASK));
		cpustate->IO_TRDR = (cpustate->IO_TRDR & ~Z180_TRDR_WMASK) | (data & Z180_TRDR_WMASK);
		break;

	case Z180_TMDR0L:
		LOG(("Z180 '%s' TMDR0L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TMDR0L_WMASK));
		cpustate->IO_TMDR0L = data & Z180_TMDR0L_WMASK;
		cpustate->tmdr_value[0] = (cpustate->tmdr_value[0] & 0xff00) | cpustate->IO_TMDR0L;
		break;

	case Z180_TMDR0H:
		LOG(("Z180 '%s' TMDR0H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TMDR0H_WMASK));
		cpustate->IO_TMDR0H = data & Z180_TMDR0H_WMASK;
		cpustate->tmdr_value[0] = (cpustate->tmdr_value[0] & 0x00ff) | (cpustate->IO_TMDR0H << 8);
		break;

	case Z180_RLDR0L:
		LOG(("Z180 '%s' RLDR0L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RLDR0L_WMASK));
		cpustate->IO_RLDR0L = (cpustate->IO_RLDR0L & ~Z180_RLDR0L_WMASK) | (data & Z180_RLDR0L_WMASK);
		break;

	case Z180_RLDR0H:
		LOG(("Z180 '%s' RLDR0H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RLDR0H_WMASK));
		cpustate->IO_RLDR0H = (cpustate->IO_RLDR0H & ~Z180_RLDR0H_WMASK) | (data & Z180_RLDR0H_WMASK);
		break;

	case Z180_TCR:
		LOG(("Z180 '%s' TCR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TCR_WMASK));
		cpustate->IO_TCR = (cpustate->IO_TCR & ~Z180_TCR_WMASK) | (data & Z180_TCR_WMASK);
		break;

	case Z180_IO11:
		LOG(("Z180 '%s' IO11   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO11_WMASK));
		cpustate->IO_IO11 = (cpustate->IO_IO11 & ~Z180_IO11_WMASK) | (data & Z180_IO11_WMASK);
		break;

	case Z180_ASEXT0:
		LOG(("Z180 '%s' ASEXT0 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASEXT0_WMASK));
		cpustate->IO_ASEXT0 = (cpustate->IO_ASEXT0 & ~Z180_ASEXT0_WMASK) | (data & Z180_ASEXT0_WMASK);
		break;

	case Z180_ASEXT1:
		LOG(("Z180 '%s' ASEXT1 wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASEXT1_WMASK));
		cpustate->IO_ASEXT1 = (cpustate->IO_ASEXT1 & ~Z180_ASEXT1_WMASK) | (data & Z180_ASEXT1_WMASK);
		break;

	case Z180_TMDR1L:
		LOG(("Z180 '%s' TMDR1L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TMDR1L_WMASK));
		cpustate->IO_TMDR1L = data & Z180_TMDR1L_WMASK;
		cpustate->tmdr_value[1] = (cpustate->tmdr_value[1] & 0xff00) | cpustate->IO_TMDR1L;
		break;

	case Z180_TMDR1H:
		LOG(("Z180 '%s' TMDR1H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_TMDR1H_WMASK));
		cpustate->IO_TMDR1H = data & Z180_TMDR1H_WMASK;
		cpustate->tmdr_value[1] = (cpustate->tmdr_value[1] & 0x00ff) | cpustate->IO_TMDR1H;
		break;

	case Z180_RLDR1L:
		LOG(("Z180 '%s' RLDR1L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RLDR1L_WMASK));
		cpustate->IO_RLDR1L = (cpustate->IO_RLDR1L & ~Z180_RLDR1L_WMASK) | (data & Z180_RLDR1L_WMASK);
		break;

	case Z180_RLDR1H:
		LOG(("Z180 '%s' RLDR1H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RLDR1H_WMASK));
		cpustate->IO_RLDR1H = (cpustate->IO_RLDR1H & ~Z180_RLDR1H_WMASK) | (data & Z180_RLDR1H_WMASK);
		break;

	case Z180_FRC:
		LOG(("Z180 '%s' FRC    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_FRC_WMASK));
		cpustate->IO_FRC = (cpustate->IO_FRC & ~Z180_FRC_WMASK) | (data & Z180_FRC_WMASK);
		break;

	case Z180_IO19:
		LOG(("Z180 '%s' IO19   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO19_WMASK));
		cpustate->IO_IO19 = (cpustate->IO_IO19 & ~Z180_IO19_WMASK) | (data & Z180_IO19_WMASK);
		break;

	case Z180_ASTC0L:
		LOG(("Z180 '%s' ASTC0L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASTC0L_WMASK));
		cpustate->IO_ASTC0L = (cpustate->IO_ASTC0L & ~Z180_ASTC0L_WMASK) | (data & Z180_ASTC0L_WMASK);
		break;

	case Z180_ASTC0H:
		LOG(("Z180 '%s' ASTC0H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASTC0H_WMASK));
		cpustate->IO_ASTC0H = (cpustate->IO_ASTC0H & ~Z180_ASTC0H_WMASK) | (data & Z180_ASTC0H_WMASK);
		break;

	case Z180_ASTC1L:
		LOG(("Z180 '%s' ASTC1L wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASTC1L_WMASK));
		cpustate->IO_ASTC1L = (cpustate->IO_ASTC1L & ~Z180_ASTC1L_WMASK) | (data & Z180_ASTC1L_WMASK);
		break;

	case Z180_ASTC1H:
		LOG(("Z180 '%s' ASTC1H wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ASTC1H_WMASK));
		cpustate->IO_ASTC1H = (cpustate->IO_ASTC1H & ~Z180_ASTC1H_WMASK) | (data & Z180_ASTC1H_WMASK);
		break;

	case Z180_CMR:
		LOG(("Z180 '%s' CMR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CMR_WMASK));
		cpustate->IO_CMR = (cpustate->IO_CMR & ~Z180_CMR_WMASK) | (data & Z180_CMR_WMASK);
		break;

	case Z180_CCR:
		LOG(("Z180 '%s' CCR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CCR_WMASK));
		cpustate->IO_CCR = (cpustate->IO_CCR & ~Z180_CCR_WMASK) | (data & Z180_CCR_WMASK);
		break;

	case Z180_SAR0L:
		LOG(("Z180 '%s' SAR0L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_SAR0L_WMASK));
		cpustate->IO_SAR0L = (cpustate->IO_SAR0L & ~Z180_SAR0L_WMASK) | (data & Z180_SAR0L_WMASK);
		break;

	case Z180_SAR0H:
		LOG(("Z180 '%s' SAR0H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_SAR0H_WMASK));
		cpustate->IO_SAR0H = (cpustate->IO_SAR0H & ~Z180_SAR0H_WMASK) | (data & Z180_SAR0H_WMASK);
		break;

	case Z180_SAR0B:
		LOG(("Z180 '%s' SAR0B  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_SAR0B_WMASK));
		cpustate->IO_SAR0B = (cpustate->IO_SAR0B & ~Z180_SAR0B_WMASK) | (data & Z180_SAR0B_WMASK);
		break;

	case Z180_DAR0L:
		LOG(("Z180 '%s' DAR0L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DAR0L_WMASK));
		cpustate->IO_DAR0L = (cpustate->IO_DAR0L & ~Z180_DAR0L_WMASK) | (data & Z180_DAR0L_WMASK);
		break;

	case Z180_DAR0H:
		LOG(("Z180 '%s' DAR0H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DAR0H_WMASK));
		cpustate->IO_DAR0H = (cpustate->IO_DAR0H & ~Z180_DAR0H_WMASK) | (data & Z180_DAR0H_WMASK);
		break;

	case Z180_DAR0B:
		LOG(("Z180 '%s' DAR0B  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DAR0B_WMASK));
		cpustate->IO_DAR0B = (cpustate->IO_DAR0B & ~Z180_DAR0B_WMASK) | (data & Z180_DAR0B_WMASK);
		break;

	case Z180_BCR0L:
		LOG(("Z180 '%s' BCR0L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_BCR0L_WMASK));
		cpustate->IO_BCR0L = (cpustate->IO_BCR0L & ~Z180_BCR0L_WMASK) | (data & Z180_BCR0L_WMASK);
		break;

	case Z180_BCR0H:
		LOG(("Z180 '%s' BCR0H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_BCR0H_WMASK));
		cpustate->IO_BCR0H = (cpustate->IO_BCR0H & ~Z180_BCR0H_WMASK) | (data & Z180_BCR0H_WMASK);
		break;

	case Z180_MAR1L:
		LOG(("Z180 '%s' MAR1L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_MAR1L_WMASK));
		cpustate->IO_MAR1L = (cpustate->IO_MAR1L & ~Z180_MAR1L_WMASK) | (data & Z180_MAR1L_WMASK);
		break;

	case Z180_MAR1H:
		LOG(("Z180 '%s' MAR1H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_MAR1H_WMASK));
		cpustate->IO_MAR1H = (cpustate->IO_MAR1H & ~Z180_MAR1H_WMASK) | (data & Z180_MAR1H_WMASK);
		break;

	case Z180_MAR1B:
		LOG(("Z180 '%s' MAR1B  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_MAR1B_WMASK));
		cpustate->IO_MAR1B = (cpustate->IO_MAR1B & ~Z180_MAR1B_WMASK) | (data & Z180_MAR1B_WMASK);
		break;

	case Z180_IAR1L:
		LOG(("Z180 '%s' IAR1L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IAR1L_WMASK));
		cpustate->IO_IAR1L = (cpustate->IO_IAR1L & ~Z180_IAR1L_WMASK) | (data & Z180_IAR1L_WMASK);
		break;

	case Z180_IAR1H:
		LOG(("Z180 '%s' IAR1H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IAR1H_WMASK));
		cpustate->IO_IAR1H = (cpustate->IO_IAR1H & ~Z180_IAR1H_WMASK) | (data & Z180_IAR1H_WMASK);
		break;

	case Z180_IAR1B:
		LOG(("Z180 '%s' IAR1B  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IAR1B_WMASK));
		cpustate->IO_IAR1B = (cpustate->IO_IAR1B & ~Z180_IAR1B_WMASK) | (data & Z180_IAR1B_WMASK);
		break;

	case Z180_BCR1L:
		LOG(("Z180 '%s' BCR1L  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_BCR1L_WMASK));
		cpustate->IO_BCR1L = (cpustate->IO_BCR1L & ~Z180_BCR1L_WMASK) | (data & Z180_BCR1L_WMASK);
		break;

	case Z180_BCR1H:
		LOG(("Z180 '%s' BCR1H  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_BCR1H_WMASK));
		cpustate->IO_BCR1H = (cpustate->IO_BCR1H & ~Z180_BCR1H_WMASK) | (data & Z180_BCR1H_WMASK);
		break;

	case Z180_DSTAT:
		LOG(("Z180 '%s' DSTAT  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DSTAT_WMASK));
		cpustate->IO_DSTAT = (cpustate->IO_DSTAT & ~Z180_DSTAT_WMASK) | (data & Z180_DSTAT_WMASK);
		if ((data & (Z180_DSTAT_DE1 | Z180_DSTAT_DWE1)) == Z180_DSTAT_DE1)
			cpustate->IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		if ((data & (Z180_DSTAT_DE0 | Z180_DSTAT_DWE0)) == Z180_DSTAT_DE0)
			cpustate->IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		break;

	case Z180_DMODE:
		LOG(("Z180 '%s' DMODE  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DMODE_WMASK));
		cpustate->IO_DMODE = (cpustate->IO_DMODE & ~Z180_DMODE_WMASK) | (data & Z180_DMODE_WMASK);
		break;

	case Z180_DCNTL:
		LOG(("Z180 '%s' DCNTL  wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_DCNTL_WMASK));
		cpustate->IO_DCNTL = (cpustate->IO_DCNTL & ~Z180_DCNTL_WMASK) | (data & Z180_DCNTL_WMASK);
		break;

	case Z180_IL:
		LOG(("Z180 '%s' IL     wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IL_WMASK));
		cpustate->IO_IL = (cpustate->IO_IL & ~Z180_IL_WMASK) | (data & Z180_IL_WMASK);
		break;

	case Z180_ITC:
		LOG(("Z180 '%s' ITC    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_ITC_WMASK));
		cpustate->IO_ITC = (cpustate->IO_ITC & ~Z180_ITC_WMASK) | (data & Z180_ITC_WMASK);
		break;

	case Z180_IO35:
		LOG(("Z180 '%s' IO35   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO35_WMASK));
		cpustate->IO_IO35 = (cpustate->IO_IO35 & ~Z180_IO35_WMASK) | (data & Z180_IO35_WMASK);
		break;

	case Z180_RCR:
		LOG(("Z180 '%s' RCR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_RCR_WMASK));
		cpustate->IO_RCR = (cpustate->IO_RCR & ~Z180_RCR_WMASK) | (data & Z180_RCR_WMASK);
		break;

	case Z180_IO37:
		LOG(("Z180 '%s' IO37   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO37_WMASK));
		cpustate->IO_IO37 = (cpustate->IO_IO37 & ~Z180_IO37_WMASK) | (data & Z180_IO37_WMASK);
		break;

	case Z180_CBR:
		LOG(("Z180 '%s' CBR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CBR_WMASK));
		cpustate->IO_CBR = (cpustate->IO_CBR & ~Z180_CBR_WMASK) | (data & Z180_CBR_WMASK);
		z180_mmu(cpustate);
		break;

	case Z180_BBR:
		LOG(("Z180 '%s' BBR    wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_BBR_WMASK));
		cpustate->IO_BBR = (cpustate->IO_BBR & ~Z180_BBR_WMASK) | (data & Z180_BBR_WMASK);
		z180_mmu(cpustate);
		break;

	case Z180_CBAR:
		LOG(("Z180 '%s' CBAR   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_CBAR_WMASK));
		cpustate->IO_CBAR = (cpustate->IO_CBAR & ~Z180_CBAR_WMASK) | (data & Z180_CBAR_WMASK);
		z180_mmu(cpustate);
		break;

	case Z180_IO3B:
		LOG(("Z180 '%s' IO3B   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO3B_WMASK));
		cpustate->IO_IO3B = (cpustate->IO_IO3B & ~Z180_IO3B_WMASK) | (data & Z180_IO3B_WMASK);
		break;

	case Z180_IO3C:
		LOG(("Z180 '%s' IO3C   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO3C_WMASK));
		cpustate->IO_IO3C = (cpustate->IO_IO3C & ~Z180_IO3C_WMASK) | (data & Z180_IO3C_WMASK);
		break;

	case Z180_IO3D:
		LOG(("Z180 '%s' IO3D   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IO3D_WMASK));
		cpustate->IO_IO3D = (cpustate->IO_IO3D & ~Z180_IO3D_WMASK) | (data & Z180_IO3D_WMASK);
		break;

	case Z180_OMCR:
		LOG(("Z180 '%s' OMCR   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_OMCR_WMASK));
		cpustate->IO_OMCR = (cpustate->IO_OMCR & ~Z180_OMCR_WMASK) | (data & Z180_OMCR_WMASK);
		break;

	case Z180_IOCR:
		LOG(("Z180 '%s' IOCR   wr $%02x ($%02x)\n", cpustate->device->tag, data,  data & Z180_IOCR_WMASK));
		cpustate->IO_IOCR = (cpustate->IO_IOCR & ~Z180_IOCR_WMASK) | (data & Z180_IOCR_WMASK);
		break;
	}
}

static void z180_dma0(z180_state *cpustate)
{
	offs_t sar0 = 65536 * cpustate->IO_SAR0B + 256 * cpustate->IO_SAR0H + cpustate->IO_SAR0L;
	offs_t dar0 = 65536 * cpustate->IO_DAR0B + 256 * cpustate->IO_DAR0H + cpustate->IO_DAR0L;
	int bcr0 = 256 * cpustate->IO_BCR0H + cpustate->IO_BCR0L;
	int count = (cpustate->IO_DMODE & Z180_DMODE_MMOD) ? bcr0 : 1;

	if (bcr0 == 0)
	{
		cpustate->IO_DSTAT &= ~Z180_DSTAT_DE0;
		return;
	}

	while (count-- > 0)
	{
		/* last transfer happening now? */
		if (bcr0 == 1)
		{
			cpustate->iol |= Z180_TEND0;
		}
		switch( cpustate->IO_DMODE & (Z180_DMODE_SM | Z180_DMODE_DM) )
		{
		case 0x00:	/* memory SAR0+1 to memory DAR0+1 */
			memory_write_byte_8le(cpustate->program, dar0++, memory_read_byte_8le(cpustate->program, sar0++));
			break;
		case 0x04:	/* memory SAR0-1 to memory DAR0+1 */
			memory_write_byte_8le(cpustate->program, dar0++, memory_read_byte_8le(cpustate->program, sar0--));
			break;
		case 0x08:	/* memory SAR0 fixed to memory DAR0+1 */
			memory_write_byte_8le(cpustate->program, dar0++, memory_read_byte_8le(cpustate->program, sar0));
			break;
		case 0x0c:	/* I/O SAR0 fixed to memory DAR0+1 */
			if (cpustate->iol & Z180_DREQ0)
			{
				memory_write_byte_8le(cpustate->program, dar0++, IN(cpustate, sar0));
				/* edge sensitive DREQ0 ? */
				if (cpustate->IO_DCNTL & Z180_DCNTL_DIM0)
				{
					cpustate->iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x10:	/* memory SAR0+1 to memory DAR0-1 */
			memory_write_byte_8le(cpustate->program, dar0--, memory_read_byte_8le(cpustate->program, sar0++));
			break;
		case 0x14:	/* memory SAR0-1 to memory DAR0-1 */
			memory_write_byte_8le(cpustate->program, dar0--, memory_read_byte_8le(cpustate->program, sar0--));
			break;
		case 0x18:	/* memory SAR0 fixed to memory DAR0-1 */
			memory_write_byte_8le(cpustate->program, dar0--, memory_read_byte_8le(cpustate->program, sar0));
			break;
		case 0x1c:	/* I/O SAR0 fixed to memory DAR0-1 */
			if (cpustate->iol & Z180_DREQ0)
            {
				memory_write_byte_8le(cpustate->program, dar0--, IN(cpustate, sar0));
				/* edge sensitive DREQ0 ? */
				if (cpustate->IO_DCNTL & Z180_DCNTL_DIM0)
				{
					cpustate->iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x20:	/* memory SAR0+1 to memory DAR0 fixed */
			memory_write_byte_8le(cpustate->program, dar0, memory_read_byte_8le(cpustate->program, sar0++));
			break;
		case 0x24:	/* memory SAR0-1 to memory DAR0 fixed */
			memory_write_byte_8le(cpustate->program, dar0, memory_read_byte_8le(cpustate->program, sar0--));
			break;
		case 0x28:	/* reserved */
			break;
		case 0x2c:	/* reserved */
			break;
		case 0x30:	/* memory SAR0+1 to I/O DAR0 fixed */
			if (cpustate->iol & Z180_DREQ0)
            {
				OUT(cpustate, dar0, memory_read_byte_8le(cpustate->program, sar0++));
				/* edge sensitive DREQ0 ? */
				if (cpustate->IO_DCNTL & Z180_DCNTL_DIM0)
				{
					cpustate->iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x34:	/* memory SAR0-1 to I/O DAR0 fixed */
			if (cpustate->iol & Z180_DREQ0)
            {
				OUT(cpustate, dar0, memory_read_byte_8le(cpustate->program, sar0--));
				/* edge sensitive DREQ0 ? */
				if (cpustate->IO_DCNTL & Z180_DCNTL_DIM0)
				{
					cpustate->iol &= ~Z180_DREQ0;
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
		if ((cpustate->icount -= 6) < 0)
			break;
	}

	cpustate->IO_SAR0L = sar0;
	cpustate->IO_SAR0H = sar0 >> 8;
	cpustate->IO_SAR0B = sar0 >> 16;
	cpustate->IO_DAR0L = dar0;
	cpustate->IO_DAR0H = dar0 >> 8;
	cpustate->IO_DAR0B = dar0 >> 16;
	cpustate->IO_BCR0L = bcr0;
	cpustate->IO_BCR0H = bcr0 >> 8;

	/* DMA terminal count? */
	if (bcr0 == 0)
	{
		cpustate->iol &= ~Z180_TEND0;
		cpustate->IO_DSTAT &= ~Z180_DSTAT_DE0;
		/* terminal count interrupt enabled? */
		if (cpustate->IO_DSTAT & Z180_DSTAT_DIE0 && cpustate->IFF1)
			take_interrupt(cpustate, Z180_INT_DMA0);
	}
}

static void z180_dma1(z180_state *cpustate)
{
	offs_t mar1 = 65536 * cpustate->IO_MAR1B + 256 * cpustate->IO_MAR1H + cpustate->IO_MAR1L;
	offs_t iar1 = 256 * cpustate->IO_IAR1H + cpustate->IO_IAR1L;
	int bcr1 = 256 * cpustate->IO_BCR1H + cpustate->IO_BCR1L;

	if ((cpustate->iol & Z180_DREQ1) == 0)
		return;

	/* counter is zero? */
	if (bcr1 == 0)
	{
		cpustate->IO_DSTAT &= ~Z180_DSTAT_DE1;
		return;
	}

	/* last transfer happening now? */
	if (bcr1 == 1)
	{
		cpustate->iol |= Z180_TEND1;
	}

	switch (cpustate->IO_DCNTL & (Z180_DCNTL_DIM1 | Z180_DCNTL_DIM0))
	{
	case 0x00:	/* memory MAR1+1 to I/O IAR1 fixed */
		memory_write_byte_8le(cpustate->iospace, iar1, memory_read_byte_8le(cpustate->program, mar1++));
		break;
	case 0x01:	/* memory MAR1-1 to I/O IAR1 fixed */
		memory_write_byte_8le(cpustate->iospace, iar1, memory_read_byte_8le(cpustate->program, mar1--));
		break;
	case 0x02:	/* I/O IAR1 fixed to memory MAR1+1 */
		memory_write_byte_8le(cpustate->program, mar1++, memory_read_byte_8le(cpustate->iospace, iar1));
		break;
	case 0x03:	/* I/O IAR1 fixed to memory MAR1-1 */
		memory_write_byte_8le(cpustate->program, mar1--, memory_read_byte_8le(cpustate->iospace, iar1));
		break;
	}

	/* edge sensitive DREQ1 ? */
	if (cpustate->IO_DCNTL & Z180_DCNTL_DIM1)
		cpustate->iol &= ~Z180_DREQ1;

	cpustate->IO_MAR1L = mar1;
	cpustate->IO_MAR1H = mar1 >> 8;
	cpustate->IO_MAR1B = mar1 >> 16;
	cpustate->IO_BCR1L = bcr1;
	cpustate->IO_BCR1H = bcr1 >> 8;

	/* DMA terminal count? */
	if (bcr1 == 0)
	{
		cpustate->iol &= ~Z180_TEND1;
		cpustate->IO_DSTAT &= ~Z180_DSTAT_DE1;
		if (cpustate->IO_DSTAT & Z180_DSTAT_DIE1 && cpustate->IFF1)
			take_interrupt(cpustate, Z180_INT_DMA1);
	}

	/* six cycles per transfer (minimum) */
	cpustate->icount -= 6;
}

static void z180_write_iolines(z180_state *cpustate, UINT32 data)
{
	UINT32 changes = cpustate->iol ^ data;

    /* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
	if (changes & Z180_CKA0)
	{
		LOG(("Z180 '%s' CKA0   %d\n", cpustate->device->tag, data & Z180_CKA0 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_CKA0) | (data & Z180_CKA0);
    }

    /* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
	if (changes & Z180_CKA1)
	{
		LOG(("Z180 '%s' CKA1   %d\n", cpustate->device->tag, data & Z180_CKA1 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_CKA1) | (data & Z180_CKA1);
    }

    /* I/O serial clock (active high) */
	if (changes & Z180_CKS)
	{
		LOG(("Z180 '%s' CKS    %d\n", cpustate->device->tag, data & Z180_CKS ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_CKS) | (data & Z180_CKS);
    }

    /* I   clear to send 0 (active low) */
	if (changes & Z180_CTS0)
	{
		LOG(("Z180 '%s' CTS0   %d\n", cpustate->device->tag, data & Z180_CTS0 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_CTS0) | (data & Z180_CTS0);
    }

    /* I   clear to send 1 (active low) or RXS (mux) */
	if (changes & Z180_CTS1)
	{
		LOG(("Z180 '%s' CTS1   %d\n", cpustate->device->tag, data & Z180_CTS1 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_CTS1) | (data & Z180_CTS1);
    }

    /* I   data carrier detect (active low) */
	if (changes & Z180_DCD0)
	{
		LOG(("Z180 '%s' DCD0   %d\n", cpustate->device->tag, data & Z180_DCD0 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_DCD0) | (data & Z180_DCD0);
    }

    /* I   data request DMA ch 0 (active low) or CKA0 (mux) */
	if (changes & Z180_DREQ0)
	{
		LOG(("Z180 '%s' DREQ0  %d\n", cpustate->device->tag, data & Z180_DREQ0 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_DREQ0) | (data & Z180_DREQ0);
    }

    /* I   data request DMA ch 1 (active low) */
	if (changes & Z180_DREQ1)
	{
		LOG(("Z180 '%s' DREQ1  %d\n", cpustate->device->tag, data & Z180_DREQ1 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_DREQ1) | (data & Z180_DREQ1);
    }

    /* I   asynchronous receive data 0 (active high) */
	if (changes & Z180_RXA0)
	{
		LOG(("Z180 '%s' RXA0   %d\n", cpustate->device->tag, data & Z180_RXA0 ? 1 : 0));
        cpustate->iol = (cpustate->iol & ~Z180_RXA0) | (data & Z180_RXA0);
    }

    /* I   asynchronous receive data 1 (active high) */
	if (changes & Z180_RXA1)
	{
		LOG(("Z180 '%s' RXA1   %d\n", cpustate->device->tag, data & Z180_RXA1 ? 1 : 0));
		cpustate->iol = (cpustate->iol & ~Z180_RXA1) | (data & Z180_RXA1);
    }

    /* I   clocked serial receive data (active high) or CTS1 (mux) */
	if (changes & Z180_RXS)
	{
		LOG(("Z180 '%s' RXS    %d\n", cpustate->device->tag, data & Z180_RXS ? 1 : 0));
        cpustate->iol = (cpustate->iol & ~Z180_RXS) | (data & Z180_RXS);
    }

    /*   O request to send (active low) */
	if (changes & Z180_RTS0)
	{
		LOG(("Z180 '%s' RTS0   won't change output\n", cpustate->device->tag));
    }

    /*   O transfer end 0 (active low) or CKA1 (mux) */
	if (changes & Z180_TEND0)
	{
		LOG(("Z180 '%s' TEND0  won't change output\n", cpustate->device->tag));
    }

    /*   O transfer end 1 (active low) */
	if (changes & Z180_TEND1)
	{
		LOG(("Z180 '%s' TEND1  won't change output\n", cpustate->device->tag));
    }

    /*   O transfer out (PRT channel, active low) or A18 (mux) */
	if (changes & Z180_A18_TOUT)
	{
		LOG(("Z180 '%s' TOUT   won't change output\n", cpustate->device->tag));
    }

    /*   O asynchronous transmit data 0 (active high) */
	if (changes & Z180_TXA0)
	{
		LOG(("Z180 '%s' TXA0   won't change output\n", cpustate->device->tag));
    }

    /*   O asynchronous transmit data 1 (active high) */
	if (changes & Z180_TXA1)
	{
		LOG(("Z180 '%s' TXA1   won't change output\n", cpustate->device->tag));
    }

    /*   O clocked serial transmit data (active high) */
	if (changes & Z180_TXS)
	{
		LOG(("Z180 '%s' TXS    won't change output\n", cpustate->device->tag));
    }
}


static CPU_INIT( z180 )
{
	z180_state *cpustate = get_safe_token(device);
	cpustate->daisy = NULL;
	if (device->static_config)
		cpustate->daisy = z80daisy_init(device, (const z80_daisy_chain *)device->static_config);
	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->iospace = memory_find_address_space(device, ADDRESS_SPACE_IO);

	/* set up the state table */
	cpustate->state = state_table_template;
	cpustate->state.baseptr = cpustate;
	cpustate->state.subtypemask = 1;

	state_save_register_device_item(device, 0, cpustate->AF.w.l);
	state_save_register_device_item(device, 0, cpustate->BC.w.l);
	state_save_register_device_item(device, 0, cpustate->DE.w.l);
	state_save_register_device_item(device, 0, cpustate->HL.w.l);
	state_save_register_device_item(device, 0, cpustate->IX.w.l);
	state_save_register_device_item(device, 0, cpustate->IY.w.l);
	state_save_register_device_item(device, 0, cpustate->PC.w.l);
	state_save_register_device_item(device, 0, cpustate->SP.w.l);
	state_save_register_device_item(device, 0, cpustate->AF2.w.l);
	state_save_register_device_item(device, 0, cpustate->BC2.w.l);
	state_save_register_device_item(device, 0, cpustate->DE2.w.l);
	state_save_register_device_item(device, 0, cpustate->HL2.w.l);
	state_save_register_device_item(device, 0, cpustate->R);
	state_save_register_device_item(device, 0, cpustate->R2);
	state_save_register_device_item(device, 0, cpustate->IFF1);
	state_save_register_device_item(device, 0, cpustate->IFF2);
	state_save_register_device_item(device, 0, cpustate->HALT);
	state_save_register_device_item(device, 0, cpustate->IM);
	state_save_register_device_item(device, 0, cpustate->I);
	state_save_register_device_item(device, 0, cpustate->nmi_state);
	state_save_register_device_item(device, 0, cpustate->nmi_pending);
	state_save_register_device_item(device, 0, cpustate->irq_state[0]);
	state_save_register_device_item(device, 0, cpustate->irq_state[1]);
	state_save_register_device_item(device, 0, cpustate->irq_state[2]);
	state_save_register_device_item(device, 0, cpustate->after_EI);
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( z180 )
{
	z180_state *cpustate = get_safe_token(device);
	z80_daisy_state *save_daisy;
	cpu_irq_callback save_irqcallback;
	cpu_state_table save_table;
	int i, p;
	int oldval, newval, val;
	UINT8 *padd, *padc, *psub, *psbc;
	/* allocate big flag arrays once */
	SZHVC_add = auto_alloc_array(device->machine, UINT8, 2*256*256);
	SZHVC_sub = auto_alloc_array(device->machine, UINT8, 2*256*256);
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

	save_daisy = cpustate->daisy;
	save_irqcallback = cpustate->irq_callback;
	save_table = cpustate->state;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->daisy = save_daisy;
	cpustate->irq_callback = save_irqcallback;
	cpustate->state = save_table;
	cpustate->device = device;
	cpustate->program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);
	cpustate->iospace = memory_find_address_space(device, ADDRESS_SPACE_IO);
	cpustate->_IX = cpustate->_IY = 0xffff; /* IX and IY are FFFF after a reset! */
	cpustate->_F = ZF;			/* Zero flag is set */
	cpustate->nmi_state = CLEAR_LINE;
	cpustate->nmi_pending = 0;
	cpustate->irq_state[0] = CLEAR_LINE;
	cpustate->irq_state[1] = CLEAR_LINE;
	cpustate->irq_state[2] = CLEAR_LINE;
	cpustate->after_EI = 0;
	cpustate->tif[0] = 0;
	cpustate->tif[1] = 0;
	cpustate->read_tcr_tmdr[0] = 0;
	cpustate->read_tcr_tmdr[1] = 0;
	cpustate->tmdr_value[0] = 0xffff;
	cpustate->tmdr_value[1] = 0xffff;
	cpustate->device = device;

	/* reset io registers */
	cpustate->IO_CNTLA0  = Z180_CNTLA0_RESET;
	cpustate->IO_CNTLA1  = Z180_CNTLA1_RESET;
	cpustate->IO_CNTLB0  = Z180_CNTLB0_RESET;
	cpustate->IO_CNTLB1  = Z180_CNTLB1_RESET;
	cpustate->IO_STAT0   = Z180_STAT0_RESET;
	cpustate->IO_STAT1   = Z180_STAT1_RESET;
	cpustate->IO_TDR0    = Z180_TDR0_RESET;
	cpustate->IO_TDR1    = Z180_TDR1_RESET;
	cpustate->IO_RDR0    = Z180_RDR0_RESET;
	cpustate->IO_RDR1    = Z180_RDR1_RESET;
	cpustate->IO_CNTR    = Z180_CNTR_RESET;
	cpustate->IO_TRDR    = Z180_TRDR_RESET;
	cpustate->IO_TMDR0L  = Z180_TMDR0L_RESET;
	cpustate->IO_TMDR0H  = Z180_TMDR0H_RESET;
	cpustate->IO_RLDR0L  = Z180_RLDR0L_RESET;
	cpustate->IO_RLDR0H  = Z180_RLDR0H_RESET;
	cpustate->IO_TCR	   = Z180_TCR_RESET;
	cpustate->IO_IO11    = Z180_IO11_RESET;
	cpustate->IO_ASEXT0  = Z180_ASEXT0_RESET;
	cpustate->IO_ASEXT1  = Z180_ASEXT1_RESET;
	cpustate->IO_TMDR1L  = Z180_TMDR1L_RESET;
	cpustate->IO_TMDR1H  = Z180_TMDR1H_RESET;
	cpustate->IO_RLDR1L  = Z180_RLDR1L_RESET;
	cpustate->IO_RLDR1H  = Z180_RLDR1H_RESET;
	cpustate->IO_FRC	   = Z180_FRC_RESET;
	cpustate->IO_IO19    = Z180_IO19_RESET;
	cpustate->IO_ASTC0L  = Z180_ASTC0L_RESET;
	cpustate->IO_ASTC0H  = Z180_ASTC0H_RESET;
	cpustate->IO_ASTC1L  = Z180_ASTC1L_RESET;
	cpustate->IO_ASTC1H  = Z180_ASTC1H_RESET;
	cpustate->IO_CMR	   = Z180_CMR_RESET;
	cpustate->IO_CCR	   = Z180_CCR_RESET;
	cpustate->IO_SAR0L   = Z180_SAR0L_RESET;
	cpustate->IO_SAR0H   = Z180_SAR0H_RESET;
	cpustate->IO_SAR0B   = Z180_SAR0B_RESET;
	cpustate->IO_DAR0L   = Z180_DAR0L_RESET;
	cpustate->IO_DAR0H   = Z180_DAR0H_RESET;
	cpustate->IO_DAR0B   = Z180_DAR0B_RESET;
	cpustate->IO_BCR0L   = Z180_BCR0L_RESET;
	cpustate->IO_BCR0H   = Z180_BCR0H_RESET;
	cpustate->IO_MAR1L   = Z180_MAR1L_RESET;
	cpustate->IO_MAR1H   = Z180_MAR1H_RESET;
	cpustate->IO_MAR1B   = Z180_MAR1B_RESET;
	cpustate->IO_IAR1L   = Z180_IAR1L_RESET;
	cpustate->IO_IAR1H   = Z180_IAR1H_RESET;
	cpustate->IO_IAR1B   = Z180_IAR1B_RESET;
	cpustate->IO_BCR1L   = Z180_BCR1L_RESET;
	cpustate->IO_BCR1H   = Z180_BCR1H_RESET;
	cpustate->IO_DSTAT   = Z180_DSTAT_RESET;
	cpustate->IO_DMODE   = Z180_DMODE_RESET;
	cpustate->IO_DCNTL   = Z180_DCNTL_RESET;
	cpustate->IO_IL	   = Z180_IL_RESET;
	cpustate->IO_ITC	   = Z180_ITC_RESET;
	cpustate->IO_IO35    = Z180_IO35_RESET;
	cpustate->IO_RCR	   = Z180_RCR_RESET;
	cpustate->IO_IO37    = Z180_IO37_RESET;
	cpustate->IO_CBR	   = Z180_CBR_RESET;
	cpustate->IO_BBR	   = Z180_BBR_RESET;
	cpustate->IO_CBAR    = Z180_CBAR_RESET;
	cpustate->IO_IO3B    = Z180_IO3B_RESET;
	cpustate->IO_IO3C    = Z180_IO3C_RESET;
	cpustate->IO_IO3D    = Z180_IO3D_RESET;
	cpustate->IO_OMCR    = Z180_OMCR_RESET;
	cpustate->IO_IOCR    = Z180_IOCR_RESET;

	if (cpustate->daisy)
		z80daisy_reset(cpustate->daisy);
	z180_mmu(cpustate);
}

/* Handle PRT timers, decreasing them after 20 clocks and returning the new icount base that needs to be used for the next check */
static int handle_timers(z180_state *cpustate, int current_icount, int previous_icount)
{
	int diff = previous_icount - current_icount;
	int new_icount_base;

	if(diff >= 20)
	{
		/* Programmable Reload Timer 0 */
		if(cpustate->IO_TCR & Z180_TCR_TDE0)
		{
			cpustate->tmdr_value[0]--;
			if(cpustate->tmdr_value[0] == 0)
			{
				cpustate->tmdr_value[0] = cpustate->IO_RLDR0L | (cpustate->IO_RLDR0H << 8);
				cpustate->tif[0] = 1;
			}
		}

		/* Programmable Reload Timer 1 */
		if(cpustate->IO_TCR & Z180_TCR_TDE1)
		{
			cpustate->tmdr_value[1]--;
			if(cpustate->tmdr_value[1] == 0)
			{
				cpustate->tmdr_value[1] = cpustate->IO_RLDR1L | (cpustate->IO_RLDR1H << 8);
				cpustate->tif[1] = 1;
			}
		}

		if((cpustate->IO_TCR & Z180_TCR_TIE0) && cpustate->tif[0])
		{
			// check if we can take the interrupt
			if(cpustate->IFF1 && !cpustate->after_EI)
			{
				take_interrupt(cpustate, Z180_INT_PRT0);
			}
		}

		if((cpustate->IO_TCR & Z180_TCR_TIE1) && cpustate->tif[1])
		{
			// check if we can take the interrupt
			if(cpustate->IFF1 && !cpustate->after_EI)
			{
				take_interrupt(cpustate, Z180_INT_PRT1);
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

static void check_interrupts(z180_state *cpustate)
{
	int i;
	for(i = 0; i <= 2; i++)
	{
		/* check for IRQs before each instruction */
		if(cpustate->irq_state[i] != CLEAR_LINE && cpustate->IFF1 && !cpustate->after_EI)
		{
			take_interrupt(cpustate, Z180_INT0 + i);
		}
	}
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
static CPU_EXECUTE( z180 )
{
	z180_state *cpustate = get_safe_token(device);
	int old_icount = cycles;
	cpustate->icount = cycles;

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (cpustate->nmi_pending)
	{
		LOG(("Z180 '%s' take NMI\n", cpustate->device->tag));
		cpustate->_PPC = -1;			/* there isn't a valid previous program counter */
		LEAVE_HALT(cpustate); 		/* Check if processor was halted */

		/* disable DMA transfers!! */
		cpustate->IO_DSTAT &= ~Z180_DSTAT_DME;

		cpustate->IFF2 = cpustate->IFF1;
		cpustate->IFF1 = 0;
		PUSH(cpustate,  PC );
		cpustate->_PCD = 0x0066;
		cpustate->icount -= 11;
		cpustate->nmi_pending = 0;
	}

again:
    /* check if any DMA transfer is running */
	if ((cpustate->IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
	{
		/* check if DMA channel 0 is running and also is in burst mode */
		if ((cpustate->IO_DSTAT & Z180_DSTAT_DE0) == Z180_DSTAT_DE0 &&
			(cpustate->IO_DMODE & Z180_DMODE_MMOD) == Z180_DMODE_MMOD)
		{
			debugger_instruction_hook(device, cpustate->_PCD);

			z180_dma0(cpustate);
			old_icount = handle_timers(cpustate, cpustate->icount, old_icount);
		}
		else
		{
			do
			{
				check_interrupts(cpustate);
				cpustate->after_EI = 0;

				cpustate->_PPC = cpustate->_PCD;
				debugger_instruction_hook(device, cpustate->_PCD);
				cpustate->R++;

				EXEC_INLINE(op,ROP(cpustate));
				old_icount = handle_timers(cpustate, cpustate->icount, old_icount);

				z180_dma0(cpustate);
				old_icount = handle_timers(cpustate, cpustate->icount, old_icount);

				z180_dma1(cpustate);
				old_icount = handle_timers(cpustate, cpustate->icount, old_icount);

				/* If DMA is done break out to the faster loop */
				if ((cpustate->IO_DSTAT & Z180_DSTAT_DME) != Z180_DSTAT_DME)
					break;
			} while( cpustate->icount > 0 );
		}
    }

    if (cpustate->icount > 0)
    {
        do
		{
			check_interrupts(cpustate);
			cpustate->after_EI = 0;

			cpustate->_PPC = cpustate->_PCD;
			debugger_instruction_hook(device, cpustate->_PCD);
			cpustate->R++;
			EXEC_INLINE(op,ROP(cpustate));
			old_icount = handle_timers(cpustate, cpustate->icount, old_icount);

			/* If DMA is started go to check the mode */
			if ((cpustate->IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
				goto again;
        } while( cpustate->icount > 0 );
	}

	return cycles - cpustate->icount;
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
static CPU_BURN( z180 )
{
	z180_state *cpustate = get_safe_token(device);
	if( cycles > 0 )
	{
		/* NOP takes 3 cycles per instruction */
		int n = (cycles + 2) / 3;
		cpustate->R += n;
		cpustate->icount -= 3 * n;
	}
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
static void set_irq_line(z180_state *cpustate, int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* mark an NMI pending on the rising edge */
		if (cpustate->nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			cpustate->nmi_pending = 1;
		cpustate->nmi_state = state;
	}
	else
	{
		LOG(("Z180 '%s' set_irq_line %d = %d\n",cpustate->device->tag , irqline,state));

		/* update the IRQ state */
		cpustate->irq_state[irqline] = state;
		if (cpustate->daisy)
			cpustate->irq_state[0] = z80daisy_update_irq_state(cpustate->daisy);

		/* the main execute loop will take the interrupt */
	}
}

/* logical to physical address translation */
static CPU_TRANSLATE( z180 )
{
	if (space == ADDRESS_SPACE_PROGRAM)
	{
		z180_state *cpustate = get_safe_token(device);
		*address = MMU_REMAP_ADDR(cpustate, *address);
	}
	return TRUE;
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

static CPU_IMPORT_STATE( z180 )
{
	z180_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case Z180_R:
			cpustate->R = cpustate->rtemp & 0x7f;
			cpustate->R2 = cpustate->rtemp & 0x80;
			break;

		case Z180_CBR:
		case Z180_BBR:
		case Z180_CBAR:
			z180_mmu(cpustate);
			break;

		case Z180_IOLINES:
			z180_write_iolines(cpustate, cpustate->ioltemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z80) called for unexpected value\n");
			break;
	}
}


static CPU_EXPORT_STATE( z180 )
{
	z180_state *cpustate = get_safe_token(device);

	switch (entry->index)
	{
		case Z180_R:
			cpustate->rtemp = (cpustate->R & 0x7f) | (cpustate->R2 & 0x80);
			break;

		case Z180_IOLINES:
			cpustate->ioltemp = cpustate->iol;
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z80) called for unexpected value\n");
			break;
	}
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( z180 )
{
	z180_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	set_irq_line(cpustate, INPUT_LINE_NMI, info->i);	break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT0:		set_irq_line(cpustate, Z180_INT0, info->i);			break;

		/* --- the following bits of info are set as pointers to data or functions --- */
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_op: 		cc[Z180_TABLE_op] = (const UINT8 *)info->p;				break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_cb: 		cc[Z180_TABLE_cb] = (const UINT8 *)info->p;				break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ed: 		cc[Z180_TABLE_ed] = (const UINT8 *)info->p;				break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xy: 		cc[Z180_TABLE_xy] = (const UINT8 *)info->p;				break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xycb: 	cc[Z180_TABLE_xycb] = (const UINT8 *)info->p;				break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ex: 		cc[Z180_TABLE_ex] = (const UINT8 *)info->p;				break;
	}
}


/**************************************************************************
 * Generic get_info
 **************************************************************************/

CPU_GET_INFO( z180 )
{
	z180_state *cpustate = (device != NULL && device->token != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(z180_state);			break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 3;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0xff;							break;
		case DEVINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;			break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 4;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH_PROGRAM:			info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_PROGRAM: 		info->i = 20;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_PROGRAM: 		info->i = 0;							break;
		case CPUINFO_INT_DATABUS_WIDTH_IO:				info->i = 8;							break;
		case CPUINFO_INT_ADDRBUS_WIDTH_IO: 				info->i = 16;							break;
		case CPUINFO_INT_ADDRBUS_SHIFT_IO: 				info->i = 0;							break;

		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_NMI:	info->i = cpustate->nmi_state;			break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT0:		info->i = cpustate->irq_state[0];		break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT1:		info->i = cpustate->irq_state[1];		break;
		case CPUINFO_INT_INPUT_STATE + Z180_INT2:		info->i = cpustate->irq_state[2];		break;

		/* --- the following bits of info are returned as pointers --- */
		case CPUINFO_FCT_SET_INFO:		info->setinfo = CPU_SET_INFO_NAME(z180);				break;
		case CPUINFO_FCT_INIT:			info->init = CPU_INIT_NAME(z180);						break;
		case CPUINFO_FCT_RESET:			info->reset = CPU_RESET_NAME(z180);						break;
		case CPUINFO_FCT_EXECUTE:		info->execute = CPU_EXECUTE_NAME(z180);					break;
		case CPUINFO_FCT_BURN:			info->burn = CPU_BURN_NAME(z180);						break;
		case CPUINFO_FCT_DISASSEMBLE:	info->disassemble = CPU_DISASSEMBLE_NAME(z180);			break;
		case CPUINFO_FCT_TRANSLATE:		info->translate = CPU_TRANSLATE_NAME(z180);				break;
		case CPUINFO_FCT_IMPORT_STATE:	info->import_state = CPU_IMPORT_STATE_NAME(z180);		break;
		case CPUINFO_FCT_EXPORT_STATE:	info->export_state = CPU_EXPORT_STATE_NAME(z180);		break;

		/* --- the following bits of info are returned as pointers to functions --- */
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;
		case CPUINFO_PTR_STATE_TABLE:					info->state_table = &cpustate->state;	break;

		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_op: 		info->p = (void *)cc[Z180_TABLE_op];	break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_cb: 		info->p = (void *)cc[Z180_TABLE_cb];	break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ed: 		info->p = (void *)cc[Z180_TABLE_ed];	break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xy: 		info->p = (void *)cc[Z180_TABLE_xy];	break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_xycb:	info->p = (void *)cc[Z180_TABLE_xycb];	break;
		case CPUINFO_PTR_Z180_CYCLE_TABLE + Z180_TABLE_ex: 		info->p = (void *)cc[Z180_TABLE_ex];	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case DEVINFO_STR_NAME:							strcpy(info->s, "Z180");				break;
		case DEVINFO_STR_FAMILY:					strcpy(info->s, "Zilog Z8x180");		break;
		case DEVINFO_STR_VERSION:					strcpy(info->s, "0.4");					break;
		case DEVINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case DEVINFO_STR_CREDITS:					strcpy(info->s, "Copyright Juergen Buchmueller, all rights reserved."); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c",
				cpustate->AF.b.l & 0x80 ? 'S':'.',
				cpustate->AF.b.l & 0x40 ? 'Z':'.',
				cpustate->AF.b.l & 0x20 ? '5':'.',
				cpustate->AF.b.l & 0x10 ? 'H':'.',
				cpustate->AF.b.l & 0x08 ? '3':'.',
				cpustate->AF.b.l & 0x04 ? 'P':'.',
				cpustate->AF.b.l & 0x02 ? 'N':'.',
				cpustate->AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}
