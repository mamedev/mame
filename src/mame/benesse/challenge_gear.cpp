// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/olms66k/msm665xx.h"
#include "video/sed1520.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "emupal.h"
#include "screen.h"
#include "softlist_dev.h"

/*

TSOP32 mask ROM:
The pinout nearly matches the 29LV040 (TSOP32) Flash ROM. It has no WE# and pin0 7 and 9 are swapped.

       --------
A11 --|01    32|-- OE#
A09 --|02    31|-- A10
A08 --|03    30|-- CE#
A13 --|04    29|-- Q07
A14 --|05    28|-- Q06
A17 --|06    27|-- Q05
A18 --|07    26|-- Q04
VCC --|08    25|-- Q03
NC  --|09    24|-- GND
A16 --|10    23|-- Q02
A15 --|11    22|-- Q01
A12 --|12    21|-- Q00
A07 --|13    20|-- A00
A06 --|14    19|-- A01
A05 --|15    18|-- A02
A04 --|16    17|-- A03
       --------

*/

namespace {

class challenge_gear_state : public driver_device
{
public:
	challenge_gear_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		//, m_maincpu(*this, "maincpu")
		, m_cart(*this, "cartslot")
	{ }

	void challenge_gear(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	NT7502_UPDATE_CB(lcd_update);

	void program_map(address_map &map) ATTR_COLD;
	void data_map(address_map &map) ATTR_COLD;

	//required_device<cpu_device> m_maincpu;
	required_device<generic_slot_device> m_cart;
	uint32_t m_rom_size = 0;
};


DEVICE_IMAGE_LOAD_MEMBER( challenge_gear_state::cart_load )
{
	m_rom_size = m_cart->common_get_size("rom");
	m_cart->rom_alloc(m_rom_size, GENERIC_ROM8_WIDTH, ENDIANNESS_LITTLE);
	m_cart->common_load_rom(m_cart->get_rom_base(), m_rom_size, "rom");

	return std::make_pair(std::error_condition(), std::string());
}

NT7502_UPDATE_CB( challenge_gear_state::lcd_update )
{
	// TODO
	return 0;
}

void challenge_gear_state::program_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("maincpu", 0);
}

void challenge_gear_state::data_map(address_map &map)
{
	map.global_mask(0xffff);
	map(0x8000, 0x8001).rw("lcdc", FUNC(nt7502_device::read), FUNC(nt7502_device::write));
}

static INPUT_PORTS_START( challenge_gear )
INPUT_PORTS_END

void challenge_gear_state::machine_start()
{
}

void challenge_gear_state::machine_reset()
{
}


void challenge_gear_state::challenge_gear(machine_config &config)
{
	/* basic machine hardware */
	msm665xx_device &maincpu(MSM66573(config, "maincpu", 4_MHz_XTAL)); // 100-lead square glob (type guessed); XL2 = 4.00H
	//maincpu.set_xtclk(32.768_kHz_XTAL);
	maincpu.set_addrmap(AS_PROGRAM, &challenge_gear_state::program_map);
	maincpu.set_addrmap(AS_DATA, &challenge_gear_state::data_map);
	// P6.5 <-> I2C SDA
	// P6.6 -> I2C SCL
	// P8 -> key matrix -> P10

	//I2C_24C16(config, "eeprom"); // "BR24C16FV" silkscreened on PCB

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(100, 64);
	screen.set_visarea_full();
	screen.set_palette("palette");
	screen.set_screen_update("lcdc", FUNC(nt7502_device::screen_update));

	PALETTE(config, "palette", palette_device::MONOCHROME_INVERTED);

	NT7502(config, "lcdc").set_screen_update_cb(FUNC(challenge_gear_state::lcd_update));

	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "challenge_gear_cart", "bin"));
	cartslot.set_device_load(FUNC(challenge_gear_state::cart_load));

	SOFTWARE_LIST(config, "cg_list").set_compatible("challenge_gear_cart");
}


ROM_START( chalgear )
	ROM_REGION(0x80000, "maincpu", 0)
	ROM_SYSTEM_BIOS(0, "rom501", "ms040501") // yellow function buttons (blue text), orange number buttons (blue text), patterned purple screen surround
	ROMX_LOAD("ms040501.ic2", 0x0000, 0x80000, CRC(fe8918ca) SHA1(e1e560d0bf811f3f8a6c122fbff065da4a30a9b6), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "rom402", "ms040402") // blue function buttons (yellow text), yellow numbers buttons (blue text), plain black screen surrounded
	ROMX_LOAD("ms040402.ic2", 0x0000, 0x80000, CRC(25aa24b2) SHA1(72c3f304289da531165d83c63f2c3632985428ba), ROM_BIOS(1))
ROM_END

} // Anonymous namespace

CONS( 2002, chalgear, 0,      0,      challenge_gear, challenge_gear, challenge_gear_state, empty_init, "Benesse Corporation", "Challenge Gear (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
