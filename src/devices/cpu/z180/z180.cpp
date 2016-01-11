// license:BSD-3-Clause
// copyright-holders:Juergen Buchmueller
/*****************************************************************************
 *
 *   z180.c
 *   Portable Z180 emulator V0.3
 *
 *****************************************************************************/

/*****************************************************************************

    TODO:
        - HALT processing is not yet perfect. The manual states that
          during HALT, all dma and internal i/o incl. timers continue to
          work. Currently, only timers are implemented. Ideally, the
          burn_cycles routine would go away and halt processing be
          implemented in cpu_execute.
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

#include "emu.h"
#include "debugger.h"
#include "z180.h"

#define VERBOSE 0

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

/* interrupt priorities */
#define Z180_INT_TRAP   0           /* Undefined opcode */
#define Z180_INT_NMI    1           /* NMI */
#define Z180_INT_IRQ0   2           /* Execute IRQ1 */
#define Z180_INT_IRQ1   3           /* Execute IRQ1 */
#define Z180_INT_IRQ2   4           /* Execute IRQ2 */
#define Z180_INT_PRT0   5           /* Internal PRT channel 0 */
#define Z180_INT_PRT1   6           /* Internal PRT channel 1 */
#define Z180_INT_DMA0   7           /* Internal DMA channel 0 */
#define Z180_INT_DMA1   8           /* Internal DMA channel 1 */
#define Z180_INT_CSIO   9           /* Internal CSI/O */
#define Z180_INT_ASCI0  10          /* Internal ASCI channel 0 */
#define Z180_INT_ASCI1  11          /* Internal ASCI channel 1 */
#define Z180_INT_MAX    Z180_INT_ASCI1

/****************************************************************************/
/* The Z180 registers. HALT is set to 1 when the CPU is halted, the refresh */
/* register is calculated as follows: refresh=(Regs.R&127)|(Regs.R2&128)    */
/****************************************************************************/

const device_type Z180 = &device_creator<z180_device>;


z180_device::z180_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, Z180, "Z180", tag, owner, clock, "z180", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 20, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 16, 0)
	, m_decrypted_opcodes_config("program", ENDIANNESS_LITTLE, 8, 20, 0)
{
}


offs_t z180_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( z180 );
	return CPU_DISASSEMBLE_NAME(z180)(this, buffer, pc, oprom, opram, options);
}


#define CF  0x01
#define NF  0x02
#define PF  0x04
#define VF  PF
#define XF  0x08
#define HF  0x10
#define YF  0x20
#define ZF  0x40
#define SF  0x80

/* I/O line status flags */
#define Z180_CKA0     0x00000001  /* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
#define Z180_CKA1     0x00000002  /* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
#define Z180_CKS      0x00000004  /* I/O serial clock (active high) */
#define Z180_CTS0     0x00000100  /* I   clear to send 0 (active low) */
#define Z180_CTS1     0x00000200  /* I   clear to send 1 (active low) or RXS (mux) */
#define Z180_DCD0     0x00000400  /* I   data carrier detect (active low) */
#define Z180_DREQ0    0x00000800  /* I   data request DMA ch 0 (active low) or CKA0 (mux) */
#define Z180_DREQ1    0x00001000  /* I   data request DMA ch 1 (active low) */
#define Z180_RXA0     0x00002000  /* I   asynchronous receive data 0 (active high) */
#define Z180_RXA1     0x00004000  /* I   asynchronous receive data 1 (active high) */
#define Z180_RXS      0x00008000  /* I   clocked serial receive data (active high) or CTS1 (mux) */
#define Z180_RTS0     0x00010000  /*   O request to send (active low) */
#define Z180_TEND0    0x00020000  /*   O transfer end 0 (active low) or CKA1 (mux) */
#define Z180_TEND1    0x00040000  /*   O transfer end 1 (active low) */
#define Z180_A18_TOUT 0x00080000  /*   O transfer out (PRT channel, active low) or A18 (mux) */
#define Z180_TXA0     0x00100000  /*   O asynchronous transmit data 0 (active high) */
#define Z180_TXA1     0x00200000  /*   O asynchronous transmit data 1 (active high) */
#define Z180_TXS      0x00400000  /*   O clocked serial transmit data (active high) */

/*
 * Prevent warnings on NetBSD.  All identifiers beginning with an underscore
 * followed by an uppercase letter are reserved by the C standard (ISO/IEC
 * 9899:1999, 7.1.3) to be used by the implementation.  It'd be best to rename
 * all such instances, but this is less intrusive and error-prone.
 */
#undef _B
#undef _C
#undef _L

#define _PPC    m_PREPC.d /* previous program counter */

#define _PCD    m_PC.d
#define _PC     m_PC.w.l

#define _SPD    m_SP.d
#define _SP     m_SP.w.l

#define _AFD    m_AF.d
#define _AF     m_AF.w.l
#define _A      m_AF.b.h
#define _F      m_AF.b.l

#define _BCD    m_BC.d
#define _BC     m_BC.w.l
#define _B      m_BC.b.h
#define _C      m_BC.b.l

#define _DED    m_DE.d
#define _DE     m_DE.w.l
#define _D      m_DE.b.h
#define _E      m_DE.b.l

#define _HLD    m_HL.d
#define _HL     m_HL.w.l
#define _H      m_HL.b.h
#define _L      m_HL.b.l

#define _IXD    m_IX.d
#define _IX     m_IX.w.l
#define _HX     m_IX.b.h
#define _LX     m_IX.b.l

#define _IYD    m_IY.d
#define _IY     m_IY.w.l
#define _HY     m_IY.b.h
#define _LY     m_IY.b.l

#define IO(n)       m_io[(n)-Z180_CNTLA0]
#define IO_CNTLA0   IO(Z180_CNTLA0)
#define IO_CNTLA1   IO(Z180_CNTLA1)
#define IO_CNTLB0   IO(Z180_CNTLB0)
#define IO_CNTLB1   IO(Z180_CNTLB1)
#define IO_STAT0    IO(Z180_STAT0)
#define IO_STAT1    IO(Z180_STAT1)
#define IO_TDR0     IO(Z180_TDR0)
#define IO_TDR1     IO(Z180_TDR1)
#define IO_RDR0     IO(Z180_RDR0)
#define IO_RDR1     IO(Z180_RDR1)
#define IO_CNTR     IO(Z180_CNTR)
#define IO_TRDR     IO(Z180_TRDR)
#define IO_TMDR0L   IO(Z180_TMDR0L)
#define IO_TMDR0H   IO(Z180_TMDR0H)
#define IO_RLDR0L   IO(Z180_RLDR0L)
#define IO_RLDR0H   IO(Z180_RLDR0H)
#define IO_TCR      IO(Z180_TCR)
#define IO_IO11     IO(Z180_IO11)
#define IO_ASEXT0   IO(Z180_ASEXT0)
#define IO_ASEXT1   IO(Z180_ASEXT1)
#define IO_TMDR1L   IO(Z180_TMDR1L)
#define IO_TMDR1H   IO(Z180_TMDR1H)
#define IO_RLDR1L   IO(Z180_RLDR1L)
#define IO_RLDR1H   IO(Z180_RLDR1H)
#define IO_FRC      IO(Z180_FRC)
#define IO_IO19     IO(Z180_IO19)
#define IO_ASTC0L   IO(Z180_ASTC0L)
#define IO_ASTC0H   IO(Z180_ASTC0H)
#define IO_ASTC1L   IO(Z180_ASTC1L)
#define IO_ASTC1H   IO(Z180_ASTC1H)
#define IO_CMR      IO(Z180_CMR)
#define IO_CCR      IO(Z180_CCR)
#define IO_SAR0L    IO(Z180_SAR0L)
#define IO_SAR0H    IO(Z180_SAR0H)
#define IO_SAR0B    IO(Z180_SAR0B)
#define IO_DAR0L    IO(Z180_DAR0L)
#define IO_DAR0H    IO(Z180_DAR0H)
#define IO_DAR0B    IO(Z180_DAR0B)
#define IO_BCR0L    IO(Z180_BCR0L)
#define IO_BCR0H    IO(Z180_BCR0H)
#define IO_MAR1L    IO(Z180_MAR1L)
#define IO_MAR1H    IO(Z180_MAR1H)
#define IO_MAR1B    IO(Z180_MAR1B)
#define IO_IAR1L    IO(Z180_IAR1L)
#define IO_IAR1H    IO(Z180_IAR1H)
#define IO_IAR1B    IO(Z180_IAR1B)
#define IO_BCR1L    IO(Z180_BCR1L)
#define IO_BCR1H    IO(Z180_BCR1H)
#define IO_DSTAT    IO(Z180_DSTAT)
#define IO_DMODE    IO(Z180_DMODE)
#define IO_DCNTL    IO(Z180_DCNTL)
#define IO_IL       IO(Z180_IL)
#define IO_ITC      IO(Z180_ITC)
#define IO_IO35     IO(Z180_IO35)
#define IO_RCR      IO(Z180_RCR)
#define IO_IO37     IO(Z180_IO37)
#define IO_CBR      IO(Z180_CBR)
#define IO_BBR      IO(Z180_BBR)
#define IO_CBAR     IO(Z180_CBAR)
#define IO_IO3B     IO(Z180_IO3B)
#define IO_IO3C     IO(Z180_IO3C)
#define IO_IO3D     IO(Z180_IO3D)
#define IO_OMCR     IO(Z180_OMCR)
#define IO_IOCR     IO(Z180_IOCR)

/* 00 ASCI control register A ch 0 */
#define Z180_CNTLA0_MPE         0x80
#define Z180_CNTLA0_RE          0x40
#define Z180_CNTLA0_TE          0x20
#define Z180_CNTLA0_RTS0        0x10
#define Z180_CNTLA0_MPBR_EFR    0x08
#define Z180_CNTLA0_MODE_DATA   0x04
#define Z180_CNTLA0_MODE_PARITY 0x02
#define Z180_CNTLA0_MODE_STOPB  0x01

#define Z180_CNTLA0_RESET       0x10
#define Z180_CNTLA0_RMASK       0xff
#define Z180_CNTLA0_WMASK       0xff

/* 01 ASCI control register A ch 1 */
#define Z180_CNTLA1_MPE         0x80
#define Z180_CNTLA1_RE          0x40
#define Z180_CNTLA1_TE          0x20
#define Z180_CNTLA1_CKA1D       0x10
#define Z180_CNTLA1_MPBR_EFR    0x08
#define Z180_CNTLA1_MODE        0x07

#define Z180_CNTLA1_RESET       0x10
#define Z180_CNTLA1_RMASK       0xff
#define Z180_CNTLA1_WMASK       0xff

/* 02 ASCI control register B ch 0 */
#define Z180_CNTLB0_MPBT        0x80
#define Z180_CNTLB0_MP          0x40
#define Z180_CNTLB0_CTS_PS      0x20
#define Z180_CNTLB0_PEO         0x10
#define Z180_CNTLB0_DR          0x08
#define Z180_CNTLB0_SS          0x07

#define Z180_CNTLB0_RESET       0x07
#define Z180_CNTLB0_RMASK       0xff
#define Z180_CNTLB0_WMASK       0xff

/* 03 ASCI control register B ch 1 */
#define Z180_CNTLB1_MPBT        0x80
#define Z180_CNTLB1_MP          0x40
#define Z180_CNTLB1_CTS_PS      0x20
#define Z180_CNTLB1_PEO         0x10
#define Z180_CNTLB1_DR          0x08
#define Z180_CNTLB1_SS          0x07

#define Z180_CNTLB1_RESET       0x07
#define Z180_CNTLB1_RMASK       0xff
#define Z180_CNTLB1_WMASK       0xff

/* 04 ASCI status register 0 */
#define Z180_STAT0_RDRF         0x80
#define Z180_STAT0_OVRN         0x40
#define Z180_STAT0_PE           0x20
#define Z180_STAT0_FE           0x10
#define Z180_STAT0_RIE          0x08
#define Z180_STAT0_DCD0         0x04
#define Z180_STAT0_TDRE         0x02
#define Z180_STAT0_TIE          0x01

