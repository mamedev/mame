// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard, Berger
/*******************************************************************************

Fidelity Sensory Chess Challenger 6 (model SC6)
Fidelity Mini Sensory Chess Challenger (model MSC, 1982 version)
Fidelity The Gambit (model 6084)

TODO:
- MSC MCU is currently emulated as I8039, due to missing EA pin emulation
- different button panel for fidel_msc_v2 artwork
- add the older versions of gambit(assuming different ROM): 1st version is
  probably the same as "The Classic", and 2nd version has voice capability

--------------------------------------------------------------------------------

SC6 hardware notes:
- PCB label 510-1045B01
- INS8040N-11 MCU, 11MHz XTAL
- external 4KB ROM 2332 101-1035A01, in module slot
- buzzer, 2 7seg LEDs, 8*8 chessboard buttons

SC6 released modules, * denotes not dumped yet:
- *BO6: Book Openings I
- *CG6: Greatest Chess Games 1
- SC6: pack-in, original program

SC6 program is contained in BO6 and CG6.

--------------------------------------------------------------------------------

MSC hardware notes:
- PCB label 510-1044B01
- P8049H MCU, 2KB internal ROM, 11MHz XTAL
- buzzer, 18 leds, 8*8 chessboard buttons, module slot

MCU ports I/O is identical to SC6.

It accepts the same modules as the 1st MSC version. See msc.cpp for known modules.
The module overrides the internal ROM, by asserting the EA pin.

--------------------------------------------------------------------------------

Gambit(v3) hardware notes:
- PCB label 510-1115A01 (1986 PCB, but chips from 1989)
- TMP80C50AP-6-9311 MCU, 4KB internal ROM, 6MHz XTAL
- buzzer, 16 leds, 8*8 chessboard buttons

MCU ports I/O again identical to SC6.
The same MCU+ROM was also used in Designer 1500(PCB label 510.1131A01).
And also in The Classic(PCB label 510-1095A01), 100-1020B02 MCU.

100-1020B02 ROM contents is confirmed to be identical to 100-1020B01.

Gambit Voice hardware notes:
- TMP80C50AP-6-9311 MCU, 4KB internal ROM, 6MHz XTAL
- 510.1117A01 sound PCB, the one from Excel Voice, but with 2332 ROM
- speaker, 16 leds, 8*8 chessboard buttons

Silver Bullet hardware notes:
- PCB from MSC, but lose/check leds unpopulated
- TMP80C50AP-6-9311 MCU, 4KB internal ROM, 6MHz XTAL
- buzzer, 16 leds, 8*8 chessboard buttons, module slot

To summarize, known MCU chip ROM serials+year:
- 100-1020B01 (1989), The Gambit, Designer 1500, Peri Beta
- 100-1020B02 (1986), Silver Bullet
- 100-1020B02 (1987), The Classic
- 100-1020C01 (1987), Gambit Voice

*******************************************************************************/

#include "emu.h"

#include "cpu/mcs48/mcs48.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"
#include "bus/generic/slot.h"
#include "bus/generic/carts.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_gambit.lh"
#include "fidel_msc_v2.lh"
#include "fidel_sc6.lh"


