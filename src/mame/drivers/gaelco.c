// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

    Gaelco game hardware from 1991-1996

    Driver by Manuel Abadia

Games supported:

Year   Game                PCB            NOTES
-------------------------------------------------------------
1991   Big Karnak          REF 901112-1   Unprotected
1992   Squash              REF 922804/1   Encrypted Video RAM
1992   Thunder Hoop        REF 922804/1   Encrypted Video RAM
1995   Biomechanical Toy   REF 922804/2   Unprotected
1996   Maniac Square       REF 922804/2   Prototype

***************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"
#include "includes/gaelcrpt.h"
#include "includes/gaelco.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE16_MEMBER(gaelco_state::bigkarnk_sound_command_w)
{
	if (ACCESSING_BITS_0_7)
	{
		soundlatch_byte_w(space, 0, data & 0xff);
		m_audiocpu->set_input_line(M6809_FIRQ_LINE, HOLD_LINE);
	}
}

WRITE16_MEMBER(gaelco_state::bigkarnk_coin_w)
{
	if (ACCESSING_BITS_0_7)
	{
		switch ((offset >> 3))
		{
			case 0x00:  /* Coin Lockouts */
			case 0x01:
				coin_lockout_w(machine(), (offset >> 3) & 0x01, ~data & 0x01);
				break;
			case 0x02:  /* Coin Counters */
			case 0x03:
				coin_counter_w(machine(), (offset >> 3) & 0x01, data & 0x01);
				break;
		}
	}
}

WRITE16_MEMBER(gaelco_state::OKIM6295_bankswitch_w)
{
	UINT8 *RAM = memregion("oki")->base();

	if (ACCESSING_BITS_0_7)
	{
		memcpy(&RAM[0x30000], &RAM[0x40000 + (data & 0x0f) * 0x10000], 0x10000);
	}
}

/*********** Squash Encryption Related Code ******************/

