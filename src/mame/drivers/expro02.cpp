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
|M                     6116  41464  |         |                           |
|M                                  |         |          PM019U_U93-01.U93|
|A                     6116         |---------| PM206E.U82                |
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
Custom: VU-002, VIEW2, CACL1

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
the one in galspanic.c.  Fantasia even still has the encrypted tile
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
- fantasia and newfant have a service mode but they doesn't work well (text
  is missing or replaced by garbage). This might be just how the games are.
- Is there a background enable register? Or a background clear. fantasia and
  newfant certainly look ugly as they are.

*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/kaneko_hit.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"
#include "includes/galpnipt.h"

class expro02_state : public driver_device
{
public:
	expro02_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_expro02_fg_ind8_pixram(*this, "fg_ind8ram"),
		m_expro02_bg_rgb555_pixram(*this, "bg_rgb555ram"),
		m_view2_0(*this, "view2_0"),
		m_kaneko_spr(*this, "kan_spr"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_palette(*this, "palette"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_screen(*this, "screen")
	{ }

	optional_shared_ptr<UINT16> m_expro02_fg_ind8_pixram;
	optional_shared_ptr<UINT16> m_expro02_bg_rgb555_pixram;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	optional_shared_ptr<UINT16> m_spriteram;

	UINT16 m_vram_0_bank_num;
	UINT16 m_vram_1_bank_num;
	DECLARE_WRITE16_MEMBER(expro02_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(expro02_paletteram_w);
	DECLARE_WRITE16_MEMBER(expro02_vram_0_bank_w);
	DECLARE_WRITE16_MEMBER(expro02_vram_1_bank_w);
	DECLARE_DRIVER_INIT(expro02);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(expro02);
	UINT32 screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_expro02(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_zipzap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(expro02_scanline);

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT16> m_generic_paletteram_16;
	required_device<screen_device> m_screen;

	// comad
	READ16_MEMBER(comad_timer_r);
	READ8_MEMBER(comad_okim6295_r);
	WRITE16_MEMBER(galpanica_6295_bankswitch_w);
};


PALETTE_INIT_MEMBER(expro02_state, expro02)
{
	int i;

	/* first 2048 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette.set_pen_color(2048 + i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

UINT32 expro02_state::screen_update_backgrounds(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
//  kaneko16_fill_bitmap(machine(),bitmap,cliprect);
	int y,x;
	int count;


	count = 0;
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (m_expro02_bg_rgb555_pixram[count] & 0xfffe)>>1;
			dat+=2048;

			// never seen to test
			//if (!(m_expro02_bg_rgb555_pixram[count] & 0x0001))
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
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (m_expro02_fg_ind8_pixram[count]);
			dat &=0x7ff;
			if (!(m_generic_paletteram_16[(dat&0x7ff)] & 0x0001))
				dest[x] = dat;

			count++;
		}
	}



	int i;

	screen.priority().fill(0, cliprect);

	if (m_view2_0)
	{
		m_view2_0->kaneko16_prepare(bitmap, cliprect);

		for (i = 0; i < 8; i++)
		{
			m_view2_0->render_tilemap_chip(screen, bitmap, cliprect, i);
		}
	}
	return 0;
}

UINT32 expro02_state::screen_update_expro02(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_backgrounds(screen, bitmap, cliprect);
	m_kaneko_spr->kaneko16_render_sprites(bitmap,cliprect, screen.priority(), m_spriteram, m_spriteram.bytes());
	return 0;
}

UINT32 expro02_state::screen_update_zipzap(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	screen_update_backgrounds(screen, bitmap, cliprect);
	m_kaneko_spr->bootleg_draw_sprites(bitmap,cliprect, m_spriteram, m_spriteram.bytes());
	return 0;
}


void expro02_state::video_start()
{
}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( expro02 )

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


static INPUT_PORTS_START( galsnewa )
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
	PORT_INCLUDE( galsnewa )

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
	PORT_INCLUDE( galsnewa )

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
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Unknown ) )
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

WRITE16_MEMBER(expro02_state::expro02_6295_bankswitch_w)
{
	if (ACCESSING_BITS_8_15)
	{
		UINT8 *rom = memregion("oki")->base();
		memcpy(&rom[0x30000],&rom[0x40000 + ((data >> 8) & 0x0f) * 0x10000],0x10000);
	}
}

/*************************************
 *
 *  Video handlers
 *
 *************************************/

WRITE16_MEMBER(expro02_state::expro02_paletteram_w)
{
	data = COMBINE_DATA(&m_generic_paletteram_16[offset]);
	m_palette->set_pen_color(offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}




/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/


static ADDRESS_MAP_START( expro02_video_base_map, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("fg_ind8ram")
	AM_RANGE(0x520000, 0x53ffff) AM_RAM AM_SHARE("bg_rgb555ram")
	AM_RANGE(0x580000, 0x583fff) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_vram_r, kaneko_tmap_vram_w )
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE(expro02_paletteram_w) AM_SHARE("paletteram") // palette?
	AM_RANGE(0x680000, 0x68001f) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_regs_r, kaneko_tmap_regs_w)
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_SHARE("spriteram")    // sprites? 0x72f words tested
	AM_RANGE(0x780000, 0x78001f) AM_DEVREADWRITE("kan_spr", kaneko16_sprite_device, kaneko16_sprites_regs_r, kaneko16_sprites_regs_w)
	AM_RANGE(0xd80000, 0xd80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_1_tilebank_w)   /* ??? */
	AM_RANGE(0xe80000, 0xe80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_0_tilebank_w)   /* ??? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( expro02_video_base_map_noview2, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("fg_ind8ram")
	AM_RANGE(0x520000, 0x53ffff) AM_RAM AM_SHARE("bg_rgb555ram")
	AM_RANGE(0x580000, 0x583fff) AM_NOP // games still makes leftover accesses
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE(expro02_paletteram_w) AM_SHARE("paletteram") // palette?
	AM_RANGE(0x680000, 0x68001f) AM_NOP // games still makes leftover accesses
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_SHARE("spriteram")    // sprites? 0x72f words tested
	AM_RANGE(0x780000, 0x78001f) AM_DEVREADWRITE("kan_spr", kaneko16_sprite_device, kaneko16_sprites_regs_r, kaneko16_sprites_regs_w)
	AM_RANGE(0xd80000, 0xd80001) AM_NOP // games still makes leftover accesses
	AM_RANGE(0xe80000, 0xe80001) AM_NOP // games still makes leftover accesses
ADDRESS_MAP_END

static ADDRESS_MAP_START( expro02_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM // main program
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("user2",0) // other data
	AM_RANGE(0x100000, 0x3fffff) AM_ROM AM_REGION("user1",0) // main data
	AM_RANGE(0x400000, 0x400001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x900000, 0x900001) AM_WRITE(expro02_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP    /* ??? */
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xe00000, 0xe00015) AM_DEVREADWRITE("calc1_mcu", kaneko_hit_device, kaneko_hit_r,kaneko_hit_w)
ADDRESS_MAP_END


// bigger ROM space, OKI commands moved, no CALC mcu
static ADDRESS_MAP_START( fantasia_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map)
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x800006, 0x800007) AM_NOP // ? used ?
	AM_RANGE(0x900000, 0x900001) AM_WRITE(expro02_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP    /* ??? */
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xf00000, 0xf00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
ADDRESS_MAP_END



static ADDRESS_MAP_START( comad_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map_noview2)
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
//  AM_RANGE(0x800006, 0x800007)    ??
	AM_RANGE(0x80000a, 0x80000b) AM_READ(comad_timer_r) /* bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x80000c, 0x80000d) AM_READ(comad_timer_r) /* missw96 bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)  /* not sure */
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xf00000, 0xf00001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00) /* fantasia, missw96 */
	AM_RANGE(0xf80000, 0xf80001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00) /* newfant */
ADDRESS_MAP_END

static ADDRESS_MAP_START( fantsia2_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map_noview2)
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
//  AM_RANGE(0x800006, 0x800007)    ??
	AM_RANGE(0x800008, 0x800009) AM_READ(comad_timer_r) /* bits 8-a = timer? palette update code waits for them to be 111 */
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)  /* not sure */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP    /* coin counters, + ? */
	AM_RANGE(0xc80000, 0xc80001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00)
	AM_RANGE(0xf80000, 0xf8ffff) AM_RAM
ADDRESS_MAP_END




static ADDRESS_MAP_START( galhustl_map, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x200000, 0x2fffff) AM_ROM AM_REGION("maincpudata", 0)

	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP // ?
	AM_RANGE(0xd00000, 0xd00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
	AM_RANGE(0xe80000, 0xe8ffff) AM_RAM

	AM_RANGE(0x780000, 0x78001f) AM_NOP // prevent sprites being flipped

	AM_IMPORT_FROM(expro02_video_base_map_noview2)

ADDRESS_MAP_END

static ADDRESS_MAP_START( zipzap_map, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x701000, 0x71ffff) AM_RAM
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)
	AM_RANGE(0xc00000, 0xc00001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00) /* fantasia, missw96 */
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM     // main ram

	AM_RANGE(0x780000, 0x78001f) AM_NOP // prevent sprites being flipped

	AM_IMPORT_FROM(expro02_video_base_map_noview2)
ADDRESS_MAP_END

static ADDRESS_MAP_START( supmodel_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map_noview2)
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("fgvideoram")
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x800006, 0x800007) AM_READ(comad_timer_r)
	AM_RANGE(0x800008, 0x800009) AM_READ(comad_timer_r)
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)  /* not sure */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xd80000, 0xd80001) AM_WRITENOP
	AM_RANGE(0xe00012, 0xe00013) AM_WRITENOP
	AM_RANGE(0xe80000, 0xe80001) AM_WRITENOP
	AM_RANGE(0xf80000, 0xf80001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00) /* fantasia, missw96 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( smissw_map, AS_PROGRAM, 16, expro02_state )
	AM_IMPORT_FROM(expro02_video_base_map_noview2)
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("fgvideoram")
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("SYSTEM")
	AM_RANGE(0x800006, 0x800007) AM_READ(comad_timer_r)
	AM_RANGE(0x80000e, 0x80000f) AM_READ(comad_timer_r)
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galpanica_6295_bankswitch_w)  /* not sure */
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP
	AM_RANGE(0xc00000, 0xc0ffff) AM_RAM
	AM_RANGE(0xd80000, 0xd80001) AM_WRITENOP
	AM_RANGE(0xe00012, 0xe00013) AM_WRITENOP
	AM_RANGE(0xe80000, 0xe80001) AM_WRITENOP
	AM_RANGE(0xf00000, 0xf00001) AM_READ8(comad_okim6295_r, 0xff00) AM_DEVWRITE8("oki", okim6295_device, write, 0xff00) /* fantasia, missw96 */