namespace {

class sc6_state : public driver_device
{
public:
	sc6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_rom(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void msc(machine_config &config);
	void sc6(machine_config &config);
	void gambit(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;
	required_region_ptr<u8> m_rom;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_1bit_device> m_dac;
	optional_device<generic_slot_device> m_cart;
	required_ioport m_inputs;

	u8 m_led_select = 0;
	u8 m_inp_mux = 0;

	// address maps
	void msc_map(address_map &map);
	void sc6_map(address_map &map);

	// I/O handlers
	void update_display();
	void mux_w(u8 data);
	void select_w(u8 data);
	u8 rom_r(offs_t offset);

	u8 read_inputs();
	u8 input_r();
	int input6_r();
	int input7_r();
};

void sc6_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_led_select));
	save_item(NAME(m_inp_mux));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void sc6_state::update_display()
{
	// MSC: 18 leds, SC6: 2 7seg leds
	m_display->matrix(m_led_select, 1 << m_inp_mux);
}

void sc6_state::mux_w(u8 data)
{
	// P24-P27: 7442 A-D (or 74145)
	// 7442 0-8: input mux, led data
	m_inp_mux = data >> 4 & 0xf;
	update_display();

	// 7442 9: speaker out
	m_dac->write(BIT(1 << m_inp_mux, 9));
}

void sc6_state::select_w(u8 data)
{
	// P16,P17: led select
	m_led_select = ~data >> 6 & 3;
	update_display();
}

u8 sc6_state::rom_r(offs_t offset)
{
	// MSC reads from cartridge if it's inserted(A12 high), otherwise from internal ROM
	return m_cart->exists() ? m_cart->read_rom(offset | 0x1000) : m_rom[offset];
}

u8 sc6_state::read_inputs()
{
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs->read();

	return ~data;
}

u8 sc6_state::input_r()
{
	// P10-P15: multiplexed inputs low
	return (read_inputs() & 0x3f) | 0xc0;
}

int sc6_state::input6_r()
{
	// T0: multiplexed inputs bit 6
	return read_inputs() >> 6 & 1;
}

int sc6_state::input7_r()
{
	// T1: multiplexed inputs bit 7
	return read_inputs() >> 7 & 1;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sc6_state::msc_map(address_map &map)
{
	map(0x0000, 0x0fff).r(FUNC(sc6_state::rom_r));
}

void sc6_state::sc6_map(address_map &map)
{
	map(0x0000, 0x0fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sc6 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("RV / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("DM / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("TB / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("LV / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("PV / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("PB / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CL")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( msc )
	PORT_INCLUDE( sc6 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Speaker / Bishop")
INPUT_PORTS_END

static INPUT_PORTS_START( gambit )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Problem / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sc6_state::gambit(machine_config &config)
{
	// basic machine hardware
	I8050(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->p2_out_cb().set(FUNC(sc6_state::mux_w));
	m_maincpu->p1_in_cb().set(FUNC(sc6_state::input_r));
	m_maincpu->p1_out_cb().set(FUNC(sc6_state::select_w));
	m_maincpu->t0_in_cb().set(FUNC(sc6_state::input6_r));
	m_maincpu->t1_in_cb().set(FUNC(sc6_state::input7_r));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 9);
	config.set_default_layout(layout_fidel_gambit);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void sc6_state::msc(machine_config &config)
{
	gambit(config);

	// basic machine hardware
	I8039(config.replace(), m_maincpu, 11_MHz_XTAL); // actually I8049
	m_maincpu->set_addrmap(AS_PROGRAM, &sc6_state::msc_map);
	m_maincpu->p2_out_cb().set(FUNC(sc6_state::mux_w));
	m_maincpu->p1_in_cb().set(FUNC(sc6_state::input_r));
	m_maincpu->p1_out_cb().set(FUNC(sc6_state::select_w));
	m_maincpu->t0_in_cb().set(FUNC(sc6_state::input6_r));
	m_maincpu->t1_in_cb().set(FUNC(sc6_state::input7_r));

	config.set_default_layout(layout_fidel_msc_v2);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_msc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_msc");
}

void sc6_state::sc6(machine_config &config)
{
	gambit(config);

	// basic machine hardware
	I8040(config.replace(), m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc6_state::sc6_map);
	m_maincpu->p2_out_cb().set(FUNC(sc6_state::mux_w));
	m_maincpu->p1_in_cb().set(FUNC(sc6_state::input_r));
	m_maincpu->p1_out_cb().set(FUNC(sc6_state::select_w));
	m_maincpu->t0_in_cb().set(FUNC(sc6_state::input6_r));
	m_maincpu->t1_in_cb().set(FUNC(sc6_state::input7_r));

	// video hardware
	m_display->set_segmask(0x3, 0x7f);
	config.set_default_layout(layout_fidel_sc6);

	// cartridge
	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "fidel_sc6").set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_sc6");
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fscc6 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

ROM_START( miniscc )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("100-1012b01", 0x0000, 0x0800, CRC(ea3261f7) SHA1(1601358fdf0eee0b973c0f4c78bf679b8dada72a) ) // internal ROM
	ROM_RELOAD(             0x0800, 0x0800)
ROM_END

ROM_START( gambit )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("100-1020b01", 0x0000, 0x1000, CRC(ba41b5ba) SHA1(1a5c5b2e990a07b9ff51eecfa952a4b890107797) ) // internal ROM
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT   CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, fscc6,   0,      0,      sc6,     sc6,    sc6_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger \"6\"", MACHINE_SUPPORTS_SAVE )
SYST( 1982, miniscc, 0,      0,      msc,     msc,    sc6_state, empty_init, "Fidelity Electronics", "Mini Sensory Chess Challenger (1982 version)", MACHINE_SUPPORTS_SAVE ) // aka "Mini Sensory II"

SYST( 1989, gambit,  0,      0,      gambit,  gambit, sc6_state, empty_init, "Fidelity Electronics", "The Gambit (1989 version)", MACHINE_SUPPORTS_SAVE )
