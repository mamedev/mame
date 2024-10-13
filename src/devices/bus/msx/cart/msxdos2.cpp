// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#include "emu.h"
#include "msxdos2.h"

#include "bus/generic/slot.h"


/*
Emulation of MSX-DOS2 cartridges.

Japanese MSX-DOS2 uses an expanded slot model:
- Subslot 1 - Memory mapper with no, 128KB, or 256KB RAM
- Subslot 2 - Kanji Basic ROM
- Subslot 3 - MSX-DOS2 banked ROM

European MSX-DOS2 releases only contain MSX-DOS2 banked ROM.

The rom banking address is either $6000 or $7ffe depending on the
version of the hardware.

*/

namespace {

class msx_cart_msxdos2_base_device : public device_t, public msx_cart_interface
{
protected:
	msx_cart_msxdos2_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_rombank(*this, "rombank")
		, m_bank_address(0)
	{ }

	virtual void device_start() override { }
	virtual void device_reset() override ATTR_COLD;
	virtual std::error_condition initialize_cartridge(std::string &message) override;

	void bank_w(u8 data);

	static inline constexpr unsigned PAGE_SIZE = 0x4000;

	memory_bank_creator m_rombank;
	u16 m_bank_address;
};

void msx_cart_msxdos2_base_device::device_reset()
{
	m_rombank->set_entry(0);
}

std::error_condition msx_cart_msxdos2_base_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_msxdos2: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x10000)
	{
		message = "msx_cart_msxdos2: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	const char *bank_address_str = get_feature("bank_address");
	if (!bank_address_str)
	{
		message = "msx_cart_msxdos2: Feature 'bank_address' was not found.";
		return image_error::BADSOFTWARE;
	}
	m_bank_address = strtol(bank_address_str, nullptr, 0);
	if (m_bank_address < 0x4000 || m_bank_address > 0x7fff)
	{
		message = "msx_cart_msxdos2: bank_address must be between 0x4000 and 0x7fff.";
		return image_error::BADSOFTWARE;
	}

	m_rombank->configure_entries(0, 4, cart_rom_region()->base(), PAGE_SIZE);

	return std::error_condition();
}

void msx_cart_msxdos2_base_device::bank_w(u8 data)
{
	m_rombank->set_entry(data & 0x03);
}



class msx_cart_msxdos2j_device : public msx_cart_msxdos2_base_device
{
public:
	msx_cart_msxdos2j_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_msxdos2_base_device(mconfig, MSX_CART_MSXDOS2J, tag, owner, clock)
		, m_rambank(*this, "rambank%u", 0)
		, m_view_page{ {*this, "view_page0"}, {*this, "view_page1"}, {*this, "view_page2"}, {*this, "view_page3"} }
		, m_secondary_slot(0)
		, m_bank_mask(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override ATTR_COLD;

private:
	u8 secondary_slot_r();
	void secondary_slot_w(u8 data);
	template <int Bank> void mm_bank_w(u8 data);

	memory_bank_array_creator<4> m_rambank;
	memory_view m_view_page[4];
	u8 m_secondary_slot;
	u8 m_bank_mask;
};


void msx_cart_msxdos2j_device::device_start()
{
	msx_cart_msxdos2_base_device::device_start();
	save_item(NAME(m_secondary_slot));
}

std::error_condition msx_cart_msxdos2j_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = msx_cart_msxdos2_base_device::initialize_cartridge(message);
	if (result)
		return result;

	u32 ram_size = cart_ram_region() ? cart_ram_region()->bytes() : 0;
	if (ram_size > 256 * PAGE_SIZE || (ram_size & (PAGE_SIZE - 1)) != 0)
	{
		message = "msx_cart_msxdos2: Region 'ram' size must be a multiple of 0x4000 and at most 0x400000.";
		return image_error::BADSOFTWARE;
	}

