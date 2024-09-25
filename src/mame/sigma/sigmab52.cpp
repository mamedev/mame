// license:BSD-3-Clause
// copyright-holders:Roberto Fresca, Sandro Ronco
/******************************************************************************

  SIGMA B52 SYSTEM.
  -----------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Joker's Wild (B52 system, BP55114-V1104, Ver.054NMV),               199?, Sigma.
  * Joker's Wild (B52 system, BP55114-V1104, Ver.054NMV, Harrah's GFX), 199?, Sigma.
  * Joker's Wild (B52 system, WP02001-054, Ver.031WM),                  1989, Sigma.
  * Super 8 Ways FC (DB98103-011, Fruit combination),                   1989, Sigma.


*******************************************************************************

  Hardware Notes:
  ---------------

  CPU:

  - 2x MC68B09P          ; 6809 CPU @ 2 MHz, from Motorola.
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

  ------------------------
  *****  Memory Map  *****
  ------------------------

  Main CPU:
  ---------

  0x0000 - 0x3FFF    ; NVRAM
  0x4000 - 0x7FFF    ; R.Bank
  0x8000 - 0xF6FF    ; ROM

  0xF700 - 0xF700    ; ACIA? (W)
  0xF701 - 0xF701    ; Unknown (R)
  0xF710 - 0xF710    ; Bank selector

  0xF720 - 0xF727    ; PTM6840
  0xF730 - 0xF731    ; ACRTC

  0xF740 - 0xF747    ; I/O

  0xF750 - 0xF750    ; Palette Bank
  0xF760 - 0xF760    ; Unknown (R)
  0xF770 - 0xF77F    ; Bill validator?
  0xF780 - 0xF780    ; Audio CPU IRQ
  0xF790 - 0xF790    ; Sound latch
  0xF7B0 - 0xF7B0    ; Coin enable
  0xF7B2 - 0xF7B7    ; Lamps 1
  0xF7C0 - 0xF7C3    ; Lamps 2
  0xF7D4 - 0xF7D4    ; Unknown (W)
  0xF7D5 - 0xF7D5    ; Hopper (W)
  0xF7D6 - 0xF7D7    ; Tower lamps

  0xF7E6 - 0xF7E6    ; Unknown (R/W)
  0xF7E7 - 0xF7E7    ; Unknown (R/W)

  0xF800 - 0xFFFF    ; ROM


  Audio CPU:
  ----------

  0x0000 - 0x1FFF    ; RAM
  0x6020 - 0x6027    ; PTM6840
  0x6030 - 0x6030    ; Audio CPU IRQ ack
  0x6050 - 0x6050    ; Sound latch

  0x6060 - 0x6061    ; YM3812

  0x8000 - 0xFFFF    ; ROM space.


*******************************************************************************

  TODO:

  - Verify clocks.
  - Bill validator.
  - ACIA
  - Some unknown R/W...


*******************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/gen_latch.h"
#include "machine/nvram.h"
#include "sound/ymopl.h"
#include "video/hd63484.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "sigmab52.lh"


namespace {

class sigmab52_state : public driver_device
{
public:
	sigmab52_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_6840ptm_2(*this, "6840ptm_2"),
		m_palette(*this, "palette"),
		m_bank1(*this, "bank1"),
		m_prom(*this, "proms"),
		m_in0(*this, "IN0"),
		m_lamps(*this, "lamp%u", 0U),
		m_towerlamps(*this, "towerlamp%u", 0U)
	{ }

	void jwildb52(machine_config &config);

	void init_jwildb52();

	DECLARE_INPUT_CHANGED_MEMBER(coin_drop_start);

private:
	uint8_t unk_f700_r();
	uint8_t unk_f760_r();
	uint8_t in0_r();
	void bank1_w(uint8_t data);
	void palette_bank_w(uint8_t data);
	void audiocpu_cmd_irq_w(uint8_t data);
	void audiocpu_irq_ack_w(uint8_t data);
	void hopper_w(uint8_t data);
	void lamps1_w(offs_t offset, uint8_t data);
	void lamps2_w(offs_t offset, uint8_t data);
	void tower_lamps_w(offs_t offset, uint8_t data);
	void coin_enable_w(uint8_t data);
	void ptm2_irq(int state);
	void audiocpu_irq_update();

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void jwildb52_hd63484_map(address_map &map) ATTR_COLD;
	void jwildb52_map(address_map &map) ATTR_COLD;
	void sound_prog_map(address_map &map) ATTR_COLD;

	required_device<cpu_device>     m_maincpu;
	required_device<cpu_device>     m_audiocpu;
	required_device<ptm6840_device> m_6840ptm_2;
	required_device<palette_device> m_palette;
	required_memory_bank            m_bank1;
	required_region_ptr<uint8_t>    m_prom;
	required_ioport                 m_in0;
	output_finder<10>               m_lamps;
	output_finder<2>                m_towerlamps;

	uint64_t    m_coin_start_cycles;
	uint64_t    m_hopper_start_cycles;
	int         m_audiocpu_cmd_irq;
};


/*********************************************
*               Misc. Handlers               *
*********************************************/

