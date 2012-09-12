/*

   Kaneko EXPRO-02 board

  Used by the newer revisions of Gals Panic

 Notes:
  - In gfx data banking function, some strange gfx are shown. Timing issue?

 TODO:
 - irq sources are unknown at current time


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



*/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "machine/kaneko_hit.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"

class expro02_state : public driver_device
{
public:
	expro02_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_galsnew_bg_pixram(*this, "galsnew_bgram"),
		m_galsnew_fg_pixram(*this, "galsnew_fgram"),
		m_view2_0(*this, "view2_0"),
		m_kaneko_spr(*this, "kan_spr"),
		m_spriteram(*this, "spriteram")
	{ }

	optional_shared_ptr<UINT16> m_galsnew_bg_pixram;
	optional_shared_ptr<UINT16> m_galsnew_fg_pixram;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;
	optional_shared_ptr<UINT16> m_spriteram;

	UINT16 m_vram_0_bank_num;
	UINT16 m_vram_1_bank_num;
	DECLARE_WRITE16_MEMBER(galsnew_6295_bankswitch_w);
	DECLARE_WRITE16_MEMBER(galsnew_paletteram_w);
	DECLARE_WRITE16_MEMBER(galsnew_vram_0_bank_w);
	DECLARE_WRITE16_MEMBER(galsnew_vram_1_bank_w);
	DECLARE_DRIVER_INIT(galsnew);
};


PALETTE_INIT( galsnew )
{
	int i;

	/* first 2048 colors are dynamic */

	/* initialize 555 RGB lookup */
	for (i = 0; i < 32768; i++)
		palette_set_color_rgb(machine,2048 + i,pal5bit(i >> 5),pal5bit(i >> 10),pal5bit(i >> 0));
}

SCREEN_UPDATE_IND16( galsnew )
{
	expro02_state *state = screen.machine().driver_data<expro02_state>();
//  kaneko16_fill_bitmap(screen.machine(),bitmap,cliprect);
	int y,x;
	int count;


	count = 0;
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (state->m_galsnew_fg_pixram[count] & 0xfffe)>>1;
			dat+=2048;
			dest[x] = dat;
			count++;
		}
	}

	count = 0;
	for (y=0;y<256;y++)
	{
		UINT16 *dest = &bitmap.pix16(y);

		for (x=0;x<256;x++)
		{
			UINT16 dat = (state->m_galsnew_bg_pixram[count]);
			//dat &=0x3ff;
			if (dat)
				dest[x] = dat;

			count++;
		}
	}



	int i;

	screen.machine().priority_bitmap.fill(0, cliprect);

	state->m_view2_0->kaneko16_prepare(bitmap, cliprect);

	for ( i = 0; i < 8; i++ )
	{
		state->m_view2_0->render_tilemap_chip(bitmap,cliprect,i);
	}

	state->m_kaneko_spr->kaneko16_render_sprites(screen.machine(),bitmap,cliprect, state->m_spriteram, state->m_spriteram.bytes());
	return 0;
}

VIDEO_START( galsnew )
{

}


/*************************************
 *
 *  Game-specific port definitions
 *
 *************************************/

static INPUT_PORTS_START( galsnew )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Difficulty ) )	PORT_DIPLOCATION("SWA:1,2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0x0003, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x000C, 0x000C, DEF_STR( Lives ) )	PORT_DIPLOCATION("SWA:3,4")
	PORT_DIPSETTING(      0x0004, "2" )
	PORT_DIPSETTING(      0x000C, "3" )
	PORT_DIPSETTING(      0x0008, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0010, 0x0010, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( On ) )
	PORT_DIPNAME( 0x0020, 0x0020, "Use Button" )		PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0040, 0x0040, "Censored Girls" )	PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0080, 0x0080, "Show Ending Picture" )	PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )

	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(1)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)	/* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0001, 0x0001, "SWB:1" )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Flip_Screen ) )	PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x0004, IP_ACTIVE_LOW, "SWB:3" )
	PORT_DIPNAME( 0x0038, 0x0038, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SWB:4,5,6")
	PORT_DIPSETTING(      0x0028, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0038, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0018, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0010, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(      0x0008, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x0080, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 1C_6C ) )


	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)	/* "Shot2" in "test mode" */
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW3")
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
	PORT_INCLUDE( galsnew )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SWA:3")
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0008, 0x0008, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(      0x0008, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Lives ) )	PORT_DIPLOCATION("SWA:5,6")
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0030, "3" )
	PORT_DIPSETTING(      0x0020, "4" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Demo_Sounds ) )	PORT_DIPLOCATION("SWA:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( galsnewj )
	PORT_INCLUDE( galsnewa )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x0008, 0x0008, "SWB:4" )
	PORT_DIPNAME( 0x0030, 0x0030, DEF_STR( Coin_A ) )	PORT_DIPLOCATION("SWB:5,6")
	PORT_DIPSETTING(      0x0010, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0030, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x00c0, 0x00c0, DEF_STR( Coin_B ) )	PORT_DIPLOCATION("SWB:7,8")
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x00c0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


