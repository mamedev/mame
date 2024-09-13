// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/*

   Kaneko EXPRO-02 board

  Used by the newer (or possibly older/original) revisions of Gals Panic
  also the basis of the various Comad clones

    Fantasia         1994 Comad
    Super Model      1994 Comad
    New Fantasia     1994 Comad
    New Fantasia     1995 Comad (set 2)
    Fantasy '95      1995 Hi-max Technology Inc. (Running on a Comad PCB)
    Miss World '96   1996 Comad
    Ms/Mr World '96  1996 Comad
    Fantasia II      1997 Comad
    Fantasia II      1998 Comad

  The following seem similar but could have other changes

    Pocket Gal VIP /
    Gals Hustler     1997 Ace International
    Zip & Zap        1995 Barko Corp

 Notes:
  - In gfx data banking function, some strange gfx are shown. Timing issue?

 TODO:
 - irq sources are unknown at current time

This is a single board, no daughterboard. There are only 4 IC's socketed, the
rest is soldered to the board, and no piggybacked ROMs.
Board number is MDK 321 V-0    EXPRO-02

Gals Panic
Kaneko, 1990

PCB Layout
----------

EXPRO-02
|-------------------------------------------------------------------------|
|     M6295 PM007E.U47 12MHz          PM000E.U74    PM004E.U86            |
|      VOL  PM008E.U46 16MHz 62256          PM002E.U76   PM109U_U88-01.U88|
|LA4460                                                                   |
|             PAL PAL  PAL                                                |
|             PAL PAL  PAL            PM001E.U73    PM005E.U85            |
|                      PAL   62256          PM003E.U75   PM110U_U87-01.U87|
|    |--------------------|                                               |
|    |       68000        |                                               |
|    |                    |  41464                                        |
|    |--------------------|  41464              PM017E.U84                |
|    GP-U27             PAL  41464  |---------|                           |
|J   PAL   GP-U41            41464  |KANEKO   |                           |
|A MC-8282  PAL              41464  |VU-002   | PM006E.U83     PM018E.U94 |
|M                   D42101C 41464  |         |                           |
|M                                  |         |          PM019U_U93-01.U93|
|A                   D42101C        |---------| PM206E.U82                |
|            HM53461                                                      |
|    PAL     HM53461   PAL    |-------|         CALC1-CHIP                |
|            HM53461   PAL    |KANEKO |                      PM016E.U92   |
|    PAL     HM53461   PAL    |VIEW2- |    6264                           |
|            HM53461   PAL    |   CHIP|                      PM015E.U91   |
|    PAL     HM53461   PAL    |-------|    6264                           |
|DSW2         6116     PAL   PAL                             PM014E.U90   |
|                      PAL                 PAL                            |
|DSW1         6116                         PAL               PM013E.U89   |
|-------------------------------------------------------------------------|

    Notes
    -----
    GP-U27/U41 - These are DIP40 chips, but are not MCUs because there is no stable
                 clock input on any of the pins of these chips. They're not ROMs either
                 because the pinout doesn't match any known EPROMs.
                 There are no markings on the chips other than 'GP-U27' & 'GP-U41'
                 If GP-U41 is removed, on bootup the PCB gives an error 'BG ERROR' and
                 a memory address. If GP-U27 is removed, the PCB works but there are no
                 background graphics.

    68000 clock - 12.0MHz
    CALC1-CHIP clock - 16.0MHz
    GP-U41 clocks - pins 21 & 22 - 12.0MHz, pins 1 & 2 - 6.0MHz, pins 8 & 9 - 15.6249kHz (HSync?)
    GP-U27 clock - none (so it's not an MCU)
    D42101C - Line buffer for NTSC TV

    (TODO: which is correct?)
    OKI M6295 clock - 2.0MHz (12/6). pin7 = low
    OKI M6295 clock - 2.000 MHz [16/8]. Sample rate 2000000/165

    VSync - 60Hz
    HSync - 15.68kHz
    MC-8282 - Kaneko custom I/O JAMMA ceramic module
    41464 - 64k x4 DRAM
    HM53461 - 64k x4 Multiport CMOS VRAM
    6116 - 2k x8 SRAM
    6264 - 8k x8 SRAM
    62256 - 32k x8 SRAM

**************************************************************************************************

Gals Panic (Japan)
(C)1990 Kaneko

EXPRO-02 PCB
M6100575A GALS PANIC (PCB manufactured by Taito)

CPU: MC68000
Sound: M6295
OSC: 12.0000MHz, 16.0000MHz
Custom: VU-002, VIEW2, CALC1

ROMs:
PM109J.U88 (OKI M271000ZB) - Main programs
PM110J.U87 (OKI M271000ZB)
PM004E.U86
PM005E.U85
PM-002E.U76
PM-003E.U75
PM-000E.U74
PM-001E.U73

PM006E.U83 - Sprites
PM206E.U82
PM018E.U94

PM-013E.U89 (40pin mask)
PM-014E.U90
PM-015E.U91
PM-016E.U92

GP-U41.U41 (40pin mask?)
GP-U27.U27


PAL/GALs:
U10 (18CV8)
U11 (18CV8)
U12 (18CV8)
U15 (22CV10)
GP-U28 (16V8)
U29 (18CV8)
U32 (18CV8)
U33 (18CV8)
U34 (18CV8)
U35 (18CV8)
U36 (22CV10)
U37 (22CV10)
U38 (18CV8)
U42 (18CV8)
GP-U44 (16V8)
U45 (18CV8)
U48 (18CV8)
GP-U57 (16V8)
GP-U58 (16V8)
GP-U60 (16V8)
U77 (22CV10)
U78 (22CV10)

------------- Comad games ------------------

The Comad games are clearly derived from this version of the game, not
the one in galspanic.cpp.  Fantasia even still has the encrypted tile
roms and makes use of the extra layer.  The other games write to the
RAM for this layer, but don't have any roms.

the layer is misplaced however, different scroll regs?

 - On Gals Hustler there is an extra test mode if you hold down player 2
   button 1, I have no idea if its complete or not


-- Zip Zap notes ---

Bg for select screens seems to be corrupt

-- General Notes --

Fantasia etc. games are locking up when the girl 'changes' due to not liking
the way we handle OKI status reads.. however these reads are correct according to
tests done with a real chip so there must be something odd going on on this hardware

From Miss World 96 manual/dipswitch sheet:

A/B/C Three Versions depending on nude grade
 A-Version is the extreme hottest nude models
 B-Version is the more attractive nude models
 C-Version is very beautiful bikini models

An example of this can be seen in the Fantasia II sets with type A & B
The current set of Super Model is an example of type C

TODO:
- There is a vector for IRQ4. The function does nothing in galpanic but is
  more complicated in the Comad ones. However I'm not triggering it, and
  they seems to work anyway...
- There was a ROM in the newfant set, obj2_14.rom, which was identical to
  Terminator 2's t2.107. I can only assume this was a mistake of the dumper.
- lots of unknown reads and writes, also in galpanic but particularly in
  the Comad ones.
- Is there a background enable register? Or a background clear. fantasia and
  newfant certainly look ugly as they are.

BTANB:
- fantasia and newfant have a service mode but it doesn't work well (text
  is missing or replaced by garbage). This has been verified to happen with
  New Fantasia (1994 copyright) on a Comad 940630 PCB.
*/

#include "emu.h"
#include "galpnipt.h"

#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "kaneko_hit.h"
#include "machine/timer.h"
#include "machine/watchdog.h"
#include "kaneko_tmap.h"
#include "kaneko_spr.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class expro02_state : public driver_device
{
public:
	expro02_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_view2(*this, "view2"),
		m_kaneko_spr(*this, "kan_spr"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_paletteram(*this, "palette"),
		m_fg_ind8_pixram(*this, "fg_ind8ram"),
		m_bg_rgb555_pixram(*this, "bg_rgb555ram"),
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank")
	{ }

	void supmodel(machine_config &config);
	void supmodl2(machine_config &config);
	void zipzap(machine_config &config);
	void fantasia(machine_config &config);
	void fantsia2(machine_config &config);
	void comad(machine_config &config);
	void comad_noview2(machine_config &config);
	void smissw(machine_config &config);
	void galhustl(machine_config &config);
	void expro02(machine_config &config);

	void init_expro02();

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	optional_device<kaneko_view2_tilemap_device> m_view2;
	required_device<kaneko16_sprite_device> m_kaneko_spr;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	required_shared_ptr<uint16_t> m_paletteram;
	required_shared_ptr<uint16_t> m_fg_ind8_pixram;
	required_shared_ptr<uint16_t> m_bg_rgb555_pixram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_okibank;

	uint16_t m_vram_tile_addition[2]; // galsnew

	template<unsigned Layer> void tilebank_w(u8 data);

	void tile_callback(u8 layer, u32 *code);

	void oki_bankswitch_w(u8 data);

	void expro02_palette(palette_device &palette) const;

	uint32_t screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_zipzap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);

	// comad
	uint16_t comad_timer_r();
	uint8_t comad_okim6295_r();
	void comad_map(address_map &map);
	void expro02_map(address_map &map);
	void expro02_video_base_map(address_map &map);
	void expro02_video_base_map_noview2(address_map &map);
	void fantasia_map(address_map &map);
	void fantsia2_map(address_map &map);
	void galhustl_map(address_map &map);
	void oki_map(address_map &map);
	void smissw_map(address_map &map);
	void supmodel_map(address_map &map);
	void supmodl2_map(address_map &map);
	void zipzap_map(address_map &map);
};

/* some weird logic needed for Gals Panic on the EXPRO02 board */
template<unsigned Layer>
void expro02_state::tilebank_w(u8 data)
{
	int val = data << 8;

	if (m_vram_tile_addition[Layer] != val)
	{
		m_vram_tile_addition[Layer] = val;
		m_view2->mark_layer_dirty(Layer);
	}
}

void expro02_state::tile_callback(u8 layer, u32 *code)
{
	u32 res = *code;
	res += m_vram_tile_addition[layer];
	*code = res;
}

void expro02_state::machine_start()
{
	m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
	save_item(NAME(m_vram_tile_addition));
}

void expro02_state::expro02_palette(palette_device &palette) const
{
	// first 2048 colors are dynamic

	// initialize 555 RGB lookup
	for (int i = 0; i < 32768; i++)
		palette.set_pen_color(2048 + i, pal5bit(i >> 5), pal5bit(i >> 10), pal5bit(i >> 0));
}

uint32_t expro02_state::screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  kaneko16_fill_bitmap(machine(),bitmap,cliprect);
	int count;

	count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		for (int x = 0; x < 256; x++)
		{
			uint16_t dat = (m_bg_rgb555_pixram[count] & 0xfffe)>>1;
			dat += 2048;

			// never seen to test
			//if (!(m_bg_rgb555_pixram[count] & 0x0001))
			{
				dest[x] = dat;
			}
			/*
			else
			{
			    dest[x] = 0x0000;
			}
			*/

			count++;
		}
	}

	count = 0;
	for (int y = 0; y < 256; y++)
	{
		uint16_t *const dest = &bitmap.pix(y);
		for (int x = 0; x < 256; x++)
		{
			uint16_t const dat = m_fg_ind8_pixram[count] & 0x7ff;
			if (!(m_paletteram[dat] & 0x0001))
				dest[x] = dat;

			count++;
		}
	}

	screen.priority().fill(0, cliprect);

	if (m_view2)
	{
		m_view2->prepare(bitmap, cliprect);

		for (int i = 0; i < 8; i++)
			m_view2->render_tilemap(screen, bitmap, cliprect, i);
	}

	return 0;
}

