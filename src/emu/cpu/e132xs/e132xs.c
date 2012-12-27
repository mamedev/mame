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

 MooglyGuy 29/03/2004
    - Changed MOVI to use unsigned values instead of signed, correcting
      an ugly glitch when loading 32-bit immediates.
 Pierpaolo Prazzoli
    - Same fix in get_const

 MooglyGuy - 02/27/04
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

 MooglyGuy - 10/25/03
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

 MooglyGuy - 08/20/03
    - Added H flag support for MOV and MOVI
    - Changed init routine to set S flag on boot. Apparently the CPU defaults to
      supervisor mode as opposed to user mode when it powers on, as shown by the
      vamphalf power-on routines. Makes sense, too, since if the machine booted
      in user mode, it would be impossible to get into supervisor mode.

 Pierpaolo Prazzoli - 08/19/03
    - Added check for D_BIT and S_BIT where PC or SR must or must not be denoted.
      (movd, divu, divs, ldxx1, ldxx2, stxx1, stxx2, mulu, muls, set, mul
      call, chk)

 MooglyGuy - 08/17/03
    - Working on support for H flag, nothing quite done yet
    - Added trap Range Error for CHK PC, PC
    - Fixed relative jumps, they have to be taken from the opcode following the
      jump minstead of the jump opcode itself.

 Pierpaolo Prazzoli - 08/17/03
    - Fixed get_pcrel() when OP & 0x80 is set.
    - Decremented PC by 2 also in MOV, ADD, ADDI, SUM, SUB and added the check if
      D_BIT is not set. (when pc is changed they are implicit branch)

 MooglyGuy - 08/17/03
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

 MooglyGuy - 08/16/03
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
#define DEBUG_PRINTF(x) do { mame_printf_debug x; } while (0)
#else
#define DEBUG_PRINTF(x) do { } while (0)
#endif

// set C in adds/addsi/subs/sums
#define SETCARRYS 0
#define MISSIONCRAFT_FLAGS 1

/* Registers */

enum
{
	E132XS_PC = 1,
	E132XS_SR,
	E132XS_FER,
	E132XS_G3,
	E132XS_G4,
	E132XS_G5,
	E132XS_G6,
	E132XS_G7,
	E132XS_G8,
	E132XS_G9,
	E132XS_G10,
	E132XS_G11,
	E132XS_G12,
	E132XS_G13,
	E132XS_G14,
	E132XS_G15,
	E132XS_G16,
	E132XS_G17,
	E132XS_SP,
	E132XS_UB,
	E132XS_BCR,
	E132XS_TPR,
	E132XS_TCR,
	E132XS_TR,
	E132XS_WCR,
	E132XS_ISR,
	E132XS_FCR,
	E132XS_MCR,
	E132XS_G28,
	E132XS_G29,
	E132XS_G30,
	E132XS_G31,
	E132XS_CL0, E132XS_CL1, E132XS_CL2, E132XS_CL3,
	E132XS_CL4, E132XS_CL5, E132XS_CL6, E132XS_CL7,
	E132XS_CL8, E132XS_CL9, E132XS_CL10,E132XS_CL11,
	E132XS_CL12,E132XS_CL13,E132XS_CL14,E132XS_CL15,
	E132XS_L0,  E132XS_L1,  E132XS_L2,  E132XS_L3,
	E132XS_L4,  E132XS_L5,  E132XS_L6,  E132XS_L7,
	E132XS_L8,  E132XS_L9,  E132XS_L10, E132XS_L11,
	E132XS_L12, E132XS_L13, E132XS_L14, E132XS_L15,
	E132XS_L16, E132XS_L17, E132XS_L18, E132XS_L19,
	E132XS_L20, E132XS_L21, E132XS_L22, E132XS_L23,
	E132XS_L24, E132XS_L25, E132XS_L26, E132XS_L27,
	E132XS_L28, E132XS_L29, E132XS_L30, E132XS_L31,
	E132XS_L32, E132XS_L33, E132XS_L34, E132XS_L35,
	E132XS_L36, E132XS_L37, E132XS_L38, E132XS_L39,
	E132XS_L40, E132XS_L41, E132XS_L42, E132XS_L43,
	E132XS_L44, E132XS_L45, E132XS_L46, E132XS_L47,
	E132XS_L48, E132XS_L49, E132XS_L50, E132XS_L51,
	E132XS_L52, E132XS_L53, E132XS_L54, E132XS_L55,
	E132XS_L56, E132XS_L57, E132XS_L58, E132XS_L59,
	E132XS_L60, E132XS_L61, E132XS_L62, E132XS_L63
};


/* Delay information */
struct delay_info
{
	INT32	delay_cmd;
	UINT32	delay_pc;
};

/* Internal registers */
struct hyperstone_state
{
	UINT32	global_regs[32];
	UINT32	local_regs[64];

	/* internal stuff */
	UINT32	ppc;	// previous pc
	UINT16	op;		// opcode
	UINT32	trap_entry; // entry point to get trap address

	UINT8	clock_scale_mask;
	UINT8	clock_scale;
	UINT8	clock_cycles_1;
	UINT8	clock_cycles_2;
	UINT8	clock_cycles_4;
	UINT8	clock_cycles_6;

	UINT64	tr_base_cycles;
	UINT32	tr_base_value;
	UINT32	tr_clocks_per_tick;
	UINT8	timer_int_pending;
	emu_timer *timer;

	delay_info delay;

	device_irq_acknowledge_callback irq_callback;
	legacy_cpu_device *device;
	address_space *program;
	direct_read_data *direct;
	address_space *io;
	UINT32 opcodexor;

	INT32 instruction_length;
	INT32 intblock;

	int icount;
};

struct regs_decode
{
	UINT8	src, dst;	    // destination and source register code
	UINT32	src_value;      // current source register value
	UINT32	next_src_value; // current next source register value
	UINT32	dst_value;      // current destination register value
	UINT32	next_dst_value; // current next destination register value
	UINT8	sub_type;		// sub type opcode (for DD and X_CODE bits)
	union
	{
		UINT32 u;
		INT32  s;
	} extra;				// extra value such as immediate value, const, pcrel, ...
	UINT8	src_is_local;
	UINT8	dst_is_local;
	UINT8   same_src_dst;
	UINT8   same_src_dstf;
	UINT8   same_srcf_dst;
};

static void check_interrupts(hyperstone_state *cpustate);

#define SREG  (decode)->src_value
#define SREGF (decode)->next_src_value
#define DREG  (decode)->dst_value
#define DREGF (decode)->next_dst_value
#define EXTRA_U (decode)->extra.u
#define EXTRA_S (decode)->extra.s

#define SET_SREG( _data_ )  ((decode)->src_is_local ? set_local_register(cpustate, (decode)->src, (UINT32)_data_) : set_global_register(cpustate, (decode)->src, (UINT32)_data_))
#define SET_SREGF( _data_ ) ((decode)->src_is_local ? set_local_register(cpustate, (decode)->src + 1, (UINT32)_data_) : set_global_register(cpustate, (decode)->src + 1, (UINT32)_data_))
#define SET_DREG( _data_ )  ((decode)->dst_is_local ? set_local_register(cpustate, (decode)->dst, (UINT32)_data_) : set_global_register(cpustate, (decode)->dst, (UINT32)_data_))
#define SET_DREGF( _data_ ) ((decode)->dst_is_local ? set_local_register(cpustate, (decode)->dst + 1, (UINT32)_data_) : set_global_register(cpustate, (decode)->dst + 1, (UINT32)_data_))

#define SRC_IS_PC      (!(decode)->src_is_local && (decode)->src == PC_REGISTER)
#define DST_IS_PC      (!(decode)->dst_is_local && (decode)->dst == PC_REGISTER)
#define SRC_IS_SR      (!(decode)->src_is_local && (decode)->src == SR_REGISTER)
#define DST_IS_SR      (!(decode)->dst_is_local && (decode)->dst == SR_REGISTER)
#define SAME_SRC_DST   (decode)->same_src_dst
#define SAME_SRC_DSTF  (decode)->same_src_dstf
#define SAME_SRCF_DST  (decode)->same_srcf_dst

// 4Kb IRAM (On-Chip Memory)

static ADDRESS_MAP_START( e116_4k_iram_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM AM_MIRROR(0x1ffff000)
ADDRESS_MAP_END



static ADDRESS_MAP_START( e132_4k_iram_map, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0xc0000000, 0xc0000fff) AM_RAM AM_MIRROR(0x1ffff000)
ADDRESS_MAP_END


// 8Kb IRAM (On-Chip Memory)

static ADDRESS_MAP_START( e116_8k_iram_map, AS_PROGRAM, 16, legacy_cpu_device )

	AM_RANGE(0xc0000000, 0xc0001fff) AM_RAM AM_MIRROR(0x1fffe000)
ADDRESS_MAP_END



static ADDRESS_MAP_START( e132_8k_iram_map, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0xc0000000, 0xc0001fff) AM_RAM AM_MIRROR(0x1fffe000)
ADDRESS_MAP_END


// 16Kb IRAM (On-Chip Memory)


static ADDRESS_MAP_START( e116_16k_iram_map, AS_PROGRAM, 16, legacy_cpu_device )
	AM_RANGE(0xc0000000, 0xc0003fff) AM_RAM AM_MIRROR(0x1fffc000)
ADDRESS_MAP_END



static ADDRESS_MAP_START( e132_16k_iram_map, AS_PROGRAM, 32, legacy_cpu_device )
	AM_RANGE(0xc0000000, 0xc0003fff) AM_RAM AM_MIRROR(0x1fffc000)
ADDRESS_MAP_END


INLINE hyperstone_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == E116T ||
		   device->type() == E116XT ||
		   device->type() == E116XS ||
		   device->type() == E116XSR ||
		   device->type() == E132N ||
		   device->type() == E132T ||
		   device->type() == E132XN ||
		   device->type() == E132XT ||
		   device->type() == E132XS ||
		   device->type() == E132XSR ||
		   device->type() == GMS30C2116 ||
		   device->type() == GMS30C2132 ||
		   device->type() == GMS30C2216 ||
		   device->type() == GMS30C2232);
	return (hyperstone_state *)downcast<legacy_cpu_device *>(device)->token();
}

/* Return the entry point for a determinated trap */
static UINT32 get_trap_addr(hyperstone_state *cpustate, UINT8 trapno)
{
	UINT32 addr;
	if( cpustate->trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = trapno * 4;
	}
	else
	{
		addr = (63 - trapno) * 4;
	}
	addr |= cpustate->trap_entry;

	return addr;
}

/* Return the entry point for a determinated emulated code (the one for "extend" opcode is reserved) */
static UINT32 get_emu_code_addr(hyperstone_state *cpustate, UINT8 num) /* num is OP */
{
	UINT32 addr;
	if( cpustate->trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = (cpustate->trap_entry - 0x100) | ((num & 0xf) << 4);
	}
	else
	{
		addr = cpustate->trap_entry | (0x10c | ((0xcf - num) << 4));
	}
	return addr;
}

static void hyperstone_set_trap_entry(hyperstone_state *cpustate, int which)
{
	switch( which )
	{
		case E132XS_ENTRY_MEM0:
			cpustate->trap_entry = 0x00000000;
			break;

		case E132XS_ENTRY_MEM1:
			cpustate->trap_entry = 0x40000000;
			break;

		case E132XS_ENTRY_MEM2:
			cpustate->trap_entry = 0x80000000;
			break;

		case E132XS_ENTRY_MEM3:
			cpustate->trap_entry = 0xffffff00;
			break;

		case E132XS_ENTRY_IRAM:
			cpustate->trap_entry = 0xc0000000;
			break;

		default:
			DEBUG_PRINTF(("Set entry point to a reserved value: %d\n", which));
			break;
	}
}

#define OP  			cpustate->op
#define PPC				cpustate->ppc //previous pc
#define PC				cpustate->global_regs[0] //Program Counter
#define SR				cpustate->global_regs[1] //Status Register
#define FER				cpustate->global_regs[2] //Floating-Point Exception Register
// 03 - 15  General Purpose Registers
// 16 - 17  Reserved
#define SP				cpustate->global_regs[18] //Stack Pointer
#define UB				cpustate->global_regs[19] //Upper Stack Bound
#define BCR				cpustate->global_regs[20] //Bus Control Register
#define TPR				cpustate->global_regs[21] //Timer Prescaler Register
#define TCR				cpustate->global_regs[22] //Timer Compare Register
#define TR				compute_tr(cpustate) //Timer Register
#define WCR				cpustate->global_regs[24] //Watchdog Compare Register
#define ISR				cpustate->global_regs[25] //Input Status Register
#define FCR				cpustate->global_regs[26] //Function Control Register
#define MCR				cpustate->global_regs[27] //Memory Control Register
// 28 - 31  Reserved

/* SR flags */
#define GET_C					( SR & 0x00000001)      // bit 0 //CARRY
#define GET_Z					((SR & 0x00000002)>>1)  // bit 1 //ZERO
#define GET_N					((SR & 0x00000004)>>2)  // bit 2 //NEGATIVE
#define GET_V					((SR & 0x00000008)>>3)  // bit 3 //OVERFLOW
#define GET_M					((SR & 0x00000010)>>4)  // bit 4 //CACHE-MODE
#define GET_H					((SR & 0x00000020)>>5)  // bit 5 //HIGHGLOBAL
// bit 6 RESERVED (always 0)
#define GET_I					((SR & 0x00000080)>>7)  // bit 7 //INTERRUPT-MODE
#define GET_FTE					((SR & 0x00001f00)>>8)  // bits 12 - 8  //Floating-Point Trap Enable
#define GET_FRM					((SR & 0x00006000)>>13) // bits 14 - 13 //Floating-Point Rounding Mode
#define GET_L					((SR & 0x00008000)>>15) // bit 15 //INTERRUPT-LOCK
#define GET_T					((SR & 0x00010000)>>16) // bit 16 //TRACE-MODE
#define GET_P					((SR & 0x00020000)>>17) // bit 17 //TRACE PENDING
#define GET_S					((SR & 0x00040000)>>18) // bit 18 //SUPERVISOR STATE
#define GET_ILC					((SR & 0x00180000)>>19) // bits 20 - 19 //INSTRUCTION-LENGTH
/* if FL is zero it is always interpreted as 16 */
#define GET_FL					((SR & 0x01e00000) ? ((SR & 0x01e00000)>>21) : 16) // bits 24 - 21 //FRAME LENGTH
#define GET_FP					((SR & 0xfe000000)>>25) // bits 31 - 25 //FRAME POINTER