static INPUT_PORTS_START( fantasia )
	PORT_INCLUDE( galsnewa )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SWA:8")
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
INPUT_PORTS_END

/*************************************
 *
 *  Sound handlers
 *
 *************************************/

WRITE16_MEMBER(expro02_state::galsnew_6295_bankswitch_w)
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

WRITE16_MEMBER(expro02_state::galsnew_paletteram_w)
{

	data = COMBINE_DATA(&m_generic_paletteram_16[offset]);
	palette_set_color_rgb(machine(),offset,pal5bit(data >> 6),pal5bit(data >> 11),pal5bit(data >> 1));
}




/*************************************
 *
 *  CPU memory handlers
 *
 *************************************/

static ADDRESS_MAP_START( galsnew_map, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM // main program
	AM_RANGE(0x080000, 0x0fffff) AM_ROM AM_REGION("user2",0) // other data
	AM_RANGE(0x100000, 0x3fffff) AM_ROM AM_REGION("user1",0) // main data
	AM_RANGE(0x400000, 0x400001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)


	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("galsnew_bgram")
	AM_RANGE(0x520000, 0x53ffff) AM_RAM AM_SHARE("galsnew_fgram")

	AM_RANGE(0x580000, 0x583fff) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_vram_r, kaneko_tmap_vram_w )

	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE(galsnew_paletteram_w) AM_SHARE("paletteram") // palette?

	AM_RANGE(0x680000, 0x68001f) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_regs_r, kaneko_tmap_regs_w)

	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_SHARE("spriteram")	 // sprites? 0x72f words tested

	AM_RANGE(0x780000, 0x78001f) AM_DEVREADWRITE("kan_spr", kaneko16_sprite_device, kaneko16_sprites_regs_r, kaneko16_sprites_regs_w)

	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("DSW3")

	AM_RANGE(0x900000, 0x900001) AM_WRITE(galsnew_6295_bankswitch_w)

	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP	/* ??? */

	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM

	AM_RANGE(0xd80000, 0xd80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_1_tilebank_w)	/* ??? */

	AM_RANGE(0xe00000, 0xe00015) AM_DEVREADWRITE("calc1_mcu", kaneko_hit_device, kaneko_hit_r,kaneko_hit_w)

	AM_RANGE(0xe80000, 0xe80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_0_tilebank_w)	/* ??? */
ADDRESS_MAP_END


// bigger rom space, OKI commands moved
//  no CALC mcu
static ADDRESS_MAP_START( fantasia_map, AS_PROGRAM, 16, expro02_state )
	AM_RANGE(0x000000, 0x4fffff) AM_ROM
	AM_RANGE(0x500000, 0x51ffff) AM_RAM AM_SHARE("galsnew_bgram")
	AM_RANGE(0x520000, 0x53ffff) AM_RAM AM_SHARE("galsnew_fgram")
	AM_RANGE(0x580000, 0x583fff) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_vram_r, kaneko_tmap_vram_w )
	AM_RANGE(0x600000, 0x600fff) AM_RAM_WRITE(galsnew_paletteram_w) AM_SHARE("paletteram") // palette?
	AM_RANGE(0x680000, 0x68001f) AM_DEVREADWRITE("view2_0", kaneko_view2_tilemap_device,  kaneko_tmap_regs_r, kaneko_tmap_regs_w)
	AM_RANGE(0x700000, 0x700fff) AM_RAM AM_SHARE("spriteram")	 // sprites? 0x72f words tested
	AM_RANGE(0x780000, 0x78001f) AM_DEVREADWRITE("kan_spr", kaneko16_sprite_device, kaneko16_sprites_regs_r, kaneko16_sprites_regs_w)
	AM_RANGE(0x800000, 0x800001) AM_READ_PORT("DSW1")
	AM_RANGE(0x800002, 0x800003) AM_READ_PORT("DSW2")
	AM_RANGE(0x800004, 0x800005) AM_READ_PORT("DSW3")
	AM_RANGE(0x800006, 0x800007) AM_NOP // ? used ?
	AM_RANGE(0x900000, 0x900001) AM_WRITE(galsnew_6295_bankswitch_w)
	AM_RANGE(0xa00000, 0xa00001) AM_WRITENOP	/* ??? */
	AM_RANGE(0xc80000, 0xc8ffff) AM_RAM
	AM_RANGE(0xd80000, 0xd80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_1_tilebank_w)	/* ??? */
	AM_RANGE(0xe80000, 0xe80001) AM_DEVWRITE("view2_0",kaneko_view2_tilemap_device,galsnew_vram_0_tilebank_w)	/* ??? */
	AM_RANGE(0xf00000, 0xf00001) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0xff00)