ADDRESS_MAP_END


/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

TIMER_DEVICE_CALLBACK_MEMBER(expro02_state::expro02_scanline)
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		m_maincpu->set_input_line(3, HOLD_LINE);
	else if(scanline == 0) // vblank-in irq?
		m_maincpu->set_input_line(5, HOLD_LINE);
	else if(scanline == 112) // VDP end task? (controls sprite colors in gameplay)
		m_maincpu->set_input_line(4, HOLD_LINE);
}

void expro02_state::machine_reset()
{
}

/*************************************
 *
 *  Comad specific (kludges?)
 *
 *************************************/

READ16_MEMBER(expro02_state::comad_timer_r)
{
	return (m_screen->vpos() & 0x07) << 8;
}

/* a kludge! */
READ8_MEMBER(expro02_state::comad_okim6295_r)
{
	UINT16 retvalue;
//  retvalue = m_oki->read_status(); // doesn't work, causes lockups when girls change..
	retvalue = machine().rand();
	return retvalue;
}

WRITE16_MEMBER(expro02_state::galpanica_6295_bankswitch_w)
{
	if (ACCESSING_BITS_8_15)
	{
		UINT8 *rom = memregion("oki")->base();

		memcpy(&rom[0x30000],&rom[0x40000 + ((data >> 8) & 0x0f) * 0x10000],0x10000);
	}
}




/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout layout_16x16x4 =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ STEP4(0,1) },
	{ STEP8(8*8*4*0,4),   STEP8(8*8*4*1,4)   },
	{ STEP8(8*8*4*0,8*4), STEP8(8*8*4*2,8*4) },
	16*16*4
};


static GFXDECODE_START( expro02 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x100,      0x40 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x400,      0x40 ) // [0] View2 tiles
GFXDECODE_END

static GFXDECODE_START( expro02_noview2 )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x100,      0x40 ) // [0] Sprites
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( expro02, expro02_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(expro02_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", expro02_state, expro02_scanline, "screen", 0, 1)

	/* CALC01 MCU @ 16Mhz (unknown type, simulated) */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-32-1)
	MCFG_SCREEN_UPDATE_DRIVER(expro02_state, screen_update_expro02)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", expro02)
	MCFG_PALETTE_ADD("palette", 2048 + 32768)
	MCFG_PALETTE_INIT_OWNER(expro02_state, expro02)

	MCFG_DEVICE_ADD("view2_0", KANEKO_TMAP, 0)
	kaneko_view2_tilemap_device::set_gfx_region(*device, 1);
	kaneko_view2_tilemap_device::set_offset(*device, 0x5b, 0x8, 256, 224);
	MCFG_KANEKO_TMAP_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD_VU002_SPRITES
	kaneko16_sprite_device::set_priorities(*device, 8,8,8,8); // above all (not verified)
	kaneko16_sprite_device::set_offsets(*device, 0, -0x40);
	MCFG_KANEKO16_SPRITE_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("calc1_mcu", KANEKO_HIT, 0)
	kaneko_hit_device::set_type(*device, 0);



	/* arm watchdog */
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))  /* a guess, and certainly wrong */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 12000000/6, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( comad, expro02 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fantasia_map)

	MCFG_DEVICE_REMOVE("calc1_mcu")

	MCFG_DEVICE_MODIFY("view2_0")
	// these values might not be correct, behavior differs from original boards
	kaneko_view2_tilemap_device::set_invert_flip(*device, 1);
	kaneko_view2_tilemap_device::set_offset(*device, -256, -216, 256, 224);

	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(0))  /* a guess, and certainly wrong */
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( comad_noview2, comad )
	MCFG_DEVICE_REMOVE("view2_0")

	MCFG_GFXDECODE_MODIFY("gfxdecode", expro02_noview2)
MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( fantasia, comad_noview2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(10000000)
	MCFG_CPU_PROGRAM_MAP(comad_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( supmodel, comad_noview2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(supmodel_map)
	MCFG_OKIM6295_REPLACE("oki", 1584000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( smissw, comad_noview2 ) // 951127 PCB, 12 & 16 clocks
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(smissw_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( fantsia2, comad_noview2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fantsia2_map)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( galhustl, comad_noview2 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(galhustl_map)
	MCFG_OKIM6295_REPLACE("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SCREEN_MODIFY("screen")
	MCFG_SCREEN_UPDATE_DRIVER(expro02_state, screen_update_zipzap)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( zipzap, comad_noview2 )
	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(zipzap_map)
	MCFG_OKIM6295_REPLACE("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_SCREEN_MODIFY("screen") // doesn't work with original kaneko_spr implementation
	MCFG_SCREEN_UPDATE_DRIVER(expro02_state, screen_update_zipzap)
MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( galsnew ) /* EXPRO-02 PCB */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "pm110u_u87-01.u87", 0x000000, 0x20000, CRC(b793a57d) SHA1(12d57b2b4add532f0d0453c25b30d34b3449d717) ) /* US region */
	ROM_LOAD16_BYTE( "pm109u_u88-01.u88", 0x000001, 0x20000, CRC(35b936f8) SHA1(d272067f10542d511a777802cafa4d72b93fa5e8) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) /* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )  /* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )   /* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83",        0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82",        0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94",        0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm019u_u93-01.u93", 0x180000, 0x010000, CRC(3cb79005) SHA1(05a0b993b9071467265067c3762644f46343d8de) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* sprites - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )


	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.u46", 0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_RELOAD(             0x40000, 0x80000 )
	ROM_LOAD( "pm007e.u47", 0xc0000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galsnewa ) /* EXPRO-02 PCB */
	ROM_REGION( 0x40000, "maincpu", 0 )
	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110e.u87-01",  0x000000, 0x20000, CRC(34e1ee0d) SHA1(567df65b04667a6d35725c4a131fb174acb3ad0a) ) /* Export region */
	ROM_LOAD16_BYTE( "pm109e.u88-01",  0x000001, 0x20000, CRC(c694255a) SHA1(16faf5ea5ff69a0e7a981021ea5fc09a0aefd7cf) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) /* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )  /* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )   /* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	/* U93 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* tiles - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008e.u46", 0x00000, 0x80000, CRC(d9379ba8) SHA1(5ae7c743319b1a12f2b101a9f0f8fe0728ed1476) )
	ROM_RELOAD(             0x40000, 0x80000 )
	ROM_LOAD( "pm007e.u47", 0xc0000, 0x80000, CRC(c7ed7950) SHA1(133258b058d3c562208d0d00b9fac71202647c32) )
ROM_END

ROM_START( galsnewj ) /* EXPRO-02 PCB */
	ROM_REGION( 0x40000, "maincpu", 0 )
	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110j.u87", 0x000000, 0x20000, CRC(220b6df5) SHA1(d653b67bc66ca341bc660c2bb39b05dcf186fcb7) ) /* Japan region */
	ROM_LOAD16_BYTE( "pm109j.u88", 0x000001, 0x20000, CRC(17721444) SHA1(9d97fe1ddac99105798fc22375a0b89ab316459a) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) /* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", ROMREGION_ERASEFF )  /* contains real (non-cartoon) women, used after each 3rd round */
	/* U84 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )   /* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	/* U93 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* tiles - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008j.u46", 0x00000, 0x80000, CRC(f394670e) SHA1(171f8dc519a13f352e6440aaadebe490c82361f0) )
	ROM_RELOAD(             0x40000, 0x80000 )
	ROM_LOAD( "pm007j.u47", 0xc0000, 0x80000, CRC(06780287) SHA1(8b9b57f6604b86d6dff42e5e51cd59a7111e1e79) )
ROM_END

ROM_START( galsnewk ) /* EXPRO-02 PCB, Korean title is "Ddang Dda Meok Gi" */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "pm110k.u87", 0x000000, 0x20000, CRC(babe6a71) SHA1(91a5fc5e93affd01f8c6d5a4851233edcf8746f0) )
	ROM_LOAD16_BYTE( "pm109k.u88", 0x000001, 0x20000, CRC(e486d98f) SHA1(9923f1dc69bd2746c06da6a5e518211391052259) )

	ROM_REGION16_BE( 0x300000, "user1", 0 ) /* 68000 data */
	ROM_LOAD16_BYTE( "pm004k.u86", 0x000001, 0x80000, CRC(9a14c8a3) SHA1(c3992eceb8d7d65f781b31dc77bebc73cf9303b6) )
	ROM_LOAD16_BYTE( "pm005k.u85", 0x000000, 0x80000, CRC(33b5d0e3) SHA1(88eef6aff8054b07173da3bb1383fb47a1f7980c) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )  /* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017k.u84", 0x00000, 0x80000, CRC(0c656fb5) SHA1(4610800a460c9f50f7a2ee7b2984bf8e79b62124) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )   /* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83",        0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82",        0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94",        0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm19k.u93",         0x180000, 0x010000, CRC(c17d2989) SHA1(895f44a58dcf0065d42125d439dcc10f41563a94) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* sprites - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )


	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008k.u46", 0x00000, 0x80000, CRC(7498483f) SHA1(d1f7461c8d1469704cc34460d7283f0a914afc29) )
	ROM_RELOAD(             0x40000, 0x80000 )
	ROM_LOAD( "pm007k.u47", 0xc0000, 0x80000, CRC(a8dc1fd5) SHA1(c324f7eab7302e4a71d505c915ab2ad591b8ff33) )
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
 Sound: OKI M6295 (rebaged as AD-65)
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
 Sound: OKI M6295 (rebaged as AD-65)
 Video: TI TPC1020AFN-084C x 2
   OSC: 16MHz & 12MHz
   DSW: 8-way switch x 2
Memory: KM62256BLP-8, HY6264ALP-10
   VR1: Sound adjust pot

* denotes unpopulated positions

*/

ROM_START( fantasia ) /* PCB silkscreened COMAD INDUSTRY CO.,LTD940429 MADE IN KOREA */
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "16.pro2",  0x000000, 0x80000, CRC(e27c6c57) SHA1(420b66928c46e76fa2496f221691dd6c34542287) ) /* PCB location is silkscreened under EPROM sockets */
	ROM_LOAD16_BYTE( "13.pro1",  0x000001, 0x80000, CRC(68d27413) SHA1(84cb7d6523325496469d621f6f4da1b719162147) )
	ROM_LOAD16_BYTE( "9.fg_ind87",    0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) )
	ROM_LOAD16_BYTE( "5.fg_ind83",    0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "8.fg_ind86",    0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "4.fg_ind82",    0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "7.fg_ind85",    0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "3.fg_ind81",    0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "10.imag2", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "6.imag1",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "17.scr3",  0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) )
	/* SCR1 and SCR2 are unpopulated */

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "2.music1", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) )
	ROM_RELOAD(           0x40000, 0x80000 )
	ROM_LOAD( "1.music2", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* tiles - encrypted */
	ROM_LOAD16_BYTE( "15.obj3", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) )
	ROM_LOAD16_BYTE( "12.obj1", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "14.obj4", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "11.obj2", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END


ROM_START( fantasiaa ) /* PCB silkscreened COMAD INDUSTRY CO.,LTD 940307 MADE IN KOREA */
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "prog2_16.ue17",  0x000000, 0x80000, CRC(0b41ad10) SHA1(386b59f6892cdd2f90df86dc77172919079f0200) )
	ROM_LOAD16_BYTE( "prog1_13.ud17",  0x000001, 0x80000, CRC(a3748726) SHA1(8dc922e01edb777eb853f40556315a34e1aced62) )
	ROM_LOAD16_BYTE( "i-scr6_9.ue16b", 0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) ) /* ROMS 3 through 10 contain the same data */
	ROM_LOAD16_BYTE( "i-scr5_5.ue16a", 0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) ) /* just in different PCB locations */
	ROM_LOAD16_BYTE( "i-scr4_8.ue15b", 0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "i-scr3_4.ue15a", 0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "i-scr2_7.ue14b", 0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "i-scr1_3.ue14a", 0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "imag2_10.ue20b", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "imag1_6.ue20a",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "obj1_17.u5",  0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) ) /* same data, different PCB location */
	/* U4 OBJ2 18 and U3 OBJ3 19 are unpopulated */

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1_1.ub6", 0x00000, 0x80000, CRC(af0be817) SHA1(5c8897dcd9957add19ff9553c01ce03fec68b354) ) /* This sound sample is different, Earlier ver or BAD??? */
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2_2.uc6", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) ) /* same data, different PCB location */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* tiles - encrypted */
	ROM_LOAD16_BYTE( "g-scr2_15.ul16b", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) ) /* same data, different PCB location */
	ROM_LOAD16_BYTE( "g-scr1_12.ul16a", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "g-scr4_14.ul19b", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "g-scr3_11.ul19a", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END

ROM_START( fantasiab )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "fantasia_16",  0x000000, 0x80000, CRC(c5d93077) SHA1(da615ea0704e77e888dbda664fc9f9fd873edbfa) )
	ROM_LOAD16_BYTE( "fantasia_13",  0x000001, 0x80000, CRC(d88529bd) SHA1(06eb928f4aefe101140140ba7a3ce416215f9e39) )
	ROM_LOAD16_BYTE( "9.fg_ind87",    0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) )
	ROM_LOAD16_BYTE( "5.fg_ind83",    0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "8.fg_ind86",    0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "4.fg_ind82",    0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "7.fg_ind85",    0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "3.fg_ind81",    0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "10.imag2", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "6.imag1",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "17.scr3",  0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) )
	/* SCR1 and SCR2 are unpopulated */

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "2.music1", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) )
	ROM_RELOAD(           0x40000, 0x80000 )
	ROM_LOAD( "1.music2", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )   /* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )   /* tiles - encrypted */
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



