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
#include "includes/gaelco.h"

#include "includes/gaelcrpt.h"

#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"
#include "sound/okim6295.h"
#include "sound/3812intf.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

WRITE_LINE_MEMBER(gaelco_state::coin1_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(0, state);
}

WRITE_LINE_MEMBER(gaelco_state::coin2_lockout_w)
{
	machine().bookkeeping().coin_lockout_w(1, state);
}

WRITE_LINE_MEMBER(gaelco_state::coin1_counter_w)
{
	machine().bookkeeping().coin_counter_w(0, state);
}

WRITE_LINE_MEMBER(gaelco_state::coin2_counter_w)
{
	machine().bookkeeping().coin_counter_w(1, state);
}

WRITE8_MEMBER(gaelco_state::oki_bankswitch_w)
{
	m_okibank->set_entry(data & 0x0f);
}

/*********** Squash Encryption Related Code ******************/

WRITE16_MEMBER(gaelco_state::vram_encrypted_w)
{
	// osd_printf_debug("vram_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0f, 0x4228);
	vram_w(offset, data, mem_mask);
}


WRITE16_MEMBER(gaelco_state::encrypted_w)
{
	// osd_printf_debug("encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0f, 0x4228);
	COMBINE_DATA(&m_screenram[offset]);
}

/*********** Thunder Hoop Encryption Related Code ******************/

WRITE16_MEMBER(gaelco_state::thoop_vram_encrypted_w)
{
	// osd_printf_debug("vram_encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0e, 0x4228);
	vram_w(offset, data, mem_mask);
}

WRITE16_MEMBER(gaelco_state::thoop_encrypted_w)
{
	// osd_printf_debug("encrypted_w!!\n");
	data = gaelco_decrypt(space, offset, data, 0x0e, 0x4228);
	COMBINE_DATA(&m_screenram[offset]);
}

/*************************************
 *
 *  Address maps
 *
 *************************************/

void gaelco_state::bigkarnk_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();                                                         /* ROM */
	map(0x100000, 0x101fff).ram().w(FUNC(gaelco_state::vram_w)).share("videoram");               /* Video RAM */
	map(0x102000, 0x103fff).ram();                                                         /* Screen RAM */
	map(0x108000, 0x108007).writeonly().share("vregs");                         /* Video Registers */
//  map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w)); /* INT 6 ACK/Watchdog timer */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    /* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");                               /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW1");
	map(0x700002, 0x700003).portr("DSW2");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x700008, 0x700009).portr("SERVICE");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000f, 0x70000f).w(m_soundlatch, FUNC(generic_latch_8_device::write));               /* Triggers a FIRQ on the sound CPU */
	map(0xff8000, 0xffffff).ram();                                                         /* Work RAM */
}

void gaelco_state::bigkarnk_snd_map(address_map &map)
{
	map(0x0000, 0x07ff).ram();                                         /* RAM */
	map(0x0800, 0x0801).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));   /* OKI6295 */
//  map(0x0900, 0x0900).nopw();                                    /* enable sound output? */
	map(0x0a00, 0x0a01).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));        /* YM3812 */
	map(0x0b00, 0x0b00).r(m_soundlatch, FUNC(generic_latch_8_device::read));      /* Sound latch */
	map(0x0c00, 0xffff).rom();                                         /* ROM */
}

void gaelco_state::maniacsq_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         /* ROM */
	map(0x100000, 0x101fff).ram().w(FUNC(gaelco_state::vram_w)).share("videoram");               /* Video RAM */
	map(0x102000, 0x103fff).ram();                                                         /* Screen RAM */
	map(0x108000, 0x108007).writeonly().share("vregs");                         /* Video Registers */
//  map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w));  /* INT 6 ACK/Watchdog timer */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    /* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");                               /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000d, 0x70000d).w(FUNC(gaelco_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));                      /* OKI6295 status register */
	map(0xff0000, 0xffffff).ram();                                                         /* Work RAM */
}

