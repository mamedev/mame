// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Charles MacDonald, David Haywood
/***************************************************************************

  Super Shanghai Dragon's Eye             (c) 1992 Hot-B

  PCB is manufactured by either Hot-B or Taito, but uses Data East custom
  chips.

  HB-PCB-A4
  M6100691A (distributed by Taito)

  CPU  : 68000
  Sound: Z80B YM2203 Y3014 M6295
  OSC  : 28.0000MHz 16.0000MHz


  Emulation by Bryan McPhail, mish@tendril.co.uk
  + Charles MacDonald, David Haywood

  ToDo:

  Palette handling is somewhat hacked, see paletteram16_xbgr_word_be_sprites_w
   - check this on the original set

  ----

Stephh's notes (based on the games M68000 code and some tests) :

0) all games

  - There is no confirmation yet that the "Demo Sounds" Dip Switch does
    something as I don't see where bit 0 of 0xfec04a is tested 8(

  - The First "Unused" Dip Switch is probably used in other (older ?) versions
    to act as a "Debug Mode" Dip Switch. When it's ON, you have these features :

      * there is an extended "test mode" that also allows you to test the
        BG and Object ROMS via a menu.
      * You can end a level by pressing BUTTON3 from player 2 8)

  - The "Adult Mode" Dip Switch determines if "Shanghai Paradise" is available.
  - The "Quest Mode" Dip Switch determines if "Shanghai Quest" is available.
  - The "Use Mahjong Tiles" Dip Switch only has an effect when playing
    "Shanghai Advanced".

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "includes/sshangha.h"
#include "machine/deco146.h"

#define SSHANGHA_HACK   0



/******************************************************************************/




READ16_MEMBER(sshangha_state::sshanghb_protection16_r) // bootleg inputs
{
	switch (offset)
	{
		case 0x050 >> 1:
			return ioport("INPUTS")->read();
		case 0x76a >> 1:
			return ioport("SYSTEM")->read();
		case 0x0ac >> 1:
			return ioport("DSW")->read();
	}

	return m_prot_data[offset];
}

/* Probably returns 0xffff when sprite DMA is complete, the game waits on it */
READ16_MEMBER(sshangha_state::deco_71_r)
{
	return 0xffff;
}

/******************************************************************************/

void sshangha_state::machine_reset()
{
}

/******************************************************************************/

inline void sshangha_state::sshangha_set_color_888(pen_t color, int rshift, int gshift, int bshift, UINT32 data)
{
	m_palette->set_pen_color(color, (data >> rshift) & 0xff, (data >> gshift) & 0xff, (data >> bshift) & 0xff);
}


WRITE16_MEMBER(sshangha_state::paletteram16_xbgr_word_be_sprites2_w)
{
	COMBINE_DATA(&m_sprite_paletteram2[offset]);
	sshangha_set_color_888((offset/2)+0x100, 0, 8, 16, m_sprite_paletteram2[(offset) | 1] | (m_sprite_paletteram2[(offset) & ~1] << 16) );
}

WRITE16_MEMBER(sshangha_state::paletteram16_xbgr_word_be_sprites_w)
{
	// hack??? we have to call this otherwise the sprite colours for some selected tiles are wrong (most noticeable on the 2nd level of quest mode)
	// however if we simply mirror the memory both ways the how to play screen ends up with bad colours
	// we use the 2nd copy of palette ram for low priority tiles only..
	// is this related to the bootleg only, or does the original have this issue too?
	// maybe related to sprite DMA on the original, or the apparent lack of a 2nd sprite controller on the bootleg.
	paletteram16_xbgr_word_be_sprites2_w(space,offset,data,mem_mask);

	COMBINE_DATA(&m_sprite_paletteram[offset]);
	sshangha_set_color_888((offset/2)+0x000, 0, 8, 16, m_sprite_paletteram[(offset) | 1] | (m_sprite_paletteram[(offset) & ~1] << 16) );
}

WRITE16_MEMBER(sshangha_state::paletteram16_xbgr_word_be_tilelow_w)
{
	COMBINE_DATA(&m_tile_paletteram1[offset]);
	sshangha_set_color_888((offset/2)+0x200, 0, 8, 16, m_tile_paletteram1[(offset) | 1] | (m_tile_paletteram1[(offset) & ~1] << 16) );
}

WRITE16_MEMBER(sshangha_state::paletteram16_xbgr_word_be_tilehigh_w)
{
	COMBINE_DATA(&m_tile_paletteram2[offset]);
	sshangha_set_color_888((offset/2)+0x300, 0, 8, 16, m_tile_paletteram2[(offset) | 1] | (m_tile_paletteram2[(offset) & ~1] << 16) );
}

