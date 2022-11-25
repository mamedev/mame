// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 Nintendo Game Boy MMM01 memory controller

 Designed to support multi-game cartridges.  Included games can vary in
 program ROM non-volatile RAM size requirements.  Starts in configuration
 mode, where coarse ROM and RAM page selection can be configured.  After
 leaving configuration mode, the only way to re-enter it is to reset the
 MMM01 chip.

 After leaving configuration mode, the behaviour will be similar to one of
 the two standard wiring arrangements for cartridges using the MBC1 chip.

 Supports up to 8 MiB ROM (512 16 KiB pages) and 128 KiB static RAM (16
 8 KiB pages) total.  Each included game can use up to 2 MiB ROM (128 16 KiB
 pages) and 8 KiB static RAM (one page), or up to 512 KiB ROM (32 16 KiB
 pages) and 32 KiB static RAM (4 8 KiB pages).  In configuration mode, up to
 32 KiB ROM and 128 KiB static RAM (16 8 KiB pages) can be used.

 The MMM01 controller only responds to A15-A13 and D6-D0, i.e. addresses are
 effectively masked with 0xE000 and data is effectively masked with 0x7F.
 In configuration mode, most of the control register bits that are unused by
 the MBC1 chip are repurposed.

 0x0000-3FFF R  - Low ROM bank.
 0x4000-7FFF R  - High ROM bank.
 0xA000-BFFF RW - Static RAM if enabled.

 0x0000-1FFF W  - -X------ * Exit configuration mode.
                  --XX---- * Static RAM bank fine protect.
                  ----XXXX   Static RAM enable.
 0x2000-3FFF W  - -XX----- * ROM bank mid (RA20-RA19) or static RAM bank
                             fine (AA14-AA13), depending on mode.
                  ---XXXXX   ROM bank fine (RA18-RA14).
 0x4000-5FFF W  - -X------ * Low bank coarse enable protect.
                  --XX---- * ROM bank coarse (RA22-RA21).
                  ----XX-- * Static RAM bank coarse (AA16-AA15).
                  ------XX   Static RAM bank fine (AA14-AA13) or ROM bank
                             mid (RA20-RA19), depending on mode.
 0x6000-7FFF W  - -X------ * ROM bank mid/RAM bank coarse select.
                  --XXXX-- * ROM bank fine protect.
                  -------X   Low bank coarse enable.

 * Fields only writable in configuration mode.

 ***************************************************************************/

#include "emu.h"
#include "mmm01.h"

#include "cartbase.ipp"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class mmm01_device : public mbc_ram_device_base<mbc_dual_uniform_device_base>
{
public:
	mmm01_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void enable_ram(u8 data);
	void bank_switch_fine(u8 data);
	void bank_switch_coarse(u8 data);
	void enable_bank_low_mid(u8 data);

	void update_bank_rom_low()
	{
		u16 entry;
		if (!m_config)
		{
			entry = (u16(m_bank_sel_rom_coarse) << 7) | (m_bank_sel_rom_fine & m_bank_prot_sel[1]);
			if (!m_mode_large_prg)
				entry |= m_bank_sel_mid[1] << 5;
			else if (m_bank_low_mid)
				entry |= m_bank_sel_mid[0] << 5;
		}
		else
		{
			entry = 0x01fe;
		}
		set_bank_rom_low(entry);
	}

	void update_bank_rom_high()
	{
		u16 entry;
		if (!m_config)
		{
			entry = (u16(m_bank_sel_rom_coarse) << 7) | m_bank_sel_rom_fine;
			if (!m_mode_large_prg)
				entry |= m_bank_sel_mid[1] << 5;
			else
				entry |= m_bank_sel_mid[0] << 5;
		}
		else
		{
			entry = 0x01fe | m_bank_sel_rom_fine;
		}
		if (!(m_bank_sel_rom_fine & ~m_bank_prot_sel[1]))
			entry |= 0x0001;
		set_bank_rom_high(entry);
	}

	void update_bank_ram()
	{
		u8 entry(m_bank_sel_ram_coarse << 2);
		if (m_mode_large_prg)
			entry |= m_bank_sel_mid[1];
		else if (m_bank_low_mid)
			entry |= m_bank_sel_mid[0];
		set_bank_ram(entry);
	}

	memory_view m_view_ram;

	u8 m_config;
	u8 m_mode_large_prg;
	u8 m_bank_sel_mid[2];
	u8 m_bank_sel_ram_coarse;
	u8 m_bank_sel_rom_coarse;
	u8 m_bank_sel_rom_fine;
	u8 m_bank_low_mid;
	u8 m_bank_prot_sel[2];
	u8 m_bank_prot_low_mid;
};


mmm01_device::mmm01_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	mbc_ram_device_base<mbc_dual_uniform_device_base>(mconfig, GB_ROM_MMM01, tag, owner, clock),
	m_view_ram(*this, "ram"),
	m_config(0U),
	m_mode_large_prg(0U),
	m_bank_sel_mid{ 0U, 0U },
	m_bank_sel_ram_coarse(0U),
	m_bank_sel_rom_coarse(0U),
	m_bank_sel_rom_fine(0U),
	m_bank_low_mid(0U),
	m_bank_prot_sel{ 0U, 0U },
	m_bank_prot_low_mid(0U)
{
}


