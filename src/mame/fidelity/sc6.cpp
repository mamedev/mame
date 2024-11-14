// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:yoyo_chessboard, Berger, Sean Riddle
/*******************************************************************************

Fidelity Sensory Chess Challenger 6 (model SC6)
Fidelity Mini Sensory Chess Challenger (model MSC, 1982 version)
Fidelity The Classic (model CC8/6079)
Fidelity Gambit Voice (model 6085)

The chess engine is by Ron Nelson. These are all on similar hardware. Several
models are equal to eachother from an emulator's perspective, hence some aren't
included in MAME, see list below.

Known MCU ROM serials:
- 7203 100-1012B01 (1982), Mini Sensory Chess Challenger
- 9517 [no label]  (1985), The Classic (model CC8)
- 9311 100-1020B02 (1986), The Classic (model 6079), The Gambit, Silver Bullet
- 9311 100-1020B01 (1989), The Gambit, Designer 1500, Peri Beta
- 9337 100-1020C01 (1987), Gambit Voice

100-1020B01 ROM contents is confirmed to be identical to 100-1020B02.

TODO:
- is The Classic model 6079D a different ROM?

================================================================================

SC6 hardware notes:
- PCB label: 510-1045B01
- INS8040N-11 MCU, 11MHz XTAL
- external 4KB ROM 2332 101-1035A01, in module slot
- buzzer, 2 7seg LEDs, 8*8 chessboard buttons

SC6 released modules, * denotes not dumped yet:
- *BO6: Book Openings I
- *CG6: Greatest Chess Games 1
- SC6: pack-in, original program

SC6 program is contained in BO6 and CG6.

================================================================================

MSC hardware notes:
- PCB label: 510-1044B01
- P8049H MCU, 2KB internal ROM, 11MHz XTAL
- buzzer, 18 leds, 8*8 chessboard buttons, module slot

MCU ports I/O is identical to SC6.

It accepts the same modules as the 1st MSC version. See msc.cpp for known modules.
The module overrides the internal ROM, by asserting the EA pin.

Silver Bullet hardware notes:
- PCB from MSC, but lose/check leds unpopulated
- TMP80C50AP-6-9311 MCU, 4KB internal ROM, 6MHz XTAL
- buzzer, 16 leds, 8*8 chessboard buttons, module slot (takes MSC modules)

================================================================================

The Classic (model CC8) hardware notes:
- PCB label: 510-1095A01
- TMP80C50AP-6-9517 MCU, 4KB internal ROM, 6MHz XTAL
- buzzer, 16 leds, 8*8 chessboard buttons

The Classic (model 6079) hardware notes:
- TMP80C50AP-6-9311 MCU, rest is same as model CC8

The Gambit hardware notes:
- PCB label: 510-1115A01
- rest is same as The Classic (model 6079)

The Gambit either has a black/white button panel color theme, or black/red/white,
which was more commonly seen on newer versions and Gambit Voice.

Designer 1500 hardware notes:
- PCB label: 510.1131A01
- rest is same as The Classic (model 6079)

Peri Beta has the same PCB/ROM as Designer 1500, only the housing differs.

================================================================================

Gambit Voice hardware notes:
- TMP80C50AP-6-9337 MCU, 4KB internal ROM, 6MHz XTAL
- 510.1117A01 sound PCB (TSI S14001A, 4KB or 8KB ROM)
- speaker, 16 leds, 8*8 chessboard buttons

The sound PCB is the same as the one from Excel Voice (see excel.cpp), except
with a smaller ROM, and no language selection DIP switches. It only supports
the English or French speech ROM.

As noted above, the non-voice Gambit is the same ROM as The Classic (model 6079).

*******************************************************************************/

#include "emu.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "fidel_classic.lh"
#include "fidel_gambitv.lh"
#include "fidel_msc_v2.lh"
#include "fidel_sc6.lh"


namespace {

class sc6_state : public driver_device
{
public:
	sc6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_speech(*this, "speech"),
		m_language(*this, "language"),
		m_cart(*this, "cartslot"),
		m_inputs(*this, "IN.0")
	{ }

