// license:BSD-3-Clause
// copyright-holders:Tatsuyuki Satoh
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


const device_type ALPHA8201L = &device_creator<alpha8201_cpu_device>;
const device_type ALPHA8301L = &device_creator<alpha8301_cpu_device>;


/* instruction cycle count */
#define C1 16
#define C2 32

/* debug option */
#define TRACE_PC 0
#define SHOW_ENTRY_POINT 0
#define SHOW_MESSAGE_CONSOLE 0
#define BREAK_ON_UNKNOWN_OPCODE 0
#define BREAK_ON_UNCERTAIN_OPCODE 0


#define FN(x) &alpha8201_cpu_device::x


alpha8201_cpu_device::alpha8201_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: cpu_device(mconfig, ALPHA8201L, "ALPHA-8201L", tag, owner, clock, "alpha8201l", __FILE__)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 10, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 6, 0)
	, m_opmap(opcode_8201)
{
}


alpha8201_cpu_device::alpha8201_cpu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 10, 0)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 6, 0)
	, m_opmap(opcode_8201)
{
}

alpha8301_cpu_device::alpha8301_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: alpha8201_cpu_device(mconfig, ALPHA8301L, "ALPHA-8301L", tag, owner, clock, "alpha8301l", __FILE__)
{
	m_opmap = opcode_8301;
}


/* Get next opcode argument and increment program counter */
unsigned alpha8201_cpu_device::M_RDMEM_OPCODE()
{
	unsigned retval;
	retval=M_RDOP_ARG(m_pc.w.l);
	m_pc.b.l++;
	return retval;
}

void alpha8201_cpu_device::M_ADD(UINT8 dat)
{
	UINT16 temp = m_A + dat;
	m_A = temp & 0xff;
	m_zf = (m_A==0);
	m_cf = temp>>8;
}

void alpha8201_cpu_device::M_ADDB(UINT8 dat)
{
	UINT16 temp = m_B + dat;
	m_B = temp & 0xff;
	m_zf = (m_B==0);
	m_cf = temp>>8;
}

void alpha8201_cpu_device::M_SUB(UINT8 dat)
{
	m_cf = (m_A>=dat);  // m_cf is No Borrow
	m_A -= dat;
	m_zf = (m_A==0);
}

void alpha8201_cpu_device::M_AND(UINT8 dat)
{
	m_A &= dat;
	m_zf = (m_A==0);
}

void alpha8201_cpu_device::M_OR(UINT8 dat)
{
	m_A |= dat;
	m_zf = (m_A==0);
}

void alpha8201_cpu_device::M_XOR(UINT8 dat)
{
	m_A ^= dat;
	m_zf = (m_A==0);
	m_cf = 0;
}

void alpha8201_cpu_device::M_JMP(UINT8 dat)
{
	m_pc.b.l = dat;
	/* update pc page */
	m_pc.b.h  = m_ix0.b.h = m_ix1.b.h = m_ix2.b.h = m_mb & 3;
}

void alpha8201_cpu_device::M_UNDEFINED()
{
	logerror("alpha8201:  PC = %03x,  Unimplemented opcode = %02x\n", m_pc.w.l-1, M_RDMEM(m_pc.w.l-1));
#if SHOW_MESSAGE_CONSOLE
	osd_printf_debug("alpha8201:  PC = %03x,  Unimplemented opcode = %02x\n", m_pc.w.l-1, M_RDMEM(m_pc.w.l-1));
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(machine());
#endif
}

void alpha8201_cpu_device::M_UNDEFINED2()
{
	UINT8 op  = M_RDOP(m_pc.w.l-1);
	UINT8 imm = M_RDMEM_OPCODE();
	logerror("alpha8201:  PC = %03x,  Unimplemented opcode = %02x,%02x\n", m_pc.w.l-2, op,imm);
#if SHOW_MESSAGE_CONSOLE
	osd_printf_debug("alpha8201:  PC = %03x,  Unimplemented opcode = %02x,%02x\n", m_pc.w.l-2, op,imm);
#endif
#if BREAK_ON_UNKNOWN_OPCODE
	debugger_break(machine());
#endif
}


void alpha8201_cpu_device::stop()
{
	UINT8 pcptr = M_RDMEM(0x001) & 0x1f;
	M_WRMEM(pcptr,(M_RDMEM(pcptr)&0xf)+0x08); /* mark entry point ODD to HALT */
	m_mb |= 0x08;        /* mark internal HALT state */
}


