// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#include "emu.h"
#include "ie15.h"
#include "ie15dasm.h"


//**************************************************************************
//  MACROS
//**************************************************************************

#define SKIP_OP(x) do { \
			x = rop() & 0xf0; \
			if (x == 0x10 || x == 0x20 || x == 0x30)  \
				m_PC.w.l = m_PC.w.l + 1; \
			} while(0)

//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DEFINE_DEVICE_TYPE(IE15_CPU, ie15_cpu_device, "ie15_cpu", "ie15 CPU")

//**************************************************************************
//  DEVICE INTERFACE
//**************************************************************************

//-------------------------------------------------
//  ie15_cpu_device - constructor
//-------------------------------------------------
ie15_cpu_device::ie15_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, IE15_CPU, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 8, 14)
	, m_io_config("io", ENDIANNESS_LITTLE, 8, 8), m_A(0), m_CF(0), m_ZF(0), m_RF(0), m_flags(0)
{
	// set our instruction counter
	set_icountptr(m_icount);
}

//-------------------------------------------------
//  device_start - start up the device
//-------------------------------------------------

void ie15_cpu_device::device_start()
{
	// find address spaces
	space(AS_PROGRAM).cache(m_cache);
	space(AS_PROGRAM).specific(m_program);
	space(AS_IO).specific(m_io);

	// save state
	save_item(NAME(m_PC));
	save_item(NAME(m_A));
	save_item(NAME(m_CF));
	save_item(NAME(m_ZF));
	save_item(NAME(m_RF));
	// XXX save registers

	// register our state for the debugger
	state_add(IE15_PC,       "PC",       m_PC.w.l).mask(0x0fff);
	state_add(STATE_GENPC,   "GENPC",    m_PC.w.l).mask(0x0fff).noshow();
	state_add(STATE_GENPCBASE, "CURPC",  m_PC.w.l).mask(0x0fff).noshow();
	state_add(STATE_GENFLAGS,"GENFLAGS", m_flags).mask(0x0f).callimport().callexport().noshow().formatstr("%4s");
	state_add(IE15_A,        "A",        m_A);

	for (int ireg = 0; ireg < 32; ireg++)
		state_add(IE15_R0 + ireg, string_format("R%d", ireg).c_str(), m_REGS[ireg]);
}

//-------------------------------------------------
//  device_reset - reset the device
//-------------------------------------------------

void ie15_cpu_device::device_reset()
{
	m_CF = m_ZF = m_RF = 0;
	m_A = 0;
	m_PC.d = 0;
	memset(m_REGS,0,sizeof(m_REGS));
}

//-------------------------------------------------
//  memory_space_config - return the configuration
//  of the specified address space, or nullptr if
//  the space doesn't exist
//-------------------------------------------------

device_memory_interface::space_config_vector ie15_cpu_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void ie15_cpu_device::state_import(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_CF = (m_flags >> 3) & 1;
			m_ZF = (m_flags >> 2) & 1;
			m_RF = (m_flags >> 1) & 1;
			break;
	}
}

//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void ie15_cpu_device::state_export(const device_state_entry &entry)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			m_flags = (m_CF ? 0x08 : 0x00) |
								(m_ZF ? 0x04 : 0x00) |
								(m_RF ? 0x02 : 0x00);
			break;
	}
}

//-------------------------------------------------
//  state_string_export - export state as a string
//  for the debugger
//-------------------------------------------------

void ie15_cpu_device::state_string_export(const device_state_entry &entry, std::string &str) const
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			str = string_format("%c%c%c",
				m_CF ? 'C':'.',
				m_ZF ? 'Z':'.',
				m_RF ? 'R':'.');
			break;
	}
}

//-------------------------------------------------
//  create_disassembler
//-------------------------------------------------

std::unique_ptr<util::disasm_interface> ie15_cpu_device::create_disassembler()
{
	return std::make_unique<ie15_disassembler>();
}

//**************************************************************************
//  EXECUTION
//**************************************************************************

