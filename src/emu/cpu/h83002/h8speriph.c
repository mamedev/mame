/*

    Hitachi H8S/2xxx MCU peripherals

    (c) 2001-2007 Tim Schuerewegen

    H8S/2241
    H8S/2246
    H8S/2323
    H8S/2394
*/

#include "emu.h"
#include "debugger.h"
#include "h8.h"
#include "h8priv.h"

#define VERBOSE_LEVEL ( 0 )

INLINE void ATTR_PRINTF(3,4) verboselog( running_machine &machine, int n_level, const char *s_fmt, ...)
{
	if (VERBOSE_LEVEL >= n_level)
	{
		va_list v;
		char buf[32768];
		va_start( v, s_fmt);
		vsprintf( buf, s_fmt, v);
		va_end( v);
		logerror( "%s: %s", machine.describe_context( ), buf);
	}
}

const UINT8 H8S_RESET_H8S_IO_224x[0x1C0] = // values from "e602100_h8s2245.pdf"
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE40
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE50
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE70
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE80
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE90
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEA0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, // FEB0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEC0
	0xFF, 0xFF, 0xFF, 0xFF, 0xD0, 0x3C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FED0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEE0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEF0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF00
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x3F, 0xFF, 0x00, 0x00, // FF30
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF40
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF50
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, // FF70
	0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, // FF80
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF90
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FFA0
	0x00, 0x00, 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FFB0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FFC0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // FFD0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FFE0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FFF0
};

const UINT8 H8S_RESET_H8S_IO_2323[0x1C0] = // values from "rej09b0220_h8s2329.pdf"
{
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE40
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE50
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FE70
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // FE80
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FE90
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FEA0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00, // FEB0
	0x00, 0x00, 0x00, 0x00, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x00, // FEC0
	0xFF, 0xFF, 0xFF, 0xFF, 0xD0, 0x3C, 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FED0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEE0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FEF0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF00
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF10
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x01, 0x00, 0x00, 0x3F, 0xFF, 0x00, 0x00, // FF30
	0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF40
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF50
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF60
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, // FF70
	0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, 0x00, 0xFF, 0x00, 0xFF, 0x84, 0x00, 0xF2, 0x00, // FF80
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FF90
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, // FFA0
	0x00, 0x00, 0x00, 0x10, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FFB0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // FFC0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // FFD0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FFE0
	0x00, 0xC0, 0x00, 0x00, 0x40, 0xC0, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, // FFF0
};

#define SCR_TIE		0x80
#define SCR_RIE		0x40
#define SCR_TE		0x20
#define SCR_RE		0x10

#define SSR_TDRE  0x80
#define SSR_RDRF  0x40
#define SSR_ORER  0x20
#define SSR_FER   0x10
#define SSR_PER   0x08
#define SSR_TEND  0x04

#define SMR_CM    0x80
#define SMR_CHR   0x40
#define SMR_PE    0x20
#define SMR_PM    0x10
#define SMR_STOP  0x08
#define SMR_MP    0x04
#define SMR_CKS1  0x02
#define SMR_CKS0  0x01

#define TCSR_CMFB  0x80
#define TCSR_CMFA  0x40
#define TCSR_OVF   0x20

#define TCR_CMIEB  0x80
#define TCR_CMIEA  0x40
#define TCR_OVIE   0x20

#define TSR_TCFD  0x80
#define TSR_TCFU  0x20
#define TSR_TCFV  0x10
#define TSR_TGFD  0x08
#define TSR_TGFC  0x04
#define TSR_TGFB  0x02
#define TSR_TGFA  0x01

#define TIER_TCIEU  0x20
#define TIER_TCIEV  0x10
#define TIER_TGIED  0x08
#define TIER_TGIEC  0x04
#define TIER_TGIEB  0x02
#define TIER_TGIEA  0x01

#define DTC_MRA_MD_NORMAL  0
#define DTC_MRA_MD_REPEAT  1
#define DTC_MRA_MD_BLOCK   2

#define DTC_MRA_SM1  0x80
#define DTC_MRA_SM0  0x40
#define DTC_MRA_DM1  0x20
#define DTC_MRA_DM0  0x10
#define DTC_MRA_DTS  0x02

#define DTC_MRB_CHNE   0x80
#define DTC_MRB_DISEL  0x40
#define DTC_MRB_CHNS   0x20

#define DMABCRL_DTME1   0x80
#define DMABCRL_DTE1    0x40
#define DMABCRL_DTME0   0x20
#define DMABCRL_DTE0    0x10
#define DMABCRL_DTIE1B  0x08
#define DMABCRL_DTIE1A  0x04
#define DMABCRL_DTIE0B  0x02
#define DMABCRL_DTIE0A  0x01

#define ADCSR_ADF   0x80
#define ADCSR_ADIE  0x40
#define ADCSR_ADST  0x20
#define ADCSR_SCAN  0x10

enum
{
	PIN_P10, PIN_P11, PIN_P12, PIN_P13, PIN_P14, PIN_P15, PIN_P16, PIN_P17,
	PIN_P20, PIN_P21, PIN_P22, PIN_P23, PIN_P24, PIN_P25, PIN_P26, PIN_P27,
	PIN_P30, PIN_P31, PIN_P32, PIN_P33, PIN_P34, PIN_P35,
	PIN_P40, PIN_P41, PIN_P42, PIN_P43, PIN_P44, PIN_P45, PIN_P46, PIN_P47,
	PIN_P50, PIN_P51, PIN_P52, PIN_P53,
	PIN_P60, PIN_P61, PIN_P62, PIN_P63, PIN_P64, PIN_P65, PIN_P66, PIN_P67,
	PIN_PA0, PIN_PA1, PIN_PA2, PIN_PA3, PIN_PA4, PIN_PA5, PIN_PA6, PIN_PA7,
	PIN_PB0, PIN_PB1, PIN_PB2, PIN_PB3, PIN_PB4, PIN_PB5, PIN_PB6, PIN_PB7,
	PIN_PC0, PIN_PC1, PIN_PC2, PIN_PC3, PIN_PC4, PIN_PC5, PIN_PC6, PIN_PC7,
	PIN_PD0, PIN_PD1, PIN_PD2, PIN_PD3, PIN_PD4, PIN_PD5, PIN_PD6, PIN_PD7,
	PIN_PE0, PIN_PE1, PIN_PE2, PIN_PE3, PIN_PE4, PIN_PE5, PIN_PE6, PIN_PE7,
	PIN_PF0, PIN_PF1, PIN_PF2, PIN_PF3, PIN_PF4, PIN_PF5, PIN_PF6, PIN_PF7,
	PIN_PG0, PIN_PG1, PIN_PG2, PIN_PG3, PIN_PG4
};

#define PIN_TIOCA0	PIN_P10
#define PIN_TIOCB0	PIN_P11
#define PIN_TIOCC0	PIN_P12
#define PIN_TIOCD0	PIN_P13
#define PIN_TIOCA1	PIN_P14
#define PIN_TIOCB1	PIN_P15
#define PIN_TIOCA2	PIN_P16
#define PIN_TIOCB2	PIN_P17

#define PIN_TCLKA	PIN_P12
#define PIN_TCLKB	PIN_P13
#define PIN_TCLKC	PIN_P15
#define PIN_TCLKD	PIN_P17

#define PIN_TXD0	PIN_P30
#define PIN_TXD1	PIN_P31
#define PIN_RXD0	PIN_P32
#define PIN_RXD1	PIN_P33
#define PIN_SCK0	PIN_P34
#define PIN_SCK1	PIN_P35

#define PIN_TIOCA3	PIN_P20
#define PIN_TIOCB3	PIN_P21
#define PIN_TIOCC3	PIN_P22
#define PIN_TIOCD3	PIN_P23
#define PIN_TIOCA4	PIN_P24
#define PIN_TIOCB4	PIN_P25
#define PIN_TIOCA5	PIN_P26
#define PIN_TIOCB5	PIN_P27

#define PIN_TMRI0	PIN_P22
#define PIN_TMCI0	PIN_P23
#define PIN_TMRI1	PIN_P24
#define PIN_TMCI1	PIN_P25
#define PIN_TMO0	PIN_P26
#define PIN_TMO1	PIN_P27

const int TPU_0_DIV[] = { 1, 4, 16, 64, 0, 0, 0,0 };
const int TPU_1_DIV[] = { 1, 4, 16, 64, 0, 0, 256, 0 };
const int TPU_2_DIV[] = { 1, 4, 16, 64, 0, 0, 0, 1024 };
const int TPU_3_DIV[] = { 1, 4, 16, 64, 0, 1024, 256, 4096 };
const int TPU_4_DIV[] = { 1, 4, 16, 64, 0, 0, 1024, 0 };
const int TPU_5_DIV[] = { 1, 4, 16, 64, 0, 0, 256, 0 };

const int H8S_IO_SMR[] = { H8S_IO_SMR0, H8S_IO_SMR1, H8S_IO_SMR2 };
const int H8S_IO_BRR[] = { H8S_IO_BRR0, H8S_IO_BRR1, H8S_IO_BRR2 };
const int H8S_IO_SCR[] = { H8S_IO_SCR0, H8S_IO_SCR1, H8S_IO_SCR2 };
const int H8S_IO_TDR[] = { H8S_IO_TDR0, H8S_IO_TDR1, H8S_IO_TDR2 };
const int H8S_IO_SSR[] = { H8S_IO_SSR0, H8S_IO_SSR1, H8S_IO_SSR2 };
const int H8S_IO_RDR[] = { H8S_IO_RDR0, H8S_IO_RDR1, H8S_IO_RDR2 };