#define Z180_STAT0_RESET        0x00
#define Z180_STAT0_RMASK        0xff
#define Z180_STAT0_WMASK        0x09

/* 05 ASCI status register 1 */
#define Z180_STAT1_RDRF         0x80
#define Z180_STAT1_OVRN         0x40
#define Z180_STAT1_PE           0x20
#define Z180_STAT1_FE           0x10
#define Z180_STAT1_RIE          0x08
#define Z180_STAT1_CTS1E        0x04
#define Z180_STAT1_TDRE         0x02
#define Z180_STAT1_TIE          0x01

#define Z180_STAT1_RESET        0x02
#define Z180_STAT1_RMASK        0xff
#define Z180_STAT1_WMASK        0x0d

/* 06 ASCI transmit data register 0 */
#define Z180_TDR0_TDR           0xff

#define Z180_TDR0_RESET         0x00
#define Z180_TDR0_RMASK         0xff
#define Z180_TDR0_WMASK         0xff

/* 07 ASCI transmit data register 1 */
#define Z180_TDR1_TDR           0xff

#define Z180_TDR1_RESET         0x00
#define Z180_TDR1_RMASK         0xff
#define Z180_TDR1_WMASK         0xff

/* 08 ASCI receive register 0 */
#define Z180_RDR0_RDR           0xff

#define Z180_RDR0_RESET         0x00
#define Z180_RDR0_RMASK         0xff
#define Z180_RDR0_WMASK         0xff

/* 09 ASCI receive register 1 */
#define Z180_RDR1_RDR           0xff

#define Z180_RDR1_RESET         0x00
#define Z180_RDR1_RMASK         0xff
#define Z180_RDR1_WMASK         0xff

/* 0a CSI/O control/status register */
#define Z180_CNTR_EF            0x80
#define Z180_CNTR_EIE           0x40
#define Z180_CNTR_RE            0x20
#define Z180_CNTR_TE            0x10
#define Z180_CNTR_SS            0x07

#define Z180_CNTR_RESET         0x07
#define Z180_CNTR_RMASK         0xff
#define Z180_CNTR_WMASK         0x7f

/* 0b CSI/O transmit/receive register */
#define Z180_TRDR_RESET         0x00
#define Z180_TRDR_RMASK         0xff
#define Z180_TRDR_WMASK         0xff

/* 0c TIMER data register ch 0 L */
#define Z180_TMDR0L_RESET       0x00
#define Z180_TMDR0L_RMASK       0xff
#define Z180_TMDR0L_WMASK       0xff

/* 0d TIMER data register ch 0 H */
#define Z180_TMDR0H_RESET       0x00
#define Z180_TMDR0H_RMASK       0xff
#define Z180_TMDR0H_WMASK       0xff

/* 0e TIMER reload register ch 0 L */
#define Z180_RLDR0L_RESET       0xff
#define Z180_RLDR0L_RMASK       0xff
#define Z180_RLDR0L_WMASK       0xff

/* 0f TIMER reload register ch 0 H */
#define Z180_RLDR0H_RESET       0xff
#define Z180_RLDR0H_RMASK       0xff
#define Z180_RLDR0H_WMASK       0xff

/* 10 TIMER control register */
#define Z180_TCR_TIF1           0x80
#define Z180_TCR_TIF0           0x40
#define Z180_TCR_TIE1           0x20
#define Z180_TCR_TIE0           0x10
#define Z180_TCR_TOC1           0x08
#define Z180_TCR_TOC0           0x04
#define Z180_TCR_TDE1           0x02
#define Z180_TCR_TDE0           0x01

#define Z180_TCR_RESET          0x00
#define Z180_TCR_RMASK          0xff
#define Z180_TCR_WMASK          0x3f

/* 11 reserved */
#define Z180_IO11_RESET         0x00
#define Z180_IO11_RMASK         0xff
#define Z180_IO11_WMASK         0xff

/* 12 (Z8S180/Z8L180) ASCI extension control register 0 */
#define Z180_ASEXT0_RDRF        0x80
#define Z180_ASEXT0_DCD0        0x40
#define Z180_ASEXT0_CTS0        0x20
#define Z180_ASEXT0_X1_BIT_CLK0 0x10
#define Z180_ASEXT0_BRG0_MODE   0x08
#define Z180_ASEXT0_BRK_EN      0x04
#define Z180_ASEXT0_BRK_DET     0x02
#define Z180_ASEXT0_BRK_SEND    0x01

#define Z180_ASEXT0_RESET       0x00
#define Z180_ASEXT0_RMASK       0xff
#define Z180_ASEXT0_WMASK       0xfd

/* 13 (Z8S180/Z8L180) ASCI extension control register 0 */
#define Z180_ASEXT1_RDRF        0x80
#define Z180_ASEXT1_X1_BIT_CLK1 0x10
#define Z180_ASEXT1_BRG1_MODE   0x08
#define Z180_ASEXT1_BRK_EN      0x04
#define Z180_ASEXT1_BRK_DET     0x02
#define Z180_ASEXT1_BRK_SEND    0x01

#define Z180_ASEXT1_RESET       0x00
#define Z180_ASEXT1_RMASK       0xff
#define Z180_ASEXT1_WMASK       0xfd


/* 14 TIMER data register ch 1 L */
#define Z180_TMDR1L_RESET       0x00
#define Z180_TMDR1L_RMASK       0xff
#define Z180_TMDR1L_WMASK       0xff

/* 15 TIMER data register ch 1 H */
#define Z180_TMDR1H_RESET       0x00
#define Z180_TMDR1H_RMASK       0xff
#define Z180_TMDR1H_WMASK       0xff

/* 16 TIMER reload register ch 1 L */
#define Z180_RLDR1L_RESET       0x00
#define Z180_RLDR1L_RMASK       0xff
#define Z180_RLDR1L_WMASK       0xff

/* 17 TIMER reload register ch 1 H */
#define Z180_RLDR1H_RESET       0x00
#define Z180_RLDR1H_RMASK       0xff
#define Z180_RLDR1H_WMASK       0xff

/* 18 free running counter */
#define Z180_FRC_RESET          0x00
#define Z180_FRC_RMASK          0xff
#define Z180_FRC_WMASK          0xff

/* 19 reserved */
#define Z180_IO19_RESET         0x00
#define Z180_IO19_RMASK         0xff
#define Z180_IO19_WMASK         0xff

/* 1a ASCI time constant ch 0 L */
#define Z180_ASTC0L_RESET       0x00
#define Z180_ASTC0L_RMASK       0xff
#define Z180_ASTC0L_WMASK       0xff

/* 1b ASCI time constant ch 0 H */
#define Z180_ASTC0H_RESET       0x00
#define Z180_ASTC0H_RMASK       0xff
#define Z180_ASTC0H_WMASK       0xff

/* 1c ASCI time constant ch 1 L */
#define Z180_ASTC1L_RESET       0x00
#define Z180_ASTC1L_RMASK       0xff
#define Z180_ASTC1L_WMASK       0xff

/* 1d ASCI time constant ch 1 H */
#define Z180_ASTC1H_RESET       0x00
#define Z180_ASTC1H_RMASK       0xff
#define Z180_ASTC1H_WMASK       0xff

/* 1e clock multiplier */
#define Z180_CMR_X2             0x80

#define Z180_CMR_RESET          0x7f
#define Z180_CMR_RMASK          0x80
#define Z180_CMR_WMASK          0x80

/* 1f chip control register */
#define Z180_CCR_CLOCK_DIVIDE   0x80
#define Z180_CCR_STDBY_IDLE1    0x40
#define Z180_CCR_BREXT          0x20
#define Z180_CCR_LNPHI          0x10
#define Z180_CCR_STDBY_IDLE0    0x08
#define Z180_CCR_LNIO           0x04
#define Z180_CCR_LNCPU_CTL      0x02
#define Z180_CCR_LNAD_DATA      0x01

#define Z180_CCR_RESET          0x00
#define Z180_CCR_RMASK          0xff
#define Z180_CCR_WMASK          0xff

/* 20 DMA source address register ch 0 L */
#define Z180_SAR0L_SAR          0xff

#define Z180_SAR0L_RESET        0x00
#define Z180_SAR0L_RMASK        0xff
#define Z180_SAR0L_WMASK        0xff

/* 21 DMA source address register ch 0 H */
#define Z180_SAR0H_SAR          0xff

#define Z180_SAR0H_RESET        0x00
#define Z180_SAR0H_RMASK        0xff
#define Z180_SAR0H_WMASK        0xff

/* 22 DMA source address register ch 0 B */
#define Z180_SAR0B_SAR          0x0f

#define Z180_SAR0B_RESET        0x00
#define Z180_SAR0B_RMASK        0x0f
#define Z180_SAR0B_WMASK        0x0f

/* 23 DMA destination address register ch 0 L */
#define Z180_DAR0L_DAR          0xff

#define Z180_DAR0L_RESET        0x00
#define Z180_DAR0L_RMASK        0xff
#define Z180_DAR0L_WMASK        0xff

/* 24 DMA destination address register ch 0 H */
#define Z180_DAR0H_DAR          0xff

#define Z180_DAR0H_RESET        0x00
#define Z180_DAR0H_RMASK        0xff
#define Z180_DAR0H_WMASK        0xff

/* 25 DMA destination address register ch 0 B */
#define Z180_DAR0B_DAR          0x00

#define Z180_DAR0B_RESET        0x00
#define Z180_DAR0B_RMASK        0x0f
#define Z180_DAR0B_WMASK        0x0f

/* 26 DMA byte count register ch 0 L */
#define Z180_BCR0L_BCR          0xff

#define Z180_BCR0L_RESET        0x00
#define Z180_BCR0L_RMASK        0xff
#define Z180_BCR0L_WMASK        0xff

/* 27 DMA byte count register ch 0 H */
#define Z180_BCR0H_BCR          0xff

#define Z180_BCR0H_RESET        0x00
#define Z180_BCR0H_RMASK        0xff
#define Z180_BCR0H_WMASK        0xff

/* 28 DMA memory address register ch 1 L */
#define Z180_MAR1L_MAR          0xff

#define Z180_MAR1L_RESET        0x00
#define Z180_MAR1L_RMASK        0xff
#define Z180_MAR1L_WMASK        0xff

/* 29 DMA memory address register ch 1 H */
#define Z180_MAR1H_MAR          0xff

#define Z180_MAR1H_RESET        0x00
#define Z180_MAR1H_RMASK        0xff
#define Z180_MAR1H_WMASK        0xff

/* 2a DMA memory address register ch 1 B */
#define Z180_MAR1B_MAR          0x0f

#define Z180_MAR1B_RESET        0x00
#define Z180_MAR1B_RMASK        0x0f
#define Z180_MAR1B_WMASK        0x0f

/* 2b DMA I/O address register ch 1 L */
#define Z180_IAR1L_IAR          0xff

#define Z180_IAR1L_RESET        0x00
#define Z180_IAR1L_RMASK        0xff
#define Z180_IAR1L_WMASK        0xff

/* 2c DMA I/O address register ch 1 H */
#define Z180_IAR1H_IAR          0xff

#define Z180_IAR1H_RESET        0x00
#define Z180_IAR1H_RMASK        0xff
#define Z180_IAR1H_WMASK        0xff

/* 2d (Z8S180/Z8L180) DMA I/O address register ch 1 B */
#define Z180_IAR1B_IAR          0x0f

#define Z180_IAR1B_RESET        0x00
#define Z180_IAR1B_RMASK        0x0f
#define Z180_IAR1B_WMASK        0x0f

/* 2e DMA byte count register ch 1 L */
#define Z180_BCR1L_BCR          0xff

#define Z180_BCR1L_RESET        0x00
#define Z180_BCR1L_RMASK        0xff
#define Z180_BCR1L_WMASK        0xff

/* 2f DMA byte count register ch 1 H */
#define Z180_BCR1H_BCR          0xff

#define Z180_BCR1H_RESET        0x00
#define Z180_BCR1H_RMASK        0xff
#define Z180_BCR1H_WMASK        0xff