#define SET_C(val)				(SR = (SR & ~0x00000001) | (val))
#define SET_Z(val)				(SR = (SR & ~0x00000002) | ((val) << 1))
#define SET_N(val)				(SR = (SR & ~0x00000004) | ((val) << 2))
#define SET_V(val)				(SR = (SR & ~0x00000008) | ((val) << 3))
#define SET_M(val)				(SR = (SR & ~0x00000010) | ((val) << 4))
#define SET_H(val)				(SR = (SR & ~0x00000020) | ((val) << 5))
#define SET_I(val)				(SR = (SR & ~0x00000080) | ((val) << 7))
#define SET_FTE(val)			(SR = (SR & ~0x00001f00) | ((val) << 8))
#define	SET_FRM(val)			(SR = (SR & ~0x00006000) | ((val) << 13))
#define SET_L(val)				(SR = (SR & ~0x00008000) | ((val) << 15))
#define SET_T(val)				(SR = (SR & ~0x00010000) | ((val) << 16))
#define SET_P(val)				(SR = (SR & ~0x00020000) | ((val) << 17))
#define SET_S(val)				(SR = (SR & ~0x00040000) | ((val) << 18))
#define SET_ILC(val)			(SR = (SR & ~0x00180000) | ((val) << 19))
#define SET_FL(val)				(SR = (SR & ~0x01e00000) | ((val) << 21))
#define SET_FP(val)				(SR = (SR & ~0xfe000000) | ((val) << 25))

#define SET_PC(val)				PC = ((val) & 0xfffffffe) //PC(0) = 0
#define SET_SP(val)				SP = ((val) & 0xfffffffc) //SP(0) = SP(1) = 0
#define SET_UB(val)				UB = ((val) & 0xfffffffc) //UB(0) = UB(1) = 0

#define SET_LOW_SR(val)			(SR = (SR & 0xffff0000) | ((val) & 0x0000ffff)) // when SR is addressed, only low 16 bits can be changed


#define CHECK_C(x)				(SR = (SR & ~0x00000001) | (((x) & (((UINT64)1) << 32)) ? 1 : 0 ))
#define CHECK_VADD(x,y,z)		(SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & 0x80000000) ? 8: 0))
#define CHECK_VADD3(x,y,w,z)	(SR = (SR & ~0x00000008) | ((((x) ^ (z)) & ((y) ^ (z)) & ((w) ^ (z)) & 0x80000000) ? 8: 0))
#define CHECK_VSUB(x,y,z)		(SR = (SR & ~0x00000008) | ((((z) ^ (y)) & ((y) ^ (x)) & 0x80000000) ? 8: 0))


/* FER flags */
#define GET_ACCRUED				(FER & 0x0000001f) //bits  4 - 0 //Floating-Point Accrued Exceptions
#define GET_ACTUAL				(FER & 0x00001f00) //bits 12 - 8 //Floating-Point Actual  Exceptions
//other bits are reversed, in particular 7 - 5 for the operating system.
//the user program can only changes the above 2 flags




static UINT32 compute_tr(hyperstone_state *cpustate)
{
	UINT64 cycles_since_base = cpustate->device->total_cycles() - cpustate->tr_base_cycles;
	UINT64 clocks_since_base = cycles_since_base >> cpustate->clock_scale;
	return cpustate->tr_base_value + (clocks_since_base / cpustate->tr_clocks_per_tick);
}

static void update_timer_prescale(hyperstone_state *cpustate)
{
	UINT32 prevtr = compute_tr(cpustate);
	TPR &= ~0x80000000;
	cpustate->clock_scale = (TPR >> 26) & cpustate->clock_scale_mask;
	cpustate->clock_cycles_1 = 1 << cpustate->clock_scale;
	cpustate->clock_cycles_2 = 2 << cpustate->clock_scale;
	cpustate->clock_cycles_4 = 4 << cpustate->clock_scale;
	cpustate->clock_cycles_6 = 6 << cpustate->clock_scale;
	cpustate->tr_clocks_per_tick = ((TPR >> 16) & 0xff) + 2;
	cpustate->tr_base_value = prevtr;
	cpustate->tr_base_cycles = cpustate->device->total_cycles();
}

static void adjust_timer_interrupt(hyperstone_state *cpustate)
{
	UINT64 cycles_since_base = cpustate->device->total_cycles() - cpustate->tr_base_cycles;
	UINT64 clocks_since_base = cycles_since_base >> cpustate->clock_scale;
	UINT64 cycles_until_next_clock = cycles_since_base - (clocks_since_base << cpustate->clock_scale);

	if (cycles_until_next_clock == 0)
		cycles_until_next_clock = (UINT64)(1 << cpustate->clock_scale);

	/* special case: if we have a change pending, set a timer to fire then */
	if (TPR & 0x80000000)
	{
		UINT64 clocks_until_int = cpustate->tr_clocks_per_tick - (clocks_since_base % cpustate->tr_clocks_per_tick);
		UINT64 cycles_until_int = (clocks_until_int << cpustate->clock_scale) + cycles_until_next_clock;
		cpustate->timer->adjust(cpustate->device->cycles_to_attotime(cycles_until_int + 1), 1);
	}

	/* else if the timer interrupt is enabled, configure it to fire at the appropriate time */
	else if (!(FCR & 0x00800000))
	{
		UINT32 curtr = cpustate->tr_base_value + (clocks_since_base / cpustate->tr_clocks_per_tick);
		UINT32 delta = TCR - curtr;
		if (delta > 0x80000000)
		{
			if (!cpustate->timer_int_pending)
				cpustate->timer->adjust(attotime::zero);
		}
		else
		{
			UINT64 clocks_until_int = mulu_32x32(delta, cpustate->tr_clocks_per_tick);
			UINT64 cycles_until_int = (clocks_until_int << cpustate->clock_scale) + cycles_until_next_clock;
			cpustate->timer->adjust(cpustate->device->cycles_to_attotime(cycles_until_int));
		}
	}

	/* otherwise, disable the timer */
	else
		cpustate->timer->adjust(attotime::never);
}

static TIMER_CALLBACK( e132xs_timer_callback )
{
	legacy_cpu_device *device = (legacy_cpu_device *)ptr;
	hyperstone_state *cpustate = get_safe_token(device);
	int update = param;

	/* update the values if necessary */
	if (update)
		update_timer_prescale(cpustate);

	/* see if the timer is right for firing */
	if (!((compute_tr(cpustate) - TCR) & 0x80000000))
		cpustate->timer_int_pending = 1;

	/* adjust ourselves for the next time */
	else
		adjust_timer_interrupt(cpustate);
}




static UINT32 get_global_register(hyperstone_state *cpustate, UINT8 code)
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
		if (cpustate->icount > cpustate->tr_clocks_per_tick / 2)
			cpustate->icount -= cpustate->tr_clocks_per_tick / 2;
		return compute_tr(cpustate);
	}
	return cpustate->global_regs[code];
}

INLINE void set_global_register(hyperstone_state *cpustate, UINT8 code, UINT32 val)
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
		if (cpustate->intblock < 1)
			cpustate->intblock = 1;
	}
	else
	{
		UINT32 oldval = cpustate->global_regs[code];
		if( code != ISR_REGISTER )
			cpustate->global_regs[code] = val;
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
				cpustate->tr_base_value = val;
				cpustate->tr_base_cycles = cpustate->device->total_cycles();
				adjust_timer_interrupt(cpustate);
				break;

			case TPR_REGISTER:
				if (!(val & 0x80000000)) /* change immediately */
					update_timer_prescale(cpustate);
				adjust_timer_interrupt(cpustate);
				break;

            case TCR_REGISTER:
            	if (oldval != val)
            	{
					adjust_timer_interrupt(cpustate);
					if (cpustate->intblock < 1)
						cpustate->intblock = 1;
				}
                break;

            case FCR_REGISTER:
            	if ((oldval ^ val) & 0x00800000)
					adjust_timer_interrupt(cpustate);
				if (cpustate->intblock < 1)
					cpustate->intblock = 1;
                break;

			case MCR_REGISTER:
				// bits 14..12 EntryTableMap
				hyperstone_set_trap_entry(cpustate, (val & 0x7000) >> 12);
				break;
			}
		}
	}
}

INLINE void set_local_register(hyperstone_state *cpustate, UINT8 code, UINT32 val)
{
	UINT8 new_code = (code + GET_FP) % 64;

	cpustate->local_regs[new_code] = val;
}

#define GET_ABS_L_REG(code)			cpustate->local_regs[code]
#define SET_L_REG(code, val)	    set_local_register(cpustate, code, val)
#define SET_ABS_L_REG(code, val)	cpustate->local_regs[code] = val
#define GET_G_REG(code)				get_global_register(cpustate, code)
#define SET_G_REG(code, val)	    set_global_register(cpustate, code, val)

#define S_BIT					((OP & 0x100) >> 8)
#define N_BIT					S_BIT
#define D_BIT					((OP & 0x200) >> 9)
#define N_VALUE					((N_BIT << 4) | (OP & 0x0f))
#define DST_CODE				((OP & 0xf0) >> 4)
#define SRC_CODE				(OP & 0x0f)
#define SIGN_BIT(val)			((val & 0x80000000) >> 31)

#define LOCAL  1

static const INT32 immediate_values[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 0, 0, 0, 32, 64, 128, 0x80000000,
	-8, -7, -6, -5, -4, -3, -2, -1
};

#define WRITE_ONLY_REGMASK	((1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER))

#define decode_source(decode, local, hflag)											\
do																					\
{																					\
	if(local)																		\
	{																				\
		UINT8 code = (decode)->src;													\
		(decode)->src_is_local = 1;													\
		code = ((decode)->src + GET_FP) % 64; /* registers offset by frame pointer  */\
		SREG = cpustate->local_regs[code];											\
		code = ((decode)->src + 1 + GET_FP) % 64;									\
		SREGF = cpustate->local_regs[code];										\
	}																				\
	else																			\
	{																				\
		(decode)->src_is_local = 0;													\
																					\
		if (!hflag)																	\
		{																			\
			SREG = get_global_register(cpustate, (decode)->src);								\
																					\
			/* bound safe */														\
			if ((decode)->src != 15)												\
				SREGF = get_global_register(cpustate, (decode)->src + 1);						\
		}																			\
		else																		\
		{																			\
			(decode)->src += 16;													\
																					\
			SREG = get_global_register(cpustate, (decode)->src);								\
			if ((WRITE_ONLY_REGMASK >> (decode)->src) & 1)							\
				SREG = 0; /* write-only registers */								\
			else if ((decode)->src == ISR_REGISTER)									\
				DEBUG_PRINTF(("read src ISR. PC = %08X\n",PPC));					\
																					\
			/* bound safe */														\
			if ((decode)->src != 31)												\
				SREGF = get_global_register(cpustate, (decode)->src + 1);						\
		}																			\
	}																				\
} while (0)

#define decode_dest(decode, local, hflag)											\
do																					\
{																					\
	if(local)																		\
	{																				\
		UINT8 code = (decode)->dst;													\
		(decode)->dst_is_local = 1;													\
		code = ((decode)->dst + GET_FP) % 64; /* registers offset by frame pointer */\
		DREG = cpustate->local_regs[code];											\
		code = ((decode)->dst + 1 + GET_FP) % 64;									\
		DREGF = cpustate->local_regs[code];										\
	}																				\
	else																			\
	{																				\
		(decode)->dst_is_local = 0;													\
																					\
		if (!hflag)																	\
		{																			\
			DREG = get_global_register(cpustate, (decode)->dst);								\
																					\
			/* bound safe */														\
			if ((decode)->dst != 15)												\
				DREGF = get_global_register(cpustate, (decode)->dst + 1);						\
		}																			\
		else																		\
		{																			\
			(decode)->dst += 16;													\
																					\
			DREG = get_global_register(cpustate, (decode)->dst);								\
			if( (decode)->dst == ISR_REGISTER )										\
				DEBUG_PRINTF(("read dst ISR. PC = %08X\n",PPC));					\
																					\
			/* bound safe */														\
			if ((decode)->dst != 31)												\
				DREGF = get_global_register(cpustate, (decode)->dst + 1);						\
		}																			\
	}																				\
} while (0)

#define decode_RR(decode, dlocal, slocal)											\
do																					\
{																					\
	(decode)->src = SRC_CODE;														\
	(decode)->dst = DST_CODE;														\
	decode_source(decode, slocal, 0);												\
	decode_dest(decode, dlocal, 0);													\
																					\
	if( (slocal) == (dlocal) && SRC_CODE == DST_CODE )								\
		SAME_SRC_DST = 1;															\
																					\
	if( (slocal) == LOCAL && (dlocal) == LOCAL )									\
	{																				\
		if( SRC_CODE == ((DST_CODE + 1) % 64) )										\
			SAME_SRC_DSTF = 1;														\
																					\
		if( ((SRC_CODE + 1) % 64) == DST_CODE )										\
			SAME_SRCF_DST = 1;														\
	}																				\
	else if( (slocal) == 0 && (dlocal) == 0 )										\
	{																				\
		if( SRC_CODE == (DST_CODE + 1) )											\
			SAME_SRC_DSTF = 1;														\
																					\
		if( (SRC_CODE + 1) == DST_CODE )											\
			SAME_SRCF_DST = 1;														\
	}																				\
} while (0)

