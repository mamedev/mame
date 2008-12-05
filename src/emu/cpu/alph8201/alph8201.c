/****************************************************************************
                         Alpha8201 Emulator

                      Copyright Tatsuyuki Satoh
                   Originally written for the MAME project.


The Alpha8201/830x isn't a real CPU. It is a Hitachi HD44801 4-bit MCU,
programmed to interpret an external program using a custom instruction set.
Alpha8301 has an expanded instruction set, backwards compatible with Alpha8201

The internal ROM hasn't been read (yet), so here we provide a simulation of
the behaviour.


Game                      Year  MCU
------------------------  ----  ----
Shougi                    1982? 8201 (pcb)
Shougi 2                  1982? 8201 (pcb)
Talbot                    1982  8201?
Champion Base Ball        1983  8201 (schematics)
Exciting Soccer           1983  8301?
Champion Base Ball II     1983  8302 (pcb, unofficial schematics)
Exciting Soccer II        1984  8303 (uses 8303+ opcodes)
Equites                   1984  8303 (post)
Bull Fighter              1984  8303 (post)
Splendor Blast            1985  8303 (post)
Gekisou                   1985  8304 (post)
The Koukouyakyuh          1985  8304 (post)
High Voltage              1985  8404?(post says 8404, but readme says 8304)

ALPHA8201: "44801A75" -> HD44801 , ROM code = A75
ALPHA8302: "44801B35" -> HD44801 , ROM code = B35
ALPHA8303: "44801B42" -> HD44801 , ROM code = B42
ALPHA8304: ?


    Notes :
      some unknown instruction are not emulated.

      Because there was no information, opcode-syntax was created.

    TODO:
        verify with real chip or analyze more.
            -A lot of 8301 opcode.
            -memory address 000 specification
            -memory address 001 bit 7-5 specification
            -write value after HALT operation to ODD of vector memory.
            -operation cycle(execution speed).

****************************************************************************/

/****************************************************************************

-----------------------
package / pin assign
-----------------------
ALPHA 8201 DIP 42

pin    HD44801  Alpha
---    -------  -----
1    : D3       WR
2-4  : D4-D6    n.c.
5-7  : D7-D9    GND in shougi , n.c. in champbas
8-13 : D10-D15  n.c.
14   : n.c.     n.c.
15   : RESET    RESET
16   : GND      GND
17   : OSC1     (champbas=384KHz)
18   : OSC2     n.c.
19   : !HLT     Vcc
20   : !TEST    Vcc
21   : Vcc      Vcc
22-25: R00-R03  DB4-DB7
26-29: R10-R13  DB0-DB3
30   : INT0     GO (input)
31   : INT1     n.c.
32-35: R20-R23  A4-A7
36-39: R30-R33  A0-A3
40-41: D0-D1    A8-A9
42   : D2       /RD


-----------------------
Register Set
-----------------------

PC      : 10bit Program Pointer
          A lower 8bits are loaded from the immidate.
          A higher 2bits are loaded from the MB register.

MB      : 2bit memory bank register, load PC[9:8] after branch
          load high to higher 2bit of PC after branch.

RB      : 3bit register bank select register

R0-R7   : internal? RAM register 8bitx8 (x8 bank)

A       : 8bit

IX0/1   : memory indirect 'read' access pointer

IX2     : memory indirect 'write' access pointer

RXB     : unknown , looks index register

LP0/1/2 : loop count register used by DJNZ operation

CF      : carry flag
ZF      : zero flag

-----------------------
Memoy Space
-----------------------

000     : unknown ...
001     : bit4..0 = pointer of current entry , bit7..6 = unknown
002-003 : entrypoint1  vector
004-005 : entrypoint2  vector
006-007 : entrypoint3  vector
008-009 : entrypoint4  vector
00A-00B : entrypoint5  vector
00C-00D : entrypoint6  vector
00E-00F : entrypoint7  vector
010-011 : entrypoint8  vector
012-013 : entrypoint9  vector
014-015 : entrypoint10 vector
016-017 : entrypoint11 vector
018-019 : entrypoint12 vector
01A-01B : entrypoint13 vector
01C-01D : entrypoint14 vector
01E-01F : entrypoint15 vector
020-0FF : bank 0, program / data memory
100-1FF : bank 1, program / data memory
200-2FF : bank 2, program / data memory
300-3FF : bank 3, program / data memory

The even address is the lower byte of the entry address.
The odd-address of entry point is a MB and status.
  Bit 0 and 1 are a memory bank.
  Bit 2 is HALT.At the time of set, it doesn't execute entry address.
  After EXIT operation, Bit2 is set.

-----------------------
Timming
-----------------------

****************************************************************************/

#include "debugger.h"
#include "deprecat.h"
#include "alph8201.h"

/* instruction cycle count */
#define C1 16
#define C2 32

/* debug option */
#define TRACE_PC 0
#define SHOW_ENTRY_POINT 0
#define SHOW_MESSAGE_CONSOLE 0
#define BREAK_ON_UNKNOWN_OPCODE 0
#define BREAK_ON_UNCERTAIN_OPCODE 0

/* MAME is unnecessary */
#define HANDLE_HALT_LINE 0

#define M_RDMEM(A)		ALPHA8201_RDMEM(A)
#define M_WRMEM(A,V)	ALPHA8201_WRMEM(A,V)
#define M_RDOP(A)		ALPHA8201_RDOP(A)
#define M_RDOP_ARG(A)	ALPHA8201_RDOP_ARG(A)

typedef struct
{
	UINT8 RAM[8*8];  /* internal GP register 8 * 8bank       */
	unsigned PREVPC;
	PAIR  retptr;   /* for 8301, return address of CALL       */
	PAIR  pc;       /* 2bit+8bit program counter              */
	UINT8 regPtr;   /* RB register base                       */
	UINT8 mb;       /* MB memory bank reg. latch after Branch */
	UINT8 cf;       /* C flag                                 */
	UINT8 zf;       /* Z flag                                 */
	UINT8 savec;    /* for 8301, save flags                   */
	UINT8 savez;    /* for 8301, save flags                   */
//
	PAIR ix0;		/* 8bit memory read index reg. */
	PAIR ix1;		/* 8bitmemory read index reg.  */
	PAIR ix2;		/* 8bitmemory write index reg. */
	UINT8 lp0;       /* 8bit loop reg.             */
	UINT8 lp1;       /* 8bit loop reg.             */
	UINT8 lp2;       /* 8bit loop reg.             */
	UINT8 A;         /* 8bit accumerator           */
	UINT8 B;         /* 8bit regiser               */
//
#if HANDLE_HALT_LINE
	UINT8 halt;     /* halt input line                        */
#endif

	const device_config *device;
	const address_space *program;
} ALPHA8201_Regs;