void gaelco_state::squash_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         /* ROM */
	map(0x100000, 0x101fff).ram().w(FUNC(gaelco_state::vram_encrypted_w)).share("videoram");         /* Video RAM */
	map(0x102000, 0x103fff).ram().w(FUNC(gaelco_state::encrypted_w)).share("screenram");                /* Screen RAM */
	map(0x108000, 0x108007).writeonly().share("vregs");                         /* Video Registers */
//  map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w)); /* INT 6 ACK/Watchdog timer */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");    /* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");                               /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(gaelco_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));                      /* OKI6295 status register */
	map(0xff0000, 0xffffff).ram();                                                         /* Work RAM */
}

void gaelco_state::thoop_map(address_map &map)
{
	map(0x000000, 0x0fffff).rom();                                                         /* ROM */
	map(0x100000, 0x101fff).ram().w(FUNC(gaelco_state::thoop_vram_encrypted_w)).share("videoram");          /* Video RAM */
	map(0x102000, 0x103fff).ram().w(FUNC(gaelco_state::thoop_encrypted_w)).share("screenram");             /* Screen RAM */
	map(0x108000, 0x108007).writeonly().share("vregs");                         /* Video Registers */
//  map(0x10800c, 0x10800d).w("watchdog", FUNC(watchdog_timer_device::reset16_w));  /* INT 6 ACK/Watchdog timer */
	map(0x200000, 0x2007ff).ram().w(m_palette, FUNC(palette_device::write16)).share("palette");        /* Palette */
	map(0x440000, 0x440fff).ram().share("spriteram");                               /* Sprite RAM */
	map(0x700000, 0x700001).portr("DSW2");
	map(0x700002, 0x700003).portr("DSW1");
	map(0x700004, 0x700005).portr("P1");
	map(0x700006, 0x700007).portr("P2");
	map(0x70000b, 0x70000b).select(0x000070).lw8(NAME([this] (offs_t offset, u8 data) { m_outlatch->write_d0(offset >> 4, data); }));
	map(0x70000d, 0x70000d).w(FUNC(gaelco_state::oki_bankswitch_w));
	map(0x70000f, 0x70000f).rw("oki", FUNC(okim6295_device::read), FUNC(okim6295_device::write));                      /* OKI6295 status register */
	map(0xff0000, 0xffffff).ram();                                                         /* Work RAM */
}


void gaelco_state::oki_map(address_map &map)
{
	map(0x00000, 0x2ffff).rom();
	map(0x30000, 0x3ffff).bankr("okibank");
}


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
	PORT_DIPNAME( 0x30, 0x30, "Lives in Reserve" ) PORT_DIPLOCATION("SW2:4,3") // Test mode doesn't show the value of the lives given, but of the ones after you die the first time
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

static INPUT_PORTS_START( biomtoyc )
	PORT_INCLUDE( biomtoy )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x0c, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0xc0, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )
INPUT_PORTS_END

static INPUT_PORTS_START( bioplayc )
	PORT_INCLUDE( biomtoy )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x0f, 0x0f, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x0e, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0x0a, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x09, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0f, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0b, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x0d, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x0c, "Free Play (if Coin A too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin A too)" )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:4,3,2,1")
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xd0, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x80, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0xc0, "Free Play (if Coin B too)" )
	PORT_DIPSETTING(    0x00, "Free Play (if Coin B too)" )

	PORT_MODIFY("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:8" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) PORT_DIPLOCATION("SW2:5") /* Not Listed/shown in test mode */
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) ) PORT_DIPLOCATION("SW2:4,3")
	PORT_DIPSETTING(    0x10, "1" )
	PORT_DIPSETTING(    0x30, "2" )
	PORT_DIPSETTING(    0x20, "3" )
	PORT_DIPSETTING(    0x00, "4" )
INPUT_PORTS_END


