// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************
 Hyperstone cpu emulator
 written by Pierpaolo Prazzoli

 All the types are compatible, but they have different IRAM size and cycles

 Hyperstone models:

 16 bits
 - E1-16T
 - E1-16XT
 - E1-16XS
 - E1-16XSR

 32bits
 - E1-32N   or  E1-32T
 - E1-32XN  or  E1-32XT
 - E1-32XS
 - E1-32XSR

 Hynix models:

 16 bits
 - GMS30C2116
 - GMS30C2216

 32bits
 - GMS30C2132
 - GMS30C2232

 TODO:
 - some wrong cycle counts

 CHANGELOG:

 Pierpaolo Prazzoli
 - Fixed LDxx.N/P/S opcodes not to increment the destination register when
   it's the same as the source or "next source" one.

 Pierpaolo Prazzoli
 - Removed nested delays
 - Added better delay branch support
 - Fixed PC seen by a delay instruction, because a delay instruction
   should use the delayed PC (thus allowing the execution of software
   opcodes too)

 Tomasz Slanina
 - Fixed delayed branching for delay instructions longer than 2 bytes

 Pierpaolo Prazzoli
 - Added and fixed Timer without hack

 Tomasz Slanina
 - Fixed MULU/MULS
 - Fixed Carry in ADDC/SUBC

 Pierpaolo Prazzoli
 - Fixed software opcodes used as delay instructions
 - Added nested delays

 Tomasz Slanina
 - Added "undefined" C flag to shift left instructions

 Pierpaolo Prazzoli
 - Added interrupts-block for delay instructions
 - Fixed get_emu_code_addr
 - Added LDW.S and STW.S instructions
 - Fixed floating point opcodes

 Tomasz Slanina
 - interrputs after call and before frame are prohibited now
 - emulation of FCR register
 - Floating point opcodes (preliminary)
 - Fixed stack addressing in RET/FRAME opcodes
 - Fixed bug in SET_RS macro
 - Fixed bug in return opcode (S flag)
 - Added C/N flags calculation in add/adc/addi/adds/addsi and some shift opcodes
 - Added writeback to ROL
 - Fixed ROL/SAR/SARD/SHR/SHRD/SHL/SHLD opcode decoding (Local/Global regs)
 - Fixed I and T flag in RET opcode
 - Fixed XX/XM opcodes
 - Fixed MOV opcode, when RD = PC
 - Fixed execute_trap()
 - Fixed ST opcodes, when when RS = SR
 - Added interrupts
 - Fixed I/O addressing

 Pierpaolo Prazzoli
 - Fixed fetch
 - Fixed decode of hyperstone_xm opcode
 - Fixed 7 bits difference number in FRAME / RET instructions
 - Some debbugger fixes
 - Added generic registers decode function
 - Some other little fixes.

 Ryan Holtz 29/03/2004
    - Changed MOVI to use unsigned values instead of signed, correcting
      an ugly glitch when loading 32-bit immediates.
 Pierpaolo Prazzoli
    - Same fix in get_const

 Ryan Holtz - 02/27/04
    - Fixed delayed branching
    - const_val for CALL should always have bit 0 clear

 Pierpaolo Prazzoli - 02/25/04
    - Fixed some wrong addresses to address local registers instead of memory
    - Fixed FRAME and RET instruction
    - Added preliminary I/O space
    - Fixed some load / store instructions

 Pierpaolo Prazzoli - 02/20/04
    - Added execute_exception function
    - Added FL == 0 always interpreted as 16

 Pierpaolo Prazzoli - 02/19/04
    - Changed the reset to use the execute_trap(reset) which should be right to set
      the initiale state of the cpu
    - Added Trace exception
    - Set of T flag in RET instruction
    - Set I flag in interrupts entries and resetted by a RET instruction
    - Added correct set instruction for SR

 Pierpaolo Prazzoli - 10/26/03
    - Changed get_lrconst to get_const and changed it to use the removed GET_CONST_RR
      macro.
    - Removed the High flag used in some opcodes, it should be used only in
      MOV and MOVI instruction.
    - Fixed MOV and MOVI instruction.
    - Set to 1 FP is SR register at reset.
      (From the doc: A Call, Trap or Software instruction increments the FP and sets FL
      to 6, thus creating a new stack frame with the length of 6 registers).

 Ryan Holtz - 10/25/03
    - Fixed CALL enough that it at least jumps to the right address, no word
      yet as to whether or not it's working enough to return.
    - Added get_lrconst() to get the const value for the CALL operand, since
      apparently using immediate_value() was wrong. The code is ugly, but it
      works properly. Vampire 1/2 now gets far enough to try to test its RAM.
    - Just from looking at it, CALL apparently doesn't frame properly. I'm not
      sure about FRAME, but perhaps it doesn't work properly - I'm not entirely
      positive. The return address when vamphalf's memory check routine is
      called at FFFFFD7E is stored in register L8, and then the RET instruction
      at the end of the routine uses L1 as the return address, so that might
      provide some clues as to how it works.
    - I'd almost be willing to bet money that there's no framing at all since
      the values in L0 - L15 as displayed by the debugger would change during a
      CALL or FRAME operation. I'll look when I'm in the mood.
    - The mood struck me, and I took a look at SET_L_REG and GET_L_REG.
      Apparently no matter what the current frame pointer is they'll always use
      local_regs[0] through local_regs[15].

 Ryan Holtz - 08/20/03
    - Added H flag support for MOV and MOVI
    - Changed init routine to set S flag on boot. Apparently the CPU defaults to
      supervisor mode as opposed to user mode when it powers on, as shown by the
      vamphalf power-on routines. Makes sense, too, since if the machine booted
      in user mode, it would be impossible to get into supervisor mode.

 Pierpaolo Prazzoli - 08/19/03
    - Added check for D_BIT and S_BIT where PC or SR must or must not be denoted.
      (movd, divu, divs, ldxx1, ldxx2, stxx1, stxx2, mulu, muls, set, mul
      call, chk)

 Ryan Holtz - 08/17/03
    - Working on support for H flag, nothing quite done yet
    - Added trap Range Error for CHK PC, PC
    - Fixed relative jumps, they have to be taken from the opcode following the
      jump instead of the jump opcode itself.

 Pierpaolo Prazzoli - 08/17/03
    - Fixed get_pcrel() when OP & 0x80 is set.
    - Decremented PC by 2 also in MOV, ADD, ADDI, SUM, SUB and added the check if
      D_BIT is not set. (when pc is changed they are implicit branch)

 Ryan Holtz - 08/17/03
    - Implemented a crude hack to set FL in the SR to 6, since according to the docs
      that's supposed to happen each time a trap occurs, apparently including when
      the processor starts up. The 3rd opcode executed in vamphalf checks to see if
      the FL flag in SR 6, so it's apparently the "correct" behaviour despite the
      docs not saying anything on it. If FL is not 6, the branch falls through and
      encounters a CHK PC, L2, which at that point will always throw a range trap.
      The range trap vector contains 00000000 (CHK PC, PC), which according to the
      docs will always throw a range trap (which would effectively lock the system).
      This revealed a bug: CHK PC, PC apparently does not throw a range trap, which
      needs to be fixed. Now that the "correct" behaviour is hacked in with the FL
      flags, it reveals yet another bug in that the branch is interpreted as being
      +0x8700. This means that the PC then wraps around to 000082B0, give or take
      a few bytes. While it does indeed branch to valid code, I highly doubt that
      this is the desired effect. Check for signed/unsigned relative branch, maybe?

 Ryan Holtz - 08/16/03
    - Fixed the debugger at least somewhat so that it displays hex instead of decimal,
      and so that it disassembles opcodes properly.
    - Fixed hyperstone_execute() to increment PC *after* executing the opcode instead of
      before. This is probably why vamphalf was booting to fffffff8, but executing at
      fffffffa instead.
    - Changed execute_trap to decrement PC by 2 so that the next opcode isn't skipped
      after a trap
    - Changed execute_br to decrement PC by 2 so that the next opcode isn't skipped
      after a branch
    - Changed hyperstone_movi to decrement PC by 2 when G0 (PC) is modified so that the
      next opcode isn't skipped after a branch
    - Changed hyperstone_movi to default to a UINT32 being moved into the register
      as opposed to a UINT8. This is wrong, the bit width is quite likely to be
      dependent on the n field in the Rimm instruction type. However, vamphalf uses
      MOVI G0,[FFFF]FBAC (n=$13) since there's apparently no absolute branch opcode.
      What kind of CPU is this that it doesn't have an absolute jump in its branch
      instructions and you have to use an immediate MOV to do an abs. jump!?
    - Replaced usage of logerror() with smf's verboselog()

*********************************************************************/

#include "emu.h"
#include "debugger.h"
#include "e132xs.h"

#ifdef MAME_DEBUG
#define DEBUG_PRINTF(x) do { osd_printf_debug x; } while (0)
#else
#define DEBUG_PRINTF(x) do { } while (0)
#endif

// set C in adds/addsi/subs/sums
#define SETCARRYS 0
#define MISSIONCRAFT_FLAGS 1

/* Registers */

/* Internal registers */

#define SREG  (decode)->src_value
#define SREGF (decode)->next_src_value
#define DREG  (decode)->dst_value
#define DREGF (decode)->next_dst_value
#define EXTRA_U (decode)->extra.u
#define EXTRA_S (decode)->extra.s

#define SET_SREG( _data_ )  ((decode)->src_is_local ? set_local_register((decode)->src, (UINT32)_data_) : set_global_register((decode)->src, (UINT32)_data_))
#define SET_SREGF( _data_ ) ((decode)->src_is_local ? set_local_register((decode)->src + 1, (UINT32)_data_) : set_global_register((decode)->src + 1, (UINT32)_data_))
#define SET_DREG( _data_ )  ((decode)->dst_is_local ? set_local_register((decode)->dst, (UINT32)_data_) : set_global_register((decode)->dst, (UINT32)_data_))
#define SET_DREGF( _data_ ) ((decode)->dst_is_local ? set_local_register((decode)->dst + 1, (UINT32)_data_) : set_global_register((decode)->dst + 1, (UINT32)_data_))

#define SRC_IS_PC      (!(decode)->src_is_local && (decode)->src == PC_REGISTER)
#define DST_IS_PC      (!(decode)->dst_is_local && (decode)->dst == PC_REGISTER)
#define SRC_IS_SR      (!(decode)->src_is_local && (decode)->src == SR_REGISTER)
#define DST_IS_SR      (!(decode)->dst_is_local && (decode)->dst == SR_REGISTER)
#define SAME_SRC_DST   (decode)->same_src_dst
#define SAME_SRC_DSTF  (decode)->same_src_dstf
#define SAME_SRCF_DST  (decode)->same_srcf_dst

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

// 4Kb IRAM (On-Chip Memory)

static ADDRESS_MAP_START( e116_4k_iram_map, AS_PROGRAM, 16, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM AM_MIRROR(0x1ffff000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( e132_4k_iram_map, AS_PROGRAM, 32, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM AM_MIRROR(0x1ffff000)
ADDRESS_MAP_END


// 8Kb IRAM (On-Chip Memory)

static ADDRESS_MAP_START( e116_8k_iram_map, AS_PROGRAM, 16, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0001fff) AM_RAM AM_MIRROR(0x1fffe000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( e132_8k_iram_map, AS_PROGRAM, 32, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0001fff) AM_RAM AM_MIRROR(0x1fffe000)
ADDRESS_MAP_END


// 16Kb IRAM (On-Chip Memory)

static ADDRESS_MAP_START( e116_16k_iram_map, AS_PROGRAM, 16, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0003fff) AM_RAM AM_MIRROR(0x1fffc000)
ADDRESS_MAP_END

static ADDRESS_MAP_START( e132_16k_iram_map, AS_PROGRAM, 32, hyperstone_device )
	AM_RANGE(0xc0000000, 0xc0003fff) AM_RAM AM_MIRROR(0x1fffc000)
ADDRESS_MAP_END


//-------------------------------------------------
//  hyperstone_device - constructor
//-------------------------------------------------

hyperstone_device::hyperstone_device(const machine_config &mconfig, const char *name, const char *tag, device_t *owner, UINT32 clock,
										const device_type type, UINT32 prg_data_width, UINT32 io_data_width, address_map_constructor internal_map, const char *shortname, const char *source)
	: cpu_device(mconfig, type, name, tag, owner, clock, shortname, source),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, 32, 0, internal_map),
		m_io_config("io", ENDIANNESS_BIG, io_data_width, 15),
		m_icount(0)
{
	// build the opcode table
	for (int op = 0; op < 256; op++)
		m_opcode[op] = s_opcodetable[op];
}


//-------------------------------------------------
//  e116t_device - constructor
//-------------------------------------------------

e116t_device::e116t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-16T", tag, owner, clock, E116T, 16, 16, ADDRESS_MAP_NAME(e116_4k_iram_map), "e116t", __FILE__)
{
}


//-------------------------------------------------
//  e116xt_device - constructor
//-------------------------------------------------

e116xt_device::e116xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-16XT", tag, owner, clock, E116XT, 16, 16, ADDRESS_MAP_NAME(e116_8k_iram_map), "e116xt", __FILE__)
{
}


//-------------------------------------------------
//  e116xs_device - constructor
//-------------------------------------------------

e116xs_device::e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-16XS", tag, owner, clock, E116XS, 16, 16, ADDRESS_MAP_NAME(e116_16k_iram_map), "e116xs", __FILE__)
{
}


//-------------------------------------------------
//  e116xsr_device - constructor
//-------------------------------------------------

e116xsr_device::e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-16XSR", tag, owner, clock, E116XT, 16, 16, ADDRESS_MAP_NAME(e116_16k_iram_map), "e116xsr", __FILE__)
{
}


//-------------------------------------------------
//  e132n_device - constructor
//-------------------------------------------------

e132n_device::e132n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32N", tag, owner, clock, E132N, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map), "e132n", __FILE__)
{
}


//-------------------------------------------------
//  e132t_device - constructor
//-------------------------------------------------

e132t_device::e132t_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32T", tag, owner, clock, E132T, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map), "e132t", __FILE__)
{
}


//-------------------------------------------------
//  e132xn_device - constructor
//-------------------------------------------------

e132xn_device::e132xn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32XN", tag, owner, clock, E132XN, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map), "e132xn", __FILE__)
{
}


//-------------------------------------------------
//  e132xt_device - constructor
//-------------------------------------------------

e132xt_device::e132xt_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32XT", tag, owner, clock, E132XT, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map), "e132xt", __FILE__)
{
}


//-------------------------------------------------
//  e132xs_device - constructor
//-------------------------------------------------

e132xs_device::e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32XS", tag, owner, clock, E132XS, 32, 32, ADDRESS_MAP_NAME(e132_16k_iram_map), "e132xs", __FILE__)
{
}


//-------------------------------------------------
//  e132xsr_device - constructor
//-------------------------------------------------

e132xsr_device::e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "E1-32XSR", tag, owner, clock, E132XSR, 32, 32, ADDRESS_MAP_NAME(e132_16k_iram_map), "e132xsr", __FILE__)
{
}


//-------------------------------------------------
//  gms30c2116_device - constructor
//-------------------------------------------------

gms30c2116_device::gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "GMS30C2116", tag, owner, clock, GMS30C2116, 16, 16, ADDRESS_MAP_NAME(e116_4k_iram_map), "gms30c2116", __FILE__)
{
}


//-------------------------------------------------
//  gms30c2132_device - constructor
//-------------------------------------------------

gms30c2132_device::gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "GMS30C2132", tag, owner, clock, GMS30C2132, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map), "gms30c2132", __FILE__)
{
}


