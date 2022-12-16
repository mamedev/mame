// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

 GBCK003 multi-game board soldered inside some units

 Used with a menu program for selecting between multiple games designed for
 MBC1 cartridges.  Supports up to 8 MiB ROM (512 16 KiB pages).  Included
 games can use up to 4 MiB ROM (256 16 KiB pages).

 The menu program sets up code in WRAM to switch to the selected game's ROM
 and exit configuration mode.  The configuration values are read directly
 from ROM, so the configuration is presumably applied when exiting
 configuration mode.  Leaving configuration mode presumably asserts the
 reset line on the cartridge edge connector to get out of CGB mode for games
 that expect DMG mode (e.g. Contra has all-black player sprites if the
 system remains in CGB mode).

 The write to 0x7B02 that presumably exits configuration mode is followed by
 code to do the following (SP contains 0xFFFE at this point):
 * Set HL to 0x000D
 * Set DE to 0xFF56
 * Set AF to 0x1180 (Znhc)
 * Set BC to 0x0000
 * Jump to 0x0100

 This doesn't match the state set up by any official Nintendo bootstrap
 programs.  This code is probably left over from earlier multi-game
 cartridges that lacked CGB support and didn't reset the system when
 starting a game.

 0x0000-3FFF   R  - Low ROM bank.
 0x4000-7FFF   R  - High ROM bank.

 0x2000-2FFF   W  - Set ROM A21-A14 for 0x4000-7FFF as allowed by mask.
 0x7B00      * W  - Set ROM A22-A15 as allowed by mask.
 0x7B01      * W  - Set mask for A22-A15 - high for bits controlled by
                    0x7B00, low for bits controlled by 0x2000-2FFF.
 0x7B02      * W  - Software writes 0xF0 to exit configuration mode.

 * Registers only writable in configuration mode.

 TODO:
 * Are large games that require coarse ROM banking supported?
 * Is static RAM supported?  If so, is it banked?
 * What actual ranges are the configuration registers accessible in?
 * This isn't really a cartridge - does it belong elsewhere?

 ***************************************************************************/

#include "emu.h"
#include "gbck003.h"

#include "cartbase.ipp"

#include <string>

//#define VERBOSE 1
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


namespace bus::gameboy {

namespace {

class gbck003_device : public flat_ram_device_base<mbc_dual_uniform_device_base>
{
public:
	gbck003_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	virtual image_init_result load(std::string &message) override ATTR_COLD;

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void bank_switch_inner(u8 data);
	void bank_switch_outer(u8 data);
	void bank_set_mux(u8 data);
	void exit_config_mode(u8 data);

	TIMER_CALLBACK_MEMBER(do_reset);

	u16 bank_rom_entry_low() const noexcept
	{
		// TODO: is this affected by the multiplexer?
		return u16(m_bank_setting[0]) << 1;
	}

	u16 bank_rom_entry_high() const noexcept
	{
		return (u16(m_bank_setting[0] & m_bank_mux_rom) << 1) | (m_bank_setting[1] & ~(u16(m_bank_mux_rom) << 1));
	}

	u8 m_bank_setting[2];
	u8 m_bank_mux_rom;
	u8 m_config_mode;
};


gbck003_device::gbck003_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		u32 clock) :
	flat_ram_device_base<mbc_dual_uniform_device_base>(mconfig, GB_ROM_GBCK003, tag, owner, clock),
	m_bank_setting{ 0U, 0U },
	m_bank_mux_rom(0U),
	m_config_mode(0U)
{
}


image_init_result gbck003_device::load(std::string &message)
{
	// set up ROM/RAM
	set_bank_bits_rom(9);
	if (!check_rom(message))
		return image_init_result::FAIL;
	install_rom();
	install_ram();

	// install bank switch handlers
	cart_space()->install_write_handler(
			0x2000, 0x3fff,
			write8smo_delegate(*this, FUNC(gbck003_device::bank_switch_inner)));
	cart_space()->install_write_handler(
			0x7b00, 0x7b00,
			write8smo_delegate(*this, FUNC(gbck003_device::bank_switch_outer)));
	cart_space()->install_write_handler(
			0x7b01, 0x7b01,
			write8smo_delegate(*this, FUNC(gbck003_device::bank_set_mux)));
	cart_space()->install_write_handler(
			0x7b02, 0x7b02,
			write8smo_delegate(*this, FUNC(gbck003_device::exit_config_mode)));

	// set initial power-on state
	set_bank_rom_low(0);
	set_bank_rom_high(1);

	// all good
	return image_init_result::PASS;
}


void gbck003_device::device_start()
{
	flat_ram_device_base<mbc_dual_uniform_device_base>::device_start();

	// this stuff must survive reset for getting out of CGB mode
	m_bank_setting[0] = 0U;
	m_bank_mux_rom = 0xff;
	m_config_mode = 1U;

	save_item(NAME(m_bank_setting));
	save_item(NAME(m_bank_mux_rom));
	save_item(NAME(m_config_mode));
}


void gbck003_device::device_reset()
{
	flat_ram_device_base<mbc_dual_uniform_device_base>::device_reset();

	// TODO: proper reset state
	m_bank_setting[1] = 1U;

	set_bank_rom_high(bank_rom_entry_high());
}


void gbck003_device::bank_switch_inner(u8 data)
{
	data &= 0x1f;
	m_bank_setting[1] = data ? data : 1;
	set_bank_rom_high(bank_rom_entry_high());
}


void gbck003_device::bank_switch_outer(u8 data)
{
	if (m_config_mode)
	{
		LOG(
				"%s: Set base page = 0x%03X (0x%06X)\n",
				machine().describe_context(),
				u16(data) << 1,
				u32(data) << 15);
		m_bank_setting[0] = data;
		set_bank_rom_low(bank_rom_entry_low());
		set_bank_rom_high(bank_rom_entry_high());
	}
	else
	{
		logerror(
				"%s: Write 0x7B00 = 0x%02X after disabling configuration mode\n",
				machine().describe_context(),
				data);
	}
}


void gbck003_device::bank_set_mux(u8 data)
{
	if (m_config_mode)
	{
		LOG(
				"%s: Set bank multiplexer = 0x%03X\n",
				machine().describe_context(),
				u16(data) << 1);
		m_bank_mux_rom = data;
		set_bank_rom_high(bank_rom_entry_high());
	}
	else
	{
		logerror(
				"%s: Write 0x7B01 = 0x%02X after disabling configuration mode\n",
				machine().describe_context(),
				data);
	}
}


void gbck003_device::exit_config_mode(u8 data)
{
	if (m_config_mode)
	{
		if (0xf0 == data)
		{
			LOG("%s: Configuration mode disabled, resetting system\n", machine().describe_context());
			m_config_mode = 0U;
			machine().scheduler().synchronize(timer_expired_delegate(FUNC(gbck003_device::do_reset), this));
		}
		else
		{
			logerror(
					"%s: Unknown write 0x7B02 = 0x%02X in configuration mode\n",
					machine().describe_context(),
					data);
		}
	}
	else
	{
		logerror(
				"%s: Write 0x7B02 = 0x%02X after disabling configuration mode\n",
				machine().describe_context(),
				data);
	}
}


TIMER_CALLBACK_MEMBER(gbck003_device::do_reset)
{
	machine().root_device().reset(); // TODO: expose reset line on cartridge interface
}

} // anonymous namespace

} // namespace bus::gameboy


DEFINE_DEVICE_TYPE_PRIVATE(GB_ROM_GBCK003, device_gb_cart_interface, bus::gameboy::gbck003_device, "gb_rom_gbck003", "Game Boy GBCK003 Multi-Game Board")