void sigmab52_state::audiocpu_irq_update()
{
	m_audiocpu->set_input_line(M6809_IRQ_LINE, (m_6840ptm_2->irq_state() || m_audiocpu_cmd_irq) ? ASSERT_LINE : CLEAR_LINE);
}

void sigmab52_state::ptm2_irq(int state)
{
	audiocpu_irq_update();
}

uint8_t sigmab52_state::unk_f700_r()
{
	return 0x7f;
}

uint8_t sigmab52_state::unk_f760_r()
{
	return 0x80;    // used for test the sound CPU
}

uint8_t sigmab52_state::in0_r()
{
	uint8_t data = 0xff;

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

	uint16_t in0 = m_in0->read();
	for (int i = 0; i < 16; i++)
		if (!BIT(in0, i))
		{
			data &= ~(i << 4);
			break;
		}

	return data;
}

void sigmab52_state::bank1_w(uint8_t data)
{
	m_bank1->set_entry(BIT(data, 7));
}

void sigmab52_state::hopper_w(uint8_t data)
{
	m_hopper_start_cycles = data & 0x01 ? m_maincpu->total_cycles() : 0;
}

void sigmab52_state::lamps1_w(offs_t offset, uint8_t data)
{
	m_lamps[offset] = data & 1;
}

void sigmab52_state::lamps2_w(offs_t offset, uint8_t data)
{
	m_lamps[6 + offset] = data & 1;
}

void sigmab52_state::tower_lamps_w(offs_t offset, uint8_t data)
{
	m_towerlamps[offset] = data & 1;
}

void sigmab52_state::coin_enable_w(uint8_t data)
{
	machine().bookkeeping().coin_lockout_w(0, data & 0x01 ? 0 : 1);
}

void sigmab52_state::audiocpu_cmd_irq_w(uint8_t data)
{
	m_audiocpu_cmd_irq = ASSERT_LINE;
	audiocpu_irq_update();
}

void sigmab52_state::audiocpu_irq_ack_w(uint8_t data)
{
	if (data & 0x01)
	{
		m_audiocpu_cmd_irq = CLEAR_LINE;
		audiocpu_irq_update();
	}
}

void sigmab52_state::palette_bank_w(uint8_t data)
{
	int bank = data & 0x0f;

	for (int i = 0; i < m_palette->entries(); i++)
	{
		uint8_t d = m_prom[(bank << 4) | i];
		m_palette->set_pen_color(i, pal3bit(d >> 5), pal3bit(d >> 2), pal2bit(d >> 0));
	}
}


/*********************************************
*           Memory Map Information           *
*********************************************/

void sigmab52_state::jwildb52_map(address_map &map)
{
	map(0x0000, 0x3fff).ram().share("nvram");
	map(0x4000, 0x7fff).bankr("bank1");

	map(0x8000, 0xf6ff).rom();

	map(0xf700, 0xf700).r(FUNC(sigmab52_state::unk_f700_r));    // ACIA ???
	map(0xf710, 0xf710).w(FUNC(sigmab52_state::bank1_w));

	map(0xf720, 0xf727).rw("6840ptm_1", FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));

	map(0xf730, 0xf731).rw("hd63484", FUNC(hd63484_device::read8), FUNC(hd63484_device::write8));

	map(0xf740, 0xf740).r(FUNC(sigmab52_state::in0_r));
	map(0xf741, 0xf741).portr("IN1");
	map(0xf742, 0xf742).portr("IN2");
	map(0xf743, 0xf743).portr("DSW1");
	map(0xf744, 0xf744).portr("DSW2");
	map(0xf745, 0xf745).portr("DSW3");
	map(0xf746, 0xf746).portr("DSW4");
	map(0xf747, 0xf747).portr("IN3");
	map(0xf750, 0xf750).w(FUNC(sigmab52_state::palette_bank_w));

	map(0xf760, 0xf760).r(FUNC(sigmab52_state::unk_f760_r));