uint32_t expro02_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_backgrounds(screen, bitmap, cliprect);
	m_kaneko_spr->render_sprites(cliprect, m_spriteram, m_spriteram.bytes());
	m_kaneko_spr->copybitmap(bitmap, cliprect, screen.priority());
	return 0;
}

uint32_t expro02_state::screen_update_zipzap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_backgrounds(screen, bitmap, cliprect);
	m_kaneko_spr->bootleg_draw_sprites(bitmap,cliprect, m_spriteram, m_spriteram.bytes());
	return 0;
}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( expro02 ) // TODO: at least for the 1994 version of New Fantasia running on the 940630 SWA and SWB should be inverted
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000C, 0x000C, DEF_STR( Lives ) )    PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000C, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Use Button" )        PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Censored Girls" )    PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Show Ending Picture" )   PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)   /* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )


	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)   /* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x00ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


static INPUT_PORTS_START( galsnew )
	PORT_INCLUDE( expro02 )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )    PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( galsnewj )
	PORT_INCLUDE( galsnew )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SWB:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fantasia )
	PORT_INCLUDE( galsnew )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fantasiaa )
	PORT_START("DSW1")
	DIFFICULTY_DEMO_SOUNDS      /* Unknown dip might be freeze/vblank? - code at 0x000734 ('fantasia') or 0x00075a ('newfant') - not called ? */
	GALS_PANIC_JOYSTICK_4WAY(1) /* "Shot2" is shown in "test mode" but not used by the game */

	PORT_START("DSW2")
	COINAGE_TEST_LOC        /* Unknown DSW switch 2 is flip screen? - code at 0x00021c */
	GALS_PANIC_JOYSTICK_4WAY(2) /* "Shot2" is shown in "test mode" but not used by the game */

	SYSTEM_NO_SERVICE       /* MAME may crash when TILT is pressed (see notes), "Service" is shown in "test mode" */
INPUT_PORTS_END

/* Same as 'fantasia', but no "Service Mode" Dip Switch (and thus no "hidden" buttons) */
static INPUT_PORTS_START( missw96 )
	PORT_START("DSW1")
	DIFFICULTY_DEMO_SOUNDS      /* Unknown dip might be freeze/vblank? - code at 0x00074e - not called ? */
	GALS_PANIC_JOYSTICK_4WAY(1)

	PORT_START("DSW2")
	COINAGE_NO_TEST_LOC     /* Unknown DSW switch 2 is flip screen? - code at 0x00021c */
	GALS_PANIC_JOYSTICK_4WAY(2)

	SYSTEM_NO_SERVICE       /* MAME may crash when TILT is pressed (see notes) */
INPUT_PORTS_END

static INPUT_PORTS_START( galhustl )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Lives ) )    PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x0000, "6" )
	PORT_DIPSETTING(      0x0001, "7" )
	PORT_DIPSETTING(      0x0003, "8" )
	PORT_DIPSETTING(      0x0002, "10" )
	PORT_DIPUNUSED_DIPLOC( 0x0004, 0x0004, "SW2:3" )
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SW2:4" )
	PORT_DIPUNUSED_DIPLOC( 0x0010, 0x0010, "SW2:5" )
	PORT_DIPUNUSED_DIPLOC( 0x0020, 0x0020, "SW2:6" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0080, IP_ACTIVE_LOW, "SW2:8" )
	GALS_PANIC_JOYSTICK_8WAY(1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0007, 0x0007, DEF_STR( Coinage ) )  PORT_DIPLOCATION("SW1:1,2,3")
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(      0x0007, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0006, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0005, DEF_STR( 1C_3C ) )
	PORT_DIPNAME( 0x0018, 0x0018, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW1:4,5")
	PORT_DIPSETTING(      0x0010, DEF_STR( Easy ) )         /* 5000 - 7000 */
	PORT_DIPSETTING(      0x0018, DEF_STR( Normal ) )       /* 4000 - 6000 */
	PORT_DIPSETTING(      0x0008, DEF_STR( Hard ) )         /* 6000 - 8000 */
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )      /* 7000 - 9000 */
	PORT_DIPNAME( 0x0060, 0x0060, "Play Time" )     PORT_DIPLOCATION("SW1:6,7")
	PORT_DIPSETTING(      0x0040, "120 Sec" )
	PORT_DIPSETTING(      0x0060, "100 Sec" )
	PORT_DIPSETTING(      0x0020, "80 Sec" )
	PORT_DIPSETTING(      0x0000, "70 Sec" )
	PORT_DIPUNUSED_DIPLOC( 0x0080, 0x0080, "SW1:8" )
	GALS_PANIC_JOYSTICK_8WAY(2)

	SYSTEM_NO_TILT
INPUT_PORTS_END

static INPUT_PORTS_START( zipzap )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, "Additional Obstacles" ) /* Adds 4 Blocker/Bumpers to playing field */
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0000, "1" )
	PORT_DIPSETTING(      0x0008, "2" )
	PORT_DIPSETTING(      0x000c, "3" )
	PORT_DIPSETTING(      0x0004, "4" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_8WAY(1)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x0004, 0x0004, "Select Player Mode" ) /* Amateur, Normal & Exelent Modes */
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( On ) )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Level Skip with Button 2" )
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	GALS_PANIC_JOYSTICK_8WAY(2)

	SYSTEM_NO_TILT
INPUT_PORTS_END


/*************************************
 *
 *  Sound handlers
 *
 *************************************/

void expro02_state::oki_bankswitch_w(u8 data)
{
	m_okibank->set_entry(data & 0x0f);
}


/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/


void expro02_state::expro02_video_base_map(address_map &map)
{
	map(0x500000, 0x51ffff).ram().share("fg_ind8ram");
	map(0x520000, 0x53ffff).ram().share("bg_rgb555ram");
	map(0x580000, 0x583fff).m(m_view2, FUNC(kaneko_view2_tilemap_device::vram_map));
	map(0x600000, 0x600fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // palette?
	map(0x680000, 0x68001f).rw(m_view2, FUNC(kaneko_view2_tilemap_device::regs_r), FUNC(kaneko_view2_tilemap_device::regs_w));
	map(0x700000, 0x700fff).ram().share("spriteram");    // sprites? 0x72f words tested
	map(0x780000, 0x78001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0xd80001, 0xd80001).w(FUNC(expro02_state::tilebank_w<1>));   /* ??? */
	map(0xe80001, 0xe80001).w(FUNC(expro02_state::tilebank_w<0>));   /* ??? */
}

void expro02_state::expro02_video_base_map_noview2(address_map &map)
{
	map(0x500000, 0x51ffff).ram().share("fg_ind8ram");
	map(0x520000, 0x53ffff).ram().share("bg_rgb555ram");
	map(0x580000, 0x583fff).noprw(); // games still makes leftover accesses
	map(0x600000, 0x600fff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette"); // palette?
	map(0x680000, 0x68001f).noprw(); // games still makes leftover accesses
	map(0x700000, 0x700fff).ram().share("spriteram");    // sprites? 0x72f words tested
	map(0x780000, 0x78001f).rw(m_kaneko_spr, FUNC(kaneko16_sprite_device::regs_r), FUNC(kaneko16_sprite_device::regs_w));
	map(0xd80000, 0xd80001).noprw(); // games still makes leftover accesses
	map(0xe80000, 0xe80001).noprw(); // games still makes leftover accesses
}

void expro02_state::expro02_map(address_map &map)
{
	expro02_video_base_map(map);
	map(0x000000, 0x03ffff).rom(); // main program
	map(0x080000, 0x0fffff).rom().region("user2", 0); // other data
	map(0x100000, 0x3fffff).rom().region("user1", 0); // main data
	map(0x400001, 0x400001).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));
	map(0xa00000, 0xa00001).nopw();    /* ??? */
	map(0xc80000, 0xc8ffff).ram();
	map(0xe00000, 0xe00015).rw("calc1", FUNC(kaneko_hit_device::kaneko_hit_r), FUNC(kaneko_hit_device::kaneko_hit_w));
}


// bigger ROM space, OKI commands moved, no CALC1 chip
void expro02_state::fantasia_map(address_map &map)
{
	expro02_video_base_map(map);
	map(0x000000, 0x4fffff).rom();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x800006, 0x800007).noprw(); // ? used ?
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));
	map(0xa00000, 0xa00001).nopw();    /* ??? */
	map(0xc80000, 0xc8ffff).ram();
	map(0xf00000, 0xf00000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
}



void expro02_state::comad_map(address_map &map)
{
	expro02_video_base_map_noview2(map);
	map(0x000000, 0x4fffff).rom();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
//  map(0x800006, 0x800007);    ??
	map(0x80000a, 0x80000b).r(FUNC(expro02_state::comad_timer_r)); /* bits 8-a = timer? palette update code waits for them to be 111 */
	map(0x80000c, 0x80000d).r(FUNC(expro02_state::comad_timer_r)); /* missw96 bits 8-a = timer? palette update code waits for them to be 111 */
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));  /* not sure */
	map(0xc00000, 0xc0ffff).ram();
	map(0xc80000, 0xc8ffff).ram();
	map(0xf00000, 0xf00000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write)); /* fantasia, missw96 */
	map(0xf80000, 0xf80000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write)); /* newfant */
}

void expro02_state::fantsia2_map(address_map &map)
{
	expro02_video_base_map_noview2(map);
	map(0x000000, 0x4fffff).rom();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
//  map(0x800006, 0x800007);    ??
	map(0x800008, 0x800009).r(FUNC(expro02_state::comad_timer_r)); /* bits 8-a = timer? palette update code waits for them to be 111 */
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));  /* not sure */
	map(0xa00000, 0xa00001).nopw();    /* coin counters, + ? */
	map(0xc80000, 0xc80000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write));
	map(0xf80000, 0xf8ffff).ram();
}

void expro02_state::supmodl2_map(address_map &map)
{
	fantsia2_map(map);
	map(0x800006, 0x800007).r(FUNC(expro02_state::comad_timer_r));
	map(0x80000a, 0x80000b).r(FUNC(expro02_state::comad_timer_r));
}




void expro02_state::galhustl_map(address_map &map)
{
	expro02_video_base_map_noview2(map);

	map(0x000000, 0x0fffff).rom();
	map(0x200000, 0x2fffff).rom().region("maincpudata", 0);

	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));
	map(0xa00000, 0xa00001).nopw(); // ?
	map(0xd00000, 0xd00000).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe80000, 0xe8ffff).ram();

	map(0x780000, 0x78001f).noprw(); // prevent sprites being flipped
}

void expro02_state::zipzap_map(address_map &map)
{
	expro02_video_base_map_noview2(map);

	map(0x000000, 0x4fffff).rom();
	map(0x701000, 0x71ffff).ram();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));
	map(0xc00000, 0xc00000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write)); /* fantasia, missw96 */
	map(0xc80000, 0xc8ffff).ram();     // main ram

	map(0x780000, 0x78001f).noprw(); // prevent sprites being flipped
}

void expro02_state::supmodel_map(address_map &map)
{
	expro02_video_base_map_noview2(map);
	map(0x000000, 0x4fffff).rom();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x800006, 0x800007).r(FUNC(expro02_state::comad_timer_r));
	map(0x800008, 0x800009).r(FUNC(expro02_state::comad_timer_r));
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));  /* not sure */
	map(0xa00000, 0xa00001).nopw();
	map(0xc80000, 0xc8ffff).ram();
	map(0xd80000, 0xd80001).nopw();
	map(0xe00012, 0xe00013).nopw();
	map(0xe80000, 0xe80001).nopw();
	map(0xf80000, 0xf80000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write)); /* fantasia, missw96 */
}