//-------------------------------------------------
//  execute_min_cycles - return minimum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t ie15_cpu_device::execute_min_cycles() const noexcept
{
	return 1;
}

//-------------------------------------------------
//  execute_max_cycles - return maximum number of
//  cycles it takes for one instruction to execute
//-------------------------------------------------

uint32_t ie15_cpu_device::execute_max_cycles() const noexcept
{
	return 1;
}

//-------------------------------------------------
//  execute_run - execute until our icount expires
//-------------------------------------------------

void ie15_cpu_device::execute_run()
{
	// Removing the hook entirely is considerably faster than calling it for every instruction if the debugger is disabled entirely
	if (debugger_enabled())
	{
		do
		{
			debugger_instruction_hook(m_PC.d);
			execute_one(rop());
		} while (m_icount > 0);
	}
	else
	{
		do
		{
			execute_one(rop());
		} while (m_icount > 0);
	}
}

inline void ie15_cpu_device::illegal(uint8_t opcode)
{
	if (debugger_enabled())
	{
		logerror("IE15 illegal instruction %04X $%02X\n", m_PC.w.l, opcode);
	}
}

// XXX verify that m_ZF and m_CF are set and handled right
// XXX 'ota' apparently writes the ALU buffer register, not accumulator
// XXX what if ldc was at 0x_ff?
inline void ie15_cpu_device::execute_one(int opcode)
{
	uint16_t tmp;

	m_icount -= 1;

	switch (opcode & 0xf0)
	{
		case 0x00:  // add
			tmp = m_A + get_reg_lo(opcode & 15);
			m_A = tmp & 255;
			update_flags(m_A);
			m_CF = BIT(tmp, 8);
			break;
		case 0x10:  // jmp
			m_PC.w.l = get_addr(opcode);
//          m_CF = 0;
			break;
		case 0x20:  // ldc
			set_reg(opcode & 15, arg() | (m_PC.w.l & 0xf00));
			m_PC.w.l = m_PC.w.l + 1;
//          m_CF = 0;
			break;
		case 0x40:  // dsr
			tmp = get_reg_lo(opcode & 15) - 1;
//          m_CF = BIT(tmp, 8);
			tmp &= 255;
			set_reg(opcode & 15, tmp);
			update_flags(tmp);
			if (m_ZF) {
				SKIP_OP(tmp);
			}
			break;
		case 0x30:
			switch (opcode)
			{
				case 0x30:  // lca
					m_A = arg();
					update_flags(m_A);
					m_PC.w.l = m_PC.w.l + 1;
					break;
				case 0x33:  // ral
					tmp = m_A;
					m_A = (m_A << 1) | BIT(tmp,7);
					update_flags(m_A);
					break;
				case 0x35:  // rar
					tmp = m_A;
					m_A = (m_A >> 1) | (BIT(tmp,0) ? 0x80 : 0x00);
					update_flags(m_A);
					break;
				default:
					illegal(opcode);
					break;
			};
			break;
		case 0x50:
			switch (opcode)
			{
				case 0x51:  // inc
				case 0x50:  // isn
				case 0x58:  // ise
					tmp = m_A + 1;
					m_A = tmp & 255;
					update_flags(m_A);
					m_CF = BIT(tmp, 8);
					if (opcode == 0x50 && m_ZF)
						SKIP_OP(tmp);
					if (opcode == 0x58 && !m_ZF)
						SKIP_OP(tmp);
					break;
				case 0x5b:  // dec
				case 0x52:  // dsn
				case 0x5a:  // dse
					tmp = m_A - 1;
					m_A = tmp & 255;
					update_flags(m_A);
					m_CF = BIT(tmp, 8);
					if (opcode == 0x52 && m_ZF)
						SKIP_OP(tmp);
					if (opcode == 0x5a && !m_ZF)
						SKIP_OP(tmp);
					break;
				case 0x5d:  // com
					m_A ^= 255;
					update_flags(m_A);
					break;
				case 0x5f:  // clr
					m_A = 0;
					update_flags(m_A);
					break;
				default:
					illegal(opcode);
					break;
			};
			break;
		case 0x70:  // jmi
//          m_CF = 0;
			m_PC.w.l = get_reg(opcode & 15);
			break;
		case 0x60:  // lla
			// special case -- port 7
			if (opcode == 0x67)
				m_A = 255;
			else
				m_A = m_io.read_byte(opcode & 15);
			update_flags(m_A);
			break;
		case 0xf0:  // ota
			// special case -- ports 016, 017
			if (opcode == 0xfe)
				m_RF = 1;
			else if (opcode == 0xff)
				m_RF = 0;
			else
				m_io.write_byte(opcode & 15, m_A);
//          m_CF = 0;
			break;
		case 0xc0:  // cfl, sfl
			switch (opcode)
			{
				// special case -- accessing control flag 05 resets CF
				case 0xc5:
				case 0xcd:
					m_CF = 0;
					break;
				default:
					m_io.write_byte(020 | (opcode & 7), BIT(opcode, 3));
					break;
			}
			break;
		case 0x80:  // sfc, skp, sfs, nop
			tmp = opcode & 7;
			switch (tmp)
			{
				case 5:
					tmp = BIT(m_A, 7);
					break;
				case 6:
					tmp = m_CF;
					break;
				case 7:
					tmp = 0;
					break;
				default:
					tmp = m_io.read_byte(020 | tmp);
					break;

			}
			if (!BIT(opcode, 3) && !tmp)
				SKIP_OP(tmp);
			if (BIT(opcode, 3) && tmp)
				SKIP_OP(tmp);
			break;
		case 0xb0:  // cs
//          m_CF = 0;
			if (m_A == get_reg_lo(opcode & 15)) {
				m_ZF = 1;
				SKIP_OP(tmp);
			}
			break;
		case 0x90:  // and
			m_A &= get_reg_lo(opcode & 15);
			update_flags(m_A);
			break;
		case 0xa0:  // xor
			m_A ^= get_reg_lo(opcode & 15);
			update_flags(m_A);
			break;
		case 0xd0:  // lda
			m_A = get_reg_lo(opcode & 15);
			update_flags(m_A);
			break;
		case 0xe0:  // sta
			set_reg(opcode & 15, m_A | (m_PC.w.l & 0xf00));
//          m_CF = 0;
			break;
		default:
			illegal(opcode);
			break;
	}
}

