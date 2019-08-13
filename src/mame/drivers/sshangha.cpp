// license:BSD-3-Clause
// copyright-holders:Bryan McPhail, Charles MacDonald, David Haywood
/***************************************************************************

  Super Shanghai Dragon's Eye             (c) 1992 Hot-B

  PCB is manufactured by either Hot-B or Taito, but uses Data East custom
  chips.

  Emulation by Bryan McPhail, mish@tendril.co.uk
  + Charles MacDonald, David Haywood

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



HB-PCB-A5   M6100691A (distributed by Taito)
+-----------------------------------------------------------+
|         GAL.U89  16.000MHz  28.000MHz                     |
|       YM2203C                                             |
|                   +----+  +------+    SS004.U46 SS003.U47 |
|VR1  Y3014B  Z80   |DE71|  |  DE  |                        |
|                   +----+  |  52  |                        |
|         SS008-1   +----+  +------+          U36*      U38*|
|                   |DE71|  +------+                        |
|                   +----+  |  DE  |                        |
|J      M6295               |  52  |    SS004.U37 SS003.U39 |
|A                  GAL.U64 +------+                        |
|M            58257 GAL.U70                                 |
|M                   84256 +------+                         |
|A  SS005.U86 58257  84256 |  DE  |                         |
|           SS007E.U28     |  55  |                         |
|           SS006E.U27     +------+                     U9* |
|                                                           |
|         MCM2018                                       U10*|
|         MCM2018  GAL.U94                                  |
|         MCM2018  GAL.U63                         SS002.U7 |
|          +-----+ GAL.U66                                  |
|          |DE146|         MC68000P12F-16MHZ       SS001.U8 |
|  SW2 SW1 +-----+                                          |
+-----------------------------------------------------------+

* Denotes unpopulated:
    U9 & U10 are 28pin 27C512
    U36 & U38 are 42pin 8/16Meg mask

    CPU: MC68000P12F 16 MHZ
  Sound: Z80B, YM2203C, Y3014B, OKI M6295
    OSC: 28.0000MHz, 16.0000MHz
    DSW: 8 position dipswitch x 2
    RAM: Sony CXK58257P-12L 32K x 8 SRAM x 2
         Motorola MCM2018AN25 2K x 8 SRAM x 3
         Fujitsu 84256A-70L 32K x 8 SRAM x 2
 Custom: Data East 52 x 2 + Data East 71 x 2 (Sprites)
         Data East 55 (Playfield)
         Data East 146 (I/O, Protection)
         VR1 - Sound pot

***************************************************************************/

#include "emu.h"
#include "includes/sshangha.h"

#include "cpu/z80/z80.h"
#include "cpu/m68000/m68000.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "screen.h"
#include "speaker.h"


#define SSHANGHA_HACK   0



/******************************************************************************/




READ16_MEMBER(sshangha_state::sshanghab_protection16_r) // bootleg inputs
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

READ16_MEMBER( sshangha_state::sshangha_protection_region_d_146_r )
{
	int real_address = 0x3f4000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_deco146->read_data( deco146_addr, cs );
	return data;
}

WRITE16_MEMBER( sshangha_state::sshangha_protection_region_d_146_w )
{
	int real_address = 0x3f4000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco146->write_data( deco146_addr, data, mem_mask, cs );
}

READ16_MEMBER( sshangha_state::sshangha_protection_region_8_146_r )
{
	int real_address = 0x3e0000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	uint16_t data = m_deco146->read_data( deco146_addr, cs );
	return data;
}

WRITE16_MEMBER( sshangha_state::sshangha_protection_region_8_146_w )
{
	int real_address = 0x3e0000 + (offset *2);
	int deco146_addr = bitswap<32>(real_address, /* NC */31,30,29,28,27,26,25,24,23,22,21,20,19,18, 13,12,11,/**/      17,16,15,14,    10,9,8, 7,6,5,4, 3,2,1,0) & 0x7fff;
	uint8_t cs = 0;
	m_deco146->write_data( deco146_addr, data, mem_mask, cs );
}

/*

 Swizzle palette writes a bit so that the 'tilemap_12_combine_draw' code in the tilemap device works with this game (used for girl, see attract mode)

 Normal Palette layout

 0x000 - 0x3ff  Sprites 2
 0x400 - 0x7ff  Tilemap PF1
 0x800 - 0xbff  Sprites 1
 0xc00 - 0xfff  Tilemap PF2

 rearranged to

 0x000 - 0x3ff  Sprites 1
 0x400 - 0x7ff  Sprites 2
 0x800 - 0xbff  Tilemap PF2
 0xc00 - 0xfff  Tilemap PF1

*/