const alpha8201_cpu_device::s_opcode alpha8201_cpu_device::opcode_8201[256]=
{
	{C1, FN(nop)        },{C1,FN(rora)      },{C1, FN(rola)      },{C1,FN(inc_b)     },{C1,FN(dec_b)     },{C1, FN(inc_a)    },{C1, FN(dec_a)    },{C1, FN(cpl)      },
	{C2,FN(ld_a_ix0_0)  },{C2,FN(ld_a_ix0_1)},{C2, FN(ld_a_ix0_2)},{C2,FN(ld_a_ix0_3)},{C2,FN(ld_a_ix0_4)},{C2,FN(ld_a_ix0_5)},{C2,FN(ld_a_ix0_6)},{C2,FN(ld_a_ix0_7)},
	{C2,FN(ld_a_ix1_0)  },{C2,FN(ld_a_ix1_1)},{C2, FN(ld_a_ix1_2)},{C2,FN(ld_a_ix1_3)},{C2,FN(ld_a_ix1_4)},{C2,FN(ld_a_ix1_5)},{C2,FN(ld_a_ix1_6)},{C2,FN(ld_a_ix1_7)},
	{C2,FN(ld_ix2_0_a)  },{C2,FN(ld_ix2_1_a)},{C2, FN(ld_ix2_2_a)},{C2,FN(ld_ix2_3_a)},{C2,FN(ld_ix2_4_a)},{C2,FN(ld_ix2_5_a)},{C2,FN(ld_ix2_6_a)},{C2,FN(ld_ix2_7_a)},
/* 20 */
	{C2,FN(ld_ix0_0_b)  },{C2,FN(ld_ix0_1_b)},{C2, FN(ld_ix0_2_b)},{C2,FN(ld_ix0_3_b)},{C2,FN(ld_ix0_4_b)},{C2,FN(ld_ix0_5_b)},{C2,FN(ld_ix0_6_b)},{C2,FN(ld_ix0_7_b)},
	{C2,FN(undefined)   },{C2,FN(undefined) },{C2, FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },
	{C2,FN(undefined)   },{C2,FN(undefined) },{C2, FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },
	{C2,FN(bit_r0_0)    },{C2,FN(bit_r0_1)  },{C2, FN(bit_r0_2) },{C2, FN(bit_r0_3) },{C2, FN(bit_r0_4) },{C2, FN(bit_r0_5) },{C2, FN(bit_r0_6) },{C2, FN(bit_r0_7) },
/* 40 : 8201 */
	{C2, FN(ld_a_r0)    },{C2, FN(ld_r0_a)  },{C2, FN(ld_a_r1)  },{C2, FN(ld_r1_a)  },{C2, FN(ld_a_r2)  },{C2, FN(ld_r2_a)  },{C2, FN(ld_a_r3)  },{C2, FN(ld_r3_a)  },
	{C2, FN(ld_a_r4)    },{C2, FN(ld_r4_a)  },{C2, FN(ld_a_r5)  },{C2, FN(ld_r5_a)  },{C2, FN(ld_a_r6)  },{C2, FN(ld_r6_a)  },{C2, FN(ld_a_r7)  },{C2, FN(ld_r7_a)  },
	{C1, FN(add_a_r0)   },{C1, FN(sub_a_r0) },{C1, FN(add_a_r1) },{C1, FN(sub_a_r1) },{C1, FN(add_a_r2) },{C1, FN(sub_a_r2) },{C1, FN(add_a_r3) },{C1, FN(sub_a_r3) },
	{C1, FN(add_a_r4)   },{C1, FN(sub_a_r4) },{C1, FN(add_a_r5) },{C1, FN(sub_a_r5) },{C1, FN(add_a_r6) },{C1, FN(sub_a_r6) },{C1, FN(add_a_r7) },{C1, FN(sub_a_r7) },
	{C1, FN(and_a_r0)   },{C1, FN(or_a_r0)  },{C1, FN(and_a_r1) },{C1, FN(or_a_r1)  },{C1, FN(and_a_r2) },{C1, FN(or_a_r2)  },{C1, FN(and_a_r3) },{C1, FN(or_a_r3)  },
	{C1, FN(and_a_r4)   },{C1, FN(or_a_r4)  },{C1, FN(and_a_r5) },{C1, FN(or_a_r5)  },{C1, FN(and_a_r6) },{C1, FN(or_a_r6)  },{C1, FN(and_a_r7) },{C1, FN(or_a_r7)  },
	{C1, FN(add_ix0_0)  },{C1, FN(add_ix0_1)},{C1, FN(add_ix0_2)},{C1, FN(add_ix0_3)},{C1, FN(add_ix0_4)},{C1, FN(add_ix0_5)},{C1, FN(add_ix0_6)},{C1, FN(add_ix0_7)},
	{C1, FN(add_ix0_8)  },{C1, FN(add_ix0_9)},{C1, FN(add_ix0_a)},{C1, FN(add_ix0_b)},{C1, FN(add_ix0_c)},{C1, FN(add_ix0_d)},{C1, FN(add_ix0_e)},{C1, FN(add_ix0_f)},
/* 80 : 8201 */
	{C1, FN(add_ix1_0)  },{C1, FN(add_ix1_1)},{C1, FN(add_ix1_2)},{C1, FN(add_ix1_3)},{C1, FN(add_ix1_4)},{C1, FN(add_ix1_5)},{C1, FN(add_ix1_6)},{C1, FN(add_ix1_7)},
	{C1, FN(add_ix1_8)  },{C1, FN(add_ix1_9)},{C1, FN(add_ix1_a)},{C1, FN(add_ix1_b)},{C1, FN(add_ix1_c)},{C1, FN(add_ix1_d)},{C1, FN(add_ix1_e)},{C1, FN(add_ix1_f)},
	{C1, FN(add_ix2_0)  },{C1, FN(add_ix2_1)},{C1, FN(add_ix2_2)},{C1, FN(add_ix2_3)},{C1, FN(add_ix2_4)},{C1, FN(add_ix2_5)},{C1, FN(add_ix2_6)},{C1, FN(add_ix2_7)},
	{C1, FN(add_ix2_8)  },{C1, FN(add_ix2_9)},{C1, FN(add_ix2_a)},{C1, FN(add_ix2_b)},{C1, FN(add_ix2_c)},{C1, FN(add_ix2_d)},{C1, FN(add_ix2_e)},{C1, FN(add_ix2_f)},
	{C1, FN(ld_base_0)  },{C1, FN(ld_base_1)},{C1, FN(ld_base_2)},{C1, FN(ld_base_3)},{C1, FN(ld_base_4)},{C1, FN(ld_base_5)},{C1, FN(ld_base_6)},{C1, FN(ld_base_7)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(ld_bank_0)  },{C1, FN(ld_bank_1)},{C1, FN(ld_bank_2)},{C1, FN(ld_bank_3)},{C2, FN(stop)     },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
/* c0 : 8201 */
	{C2, FN(ld_ix0_n)   },{C2, FN(ld_ix1_n) },{C2, FN(ld_ix2_n) },{C2, FN(ld_a_n)   },{C2, FN(ld_lp0_n) },{C2, FN(ld_lp1_n) },{C2, FN(ld_lp2_n) },{C2, FN(ld_b_n)   },
	{C2, FN(add_a_n)    },{C2, FN(sub_a_n)  },{C2, FN(and_a_n)  },{C2, FN(or_a_n)   },{C2, FN(djnz_lp0) },{C2, FN(djnz_lp1) },{C2, FN(djnz_lp2) },{C2, FN(jnz)      },
	{C2, FN(jnc)            },{C2, FN(jz)       },{C2, FN(jmp)      },{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2, FN(undefined2)},
	{C2, FN(undefined2) },{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2,FN(undefined2)},{C2, FN(undefined2)},
/* E0 : 8201*/
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined) }
};