/* The opcode table now is a combination of cycle counts and function pointers */
typedef struct {
	unsigned cycles;
	void (*function) (void);
}	s_opcode;

static ALPHA8201_Regs R;
static int	   ALPHA8201_ICount;
static int    inst_cycles;

#define regRAM	R.RAM
#define regPTR	R.regPtr

#define PC	R.pc.w.l
#define PCL	 R.pc.b.l
#define RD_REG(x)	regRAM[(regPTR<<3)+(x)]
#define WR_REG(x,d)	regRAM[(regPTR<<3)+(x)]=(d)
#define ZF	R.zf
#define CF	R.cf
#define IX0	R.ix0.b.l
#define IX1	R.ix1.b.l
#define IX2	R.ix2.b.l
#define BIX0	R.ix0.w.l
#define BIX1	R.ix1.w.l
#define BIX2	R.ix2.w.l
#define LP0	R.lp0
#define LP1	R.lp1
#define LP2	R.lp2

/* Get next opcode argument and increment program counter */
INLINE unsigned M_RDMEM_OPCODE (void)
{
	unsigned retval;
	retval=M_RDOP_ARG(PC);
	PCL++;
	return retval;
}

INLINE void M_ADD(UINT8 dat)
{
	UINT16 temp = R.A + dat;
	R.A = temp & 0xff;
	ZF = (R.A==0);
	CF = temp>>8;
}

INLINE void M_ADDB(UINT8 dat)
{
	UINT16 temp = R.B + dat;
	R.B = temp & 0xff;
	ZF = (R.B==0);
	CF = temp>>8;
}

INLINE void M_SUB(UINT8 dat)
{
	CF = (R.A>=dat);	// CF is No Borrow
	R.A -= dat;
	ZF = (R.A==0);
}

INLINE void M_AND(UINT8 dat)
{
	R.A &= dat;
	ZF = (R.A==0);
}

INLINE void M_OR(UINT8 dat)
{
	R.A |= dat;
	ZF = (R.A==0);
}

INLINE void M_XOR(UINT8 dat)
{
	R.A ^= dat;
	ZF = (R.A==0);
	CF = 0;
}

INLINE void M_JMP(UINT8 dat)
{
	PCL = dat;
	/* update pc page */
	R.pc.b.h  = R.ix0.b.h = R.ix1.b.h = R.ix2.b.h = R.mb & 3;
}

INLINE void M_UNDEFINED(void)
{
	logerror("ALPHA8201:  PC = %03x,  Unimplemented opcode = %02x\n", PC-1, M_RDMEM(PC-1));
#if SHOW_MESSAGE_CONSOLE
	mame_printf_debug("ALPHA8201:  PC = %03x,  Unimplemented opcode = %02x\n", PC-1, M_RDMEM(PC-1));
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(Machine);
#endif
}

INLINE void M_UNDEFINED2(void)
{
	UINT8 op  = M_RDOP(PC-1);
	UINT8 imm = M_RDMEM_OPCODE();
	logerror("ALPHA8201:  PC = %03x,  Unimplemented opcode = %02x,%02x\n", PC-2, op,imm);
#if SHOW_MESSAGE_CONSOLE
	mame_printf_debug("ALPHA8201:  PC = %03x,  Unimplemented opcode = %02x,%02x\n", PC-2, op,imm);
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(Machine);
#endif
}

static void undefined(void)	{ M_UNDEFINED(); }
static void undefined2(void)	{ M_UNDEFINED2(); }

static void nop(void)		 { }
static void rora(void)		 { CF = R.A &1;     R.A = (R.A>>1) | (R.A<<7); }
static void rola(void)		 { CF = (R.A>>7)&1; R.A = (R.A<<1) | (R.A>>7); }
static void inc_b(void)	 	 { M_ADDB(0x02); }
static void dec_b(void)	 	 { M_ADDB(0xfe); }
static void inc_a(void)		 { M_ADD(0x01); }
static void dec_a(void)		 { M_ADD(0xff); }
static void cpl(void)		 { R.A ^= 0xff; };

static void ld_a_ix0_0(void) { R.A = M_RDMEM(BIX0+0); }
static void ld_a_ix0_1(void) { R.A = M_RDMEM(BIX0+1); }
static void ld_a_ix0_2(void) { R.A = M_RDMEM(BIX0+2); }
static void ld_a_ix0_3(void) { R.A = M_RDMEM(BIX0+3); }
static void ld_a_ix0_4(void) { R.A = M_RDMEM(BIX0+4); }
static void ld_a_ix0_5(void) { R.A = M_RDMEM(BIX0+5); }
static void ld_a_ix0_6(void) { R.A = M_RDMEM(BIX0+6); }
static void ld_a_ix0_7(void) { R.A = M_RDMEM(BIX0+7); }

static void ld_a_ix1_0(void) { R.A = M_RDMEM(BIX1+0); }
static void ld_a_ix1_1(void) { R.A = M_RDMEM(BIX1+1); }
static void ld_a_ix1_2(void) { R.A = M_RDMEM(BIX1+2); }
static void ld_a_ix1_3(void) { R.A = M_RDMEM(BIX1+3); }
static void ld_a_ix1_4(void) { R.A = M_RDMEM(BIX1+4); }
static void ld_a_ix1_5(void) { R.A = M_RDMEM(BIX1+5); }
static void ld_a_ix1_6(void) { R.A = M_RDMEM(BIX1+6); }
static void ld_a_ix1_7(void) { R.A = M_RDMEM(BIX1+7); }

static void ld_ix2_0_a(void) { M_WRMEM(BIX2+0,R.A); }
static void ld_ix2_1_a(void) { M_WRMEM(BIX2+1,R.A); }
static void ld_ix2_2_a(void) { M_WRMEM(BIX2+2,R.A); }
static void ld_ix2_3_a(void) { M_WRMEM(BIX2+3,R.A); }
static void ld_ix2_4_a(void) { M_WRMEM(BIX2+4,R.A); }
static void ld_ix2_5_a(void) { M_WRMEM(BIX2+5,R.A); }
static void ld_ix2_6_a(void) { M_WRMEM(BIX2+6,R.A); }
static void ld_ix2_7_a(void) { M_WRMEM(BIX2+7,R.A); }

static void ld_ix0_0_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+0); }
static void ld_ix0_1_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+1); }
static void ld_ix0_2_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+2); }
static void ld_ix0_3_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+3); }
static void ld_ix0_4_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+4); }
static void ld_ix0_5_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+5); }
static void ld_ix0_6_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+6); }
static void ld_ix0_7_b(void) { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0+7); }