// fantasy 95 - derived from new fantasia?
ROM_START( fantsy95 )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
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

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "obj1.13",  0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // sldh

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(3117e2ef) SHA1(6581a7104556d44f814c537bbd74998922927034) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2.2", 0xc0000, 0x80000, CRC(0c1109f9) SHA1(0e4ea534a32b1649e2e9bb8af7254b917ec03a90) )
ROM_END

ROM_START( fantasian ) /* PCB silkscreened COMAD INDUSTRY CO.,LTD 940803 MADE IN KOREA */
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "prog2_12.ue17",   0x000000, 0x80000, CRC(8bb70be1) SHA1(d8854defcffa4cc1a6f4087acdaa05cc80444089) )
	ROM_LOAD16_BYTE( "prog1_7.ud17",    0x000001, 0x80000, CRC(d1616a3e) SHA1(49a647c16d3bdb895ca14871c1f5cb5abcf59c9a) )
	ROM_LOAD16_BYTE( "i-scr2_10.ue16b", 0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) ) /* data roms same as expro02.c sets, but different positions */
	ROM_LOAD16_BYTE( "i-scr1_5.ue16a",  0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "i-scr4_9.ue15b",  0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "i-scr3_4.ue15a",  0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "i-scr6_8.ue14b",  0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "i-scr5_3.ue14a",  0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "i-scr8_11.ue20b", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "i-scr7_6.ue20a",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "obj1_13.u5",  0x00000, 0x80000, CRC(f99751f5) SHA1(10f0a2e369abc36a6df2f0c9879ffb7071ee214b) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1_1.ub6", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) ) /* sample roms same as expro02.c sets, but different positions */
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2_2.uc6", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )
ROM_END

ROM_START( newfant )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
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

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "nf95obj1.13",  0x00000, 0x80000, CRC(e6d1bc71) SHA1(df0b6c1742c01991196659bab2691230323e7b8d) ) // sldh

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "musc1.01", 0x00000, 0x80000, CRC(10347fce) SHA1(f5fbe8ef363fe18b7104be5d2fa92943d1a5d7a2) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "musc2.02", 0xc0000, 0x80000, CRC(b9646a8c) SHA1(e9432261ac86e4251a2c97301c6d014c05110a9c) )
ROM_END

ROM_START( newfanta )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
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

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "obj1.13",  0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // sldh

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "musc1.01", 0x00000, 0x80000, CRC(10347fce) SHA1(f5fbe8ef363fe18b7104be5d2fa92943d1a5d7a2) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "musc2.02", 0xc0000, 0x80000, CRC(b9646a8c) SHA1(e9432261ac86e4251a2c97301c6d014c05110a9c) )
ROM_END

