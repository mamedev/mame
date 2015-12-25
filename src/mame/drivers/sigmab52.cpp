// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Sandro Ronco
/******************************************************************************

  SIGMA B52 SYSTEM.
  -----------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Joker's Wild (B52 system, set 1),        199?, Sigma.
  * Joker's Wild (B52 system, set 2),        199?, Sigma.
  * Joker's Wild (B52 system, Harrah's GFX), 199?, Sigma.


  The HD63484 ACRTC support is a bit hacky and incomplete,
  due to its preliminary emulation state.


*******************************************************************************

  Hardware Notes:
  ---------------

  CPU:

  - 2x MC68B09CP         ; 6809 CPU @ 2 MHz, from Motorola.
  - 1x HD63484P8 @ 8MHz  ; Advanced CRT controller (ACRTC), from Hitachi Semiconductor.

  RAM devices:

  - 8x TC51832ASPL-10    ; 32K x 8-bit CMOS Pseudo Static RAM.
  - 1x HM62256ALP-10     ; 32K x 8-bit High Speed CMOS Static RAM.
  - 1x HM6264ALP-10      ; 8K x 8-bit High Speed CMOS Static RAM.

  ROM devices:

  - 1x 64K main program ROM.
  - 4x 64K graphics ROM.
  - 1x 32K sound program ROM.
  - 1x 256 bytes bipolar PROM.

  Sound device:

  - 1x YM3812            ; Sound IC, from Yamaha.

  Other:

  - 2x EF68B40P          ; Frequency clock 2 MHz Programmable Timer, from SGS-Thomson Microelectronics.
  - 1x EF68B50P          ; Asynchronous Communications Interface Adapter (ACIA, 2 MHz), from SGS-Thomson Microelectronics.

  - 1x Xtal @ 18 MHz.
  - 1x Xtal @ 8 MHz.
  - 1x Xtal @ 3.579545 MHz.


  Silkscreened on main PCB:

  "SIGMA GAME INC."
  "VIDEO PCB 340016"
  "REV. A"

  Silkscreened on daughterboard:

  "SIGMA GAMES   340003"
  "BILL VALID BOARD"


  - Seems that you can set the node (01-32) for a network.
  - Cards graphics from set 2 have the Harrah's Casino logo.


*******************************************************************************


  *** Game Notes ***

  Nothing yet...


*******************************************************************************

  ---------------------------------
  ***  Memory Map (preliminary) ***
  ---------------------------------

  0x0000 - 0x3FFF    ; RAM.
  0xF730 - 0xF731    ; ACRTC.
  0xF740 - 0xF746    ; I/O.

  Sound:

  0x8000 - 0xFFFF    ; ROM space.


*******************************************************************************


  DRIVER UPDATES:


  [2010-02-04]

  - Initial release.
  - Pre-defined Xtals.
  - Started a preliminary memory map.
  - Hooked both CPUs.
  - Preliminary ACRTC support.
  - Added technical notes.


  TODO:

  - Improve memory map.
  - Layout.
  - Bill validator.
  - ACIA


*******************************************************************************/


#define MAIN_CLOCK  XTAL_18MHz
#define SEC_CLOCK   XTAL_8MHz
#define AUX_CLOCK   XTAL_3_579545MHz

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/nvram.h"
#include "sound/3812intf.h"
#include "video/h63484.h"

#include "sigmab52.lh"

class sigmab52_state : public driver_device
{
public:
	sigmab52_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_6840ptm_2(*this, "6840ptm_2"),
		m_palette(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_prom(*this, "proms"),
		m_in0(*this, "IN0")
	{ }

	DECLARE_READ8_MEMBER(unk_f700_r);
	DECLARE_READ8_MEMBER(unk_f760_r);
	DECLARE_READ8_MEMBER(in0_r);
	DECLARE_WRITE8_MEMBER(bank1_w);
	DECLARE_WRITE8_MEMBER(palette_bank_w);
	DECLARE_WRITE8_MEMBER(audiocpu_cmd_irq_w);
	DECLARE_WRITE8_MEMBER(audiocpu_irq_ack_w);
	DECLARE_WRITE8_MEMBER(hopper_w);
	DECLARE_WRITE8_MEMBER(lamps1_w);
	DECLARE_WRITE8_MEMBER(lamps2_w);
	DECLARE_WRITE8_MEMBER(tower_lamps_w);
	DECLARE_WRITE8_MEMBER(coin_enable_w);
	DECLARE_DRIVER_INIT(jwildb52);
	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);
	DECLARE_WRITE_LINE_MEMBER(ptm2_irq);
	void audiocpu_irq_update();
	virtual void machine_start() override;
	virtual void machine_reset() override;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<ptm6840_device> m_6840ptm_2;
	required_device<palette_device> m_palette;
	required_memory_bank m_bank1;
	required_region_ptr<UINT8> m_prom;
	required_ioport m_in0;

