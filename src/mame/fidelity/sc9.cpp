// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger, yoyo_chessboard
/*******************************************************************************

Fidelity SC9, Fidelity Playmatic "S"

TODO:
- fscc9ps module switch and led
- verify fscc9ps XTAL (checked against sound recording, 99.97% similarity)

Hardware notes:

Fidelity Sensory Chess Challenger "9" (SC9) overview:
- PCB label: 510-1046C01 2-1-82
- R6502-13, 1.4MHz from resonator, another pcb with the same resonator was measured 1.49MHz*
- 2KB RAM(TMM2016P), 2*8KB ROM(HN48364P)
- 36-pin edge connector, assume same as SC12
- 8*(8+1) buttons, 8*8+1 LEDs

*: 2 other boards were measured 1.60MHz and 1.88MHz(newest serial). Online references
suggest 3 versions of SC9(C01) total: 1.5MHz, 1.6MHz, and 1.9MHz.

I/O is via TTL, not further documented here

3 versions were available, the newest "B" version was 2MHz and included the Budapest program.
The Playmatic S was only released in Germany, it's basically a 'deluxe' version of SC9
with magnet sensors and came with CB9 and CB16.

================================================================================

Starting with SC9, Fidelity added a cartridge slot to their chess computers, meant for
extra book opening databases and recorded games.

Known modules (*denotes undumped):
- CB9: Challenger Book Openings 1
- CB16: Challenger Book Openings 2
- *CG64: 64 Greatest Games
- *EOA-EOE: Challenger Book Openings: Chess Encyclopedia Volume A-E (5 modules)
- *TDF: Challenger Book Openings: Tarrasch Defense to the Queen's Gambit

The edge connector has D0-D7, A0-A13, 2 chip select lines, read/write lines, IRQ line.
IRQ and write strobe are unused. Maximum known size is 16KB.

*******************************************************************************/

#include "emu.h"

#include "bus/generic/carts.h"
#include "bus/generic/slot.h"
#include "cpu/m6502/m6502.h"
#include "machine/clock.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_playmatic.lh"
#include "fidel_sc9.lh"


namespace {

class sc9_state : public driver_device
{
public:
	sc9_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void sc9b(machine_config &config);
	void sc9c(machine_config &config);
	void sc9d(machine_config &config);
	void playmatic(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(sc9c_change_cpu_freq);

protected:
	virtual void machine_start() override ATTR_COLD;

	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;

	u8 m_inp_mux = 0;
	u8 m_led_data = 0;

	// address maps
	void sc9_map(address_map &map) ATTR_COLD;
	void sc9d_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void control_w(u8 data);
	void led_w(offs_t offset, u8 data);
	u8 input_r();
	u8 input_d7_r(offs_t offset);
};

void sc9_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_data));
}

INPUT_CHANGED_MEMBER(sc9_state::sc9c_change_cpu_freq)
{
	// SC9(C01) was released with 1.5MHz, 1.6MHz, or 1.9MHz CPU
	static const u32 freq[3] = { 1'500'000, 1'600'000, 1'900'000 };
	m_maincpu->set_unscaled_clock(freq[newval % 3]);
}



/*******************************************************************************
    I/O
*******************************************************************************/

void sc9_state::control_w(u8 data)
{
	// d0-d3: 74245 P0-P3
	// 74245 Q0-Q8: input mux, led select
	m_inp_mux = data & 0xf;
	u16 sel = 1 << m_inp_mux;
	m_display->write_my(sel);

	// 74245 Q9: speaker out
	m_dac->write(BIT(sel, 9));

	// d4,d5: ?
	// d6,d7: N/C
}

void sc9_state::led_w(offs_t offset, u8 data)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (m_led_data & ~(1 << offset)) | ((data & 1) << offset);
	m_display->write_mx(m_led_data);
}

u8 sc9_state::input_r()
{
	// d0-d7: multiplexed inputs (active low)
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs->read();

	return ~data;
}

u8 sc9_state::input_d7_r(offs_t offset)
{
	// a0-a2,d7: multiplexed inputs
	return (input_r() >> offset & 1) ? 0x80 : 0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sc9_state::sc9_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x5fff).r("cartslot", FUNC(generic_slot_device::read_rom));
	map(0x6000, 0x6000).mirror(0x1fff).w(FUNC(sc9_state::control_w));
	map(0x8000, 0x8007).mirror(0x1ff8).w(FUNC(sc9_state::led_w)).nopr();
	map(0xa000, 0xa000).mirror(0x1fff).r(FUNC(sc9_state::input_r));
	map(0xc000, 0xffff).rom();
}