	for (int pg = 0; pg < 4; pg++)
		page(pg)->install_view(0x4000 * pg, 0x4000 * pg + 0x3fff, m_view_page[pg]);
	page(3)->install_readwrite_handler(0xffff, 0xffff, emu::rw_delegate(*this, FUNC(msx_cart_msxdos2j_device::secondary_slot_r)), emu::rw_delegate(*this, FUNC(msx_cart_msxdos2j_device::secondary_slot_w)));

	// Make sure all views we can select do exist.
	for (int subslot = 0; subslot < 4; subslot++)
	{
		for (int page = 0; page < 4; page++)
			m_view_page[page][subslot];
	}

	// Kanji driver
	m_view_page[1][2].install_rom(0x4000, 0x7fff, cart_rom_region()->base() + 0xc000);

	// MSX-DOS2 rom
	m_view_page[1][3].install_read_bank(0x4000, 0x7fff, m_rombank);
	m_view_page[1][3].install_write_handler(m_bank_address, m_bank_address, emu::rw_delegate(*this, FUNC(msx_cart_msxdos2j_device::bank_w)));

	// On-cartridge memory mapper RAM
	if (ram_size)
	{
		m_bank_mask = device_generic_cart_interface::map_non_power_of_two(
			unsigned(ram_size / PAGE_SIZE),
			[this] (unsigned entry, unsigned page)
			{
				for (int i = 0; i < 4; i++)
					m_rambank[i]->configure_entry(entry, cart_ram_region()->base() + PAGE_SIZE * page);
			}
		);

		for (int pg = 0; pg < 4; pg++)
			m_view_page[pg][1].install_readwrite_bank(0x4000 * pg, 0x4000 * pg + 0x3fff, m_rambank[pg]);

		io_space().install_write_tap(0xfc, 0xfc, "bank0", [this] (offs_t, u8& data, u8){ this->mm_bank_w<0>(data); });
		io_space().install_write_tap(0xfd, 0xfd, "bank1", [this] (offs_t, u8& data, u8){ this->mm_bank_w<1>(data); });
		io_space().install_write_tap(0xfe, 0xfe, "bank2", [this] (offs_t, u8& data, u8){ this->mm_bank_w<2>(data); });
		io_space().install_write_tap(0xff, 0xff, "bank3", [this] (offs_t, u8& data, u8){ this->mm_bank_w<3>(data); });
	}

	return std::error_condition();
}

u8 msx_cart_msxdos2j_device::secondary_slot_r()
{
	return ~m_secondary_slot;
}

void msx_cart_msxdos2j_device::secondary_slot_w(u8 data)
{
	for (int page = 0; page < 4; page++)
		m_view_page[page].select((data >> (2 * page)) & 0x03);
	m_secondary_slot = data;
}

template <int Bank>
void msx_cart_msxdos2j_device::mm_bank_w(u8 data)
{
	m_rambank[Bank]->set_entry(data & m_bank_mask);
}



class msx_cart_msxdos2e_device : public msx_cart_msxdos2_base_device
{
public:
	msx_cart_msxdos2e_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: msx_cart_msxdos2_base_device(mconfig, MSX_CART_MSXDOS2E, tag, owner, clock)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;
};

std::error_condition msx_cart_msxdos2e_device::initialize_cartridge(std::string &message)
{
	std::error_condition result = msx_cart_msxdos2_base_device::initialize_cartridge(message);
	if (result)
		return result;

	page(1)->install_read_bank(0x4000, 0x7fff, m_rombank);
	page(1)->install_write_handler(m_bank_address, m_bank_address, emu::rw_delegate(*this, FUNC(msx_cart_msxdos2e_device::bank_w)));

	return std::error_condition();
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSXDOS2J, msx_cart_interface, msx_cart_msxdos2j_device, "msx_cart_msxdos2j", "MSX Cartridge - MSXDOS2 Japan")
DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_MSXDOS2E, msx_cart_interface, msx_cart_msxdos2e_device, "msx_cart_msxdos2e", "MSX Cartridge - MSXDOS2 Europe")