static void bit_r0_0(void)	 { ZF = RD_REG(0)&(1<<0)?0:1; }
static void bit_r0_1(void)	 { ZF = RD_REG(0)&(1<<1)?0:1; }
static void bit_r0_2(void)	 { ZF = RD_REG(0)&(1<<2)?0:1; }
static void bit_r0_3(void)	 { ZF = RD_REG(0)&(1<<3)?0:1; }
static void bit_r0_4(void)	 { ZF = RD_REG(0)&(1<<4)?0:1; }
static void bit_r0_5(void)	 { ZF = RD_REG(0)&(1<<5)?0:1; }
static void bit_r0_6(void)	 { ZF = RD_REG(0)&(1<<6)?0:1; }
static void bit_r0_7(void)	 { ZF = RD_REG(0)&(1<<7)?0:1; }

static void ld_a_n(void)	 { R.A = M_RDMEM_OPCODE(); }

static void ld_a_r0(void)	 { R.A = RD_REG(0); ZF = (R.A==0); }
static void ld_a_r1(void)	 { R.A = RD_REG(1); ZF = (R.A==0); }
static void ld_a_r2(void)	 { R.A = RD_REG(2); ZF = (R.A==0); }
static void ld_a_r3(void)	 { R.A = RD_REG(3); ZF = (R.A==0); }
static void ld_a_r4(void)	 { R.A = RD_REG(4); ZF = (R.A==0); }
static void ld_a_r5(void)	 { R.A = RD_REG(5); ZF = (R.A==0); }
static void ld_a_r6(void)	 { R.A = RD_REG(6); ZF = (R.A==0); }
static void ld_a_r7(void)	 { R.A = RD_REG(7); ZF = (R.A==0); }

static void ld_r0_a(void)	 { WR_REG(0,R.A); }
static void ld_r1_a(void)	 { WR_REG(1,R.A); }
static void ld_r2_a(void)	 { WR_REG(2,R.A); }
static void ld_r3_a(void)	 { WR_REG(3,R.A); }
static void ld_r4_a(void)	 { WR_REG(4,R.A); }
static void ld_r5_a(void)	 { WR_REG(5,R.A); }
static void ld_r6_a(void)	 { WR_REG(6,R.A); }
static void ld_r7_a(void)	 { WR_REG(7,R.A); }

static void add_a_n(void)	 { M_ADD(M_RDMEM_OPCODE()); }

static void add_a_r0(void)	 { M_ADD(RD_REG(0)); }
static void add_a_r1(void)	 { M_ADD(RD_REG(1)); }
static void add_a_r2(void)	 { M_ADD(RD_REG(2)); }
static void add_a_r3(void)	 { M_ADD(RD_REG(3)); }
static void add_a_r4(void)	 { M_ADD(RD_REG(4)); }
static void add_a_r5(void)	 { M_ADD(RD_REG(5)); }
static void add_a_r6(void)	 { M_ADD(RD_REG(6)); }
static void add_a_r7(void)	 { M_ADD(RD_REG(7)); }

static void sub_a_n(void)	 { M_SUB(M_RDMEM_OPCODE()); }

static void sub_a_r0(void)	 { M_SUB(RD_REG(0)); }
static void sub_a_r1(void)	 { M_SUB(RD_REG(1)); }
static void sub_a_r2(void)	 { M_SUB(RD_REG(2)); }
static void sub_a_r3(void)	 { M_SUB(RD_REG(3)); }
static void sub_a_r4(void)	 { M_SUB(RD_REG(4)); }
static void sub_a_r5(void)	 { M_SUB(RD_REG(5)); }
static void sub_a_r6(void)	 { M_SUB(RD_REG(6)); }
static void sub_a_r7(void)	 { M_SUB(RD_REG(7)); }

static void and_a_n(void)	 { M_AND(M_RDMEM_OPCODE()); }

static void and_a_r0(void)	 { M_AND(RD_REG(0)); }
static void and_a_r1(void)	 { M_AND(RD_REG(1)); }
static void and_a_r2(void)	 { M_AND(RD_REG(2)); }
static void and_a_r3(void)	 { M_AND(RD_REG(3)); }
static void and_a_r4(void)	 { M_AND(RD_REG(4)); }
static void and_a_r5(void)	 { M_AND(RD_REG(5)); }
static void and_a_r6(void)	 { M_AND(RD_REG(6)); }
static void and_a_r7(void)	 { M_AND(RD_REG(7)); }

static void or_a_n(void)	 { M_OR(M_RDMEM_OPCODE()); }

static void or_a_r0(void)	 { M_OR(RD_REG(0)); }
static void or_a_r1(void)	 { M_OR(RD_REG(1)); }
static void or_a_r2(void)	 { M_OR(RD_REG(2)); }
static void or_a_r3(void)	 { M_OR(RD_REG(3)); }
static void or_a_r4(void)	 { M_OR(RD_REG(4)); }
static void or_a_r5(void)	 { M_OR(RD_REG(5)); }
static void or_a_r6(void)	 { M_OR(RD_REG(6)); }
static void or_a_r7(void)	 { M_OR(RD_REG(7)); }

static void add_ix0_0(void)	 { }
static void add_ix0_1(void)	 { IX0 += 1; }
static void add_ix0_2(void)	 { IX0 += 2; }
static void add_ix0_3(void)	 { IX0 += 3; }
static void add_ix0_4(void)	 { IX0 += 4; }
static void add_ix0_5(void)	 { IX0 += 5; }
static void add_ix0_6(void)	 { IX0 += 6; }
static void add_ix0_7(void)	 { IX0 += 7; }
static void add_ix0_8(void)	 { IX0 += 8; }
static void add_ix0_9(void)	 { IX0 += 9; }
static void add_ix0_a(void)	 { IX0 += 10; }
static void add_ix0_b(void)	 { IX0 += 11; }
static void add_ix0_c(void)	 { IX0 += 12; }
static void add_ix0_d(void)	 { IX0 += 13; }
static void add_ix0_e(void)	 { IX0 += 14; }
static void add_ix0_f(void)	 { IX0 += 15; }

static void add_ix1_0(void)	 { }
static void add_ix1_1(void)	 { IX1 += 1; }
static void add_ix1_2(void)	 { IX1 += 2; }
static void add_ix1_3(void)	 { IX1 += 3; }
static void add_ix1_4(void)	 { IX1 += 4; }
static void add_ix1_5(void)	 { IX1 += 5; }
static void add_ix1_6(void)	 { IX1 += 6; }
static void add_ix1_7(void)	 { IX1 += 7; }
static void add_ix1_8(void)	 { IX1 += 8; }
static void add_ix1_9(void)	 { IX1 += 9; }
static void add_ix1_a(void)	 { IX1 += 10; }
static void add_ix1_b(void)	 { IX1 += 11; }
static void add_ix1_c(void)	 { IX1 += 12; }
static void add_ix1_d(void)	 { IX1 += 13; }
static void add_ix1_e(void)	 { IX1 += 14; }
static void add_ix1_f(void)	 { IX1 += 15; }

