// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

Track & Field Challenge TV Game
https://www.youtube.com/watch?v=wjn1lLylqog

Uses epoxy blobs for CPU etc.
These have been identified as Winbond 2005 BA5962 (large glob) + Winbond 200506 BA5934 (smaller glob)
seems to be G65816 derived with custom vectors?

PCB               Game
TV0001 R1.1       My First DDR
TV0002 R1.0       Track & Field

DDR & TF PCBs look identical, all the parts are in the same place, the traces are the same, and the silkscreened part # for resistors and caps are the same.

currently dies after call at

00:AE85: LDA $0b
00:AE87: TAX
00:AE88: LDA $0d
00:AE8A: JSL $00a044

00:A044: SEP #$20
00:A046: PHA
00:A047: REP #$20
00:A049: DEX
00:A04A: PHX
00:A04B: RTL

which pushes some values onto the stack, then RTLs to them (but the values at $0b and $0d at both 00, so it jumps to 0 and dies)

*/

#include "emu.h"

#include "cpu/g65816/g65816.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"

class trkfldch_state : public driver_device
{
public:
	trkfldch_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void trkfldch(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;

	uint32_t screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void trkfldch_map(address_map &map);

	DECLARE_READ8_MEMBER(unk_7804_read);
	DECLARE_READ8_MEMBER(unk_7805_read);
};

void trkfldch_state::video_start()
{
}

uint32_t trkfldch_state::screen_update_trkfldch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}


READ8_MEMBER(trkfldch_state::unk_7804_read)
{
	return 0xff;
}

READ8_MEMBER(trkfldch_state::unk_7805_read)
{
	return 0xff;
}

void trkfldch_state::trkfldch_map(address_map &map)
{
	map(0x000000, 0x003fff).ram();

	map(0x006800, 0x006cff).ram();

	map(0x007000, 0x0072ff).ram();

	// 7800 - 78xx look like registers?
	map(0x007804, 0x007804).r(FUNC(trkfldch_state::unk_7804_read));
	map(0x007805, 0x007805).r(FUNC(trkfldch_state::unk_7805_read));

	map(0x008000, 0x3fffff).rom().region("maincpu", 0x000000); // good for code mapped at 008000 and 050000 at least
}

static INPUT_PORTS_START( trkfldch )
INPUT_PORTS_END

// dummy, doesn't appear to be tile based
static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0, 8, 16, 24, 32, 40, 48, 56 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	64*8
};

static GFXDECODE_START( gfx_trkfldch )
	GFXDECODE_ENTRY( "maincpu", 0, tiles8x8_layout, 0, 1 )
GFXDECODE_END

void trkfldch_state::machine_start()
{
}

void trkfldch_state::machine_reset()
{
	uint8_t *rom = memregion("maincpu")->base();

	int vector = 0xe;

	/* what appears to be a table of vectors apepars at the START of ROM, maybe this gets copied to RAM, maybe used directly?
	0: (invalid)
	1: (invalid)
	2: 0xA2C6
	3: 0xA334
	4: 0xA300
	5: 0xA2E0
	6: 0xA2B9
	7: 0xA2ED   // possible irq vector pointer, THIS IS NOT THE BOOT CODE!
	8: 0xA2D3
	9: 0xA327
	a: 0xA30D
	b: 0x6000
	c: 0xA31A
	d: 0xA2AC
	e: 0xA341
	f: (invalid)
	*/

	uint16_t addr = (rom[vector * 2 + 1] << 8) | (rom[vector * 2]);

	m_maincpu->set_state_int(1, addr);
}

void trkfldch_state::trkfldch(machine_config &config)
{
	/* basic machine hardware */
	G65816(config, m_maincpu, 20000000);
	//m_maincpu->set_addrmap(AS_DATA, &tv965_state::mem_map);
	m_maincpu->set_addrmap(AS_PROGRAM, &trkfldch_state::trkfldch_map);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(320, 240);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(trkfldch_state::screen_update_trkfldch));
	m_screen->set_palette("palette");


	GFXDECODE(config, m_gfxdecode, "palette", gfx_trkfldch); // dummy
	PALETTE(config, "palette").set_format(palette_device::xRGB_444, 0x100).set_endianness(ENDIANNESS_BIG); // dummy
}

ROM_START( trkfldch )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "trackandfield.bin", 0x000000, 0x400000,  CRC(f4f1959d) SHA1(344dbfe8df1897adf77da6e5ca0435c4d47d6842) )
ROM_END

ROM_START( my1stddr )
	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "myfirstddr.bin", 0x000000, 0x400000, CRC(2ef57bfc) SHA1(9feea5adb9de8fe17e915f3a037e8ddd70e58ae7) )
ROM_END


CONS( 2007, trkfldch,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "Track & Field Challenge", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 2006, my1stddr,  0,          0,  trkfldch, trkfldch,trkfldch_state,      empty_init,    "Konami",             "My First Dance Dance Revolution (US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // Japan version has different songs