void expro02_state::smissw_map(address_map &map)
{
	expro02_video_base_map_noview2(map);
	map(0x000000, 0x4fffff).rom();
	map(0x800000, 0x800001).portr("DSW1");
	map(0x800002, 0x800003).portr("DSW2");
	map(0x800004, 0x800005).portr("SYSTEM");
	map(0x800006, 0x800007).r(FUNC(expro02_state::comad_timer_r));
	map(0x80000e, 0x80000f).r(FUNC(expro02_state::comad_timer_r));
	map(0x900000, 0x900000).w(FUNC(expro02_state::oki_bankswitch_w));  /* not sure */
	map(0xa00000, 0xa00001).nopw();
	map(0xc00000, 0xc0ffff).ram();
	map(0xd80000, 0xd80001).nopw();
	map(0xe00012, 0xe00013).nopw();
	map(0xe80000, 0xe80001).nopw();
	map(0xf00000, 0xf00000).r(FUNC(expro02_state::comad_okim6295_r)).w("oki", FUNC(okim6295_device::write)); /* fantasia, missw96 */
}

void expro02_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}

/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(expro02_state::scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line(3, HOLD_LINE);
	else if(scanline == 0) // vblank-in irq?
		m_maincpu->set_input_line(5, HOLD_LINE);
	else if(scanline == 112) // VDP end task? (controls sprite colors in gameplay)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

/*************************************
 *
 *  Comad specific (kludges?)
 *
 *************************************/

uint16_t expro02_state::comad_timer_r()
{
	return (m_screen->vpos() & 0x07) << 8;
}

/* a kludge! */
uint8_t expro02_state::comad_okim6295_r()
{
	uint16_t retvalue;
//  retvalue = m_oki->read_status(); // doesn't work, causes lockups when girls change..
	retvalue = machine().rand();
	return retvalue;
}


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void expro02_state::expro02(machine_config &config)
{
	static constexpr XTAL CPU_CLOCK = XTAL(12'000'000);
	static constexpr XTAL VDP_CLOCK = XTAL(16'000'000);

	/* basic machine hardware */
	M68000(config, m_maincpu, CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::expro02_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(expro02_state::scanline), "screen", 0, 1);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_raw(VDP_CLOCK / 3, 341, 0, 256, 262, 0, 224); // ~60 Hz
	m_screen->set_screen_update(FUNC(expro02_state::screen_update));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette, FUNC(expro02_state::expro02_palette)).set_format(palette_device::GRBx_555, 2048 + 32768);

	KANEKO_TMAP(config, m_view2);
	m_view2->set_colbase(0x400);
	m_view2->set_offset(0x5b, 0x8, 256, 224);
	m_view2->set_palette(m_palette);
	m_view2->set_tile_callback(FUNC(expro02_state::tile_callback));

	KANEKO_VU002_SPRITE(config, m_kaneko_spr);
	m_kaneko_spr->set_priorities(8,8,8,8); // above all (not verified)
	m_kaneko_spr->set_offsets(0, -0x40);
	m_kaneko_spr->set_palette(m_palette);
	m_kaneko_spr->set_color_base(0x100);

	KANEKO_HIT(config, "calc1").set_type(0);

	/* arm watchdog */
	WATCHDOG_TIMER(config, "watchdog").set_time(attotime::from_seconds(3));  /* a guess, and certainly wrong */

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", CPU_CLOCK / 6, okim6295_device::PIN7_LOW));
	oki.set_addrmap(0, &expro02_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


void expro02_state::comad(machine_config &config)
{
	expro02(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::fantasia_map);

	config.device_remove("calc1");

	// these values might not be correct, behavior differs from original boards
	m_view2->set_invert_flip(1);
	m_view2->set_offset(-256, -216, 256, 224);

	// FIXME: can't be 0 seconds
	subdevice<watchdog_timer_device>("watchdog")->set_time(attotime::from_seconds(0));
}

void expro02_state::comad_noview2(machine_config &config)
{
	comad(config);

	config.device_remove("view2");
}


void expro02_state::fantasia(machine_config &config)
{
	comad_noview2(config);
	m_maincpu->set_clock(10000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::comad_map);
}

void expro02_state::supmodel(machine_config &config)
{
	comad_noview2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::supmodel_map);

	okim6295_device &oki(OKIM6295(config.replace(), "oki", 1584000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &expro02_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void expro02_state::smissw(machine_config &config) // 951127 PCB, 12 & 16 clocks
{
	comad_noview2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::smissw_map);
}

void expro02_state::fantsia2(machine_config &config)
{
	comad_noview2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::fantsia2_map);
}

void expro02_state::supmodl2(machine_config &config)
{
	fantsia2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::supmodl2_map);
}

void expro02_state::galhustl(machine_config &config)
{
	comad_noview2(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::galhustl_map);

	okim6295_device &oki(OKIM6295(config.replace(), "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &expro02_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);

	m_screen->set_screen_update(FUNC(expro02_state::screen_update_zipzap));
}

void expro02_state::zipzap(machine_config &config)
{
	comad_noview2(config);

	/* basic machine hardware */
	m_maincpu->set_addrmap(AS_PROGRAM, &expro02_state::zipzap_map);

	okim6295_device &oki(OKIM6295(config.replace(), "oki", 1056000, okim6295_device::PIN7_HIGH)); // clock frequency & pin 7 not verified
	oki.set_addrmap(0, &expro02_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);

	m_screen->set_screen_update(FUNC(expro02_state::screen_update_zipzap)); // doesn't work with original kaneko_spr implementation
}

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( galsnew ) // EXPRO-02 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pm110e_u87-01.u87",  0x000000, 0x20000, CRC(34e1ee0d) SHA1(567df65b04667a6d35725c4a131fb174acb3ad0a) ) // Export region
	ROM_LOAD16_BYTE( "pm109e_u88-01.u88",  0x000001, 0x20000, CRC(c694255a) SHA1(16faf5ea5ff69a0e7a981021ea5fc09a0aefd7cf) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) // 68000 data
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) // contains real (non-cartoon) women, used after each 3rd round
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF ) // sprites
	// the 06e rom from the other type gals panic board ends up split across 2 roms here
	ROM_LOAD( "pm006e.u83",        0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82",        0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94",        0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	// U93 is an empty socket and not used with this set

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "pm008e.u46", 0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_LOAD( "pm007e.u47", 0x80000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galsnewu ) // EXPRO-02 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pm110u_u87-01.u87", 0x000000, 0x20000, CRC(b793a57d) SHA1(12d57b2b4add532f0d0453c25b30d34b3449d717) ) // US region
	ROM_LOAD16_BYTE( "pm109u_u88-01.u88", 0x000001, 0x20000, CRC(35b936f8) SHA1(d272067f10542d511a777802cafa4d72b93fa5e8) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) // 68000 data
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) // contains real (non-cartoon) women, used after each 3rd round
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF ) // sprites
	// the 06e rom from the other type gals panic board ends up split across 2 roms here
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm019u_u93-01.u93", 0x180000, 0x010000, CRC(3cb79005) SHA1(05a0b993b9071467265067c3762644f46343d8de) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF )   // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.u46", 0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_LOAD( "pm007e.u47", 0x80000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galsnewj ) // EXPRO-02 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pm110j.u87", 0x000000, 0x20000, CRC(220b6df5) SHA1(d653b67bc66ca341bc660c2bb39b05dcf186fcb7) ) // Japan region
	ROM_LOAD16_BYTE( "pm109j.u88", 0x000001, 0x20000, CRC(17721444) SHA1(9d97fe1ddac99105798fc22375a0b89ab316459a) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) // 68000 data
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", ROMREGION_ERASEFF ) // sprites
	// U84 is an empty socket and not used with this set

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF ) // sprites
	// the 06e rom from the other type gals panic board ends up split across 2 roms here
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	// U93 is an empty socket and not used with this set

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "pm008j.u46", 0x00000, 0x80000, CRC(f394670e) SHA1(171f8dc519a13f352e6440aaadebe490c82361f0) )
	ROM_LOAD( "pm007j.u47", 0x80000, 0x80000, CRC(06780287) SHA1(8b9b57f6604b86d6dff42e5e51cd59a7111e1e79) )
ROM_END

ROM_START( galsnewk ) // EXPRO-02 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pm110k.u87", 0x000000, 0x20000, CRC(babe6a71) SHA1(91a5fc5e93affd01f8c6d5a4851233edcf8746f0) ) // Korean region, title is "Ddang Dda Meok Gi"
	ROM_LOAD16_BYTE( "pm109k.u88", 0x000001, 0x20000, CRC(e486d98f) SHA1(9923f1dc69bd2746c06da6a5e518211391052259) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) // 68000 data
	ROM_LOAD16_BYTE( "pm004k.u86", 0x000001, 0x80000, CRC(9a14c8a3) SHA1(c3992eceb8d7d65f781b31dc77bebc73cf9303b6) )
	ROM_LOAD16_BYTE( "pm005k.u85", 0x000000, 0x80000, CRC(33b5d0e3) SHA1(88eef6aff8054b07173da3bb1383fb47a1f7980c) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) // contains real (non-cartoon) women, used after each 3rd round
	ROM_LOAD16_WORD_SWAP( "pm017k.u84", 0x00000, 0x80000, CRC(0c656fb5) SHA1(4610800a460c9f50f7a2ee7b2984bf8e79b62124) )

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF ) // sprites
	// the 06e rom from the other type gals panic board ends up split across 2 roms here
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm19k.u93",  0x180000, 0x010000, CRC(c17d2989) SHA1(895f44a58dcf0065d42125d439dcc10f41563a94) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "pm008k.u46", 0x00000, 0x80000, CRC(7498483f) SHA1(d1f7461c8d1469704cc34460d7283f0a914afc29) )
	ROM_LOAD( "pm007k.u47", 0x80000, 0x80000, CRC(a8dc1fd5) SHA1(c324f7eab7302e4a71d505c915ab2ad591b8ff33) )
ROM_END

ROM_START( galsnewt ) // EXPRO-02 PCB
	ROM_REGION( 0x40000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pm110t_u87-01.u87", 0x000000, 0x20000, CRC(356c65de) SHA1(2f9b6a403488f9a05d8d4b341598f05914cfd9ea) ) // Taiwan region
	ROM_LOAD16_BYTE( "pm109t_u88-01.u88", 0x000001, 0x20000, CRC(6d7b4131) SHA1(fa7e2f1010a66ea069533e00d05133829247f115) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) // 68000 data
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 ) // contains real (non-cartoon) women, used after each 3rd round
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "kan_spr", ROMREGION_ERASEFF ) // sprites
	// The 06e rom from the other type gals panic board ends up split across 2 roms here
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	// U93 is an empty socket and not used with this set

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples (Japanese versions on the Taiwan board)
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "pm008j.u46", 0x00000, 0x80000, CRC(f394670e) SHA1(171f8dc519a13f352e6440aaadebe490c82361f0) )
	ROM_LOAD( "pm007j.u47", 0x80000, 0x80000, CRC(06780287) SHA1(8b9b57f6604b86d6dff42e5e51cd59a7111e1e79) )
ROM_END