/* 30 DMA status register */
#define Z180_DSTAT_DE1          0x80    /* DMA enable ch 1 */
#define Z180_DSTAT_DE0          0x40    /* DMA enable ch 0 */
#define Z180_DSTAT_DWE1         0x20    /* DMA write enable ch 0 (active low) */
#define Z180_DSTAT_DWE0         0x10    /* DMA write enable ch 1 (active low) */
#define Z180_DSTAT_DIE1         0x08    /* DMA IRQ enable ch 1 */
#define Z180_DSTAT_DIE0         0x04    /* DMA IRQ enable ch 0 */
#define Z180_DSTAT_DME          0x01    /* DMA enable (read only) */

#define Z180_DSTAT_RESET        0x30
#define Z180_DSTAT_RMASK        0xfd
#define Z180_DSTAT_WMASK        0xcc

/* 31 DMA mode register */
#define Z180_DMODE_DM           0x30
#define Z180_DMODE_SM           0x0c
#define Z180_DMODE_MMOD         0x04

#define Z180_DMODE_RESET        0x00
#define Z180_DMODE_RMASK        0x3e
#define Z180_DMODE_WMASK        0x3e

/* 32 DMA/WAIT control register */
#define Z180_DCNTL_MWI1         0x80
#define Z180_DCNTL_MWI0         0x40
#define Z180_DCNTL_IWI1         0x20
#define Z180_DCNTL_IWI0         0x10
#define Z180_DCNTL_DMS1         0x08
#define Z180_DCNTL_DMS0         0x04
#define Z180_DCNTL_DIM1         0x02
#define Z180_DCNTL_DIM0         0x01

#define Z180_DCNTL_RESET        0x00
#define Z180_DCNTL_RMASK        0xff
#define Z180_DCNTL_WMASK        0xff

/* 33 INT vector low register */
#define Z180_IL_IL              0xe0

#define Z180_IL_RESET           0x00
#define Z180_IL_RMASK           0xe0
#define Z180_IL_WMASK           0xe0

/* 34 INT/TRAP control register */
#define Z180_ITC_TRAP           0x80
#define Z180_ITC_UFO            0x40
#define Z180_ITC_ITE2           0x04
#define Z180_ITC_ITE1           0x02
#define Z180_ITC_ITE0           0x01

#define Z180_ITC_RESET          0x01
#define Z180_ITC_RMASK          0xc7
#define Z180_ITC_WMASK          0x87

/* 35 reserved */
#define Z180_IO35_RESET         0x00
#define Z180_IO35_RMASK         0xff
#define Z180_IO35_WMASK         0xff

/* 36 refresh control register */
#define Z180_RCR_REFE           0x80
#define Z180_RCR_REFW           0x80
#define Z180_RCR_CYC            0x03

#define Z180_RCR_RESET          0xc0
#define Z180_RCR_RMASK          0xc3
#define Z180_RCR_WMASK          0xc3

/* 37 reserved */
#define Z180_IO37_RESET         0x00
#define Z180_IO37_RMASK         0xff
#define Z180_IO37_WMASK         0xff

/* 38 MMU common base register */
#define Z180_CBR_CB             0xff

#define Z180_CBR_RESET          0x00
#define Z180_CBR_RMASK          0xff
#define Z180_CBR_WMASK          0xff

/* 39 MMU bank base register */
#define Z180_BBR_BB             0xff

#define Z180_BBR_RESET          0x00
#define Z180_BBR_RMASK          0xff
#define Z180_BBR_WMASK          0xff

/* 3a MMU common/bank area register */
#define Z180_CBAR_CA            0xf0
#define Z180_CBAR_BA            0x0f

#define Z180_CBAR_RESET         0xf0
#define Z180_CBAR_RMASK         0xff
#define Z180_CBAR_WMASK         0xff

/* 3b reserved */
#define Z180_IO3B_RESET         0x00
#define Z180_IO3B_RMASK         0xff
#define Z180_IO3B_WMASK         0xff

/* 3c reserved */
#define Z180_IO3C_RESET         0x00
#define Z180_IO3C_RMASK         0xff
#define Z180_IO3C_WMASK         0xff

/* 3d reserved */
#define Z180_IO3D_RESET         0x00
#define Z180_IO3D_RMASK         0xff
#define Z180_IO3D_WMASK         0xff

/* 3e operation mode control register */
#define Z180_OMCR_RESET         0x00
#define Z180_OMCR_RMASK         0xff
#define Z180_OMCR_WMASK         0xff

/* 3f I/O control register */
#define Z180_IOCR_RESET         0x00
#define Z180_IOCR_RMASK         0xff
#define Z180_IOCR_WMASK         0xff

/***************************************************************************
    CPU PREFIXES

    order is important here - see z180tbl.h
***************************************************************************/

#define Z180_PREFIX_op          0
#define Z180_PREFIX_cb          1
#define Z180_PREFIX_dd          2
#define Z180_PREFIX_ed          3
#define Z180_PREFIX_fd          4
#define Z180_PREFIX_xycb        5

#define Z180_PREFIX_COUNT       (Z180_PREFIX_xycb + 1)



static UINT8 SZ[256];       /* zero and sign flags */
static UINT8 SZ_BIT[256];   /* zero, sign and parity/overflow (=zero) flags for BIT opcode */
static UINT8 SZP[256];      /* zero, sign and parity flags */
static UINT8 SZHV_inc[256]; /* zero, sign, half carry and overflow flags INC r8 */
static UINT8 SZHV_dec[256]; /* zero, sign, half carry and overflow flags DEC r8 */

static std::unique_ptr<UINT8[]> SZHVC_add;
static std::unique_ptr<UINT8[]> SZHVC_sub;

#include "z180ops.h"
#include "z180tbl.h"

#include "z180cb.inc"
#include "z180xy.inc"
#include "z180dd.inc"
#include "z180fd.inc"
#include "z180ed.inc"
#include "z180op.inc"


const address_space_config *z180_device::memory_space_config(address_spacenum spacenum) const
{
	switch(spacenum)
	{
	case AS_PROGRAM:           return &m_program_config;
	case AS_IO:                return &m_io_config;
	case AS_DECRYPTED_OPCODES: return has_configured_map(AS_DECRYPTED_OPCODES) ? &m_decrypted_opcodes_config : nullptr;
	default:                   return nullptr;
	}
}