/***************************************************************************
    INLINE FUNCTIONS
***************************************************************************/

inline uint8_t ie15_cpu_device::rop()
{
	uint8_t retVal = m_cache.read_byte(m_PC.w.l);
	m_PC.w.l = (m_PC.w.l + 1) & 0x0fff;
	return retVal;
}

inline uint8_t ie15_cpu_device::arg()
{
	uint8_t retVal = m_cache.read_byte(m_PC.w.l);
	return retVal;
}

inline uint8_t ie15_cpu_device::get_reg_lo(uint8_t reg)
{
	uint16_t tmp = m_RF ? m_REGS[16 + reg] : m_REGS[reg];
	return tmp & 255;
}

inline uint16_t ie15_cpu_device::get_reg(uint8_t reg)
{
	return m_RF ? m_REGS[16 + reg] : m_REGS[reg];
}

inline void ie15_cpu_device::set_reg(uint8_t reg, uint16_t val)
{
	(m_RF ? m_REGS[16 + reg] : m_REGS[reg]) = val;

}

inline void ie15_cpu_device::update_flags(uint8_t val)
{
	m_ZF = (val == 0xff) ? 1 : 0;
}

inline uint8_t ie15_cpu_device::do_condition(uint8_t val)
{
	uint8_t v = (val >> 5) & 1;
	uint8_t cond = 0;
	switch((val>> 3) & 0x03) {
		case 0 :
				if (m_CF==v) cond = 1;
				break;
		case 1 :
				if (m_ZF==v) cond = 1;
				break;
	}
	return cond;
}

inline uint16_t ie15_cpu_device::get_addr(uint8_t val)
{
	uint8_t lo = arg();
	return ((val & 0x0f) << 8) + lo + 1;
}