	UINT64      m_coin_start_cycles;
	UINT64      m_hopper_start_cycles;
	int         m_audiocpu_cmd_irq;
};


/*************************
*      Misc Handlers     *
*************************/

void sigmab52_state::audiocpu_irq_update()
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, (m_6840ptm_2->irq_state() || m_audiocpu_cmd_irq) ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER(sigmab52_state::ptm2_irq)
{
	audiocpu_irq_update();
}

READ8_MEMBER(sigmab52_state::unk_f700_r)
{
	return 0x7f;
}

READ8_MEMBER(sigmab52_state::unk_f760_r)
{
	return 0x80;    // used for test the sound CPU
}

READ8_MEMBER(sigmab52_state::in0_r)
{
	UINT8 data = 0xff;

	// if the hopper is active simulate the coin-out sensor
	if (m_hopper_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_hopper_start_cycles);

		if (diff > attotime::from_msec(100))
			data &= ~0x01;

		if (diff > attotime::from_msec(200))
			m_hopper_start_cycles = m_maincpu->total_cycles();
	}

	// simulates the passage of coins through multiple sensors
	if (m_coin_start_cycles)
	{
		attotime diff = m_maincpu->cycles_to_attotime(m_maincpu->total_cycles() - m_coin_start_cycles);

		if (diff > attotime::from_msec(20) && diff < attotime::from_msec(100))
			data &= ~0x02;
		if (diff > attotime::from_msec(50) && diff < attotime::from_msec(200))
			data &= ~0x04;

		if (diff > attotime::from_msec(200))
			m_coin_start_cycles = 0;
	}

	UINT16 in0 = m_in0->read();
	for(int i=0; i<16; i++)
		if (!BIT(in0, i))
		{
			data &= ~(i << 4);
			break;
		}

	return data;
}

WRITE8_MEMBER(sigmab52_state::bank1_w)
{
	m_bank1->set_entry(BIT(data, 7));
}

WRITE8_MEMBER(sigmab52_state::hopper_w)
{
	m_hopper_start_cycles = data & 0x01 ? m_maincpu->total_cycles() : 0;
}

WRITE8_MEMBER(sigmab52_state::lamps1_w)
{
	output_set_lamp_value(offset, data & 1);
}

WRITE8_MEMBER(sigmab52_state::lamps2_w)
{
	output_set_lamp_value(6 + offset, data & 1);
}

WRITE8_MEMBER(sigmab52_state::tower_lamps_w)
{
	output_set_indexed_value("towerlamp", offset, data & 1);
}

WRITE8_MEMBER(sigmab52_state::coin_enable_w)
{
	coin_lockout_w(machine(), 0, data & 0x01 ? 0 : 1);
}

WRITE8_MEMBER(sigmab52_state::audiocpu_cmd_irq_w)
{
	m_audiocpu_cmd_irq = ASSERT_LINE;
	audiocpu_irq_update();
}

WRITE8_MEMBER(sigmab52_state::audiocpu_irq_ack_w)
{
	if (data & 0x01)
	{
		m_audiocpu_cmd_irq = CLEAR_LINE;
		audiocpu_irq_update();
	}
}

WRITE8_MEMBER(sigmab52_state::palette_bank_w)
{
	int bank = data & 0x0f;

	for (int i = 0; i<m_palette->entries(); i++)
	{
		UINT8 d = m_prom[(bank << 4) | i];
		m_palette->set_pen_color(i, pal3bit(d >> 5), pal3bit(d >> 2), pal2bit(d >> 0));
	}
}

/*************************
*      Memory Maps       *
*************************/

