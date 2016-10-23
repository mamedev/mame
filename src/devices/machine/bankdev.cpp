// license:BSD-3-Clause
// copyright-holders:smf
#include "bankdev.h"

// device type definition
const device_type ADDRESS_MAP_BANK = &device_creator<address_map_bank_device>;

address_map_bank_device::address_map_bank_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, ADDRESS_MAP_BANK, "Address Map Bank", tag, owner, clock, "address_map_bank", __FILE__),
		device_memory_interface(mconfig, *this),
		m_endianness(ENDIANNESS_NATIVE),
		m_databus_width(0),
		m_addrbus_width(32),
		m_stride(1),
		m_program(nullptr),
		m_offset(0)
{
}

DEVICE_ADDRESS_MAP_START(amap8, 8, address_map_bank_device)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read8, write8)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(amap16, 16, address_map_bank_device)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read16, write16)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(amap32, 32, address_map_bank_device)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read32, write32)
ADDRESS_MAP_END

DEVICE_ADDRESS_MAP_START(amap64, 64, address_map_bank_device)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read64, write64)
ADDRESS_MAP_END

void address_map_bank_device::write8(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	m_program->write_byte(m_offset + offset, data);
}

void address_map_bank_device::write16(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	m_program->write_word(m_offset + (offset * 2), data, mem_mask);
}

void address_map_bank_device::write32(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	m_program->write_dword(m_offset + (offset * 4), data, mem_mask);
}

void address_map_bank_device::write64(address_space &space, offs_t offset, uint64_t data, uint64_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	m_program->write_qword(m_offset + (offset * 8), data, mem_mask);
}

uint8_t address_map_bank_device::read8(address_space &space, offs_t offset, uint8_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	return m_program->read_byte(m_offset + offset);
}

uint16_t address_map_bank_device::read16(address_space &space, offs_t offset, uint16_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	return m_program->read_word(m_offset + (offset * 2), mem_mask);
}

uint32_t address_map_bank_device::read32(address_space &space, offs_t offset, uint32_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	return m_program->read_dword(m_offset + (offset * 4), mem_mask);
}

uint64_t address_map_bank_device::read64(address_space &space, offs_t offset, uint64_t mem_mask)
{
	m_program->set_debugger_access(space.debugger_access());
	return m_program->read_qword(m_offset + (offset * 8), mem_mask);
}

void address_map_bank_device::device_config_complete()
{
	m_program_config = address_space_config( "program", m_endianness, m_databus_width, m_addrbus_width );
}

void address_map_bank_device::device_start()
{
	m_program = &space(AS_PROGRAM);

	save_item(NAME(m_offset));
}

void address_map_bank_device::set_bank(offs_t bank)
{
	m_offset = bank * m_stride;
}
