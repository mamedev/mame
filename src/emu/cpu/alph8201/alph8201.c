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
Exciting Soccer           1983  8302 (pcb)
Champion Base Ball II     1983  8302 (pcb, unofficial schematics)
Exciting Soccer II        1984  8303 (uses 8303+ opcodes)
Equites                   1984  8303 (post)
Bull Fighter              1984  8303 (post)
Splendor Blast            1985  8303 (post)
Gekisou                   1985  8304 (post)
The Koukouyakyuh          1985  8304 (post)
High Voltage              1985  8404?(post says 8404, but readme says 8304)

alpha8201: "44801A75" -> HD44801 , ROM code = A75
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

cpustate->IX0/1   : memory indirect 'read' access pointer

cpustate->IX2     : memory indirect 'write' access pointer

RXB     : unknown , looks index register

cpustate->LP0/1/2 : loop count register used by DJNZ operation

cpustate->cf      : carry flag
cpustate->zf      : zero flag

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

#include "emu.h"
#include "debugger.h"
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

#define M_RDMEM(A)		cpustate->program->read_byte(A)
#define M_WRMEM(A,V)	cpustate->program->write_byte(A, V)
#define M_RDOP(A)		cpustate->direct->read_decrypted_byte(A)
#define M_RDOP_ARG(A)	cpustate->direct->read_raw_byte(A)

struct alpha8201_state
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

	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	int icount;
	int inst_cycles;
};

/* The opcode table now is a combination of cycle counts and function pointers */
struct s_opcode {
	unsigned cycles;
	void (*function) (alpha8201_state *cpustate);
};


#define PC				pc.w.l
#define PCL				pc.b.l
#define RD_REG(x)		cpustate->RAM[(cpustate->regPtr<<3)+(x)]
#define WR_REG(x,d)		cpustate->RAM[(cpustate->regPtr<<3)+(x)]=(d)
#define IX0				ix0.b.l
#define IX1				ix1.b.l
#define IX2				ix2.b.l
#define BIX0			ix0.w.l
#define BIX1			ix1.w.l
#define BIX2			ix2.w.l
#define LP0				lp0
#define LP1				lp1
#define LP2				lp2

INLINE alpha8201_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == ALPHA8201 ||
		   device->type() == ALPHA8301);
	return (alpha8201_state *)downcast<legacy_cpu_device *>(device)->token();
}

/* Get next opcode argument and increment program counter */
INLINE unsigned M_RDMEM_OPCODE (alpha8201_state *cpustate)
{
	unsigned retval;
	retval=M_RDOP_ARG(cpustate->PC);
	cpustate->PCL++;
	return retval;
}

INLINE void M_ADD(alpha8201_state *cpustate, UINT8 dat)
{
	UINT16 temp = cpustate->A + dat;
	cpustate->A = temp & 0xff;
	cpustate->zf = (cpustate->A==0);
	cpustate->cf = temp>>8;
}

INLINE void M_ADDB(alpha8201_state *cpustate, UINT8 dat)
{
	UINT16 temp = cpustate->B + dat;
	cpustate->B = temp & 0xff;
	cpustate->zf = (cpustate->B==0);
	cpustate->cf = temp>>8;
}

INLINE void M_SUB(alpha8201_state *cpustate, UINT8 dat)
{
	cpustate->cf = (cpustate->A>=dat);	// cpustate->cf is No Borrow
	cpustate->A -= dat;
	cpustate->zf = (cpustate->A==0);
}

INLINE void M_AND(alpha8201_state *cpustate, UINT8 dat)
{
	cpustate->A &= dat;
	cpustate->zf = (cpustate->A==0);
}

INLINE void M_OR(alpha8201_state *cpustate, UINT8 dat)
{
	cpustate->A |= dat;
	cpustate->zf = (cpustate->A==0);
}

INLINE void M_XOR(alpha8201_state *cpustate, UINT8 dat)
{
	cpustate->A ^= dat;
	cpustate->zf = (cpustate->A==0);
	cpustate->cf = 0;
}

INLINE void M_JMP(alpha8201_state *cpustate, UINT8 dat)
{
	cpustate->PCL = dat;
	/* update pc page */
	cpustate->pc.b.h  = cpustate->ix0.b.h = cpustate->ix1.b.h = cpustate->ix2.b.h = cpustate->mb & 3;
}

INLINE void M_UNDEFINED(alpha8201_state *cpustate)
{
	logerror("alpha8201:  cpustate->PC = %03x,  Unimplemented opcode = %02x\n", cpustate->PC-1, M_RDMEM(cpustate->PC-1));
#if SHOW_MESSAGE_CONSOLE
	mame_printf_debug("alpha8201:  cpustate->PC = %03x,  Unimplemented opcode = %02x\n", cpustate->PC-1, M_RDMEM(cpustate->PC-1));
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(cpustate->device->machine());
#endif
}

