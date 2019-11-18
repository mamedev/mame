// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

        DEC DCT11-EM

        03/12/2010 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/t11/t11.h"
#include "emupal.h"
#include "screen.h"


class dct11em_state : public driver_device
{
public:
	dct11em_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void dct11em(machine_config &config);

private:
	virtual void machine_reset() override;
	virtual void video_start() override;

	void dct11em_mem(address_map &map);

	uint32_t screen_update_dct11em(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<t11_device> m_maincpu;
};

void dct11em_state::dct11em_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x0fff).ram();  // RAM
	map(0x2000, 0x2fff).ram();  // Optional RAM
	map(0xa000, 0xdfff).rom();  // RAM
}

/* Input ports */
static INPUT_PORTS_START( dct11em )
INPUT_PORTS_END


void dct11em_state::machine_reset()
{
}

void dct11em_state::video_start()
{
}

uint32_t dct11em_state::screen_update_dct11em(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void dct11em_state::dct11em(machine_config &config)
{
	/* basic machine hardware */
	T11(config, m_maincpu, 7500000); // 7.5MHz XTAL
	m_maincpu->set_initial_mode(0x1403);  /* according to specs */
	m_maincpu->set_addrmap(AS_PROGRAM, &dct11em_state::dct11em_mem);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(dct11em_state::screen_update_dct11em));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::MONOCHROME);
}

/* ROM definition */
ROM_START( dct11em )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	// Highest address line inverted
	ROM_LOAD16_BYTE( "23-213e4.bin", 0x8000, 0x2000, CRC(bdd82f39) SHA1(347deeff77596b67eee27a39a9c40075fcf5c10d))
	ROM_LOAD16_BYTE( "23-214e4.bin", 0x8001, 0x2000, CRC(b523dae8) SHA1(cd1a64a2bce9730f7a9177d391663919c7f56073))
	ROM_COPY("maincpu", 0x8000, 0xc000, 0x2000)
ROM_END

/* Driver */

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                          FULLNAME    FLAGS */
COMP( 1983, dct11em, 0,      0,      dct11em, dct11em, dct11em_state, empty_init, "Digital Equipment Corporation", "DCT11-EM", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
