// license:BSD-3-Clause
// copyright-holders:Phil Stroffolino
/*
DJ Boy (c)1989 Kaneko

Hardware has many similarities to Airbusters.

Self Test has two parts:
1) color test : press button#3 to advance past color pattern
2) i/o and sound test: use buttons 1,2,3 to select and play sound/music

- CPU0 manages sprites, which are also used to display text
        irq (0x10) - timing/watchdog
        irq (0x30) - processes sprites
        nmi: wakes up this CPU

- CPU1 manages the protection device, palette, and tilemap(s)
        nmi: resets this CPU
        irq: game update

- CPU2 manages sound chips
        irq: update music
        nmi: handle sound command

- The "BEAST" protection device has access to DIP switches and player inputs.


PCB Layout
----------

BS
|----------------------------------------------|
| 6264               BS15   6116               |
|                    BS-101 6116               |
| BS-005             780C-2                    |
| BS-004                           DSW1 DSW2   |
|                    6264              IO-JAMMA|
| 16MHz                     PAL1|----------|   |
| 12MHz                         |  BEAST   |  J|
|                               |----------|  A|
| BS-003              *                       M|
|            6116     BS-203  6295     YM2203 M|
| BS-000                           YM3014     A|
|          |-------|          6295  324  4558  |
| BS-001   |KANEKO |  780C-2        324        |
|          |PANDORA|                    VOL  JP|
| BS-002   |       |  BS-100  PAL2   324       |
|          |-------|           6264     VOL CN1|
| BS07     4464 4464  BS19     BS-200   LA4460 |
|          4464 4464  PAL3     D780C-2  LA4460 |
|----------------------------------------------|

Notes:
      D780C-2 - Z80 CPU. Clock 6.000MHz [12/2] (for all 3 Z80 CPUs)
      BEAST   - OKI MSM80C51F microcontroller with internal ROM. Clock 6.000MHz on pins 18 & 19
                chip is stamped 'KANEKO Beast (C)Intel '80 (C)KANEKO 1988'
      YM2203  - Yamaha YM2203, clock 3.000MHz [12/4]
      6295    - OKI M6295, clock 1.500MHz [12/8]. Sample rate (Hz) = 12000000 / 8 / 165
      PANDORA - Custom Kaneko graphics generator chip stamped 'PX79C480FP-3 PANDORA-CHIP' (QFP160)
      4464    - 64k x4 DRAM (DIP18)
      6116    - 2k x8 SRAM (DIP24)
      6264    - 8k x8 SRAM (DIP28)
      VSync   - 57.5Hz
      HSync   - 15.68kHz
      JP      - 3 pin jumper to set mono/stereo sound output
      CN1     - 4 pin connector for speakers when jumper is set for stereo sound output
      PAL1    - PAL16L8 stamped 'BS-501'
      PAL2    - PAL16L8 stamped 'BS-502'
      PAL3    - PAL16L8 stamped 'BS-500'
      IO-JAMMA- Custom Kaneko ceramic I/O input resistor pack stamped 'I/O JAMMA MC-8282837'
      LA4460  - Sanyo 12W Power Amplifier (SIL10)
      *       - Unpopulated DIP32 position
      ROMs    -
                BS15.6Y    27C512 EPROM (DIP28)   \ There is an alt. set of labels used for these ROMs with an 'S'
                BS07.1B    27C512 EPROM (DIP28)   | added to the name (i.e. 'BS15S'), but the actual ROM contents is identical
                BS19.4B    27C1001 EPROM (DIP32)  / to the regular set (both sets dumped / verified)
                BS-000.1H  4M MASKROM (DIP32) {sprite}
                BS-001.1F  4M MASKROM (DIP32) {sprite}
                BS-002.1D  4M MASKROM (DIP32) {sprite}
                BS-003.1K  4M MASKROM (DIP32) {sprite}
                BS-004.1S  4M MASKROM (DIP32) {tile}
                BS-005.1U  4M MASKROM (DIP32) {tile}
                BS-100.4D  1M MASKROM (DIP28) {z80}
                BS-101.6W  1M MASKROM (DIP28) {z80 data}
                BS-200.8C  1M MASKROM (DIP28) {z80}
                BS-203.5J  2M MASKROM (DIP32) {oki-m6295 samples}

      DIPs    - SW1
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |SCREEN NORMAL    OFF                        |
                |       FLIP      ON                         |
                |--------------------------------------------|
                |GAME   NORMAL        OFF                    |
                |MODE   TEST          ON                     |
                |--------------------------------------------|
                |COIN1  1C/1P                 OFF OFF        |
                |       1C/2P                 ON  OFF        |
                |       2C/1P                 OFF ON         |
                |       2C/3P                 ON  ON         |
                |                                            |
                |COIN2  1C/1P                         OFF OFF|
                |       1C/2P                         ON  OFF|
                |       2C/1P                         OFF ON |
                |       2C/3P                         ON  ON |
                |--------------------------------------------|
                |SW1 & SW4 NOT USED ALWAYS OFF               |
                |--------------------------------------------|

                SW2
                |--------------------------------------------|
                |              1   2   3   4   5   6   7   8 |
                |--------------------------------------------|
                |DIFFICULTY                                  |
                |NORMAL       OFF OFF                        |
                |EASY         ON  OFF                        |
                |HARD         OFF ON                         |
                |HARDEST      ON  ON                         |
                |--------------------------------------------|
                |BONUS                                       |
                |10,30,50,70,90       OFF OFF                |
                |10,20,30,40,50,                             |
                |60,70,80,90          ON  OFF                |
                |20,50                OFF ON                 |
                |NONE                 ON  ON                 |
                |--------------------------------------------|
                |LIVES    5                   OFF OFF        |
                |         3                   ON  OFF        |
                |         7                   OFF ON         |
                |         9                   ON  ON         |
                |--------------------------------------------|
                |DEMO SOUND  YES                      OFF    |
                |            NO                       ON     |
                |--------------------------------------------|
                |SPEAKER     STEREO                       OFF|
                |OUTPUT      MONO                          ON|
                |--------------------------------------------|
*/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/2203intf.h"
#include "sound/okim6295.h"
#include "includes/djboy.h"


