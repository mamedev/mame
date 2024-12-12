// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/*** m6800: Portable 6800 class emulator *************************************

m68xx.cpp

References:

    6809 Simulator V09, By L.C. Benschop, Eindhoven The Netherlands.

    m6809: Portable 6809 emulator, DS (6809 code in MAME, derived from
        the 6809 Simulator V09)

    6809 Microcomputer Programming & Interfacing with Experiments"
        by Andrew C. Staugaard, Jr.; Howard W. Sams & Co., Inc.

System dependencies:    u16 must be 16 bit unsigned int
                        u8 must be 8 bit unsigned int
                        u32 must be more than 16 bits
                        arrays up to 65536 bytes must be supported
                        machine must be twos complement

TODO:
- verify invalid opcodes for the different CPU types
- add 6802 nvram (only in case VCC STANDBY is connected to battery)
- cleanups (difficult to do maintenance work right now)
- (see m6801.cpp for 6801-specific TODO)

*****************************************************************************/

/*

    Chip                RAM     NVRAM   ROM     SCI     r15-f   ports
    -----------------------------------------------------------------
    MC6800              -       -       -       no      no      -
    MC6802              128     32      -       no      no      -
    MC6802NS            128     -       -       no      no      -
    MC6808              -       -       -       no      no      -

    MC6801              128     64      2K      yes     no      4
    MC68701             128     64      2K      yes     no      4
    MC6803              128     64      -       yes     no      4
    MC6803NR            -       -       -       yes     no      4

    MC6801U4            192     32      4K      yes     yes     4
    MC6803U4            192     32      -       yes     yes     4

    MC68120             128(DP) -       2K      yes     IPC     3
    MC68121             128(DP) -       -       yes     IPC     3

    HD6801              128     64      2K      yes     no      4
    HD6301V             128     -       4K      yes     no      4
    HD63701V            192     -       4K      yes     no      4
    HD6303R             128     -       -       yes     no      4

    HD6301X             192     -       4K      yes     yes     6
    HD6301Y             256     -       16K     yes     yes     6
    HD6303X             192     -       -       yes     yes     6
    HD6303Y             256     -       -       yes     yes     6

    NSC8105
    MS2010-A

*/

#include "emu.h"
#include "m6800.h"
#include "6800dasm.h"

#define LOG_IRQ (1U << 1)

//#define VERBOSE (LOG_IRQ)
#include "logmacro.h"

#define LOGIRQ(...) LOGMASKED(LOG_IRQ, __VA_ARGS__)


#define pPPC    m_ppc
#define pPC     m_pc
#define pS      m_s
#define pX      m_x
#define pD      m_d

#define PC      m_pc.w.l
#define PCD     m_pc.d
#define S       m_s.w.l
#define SD      m_s.d
#define X       m_x.w.l
#define D       m_d.w.l
#define A       m_d.b.h
#define B       m_d.b.l
#define CC      m_cc

#define EAD     m_ea.d
#define EA      m_ea.w.l

/* memory interface */

/****************************************************************************/
/* Read a byte from given memory location                                   */
/****************************************************************************/
#define RM(Addr) (m_program.read_byte(Addr))

/****************************************************************************/
/* Write a byte to given memory location                                    */
/****************************************************************************/
#define WM(Addr,Value) (m_program.write_byte(Addr,Value))

/****************************************************************************/
/* M6800_RDOP() is identical to M6800_RDMEM() except it is used for reading */
/* opcodes. In case of system with memory mapped I/O, this function can be  */
/* used to greatly speed up emulation                                       */
/****************************************************************************/
#define M_RDOP(Addr) (m_copcodes.read_byte(Addr))

/****************************************************************************/
/* M6800_RDOP_ARG() is identical to M6800_RDOP() but it's used for reading  */
/* opcode arguments. This difference can be used to support systems that    */
/* use different encoding mechanisms for opcodes and opcode arguments       */
/****************************************************************************/
#define M_RDOP_ARG(Addr) (m_cprogram.read_byte(Addr))

/* macros to access memory */
#define IMMBYTE(b)  b = M_RDOP_ARG(PCD); PC++
#define IMMWORD(w)  w.d = (M_RDOP_ARG(PCD)<<8) | M_RDOP_ARG((PCD+1)&0xffff); PC+=2

#define PUSHBYTE(b) WM(SD,b); --S
#define PUSHWORD(w) WM(SD,w.b.l); --S; WM(SD,w.b.h); --S
#define PULLBYTE(b) S++; b = RM(SD)
#define PULLWORD(w) S++; w.d = RM(SD)<<8; S++; w.d |= RM(SD)

/* CC masks                       HI NZVC
                                7654 3210   */
#define CLR_HNZVC   CC&=0xd0
#define CLR_NZV     CC&=0xf1
#define CLR_HNZC    CC&=0xd2
#define CLR_NZVC    CC&=0xf0
#define CLR_Z       CC&=0xfb
#define CLR_ZC      CC&=0xfa
#define CLR_C       CC&=0xfe

