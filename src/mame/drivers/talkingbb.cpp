// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton
/******************************************************************************

Parker Brothers Starting Lineup Talking Baseball

Hardware notes:
- PCB label KPT-349 REV 6C<1>
- 80C31 MCU @ 12MHz
- 2KB RAM, 128KB ROM, 2 cartridge slots
- 12 leds, 2 16-button keypads

ROM chip has (C) 1987 Tonka (Kenner Parker Toys was acquired by Tonka in 1987, Parker
Brothers itself was acquired by Kenner in 1985)

The speech driver is by Forrest S. Mozer's company.

The cartridge slots are for extra teams. There are around 8 cartridges, each one
included 3 or 4 baseball teams, with exception to the "Hall of Fame" cartridge.
Players enter 3-digit code to select the team.

TODO:
- add cartridge slots
- Buttons are unresponsive at initial game setup, you need to hold the yes/no button
  until it responds. The keypad reading routine is at $1A5D, it gets skipped for several
  seconds at a time. Once in-game, everything is fine though. BTANB or different cause?
- get rid of savestate workaround once outputs support savestates

*******************************************************************************

Field positions (used when substituting):

1: Pitcher
2: Catcher
3: First Base
4: Second Base
5: Third Base
6: Shortstop
7: Left Field
8: Center Field
9: Right Field

The game includes baseball cards with player stats. At the very least, the roster
cards are needed to play the game properly. See below for the default teams.

American Team:
--------------

Starting lineup batting order:

1: #23  Rickey Henderson, CF
2: #14  Don Mattingly, 1B
3: #18  Wade Boggs, 3B
4: #22  George Bell, LF
5: #24  Dave Winfield, RF
6: #16  Cal Ripken, SS
7: #11  Terry Kennedy, C
8: #15  Willie Randolph, 2B
9: #20  Alan Trammell, SS & DH
10:#27  Roger Clemens, P

Substitutes:

#19: George Brett, 3B
#12: Carlton Fisk, C
#25: Jack Morris, P
#13: Eddie Murray, 1B
#21: Kirby Puckett, OF
#30: Dan Quisenberry, P
#29: Dave Righetti, P
#28: Bret Saberhagen, P
#17: Lou Whitaker, 2B
#26: Robin Yount, SS

National Team:
--------------

Starting lineup batting order:

1: #23: Tim Raines, LF
2: #16: Ryne Sandberg, 2B
3: #22: Darryl Strawberry, RF
4: #19: Mike Schmidt, 3B
5: #13: Jack Clark, 1B
6: #20: Eric Davis, CF
7: #11: Gary Carter, C
8: #17: Ozzie Smith, SS
9: #18: Dale Murphy, OF & DH
10:#25: Mike Scott, P

Substitutes:

#15: Buddy Bell, 3B
#26: Jody Davis, C
#24: Andre Dawson, CF
#29: Dwight Gooden, P
#21: Tony Gwynn, OF
#14: Keith Hernandez, 1B
#30: Nolan Ryan, P
#12: Steve Sax, 2B
#28: Fernando Valenzuela, P
#27: Todd Worrell, P

******************************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "talkingbb.lh"


namespace {

class talkingbb_state : public driver_device
{
public:
	talkingbb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu"),
		m_delay_display(*this, "delay_display_%u", 0),
		m_out_led(*this, "led%u", 0U),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void talkingbb(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void device_post_load() override;

private:
	// devices/pointers
	required_device<mcs51_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device_array<timer_device, 12> m_delay_display;
	output_finder<12> m_out_led;
	required_ioport_array<5> m_inputs;

	void main_map(address_map &map);
	void main_io(address_map &map);

	u8 m_bank;
	u8 m_led_select;
	u8 m_led_cache[12];

	void set_led(int i, int value) { m_led_cache[i] = m_out_led[i] = value; }
	TIMER_DEVICE_CALLBACK_MEMBER(delay_display) { set_led(param, 0); }

	// I/O handlers
	DECLARE_WRITE8_MEMBER(bank_w);
	DECLARE_READ8_MEMBER(bank_r);
	DECLARE_WRITE8_MEMBER(input_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ8_MEMBER(switch_r);
};

void talkingbb_state::machine_start()
{
	// resolve handlers
	m_out_led.resolve();

	// zerofill
	m_bank = 0;
	m_led_select = 0;
	memset(m_led_cache, 0, sizeof(m_led_cache));

	// register for savestates
	save_item(NAME(m_bank));
	save_item(NAME(m_led_select));
	save_item(NAME(m_led_cache));
}

void talkingbb_state::device_post_load()
{
	for (int i = 0; i < 4; i++)
		m_out_led[i] = m_led_cache[i];
}



/******************************************************************************
    I/O
******************************************************************************/

WRITE8_MEMBER(talkingbb_state::bank_w)
{
	// d0-d1: upper rom bank
	// d2-d4: upper rom enable (bus conflict possible)
	// d2: internal rom, d3: cart slot 1, d4: cart slot 2
	m_bank = data;
}

READ8_MEMBER(talkingbb_state::bank_r)
{
	u32 hi = (~m_bank & 3) << 15;
	u8 data = (m_bank & 4) ? 0xff : m_rom[offset | hi];

	// TODO: cartridge

	return data;
}

