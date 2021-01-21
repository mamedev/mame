// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

CPUs
QTY     Type    clock   position    function
1x  MC68HC000FN10       u3  16/32-bit Microprocessor - main
1x  u6295       u98     4-Channel Mixing ADCPM Voice Synthesis LSI - sound
1x  HA17358         u101    Dual Operational Amplifier - sound
1x  TDA2003         u104    Audio Amplifier - sound
1x  oscillator  24.000MHz   osc1
ROMs
QTY     Type    position    status
2x  M27C1001    2,3     dumped
1x  M27C2001    1   dumped
3x  M27C4001    4,5,6   dumped
RAMs
QTY     Type    position
11x     LH52B256-10PLL  u16a,u17a,u27,u28,u29,u30,u39,u40,u74,u75,u76
PLDs
QTY     Type    position    status
1x  ATF20V8B-15PC   u37     read protected
2x  A40MX04-F-PL84  u83,u86     read protected
Others

1x 28x2 JAMMA edge connector
1x pushbutton (SW1)
1x trimmer (volume) (VR1)
4x 8x2 switches DIP (SW1,SW2,SW3,SW4)
1x battery 3.6V (BT1)
Notes

PCB silkscreened: "MADE IN TAIWAN YONSHI PCB NO-006F"

TODO:
- currently stops after passing SRAM text;
- driver is skeleton-ish, video emulation is at the bare minimum to see what's going on;
- decryption is probably incomplete.
*/


#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/nvram.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class jungleyo_state : public driver_device
{
public:
	jungleyo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_gfxdecode(*this, "gfxdecode")
		, m_fg_videoram(*this, "fg_videoram")
	{ }

	void jungleyo(machine_config &config);

	void init_jungleyo();

protected:
	virtual void video_start() override;

private:
	// video-related
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	void fg_videoram_w(offs_t offset, uint16_t data);

	void main_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint16_t> m_fg_videoram;

	tilemap_t *m_fg_tilemap;
};


TILE_GET_INFO_MEMBER(jungleyo_state::get_fg_tile_info)
{
	uint16_t code = m_fg_videoram[tile_index];
	tileinfo.set(1, code, 0, 0);
}

void jungleyo_state::fg_videoram_w(offs_t offset, uint16_t data)
{
	m_fg_videoram[offset / 2] = data;
	m_fg_tilemap->mark_tile_dirty(offset / 2);
}

void jungleyo_state::video_start()
{
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(jungleyo_state::get_fg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 32);
}

uint32_t jungleyo_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_fg_tilemap->draw(screen, bitmap, cliprect, 0, 1);

	return 0;
}


void jungleyo_state::main_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom().region("maincpu", 0);
	map(0xa00000, 0xa00fff).ram();
	map(0xb00000, 0xb0ffff).ram();
	map(0xb10000, 0xb1ffff).ram();
	map(0xb20000, 0xb2ffff).ram();
	map(0xb80000, 0xb81fff).ram();
	map(0xb90000, 0xb91fff).ram().share(m_fg_videoram).w(FUNC(jungleyo_state::fg_videoram_w));
	map(0xba0000, 0xba1fff).ram();
	map(0xff0000, 0xff7fff).ram();
	map(0xff8000, 0xffffff).ram().share("nvram");
}


static INPUT_PORTS_START( jungleyo )
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

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW4:8")
INPUT_PORTS_END


static const gfx_layout jungleyo_layout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64 },
	8*64
};

static const gfx_layout jungleyo16_layout =
{
	8,32,
	RGN_FRAC(1,1),
	8,
	{ 0,1,2,3,4,5,6,7 },
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
		8*64, 9*64,10*64,11*64,12*64,13*64,14*64,15*64,
		16*64,17*64,18*64,19*64,20*64,21*64,22*64,23*64,
		24*64,25*64,26*64,27*64,28*64,29*64,30*64,31*64 },
	8*64*4
};

static GFXDECODE_START( gfx_jungleyo )
	GFXDECODE_ENTRY( "reelgfx", 0, jungleyo16_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx2", 0, jungleyo_layout,   0x0, 2  )
	GFXDECODE_ENTRY( "gfx3", 0, jungleyo_layout,   0x0, 2  )
GFXDECODE_END


void jungleyo_state::jungleyo(machine_config &config)
{
	M68000(config, m_maincpu, 24_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &jungleyo_state::main_map);
	m_maincpu->set_vblank_int("screen", FUNC(jungleyo_state::irq1_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	GFXDECODE(config, m_gfxdecode, "palette", gfx_jungleyo);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(jungleyo_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x200);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	okim6295_device &oki(OKIM6295(config, "oki", 24_MHz_XTAL / 20, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.47);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.47);
}


ROM_START( jungleyo )
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code, encrypted
	ROM_LOAD16_BYTE( "jungle_=record=_rom3_vi3.02.u15", 0x00000, 0x20000, CRC(7c9f431e) SHA1(fb3f90c4fe59c938f36b30c5fa3af227031e7d7a) )
	ROM_LOAD16_BYTE( "jungle_=record=_rom2_vi3.02.u14", 0x00001, 0x20000, CRC(f6a71260) SHA1(8e48cbb9d701ad968540244396820359afe97c28) )

	ROM_REGION( 0x40000, "oki", 0 )
	ROM_LOAD( "jungle_rom1.u99", 0x00000, 0x40000, CRC(05ef5b85) SHA1(ca7584646271c6adc7880eca5cf43a412340c522) )

	ROM_REGION( 0x80000, "reelgfx", 0 )
	ROM_LOAD( "jungle_rom4.u58", 0x000000, 0x80000, CRC(2f37da94) SHA1(6479e3bcff665316903964286d72df9822c05485) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "jungle_rom5.u59", 0x000000, 0x80000, CRC(0ccd9b94) SHA1(f209c7e15967be2e43be018aca89edd0c311503e) )

	ROM_REGION( 0x80000, "gfx3", 0 )
	ROM_LOAD( "jungle_rom6.u60", 0x000000, 0x80000, CRC(caab8eb2) SHA1(472ca9f396d7c01a1bd03485581cfae677a3b365) )
ROM_END


void jungleyo_state::init_jungleyo() // TODO: just a start, gives correct (?) strings at 0x2000-0x2fff, 0x7000-0x9000 and 0xc000-0xd000 ranges. From 0x10000 onwards needs something different
{
	uint16_t *src = (uint16_t *)memregion("maincpu")->base();

	for (int i = 0x00000; i < 0x40000 / 2; i++)
		src[i] = bitswap<16>(src[i] ^ 0x00ff, 8, 10, 15, 11, 9, 14, 12, 13, 6, 4, 2, 7, 3, 0, 1, 5); // TODO: possibly the bitswap and XOR are applied per byte and not per word, verify when more is discovered

	// hack these until better understood (still wrong values)
	src[0x000/2] = 0x0000;
	src[0x002/2] = 0x0000;
	src[0x004/2] = 0x0000;
	src[0x006/2] = 0x01fa;

	/*char filename[256];
	sprintf(filename,"p_decrypted_%s", machine().system().name);
	FILE *fp = fopen(filename, "w+b");
	if (fp)
	{
	    fwrite(src, 0x40000, 1, fp);
	    fclose(fp);
	}*/
}

} // Anonymous namespace


GAME( 1999, jungleyo, 0, jungleyo, jungleyo, jungleyo_state, init_jungleyo, ROT0, "Yonshi", "Jungle (VI3.02)", MACHINE_IS_SKELETON ) // version 3.02 built on 2001/02/09, there's copyright both for Yonshi and Global in strings
