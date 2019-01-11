// license:BSD-3-Clause
// copyright-holders:Manuel Abadia
/***************************************************************************

Dragon Buster (c) 1984 Namco
Sky Kid       (c) 1985 Namco

Driver by Manuel Abadia <emumanu+mame@gmail.com>

Notes:
-----
- Sky Kid sets:
  There are 2 basic versions, one with CUS63 and rom prefix "SK2" and
  the other which uses CUS60 and has the rom prefix "SK1"

***************************************************************************/

#include "emu.h"
#include "includes/skykid.h"

#include "cpu/m6809/m6809.h"
#include "machine/watchdog.h"
#include "sound/namco.h"
#include "screen.h"
#include "speaker.h"


WRITE8_MEMBER(skykid_state::inputport_select_w)
{
	if ((data & 0xe0) == 0x60)
		m_inputport_selected = data & 0x07;
	else if ((data & 0xe0) == 0xc0)
	{
		machine().bookkeeping().coin_lockout_global_w(~data & 1);
		machine().bookkeeping().coin_counter_w(0,data & 2);
		machine().bookkeeping().coin_counter_w(1,data & 4);
	}
}

READ8_MEMBER(skykid_state::inputport_r)
{
	switch (m_inputport_selected)
	{
		case 0x00:  /* DSW B (bits 0-4) */
			return (ioport("DSWB")->read() & 0xf8) >> 3;
		case 0x01:  /* DSW B (bits 5-7), DSW A (bits 0-1) */
			return ((ioport("DSWB")->read() & 0x07) << 2) | ((ioport("DSWA")->read() & 0xc0) >> 6);
		case 0x02:  /* DSW A (bits 2-6) */
			return (ioport("DSWA")->read() & 0x3e) >> 1;
		case 0x03:  /* DSW A (bit 7), DSW C (bits 0-3) */
			return ((ioport("DSWA")->read() & 0x01) << 4) | (ioport("BUTTON2")->read() & 0x0f);
		case 0x04:  /* coins, start */
			return ioport("SYSTEM")->read();
		case 0x05:  /* 2P controls */
			return ioport("P2")->read();
		case 0x06:  /* 1P controls */
			return ioport("P1")->read();
		default:
			return 0xff;
	}
}

WRITE8_MEMBER(skykid_state::skykid_led_w)
{
	m_leds[0] = BIT(data, 3);
	m_leds[1] = BIT(data, 4);
}

WRITE8_MEMBER(skykid_state::skykid_subreset_w)
{
	int bit = !BIT(offset,11);
	m_mcu->set_input_line(INPUT_LINE_RESET, bit ? CLEAR_LINE : ASSERT_LINE);
}

WRITE8_MEMBER(skykid_state::skykid_bankswitch_w)
{
	membank("bank1")->set_entry(!BIT(offset,11));
}

WRITE8_MEMBER(skykid_state::skykid_irq_1_ctrl_w)
{
	int bit = !BIT(offset,11);
	m_main_irq_mask = bit;
	if (!bit)
		m_maincpu->set_input_line(0, CLEAR_LINE);
}

WRITE8_MEMBER(skykid_state::skykid_irq_2_ctrl_w)
{
	int bit = !BIT(offset,13);
	m_mcu_irq_mask = bit;
	if (!bit)
		m_mcu->set_input_line(0, CLEAR_LINE);
}

void skykid_state::machine_start()
{
	m_leds.resolve();

	/* configure the banks */
	membank("bank1")->configure_entries(0, 2, memregion("maincpu")->base() + 0x10000, 0x2000);

	save_item(NAME(m_inputport_selected));
}