UINT8 z180_device::z180_readcontrol(offs_t port)
{
	/* normal external readport */
	UINT8 data = m_iospace->read_byte(port);

	/* remap internal I/O registers */
	if((port & (IO_IOCR & 0xc0)) == (IO_IOCR & 0xc0))
		port = port - (IO_IOCR & 0xc0);

	/* but ignore the data and read the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		data = IO_CNTLA0 & Z180_CNTLA0_RMASK;
		LOG(("Z180 '%s' CNTLA0 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CNTLA1:
		data = IO_CNTLA1 & Z180_CNTLA1_RMASK;
		LOG(("Z180 '%s' CNTLA1 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CNTLB0:
		data = IO_CNTLB0 & Z180_CNTLB0_RMASK;
		LOG(("Z180 '%s' CNTLB0 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CNTLB1:
		data = IO_CNTLB1 & Z180_CNTLB1_RMASK;
		LOG(("Z180 '%s' CNTLB1 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_STAT0:
		data = IO_STAT0 & Z180_STAT0_RMASK;
data |= 0x02; // kludge for 20pacgal
		LOG(("Z180 '%s' STAT0  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_STAT1:
		data = IO_STAT1 & Z180_STAT1_RMASK;
		LOG(("Z180 '%s' STAT1  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_TDR0:
		data = IO_TDR0 & Z180_TDR0_RMASK;
		LOG(("Z180 '%s' TDR0   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_TDR1:
		data = IO_TDR1 & Z180_TDR1_RMASK;
		LOG(("Z180 '%s' TDR1   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RDR0:
		data = IO_RDR0 & Z180_RDR0_RMASK;
		LOG(("Z180 '%s' RDR0   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RDR1:
		data = IO_RDR1 & Z180_RDR1_RMASK;
		LOG(("Z180 '%s' RDR1   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CNTR:
		data = IO_CNTR & Z180_CNTR_RMASK;
		data &= ~0x10; // Super Famicom Box sets the TE bit then wants it to be toggled after 8 bits transmitted
		LOG(("Z180 '%s' CNTR   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_TRDR:
		data = IO_TRDR & Z180_TRDR_RMASK;
		logerror("Z180 '%s' TRDR   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]);
		break;

	case Z180_TMDR0L:
		data = m_tmdr_value[0] & Z180_TMDR0L_RMASK;
		LOG(("Z180 '%s' TMDR0L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((IO_TCR & Z180_TCR_TDE0) == 0)
		{
			m_tmdr_latch |= 1;
			m_tmdrh[0] = (m_tmdr_value[0] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[0])
		{
			m_tif[0] = 0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}
		break;

	case Z180_TMDR0H:
		/* read latched value? */
		if (m_tmdr_latch & 1)
		{
			m_tmdr_latch &= ~1;
			data = m_tmdrh[0];
		}
		else
		{
			data = (m_tmdr_value[0] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[0])
		{
			m_tif[0] = 0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}
		LOG(("Z180 '%s' TMDR0H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RLDR0L:
		data = IO_RLDR0L & Z180_RLDR0L_RMASK;
		LOG(("Z180 '%s' RLDR0L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RLDR0H:
		data = IO_RLDR0H & Z180_RLDR0H_RMASK;
		LOG(("Z180 '%s' RLDR0H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_TCR:
		data = (IO_TCR & Z180_TCR_RMASK) | (m_tif[0] << 6) | (m_tif[1] << 7);

		if(m_read_tcr_tmdr[0])
		{
			m_tif[0] = 0; // reset TIF0
			m_read_tcr_tmdr[0] = 0;
		}
		else
		{
			m_read_tcr_tmdr[0] = 1;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tif[1] = 0; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}

		LOG(("Z180 '%s' TCR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO11:
		data = IO_IO11 & Z180_IO11_RMASK;
		LOG(("Z180 '%s' IO11   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASEXT0:
		data = IO_ASEXT0 & Z180_ASEXT0_RMASK;
		LOG(("Z180 '%s' ASEXT0 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASEXT1:
		data = IO_ASEXT1 & Z180_ASEXT1_RMASK;
		LOG(("Z180 '%s' ASEXT1 rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_TMDR1L:
		data = m_tmdr_value[1] & Z180_TMDR1L_RMASK;
		LOG(("Z180 '%s' TMDR1L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		/* if timer is counting, latch the MSB and set the latch flag */
		if ((IO_TCR & Z180_TCR_TDE1) == 0)
		{
			m_tmdr_latch |= 2;
			m_tmdrh[1] = (m_tmdr_value[1] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tif[1] = 0; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}
		break;

	case Z180_TMDR1H:
		/* read latched value? */
		if (m_tmdr_latch & 2)
		{
			m_tmdr_latch &= ~2;
			data = m_tmdrh[1];
		}
		else
		{
			data = (m_tmdr_value[1] & 0xff00) >> 8;
		}

		if(m_read_tcr_tmdr[1])
		{
			m_tif[1] = 0; // reset TIF1
			m_read_tcr_tmdr[1] = 0;
		}
		else
		{
			m_read_tcr_tmdr[1] = 1;
		}
		LOG(("Z180 '%s' TMDR1H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RLDR1L:
		data = IO_RLDR1L & Z180_RLDR1L_RMASK;
		LOG(("Z180 '%s' RLDR1L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RLDR1H:
		data = IO_RLDR1H & Z180_RLDR1H_RMASK;
		LOG(("Z180 '%s' RLDR1H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_FRC:
		data = IO_FRC & Z180_FRC_RMASK;
		LOG(("Z180 '%s' FRC    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO19:
		data = IO_IO19 & Z180_IO19_RMASK;
		LOG(("Z180 '%s' IO19   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASTC0L:
		data = IO_ASTC0L & Z180_ASTC0L_RMASK;
		LOG(("Z180 '%s' ASTC0L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASTC0H:
		data = IO_ASTC0H & Z180_ASTC0H_RMASK;
		LOG(("Z180 '%s' ASTC0H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASTC1L:
		data = IO_ASTC1L & Z180_ASTC1L_RMASK;
		LOG(("Z180 '%s' ASTC1L rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ASTC1H:
		data = IO_ASTC1H & Z180_ASTC1H_RMASK;
		LOG(("Z180 '%s' ASTC1H rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CMR:
		data = IO_CMR & Z180_CMR_RMASK;
		LOG(("Z180 '%s' CMR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CCR:
		data = IO_CCR & Z180_CCR_RMASK;
		LOG(("Z180 '%s' CCR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_SAR0L:
		data = IO_SAR0L & Z180_SAR0L_RMASK;
		LOG(("Z180 '%s' SAR0L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_SAR0H:
		data = IO_SAR0H & Z180_SAR0H_RMASK;
		LOG(("Z180 '%s' SAR0H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_SAR0B:
		data = IO_SAR0B & Z180_SAR0B_RMASK;
		LOG(("Z180 '%s' SAR0B  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DAR0L:
		data = IO_DAR0L & Z180_DAR0L_RMASK;
		LOG(("Z180 '%s' DAR0L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DAR0H:
		data = IO_DAR0H & Z180_DAR0H_RMASK;
		LOG(("Z180 '%s' DAR0H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DAR0B:
		data = IO_DAR0B & Z180_DAR0B_RMASK;
		LOG(("Z180 '%s' DAR0B  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_BCR0L:
		data = IO_BCR0L & Z180_BCR0L_RMASK;
		LOG(("Z180 '%s' BCR0L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_BCR0H:
		data = IO_BCR0H & Z180_BCR0H_RMASK;
		LOG(("Z180 '%s' BCR0H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_MAR1L:
		data = IO_MAR1L & Z180_MAR1L_RMASK;
		LOG(("Z180 '%s' MAR1L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_MAR1H:
		data = IO_MAR1H & Z180_MAR1H_RMASK;
		LOG(("Z180 '%s' MAR1H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_MAR1B:
		data = IO_MAR1B & Z180_MAR1B_RMASK;
		LOG(("Z180 '%s' MAR1B  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IAR1L:
		data = IO_IAR1L & Z180_IAR1L_RMASK;
		LOG(("Z180 '%s' IAR1L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IAR1H:
		data = IO_IAR1H & Z180_IAR1H_RMASK;
		LOG(("Z180 '%s' IAR1H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IAR1B:
		data = IO_IAR1B & Z180_IAR1B_RMASK;
		LOG(("Z180 '%s' IAR1B  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_BCR1L:
		data = IO_BCR1L & Z180_BCR1L_RMASK;
		LOG(("Z180 '%s' BCR1L  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_BCR1H:
		data = IO_BCR1H & Z180_BCR1H_RMASK;
		LOG(("Z180 '%s' BCR1H  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DSTAT:
		data = IO_DSTAT & Z180_DSTAT_RMASK;
		LOG(("Z180 '%s' DSTAT  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DMODE:
		data = IO_DMODE & Z180_DMODE_RMASK;
		LOG(("Z180 '%s' DMODE  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_DCNTL:
		data = IO_DCNTL & Z180_DCNTL_RMASK;
		LOG(("Z180 '%s' DCNTL  rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IL:
		data = IO_IL & Z180_IL_RMASK;
		LOG(("Z180 '%s' IL     rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_ITC:
		data = IO_ITC & Z180_ITC_RMASK;
		LOG(("Z180 '%s' ITC    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO35:
		data = IO_IO35 & Z180_IO35_RMASK;
		LOG(("Z180 '%s' IO35   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_RCR:
		data = IO_RCR & Z180_RCR_RMASK;
		LOG(("Z180 '%s' RCR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO37:
		data = IO_IO37 & Z180_IO37_RMASK;
		LOG(("Z180 '%s' IO37   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CBR:
		data = IO_CBR & Z180_CBR_RMASK;
		LOG(("Z180 '%s' CBR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_BBR:
		data = IO_BBR & Z180_BBR_RMASK;
		LOG(("Z180 '%s' BBR    rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_CBAR:
		data = IO_CBAR & Z180_CBAR_RMASK;
		LOG(("Z180 '%s' CBAR   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO3B:
		data = IO_IO3B & Z180_IO3B_RMASK;
		LOG(("Z180 '%s' IO3B   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO3C:
		data = IO_IO3C & Z180_IO3C_RMASK;
		LOG(("Z180 '%s' IO3C   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IO3D:
		data = IO_IO3D & Z180_IO3D_RMASK;
		LOG(("Z180 '%s' IO3D   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_OMCR:
		data = IO_OMCR & Z180_OMCR_RMASK;
		LOG(("Z180 '%s' OMCR   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;

	case Z180_IOCR:
		data = IO_IOCR & Z180_IOCR_RMASK;
		LOG(("Z180 '%s' IOCR   rd $%02x ($%02x)\n", tag(), data, m_io[port & 0x3f]));
		break;
	}

	return data;
}

void z180_device::z180_writecontrol(offs_t port, UINT8 data)
{
	/* normal external write port */
	m_iospace->write_byte(port, data);

	/* remap internal I/O registers */
	if((port & (IO_IOCR & 0xc0)) == (IO_IOCR & 0xc0))
		port = port - (IO_IOCR & 0xc0);

	/* store the data in the internal register */
	switch (port + Z180_CNTLA0)
	{
	case Z180_CNTLA0:
		LOG(("Z180 '%s' CNTLA0 wr $%02x ($%02x)\n", tag(), data,  data & Z180_CNTLA0_WMASK));
		IO_CNTLA0 = (IO_CNTLA0 & ~Z180_CNTLA0_WMASK) | (data & Z180_CNTLA0_WMASK);
		break;

	case Z180_CNTLA1:
		LOG(("Z180 '%s' CNTLA1 wr $%02x ($%02x)\n", tag(), data,  data & Z180_CNTLA1_WMASK));
		IO_CNTLA1 = (IO_CNTLA1 & ~Z180_CNTLA1_WMASK) | (data & Z180_CNTLA1_WMASK);
		break;

	case Z180_CNTLB0:
		LOG(("Z180 '%s' CNTLB0 wr $%02x ($%02x)\n", tag(), data,  data & Z180_CNTLB0_WMASK));
		IO_CNTLB0 = (IO_CNTLB0 & ~Z180_CNTLB0_WMASK) | (data & Z180_CNTLB0_WMASK);
		break;

	case Z180_CNTLB1:
		LOG(("Z180 '%s' CNTLB1 wr $%02x ($%02x)\n", tag(), data,  data & Z180_CNTLB1_WMASK));
		IO_CNTLB1 = (IO_CNTLB1 & ~Z180_CNTLB1_WMASK) | (data & Z180_CNTLB1_WMASK);
		break;

	case Z180_STAT0:
		LOG(("Z180 '%s' STAT0  wr $%02x ($%02x)\n", tag(), data,  data & Z180_STAT0_WMASK));
		IO_STAT0 = (IO_STAT0 & ~Z180_STAT0_WMASK) | (data & Z180_STAT0_WMASK);
		break;

	case Z180_STAT1:
		LOG(("Z180 '%s' STAT1  wr $%02x ($%02x)\n", tag(), data,  data & Z180_STAT1_WMASK));
		IO_STAT1 = (IO_STAT1 & ~Z180_STAT1_WMASK) | (data & Z180_STAT1_WMASK);
		break;

	case Z180_TDR0:
		LOG(("Z180 '%s' TDR0   wr $%02x ($%02x)\n", tag(), data,  data & Z180_TDR0_WMASK));
		IO_TDR0 = (IO_TDR0 & ~Z180_TDR0_WMASK) | (data & Z180_TDR0_WMASK);
		break;

	case Z180_TDR1:
		LOG(("Z180 '%s' TDR1   wr $%02x ($%02x)\n", tag(), data,  data & Z180_TDR1_WMASK));
		IO_TDR1 = (IO_TDR1 & ~Z180_TDR1_WMASK) | (data & Z180_TDR1_WMASK);
		break;

	case Z180_RDR0:
		LOG(("Z180 '%s' RDR0   wr $%02x ($%02x)\n", tag(), data,  data & Z180_RDR0_WMASK));
		IO_RDR0 = (IO_RDR0 & ~Z180_RDR0_WMASK) | (data & Z180_RDR0_WMASK);
		break;

	case Z180_RDR1:
		LOG(("Z180 '%s' RDR1   wr $%02x ($%02x)\n", tag(), data,  data & Z180_RDR1_WMASK));
		IO_RDR1 = (IO_RDR1 & ~Z180_RDR1_WMASK) | (data & Z180_RDR1_WMASK);
		break;

	case Z180_CNTR:
		LOG(("Z180 '%s' CNTR   wr $%02x ($%02x)\n", tag(), data,  data & Z180_CNTR_WMASK));
		IO_CNTR = (IO_CNTR & ~Z180_CNTR_WMASK) | (data & Z180_CNTR_WMASK);
		break;

	case Z180_TRDR:
		LOG(("Z180 '%s' TRDR   wr $%02x ($%02x)\n", tag(), data,  data & Z180_TRDR_WMASK));
		IO_TRDR = (IO_TRDR & ~Z180_TRDR_WMASK) | (data & Z180_TRDR_WMASK);
		break;

	case Z180_TMDR0L:
		LOG(("Z180 '%s' TMDR0L wr $%02x ($%02x)\n", tag(), data,  data & Z180_TMDR0L_WMASK));
		IO_TMDR0L = data & Z180_TMDR0L_WMASK;
		m_tmdr_value[0] = (m_tmdr_value[0] & 0xff00) | IO_TMDR0L;
		break;

	case Z180_TMDR0H:
		LOG(("Z180 '%s' TMDR0H wr $%02x ($%02x)\n", tag(), data,  data & Z180_TMDR0H_WMASK));
		IO_TMDR0H = data & Z180_TMDR0H_WMASK;
		m_tmdr_value[0] = (m_tmdr_value[0] & 0x00ff) | (IO_TMDR0H << 8);
		break;

	case Z180_RLDR0L:
		LOG(("Z180 '%s' RLDR0L wr $%02x ($%02x)\n", tag(), data,  data & Z180_RLDR0L_WMASK));
		IO_RLDR0L = (IO_RLDR0L & ~Z180_RLDR0L_WMASK) | (data & Z180_RLDR0L_WMASK);
		break;

	case Z180_RLDR0H:
		LOG(("Z180 '%s' RLDR0H wr $%02x ($%02x)\n", tag(), data,  data & Z180_RLDR0H_WMASK));
		IO_RLDR0H = (IO_RLDR0H & ~Z180_RLDR0H_WMASK) | (data & Z180_RLDR0H_WMASK);
		break;

	case Z180_TCR:
		LOG(("Z180 '%s' TCR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_TCR_WMASK));
		{
			UINT16 old = IO_TCR;
			/* Force reload on state change */
			IO_TCR = (IO_TCR & ~Z180_TCR_WMASK) | (data & Z180_TCR_WMASK);
			if (!(old & Z180_TCR_TDE0) && (IO_TCR & Z180_TCR_TDE0))
				m_tmdr_value[0] = 0; //IO_RLDR0L | (IO_RLDR0H << 8);
			if (!(old & Z180_TCR_TDE1) && (IO_TCR & Z180_TCR_TDE1))
				m_tmdr_value[1] = 0; //IO_RLDR1L | (IO_RLDR1H << 8);
		}

		break;

	case Z180_IO11:
		LOG(("Z180 '%s' IO11   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO11_WMASK));
		IO_IO11 = (IO_IO11 & ~Z180_IO11_WMASK) | (data & Z180_IO11_WMASK);
		break;

	case Z180_ASEXT0:
		LOG(("Z180 '%s' ASEXT0 wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASEXT0_WMASK));
		IO_ASEXT0 = (IO_ASEXT0 & ~Z180_ASEXT0_WMASK) | (data & Z180_ASEXT0_WMASK);
		break;

	case Z180_ASEXT1:
		LOG(("Z180 '%s' ASEXT1 wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASEXT1_WMASK));
		IO_ASEXT1 = (IO_ASEXT1 & ~Z180_ASEXT1_WMASK) | (data & Z180_ASEXT1_WMASK);
		break;

	case Z180_TMDR1L:
		LOG(("Z180 '%s' TMDR1L wr $%02x ($%02x)\n", tag(), data,  data & Z180_TMDR1L_WMASK));
		IO_TMDR1L = data & Z180_TMDR1L_WMASK;
		m_tmdr_value[1] = (m_tmdr_value[1] & 0xff00) | IO_TMDR1L;
		break;

	case Z180_TMDR1H:
		LOG(("Z180 '%s' TMDR1H wr $%02x ($%02x)\n", tag(), data,  data & Z180_TMDR1H_WMASK));
		IO_TMDR1H = data & Z180_TMDR1H_WMASK;
		m_tmdr_value[1] = (m_tmdr_value[1] & 0x00ff) | IO_TMDR1H;
		break;

	case Z180_RLDR1L:
		LOG(("Z180 '%s' RLDR1L wr $%02x ($%02x)\n", tag(), data,  data & Z180_RLDR1L_WMASK));
		IO_RLDR1L = (IO_RLDR1L & ~Z180_RLDR1L_WMASK) | (data & Z180_RLDR1L_WMASK);
		break;

	case Z180_RLDR1H:
		LOG(("Z180 '%s' RLDR1H wr $%02x ($%02x)\n", tag(), data,  data & Z180_RLDR1H_WMASK));
		IO_RLDR1H = (IO_RLDR1H & ~Z180_RLDR1H_WMASK) | (data & Z180_RLDR1H_WMASK);
		break;

	case Z180_FRC:
		LOG(("Z180 '%s' FRC    wr $%02x ($%02x)\n", tag(), data,  data & Z180_FRC_WMASK));
		IO_FRC = (IO_FRC & ~Z180_FRC_WMASK) | (data & Z180_FRC_WMASK);
		break;

	case Z180_IO19:
		LOG(("Z180 '%s' IO19   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO19_WMASK));
		IO_IO19 = (IO_IO19 & ~Z180_IO19_WMASK) | (data & Z180_IO19_WMASK);
		break;

	case Z180_ASTC0L:
		LOG(("Z180 '%s' ASTC0L wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASTC0L_WMASK));
		IO_ASTC0L = (IO_ASTC0L & ~Z180_ASTC0L_WMASK) | (data & Z180_ASTC0L_WMASK);
		break;

	case Z180_ASTC0H:
		LOG(("Z180 '%s' ASTC0H wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASTC0H_WMASK));
		IO_ASTC0H = (IO_ASTC0H & ~Z180_ASTC0H_WMASK) | (data & Z180_ASTC0H_WMASK);
		break;

	case Z180_ASTC1L:
		LOG(("Z180 '%s' ASTC1L wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASTC1L_WMASK));
		IO_ASTC1L = (IO_ASTC1L & ~Z180_ASTC1L_WMASK) | (data & Z180_ASTC1L_WMASK);
		break;

	case Z180_ASTC1H:
		LOG(("Z180 '%s' ASTC1H wr $%02x ($%02x)\n", tag(), data,  data & Z180_ASTC1H_WMASK));
		IO_ASTC1H = (IO_ASTC1H & ~Z180_ASTC1H_WMASK) | (data & Z180_ASTC1H_WMASK);
		break;

	case Z180_CMR:
		LOG(("Z180 '%s' CMR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_CMR_WMASK));
		IO_CMR = (IO_CMR & ~Z180_CMR_WMASK) | (data & Z180_CMR_WMASK);
		break;

	case Z180_CCR:
		LOG(("Z180 '%s' CCR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_CCR_WMASK));
		IO_CCR = (IO_CCR & ~Z180_CCR_WMASK) | (data & Z180_CCR_WMASK);
		break;

	case Z180_SAR0L:
		LOG(("Z180 '%s' SAR0L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_SAR0L_WMASK));
		IO_SAR0L = (IO_SAR0L & ~Z180_SAR0L_WMASK) | (data & Z180_SAR0L_WMASK);
		break;

	case Z180_SAR0H:
		LOG(("Z180 '%s' SAR0H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_SAR0H_WMASK));
		IO_SAR0H = (IO_SAR0H & ~Z180_SAR0H_WMASK) | (data & Z180_SAR0H_WMASK);
		break;

	case Z180_SAR0B:
		LOG(("Z180 '%s' SAR0B  wr $%02x ($%02x)\n", tag(), data,  data & Z180_SAR0B_WMASK));
		IO_SAR0B = (IO_SAR0B & ~Z180_SAR0B_WMASK) | (data & Z180_SAR0B_WMASK);
		break;

	case Z180_DAR0L:
		LOG(("Z180 '%s' DAR0L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DAR0L_WMASK));
		IO_DAR0L = (IO_DAR0L & ~Z180_DAR0L_WMASK) | (data & Z180_DAR0L_WMASK);
		break;

	case Z180_DAR0H:
		LOG(("Z180 '%s' DAR0H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DAR0H_WMASK));
		IO_DAR0H = (IO_DAR0H & ~Z180_DAR0H_WMASK) | (data & Z180_DAR0H_WMASK);
		break;

	case Z180_DAR0B:
		LOG(("Z180 '%s' DAR0B  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DAR0B_WMASK));
		IO_DAR0B = (IO_DAR0B & ~Z180_DAR0B_WMASK) | (data & Z180_DAR0B_WMASK);
		break;

	case Z180_BCR0L:
		LOG(("Z180 '%s' BCR0L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_BCR0L_WMASK));
		IO_BCR0L = (IO_BCR0L & ~Z180_BCR0L_WMASK) | (data & Z180_BCR0L_WMASK);
		break;

	case Z180_BCR0H:
		LOG(("Z180 '%s' BCR0H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_BCR0H_WMASK));
		IO_BCR0H = (IO_BCR0H & ~Z180_BCR0H_WMASK) | (data & Z180_BCR0H_WMASK);
		break;

	case Z180_MAR1L:
		LOG(("Z180 '%s' MAR1L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_MAR1L_WMASK));
		IO_MAR1L = (IO_MAR1L & ~Z180_MAR1L_WMASK) | (data & Z180_MAR1L_WMASK);
		break;

	case Z180_MAR1H:
		LOG(("Z180 '%s' MAR1H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_MAR1H_WMASK));
		IO_MAR1H = (IO_MAR1H & ~Z180_MAR1H_WMASK) | (data & Z180_MAR1H_WMASK);
		break;

	case Z180_MAR1B:
		LOG(("Z180 '%s' MAR1B  wr $%02x ($%02x)\n", tag(), data,  data & Z180_MAR1B_WMASK));
		IO_MAR1B = (IO_MAR1B & ~Z180_MAR1B_WMASK) | (data & Z180_MAR1B_WMASK);
		break;

	case Z180_IAR1L:
		LOG(("Z180 '%s' IAR1L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_IAR1L_WMASK));
		IO_IAR1L = (IO_IAR1L & ~Z180_IAR1L_WMASK) | (data & Z180_IAR1L_WMASK);
		break;

	case Z180_IAR1H:
		LOG(("Z180 '%s' IAR1H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_IAR1H_WMASK));
		IO_IAR1H = (IO_IAR1H & ~Z180_IAR1H_WMASK) | (data & Z180_IAR1H_WMASK);
		break;

	case Z180_IAR1B:
		LOG(("Z180 '%s' IAR1B  wr $%02x ($%02x)\n", tag(), data,  data & Z180_IAR1B_WMASK));
		IO_IAR1B = (IO_IAR1B & ~Z180_IAR1B_WMASK) | (data & Z180_IAR1B_WMASK);
		break;

	case Z180_BCR1L:
		LOG(("Z180 '%s' BCR1L  wr $%02x ($%02x)\n", tag(), data,  data & Z180_BCR1L_WMASK));
		IO_BCR1L = (IO_BCR1L & ~Z180_BCR1L_WMASK) | (data & Z180_BCR1L_WMASK);
		break;

	case Z180_BCR1H:
		LOG(("Z180 '%s' BCR1H  wr $%02x ($%02x)\n", tag(), data,  data & Z180_BCR1H_WMASK));
		IO_BCR1H = (IO_BCR1H & ~Z180_BCR1H_WMASK) | (data & Z180_BCR1H_WMASK);
		break;

	case Z180_DSTAT:
		LOG(("Z180 '%s' DSTAT  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DSTAT_WMASK));
		IO_DSTAT = (IO_DSTAT & ~Z180_DSTAT_WMASK) | (data & Z180_DSTAT_WMASK);
		if ((data & (Z180_DSTAT_DE1 | Z180_DSTAT_DWE1)) == Z180_DSTAT_DE1)
			IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		if ((data & (Z180_DSTAT_DE0 | Z180_DSTAT_DWE0)) == Z180_DSTAT_DE0)
			IO_DSTAT |= Z180_DSTAT_DME;  /* DMA enable */
		break;

	case Z180_DMODE:
		LOG(("Z180 '%s' DMODE  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DMODE_WMASK));
		IO_DMODE = (IO_DMODE & ~Z180_DMODE_WMASK) | (data & Z180_DMODE_WMASK);
		break;

	case Z180_DCNTL:
		LOG(("Z180 '%s' DCNTL  wr $%02x ($%02x)\n", tag(), data,  data & Z180_DCNTL_WMASK));
		IO_DCNTL = (IO_DCNTL & ~Z180_DCNTL_WMASK) | (data & Z180_DCNTL_WMASK);
		break;

	case Z180_IL:
		LOG(("Z180 '%s' IL     wr $%02x ($%02x)\n", tag(), data,  data & Z180_IL_WMASK));
		IO_IL = (IO_IL & ~Z180_IL_WMASK) | (data & Z180_IL_WMASK);
		break;

	case Z180_ITC:
		LOG(("Z180 '%s' ITC    wr $%02x ($%02x)\n", tag(), data,  data & Z180_ITC_WMASK));
		IO_ITC = (IO_ITC & ~Z180_ITC_WMASK) | (data & Z180_ITC_WMASK);
		break;

	case Z180_IO35:
		LOG(("Z180 '%s' IO35   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO35_WMASK));
		IO_IO35 = (IO_IO35 & ~Z180_IO35_WMASK) | (data & Z180_IO35_WMASK);
		break;

	case Z180_RCR:
		LOG(("Z180 '%s' RCR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_RCR_WMASK));
		IO_RCR = (IO_RCR & ~Z180_RCR_WMASK) | (data & Z180_RCR_WMASK);
		break;

	case Z180_IO37:
		LOG(("Z180 '%s' IO37   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO37_WMASK));
		IO_IO37 = (IO_IO37 & ~Z180_IO37_WMASK) | (data & Z180_IO37_WMASK);
		break;

	case Z180_CBR:
		LOG(("Z180 '%s' CBR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_CBR_WMASK));
		IO_CBR = (IO_CBR & ~Z180_CBR_WMASK) | (data & Z180_CBR_WMASK);
		z180_mmu();
		break;

	case Z180_BBR:
		LOG(("Z180 '%s' BBR    wr $%02x ($%02x)\n", tag(), data,  data & Z180_BBR_WMASK));
		IO_BBR = (IO_BBR & ~Z180_BBR_WMASK) | (data & Z180_BBR_WMASK);
		z180_mmu();
		break;

	case Z180_CBAR:
		LOG(("Z180 '%s' CBAR   wr $%02x ($%02x)\n", tag(), data,  data & Z180_CBAR_WMASK));
		IO_CBAR = (IO_CBAR & ~Z180_CBAR_WMASK) | (data & Z180_CBAR_WMASK);
		z180_mmu();
		break;

	case Z180_IO3B:
		LOG(("Z180 '%s' IO3B   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO3B_WMASK));
		IO_IO3B = (IO_IO3B & ~Z180_IO3B_WMASK) | (data & Z180_IO3B_WMASK);
		break;

	case Z180_IO3C:
		LOG(("Z180 '%s' IO3C   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO3C_WMASK));
		IO_IO3C = (IO_IO3C & ~Z180_IO3C_WMASK) | (data & Z180_IO3C_WMASK);
		break;

	case Z180_IO3D:
		LOG(("Z180 '%s' IO3D   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IO3D_WMASK));
		IO_IO3D = (IO_IO3D & ~Z180_IO3D_WMASK) | (data & Z180_IO3D_WMASK);
		break;

	case Z180_OMCR:
		LOG(("Z180 '%s' OMCR   wr $%02x ($%02x)\n", tag(), data,  data & Z180_OMCR_WMASK));
		IO_OMCR = (IO_OMCR & ~Z180_OMCR_WMASK) | (data & Z180_OMCR_WMASK);
		break;

	case Z180_IOCR:
		LOG(("Z180 '%s' IOCR   wr $%02x ($%02x)\n", tag(), data,  data & Z180_IOCR_WMASK));
		IO_IOCR = (IO_IOCR & ~Z180_IOCR_WMASK) | (data & Z180_IOCR_WMASK);
		break;
	}
}

int z180_device::z180_dma0(int max_cycles)
{
	offs_t sar0 = 65536 * IO_SAR0B + 256 * IO_SAR0H + IO_SAR0L;
	offs_t dar0 = 65536 * IO_DAR0B + 256 * IO_DAR0H + IO_DAR0L;
	int bcr0 = 256 * IO_BCR0H + IO_BCR0L;
	int count = (IO_DMODE & Z180_DMODE_MMOD) ? bcr0 : 1;
	int cycles = 0;

	if (bcr0 == 0)
	{
		IO_DSTAT &= ~Z180_DSTAT_DE0;
		return 0;
	}

	while (count-- > 0)
	{
		/* last transfer happening now? */
		if (bcr0 == 1)
		{
			m_iol |= Z180_TEND0;
		}
		switch( IO_DMODE & (Z180_DMODE_SM | Z180_DMODE_DM) )
		{
		case 0x00:  /* memory SAR0+1 to memory DAR0+1 */
			m_program->write_byte(dar0++, m_program->read_byte(sar0++));
			break;
		case 0x04:  /* memory SAR0-1 to memory DAR0+1 */
			m_program->write_byte(dar0++, m_program->read_byte(sar0--));
			break;
		case 0x08:  /* memory SAR0 fixed to memory DAR0+1 */
			m_program->write_byte(dar0++, m_program->read_byte(sar0));
			break;
		case 0x0c:  /* I/O SAR0 fixed to memory DAR0+1 */
			if (m_iol & Z180_DREQ0)
			{
				m_program->write_byte(dar0++, IN(sar0));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x10:  /* memory SAR0+1 to memory DAR0-1 */
			m_program->write_byte(dar0--, m_program->read_byte(sar0++));
			break;
		case 0x14:  /* memory SAR0-1 to memory DAR0-1 */
			m_program->write_byte(dar0--, m_program->read_byte(sar0--));
			break;
		case 0x18:  /* memory SAR0 fixed to memory DAR0-1 */
			m_program->write_byte(dar0--, m_program->read_byte(sar0));
			break;
		case 0x1c:  /* I/O SAR0 fixed to memory DAR0-1 */
			if (m_iol & Z180_DREQ0)
			{
				m_program->write_byte(dar0--, IN(sar0));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x20:  /* memory SAR0+1 to memory DAR0 fixed */
			m_program->write_byte(dar0, m_program->read_byte(sar0++));
			break;
		case 0x24:  /* memory SAR0-1 to memory DAR0 fixed */
			m_program->write_byte(dar0, m_program->read_byte(sar0--));
			break;
		case 0x28:  /* reserved */
			break;
		case 0x2c:  /* reserved */
			break;
		case 0x30:  /* memory SAR0+1 to I/O DAR0 fixed */
			if (m_iol & Z180_DREQ0)
			{
				OUT(dar0, m_program->read_byte(sar0++));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x34:  /* memory SAR0-1 to I/O DAR0 fixed */
			if (m_iol & Z180_DREQ0)
			{
				OUT(dar0, m_program->read_byte(sar0--));
				/* edge sensitive DREQ0 ? */
				if (IO_DCNTL & Z180_DCNTL_DIM0)
				{
					m_iol &= ~Z180_DREQ0;
					count = 0;
				}
			}
			break;
		case 0x38:  /* reserved */
			break;
		case 0x3c:  /* reserved */
			break;
		}
		bcr0--;
		count--;
		cycles += 6;
		if (cycles > max_cycles)
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
		m_iol &= ~Z180_TEND0;
		IO_DSTAT &= ~Z180_DSTAT_DE0;
		/* terminal count interrupt enabled? */
		if (IO_DSTAT & Z180_DSTAT_DIE0 && m_IFF1)
			m_int_pending[Z180_INT_DMA0] = 1;
	}
	return cycles;
}

int z180_device::z180_dma1()
{
	offs_t mar1 = 65536 * IO_MAR1B + 256 * IO_MAR1H + IO_MAR1L;
	offs_t iar1 = 256 * IO_IAR1H + IO_IAR1L;
	int bcr1 = 256 * IO_BCR1H + IO_BCR1L;
	int cycles = 0;

	if ((m_iol & Z180_DREQ1) == 0)
		return 0;

	/* counter is zero? */
	if (bcr1 == 0)
	{
		IO_DSTAT &= ~Z180_DSTAT_DE1;
		return 0;
	}

	/* last transfer happening now? */
	if (bcr1 == 1)
	{
		m_iol |= Z180_TEND1;
	}

	switch (IO_DCNTL & (Z180_DCNTL_DIM1 | Z180_DCNTL_DIM0))
	{
	case 0x00:  /* memory MAR1+1 to I/O IAR1 fixed */
		m_iospace->write_byte(iar1, m_program->read_byte(mar1++));
		break;
	case 0x01:  /* memory MAR1-1 to I/O IAR1 fixed */
		m_iospace->write_byte(iar1, m_program->read_byte(mar1--));
		break;
	case 0x02:  /* I/O IAR1 fixed to memory MAR1+1 */
		m_program->write_byte(mar1++, m_iospace->read_byte(iar1));
		break;
	case 0x03:  /* I/O IAR1 fixed to memory MAR1-1 */
		m_program->write_byte(mar1--, m_iospace->read_byte(iar1));
		break;
	}

	/* edge sensitive DREQ1 ? */
	if (IO_DCNTL & Z180_DCNTL_DIM1)
		m_iol &= ~Z180_DREQ1;

	IO_MAR1L = mar1;
	IO_MAR1H = mar1 >> 8;
	IO_MAR1B = mar1 >> 16;
	IO_BCR1L = bcr1;
	IO_BCR1H = bcr1 >> 8;

	/* DMA terminal count? */
	if (bcr1 == 0)
	{
		m_iol &= ~Z180_TEND1;
		IO_DSTAT &= ~Z180_DSTAT_DE1;
		if (IO_DSTAT & Z180_DSTAT_DIE1 && m_IFF1)
			m_int_pending[Z180_INT_DMA1] = 1;
	}

	/* six cycles per transfer (minimum) */
	return 6 + cycles;
}

void z180_device::z180_write_iolines(UINT32 data)
{
	UINT32 changes = m_iol ^ data;

	/* I/O asynchronous clock 0 (active high) or DREQ0 (mux) */
	if (changes & Z180_CKA0)
	{
		LOG(("Z180 '%s' CKA0   %d\n", tag(), data & Z180_CKA0 ? 1 : 0));
		m_iol = (m_iol & ~Z180_CKA0) | (data & Z180_CKA0);
	}

	/* I/O asynchronous clock 1 (active high) or TEND1 (mux) */
	if (changes & Z180_CKA1)
	{
		LOG(("Z180 '%s' CKA1   %d\n", tag(), data & Z180_CKA1 ? 1 : 0));
		m_iol = (m_iol & ~Z180_CKA1) | (data & Z180_CKA1);
	}

	/* I/O serial clock (active high) */
	if (changes & Z180_CKS)
	{
		LOG(("Z180 '%s' CKS    %d\n", tag(), data & Z180_CKS ? 1 : 0));
		m_iol = (m_iol & ~Z180_CKS) | (data & Z180_CKS);
	}

	/* I   clear to send 0 (active low) */
	if (changes & Z180_CTS0)
	{
		LOG(("Z180 '%s' CTS0   %d\n", tag(), data & Z180_CTS0 ? 1 : 0));
		m_iol = (m_iol & ~Z180_CTS0) | (data & Z180_CTS0);
	}

	/* I   clear to send 1 (active low) or RXS (mux) */
	if (changes & Z180_CTS1)
	{
		LOG(("Z180 '%s' CTS1   %d\n", tag(), data & Z180_CTS1 ? 1 : 0));
		m_iol = (m_iol & ~Z180_CTS1) | (data & Z180_CTS1);
	}

	/* I   data carrier detect (active low) */
	if (changes & Z180_DCD0)
	{
		LOG(("Z180 '%s' DCD0   %d\n", tag(), data & Z180_DCD0 ? 1 : 0));
		m_iol = (m_iol & ~Z180_DCD0) | (data & Z180_DCD0);
	}

	/* I   data request DMA ch 0 (active low) or CKA0 (mux) */
	if (changes & Z180_DREQ0)
	{
		LOG(("Z180 '%s' DREQ0  %d\n", tag(), data & Z180_DREQ0 ? 1 : 0));
		m_iol = (m_iol & ~Z180_DREQ0) | (data & Z180_DREQ0);
	}

	/* I   data request DMA ch 1 (active low) */
	if (changes & Z180_DREQ1)
	{
		LOG(("Z180 '%s' DREQ1  %d\n", tag(), data & Z180_DREQ1 ? 1 : 0));
		m_iol = (m_iol & ~Z180_DREQ1) | (data & Z180_DREQ1);
	}

	/* I   asynchronous receive data 0 (active high) */
	if (changes & Z180_RXA0)
	{
		LOG(("Z180 '%s' RXA0   %d\n", tag(), data & Z180_RXA0 ? 1 : 0));
		m_iol = (m_iol & ~Z180_RXA0) | (data & Z180_RXA0);
	}

	/* I   asynchronous receive data 1 (active high) */
	if (changes & Z180_RXA1)
	{
		LOG(("Z180 '%s' RXA1   %d\n", tag(), data & Z180_RXA1 ? 1 : 0));
		m_iol = (m_iol & ~Z180_RXA1) | (data & Z180_RXA1);
	}

	/* I   clocked serial receive data (active high) or CTS1 (mux) */
	if (changes & Z180_RXS)
	{
		LOG(("Z180 '%s' RXS    %d\n", tag(), data & Z180_RXS ? 1 : 0));
		m_iol = (m_iol & ~Z180_RXS) | (data & Z180_RXS);
	}

	/*   O request to send (active low) */
	if (changes & Z180_RTS0)
	{
		LOG(("Z180 '%s' RTS0   won't change output\n", tag()));
	}

	/*   O transfer end 0 (active low) or CKA1 (mux) */
	if (changes & Z180_TEND0)
	{
		LOG(("Z180 '%s' TEND0  won't change output\n", tag()));
	}

	/*   O transfer end 1 (active low) */
	if (changes & Z180_TEND1)
	{
		LOG(("Z180 '%s' TEND1  won't change output\n", tag()));
	}

	/*   O transfer out (PRT channel, active low) or A18 (mux) */
	if (changes & Z180_A18_TOUT)
	{
		LOG(("Z180 '%s' TOUT   won't change output\n", tag()));
	}

	/*   O asynchronous transmit data 0 (active high) */
	if (changes & Z180_TXA0)
	{
		LOG(("Z180 '%s' TXA0   won't change output\n", tag()));
	}

	/*   O asynchronous transmit data 1 (active high) */
	if (changes & Z180_TXA1)
	{
		LOG(("Z180 '%s' TXA1   won't change output\n", tag()));
	}

	/*   O clocked serial transmit data (active high) */
	if (changes & Z180_TXS)
	{
		LOG(("Z180 '%s' TXS    won't change output\n", tag()));
	}
}


void z180_device::device_start()
{
	int i, p;
	int oldval, newval, val;
	UINT8 *padd, *padc, *psub, *psbc;

	if (static_config() != nullptr)
	{
		m_daisy.init(this, (const z80_daisy_config *)static_config());
	}

	/* allocate big flag arrays once */
	SZHVC_add = std::make_unique<UINT8[]>(2*256*256);
	SZHVC_sub = std::make_unique<UINT8[]>(2*256*256);

	padd = &SZHVC_add[  0*256];
	padc = &SZHVC_add[256*256];
	psub = &SZHVC_sub[  0*256];
	psbc = &SZHVC_sub[256*256];
	for (oldval = 0; oldval < 256; oldval++)
	{
		for (newval = 0; newval < 256; newval++)
		{
			/* add or adc w/o carry set */
			val = newval - oldval;
			*padd = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padd |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */

			if( (newval & 0x0f) < (oldval & 0x0f) ) *padd |= HF;
			if( newval < oldval ) *padd |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padd |= VF;
			padd++;

			/* adc with carry set */
			val = newval - oldval - 1;
			*padc = (newval) ? ((newval & 0x80) ? SF : 0) : ZF;
			*padc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
			if( (newval & 0x0f) <= (oldval & 0x0f) ) *padc |= HF;
			if( newval <= oldval ) *padc |= CF;
			if( (val^oldval^0x80) & (val^newval) & 0x80 ) *padc |= VF;
			padc++;

			/* cp, sub or sbc w/o carry set */
			val = oldval - newval;
			*psub = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psub |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
			if( (newval & 0x0f) > (oldval & 0x0f) ) *psub |= HF;
			if( newval > oldval ) *psub |= CF;
			if( (val^oldval) & (oldval^newval) & 0x80 ) *psub |= VF;
			psub++;

			/* sbc with carry set */
			val = oldval - newval - 1;
			*psbc = NF | ((newval) ? ((newval & 0x80) ? SF : 0) : ZF);
			*psbc |= (newval & (YF | XF));  /* undocumented flag bits 5+3 */
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
		SZ[i] |= (i & (YF | XF));       /* undocumented flag bits 5+3 */
		SZ_BIT[i] = i ? i & SF : ZF | PF;
		SZ_BIT[i] |= (i & (YF | XF));   /* undocumented flag bits 5+3 */
		SZP[i] = SZ[i] | ((p & 1) ? 0 : PF);
		SZHV_inc[i] = SZ[i];
		if( i == 0x80 ) SZHV_inc[i] |= VF;
		if( (i & 0x0f) == 0x00 ) SZHV_inc[i] |= HF;
		SZHV_dec[i] = SZ[i] | NF;
		if( i == 0x7f ) SZHV_dec[i] |= VF;
		if( (i & 0x0f) == 0x0f ) SZHV_dec[i] |= HF;
	}

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_oprogram = has_space(AS_DECRYPTED_OPCODES) ? &space(AS_DECRYPTED_OPCODES) : m_program;
	m_odirect = &m_oprogram->direct();
	m_iospace = &space(AS_IO);

	/* set up the state table */
	{
		state_add(Z180_PC,         "PC",        m_PC.w.l);
		state_add(STATE_GENPC,     "GENPC",     _PCD).noshow();
		state_add(STATE_GENPCBASE, "GENPCBASE", m_PREPC.w.l).noshow();
		state_add(Z180_SP,         "SP",        _SPD);
		state_add(STATE_GENSP,     "GENSP",     m_SP.w.l).noshow();
		state_add(STATE_GENFLAGS,  "GENFLAGS",  m_AF.b.l).noshow().formatstr("%8s");
		state_add(Z180_A,          "A",         _A).noshow();
		state_add(Z180_B,          "B",         _B).noshow();
		state_add(Z180_C,          "C",         _C).noshow();
		state_add(Z180_D,          "D",         _D).noshow();
		state_add(Z180_E,          "E",         _E).noshow();
		state_add(Z180_H,          "H",         _H).noshow();
		state_add(Z180_L,          "L",         _L).noshow();
		state_add(Z180_AF,         "AF",        m_AF.w.l);
		state_add(Z180_BC,         "BC",        m_BC.w.l);
		state_add(Z180_DE,         "DE",        m_DE.w.l);
		state_add(Z180_HL,         "HL",        m_HL.w.l);
		state_add(Z180_IX,         "IX",        m_IX.w.l);
		state_add(Z180_IY,         "IY",        m_IY.w.l);
		state_add(Z180_AF2,        "AF2",       m_AF2.w.l);
		state_add(Z180_BC2,        "BC2",       m_BC2.w.l);
		state_add(Z180_DE2,        "DE2",       m_DE2.w.l);
		state_add(Z180_HL2,        "HL2",       m_HL2.w.l);
		state_add(Z180_R,          "R",         m_rtemp).callimport().callexport();
		state_add(Z180_I,          "I",         m_I);
		state_add(Z180_IM,         "IM",        m_IM).mask(0x3);
		state_add(Z180_IFF1,       "IFF1",      m_IFF1).mask(0x1);
		state_add(Z180_IFF2,       "IFF2",      m_IFF2).mask(0x1);
		state_add(Z180_HALT,       "HALT",      m_HALT).mask(0x1);

		state_add(Z180_IOLINES,    "IOLINES",   m_ioltemp).mask(0xffffff).callimport();

		state_add(Z180_CNTLA0,     "CNTLA0",    IO_CNTLA0);
		state_add(Z180_CNTLA1,     "CNTLA1",    IO_CNTLA1);
		state_add(Z180_CNTLB0,     "CNTLB0",    IO_CNTLB0);
		state_add(Z180_CNTLB1,     "CNTLB1",    IO_CNTLB1);
		state_add(Z180_STAT0,      "STAT0",     IO_STAT0);
		state_add(Z180_STAT1,      "STAT1",     IO_STAT1);
		state_add(Z180_TDR0,       "TDR0",      IO_TDR0);
		state_add(Z180_TDR1,       "TDR1",      IO_TDR1);
		state_add(Z180_RDR0,       "RDR0",      IO_RDR0);
		state_add(Z180_RDR1,       "RDR1",      IO_RDR1);
		state_add(Z180_CNTR,       "CNTR",      IO_CNTR);
		state_add(Z180_TRDR,       "TRDR",      IO_TRDR);
		state_add(Z180_TMDR0L,     "TMDR0L",    IO_TMDR0L);
		state_add(Z180_TMDR0H,     "TMDR0H",    IO_TMDR0H);
		state_add(Z180_RLDR0L,     "RLDR0L",    IO_RLDR0L);
		state_add(Z180_RLDR0H,     "RLDR0H",    IO_RLDR0H);
		state_add(Z180_TCR,        "TCR",       IO_TCR);
		state_add(Z180_IO11,       "IO11",      IO_IO11);
		state_add(Z180_ASEXT0,     "ASEXT0",    IO_ASEXT0);
		state_add(Z180_ASEXT1,     "ASEXT1",    IO_ASEXT1);
		state_add(Z180_TMDR1L,     "TMDR1L",    IO_TMDR1L);
		state_add(Z180_TMDR1H,     "TMDR1H",    IO_TMDR1H);
		state_add(Z180_RLDR1L,     "RLDR1L",    IO_RLDR1L);
		state_add(Z180_RLDR1H,     "RLDR1H",    IO_RLDR1H);
		state_add(Z180_FRC,        "FRC",       IO_FRC);
		state_add(Z180_IO19,       "IO19",      IO_IO19);
		state_add(Z180_ASTC0L,     "ASTC0L",    IO_ASTC0L);
		state_add(Z180_ASTC0H,     "ASTC0H",    IO_ASTC0H);
		state_add(Z180_ASTC1L,     "ASTC1L",    IO_ASTC1L);
		state_add(Z180_ASTC1H,     "ASTC1H",    IO_ASTC1H);
		state_add(Z180_CMR,        "CMR",       IO_CMR);
		state_add(Z180_CCR,        "CCR",       IO_CCR);
		state_add(Z180_SAR0L,      "SAR0L",     IO_SAR0L);
		state_add(Z180_SAR0H,      "SAR0H",     IO_SAR0H);
		state_add(Z180_SAR0B,      "SAR0B",     IO_SAR0B);
		state_add(Z180_DAR0L,      "DAR0L",     IO_DAR0L);
		state_add(Z180_DAR0H,      "DAR0H",     IO_DAR0H);
		state_add(Z180_DAR0B,      "DAR0B",     IO_DAR0B);
		state_add(Z180_BCR0L,      "BCR0L",     IO_BCR0L);
		state_add(Z180_BCR0H,      "BCR0H",     IO_BCR0H);
		state_add(Z180_MAR1L,      "MAR1L",     IO_MAR1L);
		state_add(Z180_MAR1H,      "MAR1H",     IO_MAR1H);
		state_add(Z180_MAR1B,      "MAR1B",     IO_MAR1B);
		state_add(Z180_IAR1L,      "IAR1L",     IO_IAR1L);
		state_add(Z180_IAR1H,      "IAR1H",     IO_IAR1H);
		state_add(Z180_IAR1B,      "IAR1B",     IO_IAR1B);
		state_add(Z180_BCR1L,      "BCR1L",     IO_BCR1L);
		state_add(Z180_BCR1H,      "BCR1H",     IO_BCR1H);
		state_add(Z180_DSTAT,      "DSTAT",     IO_DSTAT);
		state_add(Z180_DMODE,      "DMODE",     IO_DMODE);
		state_add(Z180_DCNTL,      "DCNTL",     IO_DCNTL);
		state_add(Z180_IL,         "IL",        IO_IL);
		state_add(Z180_ITC,        "ITC",       IO_ITC);
		state_add(Z180_IO35,       "IO35",      IO_IO35);
		state_add(Z180_RCR,        "RCR",       IO_RCR);
		state_add(Z180_IO37,       "IO37",      IO_IO37);
		state_add(Z180_CBR,        "CBR",       IO_CBR).callimport();
		state_add(Z180_BBR,        "BBR",       IO_BBR).callimport();
		state_add(Z180_CBAR,       "CBAR",      IO_CBAR).callimport();
		state_add(Z180_IO3B,       "IO3B",      IO_IO3B);
		state_add(Z180_IO3C,       "IO3C",      IO_IO3C);
		state_add(Z180_IO3D,       "IO3D",      IO_IO3D);
		state_add(Z180_OMCR,       "OMCR",      IO_OMCR);
		state_add(Z180_IOCR,       "IOCR",      IO_IOCR);
	}

	save_item(NAME(m_AF.w.l));
	save_item(NAME(m_BC.w.l));
	save_item(NAME(m_DE.w.l));
	save_item(NAME(m_HL.w.l));
	save_item(NAME(m_IX.w.l));
	save_item(NAME(m_IY.w.l));
	save_item(NAME(m_PC.w.l));
	save_item(NAME(m_SP.w.l));
	save_item(NAME(m_AF2.w.l));
	save_item(NAME(m_BC2.w.l));
	save_item(NAME(m_DE2.w.l));
	save_item(NAME(m_HL2.w.l));
	save_item(NAME(m_R));
	save_item(NAME(m_R2));
	save_item(NAME(m_IFF1));
	save_item(NAME(m_IFF2));
	save_item(NAME(m_HALT));
	save_item(NAME(m_IM));
	save_item(NAME(m_I));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));
	save_item(NAME(m_int_pending));
	save_item(NAME(m_timer_cnt));
	save_item(NAME(m_dma0_cnt));
	save_item(NAME(m_dma1_cnt));
	save_item(NAME(m_after_EI));

	save_item(NAME(m_tif));

	save_item(NAME(m_read_tcr_tmdr));
	save_item(NAME(m_tmdr_value));
	save_item(NAME(m_tmdrh));
	save_item(NAME(m_tmdr_latch));

	save_item(NAME(m_io));
	save_item(NAME(m_iol));
	save_item(NAME(m_ioltemp));

	save_item(NAME(m_mmu));

	m_icountptr = &m_icount;
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
void z180_device::device_reset()
{
	_PPC = 0;
	_PCD = 0;
	_SPD = 0;
	_AFD = 0;
	_BCD = 0;
	_DED = 0;
	_HLD = 0;
	_IXD = 0;
	_IYD = 0;
	m_AF2.d = 0;
	m_BC2.d = 0;
	m_DE2.d = 0;
	m_HL2.d = 0;
	m_R = 0;
	m_R2 = 0;
	m_IFF1 = 0;
	m_IFF2 = 0;
	m_HALT = 0;
	m_IM = 0;
	m_I = 0;
	m_tmdr_latch = 0;
	m_read_tcr_tmdr[0] = 0;
	m_read_tcr_tmdr[1] = 0;
	m_iol = 0;
	memset(m_io, 0, sizeof(m_io));
	memset(m_mmu, 0, sizeof(m_mmu));
	m_tmdrh[0] = 0;
	m_tmdrh[1] = 0;
	m_tmdr_value[0] = 0xffff;
	m_tmdr_value[1] = 0xffff;
	m_tif[0] = 0;
	m_tif[1] = 0;
	m_nmi_state = CLEAR_LINE;
	m_nmi_pending = 0;
	m_irq_state[0] = CLEAR_LINE;
	m_irq_state[1] = CLEAR_LINE;
	m_irq_state[2] = CLEAR_LINE;
	m_after_EI = 0;
	m_ea = 0;

	memcpy(m_cc, (UINT8 *)cc_default, sizeof(m_cc));
	_IX = _IY = 0xffff; /* IX and IY are FFFF after a reset! */
	_F = ZF;          /* Zero flag is set */

	for (int i=0; i <= Z180_INT_MAX; i++)
	{
		m_int_pending[i] = 0;
	}

	m_timer_cnt = 0;
	m_dma0_cnt = 0;
	m_dma1_cnt = 0;

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
	IO_TCR       = Z180_TCR_RESET;
	IO_IO11    = Z180_IO11_RESET;
	IO_ASEXT0  = Z180_ASEXT0_RESET;
	IO_ASEXT1  = Z180_ASEXT1_RESET;
	IO_TMDR1L  = Z180_TMDR1L_RESET;
	IO_TMDR1H  = Z180_TMDR1H_RESET;
	IO_RLDR1L  = Z180_RLDR1L_RESET;
	IO_RLDR1H  = Z180_RLDR1H_RESET;
	IO_FRC       = Z180_FRC_RESET;
	IO_IO19    = Z180_IO19_RESET;
	IO_ASTC0L  = Z180_ASTC0L_RESET;
	IO_ASTC0H  = Z180_ASTC0H_RESET;
	IO_ASTC1L  = Z180_ASTC1L_RESET;
	IO_ASTC1H  = Z180_ASTC1H_RESET;
	IO_CMR       = Z180_CMR_RESET;
	IO_CCR       = Z180_CCR_RESET;
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
	IO_IL    = Z180_IL_RESET;
	IO_ITC       = Z180_ITC_RESET;
	IO_IO35    = Z180_IO35_RESET;
	IO_RCR       = Z180_RCR_RESET;
	IO_IO37    = Z180_IO37_RESET;
	IO_CBR       = Z180_CBR_RESET;
	IO_BBR       = Z180_BBR_RESET;
	IO_CBAR    = Z180_CBAR_RESET;
	IO_IO3B    = Z180_IO3B_RESET;
	IO_IO3C    = Z180_IO3C_RESET;
	IO_IO3D    = Z180_IO3D_RESET;
	IO_OMCR    = Z180_OMCR_RESET;
	IO_IOCR    = Z180_IOCR_RESET;

	m_daisy.reset();
	z180_mmu();
}

/* Handle PRT timers, decreasing them after 20 clocks and returning the new icount base that needs to be used for the next check */
void z180_device::clock_timers()
{
	m_timer_cnt++;
	if (m_timer_cnt >= 20)
	{
		m_timer_cnt = 0;
		/* Programmable Reload Timer 0 */
		if(IO_TCR & Z180_TCR_TDE0)
		{
			if(m_tmdr_value[0] == 0)
			{
				m_tmdr_value[0] = IO_RLDR0L | (IO_RLDR0H << 8);
				m_tif[0] = 1;
			}
			else
				m_tmdr_value[0]--;
		}

		/* Programmable Reload Timer 1 */
		if(IO_TCR & Z180_TCR_TDE1)
		{
			if(m_tmdr_value[1] == 0)
			{
				m_tmdr_value[1] = IO_RLDR1L | (IO_RLDR1H << 8);
				m_tif[1] = 1;
			}
			else
				m_tmdr_value[1]--;
		}

		if((IO_TCR & Z180_TCR_TIE0) && m_tif[0])
		{
			// check if we can take the interrupt
			if(m_IFF1 && !m_after_EI)
			{
				m_int_pending[Z180_INT_PRT0] = 1;
			}
		}

		if((IO_TCR & Z180_TCR_TIE1) && m_tif[1])
		{
			// check if we can take the interrupt
			if(m_IFF1 && !m_after_EI)
			{
				m_int_pending[Z180_INT_PRT1] = 1;
			}
		}

	}
}

int z180_device::check_interrupts()
{
	int i;
	int cycles = 0;

	/* check for IRQs before each instruction */
	if (m_IFF1 && !m_after_EI)
	{
		if (m_irq_state[0] != CLEAR_LINE && (IO_ITC & Z180_ITC_ITE0) == Z180_ITC_ITE0)
			m_int_pending[Z180_INT_IRQ0] = 1;

		if (m_irq_state[1] != CLEAR_LINE && (IO_ITC & Z180_ITC_ITE1) == Z180_ITC_ITE1)
			m_int_pending[Z180_INT_IRQ1] = 1;

		if (m_irq_state[2] != CLEAR_LINE && (IO_ITC & Z180_ITC_ITE2) == Z180_ITC_ITE2)
			m_int_pending[Z180_INT_IRQ2] = 1;
	}

	for (i = 0; i <= Z180_INT_MAX; i++)
		if (m_int_pending[i])
		{
			cycles += take_interrupt(i);
			m_int_pending[i] = 0;
			break;
		}

	return cycles;
}

/****************************************************************************
 * Handle I/O and timers
 ****************************************************************************/

void z180_device::handle_io_timers(int cycles)
{
	while (cycles-- > 0)
	{
		clock_timers();
	}
}

/****************************************************************************
 * Execute 'cycles' T-states. Return number of T-states really executed
 ****************************************************************************/
void z180_device::execute_run()
{
	int curcycles;

	/* check for NMIs on the way in; they can only be set externally */
	/* via timers, and can't be dynamically enabled, so it is safe */
	/* to just check here */
	if (m_nmi_pending)
	{
		LOG(("Z180 '%s' take NMI\n", tag()));
		_PPC = -1;            /* there isn't a valid previous program counter */
		LEAVE_HALT();       /* Check if processor was halted */

		/* disable DMA transfers!! */
		IO_DSTAT &= ~Z180_DSTAT_DME;

		m_IFF2 = m_IFF1;
		m_IFF1 = 0;
		PUSH( PC );
		_PCD = 0x0066;
		m_icount -= 11;
		m_nmi_pending = 0;
		handle_io_timers(11);
	}

again:
	/* check if any DMA transfer is running */
	if ((IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
	{
		/* check if DMA channel 0 is running and also is in burst mode */
		if ((IO_DSTAT & Z180_DSTAT_DE0) == Z180_DSTAT_DE0 &&
			(IO_DMODE & Z180_DMODE_MMOD) == Z180_DMODE_MMOD)
		{
			debugger_instruction_hook(this, _PCD);

			/* FIXME z180_dma0 should be handled in handle_io_timers */
			curcycles = z180_dma0(m_icount);
			m_icount -= curcycles;
			handle_io_timers(curcycles);
		}
		else
		{
			do
			{
				curcycles = check_interrupts();
				m_icount -= curcycles;
				handle_io_timers(curcycles);
				m_after_EI = 0;

				_PPC = _PCD;
				debugger_instruction_hook(this, _PCD);

				if (!m_HALT)
				{
					m_R++;
					m_extra_cycles = 0;
					curcycles = exec_op(ROP());
					curcycles += m_extra_cycles;
				}
				else
					curcycles = 3;

				m_icount -= curcycles;

				handle_io_timers(curcycles);

				/* FIXME:
				 * For simultaneous DREQ0 and DREQ1 requests, channel 0 has priority
				 * over channel 1. When channel 0 is performing a memory to/from memory
				 * transfer, channel 1 cannot operate until the channel 0 operation has
				 * terminated. If channel 1 is operating, channel 0 cannot operate until
				 * channel 1 releases control of the bus.
				 *
				 */
				curcycles = z180_dma0(6);
				m_icount -= curcycles;
				handle_io_timers(curcycles);

				curcycles = z180_dma1();
				m_icount -= curcycles;
				handle_io_timers(curcycles);

				/* If DMA is done break out to the faster loop */
				if ((IO_DSTAT & Z180_DSTAT_DME) != Z180_DSTAT_DME)
					break;
			} while( m_icount > 0 );
		}
	}

	if (m_icount > 0)
	{
		do
		{
			curcycles = check_interrupts();
			m_icount -= curcycles;
			handle_io_timers(curcycles);
			m_after_EI = 0;

			_PPC = _PCD;
			debugger_instruction_hook(this, _PCD);

			if (!m_HALT)
			{
				m_R++;
				m_extra_cycles = 0;
				curcycles = exec_op(ROP());
				curcycles += m_extra_cycles;
			}
			else
				curcycles = 3;

			m_icount -= curcycles;
			handle_io_timers(curcycles);

			/* If DMA is started go to check the mode */
			if ((IO_DSTAT & Z180_DSTAT_DME) == Z180_DSTAT_DME)
				goto again;
		} while( m_icount > 0 );
	}
}

/****************************************************************************
 * Burn 'cycles' T-states. Adjust R register for the lost time
 ****************************************************************************/
void z180_device::execute_burn(INT32 cycles)
{
	/* FIXME: This is not appropriate for dma */
	while ( (cycles > 0) )
	{
		handle_io_timers(3);
		/* NOP takes 3 cycles per instruction */
		m_R += 1;
		m_icount -= 3;
		cycles -= 3;
	}
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void z180_device::execute_set_input(int irqline, int state)
{
	if (irqline == INPUT_LINE_NMI)
	{
		/* mark an NMI pending on the rising edge */
		if (m_nmi_state == CLEAR_LINE && state != CLEAR_LINE)
			m_nmi_pending = 1;
		m_nmi_state = state;
	}
	else
	{
		LOG(("Z180 '%s' set_irq_line %d = %d\n",tag() , irqline,state));

		/* update the IRQ state */
		m_irq_state[irqline] = state;
		if (m_daisy.present())
			m_irq_state[0] = m_daisy.update_irq_state();

		/* the main execute loop will take the interrupt */
	}
}

/* logical to physical address translation */
bool z180_device::memory_translate(address_spacenum spacenum, int intention, offs_t &address)
{
	if (spacenum == AS_PROGRAM)
	{
		address = MMU_REMAP_ADDR(address);
	}
	return true;
}


/**************************************************************************
 * STATE IMPORT/EXPORT
 **************************************************************************/

void z180_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z180_R:
			m_R = m_rtemp & 0x7f;
			m_R2 = m_rtemp & 0x80;
			break;

		case Z180_CBR:
		case Z180_BBR:
		case Z180_CBAR:
			z180_mmu();
			break;

		case Z180_IOLINES:
			z180_write_iolines(m_ioltemp);
			break;

		default:
			fatalerror("CPU_IMPORT_STATE(z80) called for unexpected value\n");
	}
}


void z180_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case Z180_R:
			m_rtemp = (m_R & 0x7f) | (m_R2 & 0x80);
			break;

		case Z180_IOLINES:
			m_ioltemp = m_iol;
			break;

		default:
			fatalerror("CPU_EXPORT_STATE(z80) called for unexpected value\n");
	}
}

void z180_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c",
				m_AF.b.l & 0x80 ? 'S':'.',
				m_AF.b.l & 0x40 ? 'Z':'.',
				m_AF.b.l & 0x20 ? '5':'.',
				m_AF.b.l & 0x10 ? 'H':'.',
				m_AF.b.l & 0x08 ? '3':'.',
				m_AF.b.l & 0x04 ? 'P':'.',
				m_AF.b.l & 0x02 ? 'N':'.',
				m_AF.b.l & 0x01 ? 'C':'.');
			break;
	}
}
