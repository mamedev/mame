// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/******************************************************************************

    Flower (c) 1986 Clarue (licensed to Komax/Sega)

    driver by Angelo Salese,
    original "wiped off due of not anymore licenseable" driver by insideoutboy.

    TODO:
    - priority might be wrong in some places (title screen stars around the
      galaxy, planet ship 3rd boss, 2nd boss);
    - sound chips (similar to Namco custom chips?)

===============================================================================

Flower (c)1986 Komax (USA license)
       (c)1986 Sega/Alpha (Sega game number 834-5998)

There is a PCB picture that shows two stickers, the first says
 "Flower (c) 1986 Clarue" while the second one is an original
 serial number tag also showing "Clarue". GFX ROM contents also
 show (C) 1986 CLARUE. A Wood Place Inc. spinoff perhaps?

        FLOWER   CHIP PLACEMENT

XTAL: 18.4320 MHz
USES THREE Z80A CPU'S

CHIP #  POSITION   TYPE
------------------------
1        5J         27256   CONN BD
2        5F         27256    "
3        D9         27128    "
4        12A        27128    "
5        16A        27256    "
6        7E         2764    BOTTOM BD
15       9E          "       "
8        10E         "       "
9        12E         "       "
10       13E         "       "
11       14E         "       "
12       16E         "       "
13       17E         "       "
14       19E         "       "

                Upright or Cocktail cabinet
     Two 8-Way joysticks with three (3) fire buttons each

    Button 1: Laser    Button 2: Missle    Button 3: Cutter

                        44 Pin Edge Connector
          Solder Side             |             Parts Side
------------------------------------------------------------------
             GND             |  1 | 2  |             GND
             GND             |  3 | 4  |             GND
             +5V             |  5 | 6  |             +5V
             +5V             |  7 | 8  |             +5V
             +12V            |  9 | 10 |             +5V
         Speaker (-)         | 11 | 12 |        Speaker (+)
       Player 1 - Up         | 13 | 14 |       Player 1 - Down
       Player 1 - Left       | 15 | 16 |       Player 1 - Right
       Player 1 - Laser      | 17 | 18 |       Player 1 - Missile
       Player 1 - Cutter     | 19 | 20 |
       Player 2 - Up         | 21 | 22 |       Player 2 - Down
       Player 2 - Left       | 23 | 24 |       Player 2 - Right
       Player 2 - Laser      | 25 | 26 |       Player 2 - Missile
       Player 2 - Cutter     | 27 | 28 |
        Coin Switch 1        | 29 | 30 |       Player 1 Start
       Player 2 Start        | 31 | 32 |
                             | 33 | 34 |
       Coin Counter 1        | 35 | 36 |
        Video Sync           | 37 | 38 |        Video Blue
        Video Green          | 39 | 40 |        Video Red
             GND             | 41 | 42 |           GND
             GND             | 43 | 44 |           GND

******************************************************************************/


#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/gen_latch.h"
#include "audio/flower.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#define MASTER_CLOCK XTAL(18'432'000)

