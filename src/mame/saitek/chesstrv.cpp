// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:Sean Riddle, Berger
/*******************************************************************************

SciSys Chess Traveler (aka Novag Pocket Chess)
SciSys Chess Intercontinental Traveler

Chess Traveler hardware notes:
- Fairchild 3870 MCU, label SL90387 (does not use the timer or irq at all)
- 256 bytes RAM(3539)
- 4-digit 7seg led panel

It was also redistributed by Acetronic as "Chess Traveller"(British spelling there),
and by Prinztronic as well, another British brand

SciSys/Novag's "Chess Champion: Pocket Chess" is assumed to be the same game,
it has the same MCU serial (SL90387). They added battery low voltage detection
to it (rightmost digit DP lights up).

Chess Intercontinental Traveler, released after the SciSys/Novag partnership splitup,
is nearly the same program. Only a few bytes different, not counting changed jump
addresses due to inserted code.

Chess Intercontinental Traveler hardware notes:
- Fairchild 3870 MCU, label SL90594, backside label 3870T-0594
- 256 bytes RAM(2*TC5501P)
- 4-digit LCD screen, the exact same one as in Mini Chess, but of the extra segments,
  only the 'low battery' and 'computing' segments are used

Regarding MCU frequency:

Chess Traveler should be around 4.5MHz. An Acetronic version was measured ~3MHz,
but that is far too slow. On Novag Pocket Chess, opening move F2F3 at level 6 is
answered after around 60 seconds.

Likewise, Intercontinental Traveler was measured ~5MHz, but in reality is much
closer to 6MHz. This was confirmed by hooking up a 6MHz XTAL, taking around
44 seconds after the same F2F3 move.

Both chesscomputers have a cheap RC circuit for the MCU clock, it seems that
electronically measuring them affected the frequency.

*******************************************************************************/

#include "emu.h"

#include "cpu/f8/f8.h"
#include "machine/f3853.h"
#include "machine/timer.h"
#include "video/pwm.h"

#include "screen.h"

// internal artwork
#include "saitek_chesstrv.lh"
#include "saitek_chesstrvi.lh"


namespace {

class chesstrv_state : public driver_device
{
public:
	chesstrv_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_display(*this, "display"),
		m_comp_timer(*this, "comp_timer"),
		m_computing(*this, "computing"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void chesstrv(machine_config &config);
	void chesstrvi(machine_config &config);

	// battery status indicator is not software controlled
	DECLARE_INPUT_CHANGED_MEMBER(battery) { update_display(); }

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<pwm_display_device> m_display;
	optional_device<timer_device> m_comp_timer;
	output_finder<> m_computing;
	required_ioport_array<5> m_inputs;

	std::unique_ptr<u8[]> m_ram;
	u8 m_ram_address = 0;
	u8 m_inp_mux = 0;
	u8 m_7seg_data = 0;

	void chesstrv_mem(address_map &map) ATTR_COLD;
	void chesstrv_io(address_map &map) ATTR_COLD;

	TIMER_DEVICE_CALLBACK_MEMBER(computing) { m_computing = 1; }

	void update_display();
	void matrix_w(u8 data);
	void digit_w(u8 data);
	u8 input_r();

	// 256 bytes data RAM accessed via I/O ports
	u8 ram_address_r() { return m_ram_address; }
	void ram_address_w(u8 data) { m_ram_address = data; }
	u8 ram_data_r() { return m_ram[m_ram_address]; }
	void ram_data_w(u8 data) { m_ram[m_ram_address] = data; }
};

void chesstrv_state::machine_start()
{
	m_ram = make_unique_clear<u8[]>(0x100);
	m_computing.resolve();

	// register for savestates
	save_pointer(NAME(m_ram), 0x100);
	save_item(NAME(m_ram_address));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_7seg_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

void chesstrv_state::update_display()
{
	u8 battery = m_inputs[4]->read() << 7 & 0x80;
	m_display->matrix(~m_inp_mux, m_7seg_data | battery);
}

void chesstrv_state::digit_w(u8 data)
{
	// digit segments
	m_7seg_data = bitswap<8>(data,0,1,2,3,4,5,6,7) & 0x7f;
	update_display();
}

void chesstrv_state::matrix_w(u8 data)
{
	// chesstrvi: "computing" segment goes on when LCD isn't driven
	if (m_comp_timer != nullptr && ~data & m_inp_mux & 1)
	{
		m_computing = 0;
		m_comp_timer->adjust(attotime::from_msec(100));
	}

	// d0-d3: input/digit select (active low)
	m_inp_mux = data;
	update_display();
}

u8 chesstrv_state::input_r()
{
	u8 data = m_inp_mux;

	// d0-d3: multiplexed inputs from d4-d7
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i+4))
			data |= m_inputs[i]->read();

