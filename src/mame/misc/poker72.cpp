// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/**************************************************************************************************

Poker Monarch

GFX ROMs contain
'Extrema Systems International Ltd'
as well as a logo for the company.

There are also 'Lucky Boy' graphics in various places.

TODO:
- NVRAM;
- Doesn't initialize properly, cfr. 892314112 for total out and coins out in Analyzer,
  not even with "clear page" button;
- Coin in doesn't work, accepts key-in from IPT_SERVICE1 only
  (hold it then press any of the IPT_POKER_HOLD);
- Crashes or starts corrupting graphics after some time of gameplay;
- $fe08-$fe0b looks PPI?
- Reads at $ffff are very suspicious, joins value with R register and puts it to $c3ff (!?)
- Requires ROM patch to avoid a tight loop at boot;
- Demo Sound enabled doesn't produce any sound (?)

Notes:
- On first boot it will moan about uninitailized RAM, enable service mode then
  press all five hold buttons at same time
  (game is fussy on being exactly pressed together)

**************************************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class poker72_state : public driver_device
{
public:
	poker72_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_vram(*this, "vram"),
		m_pal(*this, "pal"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_rombank(*this, "rombank")
	{ }

	void poker72(machine_config &config);

	void init_poker72();

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_pal;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_rombank;

	uint8_t m_tile_bank;

	void paletteram_w(offs_t offset, uint8_t data);
	void output_w(uint8_t data);
	void tile_bank_w(uint8_t data);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void prg_map(address_map &map) ATTR_COLD;
};


void poker72_state::video_start()
{
	m_tile_bank = 0;

	save_item(NAME(m_tile_bank));
}

uint32_t poker72_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int count = 0;

	for (int y = 0; y < 32; y++)
	{
		for (int x = 0; x < 64; x++)
		{
			int tile = ((m_vram[count + 1] & 0x0f) << 8 ) | (m_vram[count + 0] & 0xff); //TODO: tile bank
			int fx = (m_vram[count + 1] & 0x10);
			int fy = (m_vram[count + 1] & 0x20);
			int color = (m_vram[count + 1] & 0xc0) >> 6;

			tile |= m_tile_bank << 12;

			m_gfxdecode->gfx(0)->opaque(bitmap, cliprect, tile, color, fx, fy, x * 8, y * 8);

			count += 2;
		}
	}

	return 0;
}

void poker72_state::paletteram_w(offs_t offset, uint8_t data)
{
	m_pal[offset] = data;

	int const r = m_pal[(offset & 0x3ff) + 0x000] & 0x3f;
	int const g = m_pal[(offset & 0x3ff) + 0x400] & 0x3f;
	int const b = m_pal[(offset & 0x3ff) + 0x800] & 0x3f;

	m_palette->set_pen_color(offset & 0x3ff, pal6bit(r), pal6bit(g), pal6bit(b));
}

void poker72_state::output_w(uint8_t data)
{
	logerror("output_w: %02x\n", data);

/*  if ((data & 0xc) == 0xc)
        m_rombank->set_entry(2);
    else*/
	if (data & 8)
		m_rombank->set_entry(1);
	else
		m_rombank->set_entry(0);
}

void poker72_state::tile_bank_w(uint8_t data)
{
	m_tile_bank = (data & 4) >> 2;
}

void poker72_state::prg_map(address_map &map)
{
	map(0x0000, 0x7fff).bankr(m_rombank);
	map(0xc000, 0xdfff).ram(); //work ram
	map(0xe000, 0xefff).ram().share(m_vram);
	map(0xf000, 0xfbff).ram().w(FUNC(poker72_state::paletteram_w)).share(m_pal);
	map(0xfc00, 0xfdff).ram(); //???
	map(0xfe08, 0xfe08).portr("DSW1");
	map(0xfe09, 0xfe09).portr("IN1");
	map(0xfe0a, 0xfe0a).portr("IN2");
	map(0xfe0c, 0xfe0c).portr("DSW4");
	map(0xfe0d, 0xfe0d).portr("DSW5");
	map(0xfe0e, 0xfe0e).portr("DSW6");

	map(0xfe17, 0xfe17).nopr(); //irq ack
	map(0xfe20, 0xfe20).w(FUNC(poker72_state::output_w)); //output, irq enable?
	map(0xfe22, 0xfe22).w(FUNC(poker72_state::tile_bank_w));
	map(0xfe40, 0xfe40).rw("ay", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xfe60, 0xfe60).w("ay", FUNC(ay8910_device::address_w));

	map(0xff00, 0xffff).ram(); //??
/*
bp 13a

fe06 w
fe08 w
fe0b = 9b (ppi?)
fe24 w

01f9 : call 6399 --> cls

*/
}