ADDRESS_MAP_END

/*************************************
 *
 *  Initialization & interrupts
 *
 *************************************/

static TIMER_DEVICE_CALLBACK( expro02_scanline )
{
	int scanline = param;

	if(scanline == 224) // vblank-out irq
		timer.machine().device("maincpu")->execute().set_input_line(3, HOLD_LINE);
	else if(scanline == 0) // vblank-in irq?
		timer.machine().device("maincpu")->execute().set_input_line(5, HOLD_LINE);
	else if(scanline == 112) // VDP end task? (controls sprite colors in gameplay)
		timer.machine().device("maincpu")->execute().set_input_line(4, HOLD_LINE);
}

static MACHINE_RESET( galsnew )
{
//  expro02_state *state = machine.driver_data<expro02_state>();
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


static GFXDECODE_START( 1x4bit_1x4bit )
	GFXDECODE_ENTRY( "gfx1", 0, layout_16x16x4, 0x100,	    0x40 ) // [0] Sprites
	GFXDECODE_ENTRY( "gfx2", 0, layout_16x16x4, 0x400,		0x40 ) // [0] bg tiles
GFXDECODE_END

/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( galsnew, expro02_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)
	MCFG_CPU_PROGRAM_MAP(galsnew_map)
	MCFG_TIMER_ADD_SCANLINE("scantimer", expro02_scanline, "screen", 0, 1)

	/* CALC01 MCU @ 16Mhz (unknown type, simulated) */

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 0, 256-32-1)
	MCFG_SCREEN_UPDATE_STATIC(galsnew)

	MCFG_GFXDECODE(1x4bit_1x4bit)
	MCFG_PALETTE_LENGTH(2048 + 32768)
	MCFG_MACHINE_RESET( galsnew )

	MCFG_DEVICE_ADD("view2_0", KANEKO_TMAP, 0)
	kaneko_view2_tilemap_device::set_gfx_region(*device, 1);
	kaneko_view2_tilemap_device::set_offset(*device, 0x5b, 0x8, 256, 224);

	MCFG_DEVICE_ADD_VU002_SPRITES
	kaneko16_sprite_device::set_priorities(*device, 8,8,8,8); // above all (not verified)
	kaneko16_sprite_device::set_offsets(*device, 0, -0x40);

	MCFG_DEVICE_ADD("calc1_mcu", KANEKO_HIT, 0)
	kaneko_hit_device::set_type(*device, 0);


	MCFG_VIDEO_START(galsnew)
	MCFG_PALETTE_INIT(galsnew)

	/* arm watchdog */
	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(3))	/* a guess, and certainly wrong */

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 12000000/6, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


static MACHINE_CONFIG_DERIVED( fantasia, galsnew )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(fantasia_map)

	MCFG_DEVICE_MODIFY("view2_0")
	// these values might not be correct, behavior differs from original boards
	kaneko_view2_tilemap_device::set_invert_flip(*device, 1);
	kaneko_view2_tilemap_device::set_offset(*device, -256, -216, 256, 224);


	MCFG_WATCHDOG_TIME_INIT(attotime::from_seconds(0))	/* a guess, and certainly wrong */

MACHINE_CONFIG_END