INLINE void M_UNDEFINED2(alpha8201_state *cpustate)
{
	UINT8 op  = M_RDOP(cpustate->PC-1);
	UINT8 imm = M_RDMEM_OPCODE(cpustate);
	logerror("alpha8201:  cpustate->PC = %03x,  Unimplemented opcode = %02x,%02x\n", cpustate->PC-2, op,imm);
#if SHOW_MESSAGE_CONSOLE
	mame_printf_debug("alpha8201:  cpustate->PC = %03x,  Unimplemented opcode = %02x,%02x\n", cpustate->PC-2, op,imm);
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(cpustate->device->machine());
#endif
}

static void undefined(alpha8201_state *cpustate)	{ M_UNDEFINED(cpustate); }
static void undefined2(alpha8201_state *cpustate)	{ M_UNDEFINED2(cpustate); }

static void nop(alpha8201_state *cpustate)		 { }
static void rora(alpha8201_state *cpustate)		 { cpustate->cf = cpustate->A &1;     cpustate->A = (cpustate->A>>1) | (cpustate->A<<7); }
static void rola(alpha8201_state *cpustate)		 { cpustate->cf = (cpustate->A>>7)&1; cpustate->A = (cpustate->A<<1) | (cpustate->A>>7); }
static void inc_b(alpha8201_state *cpustate)		 { M_ADDB(cpustate, 0x02); }
static void dec_b(alpha8201_state *cpustate)		 { M_ADDB(cpustate, 0xfe); }
static void inc_a(alpha8201_state *cpustate)		 { M_ADD(cpustate, 0x01); }
static void dec_a(alpha8201_state *cpustate)		 { M_ADD(cpustate, 0xff); }
static void cpl(alpha8201_state *cpustate)		 { cpustate->A ^= 0xff; };

static void ld_a_ix0_0(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+0); }
static void ld_a_ix0_1(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+1); }
static void ld_a_ix0_2(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+2); }
static void ld_a_ix0_3(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+3); }
static void ld_a_ix0_4(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+4); }
static void ld_a_ix0_5(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+5); }
static void ld_a_ix0_6(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+6); }
static void ld_a_ix0_7(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+7); }

static void ld_a_ix1_0(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+0); }
static void ld_a_ix1_1(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+1); }
static void ld_a_ix1_2(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+2); }
static void ld_a_ix1_3(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+3); }
static void ld_a_ix1_4(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+4); }
static void ld_a_ix1_5(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+5); }
static void ld_a_ix1_6(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+6); }
static void ld_a_ix1_7(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX1+7); }

static void ld_ix2_0_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+0,cpustate->A); }
static void ld_ix2_1_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+1,cpustate->A); }
static void ld_ix2_2_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+2,cpustate->A); }
static void ld_ix2_3_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+3,cpustate->A); }
static void ld_ix2_4_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+4,cpustate->A); }
static void ld_ix2_5_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+5,cpustate->A); }
static void ld_ix2_6_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+6,cpustate->A); }
static void ld_ix2_7_a(alpha8201_state *cpustate) { M_WRMEM(cpustate->BIX2+7,cpustate->A); }

static void ld_ix0_0_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+0); }
static void ld_ix0_1_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+1); }
static void ld_ix0_2_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+2); }
static void ld_ix0_3_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+3); }
static void ld_ix0_4_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+4); }
static void ld_ix0_5_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+5); }
static void ld_ix0_6_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+6); }
static void ld_ix0_7_b(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0+7); }

static void bit_r0_0(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<0)?0:1; }
static void bit_r0_1(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<1)?0:1; }
static void bit_r0_2(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<2)?0:1; }
static void bit_r0_3(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<3)?0:1; }
static void bit_r0_4(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<4)?0:1; }
static void bit_r0_5(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<5)?0:1; }
static void bit_r0_6(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<6)?0:1; }
static void bit_r0_7(alpha8201_state *cpustate)	 { cpustate->zf = RD_REG(0)&(1<<7)?0:1; }

static void ld_a_n(alpha8201_state *cpustate)	 { cpustate->A = M_RDMEM_OPCODE(cpustate); }

static void ld_a_r0(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(0); cpustate->zf = (cpustate->A==0); }
static void ld_a_r1(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(1); cpustate->zf = (cpustate->A==0); }
static void ld_a_r2(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(2); cpustate->zf = (cpustate->A==0); }
static void ld_a_r3(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(3); cpustate->zf = (cpustate->A==0); }
static void ld_a_r4(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(4); cpustate->zf = (cpustate->A==0); }
static void ld_a_r5(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(5); cpustate->zf = (cpustate->A==0); }
static void ld_a_r6(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(6); cpustate->zf = (cpustate->A==0); }
static void ld_a_r7(alpha8201_state *cpustate)	 { cpustate->A = RD_REG(7); cpustate->zf = (cpustate->A==0); }