	// d4-d7: multiplexed inputs from d0-d3
	for (int i = 0; i < 4; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << (i+4);

	return data;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void chesstrv_state::chesstrv_mem(address_map &map)
{
	map.global_mask(0x7ff);
	map(0x0000, 0x07ff).rom();
}

void chesstrv_state::chesstrv_io(address_map &map)
{
	map(0x00, 0x00).rw(FUNC(chesstrv_state::ram_address_r), FUNC(chesstrv_state::ram_address_w));
	map(0x01, 0x01).w(FUNC(chesstrv_state::digit_w));
	map(0x04, 0x07).rw("psu", FUNC(f38t56_device::read), FUNC(f38t56_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( chesstrv )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1 / Pawn")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2 / Knight")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3 / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4 / Rook")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5 / Queen")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6 / King")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7 / White")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8 / Black")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("LV / CS") // level/clear square
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("FP") // find position
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("EP") // enter position
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("CB") // clear board

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("CE") // clear entry
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("MM") // multi move
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("IN.4")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" ) PORT_CHANGED_MEMBER(DEVICE_SELF, chesstrv_state, battery, 0)
	PORT_CONFSETTING(    0x01, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )
INPUT_PORTS_END

static INPUT_PORTS_START( chesstrvi )
	PORT_INCLUDE( chesstrv )

	PORT_MODIFY("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("Level / CS")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Find Position")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Enter Position")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")

	PORT_MODIFY("IN.3")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("Multi Move")
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void chesstrv_state::chesstrv(machine_config &config)
{
	// basic machine hardware
	F8(config, m_maincpu, 4'500'000/2); // approximation
	m_maincpu->set_addrmap(AS_PROGRAM, &chesstrv_state::chesstrv_mem);
	m_maincpu->set_addrmap(AS_IO, &chesstrv_state::chesstrv_io);

	f38t56_device &psu(F38T56(config, "psu", 4'500'000/2));
	psu.read_a().set(FUNC(chesstrv_state::ram_data_r));
	psu.write_a().set(FUNC(chesstrv_state::ram_data_w));
	psu.read_b().set(FUNC(chesstrv_state::input_r));
	psu.write_b().set(FUNC(chesstrv_state::matrix_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	m_display->set_segmask(0x1, 0xff); // DP for low battery
	config.set_default_layout(layout_saitek_chesstrv);
}

void chesstrv_state::chesstrvi(machine_config &config)
{
	chesstrv(config);

	// basic machine hardware
	m_maincpu->set_clock(6'000'000/2); // approximation
	subdevice<f38t56_device>("psu")->set_clock(6'000'000/2);

	TIMER(config, m_comp_timer).configure_generic(FUNC(chesstrv_state::computing));
	config.set_default_layout(layout_saitek_chesstrvi);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2, 567/2);
	screen.set_visarea_full();
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( chesstrv )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("sl90387", 0x0000, 0x0800, CRC(b76214d8) SHA1(7760903a64d9c513eb54c4787f535dabec62eb64) )
ROM_END

ROM_START( chesstrvi )
	ROM_REGION( 0x0800, "maincpu", 0 )
	ROM_LOAD("sl90594.u3", 0x0000, 0x0800, CRC(9162e89a) SHA1(c3f71365b73b0112aae09f11722bd78186c78408) )

	ROM_REGION( 48655, "screen", 0 )
	ROM_LOAD("chesstrvi.svg", 0, 48655, CRC(13aa3a99) SHA1(6ea2c55dc8c617532455c4754da9bcc5cad170e2) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME       PARENT  COMPAT  MACHINE    INPUT      CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1980, chesstrv,  0,      0,      chesstrv,  chesstrv,  chesstrv_state, empty_init, "SciSys / Novag Industries / Philidor Software", "Chess Traveler", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
SYST( 1982, chesstrvi, 0,      0,      chesstrvi, chesstrvi, chesstrv_state, empty_init, "SciSys / Philidor Software", "Chess Intercontinental Traveler", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