/* macros for CC -- CC bits affected should be reset before calling */
#define SET_Z(a)        if(!(a))SEZ
#define SET_Z8(a)       SET_Z((u8)(a))
#define SET_Z16(a)      SET_Z((u16)(a))
#define SET_N8(a)       CC|=(((a)&0x80)>>4)
#define SET_N16(a)      CC|=(((a)&0x8000)>>12)
#define SET_H(a,b,r)    CC|=((((a)^(b)^(r))&0x10)<<1)
#define SET_C8(a)       CC|=(((a)&0x100)>>8)
#define SET_C16(a)      CC|=(((a)&0x10000)>>16)
#define SET_V8(a,b,r)   CC|=((((a)^(b)^(r)^((r)>>1))&0x80)>>6)
#define SET_V16(a,b,r)  CC|=((((a)^(b)^(r)^((r)>>1))&0x8000)>>14)

const u8 m6800_cpu_device::flags8i[256]= /* increment */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x0a,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};


const u8 m6800_cpu_device::flags8d[256]= /* decrement */
{
0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08
};

#define SET_FLAGS8I(a)      {CC|=flags8i[(a)&0xff];}
#define SET_FLAGS8D(a)      {CC|=flags8d[(a)&0xff];}

/* combos */
#define SET_NZ8(a)          {SET_N8(a);SET_Z8(a);}
#define SET_NZ16(a)         {SET_N16(a);SET_Z16(a);}
#define SET_FLAGS8(a,b,r)   {SET_N8(r);SET_Z8(r);SET_V8(a,b,r);SET_C8(r);}
#define SET_FLAGS16(a,b,r)  {SET_N16(r);SET_Z16(r);SET_V16(a,b,r);SET_C16(r);}

/* for treating an u8 as a signed s16 */
#define SIGNED(b) ((s16)(b&0x80?b|0xff00:b))

/* Macros for addressing modes */
#define DIRECT IMMBYTE(EAD)
#define IMM8 EA=PC++
#define IMM16 {EA=PC;PC+=2;}
#define EXTENDED IMMWORD(m_ea)
#define INDEXED {EA=X+(u8)M_RDOP_ARG(PCD);PC++;}

/* macros to set status flags */
#if defined(SEC)
#undef SEC
#endif
#define SEC CC|=0x01
#define CLC CC&=0xfe
#define SEZ CC|=0x04
#define CLZ CC&=0xfb
#define SEN CC|=0x08
#define CLN CC&=0xf7
#define SEV CC|=0x02
#define CLV CC&=0xfd
#define SEH CC|=0x20
#define CLH CC&=0xdf
#define SEI CC|=0x10
#define CLI CC&=~0x10

/* macros for convenience */
#define DIRBYTE(b) {DIRECT;b=RM(EAD);}
#define DIRWORD(w) {DIRECT;w.d=RM16(EAD);}
#define EXTBYTE(b) {EXTENDED;b=RM(EAD);}
#define EXTWORD(w) {EXTENDED;w.d=RM16(EAD);}

#define IDXBYTE(b) {INDEXED;b=RM(EAD);}
#define IDXWORD(w) {INDEXED;w.d=RM16(EAD);}

/* Macros for branch instructions */
#define BRANCH(f) {IMMBYTE(t);if(f){PC+=SIGNED(t);}}
#define NXORV  ((CC&0x08)^((CC&0x02)<<2))
#define NXORC  ((CC&0x08)^((CC&0x01)<<3))

/* include the opcode functions */
#include "6800ops.hxx"

// to prevent the possibility of MAME locking up, don't use 0 cycles here
#define XX 4 // illegal opcode unknown cycle count