WRITE16_MEMBER(sshangha_state::palette_w)
{
	switch (offset & 0x600)
	{
	case 0x000: offset = (offset & 0x1ff) | 0x200; break;
	case 0x200: offset = (offset & 0x1ff) | 0x600; break;
	case 0x400: offset = (offset & 0x1ff) | 0x000; break;
	case 0x600: offset = (offset & 0x1ff) | 0x400; break;
	}

	m_palette->write16(offset, data, mem_mask);
}

READ16_MEMBER(sshangha_state::palette_r)
{
	switch (offset & 0x600)
	{
	case 0x000: offset = (offset & 0x1ff) | 0x200; break;
	case 0x200: offset = (offset & 0x1ff) | 0x600; break;
	case 0x400: offset = (offset & 0x1ff) | 0x000; break;
	case 0x600: offset = (offset & 0x1ff) | 0x400; break;
	}
	return m_palette->read16(offset);
}

void sshangha_state::sshangha_map(address_map &map)
{
	map.global_mask(0x3fffff);
	map(0x000000, 0x03ffff).rom();
	map(0x100000, 0x10000f).ram().share(m_sound_shared_ram);

	map(0x200000, 0x201fff).rw(m_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x203fff).rw(m_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x204000, 0x2047ff).ram().share(m_pf1_rowscroll);
	map(0x206000, 0x2067ff).ram().share(m_pf2_rowscroll);
	map(0x206800, 0x207fff).ram();
	map(0x300000, 0x30000f).w(m_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x320001).w(FUNC(sshangha_state::video_w));
	map(0x320002, 0x320005).nopw();
	map(0x320006, 0x320007).nopr(); //irq ack

	map(0x340000, 0x3407ff).mirror(0x800).ram().share(m_spriteram2);
	map(0x350000, 0x350001).r(FUNC(sshangha_state::deco_71_r));
	map(0x350000, 0x350007).nopw();
	map(0x360000, 0x3607ff).mirror(0x800).ram().share(m_spriteram);
	map(0x370000, 0x370001).r(FUNC(sshangha_state::deco_71_r));
	map(0x370000, 0x370007).nopw();

	map(0x380000, 0x380fff).ram().rw(FUNC(sshangha_state::palette_r),FUNC(sshangha_state::palette_w)).share("palette");
	map(0x381000, 0x383fff).ram(); // unused palette area
	map(0x3e0000, 0x3e3fff).rw(FUNC(sshangha_state::sshangha_protection_region_8_146_r), FUNC(sshangha_state::sshangha_protection_region_8_146_w));
	map(0x3ec000, 0x3f3fff).ram();
	map(0x3f4000, 0x3f7fff).rw(FUNC(sshangha_state::sshangha_protection_region_d_146_r), FUNC(sshangha_state::sshangha_protection_region_d_146_w)).share(m_prot_data);
}

void sshangha_state::sshanghab_map(address_map &map)
{
	map(0x000000, 0x03ffff).rom();
	map(0x084000, 0x0847ff).r(FUNC(sshangha_state::sshanghab_protection16_r));
	map(0x101000, 0x10100f).ram().share(m_sound_shared_ram); /* the bootleg writes here */

	map(0x200000, 0x201fff).rw(m_tilegen, FUNC(deco16ic_device::pf1_data_r), FUNC(deco16ic_device::pf1_data_w));
	map(0x202000, 0x203fff).rw(m_tilegen, FUNC(deco16ic_device::pf2_data_r), FUNC(deco16ic_device::pf2_data_w));
	map(0x204000, 0x2047ff).ram().share(m_pf1_rowscroll);
	map(0x206000, 0x2067ff).ram().share(m_pf2_rowscroll);
	map(0x206800, 0x207fff).ram();
	map(0x300000, 0x30000f).w(m_tilegen, FUNC(deco16ic_device::pf_control_w));
	map(0x320000, 0x320001).w(FUNC(sshangha_state::video_w));
	map(0x320002, 0x320005).nopw();
	map(0x320006, 0x320007).nopr(); //irq ack

	map(0x340000, 0x340fff).ram(); // original spriteram, used as a buffer here

	map(0x380000, 0x380fff).ram().rw(FUNC(sshangha_state::palette_r),FUNC(sshangha_state::palette_w)).share("palette");
	map(0x381000, 0x383fff).ram(); // unused palette area

	map(0x3c0000, 0x3c07ff).ram().share(m_spriteram); // bootleg spriteram
	map(0x3c0800, 0x3c0fff).ram().share(m_spriteram2);

	map(0xfec000, 0xff3fff).ram();
	map(0xff4000, 0xff47ff).ram();
}

/******************************************************************************/

/* 8 "sound latches" shared between main and sound cpus. */

READ8_MEMBER(sshangha_state::sound_shared_r)
{
	return m_sound_shared_ram[offset] & 0xff;
}

WRITE8_MEMBER(sshangha_state::sound_shared_w)
{
	m_sound_shared_ram[offset] = data & 0xff;
}