image_init_result mmm01_device::load(std::string &message)
{
	// set up ROM and RAM
	set_bank_bits_rom(9);
	set_bank_bits_ram(4);
	if (!check_rom(message) || !check_ram(message))
		return image_init_result::FAIL;
	cart_space()->install_view(0xa000, 0xbfff, m_view_ram);
	install_rom();
	install_ram(m_view_ram[0]);

	// install memory controller handlers
	cart_space()->install_write_handler(
			0x0000, 0x1fff,
			write8smo_delegate(*this, FUNC(mmm01_device::enable_ram)));
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(mmm01_device::bank_switch_fine)));
	cart_space()->install_write_handler(
			0x4000, 0x5fff,
			write8smo_delegate(*this, FUNC(mmm01_device::bank_switch_coarse)));
	cart_space()->install_write_handler(
			0x6000, 0x6fff,
			write8smo_delegate(*this, FUNC(mmm01_device::enable_bank_low_mid)));

	// all good
	return image_init_result::PASS;
}


void mmm01_device::device_start()
{
	mbc_ram_device_base<mbc_dual_uniform_device_base>::device_start();

	save_item(NAME(m_config));
	save_item(NAME(m_mode_large_prg));
	save_item(NAME(m_bank_sel_mid));
	save_item(NAME(m_bank_sel_ram_coarse));
	save_item(NAME(m_bank_sel_rom_coarse));
	save_item(NAME(m_bank_sel_rom_fine));
	save_item(NAME(m_bank_low_mid));
	save_item(NAME(m_bank_prot_sel));
	save_item(NAME(m_bank_prot_low_mid));
}


void mmm01_device::device_reset()
{
	mbc_ram_device_base<mbc_dual_uniform_device_base>::device_reset();

	m_view_ram.disable();
	m_config = 1U;
	m_mode_large_prg = 0U;
	m_bank_sel_mid[0] = 0U;
	m_bank_sel_mid[1] = 0U;
	m_bank_sel_ram_coarse = 0U;
	m_bank_sel_rom_coarse = 0U;
	m_bank_sel_rom_fine = 0U;
	m_bank_low_mid = 0U;
	m_bank_prot_sel[0] = 0U;
	m_bank_prot_sel[1] = 0U;
	m_bank_prot_low_mid = 0U;

	set_bank_rom_low(0x01fe);
	set_bank_rom_high(0x01ff);
	set_bank_ram(0);
}


void mmm01_device::enable_ram(u8 data)
{
	if (0x0a == (data & 0x0f))
	{
		LOG("%s: Cartridge RAM enabled\n", machine().describe_context());
		m_view_ram.select(0);
	}
	else
	{
		LOG("%s: Cartridge RAM disabled\n", machine().describe_context());
		m_view_ram.disable();
	}

	if (m_config)
	{
		m_bank_prot_sel[0] = BIT(data, 4, 2);
		LOG(
				"%s: Mid bank protect = 0x%X\n",
				machine().describe_context(),
				m_bank_prot_sel[0]);
		if (BIT(data, 6))
		{
			LOG("%s: Configuration mode disabled\n", machine().describe_context());
			m_config = 0U;
			update_bank_rom_low();
			update_bank_rom_high();
		}
	}
}


void mmm01_device::bank_switch_fine(u8 data)
{
	m_bank_sel_rom_fine = (m_bank_sel_rom_fine & m_bank_prot_sel[1]) | (BIT(data, 0, 5) & ~m_bank_prot_sel[1]);
	LOG(
			"%s: Fine ROM bank = 0x%02X\n",
			machine().describe_context(),
			m_bank_sel_rom_fine);

	if (m_config)
	{
		m_bank_sel_mid[1] = BIT(data, 5, 2);
		LOG(
				"%s: Configuration mid bank = 0x%X\n",
				machine().describe_context(),
				m_bank_sel_mid[1]);
	}

	update_bank_rom_high();
	if (m_config && m_mode_large_prg)
		update_bank_ram();
}


void mmm01_device::bank_switch_coarse(u8 data)
{
	m_bank_sel_mid[0] = (m_bank_sel_mid[0] & m_bank_prot_sel[0]) | (BIT(data, 0, 2) & ~m_bank_prot_sel[0]);
	LOG(
			"%s: User mid bank = 0x%X\n",
			machine().describe_context(),
			m_bank_sel_mid[0]);

	if (m_config)
	{
		m_bank_sel_ram_coarse = BIT(data, 2, 2);
		m_bank_sel_rom_coarse = BIT(data, 4, 2);
		m_bank_prot_low_mid = BIT(data, 6);
		LOG(
				"%s: Coarse ROM bank = 0x%X, coarse RAM bank = 0x%X, low bank mid enable %s\n",
				machine().describe_context(),
				m_bank_sel_rom_coarse,
				m_bank_sel_ram_coarse,
				m_bank_prot_low_mid ? "locked" : "unlocked");
	}

	if (m_config || (!m_mode_large_prg && m_bank_low_mid))
		update_bank_ram();
}


void mmm01_device::enable_bank_low_mid(u8 data)
{
	if (!m_bank_prot_low_mid)
	{
		m_bank_low_mid = BIT(data, 0);
		LOG(
				"%s: Low bank mid %s\n",
				machine().describe_context(),
				m_bank_low_mid ? "enabled" : "disabled");
	}

	if (m_config)
	{
		m_bank_prot_sel[1] = BIT(data, 2, 4) << 1;
		m_mode_large_prg = BIT(data, 6);
		LOG(
				"%s: Fine ROM bank protect = 0x%02X, %s mode\n",
				machine().describe_context(),
				m_bank_prot_sel[1],
				m_mode_large_prg ? "large ROM" : "banked RAM");
	}

	if (!m_bank_prot_low_mid && !m_config)
		update_bank_rom_low();
	if (m_config)
		update_bank_rom_high();
	if (!m_bank_prot_low_mid || m_config)
		update_bank_ram();
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_MMM01, device_gb_cart_interface, bus::gameboy::mmm01_device, "gb_rom_mmm01", "Game Boy MMM01 Cartridge")