/* KANEKO BEAST state */

WRITE8_MEMBER(djboy_state::beast_data_w)
{
	m_data_to_beast = data;
	m_z80_to_beast_full = 1;
	m_beast_int0_l = 0;
	m_beast->set_input_line(INPUT_LINE_IRQ0, ASSERT_LINE);
}

READ8_MEMBER(djboy_state::beast_data_r)
{
	m_beast_to_z80_full = 0;
	return m_data_to_z80;
}

READ8_MEMBER(djboy_state::beast_status_r)
{
	return (!m_beast_to_z80_full << 2) | (m_z80_to_beast_full << 3);
}

/******************************************************************************/

WRITE8_MEMBER(djboy_state::trigger_nmi_on_cpu0)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
}

WRITE8_MEMBER(djboy_state::cpu0_bankswitch_w)
{
	data ^= m_bankxor;
	membank("bank1")->set_entry(data);
	membank("bank4")->set_entry(0); /* unsure if/how this area is banked */
}

/******************************************************************************/

/**
 * xx------ msb scrollx
 * --x----- msb scrolly
 * ---x---- screen flip
 * ----xxxx bank
 */
WRITE8_MEMBER(djboy_state::cpu1_bankswitch_w)
{
	m_videoreg = data;

	switch (data & 0xf)
	{
	/* bs65.5y */
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
		membank("bank2")->set_entry((data & 0xf));
		break;

	/* bs101.6w */
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0b:
	case 0x0c:
	case 0x0d:
	case 0x0e:
	case 0x0f:
		membank("bank2")->set_entry((data & 0xf) - 4);
		break;

	default:
		break;
	}
}

WRITE8_MEMBER(djboy_state::coin_count_w)
{
	machine().bookkeeping().coin_counter_w(0, data & 1);
	machine().bookkeeping().coin_counter_w(1, data & 2);
}

/******************************************************************************/

WRITE8_MEMBER(djboy_state::trigger_nmi_on_sound_cpu2)
{
	soundlatch_byte_w(space, 0, data);
	m_cpu2->set_input_line(INPUT_LINE_NMI, PULSE_LINE);
} /* trigger_nmi_on_sound_cpu2 */