const alpha8201_cpu_device::s_opcode alpha8201_cpu_device::opcode_8301[256]=
{
	{C1, FN(nop)        },{C1,FN(rora)      },{C1, FN(rola)      },{C1,FN(inc_b)     },{C1,FN(dec_b)     },{C1, FN(inc_a)    },{C1, FN(dec_a)    },{C1, FN(cpl)      },
	{C2,FN(ld_a_ix0_0)  },{C2,FN(ld_a_ix0_1)},{C2, FN(ld_a_ix0_2)},{C2,FN(ld_a_ix0_3)},{C2,FN(ld_a_ix0_4)},{C2,FN(ld_a_ix0_5)},{C2,FN(ld_a_ix0_6)},{C2,FN(ld_a_ix0_7)},
	{C2,FN(ld_a_ix1_0)  },{C2,FN(ld_a_ix1_1)},{C2, FN(ld_a_ix1_2)},{C2,FN(ld_a_ix1_3)},{C2,FN(ld_a_ix1_4)},{C2,FN(ld_a_ix1_5)},{C2,FN(ld_a_ix1_6)},{C2,FN(ld_a_ix1_7)},
	{C2,FN(ld_ix2_0_a)  },{C2,FN(ld_ix2_1_a)},{C2, FN(ld_ix2_2_a)},{C2,FN(ld_ix2_3_a)},{C2,FN(ld_ix2_4_a)},{C2,FN(ld_ix2_5_a)},{C2,FN(ld_ix2_6_a)},{C2,FN(ld_ix2_7_a)},
/* 20 : 8301 */
	{C2,FN(ld_ix0_0_b)  },{C2,FN(ld_ix0_1_b)},{C2, FN(ld_ix0_2_b)},{C2,FN(ld_ix0_3_b)},{C2,FN(ld_ix0_4_b)},{C2,FN(ld_ix0_5_b)},{C2,FN(ld_ix0_6_b)},{C2,FN(ld_ix0_7_b)},
	{C2,FN(undefined)   },{C2,FN(undefined) },{C2, FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },
	{C2,FN(undefined)   },{C2,FN(undefined) },{C2, FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },{C2,FN(undefined) },
	{C2,FN(bit_r0_0)    },{C2,FN(bit_r0_1)  },{C2, FN(bit_r0_2) },{C2, FN(bit_r0_3) },{C2, FN(bit_r0_4) },{C2, FN(bit_r0_5) },{C2, FN(bit_r0_6) },{C2, FN(bit_r0_7) },
/* 40 : 8301 */
	{C2, FN(ld_a_r0)    },{C2, FN(ld_r0_a)  },{C2, FN(ld_a_r1)  },{C2, FN(ld_r1_a)  },{C2, FN(ld_a_r2)  },{C2, FN(ld_r2_a)  },{C2, FN(ld_a_r3)  },{C2, FN(ld_r3_a)  },
	{C2, FN(ld_a_r4)    },{C2, FN(ld_r4_a)  },{C2, FN(ld_a_r5)  },{C2, FN(ld_r5_a)  },{C2, FN(ld_a_r6)  },{C2, FN(ld_r6_a)  },{C2, FN(ld_a_r7)  },{C2, FN(ld_r7_a)  },
	{C1, FN(add_a_r0)   },{C1, FN(sub_a_r0) },{C1, FN(add_a_r1) },{C1, FN(sub_a_r1) },{C1, FN(add_a_r2) },{C1, FN(sub_a_r2) },{C1, FN(add_a_r3) },{C1, FN(sub_a_r3) },
	{C1, FN(add_a_r4)   },{C1, FN(sub_a_r4) },{C1, FN(add_a_r5) },{C1, FN(sub_a_r5) },{C1, FN(add_a_r6) },{C1, FN(sub_a_r6) },{C1, FN(add_a_r7) },{C1, FN(sub_a_r7) },
/* 60 : 8301 */
	{C1, FN(and_a_r0)   },{C1, FN(or_a_r0)  },{C1, FN(and_a_r1) },{C1, FN(or_a_r1)  },{C1, FN(and_a_r2) },{C1, FN(or_a_r2)  },{C1, FN(and_a_r3) },{C1, FN(or_a_r3)  },
	{C1, FN(and_a_r4)   },{C1, FN(or_a_r4)  },{C1, FN(and_a_r5) },{C1, FN(or_a_r5)  },{C1, FN(and_a_r6) },{C1, FN(or_a_r6)  },{C1, FN(and_a_r7) },{C1, FN(or_a_r7)  },
	{C1, FN(add_ix0_0)  },{C1, FN(add_ix0_1)},{C1, FN(add_ix0_2)},{C1, FN(add_ix0_3)},{C1, FN(add_ix0_4)},{C1, FN(add_ix0_5)},{C1, FN(add_ix0_6)},{C1, FN(add_ix0_7)},
	{C1, FN(add_ix0_8)  },{C1, FN(add_ix0_9)},{C1, FN(add_ix0_a)},{C1, FN(add_ix0_b)},{C1, FN(add_ix0_c)},{C1, FN(add_ix0_d)},{C1, FN(add_ix0_e)},{C1, FN(add_ix0_f)},
/* 80 : 8301 */
	{C1, FN(add_ix1_0)  },{C1, FN(add_ix1_1)},{C1, FN(add_ix1_2)},{C1, FN(add_ix1_3)},{C1, FN(add_ix1_4)},{C1, FN(add_ix1_5)},{C1, FN(add_ix1_6)},{C1, FN(add_ix1_7)},
	{C1, FN(add_ix1_8)  },{C1, FN(add_ix1_9)},{C1, FN(add_ix1_a)},{C1, FN(add_ix1_b)},{C1, FN(add_ix1_c)},{C1, FN(add_ix1_d)},{C1, FN(add_ix1_e)},{C1, FN(add_ix1_f)},
	{C1, FN(add_ix2_0)  },{C1, FN(add_ix2_1)},{C1, FN(add_ix2_2)},{C1, FN(add_ix2_3)},{C1, FN(add_ix2_4)},{C1, FN(add_ix2_5)},{C1, FN(add_ix2_6)},{C1, FN(add_ix2_7)},
	{C1, FN(add_ix2_8)  },{C1, FN(add_ix2_9)},{C1, FN(add_ix2_a)},{C1, FN(add_ix2_b)},{C1, FN(add_ix2_c)},{C1, FN(add_ix2_d)},{C1, FN(add_ix2_e)},{C1, FN(add_ix2_f)},
/* A0 : 8301 */
	{C1, FN(ld_base_0)  },{C1, FN(ld_base_1)},{C1, FN(ld_base_2)},{C1, FN(ld_base_3)},{C1, FN(ld_base_4)},{C1, FN(ld_base_5)},{C1, FN(ld_base_6)},{C1, FN(ld_base_7)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(ld_bank_0)  },{C1, FN(ld_bank_1)},{C1, FN(ld_bank_2)},{C1, FN(ld_bank_3)},{C2, FN(stop)     },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
	{C1, FN(undefined)  },{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},{C1, FN(undefined)},
/* c0 : 8301 */
	{C2, FN(ld_ix0_n)   },{C2, FN(ld_ix1_n)},{C2, FN(ld_ix2_n)  },{C2, FN(ld_a_n)   },{C2, FN(ld_lp0_n) },{C2, FN(ld_lp1_n) },{C2, FN(ld_lp2_n) },{C2, FN(ld_b_n)   },
	{C2, FN(add_a_n)    },{C2, FN(sub_a_n)  },{C2, FN(and_a_n)  },{C2, FN(or_a_n)   },{C2, FN(djnz_lp0) },{C2, FN(djnz_lp1) },{C2, FN(djnz_lp2) },{C2, FN(jnz)      },
	{C2, FN(jnc)            },{C2, FN(jz)       },{C2, FN(jmp)      },{C2,FN(undefined2)},{C2, FN(op_d4)    },{C2, FN(op_d5)    },{C2, FN(op_d6)    },{C2, FN(op_d7)    },
	{C2, FN(ld_a_abs)  },{C2, FN(ld_abs_a)},{C2,FN(cmp_a_n) },{C2,FN(xor_a_n)   },{C2, FN(ld_a_r)   },{C2, FN(ld_r_a)   },{C2, FN(jc)       },{C2, FN(call)},
/* E0 : 8301 */
	{C1, FN(exg_a_ix0)  },{C1, FN(exg_a_ix1)},{C1, FN(exg_a_ix2)},{C1, FN(exg_a_lp1)},{C1, FN(exg_a_lp2)},{C1, FN(exg_a_b)  },{C1, FN(exg_a_lp0)},{C1, FN(exg_a_rb) },
	{C1, FN(ld_ix0_a)   },{C1, FN(ld_ix1_a) },{C1, FN(ld_ix2_a) },{C1, FN(ld_lp1_a) },{C1, FN(ld_lp2_a) },{C1, FN(ld_b_a)   },{C1, FN(ld_lp0_a) },{C1, FN(ld_rb_a)  },
	{C1,FN(exg_ix0_ix1)},{C1,FN(exg_ix0_ix2)},{C1,FN(op_rep_ld_ix2_b)},{C1, FN(op_rep_ld_b_ix0)},{C1, FN(save_zc)},{C1, FN(rest_zc)},{C1, FN(ld_rxb_a) },{C1, FN(ld_a_rxb) },
	{C1, FN(cmp_a_rxb) },{C1, FN(xor_a_rxb)},{C1, FN(add_a_cf) },{C1, FN(sub_a_cf) },{C1, FN(tst_a)    },{C1, FN(clr_a)    },{C1, FN(ld_a_ix0_a)},{C1, FN(ret)     }
};