static INPUT_PORTS_START( poker72 )
	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) // Z
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) // X
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) // C
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) // V
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) // B
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("M. Bet")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Black")

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Red")
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) // '2'
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_GAMBLE_BET ) // M
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_SERVICE1 ) // '9'
	PORT_SERVICE( 0x0080, IP_ACTIVE_LOW ) // F2

	// FIXME: defaults from manual
	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, "Operation Mode" ) PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x00, "Input Test" )
	PORT_DIPSETTING(    0x01, "Cross Hatch" )
	PORT_DIPSETTING(    0x02, "Color Test" )
	PORT_DIPSETTING(    0x03, "Game / Analyzer" ) // latter enables with Service Mode
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW1:3")
	PORT_DIPNAME( 0x08, 0x08, "Auto Hold" ) PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW1:5")
	PORT_DIPNAME( 0x60, 0x60, "Back of Cards" ) PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x20, "Type 2" )
	PORT_DIPSETTING(    0x40, "Type 3" )
	PORT_DIPSETTING(    0x60, "Type 4" )
	PORT_DIPNAME( 0x80, 0x80, "Keyboard" ) PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "A" ) // input type extended
	PORT_DIPSETTING(    0x00, "B" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, "Minimal Bet" ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "30" )
	PORT_DIPSETTING(    0x01, "50" )
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPNAME( 0x08, 0x08, "Maximum Bet" ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "100" )
	PORT_DIPSETTING(    0x00, "500" )
	PORT_DIPNAME( 0x30, 0x30, "Main Game" ) PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x30, "Type 1" )
	PORT_DIPSETTING(    0x20, "Type 2" )
	PORT_DIPSETTING(    0x10, "Type 3" )
	PORT_DIPSETTING(    0x00, "Type 4" )
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7")
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW3:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "Erotic Mode" ) PORT_DIPLOCATION("SW3:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW3:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW3:4")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW3:5")
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW3:6")
	PORT_DIPNAME( 0xc0, 0xc0, "Background Color" ) PORT_DIPLOCATION("SW3:7,8")
	PORT_DIPSETTING(    0x00, "Type 1" )
	PORT_DIPSETTING(    0x40, "Type 2" )
	PORT_DIPSETTING(    0x80, "Type 3" )
	PORT_DIPSETTING(    0xc0, "Type 4" )

	PORT_START("DSW4")
	PORT_DIPNAME( 0x03, 0x03, "Super Jackpot Percent" ) PORT_DIPLOCATION("SW4:1,2")
	PORT_DIPSETTING(    0x03, "0%" )
	PORT_DIPSETTING(    0x02, "2%" )
	PORT_DIPSETTING(    0x01, "5%" )
	PORT_DIPSETTING(    0x00, "10%" )
	PORT_DIPUNUSED_DIPLOC( 0x04, 0x04, "SW4:3")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "SW4:4")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW4:5")
	PORT_DIPNAME( 0xe0, 0xe0, "Credit Limit" ) PORT_DIPLOCATION("SW4:6,7,8")
	PORT_DIPSETTING(    0x00, DEF_STR( None ) )
	PORT_DIPSETTING(    0x20, "500000" )
	PORT_DIPSETTING(    0x40, "200000" )
	PORT_DIPSETTING(    0x60, "100000" )
	PORT_DIPSETTING(    0x80, "50000" )
	PORT_DIPSETTING(    0xa0, "20000" )
	PORT_DIPSETTING(    0xc0, "10000" )
	PORT_DIPSETTING(    0xe0, "5000" )

	PORT_START("DSW5")
	PORT_DIPNAME( 0x07, 0x07, "Coin In Rate" ) PORT_DIPLOCATION("SW5:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "5" )
	PORT_DIPSETTING(    0x04, "10" )
	PORT_DIPSETTING(    0x03, "20" )
	PORT_DIPSETTING(    0x02, "50" )
	PORT_DIPSETTING(    0x01, "100" )
	PORT_DIPSETTING(    0x00, "200" )
	PORT_DIPNAME( 0x08, 0x08, "Coin In Level" ) PORT_DIPLOCATION("SW5:4")
	PORT_DIPSETTING(    0x08, "Low" )
	PORT_DIPSETTING(    0x00, "High" )
	PORT_DIPNAME( 0x10, 0x10, "Hopper Switch Level" ) PORT_DIPLOCATION("SW5:5")
	PORT_DIPSETTING(    0x10, "Low" )
	PORT_DIPSETTING(    0x00, "High" )
	PORT_DIPNAME( 0x60, 0x60, "Hopper Limit" ) PORT_DIPLOCATION("SW5:6,7")
	PORT_DIPSETTING(    0x00, "400" )
	PORT_DIPSETTING(    0x20, "800" )
	PORT_DIPSETTING(    0x40, "1200" )
	PORT_DIPSETTING(    0x60, DEF_STR( None ) )
	PORT_DIPNAME( 0x80, 0x80, "Pay Out Mode" ) PORT_DIPLOCATION("SW5:8")
	PORT_DIPSETTING(    0x80, "Manual" )
	PORT_DIPSETTING(    0x00, "Automatic" )

	PORT_START("DSW6")
	// TODO: following takes the full SW6 bank for ID!
	PORT_DIPNAME( 0x01, 0x01, "Network Number ID" ) PORT_DIPLOCATION("SW6:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW6:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,4),
	8,
	{ RGN_FRAC(3,4), RGN_FRAC(3,4)+4, RGN_FRAC(2,4), RGN_FRAC(2,4)+4 ,RGN_FRAC(1,4),RGN_FRAC(1,4)+4, RGN_FRAC(0,4),RGN_FRAC(0,4)+4 },
	{ 0,1,2,3,8,9,10,11 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};



static GFXDECODE_START( gfx_poker72 )
	GFXDECODE_ENTRY( "tiles", 0, tiles8x8_layout, 0, 16 )
GFXDECODE_END


void poker72_state::machine_reset()
{
	m_rombank->set_entry(0);
}

void poker72_state::poker72(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 8000000);         // ? MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &poker72_state::prg_map);
	m_maincpu->set_vblank_int("screen", FUNC(poker72_state::irq0_line_hold));

	I80C51(config, "subcpu", 8000000); // actually 89C51, ? MHz

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0, 64*8-1, 0, 32*8-1);
	screen.set_screen_update(FUNC(poker72_state::screen_update));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_poker72);
	PALETTE(config, m_palette).set_entries(0x400);

	SPEAKER(config, "mono").front_center();

	ay8910_device &ay(AY8910(config, "ay", 8000000 / 8)); // ? Mhz
	ay.port_a_read_callback().set_ioport("DSW2");
	ay.port_b_read_callback().set_ioport("DSW3");
	ay.add_route(ALL_OUTPUTS, "mono", 0.50);
}



ROM_START( poker72 )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD( "27010.bin", 0x00000, 0x20000, CRC(62447341) SHA1(e442c1f834a5dd2ab6ab3bdd316dfa86f2ca6647) )

	ROM_REGION( 0x1000, "subcpu", 0 )
	ROM_LOAD( "89c51.bin", 0x00000, 0x1000, CRC(3fdd2148) SHA1(ea39a52482967268c7387aec77cfab1ae5c427fa) )

	ROM_REGION( 0x80000, "tiles", 0 )
	ROM_LOAD( "270135.bin", 0x00000, 0x20000, CRC(188c96ee) SHA1(7e883454cb080cdc82ce47ac92f51c8d45a55085) )
	ROM_LOAD( "270136.bin", 0x20000, 0x20000, CRC(f84c5068) SHA1(49178fe7b12f547a50879002236105a882767ebb) )
	ROM_LOAD( "270137.bin", 0x40000, 0x20000, CRC(310281d1) SHA1(c28f97bb3613c0b481ab6e16e215549c44b83c47) )
	ROM_LOAD( "270138.bin", 0x60000, 0x20000, CRC(d689313d) SHA1(8b9661b3af0e2ced7fe9fa487641e445ce7835b8) )
ROM_END

void poker72_state::init_poker72()
{
	uint8_t *rom = memregion("maincpu")->base();

	// configure and initialize bank 1
	m_rombank->configure_entries(0, 4, memregion("maincpu")->base(), 0x8000);
	m_rombank->set_entry(0);

	//rom[0x4a9] = 0x28;
	rom[0x4aa] = 0x00;
}

} // Anonymous namespace


GAME( 1995, poker72,  0,    poker72, poker72, poker72_state, init_poker72, ROT0, "Extrema Systems International Ltd.", "Poker Monarch (v2.50)", MACHINE_NOT_WORKING )