WRITE8_MEMBER(talkingbb_state::input_w)
{
	// no effect if P30 is low (never happens though)
	if (~m_bank & 1)
		return;

	// d4-d7: led select (also input mux)
	// d0-d2: led data
	for (int i = 0; i < 12; i++)
	{
		u8 prev = BIT(m_led_select >> 4, i & 3) & BIT(~m_led_select, i >> 2);
		u8 cur = BIT(data >> 4, i & 3) & BIT(~data, i >> 2);

		// they're strobed, so on falling edge, delay them going off to prevent flicker or stuck display
		if (prev & ~cur)
			m_delay_display[i]->adjust(attotime::from_msec(50), i);
		else if (cur)
			set_led(i, 1);
	}

	m_led_select = data;
}

READ8_MEMBER(talkingbb_state::input_r)
{
	// open bus if P30 is low (never happens though)
	if (~m_bank & 1)
		return 0xff;

	u8 data = 0;

	// multiplexed inputs
	for (int i = 0; i < 4; i++)
		if (BIT(m_led_select >> 4, i))
			data |= m_inputs[i]->read();

	return ~data;
}

READ8_MEMBER(talkingbb_state::switch_r)
{
	// d5: mode switch
	return ~m_inputs[4]->read();
}



/******************************************************************************
    Address Maps
******************************************************************************/

void talkingbb_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom().region("maincpu", 0x18000);
	map(0x8000, 0xffff).r(FUNC(talkingbb_state::bank_r));
}

void talkingbb_state::main_io(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x7800).ram();
	map(0x8000, 0x8000).mirror(0x7fff).rw(FUNC(talkingbb_state::input_r), FUNC(talkingbb_state::input_w));
}



/******************************************************************************
    Input Ports
******************************************************************************/

/* keypad layout:        **** P1 = Batter ****  |  **** P2 = Pitcher ****
                                                |
[PINCH   [1       [2]       [3       [TIME OUT] |   [INFIELD  [1        [2        [3        [TIME OUT]
 HITTER]  BUNT]              POWER]             |    IN]       FAST]     CURVE]    CHANGE]
                                                |
[SCORE]  [4]      [5        [6]      [NO        |   [SCORE]   [4        [5        [6        [NO
                   SQUEEZE]           CANCEL]   |              PTCHOUT]  PICKOFF]  INTWALK]  CANCEL]
                                                |
[PINCH   [7       [8]       [9       [YES       |   [INSTANT  [7        [8]       [9        [YES
 RUNNER]  STEAL]             X-BASE]  ENTER]    |    REPLAY]   RELIEF]             SUB]      ENTER]
                                                |
                  [0                            |                       [0
                   SWING]                       |                        BALL]
*/

static INPUT_PORTS_START( talkingbb )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_R) PORT_NAME("P1 3 / Power")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_F) PORT_NAME("P1 6")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_V) PORT_NAME("P1 9 / Extra Base")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("P1 Time Out")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_O) PORT_NAME("P2 3 / Change")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_L) PORT_NAME("P2 6 / Intentional Walk")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("P2 9 / Sub")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("P2 Time Out")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_E) PORT_NAME("P1 2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_D) PORT_NAME("P1 5 / Squeeze")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_C) PORT_NAME("P1 8")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_LALT) PORT_NAME("P1 0 / Swing")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_I) PORT_NAME("P2 2 / Curve")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_K) PORT_NAME("P2 5 / Pickoff")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("P2 8")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RALT) PORT_NAME("P2 0 / Ball")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_W) PORT_NAME("P1 1 / Bunt")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_S) PORT_NAME("P1 4")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_X) PORT_NAME("P1 7 / Steal")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("P1 No / Cancel")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_U) PORT_NAME("P2 1 / Fast")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_J) PORT_NAME("P2 4 / Pitchout")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_M) PORT_NAME("P2 7 / Relief")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("P2 No / Cancel")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("P1 Pinch Hitter")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("P1 Score")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 Pinch Runner")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("P1 Yes / Enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("P2 Infield In")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("P2 Score")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("P2 Instant Replay")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P2 Yes / Enter")

	PORT_START("IN.4")
	PORT_CONFNAME( 0x20, 0x00, "Mode" )
	PORT_CONFSETTING(    0x00, "Player" )
	PORT_CONFSETTING(    0x20, "Coach" )
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void talkingbb_state::talkingbb(machine_config &config)
{
	/* basic machine hardware */
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &talkingbb_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &talkingbb_state::main_io);
	m_maincpu->port_out_cb<1>().set("dac", FUNC(dac_8bit_r2r_device::write));
	m_maincpu->port_out_cb<3>().set(FUNC(talkingbb_state::bank_w));
	m_maincpu->port_in_cb<3>().set(FUNC(talkingbb_state::switch_r));

	/* video hardware */
	for (int i = 0; i < 12; i++)
		TIMER(config, m_delay_display[i]).configure_generic(FUNC(talkingbb_state::delay_display));

	config.set_default_layout(layout_talkingbb);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.5);
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac", -1.0, DAC_VREF_NEG_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( talkingbb )
	ROM_REGION( 0x20000, "maincpu", 0 )
	ROM_LOAD("sp17208-002", 0x0000, 0x20000, CRC(e0e56217) SHA1(24a06b5c94a5c750799f61e3eb865e02d6cea68a) ) // GM231000-110
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME        PARENT CMP MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
CONS( 1988, talkingbb,  0,      0, talkingbb, talkingbb, talkingbb_state, empty_init, "Parker Brothers", "Starting Lineup Talking Baseball", MACHINE_SUPPORTS_SAVE )