ROM_START( missw96 )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mw96_10.bin",  0x000000, 0x80000, CRC(b1309bb1) SHA1(3cc7a903cb007d8fc0f836a33780c1c9231d1629) )
	ROM_LOAD16_BYTE( "mw96_06.bin",  0x000001, 0x80000, CRC(a5892bb3) SHA1(99130eb0af307fe66c9668414475e003f9c7d969) )
	ROM_LOAD16_BYTE( "mw96_09.bin",  0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",  0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",  0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",  0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",  0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",  0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "mw96_11.bin",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96a )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mw96n2_10.prog2", 0x000000, 0x80000, CRC(563ce811) SHA1(4013b303dc7fdfcd2b5b91f12a950eb71b27714a) )
	ROM_LOAD16_BYTE( "mw96n2_6.prog1",  0x000001, 0x80000, CRC(98e91a3b) SHA1(a135458e0373b528498408ac3288a01a666f3522) )
	ROM_LOAD16_BYTE( "mw96_09.bin",  0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",  0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",  0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",  0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",  0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",  0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "mw96_11.bin",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96b )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mw96n3_10.prog2", 0x000000, 0x80000, CRC(67bde86b) SHA1(7457a4c130a9ab1c75645e2a662a87af3fee8bba) )
	ROM_LOAD16_BYTE( "mw96n3_6.prog1",  0x000001, 0x80000, CRC(de99cc48) SHA1(ffa2597083c412fb943724048d8d5cc7bd9be11c) )
	ROM_LOAD16_BYTE( "mw96_09.bin",  0x100000, 0x80000, CRC(7032dfdf) SHA1(53728b60d0c772f6d936be47e21b069d0a75a2b4) )
	ROM_LOAD16_BYTE( "mw96_05.bin",  0x100001, 0x80000, CRC(91de5ab5) SHA1(d1153fa4745830d0fdd5bb311c38bf098ea29deb) )
	ROM_LOAD16_BYTE( "mw96_08.bin",  0x200000, 0x80000, CRC(b8e66fb5) SHA1(8abc6f8d85e0ad6acbf518e11fd81debc5a90957) )
	ROM_LOAD16_BYTE( "mw96_04.bin",  0x200001, 0x80000, CRC(e77a04f8) SHA1(e0043ec1d1bd5415c05ae93c9d785fc70174cb54) )
	ROM_LOAD16_BYTE( "mw96_07.bin",  0x300000, 0x80000, CRC(26112ed3) SHA1(f49f92a4d1bcea322b171702591315950fbd70c6) )
	ROM_LOAD16_BYTE( "mw96_03.bin",  0x300001, 0x80000, CRC(e9374a46) SHA1(eabfcc7cb9c9a2f932abc8103c3abfa8360dcbb5) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "mw96_11.bin",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( missw96c )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "10_PROG2.UE17", 0x000000, 0x80000, CRC(36a7beb6) SHA1(11f6aef506a4e357442207fef760401757deaaeb) )
	ROM_LOAD16_BYTE( "6_PROG1.UD17",  0x000001, 0x80000, CRC(e70b562f) SHA1(4affd40ab7f962824d1c7be22ea6819cf06d6347) )
	ROM_LOAD16_BYTE( "9_IM1-B.UE16B",  0x100000, 0x80000, CRC(eedc24f8) SHA1(cef822c1e3f09c484d03964f02d761139aac9d76) )
	ROM_LOAD16_BYTE( "5_IM1-A.UE16A",  0x100001, 0x80000, CRC(bb0eb7d7) SHA1(6952d153afa90924754c11872497ec83ae650220) )
	ROM_LOAD16_BYTE( "8_IM2-B.UE15B",  0x200000, 0x80000, CRC(68dd67b2) SHA1(322f3eb84277568356ae0a09f71337bd525f379a) )
	ROM_LOAD16_BYTE( "4_IM2-A.UE15A",  0x200001, 0x80000, CRC(2b39ec56) SHA1(8ea1483050287c68063e54c4de27bd82ad942c53) )
	ROM_LOAD16_BYTE( "7_IM3_B.UE14B",  0x300000, 0x80000, CRC(7fd5ca2c) SHA1(7733bd0529953bdae718bf28053d173e5ec3ca92) )
	ROM_LOAD16_BYTE( "3_IM3-A.UE14A",  0x300001, 0x80000, CRC(4ba5dab7) SHA1(81d7b6fde6d9405793f60ee7d15a15a511396332) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "20_OBJ1.U5",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "1_MUSIC1.UB6",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "2_MUSIC2.UC6",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END


ROM_START( missmw96 )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "mmw96_10.bin",  0x000000, 0x80000, CRC(45ed1cd9) SHA1(a75b1b6cddde065e6d7f7355a746819c8268c24f) )
	ROM_LOAD16_BYTE( "mmw96_06.bin",  0x000001, 0x80000, CRC(52ec9e5d) SHA1(20b7cc923e9d55e391b09d96248837bb8f28a176) )
	ROM_LOAD16_BYTE( "mmw96_09.bin",  0x100000, 0x80000, CRC(6c458b05) SHA1(249490c45cdecd6496338286a9ab6a6137cefcd0) )
	ROM_LOAD16_BYTE( "mmw96_05.bin",  0x100001, 0x80000, CRC(48159555) SHA1(a7c736f9e41915d06b7242e427282c421c4a8283) )
	ROM_LOAD16_BYTE( "mmw96_08.bin",  0x200000, 0x80000, CRC(1dc72b07) SHA1(fdbdf8298fe98d74ed2a76abf60f60af1c27a65d) )
	ROM_LOAD16_BYTE( "mmw96_04.bin",  0x200001, 0x80000, CRC(fc3e18fa) SHA1(b3ad254aab982dc75a10c2cf2b3815c2fdbba914) )
	ROM_LOAD16_BYTE( "mmw96_07.bin",  0x300000, 0x80000, CRC(001572bf) SHA1(cdf59c624baaeaea70985ee6f2f2fed08a8dfa61) )
	ROM_LOAD16_BYTE( "mmw96_03.bin",  0x300001, 0x80000, CRC(22204025) SHA1(442e7f754c65c598983d6f897a60870d7759c823) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "mmw96_11.bin",  0x00000, 0x80000, CRC(7d491f8c) SHA1(63f580bd65579cac70b90eaa0e7f2413ef1597b8) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "mw96_01.bin",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "mw96_02.bin",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END

ROM_START( smissw )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "10_PROG2.UE17",  0x000000, 0x80000, CRC(e99e520f) SHA1(edd06a3b0f8d30a4020e6ea452abb0afd79d426a) )
	ROM_LOAD16_BYTE( "6_PROG1.UD17",   0x000001, 0x80000, CRC(22831657) SHA1(eeabcdef543048ccceabc4c3b4b288aec959a14f) )
	ROM_LOAD16_BYTE( "9_IM1-B.UE16B",  0x100000, 0x80000, CRC(fff1eee4) SHA1(1b88d45b5cc0b5a03296d4dc950e570fa4dc19c2) )
	ROM_LOAD16_BYTE( "5_IM1-A.UE16A",  0x100001, 0x80000, CRC(2134a72d) SHA1(f907ec8a1d6e5755a821e69564074ff05e426bb1) )
	ROM_LOAD16_BYTE( "8_IM2-B.UE15B",  0x200000, 0x80000, CRC(cf44b638) SHA1(0fe5bdb62492c31c3efffa6d85f5d6a3b4ddb2e0) )
	ROM_LOAD16_BYTE( "4_IM2-A.UE15A",  0x200001, 0x80000, CRC(d22b270f) SHA1(21bd2ced1b5fb3c08687addaa890ee621a56fff0) )
	ROM_LOAD16_BYTE( "7_IM3-B.UE14B",  0x300000, 0x80000, CRC(12a9441d) SHA1(d9cd51e0c3ffac5fc561e0927c419bce0157337e) )
	ROM_LOAD16_BYTE( "3_IM3-A.UE14A",  0x300001, 0x80000, CRC(8c656fc9) SHA1(c3fe5de7cd6cd520bbd205ec62ac0dda51f71eeb) )

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "15_OBJ11.U5",  0x00000, 0x80000, CRC(3983152f) SHA1(6308e936ba54e88b34253f1d4fbd44725e9d88ae) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "1_MUSIC1.UB6",  0x00000, 0x80000, CRC(e78a659e) SHA1(d209184c70e0d7e6d17034c6f536535cda782d42) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "2_MUSIC2.UC6",  0xc0000, 0x80000, CRC(60fa0c00) SHA1(391aa31e61663cc083a8a2320ba48a9859f3fd4e) )
ROM_END