const int H8S_INT_TXI[] = { H8S_INT_TXI0, H8S_INT_TXI1, H8S_INT_TXI2 };
const int H8S_INT_RXI[] = { H8S_INT_RXI0, H8S_INT_RXI1, H8S_INT_RXI2 };

const int H8S_IO_TCORA[] = { H8S_IO_TCORA0, H8S_IO_TCORA1 };
const int H8S_IO_TCORB[] = { H8S_IO_TCORB0, H8S_IO_TCORB1 };
const int H8S_IO_TCR[]   = { H8S_IO_TCR0,   H8S_IO_TCR1   };
const int H8S_IO_TCNT[]  = { H8S_IO_TCNT0,  H8S_IO_TCNT1  };
const int H8S_IO_TCSR[]  = { H8S_IO_TCSR0,  H8S_IO_TCSR1  };

const int H8S_INT_OVI[]  = { H8S_INT_OVI0,  H8S_INT_OVI1  };
const int H8S_INT_CMIA[] = { H8S_INT_CMIA0, H8S_INT_CMIA1 };
const int H8S_INT_CMIB[] = { H8S_INT_CMIB0, H8S_INT_CMIB1 };

const int H8S_IO_TGRA_H[] = { H8S_IO_TGR0A_H, H8S_IO_TGR1A_H, H8S_IO_TGR2A_H, H8S_IO_TGR3A_H, H8S_IO_TGR4A_H, H8S_IO_TGR5A_H };
const int H8S_IO_TGRA_L[] = { H8S_IO_TGR0A_L, H8S_IO_TGR1A_L, H8S_IO_TGR2A_L, H8S_IO_TGR3A_L, H8S_IO_TGR4A_L, H8S_IO_TGR5A_L };
const int H8S_IO_TGRB_H[] = { H8S_IO_TGR0B_H, H8S_IO_TGR1B_H, H8S_IO_TGR2B_H, H8S_IO_TGR3B_H, H8S_IO_TGR4B_H, H8S_IO_TGR5B_H };
const int H8S_IO_TGRB_L[] = { H8S_IO_TGR0B_L, H8S_IO_TGR1B_L, H8S_IO_TGR2B_L, H8S_IO_TGR3B_L, H8S_IO_TGR4B_L, H8S_IO_TGR5B_L };
const int H8S_IO_TGRC_H[] = { H8S_IO_TGR0C_H, 0, 0, H8S_IO_TGR3C_H, 0, 0 };
const int H8S_IO_TGRC_L[] = { H8S_IO_TGR0C_L, 0, 0, H8S_IO_TGR3C_L, 0, 0 };
const int H8S_IO_TGRD_H[] = { H8S_IO_TGR0D_H, 0, 0, H8S_IO_TGR3D_H, 0, 0 };
const int H8S_IO_TGRD_L[] = { H8S_IO_TGR0D_L, 0, 0, H8S_IO_TGR3D_L, 0, 0 };
const int H8S_IO_TCNT_H[] = { H8S_IO_TCNT0_H, H8S_IO_TCNT1_H, H8S_IO_TCNT2_H, H8S_IO_TCNT3_H, H8S_IO_TCNT4_H, H8S_IO_TCNT5_H };
const int H8S_IO_TCNT_L[] = { H8S_IO_TCNT0_L, H8S_IO_TCNT1_L, H8S_IO_TCNT2_L, H8S_IO_TCNT3_L, H8S_IO_TCNT4_L, H8S_IO_TCNT5_L };
const int H8S_IO_TIOR_H[] = { H8S_IO_TIOR0_H, H8S_IO_TIOR1, H8S_IO_TIOR2, H8S_IO_TIOR3_H, H8S_IO_TIOR4, H8S_IO_TIOR5 };
const int H8S_IO_TIOR_L[] = { H8S_IO_TIOR0_L, 0, 0, H8S_IO_TIOR3_L, 0, 0 };
const int H8S_IO_TTCR[]   = { H8S_IO_TTCR0, H8S_IO_TTCR1, H8S_IO_TTCR2, H8S_IO_TTCR3, H8S_IO_TTCR4, H8S_IO_TTCR5 };
const int H8S_IO_TIER[]   = { H8S_IO_TIER0, H8S_IO_TIER1, H8S_IO_TIER2, H8S_IO_TIER3, H8S_IO_TIER4, H8S_IO_TIER5 };
const int H8S_IO_TSR[]    = { H8S_IO_TSR0, H8S_IO_TSR1, H8S_IO_TSR2, H8S_IO_TSR3, H8S_IO_TSR4, H8S_IO_TSR5 };

const int H8S_INT_TCIV[] = { H8S_INT_TCI0V, H8S_INT_TCI1V, H8S_INT_TCI2V, H8S_INT_TCI3V, H8S_INT_TCI4V, H8S_INT_TCI5V };
const int H8S_INT_TCIU[] = { 0, H8S_INT_TCI1U, H8S_INT_TCI2U, 0, H8S_INT_TCI4U, H8S_INT_TCI5U };
const int H8S_INT_TGIA[] = { H8S_INT_TGI0A, H8S_INT_TGI1A, H8S_INT_TGI2A, H8S_INT_TGI3A, H8S_INT_TGI4A, H8S_INT_TGI5A };
const int H8S_INT_TGIB[] = { H8S_INT_TGI0B, H8S_INT_TGI1B, H8S_INT_TGI2B, H8S_INT_TGI3B, H8S_INT_TGI4B, H8S_INT_TGI5B };
const int H8S_INT_TGIC[] = { H8S_INT_TGI0C, 0, 0, H8S_INT_TGI3C, 0, 0 };
const int H8S_INT_TGID[] = { H8S_INT_TGI0D, 0, 0, H8S_INT_TGI3D, 0, 0 };

const int PIN_TIOCA[] = { PIN_TIOCA0, PIN_TIOCA1, PIN_TIOCA2, PIN_TIOCA3, PIN_TIOCA4, PIN_TIOCA5 };
const int PIN_TIOCB[] = { PIN_TIOCB0, PIN_TIOCB1, PIN_TIOCB2, PIN_TIOCB3, PIN_TIOCB4, PIN_TIOCB5 };
const int PIN_TIOCC[] = { PIN_TIOCC0, 0, 0, PIN_TIOCC3, 0, 0 };
const int PIN_TIOCD[] = { PIN_TIOCD0, 0, 0, PIN_TIOCD3, 0, 0 };

const int TCR_CCLR_AND[] = { 7, 3, 3, 7, 3, 3 };

// prototypes

void h8s_tmr_x_reset( h83xx_state *h8, int x);
void h8s_tpu_x_reset( h83xx_state *h8, int x);
void h8s_tpu_x_tick( h83xx_state *h8, int x);

////////////
// DTVECR //
////////////

#define DTVECR_ADDR(x) (0x400 + (x << 1))

///////////////
// MEMCONV.H //
///////////////

static TIMER_CALLBACK( h8s_tmr_callback );
static TIMER_CALLBACK( h8s_tpu_callback );
static TIMER_CALLBACK( h8s_sci_callback );

/////////////
// TIMER.C //
/////////////

UINT16 ptr_read_16( void *ptr)
{
	return (*(((UINT8*)ptr) + 0) << 8) | (*(((UINT8*)ptr) + 1) << 0);
}

void ptr_write_16( void *ptr, UINT16 data)
{
	*(((UINT8*)ptr) + 0) = (data >> 8) & 0xFF;
	*(((UINT8*)ptr) + 1) = (data >> 0) & 0xFF;
}

int device_icount_min_timer_icount1( h83xx_state *h8, emu_timer *timer)
{
	return (timer->elapsed() * h8->device->unscaled_clock()).as_double();
}

void h8s2xxx_interrupt_request(h83xx_state *h8, UINT32 vecnum)
{
	UINT8 idx, bit;
	idx = vecnum >> 5;
	bit = vecnum & 0x1F;
	h8->irq_req[idx] |= (1 << bit);
}

void h8s_dtce_execute(h83xx_state *h8, UINT32 addr_dtce, UINT8 mask_dtce, UINT32 addr_dtvecr)
{
	UINT32 data[3], dtc_vect, dtc_sar, dtc_dar, cnt, i;
	UINT8 dtc_mra, dtc_mrb, sz;
	UINT16 dtc_cra, dtc_crb;
	// get dtc info
	dtc_vect  = 0xFF0000 | h8->program->read_word(addr_dtvecr);
	data[0]   = h8->program->read_dword( dtc_vect + 0);
	data[1]   = h8->program->read_dword( dtc_vect + 4);
	data[2]   = h8->program->read_dword( dtc_vect + 8);
	dtc_mra   = (data[0] >> 24) & 0xFF;
	dtc_sar   = (data[0] >>  0) & 0xFFFFFF;
	dtc_mrb   = (data[1] >> 24) & 0xFF;
	dtc_dar   = (data[1] >>  0) & 0xFFFFFF;
	dtc_cra   = (data[2] >> 16) & 0xFFFF;
	dtc_crb   = (data[2] >>  0) & 0xFFFF;
	verboselog( h8->device->machine(), 3, "dtc : vect %08X mra %02X sar %08X mrb %02X dar %08X cra %04X crb %04X\n", dtc_vect, dtc_mra, dtc_sar, dtc_mrb, dtc_dar, dtc_cra, dtc_crb);
	// execute
	if ((dtc_mra & 0x0E) != 0x00) fatalerror("H8S: dtc unsupported MRA %x\n", dtc_mra&0x0e);
	sz = 1 << (dtc_mra & 0x01);
	cnt = dtc_cra;
	for (i=0;i<cnt;i++)
	{
		if (dtc_sar == H8S_IO_ADDR( H8S_IO_RDR1)) h8->program->write_byte( H8S_IO_ADDR( H8S_IO_SSR1), h8->program->read_byte( H8S_IO_ADDR( H8S_IO_SSR1)) & (~H8S_SSR_TDRE));
		if (dtc_mra & 0x01) h8->program->write_word( dtc_dar, h8->program->read_word( dtc_sar)); else h8->program->write_byte( dtc_dar, h8->program->read_byte( dtc_sar));
		if (dtc_dar == H8S_IO_ADDR( H8S_IO_TDR0)) h8->program->write_byte( H8S_IO_ADDR( H8S_IO_SSR0), h8->program->read_byte( H8S_IO_ADDR( H8S_IO_SSR0)) & (~H8S_SSR_TDRE));
		if (dtc_mra & 0x80) { if (dtc_mra & 0x40) dtc_sar -= sz; else dtc_sar += sz; }
		if (dtc_mra & 0x20) { if (dtc_mra & 0x10) dtc_dar -= sz; else dtc_dar += sz; }
	}
	h8->program->write_byte( addr_dtce, h8->program->read_byte( addr_dtce) & (~mask_dtce));
}