const u8 m6800_cpu_device::cycles_6800[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX, 2,XX,XX,XX,XX, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2,
	/*1*/  2, 2,XX,XX,XX,XX, 2, 2,XX, 2,XX, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX, 5,XX,10,XX,XX, 9,12,
	/*4*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*5*/  2,XX,XX, 2, 2,XX, 2, 2, 2, 2, 2,XX, 2, 2,XX, 2,
	/*6*/  7,XX,XX, 7, 7,XX, 7, 7, 7, 7, 7,XX, 7, 7, 4, 7,
	/*7*/  6,XX,XX, 6, 6,XX, 6, 6, 6, 6, 6,XX, 6, 6, 3, 6,
	/*8*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 8, 3, 4,
	/*9*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 6, 4, 5,
	/*A*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 8, 6, 7,
	/*B*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 9, 5, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX,XX, 3, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX,XX, 4, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5,XX,XX, 6, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4,XX,XX, 5, 6
};

const u8 m6800_cpu_device::cycles_nsc8105[256] =
{
		/* 0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F */
	/*0*/ XX,XX, 2,XX,XX, 2,XX, 2, 4, 2, 4, 2, 2, 2, 2, 2,
	/*1*/  2,XX, 2,XX,XX, 2,XX, 2,XX,XX, 2, 2,XX,XX,XX,XX,
	/*2*/  4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
	/*3*/  4, 4, 4, 4, 4, 4, 4, 4,XX,XX, 5,10,XX, 9,XX,12,
	/*4*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2, 3, 3, 8, 4,
	/*5*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3, 4, 4, 6, 5,
	/*6*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 6, 6, 8, 7,
	/*7*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 5, 5, 9, 6,
	/*8*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*9*/  2,XX,XX, 2, 2, 2,XX, 2, 2, 2, 2,XX, 2,XX, 2, 2,
	/*A*/  7,XX,XX, 7, 7, 7,XX, 7, 7, 7, 7,XX, 7, 4, 7, 7,
	/*B*/  6,XX, 6, 6, 6, 6,XX, 6, 6, 6, 6, 6, 6, 3, 6, 6,
	/*C*/  2, 2, 2,XX, 2, 2, 2, 3, 2, 2, 2, 2,XX, 3,XX, 4,
	/*D*/  3, 3, 3,XX, 3, 3, 3, 4, 3, 3, 3, 3,XX, 4,XX, 5,
	/*E*/  5, 5, 5,XX, 5, 5, 5, 6, 5, 5, 5, 5, 5, 6,XX, 7,
	/*F*/  4, 4, 4,XX, 4, 4, 4, 5, 4, 4, 4, 4, 4, 5,XX, 6
};

#undef XX // /illegal opcode unknown cc