static void add_ix2_0(void)	 { }
static void add_ix2_1(void)	 { IX2 += 1; }
static void add_ix2_2(void)	 { IX2 += 2; }
static void add_ix2_3(void)	 { IX2 += 3; }
static void add_ix2_4(void)	 { IX2 += 4; }
static void add_ix2_5(void)	 { IX2 += 5; }
static void add_ix2_6(void)	 { IX2 += 6; }
static void add_ix2_7(void)	 { IX2 += 7; }
static void add_ix2_8(void)	 { IX2 += 8; }
static void add_ix2_9(void)	 { IX2 += 9; }
static void add_ix2_a(void)	 { IX2 += 10; }
static void add_ix2_b(void)	 { IX2 += 11; }
static void add_ix2_c(void)	 { IX2 += 12; }
static void add_ix2_d(void)	 { IX2 += 13; }
static void add_ix2_e(void)	 { IX2 += 14; }
static void add_ix2_f(void)	 { IX2 += 15; }

static void ld_base_0(void)	 { regPTR = 0; }
static void ld_base_1(void)	 { regPTR = 1; }
static void ld_base_2(void)	 { regPTR = 2; }
static void ld_base_3(void)	 { regPTR = 3; }
static void ld_base_4(void)	 { regPTR = 4; }
static void ld_base_5(void)	 { regPTR = 5; }
static void ld_base_6(void)	 { regPTR = 6; }
static void ld_base_7(void)	 { regPTR = 7; }

static void ld_bank_0(void)	 { R.mb = 0; }
static void ld_bank_1(void)	 { R.mb = 1; }
static void ld_bank_2(void)	 { R.mb = 2; }
static void ld_bank_3(void)	 { R.mb = 3; }

static void stop(void)
{
	UINT8 pcptr = M_RDMEM(0x001) & 0x1f;
	M_WRMEM(pcptr,(M_RDMEM(pcptr)&0xf)+0x08); /* mark entry point ODD to HALT */
	R.mb |= 0x08;        /* mark internal HALT state */
}

static void ld_ix0_n(void)	 { IX0 = M_RDMEM_OPCODE(); }
static void ld_ix1_n(void)	 { IX1 = M_RDMEM_OPCODE(); }
static void ld_ix2_n(void)	 { IX2 = M_RDMEM_OPCODE(); }
static void ld_lp0_n(void)	 { LP0 = M_RDMEM_OPCODE(); }
static void ld_lp1_n(void)	 { LP1 = M_RDMEM_OPCODE(); }
static void ld_lp2_n(void)	 { LP2 = M_RDMEM_OPCODE(); }
static void ld_b_n(void)	 { R.B = M_RDMEM_OPCODE(); }

static void djnz_lp0(void)	{ UINT8 i=M_RDMEM_OPCODE(); LP0--; if (LP0 != 0) M_JMP(i); }
static void djnz_lp1(void)	{ UINT8 i=M_RDMEM_OPCODE(); LP1--; if (LP1 != 0) M_JMP(i); }
static void djnz_lp2(void)	{ UINT8 i=M_RDMEM_OPCODE(); LP2--; if (LP2 != 0) M_JMP(i); }
static void jnz(void)	{ UINT8 i=M_RDMEM_OPCODE(); if (!ZF) M_JMP(i); }
static void jnc(void)	{ UINT8 i=M_RDMEM_OPCODE(); if (!CF) M_JMP(i);}
static void jz(void)	{ UINT8 i=M_RDMEM_OPCODE(); if ( ZF) M_JMP(i); }
static void jc(void)	{ UINT8 i=M_RDMEM_OPCODE(); if ( CF) M_JMP(i);}
static void jmp(void)	{ M_JMP( M_RDMEM_OPCODE() ); }

#if (HAS_ALPHA8201)
static const s_opcode opcode_8201[256]=
{
	{C1, nop        },{C1,rora      },{C1, rola      },{C1,inc_b     },{C1,dec_b     },{C1, inc_a    },{C1, dec_a    },{C1, cpl      },
	{C2,ld_a_ix0_0  },{C2,ld_a_ix0_1},{C2, ld_a_ix0_2},{C2,ld_a_ix0_3},{C2,ld_a_ix0_4},{C2,ld_a_ix0_5},{C2,ld_a_ix0_6},{C2,ld_a_ix0_7},
	{C2,ld_a_ix1_0  },{C2,ld_a_ix1_1},{C2, ld_a_ix1_2},{C2,ld_a_ix1_3},{C2,ld_a_ix1_4},{C2,ld_a_ix1_5},{C2,ld_a_ix1_6},{C2,ld_a_ix1_7},
	{C2,ld_ix2_0_a  },{C2,ld_ix2_1_a},{C2, ld_ix2_2_a},{C2,ld_ix2_3_a},{C2,ld_ix2_4_a},{C2,ld_ix2_5_a},{C2,ld_ix2_6_a},{C2,ld_ix2_7_a},
/* 20 */
	{C2,ld_ix0_0_b  },{C2,ld_ix0_1_b},{C2, ld_ix0_2_b},{C2,ld_ix0_3_b},{C2,ld_ix0_4_b},{C2,ld_ix0_5_b},{C2,ld_ix0_6_b},{C2,ld_ix0_7_b},
	{C2,undefined   },{C2,undefined },{C2, undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },
	{C2,undefined   },{C2,undefined },{C2, undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },
	{C2,bit_r0_0    },{C2,bit_r0_1  },{C2, bit_r0_2 },{C2, bit_r0_3 },{C2, bit_r0_4 },{C2, bit_r0_5 },{C2, bit_r0_6 },{C2, bit_r0_7 },
/* 40 : 8201 */
	{C2, ld_a_r0    },{C2, ld_r0_a	},{C2, ld_a_r1	},{C2, ld_r1_a	},{C2, ld_a_r2	},{C2, ld_r2_a	},{C2, ld_a_r3	},{C2, ld_r3_a	},
	{C2, ld_a_r4    },{C2, ld_r4_a	},{C2, ld_a_r5	},{C2, ld_r5_a	},{C2, ld_a_r6	},{C2, ld_r6_a	},{C2, ld_a_r7	},{C2, ld_r7_a	},
	{C1, add_a_r0   },{C1, sub_a_r0	},{C1, add_a_r1	},{C1, sub_a_r1	},{C1, add_a_r2	},{C1, sub_a_r2	},{C1, add_a_r3	},{C1, sub_a_r3	},
	{C1, add_a_r4   },{C1, sub_a_r4	},{C1, add_a_r5	},{C1, sub_a_r5	},{C1, add_a_r6	},{C1, sub_a_r6	},{C1, add_a_r7	},{C1, sub_a_r7	},
	{C1, and_a_r0   },{C1, or_a_r0	},{C1, and_a_r1	},{C1, or_a_r1	},{C1, and_a_r2	},{C1, or_a_r2	},{C1, and_a_r3	},{C1, or_a_r3	},
	{C1, and_a_r4	},{C1, or_a_r4	},{C1, and_a_r5	},{C1, or_a_r5	},{C1, and_a_r6	},{C1, or_a_r6	},{C1, and_a_r7	},{C1, or_a_r7	},
	{C1, add_ix0_0	},{C1, add_ix0_1},{C1, add_ix0_2},{C1, add_ix0_3},{C1, add_ix0_4},{C1, add_ix0_5},{C1, add_ix0_6},{C1, add_ix0_7},
	{C1, add_ix0_8	},{C1, add_ix0_9},{C1, add_ix0_a},{C1, add_ix0_b},{C1, add_ix0_c},{C1, add_ix0_d},{C1, add_ix0_e},{C1, add_ix0_f},
/* 80 : 8201 */
	{C1, add_ix1_0	},{C1, add_ix1_1},{C1, add_ix1_2},{C1, add_ix1_3},{C1, add_ix1_4},{C1, add_ix1_5},{C1, add_ix1_6},{C1, add_ix1_7},
	{C1, add_ix1_8	},{C1, add_ix1_9},{C1, add_ix1_a},{C1, add_ix1_b},{C1, add_ix1_c},{C1, add_ix1_d},{C1, add_ix1_e},{C1, add_ix1_f},
	{C1, add_ix2_0	},{C1, add_ix2_1},{C1, add_ix2_2},{C1, add_ix2_3},{C1, add_ix2_4},{C1, add_ix2_5},{C1, add_ix2_6},{C1, add_ix2_7},
	{C1, add_ix2_8	},{C1, add_ix2_9},{C1, add_ix2_a},{C1, add_ix2_b},{C1, add_ix2_c},{C1, add_ix2_d},{C1, add_ix2_e},{C1, add_ix2_f},
	{C1, ld_base_0	},{C1, ld_base_1},{C1, ld_base_2},{C1, ld_base_3},{C1, ld_base_4},{C1, ld_base_5},{C1, ld_base_6},{C1, ld_base_7},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, ld_bank_0	},{C1, ld_bank_1},{C1, ld_bank_2},{C1, ld_bank_3},{C2, stop     },{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
/* c0 : 8201 */
	{C2, ld_ix0_n	},{C2, ld_ix1_n	},{C2, ld_ix2_n	},{C2, ld_a_n	},{C2, ld_lp0_n	},{C2, ld_lp1_n	},{C2, ld_lp2_n	},{C2, ld_b_n	},
	{C2, add_a_n	},{C2, sub_a_n	},{C2, and_a_n	},{C2, or_a_n	},{C2, djnz_lp0	},{C2, djnz_lp1	},{C2, djnz_lp2	},{C2, jnz		},
	{C2, jnc	    	},{C2, jz		},{C2, jmp		},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2, undefined2},
	{C2, undefined2	},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2,undefined2},{C2, undefined2},