void sc9_state::sc9d_map(address_map &map)
{
	sc9_map(map);
	map(0xa000, 0xa007).mirror(0x1ff8).r(FUNC(sc9_state::input_d7_r));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sc9 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( sc9c )
	PORT_INCLUDE( sc9 )

	PORT_START("CPU")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sc9_state, sc9c_change_cpu_freq, 0) // factory set
	PORT_CONFSETTING(    0x00, "1.5MHz" )
	PORT_CONFSETTING(    0x01, "1.6MHz" )
	PORT_CONFSETTING(    0x02, "1.9MHz" )
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sc9_state::sc9d(machine_config &config)
{
	// basic machine hardware
	M6502(config, m_maincpu, 3.9_MHz_XTAL / 2); // R6502AP, 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9d_map);

	auto &irq_clock(CLOCK(config, "irq_clock", 600)); // from 555 timer (22nF, 102K, 2.7K), ideal frequency is 600Hz
	irq_clock.set_pulse_width(attotime::from_usec(41)); // active for 41us
	irq_clock.signal_handler().set_inputline(m_maincpu, M6502_IRQ_LINE);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(200));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(9, 8);
	config.set_default_layout(layout_fidel_sc9);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void sc9_state::sc9b(machine_config &config)
{
	sc9d(config);

	// basic machine hardware
	m_maincpu->set_clock(1'500'000); // from ceramic resonator "681 JSA", measured
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9_map);
}

void sc9_state::playmatic(machine_config &config)
{
	sc9b(config);

	// basic machine hardware
	m_maincpu->set_clock(5.626_MHz_XTAL / 2); // advertised as double the speed of SC9
	m_board->set_type(sensorboard_device::MAGNETS);

	config.set_default_layout(layout_fidel_playmatic);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fscc9 ) // PCB label 510-1046D01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034b01", 0xc000, 0x2000, CRC(65288753) SHA1(651f5ca5969ddd72a20cbebdec2de83c4bf10650) )
	ROM_LOAD("101-1034c02", 0xe000, 0x2000, CRC(238b092f) SHA1(7ddffc6dba822aee9d8ad6815b23024ed5cdfd26) )
ROM_END

ROM_START( fscc9b ) // PCB label 510-1046B01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034a01", 0xc000, 0x2000, CRC(b845c458) SHA1(d3fda65dbd9fae44fa4b93f8207839d8fa0c367a) )
	ROM_LOAD("101-1034a02", 0xe000, 0x2000, CRC(ecfa0a4c) SHA1(738df99a250fad0b1da5ebeb8c92a9ad1461417b) )
ROM_END

ROM_START( fscc9c ) // PCB label 510-1046C01
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-1034a01", 0xc000, 0x2000, CRC(b845c458) SHA1(d3fda65dbd9fae44fa4b93f8207839d8fa0c367a) ) // HN48364P
	ROM_LOAD("101-1034b02", 0xe000, 0x2000, CRC(cbaf97d7) SHA1(7ed8e68bb74713d9e2ff1d9c037012320b7bfcbf) ) // "
ROM_END

ROM_START( fscc9ps )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("play64c1.bin", 0xc000, 0x2000, CRC(e96aa95d) SHA1(16d90cf0ef166aef579d442671290a2c43e24dfe) )
	ROM_LOAD("play64en.bin", 0xe000, 0x2000, CRC(6fa188d2) SHA1(1b9b0209c496c89ecb7f9ec07bfd9429ff9b275e) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE    INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, fscc9,   0,      0,      sc9d,      sc9,   sc9_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger \"9\" (rev. D)", MACHINE_SUPPORTS_SAVE ) // aka version "B"
SYST( 1982, fscc9b,  fscc9,  0,      sc9b,      sc9,   sc9_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger \"9\" (rev. B)", MACHINE_SUPPORTS_SAVE )
SYST( 1982, fscc9c,  fscc9,  0,      sc9b,      sc9c,  sc9_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger \"9\" (rev. C)", MACHINE_SUPPORTS_SAVE )
SYST( 1983, fscc9ps, fscc9,  0,      playmatic, sc9,   sc9_state, empty_init, "Fidelity Deutschland", "Sensory 9 Playmatic S", MACHINE_SUPPORTS_SAVE ) // 9 is not between quotation marks here