static void ld_r0_a(alpha8201_state *cpustate)	 { WR_REG(0,cpustate->A); }
static void ld_r1_a(alpha8201_state *cpustate)	 { WR_REG(1,cpustate->A); }
static void ld_r2_a(alpha8201_state *cpustate)	 { WR_REG(2,cpustate->A); }
static void ld_r3_a(alpha8201_state *cpustate)	 { WR_REG(3,cpustate->A); }
static void ld_r4_a(alpha8201_state *cpustate)	 { WR_REG(4,cpustate->A); }
static void ld_r5_a(alpha8201_state *cpustate)	 { WR_REG(5,cpustate->A); }
static void ld_r6_a(alpha8201_state *cpustate)	 { WR_REG(6,cpustate->A); }
static void ld_r7_a(alpha8201_state *cpustate)	 { WR_REG(7,cpustate->A); }

static void add_a_n(alpha8201_state *cpustate)	 { M_ADD(cpustate, M_RDMEM_OPCODE(cpustate)); }

static void add_a_r0(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(0)); }
static void add_a_r1(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(1)); }
static void add_a_r2(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(2)); }
static void add_a_r3(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(3)); }
static void add_a_r4(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(4)); }
static void add_a_r5(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(5)); }
static void add_a_r6(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(6)); }
static void add_a_r7(alpha8201_state *cpustate)	 { M_ADD(cpustate, RD_REG(7)); }

static void sub_a_n(alpha8201_state *cpustate)	 { M_SUB(cpustate, M_RDMEM_OPCODE(cpustate)); }

static void sub_a_r0(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(0)); }
static void sub_a_r1(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(1)); }
static void sub_a_r2(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(2)); }
static void sub_a_r3(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(3)); }
static void sub_a_r4(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(4)); }
static void sub_a_r5(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(5)); }
static void sub_a_r6(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(6)); }
static void sub_a_r7(alpha8201_state *cpustate)	 { M_SUB(cpustate, RD_REG(7)); }

static void and_a_n(alpha8201_state *cpustate)	 { M_AND(cpustate, M_RDMEM_OPCODE(cpustate)); }

static void and_a_r0(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(0)); }
static void and_a_r1(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(1)); }
static void and_a_r2(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(2)); }
static void and_a_r3(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(3)); }
static void and_a_r4(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(4)); }
static void and_a_r5(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(5)); }
static void and_a_r6(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(6)); }
static void and_a_r7(alpha8201_state *cpustate)	 { M_AND(cpustate, RD_REG(7)); }

static void or_a_n(alpha8201_state *cpustate)	 { M_OR(cpustate, M_RDMEM_OPCODE(cpustate)); }

static void or_a_r0(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(0)); }
static void or_a_r1(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(1)); }
static void or_a_r2(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(2)); }
static void or_a_r3(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(3)); }
static void or_a_r4(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(4)); }
static void or_a_r5(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(5)); }
static void or_a_r6(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(6)); }
static void or_a_r7(alpha8201_state *cpustate)	 { M_OR(cpustate, RD_REG(7)); }

static void add_ix0_0(alpha8201_state *cpustate)	 { }
static void add_ix0_1(alpha8201_state *cpustate)	 { cpustate->IX0 += 1; }
static void add_ix0_2(alpha8201_state *cpustate)	 { cpustate->IX0 += 2; }
static void add_ix0_3(alpha8201_state *cpustate)	 { cpustate->IX0 += 3; }
static void add_ix0_4(alpha8201_state *cpustate)	 { cpustate->IX0 += 4; }
static void add_ix0_5(alpha8201_state *cpustate)	 { cpustate->IX0 += 5; }
static void add_ix0_6(alpha8201_state *cpustate)	 { cpustate->IX0 += 6; }
static void add_ix0_7(alpha8201_state *cpustate)	 { cpustate->IX0 += 7; }
static void add_ix0_8(alpha8201_state *cpustate)	 { cpustate->IX0 += 8; }
static void add_ix0_9(alpha8201_state *cpustate)	 { cpustate->IX0 += 9; }
static void add_ix0_a(alpha8201_state *cpustate)	 { cpustate->IX0 += 10; }
static void add_ix0_b(alpha8201_state *cpustate)	 { cpustate->IX0 += 11; }
static void add_ix0_c(alpha8201_state *cpustate)	 { cpustate->IX0 += 12; }
static void add_ix0_d(alpha8201_state *cpustate)	 { cpustate->IX0 += 13; }
static void add_ix0_e(alpha8201_state *cpustate)	 { cpustate->IX0 += 14; }
static void add_ix0_f(alpha8201_state *cpustate)	 { cpustate->IX0 += 15; }