const m6800_cpu_device::op_func m6800_cpu_device::m6800_insn[0x100] = {
// 0/8                     1/9                        2/A                        3/B                        4/C                        5/D                        6/E                        7/F
&m6800_cpu_device::illegl1,&m6800_cpu_device::nop,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::tap,    &m6800_cpu_device::tpa,    // 0
&m6800_cpu_device::inx,    &m6800_cpu_device::dex,    &m6800_cpu_device::clv,    &m6800_cpu_device::sev,    &m6800_cpu_device::clc,    &m6800_cpu_device::sec,    &m6800_cpu_device::cli,    &m6800_cpu_device::sei,
&m6800_cpu_device::sba,    &m6800_cpu_device::cba,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::tab,    &m6800_cpu_device::tba,    // 1
&m6800_cpu_device::illegl1,&m6800_cpu_device::daa,    &m6800_cpu_device::illegl1,&m6800_cpu_device::aba,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,
&m6800_cpu_device::bra,    &m6800_cpu_device::brn,    &m6800_cpu_device::bhi,    &m6800_cpu_device::bls,    &m6800_cpu_device::bcc,    &m6800_cpu_device::bcs,    &m6800_cpu_device::bne,    &m6800_cpu_device::beq,    // 2
&m6800_cpu_device::bvc,    &m6800_cpu_device::bvs,    &m6800_cpu_device::bpl,    &m6800_cpu_device::bmi,    &m6800_cpu_device::bge,    &m6800_cpu_device::blt,    &m6800_cpu_device::bgt,    &m6800_cpu_device::ble,
&m6800_cpu_device::tsx,    &m6800_cpu_device::ins,    &m6800_cpu_device::pula,   &m6800_cpu_device::pulb,   &m6800_cpu_device::des,    &m6800_cpu_device::txs,    &m6800_cpu_device::psha,   &m6800_cpu_device::pshb,   // 3
&m6800_cpu_device::illegl1,&m6800_cpu_device::rts,    &m6800_cpu_device::illegl1,&m6800_cpu_device::rti,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::wai,    &m6800_cpu_device::swi,
&m6800_cpu_device::nega,   &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::coma,   &m6800_cpu_device::lsra,   &m6800_cpu_device::illegl1,&m6800_cpu_device::rora,   &m6800_cpu_device::asra,   // 4
&m6800_cpu_device::asla,   &m6800_cpu_device::rola,   &m6800_cpu_device::deca,   &m6800_cpu_device::illegl1,&m6800_cpu_device::inca,   &m6800_cpu_device::tsta,   &m6800_cpu_device::illegl1,&m6800_cpu_device::clra,
&m6800_cpu_device::negb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::comb,   &m6800_cpu_device::lsrb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::rorb,   &m6800_cpu_device::asrb,   // 5
&m6800_cpu_device::aslb,   &m6800_cpu_device::rolb,   &m6800_cpu_device::decb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::incb,   &m6800_cpu_device::tstb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::clrb,
&m6800_cpu_device::neg_ix, &m6800_cpu_device::illegl2,&m6800_cpu_device::illegl2,&m6800_cpu_device::com_ix, &m6800_cpu_device::lsr_ix, &m6800_cpu_device::illegl2,&m6800_cpu_device::ror_ix, &m6800_cpu_device::asr_ix, // 6
&m6800_cpu_device::asl_ix, &m6800_cpu_device::rol_ix, &m6800_cpu_device::dec_ix, &m6800_cpu_device::illegl2,&m6800_cpu_device::inc_ix, &m6800_cpu_device::tst_ix, &m6800_cpu_device::jmp_ix, &m6800_cpu_device::clr_ix,
&m6800_cpu_device::neg_ex, &m6800_cpu_device::illegl3,&m6800_cpu_device::illegl3,&m6800_cpu_device::com_ex, &m6800_cpu_device::lsr_ex, &m6800_cpu_device::illegl3,&m6800_cpu_device::ror_ex, &m6800_cpu_device::asr_ex, // 7
&m6800_cpu_device::asl_ex, &m6800_cpu_device::rol_ex, &m6800_cpu_device::dec_ex, &m6800_cpu_device::illegl3,&m6800_cpu_device::inc_ex, &m6800_cpu_device::tst_ex, &m6800_cpu_device::jmp_ex, &m6800_cpu_device::clr_ex,
&m6800_cpu_device::suba_im,&m6800_cpu_device::cmpa_im,&m6800_cpu_device::sbca_im,&m6800_cpu_device::illegl2,&m6800_cpu_device::anda_im,&m6800_cpu_device::bita_im,&m6800_cpu_device::lda_im, &m6800_cpu_device::sta_im, // 8
&m6800_cpu_device::eora_im,&m6800_cpu_device::adca_im,&m6800_cpu_device::ora_im, &m6800_cpu_device::adda_im,&m6800_cpu_device::cmpx_im,&m6800_cpu_device::bsr,    &m6800_cpu_device::lds_im, &m6800_cpu_device::sts_im,
&m6800_cpu_device::suba_di,&m6800_cpu_device::cmpa_di,&m6800_cpu_device::sbca_di,&m6800_cpu_device::illegl2,&m6800_cpu_device::anda_di,&m6800_cpu_device::bita_di,&m6800_cpu_device::lda_di, &m6800_cpu_device::sta_di, // 9
&m6800_cpu_device::eora_di,&m6800_cpu_device::adca_di,&m6800_cpu_device::ora_di, &m6800_cpu_device::adda_di,&m6800_cpu_device::cmpx_di,&m6800_cpu_device::jsr_di, &m6800_cpu_device::lds_di, &m6800_cpu_device::sts_di,
&m6800_cpu_device::suba_ix,&m6800_cpu_device::cmpa_ix,&m6800_cpu_device::sbca_ix,&m6800_cpu_device::illegl2,&m6800_cpu_device::anda_ix,&m6800_cpu_device::bita_ix,&m6800_cpu_device::lda_ix, &m6800_cpu_device::sta_ix, // A
&m6800_cpu_device::eora_ix,&m6800_cpu_device::adca_ix,&m6800_cpu_device::ora_ix, &m6800_cpu_device::adda_ix,&m6800_cpu_device::cmpx_ix,&m6800_cpu_device::jsr_ix, &m6800_cpu_device::lds_ix, &m6800_cpu_device::sts_ix,
&m6800_cpu_device::suba_ex,&m6800_cpu_device::cmpa_ex,&m6800_cpu_device::sbca_ex,&m6800_cpu_device::illegl3,&m6800_cpu_device::anda_ex,&m6800_cpu_device::bita_ex,&m6800_cpu_device::lda_ex, &m6800_cpu_device::sta_ex, // B
&m6800_cpu_device::eora_ex,&m6800_cpu_device::adca_ex,&m6800_cpu_device::ora_ex, &m6800_cpu_device::adda_ex,&m6800_cpu_device::cmpx_ex,&m6800_cpu_device::jsr_ex, &m6800_cpu_device::lds_ex, &m6800_cpu_device::sts_ex,
&m6800_cpu_device::subb_im,&m6800_cpu_device::cmpb_im,&m6800_cpu_device::sbcb_im,&m6800_cpu_device::illegl2,&m6800_cpu_device::andb_im,&m6800_cpu_device::bitb_im,&m6800_cpu_device::ldb_im, &m6800_cpu_device::stb_im, // C
&m6800_cpu_device::eorb_im,&m6800_cpu_device::adcb_im,&m6800_cpu_device::orb_im, &m6800_cpu_device::addb_im,&m6800_cpu_device::illegl3,&m6800_cpu_device::illegl3,&m6800_cpu_device::ldx_im, &m6800_cpu_device::stx_im,
&m6800_cpu_device::subb_di,&m6800_cpu_device::cmpb_di,&m6800_cpu_device::sbcb_di,&m6800_cpu_device::illegl2,&m6800_cpu_device::andb_di,&m6800_cpu_device::bitb_di,&m6800_cpu_device::ldb_di, &m6800_cpu_device::stb_di, // D
&m6800_cpu_device::eorb_di,&m6800_cpu_device::adcb_di,&m6800_cpu_device::orb_di, &m6800_cpu_device::addb_di,&m6800_cpu_device::illegl2,&m6800_cpu_device::illegl2,&m6800_cpu_device::ldx_di, &m6800_cpu_device::stx_di,
&m6800_cpu_device::subb_ix,&m6800_cpu_device::cmpb_ix,&m6800_cpu_device::sbcb_ix,&m6800_cpu_device::illegl2,&m6800_cpu_device::andb_ix,&m6800_cpu_device::bitb_ix,&m6800_cpu_device::ldb_ix, &m6800_cpu_device::stb_ix, // E
&m6800_cpu_device::eorb_ix,&m6800_cpu_device::adcb_ix,&m6800_cpu_device::orb_ix, &m6800_cpu_device::addb_ix,&m6800_cpu_device::illegl2,&m6800_cpu_device::illegl2,&m6800_cpu_device::ldx_ix, &m6800_cpu_device::stx_ix,
&m6800_cpu_device::subb_ex,&m6800_cpu_device::cmpb_ex,&m6800_cpu_device::sbcb_ex,&m6800_cpu_device::illegl3,&m6800_cpu_device::andb_ex,&m6800_cpu_device::bitb_ex,&m6800_cpu_device::ldb_ex, &m6800_cpu_device::stb_ex, // F
&m6800_cpu_device::eorb_ex,&m6800_cpu_device::adcb_ex,&m6800_cpu_device::orb_ex, &m6800_cpu_device::addb_ex,&m6800_cpu_device::illegl3,&m6800_cpu_device::illegl3,&m6800_cpu_device::ldx_ex, &m6800_cpu_device::stx_ex
};