static ADDRESS_MAP_START( jwildb52_map, AS_PROGRAM, 8, sigmab52_state )
	AM_RANGE(0x0000, 0x3fff) AM_RAM AM_SHARE("nvram")
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")

	AM_RANGE(0x8000, 0xf6ff) AM_ROM

	AM_RANGE(0xf700, 0xf700) AM_READ(unk_f700_r)    // ACIA ???
	AM_RANGE(0xf710, 0xf710) AM_WRITE(bank1_w)

	AM_RANGE(0xf720, 0xf727) AM_DEVREADWRITE("6840ptm_1", ptm6840_device, read, write)

	AM_RANGE(0xf730, 0xf730) AM_DEVREADWRITE("hd63484", h63484_device, status_r, address_w)
	AM_RANGE(0xf731, 0xf731) AM_DEVREADWRITE("hd63484", h63484_device, data_r, data_w)

	AM_RANGE(0xf740, 0xf740) AM_READ(in0_r)
	AM_RANGE(0xf741, 0xf741) AM_READ_PORT("IN1")
	AM_RANGE(0xf742, 0xf742) AM_READ_PORT("IN2")
	AM_RANGE(0xf743, 0xf743) AM_READ_PORT("DSW1")
	AM_RANGE(0xf744, 0xf744) AM_READ_PORT("DSW2")
	AM_RANGE(0xf745, 0xf745) AM_READ_PORT("DSW3")
	AM_RANGE(0xf746, 0xf746) AM_READ_PORT("DSW4")
	AM_RANGE(0xf747, 0xf747) AM_READ_PORT("IN3")
	AM_RANGE(0xf750, 0xf750) AM_WRITE(palette_bank_w)

	AM_RANGE(0xf760, 0xf760) AM_READ(unk_f760_r)

//  AM_RANGE(0xf770, 0xf77f)  Bill validator

	AM_RANGE(0xf780, 0xf780) AM_WRITE(audiocpu_cmd_irq_w)
	AM_RANGE(0xf790, 0xf790) AM_WRITE(soundlatch_byte_w)

	AM_RANGE(0xf7b0, 0xf7b0) AM_WRITE(coin_enable_w)
	AM_RANGE(0xf7d5, 0xf7d5) AM_WRITE(hopper_w)
	AM_RANGE(0xf7b2, 0xf7b7) AM_WRITE(lamps1_w)
	AM_RANGE(0xf7c0, 0xf7c3) AM_WRITE(lamps2_w)
	AM_RANGE(0xf7d6, 0xf7d7) AM_WRITE(tower_lamps_w)
	AM_RANGE(0xf800, 0xffff) AM_ROM
ADDRESS_MAP_END

/* Unknown R/W:

  F700  W
  F701 R

  F7D4  W

  F7E6 RW
  F7E7 RW

*/

static ADDRESS_MAP_START( sound_prog_map, AS_PROGRAM, 8, sigmab52_state )
	AM_RANGE(0x0000, 0x1fff) AM_RAM
	AM_RANGE(0x6020, 0x6027) AM_DEVREADWRITE("6840ptm_2", ptm6840_device, read, write)
	AM_RANGE(0x6030, 0x6030) AM_WRITE(audiocpu_irq_ack_w)
	AM_RANGE(0x6050, 0x6050) AM_READ(soundlatch_byte_r)
	AM_RANGE(0x6060, 0x6061) AM_DEVREADWRITE("ymsnd", ym3812_device, read, write)
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("audiocpu", 0)
ADDRESS_MAP_END

/* Unknown R/W:


*/

static ADDRESS_MAP_START( jwildb52_hd63484_map, AS_0, 16, sigmab52_state )
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0x20000, 0x3ffff) AM_ROM AM_REGION("gfx1", 0)
ADDRESS_MAP_END