void skykid_state::skykid_map(address_map &map)
{
	map(0x0000, 0x1fff).bankr("bank1");                /* banked ROM */
	map(0x2000, 0x2fff).rw(FUNC(skykid_state::skykid_videoram_r), FUNC(skykid_state::skykid_videoram_w)).share("videoram");/* Video RAM (background) */
	map(0x4000, 0x47ff).rw(FUNC(skykid_state::skykid_textram_r), FUNC(skykid_state::skykid_textram_w)).share("textram");    /* video RAM (text layer) */
	map(0x4800, 0x5fff).ram().share("spriteram");   /* RAM + Sprite RAM */
	map(0x6000, 0x60ff).w(FUNC(skykid_state::skykid_scroll_y_w));        /* Y scroll register map */
	map(0x6200, 0x63ff).w(FUNC(skykid_state::skykid_scroll_x_w));        /* X scroll register map */
	map(0x6800, 0x6bff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); /* PSG device, shared RAM */
	map(0x7000, 0x7fff).w(FUNC(skykid_state::skykid_irq_1_ctrl_w));      /* IRQ control */
	map(0x7800, 0x7fff).r("watchdog", FUNC(watchdog_timer_device::reset_r));
	map(0x8000, 0xffff).rom();                 /* ROM */
	map(0x8000, 0x8fff).w(FUNC(skykid_state::skykid_subreset_w));        /* MCU control */
	map(0x9000, 0x9fff).w(FUNC(skykid_state::skykid_bankswitch_w));      /* Bankswitch control */
	map(0xa000, 0xa001).w(FUNC(skykid_state::skykid_flipscreen_priority_w)); /* flip screen & priority */
}

void skykid_state::mcu_map(address_map &map)
{
	map(0x0000, 0x001f).rw(m_mcu, FUNC(hd63701_cpu_device::m6801_io_r), FUNC(hd63701_cpu_device::m6801_io_w));
	map(0x0080, 0x00ff).ram();
	map(0x1000, 0x13ff).rw(m_cus30, FUNC(namco_cus30_device::namcos1_cus30_r), FUNC(namco_cus30_device::namcos1_cus30_w)); /* PSG device, shared RAM */
	map(0x2000, 0x3fff).w("watchdog", FUNC(watchdog_timer_device::reset_w));     /* watchdog? */
	map(0x4000, 0x7fff).w(FUNC(skykid_state::skykid_irq_2_ctrl_w));
	map(0x8000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xf000, 0xffff).rom();
}



static INPUT_PORTS_START( skykid )
	PORT_START("DSWA")  /* DSW A */
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Round Skip" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")  /* DSW B */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "20k every 80k" )
	PORT_DIPSETTING(    0x10, "20k and 80k" )
	PORT_DIPSETTING(    0x20, "30k every 90k" )
	PORT_DIPSETTING(    0x30, "30k and 90k" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SWB:6" )
	PORT_DIPNAME( 0x02, 0x02, "Allow Buy In" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    /* IN 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")    /* IN 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* IN 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( skykids )
	PORT_START("DSWA")  /* DSW A */
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Round Select" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")  /* DSW B */
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Lives ) ) PORT_DIPLOCATION("SWB:2,1")
	PORT_DIPSETTING(    0x80, "1" )
	PORT_DIPSETTING(    0x40, "2" )
	PORT_DIPSETTING(    0xc0, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Bonus_Life ) ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "30k every 80k" )
	PORT_DIPSETTING(    0x10, "30k and 80k" )
	PORT_DIPSETTING(    0x20, "40k every 90k" )
	PORT_DIPSETTING(    0x30, "40k and 90k" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SWB:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SWB:6" )
	PORT_DIPNAME( 0x02, 0x02, "Allow Buy In" ) PORT_DIPLOCATION("SWB:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Flip_Screen ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    /* IN 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")    /* IN 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* IN 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static INPUT_PORTS_START( drgnbstr )
	PORT_START("DSWA")  /* DSW A */
	PORT_SERVICE_DIPLOC( 0x80, IP_ACTIVE_LOW, "SWA:1" )
	PORT_DIPNAME( 0x60, 0x60, DEF_STR( Coin_A ) ) PORT_DIPLOCATION("SWA:3,2")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x60, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) ) PORT_DIPLOCATION("SWA:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "Round Skip" ) PORT_DIPLOCATION("SWA:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "Freeze" ) PORT_DIPLOCATION("SWA:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_B ) ) PORT_DIPLOCATION("SWA:8,7")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )

	PORT_START("DSWB")  /* DSW B */
	PORT_DIPNAME( 0x80, 0x80, "Spurt Time" ) PORT_DIPLOCATION("SWB:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x40, 0x40, "Level of Monster" ) PORT_DIPLOCATION("SWB:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Difficult ) )
	PORT_DIPNAME( 0x30, 0x30, "Starting Vitality" ) PORT_DIPLOCATION("SWB:4,3")
	PORT_DIPSETTING(    0x00, "160" )
	PORT_DIPSETTING(    0x30, "128" )
	PORT_DIPSETTING(    0x10, "96" )
	PORT_DIPSETTING(    0x20, "64" )
	PORT_DIPNAME( 0x0c, 0x0c, "Bonus Vitality" ) PORT_DIPLOCATION("SWB:6,5") /* Clear Mountain, Tower, ect... */
	PORT_DIPSETTING(    0x00, "64" )
	PORT_DIPSETTING(    0x08, "48/64" )
	PORT_DIPSETTING(    0x04, "32/64" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x02, 0x02, "Bonus Level" ) PORT_DIPLOCATION("SWB:7")    /* Clear Round */
	PORT_DIPSETTING(    0x02, "Full" )
	PORT_DIPSETTING(    0x00, "Partial" )
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Allow_Continue ) ) PORT_DIPLOCATION("SWB:8")
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )

	PORT_START("BUTTON2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Cabinet ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Cocktail ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")    /* IN 0 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")    /* IN 1 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P2")    /* IN 2 */
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_4WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END



