// license:BSD-3-Clause
// copyright-holders:Curt Coder,Tomasz Slanina
/*

    Vegas Roulette
    World Games 1989

    LE1000

           6116  PGM  Z80    8255

                             8255  SW1

    GFX  6116                YM2149   SW3
                                      SW2
                        2148
                        2148
                        2148
                N4
    24MHz

    ---

    Driver by Curt Coder

TODO:

Find 'payout on' command to add simulator

Tomasz Slanina 20050225
 - colors (4bpp tiles and 3bpp palette ? something is wrong then ....)
 - 8255x2
 - ball sprite (maybe it's something else in real machine , not sprite)
   (hardcoded tile number and palette for now .. maybe x/y must be swapped)
   are writes to 8000/c000 related to sprite tile/pal ?

*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


namespace {

class vroulet_state : public driver_device
{
public:
	vroulet_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_generic_paletteram_8(*this, "paletteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_ball(*this, "ball") { }

	void vroulet(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_generic_paletteram_8;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_ball;

	tilemap_t *m_bg_tilemap = nullptr;

	void paletteram_w(offs_t offset, uint8_t data);
	void videoram_w(offs_t offset, uint8_t data);
	void colorram_w(offs_t offset, uint8_t data);
	void ppi8255_a_w(uint8_t data);
	void ppi8255_b_w(uint8_t data);
	void ppi8255_c_w(uint8_t data);

	TILE_GET_INFO_MEMBER(get_bg_tile_info);

	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vroulet_io_map(address_map &map) ATTR_COLD;
	void vroulet_map(address_map &map) ATTR_COLD;
};


/* video */


void vroulet_state::paletteram_w(offs_t offset, uint8_t data)
{
	/*
	 paletteram_xxxxBBBBGGGGRRRR_byte_be_w
	 but... each palette has 8 colors only, not 16 as expected...
	*/

	int i,j,a,b;
	m_generic_paletteram_8[offset]=data;
	for(i=0;i<32;i++)
	{
		for(j=0;j<16;j++)
		{
			a=m_generic_paletteram_8[((i*8+j)*2)&0xff ];
			b=m_generic_paletteram_8[((i*8+j)*2+1)&0xff ];
			m_palette->set_pen_color(i*16+j,pal4bit(b),pal4bit(b>>4),pal4bit(a));
		}
	}
}

void vroulet_state::videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void vroulet_state::colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(vroulet_state::get_bg_tile_info)
{
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] + ((attr & 0xc0) << 2);
	int color = attr & 0x1f;

	tileinfo.set(0, code, color, 0);
}

void vroulet_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(
			*m_gfxdecode,
			tilemap_get_info_delegate(*this, FUNC(vroulet_state::get_bg_tile_info)),
			TILEMAP_SCAN_ROWS,
			8, 8, 32, 32);
}

uint32_t vroulet_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	m_gfxdecode->gfx(0)->transpen(bitmap,cliprect, 0x320, 1, 0, 0,
		m_ball[1], m_ball[0] - 12, 0);
	return 0;
}

/* Memory Maps */

void vroulet_state::vroulet_map(address_map &map)
{
	map(0x0000, 0x5fff).rom();
	map(0x6000, 0x67ff).ram().share("nvram");
	map(0x8000, 0x8000).noprw();
	map(0x9000, 0x93ff).ram().w(FUNC(vroulet_state::videoram_w)).share("videoram");
	map(0x9400, 0x97ff).ram().w(FUNC(vroulet_state::colorram_w)).share("colorram");
	map(0xa000, 0xa001).ram().share("ball");
	map(0xb000, 0xb0ff).w(FUNC(vroulet_state::paletteram_w)).share("paletteram");
	map(0xc000, 0xc000).noprw();
}

void vroulet_state::vroulet_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x00, 0x01).w("aysnd", FUNC(ym2149_device::data_address_w));
	map(0x10, 0x13).rw("ppi8255_0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x80, 0x83).rw("ppi8255_1", FUNC(i8255_device::read), FUNC(i8255_device::write));
}

/* Input Ports */