/*

Fantasia (c) 1994  Comad & New Japan System

940429
+--------------------------------------+
|TDA2003 2 62256 62256                 |
|        1 62256 62256                 |
|VR1 M6295 62256 62256                 |
|          62256 62256                 |
|          62256 62256                 |
|J                                SCR1*|
|A                                SCR2*|
|M                                  17 |
|M                                     |
|A                1020 6264 6264       |
|             6   62256   3  7         |
|DSWA         8   62256   4  8         |
|             0           5  9         |
|DSWB 12MHz   0           6  10  12  15|
|     16MHz   0          13  16  11  14|
+--------------------------------------+

   CPU: MC68000P10
 Sound: OKI M6295 (rebadged as AD-65)
 Video: TI TPC1020AFN-084C
   OSC: 16MHz & 12MHz
   DSW: 8-way switch x 2
Memory: HY62C256P-15, HY6264ALP-10
   VR1: Sound adjust pot


940307
+--------------------------------------+
|TDA2003 1 62256 62256                 |
|VR1     2 62256 62256                 |
|      M6295                           |
|                                      |
|                                      |
|J                   62256 62256       |
|A                   62256 62256    17 |
|M                   62256 62256  OBJ2*|
|M                         6264   OBJ3*|
|A               1020 1020 6264        |
|             6                        |
|DSWA         8   62256   3   7        |
|             0   62256   4   8  11  14|
|DSWB 12MHz   0   SPA*    5   9  12  15|
|     16MHz   0   SPA*    6  10  13  16|
+--------------------------------------+

   CPU: MC68000P10
 Sound: OKI M6295 (rebadged as AD-65)
 Video: TI TPC1020AFN-084C x 2
   OSC: 16MHz & 12MHz
   DSW: 8-way switch x 2
Memory: KM62256BLP-8, HY6264ALP-10
   VR1: Sound adjust pot

* denotes unpopulated positions

*/