#define decode_LL(decode)															\
do																					\
{																					\
	(decode)->src = SRC_CODE;														\
	(decode)->dst = DST_CODE;														\
	decode_source(decode, LOCAL, 0);												\
	decode_dest(decode, LOCAL, 0);													\
																					\
	if( SRC_CODE == DST_CODE )														\
		SAME_SRC_DST = 1;															\
																					\
	if( SRC_CODE == ((DST_CODE + 1) % 64) )											\
		SAME_SRC_DSTF = 1;															\
} while (0)

#define decode_LR(decode, slocal)													\
do																					\
{																					\
	(decode)->src = SRC_CODE;														\
	(decode)->dst = DST_CODE;														\
	decode_source(decode, slocal, 0);												\
	decode_dest(decode, LOCAL, 0);													\
																					\
	if( ((SRC_CODE + 1) % 64) == DST_CODE && slocal == LOCAL )						\
		SAME_SRCF_DST = 1;															\
} while (0)

#define check_delay_PC()															\
do																					\
{																					\
	/* if PC is used in a delay instruction, the delayed PC should be used */		\
	if( cpustate->delay.delay_cmd == DELAY_EXECUTE )								\
	{																				\
		PC = cpustate->delay.delay_pc;												\
		cpustate->delay.delay_cmd = NO_DELAY;										\
	}																				\
} while (0)

#define decode_immediate(decode, nbit)												\
do																					\
{																					\
	if (!nbit)																		\
		EXTRA_U = immediate_values[OP & 0x0f];										\
	else																			\
		switch( OP & 0x0f )															\
		{																			\
			default:																\
				EXTRA_U = immediate_values[0x10 + (OP & 0x0f)];						\
				break;																\
																					\
			case 1:																	\
				cpustate->instruction_length = 3;									\
				EXTRA_U = (READ_OP(cpustate, PC) << 16) | READ_OP(cpustate, PC + 2);\
				PC += 4;															\
				break;																\
																					\
			case 2:																	\
				cpustate->instruction_length = 2;									\
				EXTRA_U = READ_OP(cpustate, PC);									\
				PC += 2;															\
				break;																\
																					\
			case 3:																	\
				cpustate->instruction_length = 2;									\
				EXTRA_U = 0xffff0000 | READ_OP(cpustate, PC);						\
				PC += 2;															\
				break;																\
		}																			\
} while (0)

#define decode_const(decode)														\
do																					\
{																					\
	UINT16 imm_1 = READ_OP(cpustate, PC);											\
																					\
	PC += 2;																		\
	cpustate->instruction_length = 2;												\
																					\
	if( E_BIT(imm_1) )																\
	{																				\
		UINT16 imm_2 = READ_OP(cpustate, PC);										\
																					\
		PC += 2;																	\
		cpustate->instruction_length = 3;											\
																					\
		EXTRA_S = imm_2;															\
		EXTRA_S |= ((imm_1 & 0x3fff) << 16);										\
																					\
		if( S_BIT_CONST(imm_1) )													\
		{																			\
			EXTRA_S |= 0xc0000000;													\
		}																			\
	}																				\
	else																			\
	{																				\
		EXTRA_S = imm_1 & 0x3fff;													\
																					\
		if( S_BIT_CONST(imm_1) )													\
		{																			\
			EXTRA_S |= 0xffffc000;													\
		}																			\
	}																				\
} while (0)

#define decode_pcrel(decode)														\
do																					\
{																					\
	if( OP & 0x80 )																	\
	{																				\
		UINT16 next = READ_OP(cpustate, PC);										\
																					\
		PC += 2;																	\
		cpustate->instruction_length = 2;											\
																					\
		EXTRA_S = (OP & 0x7f) << 16;												\
		EXTRA_S |= (next & 0xfffe);													\
																					\
		if( next & 1 )																\
			EXTRA_S |= 0xff800000;													\
	}																				\
	else																			\
	{																				\
		EXTRA_S = OP & 0x7e;														\
																					\
		if( OP & 1 )																\
			EXTRA_S |= 0xffffff80;													\
	}																				\
} while (0)

#define decode_dis(decode)															\
do																					\
{																					\
	UINT16 next_1 = READ_OP(cpustate, PC);											\
																					\
	PC += 2;																		\
	cpustate->instruction_length = 2;												\
																					\
	(decode)->sub_type = DD(next_1);												\
																					\
	if( E_BIT(next_1) )																\
	{																				\
		UINT16 next_2 = READ_OP(cpustate, PC);										\
																					\
		PC += 2;																	\
		cpustate->instruction_length = 3;											\
																					\
		EXTRA_S = next_2;															\
		EXTRA_S |= ((next_1 & 0xfff) << 16);										\
																					\
		if( S_BIT_CONST(next_1) )													\
		{																			\
			EXTRA_S |= 0xf0000000;													\
		}																			\
	}																				\
	else																			\
	{																				\
		EXTRA_S = next_1 & 0xfff;													\
																					\
		if( S_BIT_CONST(next_1) )													\
		{																			\
			EXTRA_S |= 0xfffff000;													\
		}																			\
	}																				\
} while (0)

#define decode_lim(decode)															\
do																					\
{																					\
	UINT32 next = READ_OP(cpustate, PC);											\
	PC += 2;																		\
	cpustate->instruction_length = 2;												\
																					\
	(decode)->sub_type = X_CODE(next);												\
																					\
	if( E_BIT(next) )																\
	{																				\
		EXTRA_U = ((next & 0xfff) << 16) | READ_OP(cpustate, PC);					\
		PC += 2;																	\
		cpustate->instruction_length = 3;											\
	}																				\
	else																			\
	{																				\
		EXTRA_U = next & 0xfff;														\
	}																				\
} while (0)

#define RRdecode(decode, dlocal, slocal)											\
do																					\
{																					\
	check_delay_PC();																\
	decode_RR(decode, dlocal, slocal);												\
} while (0)

#define RRlimdecode(decode, dlocal, slocal)											\
do																					\
{																					\
	decode_lim(decode);																\
	check_delay_PC();																\
	decode_RR(decode, dlocal, slocal);												\
} while (0)

#define RRconstdecode(decode, dlocal, slocal)										\
do																					\
{																					\
	decode_const(decode);															\
	check_delay_PC();																\
	decode_RR(decode, dlocal, slocal);												\
} while (0)

#define RRdisdecode(decode, dlocal, slocal)											\
do																					\
{																					\
	decode_dis(decode);																\
	check_delay_PC();																\
	decode_RR(decode, dlocal, slocal);												\
} while (0)

#define RRdecodewithHflag(decode, dlocal, slocal)									\
do																					\
{																					\
	check_delay_PC();																\
	(decode)->src = SRC_CODE;														\
	(decode)->dst = DST_CODE;														\
	decode_source(decode, slocal, GET_H);											\
	decode_dest(decode, dlocal, GET_H);												\
																					\
	if(GET_H)																		\
		if(slocal == 0 && dlocal == 0)												\
			DEBUG_PRINTF(("MOV with hflag and 2 GRegs! PC = %08X\n",PPC));			\
} while (0)

#define Rimmdecode(decode, dlocal, nbit)											\
do																					\
{																					\
	decode_immediate(decode, nbit);													\
	check_delay_PC();																\
	(decode)->dst = DST_CODE;														\
	decode_dest(decode, dlocal, 0);													\
} while (0)

#define Rndecode(decode, dlocal)													\
do																					\
{																					\
	check_delay_PC();																\
	(decode)->dst = DST_CODE;														\
	decode_dest(decode, dlocal, 0);													\
} while (0)

#define RimmdecodewithHflag(decode, dlocal, nbit)									\
do																					\
{																					\
	decode_immediate(decode, nbit);													\
	check_delay_PC();																\
	(decode)->dst = DST_CODE;														\
	decode_dest(decode, dlocal, GET_H);												\
} while (0)

#define Lndecode(decode)															\
do																					\
{																					\
	check_delay_PC();																\
	(decode)->dst = DST_CODE;														\
	decode_dest(decode, LOCAL, 0);													\
} while (0)

#define LLdecode(decode)															\
do																					\
{																					\
	check_delay_PC();																\
	decode_LL(decode);																\
} while (0)

#define LLextdecode(decode)															\
do																					\
{																					\
	cpustate->instruction_length = 2;												\
	EXTRA_U = READ_OP(cpustate, PC);												\
	PC += 2;																		\
	check_delay_PC();																\
	decode_LL(decode);																\
} while (0)

#define LRdecode(decode, slocal)													\
do																					\
{																					\
	check_delay_PC();																\
	decode_LR(decode, slocal);														\
} while (0)

#define LRconstdecode(decode, slocal)												\
do																					\
{																					\
	decode_const(decode);															\
	check_delay_PC();																\
	decode_LR(decode, slocal);														\
} while (0)

#define PCreldecode(decode)															\
do																					\
{																					\
	decode_pcrel(decode);															\
	check_delay_PC();																\
} while (0)

#define PCadrdecode(decode)															\
do																					\
{																					\
	check_delay_PC();																\
} while (0)

#define no_decode(decode)															\
do																					\
{																					\
} while (0)


INLINE void execute_br(hyperstone_state *cpustate, struct regs_decode *decode)
{
	PPC = PC;
	PC += EXTRA_S;
	SET_M(0);

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void execute_dbr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	cpustate->delay.delay_cmd = DELAY_EXECUTE;
	cpustate->delay.delay_pc  = PC + EXTRA_S;

	cpustate->intblock = 3;
}


static void execute_trap(hyperstone_state *cpustate, UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(cpustate->instruction_length & 3);

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

	cpustate->icount -= cpustate->clock_cycles_2;
}


static void execute_int(hyperstone_state *cpustate, UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(cpustate->instruction_length & 3);

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

	cpustate->icount -= cpustate->clock_cycles_2;
}

/* TODO: mask Parity Error and Extended Overflow exceptions */
static void execute_exception(hyperstone_state *cpustate, UINT32 addr)
{
	UINT8 reg;
	UINT32 oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(cpustate->instruction_length & 3);

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
	cpustate->icount -= cpustate->clock_cycles_2;
}

static void execute_software(hyperstone_state *cpustate, struct regs_decode *decode)
{
	UINT8 reg;
	UINT32 oldSR;
	UINT32 addr;
	UINT32 stack_of_dst;

	SET_ILC(1);

	addr = get_emu_code_addr(cpustate, (OP & 0xff00) >> 8);
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

#define INT1_LINE_STATE		((ISR >> 0) & 1)
#define INT2_LINE_STATE		((ISR >> 1) & 1)
#define INT3_LINE_STATE		((ISR >> 2) & 1)
#define INT4_LINE_STATE		((ISR >> 3) & 1)
#define IO1_LINE_STATE		((ISR >> 4) & 1)
#define IO2_LINE_STATE		((ISR >> 5) & 1)
#define IO3_LINE_STATE		((ISR >> 6) & 1)

static void check_interrupts(hyperstone_state *cpustate)
{
	/* Interrupt-Lock flag isn't set */
	if (GET_L || cpustate->intblock > 0)
		return;

	/* quick exit if nothing */
	if (!cpustate->timer_int_pending && (ISR & 0x7f) == 0)
		return;

	/* IO3 is priority 5; state is in bit 6 of ISR; FCR bit 10 enables input and FCR bit 8 inhibits interrupt */
	if (IO3_LINE_STATE && (FCR & 0x00000500) == 0x00000400)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_IO3));
		(*cpustate->irq_callback)(cpustate->device, IRQ_IO3);
		return;
	}

	/* timer int might be priority 6 if FCR bits 20-21 == 3; FCR bit 23 inhibits interrupt */
	if (cpustate->timer_int_pending && (FCR & 0x00b00000) == 0x00300000)
	{
		cpustate->timer_int_pending = 0;
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_TIMER));
		return;
	}

	/* INT1 is priority 7; state is in bit 0 of ISR; FCR bit 28 inhibits interrupt */
	if (INT1_LINE_STATE && (FCR & 0x10000000) == 0x00000000)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_INT1));
		(*cpustate->irq_callback)(cpustate->device, IRQ_INT1);
		return;
	}

	/* timer int might be priority 8 if FCR bits 20-21 == 2; FCR bit 23 inhibits interrupt */
	if (cpustate->timer_int_pending && (FCR & 0x00b00000) == 0x00200000)
	{
		cpustate->timer_int_pending = 0;
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_TIMER));
		return;
	}

	/* INT2 is priority 9; state is in bit 1 of ISR; FCR bit 29 inhibits interrupt */
	if (INT2_LINE_STATE && (FCR & 0x20000000) == 0x00000000)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_INT2));
		(*cpustate->irq_callback)(cpustate->device, IRQ_INT2);
		return;
	}

	/* timer int might be priority 10 if FCR bits 20-21 == 1; FCR bit 23 inhibits interrupt */
	if (cpustate->timer_int_pending && (FCR & 0x00b00000) == 0x00100000)
	{
		cpustate->timer_int_pending = 0;
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_TIMER));
		return;
	}

	/* INT3 is priority 11; state is in bit 2 of ISR; FCR bit 30 inhibits interrupt */
	if (INT3_LINE_STATE && (FCR & 0x40000000) == 0x00000000)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_INT3));
		(*cpustate->irq_callback)(cpustate->device, IRQ_INT3);
		return;
	}

	/* timer int might be priority 12 if FCR bits 20-21 == 0; FCR bit 23 inhibits interrupt */
	if (cpustate->timer_int_pending && (FCR & 0x00b00000) == 0x00000000)
	{
		cpustate->timer_int_pending = 0;
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_TIMER));
		return;
	}

	/* INT4 is priority 13; state is in bit 3 of ISR; FCR bit 31 inhibits interrupt */
	if (INT4_LINE_STATE && (FCR & 0x80000000) == 0x00000000)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_INT4));
		(*cpustate->irq_callback)(cpustate->device, IRQ_INT4);
		return;
	}

	/* IO1 is priority 14; state is in bit 4 of ISR; FCR bit 2 enables input and FCR bit 0 inhibits interrupt */
	if (IO1_LINE_STATE && (FCR & 0x00000005) == 0x00000004)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_IO1));
		(*cpustate->irq_callback)(cpustate->device, IRQ_IO1);
		return;
	}

	/* IO2 is priority 15; state is in bit 5 of ISR; FCR bit 6 enables input and FCR bit 4 inhibits interrupt */
	if (IO2_LINE_STATE && (FCR & 0x00000050) == 0x00000040)
	{
		execute_int(cpustate, get_trap_addr(cpustate, TRAPNO_IO2));
		(*cpustate->irq_callback)(cpustate->device, IRQ_IO2);
		return;
	}
}

