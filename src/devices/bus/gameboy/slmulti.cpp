// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Chinese multi-game cartridges by SL and possibly others

 Supports collections containing very large numbers of games designed for
 MBC1 and MBC5 cartridges.

 ***************************************************************************/

#include "emu.h"
#include "slmulti.h"

#include "cartbase.ipp"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class slmulti_device : public mbc_ram_device_base<mbc_dual_uniform_device_base>
{
public:
	slmulti_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void enable_ram(u8 data);
	void bank_switch_rom_fine(u8 data);
	void bank_switch_rom_coarse(u8 data);
	void bank_switch_ram(u8 data);
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

	void update_bank_ram()
	{
		u16 const page(bank_ram_entry());
		LOG(
				"%s: Set RAM page 0x%X (0x%05X)\n",
				machine().describe_context(),
				page,
				u32(page) << 13);
		set_bank_ram(page);
	}

	u16 bank_rom_entry_high() const noexcept
	{
		u16 const hi(m_high_page_rom & m_page_mask_rom);
		return (m_base_page_rom & ~m_page_mask_rom) | (hi ? hi : (m_zero_remap_rom & m_page_mask_rom));
	}

	u8 bank_ram_entry() const noexcept
	{
		return (m_base_page_ram & ~m_page_mask_ram) | (m_page_ram & m_page_mask_ram);
	}

	memory_view m_view_ctrl;
	memory_view m_view_ram;

	u16 m_config_cmd;

	u16 m_base_page_rom;
	u16 m_high_page_rom;
	u16 m_page_mask_rom;
	u16 m_zero_remap_rom;

	u8 m_enable_ram;
	u8 m_base_page_ram;
	u8 m_page_ram;
	u8 m_page_mask_ram;
};


slmulti_device::slmulti_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_dual_uniform_device_base>(mconfig, GB_ROM_SLMULTI, tag, owner, clock),
	m_view_ctrl(*this, "ctrl"),
	m_view_ram(*this, "ram"),
	m_config_cmd(0U),
	m_base_page_rom(0U),
	m_high_page_rom(0U),
	m_page_mask_rom(0U),
	m_zero_remap_rom(0U),
	m_enable_ram(0U),
	m_base_page_ram(0U),
	m_page_ram(0U),
	m_page_mask_ram(0U)
{
}


image_init_result slmulti_device::load(std::string &message)
{
	// set up ROM and RAM
	set_bank_bits_rom(10);
	set_bank_bits_ram(4);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
	install_rom();
	install_ram(m_view_ram[0]);

	// install memory mapping control handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, FUNC(slmulti_device::enable_ram)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(slmulti_device::bank_switch_rom_fine)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(slmulti_device::bank_switch_ram)));

	// configuration mode and MBC5 mode partially overlay the normal handlers
	cart_space()->install_view(
			0x2000, 0x7fff,
			m_view_ctrl);

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
	mbc_ram_device_base<mbc_dual_uniform_device_base>::device_start();

	m_config_cmd = 0x0100U;

	m_base_page_rom = 0U;
	m_high_page_rom = 0U;
	m_page_mask_rom = 1U;
	m_zero_remap_rom = 0U;

	m_enable_ram = 0U;
	m_base_page_ram = 0U;
	m_page_ram = 0U;
	m_page_mask_ram = 0U;

	save_item(NAME(m_config_cmd));
	save_item(NAME(m_base_page_rom));
	save_item(NAME(m_high_page_rom));
	save_item(NAME(m_page_mask_rom));
	save_item(NAME(m_zero_remap_rom));
	save_item(NAME(m_enable_ram));
	save_item(NAME(m_base_page_ram));
	save_item(NAME(m_page_ram));
	save_item(NAME(m_page_mask_ram));
}


void slmulti_device::device_reset()
{
	mbc_ram_device_base<mbc_dual_uniform_device_base>::device_reset();

	m_view_ram.disable();

	set_bank_rom_low(m_base_page_rom & ~m_page_mask_rom);
	update_bank_rom_high();
	update_bank_ram();
}


void slmulti_device::enable_ram(u8 data)
{
	// TODO: how many bits are checked?
	if ((0x0a == (data & 0x0f)) && m_enable_ram)
	{
		LOG("%s: Cartridge RAM enabled\n", machine().describe_context());
		m_view_ram.select(0);
	}
	else
	{
		LOG("%s: Cartridge RAM disabled\n", machine().describe_context());
		m_view_ram.disable();
	}
}


void slmulti_device::bank_switch_rom_fine(u8 data)
{
	m_high_page_rom = data;
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


void slmulti_device::bank_switch_ram(u8 data)
{
	m_page_ram = data & 0x0f;
	update_bank_ram();
}


void slmulti_device::do_config_cmd(u8 data)
{
	LOG(
			"%s: Execute configuration command 0x%02X with argument 0x%02X\n",
			machine().describe_context(),
			m_config_cmd,
			data);
	switch (m_config_cmd)
	{
	case 0x55:
		// bit 4 unknown
		m_base_page_rom = (m_base_page_rom & ~0x0200) | (u16(BIT(data, 3)) << 9);
		m_page_mask_rom = (2U << BIT(~data, 0, 3)) - 1;
		switch (BIT(data, 5, 2))
		{
		case 0x0: // used for MBC5 games
			m_zero_remap_rom = 0U;
			m_view_ctrl.select(0);
			break;
		case 0x3: // used for MBC1 games
			m_zero_remap_rom = 1U;
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
				"%s: Set base ROM page = 0x%03X (0x%06X), page mask = 0x%03X, zero remap = 0x%03X%s\n",
				machine().describe_context(),
				m_base_page_rom,
				u32(m_base_page_rom) << 14,
				m_page_mask_rom,
				m_zero_remap_rom,
				BIT(data, 7) ? ", reset" : "");
		set_bank_rom_low(m_base_page_rom & ~m_page_mask_rom);
		update_bank_rom_high();
		if (BIT(data, 7))
			machine().root_device().reset(); // TODO: expose reset line on cartridge interface
		break;

	case 0xaa:
		m_base_page_rom = (m_base_page_rom & ~0x01fe) | (u16(data) << 1);
		LOG(
				"%s: Set base ROM page = 0x%03X (0x%06X)\n",
				machine().describe_context(),
				m_base_page_rom,
				u32(m_base_page_rom) << 14);
		set_bank_rom_low(m_base_page_rom & ~m_page_mask_rom);
		update_bank_rom_high();
		break;

	case 0xbb:
		m_enable_ram = BIT(data, 5);
		m_base_page_ram = BIT(data, 0, 4);
		m_page_mask_ram = BIT(data, 4) ? 0x00 : 0x03;
		LOG(
				"%s: Set RAM %s, base page = 0x%X, page mask = 0x%X\n",
				machine().describe_context(),
				m_enable_ram ? "enabled" : "disabled",
				m_base_page_ram,
				m_page_mask_ram);
		update_bank_ram();
		break;

	default:
		logerror(
				"%s: Unknown configuration command 0x%02X with argument 0x%02X\n",
				machine().describe_context(),
				m_config_cmd,
				data);
	}

	// gbcolor:vf12in1 immediately writes 0x07 without writing to 0x5000 in between
	m_config_cmd = 0x0100;
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_SLMULTI, device_gb_cart_interface, bus::gameboy::slmulti_device, "gb_rom_slmulti", "Game Boy SL Multi-Game Cartridge")
