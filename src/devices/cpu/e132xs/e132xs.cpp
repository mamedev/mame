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
 - verify register wrapping with sregf/dregf on hardware

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

 Pierpaolo Prazzoli - 08/19/03
    - Added check for D_BIT and S_BIT where PC or SR must or must not be denoted.
      (movd, divu, divs, ldxx1, ldxx2, stxx1, stxx2, mulu, muls, set, mul
      call, chk)

 Pierpaolo Prazzoli - 08/17/03
    - Fixed get_pcrel() when OP & 0x80 is set.
    - Decremented PC by 2 also in MOV, ADD, ADDI, SUM, SUB and added the check if
      D_BIT is not set. (when pc is changed they are implicit branch)

*********************************************************************/

#include "emu.h"
#include "e132xs.h"
#include "e132xsfe.h"

#include "32xsdefs.h"

//#define VERBOSE 1
#include "logmacro.h"

/* size of the execution code cache */
#define CACHE_SIZE                      (32 * 1024 * 1024)

//**************************************************************************
//  INTERNAL ADDRESS MAP
//**************************************************************************

// 4Kb IRAM (On-Chip Memory)

void hyperstone_device::e116_4k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0000fff).ram().mirror(0x1ffff000);
}

void hyperstone_device::e132_4k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0000fff).ram().mirror(0x1ffff000);
}


// 8Kb IRAM (On-Chip Memory)

void hyperstone_device::e116_8k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0001fff).ram().mirror(0x1fffe000);
}

void hyperstone_device::e132_8k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0001fff).ram().mirror(0x1fffe000);
}


// 16Kb IRAM (On-Chip Memory)

void hyperstone_device::e116_16k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0003fff).ram().mirror(0x1fffc000);
}

void hyperstone_device::e132_16k_iram_map(address_map &map)
{
	map(0xc0000000, 0xc0003fff).ram().mirror(0x1fffc000);
}


//-------------------------------------------------
//  hyperstone_device - constructor
//-------------------------------------------------

hyperstone_device::hyperstone_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock,
										const device_type type, uint32_t prg_data_width, uint32_t io_data_width, address_map_constructor internal_map)
	: cpu_device(mconfig, type, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_BIG, prg_data_width, 32, 0, internal_map)
	, m_io_config("io", ENDIANNESS_BIG, io_data_width, 15)
	, m_cache(CACHE_SIZE + sizeof(hyperstone_device))
	, m_drcuml(nullptr)
	, m_drcfe(nullptr)
	, m_drcoptions(0)
	, m_cache_dirty(0)
	, m_entry(nullptr)
	, m_nocode(nullptr)
	, m_interrupt_checks(nullptr)
	, m_out_of_cycles(nullptr)
	, m_mem_read8(nullptr)
	, m_mem_write8(nullptr)
	, m_mem_read16(nullptr)
	, m_mem_write16(nullptr)
	, m_mem_read32(nullptr)
	, m_mem_write32(nullptr)
	, m_io_read32(nullptr)
	, m_io_write32(nullptr)
	, m_enable_drc(false)
{
	std::fill(std::begin(m_exception), std::end(m_exception), nullptr);
}

hyperstone_device::~hyperstone_device()
{
}


//-------------------------------------------------
//  e116t_device - constructor
//-------------------------------------------------

e116t_device::e116t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116T, 16, 16, address_map_constructor(FUNC(e116t_device::e116_4k_iram_map), this))
{
}


//-------------------------------------------------
//  e116xt_device - constructor
//-------------------------------------------------

e116xt_device::e116xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XT, 16, 16, address_map_constructor(FUNC(e116t_device::e116_8k_iram_map), this))
{
}


//-------------------------------------------------
//  e116xs_device - constructor
//-------------------------------------------------

e116xs_device::e116xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XS, 16, 16, address_map_constructor(FUNC(e116xs_device::e116_16k_iram_map), this))
{
}


//-------------------------------------------------
//  e116xsr_device - constructor
//-------------------------------------------------

e116xsr_device::e116xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E116XSR, 16, 16, address_map_constructor(FUNC(e116xsr_device::e116_16k_iram_map), this))
{
}


//-------------------------------------------------
//  e132n_device - constructor
//-------------------------------------------------

e132n_device::e132n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132N, 32, 32, address_map_constructor(FUNC(e132n_device::e132_4k_iram_map), this))
{
}


//-------------------------------------------------
//  e132t_device - constructor
//-------------------------------------------------

e132t_device::e132t_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132T, 32, 32, address_map_constructor(FUNC(e132t_device::e132_4k_iram_map), this))
{
}


//-------------------------------------------------
//  e132xn_device - constructor
//-------------------------------------------------

e132xn_device::e132xn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XN, 32, 32, address_map_constructor(FUNC(e132xn_device::e132_8k_iram_map), this))
{
}


//-------------------------------------------------
//  e132xt_device - constructor
//-------------------------------------------------

e132xt_device::e132xt_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XT, 32, 32, address_map_constructor(FUNC(e132xt_device::e132_8k_iram_map), this))
{
}


//-------------------------------------------------
//  e132xs_device - constructor
//-------------------------------------------------

e132xs_device::e132xs_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XS, 32, 32, address_map_constructor(FUNC(e132xs_device::e132_16k_iram_map), this))
{
}


//-------------------------------------------------
//  e132xsr_device - constructor
//-------------------------------------------------

e132xsr_device::e132xsr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, E132XSR, 32, 32, address_map_constructor(FUNC(e132xsr_device::e132_16k_iram_map), this))
{
}


//-------------------------------------------------
//  gms30c2116_device - constructor
//-------------------------------------------------

gms30c2116_device::gms30c2116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2116, 16, 16, address_map_constructor(FUNC(gms30c2116_device::e116_4k_iram_map), this))
{
}


//-------------------------------------------------
//  gms30c2132_device - constructor
//-------------------------------------------------

gms30c2132_device::gms30c2132_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2132, 32, 32, address_map_constructor(FUNC(gms30c2132_device::e132_4k_iram_map), this))
{
}


//-------------------------------------------------
//  gms30c2216_device - constructor
//-------------------------------------------------

gms30c2216_device::gms30c2216_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2216, 16, 16, address_map_constructor(FUNC(gms30c2216_device::e116_8k_iram_map), this))
{
}


//-------------------------------------------------
//  gms30c2232_device - constructor
//-------------------------------------------------

gms30c2232_device::gms30c2232_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: hyperstone_device(mconfig, tag, owner, clock, GMS30C2232, 32, 32, address_map_constructor(FUNC(gms30c2232_device::e132_8k_iram_map), this))
{
}

/* Return the entry point for a determinated trap */
uint32_t hyperstone_device::get_trap_addr(uint8_t trapno)
{
	uint32_t addr;
	if( m_core->trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = trapno * 4;
	}
	else
	{
		addr = (63 - trapno) * 4;
	}
	addr |= m_core->trap_entry;

	return addr;
}

/* Return the entry point for a determinated emulated code (the one for "extend" opcode is reserved) */
uint32_t hyperstone_device::get_emu_code_addr(uint8_t num) /* num is OP */
{
	uint32_t addr;
	if( m_core->trap_entry == 0xffffff00 ) /* @ MEM3 */
	{
		addr = (m_core->trap_entry - 0x100) | ((num & 0xf) << 4);
	}
	else
	{
		addr = m_core->trap_entry | (0x10c | ((0xcf - num) << 4));
	}
	return addr;
}

/*static*/ const uint32_t hyperstone_device::s_trap_entries[8] = {
	0x00000000, // MEM0
	0x40000000, // MEM1
	0x80000000, // MEM2
	0xc0000000, // IRAM
	0,
	0,
	0,
	0xffffff00, // MEM3
};

#if E132XS_LOG_INTERPRETER_REGS
void hyperstone_device::dump_registers()
{
	uint8_t packed[4];
	packed[0] = (uint8_t)m_core->intblock;
	packed[1] = (uint8_t)(m_core->icount >> 16);
	packed[2] = (uint8_t)(m_core->icount >>  8);
	packed[3] = (uint8_t)(m_core->icount >>  0);
	fwrite(packed, 1, 4, m_trace_log);
	fwrite(m_core->global_regs, 4, 32, m_trace_log);
	fwrite(m_core->local_regs, 4, 64, m_trace_log);
}
#endif

void hyperstone_device::compute_tr()
{
	uint64_t cycles_since_base = total_cycles() - m_core->tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_core->clck_scale;
	m_core->tr_result = m_core->tr_base_value + (clocks_since_base / m_core->tr_clocks_per_tick);
}