READ16_MEMBER( sshangha_state::sshangha_protection_region_d_146_r )
{
	int real_address = 0x3f4000 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	UINT16 data = m_deco146->read_data( deco146_addr, mem_mask, cs );
	return data;
}

WRITE16_MEMBER( sshangha_state::sshangha_protection_region_d_146_w )
{
	int real_address = 0x3f4000 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	m_deco146->write_data( space, deco146_addr, data, mem_mask, cs );
}

READ16_MEMBER( sshangha_state::sshangha_protection_region_8_146_r )
{
	int real_address = 0x3e0000 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	UINT16 data = m_deco146->read_data( deco146_addr, mem_mask, cs );
	return data;
}

WRITE16_MEMBER( sshangha_state::sshangha_protection_region_8_146_w )
{
	int real_address = 0x3e0000 + (offset *2);
	int deco146_addr = BITSWAP32(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	UINT8 cs = 0;
	m_deco146->write_data( space, deco146_addr, data, mem_mask, cs );
}


static ADDRESS_MAP_START( sshangha_map, AS_PROGRAM, 16, sshangha_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fffff)
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x100000, 0x10000f) AM_RAM AM_SHARE("sound_shared")

	AM_RANGE(0x200000, 0x201fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x202000, 0x203fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x204000, 0x2047ff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x206000, 0x2067ff) AM_RAM AM_SHARE("pf2_rowscroll")
	AM_RANGE(0x206800, 0x207fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x320000, 0x320001) AM_WRITE(sshangha_video_w)
	AM_RANGE(0x320002, 0x320005) AM_WRITENOP
	AM_RANGE(0x320006, 0x320007) AM_READNOP //irq ack

	AM_RANGE(0x340000, 0x340fff) AM_RAM AM_SHARE("spriteram2")
	AM_RANGE(0x350000, 0x350001) AM_READ(deco_71_r)
	AM_RANGE(0x350000, 0x350007) AM_WRITENOP
	AM_RANGE(0x360000, 0x360fff) AM_RAM AM_SHARE("spriteram")
	AM_RANGE(0x370000, 0x370001) AM_READ(deco_71_r)
	AM_RANGE(0x370000, 0x370007) AM_WRITENOP

	AM_RANGE(0x380000, 0x3803ff) AM_RAM_WRITE(paletteram16_xbgr_word_be_sprites_w) AM_SHARE("sprite_palram")
	AM_RANGE(0x380400, 0x3807ff) AM_RAM_WRITE(paletteram16_xbgr_word_be_tilehigh_w) AM_SHARE("tile_palram2")
	AM_RANGE(0x380800, 0x380bff) AM_RAM_WRITE(paletteram16_xbgr_word_be_sprites2_w) AM_SHARE("sprite_palram2")
	AM_RANGE(0x380c00, 0x380fff) AM_RAM_WRITE(paletteram16_xbgr_word_be_tilelow_w) AM_SHARE("tile_palram1")
	AM_RANGE(0x381000, 0x383fff) AM_RAM // unused palette area
	AM_RANGE(0x3e0000, 0x3e3fff) AM_READWRITE(sshangha_protection_region_8_146_r,sshangha_protection_region_8_146_w)
	AM_RANGE(0x3ec000, 0x3f3fff) AM_RAM
	AM_RANGE(0x3f4000, 0x3f7fff) AM_READWRITE(sshangha_protection_region_d_146_r,sshangha_protection_region_d_146_w) AM_SHARE("prot_data")
ADDRESS_MAP_END

