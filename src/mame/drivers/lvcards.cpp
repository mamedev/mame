// license:BSD-3-Clause
// copyright-holders:Zsolt Vasvari, Curt Coder
/***************************************************************************
Lovely Cards/Poker/Pontoon driver, updated by El Condor from work by Uki and
Zsolt Vasvari respectively.
Later cleaned up by Curt Coder and Scott Yellig.
---

Lovely Poker by Uki/El Condor
Enter switch test mode by pressing the Deal key twice when the
crosshatch pattern comes up.

After you get into Check Mode (F2), press the Deal key to switch pages.

Memory Mapped:

0000-5fff   R   ROM
6000-67ff   RW  Battery Backed RAM
9000-93ff   RW  Video RAM
9400-97ff   RW  Color RAM
                Bits 0-3 - color
                Bits 4-5 - character bank
                Bit  6   - flip x
                Bit  7   - Is it used?
a000        R   Input Port 0
a001        R   Input Port 1
a002        R   Input Port 2
a001        W  Control Port 0
a002        W  Control Port 1

I/O Ports:
00         RW  YM2149 Data  Port
               Port A - DSW #1
               Port B - DSW #2
01          W  YM2149 Control Port

---

Pontoon Memory Map (preliminary)

Pontoon driver by Zsolt Vasvari
Enter switch test mode by pressing the Hit key twice when the
crosshatch pattern comes up
After you get into Check Mode (F2), press the Hit key to switch pages.

Memory Mapped:

0000-5fff   R   ROM
6000-67ff   RW  Battery Backed RAM
8000-83ff   RW  Video RAM
8400-87ff   RW  Color RAM
                Bits 0-3 - color
                Bits 4-5 - character bank
                Bit  6   - flip x
                Bit  7   - Is it used?
a000        R   Input Port 0
a001        R   Input Port 1
a002        R   Input Port 2
a001        W   Control Port 0
a002        W   Control Port 1

I/O Ports:
00         RW  YM2149 Data Port
               Port A - DSW #1
               Port B - DSW #2
01          W  YM2149 Control Port

TODO:

- What do the control ports do? Payout?
- CPU speed/ YM2149 frequencies
- Input ports need to be cleaned up
- NVRAM does not work for lvcards?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "machine/nvram.h"
#include "includes/lvcards.h"


MACHINE_START_MEMBER(lvcards_state,lvpoker)
{
	save_item(NAME(m_payout));
	save_item(NAME(m_pulse));
	save_item(NAME(m_result));
}

MACHINE_RESET_MEMBER(lvcards_state,lvpoker)
{
	m_payout = 0;
	m_pulse = 0;
	m_result = 0;
}

WRITE8_MEMBER(lvcards_state::control_port_2_w)
{
	switch (data)
	{
		case 0x60:
		m_payout = 1;
		break;
		case 0xc0:
		m_payout = 1;
		break;
		default:
		m_payout = 0;
		break;
	}
}

WRITE8_MEMBER(lvcards_state::control_port_2a_w)
{
	switch (data)
	{
		case 0x60:
		m_payout = 1;
		break;
		case 0x80:
		m_payout = 1;
		break;
		default:
		m_payout = 0;
		break;
	}
}

READ8_MEMBER(lvcards_state::payout_r)
{
	m_result = ioport("IN2")->read();

	if (m_payout)
	{
		if ( m_pulse < 3 )
		{
			m_result = m_result | 0x40;
			m_pulse++;
		}
		else
		{
			m_pulse = 0;
		}
	}
	return m_result;
}

static ADDRESS_MAP_START( ponttehk_map, AS_PROGRAM, 8, lvcards_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x8000, 0x83ff) AM_RAM_WRITE(lvcards_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x8400, 0x87ff) AM_RAM_WRITE(lvcards_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("IN1") AM_WRITENOP // lamps
	AM_RANGE(0xa002, 0xa002) AM_READ(payout_r) AM_WRITE(control_port_2a_w)//AM_WRITENOP // ???
ADDRESS_MAP_END

static ADDRESS_MAP_START( lvcards_map, AS_PROGRAM, 8, lvcards_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(lvcards_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(lvcards_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("IN1") AM_WRITENOP
	AM_RANGE(0xa002, 0xa002) AM_READ_PORT("IN2") AM_WRITENOP
	AM_RANGE(0xc000, 0xdfff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( lvcards_io_map, AS_IO, 8, lvcards_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_DEVREAD("aysnd", ay8910_device, data_r)
	AM_RANGE(0x00, 0x01) AM_DEVWRITE("aysnd", ay8910_device, data_address_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( lvpoker_map, AS_PROGRAM, 8, lvcards_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x67ff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x9000, 0x93ff) AM_RAM_WRITE(lvcards_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x9400, 0x97ff) AM_RAM_WRITE(lvcards_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0xa000, 0xa000) AM_READ_PORT("IN0")
	AM_RANGE(0xa001, 0xa001) AM_READ_PORT("IN1") AM_WRITENOP // lamps
	AM_RANGE(0xa002, 0xa002) AM_READ(payout_r) AM_WRITE(control_port_2_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROM
ADDRESS_MAP_END

static INPUT_PORTS_START( lvcards )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Analyzer")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW0")
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
	PORT_DIPNAME( 0x80, 0x80, "Reset?" )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x07, "1 COIN =" )
	PORT_DIPSETTING(    0x06, "5$" )
	PORT_DIPSETTING(    0x05, "10$" )
	PORT_DIPSETTING(    0x04, "15$" )
	PORT_DIPSETTING(    0x03, "20$" )
	PORT_DIPSETTING(    0x07, "25$" )
	PORT_DIPSETTING(    0x02, "50$" )
	PORT_DIPSETTING(    0x01, "75$" )
	PORT_DIPSETTING(    0x00, "100$" )
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
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( lvpoker )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Analyzer") PORT_TOGGLE
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Memory Reset")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Clear Stats")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Hopper Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH ) PORT_NAME("Red")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_LOW ) PORT_NAME("Black")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL ) PORT_NAME("Deal / Double")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Remove Credit as coins")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL ) // Token Drop
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancel / Take Score")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) // Overflow
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SPECIAL ) // Token Out
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SPECIAL)

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, "Winning Percentage" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "74%" )
	PORT_DIPSETTING(    0x04, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x01, "94%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_DIPNAME( 0x08, 0x08, "Max. Payout Adjustment")
	PORT_DIPSETTING(    0x08, "Free")
	PORT_DIPSETTING(    0x00, "2000")
	PORT_DIPNAME( 0x10, 0x10, "Bonus Game Difficulty")
	PORT_DIPSETTING(    0x10, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPNAME( 0x60, 0x20, "Payment Method" )
	PORT_DIPSETTING(    0x00, "Credit In/Coin Out")
	PORT_DIPSETTING(    0x20, "Credit In/Credit Out")
	PORT_DIPSETTING(    0x40, "Coin In/Coin Out")
	//PORT_DIPSETTING(    0x60, "Credit In/Coin Out") Again, clearly no Coin in, Credit out
	PORT_DIPNAME( 0x80, 0x80, "Memory Reset Switch" )
	PORT_DIPSETTING(    0x80, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin C (Service Switch)" )
	PORT_DIPSETTING(    0x40, "1 Push/1 Credit" )
	PORT_DIPSETTING(    0x00, "1 Push/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( ponttehk )
	PORT_START("IN0")
	PORT_SERVICE( 0x01, IP_ACTIVE_LOW )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Reset All")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE3 ) PORT_NAME("Clear Stats")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U)  PORT_NAME("Call Attendant")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Hopper Reset")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Bonus Game")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Stand")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Hit")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Use Credit") PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Remove Credit as coins") PORT_CODE(KEYCODE_A)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SPECIAL )// Token Drop
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 ) PORT_IMPULSE(3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN1 ) PORT_IMPULSE(3)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN3 ) PORT_IMPULSE(3)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL )   // Overflow optometric switch - reversed logic
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("Coinout Sensor") //Token Out
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SPECIAL )//Motor On?

	PORT_START("DSW0")
	PORT_DIPNAME( 0x07, 0x07, "Winning Percentage" )
	PORT_DIPSETTING(    0x06, "70%" )
	PORT_DIPSETTING(    0x05, "74%" )
	PORT_DIPSETTING(    0x04, "78%" )
	PORT_DIPSETTING(    0x03, "82%" )
	PORT_DIPSETTING(    0x02, "86%" )
	PORT_DIPSETTING(    0x07, "90%" )
	PORT_DIPSETTING(    0x01, "94%" )
	PORT_DIPSETTING(    0x00, "98%" )
	PORT_BIT( 0x18, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x60, 0x20, "Payment Method" )
	PORT_DIPSETTING(    0x00, "Credit In/Coin Out" )
	PORT_DIPSETTING(    0x20, "Coin In/Coin Out" )
	PORT_DIPSETTING(    0x40, "Credit In/Credit Out" )
	//PORT_DIPSETTING(    0x60, "Credit In/Coin Out" ) Again, clearly no Coin in, Credit out
	PORT_DIPNAME( 0x80, 0x80, "Reset All Switch" )
	PORT_DIPSETTING(    0x80, "Disabled" )
	PORT_DIPSETTING(    0x00, "Enabled" )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x07, 0x06, DEF_STR( Coin_A ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x05, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x06, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x07, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x38, 0x30, DEF_STR( Coin_B ) )
	PORT_DIPSETTING(    0x28, DEF_STR( 2C_2C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x38, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x18, DEF_STR( 1C_4C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPSETTING(    0x00, "1 Coin/10 Credits" )
	PORT_DIPNAME( 0x40, 0x40, "Coin C (Service Switch)" )
	PORT_DIPSETTING(    0x40, "1 Push/1 Credit" )
	PORT_DIPSETTING(    0x00, "1 Push/10 Credits" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Demo_Sounds ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,    /* 8*8 characters */
	RGN_FRAC(1,1),   /* 2048 characters */
	4,      /* 4 bits per pixel */
	{0,1,2,3},
	{4,0,12,8,20,16,28,24},
	{32*0, 32*1, 32*2, 32*3, 32*4, 32*5, 32*6, 32*7},
	32*8
};