WRITE16_MEMBER(gaelco_state::gaelco_vram_encrypted_w)
{
	// osd_printf_debug("gaelco_vram_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0f, 0x4228);
	COMBINE_DATA(&m_videoram[offset]);

	m_tilemap[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}


WRITE16_MEMBER(gaelco_state::gaelco_encrypted_w)
{
	// osd_printf_debug("gaelco_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0f, 0x4228);
	COMBINE_DATA(&m_screenram[offset]);
}

/*********** Thunder Hoop Encryption Related Code ******************/

WRITE16_MEMBER(gaelco_state::thoop_vram_encrypted_w)
{
	// osd_printf_debug("gaelco_vram_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0e, 0x4228);
	COMBINE_DATA(&m_videoram[offset]);

	m_tilemap[offset >> 11]->mark_tile_dirty(((offset << 1) & 0x0fff) >> 2);
}

WRITE16_MEMBER(gaelco_state::thoop_encrypted_w)
{
	// osd_printf_debug("gaelco_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0e, 0x4228);
	COMBINE_DATA(&m_screenram[offset]);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( bigkarnk_map, AS_PROGRAM, 16, gaelco_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(gaelco_vram_w) AM_SHARE("videoram")               /* Video RAM */
	AM_RANGE(0x102000, 0x103fff) AM_RAM                                                         /* Screen RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                         /* Video Registers */
//  AM_RANGE(0x10800c, 0x10800d) AM_WRITE(watchdog_reset_w)                                                 /* INT 6 ACK/Watchdog timer */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                               /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW1")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW2")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x700008, 0x700009) AM_READ_PORT("SERVICE")
	AM_RANGE(0x70000e, 0x70000f) AM_WRITE(bigkarnk_sound_command_w)                                     /* Triggers a FIRQ on the sound CPU */
	AM_RANGE(0x70000a, 0x70003b) AM_WRITE(bigkarnk_coin_w)                                          /* Coin Counters + Coin Lockout */
	AM_RANGE(0xff8000, 0xffffff) AM_RAM                                                         /* Work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( bigkarnk_snd_map, AS_PROGRAM, 8, gaelco_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM                                         /* RAM */
	AM_RANGE(0x0800, 0x0801) AM_DEVREADWRITE("oki", okim6295_device, read, write)   /* OKI6295 */
//  AM_RANGE(0x0900, 0x0900) AM_WRITENOP                                    /* enable sound output? */
	AM_RANGE(0x0a00, 0x0a01) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)        /* YM3812 */
	AM_RANGE(0x0b00, 0x0b00) AM_READ(soundlatch_byte_r)                         /* Sound latch */
	AM_RANGE(0x0c00, 0xffff) AM_ROM                                         /* ROM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( maniacsq_map, AS_PROGRAM, 16, gaelco_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(gaelco_vram_w) AM_SHARE("videoram")               /* Video RAM */
	AM_RANGE(0x102000, 0x103fff) AM_RAM                                                         /* Screen RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                         /* Video Registers */
//  AM_RANGE(0x10800c, 0x10800d) AM_WRITE(watchdog_reset_w)                                                 /* INT 6 ACK/Watchdog timer */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                               /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)                                        /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)                      /* OKI6295 status register */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM                                                         /* Work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( squash_map, AS_PROGRAM, 16, gaelco_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(gaelco_vram_encrypted_w) AM_SHARE("videoram")         /* Video RAM */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(gaelco_encrypted_w) AM_SHARE("screenram")                /* Screen RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                         /* Video Registers */
//  AM_RANGE(0x10800c, 0x10800d) AM_WRITE(watchdog_reset_w)                                                 /* INT 6 ACK/Watchdog timer */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")    /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                               /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)                                        /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)                      /* OKI6295 status register */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM                                                         /* Work RAM */
ADDRESS_MAP_END

static ADDRESS_MAP_START( thoop_map, AS_PROGRAM, 16, gaelco_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM                                                         /* ROM */
	AM_RANGE(0x100000, 0x101fff) AM_RAM_WRITE(thoop_vram_encrypted_w) AM_SHARE("videoram")          /* Video RAM */
	AM_RANGE(0x102000, 0x103fff) AM_RAM_WRITE(thoop_encrypted_w) AM_SHARE("screenram")             /* Screen RAM */
	AM_RANGE(0x108000, 0x108007) AM_WRITEONLY AM_SHARE("vregs")                         /* Video Registers */
//  AM_RANGE(0x10800c, 0x10800d) AM_WRITE(watchdog_reset_w)                                                     /* INT 6 ACK/Watchdog timer */
	AM_RANGE(0x200000, 0x2007ff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")        /* Palette */
	AM_RANGE(0x440000, 0x440fff) AM_RAM AM_SHARE("spriteram")                               /* Sprite RAM */
	AM_RANGE(0x700000, 0x700001) AM_READ_PORT("DSW2")
	AM_RANGE(0x700002, 0x700003) AM_READ_PORT("DSW1")
	AM_RANGE(0x700004, 0x700005) AM_READ_PORT("P1")
	AM_RANGE(0x700006, 0x700007) AM_READ_PORT("P2")
	AM_RANGE(0x70000c, 0x70000d) AM_WRITE(OKIM6295_bankswitch_w)                                        /* OKI6295 bankswitch */
	AM_RANGE(0x70000e, 0x70000f) AM_DEVREADWRITE8("oki", okim6295_device, read, write, 0x00ff)                      /* OKI6295 status register */
	AM_RANGE(0xff0000, 0xffffff) AM_RAM                                                         /* Work RAM */
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

/* Common Inputs used by all the games */
static INPUT_PORTS_START( gaelco )
	PORT_START("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x07, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0e, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0x70, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xe0, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )

	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )
INPUT_PORTS_END

static INPUT_PORTS_START( bigkarnk )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7,6")
	PORT_DIPSETTING(    0x07, "0" )
	PORT_DIPSETTING(    0x06, "1" )
	PORT_DIPSETTING(    0x05, "2" )
	PORT_DIPSETTING(    0x04, "3" )
	PORT_DIPSETTING(    0x03, "4" )
	PORT_DIPSETTING(    0x02, "5" )
	PORT_DIPSETTING(    0x01, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Impact" ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("SERVICE")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x02, 0x02, "Go to test mode now" )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0xfc, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( maniacsq )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Sound Type" ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Stereo ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Mono ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END


static INPUT_PORTS_START( biomtoy )
	PORT_INCLUDE( gaelco )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:7") /* Not Listed/shown in test mode */
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:6") /* Not Listed/shown in test mode */
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x20, "0" )
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x00, "3" )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:2,1")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
INPUT_PORTS_END

static INPUT_PORTS_START( squash )
	PORT_INCLUDE( gaelco )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6")
	PORT_DIPSETTING(    0x02, DEF_STR( 6C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 4C_3C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x38, 0x38, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:5,4,3")
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_4C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, "2 Player Continue" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, "2 Credits / 5 Games" )
	PORT_DIPSETTING(    0x00, "1 Credit / 3 Games" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Free_Play ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x0c, "Number of Faults" ) PORT_DIPLOCATION("SW2:6,5")
	PORT_DIPSETTING(    0x08, "4" )
	PORT_DIPSETTING(    0x0c, "5" )
	PORT_DIPSETTING(    0x04, "6" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:4") /* Not Listed/shown in test mode */
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2") /* Listed as "Unused" in test mode */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )

	PORT_START("UNK")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( thoop )
	PORT_INCLUDE( squash )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x40, 0x40, "2 Credits to Start, 1 to Continue" ) PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:8,7")
	PORT_DIPSETTING(    0x03, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, "Player Controls" ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x04, "2 Joysticks" )
	PORT_DIPSETTING(    0x00, "1 Joystick" )
	PORT_DIPNAME( 0x18, 0x08, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:5,4")
	PORT_DIPSETTING(    0x00, "4" )
	PORT_DIPSETTING(    0x08, "3" )
	PORT_DIPSETTING(    0x10, "2" )
	PORT_DIPSETTING(    0x18, "1" )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Cabinet ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_SERVICE_DIPLOC(  0x80, IP_ACTIVE_LOW, "SW2:1" )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

#define TILELAYOUT8(NUM) static const gfx_layout tilelayout8_##NUM =    \
{                                                                       \
	8,8,                                    /* 8x8 tiles */             \
	NUM/8,                                  /* number of tiles */       \
	4,                                      /* bitplanes */             \
	{ 0*NUM*8, 1*NUM*8, 2*NUM*8, 3*NUM*8 }, /* plane offsets */         \
	{ 0,1,2,3,4,5,6,7 },                                                \
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8 },                                \
	8*8                                                                 \
}

#define TILELAYOUT16(NUM) static const gfx_layout tilelayout16_##NUM =              \
{                                                                                   \
	16,16,                                  /* 16x16 tiles */                       \
	NUM/32,                                 /* number of tiles */                   \
	4,                                      /* bitplanes */                         \
	{ 0*NUM*8, 1*NUM*8, 2*NUM*8, 3*NUM*8 }, /* plane offsets */                     \
	{ 0,1,2,3,4,5,6,7, 16*8+0,16*8+1,16*8+2,16*8+3,16*8+4,16*8+5,16*8+6,16*8+7 },   \
	{ 0*8,1*8,2*8,3*8,4*8,5*8,6*8,7*8, 8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 },     \
	32*8                                                                            \
}

#define GFXDECODEINFO(NUM,ENTRIES) \
static GFXDECODE_START( NUM )\
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout8_##NUM,0, ENTRIES )                           \
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout16_##NUM,0,    ENTRIES )                           \
GFXDECODE_END


TILELAYOUT8(0x100000);
TILELAYOUT16(0x100000);

GFXDECODEINFO(0x100000,64)


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void gaelco_state::machine_start()
{
}

static MACHINE_CONFIG_START( bigkarnk, gaelco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 10000000)   /* MC68000P10, 10 MHz */
	MCFG_CPU_PROGRAM_MAP(bigkarnk_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco_state,  irq6_line_hold)

	MCFG_CPU_ADD("audiocpu", M6809, 8867000/4)  /* 68B09, 2.21675 MHz? */
	MCFG_CPU_PROGRAM_MAP(bigkarnk_snd_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco_state, screen_update_bigkarnk)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x100000)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,bigkarnk)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM3812, 3580000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( maniacsq, gaelco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000,24000000/2)          /* 12 MHz */
	MCFG_CPU_PROGRAM_MAP(maniacsq_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco_state,  irq6_line_hold)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco_state, screen_update_maniacsq)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x100000)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( squash, gaelco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* MC68000P12, 12 MHz */
	MCFG_CPU_PROGRAM_MAP(squash_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco_state,  irq6_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco_state, screen_update_maniacsq)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x100000)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( thoop, gaelco_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, 12000000)   /* MC68000P12, 12 MHz */
	MCFG_CPU_PROGRAM_MAP(thoop_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", gaelco_state,  irq6_line_hold)

	MCFG_QUANTUM_TIME(attotime::from_hz(600))


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*16, 32*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 320-1, 16, 256-1)
	MCFG_SCREEN_UPDATE_DRIVER(gaelco_state, screen_update_maniacsq)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", 0x100000)
	MCFG_PALETTE_ADD("palette", 1024)
	MCFG_PALETTE_FORMAT(xBBBBBGGGGGRRRRR)

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_OKIM6295_ADD("oki", 1056000, OKIM6295_PIN7_HIGH) // clock frequency & pin 7 not verified
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( bigkarnk ) /* PCB silkscreened REF.901112 */
	ROM_REGION( 0x080000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "d16",  0x000000, 0x040000, CRC(44fb9c73) SHA1(c33852b37afea15482f4a43cb045434660e7a056) )
	ROM_LOAD16_BYTE(    "d19",  0x000001, 0x040000, CRC(ff79dfdd) SHA1(2bfa440299317967ba2018d3a148291ae0c144ae) )

	ROM_REGION( 0x01e000, "audiocpu", 0 )   /* 6809 code */
	ROM_LOAD(   "d5",   0x000000, 0x010000, CRC(3b73b9c5) SHA1(1b1c5545609a695dab87d611bd53e0c3dd91e6b7) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "h5", 0x000000, 0x080000, CRC(20e239ff) SHA1(685059340f0f3a8e3c98702bd760dae685a58ddb) )
	ROM_RELOAD(     0x080000, 0x080000 )
	ROM_LOAD( "h10",0x100000, 0x080000, CRC(ab442855) SHA1(bcd69d4908ff8dc1b2215d2c2d2e54b950e0c015) )
	ROM_RELOAD(     0x180000, 0x080000 )
	ROM_LOAD( "h8", 0x200000, 0x080000, CRC(83dce5a3) SHA1(b4f9473e93c96f4b86c446e89d13fd3ef2b03996) )
	ROM_RELOAD(     0x280000, 0x080000 )
	ROM_LOAD( "h6", 0x300000, 0x080000, CRC(24e84b24) SHA1(c0ad6ce1e4b8aa7b9c9a3db8bb0165e90f4b48ed) )
	ROM_RELOAD(     0x380000, 0x080000 )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "d1", 0x000000, 0x040000, CRC(26444ad1) SHA1(804101b9bbb6e1b6d43a1e9d91737f9c3b27802a) )
