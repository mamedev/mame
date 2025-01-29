// license:BSD-3-Clause
// copyright-holders:

/*
Excellent System's ES-8906B PCB

Main components:
R6502AP CPU
6116ALSP-12 RAM (near CPU)
20.0000 MHz XTAL (near CPU)
HD46505SP CRTC
Excellent ES-8712 custom
4x 6116ALSP-12 RAM (near ES-8712)
4x bank of 8 DIP switches (SW2-5)
reset button (SW1)
NE555P (near SW1)

TODO: everything
*/

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "sound/es8712.h"
#include "video/mc6845.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class es8906_state : public driver_device
{
public:
	es8906_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void es8906(machine_config &config);

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void program_map(address_map &map) ATTR_COLD;
};


void es8906_state::video_start()
{
}

uint32_t es8906_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


void es8906_state::program_map(address_map &map)
{
	map.unmap_value_high();

	map(0x0000, 0x07ff).ram();
	map(0x0800, 0x0800).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x0801, 0x0801).rw("crtc", FUNC(hd6845s_device::register_r), FUNC(hd6845s_device::register_w));
	map(0x1000, 0x2fff).ram();
	map(0x8000, 0xffff).rom();
}


static INPUT_PORTS_START( dream9 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("SW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("SW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")

	PORT_START("SW5")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW5:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW5:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW5:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW5:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW5:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW5:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW5:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW5:8")
INPUT_PORTS_END


static GFXDECODE_START( gfx_es8906 )
	GFXDECODE_ENTRY( "tiles1", 0, gfx_8x8x4_planar, 0, 16 )
	GFXDECODE_ENTRY( "tiles2", 0, gfx_8x8x4_planar, 0, 16 )
GFXDECODE_END


void es8906_state::es8906(machine_config &config)
{
	M6502(config, m_maincpu, 20_MHz_XTAL / 10); // TODO: divider not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &es8906_state::program_map);

	hd6845s_device &crtc(HD6845S(config, "crtc", 20_MHz_XTAL / 10)); // TODO: verify
	crtc.set_screen(m_screen);
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline(m_maincpu, 0);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO: everything
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(40*8, 32*8);
	m_screen->set_visarea(0*8, 40*8-1, 2*8, 30*8-1);
	m_screen->set_screen_update(FUNC(es8906_state::screen_update));
	m_screen->set_palette("palette");

	GFXDECODE(config, m_gfxdecode, "palette", gfx_es8906);
	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 256); // TODO

	SPEAKER(config, "speaker").front_center();

	ES8712(config, "essnd", 0);
}


ROM_START( dream9 ) // (C)1989 EXCELLENT SYSTEM SUPER 9 Ver1.52 PROGRAM by KAY/AMTECH Feb,6 1990, but pics show "Dream 9"
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "d5-00_excellent.3h", 0x00000, 0x10000, CRC(7c0ad390) SHA1(b173f7d4521a4bea4246b988e6b4fce179ac12bc) )

	ROM_REGION( 0x20000, "tiles1", 0 ) // all labels have "エクセレント システム" (means "Excellent System") before what's below
	ROM_LOAD( "9l.9l",   0x00000, 0x10000, CRC(63692c62) SHA1(ef06ffe4204b0c1e40f9685b62f403b94a9b6693) )
	ROM_LOAD( "11l.11l", 0x10000, 0x10000, CRC(9251d1a6) SHA1(ccce3240d825e7611a4e4412b4f37710d0957179) )

	ROM_REGION( 0x20000, "tiles2", 0 ) // idem
	ROM_LOAD( "12l.12l", 0x00000, 0x10000, CRC(25395909) SHA1(d5e8caac642847450896a855e293431ad6620a77) )
	ROM_LOAD( "14l.14l", 0x10000, 0x10000, CRC(45757954) SHA1(bda96f437e5b65058e2f068b73998bad4b0bd5a3) )

	ROM_REGION( 0x300, "proms", 0 )
	ROM_LOAD( "r.11b", 0x0000, 0x0100, CRC(b41df116) SHA1(5cae3e6c70775ad63bccbfc4847b4feee5b79ddf) )
	ROM_LOAD( "g.12b", 0x0100, 0x0100, CRC(70ac57cd) SHA1(fe238c363f09f20cdc1c6f7f6279201671664750) )
	ROM_LOAD( "b.13b", 0x0200, 0x0100, CRC(99458711) SHA1(0be119f928b9085e36056b9b6b7e8623765eef44) )
ROM_END

} // anonymous namespace


GAME( 1990, dream9, 0, es8906, dream9, es8906_state, empty_init, ROT0, "Excellent System", "Dream 9 (v1.52)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