/* Note: there's rom data after 0x8000 but the game never seem to call a rom bank, left-over? */
void sshangha_state::sound_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xc001).rw("ymsnd", FUNC(ym2203_device::read), FUNC(ym2203_device::write));
	map(0xc200, 0xc201).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xf800, 0xf807).rw(FUNC(sshangha_state::sound_shared_r), FUNC(sshangha_state::sound_shared_w));
	map(0xf808, 0xffff).ram();
}

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

static GFXDECODE_START( gfx_sshangha )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,  0x000, 64 ) /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,  0x000, 64 ) /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,  0x000, 64 ) /* Sprites 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, tilelayout,  0x000, 64 ) /* Sprites 16x16 */
GFXDECODE_END

/******************************************************************************/

DECO16IC_BANK_CB_MEMBER(sshangha_state::bank_callback)
{
	return (bank >> 4) * 0x1000;
}

void sshangha_state::sshangha(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, 16_MHz_XTAL); /* CPU marked as 16MHz part */
	m_maincpu->set_addrmap(AS_PROGRAM, &sshangha_state::sshangha_map);
	m_maincpu->set_vblank_int("screen", FUNC(sshangha_state::irq6_line_hold));

	Z80(config, m_audiocpu, 16_MHz_XTAL / 4);
	m_audiocpu->set_addrmap(AS_PROGRAM, &sshangha_state::sound_map);


	config.m_minimum_quantum = attotime::from_hz(6000);

	screen_device& screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(529));
	screen.set_size(40*8, 32*8);
	screen.set_visarea(0*8, 40*8-1, 1*8, 31*8-1);
	screen.set_screen_update(FUNC(sshangha_state::screen_update));

	GFXDECODE(config, "gfxdecode", m_palette, gfx_sshangha);

	PALETTE(config, m_palette).set_format(palette_device::xBGR_888, 0x4000);

	DECO16IC(config, m_tilegen, 0);
	m_tilegen->set_pf1_size(DECO_64x32);
	m_tilegen->set_pf2_size(DECO_64x32);
	m_tilegen->set_pf1_trans_mask(0x0f);
	m_tilegen->set_pf2_trans_mask(0x0f);
	m_tilegen->set_pf1_col_bank(0x30);
	m_tilegen->set_pf2_col_bank(0x20);
	m_tilegen->set_pf1_col_mask(0x0f);
	m_tilegen->set_pf2_col_mask(0x0f);
	m_tilegen->set_bank1_callback(FUNC(sshangha_state::bank_callback), this);
	m_tilegen->set_bank2_callback(FUNC(sshangha_state::bank_callback), this);
	m_tilegen->set_pf12_8x8_bank(0);
	m_tilegen->set_pf12_16x16_bank(1);
	m_tilegen->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen1, 0);
	m_sprgen1->set_gfx_region(2);
	m_sprgen1->set_gfxdecode_tag("gfxdecode");

	DECO_SPRITE(config, m_sprgen2, 0);
	m_sprgen2->set_gfx_region(3);
	m_sprgen2->set_gfxdecode_tag("gfxdecode");

	DECO146PROT(config, m_deco146, 0);
	m_deco146->port_a_cb().set_ioport("INPUTS");
	m_deco146->port_b_cb().set_ioport("SYSTEM");
	m_deco146->port_c_cb().set_ioport("DSW");

	/* sound hardware */
	SPEAKER(config, "lspeaker").front_left(); // sure it's stereo?
	SPEAKER(config, "rspeaker").front_right();

	ym2203_device &ymsnd(YM2203(config, "ymsnd", 16_MHz_XTAL / 4));
	ymsnd.irq_handler().set_inputline(m_audiocpu, 0);
	ymsnd.add_route(ALL_OUTPUTS, "lspeaker", 0.33);
	ymsnd.add_route(ALL_OUTPUTS, "rspeaker", 0.33);

	okim6295_device &oki(OKIM6295(config, "oki", 16_MHz_XTAL / 8, okim6295_device::PIN7_LOW)); // clock frequency & pin 7 not verified
	oki.add_route(ALL_OUTPUTS, "lspeaker", 0.27);
	oki.add_route(ALL_OUTPUTS, "rspeaker", 0.27);
}

void sshangha_state::sshanghab(machine_config &config)
{
	sshangha(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &sshangha_state::sshanghab_map);

	config.device_remove("ioprot");
}

/******************************************************************************/

