// license:BSD-3-Clause
// copyright-holders:David Haywood

// unknown platform capable of running high resolution NES games (VT268?)

// is the dump good? can't locate any code, some 0x80000 blocks have what might be vectors at the end, but non-standard?

#include "emu.h"

#include "cpu/m6502/m6502.h"
#include "screen.h"
#include "emupal.h"
#include "speaker.h"


namespace {

class vt_unknown_state : public driver_device
{
public:
	vt_unknown_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void vt_unknown(machine_config &config);

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void vt_unknown_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
};

uint32_t vt_unknown_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void vt_unknown_state::machine_start()
{
}

void vt_unknown_state::machine_reset()
{
}

void vt_unknown_state::vt_unknown_map(address_map &map)
{
	map(0x000000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( vt_unknown )
INPUT_PORTS_END


static const gfx_layout test_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8*1) },
	8 * 1 * 8
};

static GFXDECODE_START( gfx_test )
	GFXDECODE_ENTRY( "maincpu", 0, test_layout,  0x0, 1  )
GFXDECODE_END


void vt_unknown_state::vt_unknown(machine_config &config)
{
	M6502(config, m_maincpu, 8000000); // unknown, assumed to be a 6502 based CPU as it has NES games, but could be emulating them (like the S+Core units, assuming this isn't one)
	m_maincpu->set_addrmap(AS_PROGRAM, &vt_unknown_state::vt_unknown_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_refresh_hz(60);
	m_screen->set_size(300, 262);
	m_screen->set_visarea(0, 256-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(vt_unknown_state::screen_update));

	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 0x200);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_test);
}

ROM_START( dgun2572 )
	ROM_REGION( 0x2000000, "maincpu", 0 ) // extra pins on subboard not marked, is this a good dump?
	ROM_LOAD( "dreamgearwgun.bin", 0x00000, 0x2000000, CRC(92b55c75) SHA1(c7b2319e304a4bf480b5dcd4f24af2e6ba834d0d) )
ROM_END

} // anonymous namespace


CONS( 201?, dgun2572,  0,         0,  vt_unknown,     vt_unknown, vt_unknown_state, empty_init, "dreamGEAR", "My Arcade Wireless Video Game Station 200-in-1 (DGUN-2572)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