ROM_START( fantsia2 )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "prog2.g17",    0x000000, 0x80000, CRC(57c59972) SHA1(4b1da928b537cf340a67026d07bc3dfc078b0d0f) )
	ROM_LOAD16_BYTE( "prog1.f17",    0x000001, 0x80000, CRC(bf2d9a26) SHA1(92f0c1bd32f1e5e0ede3ba847242a212dfae4986) )
	ROM_LOAD16_BYTE( "scr2.g16",     0x100000, 0x80000, CRC(887b1bc5) SHA1(b6fcdc8a56ea25758f363224d256e9b6c8e30244) )
	ROM_LOAD16_BYTE( "scr1.f16",     0x100001, 0x80000, CRC(cbba3182) SHA1(a484819940fa1ef18ce679465c31075798748bac) )
	ROM_LOAD16_BYTE( "scr4.g15",     0x200000, 0x80000, CRC(ce97e411) SHA1(be0ed41362db03f384229c708f2ba4146e5cb501) )
	ROM_LOAD16_BYTE( "scr3.f15",     0x200001, 0x80000, CRC(480cc2e8) SHA1(38fe57ba1e34537f8be65fcc023ccd43369a5d94) )
	ROM_LOAD16_BYTE( "scr6.g14",     0x300000, 0x80000, CRC(b29d49de) SHA1(854b76755acf58fb8a4648a0ce72ea6bdf26c555) )
	ROM_LOAD16_BYTE( "scr5.f14",     0x300001, 0x80000, CRC(d5f88b83) SHA1(518a1f6732149f2851bbedca61f7313c39beb91b) )
	ROM_LOAD16_BYTE( "scr8.g20",     0x400000, 0x80000, CRC(694ae2b3) SHA1(82b7a565290fce07c8393af4718fd1e6136928e9) )
	ROM_LOAD16_BYTE( "scr7.f20",     0x400001, 0x80000, CRC(6068712c) SHA1(80a136d76dca566772e34d832ac11b8c7d6ce9ab) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "obj1.1i",      0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i",      0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music2.1b",    0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music1.1a",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( fantsia2a )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "fnt2-22.bin",    0x000000, 0x80000, CRC(a3a92c4b) SHA1(6affdcb57e1e0a77c7cc33135dafe86843e9e3d8) )
	ROM_LOAD16_BYTE( "fnt2-17.bin",    0x000001, 0x80000, CRC(d0ce4493) SHA1(9cec088e6630555b6d584df23236c279909820cf) )
	ROM_LOAD16_BYTE( "fnt2-21.bin",    0x100000, 0x80000, CRC(e989c2e7) SHA1(c9eea2a89843cdd9db4a4a0539d0315c125e3e02) )
	ROM_LOAD16_BYTE( "fnt2-16.bin",    0x100001, 0x80000, CRC(8c06d372) SHA1(14fe2c8450f0f2e11e204dd524bfe32a72ddc144) )
	ROM_LOAD16_BYTE( "fnt2-20.bin",    0x200000, 0x80000, CRC(6e9f1e65) SHA1(b6f1eb1a52de18ed5b17de3ef365e5c041d15314) )
	ROM_LOAD16_BYTE( "fnt2-15.bin",    0x200001, 0x80000, CRC(85cbeb2b) SHA1(a213b461019ddb3b319b9815a76c6fb2ecfbe937) )
	ROM_LOAD16_BYTE( "fnt2-19.bin",    0x300000, 0x80000, CRC(7953226a) SHA1(955c779eae496688be2ed416d879d6e83c888368) )
	ROM_LOAD16_BYTE( "fnt2-14.bin",    0x300001, 0x80000, CRC(10d8ccff) SHA1(bf4c49d85556edf49289631ee6178d3fb7dea2cc) )
	ROM_LOAD16_BYTE( "fnt2-18.bin",    0x400000, 0x80000, CRC(4cdaeda3) SHA1(f5b478e49b59496865982409517654f48296565d) )
	ROM_LOAD16_BYTE( "fnt2-13.bin",    0x400001, 0x80000, CRC(68c7f042) SHA1(ed3c864f3d91377fec78f19897ba0b0d2bcf0d2b) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "obj1.1i",      0x00000, 0x80000, CRC(52e6872a) SHA1(7e5274b9a415ee0e536cd3b87f73d3eae9644669) )
	ROM_LOAD( "obj2.2i",      0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music2.1b",    0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music1.1a",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

/* sole change seems to be copyright date, PCB has chip references instead of grid references.  Not correcting all labels in other sets in case these are legitimate labels */
ROM_START( fantsia2n )
		ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
		ROM_LOAD16_BYTE( "prog2.g17",    0x000000, 0x80000, CRC(57c59972) SHA1(4b1da928b537cf340a67026d07bc3dfc078b0d0f) )
		ROM_LOAD16_BYTE( "prog1.f17",    0x000001, 0x80000, CRC(bf2d9a26) SHA1(92f0c1bd32f1e5e0ede3ba847242a212dfae4986) )
		ROM_LOAD16_BYTE( "scr2.g16",     0x100000, 0x80000, CRC(887b1bc5) SHA1(b6fcdc8a56ea25758f363224d256e9b6c8e30244) )
		ROM_LOAD16_BYTE( "scr1.f16",     0x100001, 0x80000, CRC(cbba3182) SHA1(a484819940fa1ef18ce679465c31075798748bac) )
		ROM_LOAD16_BYTE( "scr4.g15",     0x200000, 0x80000, CRC(ce97e411) SHA1(be0ed41362db03f384229c708f2ba4146e5cb501) )
		ROM_LOAD16_BYTE( "scr3.f15",     0x200001, 0x80000, CRC(480cc2e8) SHA1(38fe57ba1e34537f8be65fcc023ccd43369a5d94) )
		ROM_LOAD16_BYTE( "scr6.g14",     0x300000, 0x80000, CRC(b29d49de) SHA1(854b76755acf58fb8a4648a0ce72ea6bdf26c555) )
		ROM_LOAD16_BYTE( "scr5.f14",     0x300001, 0x80000, CRC(d5f88b83) SHA1(518a1f6732149f2851bbedca61f7313c39beb91b) )
		ROM_LOAD16_BYTE( "scr8.g20",     0x400000, 0x80000, CRC(694ae2b3) SHA1(82b7a565290fce07c8393af4718fd1e6136928e9) )
		ROM_LOAD16_BYTE( "scr7.f20",     0x400001, 0x80000, CRC(6068712c) SHA1(80a136d76dca566772e34d832ac11b8c7d6ce9ab) )

		ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
		ROM_LOAD( "23_OBJ1.U5",      0x00000, 0x80000, CRC(b45c9234) SHA1(b5eeec91b9c6952b338130458405997e1a51bf2f) )
		ROM_LOAD( "obj2.2i",      0x80000, 0x80000, CRC(ea6e3861) SHA1(463b40f5441231a0451571a0b8afe1ed0fd4b164) )

		ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
		/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
		ROM_LOAD( "music2.1b",    0x00000, 0x80000, CRC(23cc4f9c) SHA1(06b5342c25de966ce590917c571e5b19af1fef7d) )
		ROM_RELOAD(               0x40000, 0x80000 )
		ROM_LOAD( "music1.1a",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( wownfant)
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "ep-4001 42750001 u81.bin",    0x000000, 0x80000, CRC(9942d200) SHA1(d2f69c0949881ef4aef202b564eac069c030a497) )
	ROM_LOAD16_BYTE( "ep-4001 42750001 u80.bin",    0x000001, 0x80000, CRC(17359eeb) SHA1(90bb9da6bdf56fa9eb0ad03691750518a2a3f879) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 1.bin",    0x100000, 0x200000, CRC(c318e841) SHA1(ba7af736d3b0accca474b0de1c8299eb3c449ef9) )
	ROM_LOAD16_WORD_SWAP( "ep-061 43750002 - 2.bin",    0x300000, 0x200000, CRC(8871dc3a) SHA1(8e028f1430474df19bb9a912ee9e407fe4582558) )

	ROM_REGION( 0x100000, "gfx1", 0 )   /* sprites */
	ROM_LOAD( "ep-4001 42750001 u113.bin",      0x00000, 0x80000, CRC(3e77ca1f) SHA1(f946e65a29bc02b89c02b2a869578d38cfe7e2d0) )
	ROM_LOAD( "ep-4001 42750001 u112.bin",      0x80000, 0x80000, CRC(51f4b604) SHA1(52e8ce0a2c1b9b00f04e0c775789bc550bad8ae0) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "ep-4001 42750001 u4.bin",    0x00000, 0x80000, CRC(06dc889e) SHA1(726561ff01bbde43669293a6ff7ee22b048b4118) ) // almost the same as fantasia2, just some changes to the sample references in the header
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "ep-4001 42750001 u1.bin",    0xc0000, 0x80000, CRC(864167c2) SHA1(c454b26b6dea993e6bd64546f92beef05e46d7d7) )
ROM_END

ROM_START( galhustl ) // An original PCB has been seen with genuine AFEGA labels
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "ue17.3", 0x00000, 0x80000, CRC(b2583dbb) SHA1(536f4aa2246ec816c4f270f9d42acc090718ee8b) ) // Also found as AFEGA 3
	ROM_LOAD16_BYTE( "ud17.4", 0x00001, 0x80000, CRC(470a3668) SHA1(ad86e96ab8f1f5da23fb1feaabfb9c757965418e) ) // Also found as AFEGA 4

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) /* 68000 Data */

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "galhstl1.ub6", 0x00000, 0x80000,  CRC(23848790) SHA1(2e77fbe04f46e258daecb4c5917e383c7c06a306) ) // Also found as AFEGA 1
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "galhstl2.uc6", 0xc0000, 0x80000,  CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) ) // Also found as AFEGA 2

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "galhstl5.u5", 0x00000, 0x80000, CRC(44a18f15) SHA1(1217cf7fbbb442358b15016099efeface5dcbd22) ) // Also found as AFEGA 5
ROM_END