void h8s_dtce_check(h83xx_state *h8,  int vecnum)
{
	UINT32 dtce = 0;
	int bit = 0;
	// get dtce info
	switch (vecnum)
	{
		// DTCEA
		case H8S_INT_IRQ0  : dtce = H8S_IO_DTCEA; bit = 7; break;
		case H8S_INT_IRQ1  : dtce = H8S_IO_DTCEA; bit = 6; break;
		case H8S_INT_IRQ2  : dtce = H8S_IO_DTCEA; bit = 5; break;
		case H8S_INT_IRQ3  : dtce = H8S_IO_DTCEA; bit = 4; break;
		case H8S_INT_IRQ4  : dtce = H8S_IO_DTCEA; bit = 3; break;
		case H8S_INT_IRQ5  : dtce = H8S_IO_DTCEA; bit = 2; break;
		case H8S_INT_IRQ6  : dtce = H8S_IO_DTCEA; bit = 1; break;
		case H8S_INT_IRQ7  : dtce = H8S_IO_DTCEA; bit = 0; break;
		// DTCEB
		case H8S_INT_ADI   : dtce = H8S_IO_DTCEB; bit = 6; break;
		case H8S_INT_TGI0A : dtce = H8S_IO_DTCEB; bit = 5; break;
		case H8S_INT_TGI0B : dtce = H8S_IO_DTCEB; bit = 4; break;
		case H8S_INT_TGI0C : dtce = H8S_IO_DTCEB; bit = 3; break;
		case H8S_INT_TGI0D : dtce = H8S_IO_DTCEB; bit = 2; break;
		case H8S_INT_TGI1A : dtce = H8S_IO_DTCEB; bit = 1; break;
		case H8S_INT_TGI1B : dtce = H8S_IO_DTCEB; bit = 0; break;
		// DTCEC
		case H8S_INT_TGI2A : dtce = H8S_IO_DTCEC; bit = 7; break;
		case H8S_INT_TGI2B : dtce = H8S_IO_DTCEC; bit = 6; break;
		// DTCED
		case H8S_INT_CMIA0 : dtce = H8S_IO_DTCED; bit = 3; break;
		case H8S_INT_CMIB0 : dtce = H8S_IO_DTCED; bit = 2; break;
		case H8S_INT_CMIA1 : dtce = H8S_IO_DTCED; bit = 1; break;
		case H8S_INT_CMIB1 : dtce = H8S_IO_DTCED; bit = 0; break;
		// DTCEE
		case H8S_INT_RXI0  : dtce = H8S_IO_DTCEE; bit = 3; break;
		case H8S_INT_TXI0  : dtce = H8S_IO_DTCEE; bit = 2; break;
		case H8S_INT_RXI1  : dtce = H8S_IO_DTCEE; bit = 1; break;
		case H8S_INT_TXI1  : dtce = H8S_IO_DTCEE; bit = 0; break;
		// DTCEF
		case H8S_INT_RXI2  : dtce = H8S_IO_DTCEF; bit = 7; break;
		case H8S_INT_TXI2  : dtce = H8S_IO_DTCEF; bit = 6; break;
	}
	// execute
	if ((dtce != 0) && (h8->per_regs[dtce] & (1 << bit))) h8s_dtce_execute(h8, H8S_IO_ADDR( dtce), (1 << bit), DTVECR_ADDR( vecnum));
}

void h8s_periph_reset(h83xx_state *h8)
{
	const int tpu_max = ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) ? 6 : 3;
	if ((h8->device->type() == H8S2241) || (h8->device->type() == H8S2246))
	{
		memcpy( h8->per_regs, H8S_RESET_H8S_IO_224x, sizeof( h8->per_regs));
	}
	else if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394))
	{
		memcpy( h8->per_regs, H8S_RESET_H8S_IO_2323, sizeof( h8->per_regs));
	}
	for (int i = 0; i < 2; i++) h8s_tmr_x_reset( h8, i);
	for (int i = 0; i < tpu_max; i++) h8s_tpu_x_reset( h8, i);
}

/////////////////
// TIMER 8-BIT //
/////////////////

void h8s_tmr_init(h83xx_state *h8)
{
	for (int i = 0; i < 2; i++)
	{
		h8->tmr[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_tmr_callback), h8);
		h8->tmr[i].timer->adjust(attotime::never, i);
	}
}

void h8s_tmr_x_reset( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	tmr->cycles_per_tick = 0;
	tmr->timer->adjust(attotime::never, x);
}

UINT64 h8s_tmr_x_calculate_ticks( h83xx_state *h8, int x, UINT8 tcnt)
{
	UINT64 cycles1, cycles2;
	UINT8 tcor;
	// overflow
	if (tcnt == 0) cycles1 = 0xFF + 1; else cycles1 = (UINT8)(0xFF - tcnt + 1);
	// tcora
	tcor = h8->per_regs[H8S_IO_TCORA[x]];
	if (tcnt == tcor) cycles2 = 0xFF + 1; else cycles2 = (UINT8)(tcor - tcnt);
	if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	// tcorb
	tcor = h8->per_regs[H8S_IO_TCORB[x]];
	if (tcnt == tcor) cycles2 = 0xFF + 1; else cycles2 = (UINT8)(tcor - tcnt);
	if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	// done
	return cycles1;
}

void h8s_tmr_x_update( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	UINT64 cycles;
	int div = 0;
	UINT8 tcr, tcnt;
	tcr = h8->per_regs[H8S_IO_TCR[x]];
	switch ((tcr >> 0) & 7)
	{
		case 1  : div = 8; break;
		case 2  : div = 64; break;
		case 3  : div = 8192; break;
//      default : OS_MessageBox( NULL, "tmr_x_update", "error", 0); break;
	}
	tmr->cycles_per_tick = div;
	tcnt = h8->per_regs[H8S_IO_TCNT[x]];
	verboselog( h8->device->machine(), 5, "TMR %d - update (div %d tcnt %02X)\n", x, div, tcnt);
	// update timer
	cycles = h8s_tmr_x_calculate_ticks( h8, x, tcnt) * tmr->cycles_per_tick;
	tmr->timer_cycles = cycles;
	tmr->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
}

UINT8 h8s_tmr_x_calculate_tcnt( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	UINT8 tcnt;
	tcnt = h8->per_regs[H8S_IO_TCNT[x]];
	if (tmr->timer->enabled())
	{
		tcnt = (UINT8)(tcnt + ((device_icount_min_timer_icount1( h8, tmr->timer)) / tmr->cycles_per_tick));
	}
	return tcnt;
}

void h8s_tmr_x_start( h83xx_state *h8, int x)
{
	verboselog( h8->device->machine(), 5, "TMR %d - start\n", x);
	// ...
	h8s_tmr_x_update( h8, x);
}

void h8s_tmr_x_stop( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	UINT8 tcnt;
	// ...
	verboselog( h8->device->machine(), 5, "TMR %d - stop\n", x);
	// calculate tcnt
	tcnt = h8s_tmr_x_calculate_tcnt( h8, x);
	// update tcnt
	h8->per_regs[H8S_IO_TCNT[x]] = tcnt;
	// disable timer
	tmr->timer->reset();
}

UINT8 h8s_tmr_x_tcnt_change( h83xx_state *h8, int x, UINT8 tcnt)
{
	UINT8 tcr, tcor;
	int clear = 0;
	// ...
	verboselog( h8->device->machine(), 9, "TMR %d - tmr_x_tcnt_change (%02X)\n", x, tcnt);
	// ...
	tcr = h8->per_regs[H8S_IO_TCR[x]];
	// overflow
	if (tcnt == 0)
	{
		verboselog( h8->device->machine(), 9, "TMR %d - overflow match\n", x);
		h8->per_regs[H8S_IO_TCSR[x]] |= TCSR_OVF;
		if ((tcr & TCR_OVIE) != 0)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_OVI[x]);
		}
	}
	// tcora
	tcor = h8->per_regs[H8S_IO_TCORA[x]];
	if (tcnt == tcor)
	{
		verboselog( h8->device->machine(), 9, "TMR %d - tcora match\n", x);
		h8->per_regs[H8S_IO_TCSR[x]] |= TCSR_CMFA;
		if ((tcr & TCR_CMIEA) != 0)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_CMIA[x]);
		}
		if (((tcr >> 3) & 3) == 1) // CCLR 1 -> "clear by compare match a"
		{
			clear = 1;
		}
	}
	// tcorb
	tcor = h8->per_regs[H8S_IO_TCORB[x]];
	if (tcnt == tcor)
	{
		verboselog( h8->device->machine(), 9, "TMR %d - tcorb match\n", x);
		h8->per_regs[H8S_IO_TCSR[x]] |= TCSR_CMFB;
		if ((tcr & TCR_CMIEB) != 0)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_CMIB[x]);
		}
		if (((tcr >> 3) & 3) == 2) // CCLR 2 -> "clear by compare match b"
		{
			clear = 1;
		}
	}
	// ...
	if (clear)
	{
		verboselog( h8->device->machine(), 5, "TMR %d - tcnt clear\n", x);
		tcnt = 0;
	}
	// ...
	return tcnt;
}