WRITE8_MEMBER(djboy_state::cpu2_bankswitch_w)
{
	membank("bank3")->set_entry(data);  // shall we check data<0x07?
}

/******************************************************************************/

static ADDRESS_MAP_START( cpu0_am, AS_PROGRAM, 8, djboy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xafff) AM_ROMBANK("bank4")
	AM_RANGE(0xb000, 0xbfff) AM_DEVREADWRITE("pandora", kaneko_pandora_device, spriteram_r, spriteram_w)
	AM_RANGE(0xc000, 0xdfff) AM_ROMBANK("bank1")
	AM_RANGE(0xe000, 0xefff) AM_RAM AM_SHARE("share1")
	AM_RANGE(0xf000, 0xf7ff) AM_RAM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu0_port_am, AS_IO, 8, djboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu0_bankswitch_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( cpu1_am, AS_PROGRAM, 8, djboy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xcfff) AM_RAM_WRITE(djboy_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0xd000, 0xd3ff) AM_RAM_WRITE(djboy_paletteram_w) AM_SHARE("paletteram")
	AM_RANGE(0xd400, 0xd8ff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu1_port_am, AS_IO, 8, djboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu1_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_sound_cpu2)
	AM_RANGE(0x04, 0x04) AM_READWRITE(beast_data_r, beast_data_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(djboy_scrolly_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(djboy_scrollx_w)
	AM_RANGE(0x0a, 0x0a) AM_WRITE(trigger_nmi_on_cpu0)
	AM_RANGE(0x0c, 0x0c) AM_READ(beast_status_r)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(coin_count_w)
ADDRESS_MAP_END

/******************************************************************************/

static ADDRESS_MAP_START( cpu2_am, AS_PROGRAM, 8, djboy_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xdfff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cpu2_port_am, AS_IO, 8, djboy_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(cpu2_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_device, read, write)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x06, 0x06) AM_DEVREADWRITE("oki1", okim6295_device, read, write)
	AM_RANGE(0x07, 0x07) AM_DEVREADWRITE("oki2", okim6295_device, read, write)
ADDRESS_MAP_END

/******************************************************************************/

READ8_MEMBER(djboy_state::beast_p0_r)
{
	// ?
	return 0;
}

WRITE8_MEMBER(djboy_state::beast_p0_w)
{
	if (!BIT(m_beast_p0, 1) && BIT(data, 1))
	{
		m_beast_to_z80_full = 1;
		m_data_to_z80 = m_beast_p1;
	}

	if (BIT(data, 0) == 1)
		m_z80_to_beast_full = 0;

	m_beast_p0 = data;
}

READ8_MEMBER(djboy_state::beast_p1_r)
{
	if (BIT(m_beast_p0, 0) == 0)
		return m_data_to_beast;
	else
		return 0; // ?
}

WRITE8_MEMBER(djboy_state::beast_p1_w)
{
	if (data == 0xff)
	{
		m_beast_int0_l = 1;
		m_beast->set_input_line(INPUT_LINE_IRQ0, CLEAR_LINE);
	}

	m_beast_p1 = data;
}

READ8_MEMBER(djboy_state::beast_p2_r)
{
	switch ((m_beast_p0 >> 2) & 3)
	{
		case 0: return ioport("IN1")->read();
		case 1: return ioport("IN2")->read();
		case 2: return ioport("IN0")->read();
		default: return 0xff;
	}
}

WRITE8_MEMBER(djboy_state::beast_p2_w)
{
	m_beast_p2 = data;
}

READ8_MEMBER(djboy_state::beast_p3_r)
{
	UINT8 dsw = 0;
	UINT8 dsw1 = ~ioport("DSW1")->read();
	UINT8 dsw2 = ~ioport("DSW2")->read();

	switch ((m_beast_p0 >> 5) & 3)
	{
		case 0: dsw = (BIT(dsw2, 4) << 3) | (BIT(dsw2, 0) << 2) | (BIT(dsw1, 4) << 1) | BIT(dsw1, 0); break;
		case 1: dsw = (BIT(dsw2, 5) << 3) | (BIT(dsw2, 1) << 2) | (BIT(dsw1, 5) << 1) | BIT(dsw1, 1); break;
		case 2: dsw = (BIT(dsw2, 6) << 3) | (BIT(dsw2, 2) << 2) | (BIT(dsw1, 6) << 1) | BIT(dsw1, 2); break;
		case 3: dsw = (BIT(dsw2, 7) << 3) | (BIT(dsw2, 3) << 2) | (BIT(dsw1, 7) << 1) | BIT(dsw1, 3); break;
	}
	return (dsw << 4) | (m_beast_int0_l << 2) | (m_beast_to_z80_full << 3);
}

WRITE8_MEMBER(djboy_state::beast_p3_w)
{
	m_beast_p3 = data;
	m_cpu1->set_input_line(INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);
}
/* Program/data maps are defined in the 8051 core */

static ADDRESS_MAP_START( djboy_mcu_io_map, AS_IO, 8, djboy_state )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READWRITE(beast_p0_r, beast_p0_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(beast_p1_r, beast_p1_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READWRITE(beast_p2_r, beast_p2_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(beast_p3_r, beast_p3_w)
ADDRESS_MAP_END

/******************************************************************************/

static INPUT_PORTS_START( djboy )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) /* labeled "TEST" in self test */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) /* punch */
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) /* kick */
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) /* jump */
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:1") /* Manual states "CAUTION  !! .... Don't use ." */
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Flip_Screen ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC(  0x04, IP_ACTIVE_HIGH, "SW1:3" )
//  PORT_DIPNAME( 0x04, 0x00, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW1:3")
//  PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
//  PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )      PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Coin_A ) )       PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Coin_B ) )       PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x00, DEF_STR( Difficulty ) )   PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x0c, 0x00, "Bonus Levels (in thousands)" ) PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, "10,30,50,70,90" )
	PORT_DIPSETTING(    0x04, "10,20,30,40,50,60,70,80,90" )
	PORT_DIPSETTING(    0x08, "20,50" )
	PORT_DIPSETTING(    0x0c, DEF_STR( None ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Lives ) )        PORT_DIPLOCATION("SW2:5,6")
	PORT_DIPSETTING(    0x10, "3" )
	PORT_DIPSETTING(    0x00, "5" )
	PORT_DIPSETTING(    0x20, "7" )
	PORT_DIPSETTING(    0x30, "9" )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "Stereo Sound" )      PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,
		8*32+0*4,8*32+1*4,8*32+2*4,8*32+3*4,8*32+4*4,8*32+5*4,8*32+6*4,8*32+7*4
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		16*32+0*32,16*32+1*32,16*32+2*32,16*32+3*32,16*32+4*32,16*32+5*32,16*32+6*32,16*32+7*32
	},
	4*8*32
};