/* Graphics Decode Information */

static GFXDECODE_START( lvcards )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END

/* Sound Interfaces */

static MACHINE_CONFIG_START( lvcards, lvcards_state )
	// basic machine hardware
	MCFG_CPU_ADD("maincpu",Z80, 18432000/6) // 3.072 MHz ?

	MCFG_CPU_PROGRAM_MAP(lvcards_map)
	MCFG_CPU_IO_MAP(lvcards_io_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", lvcards_state,  irq0_line_hold)

	// video hardware

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(8*0, 32*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(lvcards_state, screen_update_lvcards)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", lvcards)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(lvcards_state, lvcards)

	// sound hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("aysnd", AY8910, 18432000/12)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("DSW0"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("DSW1"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( lvpoker, lvcards )

	// basic machine hardware
	MCFG_NVRAM_ADD_1FILL("nvram")
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(lvpoker_map)
	MCFG_MACHINE_START_OVERRIDE(lvcards_state,lvpoker)
	MCFG_MACHINE_RESET_OVERRIDE(lvcards_state,lvpoker)
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( ponttehk, lvcards )

	// basic machine hardware
	MCFG_NVRAM_ADD_1FILL("nvram")
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(ponttehk_map)
	MCFG_MACHINE_RESET_OVERRIDE(lvcards_state,lvpoker)

	// video hardware
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(lvcards_state,ponttehk)
MACHINE_CONFIG_END

ROM_START( lvpoker )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lp1.bin",      0x0000, 0x4000, CRC(ecfefa91) SHA1(7f6e0f30a2c4299797a8066419bf247b2900e312) )
	ROM_LOAD( "lp2.bin",      0x4000, 0x2000, CRC(06d5484f) SHA1(326756a03eaeefc944428c7e011fcdc128aa415a) )
	ROM_LOAD( "lp3.bin",      0xc000, 0x2000, CRC(05e17de8) SHA1(76b38e414f225789de8af9ca0556008e17285ffe) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "lp4.bin",      0x0000, 0x2000, CRC(04fd2a6b) SHA1(33fb42f54646dc91f5aca1c55cfc932fa04f5d77) )
	ROM_CONTINUE(             0x8000, 0x2000 )
	ROM_LOAD( "lp5.bin",      0x2000, 0x2000, CRC(9b5b531c) SHA1(1ce700361ea39a15c9c62fc0fa61df0cda62a340) )
	ROM_CONTINUE(             0xa000, 0x2000 )
	ROM_LOAD( "lc6.bin",      0x4000, 0x2000, CRC(2991a6ec) SHA1(b2c32550884b7b708db48bb7f0854bbad504417d) )
	ROM_RELOAD(               0xc000, 0x2000 )
	ROM_LOAD( "lp7.bin",      0x6000, 0x2000, CRC(217e9cfb) SHA1(3e7d76db5195c599c2bf186ae6616b29bc0fd337) )
	ROM_RELOAD(               0xe000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "3.7c",         0x0000, 0x0100, CRC(0c2ead4a) SHA1(e8e29e21366622c9bf3ee5e807c83b5cd7da8e3e) )
	ROM_LOAD( "2.7b",         0x0100, 0x0100, CRC(f4bc51e2) SHA1(38293c1117d6f3076081b6f2da3f42819558b04f) )
	ROM_LOAD( "1.7a",         0x0200, 0x0100, CRC(e40f2363) SHA1(cea598b6ed037dd3b4306c2ca3b0b4d5197d42a4) )
ROM_END

ROM_START( lvcards )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "lc1.bin", 0x0000, 0x4000, CRC(0c5fbf05) SHA1(bf996bdccfc5748cee91d004f2b1da10bcd8e329) )
	ROM_LOAD( "lc2.bin", 0x4000, 0x2000, CRC(deb54548) SHA1(a245898635c5cd3c26989c2bba89bb71edacd906) )
	ROM_LOAD( "lc3.bin", 0xc000, 0x2000, CRC(45c2bea9) SHA1(3a33501824769656aa87649c3fd0a8b8a4d83f3c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "lc4.bin", 0x0000, 0x2000, CRC(dd705389) SHA1(271c11c2bd9affd976d65e318fd9fb01dbdde040) )
	ROM_CONTINUE(        0x8000, 0x2000 )
	ROM_LOAD( "lc5.bin", 0x2000, 0x2000, CRC(ddd1e3e5) SHA1(b7e8ccaab318b61b91eae4eee9e04606f9717037) )
	ROM_CONTINUE(        0xa000, 0x2000 )
	ROM_LOAD( "lc6.bin", 0x4000, 0x2000, CRC(2991a6ec) SHA1(b2c32550884b7b708db48bb7f0854bbad504417d) )
	ROM_RELOAD(          0xc000, 0x2000 )
	ROM_LOAD( "lc7.bin", 0x6000, 0x2000, CRC(f1b84c56) SHA1(6834139400bf8aa8db17f65bfdcbdcb2433d5fdc) )
	ROM_RELOAD(          0xe000, 0x2000 )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "3.7c", 0x0000,  0x0100, CRC(0c2ead4a) SHA1(e8e29e21366622c9bf3ee5e807c83b5cd7da8e3e) )
	ROM_LOAD( "2.7b", 0x0100,  0x0100, CRC(f4bc51e2) SHA1(38293c1117d6f3076081b6f2da3f42819558b04f) )
	ROM_LOAD( "1.7a", 0x0200,  0x0100, CRC(e40f2363) SHA1(cea598b6ed037dd3b4306c2ca3b0b4d5197d42a4) )
ROM_END

ROM_START( ponttehk )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "ponttehk.001", 0x0000, 0x4000, CRC(1f8c1b38) SHA1(3776ddd695741223bd9ad41f74187bff31f2cd3b) )
	ROM_LOAD( "ponttehk.002", 0x4000, 0x2000, CRC(befb4f48) SHA1(8ca146c8b52afab5deb6f0ff52bdbb2b1ff3ded7) )

	ROM_REGION( 0x8000, "gfx1", 0 )
	ROM_LOAD( "ponttehk.003", 0x0000, 0x2000, CRC(a6a91b3d) SHA1(d180eabe67efd3fd1205570b661a74acf7ed93b3) )
	ROM_LOAD( "ponttehk.004", 0x2000, 0x2000, CRC(976ed924) SHA1(4d305694b3e157411068baf3052e3aac7d0b32d5) )
	ROM_LOAD( "ponttehk.005", 0x4000, 0x2000, CRC(2b8e8ca7) SHA1(dd86d3b4fd1627bdaa0603ffd2f1bc2953bc51f8) )
	ROM_LOAD( "ponttehk.006", 0x6000, 0x2000, CRC(6bc23965) SHA1(b73a584fc5b2dd9436bbb8bc1620f5a51d351aa8) )

	ROM_REGION( 0x0300, "proms", 0 )
	ROM_LOAD( "pon24s10.003", 0x0000, 0x0100, CRC(4623b7f3) SHA1(55948753dec09d0a476b90ca75e7e092ce0f68ee) )  /* red component */
	ROM_LOAD( "pon24s10.002", 0x0100, 0x0100, CRC(117e1b67) SHA1(b753137878fe5cd650722cf526cd4929821240a8) )  /* green component */
	ROM_LOAD( "pon24s10.001", 0x0200, 0x0100, CRC(c64ecee8) SHA1(80c9ec21e135235f7f2d41ce7900cf3904123823) )  /* blue component */
ROM_END

GAME( 1985, lvcards,        0, lvcards,  lvcards, driver_device,  0, ROT0, "Tehkan", "Lovely Cards", 0 )
GAME( 1985, lvpoker,  lvcards, lvpoker,  lvpoker, driver_device,  0, ROT0, "Tehkan", "Lovely Poker [BET]", 0 )
GAME( 1985, ponttehk,       0, ponttehk, ponttehk, driver_device, 0, ROT0, "Tehkan", "Pontoon (Tehkan)", 0 )
