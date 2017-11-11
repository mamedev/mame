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
    - The mood struck me, and I took a look at set_local_register and GET_L_REG.
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
    - Changed hyperstone_movi to default to a uint32_t being moved into the register
      as opposed to a uint8_t. This is wrong, the bit width is quite likely to be
      dependent on the n field in the Rimm instruction type. However, vamphalf uses
      MOVI G0,[FFFF]FBAC (n=$13) since there's apparently no absolute branch opcode.
      What kind of CPU is this that it doesn't have an absolute jump in its branch
      instructions and you have to use an immediate MOV to do an abs. jump!?
    - Replaced usage of logerror() with smf's verboselog()

*********************************************************************/

#include "emu.h"
#include "e132xs.h"

#include "debugger.h"

#include "32xsdefs.h"

#ifdef MAME_DEBUG
#define DEBUG_PRINTF(x) do { osd_printf_debug x; } while (0)
#else
#define DEBUG_PRINTF(x) do { } while (0)
#endif

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

hyperstone_device::hyperstone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
										const device_type type, uint32_t prg_data_width, uint32_t io_data_width, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock),
		m_program_config("program", ENDIANNESS_BIG, prg_data_width, 32, 0, internal_map),
		m_io_config("io", ENDIANNESS_BIG, io_data_width, 15),
		m_icount(0)
{
}


//-------------------------------------------------
//  e116t_device - constructor
//-------------------------------------------------

e116t_device::e116t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116T, 16, 16, ADDRESS_MAP_NAME(e116_4k_iram_map))
{
}


//-------------------------------------------------
//  e116xt_device - constructor
//-------------------------------------------------

e116xt_device::e116xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XT, 16, 16, ADDRESS_MAP_NAME(e116_8k_iram_map))
{
}


//-------------------------------------------------
//  e116xs_device - constructor
//-------------------------------------------------

e116xs_device::e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XS, 16, 16, ADDRESS_MAP_NAME(e116_16k_iram_map))
{
}


//-------------------------------------------------
//  e116xsr_device - constructor
//-------------------------------------------------

e116xsr_device::e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XSR, 16, 16, ADDRESS_MAP_NAME(e116_16k_iram_map))
{
}


//-------------------------------------------------
//  e132n_device - constructor
//-------------------------------------------------

e132n_device::e132n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132N, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map))
{
}


//-------------------------------------------------
//  e132t_device - constructor
//-------------------------------------------------

e132t_device::e132t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132T, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map))
{
}


//-------------------------------------------------
//  e132xn_device - constructor
//-------------------------------------------------

e132xn_device::e132xn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XN, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map))
{
}


//-------------------------------------------------
//  e132xt_device - constructor
//-------------------------------------------------

e132xt_device::e132xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XT, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map))
{
}


//-------------------------------------------------
//  e132xs_device - constructor
//-------------------------------------------------

e132xs_device::e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XS, 32, 32, ADDRESS_MAP_NAME(e132_16k_iram_map))
{
}


//-------------------------------------------------
//  e132xsr_device - constructor
//-------------------------------------------------

e132xsr_device::e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XSR, 32, 32, ADDRESS_MAP_NAME(e132_16k_iram_map))
{
}


//-------------------------------------------------
//  gms30c2116_device - constructor
//-------------------------------------------------

gms30c2116_device::gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2116, 16, 16, ADDRESS_MAP_NAME(e116_4k_iram_map))
{
}


//-------------------------------------------------
//  gms30c2132_device - constructor
//-------------------------------------------------

gms30c2132_device::gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2132, 32, 32, ADDRESS_MAP_NAME(e132_4k_iram_map))
{
}


//-------------------------------------------------
//  gms30c2216_device - constructor
//-------------------------------------------------

gms30c2216_device::gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2216, 16, 16, ADDRESS_MAP_NAME(e116_8k_iram_map))
{
}


//-------------------------------------------------
//  gms30c2232_device - constructor
//-------------------------------------------------

gms30c2232_device::gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2232, 32, 32, ADDRESS_MAP_NAME(e132_8k_iram_map))
{
}