static ADDRESS_MAP_START( sshanghb_map, AS_PROGRAM, 16, sshangha_state )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x084000, 0x0847ff) AM_READ(sshanghb_protection16_r)
	AM_RANGE(0x101000, 0x10100f) AM_RAM AM_SHARE("sound_shared") /* the bootleg writes here */

	AM_RANGE(0x200000, 0x201fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_r, pf1_data_w)
	AM_RANGE(0x202000, 0x203fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_r, pf2_data_w)
	AM_RANGE(0x204000, 0x2047ff) AM_RAM AM_SHARE("pf1_rowscroll")
	AM_RANGE(0x206000, 0x2067ff) AM_RAM AM_SHARE("pf2_rowscroll")
	AM_RANGE(0x206800, 0x207fff) AM_RAM
	AM_RANGE(0x300000, 0x30000f) AM_DEVWRITE("tilegen1", deco16ic_device, pf_control_w)
	AM_RANGE(0x320000, 0x320001) AM_WRITE(sshangha_video_w)
	AM_RANGE(0x320002, 0x320005) AM_WRITENOP
	AM_RANGE(0x320006, 0x320007) AM_READNOP //irq ack

	AM_RANGE(0x340000, 0x340fff) AM_RAM // original spriteram

	AM_RANGE(0x380000, 0x3803ff) AM_RAM_WRITE(paletteram16_xbgr_word_be_sprites_w) AM_SHARE("sprite_palram")
	AM_RANGE(0x380400, 0x3807ff) AM_RAM_WRITE(paletteram16_xbgr_word_be_tilehigh_w) AM_SHARE("tile_palram2")
	AM_RANGE(0x380800, 0x380bff) AM_RAM_WRITE(paletteram16_xbgr_word_be_sprites2_w) AM_SHARE("sprite_palram2")
	AM_RANGE(0x380c00, 0x380fff) AM_RAM_WRITE(paletteram16_xbgr_word_be_tilelow_w) AM_SHARE("tile_palram1")
	AM_RANGE(0x381000, 0x383fff) AM_RAM // unused palette area

	AM_RANGE(0x3c0000, 0x3c0fff) AM_RAM AM_SHARE("spriteram") // bootleg spriteram
	AM_RANGE(0xfec000, 0xff3fff) AM_RAM
	AM_RANGE(0xff4000, 0xff47ff) AM_RAM
ADDRESS_MAP_END

/******************************************************************************/

/* 8 "sound latches" shared between main and sound cpus. */

READ8_MEMBER(sshangha_state::sshangha_sound_shared_r)
{
	return m_sound_shared_ram[offset] & 0xff;
}

WRITE8_MEMBER(sshangha_state::sshangha_sound_shared_w)
{
	m_sound_shared_ram[offset] = data & 0xff;
}

