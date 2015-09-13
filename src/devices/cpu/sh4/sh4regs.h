// license:BSD-3-Clause
// copyright-holders:R. Belmont
#pragma once

#ifndef __SH4REGS_H__
#define __SH4REGS_H__

/* 00000001111111100000000011111100 */
#define PTEH    0x2000  /* FF000000 */
#define PTEL    0x2001  /* FF000004 */
#define TTB     0x2002  /* FF000008 */
#define TEA     0x2003  /* FF00000C */
#define MMUCR   0x2004  /* FF000010 */
#define BASRA   0x2005  /* FF000014 */
#define BASRB   0x2006  /* FF000018 */
#define CCR     0x2007  /* FF00001C */
#define TRA     0x2008  /* FF000020 */
#define EXPEVT  0x2009  /* FF000024 */
#define INTEVT  0x200A  /* FF000028 */
#define VERSION 0x200C  /* FF000030 */
#define PTEA    0x200D  /* FF000034 */
#define QACR0   0x200E  /* FF000038 */
#define QACR1   0x200F  /* FF00003C */
#define PRR     0x2011  /* FF000044 */
#define BARA    0x2400  /* FF200000 */
#define BAMRA   0x2401  /* FF200004 */
#define BBRA    0x2402  /* FF200008 */
#define BARB    0x2403  /* FF20000C */
#define BAMRB   0x2404  /* FF200010 */
#define BBRB    0x2405  /* FF200014 */
#define BDRB    0x2406  /* FF200018 */
#define BDMRB   0x2407  /* FF20001C */
#define BRCR    0x2408  /* FF200020 */
#define BCR1    0x3000  /* FF800000 */
#define BCR2    0x3001  /* FF800004 */
#define BCR3    0x3014  /* FF800050 */
#define BCR4    0x17C   /* FE0A00F0 */
#define WCR1    0x3002  /* FF800008 */
#define WCR2    0x3003  /* FF80000C */
#define WCR3    0x3004  /* FF800010 */
#define MCR     0x3005  /* FF800014 */
#define PCR     0x3006  /* FF800018 */
#define RTCSR   0x3007  /* FF80001C */
#define RTCNT   0x3008  /* FF800020 */
#define RTCOR   0x3009  /* FF800024 */
#define RFCR    0x300A  /* FF800028 */
#define PCTRA   0x300B  /* FF80002C */
#define PDTRA   0x300C  /* FF800030 */
#define PCTRB   0x3010  /* FF800040 */
#define PDTRB   0x3011  /* FF800044 */
#define GPIOIC  0x3012  /* FF800048 */
#define SDMR2   0x3200  /* FF900000 */
#define SDMR3   0x3280  /* FF940000 */
#define SH4_SAR0_ADDR       0x3400  /* FFA00000 */
#define SH4_DAR0_ADDR       0x3401  /* FFA00004 */
#define SH4_DMATCR0_ADDR    0x3402  /* FFA00008 */
#define SH4_CHCR0_ADDR      0x3403  /* FFA0000C */
#define SH4_SAR1_ADDR       0x3404  /* FFA00010 */
#define SH4_DAR1_ADDR       0x3405  /* FFA00014 */
#define SH4_DMATCR1_ADDR    0x3406  /* FFA00018 */
#define SH4_CHCR1_ADDR      0x3407  /* FFA0001C */
#define SH4_SAR2_ADDR       0x3408  /* FFA00020 */
#define SH4_DAR2_ADDR       0x3409  /* FFA00024 */
#define SH4_DMATCR2_ADDR    0x340A  /* FFA00028 */
#define SH4_CHCR2_ADDR      0x340B  /* FFA0002C */
#define SH4_SAR3_ADDR       0x340C  /* FFA00030 */
#define SH4_DAR3_ADDR       0x340D  /* FFA00034 */
#define SH4_DMATCR3_ADDR    0x340E  /* FFA00038 */
#define SH4_CHCR3_ADDR      0x340F  /* FFA0003C */
#define SH4_DMAOR_ADDR      0x3410  /* FFA00040 */
#define SAR4    0x3414  /* FFA00050 */
#define DAR4    0x3415  /* FFA00054 */
#define DMATCR4 0x3416  /* FFA00058 */
#define CHCR4   0x3417  /* FFA0005C */
#define SAR5    0x3418  /* FFA00060 */
#define DAR5    0x3419  /* FFA00064 */
#define DMATCR5 0x341A  /* FFA00068 */
#define CHCR5   0x341B  /* FFA0006C */
#define SAR6    0x341C  /* FFA00070 */
#define DAR6    0x341D  /* FFA00074 */
#define DMATCR6 0x341E  /* FFA00078 */
#define CHCR6   0x341F  /* FFA0007C */
#define SAR7    0x3420  /* FFA00080 */
#define DAR7    0x3421  /* FFA00084 */
#define DMATCR7 0x3422  /* FFA00088 */
#define CHCR7   0x3423  /* FFA0008C */
#define FRQCR   0x3800  /* FFC00000 */
#define STBCR   0x3801  /* FFC00004 */
#define WTCNT   0x3802  /* FFC00008 */
#define WTCSR   0x3803  /* FFC0000C */
#define STBCR2  0x3804  /* FFC00010 */
#define R64CNT  0x3900  /* FFC80000 */
#define RSECCNT 0x3901  /* FFC80004 */
#define RMINCNT 0x3902  /* FFC80008 */
#define RHRCNT  0x3903  /* FFC8000C */
#define RWKCNT  0x3904  /* FFC80010 */
#define RDAYCNT 0x3905  /* FFC80014 */
#define RMONCNT 0x3906  /* FFC80018 */
#define RYRCNT  0x3907  /* FFC8001C */
#define RSECAR  0x3908  /* FFC80020 */
#define RMINAR  0x3909  /* FFC80024 */
#define RHRAR   0x390A  /* FFC80028 */
#define RWKAR   0x390B  /* FFC8002C */
#define RDAYAR  0x390C  /* FFC80030 */
#define RMONAR  0x390D  /* FFC80034 */
#define RCR1    0x390E  /* FFC80038 */
#define RCR2    0x390F  /* FFC8003C */
#define RCR3    0x3914  /* FFC80050 */
#define RYRAR   0x3915  /* FFC80054 */
#define ICR     0x3A00  /* FFD00000 */
#define IPRA    0x3A01  /* FFD00004 */
#define IPRB    0x3A02  /* FFD00008 */
#define IPRC    0x3A03  /* FFD0000C */
#define IPRD    0x3A04  /* FFD00010 */
#define INTPRI00    0x100   /* FE080000 */
#define INTREQ00    0x108   /* FE080020 */
#define INTMSK00    0x110   /* FE080040 */
#define INTMSKCLR00     0x118   /* FE080060 */
#define CLKSTP00    0x140   /* FE0A0000 */
#define CLKSTPCLR00     0x142   /* FE0A0008 */
#define TSTR2   0x201   /* FE100004 */
#define TCOR3   0x202   /* FE100008 */
#define TCNT3   0x203   /* FE10000C */
#define TCR3    0x204   /* FE100010 */
#define TCOR4   0x205   /* FE100014 */
#define TCNT4   0x206   /* FE100018 */
#define TCR4    0x207   /* FE10001C */
#define SH4_TOCR_ADDR   0x3B00  /* FFD80000 */
#define SH4_TSTR_ADDR   0x3B01  /* FFD80004 */
#define SH4_TCOR0_ADDR  0x3B02  /* FFD80008 */
#define SH4_TCNT0_ADDR  0x3B03  /* FFD8000C */
#define SH4_TCR0_ADDR   0x3B04  /* FFD80010 */
#define SH4_TCOR1_ADDR  0x3B05  /* FFD80014 */
#define SH4_TCNT1_ADDR  0x3B06  /* FFD80018 */
#define SH4_TCR1_ADDR   0x3B07  /* FFD8001C */
#define SH4_TCOR2_ADDR  0x3B08  /* FFD80020 */
#define SH4_TCNT2_ADDR  0x3B09  /* FFD80024 */
#define SH4_TCR2_ADDR   0x3B0A  /* FFD80028 */
#define SH4_TCPR2_ADDR  0x3B0B  /* FFD8002C */
#define SCSMR1  0x3C00  /* FFE00000 */
#define SCBRR1  0x3C01  /* FFE00004 */
#define SCSCR1  0x3C02  /* FFE00008 */
#define SCTDR1  0x3C03  /* FFE0000C */
#define SCSSR1  0x3C04  /* FFE00010 */
#define SCRDR1  0x3C05  /* FFE00014 */
#define SCSCMR1 0x3C06  /* FFE00018 */
#define SCSPTR1 0x3C07  /* FFE0001C */
#define SCSMR2  0x3D00  /* FFE80000 */
#define SCBRR2  0x3D01  /* FFE80004 */
#define SCSCR2  0x3D02  /* FFE80008 */
#define SCFTDR2 0x3D03  /* FFE8000C */
#define SCFSR2  0x3D04  /* FFE80010 */
#define SCFRDR2 0x3D05  /* FFE80014 */
#define SCFCR2  0x3D06  /* FFE80018 */
#define SCFDR2  0x3D07  /* FFE8001C */
#define SCSPTR2 0x3D08  /* FFE80020 */
#define SCLSR2  0x3D09  /* FFE80024 */
#define SDIR    0x3E00  /* FFF00000 */
#define SDDR    0x3E02  /* FFF00008 */
#define SDINT   0x3E05  /* FFF00014 */
#define SIZEREGS 15878



#define MMUCR_LRUI  0xfc000000
#define MMUCR_URB   0x00fc0000
#define MMUCR_URC   0x0000fc00
#define MMUCR_SQMD  0x00000200
#define MMUCR_SV    0x00000100
#define MMUCR_TI    0x00000004
#define MMUCR_AT    0x00000001

/* constants */
#define PVR_SH7091  0x040205c1
#define PVR_SH7750  0x04020500 // from TN-SH7-361B/E
#define PVR_SH7750S 0x04020600
#define PVR_SH7750R 0x04050000
#define PRR_SH7750R 0x00000100
#define PVR_SH7751  0x04110000
#define PVR_SH7751R 0x04050000
#define PRR_SH7751R 0x00000110

#endif /* __SH4REGS_H__ */