static GFXDECODE_START( djboy )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x100, 16 ) /* sprite bank */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x000, 16 ) /* background tiles */
GFXDECODE_END

/******************************************************************************/

/* Main Z80 uses IM2 */
TIMER_DEVICE_CALLBACK_MEMBER(djboy_state::djboy_scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xfd);

	/* Pandora "sprite end dma" irq? TODO: timing is clearly off, attract mode relies on this */
	if(scanline == 64)
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, 0xff);
}

void djboy_state::machine_start()
{
	UINT8 *MAIN = memregion("maincpu")->base();
	UINT8 *CPU1 = memregion("cpu1")->base();
	UINT8 *CPU2 = memregion("cpu2")->base();

	membank("bank1")->configure_entries(0, 4,  &MAIN[0x00000], 0x2000);
	membank("bank1")->configure_entries(4, 28, &MAIN[0x10000], 0x2000);
	membank("bank2")->configure_entries(0, 2,  &CPU1[0x00000], 0x4000);
	membank("bank2")->configure_entries(2, 10, &CPU1[0x10000], 0x4000);
	membank("bank3")->configure_entries(0, 3,  &CPU2[0x00000], 0x4000);
	membank("bank3")->configure_entries(3, 5,  &CPU2[0x10000], 0x4000);
	membank("bank4")->configure_entry(0, &MAIN[0x10000]); /* unsure if/how this area is banked */

	save_item(NAME(m_videoreg));
	save_item(NAME(m_scrollx));
	save_item(NAME(m_scrolly));

	/* Kaneko BEAST */
	save_item(NAME(m_data_to_beast));
	save_item(NAME(m_data_to_z80));
	save_item(NAME(m_beast_to_z80_full));
	save_item(NAME(m_z80_to_beast_full));
	save_item(NAME(m_beast_int0_l));
	save_item(NAME(m_beast_p0));
	save_item(NAME(m_beast_p1));
	save_item(NAME(m_beast_p2));
	save_item(NAME(m_beast_p3));
}