void h8s_tmr_x_callback( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	UINT64 cycles;
	UINT8 tcnt;
	// calculate tcnt
	tcnt = h8->per_regs[H8S_IO_TCNT[x]];
	tcnt = (UINT8)(tcnt + tmr->timer_cycles / tmr->cycles_per_tick);
	// ...
	tcnt = h8s_tmr_x_tcnt_change( h8, x, tcnt);
	// update tcnt
	h8->per_regs[H8S_IO_TCNT[x]] = tcnt;
	// update timer
	cycles = h8s_tmr_x_calculate_ticks( h8, x, tcnt) * tmr->cycles_per_tick;
	tmr->timer_cycles = cycles;
	tmr->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
}

void h8s_tmr_x_write_tcr( h83xx_state *h8, int x, UINT8 data)
{
	UINT8 old_data = h8->per_regs[H8S_IO_TCR[x]];
	h8->per_regs[H8S_IO_TCR[x]] = data;
	if ((data & 0x07) != (old_data & 0x07))
	{
		if ((data & 0x07) != 0)
		{
			h8s_tmr_x_start( h8, x);
		}
		else
		{
			h8s_tmr_x_stop( h8, x);
		}
	}
}

void h8s_tmr_x_write_tcnt( h83xx_state *h8, int x, UINT8 data)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	h8->per_regs[H8S_IO_TCNT[x]] = data;
	if (tmr->timer->enabled())
	{
		h8s_tmr_x_update( h8, x);
	}
}

void h8s_tmr_x_write_tcor( h83xx_state *h8, int x)
{
	H8S2XXX_TMR *tmr = &h8->tmr[x];
	if ((tmr->timer->enabled()) && (tmr->cycles_per_tick != 0))
	{
		UINT8 tcnt1, tcnt2;
		UINT64 cycles;
		tcnt1 = h8->per_regs[H8S_IO_TCNT[x]];
		tcnt2 = (UINT8)(tcnt1 + ((device_icount_min_timer_icount1( h8, tmr->timer)) / tmr->cycles_per_tick));
		cycles = h8s_tmr_x_calculate_ticks( h8, x, tcnt2) * tmr->cycles_per_tick;
		cycles = cycles + (tcnt2 - tcnt1) * tmr->cycles_per_tick;
		tmr->timer_cycles = cycles;
		tmr->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
	}
}

UINT8 h8s_tmr_x_read_tcnt( h83xx_state *h8, int x)
{
	return h8s_tmr_x_calculate_tcnt( h8, x);
}

void h8s_tmr_x_write_tcora( h83xx_state *h8, int x, UINT8 data)
{
	h8->per_regs[H8S_IO_TCORA[x]] = data;
	h8s_tmr_x_write_tcor( h8, x);
}

void h8s_tmr_x_write_tcorb( h83xx_state *h8, int x, UINT8 data)
{
	h8->per_regs[H8S_IO_TCORB[x]] = data;
	h8s_tmr_x_write_tcor( h8, x);
}

static TIMER_CALLBACK( h8s_tmr_callback)
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	h8s_tmr_x_callback( h8, param);
}

//////////////////
// TIMER 16-BIT //
//////////////////

void h8s_tpu_init(h83xx_state *h8)
{
	const int tpu_max = ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) ? 6 : 3;
	for (int i = 0; i < tpu_max; i++)
	{
		h8->tpu[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_tpu_callback), h8);
		h8->tpu[i].timer->adjust(attotime::never, i);
	}
}

void h8s_tpu_x_reset( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	tpu->cycles_per_tick = 0;
	tpu->timer->reset();
}

UINT16 h8s_tpu_x_calculate_tcnt( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	UINT16 tcnt;
	tcnt = ptr_read_16( h8->per_regs + H8S_IO_TCNT_H[x]);
	if ((tpu->timer->enabled()) && (tpu->cycles_per_tick > 0))
	{
		tcnt = (UINT16)(tcnt + ((device_icount_min_timer_icount1( h8, tpu->timer)) / tpu->cycles_per_tick));
	}
	return tcnt;
}

UINT64 h8s_tpu_x_calculate_ticks( h83xx_state *h8, int x, UINT16 tcnt)
{
	UINT64 cycles1, cycles2;
	UINT16 tgr;
	// overflow
	if (tcnt == 0) cycles1 = 0xFFFF + 1; else cycles1 = (UINT16)(0xFFFF - tcnt + 1);
	// tgra
	tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRA_H[x]);
	if (tcnt == tgr) cycles2 = 0xFFFF + 1; else cycles2 = (UINT16)(tgr - tcnt);
	if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	// tgrb
	tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRB_H[x]);
	if (tcnt == tgr) cycles2 = 0xFFFF + 1; else cycles2 = (UINT16)(tgr - tcnt);
	if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	// tgrc
	if ((x == 0) || (x == 3))
	{
		tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRC_H[x]);
		if (tcnt == tgr) cycles2 = 0xFFFF + 1; else cycles2 = (UINT16)(tgr - tcnt);
		if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	}
	// tgrd
	if ((x == 0) || (x == 3))
	{
		tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRD_H[x]);
		if (tcnt == tgr) cycles2 = 0xFFFF + 1; else cycles2 = (UINT16)(tgr - tcnt);
		if ((cycles2 > 0) && (cycles2 < cycles1)) cycles1 = cycles2;
	}
	// done
	return cycles1;
}

void h8s_pin_write( h83xx_state *h8, int pin, int data)
{
	switch (pin)
	{
		case PIN_TIOCB1 :
		{
			UINT8 p1dr;
			p1dr = h8->per_regs[H8S_IO_P1DR];
			if (data) p1dr |= H8S_P1_TIOCB1; else p1dr &= ~H8S_P1_TIOCB1;
			h8->io->write_byte(H8S_IO_ADDR(H8S_IO_P1DR), p1dr);
			h8->per_regs[H8S_IO_P1DR] = p1dr;
		}
		break;
	}
}

int h8s_pin_read( h83xx_state *h8, int pin)
{
	switch (pin)
	{
		case PIN_TIOCB1 : return (h8->per_regs[H8S_IO_P1DR] & H8S_P1_TIOCB1) ? 1 : 0;
	}
	return 0;
}

void h8s_tioc_pin_update( h83xx_state *h8, int io, int pin)
{
	switch (io)
	{
		case 1 : h8s_pin_write( h8, pin, 0); break;
		case 2 : h8s_pin_write( h8, pin, 1); break;
		case 3 : h8s_pin_write( h8, pin, h8s_pin_read( h8, pin) ? 0 : 1); break;
		case 5 : h8s_pin_write( h8, pin, 0); break;
		case 6 : h8s_pin_write( h8, pin, 1); break;
		case 7 : h8s_pin_write( h8, pin, h8s_pin_read( h8, pin) ? 0 : 1); break;
	}
}

void h8s_tioc_pin_reset( h83xx_state *h8, int io, int pin)
{
	switch (io)
	{
		case 1 : h8s_pin_write( h8, pin, 0); break;
		case 2 : h8s_pin_write( h8, pin, 0); break;
		case 3 : h8s_pin_write( h8, pin, 0); break;
		case 5 : h8s_pin_write( h8, pin, 1); break;
		case 6 : h8s_pin_write( h8, pin, 1); break;
		case 7 : h8s_pin_write( h8, pin, 1); break;
	}
}

void h8s_tpu_x_tioc_reset( h83xx_state *h8, int x)
{
	UINT8 tior;
	int cclr;
	// ...
	cclr = (h8->per_regs[H8S_IO_TTCR[x]] >> 5) & TCR_CCLR_AND[x];
	// tioca & tiocb
	tior = h8->per_regs[H8S_IO_TIOR_H[x]];
	if (cclr != 1) h8s_tioc_pin_reset( h8, (tior >> 0) & 0x0F, PIN_TIOCA[x]);
	if (cclr != 2) h8s_tioc_pin_reset( h8, (tior >> 4) & 0x0F, PIN_TIOCB[x]);
	// tiocc & tiocd
	if ((x == 0) || (x == 3))
	{
		tior = h8->per_regs[H8S_IO_TIOR_L[x]];
		if (cclr != 5) h8s_tioc_pin_reset( h8, (tior >> 0) & 0x0F, PIN_TIOCC[x]);
		if (cclr != 6) h8s_tioc_pin_reset( h8, (tior >> 4) & 0x0F, PIN_TIOCD[x]);
	}
}

void h8s_tpu_x_overflow( h83xx_state *h8, int x)
{
	if (x == 2)
	{
		UINT8 tcr = h8->per_regs[H8S_IO_TTCR[1]];
		if (((tcr >> 0) & 7) == 7)
		{
			h8s_tpu_x_tick( h8, 1);
		}
	}
	if (x == 5)
	{
		UINT8 tcr = h8->per_regs[H8S_IO_TTCR[4]];
		if (((tcr >> 0) & 7) == 7)
		{
			h8s_tpu_x_tick( h8, 4);
		}
	}
}