class flower_state : public driver_device
{
public:
	flower_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_mastercpu(*this, "mastercpu"),
		m_slavecpu(*this, "slavecpu"),
		m_audiocpu(*this, "audiocpu"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode"),
		m_txvram(*this, "txvram"),
		m_bgvram(*this, "bgvram"),
		m_fgvram(*this, "fgvram"),
		m_workram(*this, "workram"),
		m_bgscroll(*this, "bgscroll"),
		m_fgscroll(*this, "fgscroll"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void flower(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

private:
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(coin_counter_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(audio_nmi_mask_w);
	DECLARE_WRITE8_MEMBER(bgvram_w);
	DECLARE_WRITE8_MEMBER(fgvram_w);
	INTERRUPT_GEN_MEMBER(master_vblank_irq);
	INTERRUPT_GEN_MEMBER(slave_vblank_irq);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void audio_map(address_map &map);
	void shared_map(address_map &map);

	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;

	virtual void video_start() override;

	required_device<cpu_device> m_mastercpu;
	required_device<cpu_device> m_slavecpu;
	required_device<cpu_device> m_audiocpu;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;
	required_shared_ptr<uint8_t> m_txvram;
	required_shared_ptr<uint8_t> m_bgvram;
	required_shared_ptr<uint8_t> m_fgvram;
	required_shared_ptr<uint8_t> m_workram;
	required_shared_ptr<uint8_t> m_bgscroll;
	required_shared_ptr<uint8_t> m_fgscroll;
	required_device<generic_latch_8_device> m_soundlatch;
	bitmap_ind16 m_temp_bitmap;

	void draw_legacy_text(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	bool m_audio_nmi_enable;
	bool m_flip_screen;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
};

TILE_GET_INFO_MEMBER(flower_state::get_bg_tile_info)
{
	int code = m_bgvram[tile_index];
	int color = (m_bgvram[tile_index+0x100] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

TILE_GET_INFO_MEMBER(flower_state::get_fg_tile_info)
{
	int code = m_fgvram[tile_index];
	int color = (m_fgvram[tile_index+0x100] & 0xf0) >> 4;

	SET_TILE_INFO_MEMBER(1, code, color, 0);
}

void flower_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);
	m_fg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(flower_state::get_fg_tile_info),this), TILEMAP_SCAN_ROWS, 16, 16, 16, 16);

	m_screen->register_screen_bitmap(m_temp_bitmap);
	m_fg_tilemap->set_transparent_pen(15);

	save_item(NAME(m_flip_screen));

	m_bg_tilemap->set_scrolldx(16, 0);
	m_fg_tilemap->set_scrolldx(16, 0);
}

void flower_state::draw_legacy_text(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_0 = m_gfxdecode->gfx(0);
	int count;

	for (count=0;count<32*32;count++)
	{
		int x = count % 32;
		int y = count / 32;

		uint8_t tile = m_txvram[count];
		uint8_t attr = m_txvram[count+0x400];

		if(attr & 0x03) // debug
			attr = machine().rand() & 0xfc;

		gfx_0->transpen(bitmap,cliprect,tile,attr >> 2,0,0,x*8+16,y*8,3);
	}

	for (count=0;count<0x40;count++)
	{
		int x = count / 32;
		int y = count % 32;

		uint8_t tile = m_txvram[count];
		uint8_t attr = m_txvram[count+0x400];

		if(attr & 0x03) // debug
			attr = machine().rand() & 0xfc;

		gfx_0->transpen(bitmap,cliprect,tile,attr >> 2,0,0,x*8+256+16,y*8,3);
	}
}

/*
 [0] YYYY YYYY Y offset
 [1] YXoo oooo Flip Y/X, tile number
 [2] ---- b--b tile bank select
 [3] Yyyy Xxxx Y size, Y zoom, X size, X zoom
 [4] xxxx xxxx X offset LSB
 [5] XXXX XXXX X offset MSB
 [6] cccc ---- color base
 */
void flower_state::draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	uint8_t *spr_ptr = &m_workram[0x1e08];
	gfx_element *gfx_2 = m_gfxdecode->gfx(2);

	// traverse from top to bottom
	for(int i=0x1f0;i>=0;i-=8)
	{
		uint8_t tile = (spr_ptr[i+1] & 0x3f);
		uint8_t color = spr_ptr[i+6] >> 4;
		int x = (spr_ptr[i+4] | (spr_ptr[i+5]<<8))-39;
		int y = 241-spr_ptr[i+0];
		uint8_t attr = spr_ptr[i+2];
		uint8_t fy = spr_ptr[i+1] & 0x80;
		uint8_t fx = spr_ptr[i+1] & 0x40;
		uint8_t ysize = ((spr_ptr[i+3] & 0x80) >> 7) + 1;
		uint8_t xsize = ((spr_ptr[i+3] & 0x08) >> 3) + 1;
		uint8_t ydiv = ysize == 2 ? 1 : 2;
		uint8_t xdiv = xsize == 2 ? 1 : 2;
		uint32_t yshrink_zoom = ((spr_ptr[i+3] & 0x70) >> 4) + 1;
		uint32_t xshrink_zoom = ((spr_ptr[i+3] & 0x07) >> 0) + 1;
		yshrink_zoom <<= 13;
		xshrink_zoom <<= 13;
		int ypixels = (yshrink_zoom*16) >> 16;
		int xpixels = (xshrink_zoom*16) >> 16;

		tile |= (attr & 1) << 6;
		tile |= (attr & 8) << 4;

		if(flip_screen())
		{
			x += xsize*16;
			x = 288-x;
			y -= 2;
		}

		if(ysize == 2)
			y-=16;

		for(int yi=0;yi<ysize;yi++)
		{
			int yoffs = (16-ypixels)/ydiv;

			for(int xi=0;xi<xsize;xi++)
			{
				int tile_offs;
				int xoffs = (16-xpixels)/xdiv;

				tile_offs = fx ? (xsize-xi-1) * 8 : xi*8;
				tile_offs+= fy ? (ysize-yi-1) : yi;

				gfx_2->zoom_transpen(bitmap,cliprect, tile+tile_offs, color, fx, fy, x+xi*xpixels+xoffs, y+yi*ypixels+yoffs, xshrink_zoom, yshrink_zoom, 15);
			}
		}
	}
}

uint32_t flower_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	m_bg_tilemap->set_scrolly(0, m_bgscroll[0]);
	m_fg_tilemap->set_scrolly(0, m_fgscroll[0]);

	m_temp_bitmap.fill(0,cliprect);
	m_bg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
	m_fg_tilemap->draw(screen, m_temp_bitmap, cliprect, 0, 0);
	draw_sprites(m_temp_bitmap,cliprect);
	draw_legacy_text(m_temp_bitmap,cliprect);

	copybitmap(bitmap,m_temp_bitmap,m_flip_screen,m_flip_screen,m_flip_screen == true ? -154 : 0, m_flip_screen == true ? -7 : 0, cliprect);
	return 0;
}

WRITE8_MEMBER(flower_state::flipscreen_w)
{
	flip_screen_set(data & 1);
	m_flip_screen = BIT(data,0);
}

WRITE8_MEMBER(flower_state::coin_counter_w)
{
	machine().bookkeeping().coin_counter_w(0,data & 1);
}

WRITE8_MEMBER(flower_state::sound_command_w)
{
	m_soundlatch->write(space, 0, data & 0xff);
	if(m_audio_nmi_enable == true)
		m_audiocpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

WRITE8_MEMBER(flower_state::audio_nmi_mask_w)
{
	m_audio_nmi_enable = BIT(data,0);
}

WRITE8_MEMBER(flower_state::bgvram_w)
{
	m_bgvram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset & 0xff);
}

WRITE8_MEMBER(flower_state::fgvram_w)
{
	m_fgvram[offset] = data;
	m_fg_tilemap->mark_tile_dirty(offset & 0xff);
}

void flower_state::shared_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram().share("workram");
	map(0xa000, 0xa000).nopw();
	map(0xa001, 0xa001).w(FUNC(flower_state::flipscreen_w));
	map(0xa002, 0xa002).nopw(); // master irq related (0 at start, 1 at end)
	map(0xa003, 0xa003).nopw(); // slave irq related (0 at start, 1 at end)
	map(0xa004, 0xa004).w(FUNC(flower_state::coin_counter_w));
	map(0xa005, 0xa005).nopw();
	map(0xa100, 0xa100).portr("P1");
	map(0xa101, 0xa101).portr("P2");
	map(0xa102, 0xa102).portr("DSW1");
	map(0xa103, 0xa103).portr("DSW2");
	map(0xa400, 0xa400).w(FUNC(flower_state::sound_command_w));
	map(0xe000, 0xefff).ram().share("txvram");
	map(0xf000, 0xf1ff).ram().w(FUNC(flower_state::fgvram_w)).share("fgvram");
	map(0xf200, 0xf200).ram().share("fgscroll");
	map(0xf800, 0xf9ff).ram().w(FUNC(flower_state::bgvram_w)).share("bgvram");
	map(0xfa00, 0xfa00).ram().share("bgscroll");
}