ROM_START( fantasia ) // PCB silkscreened COMAD INDUSTRY CO.,LTD940429 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "16.pro2",    0x000000, 0x80000, CRC(e27c6c57) SHA1(420b66928c46e76fa2496f221691dd6c34542287) ) // PCB location is silkscreened under EPROM sockets
	ROM_LOAD16_BYTE( "13.pro1",    0x000001, 0x80000, CRC(68d27413) SHA1(84cb7d6523325496469d621f6f4da1b719162147) )
	ROM_LOAD16_BYTE( "9.fg_ind87", 0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) )
	ROM_LOAD16_BYTE( "5.fg_ind83", 0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "8.fg_ind86", 0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "4.fg_ind82", 0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "7.fg_ind85", 0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "3.fg_ind81", 0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "10.imag2",   0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "6.imag1",    0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "17.scr3", 0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) )
	// SCR1 and SCR2 are unpopulated

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "2.music1", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) )
	ROM_LOAD( "1.music2", 0x80000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD16_BYTE( "15.obj3", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) )
	ROM_LOAD16_BYTE( "12.obj1", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "14.obj4", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "11.obj2", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END


ROM_START( fantasiaa ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 940307 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2_16.ue17",  0x000000, 0x80000, CRC(0b41ad10) SHA1(386b59f6892cdd2f90df86dc77172919079f0200) )
	ROM_LOAD16_BYTE( "prog1_13.ud17",  0x000001, 0x80000, CRC(a3748726) SHA1(8dc922e01edb777eb853f40556315a34e1aced62) )
	ROM_LOAD16_BYTE( "i-scr6_9.ue16b", 0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) ) // ROMS 3 through 10 contain the same data
	ROM_LOAD16_BYTE( "i-scr5_5.ue16a", 0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) ) // just in different PCB locations
	ROM_LOAD16_BYTE( "i-scr4_8.ue15b", 0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "i-scr3_4.ue15a", 0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "i-scr2_7.ue14b", 0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "i-scr1_3.ue14a", 0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "imag2_10.ue20b", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "imag1_6.ue20a",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1_17.u5", 0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) ) // same data, different PCB location
	// U4 OBJ2 18 and U3 OBJ3 19 are unpopulated

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music1_1.ub6", 0x00000, 0x80000, CRC(af0be817) SHA1(5c8897dcd9957add19ff9553c01ce03fec68b354) ) // This sound sample is different, Earlier ver or BAD???
	ROM_LOAD( "music2_2.uc6", 0x80000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) ) // same data, different PCB location

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD16_BYTE( "g-scr2_15.ul16b", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) ) // same data, different PCB location
	ROM_LOAD16_BYTE( "g-scr1_12.ul16a", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "g-scr4_14.ul19b", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "g-scr3_11.ul19a", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END

ROM_START( fantasiab )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fantasia_16", 0x000000, 0x80000, CRC(c5d93077) SHA1(da615ea0704e77e888dbda664fc9f9fd873edbfa) )
	ROM_LOAD16_BYTE( "fantasia_13", 0x000001, 0x80000, CRC(d88529bd) SHA1(06eb928f4aefe101140140ba7a3ce416215f9e39) )
	ROM_LOAD16_BYTE( "9.fg_ind87",  0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) )
	ROM_LOAD16_BYTE( "5.fg_ind83",  0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "8.fg_ind86",  0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "4.fg_ind82",  0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "7.fg_ind85",  0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "3.fg_ind81",  0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "10.imag2",    0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "6.imag1",     0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "17.scr3", 0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) )
	// SCR1 and SCR2 are unpopulated

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "2.music1", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) )
	ROM_LOAD( "1.music2", 0x80000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )

	ROM_REGION( 0x200000, "view2", ROMREGION_ERASEFF ) // sprites

	ROM_REGION( 0x200000, "gfx3", 0 ) // tiles - encrypted
	ROM_LOAD16_BYTE( "15.obj3", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) )
	ROM_LOAD16_BYTE( "12.obj1", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "14.obj4", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "11.obj2", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END

/*
Fantasia II
Comad, 1997

Game is a copy/clone of Qix etc, with the usual Comad theme.....
The hardware looks much nicer/cleaner and more professionally made than previous
Comad boards I've seen also.


CPU   : MC68000P12
Sound : AD-65 (OKI M6295)
Osc.  : 12.000MHz, 16.000MHz (both near 68000 & PLCC84)
DIP Sw: 8 position (x2)
RAM   : 62256 (x12), 6116 (x4)
PALs  : plenty .....
OTHER : ACTEL A1020B (84 Pin PLCC)

ROMs: (all type 27C040)

music* - oki samples / music
prog*  - main program
obj*   - objects
scr*   - gfx
*/


ROM_START( fantsy95 ) // fantasy 95 - hack of Hot Night below? Are both derived from new fantasia?
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2.12",  0x000000, 0x80000, CRC(1e684da7) SHA1(2104a6fb5f019011009f4faa769afcada90cff97) ) // sldh
	ROM_LOAD16_BYTE( "prog1.7",   0x000001, 0x80000, CRC(dc4e4f6b) SHA1(9934121692a6d32164bef03c72c25dc727438e54) )
	ROM_LOAD16_BYTE( "i-scr2.10", 0x100000, 0x80000, CRC(ab8756ff) SHA1(0a7aa977151962e67b15a7e0f819b1412ff8dbdc) )
	ROM_LOAD16_BYTE( "i-scr1.5",  0x100001, 0x80000, CRC(d8e2ef77) SHA1(ec2c1dcc13e281288b5df43fa7a0b3cdf7357459) )
	ROM_LOAD16_BYTE( "i-scr4.9",  0x200000, 0x80000, CRC(4e52eb23) SHA1(be61c0dc68c49ded2dc6e8852fd92acac4986700) )
	ROM_LOAD16_BYTE( "i-scr3.4",  0x200001, 0x80000, CRC(797731f8) SHA1(571f939a7f85bd5b75a0660621961b531f44f736) )
	ROM_LOAD16_BYTE( "i-scr6.8",  0x300000, 0x80000, CRC(6f8e5239) SHA1(a1c2ec79e80906ca18cf3532ce38a1495ab37e44) )
	ROM_LOAD16_BYTE( "i-scr5.3",  0x300001, 0x80000, CRC(85420e3f) SHA1(d29e81cb1a33dca6232e14a0df2e21c8de45ba71) )
	ROM_LOAD16_BYTE( "i-scr8.11", 0x400000, 0x80000, CRC(33db8177) SHA1(9e9aa890dfa20e5aa6f1caec7d018d992217c2fe) )
	ROM_LOAD16_BYTE( "i-scr7.6",  0x400001, 0x80000, CRC(8662dd01) SHA1(a349c1cd965d3d51c20178fcce2f61ae76f4006a) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1.13", 0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // sldh

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(3117e2ef) SHA1(6581a7104556d44f814c537bbd74998922927034) )
	ROM_LOAD( "music2.2", 0x80000, 0x80000, CRC(0c1109f9) SHA1(0e4ea534a32b1649e2e9bb8af7254b917ec03a90) )
ROM_END

ROM_START( hotnight ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 940630 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12_prog2_4m_12.ue17",   0x000000, 0x80000, CRC(30094b5c) SHA1(e907273aa3b2f4a677339e2f143e1b04fb186ece) ) // *** all ROMs labeled simply 1 to 13, info between underscore
	ROM_LOAD16_BYTE( "7_prog1_4m_7.ud17",     0x000001, 0x80000, CRC(64503285) SHA1(ef9c3151a9e371073fe8fc9cd876c766f2660bcb) ) // *** and PCB location is silkscreened on the PCB
	ROM_LOAD16_BYTE( "10_i-scr2_4m_10.ue16b", 0x100000, 0x80000, CRC(ab8756ff) SHA1(0a7aa977151962e67b15a7e0f819b1412ff8dbdc) ) // == i-scr2.10 from Fantasy '95
	ROM_LOAD16_BYTE( "5_i-scr1_4m_5.ue16a",   0x100001, 0x80000, CRC(d8e2ef77) SHA1(ec2c1dcc13e281288b5df43fa7a0b3cdf7357459) ) // == i-scr1.5 from Fantasy '95
	ROM_LOAD16_BYTE( "9_i-scr4_4m_9.ue15b",   0x200000, 0x80000, CRC(4e52eb23) SHA1(be61c0dc68c49ded2dc6e8852fd92acac4986700) ) // == i-scr4.9 from Fantasy '95
	ROM_LOAD16_BYTE( "4_i-scr3_4m_4.ue15a",   0x200001, 0x80000, CRC(797731f8) SHA1(571f939a7f85bd5b75a0660621961b531f44f736) ) // == i-scr3.4 from Fantasy '95
	ROM_LOAD16_BYTE( "8_i-scr6_4m_8.ue14b",   0x300000, 0x80000, CRC(6f8e5239) SHA1(a1c2ec79e80906ca18cf3532ce38a1495ab37e44) ) // == i-scr6.8 from Fantasy '95
	ROM_LOAD16_BYTE( "3_i-scr5_4m_3.ue14a",   0x300001, 0x80000, CRC(85420e3f) SHA1(d29e81cb1a33dca6232e14a0df2e21c8de45ba71) ) // == i-scr5.3 from Fantasy '95
	ROM_LOAD16_BYTE( "11_i-scr8_4m_11.ue20b", 0x400000, 0x80000, CRC(67006b4e) SHA1(15537c0a46af1104cb0bafe08a3d9d0862ab5dea) )
	ROM_LOAD16_BYTE( "6_i-scr7_4m_6.ue20a",   0x400001, 0x80000, CRC(8979ffaf) SHA1(7ce5a3818dc230fea814a92929727617e6bfc776) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "13_obj1_4m_13.u5", 0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // == obj1.13 from Fantasy '95

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "1_music1_4m_1.ub6", 0x00000, 0x80000, CRC(3117e2ef) SHA1(6581a7104556d44f814c537bbd74998922927034) ) // == music1.1 from Fantasy '95
	ROM_LOAD( "2_music2_4m_2.uc6", 0x80000, 0x80000, CRC(0c1109f9) SHA1(0e4ea534a32b1649e2e9bb8af7254b917ec03a90) ) // == music2.2 from Fantasy '95
ROM_END

ROM_START( fantasian ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 940803 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2_12.ue17",   0x000000, 0x80000, CRC(8bb70be1) SHA1(d8854defcffa4cc1a6f4087acdaa05cc80444089) )
	ROM_LOAD16_BYTE( "prog1_7.ud17",    0x000001, 0x80000, CRC(d1616a3e) SHA1(49a647c16d3bdb895ca14871c1f5cb5abcf59c9a) )
	ROM_LOAD16_BYTE( "i-scr2_10.ue16b", 0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) ) // data roms same as expro02.c sets, but different positions
	ROM_LOAD16_BYTE( "i-scr1_5.ue16a",  0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "i-scr4_9.ue15b",  0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "i-scr3_4.ue15a",  0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "i-scr6_8.ue14b",  0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "i-scr5_3.ue14a",  0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "i-scr8_11.ue20b", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "i-scr7_6.ue20a",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1_13.u5", 0x00000, 0x80000, CRC(f99751f5) SHA1(10f0a2e369abc36a6df2f0c9879ffb7071ee214b) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music1_1.ub6", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) ) // sample roms same as expro02.c sets, but different positions
	ROM_LOAD( "music2_2.uc6", 0x80000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )
ROM_END

ROM_START( newfant ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 940630 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2.12", 0x000000, 0x80000, CRC(de43a457) SHA1(91db13f63b46146131c58e775119ea3b073ca409) )
	ROM_LOAD16_BYTE( "prog1.07", 0x000001, 0x80000, CRC(370b45be) SHA1(775873df9d3af803dbd1a392a45cad5f37b1b1c7) )
	ROM_LOAD16_BYTE( "iscr2.10", 0x100000, 0x80000, CRC(4f2da2eb) SHA1(4f0b72327d1bdfad24d822953f45218bfae29cff) )
	ROM_LOAD16_BYTE( "iscr1.05", 0x100001, 0x80000, CRC(63c6894f) SHA1(213544da570a167f3411357308c576805f6882f3) )
	ROM_LOAD16_BYTE( "iscr4.09", 0x200000, 0x80000, CRC(725741ec) SHA1(3455cf0aed9653c66b8b2f905ad683687d517419) )
	ROM_LOAD16_BYTE( "iscr3.04", 0x200001, 0x80000, CRC(51d6b362) SHA1(bcd57643ac9d79c150714ec6b6a2bb8a24acf7a5) )
	ROM_LOAD16_BYTE( "iscr6.08", 0x300000, 0x80000, CRC(178b2ef3) SHA1(d3c092a282278968a319e06731481570f217d404) )
	ROM_LOAD16_BYTE( "iscr5.03", 0x300001, 0x80000, CRC(d2b5c5fa) SHA1(80fde69bc5f4e958b5d57a5179b6e601192780f4) )
	ROM_LOAD16_BYTE( "iscr8.11", 0x400000, 0x80000, CRC(f4148528) SHA1(4e27fff0b7ead068a159b3ed80c5793a6166fc4e) )
	ROM_LOAD16_BYTE( "iscr7.06", 0x400001, 0x80000, CRC(2dee0c31) SHA1(1097006e6e5d16b24fb71615b6c0754fe0ecbe33) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "nf95obj1.13", 0x00000, 0x80000, CRC(e6d1bc71) SHA1(df0b6c1742c01991196659bab2691230323e7b8d) ) // sldh

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "musc1.01", 0x00000, 0x80000, CRC(10347fce) SHA1(f5fbe8ef363fe18b7104be5d2fa92943d1a5d7a2) )
	ROM_LOAD( "musc2.02", 0x80000, 0x80000, CRC(b9646a8c) SHA1(e9432261ac86e4251a2c97301c6d014c05110a9c) )
ROM_END

ROM_START( newfanta ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 940630 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12.ue17",  0x000000, 0x80000, CRC(de43a457) SHA1(91db13f63b46146131c58e775119ea3b073ca409) )
	ROM_LOAD16_BYTE( "7.ud17",   0x000001, 0x80000, CRC(370b45be) SHA1(775873df9d3af803dbd1a392a45cad5f37b1b1c7) )
	ROM_LOAD16_BYTE( "10.ue16b", 0x100000, 0x80000, CRC(4f2da2eb) SHA1(4f0b72327d1bdfad24d822953f45218bfae29cff) )
	ROM_LOAD16_BYTE( "5.ue16a",  0x100001, 0x80000, CRC(63c6894f) SHA1(213544da570a167f3411357308c576805f6882f3) )
	ROM_LOAD16_BYTE( "9.ue15b",  0x200000, 0x80000, CRC(725741ec) SHA1(3455cf0aed9653c66b8b2f905ad683687d517419) )
	ROM_LOAD16_BYTE( "4.ue15a",  0x200001, 0x80000, CRC(51d6b362) SHA1(bcd57643ac9d79c150714ec6b6a2bb8a24acf7a5) )
	ROM_LOAD16_BYTE( "8.ue14b",  0x300000, 0x80000, CRC(178b2ef3) SHA1(d3c092a282278968a319e06731481570f217d404) )
	ROM_LOAD16_BYTE( "3.ue14a",  0x300001, 0x80000, CRC(d2b5c5fa) SHA1(80fde69bc5f4e958b5d57a5179b6e601192780f4) )
	ROM_LOAD16_BYTE( "11.ue20b", 0x400000, 0x80000, CRC(f4148528) SHA1(4e27fff0b7ead068a159b3ed80c5793a6166fc4e) )
	ROM_LOAD16_BYTE( "6.ue20a",  0x400001, 0x80000, CRC(2dee0c31) SHA1(1097006e6e5d16b24fb71615b6c0754fe0ecbe33) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "13.u5", 0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "1.ub6", 0x00000, 0x80000, CRC(10347fce) SHA1(f5fbe8ef363fe18b7104be5d2fa92943d1a5d7a2) )
	ROM_LOAD( "2.uc6", 0x80000, 0x80000, CRC(b9646a8c) SHA1(e9432261ac86e4251a2c97301c6d014c05110a9c) )
ROM_END

ROM_START( missw96 ) // found on PCBs silkscreened COMAD INDUSTRY CO.,LTD 951005 MADE IN KOREA  or  COMAD INDUSTRY CO.,LTD 951127 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mw96_10.bin", 0x000000, 0x80000, CRC(b1309bb1) SHA1(3cc7a903cb007d8fc0f836a33780c1c9231d1629) )
	ROM_LOAD16_BYTE( "mw96_06.bin", 0x000001, 0x80000, CRC(a5892bb3) SHA1(99130eb0af307fe66c9668414475e003f9c7d969) )
	ROM_LOAD16_BYTE( "mw96_09.bin", 0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin", 0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin", 0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin", 0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin", 0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin", 0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "mw96_11.bin", 0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "mw96_01.bin", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "mw96_02.bin", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96a ) // found on PCBs silkscreened COMAD INDUSTRY CO.,LTD 951005 MADE IN KOREA  or  COMAD INDUSTRY CO.,LTD 951127 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mw96n2_10.prog2", 0x000000, 0x80000, CRC(563ce811) SHA1(4013b303dc7fdfcd2b5b91f12a950eb71b27714a) )
	ROM_LOAD16_BYTE( "mw96n2_6.prog1",  0x000001, 0x80000, CRC(98e91a3b) SHA1(a135458e0373b528498408ac3288a01a666f3522) )
	ROM_LOAD16_BYTE( "mw96_09.bin",     0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",     0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",     0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",     0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",     0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",     0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "mw96_11.bin", 0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "mw96_01.bin", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "mw96_02.bin", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96b ) // found on PCBs silkscreened COMAD INDUSTRY CO.,LTD 951005 MADE IN KOREA  or  COMAD INDUSTRY CO.,LTD 951127 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mw96n3_10.prog2", 0x000000, 0x80000, CRC(67bde86b) SHA1(7457a4c130a9ab1c75645e2a662a87af3fee8bba) )
	ROM_LOAD16_BYTE( "mw96n3_6.prog1",  0x000001, 0x80000, CRC(de99cc48) SHA1(ffa2597083c412fb943724048d8d5cc7bd9be11c) )
	ROM_LOAD16_BYTE( "mw96_09.bin",     0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",     0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",     0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",     0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",     0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",     0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "mw96_11.bin", 0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "mw96_01.bin", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "mw96_02.bin", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96c ) // found on PCBs silkscreened COMAD INDUSTRY CO.,LTD 951005 MADE IN KOREA  or  COMAD INDUSTRY CO.,LTD 951127 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "10_prog2.ue17", 0x000000, 0x80000, CRC(36a7beb6) SHA1(11f6aef506a4e357442207fef760401757deaaeb) )
	ROM_LOAD16_BYTE( "6_prog1.ud17",  0x000001, 0x80000, CRC(e70b562f) SHA1(4affd40ab7f962824d1c7be22ea6819cf06d6347) )
	ROM_LOAD16_BYTE( "9_im1-b.ue16b", 0x100000, 0x80000, CRC(eedc24f8) SHA1(cef822c1e3f09c484d03964f02d761139aac9d76) )
	ROM_LOAD16_BYTE( "5_im1-a.ue16a", 0x100001, 0x80000, CRC(bb0eb7d7) SHA1(6952d153afa90924754c11872497ec83ae650220) )
	ROM_LOAD16_BYTE( "8_im2-b.ue15b", 0x200000, 0x80000, CRC(68dd67b2) SHA1(322f3eb84277568356ae0a09f71337bd525f379a) )
	ROM_LOAD16_BYTE( "4_im2-a.ue15a", 0x200001, 0x80000, CRC(2b39ec56) SHA1(8ea1483050287c68063e54c4de27bd82ad942c53) )
	ROM_LOAD16_BYTE( "7_im3_b.ue14b", 0x300000, 0x80000, CRC(7fd5ca2c) SHA1(7733bd0529953bdae718bf28053d173e5ec3ca92) )
	ROM_LOAD16_BYTE( "3_im3-a.ue14a", 0x300001, 0x80000, CRC(4ba5dab7) SHA1(81d7b6fde6d9405793f60ee7d15a15a511396332) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "20_obj1.u5", 0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "1_music1.ub6", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "2_music2.uc6", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END


ROM_START( missmw96 )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "mmw96_10.bin", 0x000000, 0x80000, CRC(45ed1cd9) SHA1(a75b1b6cddde065e6d7f7355a746819c8268c24f) )
	ROM_LOAD16_BYTE( "mmw96_06.bin", 0x000001, 0x80000, CRC(52ec9e5d) SHA1(20b7cc923e9d55e391b09d96248837bb8f28a176) )
	ROM_LOAD16_BYTE( "mmw96_09.bin", 0x100000, 0x80000, CRC(6c458b05) SHA1(249490c45cdecd6496338286a9ab6a6137cefcd0) )
	ROM_LOAD16_BYTE( "mmw96_05.bin", 0x100001, 0x80000, CRC(48159555) SHA1(a7c736f9e41915d06b7242e427282c421c4a8283) )
	ROM_LOAD16_BYTE( "mmw96_08.bin", 0x200000, 0x80000, CRC(1dc72b07) SHA1(fdbdf8298fe98d74ed2a76abf60f60af1c27a65d) )
	ROM_LOAD16_BYTE( "mmw96_04.bin", 0x200001, 0x80000, CRC(fc3e18fa) SHA1(b3ad254aab982dc75a10c2cf2b3815c2fdbba914) )
	ROM_LOAD16_BYTE( "mmw96_07.bin", 0x300000, 0x80000, CRC(001572bf) SHA1(cdf59c624baaeaea70985ee6f2f2fed08a8dfa61) )
	ROM_LOAD16_BYTE( "mmw96_03.bin", 0x300001, 0x80000, CRC(22204025) SHA1(442e7f754c65c598983d6f897a60870d7759c823) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "mmw96_11.bin", 0x00000, 0x80000, CRC(7d491f8c) SHA1(63f580bd65579cac70b90eaa0e7f2413ef1597b8) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "mw96_01.bin", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "mw96_02.bin", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( smissw ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 951127 MADE IN KOREA
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "10_prog2.ue17", 0x000000, 0x80000, CRC(e99e520f) SHA1(edd06a3b0f8d30a4020e6ea452abb0afd79d426a) )
	ROM_LOAD16_BYTE( "6_prog1.ud17",  0x000001, 0x80000, CRC(22831657) SHA1(eeabcdef543048ccceabc4c3b4b288aec959a14f) )
	ROM_LOAD16_BYTE( "9_im1-b.ue16b", 0x100000, 0x80000, CRC(fff1eee4) SHA1(1b88d45b5cc0b5a03296d4dc950e570fa4dc19c2) )
	ROM_LOAD16_BYTE( "5_im1-a.ue16a", 0x100001, 0x80000, CRC(2134a72d) SHA1(f907ec8a1d6e5755a821e69564074ff05e426bb1) )
	ROM_LOAD16_BYTE( "8_im2-b.ue15b", 0x200000, 0x80000, CRC(cf44b638) SHA1(0fe5bdb62492c31c3efffa6d85f5d6a3b4ddb2e0) )
	ROM_LOAD16_BYTE( "4_im2-a.ue15a", 0x200001, 0x80000, CRC(d22b270f) SHA1(21bd2ced1b5fb3c08687addaa890ee621a56fff0) )
	ROM_LOAD16_BYTE( "7_im3-b.ue14b", 0x300000, 0x80000, CRC(12a9441d) SHA1(d9cd51e0c3ffac5fc561e0927c419bce0157337e) )
	ROM_LOAD16_BYTE( "3_im3-a.ue14a", 0x300001, 0x80000, CRC(8c656fc9) SHA1(c3fe5de7cd6cd520bbd205ec62ac0dda51f71eeb) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "15_obj11.u5", 0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "1_music1.ub6", 0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_LOAD( "2_music2.uc6", 0x80000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END


ROM_START( fantsia2 ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 961210 MADE IN KOREA   (PCB has an additional OSC marked 18MHz, currently unpopulated)
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2.g17", 0x000000, 0x80000, CRC(57c59972) SHA1(4b1da928b537cf340a67026d07bc3dfc078b0d0f) )
	ROM_LOAD16_BYTE( "prog1.f17", 0x000001, 0x80000, CRC(bf2d9a26) SHA1(92f0c1bd32f1e5e0ede3ba847242a212dfae4986) )
	ROM_LOAD16_BYTE( "scr2.g16",  0x100000, 0x80000, CRC(887b1bc5) SHA1(b6fcdc8a56ea25758f363224d256e9b6c8e30244) )
	ROM_LOAD16_BYTE( "scr1.f16",  0x100001, 0x80000, CRC(cbba3182) SHA1(a484819940fa1ef18ce679465c31075798748bac) )
	ROM_LOAD16_BYTE( "scr4.g15",  0x200000, 0x80000, CRC(ce97e411) SHA1(be0ed41362db03f384229c708f2ba4146e5cb501) )
	ROM_LOAD16_BYTE( "scr3.f15",  0x200001, 0x80000, CRC(480cc2e8) SHA1(38fe57ba1e34537f8be65fcc023ccd43369a5d94) )
	ROM_LOAD16_BYTE( "scr6.g14",  0x300000, 0x80000, CRC(b29d49de) SHA1(854b76755acf58fb8a4648a0ce72ea6bdf26c555) )
	ROM_LOAD16_BYTE( "scr5.f14",  0x300001, 0x80000, CRC(d5f88b83) SHA1(518a1f6732149f2851bbedca61f7313c39beb91b) )
	ROM_LOAD16_BYTE( "scr8.g20",  0x400000, 0x80000, CRC(694ae2b3) SHA1(82b7a565290fce07c8393af4718fd1e6136928e9) )
	ROM_LOAD16_BYTE( "scr7.f20",  0x400001, 0x80000, CRC(6068712c) SHA1(80a136d76dca566772e34d832ac11b8c7d6ce9ab) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1.1i", 0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i", 0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music2.1b", 0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_LOAD( "music1.1a", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( fantsia2a )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "fnt2-22.bin", 0x000000, 0x80000, CRC(a3a92c4b) SHA1(6affdcb57e1e0a77c7cc33135dafe86843e9e3d8) )
	ROM_LOAD16_BYTE( "fnt2-17.bin", 0x000001, 0x80000, CRC(d0ce4493) SHA1(9cec088e6630555b6d584df23236c279909820cf) )
	ROM_LOAD16_BYTE( "fnt2-21.bin", 0x100000, 0x80000, CRC(e989c2e7) SHA1(c9eea2a89843cdd9db4a4a0539d0315c125e3e02) )
	ROM_LOAD16_BYTE( "fnt2-16.bin", 0x100001, 0x80000, CRC(8c06d372) SHA1(14fe2c8450f0f2e11e204dd524bfe32a72ddc144) )
	ROM_LOAD16_BYTE( "fnt2-20.bin", 0x200000, 0x80000, CRC(6e9f1e65) SHA1(b6f1eb1a52de18ed5b17de3ef365e5c041d15314) )
	ROM_LOAD16_BYTE( "fnt2-15.bin", 0x200001, 0x80000, CRC(85cbeb2b) SHA1(a213b461019ddb3b319b9815a76c6fb2ecfbe937) )
	ROM_LOAD16_BYTE( "fnt2-19.bin", 0x300000, 0x80000, CRC(7953226a) SHA1(955c779eae496688be2ed416d879d6e83c888368) )
	ROM_LOAD16_BYTE( "fnt2-14.bin", 0x300001, 0x80000, CRC(10d8ccff) SHA1(bf4c49d85556edf49289631ee6178d3fb7dea2cc) )
	ROM_LOAD16_BYTE( "fnt2-18.bin", 0x400000, 0x80000, CRC(4cdaeda3) SHA1(f5b478e49b59496865982409517654f48296565d) )
	ROM_LOAD16_BYTE( "fnt2-13.bin", 0x400001, 0x80000, CRC(68c7f042) SHA1(ed3c864f3d91377fec78f19897ba0b0d2bcf0d2b) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1.1i", 0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i", 0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music2.1b", 0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_LOAD( "music1.1a", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

// sole change seems to be copyright date, PCB has chip references instead of grid references.  Not correcting all labels in other sets in case these are legitimate labels
ROM_START( fantsia2n )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2.g17", 0x000000, 0x80000, CRC(57c59972) SHA1(4b1da928b537cf340a67026d07bc3dfc078b0d0f) )
	ROM_LOAD16_BYTE( "prog1.f17", 0x000001, 0x80000, CRC(bf2d9a26) SHA1(92f0c1bd32f1e5e0ede3ba847242a212dfae4986) )
	ROM_LOAD16_BYTE( "scr2.g16",  0x100000, 0x80000, CRC(887b1bc5) SHA1(b6fcdc8a56ea25758f363224d256e9b6c8e30244) )
	ROM_LOAD16_BYTE( "scr1.f16",  0x100001, 0x80000, CRC(cbba3182) SHA1(a484819940fa1ef18ce679465c31075798748bac) )
	ROM_LOAD16_BYTE( "scr4.g15",  0x200000, 0x80000, CRC(ce97e411) SHA1(be0ed41362db03f384229c708f2ba4146e5cb501) )
	ROM_LOAD16_BYTE( "scr3.f15",  0x200001, 0x80000, CRC(480cc2e8) SHA1(38fe57ba1e34537f8be65fcc023ccd43369a5d94) )
	ROM_LOAD16_BYTE( "scr6.g14",  0x300000, 0x80000, CRC(b29d49de) SHA1(854b76755acf58fb8a4648a0ce72ea6bdf26c555) )
	ROM_LOAD16_BYTE( "scr5.f14",  0x300001, 0x80000, CRC(d5f88b83) SHA1(518a1f6732149f2851bbedca61f7313c39beb91b) )
	ROM_LOAD16_BYTE( "scr8.g20",  0x400000, 0x80000, CRC(694ae2b3) SHA1(82b7a565290fce07c8393af4718fd1e6136928e9) )
	ROM_LOAD16_BYTE( "scr7.f20",  0x400001, 0x80000, CRC(6068712c) SHA1(80a136d76dca566772e34d832ac11b8c7d6ce9ab) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "23_obj1.u5", 0x00000, 0x80000, CRC(b45c9234) SHA1(b5eeec91b9c6952b338130458405997e1a51bf2f) )
	ROM_LOAD( "obj2.2i",    0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music2.1b", 0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_LOAD( "music1.1a", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( supmodl2 ) // PCB silkscreened COMAD INDUSTRY CO.,LTD 961210 MADE IN KOREA   (PCB has an additional OSC marked 18MHz, currently unpopulated)
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "12_prog2.ue17",   0x000000, 0x80000, CRC(9107f65d) SHA1(c66c1cfeae2afc5a1bf0d6385291592c0f9b1578) )
	ROM_LOAD16_BYTE( "7_prog1.ud17",    0x000001, 0x80000, CRC(0d9253a7) SHA1(2c7e84bfbf648c22acf76dbfcbb9e2416225abc9) )
	ROM_LOAD16_BYTE( "11_i-scr2.ue16b", 0x100000, 0x80000, CRC(b836c1f3) SHA1(56c611313336dbb0ebcdba42ba73c5738ca7c8d7) )
	ROM_LOAD16_BYTE( "6_i-scr1.ue16a",  0x100001, 0x80000, CRC(d56cac96) SHA1(31fe266ba6abbd51ed7ff6089ebc86e528ac249a) )
	ROM_LOAD16_BYTE( "10_i-scr4.ue15b", 0x200000, 0x80000, CRC(aa85a247) SHA1(6508c0f8b1bc397599ecb0ae5c9a9ebcc532bdd8) )
	ROM_LOAD16_BYTE( "5_i-scr3.ue15a",  0x200001, 0x80000, CRC(3e9bd17a) SHA1(cbbd90120fe9504eac05bb6c8f38d16b44d8b475) )
	ROM_LOAD16_BYTE( "9_i-scr6.ue14b",  0x300000, 0x80000, CRC(d21355dc) SHA1(f449f6b2c815453545a798e6d7081327b8c1677c) )
	ROM_LOAD16_BYTE( "4_i-scr5.ue14a",  0x300001, 0x80000, CRC(d1c9155c) SHA1(9d50e0875feab77a91cc1d8fe575ad6361bfada2) )
	ROM_LOAD16_BYTE( "8_i-scr8.ue20b",  0x400000, 0x80000, CRC(7ffe761e) SHA1(63d0b6e3e34a465e038f8f96ae5f3f1e3666aaaa) )
	ROM_LOAD16_BYTE( "3_i-scr7.ue20a",  0x400001, 0x80000, CRC(d2bb19ef) SHA1(b8dee4c64915ae9d7e3a79a0d32f692f5e2a0d06) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "13_obj1.u5", 0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "14_obj2.u4", 0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "1_music1.ub6", 0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_LOAD( "2_music2.uc6", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( wownfant )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ep-4001 42750001 u81.u81",     0x000000, 0x080000, CRC(9942d200) SHA1(d2f69c0949881ef4aef202b564eac069c030a497) )
	ROM_LOAD16_BYTE( "ep-4001 42750001 u80.u80",     0x000001, 0x080000, CRC(17359eeb) SHA1(90bb9da6bdf56fa9eb0ad03691750518a2a3f879) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 1.bin", 0x100000, 0x200000, CRC(c318e841) SHA1(ba7af736d3b0accca474b0de1c8299eb3c449ef9) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 2.bin", 0x300000, 0x200000, CRC(8871dc3a) SHA1(8e028f1430474df19bb9a912ee9e407fe4582558) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "ep-4001 42750001 u113.u113", 0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) )
	ROM_LOAD( "ep-4001 42750001 u112.u112", 0x80000, 0x80000, CRC(51f4b604) SHA1(52e8ce0a2c1b9b00f04e0c775789bc550bad8ae0) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "ep-4001 42750001 u4.u4", 0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // almost the same as fantasia2, just some changes to the sample references in the header
	ROM_LOAD( "ep-4001 42750001 u1.u1", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( wownfanta )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "2.u81",      0x000000, 0x080000, CRC(159178f8) SHA1(6013346218131941e8d964fcd43a61df04206749) )
	ROM_LOAD16_BYTE( "1.u80",      0x000001, 0x080000, CRC(509bc2d2) SHA1(3dd277640403f189eed6f91b60e6b99bdc2019e8) )
	ROM_LOAD16_WORD_SWAP( "3.bin", 0x100000, 0x200000, CRC(4d082ec1) SHA1(fab90eb2deb0aaf30a96eb7fcdb895a0b8da3857) )
	ROM_LOAD16_WORD_SWAP( "4.bin", 0x300000, 0x200000, CRC(aee91094) SHA1(c541fc2618461d7d143437ae3b3cfe5d65fcbe8d) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "5.u113", 0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) )
	ROM_LOAD( "6.u112", 0x80000, 0x80000, CRC(0013473e) SHA1(e62416111f05ce586e50dbbda9ffee9fcd0985c2) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "8.u4", 0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // almost the same as fantasia2, just some changes to the sample references in the header
	ROM_LOAD( "7.u1", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( missw02 ) // all ROMs had non descript stickers, no labels
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "u81",       0x000000, 0x080000, CRC(86ca4d5f) SHA1(0f7781b10034383615836154b486a2a9f71c6a16) ) // stickered as Miss World 2002 Rev.A
	ROM_LOAD16_BYTE( "u80",       0x000001, 0x080000, CRC(96d40592) SHA1(9ef7d37b68b2c748238d7da6fb4c2e67566b0784) )
	ROM_LOAD16_WORD_SWAP( "gfx1", 0x100000, 0x200000, CRC(fdfe36ba) SHA1(128277e44e2368267e097bb3510c797cc690d1ff) )
	ROM_LOAD16_WORD_SWAP( "gfx2", 0x300000, 0x200000, CRC(aa769a81) SHA1(2beb6da9327ddce7bec934bcf610061fc3b9ab09) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "u113", 0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) ) // same as wowfant
	ROM_LOAD( "u112", 0x80000, 0x80000, CRC(51f4b604) SHA1(52e8ce0a2c1b9b00f04e0c775789bc550bad8ae0) ) // same as wowfant

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "u4", 0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // these 2 same as wowfant
	ROM_LOAD( "u1", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( missw02d )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "8.u81",      0x000000, 0x080000, CRC(316666d0) SHA1(0ebebc55b49c1d00adac2b04bcfe9cfb317e8e74) ) // stickered as Miss World 2002 Rev.A
	ROM_LOAD16_BYTE( "7.u80",      0x000001, 0x080000, CRC(d61f4d18) SHA1(caef5fb221cafc354875ef5b68e84419f91c0db7) )
	ROM_LOAD16_WORD_SWAP( "3.bin", 0x100000, 0x200000, CRC(fdfe36ba) SHA1(128277e44e2368267e097bb3510c797cc690d1ff) )
	ROM_LOAD16_WORD_SWAP( "4.bin", 0x300000, 0x200000, CRC(aa769a81) SHA1(2beb6da9327ddce7bec934bcf610061fc3b9ab09) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "6.u113", 0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) ) // same as wowfant
	ROM_LOAD( "5.u112", 0x80000, 0x80000, CRC(ead3411d) SHA1(ee1b071e4a556b66ecdedcdc7e1ee60851c0ddb0) )

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "2.u4", 0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // these 2 same as wowfant
	ROM_LOAD( "1.u1", 0x80000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( galhustl ) // An original PCB has been seen with genuine AFEGA labels
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "ue17.3", 0x00000, 0x80000, CRC(b2583dbb) SHA1(536f4aa2246ec816c4f270f9d42acc090718ee8b) ) // Also found as AFEGA 3
	ROM_LOAD16_BYTE( "ud17.4", 0x00001, 0x80000, CRC(470a3668) SHA1(ad86e96ab8f1f5da23fb1feaabfb9c757965418e) ) // Also found as AFEGA 4

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) // 68000 data

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "galhstl1.ub6", 0x00000, 0x80000, CRC(23848790) SHA1(2e77fbe04f46e258daecb4c5917e383c7c06a306) ) // Also found as AFEGA 1
	ROM_LOAD( "galhstl2.uc6", 0x80000, 0x80000, CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) ) // Also found as AFEGA 2

	ROM_REGION( 0x100000, "kan_spr", 0 )
	ROM_LOAD( "galhstl5.u5", 0x00000, 0x80000, CRC(44a18f15) SHA1(1217cf7fbbb442358b15016099efeface5dcbd22) ) // Also found as AFEGA 5
ROM_END

ROM_START( pgalvip ) // this set has extra data roms for the gfx
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "afega_15.ue17", 0x00000, 0x20000, CRC(050060ca) SHA1(1e2a1d6aaf5038269d192baf3520f4af7a299325) )
	ROM_LOAD16_BYTE( "afega_16.ud17", 0x00001, 0x20000, CRC(d32e4052) SHA1(632d9affee92a526c0e9399230ecf485922c6df4) )

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) // 68000 data
	ROM_LOAD16_BYTE( "afega_13.rob1", 0x00000, 0x80000, CRC(ac51ef72) SHA1(01acb29ff474c52fcb323cdb14e0d6f804c93255) )
	ROM_LOAD16_BYTE( "afega_14.roa1", 0x00001, 0x80000, CRC(0877c00f) SHA1(91c325d6c21045f08abca86a9c4d46023363dd2e) )

	ROM_REGION( 0x140000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "afega_12.ub6", 0x00000, 0x20000, CRC(d32a6c0c) SHA1(6f16043ed1e174b42de83462e2ea7a601bac6678) )
	ROM_LOAD( "afega_11.uc6", 0x80000, 0x80000, CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) )

	ROM_REGION( 0x100000, "kan_spr", 0 )
	ROM_LOAD( "afega_17.u5", 0x00000, 0x80000, CRC(a8a50745) SHA1(e51963947c7a7556b8531d172b9d7bf9f321b21b) )
ROM_END

ROM_START( pgalvipa ) // this set is more like Gals Hustler
	ROM_REGION( 0x100000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "pgalvip_3.ue17", 0x00000, 0x80000, CRC(a48e8255) SHA1(7e8f1747420ff0d599d340915712827ca2eb3092) )
	ROM_LOAD16_BYTE( "pgalvip_4.ud17", 0x00001, 0x80000, CRC(829a2085) SHA1(3ff5f2bb730572202cd427abd7f91dd886537ab6) )

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) // 68000 data

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "pgalvip_1.ub6", 0x00000, 0x20000, CRC(d32a6c0c) SHA1(6f16043ed1e174b42de83462e2ea7a601bac6678) )
	ROM_LOAD( "pgalvip_2.uc6", 0x80000, 0x80000, CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) )

	ROM_REGION( 0x100000, "kan_spr", 0 )
	ROM_LOAD( "pgalvip_5.u5", 0x00000, 0x80000, CRC(2d6e5a90) SHA1(b5487e5764d83dfecd982a8614d213c9075fbee4) )