static INPUT_PORTS_START( lastkm )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 ) // Pedal
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) // Gear Up
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) // Gear Down
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("P2")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xf0, 0xf0, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SW1:8,7,6,5")
	PORT_DIPSETTING(    0x70, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(    0xd0, "Free Play (duplicate 1)" )
	PORT_DIPSETTING(    0x00, "Free Play (duplicate 2)" )
	PORT_DIPSETTING(    0xe0, DEF_STR( 4C_1C ) )
	PORT_DIPSETTING(    0xb0, DEF_STR( 3C_2C ) )
	PORT_DIPSETTING(    0x90, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xf0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0xa0, DEF_STR( 1C_6C ) )

	PORT_START("DSW2")
	PORT_SERVICE_DIPLOC(  0x01, IP_ACTIVE_LOW, "SW2:1" )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unused ) ) PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Master" ) PORT_DIPLOCATION("SW2:8") // presumably has a link feature?
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
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

static const gfx_layout tilelayout8 =
{
	8,8,                                                /* 8x8 tiles */
	RGN_FRAC(1,4),                                      /* number of tiles */
	4,                                                  /* bitplanes */
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, /* plane offsets */
	{ STEP8(0,1) },
	{ STEP8(0,8) },
	8*8
};

static const gfx_layout tilelayout16 =
{
	16,16,                                              /* 16x16 tiles */
	RGN_FRAC(1,4),                                      /* number of tiles */
	4,                                                  /* bitplanes */
	{ 0, RGN_FRAC(1,4), RGN_FRAC(2,4), RGN_FRAC(3,4) }, /* plane offsets */
	{ STEP8(0,1), STEP8(16*8,1) },
	{ STEP16(0,8) },
	32*8
};

static GFXDECODE_START( gfx_gaelco )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout8,  0, 64 )
	GFXDECODE_ENTRY( "gfx1", 0x000000, tilelayout16, 0, 64 )
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void gaelco_state::machine_start()
{
	if (m_okibank) //bigkarnk oki isn't banked
		m_okibank->configure_entries(0, 16, memregion("oki")->base(), 0x10000);
}

// TODO: verify all clocks (XTALs are 8.0MHz & 24.000 MHz) - One PCB reported to have 8867.23 kHz instead of 8MHz
void gaelco_state::bigkarnk(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2);   /* MC68000P10, 12 MHz? */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco_state::bigkarnk_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco_state::irq6_line_hold));

	MC6809E(config, m_audiocpu, XTAL(8'000'000)/4);  /* 68B09EP, 2 MHz? */
	m_audiocpu->set_addrmap(AS_PROGRAM, &gaelco_state::bigkarnk_snd_map);

	config.set_maximum_quantum(attotime::from_hz(600));

	LS259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set(FUNC(gaelco_state::coin1_lockout_w)).invert();
	m_outlatch->q_out_cb<1>().set(FUNC(gaelco_state::coin2_lockout_w)).invert();
	m_outlatch->q_out_cb<2>().set(FUNC(gaelco_state::coin1_counter_w));
	m_outlatch->q_out_cb<3>().set(FUNC(gaelco_state::coin2_counter_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco_state::screen_update_bigkarnk));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,bigkarnk)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	YM3812(config, "ymsnd", XTAL(8'000'000)/2).add_route(ALL_OUTPUTS, "mono", 1.0); // 4 MHz matches PCB recording

	OKIM6295(config, "oki", XTAL(8'000'000)/8, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0); // clock frequency & pin 7 not verified
}

