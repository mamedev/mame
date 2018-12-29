// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        UKNC

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


class uknc_state : public driver_device
{
public:
	uknc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void uknc(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_uknc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<k1801vm2_device> m_maincpu;

	void uknc_mem(address_map &map);
	void uknc_sub_mem(address_map &map);
};


void uknc_state::uknc_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();  // RAM
	map(0x8000, 0xffff).rom();  // ROM
}

void uknc_state::uknc_sub_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram();  // RAM
	map(0x8000, 0xffff).rom();  // ROM
}

/* Input ports */
static INPUT_PORTS_START( uknc )
INPUT_PORTS_END


void uknc_state::machine_reset()
{
}

void uknc_state::video_start()
{
}

uint32_t uknc_state::screen_update_uknc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void uknc_state::uknc(machine_config &config)
{
	/* basic machine hardware */
	K1801VM2(config, m_maincpu, 8000000);
	m_maincpu->set_initial_mode(0x8000);
	m_maincpu->set_addrmap(AS_PROGRAM, &uknc_state::uknc_mem);

	k1801vm2_device &subcpu(K1801VM2(config, "subcpu", 6000000));
	subcpu.set_initial_mode(0x8000);
	subcpu.set_addrmap(AS_PROGRAM, &uknc_state::uknc_sub_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(uknc_state::screen_update_uknc));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( uknc )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "uknc.rom", 0x8000, 0x8000, CRC(a1536994) SHA1(b3c7c678c41ffa9b37f654fbf20fef7d19e6407b))

	ROM_REGION( 0x10000, "subcpu", ROMREGION_ERASEFF )
ROM_END

/* Driver */

/*    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  STATE       INIT        COMPANY      FULLNAME  FLAGS */
COMP( 1987, uknc, 0,      0,      uknc,    uknc,  uknc_state, empty_init, "<unknown>", "UKNC",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