/****************************************************************************
 * Initialize emulation
 ****************************************************************************/
void alpha8201_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();

	state_add( ALPHA8201_PC, "PC", m_pc.w.l ).mask(0x3ff).formatstr("%03X");
	state_add( ALPHA8201_SP, "SP", m_sp ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_RB, "RB", m_regPtr ).mask(0x7);
	state_add( ALPHA8201_MB, "MB", m_mb ).mask(0x3);
	state_add( ALPHA8201_CF, "CF", m_cf ).mask(0x1);
	state_add( ALPHA8201_ZF, "ZF", m_zf ).mask(0x1);
	state_add( ALPHA8201_IX0, "IX0", m_ix0.b.l );
	state_add( ALPHA8201_IX1, "IX1", m_ix1.b.l );
	state_add( ALPHA8201_IX2, "IX2", m_ix2.b.l );
	state_add( ALPHA8201_LP0, "LP0", m_lp0 );
	state_add( ALPHA8201_LP1, "LP1", m_lp1 );
	state_add( ALPHA8201_LP2, "LP2", m_lp2 );
	state_add( ALPHA8201_A, "A", m_A );
	state_add( ALPHA8201_B, "B", m_B );
	state_add( ALPHA8201_R0, "R0", m_R[0] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R1, "R1", m_R[1] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R2, "R2", m_R[2] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R3, "R3", m_R[3] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R4, "R4", m_R[4] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R5, "R5", m_R[5] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R6, "R6", m_R[6] ).callimport().callexport().formatstr("%02X");
	state_add( ALPHA8201_R7, "R7", m_R[7] ).callimport().callexport().formatstr("%02X");

	save_item(NAME(m_RAM));
	save_item(NAME(m_PREVPC));
	save_item(NAME(m_pc.w.l));
	save_item(NAME(m_regPtr));
	save_item(NAME(m_zf));
	save_item(NAME(m_cf));
	save_item(NAME(m_mb));
	save_item(NAME(m_halt));
	save_item(NAME(m_ix0.b.l));
	save_item(NAME(m_ix1.b.l));
	save_item(NAME(m_ix2.b.l));
	save_item(NAME(m_lp0));
	save_item(NAME(m_lp1));
	save_item(NAME(m_lp2));
	save_item(NAME(m_A));
	save_item(NAME(m_B));
	save_item(NAME(m_retptr));
	save_item(NAME(m_savec));
	save_item(NAME(m_savez));

	m_icountptr = &m_icount;
}