UINT16 h8s_tpu_x_tcnt_change( h83xx_state *h8, int x, UINT16 tcnt)
{
	UINT16 tgr;
	UINT8 tcr, tier, tior;
	int clear = 0;
	// ...
	verboselog( h8->device->machine(), 9, "TPU %d - tpu_x_tcnt_change (%04X)\n", x, tcnt);
	// ...
	tcr = h8->per_regs[H8S_IO_TTCR[x]];
	tier = h8->per_regs[H8S_IO_TIER[x]];
	// overflow
	if (tcnt == 0)
	{
		verboselog( h8->device->machine(), 9, "TPU %d - overflow match\n", x);
		h8->per_regs[H8S_IO_TSR[x]] |= TSR_TCFV;
		if (tier & TIER_TCIEV)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_TCIV[x]);
		}
		h8s_tpu_x_overflow( h8, x);
	}
	// tgra
	tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRA_H[x]);
	if (tcnt == tgr)
	{
		verboselog( h8->device->machine(), 9, "TPU %d - tgra match\n", x);
		h8->per_regs[H8S_IO_TSR[x]] |= TSR_TGFA;
		if (tier & TIER_TGIEA)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_TGIA[x]);
		}
		if (((tcr >> 5) & TCR_CCLR_AND[x]) == 1) // CCLR 1 -> "TCNT cleared by TGRA compare match/input capture"
		{
			clear = 1;
		}
		else
		{
			tior = h8->per_regs[H8S_IO_TIOR_H[x]];
			h8s_tioc_pin_update( h8, (tior >> 0) & 0x0F, PIN_TIOCA[x]);
		}
	}
	// tgrb
	tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRB_H[x]);
	if (tcnt == tgr)
	{
		verboselog( h8->device->machine(), 9, "TPU %d - tgrb match\n", x);
		h8->per_regs[H8S_IO_TSR[x]] |= TSR_TGFB;
		if (tier & TIER_TGIEB)
		{
			h8s2xxx_interrupt_request( h8, H8S_INT_TGIB[x]);
		}
		if (((tcr >> 5) & TCR_CCLR_AND[x]) == 2) // CCLR 2 -> "TCNT cleared by TGRB compare match/input capture"
		{
			clear = 1;
		}
		else
		{
			tior = h8->per_regs[H8S_IO_TIOR_H[x]];
			h8s_tioc_pin_update( h8, (tior >> 4) & 0x0F, PIN_TIOCB[x]);
		}
	}
	// tgrc
	if ((x == 0) || (x == 3))
	{
		tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRC_H[x]);
		if (tcnt == tgr)
		{
			verboselog( h8->device->machine(), 9, "TPU %d - tgrc match\n", x);
			h8->per_regs[H8S_IO_TSR[x]] |= TSR_TGFC;
			if (tier & TIER_TGIEC)
			{
				h8s2xxx_interrupt_request( h8, H8S_INT_TGIC[x]);
			}
			if (((tcr >> 5) & TCR_CCLR_AND[x]) == 5) // CCLR 5 -> "TCNT cleared by TGRC compare match/input capture"
			{
				clear = 1;
			}
			else
			{
				tior = h8->per_regs[H8S_IO_TIOR_L[x]];
				h8s_tioc_pin_update( h8, (tior >> 0) & 0x0F, PIN_TIOCC[x]);
			}
		}
	}
	// tgrd
	if ((x == 0) || (x == 3))
	{
		tgr = ptr_read_16( h8->per_regs + H8S_IO_TGRD_H[x]);
		if (tcnt == tgr)
		{
			verboselog( h8->device->machine(), 9, "TPU %d - tgrd match\n", x);
			h8->per_regs[H8S_IO_TSR[x]] |= TSR_TGFD;
			if (tier & TIER_TGIED)
			{
				h8s2xxx_interrupt_request( h8, H8S_INT_TGID[x]);
			}
			if (((tcr >> 5) & TCR_CCLR_AND[x]) == 6) // CCLR 6 -> "TCNT cleared by TGRD compare match/input capture"
			{
				clear = 1;
			}
			else
			{
				tior = h8->per_regs[H8S_IO_TIOR_L[x]];
				h8s_tioc_pin_update( h8, (tior >> 4) & 0x0F, PIN_TIOCD[x]);
			}
		}
	}
	// ...
	if (clear)
	{
		verboselog( h8->device->machine(), 9, "TPU %d - tcnt clear\n", x);
		h8s_tpu_x_tioc_reset( h8, x);
		tcnt = 0;
	}
	// ...
	return tcnt;
}

void h8s_tpu_x_callback( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	UINT64 cycles;
	UINT16 tcnt;
	// calculate tcnt
	tcnt = ptr_read_16( h8->per_regs + H8S_IO_TCNT_H[x]);
	tcnt = (UINT16)(tcnt + tpu->timer_cycles / tpu->cycles_per_tick);
	// ...
	tcnt = h8s_tpu_x_tcnt_change( h8, x, tcnt);
	// update tcnt
	ptr_write_16( h8->per_regs + H8S_IO_TCNT_H[x], tcnt);
	// update timer
	cycles = h8s_tpu_x_calculate_ticks( h8, x, tcnt) * tpu->cycles_per_tick;
	tpu->timer_cycles = cycles;
	tpu->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
}

void h8s_tpu_x_tick( h83xx_state *h8, int x)
{
	UINT16 tcnt;
	// calculate tcnt
	tcnt = ptr_read_16( h8->per_regs + H8S_IO_TCNT_H[x]);
	tcnt = (UINT16)(tcnt + 1);
	// ...
	tcnt = h8s_tpu_x_tcnt_change( h8, x, tcnt);
	// update tcnt
	ptr_write_16( h8->per_regs + H8S_IO_TCNT_H[x], tcnt);
}

int h8s_tpu_x_get_div( h83xx_state *h8, int x)
{
	UINT8 tcr;
	int tpsc;
	const int *TPU_X_DIV[] = { TPU_0_DIV, TPU_1_DIV, TPU_2_DIV, TPU_3_DIV, TPU_4_DIV, TPU_5_DIV };
	tcr = h8->per_regs[H8S_IO_TTCR[x]];
	tpsc = (tcr >> 0) & 7;
	return TPU_X_DIV[x][tpsc];
}

void h8s_tpu_x_update( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	// ...
	tpu->cycles_per_tick = h8s_tpu_x_get_div( h8, x);
	// update timer
	if (tpu->cycles_per_tick != 0)
	{
		UINT16 tcnt;
		UINT64 cycles;
		tcnt = ptr_read_16( h8->per_regs + H8S_IO_TCNT_H[x]);
		cycles = h8s_tpu_x_calculate_ticks( h8, x, tcnt) * tpu->cycles_per_tick;
		tpu->timer_cycles = cycles;
		tpu->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
	}
	else
	{
		tpu->timer->reset();
	}
}

void h8s_tpu_x_start( h83xx_state *h8, int x)
{
	verboselog( h8->device->machine(), 5, "TPU %d - start\n", x);
	// ...
	h8s_tpu_x_update( h8, x);
}

void h8s_tpu_x_stop( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	UINT16 tcnt;
	// ...
	verboselog( h8->device->machine(), 5, "TPU %d - stop\n", x);
	// calculate tcnt
	tcnt = h8s_tpu_x_calculate_tcnt( h8, x);
	// update tcnt
	ptr_write_16( h8->per_regs + H8S_IO_TCNT_H[x], tcnt);
	// disable timer
	tpu->timer->reset();
}

UINT16 h8s_tpu_x_read_tcnt( h83xx_state *h8, int x)
{
	return h8s_tpu_x_calculate_tcnt( h8, x);
}

void h8s_tpu_x_write_tcnt( h83xx_state *h8, int x, UINT16 data)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	verboselog( h8->device->machine(), 9, "TPU %d - write tcnt (%04X)\n", x, data);
	ptr_write_16( h8->per_regs + H8S_IO_TCNT_H[x], data);
	if (tpu->timer->enabled())
	{
		h8s_tpu_x_update( h8, x);
	}
}

void h8s_tpu_x_write_tgr( h83xx_state *h8, int x)
{
	H8S2XXX_TPU *tpu = &h8->tpu[x];
	if ((tpu->timer->enabled()) && (tpu->cycles_per_tick != 0))
	{
		UINT16 tcnt1, tcnt2;
		UINT64 cycles;
		tcnt1 = ptr_read_16( h8->per_regs + H8S_IO_TCNT_H[x]);
		tcnt2 = (UINT16)(tcnt1 + ((device_icount_min_timer_icount1( h8, tpu->timer)) / tpu->cycles_per_tick));
		cycles = h8s_tpu_x_calculate_ticks( h8, x, tcnt2) * tpu->cycles_per_tick;
		cycles = cycles + (tcnt2 - tcnt1) * tpu->cycles_per_tick;
		tpu->timer_cycles = cycles;
		tpu->timer->adjust(attotime::from_hz(h8->device->unscaled_clock() / cycles), x);
	}
}

void h8s_tpu_x_write_tgra( h83xx_state *h8, int x, UINT16 data)
{
	verboselog( h8->device->machine(), 9, "TPU %d - write tgra (%04X)\n", x, data);
	ptr_write_16( h8->per_regs + H8S_IO_TGRA_H[x], data);
	h8s_tpu_x_write_tgr( h8, x);
}

void h8s_tpu_x_write_tgrb( h83xx_state *h8, int x, UINT16 data)
{
	verboselog( h8->device->machine(), 9, "TPU %d - write tgrb (%04X)\n", x, data);
	ptr_write_16( h8->per_regs + H8S_IO_TGRB_H[x], data);
	h8s_tpu_x_write_tgr( h8, x);
}