const m6800_cpu_device::op_func m6800_cpu_device::nsc8105_insn[0x100] = {
// 0/8                     1/9                        2/A                        3/B                        4/C                        5/D                        6/E                        7/F
&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::nop,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::tap,    &m6800_cpu_device::illegl1,&m6800_cpu_device::tpa,    // 0
&m6800_cpu_device::inx,    &m6800_cpu_device::clv,    &m6800_cpu_device::dex,    &m6800_cpu_device::sev,    &m6800_cpu_device::clc,    &m6800_cpu_device::cli,    &m6800_cpu_device::sec,    &m6800_cpu_device::sei,
&m6800_cpu_device::sba,    &m6800_cpu_device::illegl1,&m6800_cpu_device::cba,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::tab,    &m6800_cpu_device::illegl1,&m6800_cpu_device::tba,    // 1
&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::daa,    &m6800_cpu_device::aba,    &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,
&m6800_cpu_device::bra,    &m6800_cpu_device::bhi,    &m6800_cpu_device::brn,    &m6800_cpu_device::bls,    &m6800_cpu_device::bcc,    &m6800_cpu_device::bne,    &m6800_cpu_device::bcs,    &m6800_cpu_device::beq,    // 2
&m6800_cpu_device::bvc,    &m6800_cpu_device::bpl,    &m6800_cpu_device::bvs,    &m6800_cpu_device::bmi,    &m6800_cpu_device::bge,    &m6800_cpu_device::bgt,    &m6800_cpu_device::blt,    &m6800_cpu_device::ble,
&m6800_cpu_device::tsx,    &m6800_cpu_device::pula,   &m6800_cpu_device::ins,    &m6800_cpu_device::pulb,   &m6800_cpu_device::des,    &m6800_cpu_device::psha,   &m6800_cpu_device::txs,    &m6800_cpu_device::pshb,   // 3
&m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::rts,    &m6800_cpu_device::rti,    &m6800_cpu_device::illegl1,&m6800_cpu_device::wai,    &m6800_cpu_device::illegl1,&m6800_cpu_device::swi,
&m6800_cpu_device::suba_im,&m6800_cpu_device::sbca_im,&m6800_cpu_device::cmpa_im,&m6800_cpu_device::illegl1,&m6800_cpu_device::anda_im,&m6800_cpu_device::lda_im, &m6800_cpu_device::bita_im,&m6800_cpu_device::sta_im, // 4
&m6800_cpu_device::eora_im,&m6800_cpu_device::ora_im, &m6800_cpu_device::adca_im,&m6800_cpu_device::adda_im,&m6800_cpu_device::cmpx_im,&m6800_cpu_device::lds_im, &m6800_cpu_device::bsr,    &m6800_cpu_device::sts_im,
&m6800_cpu_device::suba_di,&m6800_cpu_device::sbca_di,&m6800_cpu_device::cmpa_di,&m6800_cpu_device::illegl1,&m6800_cpu_device::anda_di,&m6800_cpu_device::lda_di, &m6800_cpu_device::bita_di,&m6800_cpu_device::sta_di, // 5
&m6800_cpu_device::eora_di,&m6800_cpu_device::ora_di, &m6800_cpu_device::adca_di,&m6800_cpu_device::adda_di,&m6800_cpu_device::cmpx_di,&m6800_cpu_device::lds_di, &m6800_cpu_device::jsr_di, &m6800_cpu_device::sts_di,
&m6800_cpu_device::suba_ix,&m6800_cpu_device::sbca_ix,&m6800_cpu_device::cmpa_ix,&m6800_cpu_device::illegl1,&m6800_cpu_device::anda_ix,&m6800_cpu_device::lda_ix, &m6800_cpu_device::bita_ix,&m6800_cpu_device::sta_ix, // 6
&m6800_cpu_device::eora_ix,&m6800_cpu_device::ora_ix, &m6800_cpu_device::adca_ix,&m6800_cpu_device::adda_ix,&m6800_cpu_device::cmpx_ix,&m6800_cpu_device::lds_ix, &m6800_cpu_device::jsr_ix, &m6800_cpu_device::sts_ix,
&m6800_cpu_device::suba_ex,&m6800_cpu_device::sbca_ex,&m6800_cpu_device::cmpa_ex,&m6800_cpu_device::illegl1,&m6800_cpu_device::anda_ex,&m6800_cpu_device::lda_ex, &m6800_cpu_device::bita_ex,&m6800_cpu_device::sta_ex, // 7
&m6800_cpu_device::eora_ex,&m6800_cpu_device::ora_ex, &m6800_cpu_device::adca_ex,&m6800_cpu_device::adda_ex,&m6800_cpu_device::cmpx_ex,&m6800_cpu_device::lds_ex, &m6800_cpu_device::jsr_ex, &m6800_cpu_device::sts_ex,
&m6800_cpu_device::nega,   &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::coma,   &m6800_cpu_device::lsra,   &m6800_cpu_device::rora,   &m6800_cpu_device::illegl1,&m6800_cpu_device::asra,   // 8
&m6800_cpu_device::asla,   &m6800_cpu_device::deca,   &m6800_cpu_device::rola,   &m6800_cpu_device::illegl1,&m6800_cpu_device::inca,   &m6800_cpu_device::illegl1,&m6800_cpu_device::tsta,   &m6800_cpu_device::clra,
&m6800_cpu_device::negb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::comb,   &m6800_cpu_device::lsrb,   &m6800_cpu_device::rorb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::asrb,   // 9
&m6800_cpu_device::aslb,   &m6800_cpu_device::decb,   &m6800_cpu_device::rolb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::incb,   &m6800_cpu_device::illegl1,&m6800_cpu_device::tstb,   &m6800_cpu_device::clrb,
&m6800_cpu_device::neg_ix, &m6800_cpu_device::illegl1,&m6800_cpu_device::illegl1,&m6800_cpu_device::com_ix, &m6800_cpu_device::lsr_ix, &m6800_cpu_device::ror_ix, &m6800_cpu_device::illegl1,&m6800_cpu_device::asr_ix, // A
&m6800_cpu_device::asl_ix, &m6800_cpu_device::dec_ix, &m6800_cpu_device::rol_ix, &m6800_cpu_device::illegl1,&m6800_cpu_device::inc_ix, &m6800_cpu_device::jmp_ix, &m6800_cpu_device::tst_ix, &m6800_cpu_device::clr_ix,
&m6800_cpu_device::neg_ex, &m6800_cpu_device::illegl1,&m6800_cpu_device::stx_nsc,&m6800_cpu_device::com_ex, &m6800_cpu_device::lsr_ex, &m6800_cpu_device::ror_ex, &m6800_cpu_device::illegl1,&m6800_cpu_device::asr_ex, // B
&m6800_cpu_device::asl_ex, &m6800_cpu_device::dec_ex, &m6800_cpu_device::rol_ex, &m6800_cpu_device::btst_ix,&m6800_cpu_device::inc_ex, &m6800_cpu_device::jmp_ex, &m6800_cpu_device::tst_ex, &m6800_cpu_device::clr_ex,
&m6800_cpu_device::subb_im,&m6800_cpu_device::sbcb_im,&m6800_cpu_device::cmpb_im,&m6800_cpu_device::illegl1,&m6800_cpu_device::andb_im,&m6800_cpu_device::ldb_im, &m6800_cpu_device::bitb_im,&m6800_cpu_device::stb_im, // C
&m6800_cpu_device::eorb_im,&m6800_cpu_device::orb_im, &m6800_cpu_device::adcb_im,&m6800_cpu_device::addb_im,&m6800_cpu_device::illegl1,&m6800_cpu_device::ldx_im, &m6800_cpu_device::illegl1,&m6800_cpu_device::stx_im,
&m6800_cpu_device::subb_di,&m6800_cpu_device::sbcb_di,&m6800_cpu_device::cmpb_di,&m6800_cpu_device::illegl1,&m6800_cpu_device::andb_di,&m6800_cpu_device::ldb_di, &m6800_cpu_device::bitb_di,&m6800_cpu_device::stb_di, // D
&m6800_cpu_device::eorb_di,&m6800_cpu_device::orb_di, &m6800_cpu_device::adcb_di,&m6800_cpu_device::addb_di,&m6800_cpu_device::illegl1,&m6800_cpu_device::ldx_di, &m6800_cpu_device::illegl1,&m6800_cpu_device::stx_di,
&m6800_cpu_device::subb_ix,&m6800_cpu_device::sbcb_ix,&m6800_cpu_device::cmpb_ix,&m6800_cpu_device::illegl1,&m6800_cpu_device::andb_ix,&m6800_cpu_device::ldb_ix, &m6800_cpu_device::bitb_ix,&m6800_cpu_device::stb_ix, // E
&m6800_cpu_device::eorb_ix,&m6800_cpu_device::orb_ix, &m6800_cpu_device::adcb_ix,&m6800_cpu_device::addb_ix,&m6800_cpu_device::adcx_im,&m6800_cpu_device::ldx_ix, &m6800_cpu_device::illegl1,&m6800_cpu_device::stx_ix,
&m6800_cpu_device::subb_ex,&m6800_cpu_device::sbcb_ex,&m6800_cpu_device::cmpb_ex,&m6800_cpu_device::illegl1,&m6800_cpu_device::andb_ex,&m6800_cpu_device::ldb_ex, &m6800_cpu_device::bitb_ex,&m6800_cpu_device::stb_ex, // F
&m6800_cpu_device::eorb_ex,&m6800_cpu_device::orb_ex, &m6800_cpu_device::adcb_ex,&m6800_cpu_device::addb_ex,&m6800_cpu_device::addx_ex,&m6800_cpu_device::ldx_ex, &m6800_cpu_device::illegl1,&m6800_cpu_device::stx_ex
};