//-------------------------------------------------
//  gms30c2216_device - constructor
//-------------------------------------------------

gms30c2216_device::gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "GMS30C2216", tag, owner, clock, GMS30C2216, 16, 16, ADDRESS_MAP_NAME(e116_8k_iram_map), "gms30c2216", __FILE__)
{
}


//-------------------------------------------------
//  gms30c2232_device - constructor
//-------------------------------------------------

gms30c2232_device::gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: hyperstone_device(mconfig, "GMS30C2232", tag, owner, clock, GMS30C2232, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map), "gms30c2232", __FILE__)
{
}

/* Return the entry point for a determinated trap */
UINT32 hyperstone_device::get_trap_addr(UINT8 trapno)
{
	UINT32 addr;
	if( m_trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = trapno * 4;
	}
	else
	{
		addr = (63 - trapno) * 4;
	}
	addr |= m_trap_entry;

	return addr;
}

/* Return the entry point for a determinated emulated code (the one for "extend" opcode is reserved) */
UINT32 hyperstone_device::get_emu_code_addr(UINT8 num) /* num is OP */
{
	UINT32 addr;
	if( m_trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = (m_trap_entry - 0x100) | ((num & 0xf) << 4);
	}
	else
	{
		addr = m_trap_entry | (0x10c | ((0xcf - num) << 4));
	}
	return addr;
}

void hyperstone_device::hyperstone_set_trap_entry(int which)
{
	switch( which )
	{
		case E132XS_ENTRY_MEM0:
			m_trap_entry = 0x00000000;
			break;

		case E132XS_ENTRY_MEM1:
			m_trap_entry = 0x40000000;
			break;

		case E132XS_ENTRY_MEM2:
			m_trap_entry = 0x80000000;
			break;

		case E132XS_ENTRY_MEM3:
			m_trap_entry = 0xffffff00;
			break;

		case E132XS_ENTRY_IRAM:
			m_trap_entry = 0xc0000000;
			break;

		default:
			DEBUG_PRINTF(("Set entry point to a reserved value: %d\n", which));
			break;
	}
}

#define OP              m_op
#define PPC             m_ppc //previous pc
#define PC              m_global_regs[0] //Program Counter
#define SR              m_global_regs[1] //Status Register
#define FER             m_global_regs[2] //Floating-Point Exception Register
// 03 - 15  General Purpose Registers
// 16 - 17  Reserved
#define SP              m_global_regs[18] //Stack Pointer
#define UB              m_global_regs[19] //Upper Stack Bound
#define BCR             m_global_regs[20] //Bus Control Register
#define TPR             m_global_regs[21] //Timer Prescaler Register
#define TCR             m_global_regs[22] //Timer Compare Register
#define TR              compute_tr() //Timer Register
#define WCR             m_global_regs[24] //Watchdog Compare Register
#define ISR             m_global_regs[25] //Input Status Register
#define FCR             m_global_regs[26] //Function Control Register
#define MCR             m_global_regs[27] //Memory Control Register
// 28 - 31  Reserved

/* SR flags */
#define GET_C                   ( SR & 0x00000001)      // bit 0 //CARRY
#define GET_Z                   ((SR & 0x00000002)>>1)  // bit 1 //ZERO
#define GET_N                   ((SR & 0x00000004)>>2)  // bit 2 //NEGATIVE
#define GET_V                   ((SR & 0x00000008)>>3)  // bit 3 //OVERFLOW
#define GET_M                   ((SR & 0x00000010)>>4)  // bit 4 //CACHE-MODE
#define GET_H                   ((SR & 0x00000020)>>5)  // bit 5 //HIGHGLOBAL
// bit 6 RESERVED (always 0)
#define GET_I                   ((SR & 0x00000080)>>7)  // bit 7 //INTERRUPT-MODE
#define GET_FTE                 ((SR & 0x00001f00)>>8)  // bits 12 - 8  //Floating-Point Trap Enable
#define GET_FRM                 ((SR & 0x00006000)>>13) // bits 14 - 13 //Floating-Point Rounding Mode
#define GET_L                   ((SR & 0x00008000)>>15) // bit 15 //INTERRUPT-LOCK
#define GET_T                   ((SR & 0x00010000)>>16) // bit 16 //TRACE-MODE
#define GET_P                   ((SR & 0x00020000)>>17) // bit 17 //TRACE PENDING
#define GET_S                   ((SR & 0x00040000)>>18) // bit 18 //SUPERVISOR STATE
#define GET_ILC                 ((SR & 0x00180000)>>19) // bits 20 - 19 //INSTRUCTION-LENGTH
/* if FL is zero it is always interpreted as 16 */
#define GET_FL                  ((SR & 0x01e00000) ? ((SR & 0x01e00000)>>21) : 16) // bits 24 - 21 //FRAME LENGTH
#define GET_FP                  ((SR & 0xfe000000)>>25) // bits 31 - 25 //FRAME POINTER

#define SET_C(val)              (SR = (SR & ~0x00000001) | (val))
#define SET_Z(val)              (SR = (SR & ~0x00000002) | ((val) << 1))
#define SET_N(val)              (SR = (SR & ~0x00000004) | ((val) << 2))
#define SET_V(val)              (SR = (SR & ~0x00000008) | ((val) << 3))
#define SET_M(val)              (SR = (SR & ~0x00000010) | ((val) << 4))
#define SET_H(val)              (SR = (SR & ~0x00000020) | ((val) << 5))
#define SET_I(val)              (SR = (SR & ~0x00000080) | ((val) << 7))
#define SET_FTE(val)            (SR = (SR & ~0x00001f00) | ((val) << 8))
#define SET_FRM(val)            (SR = (SR & ~0x00006000) | ((val) << 13))
#define SET_L(val)              (SR = (SR & ~0x00008000) | ((val) << 15))
#define SET_T(val)              (SR = (SR & ~0x00010000) | ((val) << 16))
#define SET_P(val)              (SR = (SR & ~0x00020000) | ((val) << 17))
#define SET_S(val)              (SR = (SR & ~0x00040000) | ((val) << 18))
#define SET_ILC(val)            (SR = (SR & ~0x00180000) | ((val) << 19))
#define SET_FL(val)             (SR = (SR & ~0x01e00000) | ((val) << 21))
#define SET_FP(val)             (SR = (SR & ~0xfe000000) | ((val) << 25))

#define SET_PC(val)             PC = ((val) & 0xfffffffe) //PC(0) = 0
#define SET_SP(val)             SP = ((val) & 0xfffffffc) //SP(0) = SP(1) = 0
#define SET_UB(val)             UB = ((val) & 0xfffffffc) //UB(0) = UB(1) = 0

#define SET_LOW_SR(val)         (SR = (SR & 0xffff0000) | ((val) & 0x0000ffff)) // when SR is addressed, only low 16 bits can be changed


#define CHECK_C(x)              (SR = (SR & ~0x00000001) | (((x) & (((UINT64)1) << 32)) ? 1 : 0 ))
#define CHECK_VADD(x,y,z)       (SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 8: 0))
#define CHECK_VADD3(x,y,w,z)    (SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & ((w) ^ (z)) & 0x80000000) ? 8: 0))
#define CHECK_VSUB(x,y,z)       (SR = (SR & ~0x00000008) | ((((z) ^ (y)) & ((y) ^ (x)) & 0x80000000) ? 8: 0))


/* FER flags */
#define GET_ACCRUED             (FER & 0x0000001f) //bits  4 - 0 //Floating-Point Accrued Exceptions
#define GET_ACTUAL              (FER & 0x00001f00) //bits 12 - 8 //Floating-Point Actual  Exceptions
//other bits are reversed, in particular 7 - 5 for the operating system.
//the user program can only changes the above 2 flags


UINT32 hyperstone_device::compute_tr()
{
	UINT64 cycles_since_base = total_cycles() - m_tr_base_cycles;
	UINT64 clocks_since_base = cycles_since_base >> m_clck_scale;
	return m_tr_base_value + (clocks_since_base / m_tr_clocks_per_tick);
}

void hyperstone_device::update_timer_prescale()
{
	UINT32 prevtr = compute_tr();
	TPR &= ~0x80000000;
	m_clck_scale = (TPR >> 26) & m_clock_scale_mask;
	m_clock_cycles_1 = 1 << m_clck_scale;
	m_clock_cycles_2 = 2 << m_clck_scale;
	m_clock_cycles_4 = 4 << m_clck_scale;
	m_clock_cycles_6 = 6 << m_clck_scale;
	m_tr_clocks_per_tick = ((TPR >> 16) & 0xff) + 2;
	m_tr_base_value = prevtr;
	m_tr_base_cycles = total_cycles();
}

void hyperstone_device::adjust_timer_interrupt()
{
	UINT64 cycles_since_base = total_cycles() - m_tr_base_cycles;
	UINT64 clocks_since_base = cycles_since_base >> m_clck_scale;
	UINT64 cycles_until_next_clock = cycles_since_base - (clocks_since_base << m_clck_scale);

	if (cycles_until_next_clock == 0)
		cycles_until_next_clock = (UINT64)(1 << m_clck_scale);

	/* special case: if we have a change pending, set a timer to fire then */
	if (TPR & 0x80000000)
	{
		UINT64 clocks_until_int = m_tr_clocks_per_tick - (clocks_since_base % m_tr_clocks_per_tick);
		UINT64 cycles_until_int = (clocks_until_int << m_clck_scale) + cycles_until_next_clock;
		m_timer->adjust(cycles_to_attotime(cycles_until_int + 1), 1);
	}

	/* else if the timer interrupt is enabled, configure it to fire at the appropriate time */
	else if (!(FCR & 0x00800000))
	{
		UINT32 curtr = m_tr_base_value + (clocks_since_base / m_tr_clocks_per_tick);
		UINT32 delta = TCR - curtr;
		if (delta > 0x80000000)
		{
			if (!m_timer_int_pending)
				m_timer->adjust(attotime::zero);
		}
		else
		{
			UINT64 clocks_until_int = mulu_32x32(delta, m_tr_clocks_per_tick);
			UINT64 cycles_until_int = (clocks_until_int << m_clck_scale) + cycles_until_next_clock;
			m_timer->adjust(cycles_to_attotime(cycles_until_int));
		}
	}

	/* otherwise, disable the timer */
	else
		m_timer->adjust(attotime::never);
}

TIMER_CALLBACK_MEMBER( hyperstone_device::timer_callback )
{
	int update = param;

	/* update the values if necessary */
	if (update)
		update_timer_prescale();

	/* see if the timer is right for firing */
	if (!((compute_tr() - TCR) & 0x80000000))
		m_timer_int_pending = 1;

	/* adjust ourselves for the next time */
	else
		adjust_timer_interrupt();
}




UINT32 hyperstone_device::get_global_register(UINT8 code)
{
/*
    if( code >= 16 )
    {
        switch( code )
        {
        case 16:
        case 17:
        case 28:
        case 29:
        case 30:
        case 31:
            DEBUG_PRINTF(("read _Reserved_ Global Register %d @ %08X\n",code,PC));
            break;

        case BCR_REGISTER:
            DEBUG_PRINTF(("read write-only BCR register @ %08X\n",PC));
            return 0;

        case TPR_REGISTER:
            DEBUG_PRINTF(("read write-only TPR register @ %08X\n",PC));
            return 0;

        case FCR_REGISTER:
            DEBUG_PRINTF(("read write-only FCR register @ %08X\n",PC));
            return 0;

        case MCR_REGISTER:
            DEBUG_PRINTF(("read write-only MCR register @ %08X\n",PC));
            return 0;
        }
    }
*/
	if (code == TR_REGISTER)
	{
		/* it is common to poll this in a loop */
		if (m_icount > m_tr_clocks_per_tick / 2)
			m_icount -= m_tr_clocks_per_tick / 2;
		return compute_tr();
	}
	return m_global_regs[code];
}

void hyperstone_device::set_local_register(UINT8 code, UINT32 val)
{
	UINT8 new_code = (code + GET_FP) % 64;

	m_local_regs[new_code] = val;
}

void hyperstone_device::set_global_register(UINT8 code, UINT32 val)
{
	//TODO: add correct FER set instruction

	if( code == PC_REGISTER )
	{
		SET_PC(val);
	}
	else if( code == SR_REGISTER )
	{
		SET_LOW_SR(val); // only a RET instruction can change the full content of SR
		SR &= ~0x40; //reserved bit 6 always zero
		if (m_intblock < 1)
			m_intblock = 1;
	}
	else
	{
		UINT32 oldval = m_global_regs[code];
		if( code != ISR_REGISTER )
			m_global_regs[code] = val;
		else
			DEBUG_PRINTF(("Written to ISR register. PC = %08X\n", PC));

		//are these set only when privilege bit is set?
		if( code >= 16 )
		{
			switch( code )
			{
			case 18:
				SET_SP(val);
				break;

			case 19:
				SET_UB(val);
				break;
/*
            case ISR_REGISTER:
                DEBUG_PRINTF(("written %08X to read-only ISR register\n",val));
                break;

            case TCR_REGISTER:
//              DEBUG_PRINTF(("written %08X to TCR register\n",val));
                break;

            case 23:
//              DEBUG_PRINTF(("written %08X to TR register\n",val));
                break;

            case 24:
//              DEBUG_PRINTF(("written %08X to WCR register\n",val));
                break;

            case 16:
            case 17:
            case 28:
            case 29:
            case 30:
            case 31:
                DEBUG_PRINTF(("written %08X to _Reserved_ Global Register %d\n",val,code));
                break;

            case BCR_REGISTER:
                break;
*/
			case TR_REGISTER:
				m_tr_base_value = val;
				m_tr_base_cycles = total_cycles();
				adjust_timer_interrupt();
				break;

			case TPR_REGISTER:
				if (!(val & 0x80000000)) /* change immediately */
					update_timer_prescale();
				adjust_timer_interrupt();
				break;

			case TCR_REGISTER:
				if (oldval != val)
				{
					adjust_timer_interrupt();
					if (m_intblock < 1)
						m_intblock = 1;
				}
				break;

			case FCR_REGISTER:
				if ((oldval ^ val) & 0x00800000)
					adjust_timer_interrupt();
				if (m_intblock < 1)
					m_intblock = 1;
				break;

			case MCR_REGISTER:
				// bits 14..12 EntryTableMap
				hyperstone_set_trap_entry((val & 0x7000) >> 12);
				break;
			}
		}
	}
}

#define GET_ABS_L_REG(code)         m_local_regs[code]
#define SET_L_REG(code, val)        set_local_register(code, val)
#define SET_ABS_L_REG(code, val)    m_local_regs[code] = val
#define GET_G_REG(code)             get_global_register(code)
#define SET_G_REG(code, val)        set_global_register(code, val)