void h8s_tpu_x_write_tgrc( h83xx_state *h8, int x, UINT16 data)
{
	verboselog( h8->device->machine(), 9, "TPU %d - write tgrc (%04X)\n", x, data);
	ptr_write_16( h8->per_regs + H8S_IO_TGRC_H[x], data);
	h8s_tpu_x_write_tgr( h8, x);
}

void h8s_tpu_x_write_tgrd( h83xx_state *h8, int x, UINT16 data)
{
	verboselog( h8->device->machine(), 9, "TPU %d - write tgrd (%04X)\n", x, data);
	ptr_write_16( h8->per_regs + H8S_IO_TGRD_H[x], data);
	h8s_tpu_x_write_tgr( h8, x);
}

void h8s_tpu_x_write_tior( h83xx_state *h8, int x)
{
	UINT8 tstr;
	tstr = h8->per_regs[H8S_IO_TSTR];
	if ((tstr & (1 << x)) == 0)
	{
		h8s_tpu_x_tioc_reset( h8, x);
	}
}

void h8s_tpu_0_write_tior_h( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR0_H] = data;
	h8s_tpu_x_write_tior( h8, 0);
}

void h8s_tpu_0_write_tior_l( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR0_L] = data;
	h8s_tpu_x_write_tior( h8, 0);
}

void h8s_tpu_1_write_tior( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR1] = data;
	h8s_tpu_x_write_tior( h8, 1);
}

void h8s_tpu_2_write_tior( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR2] = data;
	h8s_tpu_x_write_tior( h8, 2);
}

void h8s_tpu_3_write_tior_h( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR3_H] = data;
	h8s_tpu_x_write_tior( h8, 3);
}

void h8s_tpu_3_write_tior_l( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR3_L] = data;
	h8s_tpu_x_write_tior( h8, 3);
}

void h8s_tpu_4_write_tior( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR4] = data;
	h8s_tpu_x_write_tior( h8, 4);
}

void h8s_tpu_5_write_tior( h83xx_state *h8, UINT8 data)
{
	h8->per_regs[H8S_IO_TIOR5] = data;
	h8s_tpu_x_write_tior( h8, 5);
}

void h8s_tpu_write_tstr( h83xx_state *h8, UINT8 data)
{
	const int tpu_max = ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) ? 6 : 3;
	UINT8 old_data = h8->per_regs[H8S_IO_TSTR];
	int i;
	h8->per_regs[H8S_IO_TSTR] = data;
	for (i = 0; i < tpu_max; i++)
	{
		if ((data & (1 << i)) != (old_data & (1 << i)))
		{
			if ((data & (1 << i)) != 0)
			{
				h8s_tpu_x_start( h8, i);
			}
			else
			{
				h8s_tpu_x_stop( h8, i);
			}
		}
	}
}

static TIMER_CALLBACK( h8s_tpu_callback)
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	h8s_tpu_x_callback( h8, param);
}

/////////////////////////////////
// SERIAL CONTROLLER INTERFACE //
/////////////////////////////////

struct H8S_SCI_ENTRY
{
	UINT32 reg_smr, reg_brr, reg_scr, reg_tdr, reg_ssr, reg_rdr;
	UINT32 reg_pdr, reg_port;
	UINT8 port_mask_sck, port_mask_txd, port_mask_rxd;
	UINT8 int_tx, int_rx;
};

const H8S_SCI_ENTRY H8S_SCI_TABLE[] =
{
	// SCI 0
	{
		H8S_IO_SMR0, H8S_IO_BRR0, H8S_IO_SCR0, H8S_IO_TDR0, H8S_IO_SSR0, H8S_IO_RDR0,
		H8S_IO_P3DR, H8S_IO_PORT3,
		H8S_P3_SCK0, H8S_P3_TXD0, H8S_P3_RXD0,
		H8S_INT_TXI0, H8S_INT_RXI0
	},
	// SCI 1
	{
		H8S_IO_SMR1, H8S_IO_BRR1, H8S_IO_SCR1, H8S_IO_TDR1, H8S_IO_SSR1, H8S_IO_RDR1,
		H8S_IO_P3DR, H8S_IO_PORT3,
		H8S_P3_SCK1, H8S_P3_TXD1, H8S_P3_RXD1,
		H8S_INT_TXI1, H8S_INT_RXI1
	},
	// SCI 2
	{
		H8S_IO_SMR2, H8S_IO_BRR2, H8S_IO_SCR2, H8S_IO_TDR2, H8S_IO_SSR2, H8S_IO_RDR2,
		H8S_IO_P5DR, H8S_IO_PORT5,
		H8S_P5_SCK2, H8S_P5_TXD2, H8S_P5_RXD2,
		H8S_INT_TXI2, H8S_INT_RXI2
	}
};

const H8S_SCI_ENTRY *h8s_sci_entry( int num)
{
	return &H8S_SCI_TABLE[num];
}

void h8s_sci_init(h83xx_state *h8)
{
	int i;
	for (i=0;i<3;i++)
	{
		h8->sci[i].timer = h8->device->machine().scheduler().timer_alloc(FUNC(h8s_sci_callback), h8);
		h8->sci[i].timer->adjust(attotime::never, i);
	}
}

static TIMER_CALLBACK(h8s_sci_callback)
{
	h83xx_state *h8 = (h83xx_state *)ptr;
	verboselog( h8->device->machine(), 7, "h8s_sci_callback (%d)\n", param);
}

void h8s_sci_start(h83xx_state *h8, int num)
{
	#if 0
	h8->sci[num].timer->adjust(h8->sci[num].bitrate, num);
	#endif
}

void h8s_sci_stop(h83xx_state *h8, int num)
{
	h8->sci[num].timer->adjust(attotime::never, num);
}

void h8s_sci_execute(h83xx_state *h8, int num)
{
	UINT8 scr, tdr, ssr, rdr, tsr, rsr, pdr, port;
	int i;
	const H8S_SCI_ENTRY *info = h8s_sci_entry( num);
	verboselog( h8->device->machine(), 5, "h8s_sci_execute(%d)\n", num);
	// load regs
	scr = h8->per_regs[info->reg_scr];
	tdr = h8->per_regs[info->reg_tdr];
	ssr = h8->per_regs[info->reg_ssr];
	rdr = h8->per_regs[info->reg_rdr];
	tsr = 0;
	rsr = 0;
	pdr = h8->per_regs[info->reg_pdr] & (~info->port_mask_sck);
	// move byte from TDR to TSR
	if (scr & H8S_SCR_TE)
	{
		tsr = tdr;
		ssr |= H8S_SSR_TDRE;
	}
	// generate transmit data empty interrupt
	if ((scr & H8S_SCR_TIE) && (ssr & H8S_SSR_TDRE)) h8s2xxx_interrupt_request(h8, info->int_tx);
	// transmit/receive bits
	for (i=0;i<8;i++)
	{
		// write bit
		if (scr & H8S_SCR_TE)
		{
			if (tsr & (1 << i)) pdr = pdr | info->port_mask_txd; else pdr = pdr & (~info->port_mask_txd);
			h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr);
		}
		// clock high to low
		h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr | info->port_mask_sck);
		h8->io->write_byte( H8S_IO_ADDR( info->reg_pdr), pdr);
		// read bit
		if (scr & H8S_SCR_RE)
		{
			port = h8->io->read_byte(H8S_IO_ADDR( info->reg_port));
			if (port & info->port_mask_rxd) rsr = rsr | (1 << i);
		}
	}
	// move byte from RSR to RDR
	if (scr & H8S_SCR_RE)
	{
		rdr = rsr;
		//ssr |= H8S_SSR_RDRF;
	}
	// generate receive data full interrupt
	if ((scr & H8S_SCR_RIE) && (ssr & H8S_SSR_RDRF)) h8s2xxx_interrupt_request(h8, info->int_rx);
	// save regs
	h8->per_regs[info->reg_scr] = scr;
	h8->per_regs[info->reg_tdr] = tdr;
	h8->per_regs[info->reg_ssr] = ssr;
	h8->per_regs[info->reg_rdr] = rdr;
}

const double SCR_CKE[] = { 0.5, 2, 8, 32 }; // = 2 ^ ((2 * cke) - 1)
const int SMR_MODE[] = { 64, 8 };

void h8s_sci_rate(h83xx_state *h8, int num)
{
	UINT8 brr, scr, smr, cke, mode;
	UINT32 bitrate;
	const H8S_SCI_ENTRY *info = h8s_sci_entry( num);
	// read regs
	brr = h8->per_regs[info->reg_brr];
	scr = h8->per_regs[info->reg_scr];
	smr = h8->per_regs[info->reg_smr];
	verboselog( h8->device->machine(), 2, "BRR %02X SCR %02X SMR %02X\n", brr, scr, smr);
	// calculate bitrate
	cke = (scr >> 0) & 3;
	mode = (smr >> 7) & 1;
	bitrate = (UINT32)((h8->device->unscaled_clock() / (brr + 1)) / (SMR_MODE[mode] * SCR_CKE[cke]));
	verboselog( h8->device->machine(), 2, "SCI%d bitrate %d\n", num, bitrate);
	// store bitrate
	h8->sci[num].bitrate = bitrate;
}

////////////////////////////
// INTERNAL I/O REGISTERS //
////////////////////////////