INPUT_CHANGED_MEMBER( sigmab52_state::coin_drop_start )
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( jwildb52 )
	PORT_START("IN0")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Double")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Collect / Payout")

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_BET )      PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Double")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Deal")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Draw")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_BUTTON4 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Collect")

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)  PORT_NAME("Deal")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)  PORT_NAME("Draw")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_CANCEL  )  PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Meter")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("Reset")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("Last")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_DOOR ) PORT_NAME("Machine Door")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper Weight Switch")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )  // Hold 1

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Attendant Call")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )  // Hold 2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Drop Door")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )  // Hold 3
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )  // Hold 4
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )  // Hold 5
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Meter Wire")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_NAME("V Door")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, sigmab52_state, coin_drop_start, NULL)

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, "DSW1-1" )        PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW1-2" )        PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW1-3" )        PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW1-4" )        PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW1-5" )        PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW1-6" )        PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-7" )        PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-8" )        PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, "JW-1" )          PORT_DIPLOCATION("JW:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "JW-2" )          PORT_DIPLOCATION("JW:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "JW-3" )          PORT_DIPLOCATION("JW:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "JW-4" )          PORT_DIPLOCATION("JW:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "JW-5" )          PORT_DIPLOCATION("JW:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "JW-6" )          PORT_DIPLOCATION("JW:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW1-9" )        PORT_DIPLOCATION("SW1:9")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW1-10" )       PORT_DIPLOCATION("SW1:10")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, "DSW2-1" )        PORT_DIPLOCATION("SW2:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, "DSW2-2" )        PORT_DIPLOCATION("SW2:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, "DSW2-3" )        PORT_DIPLOCATION("SW2:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, "DSW2-4" )        PORT_DIPLOCATION("SW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, "DSW2-5" )        PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, "DSW2-6" )        PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-7" )        PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "DSW2-8" )        PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW4")
	PORT_BIT( 0x3f, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_DIPNAME( 0x40, 0x40, "DSW2-9" )        PORT_DIPLOCATION("SW2:9")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, "DSW2-10" )       PORT_DIPLOCATION("SW2:10")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END

static INPUT_PORTS_START( s8waysfc )
	PORT_INCLUDE( jwildb52 )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x07ff, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON3 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_NAME("Start")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 )        PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Collect / Payout")
INPUT_PORTS_END


/*************************
*     Machine Start      *
*************************/

void sigmab52_state::machine_start()
{
	m_bank1->configure_entries(0, 2, memregion("maincpu")->base(), 0x4000);
}