void gaelco_state::maniacsq(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco_state::maniacsq_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco_state::irq6_line_hold));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco_state::screen_update_maniacsq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // pin 7 not verified
	oki.set_addrmap(0, &gaelco_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gaelco_state::squash(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(20'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco_state::squash_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco_state::irq6_line_hold));

	config.set_maximum_quantum(attotime::from_hz(600));

	LS259(config, m_outlatch); // B8
	m_outlatch->q_out_cb<0>().set(FUNC(gaelco_state::coin1_lockout_w)).invert();
	m_outlatch->q_out_cb<1>().set(FUNC(gaelco_state::coin2_lockout_w)).invert();
	m_outlatch->q_out_cb<2>().set(FUNC(gaelco_state::coin1_counter_w));
	m_outlatch->q_out_cb<3>().set(FUNC(gaelco_state::coin2_counter_w));
	m_outlatch->q_out_cb<4>().set_nop(); // used

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(58);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco_state::screen_update_maniacsq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); /* verified on pcb */
	oki.set_addrmap(0, &gaelco_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}

void gaelco_state::thoop(machine_config &config)
{
	/* basic machine hardware */
	M68000(config, m_maincpu, XTAL(24'000'000)/2); /* verified on pcb */
	m_maincpu->set_addrmap(AS_PROGRAM, &gaelco_state::thoop_map);
	m_maincpu->set_vblank_int("screen", FUNC(gaelco_state::irq6_line_hold));

	config.set_maximum_quantum(attotime::from_hz(600));

	LS259(config, m_outlatch); // B8
	m_outlatch->q_out_cb<0>().set(FUNC(gaelco_state::coin1_lockout_w)); // not inverted
	m_outlatch->q_out_cb<1>().set(FUNC(gaelco_state::coin2_lockout_w)); // not inverted
	m_outlatch->q_out_cb<2>().set(FUNC(gaelco_state::coin1_counter_w));
	m_outlatch->q_out_cb<3>().set(FUNC(gaelco_state::coin2_counter_w));
	m_outlatch->q_out_cb<4>().set_nop(); // used

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500) /* not accurate */);
	screen.set_size(32*16, 32*16);
	screen.set_visarea(0, 320-1, 16, 256-1);
	screen.set_screen_update(FUNC(gaelco_state::screen_update_maniacsq));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_gaelco);
	PALETTE(config, m_palette).set_format(palette_device::xBGR_555, 1024);

	MCFG_VIDEO_START_OVERRIDE(gaelco_state,maniacsq)

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	okim6295_device &oki(OKIM6295(config, "oki", XTAL(1'000'000), okim6295_device::PIN7_HIGH)); // pin 7 not verified
	oki.set_addrmap(0, &gaelco_state::oki_map);
	oki.add_route(ALL_OUTPUTS, "mono", 1.0);
}


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

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "h5", 0x000000, 0x080000, CRC(20e239ff) SHA1(685059340f0f3a8e3c98702bd760dae685a58ddb) )
	ROM_LOAD( "h10",0x080000, 0x080000, CRC(ab442855) SHA1(bcd69d4908ff8dc1b2215d2c2d2e54b950e0c015) )
	ROM_LOAD( "h8", 0x100000, 0x080000, CRC(83dce5a3) SHA1(b4f9473e93c96f4b86c446e89d13fd3ef2b03996) )
	ROM_LOAD( "h6", 0x180000, 0x080000, CRC(24e84b24) SHA1(c0ad6ce1e4b8aa7b9c9a3db8bb0165e90f4b48ed) )

	ROM_REGION( 0x040000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "d1", 0x000000, 0x040000, CRC(26444ad1) SHA1(804101b9bbb6e1b6d43a1e9d91737f9c3b27802a) )
ROM_END

