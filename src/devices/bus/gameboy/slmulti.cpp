// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Chinese multi-game cartridges by SL and possibly others

 Supports collections containing very large numbers of games designed for
 MBC1 and MBC5 cartridges.

 ***************************************************************************/

#include "emu.h"
#include "slmulti.h"

#include "cartbase.h"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class slmulti_device : public mbc_dual_uniform_device_base
{
public:
	slmulti_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_switch_rom_fine(u8 data);
	void bank_switch_rom_coarse(u8 data);
	void set_config_cmd(u8 data);
	void do_config_cmd(u8 data);

	void update_bank_rom_high()
	{
		u16 const page(bank_rom_entry_high());
		LOG(
				"%s: Set high ROM page 0x%03X (0x%06X)\n",
				machine().describe_context(),
				page,
				u32(page) << 14);
		set_bank_rom_high(page);
	}

	u16 bank_rom_entry_high() const noexcept
	{
		u16 const hi(m_high_page & m_page_mask);
		return (m_base_page & ~m_page_mask) | (hi ? hi : (m_zero_remap & m_page_mask));
	}

	memory_view m_view_ctrl;

	u16 m_base_page;
	u16 m_high_page;
	u16 m_page_mask;
	u16 m_zero_remap;
	u8 m_config_cmd;
};


slmulti_device::slmulti_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_dual_uniform_device_base(mconfig, GB_ROM_SLMULTI, tag, owner, clock),
	m_view_ctrl(*this, "ctrl"),
	m_base_page(0U),
	m_high_page(0U),
	m_page_mask(0U),
	m_zero_remap(0U),
	m_config_cmd(0U)
{
}


image_init_result slmulti_device::load(std::string &message)
{
	// set up ROM
	set_bank_bits_rom(10);
	if (!check_rom(message))
		return image_init_result::FAIL;
	install_rom();

	// install memory mapping control handlers
	cart_space()->install_view(
			0x2000, 0x7fff,
			m_view_ctrl);
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(slmulti_device::bank_switch_rom_fine)));

	// this is for MBC5 games
	m_view_ctrl[0].install_write_handler(
			0x3000, 0x3fff,
			write8smo_delegate(*this, FUNC(slmulti_device::bank_switch_rom_coarse)));

	// install configuration handlers over the top
	m_view_ctrl[1].install_write_handler(
			0x5000, 0x5fff,
			write8smo_delegate(*this, FUNC(slmulti_device::set_config_cmd)));
	m_view_ctrl[1].install_write_handler(
			0x7000, 0x7fff,
			write8smo_delegate(*this, FUNC(slmulti_device::do_config_cmd)));

	// do this here - the menu program apparently does a system reset to get into DMG mode
	m_view_ctrl.select(1);

	// all good
	return image_init_result::PASS;
}


void slmulti_device::device_start()
{
	mbc_dual_uniform_device_base::device_start();

	m_base_page = 0U;
	m_high_page = 0U;
	m_page_mask = 1U;
	m_zero_remap = 0U;
	m_config_cmd = 0U;

	save_item(NAME(m_base_page));
	save_item(NAME(m_high_page));
	save_item(NAME(m_page_mask));
	save_item(NAME(m_zero_remap));
	save_item(NAME(m_config_cmd));
}


void slmulti_device::device_reset()
{
	mbc_dual_uniform_device_base::device_reset();

	set_bank_rom_low(m_base_page & ~m_page_mask);
	update_bank_rom_high();
}


void slmulti_device::bank_switch_rom_fine(u8 data)
{
	m_high_page = data;
	update_bank_rom_high();
}


void slmulti_device::bank_switch_rom_coarse(u8 data)
{
	// there's no way to specify a 9-bit page mask, so MBC5 games larger than 4 MiB can't be supported
	LOG("%s Set coarse ROM bank 0x%02X\n", machine().describe_context(), data);
}


void slmulti_device::set_config_cmd(u8 data)
{
	LOG("%s: Set configuration command 0x%02X\n", machine().describe_context(), data);
	m_config_cmd = data;
}


void slmulti_device::do_config_cmd(u8 data)
{
	switch (m_config_cmd)
	{
	case 0x55:
		// bit 4 unknown
		m_base_page = (m_base_page & ~0x0200) | (u16(BIT(data, 3)) << 9);
		m_page_mask = (2U << BIT(~data, 0, 3)) - 1;
		switch (BIT(data, 5, 2))
		{
		case 0x0: // used for MBC5 games
			m_zero_remap = 0U;
			m_view_ctrl.select(0);
			break;
		case 0x3: // used for MBC1 games
			m_zero_remap = 1U;
			m_view_ctrl.disable();
			break;
		default:
			logerror(
					"%s: Unknown memory mapping mode 0x%X\n",
					machine().describe_context(),
					BIT(data, 5, 2));
			m_view_ctrl.disable();
		}
		LOG(
				"%s: Set base page = 0x%03X (0x%06X), page mask = 0x%03X, zero remap = 0x%03X%s\n",
				machine().describe_context(),
				m_base_page,
				u32(m_base_page) << 14,
				m_page_mask,
				m_zero_remap,
				BIT(data, 7) ? ", reset" : "");
		set_bank_rom_low(m_base_page & ~m_page_mask);
		update_bank_rom_high();
		if (BIT(data, 7))
			machine().root_device().reset(); // TODO: expose reset line on cartridge interface
		break;

	case 0xaa:
		m_base_page = (m_base_page & ~0x01fe) | (u16(data) << 1);
		LOG(
				"%s: Set base page = 0x%03X (0x%06X)\n",
				machine().describe_context(),
				m_base_page,
				u32(m_base_page) << 14);
		break;

	default:
		LOG(
				"%s: Unknown configuration command 0x%02X with argument 0x%02X\n",
				machine().describe_context(),
				m_config_cmd,
				data);
	}
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SLMULTI, device_gb_cart_interface, bus::gameboy::slmulti_device, "gb_rom_slmulti", "Game Boy SL Multi-Game Cartridge")
