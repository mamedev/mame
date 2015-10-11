// license:BSD-3-Clause
// copyright-holders:Angelo Salese, hap
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
- flipscreen
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
#include "machine/i8255.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"

#define MASTER_CLOCK    XTAL_16MHz
#define VIDEO_CLOCK     XTAL_6MHz


class flipjack_state : public driver_device
{
public:
	flipjack_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_crtc(*this, "crtc"),
		m_fbram(*this, "fb_ram"),
		m_vram(*this, "vram"),
		m_cram(*this, "cram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{
		m_soundlatch = 0;
		m_bank = 0;
		m_layer = 0;
	}

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<hd6845_device> m_crtc;

	required_shared_ptr<UINT8> m_fbram;
	required_shared_ptr<UINT8> m_vram;
	required_shared_ptr<UINT8> m_cram;

	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	UINT8 m_soundlatch;
	UINT8 m_bank;
	UINT8 m_layer;

	DECLARE_WRITE8_MEMBER(flipjack_sound_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(flipjack_soundlatch_w);
	DECLARE_WRITE8_MEMBER(flipjack_bank_w);
	DECLARE_WRITE8_MEMBER(flipjack_layer_w);
	DECLARE_INPUT_CHANGED_MEMBER(flipjack_coin);
	DECLARE_READ8_MEMBER(flipjack_soundlatch_r);
	DECLARE_WRITE8_MEMBER(flipjack_portc_w);
	virtual void machine_start();
	DECLARE_PALETTE_INIT(flipjack);
	UINT32 screen_update_flipjack(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
};


/***************************************************************************

  Video

***************************************************************************/

PALETTE_INIT_MEMBER(flipjack_state, flipjack)
{
	// from prom
	const UINT8 *color_prom = memregion("proms")->base();
	for (int i = 0; i < 0x40; i++)
	{
		palette.set_pen_color(2*i+1, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
		palette.set_pen_color(2*i+0, pal1bit(color_prom[i] >> 1), pal1bit(color_prom[i] >> 2), pal1bit(color_prom[i] >> 0));
	}

	// standard 3bpp for blitter
	for (int i = 0; i < 8; i++)
		palette.set_pen_color(i+0x80, pal1bit(i >> 1), pal1bit(i >> 2), pal1bit(i >> 0));
}


UINT32 flipjack_state::screen_update_flipjack(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	int x,y,count;

	bitmap.fill(m_palette->black_pen(), cliprect);

	// draw playfield
	if (m_layer & 2)
	{
		const UINT8 *blit_data = memregion("gfx2")->base();

		count = 0;

		for(y=0;y<192;y++)
		{
			for(x=0;x<256;x+=8)
			{
				UINT32 pen_r,pen_g,pen_b,color;
				int xi;

				pen_r = (blit_data[count] & 0xff)>>0;
				pen_g = (blit_data[count+0x2000] & 0xff)>>0;
				pen_b = (blit_data[count+0x4000] & 0xff)>>0;

				for(xi=0;xi<8;xi++)
				{
					if(cliprect.contains(x+xi, y))
					{
						color = ((pen_r >> (7-xi)) & 1)<<0;
						color|= ((pen_g >> (7-xi)) & 1)<<1;
						color|= ((pen_b >> (7-xi)) & 1)<<2;
						bitmap.pix32(y, x+xi) = m_palette->pen(color+0x80);
					}
				}

				count++;
			}
		}
	}

	// draw tiles
	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			gfx_element *gfx = m_gfxdecode->gfx(0);
			int tile = m_bank << 8 | m_vram[x+y*0x100];
			int color = m_cram[x+y*0x100] & 0x3f;

				gfx->transpen(bitmap,cliprect, tile, color, 0, 0, x*8, y*8, 0);
		}
	}

	// draw framebuffer
	if (m_layer & 4)
	{
		count = 0;

		for(y=0;y<192;y++)
		{
			for(x=0;x<256;x+=8)
			{
				UINT32 pen,color;
				int xi;

				pen = (m_fbram[count] & 0xff)>>0;

				for(xi=0;xi<8;xi++)
				{
					if(cliprect.contains(x+xi, y))
					{
						color = ((pen >> (7-xi)) & 1) ? 0x87 : 0;
						if(color)
							bitmap.pix32(y, x+xi) = m_palette->pen(color);
					}
				}

				count++;
			}
		}
	}


	return 0;
}


/***************************************************************************

  I/O

***************************************************************************/

WRITE8_MEMBER(flipjack_state::flipjack_bank_w)
{
	// d0-d1: tile bank
	// d2: prg bank
	// d4: ?
	// other bits: unused?
	m_bank = data;
	membank("bank1")->set_entry(data >> 2 & 1);
}

WRITE8_MEMBER(flipjack_state::flipjack_layer_w)
{
	// d0: flip screen
	// d1: enable playfield layer
	// d2: enable framebuffer layer
	// d3: ?
	// other bits: unused?
	m_layer = data;
}

READ8_MEMBER(flipjack_state::flipjack_soundlatch_r)
{
	m_audiocpu->set_input_line(0, CLEAR_LINE);
	return m_soundlatch;
}

WRITE8_MEMBER(flipjack_state::flipjack_soundlatch_w)
{
	m_soundlatch = data;
	m_audiocpu->set_input_line(0, ASSERT_LINE);
}

WRITE8_MEMBER(flipjack_state::flipjack_sound_nmi_ack_w)
{
	m_audiocpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);
}

