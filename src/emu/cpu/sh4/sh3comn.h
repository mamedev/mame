// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __SH3COMN_H__
#define __SH3COMN_H__


// actual port handling is more complex than this
// which should be considered a temporary solution
// just used some arbitrary  port nubmers
#define SH3_PORT_A (0x10*8)
#define SH3_PORT_B (0x11*8)
#define SH3_PORT_C (0x12*8)
#define SH3_PORT_D (0x13*8)
#define SH3_PORT_E (0x14*8)
#define SH3_PORT_F (0x15*8)
#define SH3_PORT_G (0x16*8)
#define SH3_PORT_H (0x17*8)
/* no I */
#define SH3_PORT_J (0x18*8)
#define SH3_PORT_K (0x19*8)
#define SH3_PORT_L (0x1a*8)

/* SH3 lower area regs */

#define SH3_LOWER_REGBASE (0x04000000)
#define SH3_LOWER_REGEND  (0x07ffffff)

#define INTEVT2     ((0x4000000 - SH3_LOWER_REGBASE)/4)
#define IRR0_IRR1   ((0x4000004 - SH3_LOWER_REGBASE)/4)
#define PINTER_IPRC ((0x4000014 - SH3_LOWER_REGBASE)/4)

#define SH3_SAR0_ADDR       ((0x4000020 - SH3_LOWER_REGBASE)/4)
#define SH3_DAR0_ADDR       ((0x4000024 - SH3_LOWER_REGBASE)/4)
#define SH3_DMATCR0_ADDR    ((0x4000028 - SH3_LOWER_REGBASE)/4)
#define SH3_CHCR0_ADDR      ((0x400002c - SH3_LOWER_REGBASE)/4)
#define SH3_SAR1_ADDR       ((0x4000030 - SH3_LOWER_REGBASE)/4)
#define SH3_DAR1_ADDR       ((0x4000034 - SH3_LOWER_REGBASE)/4)
#define SH3_DMATCR1_ADDR    ((0x4000038 - SH3_LOWER_REGBASE)/4)
#define SH3_CHCR1_ADDR      ((0x400003c - SH3_LOWER_REGBASE)/4)
#define SH3_SAR2_ADDR       ((0x4000040 - SH3_LOWER_REGBASE)/4)
#define SH3_DAR2_ADDR       ((0x4000044 - SH3_LOWER_REGBASE)/4)
#define SH3_DMATCR2_ADDR    ((0x4000048 - SH3_LOWER_REGBASE)/4)
#define SH3_CHCR2_ADDR      ((0x400004c - SH3_LOWER_REGBASE)/4)
#define SH3_SAR3_ADDR       ((0x4000050 - SH3_LOWER_REGBASE)/4)
#define SH3_DAR3_ADDR       ((0x4000054 - SH3_LOWER_REGBASE)/4)
#define SH3_DMATCR3_ADDR    ((0x4000058 - SH3_LOWER_REGBASE)/4)
#define SH3_CHCR3_ADDR      ((0x400005c - SH3_LOWER_REGBASE)/4)
#define SH3_DMAOR_ADDR      ((0x4000060 - SH3_LOWER_REGBASE)/4)


#define PCCR_PDCR   ((0x4000104 - SH3_LOWER_REGBASE)/4)
#define PECR_PFCR   ((0x4000108 - SH3_LOWER_REGBASE)/4)
#define PGCR_PHCR   ((0x400010c - SH3_LOWER_REGBASE)/4)
#define PJCR_PKCR   ((0x4000110 - SH3_LOWER_REGBASE)/4)
#define PLCR_SCPCR  ((0x4000114 - SH3_LOWER_REGBASE)/4)

#define PADR_PBDR   ((0x4000120 - SH3_LOWER_REGBASE)/4)
#define PCDR_PDDR   ((0x4000124 - SH3_LOWER_REGBASE)/4)
#define PEDR_PFDR   ((0x4000128 - SH3_LOWER_REGBASE)/4)
#define PGDR_PHDR   ((0x400012c - SH3_LOWER_REGBASE)/4)
#define PJDR_PKDR   ((0x4000130 - SH3_LOWER_REGBASE)/4)
#define PLDR_SCPDR  ((0x4000134 - SH3_LOWER_REGBASE)/4)

#define SCSMR2_SCBRR2   ((0x4000150 - SH3_LOWER_REGBASE)/4)
#define SCSCR2_SCFTDR2  ((0x4000154 - SH3_LOWER_REGBASE)/4)
#define SCSSR2_SCFRDR2  ((0x4000158 - SH3_LOWER_REGBASE)/4)
#define SCFCR2_SCFDR2   ((0x400015c - SH3_LOWER_REGBASE)/4)


/* SH3 upper area */


#define SH3_UPPER_REGBASE (0xffffd000)
#define SH3_UPPER_REGEND  (0xffffffff)

#define SH3_ICR0_IPRA_ADDR  ((0xfffffee0 - SH3_UPPER_REGBASE)/4)
#define SH3_IPRB_ADDR       ((0xfffffee4 - SH3_UPPER_REGBASE)/4)

#define SH3_TOCR_TSTR_ADDR  ((0xfffffe90 - SH3_UPPER_REGBASE)/4)
#define SH3_TCOR0_ADDR      ((0xfffffe94 - SH3_UPPER_REGBASE)/4)
#define SH3_TCNT0_ADDR      ((0xfffffe98 - SH3_UPPER_REGBASE)/4)
#define SH3_TCR0_ADDR       ((0xfffffe9c - SH3_UPPER_REGBASE)/4)
#define SH3_TCOR1_ADDR      ((0xfffffea0 - SH3_UPPER_REGBASE)/4)
#define SH3_TCNT1_ADDR      ((0xfffffea4 - SH3_UPPER_REGBASE)/4)
#define SH3_TCR1_ADDR       ((0xfffffea8 - SH3_UPPER_REGBASE)/4)
#define SH3_TCOR2_ADDR      ((0xfffffeac - SH3_UPPER_REGBASE)/4)
#define SH3_TCNT2_ADDR      ((0xfffffeb0 - SH3_UPPER_REGBASE)/4)
#define SH3_TCR2_ADDR       ((0xfffffeb4 - SH3_UPPER_REGBASE)/4)
#define SH3_TCPR2_ADDR      ((0xfffffeb8 - SH3_UPPER_REGBASE)/4)
#define SH3_TRA_ADDR        ((0xffffffd0 - SH3_UPPER_REGBASE)/4)
#define SH3_EXPEVT_ADDR     ((0xffffffd4 - SH3_UPPER_REGBASE)/4)
#define SH3_INTEVT_ADDR     ((0xffffffd8 - SH3_UPPER_REGBASE)/4)

#endif /* __SH3COMN_H__ */
