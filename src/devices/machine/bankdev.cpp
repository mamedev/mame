// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "bankdev.h"

// device type definition
DEFINE_DEVICE_TYPE(ADDRESS_MAP_BANK, address_map_bank_device, "address_map_bank", "Address Map Bank")

address_map_bank_device::address_map_bank_device( const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock )
	: device_t(mconfig, ADDRESS_MAP_BANK, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_endianness(ENDIANNESS_NATIVE),
		m_data_width(0),
		m_addr_width(32),
		m_stride(1),
		m_program(nullptr),
		m_offset(0)
{
}

device_memory_interface::space_config_vector address_map_bank_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

ADDRESS_MAP_START(address_map_bank_device::amap8)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read8, write8)
ADDRESS_MAP_END

ADDRESS_MAP_START(address_map_bank_device::amap16)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read16, write16)
ADDRESS_MAP_END

ADDRESS_MAP_START(address_map_bank_device::amap32)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read32, write32)
ADDRESS_MAP_END

ADDRESS_MAP_START(address_map_bank_device::amap64)
	AM_RANGE(0x00000000, 0xffffffff) AM_READWRITE(read64, write64)
ADDRESS_MAP_END

WRITE8_MEMBER(address_map_bank_device::write8)
{
	m_program->write_byte(m_offset + offset, data);
}

WRITE16_MEMBER(address_map_bank_device::write16)
{
	m_program->write_word(m_offset + (offset * 2), data, mem_mask);
}

WRITE32_MEMBER(address_map_bank_device::write32)
{
	m_program->write_dword(m_offset + (offset * 4), data, mem_mask);
}

WRITE64_MEMBER(address_map_bank_device::write64)
{
	m_program->write_qword(m_offset + (offset * 8), data, mem_mask);
}

READ8_MEMBER(address_map_bank_device::read8)
{
	return m_program->read_byte(m_offset + offset);
}

READ16_MEMBER(address_map_bank_device::read16)
{
	return m_program->read_word(m_offset + (offset * 2), mem_mask);
}

READ32_MEMBER(address_map_bank_device::read32)
{
	return m_program->read_dword(m_offset + (offset * 4), mem_mask);
}

READ64_MEMBER(address_map_bank_device::read64)
{
	return m_program->read_qword(m_offset + (offset * 8), mem_mask);
}

void address_map_bank_device::device_config_complete()
{
	m_program_config = address_space_config( "program", m_endianness, m_data_width, m_addr_width );
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
