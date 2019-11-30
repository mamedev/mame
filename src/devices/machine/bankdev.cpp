// license:BSD-3-Clause
// copyright-holders:smf
#include "emu.h"
#include "bankdev.h"

// device type definition
DEFINE_DEVICE_TYPE(ADDRESS_MAP_BANK, address_map_bank_device, "address_map_bank", "Address Map Bank")

address_map_bank_device::address_map_bank_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, ADDRESS_MAP_BANK, tag, owner, clock),
		device_memory_interface(mconfig, *this),
		m_endianness(ENDIANNESS_NATIVE),
		m_data_width(0),
		m_addr_width(32),
		m_stride(1),
		m_program(nullptr),
		m_offset(0),
		m_shift(0)
{
}

device_memory_interface::space_config_vector address_map_bank_device::memory_space_config() const
{
	return space_config_vector {
		std::make_pair(AS_PROGRAM, &m_program_config)
	};
}

void address_map_bank_device::amap8(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(address_map_bank_device::read8), FUNC(address_map_bank_device::write8));
}

void address_map_bank_device::amap16(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(address_map_bank_device::read16), FUNC(address_map_bank_device::write16));
}

void address_map_bank_device::amap32(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(address_map_bank_device::read32), FUNC(address_map_bank_device::write32));
}

void address_map_bank_device::amap64(address_map &map)
{
	map(0x00000000, 0xffffffff).rw(FUNC(address_map_bank_device::read64), FUNC(address_map_bank_device::write64));
}

void address_map_bank_device::write8(offs_t offset, u8 data)
{
	m_program->write_byte(m_offset + offset, data);
}

void address_map_bank_device::write16(offs_t offset, u16 data, u16 mem_mask)
{
	m_program->write_word(m_offset + (offset << (m_shift+1)), data, mem_mask);
}

void address_map_bank_device::write32(offs_t offset, u32 data, u32 mem_mask)
{
	m_program->write_dword(m_offset + (offset << (m_shift+2)), data, mem_mask);
}

void address_map_bank_device::write64(offs_t offset, u64 data, u64 mem_mask)
{
	m_program->write_qword(m_offset + (offset << (m_shift+3)), data, mem_mask);
}

u8 address_map_bank_device::read8(offs_t offset)
{
	return m_program->read_byte(m_offset + offset);
}

u16 address_map_bank_device::read16(offs_t offset, u16 mem_mask)
{
	return m_program->read_word(m_offset + (offset << (m_shift+1)), mem_mask);
}

u32 address_map_bank_device::read32(offs_t offset, u32 mem_mask)
{
	return m_program->read_dword(m_offset + (offset << (m_shift+2)), mem_mask);
}

u64 address_map_bank_device::read64(offs_t offset, u64 mem_mask)
{
	return m_program->read_qword(m_offset + (offset << (m_shift+3)), mem_mask);
}

void address_map_bank_device::device_config_complete()
{
	m_program_config = address_space_config( "program", m_endianness, m_data_width, m_addr_width, m_shift );
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