DEFINE_DEVICE_TYPE(M6800, m6800_cpu_device, "m6800", "Motorola MC6800")
DEFINE_DEVICE_TYPE(M6802, m6802_cpu_device, "m6802", "Motorola MC6802")
DEFINE_DEVICE_TYPE(M6808, m6808_cpu_device, "m6808", "Motorola MC6808")
DEFINE_DEVICE_TYPE(NSC8105, nsc8105_cpu_device, "nsc8105", "NSC8105")


m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m6800_cpu_device(mconfig, M6800, tag, owner, clock, m6800_insn, cycles_6800, address_map_constructor())
{
}

m6800_cpu_device::m6800_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const op_func *insn, const u8 *cycles, address_map_constructor internal)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, 8, 16, 0, internal)
	, m_decrypted_opcodes_config("program", ENDIANNESS_BIG, 8, 16, 0)
	, m_insn(insn)
	, m_cycles(cycles)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m6802_cpu_device(mconfig, M6802, tag, owner, clock, m6800_insn, cycles_6800)
{
}

m6802_cpu_device::m6802_cpu_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock, const op_func *insn, const u8 *cycles)
	: m6800_cpu_device(mconfig, type, tag, owner, clock, insn, cycles, address_map_constructor(FUNC(m6802_cpu_device::ram_map), this))
	, m_ram_enable(true)
{
}