void djboy_state::machine_reset()
{
	m_videoreg = 0;
	m_scrollx = 0;
	m_scrolly = 0;

	m_beast_int0_l = 1;
	m_beast_to_z80_full = 0;
	m_z80_to_beast_full = 0;
}

static MACHINE_CONFIG_START( djboy, djboy_state )

	MCFG_CPU_ADD("maincpu", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpu0_am)
	MCFG_CPU_IO_MAP(cpu0_port_am)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", djboy_state, djboy_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("cpu1", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpu1_am)
	MCFG_CPU_IO_MAP(cpu1_port_am)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", djboy_state,  irq0_line_hold)

	MCFG_CPU_ADD("cpu2", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(cpu2_am)
	MCFG_CPU_IO_MAP(cpu2_port_am)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", djboy_state,  irq0_line_hold)

	MCFG_CPU_ADD("beast", I80C51, 6000000)
	MCFG_CPU_IO_MAP(djboy_mcu_io_map)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))


	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(57.5)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 256-16-1)
	MCFG_SCREEN_UPDATE_DRIVER(djboy_state, screen_update_djboy)
	MCFG_SCREEN_VBLANK_DRIVER(djboy_state, screen_eof_djboy)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", djboy)
	MCFG_PALETTE_ADD("palette", 0x200)

	MCFG_DEVICE_ADD("pandora", KANEKO_PANDORA, 0)
	MCFG_KANEKO_PANDORA_GFXDECODE("gfxdecode")

	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)

	MCFG_OKIM6295_ADD("oki1", 12000000 / 8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_OKIM6295_ADD("oki2", 12000000 / 8, OKIM6295_PIN7_LOW)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END


ROM_START( djboy )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs64.4b",   0x00000, 0x08000, CRC(b77aacc7) SHA1(78100d4695738a702f13807526eb1bcac759cce3) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d",  0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs65.5y",   0x00000, 0x08000, CRC(0f1456eb) SHA1(62ed48c0d71c1fabbb3f6ada60381f57f692cef8) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w",  0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c",  0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "beast", 0 ) /* MSM80C51F microcontroller */
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) /* Internal ROM image */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07.1b",  0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) )  // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboya )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs19s.rom", 0x00000, 0x08000, CRC(17ce9f6c) SHA1(a0c1832b05dc46991e8949067ca0278f5498835f) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d",  0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs15s.rom", 0x00000, 0x08000, CRC(e6f966b2) SHA1(f9df16035a8b09d87eb70315b216892e25d99b03) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w",  0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c",  0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "beast", 0 ) /* MSM80C51F microcontroller */
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) /* Internal ROM image */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bs07.1b",  0x1f0000, 0x10000, CRC(d9b7a220) SHA1(ba3b528d50650c209c986268bb29b42ff1276eb2) )  // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs203.5j", 0x000000, 0x40000, CRC(805341fb) SHA1(fb94e400e2283aaa806814d5a39d6196457dc822) )
ROM_END