ROM_END

ROM_START( maniacsp ) /* PCB - REF 922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "d18",  0x000000, 0x020000, CRC(740ecab2) SHA1(8d8583364cc6aeea58ea2b9cb9a2aab2a43a44df) )
	ROM_LOAD16_BYTE(    "d16",  0x000001, 0x020000, CRC(c6c42729) SHA1(1aac9f93d47a4eb57e06e206e9f50e349b1817da) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "f3", 0x000000, 0x040000, CRC(e7f6582b) SHA1(9e352edf2f71d0edecb54a11ab3fd0e3ec867d42) )
	ROM_RELOAD(     0x080000, 0x040000 )
	/* 0x040000-0x07ffff and 0x0c0000-0x0fffff empty */
	ROM_LOAD( "f2", 0x100000, 0x040000, CRC(ca43a5ae) SHA1(8d2ed537be1dee60096a58b68b735fb50cab3285) )
	ROM_RELOAD(     0x180000, 0x040000 )
	/* 0x140000-0x17ffff and 0x1c0000-0x1fffff empty */
	ROM_LOAD( "f1", 0x200000, 0x040000, CRC(fca112e8) SHA1(2a1412f8f1c856b18b6cc7794191d327a415266f) )
	ROM_RELOAD(     0x280000, 0x040000 )
	/* 0x240000-0x27ffff and 0x2c0000-0x2fffff empty */
	ROM_LOAD( "f0", 0x300000, 0x040000, CRC(6e829ee8) SHA1(b602da8d987c1bafa41baf5d5e5d753e29ff5403) )
	ROM_RELOAD(     0x380000, 0x040000 )
	/* 0x340000-0x37ffff and 0x3c0000-0x3fffff empty */

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(2557f2d6) SHA1(3a99388f2d845281f73a427d6dc797dce87b2f82) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_RELOAD(     0x0c0000, 0x080000 )
ROM_END