	// machine configs
	void shared(machine_config &config);
	void sc6(machine_config &config);
	void msc(machine_config &config);
	void classic(machine_config &config);
	void gambitv(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	// devices/pointers
	required_device<mcs48_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	optional_device<dac_1bit_device> m_dac;
	optional_device<s14001a_device> m_speech;
	optional_region_ptr<u8> m_language;
	optional_device<generic_slot_device> m_cart;
	required_ioport m_inputs;

	u8 m_inp_mux = 0;
	u8 m_speech_data = 0;

	// address maps
	void msc_map(address_map &map) ATTR_COLD;
	void sc6_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void mux_w(u8 data);
	void select_w(u8 data);
	u8 input_r();

	u8 msc_rom_r(offs_t offset);

	void speech_w(offs_t offset, u8 data, u8 mem_mask);
	u8 speech_r();
};

void sc6_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_speech_data));
}

void sc6_state::machine_reset()
{
	// EA pin is tied to VCC when a cartridge is inserted
	m_maincpu->set_input_line(MCS48_INPUT_EA, (m_cart && m_cart->exists()) ? ASSERT_LINE : CLEAR_LINE);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// common

void sc6_state::mux_w(u8 data)
{
	// P24-P27: 7442 A-D (or 74145)
	// 7442 0-8: input mux, led data
	m_inp_mux = data >> 4 & 0xf;
	u16 sel = 1 << m_inp_mux;
	m_display->write_mx(sel);

	// 7442 9: speaker out
	if (m_dac != nullptr)
		m_dac->write(BIT(sel, 9));
}

void sc6_state::select_w(u8 data)
{
	// P16,P17: digit/led select
	m_display->write_my(~data >> 6 & 3);
}

u8 sc6_state::input_r()
{
	// P10-P15,T0,T1: multiplexed inputs
	u8 data = 0;

	// read chessboard sensors
	if (m_inp_mux < 8)
		data = m_board->read_file(m_inp_mux);

	// read button panel
	else if (m_inp_mux == 8)
		data = m_inputs->read();

	return ~data;
}


// speech (gambitv)

u8 sc6_state::speech_r()
{
	// P20: S14001A busy pin
	u8 data = m_speech->busy_r();

	// P21: language jumper
	// P22,P23: unused?
	return data | (*m_language << 1 & 2) | 0xfc;
}

void sc6_state::speech_w(offs_t offset, u8 data, u8 mem_mask)
{
	data &= mem_mask;
	m_speech_data = data;

	// DB6: speech ROM A12
	m_speech->set_rom_bank(BIT(data, 6));

	// DB0-DB5: S14001A C0-C5
	// DB7: S14001A start pin
	m_speech->data_w(data & 0x3f);
	m_speech->start_w(BIT(data, 7));
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

u8 sc6_state::msc_rom_r(offs_t offset)
{
	// MSC module ROM A12 is forced high (lower half has a Z8 program)
	return m_cart->read_rom(offset | 0x1000);
}

void sc6_state::msc_map(address_map &map)
{
	map(0x0000, 0x0fff).r(FUNC(sc6_state::msc_rom_r));
}

void sc6_state::sc6_map(address_map &map)
{
	map(0x0000, 0x0fff).r(m_cart, FUNC(generic_slot_device::read_rom));
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
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("RE")
INPUT_PORTS_END

static INPUT_PORTS_START( msc )
	PORT_INCLUDE( sc6 )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Speaker / Bishop")
INPUT_PORTS_END

static INPUT_PORTS_START( gambitv )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Move / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Hint / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Take Back / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("Level / Rook")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("Verify / Queen")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("Problem / King")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("Clear")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sc6_state::shared(machine_config &config)
{
	// basic machine hardware
	m_maincpu->p1_in_cb().set(FUNC(sc6_state::input_r)).mask(0x3f);
	m_maincpu->p1_in_cb().append_constant(0xc0).mask(0xc0);
	m_maincpu->p1_out_cb().set(FUNC(sc6_state::select_w));
	m_maincpu->p2_out_cb().set(FUNC(sc6_state::mux_w));
	m_maincpu->t0_in_cb().set(FUNC(sc6_state::input_r)).bit(6);
	m_maincpu->t1_in_cb().set(FUNC(sc6_state::input_r)).bit(7);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(2, 9);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void sc6_state::sc6(machine_config &config)
{
	I8040(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc6_state::sc6_map);
	shared(config);

	// video hardware
	m_display->set_segmask(0x3, 0x7f);
	config.set_default_layout(layout_fidel_sc6);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fidel_sc6").set_must_be_loaded(true);
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_sc6");
}

void sc6_state::msc(machine_config &config)
{
	I8049(config, m_maincpu, 11_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sc6_state::msc_map);
	shared(config);

	config.set_default_layout(layout_fidel_msc_v2);

	// cartridge
	GENERIC_CARTSLOT(config, m_cart, generic_plain_slot, "fidel_msc");
	SOFTWARE_LIST(config, "cart_list").set_original("fidel_msc");
}

void sc6_state::classic(machine_config &config)
{
	I8050(config, m_maincpu, 6_MHz_XTAL);
	shared(config);

	config.set_default_layout(layout_fidel_classic);
}

void sc6_state::gambitv(machine_config &config)
{
	classic(config);

	// basic machine hardware
	m_maincpu->p2_in_cb().set(FUNC(sc6_state::speech_r));
	m_maincpu->bus_in_cb().set([this](){ return m_speech_data; });
	m_maincpu->bus_out_cb().set(FUNC(sc6_state::speech_w));

	config.set_default_layout(layout_fidel_gambitv);

	// sound hardware
	config.device_remove("dac");
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( fscc6 )
	ROM_REGION( 0x1000, "maincpu", ROMREGION_ERASE00 )
	// none here, it's in the module slot
ROM_END

ROM_START( miniscc )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("p8049h_7203_100-1012b01.ic1", 0x0000, 0x0800, CRC(ea3261f7) SHA1(1601358fdf0eee0b973c0f4c78bf679b8dada72a) )
ROM_END

ROM_START( classic )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("tmp80c50ap-6-9311_100-1020b02.ic1", 0x0000, 0x1000, CRC(ba41b5ba) SHA1(1a5c5b2e990a07b9ff51eecfa952a4b890107797) )
ROM_END

ROM_START( classica )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("tmp80c50ap-6_9517.ic1", 0x0000, 0x1000, CRC(dfd30755) SHA1(92075c7df9205b9647801487b9bbddcf230dfc91) )
ROM_END

ROM_START( gambitv )
	ROM_REGION( 0x1000, "maincpu", 0 )
	ROM_LOAD("tmp80c50ap-6-9337_100-1020c01.ic1", 0x0000, 0x1000, CRC(dafee386) SHA1(d67914fb2abd73c0068b7e61fc23d211c52d65d9) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "fr", "French")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 1, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 0, ROM_BIOS(1) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(1) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT   COMPAT  MACHINE  INPUT    CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1982, fscc6,    0,       0,      sc6,     sc6,     sc6_state, empty_init, "Fidelity Electronics", "Sensory Chess Challenger \"6\"", MACHINE_SUPPORTS_SAVE )
SYST( 1982, miniscc,  0,       0,      msc,     msc,     sc6_state, empty_init, "Fidelity Electronics", "Mini Sensory Chess Challenger (MCS-48 version)", MACHINE_SUPPORTS_SAVE ) // aka "Mini Sensory II"

SYST( 1986, classic,  0,       0,      classic, sc6,     sc6_state, empty_init, "Fidelity International", "The Classic (model 6079)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, classica, classic, 0,      classic, sc6,     sc6_state, empty_init, "Fidelity International", "The Classic (model CC8)", MACHINE_SUPPORTS_SAVE )

SYST( 1987, gambitv,  0,       0,      gambitv, gambitv, sc6_state, empty_init, "Fidelity International", "Gambit Voice", MACHINE_SUPPORTS_SAVE )
