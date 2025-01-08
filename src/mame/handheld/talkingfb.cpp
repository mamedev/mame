// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Kevin Horton
/*******************************************************************************

Parker Brothers Superstar Lineup Talking Football

Hardware notes:
- PCB label KPT-410 REV B1 (*KPT = Kenner Parker Toys)
- 80C31 MCU @ 12MHz
- 2KB RAM, 256KB ROM, 2 cartridge slots
- two 20-button keypads

Like Baseball, The ROM chip has (C) Tonka.
The speech driver is by Mozer, the ROM data includes employees names.

The cartridge slots are probably meant for extra teams. I can't find anything
about it in the manual or any proof that cartridges were released for this.
Maybe the toy went out of production soon after release.

The game includes formation cards, refer to the manual on which number is which strategy.

TODO:
- add cartridge slots (no need if no carts exist?)
- verify keypad (everything seems ok, but "Pursue" and "Go For Turnover" might
  be swapped, their function is nearly identical so it's hard to test)

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs51/mcs51.h"
#include "sound/dac.h"

#include "speaker.h"


namespace {

class talkingfb_state : public driver_device
{
public:
	talkingfb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void talkingfb(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<mcs51_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_ioport_array<5> m_inputs;

	u8 m_bank = 0;
	u8 m_inp_mux = 0;

	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	void bank_w(u8 data);
	template<int Psen> u8 bank_r(offs_t offset);
	void input_w(u8 data);
	u8 input_r();
};

void talkingfb_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_bank));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void talkingfb_state::bank_w(u8 data)
{
	// d0-d2: upper rom bank
	// d3-d5: upper rom enable (bus conflict possible)
	// d3: cart slot 1, d4: cart slot 2, d5: internal rom
	m_bank = data;
}

template<int Psen>
u8 talkingfb_state::bank_r(offs_t offset)
{
	u32 hi = (m_bank & 7) << 15;
	u8 data = (m_bank & 0x20) ? 0xff : m_rom[offset | hi];

	// cartridge slots are only enabled if PSEN is high
	// TODO

	return data;
}

void talkingfb_state::input_w(u8 data)
{
	// d3-d7: input mux
	m_inp_mux = data >> 3;
}

u8 talkingfb_state::input_r()
{
	u8 data = 0;

	// multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(~m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void talkingfb_state::main_map(address_map &map)
{
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).r(FUNC(talkingfb_state::bank_r<0>));
}

void talkingfb_state::main_io(address_map &map)
{
	map(0x0000, 0x07ff).mirror(0x3800).ram();
	map(0x4000, 0x4000).mirror(0x3fff).rw(FUNC(talkingfb_state::input_r), FUNC(talkingfb_state::input_w));
	map(0x8000, 0xffff).r(FUNC(talkingfb_state::bank_r<1>));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

/* keypad layout is like this: (P1 = Home, P2 = Away, both keypads are same)

OFFENSIVE             [1       [2      [3        DEFENSIVE
 ACTION                HB]      FB]     ST]       ACTION

[SCRAM/     [SCORE]   [4       [5      [6        [PURSUE]
 EVADE]                FL]      WR]     TE]

[PASS OUT/  [TIME     [7]      [8      [9]       [GO FOR
 RUN OUT]    OUT]               SE]               TURNOVER]

[CHECK OFF  [REPLAY]  [YES/    [0      [NO/
 TO BACK]              ENTER]   DEMO]   CANCEL]

*/

static INPUT_PORTS_START( talkingfb )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("P2 Pursue")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("P2 Scram / Evade")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("P2 Pass Out / Run Out")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("P2 Check Off To Back")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("P1 Pursue")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("P1 Scram / Evade")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("P1 Pass Out / Run Out")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("P1 Check Off To Back")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("P2 Go For Turnover")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("P2 Score")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("P2 Time Out")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("P2 Replay")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("P1 Go For Turnover")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("P1 Score")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("P1 Time Out")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("P1 Replay")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("P2 1 / HB")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("P2 4 / FL")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("P2 7")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("P2 Yes / Enter")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("P1 1 / HB")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("P1 4 / FL")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("P1 7")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("P1 Yes / Enter")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("P2 2 / FB")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("P2 5 / WR")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("P2 8 / SE")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("P2 0 / Demo")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("P1 2 / FB")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("P1 5 / WR")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("P1 8 / SE")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_NAME("P1 0 / Demo")

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("P2 3 / ST")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("P2 6 / TE")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("P2 9")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("P2 No / Cancel")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("P1 3 / ST")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("P1 6 / TE")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_NAME("P1 9")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("P1 No / Cancel")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void talkingfb_state::talkingfb(machine_config &config)
{
	// basic machine hardware
	I80C31(config, m_maincpu, 12_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &talkingfb_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &talkingfb_state::main_io);
	m_maincpu->port_out_cb<1>().set("dac", FUNC(dac_8bit_r2r_device::write));
	m_maincpu->port_out_cb<3>().set(FUNC(talkingfb_state::bank_w));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.5);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( talkingfb )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD("sp17492-001", 0x0000, 0x40000, CRC(d03851c6) SHA1(e8ac9c342bee987657c3c18f24d466b3906d6fb0) ) // Sharp LH532378
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS            INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, talkingfb, 0,      0,      talkingfb, talkingfb, talkingfb_state, empty_init, "Parker Brothers", "Superstar Lineup Talking Football", MACHINE_SUPPORTS_SAVE )