ROM_START( biomtoy ) /* PCB - REF 922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "d18",  0x000000, 0x080000, CRC(4569ce64) SHA1(96557aca55779c23f7c2c11fddc618823c04ead0) ) /* v1.0.1885 */
	ROM_LOAD16_BYTE(    "d16",  0x000001, 0x080000, CRC(739449bd) SHA1(711a8ea5081f15dea6067577516c9296239c4145) ) /* v1.0.1885 */

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* weird gfx ordering */
	ROM_LOAD( "h6",     0x040000, 0x040000, CRC(9416a729) SHA1(425149b3041554579791fc23c09fda6be054e89d) )
	ROM_CONTINUE(       0x0c0000, 0x040000 )
	ROM_LOAD( "j6",     0x000000, 0x040000, CRC(e923728b) SHA1(113eac1de73c74ef7c9d3e2e72599a1ff775176d) )
	ROM_CONTINUE(       0x080000, 0x040000 )
	ROM_LOAD( "h7",     0x140000, 0x040000, CRC(9c984d7b) SHA1(98d43a9c3fa93c9ea55f41475ecab6ca25713087) )
	ROM_CONTINUE(       0x1c0000, 0x040000 )
	ROM_LOAD( "j7",     0x100000, 0x040000, CRC(0e18fac2) SHA1(acb0a3699395a6c68cacdeadda42a785aa4020f5) )
	ROM_CONTINUE(       0x180000, 0x040000 )
	ROM_LOAD( "h9",     0x240000, 0x040000, CRC(8c1f6718) SHA1(9377e838ebb1e16d24072b9b4ed278408d7a808f) )
	ROM_CONTINUE(       0x2c0000, 0x040000 )
	ROM_LOAD( "j9",     0x200000, 0x040000, CRC(1c93f050) SHA1(fabeffa05dae7a83a199a57022bd318d6ad02c4d) )
	ROM_CONTINUE(       0x280000, 0x040000 )
	ROM_LOAD( "h10",    0x340000, 0x040000, CRC(aca1702b) SHA1(6b36b230722270dbfc2f69bd7eb07b9e718db089) )
	ROM_CONTINUE(       0x3c0000, 0x040000 )
	ROM_LOAD( "j10",    0x300000, 0x040000, CRC(8e3e96cc) SHA1(761009f3f32b18139e98f20a22c433b6a49d9168) )
	ROM_CONTINUE(       0x380000, 0x040000 )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(0f02de7e) SHA1(a8779370cc36290616794ff11eb3eebfdea5b1a9) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_LOAD( "c3", 0x0c0000, 0x080000, CRC(914e4bbc) SHA1(ca82b7481621a119f05992ed093b963da70d748a) )