WRITE8_MEMBER(flipjack_state::flipjack_portc_w)
{
	// watchdog?
}

INPUT_CHANGED_MEMBER(flipjack_state::flipjack_coin)
{
	if (newval)
		m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}


static ADDRESS_MAP_START( flipjack_main_map, AS_PROGRAM, 8, flipjack_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x3fff) AM_ROMBANK("bank1")
	AM_RANGE(0x4000, 0x5fff) AM_RAM
	AM_RANGE(0x6000, 0x67ff) AM_RAM
	AM_RANGE(0x6800, 0x6803) AM_DEVREADWRITE("ppi8255", i8255_device, read, write)
	AM_RANGE(0x7000, 0x7000) AM_WRITE(flipjack_soundlatch_w)
	AM_RANGE(0x7010, 0x7010) AM_DEVWRITE("crtc", hd6845_device, address_w)
	AM_RANGE(0x7011, 0x7011) AM_DEVWRITE("crtc", hd6845_device, register_w)
	AM_RANGE(0x7020, 0x7020) AM_READ_PORT("DSW")
	AM_RANGE(0x7800, 0x7800) AM_WRITE(flipjack_layer_w)
	AM_RANGE(0x8000, 0x9fff) AM_ROM
	AM_RANGE(0xa000, 0xbfff) AM_RAM AM_SHARE("cram")
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("fb_ram")
ADDRESS_MAP_END

static ADDRESS_MAP_START( flipjack_main_io_map, AS_IO, 8, flipjack_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xff, 0xff) AM_WRITE(flipjack_bank_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( flipjack_sound_map, AS_PROGRAM, 8, flipjack_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x27ff) AM_RAM
	AM_RANGE(0x4000, 0x4000) AM_DEVREADWRITE("ay2", ay8910_device, data_r, data_w)
	AM_RANGE(0x6000, 0x6000) AM_DEVWRITE("ay2", ay8910_device, address_w)
	AM_RANGE(0x8000, 0x8000) AM_DEVREADWRITE("ay1", ay8910_device, data_r, data_w)
	AM_RANGE(0xa000, 0xa000) AM_DEVWRITE("ay1", ay8910_device, address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( flipjack_sound_io_map, AS_IO, 8, flipjack_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(flipjack_sound_nmi_ack_w)
ADDRESS_MAP_END


/***************************************************************************

  Inputs

***************************************************************************/

static INPUT_PORTS_START( flipjack )
	PORT_START("COIN")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, flipjack_state, flipjack_coin, 0) // where in P1/P2/P3 is it mapped?

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
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
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
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static GFXDECODE_START( flipjack )
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout, 0, 64 )
GFXDECODE_END



void flipjack_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0x10000], 0x2000);
	membank("bank1")->set_entry(0);

	save_item(NAME(m_soundlatch));
	save_item(NAME(m_bank));
	save_item(NAME(m_layer));
}