void hyperstone_device::update_timer_prescale()
{
	TPR &= ~0x80000000;
	m_core->clck_scale = (TPR >> 26) & m_core->clock_scale_mask;
	m_core->clock_cycles_1 = 1 << m_core->clck_scale;
	m_core->clock_cycles_2 = 2 << m_core->clck_scale;
	m_core->clock_cycles_3 = 3 << m_core->clck_scale;
	m_core->clock_cycles_4 = 4 << m_core->clck_scale;
	m_core->clock_cycles_6 = 6 << m_core->clck_scale;
	m_core->clock_cycles_36 = 36 << m_core->clck_scale;
	m_core->tr_clocks_per_tick = ((TPR >> 16) & 0xff) + 2;
	m_core->tr_base_value = m_core->tr_result;
	m_core->tr_base_cycles = total_cycles();
}

void hyperstone_device::adjust_timer_interrupt()
{
	uint64_t cycles_since_base = total_cycles() - m_core->tr_base_cycles;
	uint64_t clocks_since_base = cycles_since_base >> m_core->clck_scale;
	uint64_t cycles_until_next_clock = cycles_since_base - (clocks_since_base << m_core->clck_scale);

	if (cycles_until_next_clock == 0)
		cycles_until_next_clock = (uint64_t)(1 << m_core->clck_scale);

	/* special case: if we have a change pending, set a timer to fire then */
	if (TPR & 0x80000000)
	{
		uint64_t clocks_until_int = m_core->tr_clocks_per_tick - (clocks_since_base % m_core->tr_clocks_per_tick);
		uint64_t cycles_until_int = (clocks_until_int << m_core->clck_scale) + cycles_until_next_clock;
		m_timer->adjust(cycles_to_attotime(cycles_until_int + 1), 1);
	}

	/* else if the timer interrupt is enabled, configure it to fire at the appropriate time */
	else if (!(FCR & 0x00800000))
	{
		uint32_t curtr = m_core->tr_base_value + (clocks_since_base / m_core->tr_clocks_per_tick);
		uint32_t delta = TCR - curtr;
		if (delta > 0x80000000)
		{
			if (!m_core->timer_int_pending)
				m_timer->adjust(attotime::zero);
		}
		else
		{
			uint64_t clocks_until_int = mulu_32x32(delta, m_core->tr_clocks_per_tick);
			uint64_t cycles_until_int = (clocks_until_int << m_core->clck_scale) + cycles_until_next_clock;
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
	{
		update_timer_prescale();
	}

	/* see if the timer is right for firing */
	compute_tr();
	if (!((m_core->tr_result - TCR) & 0x80000000))
		m_core->timer_int_pending = 1;

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
            LOG("read _Reserved_ Global Register %d @ %08X\n",code,PC);
            break;

        case BCR_REGISTER:
            LOG("read write-only BCR register @ %08X\n",PC);
            return 0;

        case TPR_REGISTER:
            LOG("read write-only TPR register @ %08X\n",PC);
            return 0;

        case FCR_REGISTER:
            LOG("read write-only FCR register @ %08X\n",PC);
            return 0;

        case MCR_REGISTER:
            LOG("read write-only MCR register @ %08X\n",PC);
            return 0;
        }
    }
*/
	if (code == TR_REGISTER)
	{
		/* it is common to poll this in a loop */
		if (m_core->icount > m_core->tr_clocks_per_tick / 2)
			m_core->icount -= m_core->tr_clocks_per_tick / 2;
		compute_tr();
		return m_core->tr_result;
	}
	return m_core->global_regs[code & 0x1f];
}

void hyperstone_device::set_local_register(uint8_t code, uint32_t val)
{
	m_core->local_regs[(code + GET_FP) & 0x3f] = val;
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
			if (m_core->intblock < 1)
				m_core->intblock = 1;
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
			m_core->global_regs[code] = val;
			return;
		case SP_REGISTER:
		case UB_REGISTER:
			m_core->global_regs[code] = val & ~3;
			return;
		case BCR_REGISTER:
			m_core->global_regs[code] = val;
			return;
		case TPR_REGISTER:
			m_core->global_regs[code] = val;
			if (!(val & 0x80000000)) /* change immediately */
			{
				compute_tr();
				update_timer_prescale();
			}
			adjust_timer_interrupt();
			return;
		case TCR_REGISTER:
			if (m_core->global_regs[code] != val)
			{
				m_core->global_regs[code] = val;
				adjust_timer_interrupt();
				if (m_core->intblock < 1)
					m_core->intblock = 1;
			}
			return;
		case TR_REGISTER:
			m_core->global_regs[code] = val;
			m_core->tr_base_value = val;
			m_core->tr_base_cycles = total_cycles();
			adjust_timer_interrupt();
			return;
		case WCR_REGISTER:
			m_core->global_regs[code] = val;
			return;
		case ISR_REGISTER:
			return;
		case FCR_REGISTER:
			if ((m_core->global_regs[code] ^ val) & 0x00800000)
				adjust_timer_interrupt();
			m_core->global_regs[code] = val;
			if (m_core->intblock < 1)
				m_core->intblock = 1;
			return;
		case MCR_REGISTER:
		{
			// bits 14..12 EntryTableMap
			const int which = (val & 0x7000) >> 12;
			assert(which < 4 || which == 7);
			m_core->trap_entry = s_trap_entries[which];
			m_core->global_regs[code] = val;
			return;
		}
		case 28:
		case 29:
		case 30:
		case 31:
			m_core->global_regs[code] = val;
			return;
	}
}

/*static*/ const int32_t hyperstone_device::s_immediate_values[16] =
{
	16, 0, 0, 0, 32, 64, 128, int32_t(0x80000000),
	-8, -7, -6, -5, -4, -3, -2, -1
};

constexpr uint32_t WRITE_ONLY_REGMASK = (1 << BCR_REGISTER) | (1 << TPR_REGISTER) | (1 << FCR_REGISTER) | (1 << MCR_REGISTER);

#define check_delay_PC()                                                            \
do                                                                                  \
{                                                                                   \
	/* if PC is used in a delay instruction, the delayed PC should be used */       \
	if (m_core->delay_slot)                                                         \
	{                                                                               \
		PC = m_core->delay_pc;                                                      \
		m_core->delay_slot = 0;                                                     \
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
			return 16;
		case 1:
		{
			m_instruction_length = (3<<19);
			uint32_t extra_u = (m_pr16(PC) << 16) | m_pr16(PC + 2);
			PC += 4;
			return extra_u;
		}
		case 2:
		{
			m_instruction_length = (2<<19);
			uint32_t extra_u = m_pr16(PC);
			PC += 2;
			return extra_u;
		}
		case 3:
		{
			m_instruction_length = (2<<19);
			uint32_t extra_u = 0xffff0000 | m_pr16(PC);
			PC += 2;
			return extra_u;
		}
		default:
			return s_immediate_values[nybble];
	}
}