ROM_END


ROM_START( biomtoya ) /* PCB - REF 922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "biomtoya.d18", 0x000000, 0x080000, CRC(39b6cdbd) SHA1(3a22eb2e304d85ecafff677d83c3c4fca3f869d5) ) /* v1.0.1884 */
	ROM_LOAD16_BYTE( "biomtoya.d16", 0x000001, 0x080000, CRC(ab340671) SHA1(83f708a535048e927fd1c7de85a65282e460f98a) ) /* v1.0.1884 */

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* weird gfx ordering */
	ROM_LOAD( "h6",     0x040000, 0x040000, CRC(9416a729) SHA1(425149b3041554579791fc23c09fda6be054e89d) )
	ROM_CONTINUE(       0x0c0000, 0x040000 )
	ROM_LOAD( "j6",     0x000000, 0x040000, CRC(e923728b) SHA1(113eac1de73c74ef7c9d3e2e72599a1ff775176d) )
	ROM_CONTINUE(       0x080000, 0x040000 )
	ROM_LOAD( "h7",     0x140000, 0x040000, CRC(9c984d7b) SHA1(98d43a9c3fa93c9ea55f41475ecab6ca25713087) )
	ROM_CONTINUE(       0x1c0000, 0x040000 )
	ROM_LOAD( "j7",     0x100000, 0x040000, CRC(0e18fac2) SHA1(acb0a3699395a6c68cacdeadda42a785aa4020f5) )
	ROM_CONTINUE(       0x180000, 0x040000 )
	ROM_LOAD( "h9",     0x240000, 0x040000, CRC(8c1f6718) SHA1(9377e838ebb1e16d24072b9b4ed278408d7a808f) )
	ROM_CONTINUE(       0x2c0000, 0x040000 )
	ROM_LOAD( "j9",     0x200000, 0x040000, CRC(1c93f050) SHA1(fabeffa05dae7a83a199a57022bd318d6ad02c4d) )
	ROM_CONTINUE(       0x280000, 0x040000 )
	ROM_LOAD( "h10",    0x340000, 0x040000, CRC(aca1702b) SHA1(6b36b230722270dbfc2f69bd7eb07b9e718db089) )
	ROM_CONTINUE(       0x3c0000, 0x040000 )
	ROM_LOAD( "j10",    0x300000, 0x040000, CRC(8e3e96cc) SHA1(761009f3f32b18139e98f20a22c433b6a49d9168) )
	ROM_CONTINUE(       0x380000, 0x040000 )

	// using the same samples as the parent set causes bad sounds for most of the game
	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, BAD_DUMP CRC(0f02de7e) SHA1(a8779370cc36290616794ff11eb3eebfdea5b1a9) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_LOAD( "c3", 0x0c0000, 0x080000, BAD_DUMP CRC(914e4bbc) SHA1(ca82b7481621a119f05992ed093b963da70d748a) )
