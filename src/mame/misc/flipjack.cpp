// license:BSD-3-Clause
// copyright-holders:Angelo Salese, hap, AJR
/***************************************************************************

prelim notes:
Flipper Jack, by Jackson, 1983
probably a prequel to superwng, it has a Falcon logo on the pcb

xtal: 16mhz, 6mhz
cpu: 2*z80
sound: 2*ay8910
other: 8255 ppi, hd6845 crtc, 1 dipsw
ram: 2*8KB, 4*2KB
rom: see romdefs

TODO:
- remaining gfx/color issues
- measure clocks


--------------------------------------------------------------------
    DipSwitch Title   |  Function  | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 |
--------------------------------------------------------------------
     Demo Sounds      |     Off    |off|                           |
                      |     On     |on |                           |*
--------------------------------------------------------------------
       Coinage        |   1C / 1C  |   |off|                       |*
                      |   1C / 2C  |   |on |                       |
--------------------------------------------------------------------
     Drop Target      |     On     |       |off|                   |*
                      |     Off    |       |on |                   |
--------------------------------------------------------------------
     Cabinet Type     |  Cocktail  |           |off|               |
                      |  Upright   |           |on |               |*
--------------------------------------------------------------------
Additional Bonus Balls| Every 70K  |               |off|           |*
 after 1st bonus ball | Every 100K |               |on |           |
--------------------------------------------------------------------
   First Bonus Ball   |  100,000   |                   |off|       |*
                      |  200,000   |                   |on |       |
--------------------------------------------------------------------
  Bonus Ball Feature  |     On     |                       |off|   |*
                      |     Off    |                       |on |   |
--------------------------------------------------------------------
   Number of Balls    |     3      |                           |off|*
                      |     5      |                           |on |
--------------------------------------------------------------------


            Solder Side | Parts Side
________________________|___________________________
                 GND  | 1 |  GND
                 GND  | 2 |  GND
                 GND  | 3 |  GND
                 +5V  | 4 |  +5V
                 +5V  | 5 |  +5V
                +12V  | 6 |  +12V
                      | 7 |  Sound (+)
                      | 8 |  Sound (-)
                      | 9 |  Coin
                      | 10|
            2P Shoot  | 11|  1P Shoot
     2P Flipper Left  | 12|  1P Flipper Left
             2P Tilt  | 13|  1P Tilt
    2P Flipper Right  | 14|  1P Flipper Right
            2P Start  | 15|  1P Start
                      | 16|
                      | 17|
                      | 18|
                      | 19|
         Video Green  | 20|  Video Blue
          Video Sync  | 21|  Video Red
                 GND  | 22|  GRD


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK    16_MHz_XTAL
#define VIDEO_CLOCK     6_MHz_XTAL


class flipjack_state : public driver_device
{
public:
	flipjack_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_prgbank(*this, "prgbank"),
		m_fbram(*this, "fb_ram"),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_tiles(*this, "tiles"),
		m_playfield(*this, "playfield"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch")
	{
	}

	void flipjack(machine_config &config);

	void coin_nmi_w(int state);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;

	required_memory_bank m_prgbank;
	required_shared_ptr<uint8_t> m_fbram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_cram;
	required_region_ptr<uint8_t> m_tiles;
	required_region_ptr<uint8_t> m_playfield;

	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	uint8_t m_bank = 0;
	uint8_t m_layer = 0;

	void sound_nmi_ack_w(uint8_t data);
	void soundlatch_w(uint8_t data);
	void bank_w(uint8_t data);
	void layer_w(uint8_t data);
	void portc_w(uint8_t data);
	void flipjack_palette(palette_device &palette) const;
	MC6845_UPDATE_ROW(update_row);

	void main_io_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void sound_io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};


/***************************************************************************

  Video

***************************************************************************/

void flipjack_state::flipjack_palette(palette_device &palette) const
{
	// from PROM
	uint8_t const *const color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x40; i++)
	{
		palette.set_pen_color(2*i+1, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
		palette.set_pen_color(2*i+0, pal1bit(color_prom[i] >> 1), pal1bit(color_prom[i] >> 2), pal1bit(color_prom[i] >> 0));
	}

	// standard 3bpp for blitter
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i+0x80, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
}