/* Note: there's rom data after 0x8000 but the game never seem to call a rom bank, left-over? */
static ADDRESS_MAP_START( sshangha_sound_map, AS_PROGRAM, 8, sshangha_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0xc000, 0xc001) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0xc200, 0xc201) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0xf800, 0xf807) AM_READWRITE(sshangha_sound_shared_r,sshangha_sound_shared_w)
	AM_RANGE(0xf808, 0xffff) AM_RAM
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( sshangha )
	PORT_START("INPUTS")    /* 0xfec046.b - 0xfec047.b */
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1) PORT_NAME("P1 Pick Tile")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1) PORT_NAME("P1 Cancel")
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1) PORT_NAME("P1 Help")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2) PORT_NAME("P2 Pick Tile")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2) PORT_NAME("P2 Cancel")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2) PORT_NAME("P2 Help")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("SYSTEM")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

	/* Dips seem inverted with respect to other Deco games */
	PORT_START("DSW")   /* 0xfec04b.b - 0xfec04a.b, inverted bits order */
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(      0x0000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( On ) )
	PORT_DIPNAME( 0x0040, 0x0040, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(      0x0040, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x0020, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPNAME( 0x0010, 0x0010, "Coin Mode" )         PORT_DIPLOCATION("SW1:4") /* Manual states "Always Off" - Check code at 0x000010f2 */
	PORT_DIPSETTING(      0x0010, "Mode 1" )
	PORT_DIPSETTING(      0x0000, "Mode 2" )
	PORT_DIPNAME( 0x000c, 0x000c, DEF_STR( Coin_A ) )   PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(      0x0008, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x0008, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x000c, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x0004, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPNAME( 0x0003, 0x0003, DEF_STR( Coin_B ) )   PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(      0x0002, DEF_STR( 2C_1C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0000, DEF_STR( 2C_3C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_2C ) )        PORT_CONDITION("DSW", 0x0010, EQUALS, 0x0010) //Mode 1
	PORT_DIPSETTING(      0x0000, DEF_STR( 4C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x0002, DEF_STR( 3C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x0003, DEF_STR( 1C_1C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPSETTING(      0x0001, DEF_STR( 1C_4C ) )        PORT_CONDITION("DSW", 0x0010, NOTEQUALS, 0x0010) //Mode 2
	PORT_DIPNAME( 0xc000, 0xc000, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(      0x4000, DEF_STR( Easy ) )
	PORT_DIPSETTING(      0xc000, DEF_STR( Normal ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Hard ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( Hardest ) )
#if SSHANGHA_HACK
	PORT_DIPNAME( 0x2000, 0x2000, "Debug Mode" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x2000, 0x2000, "SW2:3" )    /* Listed as "Unused" - However see notes */
#endif
	PORT_DIPUNUSED_DIPLOC( 0x1000, 0x1000, "SW2:4" )    /* Listed as "Unused" */
	PORT_DIPNAME( 0x0800, 0x0800, "Tile Animation" )    PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0400, 0x0400, "Use Mahjong Tiles" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0200, 0x0200, "Paradise (Adult) Course" )   PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x0100, 0x0100, "Quest Course" )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(      0x0000, DEF_STR( No ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Yes ) )
INPUT_PORTS_END

/******************************************************************************/

static const gfx_layout charlayout =
{
	8,8,    /* 8*8 chars */
	4096,
	4,      /* 4 bits per pixel  */
	{ 8, 0, 0x100000*8+8,0x100000*8+0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 8, 0, 0x100000*8+8, 0x100000*8+0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( sshangha )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0x200, 64 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0x200, 64 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,    0, 64 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

WRITE_LINE_MEMBER(sshangha_state::irqhandler)
{
	m_audiocpu->set_input_line(0, state);
}

DECO16IC_BANK_CB_MEMBER(sshangha_state::bank_callback)
{
	return (bank >> 4) * 0x1000;
}

static MACHINE_CONFIG_START( sshangha, sshangha_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 28000000/2)
	MCFG_CPU_PROGRAM_MAP(sshangha_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", sshangha_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", Z80, 16000000/4)
	MCFG_CPU_PROGRAM_MAP(sshangha_sound_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(sshangha_state, screen_update_sshangha)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", sshangha)
	MCFG_PALETTE_ADD("palette", 0x4000)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x10)
	MCFG_DECO16IC_PF2_COL_BANK(0x00)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(sshangha_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(sshangha_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen1", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen2", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	MCFG_DECO146_ADD("ioprot")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker") /* sure it's stereo? */

	MCFG_SOUND_ADD("ymsnd", YM2203, 16000000/4)
	MCFG_YM2203_IRQ_HANDLER(WRITELINE(sshangha_state, irqhandler))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.33)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.33)

	MCFG_OKIM6295_ADD("oki", 1023924, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.27)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.27)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( sshanghb, sshangha )

	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(sshanghb_map)
MACHINE_CONFIG_END

/******************************************************************************/

ROM_START( sshangha )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ss007-1.u28", 0x00000, 0x20000, CRC(bc466edf) SHA1(b96525b2c879d15b46a7753fa6ebf12a851cd019) )
	ROM_LOAD16_BYTE( "ss006-1.u27", 0x00001, 0x20000, CRC(872a2a2d) SHA1(42d7a01465d5c403354aaf0f2dab8adb9afe61b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) ) /* Copy of rom at u47 */
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) ) /* Copy of rom at u46 */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END

ROM_START( sshanghab )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "sshanb_2.010", 0x00000, 0x20000, CRC(bc7ed254) SHA1(aeee4b8a8265902bb41575cc143738ecf3aff57d) )
	ROM_LOAD16_BYTE( "sshanb_1.010", 0x00001, 0x20000, CRC(7b049f49) SHA1(2570077c67dbd35053d475a18c3f10813bf914f7) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) ) /* Copy of rom at u47 */
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) ) /* Copy of rom at u46 */

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END


DRIVER_INIT_MEMBER(sshangha_state,sshangha)
{
#if SSHANGHA_HACK
	/* This is a hack to allow you to use the extra features
	     of the first "Unused" Dip Switch (see notes above). */
	UINT16 *RAM = (UINT16 *)memregion("maincpu")->base();
	RAM[0x000384/2] = 0x4e71;
	RAM[0x000386/2] = 0x4e71;
	RAM[0x000388/2] = 0x4e71;
	RAM[0x00038a/2] = 0x4e71;
	/* To avoid checksum error (only useful for 'sshangha') */
	RAM[0x000428/2] = 0x4e71;
	RAM[0x00042a/2] = 0x4e71;
#endif
}


GAME( 1992, sshangha, 0,        sshangha, sshangha, sshangha_state, sshangha, ROT0, "Hot-B",   "Super Shanghai Dragon's Eye (Japan)", 0 )
GAME( 1992, sshanghab,sshangha, sshanghb, sshangha, sshangha_state, sshangha, ROT0, "bootleg", "Super Shanghai Dragon's Eye (World, bootleg)", 0 )
