// license:BSD-3-Clause
// copyright-holders:Wilbert Pol

#include "emu.h"
#include "ngp_cart.h"

#include <algorithm>


//**************************************************************************
//  CARTRIDGE FLASH
//**************************************************************************

DEFINE_DEVICE_TYPE(NGP_FLASH_4M,  ngp_flash_4m_device,  "ngp_flash_4m",  "SNK Neo Geo Pocket Cartridge Flash 4Mbit")
DEFINE_DEVICE_TYPE(NGP_FLASH_8M,  ngp_flash_8m_device,  "ngp_flash_8m",  "SNK Neo Geo Pocket Cartridge Flash 8Mbit")
DEFINE_DEVICE_TYPE(NGP_FLASH_16M, ngp_flash_16m_device, "ngp_flash_16m", "SNK Neo Geo Pocket Cartridge Flash 16Mbit")

ngp_flash_device::ngp_flash_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock, uint32_t size, uint16_t device_id)
	: intelfsh8_device(mconfig, type, tag, owner, clock, size, 0x98 /* Toshiba */, device_id)
	, m_cart_data(nullptr)
	, m_cart_length(0)
	, m_selected(false)
{
	m_top_boot_sector = true;
}

ngp_flash_4m_device::ngp_flash_4m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: ngp_flash_device(mconfig, NGP_FLASH_4M, tag, owner, clock, SIZE, 0xab)
{
}

ngp_flash_8m_device::ngp_flash_8m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: ngp_flash_device(mconfig, NGP_FLASH_8M, tag, owner, clock, SIZE, 0x2c)
{
}

ngp_flash_16m_device::ngp_flash_16m_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: ngp_flash_device(mconfig, NGP_FLASH_16M, tag, owner, clock, SIZE, 0x2f)
{
}

void ngp_flash_device::set_cartridge(uint8_t const *data, uint32_t length)
{
	m_cart_data = data;
	m_cart_length = length;
	m_selected = true;
}

void ngp_flash_device::deselect()
{
	m_cart_data = nullptr;
	m_cart_length = 0;
	m_selected = false;
}


//-------------------------------------------------
//  nvram handling
//-------------------------------------------------

void ngp_flash_device::nvram_default()
{
	if (m_selected && m_cart_data)
	{
		uint32_t const length = std::min(m_cart_length, m_size);
		std::copy_n(m_cart_data, length, &m_data[0]);
		std::fill_n(&m_data[length], m_size - length, 0xff);
	}
	else
	{
		std::fill_n(&m_data[0], m_size, 0xff);
	}
}

bool ngp_flash_device::nvram_read(util::read_stream &file)
{
	if (!m_selected)
		return false;

	auto const [err, actual] = util::read(file, &m_data[0], m_size);
	return !err && (actual == m_size);
}

bool ngp_flash_device::nvram_write(util::write_stream &file)
{
	auto const [err, actual] = util::write(file, &m_data[0], m_size);
	return !err;
}

bool ngp_flash_device::nvram_can_write() const
{
	return m_selected;
}


//**************************************************************************
//  CARTRIDGE SLOT
//**************************************************************************

DEFINE_DEVICE_TYPE(NGP_CART_SLOT, ngp_cart_slot_device, "ngp_cart_slot", "SNK Neo Geo Pocket Cartridge Slot")

ngp_cart_slot_device::ngp_cart_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: generic_cartslot_device(mconfig, tag, owner, clock)
	, m_flash4m(*this, "flash4m")
	, m_flash8m(*this, "flash8m")
	, m_flash16m(*this, "flash16m")
	, m_flash16m_b(*this, "flash16m_b")
	, m_chip{ nullptr, nullptr }
	, m_address_space(*this, finder_base::DUMMY_TAG, -1, 8)
{
}

void ngp_cart_slot_device::device_add_mconfig(machine_config &config)
{
	NGP_FLASH_4M(config, m_flash4m);
	NGP_FLASH_8M(config, m_flash8m);
	NGP_FLASH_16M(config, m_flash16m);
	NGP_FLASH_16M(config, m_flash16m_b);
}