ROM_START( sshangha )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ss007e.u28", 0x00000, 0x20000, CRC(5f275f6e) SHA1(ca7790d2401c95aff48098800f0da9590a0d88a2) )
	ROM_LOAD16_BYTE( "ss006e.u27", 0x00001, 0x20000, CRC(111327fe) SHA1(60f9e839a027eab5ef019dcb27cac2f0f9bf04d8) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008-1.u82", 0x000000, 0x010000, CRC(ff128b54) SHA1(2cdae94000c695417ebfe302999baa8e8cec09bf) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // 2 sprite chips, 2 copies of sprite ROMs on PCB
	ROM_LOAD( "ss003.u47", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u46", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END

ROM_START( sshanghaj )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ss007-1.u28", 0x00000, 0x20000, CRC(bc466edf) SHA1(b96525b2c879d15b46a7753fa6ebf12a851cd019) )
	ROM_LOAD16_BYTE( "ss006-1.u27", 0x00001, 0x20000, CRC(872a2a2d) SHA1(42d7a01465d5c403354aaf0f2dab8adb9afe61b0) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // 2 sprite chips, 2 copies of sprite ROMs on PCB
	ROM_LOAD( "ss003.u47", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u46", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END

ROM_START( sshanghak ) /* Korean censored version - No girls in Paradise games */
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "ss007k.u28", 0x00000, 0x20000, CRC(90dbf11c) SHA1(60ab0f3d3f43939e719196ff1775a3cd1c8c9aa0) )
	ROM_LOAD16_BYTE( "ss006k.u27", 0x00001, 0x20000, CRC(07d94579) SHA1(25e4fb1669e12c7329e45a8ac0d52ac157a83d46) )

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // 2 sprite chips, 2 copies of sprite ROMs on PCB
	ROM_LOAD( "ss003.u47", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u46", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END

ROM_START( sshanghab )
	ROM_REGION( 0x40000, "maincpu", 0 ) /* 68000 code */
	ROM_LOAD16_BYTE( "sshanb_2.010", 0x00000, 0x20000, CRC(bc7ed254) SHA1(aeee4b8a8265902bb41575cc143738ecf3aff57d) )
	ROM_LOAD16_BYTE( "sshanb_1.010", 0x00001, 0x20000, CRC(7b049f49) SHA1(2570077c67dbd35053d475a18c3f10813bf914f7) )

	// TODO: it's unlikely the bootleg used these exact ROMs, they were probably split, verify

	ROM_REGION( 0x10000, "audiocpu", 0 )    /* Sound CPU */
	ROM_LOAD( "ss008.u82", 0x000000, 0x010000, CRC(04dc3647) SHA1(c06a7e8932c03de5759a9b69da0d761006b49517) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "ss001.u8",  0x000000, 0x100000, CRC(ebeca5b7) SHA1(1746e757ad9bbef2aa9028c54f25d4aa4dedf79e) )
	ROM_LOAD( "ss002.u7",  0x100000, 0x100000, CRC(67659f29) SHA1(50944877665b7b848b3f7063892bd39a96a847cf) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD( "ss003.u39", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u37", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x200000, "gfx3", 0 ) // 2 sprite chips, 2 copies of sprite ROMs on PCB
	ROM_LOAD( "ss003.u47", 0x000000, 0x100000, CRC(fbecde72) SHA1(2fe32b28e77ec390c534d276261eefac3fbe21fd) )
	ROM_LOAD( "ss004.u46", 0x100000, 0x100000, CRC(98b82c5e) SHA1(af1b52d4b36b1776c148478b5a5581e6a57256b8) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "ss005.u86", 0x000000, 0x040000, CRC(c53a82ad) SHA1(756e453c8b5ce8e47f93fbda3a9e48bb73e93e2e) )
ROM_END


void sshangha_state::init_sshangha()
{
#if SSHANGHA_HACK
	/* This is a hack to allow you to use the extra features
	     of the first "Unused" Dip Switch (see notes above). */
	uint16_t *RAM = (uint16_t *)memregion("maincpu")->base();
	RAM[0x000384/2] = 0x4e71;
	RAM[0x000386/2] = 0x4e71;
	RAM[0x000388/2] = 0x4e71;
	RAM[0x00038a/2] = 0x4e71;
	/* To avoid checksum error (only useful for 'sshangha') */
	RAM[0x000428/2] = 0x4e71;
	RAM[0x00042a/2] = 0x4e71;
#endif
}


GAME( 1992, sshangha,  0,        sshangha,  sshangha, sshangha_state, init_sshangha, ROT0, "Hot-B",   "Super Shanghai Dragon's Eye (World)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sshanghaj, sshangha, sshangha,  sshangha, sshangha_state, init_sshangha, ROT0, "Hot-B",   "Super Shanghai Dragon's Eye (Japan)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sshanghak, sshangha, sshangha,  sshangha, sshangha_state, init_sshangha, ROT0, "Hot-B",   "Super Shanghai Dragon's Eye (Korea)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, sshanghab, sshangha, sshanghab, sshangha, sshangha_state, init_sshangha, ROT0, "bootleg", "Super Shanghai Dragon's Eye (World, bootleg)", MACHINE_SUPPORTS_SAVE )