void m6802_cpu_device::ram_map(address_map &map)
{
	if (m_ram_enable)
		map(0x0000, 0x007f).ram();
}

m6808_cpu_device::m6808_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m6802_cpu_device(mconfig, M6808, tag, owner, clock, m6800_insn, cycles_6800)
{
	set_ram_enable(false);
}

void m6808_cpu_device::device_validity_check(validity_checker &valid) const
{
	if (m_ram_enable)
		osd_printf_error("MC6808 should not have internal RAM enabled\n");
}

nsc8105_cpu_device::nsc8105_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: m6802_cpu_device(mconfig, NSC8105, tag, owner, clock, nsc8105_insn, cycles_nsc8105)
{
}

device_memory_interface::space_config_vector m6800_cpu_device::memory_space_config() const
{
	if (has_configured_map(AS_OPCODES))
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config),
			std::make_pair(AS_OPCODES, &m_decrypted_opcodes_config)
		};
	else
		return space_config_vector {
			std::make_pair(AS_PROGRAM, &m_program_config)
		};
}

u32 m6800_cpu_device::RM16(u32 Addr)
{
	u32 result = RM(Addr) << 8;
	return result | RM((Addr+1) & 0xffff);
}

void m6800_cpu_device::WM16(u32 Addr, PAIR *p)
{
	WM(Addr, p->b.h);
	WM((Addr+1) & 0xffff, p->b.l);
}

/* IRQ enter */
void m6800_cpu_device::enter_interrupt(const char *message, u16 irq_vector)
{
	int cycles_to_eat = 0;

	LOGIRQ("Take %s interrupt\n", message);

	if (m_wai_state & M6800_WAI)
	{
		cycles_to_eat = 4;
		m_wai_state &= ~M6800_WAI;
	}
	else
	{
		PUSHWORD(pPC);
		PUSHWORD(pX);
		PUSHBYTE(A);
		PUSHBYTE(B);
		PUSHBYTE(CC);
		cycles_to_eat = 12;
	}
	SEI;
	PCD = RM16(irq_vector);

	increment_counter(cycles_to_eat);
}