void h8s_onchip_reg_write_8(h83xx_state *h8, int offset, UINT8 data)
{
	verboselog( h8->device->machine(), 9, "%08X | %08X <- %02X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	switch (offset)
	{
		// SCI 0
		case H8S_IO_SSR0  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 0); break;
		case H8S_IO_SCR0  : h8->per_regs[offset] = data; if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8, h8s_sci_entry(0)->int_tx); break;
		case H8S_IO_BRR0  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 0); h8s_sci_start(h8, 0); break;
		// SCI 1
		case H8S_IO_SSR1  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 1); break;
		case H8S_IO_SCR1  :	h8->per_regs[offset] = data; if (data & H8S_SCR_RIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(1)->int_rx); break;
		case H8S_IO_BRR1  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 1); h8s_sci_start(h8, 1); break;
		// SCI 2
		case H8S_IO_SSR2  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 2); break;
		case H8S_IO_SCR2  : h8->per_regs[offset] = data; if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(2)->int_tx); break;
		case H8S_IO_BRR2  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 2); h8s_sci_start(h8, 2); break;
		// TMR 0
		case H8S_IO_TCR0   : h8s_tmr_x_write_tcr( h8, 0, data); break;
		case H8S_IO_TCNT0  : h8s_tmr_x_write_tcnt( h8, 0, data); break;
		case H8S_IO_TCORA0 : h8s_tmr_x_write_tcora( h8, 0, data); break;
		case H8S_IO_TCORB0 : h8s_tmr_x_write_tcorb( h8, 0, data); break;
		// TMR 1
		case H8S_IO_TCR1   : h8s_tmr_x_write_tcr( h8, 1, data); break;
		case H8S_IO_TCNT1  : h8s_tmr_x_write_tcnt( h8, 1, data); break;
		case H8S_IO_TCORA1 : h8s_tmr_x_write_tcora( h8, 1, data); break;
		case H8S_IO_TCORB1 : h8s_tmr_x_write_tcorb( h8, 1, data); break;
		// ports
		case H8S_IO_P1DR : case H8S_IO_P2DR : case H8S_IO_P3DR : case H8S_IO_P4DR : case H8S_IO_P5DR :
		case H8S_IO_PADR : case H8S_IO_PBDR : case H8S_IO_PCDR : case H8S_IO_PDDR : case H8S_IO_PEDR :
		case H8S_IO_PFDR : case H8S_IO_PGDR : h8->per_regs[offset] = data; h8->io->write_byte( H8S_IO_ADDR( offset), data); break;
		// ...
		case H8S_IO_PFDDR : h8->per_regs[offset] = data; h8->io->write_byte( H8S_IO_ADDR( offset), data); break;
		// TPU
		case H8S_IO_TSTR  : h8s_tpu_write_tstr( h8, data); break;
		// DMA
		case H8S_IO_DMABCRL :
		{
			h8->per_regs[offset] = data;
			if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394))
			{
				if ((data & 0x40) && (data & 0x80))
				{
					UINT32 i, dma_src, dma_dst;
					UINT16 dma_cnt, dma_con;
					int sz;
					dma_src = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1AH));
					dma_dst = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1BH));
					dma_cnt = h8->program->read_word( H8S_IO_ADDR(H8S_IO_ETCR1A));
					dma_con = h8->program->read_word( H8S_IO_ADDR(H8S_IO_DMACR1A));
					sz = (dma_con & 0x8000) ? 2 : 1;
					for (i=0;i<dma_cnt;i++)
					{
						if (dma_con & 0x8000) h8->program->write_word( dma_dst, h8->program->read_word( dma_src)); else h8->program->write_byte( dma_dst, h8->program->read_byte( dma_src));
						if (dma_con & 0x2000) { if (dma_con & 0x4000) dma_src -= sz; else dma_src += sz; }
						if (dma_con & 0x0020) { if (dma_con & 0x0040) dma_dst -= sz; else dma_dst += sz; }
					}
					h8->per_regs[H8S_IO_DMABCRL] &= ~0x40;
				}
			}
		}
		break;
		// ...
		default :
		{
			h8->per_regs[offset] = data;
		}
		break;
	}
}

void h8s_onchip_reg_write_8_ddr(h83xx_state *h8, int offset, UINT8 data)
{
	verboselog( h8->device->machine(), 9, "%08X | %08X <- %02X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	switch (offset)
	{
        #if 0
		// SCI 0
		case H8S_IO_SSR0  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 0); break;
		case H8S_IO_SCR0  : h8->per_regs[offset] = data; if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8, h8s_sci_entry(0)->int_tx); break;
		case H8S_IO_BRR0  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 0); h8s_sci_start(h8, 0); break;
		// SCI 1
		case H8S_IO_SSR1  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 1); break;
		case H8S_IO_SCR1  :	h8->per_regs[offset] = data; if (data & H8S_SCR_RIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(1)->int_rx); break;
		case H8S_IO_BRR1  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 1); h8s_sci_start(h8, 1); break;
		// SCI 2
		case H8S_IO_SSR2  : h8->per_regs[offset] = data; if ((data & H8S_SSR_TDRE) == 0) h8s_sci_execute(h8, 2); break;
		case H8S_IO_SCR2  : h8->per_regs[offset] = data; if (data & H8S_SCR_TIE) h8s2xxx_interrupt_request(h8,  h8s_sci_entry(2)->int_tx); break;
		case H8S_IO_BRR2  : h8->per_regs[offset] = data; h8s_sci_rate(h8, 2); h8s_sci_start(h8, 2); break;
		// TMR 0
		case H8S_IO_TCR0   : h8s_tmr_x_write_tcr( h8, 0, data); break;
		case H8S_IO_TCNT0  : h8s_tmr_x_write_tcnt( h8, 0, data); break;
		case H8S_IO_TCORA0 : h8s_tmr_x_write_tcora( h8, 0, data); break;
		case H8S_IO_TCORB0 : h8s_tmr_x_write_tcorb( h8, 0, data); break;
		// TMR 1
		case H8S_IO_TCR1   : h8s_tmr_x_write_tcr( h8, 1, data); break;
		case H8S_IO_TCNT1  : h8s_tmr_x_write_tcnt( h8, 1, data); break;
		case H8S_IO_TCORA1 : h8s_tmr_x_write_tcora( h8, 1, data); break;
		case H8S_IO_TCORB1 : h8s_tmr_x_write_tcorb( h8, 1, data); break;
        #endif

        case H8S_IO_IFR:
			h8->per_regs[offset] = data;
            break;

        case H8S_IO_P1DDR : case H8S_IO_P2DDR : case H8S_IO_P3DDR : case H8S_IO_P5DDR : case H8S_IO_P6DDR :
        case H8S_IO_PADDR : case H8S_IO_PBDDR : case H8S_IO_PCDDR : case H8S_IO_PDDDR : case H8S_IO_PEDDR :
        case H8S_IO_PFDDR : case H8S_IO_PGDDR :
            h8->ddrs[offset - H8S_IO_P1DDR] = data;
            break;

		case H8S_IO_P1DR : case H8S_IO_P2DR : case H8S_IO_P3DR : case H8S_IO_P5DR : case H8S_IO_P6DR :
		case H8S_IO_PADR : case H8S_IO_PBDR : case H8S_IO_PCDR : case H8S_IO_PDDR : case H8S_IO_PEDR :
        case H8S_IO_PFDR : case H8S_IO_PGDR :
            {
                int port = (offset - H8S_IO_P1DR);
                h8->drs[port] = data;
                h8->io->write_byte(H8_PORT_1 + port, data & h8->ddrs[port]);
            }
            break;

		// ...
        #if 0
        // TPU
		case H8S_IO_TSTR  : h8s_tpu_write_tstr( h8, data); break;
		// DMA
		case H8S_IO_DMABCRL :
		{
			h8->per_regs[offset] = data;
			if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394))
			{
				if ((data & 0x40) && (data & 0x80))
				{
					UINT32 i, dma_src, dma_dst;
					UINT16 dma_cnt, dma_con;
					int sz;
					dma_src = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1AH));
					dma_dst = h8->program->read_dword( H8S_IO_ADDR(H8S_IO_MAR1BH));
					dma_cnt = h8->program->read_word( H8S_IO_ADDR(H8S_IO_ETCR1A));
					dma_con = h8->program->read_word( H8S_IO_ADDR(H8S_IO_DMACR1A));
					sz = (dma_con & 0x8000) ? 2 : 1;
					for (i=0;i<dma_cnt;i++)
					{
						if (dma_con & 0x8000) h8->program->write_word( dma_dst, h8->program->read_word( dma_src)); else h8->program->write_byte( dma_dst, h8->program->read_byte( dma_src));
						if (dma_con & 0x2000) { if (dma_con & 0x4000) dma_src -= sz; else dma_src += sz; }
						if (dma_con & 0x0020) { if (dma_con & 0x0040) dma_dst -= sz; else dma_dst += sz; }
					}
					h8->per_regs[H8S_IO_DMABCRL] &= ~0x40;
				}
			}
		}
		break;
        #endif
		// ...
		default :
		{
            logerror("H8S: Unknown write %02x to I/O %x\n", data, offset);
			h8->per_regs[offset] = data;
		}
		break;
	}
}