void alpha8201_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case ALPHA8201_SP:
			M_WRMEM(0x001, m_sp);
			break;

		case ALPHA8201_R0:
			WR_REG(0, m_R[0]);
			break;

		case ALPHA8201_R1:
			WR_REG(1, m_R[1]);
			break;

		case ALPHA8201_R2:
			WR_REG(2, m_R[2]);
			break;

		case ALPHA8201_R3:
			WR_REG(3, m_R[3]);
			break;

		case ALPHA8201_R4:
			WR_REG(4, m_R[4]);
			break;

		case ALPHA8201_R5:
			WR_REG(5, m_R[5]);
			break;

		case ALPHA8201_R6:
			WR_REG(6, m_R[6]);
			break;

		case ALPHA8201_R7:
			WR_REG(7, m_R[7]);
			break;
	}
}


void alpha8201_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case ALPHA8201_SP:
			m_sp = M_RDMEM(0x001);
			break;

		case ALPHA8201_R0:
			m_R[0] = RD_REG(0);
			break;

		case ALPHA8201_R1:
			m_R[1] = RD_REG(1);
			break;

		case ALPHA8201_R2:
			m_R[2] = RD_REG(2);
			break;

		case ALPHA8201_R3:
			m_R[3] = RD_REG(3);
			break;

		case ALPHA8201_R4:
			m_R[4] = RD_REG(4);
			break;

		case ALPHA8201_R5:
			m_R[5] = RD_REG(5);
			break;

		case ALPHA8201_R6:
			m_R[6] = RD_REG(6);
			break;

		case ALPHA8201_R7:
			m_R[7] = RD_REG(7);
			break;
	}
}