ROM_END


/*
Squash (version 1.0)
Gaelco, 1992

PCB Layout
----------

REF 922804/1
|---------------------------------------------|
|   LM358  SOUND.1D    26MHz     6116         |
|   VOL                PAL       6116         |
|          M6295                 6116         |
|  1MHz            |-------|     6116         |
|                  |ACTEL  |                  |
|J                 |A1020A |    C12.6H        |
|A         6116    |PL84C  |    C11.7H        |
|M                 |-------|    C10.8H        |
|M         6116                 C09.10H       |
|A                 |-------|     PAL          |
|                  |ACTEL  |    6264          |
|                  |A1020A |    6264          |
|         D16.E16  |PL84C  |             PAL  |
| SW1     62256    |-------|                  |
|    68000                                    |
| SW2     D18.E18               6116          |
|20MHz    62256                 6116          |
|         PAL                                 |
|---------------------------------------------|
Notes:
      68000 CPU running at 10.000MHz
      OKI M6295 running at 1.000MHz. Sample Rate = 1000000 / 132
      62256 - 32k x8 SRAM (x2, DIP28)
      6264  - 8k x8 SRAM  (x2, DIP28)
      6116  - 2k x8 SRAM  (x8, DIP24)
      VSync - 58Hz

      ROMs:
           SQUASH_D16.E16    27C010   \
           SQUASH_D18.E18    27C010   /  68K Program

           SQUASH_C09.10H    27C040   \
           SQUASH_C10.8H     27C040   |
           SQUASH_C11.7H     27C040   |  GFX
           SQUASH_C12.6H     27C040   /

           SQUASH_SOUND.1D   27C040      Sound
*/