/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( galsnew ) /* EXPRO-02 PCB */
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110u_u87-01.u87", 0x000000, 0x20000, CRC(b793a57d) SHA1(12d57b2b4add532f0d0453c25b30d34b3449d717) ) /* US region */
	ROM_LOAD16_BYTE( "pm109u_u88-01.u88", 0x000001, 0x20000, CRC(35b936f8) SHA1(d272067f10542d511a777802cafa4d72b93fa5e8) )

	ROM_REGION16_BE( 0x300000, "user1", 0 )	/* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )	/* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )	/* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83",        0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82",        0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94",        0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm019u_u93-01.u93", 0x180000, 0x010000, CRC(3cb79005) SHA1(05a0b993b9071467265067c3762644f46343d8de) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* sprites - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )


	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
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

	ROM_REGION16_BE( 0x300000, "user1", 0 )	/* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )	/* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017e.u84", 0x00000, 0x80000, CRC(bc41b6ca) SHA1(0aeaf024dd7c84550e7df27230a1d4f04cc1d61c) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )	/* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	/* U93 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* tiles - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
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

	ROM_REGION16_BE( 0x300000, "user1", 0 )	/* 68000 data */
	ROM_LOAD16_BYTE( "pm004e.u86", 0x000001, 0x80000, CRC(d3af52bc) SHA1(46be057106388578defecab1cdd1793ec76ebe92) )
	ROM_LOAD16_BYTE( "pm005e.u85", 0x000000, 0x80000, CRC(d7ec650c) SHA1(6c2250c74381497154bf516e0cf1db6bb56bb446) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", ROMREGION_ERASEFF )	/* contains real (non-cartoon) women, used after each 3rd round */
	/* U84 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )	/* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83", 0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82", 0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94", 0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	/* U93 is an empty socket and not used with this set */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* tiles - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "pm008j.u46", 0x00000, 0x80000, CRC(f394670e) SHA1(171f8dc519a13f352e6440aaadebe490c82361f0) )
	ROM_RELOAD(             0x40000, 0x80000 )
	ROM_LOAD( "pm007j.u47", 0xc0000, 0x80000, CRC(06780287) SHA1(8b9b57f6604b86d6dff42e5e51cd59a7111e1e79) )
ROM_END