ROM_START( maniacsp ) /* PCB - REF 922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "d18",  0x000000, 0x020000, CRC(740ecab2) SHA1(8d8583364cc6aeea58ea2b9cb9a2aab2a43a44df) )
	ROM_LOAD16_BYTE(    "d16",  0x000001, 0x020000, CRC(c6c42729) SHA1(1aac9f93d47a4eb57e06e206e9f50e349b1817da) )

	ROM_REGION( 0x200000, "gfx1", ROMREGION_ERASE00 )
	ROM_LOAD( "f3", 0x000000, 0x040000, CRC(e7f6582b) SHA1(9e352edf2f71d0edecb54a11ab3fd0e3ec867d42) )
	/* 0x040000-0x07ffff empty */
	ROM_LOAD( "f2", 0x080000, 0x040000, CRC(ca43a5ae) SHA1(8d2ed537be1dee60096a58b68b735fb50cab3285) )
	/* 0x0c0000-0x0fffff empty */
	ROM_LOAD( "f1", 0x100000, 0x040000, CRC(fca112e8) SHA1(2a1412f8f1c856b18b6cc7794191d327a415266f) )
	/* 0x140000-0x17ffff empty */
	ROM_LOAD( "f0", 0x180000, 0x040000, CRC(6e829ee8) SHA1(b602da8d987c1bafa41baf5d5e5d753e29ff5403) )
	/* 0x1c0000-0x1fffff empty */

	ROM_REGION( 0x080000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(2557f2d6) SHA1(3a99388f2d845281f73a427d6dc797dce87b2f82) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END


/***************************************************************************

Biomechanical Toy

REF.922804/2
+-------------------------------------------------+
| 2     6      PAL                     6116       |
| 4     8    65728                     6116       |
| M SW1 0   18.D18                                |
| H     0    65728                                |
| z SW2 0   16.D16                                |
|                   +----------+                  |
|J                  |TMS       |        65764     |
|A                  |TPC1020AFN|        65764     |
|M      65764       |   -084C  |          PAL     |
|M      65764       +----------+           H10 J10|
|A                  +----------+           H9 J9  |
|                   |TMS       |           H7 J7  |
|                   |TPC1020AFN|           H6 J6  |
|  1MHz  M6295      |   -084C  |           65728  |
|                   +----------+           65728  |
|  VR1   C3     PAL                        65728  |
|        C1    26MHz                       65728  |
+-------------------------------------------------+

  CPU: MC68000P12
Sound: OKI M6295
Video: TMS TCP1020AFN-084C (x2)
  OSC: 26MHz, 24MHz & 1MHz resonator
  RAM: MHS HM3-65756K-5  32K x 8 SRAM (x2)
       MHS HM3-65728B-5  2K x 8 SRAM (x6)
  PAL: TI F20L8-25CNT DIP24 (x3)
  VR1: Volume pot
   SW: Two 8 switch dipswitches

NOTE: Sadly Gaelco didn't differentiate between versions on the ROM labels.
      Just a version number on the initial boot up screen and ROM checksum
      on the calibration screen which followed.

***************************************************************************/


ROM_START( biomtoy ) /* PCB - REF.922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "18.d18",  0x000000, 0x080000, CRC(4569ce64) SHA1(96557aca55779c23f7c2c11fddc618823c04ead0) ) /* v1.0.1885 */
	ROM_LOAD16_BYTE( "16.d16",  0x000001, 0x080000, CRC(739449bd) SHA1(711a8ea5081f15dea6067577516c9296239c4145) ) /* v1.0.1885 */

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

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(0f02de7e) SHA1(a8779370cc36290616794ff11eb3eebfdea5b1a9) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(914e4bbc) SHA1(ca82b7481621a119f05992ed093b963da70d748a) )
ROM_END


ROM_START( biomtoya ) /* PCB - REF.922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "18.d18", 0x000000, 0x080000, CRC(39b6cdbd) SHA1(3a22eb2e304d85ecafff677d83c3c4fca3f869d5) ) /* v1.0.1884 - sldh */
	ROM_LOAD16_BYTE( "16.d16", 0x000001, 0x080000, CRC(ab340671) SHA1(83f708a535048e927fd1c7de85a65282e460f98a) ) /* v1.0.1884 - sldh */

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

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END


ROM_START( biomtoyb ) /* PCB - REF.922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "18.d18", 0x000000, 0x080000, CRC(2dfadee3) SHA1(55ab563a9a69da940ca015f292476068cf21b01c) ) /* v1.0.1878 - sldh */
	ROM_LOAD16_BYTE( "16.d16", 0x000001, 0x080000, CRC(b35e3ca6) SHA1(b323fcca99d088e6fbf6a1d660ef860987af77e4) ) /* v1.0.1878 - sldh */

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Graphics & Sound ROMs soldered in, not verified 100% correct for this set */
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

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END