uint32_t hyperstone_device::decode_const()
{
	const uint16_t imm_1 = m_pr16(PC);

	PC += 2;

	if (imm_1 & 0x8000)
	{
		const uint16_t imm_2 = m_pr16(PC);

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
		m_instruction_length = (2<<19);

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
		uint16_t next = m_pr16(PC);

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

void hyperstone_device::hyperstone_br()
{
	const int32_t offset = decode_pcrel();
	check_delay_PC();

	PC += offset;
	SR &= ~M_MASK;

	m_core->icount -= m_core->clock_cycles_2;
}

void hyperstone_device::execute_trap(uint32_t addr)
{
	const uint8_t reg = GET_FP + GET_FL;
	SET_ILC(m_instruction_length);
	const uint32_t oldSR = SR;

	SET_FL(6);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}


void hyperstone_device::execute_int(uint32_t addr)
{
	const uint8_t reg = GET_FP + GET_FL;
	SET_ILC(m_instruction_length);
	const uint32_t oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK | I_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}

/* TODO: mask Parity Error and Extended Overflow exceptions */
void hyperstone_device::execute_exception(uint32_t addr)
{
	const uint8_t reg = GET_FP + GET_FL;
	SET_ILC(m_instruction_length);
	const uint32_t oldSR = SR;

	SET_FL(2);
	SET_FP(reg);

	m_core->local_regs[(0 + reg) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(1 + reg) & 0x3f] = oldSR;

	SR &= ~(M_MASK | T_MASK);
	SR |= (L_MASK | S_MASK);

	PC = addr;

	m_core->icount -= m_core->clock_cycles_2;
}

void hyperstone_device::execute_software()
{
	check_delay_PC();

	const uint32_t fp = GET_FP;
	const uint32_t src_code = SRC_CODE;
	const uint32_t sreg = m_core->local_regs[(src_code + fp) & 0x3f];
	const uint32_t sregf = m_core->local_regs[(src_code + 1 + fp) & 0x3f];

	SET_ILC(1<<19);

	const uint32_t addr = get_emu_code_addr((m_op & 0xff00) >> 8);
	const uint8_t reg = fp + GET_FL;

	//since it's sure the register is in the register part of the stack,
	//set the stack address to a value above the highest address
	//that can be set by a following frame instruction
	const uint32_t stack_of_dst = (SP & ~0xff) + 0x100 + (((fp + DST_CODE) & 0x3f) << 2); //converted to 32bits offset

	m_core->local_regs[(reg + 0) & 0x3f] = stack_of_dst;
	m_core->local_regs[(reg + 1) & 0x3f] = sreg;
	m_core->local_regs[(reg + 2) & 0x3f] = sregf;
	m_core->local_regs[(reg + 3) & 0x3f] = (PC & ~1) | GET_S;
	m_core->local_regs[(reg + 4) & 0x3f] = SR;

	SET_FL(6);
	SET_FP(reg);

	SR &= ~(M_MASK | T_MASK);
	SR |= L_MASK;

	PC = addr;

	m_core->icount -= m_core->clock_cycles_6;
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

#define INT1_LINE_STATE     (ISR & 0x01)
#define INT2_LINE_STATE     (ISR & 0x02)
#define INT3_LINE_STATE     (ISR & 0x04)
#define INT4_LINE_STATE     (ISR & 0x08)
#define IO1_LINE_STATE      (ISR & 0x10)
#define IO2_LINE_STATE      (ISR & 0x20)
#define IO3_LINE_STATE      (ISR & 0x40)

template <hyperstone_device::is_timer TIMER>
void hyperstone_device::check_interrupts()
{
	/* Interrupt-Lock flag isn't set */
	if (GET_L || m_core->intblock > 0)
		return;

	/* quick exit if nothing */
	if (TIMER == NO_TIMER && (ISR & 0x7f) == 0)
		return;

	if (TIMER == NO_TIMER)
	{
		/* IO3 is priority 5; state is in bit 6 of ISR; FCR bit 10 enables input and FCR bit 8 inhibits interrupt */
		if (IO3_LINE_STATE && (FCR & 0x00000500) == 0x00000400)
		{
			standard_irq_callback(IRQ_IO3, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO3));
			return;
		}

		/* INT1 is priority 7; state is in bit 0 of ISR; FCR bit 28 inhibits interrupt */
		if (INT1_LINE_STATE && (FCR & 0x10000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT1, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT1));
			return;
		}

		/* INT2 is priority 9; state is in bit 1 of ISR; FCR bit 29 inhibits interrupt */
		if (INT2_LINE_STATE && (FCR & 0x20000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT2, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT2));
			return;
		}

		/* INT3 is priority 11; state is in bit 2 of ISR; FCR bit 30 inhibits interrupt */
		if (INT3_LINE_STATE && (FCR & 0x40000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT3, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT3));
			return;
		}

		/* INT4 is priority 13; state is in bit 3 of ISR; FCR bit 31 inhibits interrupt */
		if (INT4_LINE_STATE && (FCR & 0x80000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT4, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT4));
			return;
		}

		/* IO1 is priority 14; state is in bit 4 of ISR; FCR bit 2 enables input and FCR bit 0 inhibits interrupt */
		if (IO1_LINE_STATE && (FCR & 0x00000005) == 0x00000004)
		{
			standard_irq_callback(IRQ_IO1, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO1));
			return;
		}

		/* IO2 is priority 15; state is in bit 5 of ISR; FCR bit 6 enables input and FCR bit 4 inhibits interrupt */
		if (IO2_LINE_STATE && (FCR & 0x00000050) == 0x00000040)
		{
			standard_irq_callback(IRQ_IO2, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO2));
			return;
		}
	}
	else
	{
		/* IO3 is priority 5; state is in bit 6 of ISR; FCR bit 10 enables input and FCR bit 8 inhibits interrupt */
		if (IO3_LINE_STATE && (FCR & 0x00000500) == 0x00000400)
		{
			standard_irq_callback(IRQ_IO3, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO3));
			return;
		}

		/* timer int might be priority 6 if FCR bits 20-21 == 3; FCR bit 23 inhibits interrupt */
		if (TIMER && (FCR & 0x00b00000) == 0x00300000)
		{
			m_core->timer_int_pending = 0;
			execute_int(get_trap_addr(TRAPNO_TIMER));
			return;
		}

		/* INT1 is priority 7; state is in bit 0 of ISR; FCR bit 28 inhibits interrupt */
		if (INT1_LINE_STATE && (FCR & 0x10000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT1, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT1));
			return;
		}

		/* timer int might be priority 8 if FCR bits 20-21 == 2; FCR bit 23 inhibits interrupt */
		if (TIMER && (FCR & 0x00b00000) == 0x00200000)
		{
			m_core->timer_int_pending = 0;
			execute_int(get_trap_addr(TRAPNO_TIMER));
			return;
		}

		/* INT2 is priority 9; state is in bit 1 of ISR; FCR bit 29 inhibits interrupt */
		if (INT2_LINE_STATE && (FCR & 0x20000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT2, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT2));
			return;
		}

		/* timer int might be priority 10 if FCR bits 20-21 == 1; FCR bit 23 inhibits interrupt */
		if (TIMER && (FCR & 0x00b00000) == 0x00100000)
		{
			m_core->timer_int_pending = 0;
			execute_int(get_trap_addr(TRAPNO_TIMER));
			return;
		}

		/* INT3 is priority 11; state is in bit 2 of ISR; FCR bit 30 inhibits interrupt */
		if (INT3_LINE_STATE && (FCR & 0x40000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT3, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT3));
			return;
		}

		/* timer int might be priority 12 if FCR bits 20-21 == 0; FCR bit 23 inhibits interrupt */
		if (TIMER && (FCR & 0x00b00000) == 0x00000000)
		{
			m_core->timer_int_pending = 0;
			execute_int(get_trap_addr(TRAPNO_TIMER));
			return;
		}

		/* INT4 is priority 13; state is in bit 3 of ISR; FCR bit 31 inhibits interrupt */
		if (INT4_LINE_STATE && (FCR & 0x80000000) == 0x00000000)
		{
			standard_irq_callback(IRQ_INT4, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_INT4));
			return;
		}

		/* IO1 is priority 14; state is in bit 4 of ISR; FCR bit 2 enables input and FCR bit 0 inhibits interrupt */
		if (IO1_LINE_STATE && (FCR & 0x00000005) == 0x00000004)
		{
			standard_irq_callback(IRQ_IO1, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO1));
			return;
		}

		/* IO2 is priority 15; state is in bit 5 of ISR; FCR bit 6 enables input and FCR bit 4 inhibits interrupt */
		if (IO2_LINE_STATE && (FCR & 0x00000050) == 0x00000040)
		{
			standard_irq_callback(IRQ_IO2, m_core->global_regs[0]);
			execute_int(get_trap_addr(TRAPNO_IO2));
			return;
		}
	}
}

void hyperstone_device::device_start()
{
	// Handled entirely by init() and derived classes
}

