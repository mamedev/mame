// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger,yoyo_chessboard
/******************************************************************************

Fidelity Sensory Chess Challenger "9" (SC9)
3 versions were available, the newest "B" version was 2MHz and included the Budapest program.
The Playmatic S was only released in Germany, it's basically a 'deluxe' version of SC9
with magnet sensors and came with CB9 and CB16.
---------------------------------

8*(8+1) buttons, 8*8+1 LEDs
36-pin edge connector, assume same as SC12
2KB RAM(TMM2016P), 2*8KB ROM(HN48364P)
R6502-13, 1.4MHz from resonator, another pcb with the same resonator was measured 1.49MHz*
PCB label 510-1046C01 2-1-82

*: 2 other boards were measured 1.60MHz and 1.88MHz(newest serial). Online references
suggest 3 versions of SC9(C01) total: 1.5MHz, 1.6MHz, and 1.9MHz.

I/O is via TTL, not further documented here

******************************************************************************/

#include "emu.h"
#include "includes/fidelbase.h"

#include "cpu/m6502/m6502.h"
#include "sound/volt_reg.h"
#include "speaker.h"

// internal artwork
#include "fidel_playmatic.lh" // clickable
#include "fidel_sc9.lh" // clickable


namespace {

class sc9_state : public fidelbase_state
{
public:
	sc9_state(const machine_config &mconfig, device_type type, const char *tag) :
		fidelbase_state(mconfig, type, tag)
	{ }

	// machine drivers
	void sc9b(machine_config &config);
	void sc9c(machine_config &config);
	void sc9d(machine_config &config);
	void playmatic(machine_config &config);

protected:
	// address maps
	void sc9_map(address_map &map);
	void sc9d_map(address_map &map);

	// I/O handlers
	void prepare_display();
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_WRITE8_MEMBER(led_w);
	DECLARE_READ8_MEMBER(input_r);
	DECLARE_READ8_MEMBER(input_d7_r);
};

class sc9c_state : public sc9_state
{
public:
	sc9c_state(const machine_config &mconfig, device_type type, const char *tag) :
		sc9_state(mconfig, type, tag)
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(sc9c_cpu_freq) { sc9c_set_cpu_freq(); }

protected:
	virtual void machine_reset() override;
	void sc9c_set_cpu_freq();
};

void sc9c_state::machine_reset()
{
	sc9_state::machine_reset();
	sc9c_set_cpu_freq();
}

void sc9c_state::sc9c_set_cpu_freq()
{
	// SC9(C01) was released with 1.5MHz, 1.6MHz, or 1.9MHz CPU
	u8 inp = ioport("FAKE")->read();
	m_maincpu->set_unscaled_clock((inp & 2) ? 1900000 : ((inp & 1) ? 1600000 : 1500000));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// TTL/generic

void sc9_state::prepare_display()
{
	// 8*8 chessboard leds + 1 corner led
	display_matrix(8, 9, m_led_data, m_inp_mux);
}

WRITE8_MEMBER(sc9_state::control_w)
{
	// d0-d3: 74245 P0-P3
	// 74245 Q0-Q8: input mux, led select
	u16 sel = 1 << (data & 0xf) & 0x3ff;
	m_inp_mux = sel & 0x1ff;
	prepare_display();

	// 74245 Q9: speaker out
	m_dac->write(BIT(sel, 9));

	// d4,d5: ?
	// d6,d7: N/C
}

WRITE8_MEMBER(sc9_state::led_w)
{
	// a0-a2,d0: led data via NE591N
	m_led_data = (data & 1) << offset;
	prepare_display();
}

READ8_MEMBER(sc9_state::input_r)
{
	// multiplexed inputs (active low)
	return read_inputs(9) ^ 0xff;
}

READ8_MEMBER(sc9_state::input_d7_r)
{
	// a0-a2,d7: multiplexed inputs (active low)
	return (read_inputs(9) >> offset & 1) ? 0 : 0x80;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void sc9_state::sc9_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).mirror(0x1800).ram();
	map(0x2000, 0x5fff).r(FUNC(sc9_state::cartridge_r));
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



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( sc9_sidepanel )
	PORT_START("IN.8")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( sc9 )
	PORT_INCLUDE( fidel_cb_buttons )
	PORT_INCLUDE( sc9_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( playmatic )
	PORT_INCLUDE( fidel_cb_magnets )
	PORT_INCLUDE( sc9_sidepanel )
INPUT_PORTS_END

static INPUT_PORTS_START( sc9c )
	PORT_INCLUDE( sc9 )

	PORT_START("FAKE")
	PORT_CONFNAME( 0x03, 0x00, "CPU Frequency" ) PORT_CHANGED_MEMBER(DEVICE_SELF, sc9c_state, sc9c_cpu_freq, nullptr) // factory set
	PORT_CONFSETTING(    0x00, "1.5MHz" )
	PORT_CONFSETTING(    0x01, "1.6MHz" )
	PORT_CONFSETTING(    0x02, "1.9MHz" )
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void sc9_state::sc9d(machine_config &config)
{
	/* basic machine hardware */
	M6502(config, m_maincpu, 3.9_MHz_XTAL/2); // R6502AP, 3.9MHz resonator
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9d_map);

	const attotime irq_period = attotime::from_hz(610); // from 555 timer (22nF, 102K, 2.7K)
	TIMER(config, m_irq_on).configure_periodic(FUNC(sc9_state::irq_on<M6502_IRQ_LINE>), irq_period);
	m_irq_on->set_start_delay(irq_period - attotime::from_usec(41)); // active for 41us
	TIMER(config, "irq_off").configure_periodic(FUNC(sc9_state::irq_off<M6502_IRQ_LINE>), irq_period);

	TIMER(config, "display_decay").configure_periodic(FUNC(fidelbase_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_fidel_sc9);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac, 0).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);

	/* cartridge */
	generic_cartslot_device &cartslot(GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_scc", "bin,dat"));
	cartslot.set_device_load(device_image_load_delegate(&fidelbase_state::device_image_load_scc_cartridge, this));

	SOFTWARE_LIST(config, "cart_list").set_original("fidel_scc");
}

void sc9_state::sc9b(machine_config &config)
{
	sc9d(config);

	/* basic machine hardware */
	m_maincpu->set_clock(1500000); // from ceramic resonator "681 JSA", measured
	m_maincpu->set_addrmap(AS_PROGRAM, &sc9_state::sc9_map);
}

void sc9_state::playmatic(machine_config &config)
{
	sc9b(config);

	/* basic machine hardware */
	m_maincpu->set_clock(3100000); // approximation
	config.set_default_layout(layout_fidel_playmatic);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

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



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME     PARENT  CMP MACHINE    INPUT      STATE       INIT        COMPANY, FULLNAME, FLAGS
CONS( 1982, fscc9,   0,       0, sc9d,      sc9,       sc9_state,  empty_init, "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. D)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // aka version "B"
CONS( 1982, fscc9b,  fscc9,   0, sc9b,      sc9,       sc9_state,  empty_init, "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. B)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1982, fscc9c,  fscc9,   0, sc9b,      sc9c,      sc9c_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger 9 (rev. C)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS )
CONS( 1983, fscc9ps, fscc9,   0, playmatic, playmatic, sc9_state,  empty_init, "Fidelity Electronics", "Sensory 9 Playmatic 'S'", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK | MACHINE_IMPERFECT_CONTROLS ) // by Fidelity West Germany