void sigmab52_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_coin_start_cycles = 0;
	m_hopper_start_cycles = 0;
	m_audiocpu_cmd_irq = CLEAR_LINE;
}

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( jwildb52, sigmab52_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MAIN_CLOCK/9)    /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(jwildb52_map)

	MCFG_CPU_ADD("audiocpu", M6809, MAIN_CLOCK/9)   /* 2 MHz */
	MCFG_CPU_PROGRAM_MAP(sound_prog_map)

	MCFG_DEVICE_ADD("6840ptm_1", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(MAIN_CLOCK/9)       // FIXME
	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", M6809_IRQ_LINE))

	MCFG_DEVICE_ADD("6840ptm_2", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(MAIN_CLOCK/18)      // FIXME
	MCFG_PTM6840_IRQ_CB(WRITELINE(sigmab52_state, ptm2_irq))

	MCFG_NVRAM_ADD_NO_FILL("nvram")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(30)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(1024, 1024)
	MCFG_SCREEN_VISIBLE_AREA(0, 544-1, 0, 436-1)
	MCFG_SCREEN_UPDATE_DEVICE("hd63484", h63484_device, update_screen)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_H63484_ADD("hd63484", SEC_CLOCK, jwildb52_hd63484_map)

	MCFG_PALETTE_ADD("palette", 16)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ymsnd", YM3812, AUX_CLOCK)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( jwildb52 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "poker.ic95", 0x00000, 0x10000, CRC(07eb9007) SHA1(ee814c40c6d8c9ea9e5246cae0cfa2c30f2976ed) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "cards_2001-1.ic45", 0x00003, 0x10000, CRC(7664455e) SHA1(c9f129060e63b9ac9058ab94208846e4dc578ead) )
	ROM_LOAD32_BYTE( "cards_2001-2.ic46", 0x00001, 0x10000, CRC(c1455d64) SHA1(ddb576ba471b5d2faa415ec425615cf5f9d87911) )
	ROM_LOAD32_BYTE( "cards_2001-3.ic47", 0x00000, 0x10000, CRC(cb2ece6e) SHA1(f2b6949085fe395d0fdd16322a880ec87e2efd50) )
	ROM_LOAD32_BYTE( "cards_2001-4.ic48", 0x00002, 0x10000, CRC(8131d236) SHA1(8984aa1f2af70df41973b61df17f184796a2ffe9) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "sound-01-00.43", 0x0000, 0x8000, CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


ROM_START( jwildb52a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sigm_wrk.bin", 0x00000, 0x10000, CRC(15c83c6c) SHA1(7a05bd94ea8b1ad051fbe6580a6550d4bb47dd15) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "c-1416-1.ic45", 0x00003, 0x10000, CRC(02a0b517) SHA1(5a0818a174683f791ca885bfdfd7555616c80758) )
	ROM_LOAD32_BYTE( "c-1416-2.ic46", 0x00001, 0x10000, CRC(3196e486) SHA1(2d264e518083ff05d1a1eb7f8e1649feb70349e7) )
	ROM_LOAD32_BYTE( "c-1416-3.ic47", 0x00000, 0x10000, CRC(1c9a2939) SHA1(e18fdf9a656687db47ac00700e7721c3d8e800c5) )
	ROM_LOAD32_BYTE( "c-1416-4.ic48", 0x00002, 0x10000, CRC(7bd8bf78) SHA1(ddacbb75df14a343e69949dcaa14ce1a7ec8407a) )

	/* No sound dumps. Using the ones from parent set for now... */

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "sound-01-00.43", 0x0000, 0x8000, BAD_DUMP CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, BAD_DUMP CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


ROM_START( jwildb52h )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jokers_wild_ver_xxx.ic95", 0x00000, 0x10000, CRC(07eb9007) SHA1(ee814c40c6d8c9ea9e5246cae0cfa2c30f2976ed) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "2006-1_harrahs.ic45", 0x00003, 0x10000, CRC(6e6871dc) SHA1(5dfc99c808c06ec34838324181988d4550c1ed1a) )
	ROM_LOAD32_BYTE( "2006-2_harrahs.ic46", 0x00001, 0x10000, CRC(1039c62d) SHA1(11f0dbcbbff5f6e9028a0305f7e16a0654be40d4) )
	ROM_LOAD32_BYTE( "2006-3_harrahs.ic47", 0x00000, 0x10000, CRC(d66af95a) SHA1(70bba1aeea9221541b82642045ce8ecf26e1d08c) )
	ROM_LOAD32_BYTE( "2006-4_harrahs.ic48", 0x00002, 0x10000, CRC(2bf196cb) SHA1(686ca0dd84c48f51efee5349ea3db65531dd4a52) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "poker-01-00.43", 0x0000, 0x8000, CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


ROM_START( s8waysfc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "dv98103.011", 0x00000, 0x10000, CRC(416190a1) SHA1(e2738644efc6c2adcea2470b482f3f818ed9af8d) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "symb112.1", 0x00003, 0x10000, CRC(b09bd4f5) SHA1(af04845e84cb381f9babe088884b5bbab927a326) )
	ROM_LOAD32_BYTE( "symb112.2", 0x00001, 0x10000, CRC(462a2d55) SHA1(3157893d150b98c80c0045f78cb2520e8b3ce4eb) )
	ROM_LOAD32_BYTE( "symb112.3", 0x00000, 0x10000, CRC(be0c2e64) SHA1(82de83fc4754ff73e80e22187b7fba041832613e) )
	ROM_LOAD32_BYTE( "symb112.4", 0x00002, 0x10000, CRC(f9d8529c) SHA1(7cd54bda71fb38c7bcbea42be4e322aec0581964) )

	ROM_REGION( 0x8000, "audiocpu", 0 )
	ROM_LOAD( "v-slot02.00", 0x00000, 0x08000, CRC(bc1eec0a) SHA1(300ebfbd314c58b434bb20a5c3c8f7463b424207) )

	/* No prom dumps. Using the ones from jwildb52 for now... */

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/*************************
*      Driver Init       *
*************************/

DRIVER_INIT_MEMBER(sigmab52_state,jwildb52)
{
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME       PARENT    MACHINE   INPUT     INIT      ROT    COMPANY  FULLNAME                                  FLAGS */
GAMEL( 199?, jwildb52,  0,        jwildb52, jwildb52, sigmab52_state, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, set 1)",        MACHINE_NOT_WORKING, layout_sigmab52 )
GAMEL( 199?, jwildb52a, jwildb52, jwildb52, jwildb52, sigmab52_state, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, set 2)",        MACHINE_NOT_WORKING, layout_sigmab52 )
GAMEL( 199?, jwildb52h, jwildb52, jwildb52, jwildb52, sigmab52_state, jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, Harrah's GFX)", MACHINE_NOT_WORKING, layout_sigmab52 )
GAME ( 199?, s8waysfc,  0,        jwildb52, s8waysfc, sigmab52_state, jwildb52, ROT0, "Sigma", "Super 8 Ways FC (Fruit combination)",     MACHINE_NOT_WORKING )
