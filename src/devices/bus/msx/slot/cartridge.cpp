// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "cartridge.h"
#include "hashfile.h"


msx_slot_cartridge_base_device::msx_slot_cartridge_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_cartrom_image_interface(mconfig, *this)
	, device_slot_interface(mconfig, *this)
	, device_mixer_interface(mconfig, *this)
	, msx_internal_slot_interface(mconfig, *this)
	, m_irq_handler(*this)
	, m_cartridge(nullptr)
{
}


void msx_slot_cartridge_base_device::device_resolve_objects()
{
	m_cartridge = dynamic_cast<msx_cart_interface *>(get_card_device());
	if (m_cartridge)
	{
		m_cartridge->set_views(page(0), page(1), page(2), page(3));
	}
}


void msx_slot_cartridge_base_device::device_start()
{
}


std::pair<std::error_condition, std::string> msx_slot_cartridge_base_device::call_load()
{
	if (m_cartridge)
	{
		if (!loaded_through_softlist())
		{
			u32 const length = this->length();

			// determine how much space to allocate
			u32 length_aligned = 0x10000;

			if (length <= 0x2000)
				length_aligned = 0x2000;
			else if (length <= 0x4000)
				length_aligned = 0x4000;
			else if (length <= 0x8000)
				length_aligned = 0x8000;
			else if (length <= 0xc000)
				length_aligned = 0xc000;
			else
			{
				while (length_aligned < length)
					length_aligned *= 2;
			}

			memory_region *const romregion = machine().memory().region_alloc(subtag("rom"), length_aligned, 1, ENDIANNESS_LITTLE);
			if (fread(romregion->base(), length) != length)
				return std::make_pair(image_error::UNSPECIFIED, "Unable to fully read file");
		}

		std::string message;
		std::error_condition result = m_cartridge->initialize_cartridge(message);
		if (result)
			return std::make_pair(result, message);

		if (m_cartridge->cart_sram_region())
			battery_load(m_cartridge->cart_sram_region()->base(), m_cartridge->cart_sram_region()->bytes(), 0x00);
	}
	return std::make_pair(std::error_condition(), std::string());
}


void msx_slot_cartridge_base_device::call_unload()
{
	if (m_cartridge && m_cartridge->cart_sram_region())
	{
		battery_save(m_cartridge->cart_sram_region()->base(), m_cartridge->cart_sram_region()->bytes());
	}
}

void msx_slot_cartridge_base_device::irq_out(int state)
{
	m_irq_handler(state);
}


msx_cart_interface::msx_cart_interface(const machine_config &mconfig, device_t &device)
	: device_interface(device, "msxcart")
	, m_exp(dynamic_cast<msx_slot_cartridge_base_device *>(device.owner()))
{
	for (int i = 0; i < 4; i++)
		m_page[i] = nullptr;
}

void msx_cart_interface::irq_out(int state)
{
	m_exp->irq_out(state);
}

address_space &msx_cart_interface::memory_space() const
{
	return m_exp->memory_space();
}

address_space &msx_cart_interface::io_space() const
{
	return m_exp->io_space();
}

cpu_device &msx_cart_interface::maincpu() const
{
	return m_exp->maincpu();
}

device_mixer_interface &msx_cart_interface::soundin() const
{
	return *m_exp;
}

void msx_cart_interface::set_views(memory_view::memory_view_entry *page0, memory_view::memory_view_entry *page1, memory_view::memory_view_entry *page2, memory_view::memory_view_entry *page3)
{
	m_page[0] = page0;
	m_page[1] = page1;
	m_page[2] = page2;
	m_page[3] = page3;
}