#define S_BIT                   ((OP & 0x100) >> 8)
#define N_BIT                   S_BIT
#define D_BIT                   ((OP & 0x200) >> 9)
#define N_VALUE                 ((N_BIT << 4) | (OP & 0x0f))
#define DST_CODE                ((OP & 0xf0) >> 4)
#define SRC_CODE                (OP & 0x0f)
#define SIGN_BIT(val)           ((val & 0x80000000) >> 31)

#define LOCAL  1

static const INT32 immediate_values[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 0, 0, 0, 32, 64, 128, 0x80000000,
	-8, -7, -6, -5, -4, -3, -2, -1
};

#define WRITE_ONLY_REGMASK  ((1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER))

#define decode_source(decode, local, hflag)                                         \
do                                                                                  \
{                                                                                   \
	if(local)                                                                       \
	{                                                                               \
		UINT8 code = (decode)->src;                                                 \
		(decode)->src_is_local = 1;                                                 \
		code = ((decode)->src + GET_FP) % 64; /* registers offset by frame pointer  */\
		SREG = m_local_regs[code];                                                  \
		code = ((decode)->src + 1 + GET_FP) % 64;                                   \
		SREGF = m_local_regs[code];                                                 \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		(decode)->src_is_local = 0;                                                 \
																					\
		if (!hflag)                                                                 \
		{                                                                           \
			SREG = get_global_register((decode)->src);                              \
																					\
			/* bound safe */                                                        \
			if ((decode)->src != 15)                                                \
				SREGF = get_global_register((decode)->src + 1);                     \
		}                                                                           \
		else                                                                        \
		{                                                                           \
			(decode)->src += 16;                                                    \
																					\
			SREG = get_global_register((decode)->src);                              \
			if ((WRITE_ONLY_REGMASK >> (decode)->src) & 1)                          \
				SREG = 0; /* write-only registers */                                \
			else if ((decode)->src == ISR_REGISTER)                                 \
				DEBUG_PRINTF(("read src ISR. PC = %08X\n",PPC));                    \
																					\
			/* bound safe */                                                        \
			if ((decode)->src != 31)                                                \
				SREGF = get_global_register((decode)->src + 1);                     \
		}                                                                           \
	}                                                                               \
} while (0)

#define decode_dest(decode, local, hflag)                                           \
do                                                                                  \
{                                                                                   \
	if(local)                                                                       \
	{                                                                               \
		UINT8 code = (decode)->dst;                                                 \
		(decode)->dst_is_local = 1;                                                 \
		code = ((decode)->dst + GET_FP) % 64; /* registers offset by frame pointer */\
		DREG = m_local_regs[code];                                                  \
		code = ((decode)->dst + 1 + GET_FP) % 64;                                   \
		DREGF = m_local_regs[code];                                                 \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		(decode)->dst_is_local = 0;                                                 \
																					\
		if (!hflag)                                                                 \
		{                                                                           \
			DREG = get_global_register((decode)->dst);                              \
																					\
			/* bound safe */                                                        \
			if ((decode)->dst != 15)                                                \
				DREGF = get_global_register((decode)->dst + 1);                     \
		}                                                                           \
		else                                                                        \
		{                                                                           \
			(decode)->dst += 16;                                                    \
																					\
			DREG = get_global_register((decode)->dst);                              \
			if( (decode)->dst == ISR_REGISTER )                                     \
				DEBUG_PRINTF(("read dst ISR. PC = %08X\n",PPC));                    \
																					\
			/* bound safe */                                                        \
			if ((decode)->dst != 31)                                                \
				DREGF = get_global_register((decode)->dst + 1);                     \
		}                                                                           \
	}                                                                               \
} while (0)

#define decode_RR(decode, dlocal, slocal)                                           \
do                                                                                  \
{                                                                                   \
	(decode)->src = SRC_CODE;                                                       \
	(decode)->dst = DST_CODE;                                                       \
	decode_source(decode, slocal, 0);                                               \
	decode_dest(decode, dlocal, 0);                                                 \
																					\
	if( (slocal) == (dlocal) && SRC_CODE == DST_CODE )                              \
		SAME_SRC_DST = 1;                                                           \
																					\
	if( (slocal) == LOCAL && (dlocal) == LOCAL )                                    \
	{                                                                               \
		if( SRC_CODE == ((DST_CODE + 1) % 64) )                                     \
			SAME_SRC_DSTF = 1;                                                      \
																					\
		if( ((SRC_CODE + 1) % 64) == DST_CODE )                                     \
			SAME_SRCF_DST = 1;                                                      \
	}                                                                               \
	else if( (slocal) == 0 && (dlocal) == 0 )                                       \
	{                                                                               \
		if( SRC_CODE == (DST_CODE + 1) )                                            \
			SAME_SRC_DSTF = 1;                                                      \
																					\
		if( (SRC_CODE + 1) == DST_CODE )                                            \
			SAME_SRCF_DST = 1;                                                      \
	}                                                                               \
} while (0)

#define decode_LL(decode)                                                           \
do                                                                                  \
{                                                                                   \
	(decode)->src = SRC_CODE;                                                       \
	(decode)->dst = DST_CODE;                                                       \
	decode_source(decode, LOCAL, 0);                                                \
	decode_dest(decode, LOCAL, 0);                                                  \
																					\
	if( SRC_CODE == DST_CODE )                                                      \
		SAME_SRC_DST = 1;                                                           \
																					\
	if( SRC_CODE == ((DST_CODE + 1) % 64) )                                         \
		SAME_SRC_DSTF = 1;                                                          \
} while (0)

#define decode_LR(decode, slocal)                                                   \
do                                                                                  \
{                                                                                   \
	(decode)->src = SRC_CODE;                                                       \
	(decode)->dst = DST_CODE;                                                       \
	decode_source(decode, slocal, 0);                                               \
	decode_dest(decode, LOCAL, 0);                                                  \
																					\
	if( ((SRC_CODE + 1) % 64) == DST_CODE && slocal == LOCAL )                      \
		SAME_SRCF_DST = 1;                                                          \
} while (0)

#define check_delay_PC()                                                            \
do                                                                                  \
{                                                                                   \
	/* if PC is used in a delay instruction, the delayed PC should be used */       \
	if( m_delay.delay_cmd == DELAY_EXECUTE )                                        \
	{                                                                               \
		PC = m_delay.delay_pc;                                                      \
		m_delay.delay_cmd = NO_DELAY;                                               \
	}                                                                               \
} while (0)

#define decode_immediate(decode, nbit)                                              \
do                                                                                  \
{                                                                                   \
	if (!nbit)                                                                      \
		EXTRA_U = immediate_values[OP & 0x0f];                                      \
	else                                                                            \
		switch( OP & 0x0f )                                                         \
		{                                                                           \
			default:                                                                \
				EXTRA_U = immediate_values[0x10 + (OP & 0x0f)];                     \
				break;                                                              \
																					\
			case 1:                                                                 \
				m_instruction_length = 3;                                           \
				EXTRA_U = (READ_OP(PC) << 16) | READ_OP(PC + 2);                    \
				PC += 4;                                                            \
				break;                                                              \
																					\
			case 2:                                                                 \
				m_instruction_length = 2;                                           \
				EXTRA_U = READ_OP(PC);                                              \
				PC += 2;                                                            \
				break;                                                              \
																					\
			case 3:                                                                 \
				m_instruction_length = 2;                                           \
				EXTRA_U = 0xffff0000 | READ_OP(PC);                                 \
				PC += 2;                                                            \
				break;                                                              \
		}                                                                           \
} while (0)

#define decode_const(decode)                                                        \
do                                                                                  \
{                                                                                   \
	UINT16 imm_1 = READ_OP(PC);                                                     \
																					\
	PC += 2;                                                                        \
	m_instruction_length = 2;                                                       \
																					\
	if( E_BIT(imm_1) )                                                              \
	{                                                                               \
		UINT16 imm_2 = READ_OP(PC);                                                 \
																					\
		PC += 2;                                                                    \
		m_instruction_length = 3;                                                   \
																					\
		EXTRA_S = imm_2;                                                            \
		EXTRA_S |= ((imm_1 & 0x3fff) << 16);                                        \
																					\
		if( S_BIT_CONST(imm_1) )                                                    \
		{                                                                           \
			EXTRA_S |= 0xc0000000;                                                  \
		}                                                                           \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		EXTRA_S = imm_1 & 0x3fff;                                                   \
																					\
		if( S_BIT_CONST(imm_1) )                                                    \
		{                                                                           \
			EXTRA_S |= 0xffffc000;                                                  \
		}                                                                           \
	}                                                                               \
} while (0)

#define decode_pcrel(decode)                                                        \
do                                                                                  \
{                                                                                   \
	if( OP & 0x80 )                                                                 \
	{                                                                               \
		UINT16 next = READ_OP(PC);                                                  \
																					\
		PC += 2;                                                                    \
		m_instruction_length = 2;                                                   \
																					\
		EXTRA_S = (OP & 0x7f) << 16;                                                \
		EXTRA_S |= (next & 0xfffe);                                                 \
																					\
		if( next & 1 )                                                              \
			EXTRA_S |= 0xff800000;                                                  \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		EXTRA_S = OP & 0x7e;                                                        \
																					\
		if( OP & 1 )                                                                \
			EXTRA_S |= 0xffffff80;                                                  \
	}                                                                               \
} while (0)

#define decode_dis(decode)                                                          \
do                                                                                  \
{                                                                                   \
	UINT16 next_1 = READ_OP(PC);                                                    \
																					\
	PC += 2;                                                                        \
	m_instruction_length = 2;                                                       \
																					\
	(decode)->sub_type = DD(next_1);                                                \
																					\
	if( E_BIT(next_1) )                                                             \
	{                                                                               \
		UINT16 next_2 = READ_OP(PC);                                                \
																					\
		PC += 2;                                                                    \
		m_instruction_length = 3;                                                   \
																					\
		EXTRA_S = next_2;                                                           \
		EXTRA_S |= ((next_1 & 0xfff) << 16);                                        \
																					\
		if( S_BIT_CONST(next_1) )                                                   \
		{                                                                           \
			EXTRA_S |= 0xf0000000;                                                  \
		}                                                                           \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		EXTRA_S = next_1 & 0xfff;                                                   \
																					\
		if( S_BIT_CONST(next_1) )                                                   \
		{                                                                           \
			EXTRA_S |= 0xfffff000;                                                  \
		}                                                                           \
	}                                                                               \
} while (0)

#define decode_lim(decode)                                                          \
do                                                                                  \
{                                                                                   \
	UINT32 next = READ_OP(PC);                                                      \
	PC += 2;                                                                        \
	m_instruction_length = 2;                                                       \
																					\
	(decode)->sub_type = X_CODE(next);                                              \
																					\
	if( E_BIT(next) )                                                               \
	{                                                                               \
		EXTRA_U = ((next & 0xfff) << 16) | READ_OP(PC);                             \
		PC += 2;                                                                    \
		m_instruction_length = 3;                                                   \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		EXTRA_U = next & 0xfff;                                                     \
	}                                                                               \
} while (0)

#define RRdecode(decode, dlocal, slocal)                                            \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	decode_RR(decode, dlocal, slocal);                                              \
} while (0)

#define RRlimdecode(decode, dlocal, slocal)                                         \
do                                                                                  \
{                                                                                   \
	decode_lim(decode);                                                             \
	check_delay_PC();                                                               \
	decode_RR(decode, dlocal, slocal);                                              \
} while (0)

#define RRconstdecode(decode, dlocal, slocal)                                       \
do                                                                                  \
{                                                                                   \
	decode_const(decode);                                                           \
	check_delay_PC();                                                               \
	decode_RR(decode, dlocal, slocal);                                              \
} while (0)

#define RRdisdecode(decode, dlocal, slocal)                                         \
do                                                                                  \
{                                                                                   \
	decode_dis(decode);                                                             \
	check_delay_PC();                                                               \
	decode_RR(decode, dlocal, slocal);                                              \
} while (0)

#define RRdecodewithHflag(decode, dlocal, slocal)                                   \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	(decode)->src = SRC_CODE;                                                       \
	(decode)->dst = DST_CODE;                                                       \
	decode_source(decode, slocal, GET_H);                                           \
	decode_dest(decode, dlocal, GET_H);                                             \
																					\
	if(GET_H)                                                                       \
		if(slocal == 0 && dlocal == 0)                                              \
			DEBUG_PRINTF(("MOV with hflag and 2 GRegs! PC = %08X\n",PPC));          \
} while (0)

#define Rimmdecode(decode, dlocal, nbit)                                            \
do                                                                                  \
{                                                                                   \
	decode_immediate(decode, nbit);                                                 \
	check_delay_PC();                                                               \
	(decode)->dst = DST_CODE;                                                       \
	decode_dest(decode, dlocal, 0);                                                 \
} while (0)

#define Rndecode(decode, dlocal)                                                    \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	(decode)->dst = DST_CODE;                                                       \
	decode_dest(decode, dlocal, 0);                                                 \
} while (0)

#define RimmdecodewithHflag(decode, dlocal, nbit)                                   \
do                                                                                  \
{                                                                                   \
	decode_immediate(decode, nbit);                                                 \
	check_delay_PC();                                                               \
	(decode)->dst = DST_CODE;                                                       \
	decode_dest(decode, dlocal, GET_H);                                             \
} while (0)

#define Lndecode(decode)                                                            \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	(decode)->dst = DST_CODE;                                                       \
	decode_dest(decode, LOCAL, 0);                                                  \
} while (0)

#define LLdecode(decode)                                                            \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	decode_LL(decode);                                                              \
} while (0)

#define LLextdecode(decode)                                                         \
do                                                                                  \
{                                                                                   \
	m_instruction_length = 2;                                                       \
	EXTRA_U = READ_OP(PC);                                                          \
	PC += 2;                                                                        \
	check_delay_PC();                                                               \
	decode_LL(decode);                                                              \
} while (0)

#define LRdecode(decode, slocal)                                                    \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
	decode_LR(decode, slocal);                                                      \
} while (0)

#define LRconstdecode(decode, slocal)                                               \
do                                                                                  \
{                                                                                   \
	decode_const(decode);                                                           \
	check_delay_PC();                                                               \
	decode_LR(decode, slocal);                                                      \
} while (0)

#define PCreldecode(decode)                                                         \
do                                                                                  \
{                                                                                   \
	decode_pcrel(decode);                                                           \
	check_delay_PC();                                                               \
} while (0)

#define PCadrdecode(decode)                                                         \
do                                                                                  \
{                                                                                   \
	check_delay_PC();                                                               \
} while (0)

#define no_decode(decode)                                                           \
do                                                                                  \
{                                                                                   \
} while (0)