ROM_START( biomtoyc ) /* PCB - REF.922804/1 & REF.922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "18.d18", 0x000000, 0x080000, CRC(05ad7d30) SHA1(4b2596d225bf9b314db5a150921d7d6c99096ddb) ) /* v1.0.1870 - sldh */
	ROM_LOAD16_BYTE( "16.d16", 0x000001, 0x080000, CRC(a288e73f) SHA1(13a53981e3fe6961494013e7466badae56481958) ) /* v1.0.1870 - sldh */

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Graphics & Sound ROMs soldered in, not verified 100% correct for this set Using prototype graphics from the set below */
	/* weird gfx ordering */
	ROM_LOAD( "toy-high-3.h6",  0x040000, 0x040000, CRC(ab19a1ce) SHA1(3cc896f8c20f692b02d43db8c30f410bd93fe3ca))
	ROM_CONTINUE(               0x0c0000, 0x040000 )
	ROM_LOAD( "toy-low-3.j6",   0x000000, 0x040000, CRC(927f5cd7) SHA1(ad5e75091146ca7935a18e5dd045410e28d8b170) )
	ROM_CONTINUE(               0x080000, 0x040000 )
	ROM_LOAD( "toy-high-2.h7",  0x140000, 0x040000, CRC(fd975d89) SHA1(89bb85ccb1ba0bb82f393ef27757c0778dd696b3) )
	ROM_CONTINUE(               0x1c0000, 0x040000 )
	ROM_LOAD( "toy-low-2.j7",   0x100000, 0x040000, CRC(6cbf9937) SHA1(77123a8afea3108df54f45033dfb7f86c1d0d1b8) )
	ROM_CONTINUE(               0x180000, 0x040000 )
	ROM_LOAD( "toy-high-1.h9",  0x240000, 0x040000, CRC(09de4799) SHA1(120b7bd8e20288c3aec62d3b2bf3f87e251c3eea) )
	ROM_CONTINUE(               0x2c0000, 0x040000 )
	ROM_LOAD( "toy-low-1.j9",   0x200000, 0x040000, CRC(57922c41) SHA1(ffbe5b418ed93e8705a7aabe69d3fad2919a160f) )
	ROM_CONTINUE(               0x280000, 0x040000 )
	ROM_LOAD( "toy-high-0.h10", 0x340000, 0x040000, CRC(5bee6df7) SHA1(ecf759de2f0909f793c84c71feb08801896e2474) )
	ROM_CONTINUE(               0x3c0000, 0x040000 )
	ROM_LOAD( "toy-low-0.j10",  0x300000, 0x040000, CRC(26c49ca2) SHA1(82079eaa2c9523c9acb72fccfbbe9493bc62e84f) )
	ROM_CONTINUE(               0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "c1", 0x000000, 0x080000, CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "c3", 0x080000, 0x080000, CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END

ROM_START( bioplayc ) /* PCB - REF.922804/2??  -  Spanish version */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "t.d18",  0x000000, 0x080000, CRC(ec518c6c) SHA1(8b96313582d252bebb4bcce8f2d993f751ad0a74) ) /* v1.0.1823 */
	ROM_LOAD16_BYTE( "t.d16",  0x000001, 0x080000, CRC(de4b031d) SHA1(d4bcdfedab1d48df0c48ffc775731a4981342c7a) ) /* v1.0.1823 */

	ROM_REGION( 0x400000, "gfx1", 0 )
	/* weird gfx ordering */
	ROM_LOAD( "toy-high-3.h6",  0x040000, 0x040000, CRC(ab19a1ce) SHA1(3cc896f8c20f692b02d43db8c30f410bd93fe3ca))
	ROM_CONTINUE(               0x0c0000, 0x040000 )
	ROM_LOAD( "toy-low-3.j6",   0x000000, 0x040000, CRC(927f5cd7) SHA1(ad5e75091146ca7935a18e5dd045410e28d8b170) )
	ROM_CONTINUE(               0x080000, 0x040000 )
	ROM_LOAD( "toy-high-2.h7",  0x140000, 0x040000, CRC(fd975d89) SHA1(89bb85ccb1ba0bb82f393ef27757c0778dd696b3) )
	ROM_CONTINUE(               0x1c0000, 0x040000 )
	ROM_LOAD( "toy-low-2.j7",   0x100000, 0x040000, CRC(6cbf9937) SHA1(77123a8afea3108df54f45033dfb7f86c1d0d1b8) )
	ROM_CONTINUE(               0x180000, 0x040000 )
	ROM_LOAD( "toy-high-1.h9",  0x240000, 0x040000, CRC(09de4799) SHA1(120b7bd8e20288c3aec62d3b2bf3f87e251c3eea) )
	ROM_CONTINUE(               0x2c0000, 0x040000 )
	ROM_LOAD( "toy-low-1.j9",   0x200000, 0x040000, CRC(57922c41) SHA1(ffbe5b418ed93e8705a7aabe69d3fad2919a160f) )
	ROM_CONTINUE(               0x280000, 0x040000 )
	ROM_LOAD( "toy-high-0.h10", 0x340000, 0x040000, CRC(5bee6df7) SHA1(ecf759de2f0909f793c84c71feb08801896e2474) )
	ROM_CONTINUE(               0x3c0000, 0x040000 )
	ROM_LOAD( "toy-low-0.j10",  0x300000, 0x040000, CRC(26c49ca2) SHA1(82079eaa2c9523c9acb72fccfbbe9493bc62e84f) )
	ROM_CONTINUE(               0x380000, 0x040000 )

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	// Missing the audio rom, the board didn't have it populated. The programmer said it was not there because the audio was ripped from other games.
	ROM_LOAD( "c1", 0x000000, 0x080000, BAD_DUMP CRC(edf77532) SHA1(cf198b14c25e1b242a65af8ce23538404cd2b12d) ) // sldh
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
	ROM_LOAD( "c3", 0x080000, 0x080000, BAD_DUMP CRC(c3aea660) SHA1(639d4195391e2608e94759e8a4385b518872263a) ) // sldh
ROM_END

ROM_START( lastkm ) /* PCB - REF 922804/2 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE(    "prog-bici-e-8.11.95.d18",  0x000000, 0x080000, CRC(1fc5fba0) SHA1(1f954fca9f25df7379eff4ea905810fa06fcebb0)) /*1.0.0275 */
	ROM_LOAD16_BYTE(    "prog-bici-o-8.11.95.d16",  0x000001, 0x080000, CRC(b93e57e3) SHA1(df307191a214a32a26018ca2a9200742e39939d2)) /*1.0.0275 */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "bici-f3.h6",     0x000000, 0x080000, CRC(0bf9f213) SHA1(052abef60df419d32bf8a86c89d87e5bb281b4eb))
	ROM_LOAD( "bici-f2.h7",     0x080000, 0x080000, CRC(c48d5376) SHA1(8e987839e7254e0fa631802733482726a289439c))
	ROM_LOAD( "bici-f1.h9",     0x100000, 0x080000, CRC(e7958070) SHA1(7f065b429a500b714dfbf497b1353e90137abbd7))
	ROM_LOAD( "bici-f0.h10",    0x180000, 0x080000, CRC(73d4b29f) SHA1(e2563562cb5fbaba7e0517ec9811645aca56f374))

	ROM_REGION( 0x80000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "sonido-bici-0-8.11.95.c1", 0x000000, 0x080000, CRC(7380c963) SHA1(d1d90912f986b944cd95bd773c9f5502d837ce3e))
ROM_END