static INPUT_PORTS_START( vroulet )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON9 ) PORT_NAME("Reset Machine")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("Payout")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Red")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Blue")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("2")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("6")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME("10")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME("20")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN2")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("DSWA")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x02, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x38, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0xc0, 0xc0, "Revolutions" )
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0xc0, "2" )
	PORT_DIPSETTING(    0x40, "3" )
	PORT_DIPSETTING(    0x00, "4" )

	PORT_START("DSWB")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Max Payout Adjust" )
	PORT_DIPSETTING(    0x02, "48" )
	PORT_DIPSETTING(    0x00, "60" )
	PORT_DIPNAME( 0x04, 0x04, "Extra Payout Control" )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "Red & Blue Select" )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xe0, 0xe0, "Winning %" )
	PORT_DIPSETTING(    0xc0, "50%" )
	PORT_DIPSETTING(    0xa0, "60%" )
	PORT_DIPSETTING(    0x80, "65%" )
	PORT_DIPSETTING(    0x60, "70%" )
	PORT_DIPSETTING(    0xe0, "75%" )
	PORT_DIPSETTING(    0x40, "80%" )
	PORT_DIPSETTING(    0x20, "90%" )
	PORT_DIPSETTING(    0x00, "100%" )
INPUT_PORTS_END

/* Graphics Layout */

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 0*4, 1*4, 2*4, 3*4, 4*4, 5*4, 6*4, 7*4 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

/* Graphics Decode Information */

static GFXDECODE_START( gfx_vroulet )
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout,    0, 32 )
GFXDECODE_END

/* PPI8255 Interface */

void vroulet_state::ppi8255_a_w(uint8_t data) {}// watchdog ?
void vroulet_state::ppi8255_b_w(uint8_t data) {}// lamps ?
void vroulet_state::ppi8255_c_w(uint8_t data) {}

/* Machine Driver */

void vroulet_state::vroulet(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4000000);   //???
	m_maincpu->set_addrmap(AS_PROGRAM, &vroulet_state::vroulet_map);
	m_maincpu->set_addrmap(AS_IO, &vroulet_state::vroulet_io_map);
	m_maincpu->set_vblank_int("screen", FUNC(vroulet_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);

	i8255_device &ppi0(I8255A(config, "ppi8255_0"));
	ppi0.in_pa_callback().set_ioport("IN0");
	ppi0.in_pb_callback().set_ioport("IN1");
	ppi0.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi1(I8255A(config, "ppi8255_1"));
	ppi1.out_pa_callback().set(FUNC(vroulet_state::ppi8255_a_w));
	ppi1.out_pb_callback().set(FUNC(vroulet_state::ppi8255_b_w));
	ppi1.out_pc_callback().set(FUNC(vroulet_state::ppi8255_c_w));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 2*8, 30*8-1);
	screen.set_screen_update(FUNC(vroulet_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_vroulet);
	PALETTE(config, m_palette).set_entries(128*4);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &aysnd(YM2149(config, "aysnd", 2000000));
	aysnd.port_a_read_callback().set_ioport("DSWA");
	aysnd.port_b_read_callback().set_ioport("DSWB");
	aysnd.add_route(ALL_OUTPUTS, "mono", 0.25);
}

/* ROMs */

ROM_START( vroulet )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "roul1.bin", 0x0000, 0x2000, CRC(0cff99e5) SHA1(0aa6680c4b8d780d71b3e6c6fe511f86f40abc4c) )
	ROM_LOAD( "roul2.bin", 0x2000, 0x2000, CRC(61924d9f) SHA1(8334d6825ed40e8347909817b8b73be97d23faf8) )
	ROM_LOAD( "roul3.bin", 0x4000, 0x2000, CRC(73dedff6) SHA1(d01c4fc99ac8dc03bd6e0cf779c221d403b2b648) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "roul.gfx", 0x0000, 0x8000, CRC(4e4f46d2) SHA1(efd00e2b564ff4a9013c67ffaaf91124089b310b) )
ROM_END

} // anonymous namespace


/* Game Driver */

GAME( 1989, vroulet, 0, vroulet, vroulet, vroulet_state, empty_init, ROT90, "World Game", "Vegas Roulette", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE )
