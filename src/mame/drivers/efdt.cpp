// license:BSD-3-Clause
// copyright-holders: ElSemi, Roberto Fresca.
/**********************************************************************************

  El Fin Del Tiempo.
  Niemer S.A., 1981.

  Driver by Elsemi & Roberto Fresca.

  TODO:
  - missing starfield
  - missing background gradient?
  - look into moving to galaxian driver, or at least a derived class

***********************************************************************************

  Specs:

  1x Zilog Z8400A PS (Z80A) CPU.
  1x Fairchild F6802P CPU.

  1x AMD 8255A PPI.
  2x GI AY-3-8910.

  8x 2114 SRAM.

  1x 2516 EPROM.
  17x 2532 EPROMs.
  1x TBP28L22N (256 x 8-bits) bipolar PROM.

  2x 6-DIP switches banks.

  1x Xtal @ 14.318 MHz. (used for 6802 CPU & AY8910 sound devices).
  1x Xtal @ 18.4320 MHz. (used for Z80 CPU).


***********************************************************************************

  Main CPU memory map (Z80):

  0000-7FFF  R   ROM Space.
  8000-87FF  RW  RAM.
  8800-8803  RW  main soundlatch.
  9000-93FF  R   PORT("P1").
  9400-97FF  R   PORT("P2").
  A000-AFFF  RW  VRAM.
  B000-B000  W   Watchdog
  B400-B40F  RW  vregs1.
  B800-B80F  RW  vregs2.


  Audio CPU memory map (M6802):

  0000-007F  RW  RAM
  8000-83FF  RW  RAM
  9000-9000  RW  AY8910 #1, data_r & data_w
  9200-9200   W  AY8910 #1, address_w
  9400-9400  RW  AY8910 #1, data_r & data_w
  9600-9600   W  AY8910 #1, address_w
  E000-FFFF  R   ROM Space.


  There are 2 tilemaps:
    A 32x32 3BPP one with tile column scroll.
    A 32x32 1BPP one fixed.

  1BPP tilemap is always shown under the main tilemap (maybe some video register controls this).

  The video hardware supports 8 16x16 sprites, built from 4 consecutive 8x8 tiles, so the low 2 bits
  of the tile code are not specified in the sprite list.

  Also there is support for up to 8 bullets, currently drawn as a pixel with white colour.

  The game supports an unemulated starfield, with fading stars. It's currently unknown how it works.

  Tiles are stored in 6 banks of 256 tiles, the current bank applies to sprite and tile codes.


  VIDEO RAM STRUCTURE

  A000-A3FF - 3bpp Tile layer tile code
  A400-A7FF - ?? doesn't seem used
  A800-A83F - Tile column scroll on even bytes. Tile column palette on odd bytes.
  A840-A85F - 8 sprites. 4 bytes per sprite:
                  76543210
                0 YYYYYYYY   Y position of the sprite
                1 yxCCCCCC   x: xflip  y: yflip C: sprite code (of a 4 sprites block)
                2 -----PPP   P: Palette
                3 XXXXXXXX   X position of the sprite
  A860-A87F - Bullets. 4 bytes per bullet
                0 --------  Unknown (X pos high byte?)
                1 XXXXXXXX  X position of the bullet
                2 --------  Unknown (Y pos high byte?)
                3 YYYYYYYY  Y position of the bullet

  AC00-AFFF - 1bpp Tile layer tile code


  VIDEO REGISTERS
  Most video registers are unknown :(

  B400 - This is usually 1, but changes to 2 sometimes during scenes, also toggles 0x80 on and off in the interrupt handler before changing some regs (disable interrupt while on interrupt?)
  B401 - Always 00
  B402 - Always 00
  B403 - Starfield on/off. Pretty sure it is.
  B404 - Always FF
  B405 - Tile column scroll on/off. Almost, but not, because it's set to 0 on the galaxian stage, and it has scrolling on the top rows
  B406 - 1bpp tilemap on/off ?. Almost but not, because it's set to 0 on the title screen and the niemer text is on the 1bpp layer and must appear
  B407 - Tile bank

  B800 -\-- these 2 values contain ror(Tilebank,1), ror(Tilebank,2). Sprites bank? always set to the same than Tile bank reg
  B801 -/
  B802 -\   these 3 registers usually contain x, ror(x,1), ror(x,2) and are related to the 1bpp bitmap palette color. 2 is red, used for the "galaxian" level lines
  B803 -|-- during the initial scene of the attract, when the bomb explodes, they cycle 1,2,3,3,3,3,4,5,6 to cycle several colors, yellow, blue and red
  B804 -/   in the logo screen, when it says "Fin del tiempo", the "Niemer" letters must be orange/brown, these are set to 3. Set to 7 in the survival stage (red laser)
  B805 - Always 00
  B806 - Always 00
  B807 - Always 00

  Some video register dumps (B400):
  01 00 00 01 FF 01 01 00 - first attract screen, starfield on, scroll not used, 1bpp layer used (for explosion)
  01 00 00 01 FF 01 00 02 - first level (scrolling) , starfield on, scroll used, 1bpp layer not used
  01 00 00 01 FF 01 01 03 - 2nd level (survival), starfield on, scroll not used, 1bpp layer used (for laser)
  01 00 00 00 FF 01 01 04 - 3rd level (rescue), starfield off, scroll not used, 1bpp layer used (for tentacles)
  01 00 00 01 FF 00 01 01 - 4rd level (galaxian), starfield on, scroll used, 1bpp layer used (red lines)
  01 00 00 01 FF 01 01 05 - last level, starfield on, scroll not used, 1bpp layer not used
  01 00 00 00 FF 01 00 01 - Logo screen, starfield off, scroll not used, 1bpp layer used (niemer logo)
  01 00 00 00 FF 01 00 01 - Scoring, starfield off, scroll used, 1bpp layer not used

***********************************************************************************

  Pinout:

            Components | Solder side
  ------------------|--|--|------------------
                GND |01| A| GND
                GND |02| B| GND
                GND |03| C| GND
     KEY (not used) |04| D| KEY (not used)
                +5V |05| E| +5V
                +5V |06| F| +5V
       Coin Counter |07| G| Audio Out (not amplified)
                N/C |08| H| N/C
  Player 1 Button 2 |09| I| Coin Counter
             Coin 1 |10| J| Player 1 Button 1
    Start 2 Players |11| K| Coin 2
      Player 1 Left |12| L| Player 1 Right
        Player 1 Up |13| M| Start 1 Player
      Reset PCB (*) |14| N| Player 1 Down
              Green |15| O| Video Sync
               Blue |16| P| Red
                N/C |17| Q| -5V
               +12V |18| R| +12V

  (*) Needs to be tied to GND in order to boot.


**********************************************************************************/

#include "emu.h"
#include "includes/efdt.h"

#include "cpu/z80/z80.h"
#include "cpu/m6800/m6800.h"
#include "machine/watchdog.h"
#include "sound/ay8910.h"
#include "screen.h"
#include "speaker.h"

#define MAIN_CLOCK     18.432_MHz_XTAL
#define SEC_CLOCK      14.318181_MHz_XTAL

#define Z80_CLOCK      MAIN_CLOCK / 6    // 3.0720 MHz. (measured 3.06850).
#define F6802_CLOCK    SEC_CLOCK / 4     // 3.5795 MHz. (measured 3.57488).
#define AY8910_CLOCK   SEC_CLOCK / 8     // 1.789750 MHz. (measured 1.78745).


/*********************************************
*               Video Hardware               *
*********************************************/

void efdt_state::efdt_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("prom")->base();

	for (int i = 0; i < 256; ++i)
	{
		uint8_t const v = color_prom[i];

		uint8_t const g = (v & 7) << 5;
		uint8_t const r = ((v >> 3) & 7) << 5;
		uint8_t const b = ((v >> 6) & 3) << 6;

		if ((i & 7) == 0)
			palette.set_pen_color(i, 0);
		else
			palette.set_pen_color(i, rgb_t(r, g, b));
	}
}

TILE_GET_INFO_MEMBER(efdt_state::get_tile_info_0)
{
	int data = m_videoram[tile_index];

	int pal = m_videoram[0x800 + ((tile_index & 0x1f) * 2) + 1];

	//int code = data + (xtra << 8);
	int code = data + m_tilebank;

	SET_TILE_INFO_MEMBER(0, code, pal, 0);
}

TILE_GET_INFO_MEMBER(efdt_state::get_tile_info_1)
{
	int data = m_videoram[tile_index + 0xc00];

	int code = data;

	SET_TILE_INFO_MEMBER(1, code, 0x1c, 0);
}

void efdt_state::video_start()
{
	m_tilemap[0] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(efdt_state::get_tile_info_0)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
	m_tilemap[1] = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(efdt_state::get_tile_info_1)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);

	m_tilemap[0]->set_transparent_pen(0);
	m_tilemap[1]->set_transparent_pen(0);
}

WRITE_LINE_MEMBER(efdt_state::vblank_nmi_w)
{
	if (state && m_vlatch[0]->q0_r())
		m_maincpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
}

WRITE_LINE_MEMBER(efdt_state::nmi_clear_w)
{
	if (!state)
		m_maincpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

uint32_t efdt_state::screen_update_efdt(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int bank = m_vlatch[0]->q7_r() | (m_vlatch[1]->q0_r() << 1) | (m_vlatch[1]->q1_r() << 2);
	//startup tests require tile bank 1, but 0 is set to the vregs (reset sets it to 1?)

	m_tilebank = bank << 8;

	m_tilemap[0]->mark_all_dirty();
	m_tilemap[1]->mark_all_dirty();

	bitmap.fill(0, cliprect);

	m_tilemap[0]->set_scroll_cols(32);

	for (int i = 0; i < 32; ++i)
	{
		m_tilemap[0]->set_scrolly(i, m_videoram[2 * i + 0x800]);
	}

	m_tilemap[1]->draw(screen, bitmap, cliprect, 0, 0);
	m_tilemap[0]->draw(screen, bitmap, cliprect, 0, 0);

	gfx_element *gfx = m_gfxdecode->gfx(0);
	uint8_t *sprram = m_videoram + 0x840;

	//sprites
	for (int i = 0; i < 8; ++i)
	{
		uint8_t y = *sprram++;
		uint16_t code = *sprram++;
		uint8_t pal = *sprram++;
		uint8_t x = *sprram++;

		int xtra = code & 0xc0; //flip

		if (y == 0 && x == 0/* && code == 0 && pal == 0 */)
			continue;

		y = 256 - y - 16;

		code <<= 2;
		code &= 0xff;

		pal &= 7;

		code += m_tilebank;

		if ((xtra & 0xc0) == 0x40)
		{
			gfx->transpen(bitmap, cliprect, code + 1, pal, 1, 0, x, y, 0);
			gfx->transpen(bitmap, cliprect, code + 0, pal, 1, 0, x + 8, y, 0);
			gfx->transpen(bitmap, cliprect, code + 3, pal, 1, 0, x + 0, y + 8, 0);
			gfx->transpen(bitmap, cliprect, code + 2, pal, 1, 0, x + 8, y + 8, 0);
		}
		else if((xtra & 0xc0) == 0x80)
		{
			gfx->transpen(bitmap, cliprect, code + 2, pal, 0, 1, x, y, 0);
			gfx->transpen(bitmap, cliprect, code + 3, pal, 0, 1, x + 8, y, 0);
			gfx->transpen(bitmap, cliprect, code + 0, pal, 0, 1, x + 0, y + 8, 0);
			gfx->transpen(bitmap, cliprect, code + 1, pal, 0, 1, x + 8, y + 8, 0);

		}
		else if ((xtra & 0xc0) == 0xc0)
		{
			gfx->transpen(bitmap, cliprect, code + 3, pal, 1, 1, x, y, 0);
			gfx->transpen(bitmap, cliprect, code + 2, pal, 1, 1, x + 8, y, 0);
			gfx->transpen(bitmap, cliprect, code + 1, pal, 1, 1, x + 0, y + 8, 0);
			gfx->transpen(bitmap, cliprect, code + 0, pal, 1, 1, x + 8, y + 8, 0);
		}
		else
		{
			gfx->transpen(bitmap, cliprect, code + 0, pal, 0, 0, x, y, 0);
			gfx->transpen(bitmap, cliprect, code + 1, pal, 0, 0, x + 8, y, 0);
			gfx->transpen(bitmap, cliprect, code + 2, pal, 0, 0, x + 0, y + 8, 0);
			gfx->transpen(bitmap, cliprect, code + 3, pal, 0, 0, x + 8, y + 8, 0);

		}
	}

	//bullets
	uint8_t *shootram = m_videoram + 0x860;

	for (int i = 0; i < 8; ++i)
	{
		shootram++;
		uint8_t x = *shootram++;
		shootram++;
		uint8_t y = *shootram++;

		x = 256 - x;
		y = 256 - y - 8;

		bitmap.pix(x, y) = 7;
	}

	return 0;
}


/*********************************************
*           Memory map information           *
*********************************************/

void efdt_state::efdt_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0);
	map(0x8000, 0x87ff).ram();

	map(0x8800, 0x8803).rw(FUNC(efdt_state::main_soundlatch_r), FUNC(efdt_state::main_soundlatch_w));
//  map(0x8800, 0x8803).rw("soundlatch", FUNC(generic_latch_8_device::read), FUNC(generic_latch_8_device::write));  // TODO...

	map(0x9000, 0x93ff).portr("P1");
	map(0x9400, 0x97ff).portr("P2");

	map(0xa000, 0xafff).ram().share("videoram");
	map(0xb000, 0xb000).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0xb400, 0xb407).w(m_vlatch[0], FUNC(ls259_device::write_d0));
	map(0xb800, 0xb807).w(m_vlatch[1], FUNC(ls259_device::write_d0));
}

void efdt_state::efdt_snd_map(address_map &map)
{
	map(0x0000, 0x007f).ram();
	map(0x6000, 0x6000).nopw();
	map(0x7000, 0x7000).nopw();
	map(0x8000, 0x83ff).ram();

	map(0x9000, 0x9000).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x9200, 0x9200).w("ay1", FUNC(ay8910_device::address_w));

	map(0x9400, 0x9400).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x9600, 0x9600).w("ay2", FUNC(ay8910_device::address_w));

	map(0xe000, 0xffff).rom().region("audiocpu", 0);
}


/*********************************************
*                Input ports                 *
*********************************************/

static INPUT_PORTS_START( efdt )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN1 )

	PORT_START("P2")
	PORT_DIPNAME(0x03, 0x00, DEF_STR(Coinage))      PORT_DIPLOCATION("SW:1,2")
	PORT_DIPSETTING(0x00, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x01, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x02, DEF_STR(1C_3C))
	PORT_DIPSETTING(0x03, DEF_STR(2C_3C))
	PORT_DIPNAME(0x0c, 0x00, DEF_STR(Coinage))      PORT_DIPLOCATION("SW:3,4")
	PORT_DIPSETTING(0x00, DEF_STR(1C_1C))
	PORT_DIPSETTING(0x04, DEF_STR(1C_2C))
	PORT_DIPSETTING(0x08, DEF_STR(1C_3C))
	PORT_DIPSETTING(0x0c, DEF_STR(2C_3C))
	PORT_DIPNAME(0x30, 0x00, DEF_STR(Lives))        PORT_DIPLOCATION("SW:5,6")
	PORT_DIPSETTING(0x00, "3")
	PORT_DIPSETTING(0x10, "4")
	PORT_DIPSETTING(0x20, "5")
	PORT_DIPSETTING(0x30, "6")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_PLAYER(1)
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_PLAYER(1)

INPUT_PORTS_END


/*************************************************
*        Machine Start & Reset Routines          *
*************************************************/

void efdt_state::machine_start()
{
	// initial state...
}

void efdt_state::machine_reset()
{
}


/*********************************************
*               Sound Latches                *
*********************************************/

READ8_MEMBER(efdt_state::main_soundlatch_r)
{
	return m_soundlatch[offset];
}

WRITE8_MEMBER(efdt_state::main_soundlatch_w)
{
	m_soundlatch[offset] = data;
	switch (offset)
	{
		case 0:
			m_soundCommand = data;
			break;

		case 1:
			if (data & 8)
				m_soundControl |= 2;
			break;
	}
}

READ8_MEMBER(efdt_state::soundlatch_0_r)
{
	return m_soundCommand;
}

READ8_MEMBER(efdt_state::soundlatch_1_r)
{
	return m_soundControl;
}

WRITE8_MEMBER(efdt_state::soundlatch_0_w)
{
	//m_soundCommand;
}

WRITE8_MEMBER(efdt_state::soundlatch_1_w)
{
	if (!(data == 0xfd || data == 0xf5))
	{
//      int a = 1;
	}

	if(data & 4)
		m_soundControl &= ~2;

	//if (data & 8)
	//  m_soundControl &= ~1;
}

READ8_MEMBER(efdt_state::soundlatch_2_r)
{
	return m_soundControl;
}

READ8_MEMBER(efdt_state::soundlatch_3_r)
{
	return m_soundControl;
}

WRITE8_MEMBER(efdt_state::soundlatch_2_w)
{
	//m_soundCommand;
}

WRITE8_MEMBER(efdt_state::soundlatch_3_w)
{
	m_soundControl = data;
}


/*********************************************
*              Graphics Layouts              *
*********************************************/

static const gfx_layout tilelayout3bpp =
{
	8,8,
	RGN_FRAC(1,3),
	3,
	{ 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8 * 8
};

static const gfx_layout tilelayout1bpp =
{
	8,8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8 * 8
};


/**************************************************
*           Graphics Decode Information           *
**************************************************/

static GFXDECODE_START( gfx_efdt )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout3bpp, 0, 256*6 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout1bpp, 0, 256 )
GFXDECODE_END


/*********************************************
*              Machine Driver                *
*********************************************/

void efdt_state::efdt(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, Z80_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &efdt_state::efdt_map);

	M6802(config, m_audiocpu, F6802_CLOCK);
	m_audiocpu->set_addrmap(AS_PROGRAM, &efdt_state::efdt_snd_map);
	m_audiocpu->set_periodic_int(FUNC(efdt_state::irq0_line_hold), attotime::from_hz(F6802_CLOCK / 8192));

	LS259(config, m_vlatch[0]);
	m_vlatch[0]->q_out_cb<0>().set(FUNC(efdt_state::nmi_clear_w));

	LS259(config, m_vlatch[1]);

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_size(32*8, 32*8);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));
	screen.set_visarea(0, 32*8 - 1, 16, 30*8 - 1);
	screen.set_screen_update(FUNC(efdt_state::screen_update_efdt));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(efdt_state::vblank_nmi_w));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_efdt);
	PALETTE(config, m_palette, FUNC(efdt_state::efdt_palette), 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	ay8910_device &ay1(AY8910(config, "ay1", AY8910_CLOCK));
	ay1.add_route(ALL_OUTPUTS, "mono", 1.0);
	ay1.port_a_read_callback().set(FUNC(efdt_state::soundlatch_0_r));
	ay1.port_b_read_callback().set(FUNC(efdt_state::soundlatch_1_r));
	ay1.port_a_write_callback().set(FUNC(efdt_state::soundlatch_0_w));
	ay1.port_b_write_callback().set(FUNC(efdt_state::soundlatch_1_w));

	ay8910_device &ay2(AY8910(config, "ay2", AY8910_CLOCK));
	ay2.add_route(ALL_OUTPUTS, "mono", 1.0);
	ay2.port_a_read_callback().set(FUNC(efdt_state::soundlatch_2_r));
	ay2.port_b_read_callback().set(FUNC(efdt_state::soundlatch_3_r));
	ay2.port_a_write_callback().set(FUNC(efdt_state::soundlatch_2_w));
	ay2.port_b_write_callback().set(FUNC(efdt_state::soundlatch_3_w));
}


/*********************************************
*                  Rom Load                  *
*********************************************/

ROM_START( efdt )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD( "22805.b10", 0x0000, 0x1000, CRC(7e27df91) SHA1(0f2ef3563af5d6e0de77a6ac929dbd3802aea8f0))
	ROM_LOAD( "22806.b9",  0x1000, 0x1000, CRC(00cad810) SHA1(e93f9365227c1e5c2dea1325b379e78e37c0a953))
	ROM_LOAD( "22807.b8",  0x2000, 0x1000, CRC(8e51af2b) SHA1(ac496781fb599d26905aa28449eefc3959de0e9a))
	ROM_LOAD( "22808.b7",  0x3000, 0x1000, CRC(932bd16d) SHA1(3df0f222b0803da9021d3144ec7bc28453fdd947))
	ROM_LOAD( "22801.a10", 0x4000, 0x1000, CRC(ea646049) SHA1(bca30cb2dde8b5c78f6108cb9a43e0dce697f761))
	ROM_LOAD( "22802.a9",  0x5000, 0x1000, CRC(74457952) SHA1(f5f4ece564cbdb650204ccd5abdf39d0d3c595b3))

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD(   "1811.d8",   0x0000, 0x1000, CRC(0ff5d0c2) SHA1(93df487d3236284765dd3d690474c130464e3e27))
	ROM_LOAD(   "1812.d7",   0x1000, 0x1000, CRC(48e5a4ac) SHA1(9da4800215c91b2be9df3375f9601b19353c0ec0))

	ROM_REGION( 0x9000, "gfx1", 0 )
	ROM_LOAD( "12822.j3", 0x2000, 0x1000, CRC(f4d28a60) SHA1(bc1d7f4392805cd204ecfe9c3301990a7b710567) )
	ROM_LOAD( "12821.j4", 0x1000, 0x1000, CRC(b7ef75a6) SHA1(057f737fce63639879db95659de7d1d659058759) )
	ROM_LOAD( "12820.j5", 0x0000, 0x1000, CRC(70126c8d) SHA1(f380868f3afad2898c136b15210aaa6231f1d3c2) )
	ROM_LOAD( "12819.j6", 0x5000, 0x1000, CRC(2987b5b6) SHA1(0e57aae21e674155e407512f1edfb3d8b31d1fa3) )
	ROM_LOAD( "12818.j7", 0x4000, 0x1000, CRC(e0a61419) SHA1(65caa9e2700a0bec9e105dc763e9fe61dae8c3d6) )
	ROM_LOAD( "12817.j8", 0x3000, 0x1000, CRC(856a2537) SHA1(5e8f96239721a0dd64b37267bb3b343ac3034898) )
	ROM_LOAD( "12816.j9", 0x8000, 0x1000, CRC(69664044) SHA1(57465c4c37be2b4846b49a13dec9e354dabb155a) )
	ROM_LOAD( "12815.j10", 0x7000, 0x1000, CRC(abe7a7b6) SHA1(e3bc6aa3a741fcfa2eafb1464be3cb5437d5fd90) )
	ROM_LOAD( "12814.j11", 0x6000, 0x1000, CRC(6c06f746) SHA1(c7e80c5dde733e9ef520b9afa78bed902f04b74d) )

	ROM_REGION( 0x800, "gfx2", 0 )
	ROM_LOAD( "12813.h10", 0x0000, 0x0800, CRC(ea03c5a8) SHA1(7ce385b43a24cbbc780162ed89031d1cc1b0b9ef))

	ROM_REGION( 0x100, "prom", 0)
	ROM_LOAD( "ft1-tbp28l22n.bin", 0x000, 0x100, CRC(04d7b68c) SHA1(c13281280340ccf4f82a2a04bfe661a26253a05c))
ROM_END


/*********************************************
*                Game Drivers                *
*********************************************/

//    YEAR  NAME    PARENT  MACHINE   INPUT    STATE        INIT        ROT     COMPANY   FULLNAME            FLAGS
GAME( 1981, efdt,   0,      efdt,     efdt,    efdt_state,  empty_init, ROT90, "Niemer", "El Fin Del Tiempo", MACHINE_IMPERFECT_GRAPHICS )