/*
Squash (version 1.0)
Gaelco, 1992

PCB Layout
----------

REF.922804/1
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

ROM_START( squash ) /* PCB - REF.922804/1 */
	ROM_REGION( 0x100000, "maincpu", 0 )    /* 68000 code */
	ROM_LOAD16_BYTE( "squash.d18", 0x000000, 0x20000, CRC(ce7aae96) SHA1(4fe8666ae571bffc5a08fa68346c0623282989eb) )
	ROM_LOAD16_BYTE( "squash.d16", 0x000001, 0x20000, CRC(8ffaedd7) SHA1(f4aada17ba67dd8b6c5a395e832bcbba2764c59d) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "squash.c09", 0x180000, 0x80000, CRC(0bb91c69) SHA1(8be945049ab411a4d49bd64bd3937542ec9ef9fb) ) /* encrypted video ram */
	ROM_LOAD( "squash.c10", 0x100000, 0x80000, CRC(892a035c) SHA1(d0156ceb9aa6639a1124c17fb12389be319bb51f) ) /* encrypted video ram */
	ROM_LOAD( "squash.c11", 0x080000, 0x80000, CRC(9e19694d) SHA1(1df4646f3147719fef516a37aa361ae26d9b23a2) ) /* encrypted video ram */
	ROM_LOAD( "squash.c12", 0x000000, 0x80000, CRC(5c440645) SHA1(4f2fc1647ffc549fa079f2dc0aaaceb447afdf44) ) /* encrypted video ram */

	ROM_REGION( 0x80000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "squash.d01", 0x000000, 0x80000, CRC(a1b9651b) SHA1(a396ba94889f70ea06d6330e3606b0f2497ff6ce) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END


ROM_START( thoop ) /* PCB - REF.922804/1 */
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

	ROM_REGION( 0x100000, "oki", 0 )    /* ADPCM samples - sound chip is OKIM6295 */
	ROM_LOAD( "sound", 0x000000, 0x100000, CRC(99f80961) SHA1(de3a514a8f46dffd5f762e52aac1f4c3b08e2e18) )
	/* 0x00000-0x2ffff is fixed, 0x30000-0x3ffff is bank switched from all the ROMs */
ROM_END



/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1991, bigkarnk, 0,        bigkarnk, bigkarnk, gaelco_state, empty_init, ROT0, "Gaelco", "Big Karnak", MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoy,  0,        maniacsq, biomtoy,  gaelco_state, empty_init, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1885)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoya, biomtoy,  maniacsq, biomtoy,  gaelco_state, empty_init, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1884)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, biomtoyb, biomtoy,  maniacsq, biomtoy,  gaelco_state, empty_init, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1878)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, biomtoyc, biomtoy,  maniacsq, biomtoyc, gaelco_state, empty_init, ROT0, "Gaelco", "Biomechanical Toy (Ver. 1.0.1870)", MACHINE_SUPPORTS_SAVE )
GAME( 1994, bioplayc, biomtoy,  maniacsq, bioplayc, gaelco_state, empty_init, ROT0, "Gaelco", "Bioplaything Cop (Ver. 1.0.1823, prototype)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_SOUND ) // copyright based on Ver. 1.0.1870
GAME( 1996, maniacsp, maniacsq, maniacsq, maniacsq, gaelco_state, empty_init, ROT0, "Gaelco", "Maniac Square (prototype)", MACHINE_SUPPORTS_SAVE ) // sometimes listed as a 1992 proto?
GAME( 1995, lastkm,   0,        maniacsq, lastkm,   gaelco_state, empty_init, ROT0, "Gaelco", "Last KM (Ver 1.0.0275)", MACHINE_SUPPORTS_SAVE ) // used on 'Salter' exercise bikes
GAME( 1992, squash,   0,        squash,   squash,   gaelco_state, empty_init, ROT0, "Gaelco", "Squash (Ver. 1.0)", MACHINE_SUPPORTS_SAVE )
GAME( 1992, thoop,    0,        thoop,    thoop,    gaelco_state, empty_init, ROT0, "Gaelco", "Thunder Hoop (Ver. 1, Checksum 02A09F7D)", MACHINE_SUPPORTS_SAVE ) // could be other versions, still Ver. 1 but different checksum listed on boot