ROM_START( squash )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "squash.d18", 0x000000, 0x20000, CRC(ce7aae96) SHA1(4fe8666ae571bffc5a08fa68346c0623282989eb) )
	ROM_LOAD16_BYTE( "squash.d16", 0x000001, 0x20000, CRC(8ffaedd7) SHA1(f4aada17ba67dd8b6c5a395e832bcbba2764c59d) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "squash.c09", 0x300000, 0x80000, CRC(0bb91c69) SHA1(8be945049ab411a4d49bd64bd3937542ec9ef9fb) ) /* encrypted video ram */
	ROM_RELOAD(               0x380000, 0x80000 )
	ROM_LOAD( "squash.c10", 0x200000, 0x80000, CRC(892a035c) SHA1(d0156ceb9aa6639a1124c17fb12389be319bb51f) ) /* encrypted video ram */
	ROM_RELOAD(               0x280000, 0x80000 )
	ROM_LOAD( "squash.c11", 0x100000, 0x80000, CRC(9e19694d) SHA1(1df4646f3147719fef516a37aa361ae26d9b23a2) ) /* encrypted video ram */
	ROM_RELOAD(               0x180000, 0x80000 )
	ROM_LOAD( "squash.c12", 0x000000, 0x80000, CRC(5c440645) SHA1(4f2fc1647ffc549fa079f2dc0aaaceb447afdf44) ) /* encrypted video ram */
	ROM_RELOAD(               0x080000, 0x80000 )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "squash.d01",   0x000000, 0x80000, CRC(a1b9651b) SHA1(a396ba94889f70ea06d6330e3606b0f2497ff6ce) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(     0x040000, 0x080000 )
	ROM_RELOAD(     0x0c0000, 0x080000 )
ROM_END


ROM_START( thoop )
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "th18dea1.040", 0x000000, 0x80000, CRC(59bad625) SHA1(28e058b2290bc5f7130b801014d026432f9e7fd5) )
	ROM_LOAD16_BYTE( "th161eb4.020", 0x000001, 0x40000, CRC(6add61ed) SHA1(0e789d9a0ac19b6143044fbc04ab2227735b2a8f) )

	ROM_REGION( 0x400000, "gfx1", 0 )
	ROM_LOAD( "c09", 0x300000, 0x040000, CRC(06f0edbf) SHA1(3cf2e5c29cd00b43d49a106084076f2ac0dbad98) ) /* encrypted video ram */
	ROM_CONTINUE(    0x380000, 0x040000 )
	ROM_CONTINUE(    0x340000, 0x040000 )
	ROM_CONTINUE(    0x3c0000, 0x040000 )
	ROM_LOAD( "c10", 0x200000, 0x040000, CRC(2d227085) SHA1(b224efd59ec83bb786fa92a23ef2d27ed36cab6c) ) /* encrypted video ram */
	ROM_CONTINUE(    0x280000, 0x040000 )
	ROM_CONTINUE(    0x240000, 0x040000 )
	ROM_CONTINUE(    0x2c0000, 0x040000 )
	ROM_LOAD( "c11", 0x100000, 0x040000, CRC(7403ef7e) SHA1(52a737816e25a07ada070ed3a5f40bbbd22ac8e0) ) /* encrypted video ram */
	ROM_CONTINUE(    0x180000, 0x040000 )
	ROM_CONTINUE(    0x140000, 0x040000 )
	ROM_CONTINUE(    0x1c0000, 0x040000 )
	ROM_LOAD( "c12", 0x000000, 0x040000, CRC(29a5ca36) SHA1(fdcfdefb3b02bfe34781fdd0295640caabe2a5fb) ) /* encrypted video ram */
	ROM_CONTINUE(    0x080000, 0x040000 )
	ROM_CONTINUE(    0x040000, 0x040000 )
	ROM_CONTINUE(    0x0c0000, 0x040000 )

	ROM_REGION( 0x140000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "sound", 0x000000, 0x100000, CRC(99f80961) SHA1(de3a514a8f46dffd5f762e52aac1f4c3b08e2e18) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_RELOAD(        0x040000, 0x100000 )
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, bigkarnk, 0,        bigkarnk, bigkarnk, driver_device, 0, ROT0, "Gaelco", "Big Karnak", MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoy,  0,        maniacsq, biomtoy, driver_device,  0, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1885)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoya, biomtoy,  maniacsq, biomtoy, driver_device,  0, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1884)", MACHINE_SUPPORTS_SAVE )
GAME( 1996, maniacsp, maniacsq, maniacsq, maniacsq, driver_device, 0, ROT0, "Gaelco", "Maniac Square (prototype)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, squash,   0,        squash,   squash, driver_device,   0, ROT0, "Gaelco", "Squash (Ver. 1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, thoop,    0,        thoop,    thoop, driver_device,    0, ROT0, "Gaelco", "Thunder Hoop (Ver. 1)", MACHINE_SUPPORTS_SAVE )
