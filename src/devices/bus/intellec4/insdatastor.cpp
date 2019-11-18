// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "insdatastor.h"


DEFINE_DEVICE_TYPE_NS(INTELLEC4_INST_DATA_STORAGE, bus::intellec4, imm4_22_device, "intlc4_imm4_22", "Intel imm4-22 Instruction/Data Storage Module")


namespace bus { namespace intellec4 {

namespace {

INPUT_PORTS_START(imm4_22)
	PORT_START("JUMPERS")
	PORT_CONFNAME( 0x03, 0x01, "4002 RAM page" )
	PORT_CONFSETTING(    0x01, "1" )
	PORT_CONFSETTING(    0x02, "2" )
	PORT_CONFSETTING(    0x03, "3" )
	PORT_CONFNAME( 0x0c, 0x04, "PROM/GPIO page" )
	PORT_CONFSETTING(    0x04, "0x400-0x7FF" )
	PORT_CONFSETTING(    0x08, "0x800-0xBFF" )
	PORT_CONFSETTING(    0x0c, "0xC00-0xFFF" )
	PORT_CONFNAME( 0x10, 0x00, "PROM/GPIO mode" )
	PORT_CONFSETTING(    0x00, "All" )
	PORT_CONFSETTING(    0x10, "MON only" )
INPUT_PORTS_END

} // anonymous namespace


imm4_22_device::imm4_22_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, INTELLEC4_INST_DATA_STORAGE, tag, owner, clock)
	, device_univ_card_interface(mconfig, *this)
	, device_image_interface(mconfig, *this)
	, m_jumpers(*this, "JUMPERS")
	, m_ram_io_mapped(false)
	, m_ram_page(0U), m_rom_page(0U), m_rom_mirror(false)
	, m_prom()
{
	std::fill(std::begin(m_memory), std::end(m_memory), 0U);
	std::fill(std::begin(m_status), std::end(m_status), 0U);
}


image_init_result imm4_22_device::call_load()
{
	if ((length() > 1024U) || (length() % 256U))
		return image_init_result::FAIL;

	allocate();
	if (fread(m_prom.get(), length()) != length())
		return image_init_result::FAIL;

	map_prom();

	return image_init_result::PASS;
}

void imm4_22_device::call_unload()
{
	unmap_prom();
}


ioport_constructor imm4_22_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(imm4_22);
}

void imm4_22_device::device_start()
{
	m_ram_io_mapped = false;

	allocate();

	save_item(NAME(m_ram_page));
	save_item(NAME(m_rom_page));
	save_item(NAME(m_rom_mirror));
	save_item(NAME(m_memory));
	save_item(NAME(m_status));
	save_pointer(NAME(m_prom), 1024U);
}

void imm4_22_device::device_reset()
{
	map_ram_io();
}


WRITE_LINE_MEMBER(imm4_22_device::reset_4002_in)
{
	// FIXME: this takes several cycles to actually erase everything, and prevents writes while asserted
	if (!state)
	{
		std::fill(std::begin(m_memory), std::end(m_memory), 0U);
		std::fill(std::begin(m_status), std::end(m_status), 0U);
	}
}


WRITE8_MEMBER(imm4_22_device::ram_out)
{
	// GPIO write - hooking this up would be a pain with MAME as it is
	logerror("4002 A%u out %X\n", 13U + (offset & 0x03U), data & 0x0fU);
}

WRITE8_MEMBER(imm4_22_device::rom_out)
{
	// GPIO write - hooking this up would be a pain with MAME as it is
	logerror("ROM %u out %X\n", (m_rom_page << 2) | ((offset >> 4) & 0x03U), data & 0x0fU);
}

READ8_MEMBER(imm4_22_device::rom_in)
{
	// GPIO read - hooking this up would be a pain with MAME as it is
	if (!machine().side_effects_disabled())
		logerror("ROM %u in\n", (m_rom_page << 2) | ((offset >> 4) & 0x03U));
	return 0x0fU;
}


void imm4_22_device::allocate()
{
	if (!m_prom)
	{
		m_prom = std::make_unique<u8 []>(1024U);
		std::fill(m_prom.get(), m_prom.get() + 1024U, 0U);
	}
}

void imm4_22_device::map_ram_io()
{
	if (!m_ram_io_mapped)
	{
		static constexpr offs_t ram_start[3][4] = {
				{ 0x01U, 0x04U, 0x05U, 0x07U },
				{ 0x02U, 0x04U, 0x06U, 0x07U },
				{ 0x03U, 0x05U, 0x06U, 0x07U } };

		m_ram_io_mapped = true;

		ioport_value const jumpers(m_jumpers->read());
		m_ram_page = jumpers & 0x03U;
		m_rom_page = (jumpers >> 2) & 0x03U;
		m_rom_mirror = BIT(~jumpers, 4);

		if ((1U > m_ram_page) || (3U < m_ram_page))
			throw emu_fatalerror("imm4_22_device: invalid RAM page %u", m_ram_page);
		if ((1U > m_rom_page) || (3U < m_rom_page))
			throw emu_fatalerror("imm4_22_device: invalid PROM/GPIO page %u", m_rom_page);

		for (offs_t const start : ram_start[m_ram_page - 1U])
		{
			memory_space().install_ram(start << 8, (start << 8) | 0x00ffU, m_memory);
			status_space().install_ram(start << 6, (start << 6) | 0x003fU, m_status);
			ram_ports_space().install_write_handler(start << 2, (start << 2) | 0x03U, write8_delegate(*this, FUNC(imm4_22_device::ram_out)));
		}

		offs_t const rom_ports_start(offs_t(m_rom_page) << 6);
		offs_t const rom_ports_end(rom_ports_start | 0x003fU);
		offs_t const rom_ports_mirror(m_rom_mirror ? 0x1f00U : 0x0700U);
		rom_ports_space().install_readwrite_handler(
				rom_ports_start, rom_ports_end, 0U, rom_ports_mirror, 0U,
				read8_delegate(*this, FUNC(imm4_22_device::rom_in)), write8_delegate(*this, FUNC(imm4_22_device::rom_out)));

		if (is_loaded())
			map_prom();
	}
}

void imm4_22_device::map_prom()
{
	if (m_ram_io_mapped)
	{
		// FIXME: gimme a cookie!
		// FIXME: if mirror is on, this will fight console RAM for the bus in 0x2000 region and also appear in the "two switches" 0x3000 region
		rom_space().install_rom(
				offs_t(m_rom_page) << 10, offs_t((offs_t(m_rom_page) << 10) | length()), m_rom_mirror ? 0x1000U : 0x0000U,
				m_prom.get());
	}
}

void imm4_22_device::unmap_prom()
{
	// FIXME: where's my magic cookie?
	rom_space().unmap_read(offs_t(m_rom_page) << 10, (offs_t(m_rom_page) << 10) | 0x03ffU, m_rom_mirror ? 0x1000U : 0x0000U);
}

} } // namespace bus::intellec4