/* check the IRQ lines for pending interrupts */
void m6800_cpu_device::check_irq_lines()
{
	if (m_nmi_pending)
	{
		m_wai_state &= ~M6800_SLP;
		m_nmi_pending = false;
		enter_interrupt("NMI", 0xfffc);
	}
	else if (check_irq1_enabled())
	{
		/* standard IRQ */
		m_wai_state &= ~M6800_SLP;

		if (!(CC & 0x10))
		{
			standard_irq_callback(M6800_IRQ_LINE, m_pc.w.l);
			enter_interrupt("IRQ1", 0xfff8);
		}
	}
	else
		check_irq2();
}

bool m6800_cpu_device::check_irq1_enabled()
{
	return m_irq_state[M6800_IRQ_LINE] != CLEAR_LINE;
}

void m6800_cpu_device::increment_counter(int amount)
{
	m_icount -= amount;
}

void m6800_cpu_device::eat_cycles()
{
	if (m_icount > 0)
		increment_counter(m_icount);
}


void m6800_cpu_device::device_start()
{
	space(AS_PROGRAM).cache(m_cprogram);
	space(has_space(AS_OPCODES) ? AS_OPCODES : AS_PROGRAM).cache(m_copcodes);
	space(AS_PROGRAM).specific(m_program);

	m_ppc.d = 0;
	m_pc.d = 0;
	m_s.d = 0;
	m_x.d = 0;
	m_d.d = 0;
	m_cc = 0;
	m_wai_state = 0;
	m_nmi_state = 0;
	m_nmi_pending = 0;
	std::fill(std::begin(m_irq_state), std::end(m_irq_state), 0);

	save_item(NAME(m_ppc.w.l));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_s.w.l));
	save_item(NAME(m_x.w.l));
	save_item(NAME(m_d.w.l));
	save_item(NAME(m_cc));
	save_item(NAME(m_wai_state));
	save_item(NAME(m_nmi_state));
	save_item(NAME(m_nmi_pending));
	save_item(NAME(m_irq_state));

	state_add( M6800_A,         "A", m_d.b.h).formatstr("%02X");
	state_add( M6800_B,         "B", m_d.b.l).formatstr("%02X");
	state_add( M6800_PC,        "PC", m_pc.w.l).formatstr("%04X");
	state_add( M6800_S,         "S", m_s.w.l).formatstr("%04X");
	state_add( M6800_X,         "X", m_x.w.l).formatstr("%04X");
	state_add( M6800_CC,        "CC", m_cc).formatstr("%02X");
	state_add( M6800_WAI_STATE, "WAI", m_wai_state).formatstr("%01X");

	state_add( STATE_GENPC, "GENPC", m_pc.w.l).noshow();
	state_add( STATE_GENPCBASE, "CURPC", m_pc.w.l).noshow();
	state_add( STATE_GENFLAGS, "GENFLAGS", m_cc).formatstr("%8s").noshow();

	set_icountptr(m_icount);
}

void m6800_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c",
				m_cc & 0x80 ? '?':'.',
				m_cc & 0x40 ? '?':'.',
				m_cc & 0x20 ? 'H':'.',
				m_cc & 0x10 ? 'I':'.',
				m_cc & 0x08 ? 'N':'.',
				m_cc & 0x04 ? 'Z':'.',
				m_cc & 0x02 ? 'V':'.',
				m_cc & 0x01 ? 'C':'.');
			break;
	}
}

void m6800_cpu_device::device_reset()
{
	m_cc = 0xc0;
	SEI; /* IRQ disabled */
	PCD = RM16(0xfffe);

	m_wai_state = 0;
	m_nmi_state = 0;
	m_nmi_pending = 0;
}


void m6800_cpu_device::execute_set_input(int irqline, int state)
{
	switch (irqline)
	{
	case INPUT_LINE_NMI:
		if (!m_nmi_state && state != CLEAR_LINE)
			m_nmi_pending = true;
		m_nmi_state = state;
		break;

	default:
		m_irq_state[irqline] = state;
		break;
	}
}


void m6800_cpu_device::execute_run()
{
	check_irq_lines();

	cleanup_counters();

	do
	{
		if (m_wai_state & (M6800_WAI | M6800_SLP))
		{
			debugger_wait_hook();
			eat_cycles();
		}
		else
		{
			execute_one();
		}
	} while (m_icount > 0);
}

void m6800_cpu_device::execute_one()
{
	pPPC = pPC;
	debugger_instruction_hook(PCD);
	u8 ireg = M_RDOP(PCD);
	PC++;
	(this->*m_insn[ireg])();
	increment_counter(m_cycles[ireg]);
}

std::unique_ptr<util::disasm_interface> m6800_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6800);
}

std::unique_ptr<util::disasm_interface> m6802_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6802);
}

std::unique_ptr<util::disasm_interface> m6808_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(6808);
}

std::unique_ptr<util::disasm_interface> nsc8105_cpu_device::create_disassembler()
{
	return std::make_unique<m680x_disassembler>(8105);
}