/* Return the entry point for a determinated trap */
uint32_t hyperstone_device::get_trap_addr(uint8_t trapno)
{
	uint32_t addr;
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
uint32_t hyperstone_device::get_emu_code_addr(uint8_t num) /* num is OP */
{
	uint32_t addr;
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

uint32_t hyperstone_device::compute_tr()
{
	uint64_t cycles_since_base = total_cycles() - m_tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_clck_scale;
	return m_tr_base_value + (clocks_since_base / m_tr_clocks_per_tick);
}

void hyperstone_device::update_timer_prescale()
{
	uint32_t prevtr = compute_tr();
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
	uint64_t cycles_since_base = total_cycles() - m_tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_clck_scale;
	uint64_t cycles_until_next_clock = cycles_since_base - (clocks_since_base << m_clck_scale);

	if (cycles_until_next_clock == 0)
		cycles_until_next_clock = (uint64_t)(1 << m_clck_scale);

	/* special case: if we have a change pending, set a timer to fire then */
	if (TPR & 0x80000000)
	{
		uint64_t clocks_until_int = m_tr_clocks_per_tick - (clocks_since_base % m_tr_clocks_per_tick);
		uint64_t cycles_until_int = (clocks_until_int << m_clck_scale) + cycles_until_next_clock;
		m_timer->adjust(cycles_to_attotime(cycles_until_int + 1), 1);
	}

	/* else if the timer interrupt is enabled, configure it to fire at the appropriate time */
	else if (!(FCR & 0x00800000))
	{
		uint32_t curtr = m_tr_base_value + (clocks_since_base / m_tr_clocks_per_tick);
		uint32_t delta = TCR - curtr;
		if (delta > 0x80000000)
		{
			if (!m_timer_int_pending)
				m_timer->adjust(attotime::zero);
		}
		else
		{
			uint64_t clocks_until_int = mulu_32x32(delta, m_tr_clocks_per_tick);
			uint64_t cycles_until_int = (clocks_until_int << m_clck_scale) + cycles_until_next_clock;
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




uint32_t hyperstone_device::get_global_register(uint8_t code)
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
	return m_global_regs[code & 0x1f];
}

void hyperstone_device::set_local_register(uint8_t code, uint32_t val)
{
	m_local_regs[(code + GET_FP) & 0x3f] = val;
}

void hyperstone_device::set_global_register(uint8_t code, uint32_t val)
{
	//TODO: add correct FER set instruction
	code &= 0x1f;
	switch (code)
	{
		case PC_REGISTER:
			SET_PC(val);
			return;
		case SR_REGISTER:
			SET_LOW_SR(val); // only a RET instruction can change the full content of SR
			SR &= ~0x40; //reserved bit 6 always zero
			if (m_intblock < 1)
				m_intblock = 1;
			return;
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
		case 16:
		// are the below ones set only when privilege bit is set?
		case 17:
			m_global_regs[code] = val;
			return;
		case SP_REGISTER:
		case UB_REGISTER:
			m_global_regs[code] = val & ~3;
			return;
		case BCR_REGISTER:
			m_global_regs[code] = val;
			return;
		case TPR_REGISTER:
			m_global_regs[code] = val;
			if (!(val & 0x80000000)) /* change immediately */
				update_timer_prescale();
			adjust_timer_interrupt();
			return;
		case TCR_REGISTER:
			if (m_global_regs[code] != val)
			{
				m_global_regs[code] = val;
				adjust_timer_interrupt();
				if (m_intblock < 1)
					m_intblock = 1;
			}
			return;
		case TR_REGISTER:
			m_global_regs[code] = val;
			m_tr_base_value = val;
			m_tr_base_cycles = total_cycles();
			adjust_timer_interrupt();
			return;
		case WCR_REGISTER:
			m_global_regs[code] = val;
			return;
		case ISR_REGISTER:
			return;
		case FCR_REGISTER:
			if ((m_global_regs[code] ^ val) & 0x00800000)
				adjust_timer_interrupt();
			m_global_regs[code] = val;
			if (m_intblock < 1)
				m_intblock = 1;
			return;
		case MCR_REGISTER:
			// bits 14..12 EntryTableMap
			hyperstone_set_trap_entry((val & 0x7000) >> 12);
			m_global_regs[code] = val;
			return;
		case 28:
		case 29:
		case 30:
		case 31:
			m_global_regs[code] = val;
			return;
	}
}

#define S_BIT                   ((OP & 0x100) >> 8)
#define D_BIT                   ((OP & 0x200) >> 9)
#define N_VALUE                 (((OP & 0x100) >> 4) | (OP & 0x0f))
#define N_OP_MASK               (m_op & 0x10f)
#define DST_CODE                ((OP & 0xf0) >> 4)
#define SRC_CODE                (OP & 0x0f)
#define SIGN_BIT(val)           ((val & 0x80000000) >> 31)
#define SIGN_TO_N(val)          ((val & 0x80000000) >> 29)

#define LOCAL  1

static const int32_t immediate_values[32] =
{
	0, 1, 2, 3, 4, 5, 6, 7,
	8, 9, 10, 11, 12, 13, 14, 15,
	16, 0, 0, 0, 32, 64, 128, static_cast<int32_t>(0x80000000),
	-8, -7, -6, -5, -4, -3, -2, -1
};

#define WRITE_ONLY_REGMASK  ((1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER))

#define decode_source_local(decode)                                                 \
do                                                                                  \
{                                                                                   \
} while (0)

#define decode_source_noh(decode)                                                   \
do                                                                                  \
{                                                                                   \
	decode.src_is_local = 0;                                                        \
																				    \
	SREG = get_global_register(decode.src);                                         \
																				    \
	/* bound safe */                                                                \
	if (decode.src != 15)                                                           \
		SREGF = get_global_register(decode.src + 1);                                \
	else                                                                            \
		SREGF = 0;                                                                  \
} while (0)

#define decode_source(decode, hflag)                                                \
do                                                                                  \
{                                                                                   \
	decode.src_is_local = 0;                                                        \
																				    \
	if (!hflag)                                                                     \
	{                                                                               \
		SREG = get_global_register(decode.src);                                     \
																				    \
		/* bound safe */                                                            \
		if (decode.src != 15)                                                       \
			SREGF = get_global_register(decode.src + 1);                            \
		else                                                                        \
			SREGF = 0;                                                              \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		decode.src += 16;                                                           \
																				    \
		SREG = get_global_register(decode.src);                                     \
		if ((WRITE_ONLY_REGMASK >> decode.src) & 1)                                 \
			SREG = 0; /* write-only registers */                                    \
		else if (decode.src == ISR_REGISTER)                                        \
			DEBUG_PRINTF(("read src ISR. PC = %08X\n",PPC));                        \
																				    \
		/* bound safe */                                                            \
		if (decode.src != 31)                                                       \
			SREGF = get_global_register(decode.src + 1);                            \
		else                                                                        \
			SREGF = 0;                                                              \
	}                                                                               \
} while (0)

#define decode_dest(decode, hflag)                                                  \
do                                                                                  \
{                                                                                   \
	decode.dst_is_local = 0;                                                        \
																				    \
	if (!hflag)                                                                     \
	{                                                                               \
		DREG = get_global_register(decode.dst);                                     \
																				    \
		/* bound safe */                                                            \
		if (decode.dst != 15)                                                       \
			DREGF = get_global_register(decode.dst + 1);                            \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		decode.dst += 16;                                                           \
																				    \
		DREG = get_global_register(decode.dst);                                     \
		if( decode.dst == ISR_REGISTER )                                            \
			DEBUG_PRINTF(("read dst ISR. PC = %08X\n",PPC));                        \
																				    \
		/* bound safe */                                                            \
		if (decode.dst != 31)                                                       \
			DREGF = get_global_register(decode.dst + 1);                            \
	}                                                                               \
} while (0)

#define check_delay_PC()                                                            \
do                                                                                  \
{                                                                                   \
	/* if PC is used in a delay instruction, the delayed PC should be used */       \
	if (m_delay_slot)                                                               \
	{                                                                               \
		PC = m_delay_pc;                                                            \
		m_delay_slot = false;                                                       \
	}                                                                               \
} while (0)

#define decode_immediate_u(decode)                                                  \
do                                                                                  \
{                                                                                   \
		EXTRA_U = immediate_values[OP & 0x0f];                                      \
} while (0)

#define DECODE_IMMEDIATE_S(decode)                                                  \
do                                                                                  \
{                                                                                   \
	switch( OP & 0x0f )                                                             \
	{                                                                               \
		default:                                                                    \
			EXTRA_U = immediate_values[0x10 + (OP & 0x0f)];                         \
			break;                                                                  \
																				    \
		case 1:                                                                     \
			m_instruction_length = (3<<19);                                         \
			EXTRA_U = (READ_OP(PC) << 16) | READ_OP(PC + 2);                        \
			PC += 4;                                                                \
			break;                                                                  \
																				    \
		case 2:                                                                     \
			m_instruction_length = (2<<19);                                         \
			EXTRA_U = READ_OP(PC);                                                  \
			PC += 2;                                                                \
			break;                                                                  \
																				    \
		case 3:                                                                     \
			m_instruction_length = (2<<19);                                         \
			EXTRA_U = 0xffff0000 | READ_OP(PC);                                     \
			PC += 2;                                                                \
			break;                                                                  \
	}                                                                               \
} while (0)

void hyperstone_device::ignore_immediate_s()
{
	static const uint32_t lengths[16] = { 1<<19, 3<<19, 2<<19, 2<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19, 1<<19 };
	static const uint32_t offsets[16] = { 0, 4, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	const uint8_t nybble = m_op & 0x0f;
	m_instruction_length = lengths[nybble];
	PC += offsets[nybble];
}

uint32_t hyperstone_device::decode_immediate_s()
{
	const uint8_t nybble = m_op & 0x0f;
	switch (nybble)
	{
		case 0:
		default:
			return immediate_values[0x10 + nybble];
		case 1:
		{
			m_instruction_length = (3<<19);
			uint32_t extra_u = (READ_OP(PC) << 16) | READ_OP(PC + 2);
			PC += 4;
			return extra_u;
		}
		case 2:
		{
			m_instruction_length = (2<<19);
			uint32_t extra_u = READ_OP(PC);
			PC += 2;
			return extra_u;
		}
		case 3:
		{
			m_instruction_length = (2<<19);
			uint32_t extra_u = 0xffff0000 | READ_OP(PC);
			PC += 2;
			return extra_u;
		}
	}
}

#define DECODE_CONST(decode)                                                        \
do                                                                                  \
{                                                                                   \
	uint16_t imm_1 = READ_OP(PC);                                                   \
																					\
	PC += 2;                                                                        \
	m_instruction_length = (2<<19);                                                 \
																					\
	if( E_BIT(imm_1) )                                                              \
	{                                                                               \
		uint16_t imm_2 = READ_OP(PC);                                               \
																					\
		PC += 2;                                                                    \
		m_instruction_length = (3<<19);                                             \
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

uint32_t hyperstone_device::decode_const()
{
	const uint16_t imm_1 = READ_OP(PC);

	PC += 2;
	m_instruction_length = (2<<19);

	if (imm_1 & 0x8000)
	{
		const uint16_t imm_2 = READ_OP(PC);

		PC += 2;
		m_instruction_length = (3<<19);

		uint32_t imm = imm_2;
		imm |= ((imm_1 & 0x3fff) << 16);

		if (imm_1 & 0x4000)
			imm |= 0xc0000000;
		return imm;
	}
	else
	{
		uint32_t imm = imm_1 & 0x3fff;

		if (imm_1 & 0x4000)
			imm |= 0xffffc000;
		return imm;
	}
}

int32_t hyperstone_device::decode_pcrel()
{
	if (OP & 0x80)
	{
		uint16_t next = READ_OP(PC);

		PC += 2;
		m_instruction_length = (2<<19);

		int32_t offset = (OP & 0x7f) << 16;
		offset |= (next & 0xfffe);

		if (next & 1)
			offset |= 0xff800000;

		return offset;
	}
	else
	{
		int32_t offset = OP & 0x7e;

		if (OP & 1)
			offset |= 0xffffff80;

		return offset;
	}
}

void hyperstone_device::ignore_pcrel()
{
	if (m_op & 0x80)
	{
		PC += 2;
		m_instruction_length = (2<<19);
	}
}

#define decode_dis(decode)                                                          \
do                                                                                  \
{                                                                                   \
	uint16_t next_1 = READ_OP(PC);                                                  \
																					\
	PC += 2;                                                                        \
	m_instruction_length = (2<<19);                                                 \
																					\
	decode.sub_type = DD(next_1);                                                   \
																					\
	if( E_BIT(next_1) )                                                             \
	{                                                                               \
		uint16_t next_2 = READ_OP(PC);                                              \
																					\
		PC += 2;                                                                    \
		m_instruction_length = (3<<19);                                             \
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
	uint32_t next = READ_OP(PC);                                                    \
	PC += 2;                                                                        \
	m_instruction_length = (2<<19);                                                 \
																					\
	decode.sub_type = X_CODE(next);                                                 \
																					\
	if( E_BIT(next) )                                                               \
	{                                                                               \
		EXTRA_U = ((next & 0xfff) << 16) | READ_OP(PC);                             \
		PC += 2;                                                                    \
		m_instruction_length = (3<<19);                                             \
	}                                                                               \
	else                                                                            \
	{                                                                               \
		EXTRA_U = next & 0xfff;                                                     \
	}                                                                               \
} while (0)

void hyperstone_device::execute_br()
{
	const int32_t offset = decode_pcrel();
	check_delay_PC();

	PPC = PC;
	PC += offset;
	SR &= ~M_MASK;

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::execute_dbr(int32_t offset)
{
	m_delay_slot = true;
	m_delay_pc = PC + offset;

	m_intblock = 3;
}


void hyperstone_device::execute_trap(uint32_t addr)
{
	uint8_t reg;
	uint32_t oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length);

	oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	PPC = PC;
	PC = addr;

	m_icount -= m_clock_cycles_2;
}


void hyperstone_device::execute_int(uint32_t addr)
{
	uint8_t reg;
	uint32_t oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length);

	oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, oldSR);

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
void hyperstone_device::execute_exception(uint32_t addr)
{
	uint8_t reg;
	uint32_t oldSR;
	reg = GET_FP + GET_FL;

	SET_ILC(m_instruction_length);

	oldSR = SR;

	SET_FP(reg);
	SET_FL(2);

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, oldSR);

	SET_M(0);
	SET_T(0);
	SET_L(1);
	SET_S(1);

	PPC = PC;
	PC = addr;

	DEBUG_PRINTF(("EXCEPTION! PPC = %08X PC = %08X\n",PPC-2,PC-2));
	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::execute_software()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_local_regs[(src_code + fp) & 0x3f];
	const uint32_t sregf = m_local_regs[(src_code + 1 + fp) & 0x3f];

	SET_ILC(1<<19);

	const uint32_t addr = get_emu_code_addr((m_op & 0xff00) >> 8);
	const uint8_t reg = fp + GET_FL;

	//since it's sure the register is in the register part of the stack,
	//set the stack address to a value above the highest address
	//that can be set by a following frame instruction
	const uint32_t stack_of_dst = (SP & ~0xff) + 64*4 + (((fp + DST_CODE) % 64) * 4); //converted to 32bits offset

	const uint32_t oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	m_local_regs[(reg + 0) & 0x3f] = stack_of_dst;
	m_local_regs[(reg + 1) & 0x3f] = sreg;
	m_local_regs[(reg + 2) & 0x3f] = sregf;
	m_local_regs[(reg + 3) & 0x3f] = (PC & ~1) | GET_S;
	m_local_regs[(reg + 4) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= L_MASK;

	PPC = PC;
	PC = addr;

	m_icount -= m_clock_cycles_6;
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
	memset(m_global_regs, 0, sizeof(uint32_t) * 32);
	memset(m_local_regs, 0, sizeof(uint32_t) * 64);
	memset(m_opcode_hits, 0, sizeof(uint64_t) * 256);
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

	for (uint8_t i = 0; i < 16; i++)
	{
		m_fl_lut[i] = (i ? i : 16);
	}

	// register our state for the debugger
	state_add(STATE_GENPC,    "GENPC",     m_global_regs[0]).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_global_regs[0]).noshow();
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
	save_item(NAME(m_delay_pc));
	save_item(NAME(m_instruction_length));
	save_item(NAME(m_intblock));
	save_item(NAME(m_delay_slot));
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

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, SR);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::device_stop()
{
#if 0
	int indices[256];
	for (int i = 0; i < 256; i++)
	{
		indices[i] = i;
	}
	for (int i = 0; i < 256; i++)
	{
		for (int j = i + 1; j < 256; j++)
		{
			if (m_opcode_hits[i] > m_opcode_hits[j])
			{
				uint64_t tmp = m_opcode_hits[i];
				m_opcode_hits[i] = m_opcode_hits[j];
				m_opcode_hits[j] = tmp;
				int tmpi = indices[i];
				indices[i] = indices[j];
				indices[j] = tmpi;
			}
		}
	}

	for (int i = 255; i >= 0 && m_opcode_hits[i] != 0; i--)
	{
		printf("%02x: %08x%08x\n", indices[i], (uint32_t)(m_opcode_hits[i] >> 32), (uint32_t)m_opcode_hits[i]);
	}
#endif
}


//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the address spaces
//-------------------------------------------------

device_memory_interface::space_config_vector hyperstone_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}


//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void hyperstone_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c%c%c%c%c%c%c%c%c%c FTE:%X FRM:%X ILC:%d FL:%d FP:%d",
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

uint32_t hyperstone_device::disasm_min_opcode_bytes() const
{
	return 2;
}


//-------------------------------------------------
//  disasm_max_opcode_bytes - return the length
//  of the longest instruction, in bytes
//-------------------------------------------------

uint32_t hyperstone_device::disasm_max_opcode_bytes() const
{
	return 6;
}


//-------------------------------------------------
//  disasm_disassemble - call the disassembly
//  helper function
//-------------------------------------------------

offs_t hyperstone_device::disasm_disassemble(std::ostream &stream, offs_t pc, const uint8_t *oprom, const uint8_t *opram, uint32_t options)
{
	extern CPU_DISASSEMBLE( hyperstone );
	return dasm_hyperstone(stream, pc, oprom, GET_H, GET_FP);
}

/* Opcodes */

void hyperstone_device::hyperstone_movd(regs_decode &decode)
{
	if (DST_IS_PC) // Rd denotes PC
	{
		// RET instruction

		if (SRC_IS_PC || SRC_IS_SR)
		{
			DEBUG_PRINTF(("Denoted PC or SR in RET instruction. PC = %08X\n", PC));
		}
		else
		{
			uint8_t old_s = GET_S;
			uint8_t old_l = GET_L;
			PPC = PC;

			SET_PC(SREG);
			SR = (SREGF & 0xffe00000) | ((SREG & 0x01) << 18 ) | (SREGF & 0x3ffff);
			if (m_intblock < 1)
				m_intblock = 1;

			m_instruction_length = 0; // undefined

			if( (!old_s && GET_S) || (!GET_S && !old_l && GET_L))
			{
				uint32_t addr = get_trap_addr(TRAPNO_PRIVILEGE_ERROR);
				execute_exception(addr);
			}

			int8_t difference = GET_FP - ((SP & 0x1fc) >> 2);

			/* convert to 8 bits */
			if(difference > 63)
				difference = (int8_t)(difference|0x80);
			else if( difference < -64 )
				difference = difference & 0x7f;

			for (; difference < 0; difference++)
			{
				SP -= 4;
				m_local_regs[(SP & 0xfc) >> 2] = READ_W(SP);
			}
		}

		//TODO: no 1!
		m_icount -= m_clock_cycles_1;
	}
	else if (SRC_IS_SR) // Rd doesn't denote PC and Rs denotes SR
	{
		SET_DREG(0);
		SET_DREGF(0);
		SET_Z(1);
		SET_N(0);

		m_icount -= m_clock_cycles_2;
	}
	else // Rd doesn't denote PC and Rs doesn't denote SR
	{
		SET_DREG(SREG);
		SET_DREGF(SREGF);

		uint64_t tmp = concat_64(SREG, SREGF);
		SET_Z(tmp == 0 ? 1 : 0);
		SET_N(SIGN_BIT(SREG));

		m_icount -= m_clock_cycles_2;
	}
}

void hyperstone_device::hyperstone_divu(regs_decode &decode)
{
	if (decode.same_src_dst || decode.same_src_dstf || SRC_IS_PC || SRC_IS_SR)
	{
		DEBUG_PRINTF(("Denoted the same register code or PC/SR as source in hyperstone_divu instruction. PC = %08X\n", PC));
	}
	else
	{
		uint64_t dividend = concat_64(DREG, DREGF);

		if (SREG == 0)
		{
			//Rd//Rdf -> undefined
			//Z -> undefined
			//N -> undefined
			SR |= V_MASK;
			uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
			execute_exception(addr);
		}
		else
		{
			/* TODO: add quotient overflow */
			uint32_t quotient = dividend / SREG;
			uint32_t remainder = dividend % SREG;

			SET_DREG(remainder);
			SET_DREGF(quotient);

			SET_Z(quotient == 0 ? 1 : 0);
			SET_N(SIGN_BIT(quotient));
			SR &= ~V_MASK;
		}
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_divs(regs_decode &decode)
{
	if( decode.same_src_dst || decode.same_src_dstf )
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
			int64_t dividend = (int64_t) concat_64(DREG, DREGF);

			if( SREG == 0 || (DREG & 0x80000000) )
			{
				//Rd//Rdf -> undefined
				//Z -> undefined
				//N -> undefined
				SR |= V_MASK;
				uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
				execute_exception(addr);
			}
			else
			{
				/* TODO: add quotient overflow */
				int32_t quotient = dividend / ((int32_t)(SREG));
				int32_t remainder = dividend % ((int32_t)(SREG));

				SET_DREG(remainder);
				SET_DREGF(quotient);

				SET_Z(quotient == 0 ? 1 : 0);
				SET_N(SIGN_BIT(quotient));
				SR &= ~V_MASK;
			}
		}
	}

	m_icount -= 36 << m_clck_scale;
}

void hyperstone_device::hyperstone_xm(regs_decode &decode)
{
	if( SRC_IS_SR || DST_IS_SR || DST_IS_PC )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_xm. PC = %08X\n", PC));
	}
	else
	{
		switch( decode.sub_type ) // x_code
		{
			case 0:
			case 1:
			case 2:
			case 3:
				if( !SRC_IS_PC && (SREG > EXTRA_U) )
				{
					uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
					execute_exception(addr);
				}
				else if( SRC_IS_PC && (SREG >= EXTRA_U) )
				{
					uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
					execute_exception(addr);
				}
				else
				{
					SREG <<= decode.sub_type;
				}

				break;

			case 4:
			case 5:
			case 6:
			case 7:
				decode.sub_type -= 4;
				SREG <<= decode.sub_type;

				break;
		}

		SET_DREG(SREG);
	}

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mask(regs_decode &decode)
{
	DREG = SREG & EXTRA_U;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sum(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	const uint64_t tmp = (uint64_t)(SREG) + (uint64_t)(EXTRA_U);

	CHECK_C(tmp);
	CHECK_VADD(SREG,EXTRA_U,tmp);

	DREG = SREG + EXTRA_U;

	SET_DREG(DREG);

	if (DST_IS_PC)
		SET_M(0);

	SET_Z(DREG == 0 ? 1 : 0);
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sums(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	int64_t tmp = (int64_t)((int32_t)(SREG)) + (int64_t)(EXTRA_S);
	CHECK_VADD(SREG,EXTRA_S,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	int32_t res = (int32_t)(SREG) + EXTRA_S;

	SET_DREG(res);

	SET_Z(res == 0 ? 1 : 0);
	SET_N(SIGN_BIT(res));

	m_icount -= m_clock_cycles_1;

	if (GET_V && !SRC_IS_SR)
	{
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_cmp(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	if (DREG == SREG)
		SR |= Z_MASK;
	else
		SR &= ~Z_MASK;

	if ((int32_t) DREG < (int32_t) SREG)
		SR |= N_MASK;
	else
		SR &= ~N_MASK;

	uint64_t tmp = (uint64_t)(DREG) - (uint64_t)(SREG);
	CHECK_VSUB(SREG, DREG, tmp);

	if (DREG < SREG)
		SR |= C_MASK;
	else
		SR &= ~C_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_add(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	uint64_t tmp = (uint64_t)(SREG) + (uint64_t)(DREG);
	CHECK_C(tmp);
	CHECK_VADD(SREG,DREG,tmp);

	DREG = SREG + DREG;
	SET_DREG(DREG);

	if (DST_IS_PC)
		SET_M(0);

	SET_Z(DREG == 0 ? 1 : 0);
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_adds(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	int64_t tmp = (int64_t)((int32_t)(SREG)) + (int64_t)((int32_t)(DREG));

	CHECK_VADD(SREG, DREG, tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	int32_t res = (int32_t)(SREG) + (int32_t)(DREG);

	SET_DREG(res);
	SET_Z(res == 0 ? 1 : 0);
	SET_N(SIGN_BIT(res));

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
	{
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_subc(regs_decode &decode)
{
	uint64_t tmp;

	if (SRC_IS_SR)
	{
		tmp = (uint64_t)(DREG) - (uint64_t)(GET_C);
		CHECK_VSUB(GET_C,DREG,tmp);
	}
	else
	{
		tmp = (uint64_t)(DREG) - ((uint64_t)(SREG) + (uint64_t)(GET_C));
		//CHECK!
		CHECK_VSUB((SREG + GET_C),DREG,tmp);
	}


	if (SRC_IS_SR)
	{
		DREG = DREG - GET_C;
	}
	else
	{
		DREG = DREG - (SREG + GET_C);
	}

	CHECK_C(tmp);

	SET_DREG(DREG);

	SET_Z(GET_Z & (DREG == 0 ? 1 : 0));
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sub(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	uint64_t tmp = (uint64_t)(DREG) - (uint64_t)(SREG);
	CHECK_C(tmp);
	CHECK_VSUB(SREG,DREG,tmp);

	DREG = DREG - SREG;
	SET_DREG(DREG);

	if (DST_IS_PC)
		SR &= ~M_MASK;

	SET_Z(DREG == 0 ? 1 : 0);
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_subs(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	int64_t tmp = (int64_t)((int32_t)(DREG)) - (int64_t)((int32_t)(SREG));

//#ifdef SETCARRYS
//  CHECK_C(tmp);
//#endif

	CHECK_VSUB(SREG,DREG,tmp);

	int32_t res = (int32_t)(DREG) - (int32_t)(SREG);

	SET_DREG(res);

	SET_Z(res == 0 ? 1 : 0);
	SET_N(SIGN_BIT(res));

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
	{
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_addc(regs_decode &decode)
{
	uint64_t tmp;
	if (SRC_IS_SR)
	{
		tmp = (uint64_t)(DREG) + (uint64_t)(GET_C);
		CHECK_VADD(DREG,GET_C,tmp);
	}
	else
	{
		tmp = (uint64_t)(SREG) + (uint64_t)(DREG) + (uint64_t)(GET_C);

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

	if (SRC_IS_SR)
		DREG = DREG + GET_C;
	else
		DREG = SREG + DREG + GET_C;

	CHECK_C(tmp);

	SET_DREG(DREG);
	SET_Z(GET_Z & (DREG == 0 ? 1 : 0));
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_neg(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	uint64_t tmp = -(uint64_t)(SREG);
	CHECK_C(tmp);
	CHECK_VSUB(SREG,0,tmp);

	DREG = -SREG;

	SET_DREG(DREG);

	SET_Z(DREG == 0 ? 1 : 0);
	SET_N(SIGN_BIT(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_negs(regs_decode &decode)
{
	if (SRC_IS_SR)
		SREG = GET_C;

	int64_t tmp = -(int64_t)((int32_t)(SREG));
	CHECK_VSUB(SREG,0,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	int32_t res = -(int32_t)(SREG);

	SET_DREG(res);

	SET_Z(res == 0 ? 1 : 0);
	SET_N(SIGN_BIT(res));


	m_icount -= m_clock_cycles_1;

	if (GET_V && !SRC_IS_SR) //trap doesn't occur when source is SR
	{
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_addsi(regs_decode &decode)
{
	int32_t imm;
	if (N_VALUE)
		imm = EXTRA_S;
	else
		imm = GET_C & ((GET_Z == 0 ? 1 : 0) | (DREG & 0x01));

	int64_t tmp = (int64_t)(imm) + (int64_t)((int32_t)(DREG));
	CHECK_VADD(imm,DREG,tmp);

//#if SETCARRYS
//  CHECK_C(tmp);
//#endif

	int32_t res = imm + (int32_t)(DREG);

	SET_DREG(res);

	SET_Z(res == 0 ? 1 : 0);
	SET_N(SIGN_BIT(res));

	m_icount -= m_clock_cycles_1;

	if (SR & V_MASK)
	{
		uint32_t addr = get_trap_addr(TRAPNO_RANGE_ERROR);
		execute_exception(addr);
	}
}

void hyperstone_device::hyperstone_andni(regs_decode &decode)
{
	uint32_t imm;

	if( N_VALUE == 31 )
		imm = 0x7fffffff; // bit 31 = 0, others = 1
	else
		imm = EXTRA_U;

	DREG = DREG & ~imm;

	SET_DREG(DREG);
	SET_Z( DREG == 0 ? 1 : 0 );

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shrdi(regs_decode &decode)
{
	uint32_t high_order = DREG;
	uint32_t low_order  = DREGF;

	uint64_t val = concat_64(high_order, low_order);

	if( N_VALUE )
		SET_C((val >> (N_VALUE - 1)) & 1);
	else
		SR &= ~C_MASK;

	val >>= N_VALUE;

	high_order = extract_64hi(val);
	low_order  = extract_64lo(val);

	SET_DREG(high_order);
	SET_DREGF(low_order);
	SET_Z(val == 0 ? 1 : 0);
	SET_N(SIGN_BIT(high_order));

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shr(regs_decode &decode)
{
	uint32_t n = SREG & 0x1f;
	uint32_t ret = DREG;

	if (n)
		SET_C((ret >> (n - 1)) & 1);
	else
		SR &= ~C_MASK;

	ret >>= n;

	SET_DREG(ret);
	SET_Z(ret == 0 ? 1 : 0);
	SET_N(SIGN_BIT(ret));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_sardi()
{
	check_delay_PC();

	const uint32_t dst_code = (DST_CODE + GET_FP) % 64;
	const uint32_t dstf_code = (DST_CODE + GET_FP + 1) % 64;

	uint64_t val = concat_64(m_local_regs[dst_code], m_local_regs[dstf_code]);

	SR &= ~(C_MASK | Z_MASK | N_MASK);

	const uint32_t n = N_VALUE;
	if (n)
	{
		SR |= (val >> (n - 1)) & 1;

		const uint64_t sign_bit = val >> 63;
		val >>= n;

		if (sign_bit)
		{
			val |= 0xffffffff00000000U << (32 - n);
		}
	}

	m_local_regs[dst_code] = (uint32_t)(val >> 32);
	m_local_regs[dstf_code] = (uint32_t)val;

	if (val == 0)
		SR |= Z_MASK;
	SR |= SIGN_TO_N(m_local_regs[dst_code]);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_sar(regs_decode &decode)
{
	uint32_t n = SREG & 0x1f;
	uint32_t ret = DREG;
	uint32_t sign_bit = (ret & 0x80000000) >> 31;

	if (n)
		SET_C((ret >> (n - 1)) & 1);
	else
		SR &= ~C_MASK;

	ret >>= n;

	if (sign_bit)
	{
		for (int i = 0; i < n; i++)
		{
			ret |= (0x80000000 >> i);
		}
	}

	SET_DREG(ret);
	SET_Z(ret == 0 ? 1 : 0);
	SET_N(SIGN_BIT(ret));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_shld()
{
	check_delay_PC();

	uint32_t src_code = (SRC_CODE + GET_FP) % 64;
	uint32_t dst_code = (DST_CODE + GET_FP) % 64;
	uint32_t dstf_code = (DST_CODE + GET_FP + 1) % 64;

	// result undefined if Ls denotes the same register as Ld or Ldf
	if (src_code == dst_code || src_code == dstf_code)
	{
		DEBUG_PRINTF(("Denoted same registers in hyperstone_shld. PC = %08X\n", PC));
	}
	else
	{
		uint32_t n = m_local_regs[src_code % 64] & 0x1f;
		uint32_t high_order = m_local_regs[dst_code]; /* registers offset by frame pointer */
		uint32_t low_order  = m_local_regs[dstf_code];

		uint64_t mask = ((((uint64_t)1) << (32 - n)) - 1) ^ 0xffffffff;

		uint64_t val = concat_64(high_order, low_order);
		SET_C( (n)?(((val<<(n-1))&0x8000000000000000U)?1:0):0);
		uint32_t tmp = high_order << n;

		if (((high_order & mask) && (!(tmp & 0x80000000))) || (((high_order & mask) ^ mask) && (tmp & 0x80000000)))
			SET_V(1);
		else
			SR &= ~V_MASK;

		val <<= n;

		m_local_regs[dst_code] = extract_64hi(val);
		m_local_regs[dstf_code] = extract_64lo(val);

		SET_Z(val == 0 ? 1 : 0);
		SET_N(SIGN_BIT(high_order));
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_shl()
{
	check_delay_PC();

	uint32_t src_code = SRC_CODE + GET_FP;
	uint32_t dst_code = DST_CODE + GET_FP;

	uint32_t n    = m_local_regs[src_code % 64] & 0x1f;
	uint32_t base = m_local_regs[dst_code % 64]; /* registers offset by frame pointer */
	uint64_t mask = ((((uint64_t)1) << (32 - n)) - 1) ^ 0xffffffff;
	SET_C( (n)?(((base<<(n-1))&0x80000000)?1:0):0);
	uint32_t ret  = base << n;

	if (((base & mask) && (!(ret & 0x80000000))) || (((base & mask) ^ mask) && (ret & 0x80000000)))
		SET_V(1);
	else
		SR &= ~V_MASK;

	m_local_regs[dst_code % 64] = ret;
	SET_Z(ret == 0 ? 1 : 0);
	SET_N(SIGN_BIT(ret));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_testlz()
{
	check_delay_PC();

	uint32_t sreg = m_local_regs[(SRC_CODE + GET_FP) % 64];
	uint32_t zeros = 0;
	for (uint32_t mask = 0x80000000; mask != 0; mask >>= 1 )
	{
		if (sreg & mask)
			break;
		else
			zeros++;
	}

	m_local_regs[(DST_CODE + GET_FP) % 64] = zeros;

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_rol(regs_decode &decode)
{
	uint32_t n = SREG & 0x1f;

	uint32_t val = DREG;
	uint32_t base = val;

	uint64_t mask = ((((uint64_t)1) << (32 - n)) - 1) ^ 0xffffffff;

	while (n > 0)
	{
		val = (val << 1) | ((val & 0x80000000) >> 31);
		n--;
	}

#ifdef MISSIONCRAFT_FLAGS
	if (((base & mask) && (!(val & 0x80000000))) || (((base & mask) ^ mask) && (val & 0x80000000)))
		SET_V(1);
	else
		SET_V(0);
#endif

	SET_DREG(val);

	SET_Z(val == 0 ? 1 : 0);
	SET_N(SIGN_BIT(val));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_ldxx2(regs_decode &decode)
{
	if (DST_IS_PC || DST_IS_SR)
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_ldxx2. PC = %08X\n", PC));
	}
	else
	{
		switch( decode.sub_type )
		{
			case 0: // LDBS.N
				SET_SREG((int32_t)(int8_t)READ_B(DREG));

				if (!decode.same_src_dst)
					SET_DREG(DREG + EXTRA_S);
				break;

			case 1: // LDBU.N
				SET_SREG(READ_B(DREG));

				if(!decode.same_src_dst)
					SET_DREG(DREG + EXTRA_S);
				break;

			case 2:
				if (EXTRA_S & 1) // LDHS.N
					SET_SREG((int32_t)(int16_t)READ_HW(DREG));
				else // LDHU.N
					SET_SREG(READ_HW(DREG));

				if(!decode.same_src_dst)
					SET_DREG(DREG + (EXTRA_S & ~1));
				break;

			case 3:
				switch (EXTRA_S & 3)
				{
					case 0: // LDW.N
						SET_SREG(READ_W(DREG));
						if(!decode.same_src_dst)
							SET_DREG(DREG + (EXTRA_S & ~1));
						break;
					case 1: // LDD.N
						SET_SREG(READ_W(DREG));
						SET_SREGF(READ_W(DREG + 4));

						if (!decode.same_src_dst && !decode.same_srcf_dst)
							SET_DREG(DREG + (EXTRA_S & ~1));

						m_icount -= m_clock_cycles_1; // extra cycle
						break;
					case 2: // Reserved
						DEBUG_PRINTF(("Executed Reserved instruction in hyperstone_ldxx2. PC = %08X\n", PC));
						break;
					case 3: // LDW.S
						if (DREG < SP)
							SET_SREG(READ_W(DREG));
						else
							SET_SREG(m_local_regs[(DREG & 0xfc) >> 2]);

						if (!decode.same_src_dst)
							SET_DREG(DREG + (EXTRA_S & ~3));

						m_icount -= m_clock_cycles_2; // extra cycles
						break;
				}
				break;
		}
	}

	m_icount -= m_clock_cycles_1;
}

//TODO: add trap error
void hyperstone_device::hyperstone_stxx1(regs_decode &decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	if( DST_IS_SR )
	{
		switch( decode.sub_type )
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
		switch( decode.sub_type )
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

void hyperstone_device::hyperstone_stxx2(regs_decode &decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	if( DST_IS_PC || DST_IS_SR )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_stxx2. PC = %08X\n", PC));
	}
	else
	{
		switch( decode.sub_type )
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
						if(((DREG & 0xfc) >> 2) == ((decode.src + GET_FP) % 64) && (OP & 0x100))
							DEBUG_PRINTF(("STW.S denoted the same local register @ %08X\n",PPC));

						m_local_regs[(DREG & 0xfc) >> 2] = SREG;
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

					if( decode.same_srcf_dst )
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

void hyperstone_device::hyperstone_sari(regs_decode &decode)
{
	uint32_t val = DREG;
	uint32_t sign_bit = val & 0x80000000;

	const uint32_t n = N_VALUE;

	SR &= ~(C_MASK | Z_MASK | N_MASK);
	if (n)
		SET_C((val >> (n - 1)) & 1);

	val >>= n;

	if (sign_bit)
		for (int i = 0; i < n; i++)
			val |= (0x80000000 >> i);

	SET_DREG(val);
	if (val == 0)
		SR |= Z_MASK;
	if (SIGN_BIT(val))
		SR |= N_MASK;

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_mulu(regs_decode &decode)
{
	uint32_t low_order, high_order;
	uint64_t double_word;

	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mulu instruction. PC = %08X\n", PC));
	}
	else
	{
		double_word = (uint64_t)SREG *(uint64_t)DREG;

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

void hyperstone_device::hyperstone_muls(regs_decode &decode)
{
	uint32_t low_order, high_order;
	int64_t double_word;

	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_muls instruction. PC = %08X\n", PC));
	}
	else
	{
		double_word = (int64_t)(int32_t)(SREG) * (int64_t)(int32_t)(DREG);
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

void hyperstone_device::hyperstone_mul(regs_decode &decode)
{
	// PC or SR aren't denoted, else result is undefined
	if( SRC_IS_PC || SRC_IS_SR || DST_IS_PC || DST_IS_SR  )
	{
		DEBUG_PRINTF(("Denoted PC or SR in hyperstone_mul instruction. PC = %08X\n", PC));
	}
	else
	{
		const uint32_t single_word = (SREG * DREG);// & 0xffffffff; // only the low-order word is taken

		SET_DREG(single_word);

		SR &= ~(Z_MASK | N_MASK);
		if (single_word == 0)
			SR |= Z_MASK;
		if (SIGN_BIT(single_word))
			SR |= N_MASK;
	}

	if ((SREG >= 0xffff8000 && SREG <= 0x7fff) && (DREG >= 0xffff8000 && DREG <= 0x7fff))
		m_icount -= 3 << m_clck_scale;
	else
		m_icount -= 5 << m_clck_scale;
}

void hyperstone_device::hyperstone_do(regs_decode &decode)
{
	fatalerror("Executed hyperstone_do instruction. PC = %08X\n", PPC);
}

void hyperstone_device::hyperstone_ldwr(regs_decode &decode)
{
	SET_SREG(READ_W(DREG));

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddr(regs_decode &decode)
{
	SET_SREG(READ_W(DREG));
	SET_SREGF(READ_W(DREG + 4));

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_ldwp(regs_decode &decode)
{
	SET_SREG(READ_W(DREG));

	// post increment the destination register if it's different from the source one
	// (needed by Hidden Catch)
	if(!(decode.src == decode.dst && (OP & 0x100)))
		SET_DREG(DREG + 4);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_lddp(regs_decode &decode)
{
	SET_SREG(READ_W(DREG));
	SET_SREGF(READ_W(DREG + 4));

	// post increment the destination register if it's different from the source one
	// and from the "next source" one
	if(!(decode.src == decode.dst && (OP & 0x100)) &&   !decode.same_srcf_dst )
	{
		SET_DREG(DREG + 8);
	}
	else
	{
		DEBUG_PRINTF(("LDD.P denoted same regs @ %08X",PPC));
	}

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stdr(regs_decode &decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(DREG, SREG);
	WRITE_W(DREG + 4, SREGF);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_stwp(regs_decode &decode)
{
	if( SRC_IS_SR )
		SREG = 0;

	WRITE_W(DREG, SREG);
	SET_DREG(DREG + 4);

	m_icount -= m_clock_cycles_1;
}

void hyperstone_device::hyperstone_stdp(regs_decode &decode)
{
	if( SRC_IS_SR )
		SREG = SREGF = 0;

	WRITE_W(DREG, SREG);
	SET_DREG(DREG + 8);

	if( decode.same_srcf_dst )
		WRITE_W(DREG + 4, SREGF + 8); // because DREG == SREGF and DREG has been incremented
	else
		WRITE_W(DREG + 4, SREGF);

	m_icount -= m_clock_cycles_2;
}

void hyperstone_device::hyperstone_trap()
{
	check_delay_PC();

	const uint8_t trapno = (m_op & 0xfc) >> 2;

	const uint32_t addr = get_trap_addr(trapno);
	const uint8_t code = ((m_op & 0x300) >> 6) | (m_op & 0x03);

	switch (code)
	{
		case TRAPLE:
			if (SR & (N_MASK | Z_MASK))
				execute_trap(addr);
			break;

		case TRAPGT:
			if(!(SR & (N_MASK | Z_MASK)))
				execute_trap(addr);
			break;

		case TRAPLT:
			if (SR & N_MASK)
				execute_trap(addr);
			break;

		case TRAPGE:
			if (!(SR & N_MASK))
				execute_trap(addr);
			break;

		case TRAPSE:
			if (SR & (C_MASK | Z_MASK))
				execute_trap(addr);
			break;

		case TRAPHT:
			if (!(SR & (C_MASK | Z_MASK)))
				execute_trap(addr);
			break;

		case TRAPST:
			if (SR & C_MASK)
				execute_trap(addr);
			break;

		case TRAPHE:
			if (!(SR & C_MASK))
				execute_trap(addr);
			break;

		case TRAPE:
			if (SR & Z_MASK)
				execute_trap(addr);
			break;

		case TRAPNE:
			if (!(SR & Z_MASK))
				execute_trap(addr);
			break;

		case TRAPV:
			if (SR & V_MASK)
				execute_trap(addr);
			break;

		case TRAP:
			execute_trap(addr);
			break;
	}

	m_icount -= m_clock_cycles_1;
}


#include "e132xsop.hxx"

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_min_cycles() const
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_max_cycles() const
{
	return 36;
}


//-------------------------------------------------
//  execute_input_lines - return the number of
//  input/interrupt lines
//-------------------------------------------------

uint32_t hyperstone_device::execute_input_lines() const
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
		uint32_t oldh = SR & 0x00000020;

		PPC = PC;   /* copy PC to previous PC */
		debugger_instruction_hook(this, PC);

		OP = READ_OP(PC);
		PC += 2;

		m_instruction_length = (1<<19);

#if 1
		//m_opcode_hits[(OP >> 8) & 0x00ff]++;
		/* execute opcode */
		switch ((OP >> 8) & 0x00ff)
		{
			case 0x00: hyperstone_chk_global_global(); break;
			case 0x01: hyperstone_chk_global_local(); break;
			case 0x02: hyperstone_chk_local_global(); break;
			case 0x03: hyperstone_chk_local_local(); break;
			case 0x04: op04(); break;
			case 0x05: hyperstone_movd_global_local(); break;
			case 0x06: hyperstone_movd_local_global(); break;
			case 0x07: hyperstone_movd_local_local(); break;
			case 0x08: op08(); break;
			case 0x09: op09(); break;
			case 0x0a: op0a(); break;
			case 0x0b: op0b(); break;
			case 0x0c: op0c(); break;
			case 0x0d: op0d(); break;
			case 0x0e: op0e(); break;
			case 0x0f: hyperstone_divs_local_local(); break;
			case 0x10: op10(); break;
			case 0x11: op11(); break;
			case 0x12: op12(); break;
			case 0x13: op13(); break;
			case 0x14: op14(); break;
			case 0x15: hyperstone_mask_global_local(); break;
			case 0x16: hyperstone_mask_local_global(); break;
			case 0x17: hyperstone_mask_local_local(); break;
			case 0x18: hyperstone_sum_global_global(); break;
			case 0x19: op19(); break;
			case 0x1a: hyperstone_sum_local_global(); break;
			case 0x1b: hyperstone_sum_local_local(); break;
			case 0x1c: op1c(); break;
			case 0x1d: op1d(); break;
			case 0x1e: op1e(); break;
			case 0x1f: op1f(); break;
			case 0x20: op20(); break;
			case 0x21: op21(); break;
			case 0x22: hyperstone_cmp_local_global(); break;
			case 0x23: hyperstone_cmp_local_local(); break;
			case 0x24: hyperstone_mov_global_global(); break;
			case 0x25: hyperstone_mov_global_local(); break;
			case 0x26: hyperstone_mov_local_global(); break;
			case 0x27: hyperstone_mov_local_local(); break;
			case 0x28: op28(); break;
			case 0x29: hyperstone_add_global_local(); break;
			case 0x2a: hyperstone_add_local_global(); break;
			case 0x2b: hyperstone_add_local_local(); break;
			case 0x2c: op2c(); break;
			case 0x2d: op2d(); break;
			case 0x2e: op2e(); break;
			case 0x2f: op2f(); break;
			case 0x30: hyperstone_cmpb_global_global(); break;
			case 0x31: hyperstone_cmpb_global_local(); break;
			case 0x32: hyperstone_cmpb_local_global(); break;
			case 0x33: hyperstone_cmpb_local_local(); break;
			case 0x34: hyperstone_andn_global_global(); break;
			case 0x35: hyperstone_andn_global_local(); break;
			case 0x36: hyperstone_andn_local_global(); break;
			case 0x37: hyperstone_andn_local_local(); break;
			case 0x38: hyperstone_or_global_global(); break;
			case 0x39: hyperstone_or_global_local(); break;
			case 0x3a: hyperstone_or_local_global(); break;
			case 0x3b: hyperstone_or_local_local(); break;
			case 0x3c: hyperstone_xor_global_global(); break;
			case 0x3d: hyperstone_xor_global_local(); break;
			case 0x3e: hyperstone_xor_local_global(); break;
			case 0x3f: hyperstone_xor_local_local(); break;
			case 0x40: op40(); break;
			case 0x41: op41(); break;
			case 0x42: op42(); break;
			case 0x43: op43(); break;
			case 0x44: hyperstone_not_global_global(); break;
			case 0x45: hyperstone_not_global_local(); break;
			case 0x46: hyperstone_not_local_global(); break;
			case 0x47: hyperstone_not_local_local(); break;
			case 0x48: op48(); break;
			case 0x49: op49(); break;
			case 0x4a: op4a(); break;
			case 0x4b: op4b(); break;
			case 0x4c: op4c(); break;
			case 0x4d: op4d(); break;
			case 0x4e: op4e(); break;
			case 0x4f: op4f(); break;
			case 0x50: op50(); break;
			case 0x51: op51(); break;
			case 0x52: op52(); break;
			case 0x53: op53(); break;
			case 0x54: hyperstone_and_global_global(); break;
			case 0x55: hyperstone_and_global_local(); break;
			case 0x56: hyperstone_and_local_global(); break;
			case 0x57: hyperstone_and_local_local(); break;
			case 0x58: op58(); break;
			case 0x59: op59(); break;
			case 0x5a: op5a(); break;
			case 0x5b: op5b(); break;
			case 0x5c: op5c(); break;
			case 0x5d: op5d(); break;
			case 0x5e: op5e(); break;
			case 0x5f: op5f(); break;
			case 0x60: hyperstone_cmpi_global_simm(); break;
			case 0x61: hyperstone_cmpi_global_limm(); break;
			case 0x62: hyperstone_cmpi_local_simm(); break;
			case 0x63: hyperstone_cmpi_local_limm(); break;
			case 0x64: hyperstone_movi_global_simm(); break;
			case 0x65: hyperstone_movi_global_limm(); break;
			case 0x66: hyperstone_movi_local_simm(); break;
			case 0x67: hyperstone_movi_local_limm(); break;
			case 0x68: hyperstone_addi_global_simm(); break;
			case 0x69: hyperstone_addi_global_limm(); break;
			case 0x6a: hyperstone_addi_local_simm(); break;
			case 0x6b: hyperstone_addi_local_limm(); break;
			case 0x6c: op6c(); break;
			case 0x6d: op6d(); break;
			case 0x6e: op6e(); break;
			case 0x6f: op6f(); break;
			case 0x70: hyperstone_cmpbi_global_simm(); break;
			case 0x71: hyperstone_cmpbi_global_limm(); break;
			case 0x72: hyperstone_cmpbi_local_simm(); break;
			case 0x73: hyperstone_cmpbi_local_limm(); break;
			case 0x74: op74(); break;
			case 0x75: op75(); break;
			case 0x76: op76(); break;
			case 0x77: op77(); break;
			case 0x78: hyperstone_ori_global_simm(); break;
			case 0x79: hyperstone_ori_global_limm(); break;
			case 0x7a: hyperstone_ori_local_simm(); break;
			case 0x7b: hyperstone_ori_local_limm(); break;
			case 0x7c: hyperstone_xori_global_simm(); break;
			case 0x7d: hyperstone_xori_global_limm(); break;
			case 0x7e: hyperstone_xori_local_simm(); break;
			case 0x7f: hyperstone_xori_local_limm(); break;
			case 0x80: op80(); break;
			case 0x81: op81(); break;
			case 0x82: hyperstone_shrd(); break;
			case 0x83: op83(); break;
			case 0x84: hyperstone_sardi(); break;
			case 0x85: hyperstone_sardi(); break;
			case 0x86: hyperstone_sard(); break;
			case 0x87: op87(); break;
			case 0x88: hyperstone_shldi(); break;
			case 0x89: hyperstone_shldi(); break;
			case 0x8a: hyperstone_shld(); break;
			case 0x8b: hyperstone_shl(); break;
			case 0x8c: op8c(); break;
			case 0x8d: op8d(); break;
			case 0x8e: hyperstone_testlz(); break;
			case 0x8f: op8f(); break;
			case 0x90: hyperstone_ldxx1_global_global(); break;
			case 0x91: hyperstone_ldxx1_global_local(); break;
			case 0x92: hyperstone_ldxx1_local_global(); break;
			case 0x93: hyperstone_ldxx1_local_local(); break;
			case 0x94: op94(); break;
			case 0x95: op95(); break;
			case 0x96: op96(); break;
			case 0x97: op97(); break;
			case 0x98: op98(); break;
			case 0x99: op99(); break;
			case 0x9a: op9a(); break;
			case 0x9b: op9b(); break;
			case 0x9c: op9c(); break;
			case 0x9d: op9d(); break;
			case 0x9e: op9e(); break;
			case 0x9f: op9f(); break;
			case 0xa0: hyperstone_shri_global(); break;
			case 0xa1: hyperstone_shri_global(); break;
			case 0xa2: hyperstone_shri_local(); break;
			case 0xa3: hyperstone_shri_local(); break;
			case 0xa4: opa4(); break;
			case 0xa5: opa5(); break;
			case 0xa6: opa6(); break;
			case 0xa7: opa7(); break;
			case 0xa8: hyperstone_shli_global(); break;
			case 0xa9: hyperstone_shli_global(); break;
			case 0xaa: hyperstone_shli_local(); break;
			case 0xab: hyperstone_shli_local(); break;
			case 0xac: opac(); break;
			case 0xad: opad(); break;
			case 0xae: opae(); break;
			case 0xaf: opaf(); break;
			case 0xb0: opb0(); break;
			case 0xb1: opb1(); break;
			case 0xb2: opb2(); break;
			case 0xb3: opb3(); break;
			case 0xb4: opb4(); break;
			case 0xb5: opb5(); break;
			case 0xb6: opb6(); break;
			case 0xb7: opb7(); break;
			case 0xb8: hyperstone_set_global(); break;
			case 0xb9: hyperstone_set_global(); break;
			case 0xba: hyperstone_set_local(); break;
			case 0xbb: hyperstone_set_local(); break;
			case 0xbc: opbc(); break;
			case 0xbd: opbd(); break;
			case 0xbe: opbe(); break;
			case 0xbf: opbf(); break;
			case 0xc0: execute_software(); break; // fadd
			case 0xc1: execute_software(); break; // faddd
			case 0xc2: execute_software(); break; // fsub
			case 0xc3: execute_software(); break; // fsubd
			case 0xc4: execute_software(); break; // fmul
			case 0xc5: execute_software(); break; // fmuld
			case 0xc6: execute_software(); break; // fdiv
			case 0xc7: execute_software(); break; // fdivd
			case 0xc8: execute_software(); break; // fcmp
			case 0xc9: execute_software(); break; // fcmpd
			case 0xca: execute_software(); break; // fcmpu
			case 0xcb: execute_software(); break; // fcmpud
			case 0xcc: execute_software(); break; // fcvt
			case 0xcd: execute_software(); break; // fcvtd
			case 0xce: opce(); break;
			case 0xcf: opcf(); break;
			case 0xd0: opd0(); break;
			case 0xd1: opd1(); break;
			case 0xd2: opd2(); break;
			case 0xd3: opd3(); break;
			case 0xd4: opd4(); break;
			case 0xd5: opd5(); break;
			case 0xd6: opd6(); break;
			case 0xd7: opd7(); break;
			case 0xd8: hyperstone_stwr_global(); break;
			case 0xd9: hyperstone_stwr_local(); break;
			case 0xda: opda(); break;
			case 0xdb: opdb(); break;
			case 0xdc: opdc(); break;
			case 0xdd: opdd(); break;
			case 0xde: opde(); break;
			case 0xdf: opdf(); break;
			case 0xe0: ope0(); break;
			case 0xe1: ope1(); break;
			case 0xe2: ope2(); break;
			case 0xe3: ope3(); break;
			case 0xe4: ope4(); break;
			case 0xe5: ope5(); break;
			case 0xe6: ope6(); break;
			case 0xe7: ope7(); break;
			case 0xe8: ope8(); break;
			case 0xe9: ope9(); break;
			case 0xea: opea(); break;
			case 0xeb: opeb(); break;
			case 0xec: opec(); break;
			case 0xed: oped(); break;
			case 0xee: opee(); break;
			case 0xef: opef(); break;
			case 0xf0: opf0(); break;
			case 0xf1: opf1(); break;
			case 0xf2: opf2(); break;
			case 0xf3: opf3(); break;
			case 0xf4: opf4(); break;
			case 0xf5: opf5(); break;
			case 0xf6: opf6(); break;
			case 0xf7: opf7(); break;
			case 0xf8: opf8(); break;
			case 0xf9: opf9(); break;
			case 0xfa: opfa(); break;
			case 0xfb: opfb(); break;
			case 0xfc: execute_br(); break;
			case 0xfd: hyperstone_trap(); break;
			case 0xfe: hyperstone_trap(); break;
			case 0xff: hyperstone_trap(); break;
		}
#else
		(this->*m_opcode[(OP & 0xff00) >> 8])();
#endif

		/* clear the H state if it was previously set */
		SR ^= oldh;

		SET_ILC(m_instruction_length);

		if (GET_T && GET_P && !m_delay_slot) /* Not in a Delayed Branch instructions */
		{
			uint32_t addr = get_trap_addr(TRAPNO_TRACE_EXCEPTION);
			execute_exception(addr);
		}

		if (--m_intblock == 0)
			check_interrupts();

	} while( m_icount > 0 );
}

DEFINE_DEVICE_TYPE(E116T,      e116t_device,      "e116t",      "E1-16T")
DEFINE_DEVICE_TYPE(E116XT,     e116xt_device,     "e116xt",     "E1-16XT")
DEFINE_DEVICE_TYPE(E116XS,     e116xs_device,     "e116xs",     "E1-16XS")
DEFINE_DEVICE_TYPE(E116XSR,    e116xsr_device,    "e116xsr",    "E1-16XSR")
DEFINE_DEVICE_TYPE(E132N,      e132n_device,      "e132n",      "E1-32N")
DEFINE_DEVICE_TYPE(E132T,      e132t_device,      "e132t",      "E1-32T")
DEFINE_DEVICE_TYPE(E132XN,     e132xn_device,     "e132xn",     "E1-32XN")
DEFINE_DEVICE_TYPE(E132XT,     e132xt_device,     "e132xt",     "E1-32XT")
DEFINE_DEVICE_TYPE(E132XS,     e132xs_device,     "e132xs",     "E1-32XS")
DEFINE_DEVICE_TYPE(E132XSR,    e132xsr_device,    "e132xsr",    "E1-32XSR")
DEFINE_DEVICE_TYPE(GMS30C2116, gms30c2116_device, "gms30c2116", "GMS30C2116")
DEFINE_DEVICE_TYPE(GMS30C2132, gms30c2132_device, "gms30c2132", "GMS30C2132")
DEFINE_DEVICE_TYPE(GMS30C2216, gms30c2216_device, "gms30c2216", "GMS30C2216")
DEFINE_DEVICE_TYPE(GMS30C2232, gms30c2232_device, "gms30c2232", "GMS30C2232")