ROM_START( pgalvip ) // this set has extra data roms for the gfx
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "afega_15.ue17", 0x00000, 0x20000, CRC(050060ca) SHA1(1e2a1d6aaf5038269d192baf3520f4af7a299325) )
	ROM_LOAD16_BYTE( "afega_16.ud17", 0x00001, 0x20000, CRC(d32e4052) SHA1(632d9affee92a526c0e9399230ecf485922c6df4) )

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) /* 68000 Data */
	ROM_LOAD16_BYTE( "afega_13.rob1", 0x00000, 0x80000, CRC(ac51ef72) SHA1(01acb29ff474c52fcb323cdb14e0d6f804c93255) )
	ROM_LOAD16_BYTE( "afega_14.roa1", 0x00001, 0x80000, CRC(0877c00f) SHA1(91c325d6c21045f08abca86a9c4d46023363dd2e) )

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "afega_12.ub6", 0x00000, 0x20000,  CRC(d32a6c0c) SHA1(6f16043ed1e174b42de83462e2ea7a601bac6678) )
	ROM_RELOAD(               0x40000, 0x20000 )
	ROM_LOAD( "afega_11.uc6", 0xc0000, 0x80000,  CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "afega_17.u5", 0x00000, 0x80000, CRC(a8a50745) SHA1(e51963947c7a7556b8531d172b9d7bf9f321b21b) )
ROM_END

ROM_START( pgalvipa ) // this set is more like Gals Hustler
	ROM_REGION( 0x100000, "maincpu", 0 ) /* 68000 Code */
	ROM_LOAD16_BYTE( "pgalvip_3.ue17", 0x00000, 0x80000, CRC(a48e8255) SHA1(7e8f1747420ff0d599d340915712827ca2eb3092) )
	ROM_LOAD16_BYTE( "pgalvip_4.ud17", 0x00001, 0x80000, CRC(829a2085) SHA1(3ff5f2bb730572202cd427abd7f91dd886537ab6) )

	ROM_REGION16_BE( 0x100000, "maincpudata", ROMREGION_ERASEFF ) /* 68000 Data */

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pgalvip_1.ub6", 0x00000, 0x20000,  CRC(d32a6c0c) SHA1(6f16043ed1e174b42de83462e2ea7a601bac6678) )
	ROM_RELOAD(               0x40000, 0x20000 )
	ROM_LOAD( "pgalvip_2.uc6", 0xc0000, 0x80000,  CRC(2168e54a) SHA1(87534334b16d3ddc3daefcb1b8086aff44157ccf) )

	ROM_REGION( 0x100000, "gfx1", 0 )
	ROM_LOAD( "pgalvip_5.u5", 0x00000, 0x80000, CRC(2d6e5a90) SHA1(b5487e5764d83dfecd982a8614d213c9075fbee4) )
ROM_END


ROM_START( supmodel )
	ROM_REGION( 0x500000, "maincpu", 0 )    /* 68000 code */
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

	ROM_REGION( 0x80000, "gfx1", 0 )    /* sprites */
	ROM_LOAD( "obj1.13",  0x00000, 0x80000, CRC(832cd451) SHA1(29dfab1d4b7a15f3fe9fbedef41d405a40235a77) ) // sldh

	ROM_REGION( 0x140000, "oki", 0 )    /* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1.1", 0x00000, 0x80000, CRC(2b1f6655) SHA1(e7b52cf4bd16590c598c375d5a97b724bc9ef631) )
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2.2", 0xc0000, 0x80000, CRC(cccae65a) SHA1(5e4e2e51884eaf191f103aa189ff33371fc91d6d) )
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
	ROM_REGION( 0x500000, "maincpu", 0 ) /* 68000 Code */
	/* all the roms for this game could do with checking on another board, this one was in pretty bad condition
	   and reads weren't always consistent */
	ROM_LOAD16_BYTE( "ud17.bin", 0x000001, 0x40000, BAD_DUMP CRC(2901fae1) SHA1(0d6ca6d48c5586c05f3c02aee51a95da38b3751f) )
	ROM_LOAD16_BYTE( "ue17.bin", 0x000000, 0x40000, BAD_DUMP CRC(da6c3fc8) SHA1(4bc01bc6f62553f6ac4f7252f7d9bf0d639f6935) )
	/* gfx bitmaps */
	ROM_LOAD16_BYTE( "938.bin",  0x400000, 0x80000, CRC(61c06b60) SHA1(b3abae020009a48b99862766e0981e1118159a47) ) // good title background
	ROM_LOAD16_BYTE( "942.bin",  0x400001, 0x80000, CRC(282413b8) SHA1(e2ecaaa3c5b2355eadc016b73d7d658f25e1e0db) ) // (and corrupt gfx on select mode screen)

	ROM_LOAD16_BYTE( "934.bin",  0x300000, 0x80000, CRC(1e65988a) SHA1(64d6f8cbdb28755515d9bbf52f589ce1176fed58) ) // good, girls
	ROM_LOAD16_BYTE( "939.bin",  0x300001, 0x80000, CRC(8790a6a3) SHA1(94f39e48b75144cab191e2de4284c28d18b8f1c7))

	ROM_LOAD16_BYTE( "936.bin",  0x200000, 0x80000, CRC(596543cc) SHA1(10a0eab4ca4a8749f1703ff6fcc80d731d07d087) ) // good, girls
	ROM_LOAD16_BYTE( "940.bin",  0x200001, 0x80000, CRC(0c9dfb53) SHA1(541bd8c79408b7415713b517eacdd565d0ac5cb8) )

	ROM_LOAD16_BYTE( "937.bin",  0x100000, 0x80000, CRC(61dd653f) SHA1(68b5ae3423363cc64d933836bf6881431dad021a) ) // good, girls
	ROM_LOAD16_BYTE( "941.bin",  0x100001, 0x80000, CRC(320321ed) SHA1(00b52cd34cd86c105ff6dbd0248ff239de31c851) )

	ROM_REGION( 0x100000, "gfx1", 0 ) // sprites
	ROM_LOAD( "u5.bin",  0x000000, 0x80000,  CRC(c274d8b5) SHA1(2c45961aaf8311f027a734df7e33fe085dfdd099) )

	ROM_REGION( 0x140000, "oki", 0 ) /* Samples */
	ROM_LOAD( "snd.bin", 0x00000, 0x80000,  CRC(bc20423e) SHA1(1f4bd52ec4f9b3b3e6b10ac2b3afaadf76a2c7c9) )
	ROM_RELOAD(          0x40000, 0x80000 )
	ROM_RELOAD(          0xc0000, 0x80000 )
