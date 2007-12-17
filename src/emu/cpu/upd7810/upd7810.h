#ifndef _UPD7810_H_
#define _UPD7810_H_

#include "cpuintrf.h"

/*
  all types have internal ram at 0xff00-0xffff
  7810
  7811 (4kbyte),7812(8),7814(16) have internal rom at 0x0000
*/

// unfortunatly memory configuration differs with internal rom size
typedef enum {
	TYPE_7810,
	TYPE_7810_GAMEMASTER, // a few modifications until internal rom dumped
	TYPE_7807
//  TYPE_78C10, // stop instruction added
//  TYPE_78IV,
//  TYPE_78K0,
//  TYPE_78K0S
//  millions of subtypes
} UPD7810_TYPE;

/* Supply an instance of this function in your driver code:
 * It will be called whenever an output signal changes or a new
 * input line state is to be sampled.
 */
typedef int (*upd7810_io_callback)(int ioline, int state);

// use it as reset parameter in the Machine struct
typedef struct {
    UPD7810_TYPE type;
    upd7810_io_callback io_callback;
} UPD7810_CONFIG;

enum {
	UPD7810_PC=1, UPD7810_SP, UPD7810_PSW,
	UPD7810_EA, UPD7810_V, UPD7810_A, UPD7810_VA,
	UPD7810_BC, UPD7810_B, UPD7810_C, UPD7810_DE, UPD7810_D, UPD7810_E, UPD7810_HL, UPD7810_H, UPD7810_L,
	UPD7810_EA2, UPD7810_V2, UPD7810_A2, UPD7810_VA2,
	UPD7810_BC2, UPD7810_B2, UPD7810_C2, UPD7810_DE2, UPD7810_D2, UPD7810_E2, UPD7810_HL2, UPD7810_H2, UPD7810_L2,
	UPD7810_CNT0, UPD7810_CNT1, UPD7810_TM0, UPD7810_TM1, UPD7810_ECNT, UPD7810_ECPT, UPD7810_ETM0, UPD7810_ETM1,
	UPD7810_MA, UPD7810_MB, UPD7810_MCC, UPD7810_MC, UPD7810_MM, UPD7810_MF,
	UPD7810_TMM, UPD7810_ETMM, UPD7810_EOM, UPD7810_SML, UPD7810_SMH,
	UPD7810_ANM, UPD7810_MKL, UPD7810_MKH, UPD7810_ZCM,
	UPD7810_TXB, UPD7810_RXB, UPD7810_CR0, UPD7810_CR1, UPD7810_CR2, UPD7810_CR3,
	UPD7810_TXD, UPD7810_RXD, UPD7810_SCK, UPD7810_TI, UPD7810_TO, UPD7810_CI, UPD7810_CO0, UPD7810_CO1
};

/* port numbers for PA,PB,PC,PD and PF */
enum {
	UPD7810_PORTA, UPD7810_PORTB, UPD7810_PORTC, UPD7810_PORTD, UPD7810_PORTF
};

enum {
	UPD7807_PORTA, UPD7807_PORTB, UPD7807_PORTC, UPD7807_PORTD, UPD7807_PORTF,
	UPD7807_PORTT
};

/* IRQ lines */
#define UPD7810_INTF1		0
#define UPD7810_INTF2		1
#define UPD7810_INTFE1      4

extern void upd7810_get_info(UINT32 state, cpuinfo *info);
extern void upd7807_get_info(UINT32 state, cpuinfo *info);

typedef struct {
	PAIR	ppc;	/* previous program counter */
	PAIR	pc; 	/* program counter */
	PAIR	sp; 	/* stack pointer */
	UINT8	op; 	/* opcode */
	UINT8	op2;	/* opcode part 2 */
	UINT8	iff;	/* interrupt enable flip flop */
	UINT8	psw;	/* processor status word */
	PAIR	ea; 	/* extended accumulator */
	PAIR	va; 	/* accumulator + vector register */
	PAIR	bc; 	/* 8bit B and C registers / 16bit BC register */
	PAIR	de; 	/* 8bit D and E registers / 16bit DE register */
	PAIR	hl; 	/* 8bit H and L registers / 16bit HL register */
	PAIR	ea2;	/* alternate register set */
	PAIR	va2;
	PAIR	bc2;
	PAIR	de2;
	PAIR	hl2;
	PAIR	cnt;	/* 8 bit timer counter */
	PAIR	tm; 	/* 8 bit timer 0/1 comparator inputs */
	PAIR	ecnt;	/* timer counter register / capture register */
	PAIR	etm;	/* timer 0/1 comparator inputs */
	UINT8	ma; 	/* port A input or output mask */
	UINT8	mb; 	/* port B input or output mask */
	UINT8	mcc;	/* port C control/port select */
	UINT8	mc; 	/* port C input or output mask */
	UINT8	mm; 	/* memory mapping */
	UINT8	mf; 	/* port F input or output mask */
	UINT8	tmm;	/* timer 0 and timer 1 operating parameters */
	UINT8	etmm;	/* 16-bit multifunction timer/event counter */
	UINT8	eom;	/* 16-bit timer/event counter output control */
	UINT8	sml;	/* serial interface parameters low */
	UINT8	smh;	/* -"- high */
	UINT8	anm;	/* analog to digital converter operating parameters */
	UINT8	mkl;	/* interrupt mask low */
	UINT8	mkh;	/* -"- high */
	UINT8	zcm;	/* bias circuitry for ac zero-cross detection */
	UINT8	pa_in;	/* port A,B,C,D,F inputs */
	UINT8	pb_in;
	UINT8	pc_in;
	UINT8	pd_in;
	UINT8	pf_in;
	UINT8	pa_out; /* port A,B,C,D,F outputs */
	UINT8	pb_out;
	UINT8	pc_out;
	UINT8	pd_out;
	UINT8	pf_out;
	UINT8	cr0;	/* analog digital conversion register 0 */
	UINT8	cr1;	/* analog digital conversion register 1 */
	UINT8	cr2;	/* analog digital conversion register 2 */
	UINT8	cr3;	/* analog digital conversion register 3 */
	UINT8	txb;	/* transmitter buffer */
	UINT8	rxb;	/* receiver buffer */
	UINT8	txd;	/* port C control line states */
	UINT8	rxd;
	UINT8	sck;
	UINT8	ti;
	UINT8	to;
	UINT8	ci;
	UINT8	co0;
	UINT8	co1;
	UINT16	irr;	/* interrupt request register */
	UINT16	itf;	/* interrupt test flag register */

/* internal helper variables */
	UINT16	txs;	/* transmitter shift register */
	UINT16	rxs;	/* receiver shift register */
	UINT8	txcnt;	/* transmitter shift register bit count */
	UINT8	rxcnt;	/* receiver shift register bit count */
	UINT8	txbuf;	/* transmitter buffer was written */
	INT32	ovc0;	/* overflow counter for timer 0 (for clock div 12/384) */
	INT32	ovc1;	/* overflow counter for timer 0 (for clock div 12/384) */
	INT32	ovce;	/* overflow counter for ecnt */
	INT32	ovcf;	/* overflow counter for fixed clock div 3 mode */
	INT32	ovcs;	/* overflow counter for serial I/O */
	UINT8	edges;	/* rising/falling edge flag for serial I/O */
	const struct opcode_s *opXX;	/* opcode table */
	const struct opcode_s *op48;
	const struct opcode_s *op4C;
	UPD7810_CONFIG config;
	int (*irq_callback)(int irqline);
}	UPD7810;

#ifdef MAME_DEBUG
offs_t upd7810_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
offs_t upd7807_dasm(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram);
#endif

#endif

