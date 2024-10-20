// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/olms66k/msm665xx.h"

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
	virtual void video_start() override ATTR_COLD;

private:
	uint32_t screen_update_challenge_gear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	DECLARE_DEVICE_IMAGE_LOAD_MEMBER(cart_load);
	void program_map(address_map &map) ATTR_COLD;

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

void challenge_gear_state::video_start()
{
}

uint32_t challenge_gear_state::screen_update_challenge_gear(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void challenge_gear_state::program_map(address_map &map)
{
	map(0x00000, 0x7ffff).rom().region("maincpu", 0);
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
	msm665xx_device &maincpu(MSM66573(config, "maincpu", 14'000'000));
	maincpu.set_addrmap(AS_PROGRAM, &challenge_gear_state::program_map);

	/* video hardware - this is incorrect, but it does have a screen */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 256-16-1);
	screen.set_screen_update(FUNC(challenge_gear_state::screen_update_challenge_gear));

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

CONS( 2002, chalgear, 0,      0,      challenge_gear, challenge_gear, challenge_gear_state, empty_init, "Benesse Corporation", "Challenge Gear (Japan)", MACHINE_IS_SKELETON )