static const gfx_layout text_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout tile_layout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
	{ 0*8, 2*8, 4*8, 6*8, 8*8, 10*8, 12*8, 14*8 },
	16*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	3,
	{ RGN_FRAC(1,2)+4, 0, 4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			32*8, 33*8, 34*8, 35*8, 36*8, 37*8, 38*8, 39*8 },
	64*8
};

static GFXDECODE_START( gfx_skykid )
	GFXDECODE_ENTRY( "gfx1", 0, text_layout,   0, 64 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,   64*4, 128 )
	GFXDECODE_ENTRY( "gfx3", 0, sprite_layout, 64*4+128*4, 64 )
GFXDECODE_END


WRITE_LINE_MEMBER(skykid_state::vblank_irq)
{
	if (state && m_main_irq_mask)
		m_maincpu->set_input_line(0, ASSERT_LINE);

	if (state && m_mcu_irq_mask)
		m_mcu->set_input_line(0, ASSERT_LINE);
}


void skykid_state::skykid(machine_config &config)
{
	/* basic machine hardware */
	MC6809E(config, m_maincpu, XTAL(49'152'000)/32);
	m_maincpu->set_addrmap(AS_PROGRAM, &skykid_state::skykid_map);

	HD63701(config, m_mcu, XTAL(49'152'000)/8); /* or compatible 6808 with extra instructions */
	m_mcu->set_addrmap(AS_PROGRAM, &skykid_state::mcu_map);
	m_mcu->in_p1_cb().set(FUNC(skykid_state::inputport_r));         /* input ports read */
	m_mcu->out_p1_cb().set(FUNC(skykid_state::inputport_select_w)); /* input port select */
	m_mcu->in_p2_cb().set_constant(0xff);                           /* leds won't work otherwise */
	m_mcu->out_p2_cb().set(FUNC(skykid_state::skykid_led_w));       /* lamps */

	config.m_minimum_quantum = attotime::from_hz(6000);  /* we need heavy synch */

	WATCHDOG_TIMER(config, "watchdog");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60.606060);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(36*8, 28*8);
	screen.set_visarea(0*8, 36*8-1, 0*8, 28*8-1);
	screen.set_screen_update(FUNC(skykid_state::screen_update_skykid));
	screen.set_palette(m_palette);
	screen.screen_vblank().set(FUNC(skykid_state::vblank_irq));

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_skykid);
	PALETTE(config, m_palette, FUNC(skykid_state::skykid_palette), 64*4 + 128*4 + 64*8, 256);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	NAMCO_CUS30(config, m_cus30, 49152000/2048);
	m_cus30->set_voices(8);
	m_cus30->add_route(ALL_OUTPUTS, "mono", 1.0);
}


ROM_START( skykid ) // a PCB was found with ROM 4 and 6 labeled sk1, but hashes match the sk2 listed here and in other sets, while they differ from the sk1 ROMs in set skykidd?
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "sk2_2.6c",     0x08000, 0x4000, CRC(ea8a5822) SHA1(5b13133410bcb7d647e662b476dbfd2edab8aac0) )
	ROM_LOAD( "sk1-1c.6b",    0x0c000, 0x4000, CRC(7abe6c6c) SHA1(7d2631cc6149fa3e02b1355cb899de5474ff5d0a) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   /* banked ROM */

	ROM_REGION( 0x10000, "mcu", 0 ) /* MCU code */
	ROM_LOAD( "sk2_4.3c",       0x8000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  /* subprogram for the MCU */
	ROM_LOAD( "cus63-63a1.mcu", 0xf000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  /* MCU internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )    /* chars */

	ROM_REGION( 0x02000, "gfx2", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )    /* sprites */
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	/* 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff */
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    /* red component */
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    /* green component */
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    /* blue component */
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    /* tiles lookup table */
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    /* sprites lookup table */
ROM_END

ROM_START( skykido )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "sk2_2.6c",     0x08000, 0x4000, CRC(ea8a5822) SHA1(5b13133410bcb7d647e662b476dbfd2edab8aac0) )
	ROM_LOAD( "sk1_1.6b",     0x0c000, 0x4000, CRC(070a49d4) SHA1(4b994bde3e34b574bd927843804d2fb1a08d1bdf) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   /* banked ROM */

	ROM_REGION( 0x10000, "mcu", 0 ) /* MCU code */
	ROM_LOAD( "sk2_4.3c",       0x8000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  /* subprogram for the MCU */
	ROM_LOAD( "cus63-63a1.mcu", 0xf000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  /* MCU internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )    /* chars */

	ROM_REGION( 0x02000, "gfx2", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )    /* sprites */
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	/* 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff */
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    /* red component */
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    /* green component */
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    /* blue component */
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    /* tiles lookup table */
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    /* sprites lookup table */
ROM_END

ROM_START( skykidd )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "sk1_2.6c",     0x08000, 0x4000, CRC(8370671a) SHA1(7038f952ebfc4482440b73ee4027fa908561d122) )
	ROM_LOAD( "sk1_1.6b",     0x0c000, 0x4000, CRC(070a49d4) SHA1(4b994bde3e34b574bd927843804d2fb1a08d1bdf) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   /* banked ROM */

	ROM_REGION( 0x10000, "mcu", 0 ) /* MCU code */
	ROM_LOAD( "sk1_4.3c",       0x8000, 0x2000, CRC(887137cc) SHA1(dd0f66afb78833c4da73539b692854346f448c0d) )  /* subprogram for the MCU */
	ROM_LOAD( "cus60-60a1.mcu", 0xf000, 0x1000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) )  /* MCU internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )    /* chars */

	ROM_REGION( 0x02000, "gfx2", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )    /* sprites */
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	/* 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff */
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    /* red component */
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    /* green component */
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    /* blue component */
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    /* tiles lookup table */
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    /* sprites lookup table */
ROM_END

ROM_START( skykids )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "sk2a.6c",     0x08000, 0x4000, CRC(68492672) SHA1(3dbe5ec930de5c526d3ef65513993c10f2153a36) )
	ROM_LOAD( "sk1a.6b",     0x0c000, 0x4000, CRC(e16abe25) SHA1(78e0d30b15fb62c4399d847784ddc61f6819feba) )
	ROM_LOAD( "sk1_3.6d",     0x10000, 0x4000, CRC(314b8765) SHA1(d90a8a853ce672fe5ee190f07bcb33262c73df3b) )   /* banked ROM */

	ROM_REGION( 0x10000, "mcu", 0 ) /* MCU code */
	ROM_LOAD( "sk2_4.3c",       0x8000, 0x2000, CRC(a460d0e0) SHA1(7124ffeb3b84b282940dcbf9421ae4934bcce1c8) )  /* subprogram for the MCU */
	ROM_LOAD( "cus63-63a1.mcu", 0xf000, 0x1000, CRC(6ef08fb3) SHA1(4842590d60035a0059b0899eb2d5f58ae72c2529) )  /* MCU internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "sk1_6.6l",     0x0000, 0x2000, CRC(58b731b9) SHA1(40f7be85914833ce02a734c20d68c0db8b77911d) )    /* chars */

	ROM_REGION( 0x02000, "gfx2", 0 )
	ROM_LOAD( "sk1_5.7e",     0x0000, 0x2000, CRC(c33a498e) SHA1(9f89a514888418a9bebbca341a8cc66e41b58acb) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "sk1_8.10n",    0x0000, 0x4000, CRC(44bb7375) SHA1(5b2fa6782671150bab5f3c3ac46b47bc23f3d7e0) )    /* sprites */
	ROM_LOAD( "sk1_7.10m",    0x4000, 0x4000, CRC(3454671d) SHA1(723b26a0f208addc2a22736457cb4be6ab6c69cc) )
	/* 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff */
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "sk1-1.2n",     0x0000, 0x0100, CRC(0218e726) SHA1(8b766162a4783c058d9a1ecf8741673d7ef955fb) )    /* red component */
	ROM_LOAD( "sk1-2.2p",     0x0100, 0x0100, CRC(fc0d5b85) SHA1(d1b13e42e735b24594cf0b840dee8110de23369e) )    /* green component */
	ROM_LOAD( "sk1-3.2r",     0x0200, 0x0100, CRC(d06b620b) SHA1(968a2d62c65e201d521e9efa8fcf6ad15898e4b3) )    /* blue component */
	ROM_LOAD( "sk1-4.5n",     0x0300, 0x0200, CRC(c697ac72) SHA1(3b79755e6cbb22c14fc4affdbd3f4521da1d90e8) )    /* tiles lookup table */
	ROM_LOAD( "sk1-5.6n",     0x0500, 0x0200, CRC(161514a4) SHA1(4488ce60d12be6586e4a1ddbbfd06bf4e7dfaceb) )    /* sprites lookup table */
ROM_END

ROM_START( drgnbstr )
	ROM_REGION( 0x14000, "maincpu", 0 ) /* 6809 code */
	ROM_LOAD( "db1_2b.6c",    0x08000, 0x04000, CRC(0f11cd17) SHA1(691d853f4f08898ecf4bccfb70a568de309329f1) )
	ROM_LOAD( "db1_1.6b",     0x0c000, 0x04000, CRC(1c7c1821) SHA1(8b6111afc42e2996bdc2fc276be0c40556cd431e) )
	ROM_LOAD( "db1_3.6d",     0x10000, 0x04000, CRC(6da169ae) SHA1(235211c26562fef0660e3fde1e87f2e52626d119) )  /* banked ROM */

	ROM_REGION( 0x10000, "mcu", 0 ) /* MCU code */
	ROM_LOAD( "db1_4.3c",       0x8000, 0x02000, CRC(8a0b1fc1) SHA1(c2861d0da63e2d17f2d1ad46dccf753ecd902ce3) ) /* subprogram for the MCU */
	ROM_LOAD( "cus60-60a1.mcu", 0xf000, 0x01000, CRC(076ea82a) SHA1(22b5e62e26390d7d5cacc0503c7aa5ed524204df) ) /* MCU internal code */

	ROM_REGION( 0x02000, "gfx1", 0 )
	ROM_LOAD( "db1_6.6l",     0x0000, 0x2000, CRC(c080b66c) SHA1(05dcd45274d0bd12ef8ae7fd10c8719e679b3e7b) )    /* tiles */

	ROM_REGION( 0x02000, "gfx2", 0 )
	ROM_LOAD( "db1_5.7e",     0x0000, 0x2000, CRC(28129aed) SHA1(d7f52e871d97179ec88c142a1c70eb6ad09e534a) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "db1_8.10n",    0x0000, 0x4000, CRC(11942c61) SHA1(0f065cb82cf83967e90b3c7326b36956f4fa9a52) )    /* sprites */
	ROM_LOAD( "db1_7.10m",    0x4000, 0x4000, CRC(cc130fe2) SHA1(4f5d4f21152b3b4e523a6d17dd5ff5cef52447f2) )
	/* 0x8000-0xbfff  will be unpacked from 0x4000-0x5fff */
	ROM_FILL(                 0xc000, 0x4000, 0x00 )    // part of the gfx is 2bpp decoded as 3bpp

	ROM_REGION( 0x0700, "proms", 0 )
	ROM_LOAD( "db1-1.2n",     0x0000, 0x0100, CRC(3f8cce97) SHA1(027b3fb0f322a9d68b434b207a40b31799a8a8d6) )    /* red component */
	ROM_LOAD( "db1-2.2p",     0x0100, 0x0100, CRC(afe32436) SHA1(e405787f7f2aa992edd63078e3944334d8acddb1) )    /* green component */
	ROM_LOAD( "db1-3.2r",     0x0200, 0x0100, CRC(c95ff576) SHA1(861a7340d29e6a6a0d5ead93abd3f73cc3df0cc7) )    /* blue component */
	ROM_LOAD( "db1-4.5n",     0x0300, 0x0200, CRC(b2180c21) SHA1(a5d14c31d54f04494ea99c3d94bd1b5e072b612e) )    /* tiles lookup table */
	ROM_LOAD( "db1-5.6n",     0x0500, 0x0200, CRC(5e2b3f74) SHA1(ef58661fa12a52bc358e81179254d37de7551b38) )    /* sprites lookup table */
ROM_END



void skykid_state::init_skykid()
{
	/* unpack the third sprite ROM */
	uint8_t *rom = memregion("gfx3")->base() + 0x4000;
	for (int i = 0;i < 0x2000;i++)
	{
		rom[i + 0x4000] = rom[i];       // sprite set #1, plane 3
		rom[i + 0x6000] = rom[i] >> 4;  // sprite set #2, plane 3

		rom[i] = rom[i + 0x2000];       // sprite set #3, planes 1&2 (plane 3 is empty)
	}
}

GAME( 1984, drgnbstr, 0,      skykid, drgnbstr, skykid_state, init_skykid, ROT0,   "Namco", "Dragon Buster", MACHINE_SUPPORTS_SAVE )
GAME( 1985, skykid,   0,      skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (new version)", MACHINE_SUPPORTS_SAVE ) /* Uses CUS63 aka 63a1 */
GAME( 1985, skykido,  skykid, skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (old version)", MACHINE_SUPPORTS_SAVE ) /* Uses CUS63 aka 63a1 */
GAME( 1985, skykidd,  skykid, skykid, skykid,   skykid_state, init_skykid, ROT180, "Namco", "Sky Kid (CUS60 version)", MACHINE_SUPPORTS_SAVE ) /* Uses CUS60 aka 60a1 */

// no license text is displayed but the PCB was licensed by Namco for production by Sipem (formerly Sidam) with Namco supplying the Custom chips (MCU etc.)
// the level select is handled in a much more user-friendly way in this set and the dip for it is inverted (although this is displayed incorrectly in the test mode)
GAME( 1985, skykids,  skykid, skykid, skykids,  skykid_state, init_skykid, ROT180, "Namco (Sipem license)", "Sky Kid (Sipem)", MACHINE_SUPPORTS_SAVE ) /* Uses CUS63 aka 63a1 */
