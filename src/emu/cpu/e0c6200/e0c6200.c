// license:BSD-3-Clause
// copyright-holders:hap
/*

  Seiko Epson E0C6200 CPU core and E0C62 MCU family

  References:
  - 1998 MF297-06a E0C6200/E0C6200A Core CPU Manual
  - 1998 MF1049-01a E0C6S46 Technical Manual

  TODO:
  - niks

*/

#include "e0c6200.h"
#include "debugger.h"

#include "e0c6200op.inc"


const device_type EPSON_E0C6S46 = &device_creator<e0c6s46_device>;


// internal memory maps
static ADDRESS_MAP_START(program_1k, AS_PROGRAM, 16, e0c6200_cpu_device)
	AM_RANGE(0x0000, 0x03ff) AM_ROM
ADDRESS_MAP_END


static ADDRESS_MAP_START(data_64x4, AS_DATA, 8, e0c6200_cpu_device)
	AM_RANGE(0x00, 0x3f) AM_RAM
ADDRESS_MAP_END


// device definitions
e0c6s46_device::e0c6s46_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: e0c6200_cpu_device(mconfig, EPSON_E0C6S46, "E0C6S46", tag, owner, clock, 10, ADDRESS_MAP_NAME(program_1k), 6, ADDRESS_MAP_NAME(data_64x4), "e0c6s46", __FILE__)
{ }


// disasm
void e0c6200_cpu_device::state_string_export(const device_state_entry &entry, std::string &str)
{
	switch (entry.index())
	{
		case STATE_GENFLAGS:
			break;

		default: break;
	}
}

offs_t e0c6200_cpu_device::disasm_disassemble(char *buffer, offs_t pc, const UINT8 *oprom, const UINT8 *opram, UINT32 options)
{
	extern CPU_DISASSEMBLE(e0c6200);
	return CPU_DISASSEMBLE_NAME(e0c6200)(this, buffer, pc, oprom, opram, options);
}



//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void e0c6200_cpu_device::device_start()
{
	m_program = &space(AS_PROGRAM);
	m_data = &space(AS_DATA);
	m_prgmask = (1 << m_prgwidth) - 1;
	m_datamask = (1 << m_datawidth) - 1;

	// zerofill

	// register for savestates

	// register state for debugger

	m_icountptr = &m_icount;
}



//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void e0c6200_cpu_device::device_reset()
{
}



//-------------------------------------------------
//  execute
//-------------------------------------------------

void e0c6200_cpu_device::execute_run()
{
	while (m_icount > 0)
	{
		m_icount--;
	}
}
