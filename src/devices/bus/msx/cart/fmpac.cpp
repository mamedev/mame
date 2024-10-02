// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

When backing up the SRAM from an FM-PAC the file seems to be prefixed
with: PAC2 BACKUP DATA. We only store the raw sram contents.

**********************************************************************************/

#include "emu.h"
#include "fmpac.h"

#include "sound/ymopl.h"

#include "speaker.h"


namespace {

class msx_cart_fmpac_device : public device_t, public msx_cart_interface
{
public:
	msx_cart_fmpac_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, MSX_CART_FMPAC, tag, owner, clock)
		, msx_cart_interface(mconfig, *this)
		, m_ym2413(*this, "ym2413")
		, m_rombank(*this, "rombank")
		, m_view(*this, "view")
		, m_sram_active(false)
		, m_opll_active(false)
		, m_sram_unlock{0, 0}
		, m_control(0)
	{ }

	virtual std::error_condition initialize_cartridge(std::string &message) override;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_t implementation
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	void write_ym2413(offs_t offset, u8 data);
	void sram_unlock(offs_t offset, u8 data);
	u8 control_r();
	void control_w(u8 data);
	u8 bank_r();
	void bank_w(u8 data);

	required_device<ym2413_device> m_ym2413;
	memory_bank_creator m_rombank;
	memory_view m_view;

	bool m_sram_active;
	bool m_opll_active;
	u8 m_sram_unlock[2];
	u8 m_control;
};

void msx_cart_fmpac_device::device_add_mconfig(machine_config &config)
{
	YM2413(config, m_ym2413, DERIVED_CLOCK(1, 1));
	if (parent_slot())
		m_ym2413->add_route(ALL_OUTPUTS, soundin(), 0.8);
}

void msx_cart_fmpac_device::device_start()
{
	save_item(NAME(m_sram_active));
	save_item(NAME(m_opll_active));
	save_item(NAME(m_sram_unlock));
	save_item(NAME(m_control));

	// Install IO read/write handlers
	io_space().install_write_handler(0x7c, 0x7d, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::write_ym2413)));
}

void msx_cart_fmpac_device::device_reset()
{
	m_sram_active = false;
	m_opll_active = false;
	m_sram_unlock[0] = 0;
	m_sram_unlock[1] = 0;
	m_control = 0;
	m_view.select(0);
	m_rombank->set_entry(0);

}

std::error_condition msx_cart_fmpac_device::initialize_cartridge(std::string &message)
{
	if (!cart_rom_region())
	{
		message = "msx_cart_fmpac_device: Required region 'rom' was not found.";
		return image_error::INTERNAL;
	}

	if (!cart_sram_region())
	{
		message = "msx_cart_fmpac_device: Required region 'sram' was not found.";
		return image_error::INTERNAL;
	}

	if (cart_rom_region()->bytes() != 0x10000)
	{
		message = "msx_cart_fmpac_device: Region 'rom' has unsupported size.";
		return image_error::INVALIDLENGTH;
	}

	if (cart_sram_region()->bytes() < 0x2000)
	{
		message = "msx_cart_fmpac_device: Region 'sram' has unsupported size.";
		return image_error::BADSOFTWARE;
	}

	m_rombank->configure_entries(0, 4, cart_rom_region()->base(), 0x4000);

	page(1)->install_view(0x4000, 0x7fff, m_view);
	m_view[0].install_read_bank(0x4000, 0x7fff, m_rombank);
	m_view[0].install_write_handler(0x5ffe, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::sram_unlock)));
	m_view[0].install_write_handler(0x7ff4, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::write_ym2413)));
	m_view[0].install_read_handler(0x7ff6, 0x7ff6, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::control_r)));
	m_view[0].install_write_handler(0x7ff6, 0x7ff6, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::control_w)));
	m_view[0].install_read_handler(0x7ff7, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::bank_r)));
	m_view[0].install_write_handler(0x7ff7, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::bank_w)));

	m_view[1].install_ram(0x4000, 0x5fff, cart_sram_region()->base());
	m_view[1].install_write_handler(0x5ffe, 0x5fff, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::sram_unlock)));
	m_view[1].install_write_handler(0x7ff4, 0x7ff5, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::write_ym2413)));
	m_view[1].install_read_handler(0x7ff6, 0x7ff6, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::control_r)));
	m_view[1].install_write_handler(0x7ff6, 0x7ff6, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::control_w)));
	m_view[1].install_read_handler(0x7ff7, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::bank_r)));
	m_view[1].install_write_handler(0x7ff7, 0x7ff7, emu::rw_delegate(*this, FUNC(msx_cart_fmpac_device::bank_w)));

	return std::error_condition();
}

void msx_cart_fmpac_device::sram_unlock(offs_t offset, u8 data)
{
	m_sram_unlock[offset] = data;
	m_sram_active = m_sram_unlock[0] == 0x4d && m_sram_unlock[1] == 0x69;
	m_view.select(m_sram_active ? 1 : 0);
}

u8 msx_cart_fmpac_device::control_r()
{
	return m_control;
}

void msx_cart_fmpac_device::control_w(u8 data)
{
	m_control = data & 0x11;
	m_opll_active = BIT(data, 0);
}

u8 msx_cart_fmpac_device::bank_r()
{
	return m_rombank->entry();
}

void msx_cart_fmpac_device::bank_w(u8 data)
{
	m_rombank->set_entry(data);
}

void msx_cart_fmpac_device::write_ym2413(offs_t offset, u8 data)
{
	if (m_opll_active)
	{
		m_ym2413->write(offset & 1, data);
	}
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(MSX_CART_FMPAC, msx_cart_interface, msx_cart_fmpac_device, "msx_cart_fmpac", "MSX Cartridge - FM-PAC")