ROM_START( djboyj )
	ROM_REGION( 0x48000, "maincpu", 0 )
	ROM_LOAD( "bs12.4b",   0x00000, 0x08000, CRC(0971523e) SHA1(f90cd02cedf8632f4b651de7ea75dc8c0e682f6e) )
	ROM_CONTINUE( 0x10000, 0x18000 )
	ROM_LOAD( "bs100.4d",  0x28000, 0x20000, CRC(081e8af8) SHA1(3589dab1cf31b109a40370b4db1f31785023e2ed) )

	ROM_REGION( 0x38000, "cpu1", 0 )
	ROM_LOAD( "bs13.5y",   0x00000, 0x08000, CRC(5c3f2f96) SHA1(bb7ee028a2d8d3c76a78a29fba60bcc36e9399f5) )
	ROM_CONTINUE( 0x10000, 0x08000 )
	ROM_LOAD( "bs101.6w",  0x18000, 0x20000, CRC(a7c85577) SHA1(8296b96d5f69f6c730b7ed77fa8c93496b33529c) )

	ROM_REGION( 0x24000, "cpu2", 0 ) /* sound */
	ROM_LOAD( "bs200.8c",  0x00000, 0x0c000, CRC(f6c19e51) SHA1(82193f71122df07cce0a7f057a87b89eb2d587a1) )
	ROM_CONTINUE( 0x10000, 0x14000 )

	ROM_REGION( 0x1000, "beast", 0 ) /* MSM80C51F microcontroller */
	ROM_LOAD( "beast.9s", 0x00000, 0x1000, CRC(ebe0f5f3) SHA1(6081343c9b4510c4c16b71f6340266a1f76170ac) ) /* Internal ROM image */

	ROM_REGION( 0x200000, "gfx1", 0 ) /* sprites */
	ROM_LOAD( "bs000.1h", 0x000000, 0x80000, CRC(be4bf805) SHA1(a73c564575fe89d26225ca8ec2d98b6ac319ac18) )
	ROM_LOAD( "bs001.1f", 0x080000, 0x80000, CRC(fdf36e6b) SHA1(a8762458dfd5201304247c113ceb85e96e33d423) )
	ROM_LOAD( "bs002.1d", 0x100000, 0x80000, CRC(c52fee7f) SHA1(bd33117f7a57899fd4ec0a77413107edd9c44629) )
	ROM_LOAD( "bs003.1k", 0x180000, 0x80000, CRC(ed89acb4) SHA1(611af362606b73cd2cf501678b463db52dcf69c4) )
	ROM_LOAD( "bsxx.1b",  0x1f0000, 0x10000, CRC(22c8aa08) SHA1(5521c9d73b4ee82a2de1992d6edc7ef62788ad72) ) // replaces last 0x200 tiles

	ROM_REGION( 0x100000, "gfx2", 0 ) /* background */
	ROM_LOAD( "bs004.1s", 0x000000, 0x80000, CRC(2f1392c3) SHA1(1bc3030b3612766a02133eef0b4d20013c0495a4) )
	ROM_LOAD( "bs005.1u", 0x080000, 0x80000, CRC(46b400c4) SHA1(35f4823364bbff1fc935994498d462bbd3bc6044) )

	ROM_REGION( 0x40000, "oki1", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs-204.5j", 0x000000, 0x40000, CRC(510244f0) SHA1(afb502d46d268ad9cd209ae1da72c50e4e785626) )

	ROM_REGION( 0x40000, "oki2", 0 ) /* OKI-M6295 samples */
	ROM_LOAD( "bs-204.5j", 0x000000, 0x40000, CRC(510244f0) SHA1(afb502d46d268ad9cd209ae1da72c50e4e785626) )
ROM_END


DRIVER_INIT_MEMBER(djboy_state,djboy)
{
	m_bankxor = 0x00;
}

DRIVER_INIT_MEMBER(djboy_state,djboyj)
{
	m_bankxor = 0x1f;
}

/*     YEAR, NAME,  PARENT, MACHINE, INPUT, INIT, MNTR,  COMPANY, FULLNAME, FLAGS */
GAME( 1989, djboy,  0,      djboy,   djboy, djboy_state, djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (set 1)", MACHINE_SUPPORTS_SAVE) // Sammy & Williams logos in FG ROM
GAME( 1989, djboya, djboy,  djboy,   djboy, djboy_state, djboy,    ROT0, "Kaneko (American Sammy license)", "DJ Boy (set 2)", MACHINE_SUPPORTS_SAVE) // Sammy & Williams logos in FG ROM
GAME( 1989, djboyj, djboy,  djboy,   djboy, djboy_state, djboyj,   ROT0, "Kaneko (Sega license)", "DJ Boy (Japan)", MACHINE_SUPPORTS_SAVE ) // Sega logo in FG ROM