MC6845_UPDATE_ROW(flipjack_state::update_row)
{
	const bool flip = !BIT(m_layer, 0);
	const uint16_t row_base = ((ma & 0x03e0) << 3 | (ra & 7) << 5) ^ (flip ? 0x1fff : 0);
	uint32_t *const pbegin = &bitmap.pix(y);
	uint32_t *const pend = &bitmap.pix(y, x_count * 8);

	std::fill(pbegin, pend, rgb_t::black());

	// draw playfield
	if (BIT(m_layer, 1))
	{
		uint16_t a1 = row_base;
		for (uint32_t *p = pbegin; p < pend; a1 = (flip ? a1 - 1 : a1 + 1) & 0x1fff)
		{
			uint8_t pen_r = (m_playfield[a1] & 0xff)>>0;
			uint8_t pen_g = (m_playfield[a1 + 0x2000] & 0xff)>>0;
			uint8_t pen_b = (m_playfield[a1 + 0x4000] & 0xff)>>0;

			for (int xi = 0; xi < 8; xi++, p++)
			{
				int xxi = flip ? xi : 7 - xi;
				int color = BIT(pen_r, xxi) << 0;
				color |= BIT(pen_g, xxi) << 1;
				color |= BIT(pen_b, xxi) << 2;
				*p = m_palette->pen(color+0x80);
			}
		}
	}

	// draw tiles
	uint16_t a2 = row_base & 0x1f1f;
	const uint8_t *const tile_data = &m_tiles[((m_bank & 3) << 11) | ((flip ? ~ra : ra) & 7)];
	for (uint32_t *p = pbegin; p < pend; a2 = (flip ? a2 - 1 : a2 + 1) & 0x1f1f)
	{
		uint8_t tile = tile_data[m_vram[a2] << 3];
		rgb_t color = m_palette->pen((m_cram[a2] & 0x3f) * 2 + 1);

		for (int xi = 0; xi < 8; xi++, p++)
		{
			int xxi = flip ? xi : 7 - xi;
			if (BIT(tile, xxi))
				*p = color;
		}
	}

	// draw framebuffer
	if (BIT(m_layer, 2))
	{
		uint16_t a3 = row_base;
		for (uint32_t *p = pbegin; p < pend; a3 = (flip ? a3 - 1 : a3 + 1) & 0x1fff)
		{
			uint8_t pen = m_fbram[a3];
			for (int xi = 0; xi < 8; xi++, p++)
			{
				int xxi = flip ? xi : 7 - xi;
				if (BIT(pen, xxi))
					*p = rgb_t::white();
			}
		}
	}
}


/***************************************************************************

  I/O

***************************************************************************/

void flipjack_state::bank_w(uint8_t data)
{
	// d0-d1: tile bank
	// d2: prg bank
	// d4: ?
	// other bits: unused?
	m_bank = data;
	m_prgbank->set_entry(BIT(data, 2));
}

void flipjack_state::layer_w(uint8_t data)
{
	// d0: flip screen
	// d1: enable playfield layer
	// d2: enable framebuffer layer
	// d3: ?
	// other bits: unused?
	m_layer = data;
}

void flipjack_state::soundlatch_w(uint8_t data)
{
	m_soundlatch->write(data);
	if (BIT(data, 7))
		m_audiocpu->set_input_line(0, ASSERT_LINE);
}

void flipjack_state::sound_nmi_ack_w(uint8_t data)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

void flipjack_state::portc_w(uint8_t data)
{
	// vestigial hopper output?
}

void flipjack_state::coin_nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, state ? CLEAR_LINE : ASSERT_LINE);
}


void flipjack_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x3fff).bankr("prgbank");
	map(0x4000, 0x5fff).ram();
	map(0x6000, 0x67ff).ram();
	map(0x6800, 0x6803).rw("ppi8255", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x7000, 0x7000).w(FUNC(flipjack_state::soundlatch_w));
	map(0x7010, 0x7010).w("crtc", FUNC(hd6845s_device::address_w));
	map(0x7011, 0x7011).w("crtc", FUNC(hd6845s_device::register_w));
	map(0x7020, 0x7020).portr("DSW");
	map(0x7800, 0x7800).w(FUNC(flipjack_state::layer_w));
	map(0x8000, 0x9fff).rom();
	map(0xa000, 0xbfff).ram().share("cram");
	map(0xc000, 0xdfff).ram().share("vram");
	map(0xe000, 0xffff).ram().share("fb_ram");
}

void flipjack_state::main_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0xff, 0xff).w(FUNC(flipjack_state::bank_w));
}

void flipjack_state::sound_map(address_map &map)
{
	map(0x0000, 0x1fff).rom();
	map(0x2000, 0x27ff).ram();
	map(0x4000, 0x4000).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0x6000, 0x6000).w("ay2", FUNC(ay8910_device::address_w));
	map(0x8000, 0x8000).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));
	map(0xa000, 0xa000).w("ay1", FUNC(ay8910_device::address_w));
}

void flipjack_state::sound_io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).w(FUNC(flipjack_state::sound_nmi_ack_w));
}


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( flipjack )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, flipjack_state, coin_nmi_w) // not mapped in P1/P2/P3?

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("P1 Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Right Flipper")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Shoot")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Left Flipper")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_COCKTAIL PORT_NAME("P2 Tilt")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Right Flipper")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) // read only by unused routine?
	PORT_BIT( 0x0e, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED ) // output

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("A0:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Coinage ) )      PORT_DIPLOCATION("A0:2")
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x04, 0x04, "Drop Target" )       PORT_DIPLOCATION("A0:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("A0:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x70, 0x70, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("A0:5,6,7")
	PORT_DIPSETTING(    0x70, "150K & Every 70K" )
	PORT_DIPSETTING(    0x60, "150K & Every 100K" )
	PORT_DIPSETTING(    0x50, "200K & Every 70K" )
	PORT_DIPSETTING(    0x40, "200K & Every 100K" )
	PORT_DIPSETTING(    0x00, "None" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Lives ) )        PORT_DIPLOCATION("A0:8")
	PORT_DIPSETTING(    0x80, "3" )
	PORT_DIPSETTING(    0x00, "5" )