void alpha8201_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c", m_cf ? 'C' : '.', m_zf ? 'Z' : '.');
			break;
	}
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
void alpha8201_cpu_device::device_reset()
{
	m_pc.w.l = 0;
	m_regPtr = 0;
	m_zf     = 0;
	m_cf     = 0;
	m_mb   = 0;
	m_ix0.w.l = 0;
	m_ix1.w.l = 0;
	m_ix2.w.l = 0;
	m_lp0  = 0;
	m_lp1  = 0;
	m_lp2  = 0;
	m_A    = 0;
	m_B   = 0;
	m_halt = 0;
}


/****************************************************************************
 * Execute cycles CPU cycles. Return number of cycles really executed
 ****************************************************************************/

void alpha8201_cpu_device::execute_run()
{
	unsigned opcode;
	UINT8 pcptr;

	if(m_halt)
	{
		m_icount = 0;
		return;
	}

	/* setup address bank & fall safe */
	m_ix0.b.h =
	m_ix1.b.h =
	m_ix2.b.h = (m_pc.b.h &= 3);

	/* reset start hack */
	if(m_pc.w.l<0x20)
		m_mb |= 0x08;

	do
	{
		if(m_mb & 0x08)
		{
			pcptr = M_RDMEM(0x001) & 0x1f; /* pointer of entry point */
			m_icount -= C1;

			/* entry point scan phase */
			if( (pcptr&1) == 0)
			{
				/* EVEN , get PC low */
				m_pc.b.l = M_RDMEM(pcptr);
//osd_printf_debug("alpha8201 load PCL ENTRY=%02X PCL=%02X\n",pcptr, m_pc.b.l);
				m_icount -= C1;
				M_WRMEM(0x001,pcptr+1);
				continue;
			}

			/* ODD , check HALT flag */
			m_mb   = M_RDMEM(pcptr) & (0x08|0x03);
			m_icount -= C1;

			/* not entryaddress 000,001 */
			if(pcptr<2) m_mb |= 0x08;

			if(m_mb & 0x08)
			{
				/* HALTED current entry point . next one */
				pcptr = (pcptr+1)&0x1f;
				M_WRMEM(0x001,pcptr);
				m_icount -= C1;
				continue;
			}

			/* goto run phase */
			M_JMP(m_pc.b.l);

#if SHOW_ENTRY_POINT
logerror("alpha8201 START ENTRY=%02X PC=%03X\n",pcptr,m_pc.w.l);
osd_printf_debug("alpha8201 START ENTRY=%02X PC=%03X\n",pcptr,m_pc.w.l);
#endif
		}

		/* run */
		m_PREVPC = m_pc.w.l;
		debugger_instruction_hook(this, m_pc.w.l);
		opcode =M_RDOP(m_pc.w.l);
#if TRACE_PC
osd_printf_debug("alpha8201:  PC = %03x,  opcode = %02x\n", m_pc.w.l, opcode);
#endif
		m_pc.b.l++;
		m_inst_cycles = m_opmap[opcode].cycles;
		(this->*m_opmap[opcode].opcode_func)();
		m_icount -= m_inst_cycles;
	} while (m_icount>0);
}


/****************************************************************************
 * Set IRQ line state
 ****************************************************************************/
void alpha8201_cpu_device::execute_set_input(int inputnum, int state)
{
	if(inputnum == INPUT_LINE_HALT)
	{
		m_halt = (state==ASSERT_LINE) ? 1 : 0;
/* osd_printf_debug("alpha8201 HALT %d\n",m_halt); */
	}
}


offs_t alpha8201_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( alpha8201 );
	return CPU_DISASSEMBLE_NAME(alpha8201)(this, buffer, pc, oprom, opram, options);
}