static void add_ix1_0(alpha8201_state *cpustate)	 { }
static void add_ix1_1(alpha8201_state *cpustate)	 { cpustate->IX1 += 1; }
static void add_ix1_2(alpha8201_state *cpustate)	 { cpustate->IX1 += 2; }
static void add_ix1_3(alpha8201_state *cpustate)	 { cpustate->IX1 += 3; }
static void add_ix1_4(alpha8201_state *cpustate)	 { cpustate->IX1 += 4; }
static void add_ix1_5(alpha8201_state *cpustate)	 { cpustate->IX1 += 5; }
static void add_ix1_6(alpha8201_state *cpustate)	 { cpustate->IX1 += 6; }
static void add_ix1_7(alpha8201_state *cpustate)	 { cpustate->IX1 += 7; }
static void add_ix1_8(alpha8201_state *cpustate)	 { cpustate->IX1 += 8; }
static void add_ix1_9(alpha8201_state *cpustate)	 { cpustate->IX1 += 9; }
static void add_ix1_a(alpha8201_state *cpustate)	 { cpustate->IX1 += 10; }
static void add_ix1_b(alpha8201_state *cpustate)	 { cpustate->IX1 += 11; }
static void add_ix1_c(alpha8201_state *cpustate)	 { cpustate->IX1 += 12; }
static void add_ix1_d(alpha8201_state *cpustate)	 { cpustate->IX1 += 13; }
static void add_ix1_e(alpha8201_state *cpustate)	 { cpustate->IX1 += 14; }
static void add_ix1_f(alpha8201_state *cpustate)	 { cpustate->IX1 += 15; }

static void add_ix2_0(alpha8201_state *cpustate)	 { }
static void add_ix2_1(alpha8201_state *cpustate)	 { cpustate->IX2 += 1; }
static void add_ix2_2(alpha8201_state *cpustate)	 { cpustate->IX2 += 2; }
static void add_ix2_3(alpha8201_state *cpustate)	 { cpustate->IX2 += 3; }
static void add_ix2_4(alpha8201_state *cpustate)	 { cpustate->IX2 += 4; }
static void add_ix2_5(alpha8201_state *cpustate)	 { cpustate->IX2 += 5; }
static void add_ix2_6(alpha8201_state *cpustate)	 { cpustate->IX2 += 6; }
static void add_ix2_7(alpha8201_state *cpustate)	 { cpustate->IX2 += 7; }
static void add_ix2_8(alpha8201_state *cpustate)	 { cpustate->IX2 += 8; }
static void add_ix2_9(alpha8201_state *cpustate)	 { cpustate->IX2 += 9; }
static void add_ix2_a(alpha8201_state *cpustate)	 { cpustate->IX2 += 10; }
static void add_ix2_b(alpha8201_state *cpustate)	 { cpustate->IX2 += 11; }
static void add_ix2_c(alpha8201_state *cpustate)	 { cpustate->IX2 += 12; }
static void add_ix2_d(alpha8201_state *cpustate)	 { cpustate->IX2 += 13; }
static void add_ix2_e(alpha8201_state *cpustate)	 { cpustate->IX2 += 14; }
static void add_ix2_f(alpha8201_state *cpustate)	 { cpustate->IX2 += 15; }

static void ld_base_0(alpha8201_state *cpustate)	 { cpustate->regPtr = 0; }
static void ld_base_1(alpha8201_state *cpustate)	 { cpustate->regPtr = 1; }
static void ld_base_2(alpha8201_state *cpustate)	 { cpustate->regPtr = 2; }
static void ld_base_3(alpha8201_state *cpustate)	 { cpustate->regPtr = 3; }
static void ld_base_4(alpha8201_state *cpustate)	 { cpustate->regPtr = 4; }
static void ld_base_5(alpha8201_state *cpustate)	 { cpustate->regPtr = 5; }
static void ld_base_6(alpha8201_state *cpustate)	 { cpustate->regPtr = 6; }
static void ld_base_7(alpha8201_state *cpustate)	 { cpustate->regPtr = 7; }

static void ld_bank_0(alpha8201_state *cpustate)	 { cpustate->mb = 0; }
static void ld_bank_1(alpha8201_state *cpustate)	 { cpustate->mb = 1; }
static void ld_bank_2(alpha8201_state *cpustate)	 { cpustate->mb = 2; }
static void ld_bank_3(alpha8201_state *cpustate)	 { cpustate->mb = 3; }

static void stop(alpha8201_state *cpustate)
{
	UINT8 pcptr = M_RDMEM(0x001) & 0x1f;
	M_WRMEM(pcptr,(M_RDMEM(pcptr)&0xf)+0x08); /* mark entry point ODD to HALT */
	cpustate->mb |= 0x08;        /* mark internal HALT state */
}

