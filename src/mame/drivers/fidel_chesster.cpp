// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard
/******************************************************************************

Fidelity Chesster (model 6120)
There is also a German version titled Kishon Chesster (model 6120G, or 6127)
----------------

8*(8+1) buttons, 8+8+1 LEDs
8KB RAM(UM6264-12), 32KB ROM(M27C256B)
Ricoh RP65C02G CPU, 5MHz XTAL
8-bit DAC speech timed via IRQ, 128KB ROM(AMI custom label)
PCB label 510.1141C01

I/O is via TTL, memory map is similar to Designer Display

The speech technology was invented by Forrest S. Mozer(same person that invented
the S14001A in the 70s), this time a 65C02 software solution.

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m6502/r65c02.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_chesster.lh" // clickable


namespace {

class chesster_state : public fidelbase_state
{
public:
	chesster_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

	// machine drivers
	void chesster(machine_config &config);
	void kishon(machine_config &config);

	void init_chesster();

private:
	void main_map(address_map &map);

	int m_numbanks;

	// I/O handlers
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(input_r);
};

void chesster_state::init_chesster()
{
	m_numbanks = memregion("rombank")->bytes() / 0x4000;
	m_rombank->configure_entries(0, m_numbanks, memregion("rombank")->base(), 0x4000);
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL/generic

WRITE8_MEMBER(chesster_state::control_w)
{
	// a0-a2,d7: 74259(1)
	u8 mask = 1 << offset;
	m_led_select = (m_led_select & ~mask) | ((data & 0x80) ? mask : 0);

	// 74259 Q4-Q7: 7442 a0-a3
	// 7442 0-8: led data, input mux
	u16 sel = 1 << (m_led_select >> 4 & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;

	// 74259 Q0,Q1: led select (active low)
	display_matrix(9, 2, m_inp_mux, ~m_led_select & 3);

	// 74259 Q2,Q3: speechrom A14,A15
	// a0-a2,d0: 74259(2) Q3,Q2,Q0 to A16,A17,A18
	m_speech_bank = (m_speech_bank & ~mask) | ((data & 1) ? mask : 0);
	u8 bank = (m_led_select >> 2 & 3) | bitswap<3>(m_speech_bank, 0,2,3) << 2;
	m_rombank->set_entry(bank & (m_numbanks - 1));
}

READ8_MEMBER(chesster_state::input_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void chesster_state::main_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).mirror(0x1ff8).rw(FUNC(chesster_state::input_r), FUNC(chesster_state::control_w));
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x6000, 0x6000).mirror(0x1fff).w("dac8", FUNC(dac_byte_interface::data_w));
	map(0x8000, 0xffff).rom();
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( chesster )
	PORT_INCLUDE( fidel_cb_buttons )

	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("Clear")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Move / No")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Hint / Yes")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Take Back / Repeat")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("Level / New")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("Option / Replay")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Verify / Problem")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Shift")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void chesster_state::chesster(machine_config &config)
{
	/* basic machine hardware */
	R65C02(config, m_maincpu, 5_MHz_XTAL); // RP65C02G
	m_maincpu->set_addrmap(AS_PROGRAM, &chesster_state::main_map);

	const attotime irq_period = attotime::from_hz(9615); // R/C circuit, measured
	TIMER(config, m_irq_on).configure_periodic(FUNC(chesster_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_nsec(2600)); // active for 2.6us
	TIMER(config, "irq_off").configure_periodic(FUNC(chesster_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_chesster);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_8BIT_R2R(config, "dac8", 0).add_route(ALL_OUTPUTS, "speaker", 0.5); // m74hc374b1.ic1 + 8l513_02.z2
	voltage_regulator_device &vref(VOLTAGE_REGULATOR(config, "vref"));
	vref.add_route(0, "dac8", 1.0, DAC_VREF_POS_INPUT);
	vref.add_route(0, "dac8", -1.0, DAC_VREF_NEG_INPUT);
}

void chesster_state::kishon(machine_config &config)
{
	chesster(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3.579545_MHz_XTAL);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( chesster ) // model 6120, PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ch_1.3.ic9", 0x8000, 0x8000, CRC(8b42d1ad) SHA1(2161fc5ab2476fe7ca4ffc226e3cb329b8a57a01) ) // 27256, CH 1.3 on sticker

	ROM_REGION( 0x20000, "rombank", 0 )
	ROM_LOAD("101-1091b02.ic10", 0x0000, 0x20000, CRC(fa370e88) SHA1(a937c8f1ec295cf9539d12466993974e40771493) ) // AMI, 27C010 or equivalent
ROM_END

ROM_START( chesstera ) // model 6120, PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("chesster.ic9", 0x8000, 0x8000, CRC(29f9a698) SHA1(4c83ca46fd5fc9c40302e9c7f16b4ae2c18b06e6) ) // M27C256B, sticker but no label

	ROM_REGION( 0x20000, "rombank", 0 )
	ROM_LOAD("101-1091a02.ic10", 0x0000, 0x20000, CRC(2b4d243c) SHA1(921e51978facb502b207b4f64a73b1e74127e826) ) // AMI, 27C010 or equivalent
ROM_END

ROM_START( kishon ) // model 6120G or 6127(same), PCB label 510.1141C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("kishon.ic9", 0x8000, 0x8000, CRC(121c007f) SHA1(652e9ea47b6bb1632d10eb0fcd7f98cdba22fce7) ) // 27C256

	ROM_REGION( 0x80000, "rombank", 0 )
	ROM_LOAD("kishon_v2.6_1-14-91.ic10", 0x0000, 0x80000, CRC(50598869) SHA1(2087e0c2f40a2408fe217a6502c8c3a247bdd063) ) // Toshiba TC544000P-12, aka 101-1094A01
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME       PARENT   CMP MACHINE   INPUT     STATE           INIT           COMPANY, FULLNAME, FLAGS
CONS( 1990, chesster,  0,        0, chesster, chesster, chesster_state, init_chesster, "Fidelity Electronics", "Chesster Challenger (V1.3)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1990, chesstera, chesster, 0, chesster, chesster, chesster_state, init_chesster, "Fidelity Electronics", "Chesster Challenger", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1991, kishon,    chesster, 0, kishon,   chesster, chesster_state, init_chesster, "Fidelity Electronics", "Kishon Chesster", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