void h8s_onchip_reg_write_16(h83xx_state *h8, int offset, UINT16 data)
{
	verboselog( h8->device->machine(), 9, "%08X | %08X <- %04X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	switch (offset)
	{
		// TPU 0
		case H8S_IO_TCNT0_H : h8s_tpu_x_write_tcnt( h8, 0, data); break;
		case H8S_IO_TGR0A_H : h8s_tpu_x_write_tgra( h8, 0, data); break;
		case H8S_IO_TGR0B_H : h8s_tpu_x_write_tgrb( h8, 0, data); break;
		case H8S_IO_TGR0C_H : h8s_tpu_x_write_tgrc( h8, 0, data); break;
		case H8S_IO_TGR0D_H : h8s_tpu_x_write_tgrd( h8, 0, data); break;
		// TPU 1
		case H8S_IO_TCNT1_H : h8s_tpu_x_write_tcnt( h8, 1, data); break;
		case H8S_IO_TGR1A_H : h8s_tpu_x_write_tgra( h8, 1, data); break;
		case H8S_IO_TGR1B_H : h8s_tpu_x_write_tgrb( h8, 1, data); break;
		// TPU 2
		case H8S_IO_TCNT2_H : h8s_tpu_x_write_tcnt( h8, 2, data); break;
		case H8S_IO_TGR2A_H : h8s_tpu_x_write_tgra( h8, 2, data); break;
		case H8S_IO_TGR2B_H : h8s_tpu_x_write_tgrb( h8, 2, data); break;
		// TPU 3
		case H8S_IO_TCNT3_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tcnt( h8, 3, data); break;
		case H8S_IO_TGR3A_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgra( h8, 3, data); break;
		case H8S_IO_TGR3B_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgrb( h8, 3, data); break;
		case H8S_IO_TGR3C_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgrc( h8, 3, data); break;
		case H8S_IO_TGR3D_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgrd( h8, 3, data); break;
		// TPU 4
		case H8S_IO_TCNT4_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tcnt( h8, 4, data); break;
		case H8S_IO_TGR4A_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgra( h8, 4, data); break;
		case H8S_IO_TGR4B_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgrb( h8, 4, data); break;
		// TPU 5
		case H8S_IO_TCNT5_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tcnt( h8, 5, data); break;
		case H8S_IO_TGR5A_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgra( h8, 5, data); break;
		case H8S_IO_TGR5B_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) h8s_tpu_x_write_tgrb( h8, 5, data); break;
		// ...
		default :
		{
			h8s_onchip_reg_write_8( h8, offset + 0, (data >> 8) & 0xFF);
			h8s_onchip_reg_write_8( h8, offset + 1, (data >> 0) & 0xFF);
		}
		break;
	}
}

UINT8 h8s_onchip_reg_read_8(h83xx_state *h8, int offset)
{
	UINT8 data;
	switch (offset)
	{
		// SCI 0
		case H8S_IO_SSR0  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		case H8S_IO_RDR0  : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		// SCI 1
		case H8S_IO_SSR1  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		// SCI 2
		case H8S_IO_SSR2 :
		{
			data = h8->per_regs[offset];
			if (!(h8->per_regs[H8S_IO_SCR2] & H8S_SCR_TE)) data |= H8S_SSR_TDRE;
		}
		break;
		// ports
		case H8S_IO_PORT1 : case H8S_IO_PORT2 : case H8S_IO_PORT3 : case H8S_IO_PORT4 : case H8S_IO_PORT5 :
		case H8S_IO_PORTA : case H8S_IO_PORTB : case H8S_IO_PORTC : case H8S_IO_PORTD : case H8S_IO_PORTE :
		case H8S_IO_PORTF : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		// ...
        case H8S_IO_P1DR : case H8S_IO_P2DR : case H8S_IO_P3DR : case H8S_IO_P4DR : case H8S_IO_P5DR :
        case H8S_IO_PADR : case H8S_IO_PBDR : case H8S_IO_PCDR : case H8S_IO_PDDR : case H8S_IO_PEDR :
        case H8S_IO_PFDR : case H8S_IO_PGDR : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		// TMR 0
		case H8S_IO_TCNT0 : data = h8s_tmr_x_read_tcnt( h8, 0); break;
		// TMR 1
		case H8S_IO_TCNT1 : data = h8s_tmr_x_read_tcnt( h8, 1); break;
		// ...
		// default
		default : data = h8->per_regs[offset]; break;
	}
	verboselog( h8->device->machine(), 9, "%08X | %08X -> %02X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	return data;
}

UINT8 h8s_onchip_reg_read_8_ddr(h83xx_state *h8, int offset)
{
	UINT8 data;
	switch (offset)
	{
        #if 0
		// SCI 0
		case H8S_IO_SSR0  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		case H8S_IO_RDR0  : data = h8->io->read_byte( H8S_IO_ADDR( offset)); break;
		// SCI 1
		case H8S_IO_SSR1  : data = H8S_SSR_TDRE | H8S_SSR_TEND; break;
		// SCI 2
		case H8S_IO_SSR2 :
		{
			data = h8->per_regs[offset];
			if (!(h8->per_regs[H8S_IO_SCR2] & H8S_SCR_TE)) data |= H8S_SSR_TDRE;
		}
		break;
        #endif

        case H8S_IO_IFR:
            data = h8->per_regs[offset];
            break;

    // ports
        case H8S_IO_PORT1 : case H8S_IO_PORT2 : case H8S_IO_PORT3 : case H8S_IO_PORT4 : case H8S_IO_PORT5 : case H8S_IO_PORT6 :
        case H8S_IO_PORTA : case H8S_IO_PORTB : case H8S_IO_PORTC : case H8S_IO_PORTD : case H8S_IO_PORTE : case H8S_IO_PORTF :
        case H8S_IO_PORTG :
            {
                int port = (offset - H8S_IO_PORT1);
                data = h8->drs[port] & h8->ddrs[port];   // result = data register for DDR "1" bits, live read for DDR "0" bits
                data |= (h8->io->read_byte(H8_PORT_1 + port) & (h8->ddrs[port] ^ 0xff));
            }
            break;

		// the manual is ambivalent, but the invqix code very much implies that reading DR also reads the pin states
        case H8S_IO_P1DR : case H8S_IO_P2DR : case H8S_IO_P3DR : case H8S_IO_P5DR : case H8S_IO_P6DR :
        case H8S_IO_PADR : case H8S_IO_PBDR : case H8S_IO_PCDR : case H8S_IO_PDDR : case H8S_IO_PEDR :
        case H8S_IO_PFDR : case H8S_IO_PGDR :
            {
                int port = (offset - H8S_IO_P1DR);
                data = h8->drs[port] & h8->ddrs[port];
                data |= (h8->io->read_byte(H8_PORT_1 + port) & (h8->ddrs[port] ^ 0xff));
            }
            break;

        #if 0
        // TMR 0
		case H8S_IO_TCNT0 : data = h8s_tmr_x_read_tcnt( h8, 0); break;
		// TMR 1
		case H8S_IO_TCNT1 : data = h8s_tmr_x_read_tcnt( h8, 1); break;
        #endif
		// ...
		// default
		default : data = h8->per_regs[offset]; logerror("H8S: unhandled I/O read at %x\n", offset); break;
	}
	verboselog( h8->device->machine(), 9, "%08X | %08X -> %02X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	return data;
}

UINT16 h8s_onchip_reg_read_16(h83xx_state *h8, int offset)
{
	UINT16 data = 0;
	switch (offset)
	{
		case H8S_IO_TCNT0_H : data = h8s_tpu_x_read_tcnt( h8, 0); break;
		case H8S_IO_TCNT1_H : data = h8s_tpu_x_read_tcnt( h8, 1); break;
		case H8S_IO_TCNT2_H : data = h8s_tpu_x_read_tcnt( h8, 2); break;
		case H8S_IO_TCNT3_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) data = h8s_tpu_x_read_tcnt( h8, 3); break;
		case H8S_IO_TCNT4_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) data = h8s_tpu_x_read_tcnt( h8, 4); break;
		case H8S_IO_TCNT5_H : if ((h8->device->type() == H8S2323) || (h8->device->type() == H8S2394)) data = h8s_tpu_x_read_tcnt( h8, 5); break;
		default :
		{
			UINT8 b[2];
			b[0] = h8s_onchip_reg_read_8( h8, offset + 0);
			b[1] = h8s_onchip_reg_read_8( h8, offset + 1);
			data = (b[0] << 8) | (b[1] << 0);
		}
		break;
	}
	verboselog( h8->device->machine(), 9, "%08X | %08X -> %04X\n", h8->ppc, H8S_IO_ADDR(offset), data);
	return data;
}

void h8s2241_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data)
{
	h8s_onchip_reg_write_8(h8, offset, data);
}

UINT8 h8s2241_per_regs_read_8(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_8(h8, offset);
}

void h8s2241_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data)
{
	h8s_onchip_reg_write_16(h8, offset, data);
}

UINT16 h8s2241_per_regs_read_16(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_16(h8, offset);
}

void h8s2246_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data)
{
	h8s_onchip_reg_write_8(h8, offset, data);
}

UINT8 h8s2246_per_regs_read_8(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_8(h8, offset);
}

void h8s2246_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data)
{
	h8s_onchip_reg_write_16(h8, offset, data);
}

UINT16 h8s2246_per_regs_read_16(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_16(h8, offset);
}

void h8s2323_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data)
{
	h8s_onchip_reg_write_8(h8, offset, data);
}

UINT8 h8s2323_per_regs_read_8(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_8(h8, offset);
}

void h8s2323_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data)
{
	h8s_onchip_reg_write_16(h8, offset, data);
}

UINT16 h8s2323_per_regs_read_16(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_16(h8, offset);
}

void h8s2394_per_regs_write_8(h83xx_state *h8, int offset, UINT8 data)
{
	h8s_onchip_reg_write_8_ddr(h8, offset, data);
}

UINT8 h8s2394_per_regs_read_8(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_8_ddr(h8, offset);
}

void h8s2394_per_regs_write_16(h83xx_state *h8, int offset, UINT16 data)
{
	h8s_onchip_reg_write_16(h8, offset, data);
}

UINT16 h8s2394_per_regs_read_16(h83xx_state *h8, int offset)
{
	return h8s_onchip_reg_read_16(h8, offset);
}


