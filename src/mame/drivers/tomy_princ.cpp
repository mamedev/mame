// license:BSD-3-Clause
// copyright-holders:David Haywood

/************************************************************************

 Prin-C use a Fujitsu MB90611A MCU (F2MC-16L)

************************************************************************/

#include "emu.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/f2mc16/mb9061x.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

class tomy_princ_state : public driver_device
{
public:
	tomy_princ_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag)
		, m_cart(*this, "cartslot")
		, m_screen(*this, "screen")
		, m_maincpu(*this, "maincpu")
	{ }

	void tomy_princ(machine_config &config);

protected:

private:
	required_device<generic_slot_device> m_cart;
	required_device<screen_device> m_screen;
	required_device<f2mc16_device> m_maincpu;

	void princ_map(address_map &map);

	uint32_t screen_update_tomy_princ(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


uint32_t tomy_princ_state::screen_update_tomy_princ(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void tomy_princ_state::princ_map(address_map &map)
{
	map(0xf00000, 0xffffff).rom().region("maincpu", 0x00000);
}

static INPUT_PORTS_START( tomy_princ )
INPUT_PORTS_END

void tomy_princ_state::tomy_princ(machine_config &config)
{
	// MB90611A microcontroller, F2MC-16L architecture
	MB90611A(config, m_maincpu, 16_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tomy_princ_state::princ_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	m_screen->set_screen_update(FUNC(tomy_princ_state::screen_update_tomy_princ));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);

	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "princ_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("princ");
}

ROM_START( princ )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD("29f800t.u4", 0x00000, 0x100000, CRC(30b6b864) SHA1(7ada3af85dd8dd3f95ca8965ad8e642c26445293))
ROM_END

COMP( 1996?, princ,    0,       0,      tomy_princ,    tomy_princ, tomy_princ_state, empty_init, "Tomy", "Prin-C", MACHINE_IS_SKELETON )