void hyperstone_device::init(int scale_mask)
{
	m_instruction_length_valid = false;

	m_core = (internal_hyperstone_state *)m_cache.alloc_near(sizeof(internal_hyperstone_state));
	memset(m_core, 0, sizeof(internal_hyperstone_state));

#if ENABLE_E132XS_DRC
	m_enable_drc = allow_drc();
#else
	m_enable_drc = false;
#endif

#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	if (m_enable_drc)
		m_trace_log = fopen("e1_drc.log", "wb");
	else
		m_trace_log = fopen("e1_interpreter.log", "wb");
#endif

	memset(m_op_counts, 0, sizeof(uint32_t) * 256);
	memset(m_core->global_regs, 0, sizeof(uint32_t) * 32);
	memset(m_core->local_regs, 0, sizeof(uint32_t) * 64);
	m_op = 0;

	m_instruction_length = 0;

	m_program = &space(AS_PROGRAM);
	if (m_program->data_width() == 16)
	{
		m_program->cache(m_cache16);
		m_pr16 = [this](offs_t address) -> u16 { return m_cache16.read_word(address); };
		m_prptr = [this](offs_t address) -> const void * { return m_cache16.read_ptr(address); };
	}
	else
	{
		m_program->cache(m_cache32);
		m_pr16 = [this](offs_t address) -> u16 { return m_cache32.read_word(address); };
		if (ENDIANNESS_NATIVE != ENDIANNESS_BIG)
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
				if(!(address & 2))
					ptr++;
				return ptr;
			};
		else
			m_prptr = [this](offs_t address) -> const void * {
				const u16 *ptr = static_cast<u16 *>(m_cache32.read_ptr(address & ~3));
				if(address & 2)
					ptr++;
				return ptr;
			};
	}
	m_io = &space(AS_IO);

	m_timer = timer_alloc(FUNC(hyperstone_device::timer_callback), this);
	m_core->clock_scale_mask = scale_mask;

	for (uint8_t i = 0; i < 16; i++)
	{
		m_core->fl_lut[i] = (i ? i : 16);
	}

	uint32_t umlflags = 0;
	m_drcuml = std::make_unique<drcuml_state>(*this, m_cache, umlflags, 1, 32, 1);

	// add UML symbols-
	m_drcuml->symbol_add(&m_core->global_regs[0], sizeof(uint32_t), "pc");
	m_drcuml->symbol_add(&m_core->global_regs[1], sizeof(uint32_t), "sr");
	m_drcuml->symbol_add(&m_core->icount, sizeof(m_core->icount), "icount");

	char buf[4];
	for (int i=0; i < 32; i++)
	{
		sprintf(buf, "g%d", i);
		m_drcuml->symbol_add(&m_core->global_regs[i], sizeof(uint32_t), buf);
	}

	for (int i=0; i < 64; i++)
	{
		sprintf(buf, "l%d", i);
		m_drcuml->symbol_add(&m_core->local_regs[i], sizeof(uint32_t), buf);
	}

	m_drcuml->symbol_add(&m_core->arg0, sizeof(uint32_t), "arg0");
	m_drcuml->symbol_add(&m_core->arg1, sizeof(uint32_t), "arg1");

	/* initialize the front-end helper */
	m_drcfe = std::make_unique<e132xs_frontend>(this, COMPILE_BACKWARDS_BYTES, COMPILE_FORWARDS_BYTES, SINGLE_INSTRUCTION_MODE ? 1 : COMPILE_MAX_SEQUENCE);

	/* mark the cache dirty so it is updated on next execute */
	m_cache_dirty = true;

	// register our state for the debugger
	state_add(STATE_GENPC,    "GENPC",     m_core->global_regs[0]).noshow();
	state_add(STATE_GENPCBASE, "CURPC",    m_core->global_regs[0]).noshow();
	state_add(STATE_GENFLAGS, "GENFLAGS",  m_core->global_regs[1]).callimport().callexport().formatstr("%40s").noshow();
	state_add(E132XS_PC,      "PC", m_core->global_regs[0]).mask(0xffffffff);
	state_add(E132XS_SR,      "SR", m_core->global_regs[1]).mask(0xffffffff);
	state_add(E132XS_FER,     "FER", m_core->global_regs[2]).mask(0xffffffff);
	state_add(E132XS_G3,      "G3", m_core->global_regs[3]).mask(0xffffffff);
	state_add(E132XS_G4,      "G4", m_core->global_regs[4]).mask(0xffffffff);
	state_add(E132XS_G5,      "G5", m_core->global_regs[5]).mask(0xffffffff);
	state_add(E132XS_G6,      "G6", m_core->global_regs[6]).mask(0xffffffff);
	state_add(E132XS_G7,      "G7", m_core->global_regs[7]).mask(0xffffffff);
	state_add(E132XS_G8,      "G8", m_core->global_regs[8]).mask(0xffffffff);
	state_add(E132XS_G9,      "G9", m_core->global_regs[9]).mask(0xffffffff);
	state_add(E132XS_G10,     "G10", m_core->global_regs[10]).mask(0xffffffff);
	state_add(E132XS_G11,     "G11", m_core->global_regs[11]).mask(0xffffffff);
	state_add(E132XS_G12,     "G12", m_core->global_regs[12]).mask(0xffffffff);
	state_add(E132XS_G13,     "G13", m_core->global_regs[13]).mask(0xffffffff);
	state_add(E132XS_G14,     "G14", m_core->global_regs[14]).mask(0xffffffff);
	state_add(E132XS_G15,     "G15", m_core->global_regs[15]).mask(0xffffffff);
	state_add(E132XS_G16,     "G16", m_core->global_regs[16]).mask(0xffffffff);
	state_add(E132XS_G17,     "G17", m_core->global_regs[17]).mask(0xffffffff);
	state_add(E132XS_SP,      "SP", m_core->global_regs[18]).mask(0xffffffff);
	state_add(E132XS_UB,      "UB", m_core->global_regs[19]).mask(0xffffffff);
	state_add(E132XS_BCR,     "BCR", m_core->global_regs[20]).mask(0xffffffff);
	state_add(E132XS_TPR,     "TPR", m_core->global_regs[21]).mask(0xffffffff);
	state_add(E132XS_TCR,     "TCR", m_core->global_regs[22]).mask(0xffffffff);
	state_add(E132XS_TR,      "TR", m_core->global_regs[23]).mask(0xffffffff);
	state_add(E132XS_WCR,     "WCR", m_core->global_regs[24]).mask(0xffffffff);
	state_add(E132XS_ISR,     "ISR", m_core->global_regs[25]).mask(0xffffffff);
	state_add(E132XS_FCR,     "FCR", m_core->global_regs[26]).mask(0xffffffff);
	state_add(E132XS_MCR,     "MCR", m_core->global_regs[27]).mask(0xffffffff);
	state_add(E132XS_G28,     "G28", m_core->global_regs[28]).mask(0xffffffff);
	state_add(E132XS_G29,     "G29", m_core->global_regs[29]).mask(0xffffffff);
	state_add(E132XS_G30,     "G30", m_core->global_regs[30]).mask(0xffffffff);
	state_add(E132XS_G31,     "G31", m_core->global_regs[31]).mask(0xffffffff);
	state_add(E132XS_CL0,     "CL0", m_core->local_regs[(0 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL1,     "CL1", m_core->local_regs[(1 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL2,     "CL2", m_core->local_regs[(2 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL3,     "CL3", m_core->local_regs[(3 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL4,     "CL4", m_core->local_regs[(4 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL5,     "CL5", m_core->local_regs[(5 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL6,     "CL6", m_core->local_regs[(6 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL7,     "CL7", m_core->local_regs[(7 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL8,     "CL8", m_core->local_regs[(8 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL9,     "CL9", m_core->local_regs[(9 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL10,    "CL10", m_core->local_regs[(10 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL11,    "CL11", m_core->local_regs[(11 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL12,    "CL12", m_core->local_regs[(12 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL13,    "CL13", m_core->local_regs[(13 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL14,    "CL14", m_core->local_regs[(14 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_CL15,    "CL15", m_core->local_regs[(15 + GET_FP) % 64]).mask(0xffffffff);
	state_add(E132XS_L0,      "L0", m_core->local_regs[0]).mask(0xffffffff);
	state_add(E132XS_L1,      "L1", m_core->local_regs[1]).mask(0xffffffff);
	state_add(E132XS_L2,      "L2", m_core->local_regs[2]).mask(0xffffffff);
	state_add(E132XS_L3,      "L3", m_core->local_regs[3]).mask(0xffffffff);
	state_add(E132XS_L4,      "L4", m_core->local_regs[4]).mask(0xffffffff);
	state_add(E132XS_L5,      "L5", m_core->local_regs[5]).mask(0xffffffff);
	state_add(E132XS_L6,      "L6", m_core->local_regs[6]).mask(0xffffffff);
	state_add(E132XS_L7,      "L7", m_core->local_regs[7]).mask(0xffffffff);
	state_add(E132XS_L8,      "L8", m_core->local_regs[8]).mask(0xffffffff);
	state_add(E132XS_L9,      "L9", m_core->local_regs[9]).mask(0xffffffff);
	state_add(E132XS_L10,     "L10", m_core->local_regs[10]).mask(0xffffffff);
	state_add(E132XS_L11,     "L11", m_core->local_regs[11]).mask(0xffffffff);
	state_add(E132XS_L12,     "L12", m_core->local_regs[12]).mask(0xffffffff);
	state_add(E132XS_L13,     "L13", m_core->local_regs[13]).mask(0xffffffff);
	state_add(E132XS_L14,     "L14", m_core->local_regs[14]).mask(0xffffffff);
	state_add(E132XS_L15,     "L15", m_core->local_regs[15]).mask(0xffffffff);
	state_add(E132XS_L16,     "L16", m_core->local_regs[16]).mask(0xffffffff);
	state_add(E132XS_L17,     "L17", m_core->local_regs[17]).mask(0xffffffff);
	state_add(E132XS_L18,     "L18", m_core->local_regs[18]).mask(0xffffffff);
	state_add(E132XS_L19,     "L19", m_core->local_regs[19]).mask(0xffffffff);
	state_add(E132XS_L20,     "L20", m_core->local_regs[20]).mask(0xffffffff);
	state_add(E132XS_L21,     "L21", m_core->local_regs[21]).mask(0xffffffff);
	state_add(E132XS_L22,     "L22", m_core->local_regs[22]).mask(0xffffffff);
	state_add(E132XS_L23,     "L23", m_core->local_regs[23]).mask(0xffffffff);
	state_add(E132XS_L24,     "L24", m_core->local_regs[24]).mask(0xffffffff);
	state_add(E132XS_L25,     "L25", m_core->local_regs[25]).mask(0xffffffff);
	state_add(E132XS_L26,     "L26", m_core->local_regs[26]).mask(0xffffffff);
	state_add(E132XS_L27,     "L27", m_core->local_regs[27]).mask(0xffffffff);
	state_add(E132XS_L28,     "L28", m_core->local_regs[28]).mask(0xffffffff);
	state_add(E132XS_L29,     "L29", m_core->local_regs[29]).mask(0xffffffff);
	state_add(E132XS_L30,     "L30", m_core->local_regs[30]).mask(0xffffffff);
	state_add(E132XS_L31,     "L31", m_core->local_regs[31]).mask(0xffffffff);
	state_add(E132XS_L32,     "L32", m_core->local_regs[32]).mask(0xffffffff);
	state_add(E132XS_L33,     "L33", m_core->local_regs[33]).mask(0xffffffff);
	state_add(E132XS_L34,     "L34", m_core->local_regs[34]).mask(0xffffffff);
	state_add(E132XS_L35,     "L35", m_core->local_regs[35]).mask(0xffffffff);
	state_add(E132XS_L36,     "L36", m_core->local_regs[36]).mask(0xffffffff);
	state_add(E132XS_L37,     "L37", m_core->local_regs[37]).mask(0xffffffff);
	state_add(E132XS_L38,     "L38", m_core->local_regs[38]).mask(0xffffffff);
	state_add(E132XS_L39,     "L39", m_core->local_regs[39]).mask(0xffffffff);
	state_add(E132XS_L40,     "L40", m_core->local_regs[40]).mask(0xffffffff);
	state_add(E132XS_L41,     "L41", m_core->local_regs[41]).mask(0xffffffff);
	state_add(E132XS_L42,     "L42", m_core->local_regs[42]).mask(0xffffffff);
	state_add(E132XS_L43,     "L43", m_core->local_regs[43]).mask(0xffffffff);
	state_add(E132XS_L44,     "L44", m_core->local_regs[44]).mask(0xffffffff);
	state_add(E132XS_L45,     "L45", m_core->local_regs[45]).mask(0xffffffff);
	state_add(E132XS_L46,     "L46", m_core->local_regs[46]).mask(0xffffffff);
	state_add(E132XS_L47,     "L47", m_core->local_regs[47]).mask(0xffffffff);
	state_add(E132XS_L48,     "L48", m_core->local_regs[48]).mask(0xffffffff);
	state_add(E132XS_L49,     "L49", m_core->local_regs[49]).mask(0xffffffff);
	state_add(E132XS_L50,     "L50", m_core->local_regs[50]).mask(0xffffffff);
	state_add(E132XS_L51,     "L51", m_core->local_regs[51]).mask(0xffffffff);
	state_add(E132XS_L52,     "L52", m_core->local_regs[52]).mask(0xffffffff);
	state_add(E132XS_L53,     "L53", m_core->local_regs[53]).mask(0xffffffff);
	state_add(E132XS_L54,     "L54", m_core->local_regs[54]).mask(0xffffffff);
	state_add(E132XS_L55,     "L55", m_core->local_regs[55]).mask(0xffffffff);
	state_add(E132XS_L56,     "L56", m_core->local_regs[56]).mask(0xffffffff);
	state_add(E132XS_L57,     "L57", m_core->local_regs[57]).mask(0xffffffff);
	state_add(E132XS_L58,     "L58", m_core->local_regs[58]).mask(0xffffffff);
	state_add(E132XS_L59,     "L59", m_core->local_regs[59]).mask(0xffffffff);
	state_add(E132XS_L60,     "L60", m_core->local_regs[60]).mask(0xffffffff);
	state_add(E132XS_L61,     "L61", m_core->local_regs[61]).mask(0xffffffff);
	state_add(E132XS_L62,     "L62", m_core->local_regs[62]).mask(0xffffffff);
	state_add(E132XS_L63,     "L63", m_core->local_regs[63]).mask(0xffffffff);

	save_item(NAME(m_core->global_regs));
	save_item(NAME(m_core->local_regs));
	save_item(NAME(m_core->trap_entry));
	save_item(NAME(m_core->delay_pc));
	save_item(NAME(m_instruction_length));
	save_item(NAME(m_core->intblock));
	save_item(NAME(m_core->delay_slot));
	save_item(NAME(m_core->delay_slot_taken));
	save_item(NAME(m_core->tr_clocks_per_tick));
	save_item(NAME(m_core->tr_base_value));
	save_item(NAME(m_core->tr_base_cycles));
	save_item(NAME(m_core->timer_int_pending));
	save_item(NAME(m_core->clck_scale));
	save_item(NAME(m_core->clock_scale_mask));
	save_item(NAME(m_core->clock_cycles_1));
	save_item(NAME(m_core->clock_cycles_2));
	save_item(NAME(m_core->clock_cycles_3));
	save_item(NAME(m_core->clock_cycles_4));
	save_item(NAME(m_core->clock_cycles_6));
	save_item(NAME(m_core->clock_cycles_36));

	// set our instruction counter
	set_icountptr(m_core->icount);
}

void e116t_device::device_start()
{
	init(0);
}

void e116xt_device::device_start()
{
	init(3);
}

void e116xs_device::device_start()
{
	init(7);
}

void e116xsr_device::device_start()
{
	init(7);
}

void gms30c2116_device::device_start()
{
	init(0);
}

void gms30c2216_device::device_start()
{
	init(0);
}

void e132n_device::device_start()
{
	init(0);
}

void e132t_device::device_start()
{
	init(0);
}

void e132xn_device::device_start()
{
	init(3);
}

void e132xt_device::device_start()
{
	init(3);
}

void e132xs_device::device_start()
{
	init(7);
}

void e132xsr_device::device_start()
{
	init(7);
}

void gms30c2132_device::device_start()
{
	init(0);
}

void gms30c2232_device::device_start()
{
	init(0);
}

void hyperstone_device::device_reset()
{
	//TODO: Add different reset initializations for BCR, MCR, FCR, TPR

	m_core->tr_clocks_per_tick = 2;

	m_core->trap_entry = s_trap_entries[E132XS_ENTRY_MEM3]; // default entry point @ MEM3

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
	SET_ILC(1<<19);

	set_local_register(0, (PC & 0xfffffffe) | GET_S);
	set_local_register(1, SR);

	m_core->icount -= m_core->clock_cycles_2;
}

void hyperstone_device::device_stop()
{
	if (m_drcfe != nullptr)
	{
		m_drcfe = nullptr;
	}
	if (m_drcuml != nullptr)
	{
		m_drcuml = nullptr;
	}
#if E132XS_LOG_DRC_REGS || E132XS_LOG_INTERPRETER_REGS
	fclose(m_trace_log);
#endif
#if E132XS_COUNT_INSTRUCTIONS
	uint32_t indices[256];
	for (uint32_t i = 0; i < 256; i++)
		indices[i] = i;
	for (uint32_t i = 0; i < 256; i++)
	{
		for (uint32_t j = 0; j < 256; j++)
		{
			if (m_op_counts[j] < m_op_counts[i])
			{
				uint32_t temp = m_op_counts[i];
				m_op_counts[i] = m_op_counts[j];
				m_op_counts[j] = temp;

				temp = indices[i];
				indices[i] = indices[j];
				indices[j] = temp;
			}
		}
	}
	for (uint32_t i = 0; i < 256; i++)
	{
		if (m_op_counts[i] != 0)
		{
			printf("%02x: %d\n", (uint8_t)indices[i], m_op_counts[i]);
		}
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
				m_core->global_regs[1] & 0x00040 ? '?':'.',
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
//  disassemble - call the disassembly
//  helper function
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> hyperstone_device::create_disassembler()
{
	return std::make_unique<hyperstone_disassembler>(this);
}

u8 hyperstone_device::get_fp() const
{
	return GET_FP;
}

bool hyperstone_device::get_h() const
{
	return GET_H;
}

/* Opcodes */

void hyperstone_device::hyperstone_trap()
{
	m_core->icount -= m_core->clock_cycles_1;

	static const uint32_t conditions[16] = {
		0, 0, 0, 0, N_MASK | Z_MASK, N_MASK | Z_MASK, N_MASK, N_MASK, C_MASK | Z_MASK, C_MASK | Z_MASK, C_MASK, C_MASK, Z_MASK, Z_MASK, V_MASK, 0
	};
	static const bool trap_if_set[16] = {
		false, false, false, false, true, false, true, false, true, false, true, false, true, false, true, false
	};

	check_delay_PC();

	const uint8_t trapno = (m_op & 0xfc) >> 2;
	const uint32_t addr = get_trap_addr(trapno);
	const uint8_t code = ((m_op & 0x300) >> 6) | (m_op & 0x03);

	if (trap_if_set[code])
	{
		if (SR & conditions[code])
			execute_trap(addr);
	}
	else
	{
		if (!(SR & conditions[code]))
			execute_trap(addr);
	}
}


#include "e132xsop.hxx"

//**************************************************************************
//  CORE EXECUTION LOOP
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_min_cycles() const noexcept
{
	return 1;
}


//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t hyperstone_device::execute_max_cycles() const noexcept
{
	return 36;
}


void hyperstone_device::execute_set_input(int inputnum, int state)
{
	if (state)
		ISR |= 1 << inputnum;
	else
		ISR &= ~(1 << inputnum);
}

void hyperstone_device::hyperstone_reserved()
{
	LOG("Executed Reserved opcode. PC = %08X OP = %04X\n", PC, OP);
}

void hyperstone_device::hyperstone_do()
{
	fatalerror("Executed hyperstone_do instruction. PC = %08X\n", PC-4);
}

uint32_t hyperstone_device::imm_length(uint16_t op)
{
	switch (op & 0x0f)
	{
		case 0:
		default:
			return 1;
		case 1:
			return 3;
		case 2:
		case 3:
			return 2;
	}
}

int32_t hyperstone_device::get_instruction_length(uint16_t op)
{
	switch (op >> 8)
	{
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97:
	case 0x98: case 0x99: case 0x9a: case 0x9b: case 0x9c: case 0x9d: case 0x9e: case 0x9f:
	case 0xee: case 0xef:
		m_instruction_length = ((m_pr16(PC+2) & 0x8000) ? 3 : 2) << ILC_SHIFT;
		break;
	case 0x61: case 0x63: case 0x65: case 0x67: case 0x69: case 0x6b: case 0x6d: case 0x6f:
	case 0x71: case 0x73: case 0x75: case 0x77: case 0x79: case 0x7b: case 0x7d: case 0x7f:
		m_instruction_length = imm_length(op) << ILC_SHIFT;
		break;
	case 0xb0: case 0xb1: case 0xb2: case 0xb3: case 0xb4: case 0xb5: case 0xb6: case 0xb7:
	case 0xb8: case 0xb9: case 0xba: case 0xbb: case 0xbc:
		m_instruction_length = ((op & 0x80) ? 2 : 1) << ILC_SHIFT;
		break;
	case 0xce:
		m_instruction_length = 2 << ILC_SHIFT;
		break;
	default:
		m_instruction_length = 1 << ILC_SHIFT;
		break;
	}
	m_instruction_length_valid = true;
	return m_instruction_length;
}

//-------------------------------------------------
//  execute_run - execute a timeslice's worth of
//  opcodes
//-------------------------------------------------

void hyperstone_device::execute_run()
{
	if (m_enable_drc)
	{
		execute_run_drc();
		return;
	}

	if (m_core->intblock < 0)
		m_core->intblock = 0;

	if (!m_instruction_length_valid)
		SET_ILC(get_instruction_length(m_pr16(PC)));

	if (m_core->timer_int_pending)
		check_interrupts<IS_TIMER>();
	else
		check_interrupts<NO_TIMER>();

	while (m_core->icount > 0)
	{
#if E132XS_LOG_INTERPRETER_REGS
		dump_registers();
#endif

		debugger_instruction_hook(PC);

		OP = m_pr16(PC);
		PC += 2;

		m_instruction_length = 1 << ILC_SHIFT;

#if E132XS_COUNT_INSTRUCTIONS
		m_op_counts[m_op >> 8]++;
#endif
		switch (m_op >> 8)
		{
			case 0x00: hyperstone_chk<GLOBAL, GLOBAL>(); break;
			case 0x01: hyperstone_chk<GLOBAL, LOCAL>(); break;
			case 0x02: hyperstone_chk<LOCAL, GLOBAL>(); break;
			case 0x03: hyperstone_chk<LOCAL, LOCAL>(); break;
			case 0x04: hyperstone_movd<GLOBAL, GLOBAL>(); break;
			case 0x05: hyperstone_movd<GLOBAL, LOCAL>(); break;
			case 0x06: hyperstone_movd<LOCAL, GLOBAL>(); break;
			case 0x07: hyperstone_movd<LOCAL, LOCAL>(); break;
			case 0x08: hyperstone_divsu<GLOBAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0x09: hyperstone_divsu<GLOBAL, LOCAL, IS_UNSIGNED>(); break;
			case 0x0a: hyperstone_divsu<LOCAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0x0b: hyperstone_divsu<LOCAL, LOCAL, IS_UNSIGNED>(); break;
			case 0x0c: hyperstone_divsu<GLOBAL, GLOBAL, IS_SIGNED>(); break;
			case 0x0d: hyperstone_divsu<GLOBAL, LOCAL, IS_SIGNED>(); break;
			case 0x0e: hyperstone_divsu<LOCAL, GLOBAL, IS_SIGNED>(); break;
			case 0x0f: hyperstone_divsu<LOCAL, LOCAL, IS_SIGNED>(); break;
			case 0x10: hyperstone_xm<GLOBAL, GLOBAL>(); break;
			case 0x11: hyperstone_xm<GLOBAL, LOCAL>(); break;
			case 0x12: hyperstone_xm<LOCAL, GLOBAL>(); break;
			case 0x13: hyperstone_xm<LOCAL, LOCAL>(); break;
			case 0x14: hyperstone_mask<GLOBAL, GLOBAL>(); break;
			case 0x15: hyperstone_mask<GLOBAL, LOCAL>(); break;
			case 0x16: hyperstone_mask<LOCAL, GLOBAL>(); break;
			case 0x17: hyperstone_mask<LOCAL, LOCAL>(); break;
			case 0x18: hyperstone_sum<GLOBAL, GLOBAL>(); break;
			case 0x19: hyperstone_sum<GLOBAL, LOCAL>(); break;
			case 0x1a: hyperstone_sum<LOCAL, GLOBAL>(); break;
			case 0x1b: hyperstone_sum<LOCAL, LOCAL>(); break;
			case 0x1c: hyperstone_sums<GLOBAL, GLOBAL>(); break;
			case 0x1d: hyperstone_sums<GLOBAL, LOCAL>(); break;
			case 0x1e: hyperstone_sums<LOCAL, GLOBAL>(); break;
			case 0x1f: hyperstone_sums<LOCAL, LOCAL>(); break;
			case 0x20: hyperstone_cmp<GLOBAL, GLOBAL>(); break;
			case 0x21: hyperstone_cmp<GLOBAL, LOCAL>(); break;
			case 0x22: hyperstone_cmp<LOCAL, GLOBAL>(); break;
			case 0x23: hyperstone_cmp<LOCAL, LOCAL>(); break;
			case 0x24: hyperstone_mov<GLOBAL, GLOBAL>(); break;
			case 0x25: hyperstone_mov<GLOBAL, LOCAL>(); break;
			case 0x26: hyperstone_mov<LOCAL, GLOBAL>(); break;
			case 0x27: hyperstone_mov<LOCAL, LOCAL>(); break;
			case 0x28: hyperstone_add<GLOBAL, GLOBAL>(); break;
			case 0x29: hyperstone_add<GLOBAL, LOCAL>(); break;
			case 0x2a: hyperstone_add<LOCAL, GLOBAL>(); break;
			case 0x2b: hyperstone_add<LOCAL, LOCAL>(); break;
			case 0x2c: hyperstone_adds<GLOBAL, GLOBAL>(); break;
			case 0x2d: hyperstone_adds<GLOBAL, LOCAL>(); break;
			case 0x2e: hyperstone_adds<LOCAL, GLOBAL>(); break;
			case 0x2f: hyperstone_adds<LOCAL, LOCAL>(); break;
			case 0x30: hyperstone_cmpb<GLOBAL, GLOBAL>(); break;
			case 0x31: hyperstone_cmpb<GLOBAL, LOCAL>(); break;
			case 0x32: hyperstone_cmpb<LOCAL, GLOBAL>(); break;
			case 0x33: hyperstone_cmpb<LOCAL, LOCAL>(); break;
			case 0x34: hyperstone_andn<GLOBAL, GLOBAL>(); break;
			case 0x35: hyperstone_andn<GLOBAL, LOCAL>(); break;
			case 0x36: hyperstone_andn<LOCAL, GLOBAL>(); break;
			case 0x37: hyperstone_andn<LOCAL, LOCAL>(); break;
			case 0x38: hyperstone_or<GLOBAL, GLOBAL>(); break;
			case 0x39: hyperstone_or<GLOBAL, LOCAL>(); break;
			case 0x3a: hyperstone_or<LOCAL, GLOBAL>(); break;
			case 0x3b: hyperstone_or<LOCAL, LOCAL>(); break;
			case 0x3c: hyperstone_xor<GLOBAL, GLOBAL>(); break;
			case 0x3d: hyperstone_xor<GLOBAL, LOCAL>(); break;
			case 0x3e: hyperstone_xor<LOCAL, GLOBAL>(); break;
			case 0x3f: hyperstone_xor<LOCAL, LOCAL>(); break;
			case 0x40: hyperstone_subc<GLOBAL, GLOBAL>(); break;
			case 0x41: hyperstone_subc<GLOBAL, LOCAL>(); break;
			case 0x42: hyperstone_subc<LOCAL, GLOBAL>(); break;
			case 0x43: hyperstone_subc<LOCAL, LOCAL>(); break;
			case 0x44: hyperstone_not<GLOBAL, GLOBAL>(); break;
			case 0x45: hyperstone_not<GLOBAL, LOCAL>(); break;
			case 0x46: hyperstone_not<LOCAL, GLOBAL>(); break;
			case 0x47: hyperstone_not<LOCAL, LOCAL>(); break;
			case 0x48: hyperstone_sub<GLOBAL, GLOBAL>(); break;
			case 0x49: hyperstone_sub<GLOBAL, LOCAL>(); break;
			case 0x4a: hyperstone_sub<LOCAL, GLOBAL>(); break;
			case 0x4b: hyperstone_sub<LOCAL, LOCAL>(); break;
			case 0x4c: hyperstone_subs<GLOBAL, GLOBAL>(); break;
			case 0x4d: hyperstone_subs<GLOBAL, LOCAL>(); break;
			case 0x4e: hyperstone_subs<LOCAL, GLOBAL>(); break;
			case 0x4f: hyperstone_subs<LOCAL, LOCAL>(); break;
			case 0x50: hyperstone_addc<GLOBAL, GLOBAL>(); break;
			case 0x51: hyperstone_addc<GLOBAL, LOCAL>(); break;
			case 0x52: hyperstone_addc<LOCAL, GLOBAL>(); break;
			case 0x53: hyperstone_addc<LOCAL, LOCAL>(); break;
			case 0x54: hyperstone_and<GLOBAL, GLOBAL>(); break;
			case 0x55: hyperstone_and<GLOBAL, LOCAL>(); break;
			case 0x56: hyperstone_and<LOCAL, GLOBAL>(); break;
			case 0x57: hyperstone_and<LOCAL, LOCAL>(); break;
			case 0x58: hyperstone_neg<GLOBAL, GLOBAL>(); break;
			case 0x59: hyperstone_neg<GLOBAL, LOCAL>(); break;
			case 0x5a: hyperstone_neg<LOCAL, GLOBAL>(); break;
			case 0x5b: hyperstone_neg<LOCAL, LOCAL>(); break;
			case 0x5c: hyperstone_negs<GLOBAL, GLOBAL>(); break;
			case 0x5d: hyperstone_negs<GLOBAL, LOCAL>(); break;
			case 0x5e: hyperstone_negs<LOCAL, GLOBAL>(); break;
			case 0x5f: hyperstone_negs<LOCAL, LOCAL>(); break;
			case 0x60: hyperstone_cmpi<GLOBAL, SIMM>(); break;
			case 0x61: hyperstone_cmpi<GLOBAL, LIMM>(); break;
			case 0x62: hyperstone_cmpi<LOCAL, SIMM>(); break;
			case 0x63: hyperstone_cmpi<LOCAL, LIMM>(); break;
			case 0x64: hyperstone_movi<GLOBAL, SIMM>(); break;
			case 0x65: hyperstone_movi<GLOBAL, LIMM>(); break;
			case 0x66: hyperstone_movi<LOCAL, SIMM>(); break;
			case 0x67: hyperstone_movi<LOCAL, LIMM>(); break;
			case 0x68: hyperstone_addi<GLOBAL, SIMM>(); break;
			case 0x69: hyperstone_addi<GLOBAL, LIMM>(); break;
			case 0x6a: hyperstone_addi<LOCAL, SIMM>(); break;
			case 0x6b: hyperstone_addi<LOCAL, LIMM>(); break;
			case 0x6c: hyperstone_addsi<GLOBAL, SIMM>(); break;
			case 0x6d: hyperstone_addsi<GLOBAL, LIMM>(); break;
			case 0x6e: hyperstone_addsi<LOCAL, SIMM>(); break;
			case 0x6f: hyperstone_addsi<LOCAL, LIMM>(); break;
			case 0x70: hyperstone_cmpbi<GLOBAL, SIMM>(); break;
			case 0x71: hyperstone_cmpbi<GLOBAL, LIMM>(); break;
			case 0x72: hyperstone_cmpbi<LOCAL, SIMM>(); break;
			case 0x73: hyperstone_cmpbi<LOCAL, LIMM>(); break;
			case 0x74: hyperstone_andni<GLOBAL, SIMM>(); break;
			case 0x75: hyperstone_andni<GLOBAL, LIMM>(); break;
			case 0x76: hyperstone_andni<LOCAL, SIMM>(); break;
			case 0x77: hyperstone_andni<LOCAL, LIMM>(); break;
			case 0x78: hyperstone_ori<GLOBAL, SIMM>(); break;
			case 0x79: hyperstone_ori<GLOBAL, LIMM>(); break;
			case 0x7a: hyperstone_ori<LOCAL, SIMM>(); break;
			case 0x7b: hyperstone_ori<LOCAL, LIMM>(); break;
			case 0x7c: hyperstone_xori<GLOBAL, SIMM>(); break;
			case 0x7d: hyperstone_xori<GLOBAL, LIMM>(); break;
			case 0x7e: hyperstone_xori<LOCAL, SIMM>(); break;
			case 0x7f: hyperstone_xori<LOCAL, LIMM>(); break;
			case 0x80: hyperstone_shrdi<N_LO>(); break;
			case 0x81: hyperstone_shrdi<N_HI>(); break;
			case 0x82: hyperstone_shrd(); break;
			case 0x83: hyperstone_shr(); break;
			case 0x84: hyperstone_sardi<N_LO>(); break;
			case 0x85: hyperstone_sardi<N_HI>(); break;
			case 0x86: hyperstone_sard(); break;
			case 0x87: hyperstone_sar(); break;
			case 0x88: hyperstone_shldi<N_LO>(); break;
			case 0x89: hyperstone_shldi<N_HI>(); break;
			case 0x8a: hyperstone_shld(); break;
			case 0x8b: hyperstone_shl(); break;
			case 0x8c: hyperstone_reserved(); break;
			case 0x8d: hyperstone_reserved(); break;
			case 0x8e: hyperstone_testlz(); break;
			case 0x8f: hyperstone_rol(); break;
			case 0x90: hyperstone_ldxx1<GLOBAL, GLOBAL>(); break;
			case 0x91: hyperstone_ldxx1<GLOBAL, LOCAL>(); break;
			case 0x92: hyperstone_ldxx1<LOCAL, GLOBAL>(); break;
			case 0x93: hyperstone_ldxx1<LOCAL, LOCAL>(); break;
			case 0x94: hyperstone_ldxx2<GLOBAL, GLOBAL>(); break;
			case 0x95: hyperstone_ldxx2<GLOBAL, LOCAL>(); break;
			case 0x96: hyperstone_ldxx2<LOCAL, GLOBAL>(); break;
			case 0x97: hyperstone_ldxx2<LOCAL, LOCAL>(); break;
			case 0x98: hyperstone_stxx1<GLOBAL, GLOBAL>(); break;
			case 0x99: hyperstone_stxx1<GLOBAL, LOCAL>(); break;
			case 0x9a: hyperstone_stxx1<LOCAL, GLOBAL>(); break;
			case 0x9b: hyperstone_stxx1<LOCAL, LOCAL>(); break;
			case 0x9c: hyperstone_stxx2<GLOBAL, GLOBAL>(); break;
			case 0x9d: hyperstone_stxx2<GLOBAL, LOCAL>(); break;
			case 0x9e: hyperstone_stxx2<LOCAL, GLOBAL>(); break;
			case 0x9f: hyperstone_stxx2<LOCAL, LOCAL>(); break;
			case 0xa0: hyperstone_shri<N_LO, GLOBAL>(); break;
			case 0xa1: hyperstone_shri<N_HI, GLOBAL>(); break;
			case 0xa2: hyperstone_shri<N_LO, LOCAL>(); break;
			case 0xa3: hyperstone_shri<N_HI, LOCAL>(); break;
			case 0xa4: hyperstone_sari<N_LO, GLOBAL>(); break;
			case 0xa5: hyperstone_sari<N_HI, GLOBAL>(); break;
			case 0xa6: hyperstone_sari<N_LO, LOCAL>(); break;
			case 0xa7: hyperstone_sari<N_HI, LOCAL>(); break;
			case 0xa8: hyperstone_shli<N_LO, GLOBAL>(); break;
			case 0xa9: hyperstone_shli<N_HI, GLOBAL>(); break;
			case 0xaa: hyperstone_shli<N_LO, LOCAL>(); break;
			case 0xab: hyperstone_shli<N_HI, LOCAL>(); break;
			case 0xac: hyperstone_reserved(); break;
			case 0xad: hyperstone_reserved(); break;
			case 0xae: hyperstone_reserved(); break;
			case 0xaf: hyperstone_reserved(); break;
			case 0xb0: hyperstone_mulsu<GLOBAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0xb1: hyperstone_mulsu<GLOBAL, LOCAL, IS_UNSIGNED>(); break;
			case 0xb2: hyperstone_mulsu<LOCAL, GLOBAL, IS_UNSIGNED>(); break;
			case 0xb3: hyperstone_mulsu<LOCAL, LOCAL, IS_UNSIGNED>(); break;
			case 0xb4: hyperstone_mulsu<GLOBAL, GLOBAL, IS_SIGNED>(); break;
			case 0xb5: hyperstone_mulsu<GLOBAL, LOCAL, IS_SIGNED>(); break;
			case 0xb6: hyperstone_mulsu<LOCAL, GLOBAL, IS_SIGNED>(); break;
			case 0xb7: hyperstone_mulsu<LOCAL, LOCAL, IS_SIGNED>(); break;
			case 0xb8: hyperstone_set<N_LO, GLOBAL>(); break;
			case 0xb9: hyperstone_set<N_HI, GLOBAL>(); break;
			case 0xba: hyperstone_set<N_LO, LOCAL>(); break;
			case 0xbb: hyperstone_set<N_HI, LOCAL>(); break;
			case 0xbc: hyperstone_mul<GLOBAL, GLOBAL>(); break;
			case 0xbd: hyperstone_mul<GLOBAL, LOCAL>(); break;
			case 0xbe: hyperstone_mul<LOCAL, GLOBAL>(); break;
			case 0xbf: hyperstone_mul<LOCAL, LOCAL>(); break;
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
			case 0xce: hyperstone_extend(); break;
			case 0xcf: hyperstone_do(); break;
			case 0xd0: hyperstone_ldwr<GLOBAL>(); break;
			case 0xd1: hyperstone_ldwr<LOCAL>(); break;
			case 0xd2: hyperstone_lddr<GLOBAL>(); break;
			case 0xd3: hyperstone_lddr<LOCAL>(); break;
			case 0xd4: hyperstone_ldwp<GLOBAL>(); break;
			case 0xd5: hyperstone_ldwp<LOCAL>(); break;
			case 0xd6: hyperstone_lddp<GLOBAL>(); break;
			case 0xd7: hyperstone_lddp<LOCAL>(); break;
			case 0xd8: hyperstone_stwr<GLOBAL>(); break;
			case 0xd9: hyperstone_stwr<LOCAL>(); break;
			case 0xda: hyperstone_stdr<GLOBAL>(); break;
			case 0xdb: hyperstone_stdr<LOCAL>(); break;
			case 0xdc: hyperstone_stwp<GLOBAL>(); break;
			case 0xdd: hyperstone_stwp<LOCAL>(); break;
			case 0xde: hyperstone_stdp<GLOBAL>(); break;
			case 0xdf: hyperstone_stdp<LOCAL>(); break;
			case 0xe0: hyperstone_db<COND_V,  IS_SET>(); break;
			case 0xe1: hyperstone_db<COND_V,  IS_CLEAR>(); break;
			case 0xe2: hyperstone_db<COND_Z,  IS_SET>(); break;
			case 0xe3: hyperstone_db<COND_Z,  IS_CLEAR>(); break;
			case 0xe4: hyperstone_db<COND_C,  IS_SET>(); break;
			case 0xe5: hyperstone_db<COND_C,  IS_CLEAR>(); break;
			case 0xe6: hyperstone_db<COND_CZ, IS_SET>(); break;
			case 0xe7: hyperstone_db<COND_CZ, IS_CLEAR>(); break;
			case 0xe8: hyperstone_db<COND_N,  IS_SET>(); break;
			case 0xe9: hyperstone_db<COND_N,  IS_CLEAR>(); break;
			case 0xea: hyperstone_db<COND_NZ, IS_SET>(); break;
			case 0xeb: hyperstone_db<COND_NZ, IS_CLEAR>(); break;
			case 0xec: hyperstone_dbr(); break;
			case 0xed: hyperstone_frame(); break;
			case 0xee: hyperstone_call<GLOBAL>(); break;
			case 0xef: hyperstone_call<LOCAL>(); break;
			case 0xf0: hyperstone_b<COND_V,  IS_SET>(); break;
			case 0xf1: hyperstone_b<COND_V,  IS_CLEAR>(); break;
			case 0xf2: hyperstone_b<COND_Z,  IS_SET>(); break;
			case 0xf3: hyperstone_b<COND_Z,  IS_CLEAR>(); break;
			case 0xf4: hyperstone_b<COND_C,  IS_SET>(); break;
			case 0xf5: hyperstone_b<COND_C,  IS_CLEAR>(); break;
			case 0xf6: hyperstone_b<COND_CZ, IS_SET>(); break;
			case 0xf7: hyperstone_b<COND_CZ, IS_CLEAR>(); break;
			case 0xf8: hyperstone_b<COND_N,  IS_SET>(); break;
			case 0xf9: hyperstone_b<COND_N,  IS_CLEAR>(); break;
			case 0xfa: hyperstone_b<COND_NZ, IS_SET>(); break;
			case 0xfb: hyperstone_b<COND_NZ, IS_CLEAR>(); break;
			case 0xfc: hyperstone_br(); break;
			case 0xfd: hyperstone_trap(); break;
			case 0xfe: hyperstone_trap(); break;
			case 0xff: hyperstone_trap(); break;
		}

		SET_ILC(m_instruction_length);

		if (GET_T && GET_P && !m_core->delay_slot) /* Not in a Delayed Branch instructions */
		{
			uint32_t addr = get_trap_addr(TRAPNO_TRACE_EXCEPTION);
			execute_exception(addr);
		}

		if (--m_core->intblock <= 0)
		{
			m_core->intblock = 0;
			if (m_core->timer_int_pending)
				check_interrupts<IS_TIMER>();
			else
				check_interrupts<NO_TIMER>();
		}
	}
}

DEFINE_DEVICE_TYPE(E116T,      e116t_device,      "e116t",      "hyperstone E1-16T")
DEFINE_DEVICE_TYPE(E116XT,     e116xt_device,     "e116xt",     "hyperstone E1-16XT")
DEFINE_DEVICE_TYPE(E116XS,     e116xs_device,     "e116xs",     "hyperstone E1-16XS")
DEFINE_DEVICE_TYPE(E116XSR,    e116xsr_device,    "e116xsr",    "hyperstone E1-16XSR")
DEFINE_DEVICE_TYPE(E132N,      e132n_device,      "e132n",      "hyperstone E1-32N")
DEFINE_DEVICE_TYPE(E132T,      e132t_device,      "e132t",      "hyperstone E1-32T")
DEFINE_DEVICE_TYPE(E132XN,     e132xn_device,     "e132xn",     "hyperstone E1-32XN")
DEFINE_DEVICE_TYPE(E132XT,     e132xt_device,     "e132xt",     "hyperstone E1-32XT")
DEFINE_DEVICE_TYPE(E132XS,     e132xs_device,     "e132xs",     "hyperstone E1-32XS")
DEFINE_DEVICE_TYPE(E132XSR,    e132xsr_device,    "e132xsr",    "hyperstone E1-32XSR")
DEFINE_DEVICE_TYPE(GMS30C2116, gms30c2116_device, "gms30c2116", "Hynix GMS30C2116")
DEFINE_DEVICE_TYPE(GMS30C2132, gms30c2132_device, "gms30c2132", "Hynix GMS30C2132")
DEFINE_DEVICE_TYPE(GMS30C2216, gms30c2216_device, "gms30c2216", "Hynix GMS30C2216")
DEFINE_DEVICE_TYPE(GMS30C2232, gms30c2232_device, "gms30c2232", "Hynix GMS30C2232")