ROM_END


ROM_START( supmodel )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "prog2.12",  0x000000, 0x80000, CRC(714b7e74) SHA1(a4f7754a4b04729084ccb1359f9bdfbad6150222) )
	ROM_LOAD16_BYTE( "prog1.7",   0x000001, 0x80000, CRC(0bb858de) SHA1(bd2039fa46fce89289e99a790400bd567f90105e) )
	ROM_LOAD16_BYTE( "i-scr2.10", 0x100000, 0x80000, CRC(d07ec0ce) SHA1(88997254ea2bffa83ab4a77087905cf646ee3c12) )
	ROM_LOAD16_BYTE( "i-scr1.5",  0x100001, 0x80000, CRC(a96a8bde) SHA1(e93de2df1391a8e94d655e1c9e148196e692e661) )
	ROM_LOAD16_BYTE( "i-scr4.9",  0x200000, 0x80000, CRC(e959cab5) SHA1(13d744aa71d9485a4530418536c38a542a269e27) )
	ROM_LOAD16_BYTE( "i-scr3.4",  0x200001, 0x80000, CRC(4bf5e082) SHA1(14ab9ebe0c7a2154275b0aeb76f99d73552d862f) )
	ROM_LOAD16_BYTE( "i-scr6.8",  0x300000, 0x80000, CRC(e71337c2) SHA1(be1b532e66e70f7d30b657a88c1f9b154187636e) )
	ROM_LOAD16_BYTE( "i-scr5.3",  0x300001, 0x80000, CRC(641ccdfb) SHA1(f48dc0461bc49cfe4adcf769e9abfe83efa077a1) )
	ROM_LOAD16_BYTE( "i-scr8.11", 0x400000, 0x80000, CRC(7c1813c8) SHA1(80fe97ac640847360529edfb728955e1067b0c14) )
	ROM_LOAD16_BYTE( "i-scr7.6",  0x400001, 0x80000, CRC(19c73268) SHA1(aa6dc8c817a2e9707ea74e219ab34cf826223741) )

	ROM_REGION( 0x80000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "obj1.13", 0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // sldh

	ROM_REGION( 0x100000, "oki", 0 ) // OKIM6295 samples
	// 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(2b1f6655) SHA1(e7b52cf4bd16590c598c375d5a97b724bc9ef631) )
	ROM_LOAD( "music2.2", 0x80000, 0x80000, CRC(cccae65a) SHA1(5e4e2e51884eaf191f103aa189ff33371fc91d6d) )
ROM_END


/*

Zip & Zap

Zip Zap (pcb marked Barko Corp 950509)

1x 68k
1x Oki m6295
1x osc 12mhz
1x osc 16mhz
1x fpga
2x dipswitch banks

*/

ROM_START( zipzap )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	// all the roms for this game could do with checking on another board, this one was in pretty bad condition and reads weren't always consistent */
	ROM_LOAD16_BYTE( "ud17.bin", 0x000001, 0x40000, BAD_DUMP CRC(2901fae1) SHA1(0d6ca6d48c5586c05f3c02aee51a95da38b3751f) )
	ROM_LOAD16_BYTE( "ue17.bin", 0x000000, 0x40000, BAD_DUMP CRC(da6c3fc8) SHA1(4bc01bc6f62553f6ac4f7252f7d9bf0d639f6935) )
	// gfx bitmaps
	ROM_LOAD16_BYTE( "937.bin", 0x100000, 0x80000, CRC(61dd653f) SHA1(68b5ae3423363cc64d933836bf6881431dad021a) ) // good, girls
	ROM_LOAD16_BYTE( "941.bin", 0x100001, 0x80000, CRC(320321ed) SHA1(00b52cd34cd86c105ff6dbd0248ff239de31c851) )
	ROM_LOAD16_BYTE( "936.bin", 0x200000, 0x80000, CRC(596543cc) SHA1(10a0eab4ca4a8749f1703ff6fcc80d731d07d087) ) // good, girls
	ROM_LOAD16_BYTE( "940.bin", 0x200001, 0x80000, CRC(0c9dfb53) SHA1(541bd8c79408b7415713b517eacdd565d0ac5cb8) )
	ROM_LOAD16_BYTE( "934.bin", 0x300000, 0x80000, CRC(1e65988a) SHA1(64d6f8cbdb28755515d9bbf52f589ce1176fed58) ) // good, girls
	ROM_LOAD16_BYTE( "939.bin", 0x300001, 0x80000, CRC(8790a6a3) SHA1(94f39e48b75144cab191e2de4284c28d18b8f1c7) )
	ROM_LOAD16_BYTE( "938.bin", 0x400000, 0x80000, CRC(61c06b60) SHA1(b3abae020009a48b99862766e0981e1118159a47) ) // good title background
	ROM_LOAD16_BYTE( "942.bin", 0x400001, 0x80000, CRC(282413b8) SHA1(e2ecaaa3c5b2355eadc016b73d7d658f25e1e0db) ) // (and corrupt gfx on select mode screen)

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "u5.bin", 0x000000, 0x80000, CRC(c274d8b5) SHA1(2c45961aaf8311f027a734df7e33fe085dfdd099) )

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "snd.bin", 0x00000, 0x80000, CRC(bc20423e) SHA1(1f4bd52ec4f9b3b3e6b10ac2b3afaadf76a2c7c9) ) // Missing a sound ROM??
	ROM_RELOAD(          0x80000, 0x80000 )