static void set_irq_line(hyperstone_state *cpustate, int irqline, int state)
{
	if (state)
		ISR |= 1 << irqline;
	else
		ISR &= ~(1 << irqline);
}

static void hyperstone_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, int scale_mask)
{
	hyperstone_state *cpustate = get_safe_token(device);

	device->save_item(NAME(cpustate->global_regs));
	device->save_item(NAME(cpustate->local_regs));
	device->save_item(NAME(cpustate->ppc));
	device->save_item(NAME(cpustate->trap_entry));
	device->save_item(NAME(cpustate->delay.delay_pc));
	device->save_item(NAME(cpustate->instruction_length));
	device->save_item(NAME(cpustate->intblock));
	device->save_item(NAME(cpustate->delay.delay_cmd));
	device->save_item(NAME(cpustate->tr_clocks_per_tick));

	cpustate->irq_callback = irqcallback;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);
	cpustate->timer = device->machine().scheduler().timer_alloc(FUNC(e132xs_timer_callback), (void *)device);
	cpustate->clock_scale_mask = scale_mask;
}

static void e116_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, int scale_mask)
{
	hyperstone_state *cpustate = get_safe_token(device);
	hyperstone_init(device, irqcallback, scale_mask);
	cpustate->opcodexor = 0;
}

static CPU_INIT( e116t )
{
	e116_init(device, irqcallback, 0);
}

static CPU_INIT( e116xt )
{
	e116_init(device, irqcallback, 3);
}

static CPU_INIT( e116xs )
{
	e116_init(device, irqcallback, 7);
}

static CPU_INIT( e116xsr )
{
	e116_init(device, irqcallback, 7);
}

static CPU_INIT( gms30c2116 )
{
	e116_init(device, irqcallback, 0);
}

static CPU_INIT( gms30c2216 )
{
	e116_init(device, irqcallback, 0);
}

static void e132_init(legacy_cpu_device *device, device_irq_acknowledge_callback irqcallback, int scale_mask)
{
	hyperstone_state *cpustate = get_safe_token(device);
	hyperstone_init(device, irqcallback, scale_mask);
	cpustate->opcodexor = WORD_XOR_BE(0);
}

static CPU_INIT( e132n )
{
	e132_init(device, irqcallback, 0);
}

static CPU_INIT( e132t )
{
	e132_init(device, irqcallback, 0);
}

static CPU_INIT( e132xn )
{
	e132_init(device, irqcallback, 3);
}

static CPU_INIT( e132xt )
{
	e132_init(device, irqcallback, 3);
}

static CPU_INIT( e132xs )
{
	e132_init(device, irqcallback, 7);
}

static CPU_INIT( e132xsr )
{
	e132_init(device, irqcallback, 7);
}

static CPU_INIT( gms30c2132 )
{
	e132_init(device, irqcallback, 0);
}

static CPU_INIT( gms30c2232 )
{
	e132_init(device, irqcallback, 0);
}

static CPU_RESET( hyperstone )
{
	hyperstone_state *cpustate = get_safe_token(device);

	//TODO: Add different reset initializations for BCR, MCR, FCR, TPR

	emu_timer *save_timer;
	device_irq_acknowledge_callback save_irqcallback;
	UINT32 save_opcodexor;

	save_timer = cpustate->timer;
	save_irqcallback = cpustate->irq_callback;
	save_opcodexor = cpustate->opcodexor;
	memset(cpustate, 0, sizeof(*cpustate));
	cpustate->irq_callback = save_irqcallback;
	cpustate->opcodexor = save_opcodexor;
	cpustate->device = device;
	cpustate->program = &device->space(AS_PROGRAM);
	cpustate->direct = &cpustate->program->direct();
	cpustate->io = &device->space(AS_IO);
	cpustate->timer = save_timer;

	cpustate->tr_clocks_per_tick = 2;

	hyperstone_set_trap_entry(cpustate, E132XS_ENTRY_MEM3); /* default entry point @ MEM3 */

	set_global_register(cpustate, BCR_REGISTER, ~0);
	set_global_register(cpustate, MCR_REGISTER, ~0);
	set_global_register(cpustate, FCR_REGISTER, ~0);
	set_global_register(cpustate, TPR_REGISTER, 0xc000000);

	PC = get_trap_addr(cpustate, TRAPNO_RESET);

	SET_FP(0);
	SET_FL(2);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	SET_L_REG(0, (PC & 0xfffffffe) | GET_S);
	SET_L_REG(1, SR);

	cpustate->icount -= cpustate->clock_cycles_2;
}

static CPU_EXIT( hyperstone )
{
	// nothing to do
}

static CPU_DISASSEMBLE( hyperstone )
{
	hyperstone_state *cpustate = get_safe_token(device);
	return dasm_hyperstone( buffer, pc, oprom, GET_H, GET_FP );
}

/* Opcodes */

INLINE void hyperstone_chk(hyperstone_state *cpustate, struct regs_decode *decode)
{
	UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);

	if( SRC_IS_SR )
	{
		if( DREG == 0 )
			execute_exception(cpustate, addr);
	}
	else
	{
		if( SRC_IS_PC )
		{
			if( DREG >= SREG )
				execute_exception(cpustate, addr);
		}
		else
		{
			if( DREG > SREG )
				execute_exception(cpustate, addr);
		}
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_movd(hyperstone_state *cpustate, struct regs_decode *decode)
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
			if (cpustate->intblock < 1)
				cpustate->intblock = 1;

			cpustate->instruction_length = 0; // undefined

			if( (!old_s && GET_S) || (!GET_S && !old_l && GET_L))
			{
				UINT32 addr = get_trap_addr(cpustate, TRAPNO_PRIVILEGE_ERROR);
				execute_exception(cpustate, addr);
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
					SET_ABS_L_REG(((SP & 0xfc) >> 2), READ_W(cpustate, SP));
					difference++;

				} while(difference != 0);
			}
		}

		//TODO: no 1!
		cpustate->icount -= cpustate->clock_cycles_1;
	}
	else if( SRC_IS_SR ) // Rd doesn't denote PC and Rs denotes SR
	{
		SET_DREG(0);
		SET_DREGF(0);
		SET_Z(1);
		SET_N(0);

		cpustate->icount -= cpustate->clock_cycles_2;
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		UINT64 tmp;

		SET_DREG(SREG);
		SET_DREGF(SREGF);

		tmp = CONCAT_64(SREG, SREGF);
		SET_Z( tmp == 0 ? 1 : 0 );
		SET_N( SIGN_BIT(SREG) );

		cpustate->icount -= cpustate->clock_cycles_2;
	}
}

INLINE void hyperstone_divu(hyperstone_state *cpustate, struct regs_decode *decode)
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
				addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
				execute_exception(cpustate, addr);
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

	cpustate->icount -= 36 << cpustate->clock_scale;
}

INLINE void hyperstone_divs(hyperstone_state *cpustate, struct regs_decode *decode)
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
				addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
				execute_exception(cpustate, addr);
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

	cpustate->icount -= 36 << cpustate->clock_scale;
}

INLINE void hyperstone_xm(hyperstone_state *cpustate, struct regs_decode *decode)
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
					UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
					execute_exception(cpustate, addr);
				}
				else if( SRC_IS_PC && (SREG >= EXTRA_U) )
				{
					UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
					execute_exception(cpustate, addr);
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_mask(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = SREG & EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_sum(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_sums(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;

	if( GET_V && !SRC_IS_SR )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
		execute_exception(cpustate, addr);
	}
}

INLINE void hyperstone_cmp(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_mov(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_S && decode->dst >= 16 )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_PRIVILEGE_ERROR);
		execute_exception(cpustate, addr);
	}

	SET_DREG(SREG);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( SREG == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(SREG) );

	cpustate->icount -= cpustate->clock_cycles_1;
}


INLINE void hyperstone_add(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_adds(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
		execute_exception(cpustate, addr);
	}
}

