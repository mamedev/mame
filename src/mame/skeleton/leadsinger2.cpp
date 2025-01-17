// license:BSD-3-Clause
// copyright-holders:David Haywood
/******************************************************************************

-------- Label on product --------

Enter Tech
Music Video Karaoke
MODEL: LS-K2

-------- Boot Screen --------

MVK
Music Video / MP3 Karaoke
Leadsinger II
www.leadsinger.com

-------- Main SoC --------

SUNPLUS
SPHE8104AW
0729-H
0001697

-------- ROM --------

EXCELSEMI
ES29LV800EB-70TG
0720

-------- RAM --------

SAMSUNG 710
K4S641632K-U75

-------- Other --------

27Mhz Xtal
Card Slot

*******************************************************************************/

#include "emu.h"

#include "screen.h"
#include "speaker.h"

namespace {

class leadsng2_state : public driver_device
{
public:
	leadsng2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_screen(*this, "screen")
	{ }

	void leadsng2(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};

uint32_t leadsng2_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void leadsng2_state::machine_start()
{

}

void leadsng2_state::machine_reset()
{
}

static INPUT_PORTS_START( leadsng2 )
INPUT_PORTS_END

void leadsng2_state::leadsng2(machine_config &config)
{
	// unknown CPU core

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(leadsng2_state::screen_update));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();
}

ROM_START( leadsng2 )
	ROM_REGION( 0x100000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "ls-k2_es29lv800eb_004a225b.bin", 0x000000, 0x100000, CRC(e70f1e1f) SHA1(5aa3187adffcba5bd4a9e3e89a1c210d7f1a978e) )
ROM_END

} // anonymous namespace

CONS( 200?, leadsng2,       0,              0,      leadsng2, leadsng2, leadsng2_state, empty_init, "Enter Tech", "Leadsinger II (LS-K2)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