//  map(0xf770, 0xf77f)  Bill validator

	map(0xf780, 0xf780).w(FUNC(sigmab52_state::audiocpu_cmd_irq_w));
	map(0xf790, 0xf790).w("soundlatch", FUNC(generic_latch_8_device::write));

	map(0xf7b0, 0xf7b0).w(FUNC(sigmab52_state::coin_enable_w));
	map(0xf7b2, 0xf7b7).w(FUNC(sigmab52_state::lamps1_w));
	map(0xf7c0, 0xf7c3).w(FUNC(sigmab52_state::lamps2_w));
	map(0xf7d5, 0xf7d5).w(FUNC(sigmab52_state::hopper_w));
	map(0xf7d6, 0xf7d7).w(FUNC(sigmab52_state::tower_lamps_w));

	map(0xf800, 0xffff).rom();
}

/* Unknown R/W:

  F700  W
  F701 R

  F7D4  W

  F7E6 RW
  F7E7 RW

*/

void sigmab52_state::sound_prog_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x6020, 0x6027).rw(m_6840ptm_2, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0x6030, 0x6030).w(FUNC(sigmab52_state::audiocpu_irq_ack_w));
	map(0x6050, 0x6050).r("soundlatch", FUNC(generic_latch_8_device::read));
	map(0x6060, 0x6061).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x8000, 0xffff).rom().region("audiocpu", 0);
}


void sigmab52_state::jwildb52_hd63484_map(address_map &map)
{
	map(0x00000, 0x1ffff).ram();
	map(0x20000, 0x3ffff).rom().region("gfx1", 0);
}

INPUT_CHANGED_MEMBER( sigmab52_state::coin_drop_start )
{
	if (newval && !m_coin_start_cycles)
		m_coin_start_cycles = m_maincpu->total_cycles();
}


/*********************************************
*                Input Ports                 *
*********************************************/

static INPUT_PORTS_START( jwildb52 )
	PORT_START("IN0")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Double")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x50, EQUALS, 0x00)  PORT_NAME("Collect / Payout")

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("Deal / Draw")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)  PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_CONDITION("DSW1", 0x50, EQUALS, 0x10)

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_GAMBLE_D_UP )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Double")
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Deal")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Draw")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x40)  PORT_NAME("Collect")

	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)  PORT_NAME("Deal")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_GAMBLE_DEAL )    PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)  PORT_NAME("Draw")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )         PORT_CONDITION("DSW1", 0x50, EQUALS, 0x50)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_UNKNOWN ) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Meter")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MEMORY_RESET ) PORT_NAME("Reset")
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
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, sigmab52_state, coin_drop_start, 0)

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
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start")
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON1 )        PORT_NAME("Max Bet")
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_GAMBLE_BET )     PORT_NAME("One Bet")
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )  PORT_NAME("Collect / Payout")
INPUT_PORTS_END


/*********************************************
*           Machine Start & Reset            *
*********************************************/

void sigmab52_state::machine_start()
{
	m_bank1->configure_entries(0, 2, memregion("maincpu")->base(), 0x4000);
	m_lamps.resolve();
	m_towerlamps.resolve();
}

void sigmab52_state::machine_reset()
{
	m_bank1->set_entry(1);
	m_coin_start_cycles = 0;
	m_hopper_start_cycles = 0;
	m_audiocpu_cmd_irq = CLEAR_LINE;
}


/*********************************************
*              Machine Drivers               *
*********************************************/