ROM_END

ROM_START( zipzapa )
	ROM_REGION( 0x500000, "maincpu", 0 ) // 68000 code
	ROM_LOAD16_BYTE( "30.ud17", 0x000001, 0x20000, CRC(769ec252) SHA1(abb0e91036d0643146d81b652defb8f6fb0b5a58) )
	ROM_LOAD16_BYTE( "31.ue17", 0x000000, 0x20000, CRC(e098ef98) SHA1(d08db27e4c6ae9a3f46c40d3c816c2efe23ac92b) )
	// gfx bitmaps
	ROM_LOAD16_BYTE( "37.rd2b", 0x100000, 0x80000, CRC(0b59d718) SHA1(9e7a9bff55396953f7ccfeb1d968e3e814cd3990) )
	ROM_LOAD16_BYTE( "33.rd2a", 0x100001, 0x80000, CRC(df35c8f5) SHA1(69b7aeea9af03611bae12a899ece53068df62eec) )
	ROM_LOAD16_BYTE( "38.rd3b", 0x200000, 0x80000, CRC(575dfc8c) SHA1(4400d5f0f6d63bfc10894c28de3fe8aa83ee49a9) )
	ROM_LOAD16_BYTE( "34.rd3a", 0x200001, 0x80000, CRC(f8bd156b) SHA1(6c95c597898b0dca1b3ccece0e46d67333fdc6ea) )
	ROM_LOAD16_BYTE( "39",      0x300000, 0x80000, CRC(302375c0) SHA1(1618bafab0bec265db9f2414f65fef7e3467e13c) )
	ROM_LOAD16_BYTE( "35",      0x300001, 0x80000, CRC(9b6409a6) SHA1(aa0c38a149bd33e287f2ada5960429352bd63a23) )
	ROM_LOAD16_BYTE( "36.rd1b", 0x400000, 0x80000, CRC(f3256bbb) SHA1(0ca9a02f1824b72fc903bc78d83392297b076094) )
	ROM_LOAD16_BYTE( "32.rd1a", 0x400001, 0x80000, CRC(1dd511a9) SHA1(c8c96ad9e56e1990bdaee5ba8e1942c1de180ef2) )

	ROM_REGION( 0x100000, "kan_spr", 0 ) // sprites
	ROM_LOAD( "40.u5", 0x000000, 0x80000, CRC(c274d8b5) SHA1(2c45961aaf8311f027a734df7e33fe085dfdd099) ) // == u5.bin above

	ROM_REGION( 0x100000, "oki", 0 ) // Samples
	ROM_LOAD( "29.ub6", 0x00000, 0x20000, CRC(2cf3b8ea) SHA1(84a306a1afdc04c9828e5fbda2c050183d9006a3) ) // Correct loading??
	ROM_RELOAD(         0x20000, 0x20000 )
	ROM_RELOAD(         0x40000, 0x20000 )
	ROM_RELOAD(         0x60000, 0x20000 )
	ROM_LOAD( "28.uc6", 0x80000, 0x80000, CRC(e5d026c7) SHA1(7e011d14ec8da493250ec7506909417e64690e73) ) // bad dump or loading / banking issue??