void flower_state::audio_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x4000).nopw(); // audio irq related (0 at start, 1 at end)
	map(0x4001, 0x4001).w(FUNC(flower_state::audio_nmi_mask_w));
	map(0x6000, 0x6000).r(m_soundlatch, FUNC(generic_latch_8_device::read));
	map(0x8000, 0x803f).w("flower", FUNC(flower_sound_device::lower_write));
	map(0xa000, 0xa03f).w("flower", FUNC(flower_sound_device::upper_write));
	map(0xc000, 0xc7ff).ram();
}

INPUT_CHANGED_MEMBER(flower_state::coin_inserted)
{
	m_mastercpu->set_input_line(INPUT_LINE_NMI, newval ? CLEAR_LINE : ASSERT_LINE);
}

static INPUT_PORTS_START( flower )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("P1 Laser")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("P1 Missile")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("P1 Cutter")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL PORT_NAME("P2 Laser")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_COCKTAIL PORT_NAME("P2 Missile")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_COCKTAIL PORT_NAME("P2 Cutter")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, flower_state,coin_inserted, 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START1  )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_DIPNAME( 0x08, 0x08, "Energy Decrease" )       PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, "Slow" )
	PORT_DIPSETTING(    0x00, "Fast" )
	PORT_DIPNAME( 0x10, 0x10, "Invulnerability (Cheat)") PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "Keep Weapons When Destroyed" ) PORT_DIPLOCATION("SW2:6") // check code at 0x74a2
	PORT_DIPSETTING(    0x20, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:7")       // "Enemy Bullets"
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x80, 0x80, "Shot Range" )            PORT_DIPLOCATION("SW2:8")       // check code at 0x75f9
	PORT_DIPSETTING(    0x80, "Short" )
	PORT_DIPSETTING(    0x00, "Long" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x05, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(    0x07, "1" )
	PORT_DIPSETTING(    0x06, "2" )
	PORT_DIPSETTING(    0x05, "3" )
	PORT_DIPSETTING(    0x04, "4" )
	PORT_DIPSETTING(    0x03, "5" )
	PORT_DIPSETTING(    0x02, "6" )
	PORT_DIPSETTING(    0x01, "7" )
	PORT_DIPSETTING(    0x00, "Infinite (Cheat)")
	PORT_DIPNAME( 0x18, 0x18, DEF_STR( Coinage ) )      PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )      PORT_DIPLOCATION("SW1:6")       // check code at 0x759f
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Bonus_Life ) )   PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, "30k, then every 50k" )
	PORT_DIPSETTING(    0x00, "50k, then every 80k" )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ STEP4(0,1), STEP4(8,1) },
	{ STEP8(0,16) },
	8*8*2
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 4, RGN_FRAC(1,2), RGN_FRAC(1,2)+4 },
	{ STEP4(0,1), STEP4(8,1), STEP4(8*8*2,1), STEP4(8*8*2+8,1) },
	{ STEP8(0,16), STEP8(8*8*4,16) },
	16*16*2
};