static void ld_ix0_n(alpha8201_state *cpustate)	 { cpustate->IX0 = M_RDMEM_OPCODE(cpustate); }
static void ld_ix1_n(alpha8201_state *cpustate)	 { cpustate->IX1 = M_RDMEM_OPCODE(cpustate); }
static void ld_ix2_n(alpha8201_state *cpustate)	 { cpustate->IX2 = M_RDMEM_OPCODE(cpustate); }
static void ld_lp0_n(alpha8201_state *cpustate)	 { cpustate->LP0 = M_RDMEM_OPCODE(cpustate); }
static void ld_lp1_n(alpha8201_state *cpustate)	 { cpustate->LP1 = M_RDMEM_OPCODE(cpustate); }
static void ld_lp2_n(alpha8201_state *cpustate)	 { cpustate->LP2 = M_RDMEM_OPCODE(cpustate); }
static void ld_b_n(alpha8201_state *cpustate)	 { cpustate->B = M_RDMEM_OPCODE(cpustate); }

static void djnz_lp0(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); cpustate->LP0--; if (cpustate->LP0 != 0) M_JMP(cpustate, i); }
static void djnz_lp1(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); cpustate->LP1--; if (cpustate->LP1 != 0) M_JMP(cpustate, i); }
static void djnz_lp2(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); cpustate->LP2--; if (cpustate->LP2 != 0) M_JMP(cpustate, i); }
static void jnz(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); if (!cpustate->zf) M_JMP(cpustate, i); }
static void jnc(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); if (!cpustate->cf) M_JMP(cpustate, i);}
static void jz(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); if ( cpustate->zf) M_JMP(cpustate, i); }
static void jc(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate); if ( cpustate->cf) M_JMP(cpustate, i);}
static void jmp(alpha8201_state *cpustate)	{ M_JMP(cpustate,  M_RDMEM_OPCODE(cpustate) ); }

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


/* ALPHA 8301 : added instruction */
static void exg_a_ix0(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->IX0; cpustate->IX0 = t; }
static void exg_a_ix1(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->IX1; cpustate->IX1 = t; }
static void exg_a_ix2(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->IX2; cpustate->IX2 = t; }
static void exg_a_lp0(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->LP0; cpustate->LP0 = t; }
static void exg_a_lp1(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->LP1; cpustate->LP1 = t; }
static void exg_a_lp2(alpha8201_state *cpustate)  { UINT8 t=cpustate->A; cpustate->A = cpustate->LP2; cpustate->LP2 = t; }
static void exg_a_b(alpha8201_state *cpustate)    { UINT8 t=cpustate->A; cpustate->A = cpustate->B; cpustate->B = t; }
static void exg_a_rb(alpha8201_state *cpustate)   { UINT8 t=cpustate->A; cpustate->A = cpustate->regPtr; cpustate->regPtr = t; }

static void ld_ix0_a(alpha8201_state *cpustate)    { cpustate->IX0 = cpustate->A; }
static void ld_ix1_a(alpha8201_state *cpustate)    { cpustate->IX1 = cpustate->A; }
static void ld_ix2_a(alpha8201_state *cpustate)    { cpustate->IX2 = cpustate->A; }
static void ld_lp0_a(alpha8201_state *cpustate)    { cpustate->LP0 = cpustate->A; }
static void ld_lp1_a(alpha8201_state *cpustate)    { cpustate->LP1 = cpustate->A; }
static void ld_lp2_a(alpha8201_state *cpustate)    { cpustate->LP2 = cpustate->A; }
static void ld_b_a(alpha8201_state *cpustate)      { cpustate->B = cpustate->A; }
static void ld_rb_a(alpha8201_state *cpustate)     { cpustate->regPtr = cpustate->A; }

static void exg_ix0_ix1(alpha8201_state *cpustate)  { UINT8 t=cpustate->IX1; cpustate->IX1 = cpustate->IX0; cpustate->IX0 = t; }
static void exg_ix0_ix2(alpha8201_state *cpustate)  { UINT8 t=cpustate->IX2; cpustate->IX2 = cpustate->IX0; cpustate->IX0 = t; }

