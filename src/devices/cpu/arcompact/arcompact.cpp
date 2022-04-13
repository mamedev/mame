// license:BSD-3-Clause
// copyright-holders:David Haywood
/*********************************\

 ARCompact Core

 The following procesors use the ARCompact instruction set

  - ARCtangent-A5
  - ARC 600
  - ARC 700

 (this is a skeleton core)

 ARCompact is a 32-bit CPU that freely mixes 32-bit and 16-bit instructions
 various user customizations could be made as with the ARC A4 based processors
 these include custom instructions and registers.

\*********************************/

#include "emu.h"
#include "arcompact.h"
#include "arcompactdasm.h"


DEFINE_DEVICE_TYPE(ARCA5, arcompact_device, "arc_a5", "Argonaut ARCtangent A5")


uint32_t arcompact_device::arcompact_auxreg002_LPSTART_r() { return m_LP_START&0xfffffffe; }
void arcompact_device::arcompact_auxreg002_LPSTART_w(uint32_t data) { m_LP_START = data&0xfffffffe; }
uint32_t arcompact_device::arcompact_auxreg003_LPEND_r() { return m_LP_END&0xfffffffe; }
void arcompact_device::arcompact_auxreg003_LPEND_w(uint32_t data) { m_LP_END = data&0xfffffffe; }

uint32_t arcompact_device::arcompact_auxreg00a_STATUS32_r() { return 0xffffdead; /*m_status32;*/ }

uint32_t arcompact_device::arcompact_auxreg025_INTVECTORBASE_r() { return m_INTVECTORBASE&0xfffffc00; }
void arcompact_device::arcompact_auxreg025_INTVECTORBASE_w(uint32_t data) { m_INTVECTORBASE = data&0xfffffc00; }




void arcompact_device::arcompact_auxreg_map(address_map &map)
{
	map(0x000000002, 0x000000002).rw(FUNC(arcompact_device::arcompact_auxreg002_LPSTART_r), FUNC(arcompact_device::arcompact_auxreg002_LPSTART_w));
	map(0x000000003, 0x000000003).rw(FUNC(arcompact_device::arcompact_auxreg003_LPEND_r), FUNC(arcompact_device::arcompact_auxreg003_LPEND_w));
	map(0x000000009, 0x000000009).r(FUNC(arcompact_device::arcompact_auxreg00a_STATUS32_r)); // r/o
	map(0x000000025, 0x000000025).rw(FUNC(arcompact_device::arcompact_auxreg025_INTVECTORBASE_r), FUNC(arcompact_device::arcompact_auxreg025_INTVECTORBASE_w));
}

#define AUX_SPACE_ADDRESS_WIDTH 32  // IO space is 32 bits of dwords

arcompact_device::arcompact_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: cpu_device(mconfig, ARCA5, tag, owner, clock)
	, m_program_config("program", ENDIANNESS_LITTLE, 32, 32, 0) // some docs describe these as 'middle endian'?!
	, m_io_config( "io", ENDIANNESS_LITTLE, 32, AUX_SPACE_ADDRESS_WIDTH, -2, address_map_constructor(FUNC(arcompact_device::arcompact_auxreg_map), this))
{
}

device_memory_interface::space_config_vector arcompact_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config),
		std::make_pair(AS_IO,      &m_io_config)
	};
}

std::unique_ptr<util::disasm_interface> arcompact_device::create_disassembler()
{
	return std::make_unique<arcompact_disassembler>();
}


/*****************************************************************************/

/*****************************************************************************/

void arcompact_device::unimplemented_opcode(uint16_t op)
{
	fatalerror("ARCOMPACT: unknown opcode %04x at %04x\n", op, m_pc << 2);
}

/*****************************************************************************/


/*****************************************************************************/

void arcompact_device::device_start()
{
	m_pc = 0;

	m_debugger_temp = 0;

	m_program = &space(AS_PROGRAM);
	m_io = &space(AS_IO);

	state_add( ARCOMPACT_PC, "PC", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add( ARCOMPACT_STATUS32, "STATUS32", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add( ARCOMPACT_LP_START, "LP_START", m_debugger_temp).callimport().callexport().formatstr("%08X");
	state_add( ARCOMPACT_LP_END, "LP_END", m_debugger_temp).callimport().callexport().formatstr("%08X");

	state_add(STATE_GENPCBASE, "CURPC", m_debugger_temp).callimport().callexport().noshow();

	for (int i = 0x100; i < 0x140; i++)
	{
		state_add(i, arcompact_disassembler::regnames[i-0x100], m_debugger_temp).callimport().callexport().formatstr("%08X");
	}


	set_icountptr(m_icount);
}


//-------------------------------------------------
//  state_export - export state from the device,
//  to a known location where it can be read
//-------------------------------------------------

void arcompact_device::state_export(const device_state_entry &entry)
{
	int index = entry.index();

	switch (index)
	{
		case ARCOMPACT_PC:
		case STATE_GENPCBASE:
			m_debugger_temp = m_pc;
			break;

		case ARCOMPACT_STATUS32:
			m_debugger_temp = m_status32;
			break;
		case ARCOMPACT_LP_START:
			m_debugger_temp = m_LP_START;
			break;
		case ARCOMPACT_LP_END:
			m_debugger_temp = m_LP_END;
			break;

		default:
			if ((index >= 0x100) && (index < 0x140))
			{
				m_debugger_temp = m_regs[index - 0x100];
			}
			break;

	}
}


//-------------------------------------------------
//  state_import - import state into the device,
//  after it has been set
//-------------------------------------------------

void arcompact_device::state_import(const device_state_entry &entry)
{
	int index = entry.index();

	switch (index)
	{
		case ARCOMPACT_PC:
		case STATE_GENPCBASE:
			m_pc = (m_debugger_temp & 0xfffffffe);
			break;

		case ARCOMPACT_STATUS32:
			m_status32 = m_debugger_temp;
			break;
		case ARCOMPACT_LP_START:
			m_LP_START = m_debugger_temp;
			break;
		case ARCOMPACT_LP_END:
			m_LP_END = m_debugger_temp;
			break;

		default:
			if ((index >= 0x100) && (index < 0x140))
			{
				m_regs[index - 0x100] = m_debugger_temp;
			}
			break;
	}
}

void arcompact_device::device_reset()
{
	m_pc = 0x00000000;

	m_delayactive = 0;
	m_delayjump = 0x00000000;

	for (auto & elem : m_regs)
		elem = 0;

	m_status32 = 0;
	m_LP_START = 0;
	m_LP_END = 0;
	m_INTVECTORBASE = 0;

}


/*****************************************************************************/


void arcompact_device::execute_set_input(int irqline, int state)
{
}