void sigmab52_state::jwildb52(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, XTAL(8'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &sigmab52_state::jwildb52_map);

	MC6809(config, m_audiocpu, XTAL(8'000'000));
	m_audiocpu->set_addrmap(AS_PROGRAM, &sigmab52_state::sound_prog_map);

	ptm6840_device &ptm1(PTM6840(config, "6840ptm_1", XTAL(8'000'000) / 8));  // FIXME
	ptm1.irq_callback().set_inputline("maincpu", M6809_IRQ_LINE);

	PTM6840(config, m_6840ptm_2, XTAL(8'000'000) / 8);  // FIXME
	m_6840ptm_2->irq_callback().set(FUNC(sigmab52_state::ptm2_irq));

	NVRAM(config, "nvram", nvram_device::DEFAULT_NONE);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(1024, 1024);
	screen.set_visarea(0, 544-1, 0, 436-1);
	screen.set_screen_update("hd63484", FUNC(hd63484_device::update_screen));
	screen.set_palette(m_palette);

	HD63484(config, "hd63484", XTAL(8'000'000)).set_addrmap(0, &sigmab52_state::jwildb52_hd63484_map);

	PALETTE(config, m_palette).set_entries(16);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	GENERIC_LATCH_8(config, "soundlatch");

	YM3812(config, "ymsnd", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);
}


/*********************************************
*                  Rom Load                  *
*********************************************/

/* Joker's Wild
   BP55114-V1104, Ver.054NMV
   Modern cards set. Normal cardsback.
*/
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

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "jwildb52_nvram.bin", 0x0000, 0x4000, CRC(16f5841e) SHA1(cad2b5769aab032990fbf3125a9f958289864edd) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/* Joker's Wild
   BP55114-V1104, Ver.054NMV
   Modern cards set. Harrah's cardsback.
*/
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

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "jwildb52h_nvram.bin", 0x0000, 0x4000, CRC(d1dc18f9) SHA1(b02f975f11b98d79ec9c4d01a3b1f4a13612b2a1) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/* Joker's Wild
   WP02001-054, Ver.031WM
   Classic cards set.  Normal cardsback.
*/
ROM_START( jwildb52a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "sigm_wrk.bin", 0x00000, 0x10000, CRC(15c83c6c) SHA1(7a05bd94ea8b1ad051fbe6580a6550d4bb47dd15) )

	ROM_REGION16_BE( 0x40000, "gfx1", 0 )
	ROM_LOAD32_BYTE( "c-1416-1.ic45", 0x00003, 0x10000, CRC(02a0b517) SHA1(5a0818a174683f791ca885bfdfd7555616c80758) )
	ROM_LOAD32_BYTE( "c-1416-2.ic46", 0x00001, 0x10000, CRC(3196e486) SHA1(2d264e518083ff05d1a1eb7f8e1649feb70349e7) )
	ROM_LOAD32_BYTE( "c-1416-3.ic47", 0x00000, 0x10000, CRC(1c9a2939) SHA1(e18fdf9a656687db47ac00700e7721c3d8e800c5) )
	ROM_LOAD32_BYTE( "c-1416-4.ic48", 0x00002, 0x10000, CRC(7bd8bf78) SHA1(ddacbb75df14a343e69949dcaa14ce1a7ec8407a) )

	ROM_REGION( 0x8000, "audiocpu", 0 )  // No sound PROM dump. Using the one from parent set for now...
	ROM_LOAD( "sound-01-00.43", 0x0000, 0x8000, BAD_DUMP CRC(2712d44c) SHA1(295526b27676cd97cbf111d47305d63c2b3ea50d) )

	ROM_REGION(0x4000, "nvram", 0)  // Default NVRAM
	ROM_LOAD( "jwildb52a_nvram.bin", 0x0000, 0x4000, CRC(99d46902) SHA1(7194b475b34918fb3204617efc6b61415098b789) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, BAD_DUMP CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/* Super 8 Ways FC
   Fruit combination.
   DB98103-011.
*/
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

	ROM_REGION( 0x0100, "proms", 0 )  // No bipolar PROM dump. Using the one from jwildb52 for now...
	ROM_LOAD( "mb7118.41", 0x0000, 0x0100, CRC(b362f9e2) SHA1(3963b40389ed6584e4cd96ab48849552857d99af) )
ROM_END


/*********************************************
*                Driver Init                 *
*********************************************/

void sigmab52_state::init_jwildb52()
{
}

} // anonymous namespace


/*********************************************
*                Game Drivers                *
*********************************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     CLASS           INIT           ROT   COMPANY  FULLNAME                                                              FLAGS  LAYOUT
GAMEL( 199?, jwildb52,  0,        jwildb52, jwildb52, sigmab52_state, init_jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, BP55114-V1104, Ver.054NMV)",               0,     layout_sigmab52 )
GAMEL( 199?, jwildb52h, jwildb52, jwildb52, jwildb52, sigmab52_state, init_jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, BP55114-V1104, Ver.054NMV, Harrah's GFX)", 0,     layout_sigmab52 )
GAMEL( 1989, jwildb52a, jwildb52, jwildb52, jwildb52, sigmab52_state, init_jwildb52, ROT0, "Sigma", "Joker's Wild (B52 system, WP02001-054, Ver.031WM)",                  0,     layout_sigmab52 )
GAME ( 1989, s8waysfc,  0,        jwildb52, s8waysfc, sigmab52_state, init_jwildb52, ROT0, "Sigma", "Super 8 Ways FC (DB98103-011, Fruit combination)",                   MACHINE_NOT_WORKING )