void ngp_cart_slot_device::configure_flash(uint32_t size)
{
	deselect_flash();

	switch (size)
	{
	case 0x8000:
	case 0x80000:
		m_flash4m->set_cartridge(get_rom_base(), size);
		m_chip[0] = m_flash4m;
		m_address_space->install_readwrite_handler(0x20'0000, 0x3f'ffff,
				read8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_r<0, ngp_flash_4m_device::MASK>))),
				write8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_w<0, ngp_flash_4m_device::MASK>))));
		break;

	case 0x100000:
		m_flash8m->set_cartridge(get_rom_base(), size);
		m_chip[0] = m_flash8m;
		m_address_space->install_readwrite_handler(0x20'0000, 0x3f'ffff,
				read8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_r<0, ngp_flash_8m_device::MASK>))),
				write8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_w<0, ngp_flash_8m_device::MASK>))));
		break;

	case 0x200000:
		m_flash16m->set_cartridge(get_rom_base(), size);
		m_chip[0] = m_flash16m;
		m_address_space->install_readwrite_handler(0x20'0000, 0x3f'ffff,
				read8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_r<0, ngp_flash_16m_device::MASK>))),
				write8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_w<0, ngp_flash_16m_device::MASK>))));
		break;

	case 0x400000:
		// 32Mbit cartridges are two 16Mbit chips, one in each window
		m_flash16m->set_cartridge(get_rom_base(), ngp_flash_16m_device::SIZE);
		m_flash16m_b->set_cartridge(get_rom_base() + ngp_flash_16m_device::SIZE, ngp_flash_16m_device::SIZE);
		m_chip[0] = m_flash16m;
		m_chip[1] = m_flash16m_b;
		m_address_space->install_readwrite_handler(0x20'0000, 0x3f'ffff,
				read8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_r<0, ngp_flash_16m_device::MASK>))),
				write8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_w<0, ngp_flash_16m_device::MASK>))));
		m_address_space->install_readwrite_handler(0x80'0000, 0x9f'ffff,
				read8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_r<1, ngp_flash_16m_device::MASK>))),
				write8sm_delegate(*this, NAME((&ngp_cart_slot_device::flash_w<1, ngp_flash_16m_device::MASK>))));
		break;
	}
}

void ngp_cart_slot_device::deselect_flash()
{
	m_flash4m->deselect();
	m_flash8m->deselect();
	m_flash16m->deselect();
	m_flash16m_b->deselect();

	m_chip[0] = m_chip[1] = nullptr;
}


std::pair<std::error_condition, std::string> ngp_cart_slot_device::call_load()
{
	if (!m_cart)
		return std::make_pair(std::error_condition(), std::string());

	uint32_t const size = common_get_size("rom");

	if (size != 0x8000 // for homebrew
			&& size != ngp_flash_4m_device::SIZE
			&& size != ngp_flash_8m_device::SIZE
			&& size != ngp_flash_16m_device::SIZE
			&& size != ngp_flash_16m_device::SIZE * 2) // 32Mbit cartridges have 2 16Mbit flash chips
		return std::make_pair(image_error::INVALIDLENGTH, "Unsupported cartridge size (must be 32K, 512K, 1M, 2M or 4M)");

	rom_alloc(size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	common_load_rom(get_rom_base(), size, "rom");

	configure_flash(size);

	return std::make_pair(std::error_condition(), std::string());
}


void ngp_cart_slot_device::call_unload()
{
	// unmap the handlers installed by call_load
	m_address_space->unmap_readwrite(0x20'0000, 0x3f'ffff);
	m_address_space->unmap_readwrite(0x80'0000, 0x9f'ffff);

	deselect_flash();
	generic_cartslot_device::call_unload();
}


template <unsigned N, uint32_t Mask>
uint8_t ngp_cart_slot_device::flash_r(offs_t offset)
{
	return m_chip[N]->read(offset & Mask);
}


template <unsigned N, uint32_t Mask>
void ngp_cart_slot_device::flash_w(offs_t offset, uint8_t data)
{
	m_chip[N]->write(offset & Mask, data);
}