ROM_END


/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(expro02_state,expro02)
{
	UINT32 *src = (UINT32 *)memregion("gfx3" )->base();
	UINT32 *dst = (UINT32 *)memregion("gfx2" )->base();
	int x, offset;

	// the VIEW2 tiledata is scrambled
	if (src)
	{
		for (x = 0; x < 0x80000; x++)
		{
			offset = x;

			// swap bits around to simplify further processing
			offset = BITSWAP24(offset, 23, 22, 21, 20, 19, 18, 15, 9, 10, 8, 7, 12, 13, 16, 17, 6, 5, 4, 3, 14, 11, 2, 1, 0);

			// invert 8 bits
			offset ^= 0x0528f;

			// addition affecting 9 bits
			offset = (offset & ~0x001ff) | ((offset + 0x00043) & 0x001ff);

			// subtraction affecting 8 bits
			offset = (offset & ~0x1fe00) | ((offset - 0x09600) & 0x1fe00);

			// reverse the initial bitswap
			offset = BITSWAP24(offset, 23, 22, 21, 20, 19, 18, 9, 10, 17, 4, 11, 12, 3, 15, 16, 14, 13, 8, 7, 6, 5, 2, 1, 0);

			// swap nibbles to use the same gfxdecode
			dst[x] = (src[offset] << 4 & 0xF0F0F0F0) | (src[offset] >> 4 & 0x0F0F0F0F);
		}
	}
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, galsnew,   0,        expro02,  expro02,  expro02_state, expro02, ROT90, "Kaneko",                  "Gals Panic (US, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL )
GAME( 1990, galsnewa,  galsnew,  expro02,  galsnewa, expro02_state, expro02, ROT90, "Kaneko",                  "Gals Panic (Export, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL )
GAME( 1990, galsnewj,  galsnew,  expro02,  galsnewj, expro02_state, expro02, ROT90, "Kaneko (Taito license)",  "Gals Panic (Japan, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL )
GAME( 1990, galsnewk,  galsnew,  expro02,  galsnewj, expro02_state, expro02, ROT90, "Kaneko (Inter license)",  "Gals Panic (Korea, EXPRO-02 PCB)", MACHINE_NO_COCKTAIL )
/* the first version of Fantasia clones the EXPRO02 almost exactly, including the encrypted tiles*/
GAME( 1994, fantasia,  0,        comad,    fantasia, expro02_state, expro02, ROT90, "Comad & New Japan System", "Fantasia (940429 PCB, set 1)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, fantasiab, fantasia, comad,    fantasia, expro02_state, expro02, ROT90, "Comad & New Japan System", "Fantasia (940429 PCB, set 2)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
GAME( 1994, fantasiaa, fantasia, comad,    fantasia, expro02_state, expro02, ROT90, "Comad & New Japan System", "Fantasia (940307 PCB)", MACHINE_NO_COCKTAIL | MACHINE_IMPERFECT_GRAPHICS )
/* subsequent releases remove the encrypted tile (View2 layer) but leave the unused writes to it in the program code */
GAME( 1994, fantasian,fantasia, fantasia,  fantasiaa, driver_device, 0, ROT90, "Comad & New Japan System", "Fantasia (940803 PCB)", MACHINE_NO_COCKTAIL )

GAME( 1994, supmodel, 0,        supmodel, fantasiaa, driver_device, 0, ROT90, "Comad & New Japan System", "Super Model",MACHINE_NO_COCKTAIL )

GAME( 1995, newfant,  0,        fantasia, fantasiaa, driver_device, 0, ROT90, "Comad & New Japan System", "New Fantasia (1995 copyright)", MACHINE_NO_COCKTAIL ) // the only difference between the two is the gfx rom containing the copyright
GAME( 1994, newfanta, newfant,  fantasia, fantasiaa, driver_device, 0, ROT90, "Comad & New Japan System", "New Fantasia (1994 copyright)", MACHINE_NO_COCKTAIL )
GAME( 1995, fantsy95, newfant,  fantasia, fantasiaa, driver_device, 0, ROT90, "Hi-max Technology Inc.",   "Fantasy '95", MACHINE_NO_COCKTAIL )

GAME( 1996, missw96,  0,        fantasia, missw96,   driver_device, 0, ROT0,  "Comad",                    "Miss World '96 (Nude) (set 1)", MACHINE_NO_COCKTAIL )
GAME( 1996, missw96a, missw96,  fantasia, missw96,   driver_device, 0, ROT0,  "Comad",                    "Miss World '96 (Nude) (set 2)", MACHINE_NO_COCKTAIL )
GAME( 1996, missw96b, missw96,  fantasia, missw96,   driver_device, 0, ROT0,  "Comad",                    "Miss World '96 (Nude) (set 3)", MACHINE_NO_COCKTAIL )
GAME( 1996, missw96c, missw96,  fantasia, missw96,   driver_device, 0, ROT0,  "Comad",                    "Miss World '96 (Nude) (set 4)", MACHINE_NO_COCKTAIL )

GAME( 1996, missmw96, missw96,  fantasia, missw96,   driver_device, 0, ROT0,  "Comad",                    "Miss Mister World '96 (Nude)", MACHINE_NO_COCKTAIL )

GAME( 1996, smissw,   0,        smissw,   missw96,   driver_device, 0, ROT0,  "Comad",                    "Super Miss World", MACHINE_NO_COCKTAIL ) // 951127 PCB

GAME( 1997, fantsia2, 0,        fantsia2, missw96,   driver_device, 0, ROT0,  "Comad",                    "Fantasia II (Explicit)", MACHINE_NO_COCKTAIL )
GAME( 1997, fantsia2a,fantsia2, fantsia2, missw96,   driver_device, 0, ROT0,  "Comad",                    "Fantasia II (Less Explicit)", MACHINE_NO_COCKTAIL )
GAME( 1998, fantsia2n,fantsia2, fantsia2, missw96,   driver_device, 0, ROT0,  "Comad",                    "Fantasia II (1998)", MACHINE_NO_COCKTAIL )

GAME( 2002, wownfant, 0,        fantsia2, missw96,   driver_device, 0, ROT0,  "Comad",                    "WOW New Fantasia", MACHINE_NO_COCKTAIL )

GAME( 1996, pgalvip,  0,        galhustl, galhustl,  driver_device, 0, ROT0,  "ACE International / Afega","Pocket Gals V.I.P (set 1)", MACHINE_IMPERFECT_GRAPHICS ) // roms were all AFEGA stickered, select screen seems wrong? maybe not a final version.
GAME( 1997, pgalvipa, pgalvip,  galhustl, galhustl,  driver_device, 0, ROT0,  "<unknown>",                "Pocket Gals V.I.P (set 2)", 0 )
GAME( 1997, galhustl, pgalvip,  galhustl, galhustl,  driver_device, 0, ROT0,  "ACE International",        "Gals Hustler", 0 ) // hack of the above?

GAME( 1995, zipzap,   0,        zipzap,   zipzap,    driver_device, 0, ROT90, "Barko Corp",               "Zip & Zap", MACHINE_IMPERFECT_GRAPHICS|MACHINE_IMPERFECT_SOUND )