static MACHINE_CONFIG_START( flipjack, flipjack_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(flipjack_main_map)
	MCFG_CPU_IO_MAP(flipjack_main_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flipjack_state,  irq0_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(flipjack_sound_map)
	MCFG_CPU_IO_MAP(flipjack_sound_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", flipjack_state,  nmi_line_assert)

	MCFG_DEVICE_ADD("ppi8255", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("P1"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("P2"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("P3"))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(flipjack_state, flipjack_portc_w))

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(VIDEO_CLOCK, 0x188, 0, 0x100, 0x100, 0, 0xc0) // from crtc
	MCFG_SCREEN_UPDATE_DRIVER(flipjack_state, screen_update_flipjack)
	MCFG_SCREEN_ORIENTATION(ROT90)

	MCFG_MC6845_ADD("crtc", HD6845, "screen", VIDEO_CLOCK/8)
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", flipjack)

	MCFG_PALETTE_ADD("palette", 128+8)
	MCFG_PALETTE_INIT_OWNER(flipjack_state, flipjack)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, MASTER_CLOCK/8)
	MCFG_AY8910_PORT_A_READ_CB(READ8(flipjack_state, flipjack_soundlatch_r))  /* Port A read */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_SOUND_ADD("ay2", AY8910, MASTER_CLOCK/8)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( flipjack )
	ROM_REGION( 0x14000, "maincpu", 0 )
	ROM_LOAD( "3.d5", 0x00000, 0x2000, CRC(123bd992) SHA1(d845e2b9af5b81d950e5edf35201f1dd1c4af651) )
	ROM_LOAD( "4.f5", 0x08000, 0x2000, CRC(d27e0184) SHA1(f108993fc3fce9173a4961a76fc60655fdd1cd25) )
	ROM_LOAD( "1.l5", 0x10000, 0x2000, CRC(4632263b) SHA1(b1fbb851ffd8aff36aff6f36672122fef3dd0af1) )
	ROM_LOAD( "2.m5", 0x12000, 0x2000, CRC(e2bdce13) SHA1(50d990095a35837570b3117763e990440d8656ae) )

	ROM_REGION( 0x2000, "audiocpu", 0 )
	ROM_LOAD( "s.s5",  0x0000, 0x2000, CRC(34515a7b) SHA1(affe34198b77bddd314fae2851fd6a29d80f734e) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "cg.l6", 0x0000, 0x2000, CRC(8d87f6b9) SHA1(55ca726f190eac9ee7e26b8f4e519f1634bec0dd) )

	ROM_REGION( 0x6000, "gfx2", 0 )
	ROM_LOAD( "b.h6",  0x0000, 0x2000, CRC(bbc8fdcc) SHA1(93758ca13cc49b87508f01c86c652155945dd484) )
	ROM_LOAD( "r.f6",  0x2000, 0x2000, CRC(8c02fe71) SHA1(148e7382dc9b7678c447ada5ad19e03a3a051a7f) )
	ROM_LOAD( "g.d6",  0x4000, 0x2000, CRC(8624d07f) SHA1(fb51c9c785d56854a6530b71868e95ad6be7cbee) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "m3-7611-5.f8", 0x0000, 0x0100, CRC(f0248102) SHA1(22d87935c941e2e8bba5427599f6fd5fa1262ebc) )
ROM_END


GAME( 1983?, flipjack,   0,      flipjack, flipjack, driver_device, 0, ROT90, "Jackson Co., Ltd.", "Flipper Jack", MACHINE_IMPERFECT_GRAPHICS | MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // copyright not shown, datecodes on pcb suggests mid-1983
