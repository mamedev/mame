// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/******************************************************************************
*
* novag_delta1.cpp, subdriver of machine/novagbase.cpp, machine/chessbase.cpp

TODO:
- ccdelta1 doesn't work, goes bonkers when you press Enter. CPU core bug?
  I suspect related to interrupts

*******************************************************************************

Novag Delta-1 overview:
- 3850PK CPU at ~2MHz, 3853PK memory interface
- 4KB ROM(2332A), 256 bytes RAM(2*2111A-4)
- 4-digit 7seg panel, no sound, no chessboard

******************************************************************************/

#include "emu.h"
#include "includes/novagbase.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"

// internal artwork
#include "novag_delta1.lh"


namespace {

class delta1_state : public novagbase_state
{
public:
	delta1_state(const machine_config &mconfig, device_type type, const char *tag) :
		novagbase_state(mconfig, type, tag)
	{ }

	// machine drivers
	void delta1(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// address maps
	void main_map(address_map &map);
	void main_io(address_map &map);

	// I/O handlers
	DECLARE_WRITE8_MEMBER(io0_w);
	DECLARE_WRITE8_MEMBER(io1_w);
	DECLARE_READ8_MEMBER(io0_r);
	DECLARE_READ8_MEMBER(io1_r);

	u8 m_io[2]; // F8 CPU I/O ports
};

void delta1_state::machine_start()
{
	novagbase_state::machine_start();

	// zerofill/register for savestates
	memset(m_io, 0, sizeof(m_io));
	save_item(NAME(m_io));
}


/******************************************************************************
    Devices, I/O
******************************************************************************/

// CPU I/O ports

WRITE8_MEMBER(delta1_state::io0_w)
{
	m_io[0] = data;

	// IO00-02: MC14028B A-C (IO03: GND)
	// MC14028B Q3-Q7: input mux
	// MC14028B Q4-Q7: digit select through 75492
	u16 sel = 1 << (~data & 7);
	m_inp_mux = sel >> 3 & 0x1f;

	// update display here
	set_display_segmask(0xf, 0x7f);
	display_matrix(7, 4, m_7seg_data, sel >> 4);
}

READ8_MEMBER(delta1_state::io0_r)
{
	// IO04-07: multiplexed inputs
	return read_inputs(5) << 4 | m_io[0];
}

WRITE8_MEMBER(delta1_state::io1_w)
{
	m_io[1] = data;

	// IO17: segment commons, active low (always 0?)
	// IO10-16: digit segments A-G
	data = (data & 0x80) ? 0 : (data & 0x7f);
	m_7seg_data = bitswap<7>(data, 0,1,2,3,4,5,6);
}

READ8_MEMBER(delta1_state::io1_r)
{
	// unused?
	return m_io[1];
}



/******************************************************************************
    Address Maps
******************************************************************************/

void delta1_state::main_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).mirror(0x1000).rom(); // _A13
	map(0x2000, 0x20ff).mirror(0x1f00).ram(); // A13
}

void delta1_state::main_io(address_map &map)
{
	map(0x0, 0x0).rw(FUNC(delta1_state::io0_r), FUNC(delta1_state::io0_w));
	map(0x1, 0x1).rw(FUNC(delta1_state::io1_r), FUNC(delta1_state::io1_w));
	map(0xc, 0xf).rw("f3853", FUNC(f3853_device::read), FUNC(f3853_device::write));
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( delta1 )
	PORT_START("IN.0")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Time Set")
	PORT_BIT(0x0d, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A) PORT_NAME("A 1 / White King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B) PORT_NAME("B 2 / White Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C) PORT_NAME("C 3 / White Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D) PORT_NAME("D 4")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E) PORT_NAME("E 5 / White Bishop")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F) PORT_NAME("F 6 / White Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G) PORT_NAME("G 7 / White Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H) PORT_NAME("H 8")

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("9 / Black King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("0 / Black Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Time Display / Black Rook")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("FP / Black Bishop") // find position
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("EP / Black Knight") // enter position
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Color / Black Pawn")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")

	PORT_START("RESET") // not on matrix
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_CHANGED_MEMBER(DEVICE_SELF, delta1_state, reset_button, nullptr) PORT_NAME("New Game")
INPUT_PORTS_END



/******************************************************************************
    Machine Drivers
******************************************************************************/

void delta1_state::delta1(machine_config &config)
{
	/* basic machine hardware */
	F8(config, m_maincpu, 2000000); // LC circuit, measured 2MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &delta1_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &delta1_state::main_io);
	m_maincpu->set_irq_acknowledge_callback("f3853", FUNC(f3853_device::int_acknowledge));

	f3853_device &f3853(F3853(config, "f3853", 2000000));
	f3853.int_req_callback().set_inputline("maincpu", F8_INPUT_LINE_INT_REQ);

	TIMER(config, "display_decay").configure_periodic(FUNC(delta1_state::display_decay_tick), attotime::from_msec(1));
	config.set_default_layout(layout_novag_delta1);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( ccdelta1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("ma_winke_y1d", 0x0000, 0x1000, CRC(ddc04aca) SHA1(bbf334c82bc89b2f131f5a50f0a617bc3bc4c329) ) // 2332a
ROM_END

} // anonymous namespace



/******************************************************************************
    Drivers
******************************************************************************/

//    YEAR  NAME      PARENT CMP MACHINE  INPUT   CLASS         INIT        COMPANY, FULLNAME, FLAGS
CONS( 1979, ccdelta1, 0,      0, delta1,  delta1, delta1_state, empty_init, "Novag", "Chess Champion: Delta-1", MACHINE_SUPPORTS_SAVE | MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