INPUT_PORTS_END


/***************************************************************************

  Machine Config

***************************************************************************/

static const gfx_layout tilelayout =
{
	8, 8,
	RGN_FRAC(1,1),
	1,
	{ 0 },
	{ 7, 6, 5, 4, 3, 2, 1, 0 },
	{ 7*8, 6*8, 5*8, 4*8, 3*8, 2*8, 1*8, 0*8 },
	8*8
};

static GFXDECODE_START( gfx_flipjack )
	GFXDECODE_ENTRY( "tiles", 0, tilelayout, 0, 64 )
GFXDECODE_END



void flipjack_state::machine_start()
{
	uint8_t *ROM = memregion("maincpu")->base();
	m_prgbank->configure_entries(0, 2, &ROM[0x10000], 0x2000);
	m_prgbank->set_entry(0);

	save_item(NAME(m_bank));
	save_item(NAME(m_layer));
}


void flipjack_state::flipjack(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MASTER_CLOCK/4);
	m_maincpu->set_addrmap(AS_PROGRAM, &flipjack_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &flipjack_state::main_io_map);

	Z80(config, m_audiocpu, MASTER_CLOCK/4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &flipjack_state::sound_map);
	m_audiocpu->set_addrmap(AS_IO, &flipjack_state::sound_io_map);

	i8255_device &ppi(I8255A(config, "ppi8255"));
	ppi.in_pa_callback().set_ioport("P1");
	ppi.in_pb_callback().set_ioport("P2");
	ppi.in_pc_callback().set_ioport("P3");
	ppi.out_pc_callback().set(FUNC(flipjack_state::portc_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(VIDEO_CLOCK, 0x188, 0, 0x100, 0x100, 0, 0xc0); // from crtc
	screen.set_screen_update("crtc", FUNC(hd6845s_device::screen_update));

	hd6845s_device &crtc(HD6845S(config, "crtc", VIDEO_CLOCK/8));
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
	crtc.out_vsync_callback().set_inputline("maincpu", INPUT_LINE_IRQ0, HOLD_LINE);
	crtc.out_vsync_callback().append_inputline("audiocpu", INPUT_LINE_NMI, ASSERT_LINE);
	crtc.set_update_row_callback(FUNC(flipjack_state::update_row));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_flipjack);
	PALETTE(config, m_palette, FUNC(flipjack_state::flipjack_palette), 128+8);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);

	ay8910_device &ay1(AY8910(config, "ay1", MASTER_CLOCK/8));
	ay1.port_a_read_callback().set(m_soundlatch, FUNC(generic_latch_8_device::read));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);

	AY8910(config, "ay2", MASTER_CLOCK/8).add_route(ALL_OUTPUTS, "mono", 0.50);
}


ROM_START( flipjack )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "3.d5", 0x00000, 0x2000, CRC(123bd992) SHA1(d845e2b9af5b81d950e5edf35201f1dd1c4af651) )
	ROM_LOAD( "4.f5", 0x08000, 0x2000, CRC(d27e0184) SHA1(f108993fc3fce9173a4961a76fc60655fdd1cd25) )
	ROM_LOAD( "1.l5", 0x10000, 0x2000, CRC(4632263b) SHA1(b1fbb851ffd8aff36aff6f36672122fef3dd0af1) )
	ROM_LOAD( "2.m5", 0x12000, 0x2000, CRC(e2bdce13) SHA1(50d990095a35837570b3117763e990440d8656ae) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "s.s5",  0x0000, 0x2000, CRC(34515a7b) SHA1(affe34198b77bddd314fae2851fd6a29d80f734e) )

	ROM_REGION( 0x2000, "tiles", 0 )
	ROM_LOAD( "cg.l6", 0x0000, 0x2000, CRC(8d87f6b9) SHA1(55ca726f190eac9ee7e26b8f4e519f1634bec0dd) )

	ROM_REGION( 0x6000, "playfield", 0 )
	ROM_LOAD( "b.h6",  0x0000, 0x2000, CRC(bbc8fdcc) SHA1(93758ca13cc49b87508f01c86c652155945dd484) )
	ROM_LOAD( "r.f6",  0x2000, 0x2000, CRC(8c02fe71) SHA1(148e7382dc9b7678c447ada5ad19e03a3a051a7f) )
	ROM_LOAD( "g.d6",  0x4000, 0x2000, CRC(8624d07f) SHA1(fb51c9c785d56854a6530b71868e95ad6be7cbee) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.f8", 0x0000, 0x0100, CRC(f0248102) SHA1(22d87935c941e2e8bba5427599f6fd5fa1262ebc) )
ROM_END

} // anonymous namespace


GAME( 1983?, flipjack, 0, flipjack, flipjack, flipjack_state, empty_init, ROT270, "Jackson Co., Ltd.", "Flipper Jack", MACHINE_IMPERFECT_COLORS | MACHINE_SUPPORTS_SAVE ) // copyright not shown, datecodes on pcb suggests mid-1983