/* E0 : 8201*/
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined }
};
#endif

#if (HAS_ALPHA8301)

/* ALPHA 8301 : added instruction */
static void exg_a_ix0(void)  { UINT8 t=R.A; R.A = IX0; IX0 = t; }
static void exg_a_ix1(void)  { UINT8 t=R.A; R.A = IX1; IX1 = t; }
static void exg_a_ix2(void)  { UINT8 t=R.A; R.A = IX2; IX2 = t; }
static void exg_a_lp0(void)  { UINT8 t=R.A; R.A = LP0; LP0 = t; }
static void exg_a_lp1(void)  { UINT8 t=R.A; R.A = LP1; LP1 = t; }
static void exg_a_lp2(void)  { UINT8 t=R.A; R.A = LP2; LP2 = t; }
static void exg_a_b(void)    { UINT8 t=R.A; R.A = R.B; R.B = t; }
static void exg_a_rb(void)   { UINT8 t=R.A; R.A = regPTR; regPTR = t; }

static void ld_ix0_a(void)    { IX0 = R.A; }
static void ld_ix1_a(void)    { IX1 = R.A; }
static void ld_ix2_a(void)    { IX2 = R.A; }
static void ld_lp0_a(void)    { LP0 = R.A; }
static void ld_lp1_a(void)    { LP1 = R.A; }
static void ld_lp2_a(void)    { LP2 = R.A; }
static void ld_b_a(void)      { R.B = R.A; }
static void ld_rb_a(void)     { regPTR = R.A; }

static void exg_ix0_ix1(void)  { UINT8 t=IX1; IX1 = IX0; IX0 = t; }
static void exg_ix0_ix2(void)  { UINT8 t=IX2; IX2 = IX0; IX0 = t; }