static GFXDECODE_START( gfx_flower )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0,  64 )
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout, 0,  16 )
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout, 0,  16 )
GFXDECODE_END

void flower_state::machine_start()
{
	save_item(NAME(m_audio_nmi_enable));
}

void flower_state::machine_reset()
{
	m_audio_nmi_enable = false;
}

INTERRUPT_GEN_MEMBER(flower_state::master_vblank_irq)
{
	//if(m_master_irq_enable == true)
		device.execute().set_input_line(0, HOLD_LINE);
}

INTERRUPT_GEN_MEMBER(flower_state::slave_vblank_irq)
{
	//if(m_slave_irq_enable == true)
		device.execute().set_input_line(0, HOLD_LINE);
}


MACHINE_CONFIG_START(flower_state::flower)
	MCFG_DEVICE_ADD("mastercpu",Z80,MASTER_CLOCK/4)
	MCFG_DEVICE_PROGRAM_MAP(shared_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", flower_state, master_vblank_irq)

	MCFG_DEVICE_ADD("slavecpu",Z80,MASTER_CLOCK/4)
	MCFG_DEVICE_PROGRAM_MAP(shared_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", flower_state, slave_vblank_irq)

	MCFG_DEVICE_ADD("audiocpu",Z80,MASTER_CLOCK/4)
	MCFG_DEVICE_PROGRAM_MAP(audio_map)
	MCFG_DEVICE_PERIODIC_INT_DRIVER(flower_state, irq0_line_hold, 90)

	config.m_perfect_cpu_quantum = subtag("mastercpu");

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(flower_state, screen_update)
	MCFG_SCREEN_RAW_PARAMS(MASTER_CLOCK/3,384,0,288,264,16,240) // derived from Galaxian HW, 60.606060
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_flower);
	PALETTE(config, m_palette, palette_device::RGB_444_PROMS, "proms", 256);

	GENERIC_LATCH_8(config, m_soundlatch);

	SPEAKER(config, "mono").front_center();

	MCFG_DEVICE_ADD("flower", FLOWER_CUSTOM, 96000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


ROM_START( flower ) /* Komax version */
	ROM_REGION( 0x10000, "mastercpu", 0 ) /* main cpu */
	ROM_LOAD( "1.5j",   0x0000, 0x8000, CRC(a4c3af78) SHA1(d149b0e0d82318273dd9cc5a143b175cdc818d0d) )

	ROM_REGION( 0x10000, "slavecpu", 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "15.9e",  0x6000, 0x2000, CRC(1cad9f72) SHA1(c38dbea266246ed4d47d12bdd8f9fae22a5f8bb8) )

	ROM_REGION( 0x8000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )  /* 8-bit samples */

	ROM_REGION( 0x4000, "soundvol", 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )  /* volume tables? */

	ROM_REGION( 0x300, "proms", 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r

	ROM_REGION( 0x0520, "user1", 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END

ROM_START( flowerj ) /* Sega/Alpha version.  Sega game number 834-5998 */
	ROM_REGION( 0x10000, "mastercpu", 0 ) /* main cpu */
	ROM_LOAD( "1",   0x0000, 0x8000, CRC(63a2ef04) SHA1(0770f5a18d58b780abcda7e000c2a5e46f96d319) ) // hacked? "AKINA.N" changed to "JUKYUNG"

	ROM_REGION( 0x10000, "slavecpu", 0 ) /* sub cpu */
	ROM_LOAD( "2.5f",   0x0000, 0x8000, CRC(7c7ee2d8) SHA1(1e67bfe0f3585be5a6e6719ccf9db764bafbcb01) )

	ROM_REGION( 0x10000, "audiocpu", 0 ) /* sound cpu */
	ROM_LOAD( "3.d9",   0x0000, 0x4000, CRC(8866c2b0) SHA1(d00f31994673e8087a1406f98e8832d07cedeb66) ) // 1xxxxxxxxxxxxx = 0xFF

	ROM_REGION( 0x2000, "gfx1", ROMREGION_INVERT ) /* tx layer */
	ROM_LOAD( "10.13e", 0x0000, 0x2000, CRC(62f9b28c) SHA1(d57d06b99e72a4f68f197a5b6c042c926cc70ca0) ) // FIRST AND SECOND HALF IDENTICAL

	ROM_REGION( 0x8000, "gfx2", ROMREGION_INVERT ) /* bg layers */
	ROM_LOAD( "8.10e",  0x0000, 0x2000, CRC(f85eb20f) SHA1(699edc970c359143dee6de2a97cc2a552454785b) )
	ROM_LOAD( "6.7e",   0x2000, 0x2000, CRC(3e97843f) SHA1(4e4e5625dbf78eca97536b1428b2e49ad58c618f) )
	ROM_LOAD( "9.12e",  0x4000, 0x2000, CRC(f1d9915e) SHA1(158e1cc8c402f9ae3906363d99f2b25c94c64212) )
	ROM_LOAD( "7.9e",   0x6000, 0x2000, CRC(e350f36c) SHA1(f97204dc95b4000c268afc053a2333c1629e07d8) )

	ROM_REGION( 0x8000, "gfx3", ROMREGION_INVERT ) /* sprites */
	ROM_LOAD( "14.19e", 0x0000, 0x2000, CRC(11b491c5) SHA1(be1c4a0fbe8fd4e124c21e0f700efa0428376691) )
	ROM_LOAD( "13.17e", 0x2000, 0x2000, CRC(ea743986) SHA1(bbef4fd0f7d21cc89a52061fa50d7c2ea37287bd) )
	ROM_LOAD( "12.16e", 0x4000, 0x2000, CRC(e3779f7f) SHA1(8e12d06b3cdc2fcb7b77cc35f8eca45544cc4873) )
	ROM_LOAD( "11.14e", 0x6000, 0x2000, CRC(8801b34f) SHA1(256059fcd16b21e076db1c18fd9669128df1d658) )

	ROM_REGION( 0x8000, "samples", 0 )
	ROM_LOAD( "4.12a",  0x0000, 0x8000, CRC(851ed9fd) SHA1(5dc048b612e45da529502bf33d968737a7b0a646) )  /* 8-bit samples */

	ROM_REGION( 0x4000, "soundvol", 0 )
	ROM_LOAD( "5.16a",  0x0000, 0x4000, CRC(42fa2853) SHA1(cc1e8b8231d6f27f48b05d59390e93ea1c1c0e4c) )  /* volume tables? */

	ROM_REGION( 0x300, "proms", 0 ) /* RGB proms */
	ROM_LOAD( "82s129.k3",  0x0000, 0x0100, CRC(5aab7b41) SHA1(8d44639c7c9f1ba34fe9c4e74c8a38b6453f7ac0) ) // b
	ROM_LOAD( "82s129.k2",  0x0100, 0x0100, CRC(ababb072) SHA1(a9d46d12534c8662c6b54df94e96907f3a156968) ) // g
	ROM_LOAD( "82s129.k1",  0x0200, 0x0100, CRC(d311ed0d) SHA1(1d530c874aecf93133d610ab3ce668548712913a) ) // r

	ROM_REGION( 0x0520, "user1", 0 ) /* Other proms, (zoom table?) */
	ROM_LOAD( "82s147.d7",  0x0000, 0x0200, CRC(f0dbb2a7) SHA1(03cd8fd41d6406894c6931e883a9ac6a4a4effc9) )
	ROM_LOAD( "82s147.j18", 0x0200, 0x0200, CRC(d7de0860) SHA1(5d3d8c5476b1edffdacde09d592c64e78d2b90c0) )
	ROM_LOAD( "82s123.k7",  0x0400, 0x0020, CRC(ea9c65e4) SHA1(1bdd77a7f3ef5f8ec4dbb9524498c0c4a356f089) )
	ROM_LOAD( "82s129.a1",  0x0420, 0x0100, CRC(c8dad3fc) SHA1(8e852efac70223d02e45b20ed8a12e38c5010a78) )
ROM_END


GAME( 1986, flower,  0,      flower, flower, flower_state, empty_init, ROT0, "Clarue (Komax license)",                   "Flower (US)",    MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS|MACHINE_NO_COCKTAIL )
GAME( 1986, flowerj, flower, flower, flower, flower_state, empty_init, ROT0, "Clarue (Sega / Alpha Denshi Co. license)", "Flower (Japan)", MACHINE_IMPERFECT_SOUND|MACHINE_IMPERFECT_GRAPHICS|MACHINE_NO_COCKTAIL )