ROM_START( galsnewk ) /* EXPRO-02 PCB, Korean title is "Ddang Dda Meok Gi" */
	ROM_REGION( 0x40000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "pm110k.u87", 0x000000, 0x20000, CRC(babe6a71) SHA1(91a5fc5e93affd01f8c6d5a4851233edcf8746f0) )
	ROM_LOAD16_BYTE( "pm109k.u88", 0x000001, 0x20000, CRC(e486d98f) SHA1(9923f1dc69bd2746c06da6a5e518211391052259) )

	ROM_REGION16_BE( 0x300000, "user1", 0 )	/* 68000 data */
	ROM_LOAD16_BYTE( "pm004k.u86", 0x000001, 0x80000, CRC(9a14c8a3) SHA1(c3992eceb8d7d65f781b31dc77bebc73cf9303b6) )
	ROM_LOAD16_BYTE( "pm005k.u85", 0x000000, 0x80000, CRC(33b5d0e3) SHA1(88eef6aff8054b07173da3bb1383fb47a1f7980c) )
	ROM_LOAD16_BYTE( "pm000e.u74", 0x100001, 0x80000, CRC(5d220f3f) SHA1(7ff373e01027c8832712f7a2d732f8e49b875878) )
	ROM_LOAD16_BYTE( "pm001e.u73", 0x100000, 0x80000, CRC(90433eb1) SHA1(8688a85747ad9ecac395d782f130baa64fb9d12b) )
	ROM_LOAD16_BYTE( "pm002e.u76", 0x200001, 0x80000, CRC(713ee898) SHA1(c9f608a57fb90e5ee15eb76a74a7afcc406d5b4e) )
	ROM_LOAD16_BYTE( "pm003e.u75", 0x200000, 0x80000, CRC(6bb060fd) SHA1(4fc3946866c5a55e8340b62b5ad9beae723ce0da) )

	ROM_REGION16_BE( 0x80000, "user2", 0 )	/* contains real (non-cartoon) women, used after each 3rd round */
	ROM_LOAD16_WORD_SWAP( "pm017k.u84", 0x00000, 0x80000, CRC(0c656fb5) SHA1(4610800a460c9f50f7a2ee7b2984bf8e79b62124) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASEFF )	/* sprites */
	/* the 06e rom from the other type gals panic board ends up split across 2 roms here */
	ROM_LOAD( "pm006e.u83",        0x000000, 0x080000, CRC(a7555d9a) SHA1(f95821b3358d9ab03ca9ead38fd358062259d89d) )
	ROM_LOAD( "pm206e.u82",        0x080000, 0x080000, CRC(cc978baa) SHA1(59a95bcbaeca9d356f61ea42af4da116afbb1491) )
	ROM_LOAD( "pm018e.u94",        0x100000, 0x080000, CRC(f542d708) SHA1(f515cca9e96401303ed45b4372f6079f29b7a999) )
	ROM_LOAD( "pm19k.u93",         0x180000, 0x010000, CRC(c17d2989) SHA1(895f44a58dcf0065d42125d439dcc10f41563a94) ) // ?? seems to be an extra / replacement enemy?, not sure where it maps, or when it's used, it might load over another rom

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* sprites - encrypted */
	ROM_LOAD( "pm013e.u89", 0x000000, 0x080000, CRC(10f27b05) SHA1(0f8ade713f6b430b5a23370a17326d53229951de) )
	ROM_LOAD( "pm014e.u90", 0x080000, 0x080000, CRC(2f367106) SHA1(1cd16e286e77e8e1b7668bbb6f2978101656b720) )
	ROM_LOAD( "pm015e.u91", 0x100000, 0x080000, CRC(a563f8ef) SHA1(6e4171746e4d401992bf3a7619d5bed0063d57e5) )
	ROM_LOAD( "pm016e.u92", 0x180000, 0x080000, CRC(c0b9494c) SHA1(f0b066dd78eb9fcf947da90ddb6c7b62299c5743) )


	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
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
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "16.pro2",  0x000000, 0x80000, CRC(e27c6c57) SHA1(420b66928c46e76fa2496f221691dd6c34542287) ) /* PCB location is silkscreened under EPROM sockets */
	ROM_LOAD16_BYTE( "13.pro1",  0x000001, 0x80000, CRC(68d27413) SHA1(84cb7d6523325496469d621f6f4da1b719162147) )
	ROM_LOAD16_BYTE( "9.bg7",    0x100000, 0x80000, CRC(2a588393) SHA1(ef66ed94dd40a95a9b0fb5c3b075c1f654f60927) )
	ROM_LOAD16_BYTE( "5.bg3",    0x100001, 0x80000, CRC(6160e0f0) SHA1(faec9d082c9039885afa4560aa87c05e9ecb5217) )
	ROM_LOAD16_BYTE( "8.bg6",    0x200000, 0x80000, CRC(f776b743) SHA1(bd4d666ede454a56181e109745ac4b3203b2a87c) )
	ROM_LOAD16_BYTE( "4.bg2",    0x200001, 0x80000, CRC(5df0dff2) SHA1(62ebd3c79f2e8ab30d6862cc4bf80f1b56f1f572) )
	ROM_LOAD16_BYTE( "7.bg5",    0x300000, 0x80000, CRC(5707d861) SHA1(33f1cff693dfcb04edbf8738d3ea2a1884e6ff0c) )
	ROM_LOAD16_BYTE( "3.bg1",    0x300001, 0x80000, CRC(36cb811a) SHA1(403cef012990b0e01b481b8afc6b5811e7137833) )
	ROM_LOAD16_BYTE( "10.imag2", 0x400000, 0x80000, CRC(1f14a395) SHA1(12ca5a5a30963ecf90f5a006029aa1098b9ee1df) )
	ROM_LOAD16_BYTE( "6.imag1",  0x400001, 0x80000, CRC(faf870e4) SHA1(163a9aa3e5c550d3760d32e31048a7aa1f93db7f) )

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "17.scr3",  0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) )
	/* SCR1 and SCR2 are unpopulated */

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "2.music1", 0x00000, 0x80000, CRC(22955efb) SHA1(791c18d1aa0c10810da05c199108f51f99fe1d49) )
	ROM_RELOAD(           0x40000, 0x80000 )
	ROM_LOAD( "1.music2", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) )

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* tiles - encrypted */
	ROM_LOAD16_BYTE( "15.obj3", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) )
	ROM_LOAD16_BYTE( "12.obj1", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "14.obj4", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "11.obj2", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END


ROM_START( fantasiaa ) /* PCB silkscreened COMAD INDUSTRY CO.,LTD 940307 MADE IN KOREA */
	ROM_REGION( 0x500000, "maincpu", 0 )	/* 68000 code */
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

	ROM_REGION( 0x80000, "gfx1", 0 )	/* sprites */
	ROM_LOAD( "obj1_17.u5",  0x00000, 0x80000, CRC(aadb6eb7) SHA1(6eaa994ad7b4e8341360eaf5ddb46240316b7274) ) /* same data, different PCB location */
	/* U4 OBJ2 18 and U3 OBJ3 19 are unpopulated */

	ROM_REGION( 0x140000, "oki", 0 )	/* OKIM6295 samples */
	/* 00000-2ffff is fixed, 30000-3ffff is bank switched from all the ROMs */
	ROM_LOAD( "music1_1.ub6", 0x00000, 0x80000, CRC(af0be817) SHA1(5c8897dcd9957add19ff9553c01ce03fec68b354) ) /* This sound sample is different???? */
	ROM_RELOAD(               0x40000, 0x80000 )
	ROM_LOAD( "music2_2.uc6", 0xc0000, 0x80000, CRC(4cd4d6c3) SHA1(a617472a810aef6d82f5fe75ef2980c03c21c2fa) ) /* same data, different PCB location */

	ROM_REGION( 0x200000, "gfx2", ROMREGION_ERASEFF )	/* sprites */

	ROM_REGION( 0x200000, "gfx3", 0 )	/* tiles - encrypted */
	ROM_LOAD16_BYTE( "g-scr2_15.ul16b", 0x000001, 0x80000, CRC(46666768) SHA1(7281c4b45f6f9f6ad89fa2bb3f67f30433c0c513) ) /* same data, different PCB location */
	ROM_LOAD16_BYTE( "g-scr1_12.ul16a", 0x000000, 0x80000, CRC(4bd25be6) SHA1(9834f081c0390ccaa1234efd2393b6495e946c64) )
	ROM_LOAD16_BYTE( "g-scr4_14.ul19b", 0x100001, 0x80000, CRC(4e7e6ed4) SHA1(3e9e942e3de398edc8ac9f82769c3f41708d3741) )
	ROM_LOAD16_BYTE( "g-scr3_11.ul19a", 0x100000, 0x80000, CRC(6d00a4c5) SHA1(8fc0d78200b82ab87658d364ebe2f2e7239722e7) )
ROM_END

/*************************************
 *
 *  Generic driver initialization
 *
 *************************************/

DRIVER_INIT_MEMBER(expro02_state,galsnew)
{
	UINT32 *src = (UINT32 *)machine().root_device().memregion("gfx3" )->base();
	UINT32 *dst = (UINT32 *)machine().root_device().memregion("gfx2" )->base();
	int x, offset;


	for (x = 0; x < 0x80000; x++)
	{
		offset = x;

		// swap bits around to simplify further processing
		offset = BITSWAP24(offset, 23,22,21,20,19, 18,15, 9,10,8,7,12,13,16,17, 6,5,4,3,14,11,2,1,0);

		// invert 8 bits
		offset ^= 0x0528f;

		// addition affecting 9 bits
		offset = (offset & ~0x001ff) | ((offset + 0x00043) & 0x001ff);

		// subtraction affecting 8 bits
		offset = (offset & ~0x1fe00) | ((offset - 0x09600) & 0x1fe00);

		// reverse the initial bitswap
		offset = BITSWAP24(offset, 23,22,21,20,19, 18,9,10,17,4,11,12,3,15,16,14,13,8,7,6,5,2,1,0);

		// swap nibbles to use the same gfxdecode
		dst[x] = (src[offset] << 4 & 0xF0F0F0F0) | (src[offset] >> 4 & 0x0F0F0F0F);
	}
}

/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1990, galsnew,   0,        galsnew,  galsnew,  expro02_state, galsnew, ROT90, "Kaneko",                  "Gals Panic (US, EXPRO-02 PCB)", GAME_NO_COCKTAIL )
GAME( 1990, galsnewa,  galsnew,  galsnew,  galsnewa, expro02_state, galsnew, ROT90, "Kaneko",                  "Gals Panic (Export, EXPRO-02 PCB)", GAME_NO_COCKTAIL )
GAME( 1990, galsnewj,  galsnew,  galsnew,  galsnewj, expro02_state, galsnew, ROT90, "Kaneko (Taito license)",  "Gals Panic (Japan, EXPRO-02 PCB)", GAME_NO_COCKTAIL )
GAME( 1990, galsnewk,  galsnew,  galsnew,  galsnewj, expro02_state, galsnew, ROT90, "Kaneko (Inter license)",  "Gals Panic (Korea, EXPRO-02 PCB)", GAME_NO_COCKTAIL )

GAME( 1994, fantasia,  0,        fantasia, fantasia, expro02_state, galsnew, ROT90, "Comad & New Japan System", "Fantasia (940429 PCB)", GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
GAME( 1994, fantasiaa, fantasia, fantasia, fantasia, expro02_state, galsnew, ROT90, "Comad & New Japan System", "Fantasia (940307 PCB)", GAME_NO_COCKTAIL | GAME_IMPERFECT_GRAPHICS )