static void op_d4(alpha8201_state *cpustate) { cpustate->A = M_RDMEM( ((cpustate->RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(cpustate) ); }
static void op_d5(alpha8201_state *cpustate) { M_WRMEM( ((cpustate->RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(cpustate), cpustate->A ); }
static void op_d6(alpha8201_state *cpustate) { cpustate->LP0 = M_RDMEM( ((cpustate->RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(cpustate) ); }
static void op_d7(alpha8201_state *cpustate) { M_WRMEM( ((cpustate->RAM[(7<<3)+7] & 3) << 8) | M_RDMEM_OPCODE(cpustate), cpustate->LP0 ); }

static void ld_a_abs(alpha8201_state *cpustate) { cpustate->A = M_RDMEM( ((cpustate->mb & 3) << 8) | M_RDMEM_OPCODE(cpustate) ); }
static void ld_abs_a(alpha8201_state *cpustate) { M_WRMEM( ((cpustate->mb & 3) << 8) | M_RDMEM_OPCODE(cpustate), cpustate->A ); }

static void ld_a_r(alpha8201_state *cpustate) { cpustate->A = cpustate->RAM[(M_RDMEM_OPCODE(cpustate)>>1)&0x3f]; }
static void ld_r_a(alpha8201_state *cpustate) { cpustate->RAM[(M_RDMEM_OPCODE(cpustate)>>1)&0x3f] = cpustate->A; }
static void op_rep_ld_ix2_b(alpha8201_state *cpustate) { do { M_WRMEM(cpustate->BIX2, cpustate->RAM[(cpustate->B>>1)&0x3f]); cpustate->IX2++; cpustate->B+=2; cpustate->LP0--; } while (cpustate->LP0 != 0); }
static void op_rep_ld_b_ix0(alpha8201_state *cpustate) { do { cpustate->RAM[(cpustate->B>>1)&0x3f] = M_RDMEM(cpustate->BIX0); cpustate->IX0++; cpustate->B+=2; cpustate->LP0--; } while (cpustate->LP0 != 0); }
static void ld_rxb_a(alpha8201_state *cpustate) { cpustate->RAM[(cpustate->B>>1)&0x3f] = cpustate->A; }
static void ld_a_rxb(alpha8201_state *cpustate) { cpustate->A = cpustate->RAM[(cpustate->B>>1)&0x3f]; }
static void cmp_a_rxb(alpha8201_state *cpustate) { UINT8 i=cpustate->RAM[(cpustate->B>>1)&0x3f];  cpustate->zf = (cpustate->A==i); cpustate->cf = (cpustate->A>=i); }
static void xor_a_rxb(alpha8201_state *cpustate) { M_XOR(cpustate, cpustate->RAM[(cpustate->B>>1)&0x3f] ); }

static void add_a_cf(alpha8201_state *cpustate) { if (cpustate->cf) inc_a(cpustate); }
static void sub_a_cf(alpha8201_state *cpustate) { if (cpustate->cf) dec_a(cpustate); }
static void tst_a(alpha8201_state *cpustate)	 { cpustate->zf = (cpustate->A==0); }
static void clr_a(alpha8201_state *cpustate)	 { cpustate->A = 0; cpustate->zf = (cpustate->A==0); }
static void cmp_a_n(alpha8201_state *cpustate)	{ UINT8 i=M_RDMEM_OPCODE(cpustate);  cpustate->zf = (cpustate->A==i); cpustate->cf = (cpustate->A>=i); }
static void xor_a_n(alpha8201_state *cpustate)	{ M_XOR(cpustate, M_RDMEM_OPCODE(cpustate) ); }
static void call(alpha8201_state *cpustate) { UINT8 i=M_RDMEM_OPCODE(cpustate); cpustate->retptr.w.l = cpustate->PC; M_JMP(cpustate, i); };
static void ld_a_ix0_a(alpha8201_state *cpustate) { cpustate->A = M_RDMEM(cpustate->BIX0+cpustate->A); }
static void ret(alpha8201_state *cpustate) { cpustate->mb = cpustate->retptr.b.h; M_JMP(cpustate,  cpustate->retptr.b.l ); };
static void save_zc(alpha8201_state *cpustate) { cpustate->savez = cpustate->zf; cpustate->savec = cpustate->cf; };
static void rest_zc(alpha8201_state *cpustate) { cpustate->zf = cpustate->savez; cpustate->cf = cpustate->savec; };

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

/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
static CPU_INIT( alpha8201 )
{
	alpha8201_state *cpustate = get_safe_token(device);

	cpustate->device = device;
	cpustate->program = device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();

	device->save_item(NAME(cpustate->RAM));
	device->save_item(NAME(cpustate->PREVPC));
	device->save_item(NAME(cpustate->PC));
	device->save_item(NAME(cpustate->regPtr));
	device->save_item(NAME(cpustate->zf));
	device->save_item(NAME(cpustate->cf));
	device->save_item(NAME(cpustate->mb));
#if HANDLE_HALT_LINE
	device->save_item(NAME(cpustate->halt));
#endif
	device->save_item(NAME(cpustate->IX0));
	device->save_item(NAME(cpustate->IX1));
	device->save_item(NAME(cpustate->IX2));
	device->save_item(NAME(cpustate->LP0));
	device->save_item(NAME(cpustate->LP1));
	device->save_item(NAME(cpustate->LP2));
	device->save_item(NAME(cpustate->A));
	device->save_item(NAME(cpustate->B));
	device->save_item(NAME(cpustate->retptr));
	device->save_item(NAME(cpustate->savec));
	device->save_item(NAME(cpustate->savez));
}
/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
static CPU_RESET( alpha8201 )
{
	alpha8201_state *cpustate = get_safe_token(device);
	cpustate->PC     = 0;
	cpustate->regPtr = 0;
	cpustate->zf     = 0;
	cpustate->cf     = 0;
	cpustate->mb   = 0;
	cpustate->BIX0   = 0;
	cpustate->BIX1   = 0;
	cpustate->BIX2   = 0;
	cpustate->LP0    = 0;
	cpustate->LP1    = 0;
	cpustate->LP2    = 0;
	cpustate->A    = 0;
	cpustate->B   = 0;
#if HANDLE_HALT_LINE
	cpustate->halt = 0;
#endif
}

/****************************************************************************
 * Shut down CPU emulation
 ****************************************************************************/
static CPU_EXIT( alpha8201 )
{
	/* nothing to do ? */
}

/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/

static void alpha8xxx_execute(device_t *device,const s_opcode *op_map)
{
	alpha8201_state *cpustate = get_safe_token(device);
	unsigned opcode;
	UINT8 pcptr;

#if HANDLE_HALT_LINE
	if(cpustate->halt)
	{
		cpustate->icount = 0;
		return;
	}
#endif

	/* setup address bank & fall safe */
	cpustate->ix0.b.h =
	cpustate->ix1.b.h =
	cpustate->ix2.b.h = (cpustate->pc.b.h &= 3);

	/* reset start hack */
	if(cpustate->PC<0x20)
		cpustate->mb |= 0x08;

	do
	{
		if(cpustate->mb & 0x08)
		{
			pcptr = M_RDMEM(0x001) & 0x1f; /* pointer of entry point */
			cpustate->icount -= C1;

			/* entry point scan phase */
			if( (pcptr&1) == 0)
			{
				/* EVEN , get cpustate->PC low */
				cpustate->pc.b.l = M_RDMEM(pcptr);
//mame_printf_debug("alpha8201 load PCL ENTRY=%02X PCL=%02X\n",pcptr, cpustate->pc.b.l);
				cpustate->icount -= C1;
				M_WRMEM(0x001,pcptr+1);
				continue;
			}

			/* ODD , check HALT flag */
			cpustate->mb   = M_RDMEM(pcptr) & (0x08|0x03);
			cpustate->icount -= C1;

			/* not entryaddress 000,001 */
			if(pcptr<2) cpustate->mb |= 0x08;

			if(cpustate->mb & 0x08)
			{
				/* HALTED current entry point . next one */
				pcptr = (pcptr+1)&0x1f;
				M_WRMEM(0x001,pcptr);
				cpustate->icount -= C1;
				continue;
			}

			/* goto run phase */
			M_JMP(cpustate, cpustate->pc.b.l);

#if SHOW_ENTRY_POINT
logerror("alpha8201 START ENTRY=%02X cpustate->PC=%03X\n",pcptr,cpustate->PC);
mame_printf_debug("alpha8201 START ENTRY=%02X cpustate->PC=%03X\n",pcptr,cpustate->PC);
#endif
		}

		/* run */
		cpustate->PREVPC = cpustate->PC;
		debugger_instruction_hook(device, cpustate->PC);
		opcode =M_RDOP(cpustate->PC);
#if TRACE_PC
mame_printf_debug("alpha8201:  cpustate->PC = %03x,  opcode = %02x\n", cpustate->PC, opcode);
#endif
		cpustate->PCL++;
		cpustate->inst_cycles = op_map[opcode].cycles;
		(*(op_map[opcode].function))(cpustate);
		cpustate->icount -= cpustate->inst_cycles;
	} while (cpustate->icount>0);
}

static CPU_EXECUTE( alpha8201 ) { alpha8xxx_execute(device,opcode_8201); }

static CPU_EXECUTE( ALPHA8301 ) { alpha8xxx_execute(device,opcode_8301); }

/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
#if HANDLE_HALT_LINE
static void set_irq_line(alpha8201_state *cpustate, int irqline, int state)
{
	if(irqline == INPUT_LINE_HALT)
	{
		cpustate->halt = (state==ASSERT_LINE) ? 1 : 0;
/* mame_printf_debug("alpha8201 HALT %d\n",cpustate->halt); */
	}
}
#endif

/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( alpha8201 )
{
	alpha8201_state *cpustate = get_safe_token(device);
	switch (state)
	{
#if HANDLE_HALT_LINE
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_HALT:	set_irq_line(cpustate, INPUT_LINE_HALT, info->i);	break;
#endif
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ALPHA8201_PC:			cpustate->PC = info->i;						break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ALPHA8201_SP:			M_WRMEM(0x001,info->i);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_RB:			cpustate->regPtr = info->i & 7;				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_MB:			cpustate->mb = info->i & 0x03;				break;
#if 0
		case CPUINFO_INT_REGISTER + ALPHA8201_ZF:			cpustate->zf= info->i & 0x01;				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_CF:			cpustate->cf= info->i & 0x01;				break;
#endif
		case CPUINFO_INT_REGISTER + ALPHA8201_IX0:			cpustate->IX0 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX1:			cpustate->IX1 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX2:			cpustate->IX2 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP0:			cpustate->LP0 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP1:			cpustate->LP1 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP2:			cpustate->LP2 = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_A:			cpustate->A = info->i;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_B:			cpustate->B = info->i;						break;
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
	alpha8201_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(alpha8201_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 0;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_LITTLE;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 1;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 16;							break;

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 10;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 8;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 6;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;
#if HANDLE_HALT_LINE
		case CPUINFO_INT_INPUT_STATE + INPUT_LINE_HALT:		info->i = cpustate->halt ? ASSERT_LINE : CLEAR_LINE; break;
#endif
		case CPUINFO_INT_PREVIOUSPC:						info->i = cpustate->PREVPC;					break;
		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + ALPHA8201_PC:			info->i = cpustate->PC & 0x3ff;				break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + ALPHA8201_SP:			info->i = M_RDMEM(0x001);			break;
		case CPUINFO_INT_REGISTER + ALPHA8201_RB:			info->i = cpustate->regPtr;					break;
		case CPUINFO_INT_REGISTER + ALPHA8201_MB:			info->i = cpustate->mb;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_ZF:			info->i = cpustate->zf;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_CF:			info->i = cpustate->cf;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX0:			info->i = cpustate->IX0;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX1:			info->i = cpustate->IX1;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_IX2:			info->i = cpustate->IX2;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP0:			info->i = cpustate->LP0;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP1:			info->i = cpustate->LP1;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_LP2:			info->i = cpustate->LP2;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_A:			info->i = cpustate->A;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_B:			info->i = cpustate->B;						break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R0:			info->i = RD_REG(0);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R1:			info->i = RD_REG(1);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R2:			info->i = RD_REG(2);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R3:			info->i = RD_REG(3);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R4:			info->i = RD_REG(4);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R5:			info->i = RD_REG(5);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R6:			info->i = RD_REG(6);				break;
		case CPUINFO_INT_REGISTER + ALPHA8201_R7:			info->i = RD_REG(7);				break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(alpha8201);		break;
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(alpha8201);			break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(alpha8201);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(alpha8201);			break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(alpha8201);		break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "AlphaDenshi MCU");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "0.1");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Tatsuyuki Satoh"); break;
		case CPUINFO_STR_FLAGS:							sprintf(info->s, "%c%c", cpustate->cf?'C':'.',cpustate->zf?'Z':'.'); break;
		case CPUINFO_STR_REGISTER + ALPHA8201_PC:		sprintf(info->s, "cpustate->PC:%03X", cpustate->PC);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_SP:		sprintf(info->s, "SP:%02X", M_RDMEM(0x001) ); break;
		case CPUINFO_STR_REGISTER + ALPHA8201_RB:		sprintf(info->s, "RB:%X", cpustate->regPtr);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_MB:		sprintf(info->s, "MB:%X", cpustate->mb);		break;
#if 0
		case CPUINFO_STR_REGISTER + ALPHA8201_ZF:		sprintf(info->s, "cpustate->zf:%X", cpustate->zf);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_CF:		sprintf(info->s, "cpustate->cf:%X", cpustate->cf);		break;
#endif
		case CPUINFO_STR_REGISTER + ALPHA8201_IX0:		sprintf(info->s, "cpustate->IX0:%02X", cpustate->IX0);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_IX1:		sprintf(info->s, "cpustate->IX1:%02X", cpustate->IX1);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_IX2:		sprintf(info->s, "cpustate->IX2:%02X", cpustate->IX2);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP0:		sprintf(info->s, "cpustate->LP0:%02X", cpustate->LP0);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP1:		sprintf(info->s, "cpustate->LP1:%02X", cpustate->LP1);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_LP2:		sprintf(info->s, "cpustate->LP2:%02X", cpustate->LP2);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_A:		sprintf(info->s, "A:%02X", cpustate->A);		break;
		case CPUINFO_STR_REGISTER + ALPHA8201_B:		sprintf(info->s, "B:%02X", cpustate->B);		break;
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
CPU_GET_INFO( alpha8201 )
{
	switch (state)
	{
	case CPUINFO_STR_NAME:							strcpy(info->s, "ALPHA-8201");				break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(alpha8201);			break;
	default:
		/* 8201 / 8301 */
		CPU_GET_INFO_CALL(alpha8xxx);
	}
}

CPU_GET_INFO( alpha8301 )
{
	switch (state)
	{
	case CPUINFO_STR_NAME:							strcpy(info->s, "ALPHA-8301");				break;
	case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(ALPHA8301);			break;
	default:
		/* 8201 / 8301 */
		CPU_GET_INFO_CALL(alpha8xxx);
	}
}

DEFINE_LEGACY_CPU_DEVICE(ALPHA8201, alpha8201);
DEFINE_LEGACY_CPU_DEVICE(ALPHA8301, alpha8301);