void hyperstone_device::execute_br(struct hyperstone_device::regs_decode *decode)
{
	PPC = PC;
	PC += EXTRA_S;
	SET_M(0);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::execute_dbr(struct hyperstone_device::regs_decode *decode)
{
	m_delay.delay_cmd = DELAY_EXECUTE;
	m_delay.delay_pc  = PC + EXTRA_S;

	m_intblock = 3;
}


void hyperstone_device::execute_trap(UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length & 3);

	oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	SET_L_REG(0, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(1, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	PPC = PC;
	PC = addr;

	m_icount -= m_clock_cycles_2;
}


void hyperstone_device::execute_int(UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length & 3);

	oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	SET_L_REG(0, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(1, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);
	SET_I(1);

	PPC = PC;
	PC = addr;

	m_icount -= m_clock_cycles_2;
}

/* TODO: mask Parity Error and Extended Overflow exceptions */
void hyperstone_device::execute_exception(UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length & 3);

	oldSR = SR;

	SET_FP(reg);
	SET_FL(2);

	SET_L_REG(0, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(1, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	PPC = PC;
	PC = addr;

	DEBUG_PRINTF(("EXCEPTION! PPC = %08X PC = %08X\n",PPC-2,PC-2));
	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::execute_software(struct hyperstone_device::regs_decode *decode)
{
	UINT8 reg;
	UINT32 oldSR;
	UINT32 addr;
	UINT32 stack_of_dst;

	SET_ILC(1);

	addr = get_emu_code_addr((OP & 0xff00) >> 8);
	reg = GET_FP + GET_FL;

	//since it's sure the register is in the register part of the stack,
	//set the stack address to a value above the highest address
	//that can be set by a following frame instruction
	stack_of_dst = (SP & ~0xff) + 64*4 + (((GET_FP + decode->dst) % 64) * 4); //converted to 32bits offset

	oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	SET_L_REG(0, stack_of_dst);
	SET_L_REG(1, SREG);
	SET_L_REG(2, SREGF);
	SET_L_REG(3, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(4, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);

	PPC = PC;
	PC = addr;
}


/*
    IRQ lines :
        0 - IO2     (trap 48)
        1 - IO1     (trap 49)
        2 - INT4    (trap 50)
        3 - INT3    (trap 51)
        4 - INT2    (trap 52)
        5 - INT1    (trap 53)
        6 - IO3     (trap 54)
        7 - TIMER   (trap 55)
*/

#define INT1_LINE_STATE     ((ISR >> 0) & 1)
#define INT2_LINE_STATE     ((ISR >> 1) & 1)
#define INT3_LINE_STATE     ((ISR >> 2) & 1)
#define INT4_LINE_STATE     ((ISR >> 3) & 1)
#define IO1_LINE_STATE      ((ISR >> 4) & 1)
#define IO2_LINE_STATE      ((ISR >> 5) & 1)
#define IO3_LINE_STATE      ((ISR >> 6) & 1)

void hyperstone_device::check_interrupts()
{
	/* Interrupt-Lock flag isn't set */
	if (GET_L || m_intblock > 0)
		return;

	/* quick exit if nothing */
	if (!m_timer_int_pending && (ISR & 0x7f) == 0)
		return;

	/* IO3 is priority 5; state is in bit 6 of ISR; FCR bit 10 enables input and FCR bit 8 inhibits interrupt */
	if (IO3_LINE_STATE && (FCR & 0x00000500) == 0x00000400)
	{
		execute_int(get_trap_addr(TRAPNO_IO3));
		standard_irq_callback(IRQ_IO3);
		return;
	}

	/* timer int might be priority 6 if FCR bits 20-21 == 3; FCR bit 23 inhibits interrupt */
	if (m_timer_int_pending && (FCR & 0x00b00000) == 0x00300000)
	{
		m_timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	/* INT1 is priority 7; state is in bit 0 of ISR; FCR bit 28 inhibits interrupt */
	if (INT1_LINE_STATE && (FCR & 0x10000000) == 0x00000000)
	{
		execute_int(get_trap_addr(TRAPNO_INT1));
		standard_irq_callback(IRQ_INT1);
		return;
	}

	/* timer int might be priority 8 if FCR bits 20-21 == 2; FCR bit 23 inhibits interrupt */
	if (m_timer_int_pending && (FCR & 0x00b00000) == 0x00200000)
	{
		m_timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	/* INT2 is priority 9; state is in bit 1 of ISR; FCR bit 29 inhibits interrupt */
	if (INT2_LINE_STATE && (FCR & 0x20000000) == 0x00000000)
	{
		execute_int(get_trap_addr(TRAPNO_INT2));
		standard_irq_callback(IRQ_INT2);
		return;
	}

	/* timer int might be priority 10 if FCR bits 20-21 == 1; FCR bit 23 inhibits interrupt */
	if (m_timer_int_pending && (FCR & 0x00b00000) == 0x00100000)
	{
		m_timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	/* INT3 is priority 11; state is in bit 2 of ISR; FCR bit 30 inhibits interrupt */
	if (INT3_LINE_STATE && (FCR & 0x40000000) == 0x00000000)
	{
		execute_int(get_trap_addr(TRAPNO_INT3));
		standard_irq_callback(IRQ_INT3);
		return;
	}

	/* timer int might be priority 12 if FCR bits 20-21 == 0; FCR bit 23 inhibits interrupt */
	if (m_timer_int_pending && (FCR & 0x00b00000) == 0x00000000)
	{
		m_timer_int_pending = 0;
		execute_int(get_trap_addr(TRAPNO_TIMER));
		return;
	}

	/* INT4 is priority 13; state is in bit 3 of ISR; FCR bit 31 inhibits interrupt */
	if (INT4_LINE_STATE && (FCR & 0x80000000) == 0x00000000)
	{
		execute_int(get_trap_addr(TRAPNO_INT4));
		standard_irq_callback(IRQ_INT4);
		return;
	}

	/* IO1 is priority 14; state is in bit 4 of ISR; FCR bit 2 enables input and FCR bit 0 inhibits interrupt */
	if (IO1_LINE_STATE && (FCR & 0x00000005) == 0x00000004)
	{
		execute_int(get_trap_addr(TRAPNO_IO1));
		standard_irq_callback(IRQ_IO1);
		return;
	}

	/* IO2 is priority 15; state is in bit 5 of ISR; FCR bit 6 enables input and FCR bit 4 inhibits interrupt */
	if (IO2_LINE_STATE && (FCR & 0x00000050) == 0x00000040)
	{
		execute_int(get_trap_addr(TRAPNO_IO2));
		standard_irq_callback(IRQ_IO2);
		return;
	}
}

void hyperstone_device::device_start()
{
	// Handled entirely by init() and derived classes
}

void hyperstone_device::init(int scale_mask)
{
	memset(m_global_regs, 0, sizeof(UINT32) * 32);
	memset(m_local_regs, 0, sizeof(UINT32) * 64);
	m_ppc = 0;
	m_op = 0;
	m_trap_entry = 0;
	m_clock_scale_mask = 0;
	m_clck_scale = 0;
	m_clock_cycles_1 = 0;
	m_clock_cycles_2 = 0;
	m_clock_cycles_4 = 0;
	m_clock_cycles_6 = 0;

	m_tr_base_cycles = 0;
	m_tr_base_value = 0;
	m_tr_clocks_per_tick = 0;
	m_timer_int_pending = 0;

	m_instruction_length = 0;
	m_intblock = 0;

	m_icount = 0;

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	m_timer = machine().scheduler().timer_alloc(timer_expired_delegate(FUNC(hyperstone_device::timer_callback), this));
	m_clock_scale_mask = scale_mask;

	// register our state for the debugger
	state_add(STATE_GENPC,    "GENPC",     m_global_regs[0]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_global_regs[1]).callimport().callexport().formatstr("%40s").noshow();
	state_add(E132XS_PC,      "PC", m_global_regs[0]).mask(0xffffffff);
	state_add(E132XS_SR,      "SR", m_global_regs[1]).mask(0xffffffff);
	state_add(E132XS_FER,     "FER", m_global_regs[2]).mask(0xffffffff);
	state_add(E132XS_G3,      "G3", m_global_regs[3]).mask(0xffffffff);
	state_add(E132XS_G4,      "G4", m_global_regs[4]).mask(0xffffffff);
	state_add(E132XS_G5,      "G5", m_global_regs[5]).mask(0xffffffff);
	state_add(E132XS_G6,      "G6", m_global_regs[6]).mask(0xffffffff);
	state_add(E132XS_G7,      "G7", m_global_regs[7]).mask(0xffffffff);
	state_add(E132XS_G8,      "G8", m_global_regs[8]).mask(0xffffffff);
	state_add(E132XS_G9,      "G9", m_global_regs[9]).mask(0xffffffff);
	state_add(E132XS_G10,     "G10", m_global_regs[10]).mask(0xffffffff);
	state_add(E132XS_G11,     "G11", m_global_regs[11]).mask(0xffffffff);
	state_add(E132XS_G12,     "G12", m_global_regs[12]).mask(0xffffffff);
	state_add(E132XS_G13,     "G13", m_global_regs[13]).mask(0xffffffff);
	state_add(E132XS_G14,     "G14", m_global_regs[14]).mask(0xffffffff);
	state_add(E132XS_G15,     "G15", m_global_regs[15]).mask(0xffffffff);
	state_add(E132XS_G16,     "G16", m_global_regs[16]).mask(0xffffffff);
	state_add(E132XS_G17,     "G17", m_global_regs[17]).mask(0xffffffff);
	state_add(E132XS_SP,      "SP", m_global_regs[18]).mask(0xffffffff);
	state_add(E132XS_UB,      "UB", m_global_regs[19]).mask(0xffffffff);
	state_add(E132XS_BCR,     "BCR", m_global_regs[20]).mask(0xffffffff);
	state_add(E132XS_TPR,     "TPR", m_global_regs[21]).mask(0xffffffff);
	state_add(E132XS_TCR,     "TCR", m_global_regs[22]).mask(0xffffffff);
	state_add(E132XS_TR,      "TR", m_global_regs[23]).mask(0xffffffff);
	state_add(E132XS_WCR,     "WCR", m_global_regs[24]).mask(0xffffffff);
	state_add(E132XS_ISR,     "ISR", m_global_regs[25]).mask(0xffffffff);
	state_add(E132XS_FCR,     "FCR", m_global_regs[26]).mask(0xffffffff);
	state_add(E132XS_MCR,     "MCR", m_global_regs[27]).mask(0xffffffff);
	state_add(E132XS_G28,     "G28", m_global_regs[28]).mask(0xffffffff);
	state_add(E132XS_G29,     "G29", m_global_regs[29]).mask(0xffffffff);
	state_add(E132XS_G30,     "G30", m_global_regs[30]).mask(0xffffffff);
	state_add(E132XS_G31,     "G31", m_global_regs[31]).mask(0xffffffff);
	state_add(E132XS_CL0,     "CL0", m_local_regs[(0 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL1,     "CL1", m_local_regs[(1 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL2,     "CL2", m_local_regs[(2 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL3,     "CL3", m_local_regs[(3 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL4,     "CL4", m_local_regs[(4 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL5,     "CL5", m_local_regs[(5 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL6,     "CL6", m_local_regs[(6 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL7,     "CL7", m_local_regs[(7 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL8,     "CL8", m_local_regs[(8 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL9,     "CL9", m_local_regs[(9 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL10,    "CL10", m_local_regs[(10 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL11,    "CL11", m_local_regs[(11 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL12,    "CL12", m_local_regs[(12 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL13,    "CL13", m_local_regs[(13 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL14,    "CL14", m_local_regs[(14 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL15,    "CL15", m_local_regs[(15 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_L0,      "L0", m_local_regs[0]).mask(0xffffffff);
	state_add(E132XS_L1,      "L1", m_local_regs[1]).mask(0xffffffff);
	state_add(E132XS_L2,      "L2", m_local_regs[2]).mask(0xffffffff);
	state_add(E132XS_L3,      "L3", m_local_regs[3]).mask(0xffffffff);
	state_add(E132XS_L4,      "L4", m_local_regs[4]).mask(0xffffffff);
	state_add(E132XS_L5,      "L5", m_local_regs[5]).mask(0xffffffff);
	state_add(E132XS_L6,      "L6", m_local_regs[6]).mask(0xffffffff);
	state_add(E132XS_L7,      "L7", m_local_regs[7]).mask(0xffffffff);
	state_add(E132XS_L8,      "L8", m_local_regs[8]).mask(0xffffffff);
	state_add(E132XS_L9,      "L9", m_local_regs[9]).mask(0xffffffff);
	state_add(E132XS_L10,     "L10", m_local_regs[10]).mask(0xffffffff);
	state_add(E132XS_L11,     "L11", m_local_regs[11]).mask(0xffffffff);
	state_add(E132XS_L12,     "L12", m_local_regs[12]).mask(0xffffffff);
	state_add(E132XS_L13,     "L13", m_local_regs[13]).mask(0xffffffff);
	state_add(E132XS_L14,     "L14", m_local_regs[14]).mask(0xffffffff);
	state_add(E132XS_L15,     "L15", m_local_regs[15]).mask(0xffffffff);
	state_add(E132XS_L16,     "L16", m_local_regs[16]).mask(0xffffffff);
	state_add(E132XS_L17,     "L17", m_local_regs[17]).mask(0xffffffff);
	state_add(E132XS_L18,     "L18", m_local_regs[18]).mask(0xffffffff);
	state_add(E132XS_L19,     "L19", m_local_regs[19]).mask(0xffffffff);
	state_add(E132XS_L20,     "L20", m_local_regs[20]).mask(0xffffffff);
	state_add(E132XS_L21,     "L21", m_local_regs[21]).mask(0xffffffff);
	state_add(E132XS_L22,     "L22", m_local_regs[22]).mask(0xffffffff);
	state_add(E132XS_L23,     "L23", m_local_regs[23]).mask(0xffffffff);
	state_add(E132XS_L24,     "L24", m_local_regs[24]).mask(0xffffffff);
	state_add(E132XS_L25,     "L25", m_local_regs[25]).mask(0xffffffff);
	state_add(E132XS_L26,     "L26", m_local_regs[26]).mask(0xffffffff);
	state_add(E132XS_L27,     "L27", m_local_regs[27]).mask(0xffffffff);
	state_add(E132XS_L28,     "L28", m_local_regs[28]).mask(0xffffffff);
	state_add(E132XS_L29,     "L29", m_local_regs[29]).mask(0xffffffff);
	state_add(E132XS_L30,     "L30", m_local_regs[30]).mask(0xffffffff);
	state_add(E132XS_L31,     "L31", m_local_regs[31]).mask(0xffffffff);
	state_add(E132XS_L32,     "L32", m_local_regs[32]).mask(0xffffffff);
	state_add(E132XS_L33,     "L33", m_local_regs[33]).mask(0xffffffff);
	state_add(E132XS_L34,     "L34", m_local_regs[34]).mask(0xffffffff);
	state_add(E132XS_L35,     "L35", m_local_regs[35]).mask(0xffffffff);
	state_add(E132XS_L36,     "L36", m_local_regs[36]).mask(0xffffffff);
	state_add(E132XS_L37,     "L37", m_local_regs[37]).mask(0xffffffff);
	state_add(E132XS_L38,     "L38", m_local_regs[38]).mask(0xffffffff);
	state_add(E132XS_L39,     "L39", m_local_regs[39]).mask(0xffffffff);
	state_add(E132XS_L40,     "L40", m_local_regs[40]).mask(0xffffffff);
	state_add(E132XS_L41,     "L41", m_local_regs[41]).mask(0xffffffff);
	state_add(E132XS_L42,     "L42", m_local_regs[42]).mask(0xffffffff);
	state_add(E132XS_L43,     "L43", m_local_regs[43]).mask(0xffffffff);
	state_add(E132XS_L44,     "L44", m_local_regs[44]).mask(0xffffffff);
	state_add(E132XS_L45,     "L45", m_local_regs[45]).mask(0xffffffff);
	state_add(E132XS_L46,     "L46", m_local_regs[46]).mask(0xffffffff);
	state_add(E132XS_L47,     "L47", m_local_regs[47]).mask(0xffffffff);
	state_add(E132XS_L48,     "L48", m_local_regs[48]).mask(0xffffffff);
	state_add(E132XS_L49,     "L49", m_local_regs[49]).mask(0xffffffff);
	state_add(E132XS_L50,     "L50", m_local_regs[50]).mask(0xffffffff);
	state_add(E132XS_L51,     "L51", m_local_regs[51]).mask(0xffffffff);
	state_add(E132XS_L52,     "L52", m_local_regs[52]).mask(0xffffffff);
	state_add(E132XS_L53,     "L53", m_local_regs[53]).mask(0xffffffff);
	state_add(E132XS_L54,     "L54", m_local_regs[54]).mask(0xffffffff);
	state_add(E132XS_L55,     "L55", m_local_regs[55]).mask(0xffffffff);
	state_add(E132XS_L56,     "L56", m_local_regs[56]).mask(0xffffffff);
	state_add(E132XS_L57,     "L57", m_local_regs[57]).mask(0xffffffff);
	state_add(E132XS_L58,     "L58", m_local_regs[58]).mask(0xffffffff);
	state_add(E132XS_L59,     "L59", m_local_regs[59]).mask(0xffffffff);
	state_add(E132XS_L60,     "L60", m_local_regs[60]).mask(0xffffffff);
	state_add(E132XS_L61,     "L61", m_local_regs[61]).mask(0xffffffff);
	state_add(E132XS_L62,     "L62", m_local_regs[62]).mask(0xffffffff);
	state_add(E132XS_L63,     "L63", m_local_regs[63]).mask(0xffffffff);

	save_item(NAME(m_global_regs));
	save_item(NAME(m_local_regs));
	save_item(NAME(m_ppc));
	save_item(NAME(m_trap_entry));
	save_item(NAME(m_delay.delay_pc));
	save_item(NAME(m_instruction_length));
	save_item(NAME(m_intblock));
	save_item(NAME(m_delay.delay_cmd));
	save_item(NAME(m_tr_clocks_per_tick));
	save_item(NAME(m_tr_base_value));
	save_item(NAME(m_tr_base_cycles));
	save_item(NAME(m_timer_int_pending));
	save_item(NAME(m_clck_scale));
	save_item(NAME(m_clock_scale_mask));
	save_item(NAME(m_clock_cycles_1));
	save_item(NAME(m_clock_cycles_2));
	save_item(NAME(m_clock_cycles_4));
	save_item(NAME(m_clock_cycles_6));

	// set our instruction counter
	m_icountptr = &m_icount;
}

void e116t_device::device_start()
{
	init(0);
	m_opcodexor = 0;
}

void e116xt_device::device_start()
{
	init(3);
	m_opcodexor = 0;
}

void e116xs_device::device_start()
{
	init(7);
	m_opcodexor = 0;
}

void e116xsr_device::device_start()
{
	init(7);
	m_opcodexor = 0;
}

void gms30c2116_device::device_start()
{
	init(0);
	m_opcodexor = 0;
}

void gms30c2216_device::device_start()
{
	init(0);
	m_opcodexor = 0;
}

void e132n_device::device_start()
{
	init(0);
	m_opcodexor = WORD_XOR_BE(0);
}

void e132t_device::device_start()
{
	init(0);
	m_opcodexor = WORD_XOR_BE(0);
}

void e132xn_device::device_start()
{
	init(3);
	m_opcodexor = WORD_XOR_BE(0);
}

void e132xt_device::device_start()
{
	init(3);
	m_opcodexor = WORD_XOR_BE(0);
}

void e132xs_device::device_start()
{
	init(7);
	m_opcodexor = WORD_XOR_BE(0);
}

void e132xsr_device::device_start()
{
	init(7);
	m_opcodexor = WORD_XOR_BE(0);
}

void gms30c2132_device::device_start()
{
	init(0);
	m_opcodexor = WORD_XOR_BE(0);
}

void gms30c2232_device::device_start()
{
	init(0);
	m_opcodexor = WORD_XOR_BE(0);
}

void hyperstone_device::device_reset()
{
	//TODO: Add different reset initializations for BCR, MCR, FCR, TPR

	m_program = &space(AS_PROGRAM);
	m_direct = &m_program->direct();
	m_io = &space(AS_IO);

	m_tr_clocks_per_tick = 2;

	hyperstone_set_trap_entry(E132XS_ENTRY_MEM3); /* default entry point @ MEM3 */

	set_global_register(BCR_REGISTER, ~0);
	set_global_register(MCR_REGISTER, ~0);
	set_global_register(FCR_REGISTER, ~0);
	set_global_register(TPR_REGISTER, 0xc000000);

	PC = get_trap_addr(TRAPNO_RESET);

	SET_FP(0);
	SET_FL(2);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	SET_L_REG(0, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(1, SR);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::device_stop()
{
	// nothing to do
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or NULL if
//  the space doesn't exist
//-------------------------------------------------

const address_space_config *hyperstone_device::memory_space_config(address_spacenum spacenum) const
{
	if (spacenum == AS_PROGRAM)
	{
		return &m_program_config;
	}
	else if (spacenum == AS_IO)
	{
		return &m_io_config;
	}
	return NULL;
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void hyperstone_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			strprintf(str, "%c%c%c%c%c%c%c%c%c%c%c%c FTE:%X FRM:%X ILC:%d FL:%d FP:%d",
				GET_S ? 'S':'.',
				GET_P ? 'P':'.',
				GET_T ? 'T':'.',
				GET_L ? 'L':'.',
				GET_I ? 'I':'.',
				m_global_regs[1] & 0x00040 ? '?':'.',
				GET_H ? 'H':'.',
				GET_M ? 'M':'.',
				GET_V ? 'V':'.',
				GET_N ? 'N':'.',
				GET_Z ? 'Z':'.',
				GET_C ? 'C':'.',
				GET_FTE,
				GET_FRM,
				GET_ILC,
				GET_FL,
				GET_FP);
			break;
	}
}


//-------------------------------------------------
//  disasm_min_opcode_bytes - return the length
//  of the shortest instruction, in bytes
//-------------------------------------------------

UINT32 hyperstone_device::disasm_min_opcode_bytes() const
{
	return 2;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

UINT32 hyperstone_device::disasm_max_opcode_bytes() const
{
	return 6;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t hyperstone_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE( hyperstone );
	return dasm_hyperstone( buffer, pc, oprom, GET_H, GET_FP );
}

/* Opcodes */

void hyperstone_device::hyperstone_chk(struct hyperstone_device::regs_decode *decode)
{
	UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);

	if( SRC_IS_SR )
	{
		if( DREG == 0 )
			execute_exception(addr);
	}
	else
	{
		if( SRC_IS_PC )
		{
			if( DREG >= SREG )
				execute_exception(addr);
		}
		else
		{
			if( DREG > SREG )
				execute_exception(addr);
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_movd(struct hyperstone_device::regs_decode *decode)
{
	if( DST_IS_PC ) // Rd denotes PC
	{
		// RET instruction

		UINT8 old_s, old_l;
		INT8 difference; // really it's 7 bits

		if( SRC_IS_PC || SRC_IS_SR )
		{
			DEBUG_PRINTF(("Denoted PC or SR in RET instruction. PC = %08X\n", PC));
		}
		else
		{
			old_s = GET_S;
			old_l = GET_L;
			PPC = PC;

			SET_PC(SREG);
			SR = (SREGF & 0xffe00000) | ((SREG & 0x01) << 18 ) | (SREGF & 0x3ffff);
			if (m_intblock < 1)
				m_intblock = 1;

			m_instruction_length = 0; // undefined

			if( (!old_s && GET_S) || (!GET_S && !old_l && GET_L))
			{
				UINT32 addr = get_trap_addr(TRAPNO_PRIVILEGE_ERROR);
				execute_exception(addr);
			}

			difference = GET_FP - ((SP & 0x1fc) >> 2);

			/* convert to 8 bits */
			if(difference > 63)
				difference = (INT8)(difference|0x80);
			else if( difference < -64 )
				difference = difference & 0x7f;

			if( difference < 0 ) //else it's finished
			{
				do
				{
					SP -= 4;
					SET_ABS_L_REG(((SP & 0xfc) >> 2), READ_W(SP));
					difference++;

				} while(difference != 0);
			}
		}

		//TODO: no 1!
		m_icount -= m_clock_cycles_1;
	}
	else if( SRC_IS_SR ) // Rd doesn't denote PC and Rs denotes SR
	{
		SET_DREG(0);
		SET_DREGF(0);
		SET_Z(1);
		SET_N(0);

		m_icount -= m_clock_cycles_2;
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		UINT64 tmp;

		SET_DREG(SREG);
		SET_DREGF(SREGF);

		tmp = CONCAT_64(SREG, SREGF);
		SET_Z( tmp == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(SREG) );

		m_icount -= m_clock_cycles_2;
	}
}

void hyperstone_device::hyperstone_divu(struct hyperstone_device::regs_decode *decode)
{
	if( SAME_SRC_DST || SAME_SRC_DSTF )
	{
		DEBUG_PRINTF(("Denoted the same register code in hyperstone_divu instruction. PC = %08X\n", PC));
	}
	else
	{
		if( SRC_IS_PC || SRC_IS_SR )
		{
			DEBUG_PRINTF(("Denoted PC or SR as source register in hyperstone_divu instruction. PC = %08X\n", PC));
		}
		else
		{
			UINT64 dividend;

			dividend = CONCAT_64(DREG, DREGF);

			if( SREG == 0 )
			{
				//Rd//Rdf -> undefined
				//Z -> undefined
				//N -> undefined
				UINT32 addr;
				SET_V(1);
				addr = get_trap_addr(TRAPNO_RANGE_ERROR);
				execute_exception(addr);
			}
			else
			{
				UINT32 quotient, remainder;

				/* TODO: add quotient overflow */
				quotient = dividend / SREG;
				remainder = dividend % SREG;

				SET_DREG(remainder);
				SET_DREGF(quotient);

				SET_Z( quotient == 0 ? 1 : 0 );
				SET_N( SIGN_BIT(quotient) );
				SET_V(0);
			}
		}
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs(struct hyperstone_device::regs_decode *decode)
{
	if( SAME_SRC_DST || SAME_SRC_DSTF )
	{
		DEBUG_PRINTF(("Denoted the same register code in hyperstone_divs instruction. PC = %08X\n", PC));
	}
	else
	{
		if( SRC_IS_PC || SRC_IS_SR )
		{
			DEBUG_PRINTF(("Denoted PC or SR as source register in hyperstone_divs instruction. PC = %08X\n", PC));
		}
		else
		{
			INT64 dividend;

			dividend = (INT64) CONCAT_64(DREG, DREGF);

			if( SREG == 0 || (DREG & 0x80000000) )
			{
				//Rd//Rdf -> undefined
				//Z -> undefined
				//N -> undefined
				UINT32 addr;
				SET_V(1);
				addr = get_trap_addr(TRAPNO_RANGE_ERROR);
				execute_exception(addr);
			}
			else
			{
				INT32 quotient, remainder;

				/* TODO: add quotient overflow */
				quotient = dividend / ((INT32)(SREG));
				remainder = dividend % ((INT32)(SREG));

				SET_DREG(remainder);
				SET_DREGF(quotient);

				SET_Z( quotient == 0 ? 1 : 0 );
				SET_N( SIGN_BIT(quotient) );
				SET_V(0);
			}
		}
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_xm(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR || DST_IS_SR || DST_IS_PC )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_xm. PC = %08X\n", PC));
	}
	else
	{
		switch( decode->sub_type ) // x_code
		{
			case 0:
			case 1:
			case 2:
			case 3:
				if( !SRC_IS_PC && (SREG > EXTRA_U) )
				{
					UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
					execute_exception(addr);
				}
				else if( SRC_IS_PC && (SREG >= EXTRA_U) )
				{
					UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
					execute_exception(addr);
				}
				else
				{
					SREG <<= decode->sub_type;
				}

				break;

			case 4:
			case 5:
			case 6:
			case 7:
				decode->sub_type -= 4;
				SREG <<= decode->sub_type;

				break;
		}

		SET_DREG(SREG);
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mask(struct hyperstone_device::regs_decode *decode)
{
	DREG = SREG & EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (UINT64)(SREG) + (UINT64)(EXTRA_U);
	CHECK_C(tmp);
	CHECK_VADD(SREG,EXTRA_U,tmp);

	DREG = SREG + EXTRA_U;

	SET_DREG(DREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( DREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sums(struct hyperstone_device::regs_decode *decode)
{
	INT32 res;
	INT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (INT64)((INT32)(SREG)) + (INT64)(EXTRA_S);
	CHECK_VADD(SREG,EXTRA_S,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	res = (INT32)(SREG) + EXTRA_S;

	SET_DREG(res);

	SET_Z( res == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(res) );

	m_icount -= m_clock_cycles_1;

	if( GET_V && !SRC_IS_SR )
	{
		UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_cmp(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	if( DREG == SREG )
		SET_Z(1);
	else
		SET_Z(0);

	if( (INT32) DREG < (INT32) SREG )
		SET_N(1);
	else
		SET_N(0);

	tmp = (UINT64)(DREG) - (UINT64)(SREG);
	CHECK_VSUB(SREG,DREG,tmp);

	if( DREG < SREG )
		SET_C(1);
	else
		SET_C(0);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mov(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_S && decode->dst >= 16 )
	{
		UINT32 addr = get_trap_addr(TRAPNO_PRIVILEGE_ERROR);
		execute_exception(addr);
	}

	SET_DREG(SREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( SREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(SREG) );

	m_icount -= m_clock_cycles_1;
}


void hyperstone_device::hyperstone_add(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (UINT64)(SREG) + (UINT64)(DREG);
	CHECK_C(tmp);
	CHECK_VADD(SREG,DREG,tmp);

	DREG = SREG + DREG;
	SET_DREG(DREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( DREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_adds(struct hyperstone_device::regs_decode *decode)
{
	INT32 res;
	INT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (INT64)((INT32)(SREG)) + (INT64)((INT32)(DREG));

	CHECK_VADD(SREG,DREG,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	res = (INT32)(SREG) + (INT32)(DREG);

	SET_DREG(res);
	SET_Z( res == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(res) );

	m_icount -= m_clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_cmpb(struct hyperstone_device::regs_decode *decode)
{
	SET_Z( (DREG & SREG) == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andn(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG & ~SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_or(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG | SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xor(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG ^ SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subc(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
	{
		tmp = (UINT64)(DREG) - (UINT64)(GET_C);
		CHECK_VSUB(GET_C,DREG,tmp);
	}
	else
	{
		tmp = (UINT64)(DREG) - ((UINT64)(SREG) + (UINT64)(GET_C));
		//CHECK!
		CHECK_VSUB(SREG + GET_C,DREG,tmp);
	}


	if( SRC_IS_SR )
	{
		DREG = DREG - GET_C;
	}
	else
	{
		DREG = DREG - (SREG + GET_C);
	}

	CHECK_C(tmp);

	SET_DREG(DREG);

	SET_Z( GET_Z & (DREG == 0 ? 1 : 0) );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_not(struct hyperstone_device::regs_decode *decode)
{
	SET_DREG(~SREG);
	SET_Z( ~SREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (UINT64)(DREG) - (UINT64)(SREG);
	CHECK_C(tmp);
	CHECK_VSUB(SREG,DREG,tmp);

	DREG = DREG - SREG;
	SET_DREG(DREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( DREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subs(struct hyperstone_device::regs_decode *decode)
{
	INT32 res;
	INT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = (INT64)((INT32)(DREG)) - (INT64)((INT32)(SREG));

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	CHECK_VSUB(SREG,DREG,tmp);

	res = (INT32)(DREG) - (INT32)(SREG);

	SET_DREG(res);

	SET_Z( res == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(res) );

	m_icount -= m_clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_addc(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
	{
		tmp = (UINT64)(DREG) + (UINT64)(GET_C);
		CHECK_VADD(DREG,GET_C,tmp);
	}
	else
	{
		tmp = (UINT64)(SREG) + (UINT64)(DREG) + (UINT64)(GET_C);

		//CHECK!
		//CHECK_VADD1: V = (DREG == 0x7FFF) && (C == 1);
		//OVERFLOW = CHECK_VADD1(DREG, C, DREG+C) | CHECK_VADD(SREG, DREG+C, SREG+DREG+C)
		/* check if DREG + GET_C overflows */
//      if( (DREG == 0x7FFFFFFF) && (GET_C == 1) )
//          SET_V(1);
//      else
//          CHECK_VADD(SREG,DREG + GET_C,tmp);

		CHECK_VADD3(SREG,DREG,GET_C,tmp);
	}



	if( SRC_IS_SR )
		DREG = DREG + GET_C;
	else
		DREG = SREG + DREG + GET_C;

	CHECK_C(tmp);

	SET_DREG(DREG);
	SET_Z( GET_Z & (DREG == 0 ? 1 : 0) );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_and(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG & SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = -(UINT64)(SREG);
	CHECK_C(tmp);
	CHECK_VSUB(SREG,0,tmp);

	DREG = -SREG;

	SET_DREG(DREG);

	SET_Z( DREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_negs(struct hyperstone_device::regs_decode *decode)
{
	INT32 res;
	INT64 tmp;

	if( SRC_IS_SR )
		SREG = GET_C;

	tmp = -(INT64)((INT32)(SREG));
	CHECK_VSUB(SREG,0,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	res = -(INT32)(SREG);

	SET_DREG(res);

	SET_Z( res == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(res) );


	m_icount -= m_clock_cycles_1;

	if( GET_V && !SRC_IS_SR ) //trap doesn't occur when source is SR
	{
		UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_cmpi(struct hyperstone_device::regs_decode *decode)
{
	UINT64 tmp;

	tmp = (UINT64)(DREG) - (UINT64)(EXTRA_U);
	CHECK_VSUB(EXTRA_U,DREG,tmp);

	if( DREG == EXTRA_U )
		SET_Z(1);
	else
		SET_Z(0);

	if( (INT32) DREG < (INT32) EXTRA_U )
		SET_N(1);
	else
		SET_N(0);

	if( DREG < EXTRA_U )
		SET_C(1);
	else
		SET_C(0);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_movi(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_S && decode->dst >= 16 )
	{
		UINT32 addr = get_trap_addr(TRAPNO_PRIVILEGE_ERROR);
		execute_exception(addr);
	}

	SET_DREG(EXTRA_U);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( EXTRA_U == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(EXTRA_U) );

#if MISSIONCRAFT_FLAGS
	SET_V(0); // or V undefined ?
#endif

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_addi(struct hyperstone_device::regs_decode *decode)
{
	UINT32 imm;
	UINT64 tmp;

	if( N_VALUE )
		imm = EXTRA_U;
	else
		imm = GET_C & ((GET_Z == 0 ? 1 : 0) | (DREG & 0x01));


	tmp = (UINT64)(imm) + (UINT64)(DREG);
	CHECK_C(tmp);
	CHECK_VADD(imm,DREG,tmp);

	DREG = imm + DREG;
	SET_DREG(DREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( DREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(DREG) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_addsi(struct hyperstone_device::regs_decode *decode)
{
	INT32 imm, res;
	INT64 tmp;

	if( N_VALUE )
		imm = EXTRA_S;
	else
		imm = GET_C & ((GET_Z == 0 ? 1 : 0) | (DREG & 0x01));

	tmp = (INT64)(imm) + (INT64)((INT32)(DREG));
	CHECK_VADD(imm,DREG,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	res = imm + (INT32)(DREG);

	SET_DREG(res);

	SET_Z( res == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(res) );

	m_icount -= m_clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_cmpbi(struct hyperstone_device::regs_decode *decode)
{
	UINT32 imm;

	if( N_VALUE )
	{
		if( N_VALUE == 31 )
		{
			imm = 0x7fffffff; // bit 31 = 0, others = 1
		}
		else
		{
			imm = EXTRA_U;
		}

		SET_Z( (DREG & imm) == 0 ? 1 : 0 );
	}
	else
	{
		if( (DREG & 0xff000000) == 0 || (DREG & 0x00ff0000) == 0 ||
			(DREG & 0x0000ff00) == 0 || (DREG & 0x000000ff) == 0 )
			SET_Z(1);
		else
			SET_Z(0);
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_andni(struct hyperstone_device::regs_decode *decode)
{
	UINT32 imm;

	if( N_VALUE == 31 )
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = EXTRA_U;

	DREG = DREG & ~imm;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ori(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG | EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_xori(struct hyperstone_device::regs_decode *decode)
{
	DREG = DREG ^ EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shrdi(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	UINT64 val;

	high_order = DREG;
	low_order  = DREGF;

	val = CONCAT_64(high_order, low_order);

	if( N_VALUE )
		SET_C((val >> (N_VALUE - 1)) & 1);
	else
		SET_C(0);

	val >>= N_VALUE;

	high_order = EXTRACT_64HI(val);
	low_order  = EXTRACT_64LO(val);

	SET_DREG(high_order);
	SET_DREGF(low_order);
	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(high_order) );

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shrd(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	UINT64 val;
	UINT8 n = SREG & 0x1f;

	// result undefined if Ls denotes the same register as Ld or Ldf
	if( SAME_SRC_DST || SAME_SRC_DSTF )
	{
		DEBUG_PRINTF(("Denoted same registers in hyperstone_shrd. PC = %08X\n", PC));
	}
	else
	{
		high_order = DREG;
		low_order  = DREGF;

		val = CONCAT_64(high_order, low_order);

		if( n )
			SET_C((val >> (n - 1)) & 1);
		else
			SET_C(0);

		val >>= n;

		high_order = EXTRACT_64HI(val);
		low_order  = EXTRACT_64LO(val);

		SET_DREG(high_order);
		SET_DREGF(low_order);

		SET_Z( val == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(high_order) );
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shr(struct hyperstone_device::regs_decode *decode)
{
	UINT32 ret;
	UINT8 n;

	n = SREG & 0x1f;
	ret = DREG;

	if( n )
		SET_C((ret >> (n - 1)) & 1);
	else
		SET_C(0);

	ret >>= n;

	SET_DREG(ret);
	SET_Z( ret == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(ret) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sardi(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	UINT64 val;
	UINT8 sign_bit;

	high_order = DREG;
	low_order  = DREGF;

	val = CONCAT_64(high_order, low_order);

	if( N_VALUE )
		SET_C((val >> (N_VALUE - 1)) & 1);
	else
		SET_C(0);

	sign_bit = val >> 63;
	val >>= N_VALUE;

	if( sign_bit )
	{
		int i;
		for( i = 0; i < N_VALUE; i++ )
		{
			val |= (U64(0x8000000000000000) >> i);
		}
	}

	high_order = val >> 32;
	low_order  = val & 0xffffffff;

	SET_DREG(high_order);
	SET_DREGF(low_order);

	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(high_order) );

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_sard(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	UINT64 val;
	UINT8 n, sign_bit;

	n = SREG & 0x1f;

	// result undefined if Ls denotes the same register as Ld or Ldf
	if( SAME_SRC_DST || SAME_SRC_DSTF )
	{
		DEBUG_PRINTF(("Denoted same registers in hyperstone_sard. PC = %08X\n", PC));
	}
	else
	{
		high_order = DREG;
		low_order  = DREGF;

		val = CONCAT_64(high_order, low_order);

		if( n )
			SET_C((val >> (n - 1)) & 1);
		else
			SET_C(0);

		sign_bit = val >> 63;

		val >>= n;

		if( sign_bit )
		{
			int i;
			for( i = 0; i < n; i++ )
			{
				val |= (U64(0x8000000000000000) >> i);
			}
		}

		high_order = val >> 32;
		low_order  = val & 0xffffffff;

		SET_DREG(high_order);
		SET_DREGF(low_order);
		SET_Z( val == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(high_order) );
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_sar(struct hyperstone_device::regs_decode *decode)
{
	UINT32 ret;
	UINT8 n, sign_bit;

	n = SREG & 0x1f;
	ret = DREG;
	sign_bit = (ret & 0x80000000) >> 31;

	if( n )
		SET_C((ret >> (n - 1)) & 1);
	else
		SET_C(0);

	ret >>= n;

	if( sign_bit )
	{
		int i;
		for( i = 0; i < n; i++ )
		{
			ret |= (0x80000000 >> i);
		}
	}

	SET_DREG(ret);
	SET_Z( ret == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(ret) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shldi(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order, tmp;
	UINT64 val, mask;

	high_order = DREG;
	low_order  = DREGF;

	val  = CONCAT_64(high_order, low_order);
	SET_C( (N_VALUE)?(((val<<(N_VALUE-1))&U64(0x8000000000000000))?1:0):0);
	mask = ((((UINT64)1) << (32 - N_VALUE)) - 1) ^ 0xffffffff;
	tmp  = high_order << N_VALUE;

	if( ((high_order & mask) && (!(tmp & 0x80000000))) ||
			(((high_order & mask) ^ mask) && (tmp & 0x80000000)) )
		SET_V(1);
	else
		SET_V(0);

	val <<= N_VALUE;

	high_order = EXTRACT_64HI(val);
	low_order  = EXTRACT_64LO(val);

	SET_DREG(high_order);
	SET_DREGF(low_order);

	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(high_order) );

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shld(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order, tmp, n;
	UINT64 val, mask;

	n = SREG & 0x1f;

	// result undefined if Ls denotes the same register as Ld or Ldf
	if( SAME_SRC_DST || SAME_SRC_DSTF )
	{
		DEBUG_PRINTF(("Denoted same registers in hyperstone_shld. PC = %08X\n", PC));
	}
	else
	{
		high_order = DREG;
		low_order  = DREGF;

		mask = ((((UINT64)1) << (32 - n)) - 1) ^ 0xffffffff;

		val = CONCAT_64(high_order, low_order);
		SET_C( (n)?(((val<<(n-1))&U64(0x8000000000000000))?1:0):0);
		tmp = high_order << n;

		if( ((high_order & mask) && (!(tmp & 0x80000000))) ||
				(((high_order & mask) ^ mask) && (tmp & 0x80000000)) )
			SET_V(1);
		else
			SET_V(0);

		val <<= n;

		high_order = EXTRACT_64HI(val);
		low_order  = EXTRACT_64LO(val);

		SET_DREG(high_order);
		SET_DREGF(low_order);

		SET_Z( val == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(high_order) );
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shl(struct hyperstone_device::regs_decode *decode)
{
	UINT32 base, ret, n;
	UINT64 mask;

	n    = SREG & 0x1f;
	base = DREG;
	mask = ((((UINT64)1) << (32 - n)) - 1) ^ 0xffffffff;
	SET_C( (n)?(((base<<(n-1))&0x80000000)?1:0):0);
	ret  = base << n;

	if( ((base & mask) && (!(ret & 0x80000000))) ||
			(((base & mask) ^ mask) && (ret & 0x80000000)) )
		SET_V(1);
	else
		SET_V(0);

	SET_DREG(ret);
	SET_Z( ret == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(ret) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::reserved(struct hyperstone_device::regs_decode *decode)
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

void hyperstone_device::hyperstone_testlz(struct hyperstone_device::regs_decode *decode)
{
	UINT8 zeros = 0;
	UINT32 mask;

	for( mask = 0x80000000; ; mask >>= 1 )
	{
		if( SREG & mask )
			break;
		else
			zeros++;

		if( zeros == 32 )
			break;
	}

	SET_DREG(zeros);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_rol(struct hyperstone_device::regs_decode *decode)
{
	UINT32 val, base;
	UINT8 n;
	UINT64 mask;

	n = SREG & 0x1f;

	val = base = DREG;

	mask = ((((UINT64)1) << (32 - n)) - 1) ^ 0xffffffff;

	while( n > 0 )
	{
		val = (val << 1) | ((val & 0x80000000) >> 31);
		n--;
	}

#ifdef MISSIONCRAFT_FLAGS

	if( ((base & mask) && (!(val & 0x80000000))) ||
			(((base & mask) ^ mask) && (val & 0x80000000)) )
		SET_V(1);
	else
		SET_V(0);

#endif

	SET_DREG(val);

	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(val) );

	m_icount -= m_clock_cycles_1;
}

//TODO: add trap error
void hyperstone_device::hyperstone_ldxx1(struct hyperstone_device::regs_decode *decode)
{
	UINT32 load;

	if( DST_IS_SR )
	{
		switch( decode->sub_type )
		{
			case 0: // LDBS.A

				load = READ_B(EXTRA_S);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				break;

			case 1: // LDBU.A

				load = READ_B(EXTRA_S);
				SET_SREG(load);

				break;

			case 2:

				load = READ_HW(EXTRA_S);

				if( EXTRA_S & 1 ) // LDHS.A
				{
					load |= (load & 0x8000) ? 0xffff0000 : 0;
				}
				/*
				else          // LDHU.A
				{
				    // nothing more
				}
				*/

				SET_SREG(load);

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // LDD.IOA
				{
					load = IO_READ_W(EXTRA_S & ~3);
					SET_SREG(load);

					load = IO_READ_W((EXTRA_S & ~3) + 4);
					SET_SREGF(load);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // LDW.IOA
				{
					load = IO_READ_W(EXTRA_S & ~3);
					SET_SREG(load);
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.A
				{
					load = READ_W(EXTRA_S & ~1);
					SET_SREG(load);

					load = READ_W((EXTRA_S & ~1) + 4);
					SET_SREGF(load);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // LDW.A
				{
					load = READ_W(EXTRA_S & ~1);
					SET_SREG(load);
				}

				break;
		}
	}
	else
	{
		switch( decode->sub_type )
		{
			case 0: // LDBS.D

				load = READ_B(DREG + EXTRA_S);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				break;

			case 1: // LDBU.D

				load = READ_B(DREG + EXTRA_S);
				SET_SREG(load);

				break;

			case 2:

				load = READ_HW(DREG + (EXTRA_S & ~1));

				if( EXTRA_S & 1 ) // LDHS.D
				{
					load |= (load & 0x8000) ? 0xffff0000 : 0;
				}
				/*
				else          // LDHU.D
				{
				    // nothing more
				}
				*/

				SET_SREG(load);

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // LDD.IOD
				{
					load = IO_READ_W(DREG + (EXTRA_S & ~3));
					SET_SREG(load);

					load = IO_READ_W(DREG + (EXTRA_S & ~3) + 4);
					SET_SREGF(load);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // LDW.IOD
				{
					load = IO_READ_W(DREG + (EXTRA_S & ~3));
					SET_SREG(load);
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.D
				{
					load = READ_W(DREG + (EXTRA_S & ~1));
					SET_SREG(load);

					load = READ_W(DREG + (EXTRA_S & ~1) + 4);
					SET_SREGF(load);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // LDW.D
				{
					load = READ_W(DREG + (EXTRA_S & ~1));
					SET_SREG(load);
				}

				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2(struct hyperstone_device::regs_decode *decode)
{
	UINT32 load;

	if( DST_IS_PC || DST_IS_SR )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", PC));
	}
	else
	{
		switch( decode->sub_type )
		{
			case 0: // LDBS.N

				if(SAME_SRC_DST)
					DEBUG_PRINTF(("LDBS.N denoted same regs @ %08X",PPC));

				load = READ_B(DREG);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				if(!SAME_SRC_DST)
					SET_DREG(DREG + EXTRA_S);

				break;

			case 1: // LDBU.N

				if(SAME_SRC_DST)
					DEBUG_PRINTF(("LDBU.N denoted same regs @ %08X",PPC));

				load = READ_B(DREG);
				SET_SREG(load);

				if(!SAME_SRC_DST)
					SET_DREG(DREG + EXTRA_S);

				break;

			case 2:

				load = READ_HW(DREG);

				if( EXTRA_S & 1 ) // LDHS.N
				{
					load |= (load & 0x8000) ? 0xffff0000 : 0;

					if(SAME_SRC_DST)
						DEBUG_PRINTF(("LDHS.N denoted same regs @ %08X",PPC));
				}
				/*
				else          // LDHU.N
				{
				    // nothing more
				}
				*/

				SET_SREG(load);

				if(!SAME_SRC_DST)
					SET_DREG(DREG + (EXTRA_S & ~1));

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // LDW.S
				{
					if(SAME_SRC_DST)
						DEBUG_PRINTF(("LDW.S denoted same regs @ %08X",PPC));

					if(DREG < SP)
						SET_SREG(READ_W(DREG));
					else
						SET_SREG(GET_ABS_L_REG((DREG & 0xfc) >> 2));

					if(!SAME_SRC_DST)
						SET_DREG(DREG + (EXTRA_S & ~3));

					m_icount -= m_clock_cycles_2; // extra cycles
				}
				else if( (EXTRA_S & 3) == 2 ) // Reserved
				{
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.N
				{
					if(SAME_SRC_DST || SAME_SRCF_DST)
						DEBUG_PRINTF(("LDD.N denoted same regs @ %08X",PPC));

					load = READ_W(DREG);
					SET_SREG(load);

					load = READ_W(DREG + 4);
					SET_SREGF(load);

					if(!SAME_SRC_DST && !SAME_SRCF_DST)
						SET_DREG(DREG + (EXTRA_S & ~1));

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // LDW.N
				{
					if(SAME_SRC_DST)
						DEBUG_PRINTF(("LDW.N denoted same regs @ %08X",PPC));

					load = READ_W(DREG);
					SET_SREG(load);

					if(!SAME_SRC_DST)
						SET_DREG(DREG + (EXTRA_S & ~1));
				}

				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

//TODO: add trap error
void hyperstone_device::hyperstone_stxx1(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	if( DST_IS_SR )
	{
		switch( decode->sub_type )
		{
			case 0: // STBS.A

				/* TODO: missing trap on range error */
				WRITE_B(EXTRA_S, SREG & 0xff);

				break;

			case 1: // STBU.A

				WRITE_B(EXTRA_S, SREG & 0xff);

				break;

			case 2:

				WRITE_HW(EXTRA_S, SREG & 0xffff);

				/*
				if( EXTRA_S & 1 ) // STHS.A
				{
				    // TODO: missing trap on range error
				}
				else          // STHU.A
				{
				    // nothing more
				}
				*/

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // STD.IOA
				{
					IO_WRITE_W(EXTRA_S & ~3, SREG);
					IO_WRITE_W((EXTRA_S & ~3) + 4, SREGF);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // STW.IOA
				{
					IO_WRITE_W(EXTRA_S & ~3, SREG);
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.A
				{
					WRITE_W(EXTRA_S & ~1, SREG);
					WRITE_W((EXTRA_S & ~1) + 4, SREGF);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // STW.A
				{
					WRITE_W(EXTRA_S & ~1, SREG);
				}

				break;
		}
	}
	else
	{
		switch( decode->sub_type )
		{
			case 0: // STBS.D

				/* TODO: missing trap on range error */
				WRITE_B(DREG + EXTRA_S, SREG & 0xff);

				break;

			case 1: // STBU.D

				WRITE_B(DREG + EXTRA_S, SREG & 0xff);

				break;

			case 2:

				WRITE_HW(DREG + (EXTRA_S & ~1), SREG & 0xffff);

				/*
				if( EXTRA_S & 1 ) // STHS.D
				{
				    // TODO: missing trap on range error
				}
				else          // STHU.D
				{
				    // nothing more
				}
				*/

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // STD.IOD
				{
					IO_WRITE_W(DREG + (EXTRA_S & ~3), SREG);
					IO_WRITE_W(DREG + (EXTRA_S & ~3) + 4, SREGF);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // STW.IOD
				{
					IO_WRITE_W(DREG + (EXTRA_S & ~3), SREG);
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.D
				{
					WRITE_W(DREG + (EXTRA_S & ~1), SREG);
					WRITE_W(DREG + (EXTRA_S & ~1) + 4, SREGF);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // STW.D
				{
					WRITE_W(DREG + (EXTRA_S & ~1), SREG);
				}

				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stxx2(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	if( DST_IS_PC || DST_IS_SR )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_stxx2. PC = %08X\n", PC));
	}
	else
	{
		switch( decode->sub_type )
		{
			case 0: // STBS.N

				/* TODO: missing trap on range error */
				WRITE_B(DREG, SREG & 0xff);
				SET_DREG(DREG + EXTRA_S);

				break;

			case 1: // STBU.N

				WRITE_B(DREG, SREG & 0xff);
				SET_DREG(DREG + EXTRA_S);

				break;

			case 2:

				WRITE_HW(DREG, SREG & 0xffff);
				SET_DREG(DREG + (EXTRA_S & ~1));

				/*
				if( EXTRA_S & 1 ) // STHS.N
				{
				    // TODO: missing trap on range error
				}
				else          // STHU.N
				{
				    // nothing more
				}
				*/

				break;

			case 3:

				if( (EXTRA_S & 3) == 3 )      // STW.S
				{
					if(DREG < SP)
						WRITE_W(DREG, SREG);
					else
					{
						if(((DREG & 0xfc) >> 2) == ((decode->src + GET_FP) % 64) && S_BIT == LOCAL)
							DEBUG_PRINTF(("STW.S denoted the same local register @ %08X\n",PPC));

						SET_ABS_L_REG((DREG & 0xfc) >> 2,SREG);
					}

					SET_DREG(DREG + (EXTRA_S & ~3));

					m_icount -= m_clock_cycles_2; // extra cycles

				}
				else if( (EXTRA_S & 3) == 2 ) // Reserved
				{
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.N
				{
					WRITE_W(DREG, SREG);
					SET_DREG(DREG + (EXTRA_S & ~1));

					if( SAME_SRCF_DST )
						WRITE_W(DREG + 4, SREGF + (EXTRA_S & ~1));  // because DREG == SREGF and DREG has been incremented
					else
						WRITE_W(DREG + 4, SREGF);

					m_icount -= m_clock_cycles_1; // extra cycle
				}
				else                      // STW.N
				{
					WRITE_W(DREG, SREG);
					SET_DREG(DREG + (EXTRA_S & ~1));
				}

				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shri(struct hyperstone_device::regs_decode *decode)
{
	UINT32 val;

	val = DREG;

	if( N_VALUE )
		SET_C((val >> (N_VALUE - 1)) & 1);
	else
		SET_C(0);

	val >>= N_VALUE;

	SET_DREG(val);
	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(val) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sari(struct hyperstone_device::regs_decode *decode)
{
	UINT32 val;
	UINT8 sign_bit;

	val = DREG;
	sign_bit = (val & 0x80000000) >> 31;

	if( N_VALUE )
		SET_C((val >> (N_VALUE - 1)) & 1);
	else
		SET_C(0);

	val >>= N_VALUE;

	if( sign_bit )
	{
		int i;
		for( i = 0; i < N_VALUE; i++ )
		{
			val |= (0x80000000 >> i);
		}
	}

	SET_DREG(val);
	SET_Z( val == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(val) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shli(struct hyperstone_device::regs_decode *decode)
{
	UINT32 val, val2;
	UINT64 mask;

	val  = DREG;
	SET_C( (N_VALUE)?(((val<<(N_VALUE-1))&0x80000000)?1:0):0);
	mask = ((((UINT64)1) << (32 - N_VALUE)) - 1) ^ 0xffffffff;
	val2 = val << N_VALUE;

	if( ((val & mask) && (!(val2 & 0x80000000))) ||
			(((val & mask) ^ mask) && (val2 & 0x80000000)) )
		SET_V(1);
	else
		SET_V(0);

	SET_DREG(val2);
	SET_Z( val2 == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(val2) );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mulu(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	UINT64 double_word;

	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mulu instruction. PC = %08X\n", PC));
	}
	else
	{
		double_word = (UINT64)SREG *(UINT64)DREG;

		low_order = double_word & 0xffffffff;
		high_order = double_word >> 32;

		SET_DREG(high_order);
		SET_DREGF(low_order);

		SET_Z( double_word == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(high_order) );
	}

	if(SREG <= 0xffff && DREG <= 0xffff)
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_muls(struct hyperstone_device::regs_decode *decode)
{
	UINT32 low_order, high_order;
	INT64 double_word;

	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_muls instruction. PC = %08X\n", PC));
	}
	else
	{
		double_word = (INT64)(INT32)(SREG) * (INT64)(INT32)(DREG);
		low_order = double_word & 0xffffffff;
		high_order = double_word >> 32;

		SET_DREG(high_order);
		SET_DREGF(low_order);

		SET_Z( double_word == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(high_order) );
	}

	if((SREG >= 0xffff8000 && SREG <= 0x7fff) && (DREG >= 0xffff8000 && DREG <= 0x7fff))
		m_icount -= m_clock_cycles_4;
	else
		m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_set(struct hyperstone_device::regs_decode *decode)
{
	int n = N_VALUE;

	if( DST_IS_PC )
	{
		DEBUG_PRINTF(("Denoted PC in hyperstone_set. PC = %08X\n", PC));
	}
	else if( DST_IS_SR )
	{
		//TODO: add fetch opcode when there's the pipeline

		//TODO: no 1!
		m_icount -= m_clock_cycles_1;
	}
	else
	{
		switch( n )
		{
			// SETADR
			case 0:
			{
				UINT32 val;
				val =  (SP & 0xfffffe00) | (GET_FP << 2);

				//plus carry into bit 9
				val += (( (SP & 0x100) && (SIGN_BIT(SR) == 0) ) ? 1 : 0);

				SET_DREG(val);

				break;
			}
			// Reserved
			case 1:
			case 16:
			case 17:
			case 19:
				DEBUG_PRINTF(("Used reserved N value (%d) in hyperstone_set. PC = %08X\n", n, PC));
				break;

			// SETxx
			case 2:
				SET_DREG(1);
				break;

			case 3:
				SET_DREG(0);
				break;

			case 4:
				if( GET_N || GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 5:
				if( !GET_N && !GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 6:
				if( GET_N )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 7:
				if( !GET_N )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 8:
				if( GET_C || GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 9:
				if( !GET_C && !GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 10:
				if( GET_C )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 11:
				if( !GET_C )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 12:
				if( GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 13:
				if( !GET_Z )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 14:
				if( GET_V )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 15:
				if( !GET_V )
				{
					SET_DREG(1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 18:
				SET_DREG(-1);
				break;

			case 20:
				if( GET_N || GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 21:
				if( !GET_N && !GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 22:
				if( GET_N )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 23:
				if( !GET_N )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 24:
				if( GET_C || GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 25:
				if( !GET_C && !GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 26:
				if( GET_C )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 27:
				if( !GET_C )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 28:
				if( GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 29:
				if( !GET_Z )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 30:
				if( GET_V )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;

			case 31:
				if( !GET_V )
				{
					SET_DREG(-1);
				}
				else
				{
					SET_DREG(0);
				}

				break;
		}

		m_icount -= m_clock_cycles_1;
	}
}

void hyperstone_device::hyperstone_mul(struct hyperstone_device::regs_decode *decode)
{
	UINT32 single_word;

	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", PC));
	}
	else
	{
		single_word = (SREG * DREG);// & 0xffffffff; // only the low-order word is taken

		SET_DREG(single_word);

		SET_Z( single_word == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(single_word) );
	}

	if((SREG >= 0xffff8000 && SREG <= 0x7fff) && (DREG >= 0xffff8000 && DREG <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}

void hyperstone_device::hyperstone_fadd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_faddd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fsub(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fsubd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fmul(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fmuld(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fdiv(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fdivd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcmp(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcmpd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcmpu(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcmpud(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcvt(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_fcvtd(struct hyperstone_device::regs_decode *decode)
{
	execute_software(decode);
	m_icount -= m_clock_cycles_6;
}

void hyperstone_device::hyperstone_extend(struct hyperstone_device::regs_decode *decode)
{
	//TODO: add locks, overflow error and other things
	UINT32 vals, vald;

	vals = SREG;
	vald = DREG;

	switch( EXTRA_U ) // extended opcode
	{
		// signed or unsigned multiplication, single word product
		case EMUL:
		case 0x100: // used in "N" type cpu
		{
			UINT32 result;

			result = vals * vald;
			SET_G_REG(15, result);

			break;
		}
		// unsigned multiplication, double word product
		case EMULU:
		{
			UINT64 result;

			result = (UINT64)vals * (UINT64)vald;
			vals = result >> 32;
			vald = result & 0xffffffff;
			SET_G_REG(14, vals);
			SET_G_REG(15, vald);

			break;
		}
		// signed multiplication, double word product
		case EMULS:
		{
			INT64 result;

			result = (INT64)(INT32)(vals) * (INT64)(INT32)(vald);
			vals = result >> 32;
			vald = result & 0xffffffff;
			SET_G_REG(14, vals);
			SET_G_REG(15, vald);

			break;
		}
		// signed multiply/add, single word product sum
		case EMAC:
		{
			INT32 result;

			result = (INT32)GET_G_REG(15) + ((INT32)(vals) * (INT32)(vald));
			SET_G_REG(15, result);

			break;
		}
		// signed multiply/add, double word product sum
		case EMACD:
		{
			INT64 result;

			result = (INT64)CONCAT_64(GET_G_REG(14), GET_G_REG(15)) + (INT64)((INT64)(INT32)(vals) * (INT64)(INT32)(vald));

			vals = result >> 32;
			vald = result & 0xffffffff;
			SET_G_REG(14, vals);
			SET_G_REG(15, vald);

			break;
		}
		// signed multiply/substract, single word product difference
		case EMSUB:
		{
			INT32 result;

			result = (INT32)GET_G_REG(15) - ((INT32)(vals) * (INT32)(vald));
			SET_G_REG(15, result);

			break;
		}
		// signed multiply/substract, double word product difference
		case EMSUBD:
		{
			INT64 result;

			result = (INT64)CONCAT_64(GET_G_REG(14), GET_G_REG(15)) - (INT64)((INT64)(INT32)(vals) * (INT64)(INT32)(vald));

			vals = result >> 32;
			vald = result & 0xffffffff;
			SET_G_REG(14, vals);
			SET_G_REG(15, vald);

			break;
		}
		// signed half-word multiply/add, single word product sum
		case EHMAC:
		{
			INT32 result;

			result = (INT32)GET_G_REG(15) + ((INT32)((vald & 0xffff0000) >> 16) * (INT32)((vals & 0xffff0000) >> 16)) + ((INT32)(vald & 0xffff) * (INT32)(vals & 0xffff));
			SET_G_REG(15, result);

			break;
		}
		// signed half-word multiply/add, double word product sum
		case EHMACD:
		{
			INT64 result;

			result = (INT64)CONCAT_64(GET_G_REG(14), GET_G_REG(15)) + (INT64)((INT64)(INT32)((vald & 0xffff0000) >> 16) * (INT64)(INT32)((vals & 0xffff0000) >> 16)) + ((INT64)(INT32)(vald & 0xffff) * (INT64)(INT32)(vals & 0xffff));

			vals = result >> 32;
			vald = result & 0xffffffff;
			SET_G_REG(14, vals);
			SET_G_REG(15, vald);

			break;
		}
		// half-word complex multiply
		case EHCMULD:
		{
			UINT32 result;

			result = (((vald & 0xffff0000) >> 16) * ((vals & 0xffff0000) >> 16)) - ((vald & 0xffff) * (vals & 0xffff));
			SET_G_REG(14, result);

			result = (((vald & 0xffff0000) >> 16) * (vals & 0xffff)) + ((vald & 0xffff) * ((vals & 0xffff0000) >> 16));
			SET_G_REG(15, result);

			break;
		}
		// half-word complex multiply/add
		case EHCMACD:
		{
			UINT32 result;

			result = GET_G_REG(14) + (((vald & 0xffff0000) >> 16) * ((vals & 0xffff0000) >> 16)) - ((vald & 0xffff) * (vals & 0xffff));
			SET_G_REG(14, result);

			result = GET_G_REG(15) + (((vald & 0xffff0000) >> 16) * (vals & 0xffff)) + ((vald & 0xffff) * ((vals & 0xffff0000) >> 16));
			SET_G_REG(15, result);

			break;
		}
		// half-word (complex) add/substract
		// Ls is not used and should denote the same register as Ld
		case EHCSUMD:
		{
			UINT32 result;

			result = ((((vals & 0xffff0000) >> 16) + GET_G_REG(14)) << 16) & 0xffff0000;
			result |= ((vals & 0xffff) + GET_G_REG(15)) & 0xffff;
			SET_G_REG(14, result);

			result = ((((vals & 0xffff0000) >> 16) - GET_G_REG(14)) << 16) & 0xffff0000;
			result |= ((vals & 0xffff) - GET_G_REG(15)) & 0xffff;
			SET_G_REG(15, result);

			break;
		}
		// half-word (complex) add/substract with fixed point adjustment
		// Ls is not used and should denote the same register as Ld
		case EHCFFTD:
		{
			UINT32 result;

			result = ((((vals & 0xffff0000) >> 16) + (GET_G_REG(14) >> 15)) << 16) & 0xffff0000;
			result |= ((vals & 0xffff) + (GET_G_REG(15) >> 15)) & 0xffff;
			SET_G_REG(14, result);

			result = ((((vals & 0xffff0000) >> 16) - (GET_G_REG(14) >> 15)) << 16) & 0xffff0000;
			result |= ((vals & 0xffff) - (GET_G_REG(15) >> 15)) & 0xffff;
			SET_G_REG(15, result);

			break;
		}
		// half-word (complex) add/substract with fixed point adjustment and shift
		// Ls is not used and should denote the same register as Ld
		case EHCFFTSD:
		{
			UINT32 result;

			result = (((((vals & 0xffff0000) >> 16) + (GET_G_REG(14) >> 15)) >> 1) << 16) & 0xffff0000;
			result |= ((((vals & 0xffff) + (GET_G_REG(15) >> 15)) >> 1) & 0xffff);
			SET_G_REG(14, result);

			result = (((((vals & 0xffff0000) >> 16) - (GET_G_REG(14) >> 15)) >> 1) << 16) & 0xffff0000;
			result |= ((((vals & 0xffff) - (GET_G_REG(15) >> 15)) >> 1) & 0xffff);
			SET_G_REG(15, result);

			break;
		}
		default:
			DEBUG_PRINTF(("Executed Illegal extended opcode (%X). PC = %08X\n", EXTRA_U, PC));
			break;
	}

	m_icount -= m_clock_cycles_1; //TODO: with the latency it can change
}

void hyperstone_device::hyperstone_do(struct hyperstone_device::regs_decode *decode)
{
	fatalerror("Executed hyperstone_do instruction. PC = %08X\n", PPC);
}

void hyperstone_device::hyperstone_ldwr(struct hyperstone_device::regs_decode *decode)
{
	SET_SREG(READ_W(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddr(struct hyperstone_device::regs_decode *decode)
{
	SET_SREG(READ_W(DREG));
	SET_SREGF(READ_W(DREG + 4));

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_ldwp(struct hyperstone_device::regs_decode *decode)
{
	SET_SREG(READ_W(DREG));

	// post increment the destination register if it's different from the source one
	// (needed by Hidden Catch)
	if(!(decode->src == decode->dst && S_BIT == LOCAL))
		SET_DREG(DREG + 4);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddp(struct hyperstone_device::regs_decode *decode)
{
	SET_SREG(READ_W(DREG));
	SET_SREGF(READ_W(DREG + 4));

	// post increment the destination register if it's different from the source one
	// and from the "next source" one
	if(!(decode->src == decode->dst && S_BIT == LOCAL) &&   !SAME_SRCF_DST )
	{
		SET_DREG(DREG + 8);
	}
	else
	{
		DEBUG_PRINTF(("LDD.P denoted same regs @ %08X",PPC));
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stwr(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	WRITE_W(DREG, SREG);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stdr(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(DREG, SREG);
	WRITE_W(DREG + 4, SREGF);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stwp(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	WRITE_W(DREG, SREG);
	SET_DREG(DREG + 4);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stdp(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(DREG, SREG);
	SET_DREG(DREG + 8);

	if( SAME_SRCF_DST )
		WRITE_W(DREG + 4, SREGF + 8); // because DREG == SREGF and DREG has been incremented
	else
		WRITE_W(DREG + 4, SREGF);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_dbv(struct hyperstone_device::regs_decode *decode)
{
	if( GET_V )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnv(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_V )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbe(struct hyperstone_device::regs_decode *decode) //or DBZ
{
	if( GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbne(struct hyperstone_device::regs_decode *decode) //or DBNZ
{
	if( !GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbc(struct hyperstone_device::regs_decode *decode) //or DBST
{
	if( GET_C )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnc(struct hyperstone_device::regs_decode *decode) //or DBHE
{
	if( !GET_C )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbse(struct hyperstone_device::regs_decode *decode)
{
	if( GET_C || GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbht(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_C && !GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbn(struct hyperstone_device::regs_decode *decode) //or DBLT
{
	if( GET_N )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbnn(struct hyperstone_device::regs_decode *decode) //or DBGE
{
	if( !GET_N )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dble(struct hyperstone_device::regs_decode *decode)
{
	if( GET_N || GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbgt(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_N && !GET_Z )
		execute_dbr(decode);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_dbr(struct hyperstone_device::regs_decode *decode)
{
	execute_dbr(decode);
}

void hyperstone_device::hyperstone_frame(struct hyperstone_device::regs_decode *decode)
{
	INT8 difference; // really it's 7 bits
	UINT8 realfp = GET_FP - SRC_CODE;

	SET_FP(realfp);
	SET_FL(DST_CODE);
	SET_M(0);

	difference = ((SP & 0x1fc) >> 2) + (64 - 10) - (realfp + GET_FL);

	/* convert to 8 bits */
	if(difference > 63)
		difference = (INT8)(difference|0x80);
	else if( difference < -64 )
		difference = difference & 0x7f;

	if( difference < 0 ) // else it's finished
	{
		UINT8 tmp_flag;

		tmp_flag = ( SP >= UB ? 1 : 0 );

		do
		{
			WRITE_W(SP, GET_ABS_L_REG((SP & 0xfc) >> 2));
			SP += 4;
			difference++;

		} while(difference != 0);

		if( tmp_flag )
		{
			UINT32 addr = get_trap_addr(TRAPNO_FRAME_ERROR);
			execute_exception(addr);
		}
	}

	//TODO: no 1!
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_call(struct hyperstone_device::regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	if( !DST_CODE )
		decode->dst = 16;

	EXTRA_S = (EXTRA_S & ~1) + SREG;

	SET_ILC(m_instruction_length & 3);

	SET_DREG((PC & 0xfffffffe) | GET_S);
	SET_DREGF(SR);

	SET_FP(GET_FP + decode->dst);

	SET_FL(6); //default value for call
	SET_M(0);

	PPC = PC;
	PC = EXTRA_S; // const value

	m_intblock = 2;

	//TODO: add interrupt locks, errors, ....

	//TODO: no 1!
	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bv(struct hyperstone_device::regs_decode *decode)
{
	if( GET_V )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bnv(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_V )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_be(struct hyperstone_device::regs_decode *decode) //or BZ
{
	if( GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bne(struct hyperstone_device::regs_decode *decode) //or BNZ
{
	if( !GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bc(struct hyperstone_device::regs_decode *decode) //or BST
{
	if( GET_C )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bnc(struct hyperstone_device::regs_decode *decode) //or BHE
{
	if( !GET_C )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bse(struct hyperstone_device::regs_decode *decode)
{
	if( GET_C || GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bht(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_C && !GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bn(struct hyperstone_device::regs_decode *decode) //or BLT
{
	if( GET_N )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bnn(struct hyperstone_device::regs_decode *decode) //or BGE
{
	if( !GET_N )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ble(struct hyperstone_device::regs_decode *decode)
{
	if( GET_N || GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_bgt(struct hyperstone_device::regs_decode *decode)
{
	if( !GET_N && !GET_Z )
		execute_br(decode);
	else
		m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_br(struct hyperstone_device::regs_decode *decode)
{
	execute_br(decode);
}

void hyperstone_device::hyperstone_trap(struct hyperstone_device::regs_decode *decode)
{
	UINT8 code, trapno;
	UINT32 addr;

	trapno = (OP & 0xfc) >> 2;

	addr = get_trap_addr(trapno);
	code = ((OP & 0x300) >> 6) | (OP & 0x03);

	switch( code )
	{
		case TRAPLE:
			if( GET_N || GET_Z )
				execute_trap(addr);

			break;

		case TRAPGT:
			if( !GET_N && !GET_Z )
				execute_trap(addr);

			break;

		case TRAPLT:
			if( GET_N )
				execute_trap(addr);

			break;

		case TRAPGE:
			if( !GET_N )
				execute_trap(addr);

			break;

		case TRAPSE:
			if( GET_C || GET_Z )
				execute_trap(addr);

			break;

		case TRAPHT:
			if( !GET_C && !GET_Z )
				execute_trap(addr);

			break;

		case TRAPST:
			if( GET_C )
				execute_trap(addr);

			break;

		case TRAPHE:
			if( !GET_C )
				execute_trap(addr);

			break;

		case TRAPE:
			if( GET_Z )
				execute_trap(addr);

			break;

		case TRAPNE:
			if( !GET_Z )
				execute_trap(addr);

			break;

		case TRAPV:
			if( GET_V )
				execute_trap(addr);

			break;

		case TRAP:
			execute_trap(addr);

			break;
	}

	m_icount -= m_clock_cycles_1;
}


#include "e132xsop.inc"

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 hyperstone_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

UINT32 hyperstone_device::execute_max_cycles() const
{
	return 36;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

UINT32 hyperstone_device::execute_input_lines() const
{
	return 8;
}


void hyperstone_device::execute_set_input(int inputnum, int state)
{
	if (state)
		ISR |= 1 << inputnum;
	else
		ISR &= ~(1 << inputnum);
}


//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void hyperstone_device::execute_run()
{
	if (m_intblock < 0)
		m_intblock = 0;

	check_interrupts();

	do
	{
		UINT32 oldh = SR & 0x00000020;

		PPC = PC;   /* copy PC to previous PC */
		debugger_instruction_hook(this, PC);

		OP = READ_OP(PC);
		PC += 2;

		m_instruction_length = 1;

		/* execute opcode */
		(this->*m_opcode[(OP & 0xff00) >> 8])();

		/* clear the H state if it was previously set */
		SR ^= oldh;

		SET_ILC(m_instruction_length & 3);

		if( GET_T && GET_P && m_delay.delay_cmd == NO_DELAY ) /* Not in a Delayed Branch instructions */
		{
			UINT32 addr = get_trap_addr(TRAPNO_TRACE_EXCEPTION);
			execute_exception(addr);
		}

		if (--m_intblock == 0)
			check_interrupts();

	} while( m_icount > 0 );
}

const device_type E116T = &device_creator<e116t_device>;
const device_type E116XT = &device_creator<e116xt_device>;
const device_type E116XS = &device_creator<e116xs_device>;
const device_type E116XSR = &device_creator<e116xsr_device>;
const device_type E132N = &device_creator<e132n_device>;
const device_type E132T = &device_creator<e132t_device>;
const device_type E132XN = &device_creator<e132xn_device>;
const device_type E132XT = &device_creator<e132xt_device>;
const device_type E132XS = &device_creator<e132xs_device>;
const device_type E132XSR = &device_creator<e132xsr_device>;
const device_type GMS30C2116 = &device_creator<gms30c2116_device>;
const device_type GMS30C2132 = &device_creator<gms30c2132_device>;
const device_type GMS30C2216 = &device_creator<gms30c2216_device>;
const device_type GMS30C2232 = &device_creator<gms30c2232_device>;