INLINE void hyperstone_cmpb(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_Z( (DREG & SREG) == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_andn(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG & ~SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_or(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG | SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_xor(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG ^ SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_subc(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_not(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_DREG(~SREG);
	SET_Z( ~SREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_sub(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_subs(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
		execute_exception(cpustate, addr);
	}
}

INLINE void hyperstone_addc(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_and(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG & SREG;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_neg(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_negs(hyperstone_state *cpustate, struct regs_decode *decode)
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


	cpustate->icount -= cpustate->clock_cycles_1;

	if( GET_V && !SRC_IS_SR ) //trap doesn't occur when source is SR
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
		execute_exception(cpustate, addr);
	}
}

INLINE void hyperstone_cmpi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_movi(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_S && decode->dst >= 16 )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_PRIVILEGE_ERROR);
		execute_exception(cpustate, addr);
	}

	SET_DREG(EXTRA_U);

	if( DST_IS_PC )
		SET_M(0);

	SET_Z( EXTRA_U == 0 ? 1 : 0 );
	SET_N( SIGN_BIT(EXTRA_U) );

#if MISSIONCRAFT_FLAGS
	SET_V(0); // or V undefined ?
#endif

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_addi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_addsi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;

	if( GET_V )
	{
		UINT32 addr = get_trap_addr(cpustate, TRAPNO_RANGE_ERROR);
		execute_exception(cpustate, addr);
	}
}

INLINE void hyperstone_cmpbi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_andni(hyperstone_state *cpustate, struct regs_decode *decode)
{
	UINT32 imm;

	if( N_VALUE == 31 )
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = EXTRA_U;

	DREG = DREG & ~imm;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_ori(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG | EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_xori(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DREG = DREG ^ EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_shrdi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_shrd(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_shr(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_sardi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_sard(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_sar(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_shldi(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_shld(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_shl(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

static void reserved(hyperstone_state *cpustate, struct regs_decode *decode)
{
	DEBUG_PRINTF(("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP));
}

INLINE void hyperstone_testlz(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_rol(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

//TODO: add trap error
INLINE void hyperstone_ldxx1(hyperstone_state *cpustate, struct regs_decode *decode)
{
	UINT32 load;

	if( DST_IS_SR )
	{
		switch( decode->sub_type )
		{
			case 0: // LDBS.A

				load = READ_B(cpustate, EXTRA_S);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				break;

			case 1: // LDBU.A

				load = READ_B(cpustate, EXTRA_S);
				SET_SREG(load);

				break;

			case 2:

				load = READ_HW(cpustate, EXTRA_S & ~1);

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
					load = IO_READ_W(cpustate, EXTRA_S & ~3);
					SET_SREG(load);

					load = IO_READ_W(cpustate, (EXTRA_S & ~3) + 4);
					SET_SREGF(load);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // LDW.IOA
				{
					load = IO_READ_W(cpustate, EXTRA_S & ~3);
					SET_SREG(load);
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.A
				{
					load = READ_W(cpustate, EXTRA_S & ~1);
					SET_SREG(load);

					load = READ_W(cpustate, (EXTRA_S & ~1) + 4);
					SET_SREGF(load);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // LDW.A
				{
					load = READ_W(cpustate, EXTRA_S & ~1);
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

				load = READ_B(cpustate, DREG + EXTRA_S);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				break;

			case 1: // LDBU.D

				load = READ_B(cpustate, DREG + EXTRA_S);
				SET_SREG(load);

				break;

			case 2:

				load = READ_HW(cpustate, DREG + (EXTRA_S & ~1));

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
					load = IO_READ_W(cpustate, DREG + (EXTRA_S & ~3));
					SET_SREG(load);

					load = IO_READ_W(cpustate, DREG + (EXTRA_S & ~3) + 4);
					SET_SREGF(load);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // LDW.IOD
				{
					load = IO_READ_W(cpustate, DREG + (EXTRA_S & ~3));
					SET_SREG(load);
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.D
				{
					load = READ_W(cpustate, DREG + (EXTRA_S & ~1));
					SET_SREG(load);

					load = READ_W(cpustate, DREG + (EXTRA_S & ~1) + 4);
					SET_SREGF(load);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // LDW.D
				{
					load = READ_W(cpustate, DREG + (EXTRA_S & ~1));
					SET_SREG(load);
				}

				break;
		}
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_ldxx2(hyperstone_state *cpustate, struct regs_decode *decode)
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

				load = READ_B(cpustate, DREG);
				load |= (load & 0x80) ? 0xffffff00 : 0;
				SET_SREG(load);

				if(!SAME_SRC_DST)
					SET_DREG(DREG + EXTRA_S);

				break;

			case 1: // LDBU.N

				if(SAME_SRC_DST)
					DEBUG_PRINTF(("LDBU.N denoted same regs @ %08X",PPC));

				load = READ_B(cpustate, DREG);
				SET_SREG(load);

				if(!SAME_SRC_DST)
					SET_DREG(DREG + EXTRA_S);

				break;

			case 2:

				load = READ_HW(cpustate, DREG);

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
						SET_SREG(READ_W(cpustate, DREG));
					else
						SET_SREG(GET_ABS_L_REG((DREG & 0xfc) >> 2));

					if(!SAME_SRC_DST)
						SET_DREG(DREG + (EXTRA_S & ~3));

					cpustate->icount -= cpustate->clock_cycles_2; // extra cycles
				}
				else if( (EXTRA_S & 3) == 2 ) // Reserved
				{
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
				}
				else if( (EXTRA_S & 3) == 1 ) // LDD.N
				{
					if(SAME_SRC_DST || SAME_SRCF_DST)
						DEBUG_PRINTF(("LDD.N denoted same regs @ %08X",PPC));

					load = READ_W(cpustate, DREG);
					SET_SREG(load);

					load = READ_W(cpustate, DREG + 4);
					SET_SREGF(load);

					if(!SAME_SRC_DST && !SAME_SRCF_DST)
						SET_DREG(DREG + (EXTRA_S & ~1));

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // LDW.N
				{
					if(SAME_SRC_DST)
						DEBUG_PRINTF(("LDW.N denoted same regs @ %08X",PPC));

					load = READ_W(cpustate, DREG);
					SET_SREG(load);

					if(!SAME_SRC_DST)
						SET_DREG(DREG + (EXTRA_S & ~1));
				}

				break;
		}
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}

//TODO: add trap error
INLINE void hyperstone_stxx1(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	if( DST_IS_SR )
	{
		switch( decode->sub_type )
		{
			case 0: // STBS.A

				/* TODO: missing trap on range error */
				WRITE_B(cpustate, EXTRA_S, SREG & 0xff);

				break;

			case 1: // STBU.A

				WRITE_B(cpustate, EXTRA_S, SREG & 0xff);

				break;

			case 2:

				WRITE_HW(cpustate, EXTRA_S & ~1, SREG & 0xffff);

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
					IO_WRITE_W(cpustate, EXTRA_S & ~3, SREG);
					IO_WRITE_W(cpustate, (EXTRA_S & ~3) + 4, SREGF);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // STW.IOA
				{
					IO_WRITE_W(cpustate, EXTRA_S & ~3, SREG);
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.A
				{
					WRITE_W(cpustate, EXTRA_S & ~1, SREG);
					WRITE_W(cpustate, (EXTRA_S & ~1) + 4, SREGF);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // STW.A
				{
					WRITE_W(cpustate, EXTRA_S & ~1, SREG);
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
				WRITE_B(cpustate, DREG + EXTRA_S, SREG & 0xff);

				break;

			case 1: // STBU.D

				WRITE_B(cpustate, DREG + EXTRA_S, SREG & 0xff);

				break;

			case 2:

				WRITE_HW(cpustate, DREG + (EXTRA_S & ~1), SREG & 0xffff);

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
					IO_WRITE_W(cpustate, DREG + (EXTRA_S & ~3), SREG);
					IO_WRITE_W(cpustate, DREG + (EXTRA_S & ~3) + 4, SREGF);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else if( (EXTRA_S & 3) == 2 ) // STW.IOD
				{
					IO_WRITE_W(cpustate, DREG + (EXTRA_S & ~3), SREG);
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.D
				{
					WRITE_W(cpustate, DREG + (EXTRA_S & ~1), SREG);
					WRITE_W(cpustate, DREG + (EXTRA_S & ~1) + 4, SREGF);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // STW.D
				{
					WRITE_W(cpustate, DREG + (EXTRA_S & ~1), SREG);
				}

				break;
		}
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_stxx2(hyperstone_state *cpustate, struct regs_decode *decode)
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
				WRITE_B(cpustate, DREG, SREG & 0xff);
				SET_DREG(DREG + EXTRA_S);

				break;

			case 1: // STBU.N

				WRITE_B(cpustate, DREG, SREG & 0xff);
				SET_DREG(DREG + EXTRA_S);

				break;

			case 2:

				WRITE_HW(cpustate, DREG, SREG & 0xffff);
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
						WRITE_W(cpustate, DREG, SREG);
					else
					{
						if(((DREG & 0xfc) >> 2) == ((decode->src + GET_FP) % 64) && S_BIT == LOCAL)
							DEBUG_PRINTF(("STW.S denoted the same local register @ %08X\n",PPC));

						SET_ABS_L_REG((DREG & 0xfc) >> 2,SREG);
					}

					SET_DREG(DREG + (EXTRA_S & ~3));

					cpustate->icount -= cpustate->clock_cycles_2; // extra cycles

				}
				else if( (EXTRA_S & 3) == 2 ) // Reserved
				{
					DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_stxx2. PC = %08X\n", PC));
				}
				else if( (EXTRA_S & 3) == 1 ) // STD.N
				{
					WRITE_W(cpustate, DREG, SREG);
					SET_DREG(DREG + (EXTRA_S & ~1));

					if( SAME_SRCF_DST )
						WRITE_W(cpustate, DREG + 4, SREGF + (EXTRA_S & ~1));  // because DREG == SREGF and DREG has been incremented
					else
						WRITE_W(cpustate, DREG + 4, SREGF);

					cpustate->icount -= cpustate->clock_cycles_1; // extra cycle
				}
				else                      // STW.N
				{
					WRITE_W(cpustate, DREG, SREG);
					SET_DREG(DREG + (EXTRA_S & ~1));
				}

				break;
		}
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_shri(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_sari(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_shli(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_mulu(hyperstone_state *cpustate, struct regs_decode *decode)
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
		cpustate->icount -= cpustate->clock_cycles_4;
	else
		cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_muls(hyperstone_state *cpustate, struct regs_decode *decode)
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
		cpustate->icount -= cpustate->clock_cycles_4;
	else
		cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_set(hyperstone_state *cpustate, struct regs_decode *decode)
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
		cpustate->icount -= cpustate->clock_cycles_1;
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

		cpustate->icount -= cpustate->clock_cycles_1;
	}
}

INLINE void hyperstone_mul(hyperstone_state *cpustate, struct regs_decode *decode)
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
		cpustate->icount -= 3 << cpustate->clock_scale;
	else
		cpustate->icount -= 5 << cpustate->clock_scale;
}

INLINE void hyperstone_fadd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_faddd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fsub(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fsubd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fmul(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fmuld(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fdiv(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fdivd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcmp(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcmpd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcmpu(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcmpud(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcvt(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_fcvtd(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_software(cpustate, decode);
	cpustate->icount -= cpustate->clock_cycles_6;
}

INLINE void hyperstone_extend(hyperstone_state *cpustate, struct regs_decode *decode)
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

	cpustate->icount -= cpustate->clock_cycles_1; //TODO: with the latency it can change
}

INLINE void hyperstone_do(hyperstone_state *cpustate, struct regs_decode *decode)
{
	fatalerror("Executed hyperstone_do instruction. PC = %08X\n", PPC);
}

INLINE void hyperstone_ldwr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_SREG(READ_W(cpustate, DREG));

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_lddr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_SREG(READ_W(cpustate, DREG));
	SET_SREGF(READ_W(cpustate, DREG + 4));

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_ldwp(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_SREG(READ_W(cpustate, DREG));

	// post increment the destination register if it's different from the source one
	// (needed by Hidden Catch)
	if(!(decode->src == decode->dst && S_BIT == LOCAL))
		SET_DREG(DREG + 4);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_lddp(hyperstone_state *cpustate, struct regs_decode *decode)
{
	SET_SREG(READ_W(cpustate, DREG));
	SET_SREGF(READ_W(cpustate, DREG + 4));

	// post increment the destination register if it's different from the source one
	// and from the "next source" one
	if(!(decode->src == decode->dst && S_BIT == LOCAL) &&	!SAME_SRCF_DST )
	{
		SET_DREG(DREG + 8);
	}
	else
	{
		DEBUG_PRINTF(("LDD.P denoted same regs @ %08X",PPC));
	}

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_stwr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	WRITE_W(cpustate, DREG, SREG);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_stdr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(cpustate, DREG, SREG);
	WRITE_W(cpustate, DREG + 4, SREGF);

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_stwp(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	WRITE_W(cpustate, DREG, SREG);
	SET_DREG(DREG + 4);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_stdp(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(cpustate, DREG, SREG);
	SET_DREG(DREG + 8);

	if( SAME_SRCF_DST )
		WRITE_W(cpustate, DREG + 4, SREGF + 8); // because DREG == SREGF and DREG has been incremented
	else
		WRITE_W(cpustate, DREG + 4, SREGF);

	cpustate->icount -= cpustate->clock_cycles_2;
}

INLINE void hyperstone_dbv(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_V )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbnv(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_V )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbe(hyperstone_state *cpustate, struct regs_decode *decode) //or DBZ
{
	if( GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbne(hyperstone_state *cpustate, struct regs_decode *decode) //or DBNZ
{
	if( !GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbc(hyperstone_state *cpustate, struct regs_decode *decode) //or DBST
{
	if( GET_C )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbnc(hyperstone_state *cpustate, struct regs_decode *decode) //or DBHE
{
	if( !GET_C )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbse(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_C || GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbht(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_C && !GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbn(hyperstone_state *cpustate, struct regs_decode *decode) //or DBLT
{
	if( GET_N )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbnn(hyperstone_state *cpustate, struct regs_decode *decode) //or DBGE
{
	if( !GET_N )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dble(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_N || GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbgt(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_N && !GET_Z )
		execute_dbr(cpustate, decode);

	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_dbr(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_dbr(cpustate, decode);
}

INLINE void hyperstone_frame(hyperstone_state *cpustate, struct regs_decode *decode)
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
			WRITE_W(cpustate, SP, GET_ABS_L_REG((SP & 0xfc) >> 2));
			SP += 4;
			difference++;

		} while(difference != 0);

		if( tmp_flag )
		{
			UINT32 addr = get_trap_addr(cpustate, TRAPNO_FRAME_ERROR);
			execute_exception(cpustate, addr);
		}
	}

	//TODO: no 1!
	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_call(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	if( !DST_CODE )
		decode->dst = 16;

	EXTRA_S = (EXTRA_S & ~1) + SREG;

	SET_ILC(cpustate->instruction_length & 3);

	SET_DREG((PC & 0xfffffffe) | GET_S);
	SET_DREGF(SR);

	SET_FP(GET_FP + decode->dst);

	SET_FL(6); //default value for call
	SET_M(0);

	PPC = PC;
	PC = EXTRA_S; // const value

	cpustate->intblock = 2;

	//TODO: add interrupt locks, errors, ....

	//TODO: no 1!
	cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bv(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_V )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bnv(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_V )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_be(hyperstone_state *cpustate, struct regs_decode *decode) //or BZ
{
	if( GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bne(hyperstone_state *cpustate, struct regs_decode *decode) //or BNZ
{
	if( !GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bc(hyperstone_state *cpustate, struct regs_decode *decode) //or BST
{
	if( GET_C )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bnc(hyperstone_state *cpustate, struct regs_decode *decode) //or BHE
{
	if( !GET_C )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bse(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_C || GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bht(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_C && !GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bn(hyperstone_state *cpustate, struct regs_decode *decode) //or BLT
{
	if( GET_N )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bnn(hyperstone_state *cpustate, struct regs_decode *decode) //or BGE
{
	if( !GET_N )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_ble(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( GET_N || GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_bgt(hyperstone_state *cpustate, struct regs_decode *decode)
{
	if( !GET_N && !GET_Z )
		execute_br(cpustate, decode);
	else
		cpustate->icount -= cpustate->clock_cycles_1;
}

INLINE void hyperstone_br(hyperstone_state *cpustate, struct regs_decode *decode)
{
	execute_br(cpustate, decode);
}

INLINE void hyperstone_trap(hyperstone_state *cpustate, struct regs_decode *decode)
{
	UINT8 code, trapno;
	UINT32 addr;

	trapno = (OP & 0xfc) >> 2;

	addr = get_trap_addr(cpustate, trapno);
	code = ((OP & 0x300) >> 6) | (OP & 0x03);

	switch( code )
	{
		case TRAPLE:
			if( GET_N || GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPGT:
			if( !GET_N && !GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPLT:
			if( GET_N )
				execute_trap(cpustate, addr);

			break;

		case TRAPGE:
			if( !GET_N )
				execute_trap(cpustate, addr);

			break;

		case TRAPSE:
			if( GET_C || GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPHT:
			if( !GET_C && !GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPST:
			if( GET_C )
				execute_trap(cpustate, addr);

			break;

		case TRAPHE:
			if( !GET_C )
				execute_trap(cpustate, addr);

			break;

		case TRAPE:
			if( GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPNE:
			if( !GET_Z )
				execute_trap(cpustate, addr);

			break;

		case TRAPV:
			if( GET_V )
				execute_trap(cpustate, addr);

			break;

		case TRAP:
			execute_trap(cpustate, addr);

			break;
	}

	cpustate->icount -= cpustate->clock_cycles_1;
}


#include "e132xsop.c"


static CPU_EXECUTE( hyperstone )
{
	hyperstone_state *cpustate = get_safe_token(device);

	if (cpustate->intblock < 0)
		cpustate->intblock = 0;
	check_interrupts(cpustate);

	do
	{
		UINT32 oldh = SR & 0x00000020;

		PPC = PC;	/* copy PC to previous PC */
		debugger_instruction_hook(device, PC);

		OP = READ_OP(cpustate, PC);
		PC += 2;

		cpustate->instruction_length = 1;

		/* execute opcode */
		(*hyperstone_op[(OP & 0xff00) >> 8])(cpustate);

		/* clear the H state if it was previously set */
		SR ^= oldh;

		SET_ILC(cpustate->instruction_length & 3);

		if( GET_T && GET_P && cpustate->delay.delay_cmd == NO_DELAY ) /* Not in a Delayed Branch instructions */
		{
			UINT32 addr = get_trap_addr(cpustate, TRAPNO_TRACE_EXCEPTION);
			execute_exception(cpustate, addr);
		}

		if (--cpustate->intblock == 0)
			check_interrupts(cpustate);

	} while( cpustate->icount > 0 );
}


/**************************************************************************
 * Generic set_info
 **************************************************************************/

static CPU_SET_INFO( hyperstone )
{
	hyperstone_state *cpustate = get_safe_token(device);
	switch (state)
	{
		/* --- the following bits of info are set as 64-bit signed integers --- */

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + E132XS_PC:			PC = info->i;						break;
		case CPUINFO_INT_REGISTER + E132XS_SR:			SR = info->i;						break;
		case CPUINFO_INT_REGISTER + E132XS_FER:			FER = info->i;						break;
		case CPUINFO_INT_REGISTER + E132XS_G3:			set_global_register(cpustate, 3, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G4:			set_global_register(cpustate, 4, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G5:			set_global_register(cpustate, 5, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G6:			set_global_register(cpustate, 6, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G7:			set_global_register(cpustate, 7, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G8:			set_global_register(cpustate, 8, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G9:			set_global_register(cpustate, 9, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G10:			set_global_register(cpustate, 10, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G11:			set_global_register(cpustate, 11, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G12:			set_global_register(cpustate, 12, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G13:			set_global_register(cpustate, 13, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G14:			set_global_register(cpustate, 14, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G15:			set_global_register(cpustate, 15, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G16:			set_global_register(cpustate, 16, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G17:			set_global_register(cpustate, 17, info->i);	break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + E132XS_SP:			SP  = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_UB:			UB  = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_BCR:			BCR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_TPR:			TPR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_TCR:			TCR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_TR:			set_global_register(cpustate, TR_REGISTER, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_WCR:			WCR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_ISR:			ISR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_FCR:			FCR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_MCR:			MCR = info->i;							break;
		case CPUINFO_INT_REGISTER + E132XS_G28:			set_global_register(cpustate, 28, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G29:			set_global_register(cpustate, 29, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G30:			set_global_register(cpustate, 30, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_G31:			set_global_register(cpustate, 31, info->i);	break;
		case CPUINFO_INT_REGISTER + E132XS_CL0:			cpustate->local_regs[(0 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL1:			cpustate->local_regs[(1 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL2:			cpustate->local_regs[(2 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL3:			cpustate->local_regs[(3 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL4:			cpustate->local_regs[(4 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL5:			cpustate->local_regs[(5 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL6:			cpustate->local_regs[(6 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL7:			cpustate->local_regs[(7 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL8:			cpustate->local_regs[(8 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL9:			cpustate->local_regs[(9 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL10:		cpustate->local_regs[(10 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL11:		cpustate->local_regs[(11 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL12:		cpustate->local_regs[(12 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL13:		cpustate->local_regs[(13 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL14:		cpustate->local_regs[(14 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_CL15:		cpustate->local_regs[(15 + GET_FP) % 64] = info->i; break;
		case CPUINFO_INT_REGISTER + E132XS_L0:			cpustate->local_regs[0] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L1:			cpustate->local_regs[1] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L2:			cpustate->local_regs[2] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L3:			cpustate->local_regs[3] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L4:			cpustate->local_regs[4] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L5:			cpustate->local_regs[5] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L6:			cpustate->local_regs[6] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L7:			cpustate->local_regs[7] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L8:			cpustate->local_regs[8] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L9:			cpustate->local_regs[9] = info->i;		break;
		case CPUINFO_INT_REGISTER + E132XS_L10:			cpustate->local_regs[10] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L11:			cpustate->local_regs[11] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L12:			cpustate->local_regs[12] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L13:			cpustate->local_regs[13] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L14:			cpustate->local_regs[14] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L15:			cpustate->local_regs[15] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L16:			cpustate->local_regs[16] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L17:			cpustate->local_regs[17] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L18:			cpustate->local_regs[18] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L19:			cpustate->local_regs[19] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L20:			cpustate->local_regs[20] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L21:			cpustate->local_regs[21] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L22:			cpustate->local_regs[22] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L23:			cpustate->local_regs[23] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L24:			cpustate->local_regs[24] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L25:			cpustate->local_regs[25] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L26:			cpustate->local_regs[26] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L27:			cpustate->local_regs[27] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L28:			cpustate->local_regs[28] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L29:			cpustate->local_regs[29] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L30:			cpustate->local_regs[30] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L31:			cpustate->local_regs[31] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L32:			cpustate->local_regs[32] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L33:			cpustate->local_regs[33] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L34:			cpustate->local_regs[34] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L35:			cpustate->local_regs[35] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L36:			cpustate->local_regs[36] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L37:			cpustate->local_regs[37] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L38:			cpustate->local_regs[38] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L39:			cpustate->local_regs[39] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L40:			cpustate->local_regs[40] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L41:			cpustate->local_regs[41] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L42:			cpustate->local_regs[42] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L43:			cpustate->local_regs[43] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L44:			cpustate->local_regs[44] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L45:			cpustate->local_regs[45] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L46:			cpustate->local_regs[46] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L47:			cpustate->local_regs[47] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L48:			cpustate->local_regs[48] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L49:			cpustate->local_regs[49] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L50:			cpustate->local_regs[50] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L51:			cpustate->local_regs[51] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L52:			cpustate->local_regs[52] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L53:			cpustate->local_regs[53] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L54:			cpustate->local_regs[54] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L55:			cpustate->local_regs[55] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L56:			cpustate->local_regs[56] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L57:			cpustate->local_regs[57] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L58:			cpustate->local_regs[58] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L59:			cpustate->local_regs[59] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L60:			cpustate->local_regs[60] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L61:			cpustate->local_regs[61] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L62:			cpustate->local_regs[62] = info->i;	break;
		case CPUINFO_INT_REGISTER + E132XS_L63:			cpustate->local_regs[63] = info->i;	break;

		case CPUINFO_INT_INPUT_STATE + 0:				set_irq_line(cpustate, 0, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 1:				set_irq_line(cpustate, 1, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 2:				set_irq_line(cpustate, 2, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 3:				set_irq_line(cpustate, 3, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 4:				set_irq_line(cpustate, 4, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 5:				set_irq_line(cpustate, 5, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 6:				set_irq_line(cpustate, 6, info->i);				break;
		case CPUINFO_INT_INPUT_STATE + 7:				set_irq_line(cpustate, 7, info->i);				break;
	}
}

/**************************************************************************
 * Generic get_info
 **************************************************************************/

static CPU_GET_INFO( hyperstone )
{
	hyperstone_state *cpustate = (device != NULL && device->token() != NULL) ? get_safe_token(device) : NULL;
	switch (state)
	{
		/* --- the following bits of info are returned as 64-bit signed integers --- */
		case CPUINFO_INT_CONTEXT_SIZE:					info->i = sizeof(hyperstone_state);		break;
		case CPUINFO_INT_INPUT_LINES:					info->i = 8;							break;
		case CPUINFO_INT_DEFAULT_IRQ_VECTOR:			info->i = 0;							break;
		case CPUINFO_INT_ENDIANNESS:					info->i = ENDIANNESS_BIG;					break;
		case CPUINFO_INT_CLOCK_MULTIPLIER:				info->i = 1;							break;
		case CPUINFO_INT_CLOCK_DIVIDER:					info->i = 1;							break;
		case CPUINFO_INT_MIN_INSTRUCTION_BYTES:			info->i = 2;							break;
		case CPUINFO_INT_MAX_INSTRUCTION_BYTES:			info->i = 6;							break;
		case CPUINFO_INT_MIN_CYCLES:					info->i = 1;							break;
		case CPUINFO_INT_MAX_CYCLES:					info->i = 36;							break;

		case CPUINFO_INT_ADDRBUS_WIDTH + AS_PROGRAM: info->i = 32;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_PROGRAM: info->i = 0;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_DATA:	info->i = 0;					break;
		case CPUINFO_INT_ADDRBUS_WIDTH + AS_IO:		info->i = 15;					break;
		case CPUINFO_INT_ADDRBUS_SHIFT + AS_IO:		info->i = 0;					break;

		case CPUINFO_INT_INPUT_STATE + 0:					/* not implemented */				break;

		case CPUINFO_INT_PREVIOUSPC:					info->i = PPC;							break;

		case CPUINFO_INT_PC:
		case CPUINFO_INT_REGISTER + E132XS_PC:			info->i =  PC;							break;
		case CPUINFO_INT_REGISTER + E132XS_SR:			info->i =  SR;							break;
		case CPUINFO_INT_REGISTER + E132XS_FER:			info->i =  FER;							break;
		case CPUINFO_INT_REGISTER + E132XS_G3:			info->i =  get_global_register(cpustate, 3);	break;
		case CPUINFO_INT_REGISTER + E132XS_G4:			info->i =  get_global_register(cpustate, 4);	break;
		case CPUINFO_INT_REGISTER + E132XS_G5:			info->i =  get_global_register(cpustate, 5);	break;
		case CPUINFO_INT_REGISTER + E132XS_G6:			info->i =  get_global_register(cpustate, 6);	break;
		case CPUINFO_INT_REGISTER + E132XS_G7:			info->i =  get_global_register(cpustate, 7);	break;
		case CPUINFO_INT_REGISTER + E132XS_G8:			info->i =  get_global_register(cpustate, 8);	break;
		case CPUINFO_INT_REGISTER + E132XS_G9:			info->i =  get_global_register(cpustate, 9);	break;
		case CPUINFO_INT_REGISTER + E132XS_G10:			info->i =  get_global_register(cpustate, 10);	break;
		case CPUINFO_INT_REGISTER + E132XS_G11:			info->i =  get_global_register(cpustate, 11);	break;
		case CPUINFO_INT_REGISTER + E132XS_G12:			info->i =  get_global_register(cpustate, 12);	break;
		case CPUINFO_INT_REGISTER + E132XS_G13:			info->i =  get_global_register(cpustate, 13);	break;
		case CPUINFO_INT_REGISTER + E132XS_G14:			info->i =  get_global_register(cpustate, 14);	break;
		case CPUINFO_INT_REGISTER + E132XS_G15:			info->i =  get_global_register(cpustate, 15);	break;
		case CPUINFO_INT_REGISTER + E132XS_G16:			info->i =  get_global_register(cpustate, 16);	break;
		case CPUINFO_INT_REGISTER + E132XS_G17:			info->i =  get_global_register(cpustate, 17);	break;
		case CPUINFO_INT_SP:
		case CPUINFO_INT_REGISTER + E132XS_SP:			info->i =  SP;							break;
		case CPUINFO_INT_REGISTER + E132XS_UB:			info->i =  UB;							break;
		case CPUINFO_INT_REGISTER + E132XS_BCR:			info->i =  BCR;							break;
		case CPUINFO_INT_REGISTER + E132XS_TPR:			info->i =  TPR;							break;
		case CPUINFO_INT_REGISTER + E132XS_TCR:			info->i =  TCR;							break;
		case CPUINFO_INT_REGISTER + E132XS_TR:			info->i =  TR;							break;
		case CPUINFO_INT_REGISTER + E132XS_WCR:			info->i =  WCR;							break;
		case CPUINFO_INT_REGISTER + E132XS_ISR:			info->i =  ISR;							break;
		case CPUINFO_INT_REGISTER + E132XS_FCR:			info->i =  FCR;							break;
		case CPUINFO_INT_REGISTER + E132XS_MCR:			info->i =  MCR;							break;
		case CPUINFO_INT_REGISTER + E132XS_G28:			info->i =  get_global_register(cpustate, 28);	break;
		case CPUINFO_INT_REGISTER + E132XS_G29:			info->i =  get_global_register(cpustate, 29);	break;
		case CPUINFO_INT_REGISTER + E132XS_G30:			info->i =  get_global_register(cpustate, 30);	break;
		case CPUINFO_INT_REGISTER + E132XS_G31:			info->i =  get_global_register(cpustate, 31);	break;
		case CPUINFO_INT_REGISTER + E132XS_CL0:			info->i =  cpustate->local_regs[(0 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL1:			info->i =  cpustate->local_regs[(1 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL2:			info->i =  cpustate->local_regs[(2 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL3:			info->i =  cpustate->local_regs[(3 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL4:			info->i =  cpustate->local_regs[(4 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL5:			info->i =  cpustate->local_regs[(5 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL6:			info->i =  cpustate->local_regs[(6 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL7:			info->i =  cpustate->local_regs[(7 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL8:			info->i =  cpustate->local_regs[(8 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL9:			info->i =  cpustate->local_regs[(9 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL10:		info->i =  cpustate->local_regs[(10 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL11:		info->i =  cpustate->local_regs[(11 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL12:		info->i =  cpustate->local_regs[(12 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL13:		info->i =  cpustate->local_regs[(13 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL14:		info->i =  cpustate->local_regs[(14 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_CL15:		info->i =  cpustate->local_regs[(15 + GET_FP) % 64]; break;
		case CPUINFO_INT_REGISTER + E132XS_L0:			info->i =  cpustate->local_regs[0];	break;
		case CPUINFO_INT_REGISTER + E132XS_L1:			info->i =  cpustate->local_regs[1];	break;
		case CPUINFO_INT_REGISTER + E132XS_L2:			info->i =  cpustate->local_regs[2];	break;
		case CPUINFO_INT_REGISTER + E132XS_L3:			info->i =  cpustate->local_regs[3];	break;
		case CPUINFO_INT_REGISTER + E132XS_L4:			info->i =  cpustate->local_regs[4];	break;
		case CPUINFO_INT_REGISTER + E132XS_L5:			info->i =  cpustate->local_regs[5];	break;
		case CPUINFO_INT_REGISTER + E132XS_L6:			info->i =  cpustate->local_regs[6];	break;
		case CPUINFO_INT_REGISTER + E132XS_L7:			info->i =  cpustate->local_regs[7];	break;
		case CPUINFO_INT_REGISTER + E132XS_L8:			info->i =  cpustate->local_regs[8];	break;
		case CPUINFO_INT_REGISTER + E132XS_L9:			info->i =  cpustate->local_regs[9];	break;
		case CPUINFO_INT_REGISTER + E132XS_L10:			info->i =  cpustate->local_regs[10];	break;
		case CPUINFO_INT_REGISTER + E132XS_L11:			info->i =  cpustate->local_regs[11];	break;
		case CPUINFO_INT_REGISTER + E132XS_L12:			info->i =  cpustate->local_regs[12];	break;
		case CPUINFO_INT_REGISTER + E132XS_L13:			info->i =  cpustate->local_regs[13];	break;
		case CPUINFO_INT_REGISTER + E132XS_L14:			info->i =  cpustate->local_regs[14];	break;
		case CPUINFO_INT_REGISTER + E132XS_L15:			info->i =  cpustate->local_regs[15];	break;
		case CPUINFO_INT_REGISTER + E132XS_L16:			info->i =  cpustate->local_regs[16];	break;
		case CPUINFO_INT_REGISTER + E132XS_L17:			info->i =  cpustate->local_regs[17];	break;
		case CPUINFO_INT_REGISTER + E132XS_L18:			info->i =  cpustate->local_regs[18];	break;
		case CPUINFO_INT_REGISTER + E132XS_L19:			info->i =  cpustate->local_regs[19];	break;
		case CPUINFO_INT_REGISTER + E132XS_L20:			info->i =  cpustate->local_regs[20];	break;
		case CPUINFO_INT_REGISTER + E132XS_L21:			info->i =  cpustate->local_regs[21];	break;
		case CPUINFO_INT_REGISTER + E132XS_L22:			info->i =  cpustate->local_regs[22];	break;
		case CPUINFO_INT_REGISTER + E132XS_L23:			info->i =  cpustate->local_regs[23];	break;
		case CPUINFO_INT_REGISTER + E132XS_L24:			info->i =  cpustate->local_regs[24];	break;
		case CPUINFO_INT_REGISTER + E132XS_L25:			info->i =  cpustate->local_regs[25];	break;
		case CPUINFO_INT_REGISTER + E132XS_L26:			info->i =  cpustate->local_regs[26];	break;
		case CPUINFO_INT_REGISTER + E132XS_L27:			info->i =  cpustate->local_regs[27];	break;
		case CPUINFO_INT_REGISTER + E132XS_L28:			info->i =  cpustate->local_regs[28];	break;
		case CPUINFO_INT_REGISTER + E132XS_L29:			info->i =  cpustate->local_regs[29];	break;
		case CPUINFO_INT_REGISTER + E132XS_L30:			info->i =  cpustate->local_regs[30];	break;
		case CPUINFO_INT_REGISTER + E132XS_L31:			info->i =  cpustate->local_regs[31];	break;
		case CPUINFO_INT_REGISTER + E132XS_L32:			info->i =  cpustate->local_regs[32];	break;
		case CPUINFO_INT_REGISTER + E132XS_L33:			info->i =  cpustate->local_regs[33];	break;
		case CPUINFO_INT_REGISTER + E132XS_L34:			info->i =  cpustate->local_regs[34];	break;
		case CPUINFO_INT_REGISTER + E132XS_L35:			info->i =  cpustate->local_regs[35];	break;
		case CPUINFO_INT_REGISTER + E132XS_L36:			info->i =  cpustate->local_regs[36];	break;
		case CPUINFO_INT_REGISTER + E132XS_L37:			info->i =  cpustate->local_regs[37];	break;
		case CPUINFO_INT_REGISTER + E132XS_L38:			info->i =  cpustate->local_regs[38];	break;
		case CPUINFO_INT_REGISTER + E132XS_L39:			info->i =  cpustate->local_regs[39];	break;
		case CPUINFO_INT_REGISTER + E132XS_L40:			info->i =  cpustate->local_regs[40];	break;
		case CPUINFO_INT_REGISTER + E132XS_L41:			info->i =  cpustate->local_regs[41];	break;
		case CPUINFO_INT_REGISTER + E132XS_L42:			info->i =  cpustate->local_regs[42];	break;
		case CPUINFO_INT_REGISTER + E132XS_L43:			info->i =  cpustate->local_regs[43];	break;
		case CPUINFO_INT_REGISTER + E132XS_L44:			info->i =  cpustate->local_regs[44];	break;
		case CPUINFO_INT_REGISTER + E132XS_L45:			info->i =  cpustate->local_regs[45];	break;
		case CPUINFO_INT_REGISTER + E132XS_L46:			info->i =  cpustate->local_regs[46];	break;
		case CPUINFO_INT_REGISTER + E132XS_L47:			info->i =  cpustate->local_regs[47];	break;
		case CPUINFO_INT_REGISTER + E132XS_L48:			info->i =  cpustate->local_regs[48];	break;
		case CPUINFO_INT_REGISTER + E132XS_L49:			info->i =  cpustate->local_regs[49];	break;
		case CPUINFO_INT_REGISTER + E132XS_L50:			info->i =  cpustate->local_regs[50];	break;
		case CPUINFO_INT_REGISTER + E132XS_L51:			info->i =  cpustate->local_regs[51];	break;
		case CPUINFO_INT_REGISTER + E132XS_L52:			info->i =  cpustate->local_regs[52];	break;
		case CPUINFO_INT_REGISTER + E132XS_L53:			info->i =  cpustate->local_regs[53];	break;
		case CPUINFO_INT_REGISTER + E132XS_L54:			info->i =  cpustate->local_regs[54];	break;
		case CPUINFO_INT_REGISTER + E132XS_L55:			info->i =  cpustate->local_regs[55];	break;
		case CPUINFO_INT_REGISTER + E132XS_L56:			info->i =  cpustate->local_regs[56];	break;
		case CPUINFO_INT_REGISTER + E132XS_L57:			info->i =  cpustate->local_regs[57];	break;
		case CPUINFO_INT_REGISTER + E132XS_L58:			info->i =  cpustate->local_regs[58];	break;
		case CPUINFO_INT_REGISTER + E132XS_L59:			info->i =  cpustate->local_regs[59];	break;
		case CPUINFO_INT_REGISTER + E132XS_L60:			info->i =  cpustate->local_regs[60];	break;
		case CPUINFO_INT_REGISTER + E132XS_L61:			info->i =  cpustate->local_regs[61];	break;
		case CPUINFO_INT_REGISTER + E132XS_L62:			info->i =  cpustate->local_regs[62];	break;
		case CPUINFO_INT_REGISTER + E132XS_L63:			info->i =  cpustate->local_regs[63];	break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_SET_INFO:						info->setinfo = CPU_SET_INFO_NAME(hyperstone);	break;
		case CPUINFO_FCT_INIT:							info->init = NULL;						break;
		case CPUINFO_FCT_RESET:							info->reset = CPU_RESET_NAME(hyperstone);			break;
		case CPUINFO_FCT_EXIT:							info->exit = CPU_EXIT_NAME(hyperstone);			break;
		case CPUINFO_FCT_EXECUTE:						info->execute = CPU_EXECUTE_NAME(hyperstone);		break;
		case CPUINFO_FCT_BURN:							info->burn = NULL;						break;
		case CPUINFO_FCT_DISASSEMBLE:					info->disassemble = CPU_DISASSEMBLE_NAME(hyperstone);	break;
		case CPUINFO_PTR_INSTRUCTION_COUNTER:			info->icount = &cpustate->icount;		break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_DATA:    info->internal_map16 = NULL;	break;
		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_IO:      info->internal_map16 = NULL;	break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_FAMILY:					strcpy(info->s, "Hyperstone CPU");		break;
		case CPUINFO_STR_VERSION:					strcpy(info->s, "0.9");					break;
		case CPUINFO_STR_SOURCE_FILE:						strcpy(info->s, __FILE__);				break;
		case CPUINFO_STR_CREDITS:					strcpy(info->s, "Copyright Pierpaolo Prazzoli and Ryan Holtz"); break;

		case CPUINFO_STR_FLAGS:
			sprintf(info->s, "%c%c%c%c%c%c%c%c%c%c%c%c FTE:%X FRM:%X ILC:%d FL:%d FP:%d",
				GET_S ? 'S':'.',
				GET_P ? 'P':'.',
				GET_T ? 'T':'.',
				GET_L ? 'L':'.',
				GET_I ? 'I':'.',
				cpustate->global_regs[1] & 0x00040 ? '?':'.',
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

		case CPUINFO_STR_REGISTER + E132XS_PC:  		sprintf(info->s, "PC  :%08X", cpustate->global_regs[0]); break;
		case CPUINFO_STR_REGISTER + E132XS_SR:  		sprintf(info->s, "SR  :%08X", cpustate->global_regs[1]); break;
		case CPUINFO_STR_REGISTER + E132XS_FER: 		sprintf(info->s, "FER :%08X", cpustate->global_regs[2]); break;
		case CPUINFO_STR_REGISTER + E132XS_G3:  		sprintf(info->s, "G3  :%08X", cpustate->global_regs[3]); break;
		case CPUINFO_STR_REGISTER + E132XS_G4:  		sprintf(info->s, "G4  :%08X", cpustate->global_regs[4]); break;
		case CPUINFO_STR_REGISTER + E132XS_G5:  		sprintf(info->s, "G5  :%08X", cpustate->global_regs[5]); break;
		case CPUINFO_STR_REGISTER + E132XS_G6:  		sprintf(info->s, "G6  :%08X", cpustate->global_regs[6]); break;
		case CPUINFO_STR_REGISTER + E132XS_G7:  		sprintf(info->s, "G7  :%08X", cpustate->global_regs[7]); break;
		case CPUINFO_STR_REGISTER + E132XS_G8:  		sprintf(info->s, "G8  :%08X", cpustate->global_regs[8]); break;
		case CPUINFO_STR_REGISTER + E132XS_G9:  		sprintf(info->s, "G9  :%08X", cpustate->global_regs[9]); break;
		case CPUINFO_STR_REGISTER + E132XS_G10: 		sprintf(info->s, "G10 :%08X", cpustate->global_regs[10]); break;
		case CPUINFO_STR_REGISTER + E132XS_G11: 		sprintf(info->s, "G11 :%08X", cpustate->global_regs[11]); break;
		case CPUINFO_STR_REGISTER + E132XS_G12: 		sprintf(info->s, "G12 :%08X", cpustate->global_regs[12]); break;
		case CPUINFO_STR_REGISTER + E132XS_G13: 		sprintf(info->s, "G13 :%08X", cpustate->global_regs[13]); break;
		case CPUINFO_STR_REGISTER + E132XS_G14: 		sprintf(info->s, "G14 :%08X", cpustate->global_regs[14]); break;
		case CPUINFO_STR_REGISTER + E132XS_G15: 		sprintf(info->s, "G15 :%08X", cpustate->global_regs[15]); break;
		case CPUINFO_STR_REGISTER + E132XS_G16: 		sprintf(info->s, "G16 :%08X", cpustate->global_regs[16]); break;
		case CPUINFO_STR_REGISTER + E132XS_G17: 		sprintf(info->s, "G17 :%08X", cpustate->global_regs[17]); break;
		case CPUINFO_STR_REGISTER + E132XS_SP:  		sprintf(info->s, "SP  :%08X", cpustate->global_regs[18]); break;
		case CPUINFO_STR_REGISTER + E132XS_UB:  		sprintf(info->s, "UB  :%08X", cpustate->global_regs[19]); break;
		case CPUINFO_STR_REGISTER + E132XS_BCR: 		sprintf(info->s, "BCR :%08X", cpustate->global_regs[20]); break;
		case CPUINFO_STR_REGISTER + E132XS_TPR: 		sprintf(info->s, "TPR :%08X", cpustate->global_regs[21]); break;
		case CPUINFO_STR_REGISTER + E132XS_TCR: 		sprintf(info->s, "TCR :%08X", cpustate->global_regs[22]); break;
		case CPUINFO_STR_REGISTER + E132XS_TR:  		sprintf(info->s, "TR  :%08X", cpustate->global_regs[23]); break;
		case CPUINFO_STR_REGISTER + E132XS_WCR: 		sprintf(info->s, "WCR :%08X", cpustate->global_regs[24]); break;
		case CPUINFO_STR_REGISTER + E132XS_ISR: 		sprintf(info->s, "ISR :%08X", cpustate->global_regs[25]); break;
		case CPUINFO_STR_REGISTER + E132XS_FCR: 		sprintf(info->s, "FCR :%08X", cpustate->global_regs[26]); break;
		case CPUINFO_STR_REGISTER + E132XS_MCR: 		sprintf(info->s, "MCR :%08X", cpustate->global_regs[27]); break;
		case CPUINFO_STR_REGISTER + E132XS_G28: 		sprintf(info->s, "G28 :%08X", cpustate->global_regs[28]); break;
		case CPUINFO_STR_REGISTER + E132XS_G29: 		sprintf(info->s, "G29 :%08X", cpustate->global_regs[29]); break;
		case CPUINFO_STR_REGISTER + E132XS_G30: 		sprintf(info->s, "G30 :%08X", cpustate->global_regs[30]); break;
		case CPUINFO_STR_REGISTER + E132XS_G31: 		sprintf(info->s, "G31 :%08X", cpustate->global_regs[31]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL0: 		sprintf(info->s, "CL0 :%08X", cpustate->local_regs[(0 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL1: 		sprintf(info->s, "CL1 :%08X", cpustate->local_regs[(1 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL2: 		sprintf(info->s, "CL2 :%08X", cpustate->local_regs[(2 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL3: 		sprintf(info->s, "CL3 :%08X", cpustate->local_regs[(3 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL4: 		sprintf(info->s, "CL4 :%08X", cpustate->local_regs[(4 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL5: 		sprintf(info->s, "CL5 :%08X", cpustate->local_regs[(5 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL6: 		sprintf(info->s, "CL6 :%08X", cpustate->local_regs[(6 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL7: 		sprintf(info->s, "CL7 :%08X", cpustate->local_regs[(7 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL8: 		sprintf(info->s, "CL8 :%08X", cpustate->local_regs[(8 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL9: 		sprintf(info->s, "CL9 :%08X", cpustate->local_regs[(9 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL10:		sprintf(info->s, "CL10:%08X", cpustate->local_regs[(10 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL11:		sprintf(info->s, "CL11:%08X", cpustate->local_regs[(11 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL12:		sprintf(info->s, "CL12:%08X", cpustate->local_regs[(12 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL13:		sprintf(info->s, "CL13:%08X", cpustate->local_regs[(13 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL14:		sprintf(info->s, "CL14:%08X", cpustate->local_regs[(14 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_CL15:		sprintf(info->s, "CL15:%08X", cpustate->local_regs[(15 + GET_FP) % 64]); break;
		case CPUINFO_STR_REGISTER + E132XS_L0:  		sprintf(info->s, "L0  :%08X", cpustate->local_regs[0]); break;
		case CPUINFO_STR_REGISTER + E132XS_L1:  		sprintf(info->s, "L1  :%08X", cpustate->local_regs[1]); break;
		case CPUINFO_STR_REGISTER + E132XS_L2:  		sprintf(info->s, "L2  :%08X", cpustate->local_regs[2]); break;
		case CPUINFO_STR_REGISTER + E132XS_L3:  		sprintf(info->s, "L3  :%08X", cpustate->local_regs[3]); break;
		case CPUINFO_STR_REGISTER + E132XS_L4:  		sprintf(info->s, "L4  :%08X", cpustate->local_regs[4]); break;
		case CPUINFO_STR_REGISTER + E132XS_L5:  		sprintf(info->s, "L5  :%08X", cpustate->local_regs[5]); break;
		case CPUINFO_STR_REGISTER + E132XS_L6:  		sprintf(info->s, "L6  :%08X", cpustate->local_regs[6]); break;
		case CPUINFO_STR_REGISTER + E132XS_L7:  		sprintf(info->s, "L7  :%08X", cpustate->local_regs[7]); break;
		case CPUINFO_STR_REGISTER + E132XS_L8:  		sprintf(info->s, "L8  :%08X", cpustate->local_regs[8]); break;
		case CPUINFO_STR_REGISTER + E132XS_L9:  		sprintf(info->s, "L9  :%08X", cpustate->local_regs[9]); break;
		case CPUINFO_STR_REGISTER + E132XS_L10: 		sprintf(info->s, "L10 :%08X", cpustate->local_regs[10]); break;
		case CPUINFO_STR_REGISTER + E132XS_L11: 		sprintf(info->s, "L11 :%08X", cpustate->local_regs[11]); break;
		case CPUINFO_STR_REGISTER + E132XS_L12: 		sprintf(info->s, "L12 :%08X", cpustate->local_regs[12]); break;
		case CPUINFO_STR_REGISTER + E132XS_L13: 		sprintf(info->s, "L13 :%08X", cpustate->local_regs[13]); break;
		case CPUINFO_STR_REGISTER + E132XS_L14: 		sprintf(info->s, "L14 :%08X", cpustate->local_regs[14]); break;
		case CPUINFO_STR_REGISTER + E132XS_L15: 		sprintf(info->s, "L15 :%08X", cpustate->local_regs[15]); break;
		case CPUINFO_STR_REGISTER + E132XS_L16: 		sprintf(info->s, "L16 :%08X", cpustate->local_regs[16]); break;
		case CPUINFO_STR_REGISTER + E132XS_L17: 		sprintf(info->s, "L17 :%08X", cpustate->local_regs[17]); break;
		case CPUINFO_STR_REGISTER + E132XS_L18: 		sprintf(info->s, "L18 :%08X", cpustate->local_regs[18]); break;
		case CPUINFO_STR_REGISTER + E132XS_L19: 		sprintf(info->s, "L19 :%08X", cpustate->local_regs[19]); break;
		case CPUINFO_STR_REGISTER + E132XS_L20: 		sprintf(info->s, "L20 :%08X", cpustate->local_regs[20]); break;
		case CPUINFO_STR_REGISTER + E132XS_L21: 		sprintf(info->s, "L21 :%08X", cpustate->local_regs[21]); break;
		case CPUINFO_STR_REGISTER + E132XS_L22: 		sprintf(info->s, "L22 :%08X", cpustate->local_regs[22]); break;
		case CPUINFO_STR_REGISTER + E132XS_L23: 		sprintf(info->s, "L23 :%08X", cpustate->local_regs[23]); break;
		case CPUINFO_STR_REGISTER + E132XS_L24: 		sprintf(info->s, "L24 :%08X", cpustate->local_regs[24]); break;
		case CPUINFO_STR_REGISTER + E132XS_L25: 		sprintf(info->s, "L25 :%08X", cpustate->local_regs[25]); break;
		case CPUINFO_STR_REGISTER + E132XS_L26: 		sprintf(info->s, "L26 :%08X", cpustate->local_regs[26]); break;
		case CPUINFO_STR_REGISTER + E132XS_L27: 		sprintf(info->s, "L27 :%08X", cpustate->local_regs[27]); break;
		case CPUINFO_STR_REGISTER + E132XS_L28: 		sprintf(info->s, "L28 :%08X", cpustate->local_regs[28]); break;
		case CPUINFO_STR_REGISTER + E132XS_L29: 		sprintf(info->s, "L29 :%08X", cpustate->local_regs[29]); break;
		case CPUINFO_STR_REGISTER + E132XS_L30: 		sprintf(info->s, "L30 :%08X", cpustate->local_regs[30]); break;
		case CPUINFO_STR_REGISTER + E132XS_L31: 		sprintf(info->s, "L31 :%08X", cpustate->local_regs[31]); break;
		case CPUINFO_STR_REGISTER + E132XS_L32: 		sprintf(info->s, "L32 :%08X", cpustate->local_regs[32]); break;
		case CPUINFO_STR_REGISTER + E132XS_L33: 		sprintf(info->s, "L33 :%08X", cpustate->local_regs[33]); break;
		case CPUINFO_STR_REGISTER + E132XS_L34: 		sprintf(info->s, "L34 :%08X", cpustate->local_regs[34]); break;
		case CPUINFO_STR_REGISTER + E132XS_L35: 		sprintf(info->s, "L35 :%08X", cpustate->local_regs[35]); break;
		case CPUINFO_STR_REGISTER + E132XS_L36: 		sprintf(info->s, "L36 :%08X", cpustate->local_regs[36]); break;
		case CPUINFO_STR_REGISTER + E132XS_L37: 		sprintf(info->s, "L37 :%08X", cpustate->local_regs[37]); break;
		case CPUINFO_STR_REGISTER + E132XS_L38: 		sprintf(info->s, "L38 :%08X", cpustate->local_regs[38]); break;
		case CPUINFO_STR_REGISTER + E132XS_L39: 		sprintf(info->s, "L39 :%08X", cpustate->local_regs[39]); break;
		case CPUINFO_STR_REGISTER + E132XS_L40: 		sprintf(info->s, "L40 :%08X", cpustate->local_regs[40]); break;
		case CPUINFO_STR_REGISTER + E132XS_L41: 		sprintf(info->s, "L41 :%08X", cpustate->local_regs[41]); break;
		case CPUINFO_STR_REGISTER + E132XS_L42: 		sprintf(info->s, "L42 :%08X", cpustate->local_regs[42]); break;
		case CPUINFO_STR_REGISTER + E132XS_L43: 		sprintf(info->s, "L43 :%08X", cpustate->local_regs[43]); break;
		case CPUINFO_STR_REGISTER + E132XS_L44: 		sprintf(info->s, "L44 :%08X", cpustate->local_regs[44]); break;
		case CPUINFO_STR_REGISTER + E132XS_L45: 		sprintf(info->s, "L45 :%08X", cpustate->local_regs[45]); break;
		case CPUINFO_STR_REGISTER + E132XS_L46: 		sprintf(info->s, "L46 :%08X", cpustate->local_regs[46]); break;
		case CPUINFO_STR_REGISTER + E132XS_L47: 		sprintf(info->s, "L47 :%08X", cpustate->local_regs[47]); break;
		case CPUINFO_STR_REGISTER + E132XS_L48: 		sprintf(info->s, "L48 :%08X", cpustate->local_regs[48]); break;
		case CPUINFO_STR_REGISTER + E132XS_L49: 		sprintf(info->s, "L49 :%08X", cpustate->local_regs[49]); break;
		case CPUINFO_STR_REGISTER + E132XS_L50: 		sprintf(info->s, "L50 :%08X", cpustate->local_regs[50]); break;
		case CPUINFO_STR_REGISTER + E132XS_L51: 		sprintf(info->s, "L51 :%08X", cpustate->local_regs[51]); break;
		case CPUINFO_STR_REGISTER + E132XS_L52: 		sprintf(info->s, "L52 :%08X", cpustate->local_regs[52]); break;
		case CPUINFO_STR_REGISTER + E132XS_L53: 		sprintf(info->s, "L53 :%08X", cpustate->local_regs[53]); break;
		case CPUINFO_STR_REGISTER + E132XS_L54: 		sprintf(info->s, "L54 :%08X", cpustate->local_regs[54]); break;
		case CPUINFO_STR_REGISTER + E132XS_L55: 		sprintf(info->s, "L55 :%08X", cpustate->local_regs[55]); break;
		case CPUINFO_STR_REGISTER + E132XS_L56: 		sprintf(info->s, "L56 :%08X", cpustate->local_regs[56]); break;
		case CPUINFO_STR_REGISTER + E132XS_L57: 		sprintf(info->s, "L57 :%08X", cpustate->local_regs[57]); break;
		case CPUINFO_STR_REGISTER + E132XS_L58: 		sprintf(info->s, "L58 :%08X", cpustate->local_regs[58]); break;
		case CPUINFO_STR_REGISTER + E132XS_L59: 		sprintf(info->s, "L59 :%08X", cpustate->local_regs[59]); break;
		case CPUINFO_STR_REGISTER + E132XS_L60: 		sprintf(info->s, "L60 :%08X", cpustate->local_regs[60]); break;
		case CPUINFO_STR_REGISTER + E132XS_L61: 		sprintf(info->s, "L61 :%08X", cpustate->local_regs[61]); break;
		case CPUINFO_STR_REGISTER + E132XS_L62: 		sprintf(info->s, "L62 :%08X", cpustate->local_regs[62]); break;
		case CPUINFO_STR_REGISTER + E132XS_L63: 		sprintf(info->s, "L63 :%08X", cpustate->local_regs[63]); break;
	}
}


CPU_GET_INFO( e116t )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_4k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e116t);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-16T");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e116xt )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_8k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e116xt);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-16XT");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e116xs )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_16k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e116xs);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-16XS");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e116xsr )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_16k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e116xsr);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-16XSR");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132n )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_4k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132n);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32N");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132t )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_4k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132t);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32T");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132xn )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_8k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132xn);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32XN");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132xt )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_8k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132xt);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32XT");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132xs )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_16k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132xs);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32XS");				break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( e132xsr )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_16k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(e132xsr);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "E1-32XSR");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( gms30c2116 )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_4k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(gms30c2116);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "GMS30C2116");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( gms30c2132 )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_4k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(gms30c2132);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "GMS30C2132");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( gms30c2216 )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 16;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 16;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map16 = ADDRESS_MAP_NAME(e116_8k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(gms30c2216);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "GMS30C2216");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

CPU_GET_INFO( gms30c2232 )
{
	switch (state)
	{

		case CPUINFO_INT_DATABUS_WIDTH + AS_PROGRAM:	info->i = 32;					break;
		case CPUINFO_INT_DATABUS_WIDTH + AS_IO:		info->i = 32;					break;

		case CPUINFO_PTR_INTERNAL_MEMORY_MAP + AS_PROGRAM: info->internal_map32 = ADDRESS_MAP_NAME(e132_8k_iram_map); break;

		/* --- the following bits of info are returned as pointers to data or functions --- */
		case CPUINFO_FCT_INIT:							info->init = CPU_INIT_NAME(gms30c2232);					break;

		/* --- the following bits of info are returned as NULL-terminated strings --- */
		case CPUINFO_STR_NAME:							strcpy(info->s, "GMS30C2232");			break;

		default:
			CPU_GET_INFO_CALL(hyperstone);
	}
}

DEFINE_LEGACY_CPU_DEVICE(E116T, e116t);
DEFINE_LEGACY_CPU_DEVICE(E116XT, e116xt);
DEFINE_LEGACY_CPU_DEVICE(E116XS, e116xs);
DEFINE_LEGACY_CPU_DEVICE(E116XSR, e116xsr);
DEFINE_LEGACY_CPU_DEVICE(E132N, e132n);
DEFINE_LEGACY_CPU_DEVICE(E132T, e132t);
DEFINE_LEGACY_CPU_DEVICE(E132XN, e132xn);
DEFINE_LEGACY_CPU_DEVICE(E132XT, e132xt);
DEFINE_LEGACY_CPU_DEVICE(E132XS, e132xs);
DEFINE_LEGACY_CPU_DEVICE(E132XSR, e132xsr);
DEFINE_LEGACY_CPU_DEVICE(GMS30C2116, gms30c2116);
DEFINE_LEGACY_CPU_DEVICE(GMS30C2132, gms30c2132);
DEFINE_LEGACY_CPU_DEVICE(GMS30C2216, gms30c2216);
DEFINE_LEGACY_CPU_DEVICE(GMS30C2232, gms30c2232);