ROM_END


/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

void expro02_state::init_expro02()
{
	uint32_t *src = (uint32_t *)memregion("gfx3" )->base();
	uint32_t *dst = (uint32_t *)memregion("view2" )->base();

	// the VIEW2 tiledata is scrambled
	if (src)
	{
		for (int x = 0; x < 0x80000; x++)
		{
			int offset = x;

			// swap bits around to simplify further processing
			offset = bitswap<24>(offset, 23, 22, 21, 20, 19, 18, 15, 9, 10, 8, 7, 12, 13, 16, 17, 6, 5, 4, 3, 14, 11, 2, 1, 0);

			// invert 8 bits
			offset ^= 0x0528f;

			// addition affecting 9 bits
			offset = (offset & ~0x001ff) | ((offset + 0x00043) & 0x001ff);

			// subtraction affecting 8 bits
			offset = (offset & ~0x1fe00) | ((offset - 0x09600) & 0x1fe00);

			// reverse the initial bitswap
			offset = bitswap<24>(offset, 23, 22, 21, 20, 19, 18, 9, 10, 17, 4, 11, 12, 3, 15, 16, 14, 13, 8, 7, 6, 5, 2, 1, 0);

			dst[x] = src[offset];
		}
	}
}

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, galsnew,   0,        expro02,  galsnew,   expro02_state, init_expro02, ROT90, "Kaneko",                   "Gals Panic (Export, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galsnewu,  galsnew,  expro02,  expro02,   expro02_state, init_expro02, ROT90, "Kaneko",                   "Gals Panic (US, EXPRO-02 PCB)",     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galsnewj,  galsnew,  expro02,  galsnewj,  expro02_state, init_expro02, ROT90, "Kaneko (Taito license)",   "Gals Panic (Japan, EXPRO-02 PCB)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galsnewk,  galsnew,  expro02,  galsnewj,  expro02_state, init_expro02, ROT90, "Kaneko (Inter license)",   "Gals Panic (Korea, EXPRO-02 PCB)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1990, galsnewt,  galsnew,  expro02,  galsnewj,  expro02_state, init_expro02, ROT90, "Kaneko",                   "Gals Panic (Taiwan, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
// the first version of Fantasia clones the EXPRO02 almost exactly, including the encrypted tiles
GAME( 1994, fantasia,  0,        comad,    fantasia,  expro02_state, init_expro02, ROT90, "Comad & New Japan System", "Fantasia (940429 PCB, set 1)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, fantasiab, fantasia, comad,    fantasia,  expro02_state, init_expro02, ROT90, "Comad & New Japan System", "Fantasia (940429 PCB, set 2)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
GAME( 1994, fantasiaa, fantasia, comad,    fantasia,  expro02_state, init_expro02, ROT90, "Comad & New Japan System", "Fantasia (940307 PCB)",        MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE )
// subsequent releases remove the encrypted tile (View2 layer) but leave the unused writes to it in the program code
GAME( 1994, fantasian, fantasia, fantasia, fantasiaa, expro02_state, empty_init,   ROT90, "Comad & New Japan System", "Fantasia (940803 PCB)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1994, supmodel,  0,        supmodel, fantasiaa, expro02_state, empty_init,   ROT90, "Comad & New Japan System", "Super Model", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "C" nudity level

GAME( 1995, newfant,   0,        fantasia, fantasiaa, expro02_state, empty_init,   ROT90, "Comad & New Japan System", "New Fantasia (1995 copyright)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // the only difference between the two is the gfx rom containing the copyright
GAME( 1994, newfanta,  newfant,  fantasia, fantasiaa, expro02_state, empty_init,   ROT90, "Comad & New Japan System", "New Fantasia (1994 copyright)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, hotnight,  newfant,  fantasia, fantasiaa, expro02_state, empty_init,   ROT90, "Bulldog Amusements Inc.",  "Hot Night",                     MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "C" nudity level
GAME( 1995, fantsy95,  newfant,  fantasia, fantasiaa, expro02_state, empty_init,   ROT90, "Hi-max Technology Inc.",   "Fantasy '95",                   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "C" nudity level

// the PCB label (A/B) could be related to the 3 different levels of nudity Comad offered
GAME( 1996, missw96,   0,        fantasia, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss World '96 (Nude) (C-3000A PCB, set 1)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 1996, missw96a,  missw96,  fantasia, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss World '96 (Nude) (C-3000A PCB, set 2)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 1996, missw96b,  missw96,  fantasia, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss World '96 (Nude) (C-3000A PCB, set 3)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 1996, missw96c,  missw96,  fantasia, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss World '96 (Nude) (C-3000B PCB)",        MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "B" nudity level

GAME( 1996, missmw96,  missw96,  fantasia, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss Mister World '96 (Nude)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )

GAME( 1996, smissw,    0,        smissw,   missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Super Miss World", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // 951127 PCB

GAME( 1997, fantsia2,  0,        fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Fantasia II (Explicit)",      MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 1997, fantsia2a, fantsia2, fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Fantasia II (Less Explicit)", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "B" nudity level
GAME( 1998, fantsia2n, fantsia2, fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Fantasia II (1998)",          MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level

GAME( 1997, supmodl2,  0,        supmodl2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Super Model II",              MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "C" nudity level

GAME( 2002, wownfant,  0,        fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "WOW New Fantasia (Explicit)",       MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "B" nudity level
GAME( 2002, wownfanta, wownfant, fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "WOW New Fantasia",                  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "C" nudity level
GAME( 2002, missw02,   0,        fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Comad",                    "Miss World 2002",                   MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 2002, missw02d,  missw02,  fantsia2, missw96,   expro02_state, empty_init,   ROT0,  "Daigom",                   "Miss World 2002 (Daigom license)",  MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE ) // "A" nudity level

GAME( 1996, pgalvip,   0,        galhustl, galhustl,  expro02_state, empty_init,   ROT0,  "ACE International / Afega","Pocket Gals V.I.P (set 1)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // roms were all AFEGA stickered, select screen seems wrong? maybe not a final version.
GAME( 1997, pgalvipa,  pgalvip,  galhustl, galhustl,  expro02_state, empty_init,   ROT0,  "<unknown>",                "Pocket Gals V.I.P (set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1997, galhustl,  pgalvip,  galhustl, galhustl,  expro02_state, empty_init,   ROT0,  "ACE International",        "Gals Hustler",              MACHINE_SUPPORTS_SAVE ) // hack of the above?

GAME( 1995, zipzap,    0,        zipzap,   zipzap,    expro02_state, empty_init,   ROT90, "Barko Corp",               "Zip & Zap (Explicit)",      MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // "A" nudity level
GAME( 1995, zipzapa,   zipzap,   zipzap,   zipzap,    expro02_state, empty_init,   ROT90, "Barko Corp",               "Zip & Zap (Less Explicit)", MACHINE_IMPERFECT_GRAPHICS | MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE ) // "B" nudity level