static void op_d4(void) { R.A = M_RDMEM( ((regRAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE() ); }
static void op_d5(void) { M_WRMEM( ((regRAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(), R.A ); }
static void op_d6(void) { LP0 = M_RDMEM( ((regRAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE() ); }
static void op_d7(void) { M_WRMEM( ((regRAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(), LP0 ); }

static void ld_a_abs(void) { R.A = M_RDMEM( ((R.mb & 3) << 8) | M_RDMEM_OPCODE() ); }
static void ld_abs_a(void) { M_WRMEM( ((R.mb & 3) << 8) | M_RDMEM_OPCODE(), R.A ); }

static void ld_a_r(void) { R.A = regRAM[(M_RDMEM_OPCODE()>>1)&0x3f]; }
static void ld_r_a(void) { regRAM[(M_RDMEM_OPCODE()>>1)&0x3f] = R.A; }
static void op_rep_ld_ix2_b(void) { do { M_WRMEM(BIX2, regRAM[(R.B>>1)&0x3f]); IX2++; R.B+=2; LP0--; } while (LP0 != 0); }
static void op_rep_ld_b_ix0(void) { do { regRAM[(R.B>>1)&0x3f] = M_RDMEM(BIX0); IX0++; R.B+=2; LP0--; } while (LP0 != 0); }
static void ld_rxb_a(void) { regRAM[(R.B>>1)&0x3f] = R.A; }
static void ld_a_rxb(void) { R.A = regRAM[(R.B>>1)&0x3f]; }
static void cmp_a_rxb(void) { UINT8 i=regRAM[(R.B>>1)&0x3f];  ZF = (R.A==i); CF = (R.A>=i); }
static void xor_a_rxb(void) { M_XOR( regRAM[(R.B>>1)&0x3f] ); }

static void add_a_cf(void) { if (CF) inc_a(); }
static void sub_a_cf(void) { if (CF) dec_a(); }
static void tst_a(void)	 { ZF = (R.A==0); }
static void clr_a(void)	 { R.A = 0; ZF = (R.A==0); }
static void cmp_a_n(void)	{ UINT8 i=M_RDMEM_OPCODE();  ZF = (R.A==i); CF = (R.A>=i); }
static void xor_a_n(void)	{ M_XOR( M_RDMEM_OPCODE() ); }
static void call(void) { UINT8 i=M_RDMEM_OPCODE(); R.retptr.w.l = PC; M_JMP(i); };
static void ld_a_ix0_a(void) { R.A = M_RDMEM(BIX0+R.A); }
static void ret(void) { R.mb = R.retptr.b.h; M_JMP( R.retptr.b.l ); };
static void save_zc(void) { R.savez = ZF; R.savec = CF; };
static void rest_zc(void) { ZF = R.savez; CF = R.savec; };

static const s_opcode opcode_8301[256]=
{
	{C1, nop        },{C1,rora      },{C1, rola      },{C1,inc_b     },{C1,dec_b     },{C1, inc_a    },{C1, dec_a    },{C1, cpl      },
	{C2,ld_a_ix0_0  },{C2,ld_a_ix0_1},{C2, ld_a_ix0_2},{C2,ld_a_ix0_3},{C2,ld_a_ix0_4},{C2,ld_a_ix0_5},{C2,ld_a_ix0_6},{C2,ld_a_ix0_7},
	{C2,ld_a_ix1_0  },{C2,ld_a_ix1_1},{C2, ld_a_ix1_2},{C2,ld_a_ix1_3},{C2,ld_a_ix1_4},{C2,ld_a_ix1_5},{C2,ld_a_ix1_6},{C2,ld_a_ix1_7},
	{C2,ld_ix2_0_a  },{C2,ld_ix2_1_a},{C2, ld_ix2_2_a},{C2,ld_ix2_3_a},{C2,ld_ix2_4_a},{C2,ld_ix2_5_a},{C2,ld_ix2_6_a},{C2,ld_ix2_7_a},
/* 20 : 8301 */
	{C2,ld_ix0_0_b  },{C2,ld_ix0_1_b},{C2, ld_ix0_2_b},{C2,ld_ix0_3_b},{C2,ld_ix0_4_b},{C2,ld_ix0_5_b},{C2,ld_ix0_6_b},{C2,ld_ix0_7_b},
	{C2,undefined   },{C2,undefined },{C2, undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },
	{C2,undefined   },{C2,undefined },{C2, undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },{C2,undefined },
	{C2,bit_r0_0    },{C2,bit_r0_1  },{C2, bit_r0_2 },{C2, bit_r0_3 },{C2, bit_r0_4 },{C2, bit_r0_5 },{C2, bit_r0_6 },{C2, bit_r0_7 },
/* 40 : 8301 */
	{C2, ld_a_r0    },{C2, ld_r0_a	},{C2, ld_a_r1	},{C2, ld_r1_a	},{C2, ld_a_r2	},{C2, ld_r2_a	},{C2, ld_a_r3	},{C2, ld_r3_a	},
	{C2, ld_a_r4    },{C2, ld_r4_a	},{C2, ld_a_r5	},{C2, ld_r5_a	},{C2, ld_a_r6	},{C2, ld_r6_a	},{C2, ld_a_r7	},{C2, ld_r7_a	},
	{C1, add_a_r0   },{C1, sub_a_r0	},{C1, add_a_r1	},{C1, sub_a_r1	},{C1, add_a_r2	},{C1, sub_a_r2	},{C1, add_a_r3	},{C1, sub_a_r3	},
	{C1, add_a_r4   },{C1, sub_a_r4	},{C1, add_a_r5	},{C1, sub_a_r5	},{C1, add_a_r6	},{C1, sub_a_r6	},{C1, add_a_r7	},{C1, sub_a_r7	},
/* 60 : 8301 */
	{C1, and_a_r0   },{C1, or_a_r0	},{C1, and_a_r1	},{C1, or_a_r1	},{C1, and_a_r2	},{C1, or_a_r2	},{C1, and_a_r3	},{C1, or_a_r3	},
	{C1, and_a_r4	},{C1, or_a_r4	},{C1, and_a_r5	},{C1, or_a_r5	},{C1, and_a_r6	},{C1, or_a_r6	},{C1, and_a_r7	},{C1, or_a_r7	},
	{C1, add_ix0_0	},{C1, add_ix0_1},{C1, add_ix0_2},{C1, add_ix0_3},{C1, add_ix0_4},{C1, add_ix0_5},{C1, add_ix0_6},{C1, add_ix0_7},
	{C1, add_ix0_8	},{C1, add_ix0_9},{C1, add_ix0_a},{C1, add_ix0_b},{C1, add_ix0_c},{C1, add_ix0_d},{C1, add_ix0_e},{C1, add_ix0_f},
/* 80 : 8301 */
	{C1, add_ix1_0	},{C1, add_ix1_1},{C1, add_ix1_2},{C1, add_ix1_3},{C1, add_ix1_4},{C1, add_ix1_5},{C1, add_ix1_6},{C1, add_ix1_7},
	{C1, add_ix1_8	},{C1, add_ix1_9},{C1, add_ix1_a},{C1, add_ix1_b},{C1, add_ix1_c},{C1, add_ix1_d},{C1, add_ix1_e},{C1, add_ix1_f},
	{C1, add_ix2_0	},{C1, add_ix2_1},{C1, add_ix2_2},{C1, add_ix2_3},{C1, add_ix2_4},{C1, add_ix2_5},{C1, add_ix2_6},{C1, add_ix2_7},
	{C1, add_ix2_8	},{C1, add_ix2_9},{C1, add_ix2_a},{C1, add_ix2_b},{C1, add_ix2_c},{C1, add_ix2_d},{C1, add_ix2_e},{C1, add_ix2_f},
/* A0 : 8301 */
	{C1, ld_base_0	},{C1, ld_base_1},{C1, ld_base_2},{C1, ld_base_3},{C1, ld_base_4},{C1, ld_base_5},{C1, ld_base_6},{C1, ld_base_7},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, ld_bank_0	},{C1, ld_bank_1},{C1, ld_bank_2},{C1, ld_bank_3},{C2, stop     },{C1, undefined},{C1, undefined},{C1, undefined},
	{C1, undefined	},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},{C1, undefined},
/* c0 : 8301 */
	{C2, ld_ix0_n	},{C2, ld_ix1_n},{C2, ld_ix2_n	},{C2, ld_a_n	},{C2, ld_lp0_n	},{C2, ld_lp1_n	},{C2, ld_lp2_n	},{C2, ld_b_n	},
	{C2, add_a_n	},{C2, sub_a_n	},{C2, and_a_n	},{C2, or_a_n	},{C2, djnz_lp0	},{C2, djnz_lp1	},{C2, djnz_lp2	},{C2, jnz		},
	{C2, jnc			},{C2, jz		},{C2, jmp		},{C2,undefined2},{C2, op_d4    },{C2, op_d5    },{C2, op_d6    },{C2, op_d7    },
	{C2, ld_a_abs  },{C2, ld_abs_a},{C2,cmp_a_n	},{C2,xor_a_n   },{C2, ld_a_r   },{C2, ld_r_a   },{C2, jc       },{C2, call},
/* E0 : 8301 */
	{C1, exg_a_ix0	},{C1, exg_a_ix1},{C1, exg_a_ix2},{C1, exg_a_lp1},{C1, exg_a_lp2},{C1, exg_a_b  },{C1, exg_a_lp0},{C1, exg_a_rb },
	{C1, ld_ix0_a	},{C1, ld_ix1_a },{C1, ld_ix2_a },{C1, ld_lp1_a },{C1, ld_lp2_a },{C1, ld_b_a   },{C1, ld_lp0_a },{C1, ld_rb_a  },
	{C1,exg_ix0_ix1},{C1,exg_ix0_ix2},{C1,op_rep_ld_ix2_b},{C1, op_rep_ld_b_ix0},{C1, save_zc},{C1, rest_zc},{C1, ld_rxb_a },{C1, ld_a_rxb },
	{C1, cmp_a_rxb },{C1, xor_a_rxb},{C1, add_a_cf },{C1, sub_a_cf },{C1, tst_a    },{C1, clr_a    },{C1, ld_a_ix0_a},{C1, ret     }
};
#endif

/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
static CPU_INIT( ALPHA8201 )
{
	R.device = device;
	R.program = memory_find_address_space(device, ADDRESS_SPACE_PROGRAM);

	state_save_register_device_item_array(device, 0, R.RAM);
	state_save_register_device_item(device, 0, R.PREVPC);
	state_save_register_device_item(device, 0, PC);
	state_save_register_device_item(device, 0, regPTR);
	state_save_register_device_item(device, 0, ZF);
	state_save_register_device_item(device, 0, CF);
	state_save_register_device_item(device, 0, R.mb);
#if HANDLE_HALT_LINE
	state_save_register_device_item(device, 0, R.halt);
#endif
	state_save_register_device_item(device, 0, IX0);
	state_save_register_device_item(device, 0, IX1);
	state_save_register_device_item(device, 0, IX2);
	state_save_register_device_item(device, 0, LP0);
	state_save_register_device_item(device, 0, LP1);
	state_save_register_device_item(device, 0, LP2);
	state_save_register_device_item(device, 0, R.A);
	state_save_register_device_item(device, 0, R.B);
	state_save_register_device_item(device, 0, R.retptr);
	state_save_register_device_item(device, 0, R.savec);
	state_save_register_device_item(device, 0, R.savez);
}
/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( ALPHA8201 )
{
	PC     = 0;
	regPTR = 0;
	ZF     = 0;
	CF     = 0;
	R.mb   = 0;
	BIX0   = 0;
	BIX1   = 0;
	BIX2   = 0;
	LP0    = 0;
	LP1    = 0;
	LP2    = 0;
	R.A    = 0;
	R.B   = 0;
#if HANDLE_HALT_LINE
	R.halt = 0;
#endif
}

/****************************************************************************
 * Shut down CPU emulation
 ****************************************************************************/
static CPU_EXIT( ALPHA8201 )
{
	/* nothing to do ? */
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/

static int alpha8xxx_execute(const device_config *device,const s_opcode *op_map,int cycles)
{
	unsigned opcode;
	UINT8 pcptr;

	ALPHA8201_ICount = cycles;

#if HANDLE_HALT_LINE
	if(R.halt)
		return cycles;
#endif

	/* setup address bank & fall safe */
	R.ix0.b.h =
	R.ix1.b.h =
	R.ix2.b.h = (R.pc.b.h &= 3);

	/* reset start hack */
	if(PC<0x20)
		R.mb |= 0x08;

	do
	{
		if(R.mb & 0x08)
		{
			pcptr = M_RDMEM(0x001) & 0x1f; /* pointer of entry point */
			ALPHA8201_ICount -= C1;

			/* entry point scan phase */
			if( (pcptr&1) == 0)
			{
				/* EVEN , get PC low */
				R.pc.b.l = M_RDMEM(pcptr);
//mame_printf_debug("ALPHA8201 load PCL ENTRY=%02X PCL=%02X\n",pcptr, R.pc.b.l);
				ALPHA8201_ICount -= C1;
				M_WRMEM(0x001,pcptr+1);
				continue;
			}

			/* ODD , check HALT flag */
			R.mb   = M_RDMEM(pcptr) & (0x08|0x03);
			ALPHA8201_ICount -= C1;

			/* not entryaddress 000,001 */
			if(pcptr<2) R.mb |= 0x08;

			if(R.mb & 0x08)
			{
				/* HALTED current entry point . next one */
				pcptr = (pcptr+1)&0x1f;
				M_WRMEM(0x001,pcptr);
				ALPHA8201_ICount -= C1;
				continue;
			}

			/* goto run phase */
			M_JMP(R.pc.b.l);

#if SHOW_ENTRY_POINT
logerror("ALPHA8201 START ENTRY=%02X PC=%03X\n",pcptr,PC);
mame_printf_debug("ALPHA8201 START ENTRY=%02X PC=%03X\n",pcptr,PC);
#endif
		}

		/* run */
		R.PREVPC = PC;
		debugger_instruction_hook(device, PC);
		opcode =M_RDOP(PC);
#if TRACE_PC
mame_printf_debug("ALPHA8201:  PC = %03x,  opcode = %02x\n", PC, opcode);
#endif
		PCL++;
		inst_cycles = op_map[opcode].cycles;
		(*(op_map[opcode].function))();
		ALPHA8201_ICount -= inst_cycles;
	} while (ALPHA8201_ICount>0);

	return cycles - ALPHA8201_ICount;
}

#if (HAS_ALPHA8201)
static CPU_EXECUTE( ALPHA8201 ) { return alpha8xxx_execute(device,opcode_8201,cycles); }
#endif

#if (HAS_ALPHA8301)
static CPU_EXECUTE( ALPHA8301 ) { return alpha8xxx_execute(device,opcode_8301,cycles); }
#endif

/****************************************************************************
 * Get all registers in given buffer
 ****************************************************************************/
static CPU_GET_CONTEXT( ALPHA8201 )
{
	if( dst )
		*(ALPHA8201_Regs*)dst = R;
}

/****************************************************************************
 * Set all registers to given values
 ****************************************************************************/
static CPU_SET_CONTEXT( ALPHA8201 )
{
	if( src )
		R = *(ALPHA8201_Regs*)src;
}

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
#if HANDLE_HALT_LINE
static void set_irq_line(int irqline, int state)
{
	if(irqline == INPUT_LINE_HALT)
	{
		R.halt = (state==ASSERT_LINE) ? 1 : 0;
/* mame_printf_debug("ALPHA8201 HALT %d\n",R.halt); */
	}
}
#endif

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( ALPHA8201 )
{
	switch (state)
	{
#if HANDLE_HALT_LINE
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_HALT:	set_irq_line(INPUT_LINE_HALT, info->i);	break;
#endif
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ALPHA8201_PC:			PC = info->i;						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ALPHA8201_SP:			M_WRMEM(0x001,info->i);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_RB:			regPTR = info->i & 7;				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_MB:			R.mb = info->i & 0x03;				break;
#if 0
		case CPUINFO_INT_REGISTER + ALPHA8201_ZF:			ZF= info->i & 0x01;				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_CF:			CF= info->i & 0x01;				break;
#endif
		case CPUINFO_INT_REGISTER + ALPHA8201_IX0:			IX0 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX1:			IX1 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX2:			IX2 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP0:			LP0 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP1:			LP1 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP2:			LP2 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_A:			R.A = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_B:			R.B = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R0:			WR_REG(0,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R1:			WR_REG(1,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R2:			WR_REG(2,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R3:			WR_REG(3,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R4:			WR_REG(4,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R5:			WR_REG(5,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R6:			WR_REG(6,info->i);					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R7:			WR_REG(7,info->i);					break;
	}
}



/**************************************************************************
 * Generic get_info
 **************************************************************************/

/* 8201 and 8301 */
static CPU_GET_INFO( alpha8xxx )
{
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(R);					break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_DATA: 	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + ADDRESS_SPACE_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + ADDRESS_SPACE_IO: 		info->i = 6;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + ADDRESS_SPACE_IO: 		info->i = 0;					break;
#if HANDLE_HALT_LINE
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_HALT:		info->i = R.halt ? ASSERT_LINE : CLEAR_LINE; break;
#endif
		case CPUINFO_INT_PREVIOUSPC:						info->i = R.PREVPC;					break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ALPHA8201_PC:			info->i = PC & 0x3ff;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ALPHA8201_SP:			info->i = M_RDMEM(0x001);			break;
		case CPUINFO_INT_REGISTER + ALPHA8201_RB:			info->i = regPTR;					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_MB:			info->i = R.mb;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_ZF:			info->i = ZF;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_CF:			info->i = CF;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX0:			info->i = IX0;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX1:			info->i = IX1;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX2:			info->i = IX2;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP0:			info->i = LP0;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP1:			info->i = LP1;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP2:			info->i = LP2;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_A:			info->i = R.A;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_B:			info->i = R.B;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R0:			info->i = RD_REG(0);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R1:			info->i = RD_REG(1);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R2:			info->i = RD_REG(2);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R3:			info->i = RD_REG(3);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R4:			info->i = RD_REG(4);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R5:			info->i = RD_REG(5);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R6:			info->i = RD_REG(6);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R7:			info->i = RD_REG(7);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_PTR_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(ALPHA8201);		break;
		case CPUINFO_PTR_GET_CONTEXT:					info->getcontext = CPU_GET_CONTEXT_NAME(ALPHA8201); break;
		case CPUINFO_PTR_SET_CONTEXT:					info->setcontext = CPU_SET_CONTEXT_NAME(ALPHA8201); break;
		case CPUINFO_PTR_INIT:							info->init = CPU_INIT_NAME(ALPHA8201);			break;
		case CPUINFO_PTR_RESET:							info->reset = CPU_RESET_NAME(ALPHA8201);			break;
		case CPUINFO_PTR_EXIT:							info->exit = CPU_EXIT_NAME(ALPHA8201);			break;
		case CPUINFO_PTR_BURN:							info->burn = NULL;						break;
		case CPUINFO_PTR_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(ALPHA8201);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &ALPHA8201_ICount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_CORE_FAMILY:					strcpy(info->s, "AlphaDenshi MCU");		break;
		case CPUINFO_STR_CORE_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_CORE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CORE_CREDITS:					strcpy(info->s, "Copyright Tatsuyuki Satoh"); break;
		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c", CF?'C':'.',ZF?'Z':'.'); break;
		case CPUINFO_STR_REGISTER + ALPHA8201_PC:		sprintf(info->s, "PC:%03X", PC);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_SP:		sprintf(info->s, "SP:%02X", M_RDMEM(0x001) ); break;
		case CPUINFO_STR_REGISTER + ALPHA8201_RB:		sprintf(info->s, "RB:%X", regPTR);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_MB:		sprintf(info->s, "MB:%X", R.mb);		break;
#if 0
		case CPUINFO_STR_REGISTER + ALPHA8201_ZF:		sprintf(info->s, "ZF:%X", ZF);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_CF:		sprintf(info->s, "CF:%X", CF);		break;
#endif
		case CPUINFO_STR_REGISTER + ALPHA8201_IX0:		sprintf(info->s, "IX0:%02X", IX0);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_IX1:		sprintf(info->s, "IX1:%02X", IX1);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_IX2:		sprintf(info->s, "IX2:%02X", IX2);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP0:		sprintf(info->s, "LP0:%02X", LP0);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP1:		sprintf(info->s, "LP1:%02X", LP1);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP2:		sprintf(info->s, "LP2:%02X", LP2);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_A:		sprintf(info->s, "A:%02X", R.A);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_B:		sprintf(info->s, "B:%02X", R.B);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R0:		sprintf(info->s, "R0:%02X", RD_REG(0));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R1:		sprintf(info->s, "R1:%02X", RD_REG(1));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R2:		sprintf(info->s, "R2:%02X", RD_REG(2));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R3:		sprintf(info->s, "R3:%02X", RD_REG(3));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R4:		sprintf(info->s, "R4:%02X", RD_REG(4));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R5:		sprintf(info->s, "R5:%02X", RD_REG(5));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R6:		sprintf(info->s, "R6:%02X", RD_REG(6));		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_R7:		sprintf(info->s, "R7:%02X", RD_REG(7));		break;
	}
}
#if (HAS_ALPHA8201)
CPU_GET_INFO( alpha8201 )
{
	switch (state)
	{
	case CPUINFO_STR_NAME:							strcpy(info->s, "ALPHA-8201");				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(ALPHA8201);			break;
	default:
		/* 8201 / 8301 */
		CPU_GET_INFO_CALL(alpha8xxx);
	}
}
#endif

#if (HAS_ALPHA8301)
CPU_GET_INFO( alpha8301 )
{
	switch (state)
	{
	case CPUINFO_STR_NAME:							strcpy(info->s, "ALPHA-8301");				break;
	case CPUINFO_PTR_EXECUTE:						info->execute = CPU_EXECUTE_NAME(ALPHA8301);			break;
	default:
		/* 8201 / 8301 */
		CPU_GET_INFO_CALL(alpha8xxx);
	}
}
#endif
