// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Saitek Kasparov Blitz (model 291)

NOTE: Turn the power switch off before exiting MAME, otherwise NVRAM won't save
properly.

This is the last Saitek chess computer with Julio Kaplan's involvement (Heuristic
Software), although Craig Barnes continued working for Saitek. Julio Kaplan kept
programming his chess engine for a few more years, he stopped altogether after
the release of Kasparov's Gambit (Socrates II) for MS-DOS in 1993.

At first power-on, the chessboard gets calibrated, this is done automatically
in MAME. Not counting the chessboard, the whole user interface is with 2 vertical
dials at the right edge. There are no pushbuttons.

If chessboard calibration goes wrong somehow (eg. it wrongly tells you to place
a piece on a square, thinking the square is empty): clear the sensorboard, go to
options and select calibration, wait until the "board clr?" message goes away,
and then reset the sensorboard.

Hardware notes:
- PCB label: SA9-PE-020 REV 3/4
- Hitachi H8/325 MCU (either Mask ROM or PROM), 20MHz XTAL
- LCD with 10 7segs and custom segments
- piezo, 64 LEDs, metal sensors chessboard*, 2 dials

*: The chessboard technology is not with magnets (reed switches or hall effect
sensors). There are of copper wires beneath the chessboard, each square resembles
a metal detector. Chess pieces have aluminium washers at the bottom.

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "saitek_blitz.lh"


namespace {

class blitz_state : public driver_device
{
public:
	blitz_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.0"),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void blitz(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off) { m_power = false; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_ioport m_inputs;
	output_finder<4, 22> m_out_lcd;

	u8 m_inp_mux = 0;
	u8 m_sensor_strength = 0;
	bool m_sensor_state = false;
	u32 m_lcd_segs = 0;
	u8 m_lcd_com = 0;
	bool m_power = false;

	u8 m_port3 = 0xff;
	u8 m_port6 = 0xff;

	attotime m_board_init_time;

	void init_board(u8 data);
	bool board_active() { return machine().time() > m_board_init_time; }

	// I/O handlers
	void standby(int state);
	u8 power_r();

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);

	void p1_w(u8 data);
	void p2_w(u8 data);
	u8 p3_r();
	void p3_w(u8 data);
	u8 p6_r();
	void p6_w(u8 data);
};



/*******************************************************************************
    Initialization
*******************************************************************************/

void blitz_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_sensor_strength));
	save_item(NAME(m_sensor_state));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_power));
	save_item(NAME(m_port3));
	save_item(NAME(m_port6));
	save_item(NAME(m_board_init_time));
}

void blitz_state::init_board(u8 data)
{
	// briefly deactivate board after a cold boot to give it time to calibrate
	if (~data & 1)
		m_board_init_time = machine().time() + attotime::from_msec(1750);
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void blitz_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

u8 blitz_state::power_r()
{
	// P46: power switch state
	return m_power ? 0xbf : 0xff;
}


// LCD

void blitz_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void blitz_state::update_lcd()
{
	u32 lcd_segs = bitswap<22>(m_lcd_segs,26,27,28,29,30,31,0,1,2,3,4,5,19,7,8,9,10,11,12,13,24,25);

	for (int i = 0; i < 4; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
		const u32 data = (com == 0) ? lcd_segs : (com == 2) ? ~lcd_segs : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

template <int N>
void blitz_state::lcd_segs_w(u8 data)
{
	// P4x, P5x, P63, P7x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}


// misc

void blitz_state::p1_w(u8 data)
{
	// P10-P15: board sensor strength (higher is more sensitive)
	m_sensor_strength = bitswap<6>(data,0,1,2,3,4,5);

	// P16: ext power (no need to emulate it)
}

void blitz_state::p2_w(u8 data)
{
	// P20-P27: LED select
	m_led_pwm->write_my(~data);
}

u8 blitz_state::p3_r()
{
	// P37: board sensor state
	return m_sensor_state ? 0x7f : 0xff;
}

void blitz_state::p3_w(u8 data)
{
	// P36: reset board sensor on falling edge
	if (m_port3 & ~data & 0x40)
	{
		// P30-P32: board x
		// P33-P35: board y
		const u8 x = data & 7;
		const u8 y = data >> 3 & 7;
		const u8 s = m_sensor_strength;

		m_sensor_state = (s > 0x38) || (s > 0x30 && board_active() && m_board->read_sensor(x, y));
	}

	m_port3 = data;
}

u8 blitz_state::p6_r()
{
	// P66: multiplexed inputs
	return (m_inp_mux & m_inputs->read()) ? 0xbf : 0xff;
}

void blitz_state::p6_w(u8 data)
{
	// P62: speaker out
	m_dac->write(BIT(data, 2));

	// P60: 74164(1) CP
	// P61: 74164(1) DSB, outputs to input mux / LED data
	if (~m_port6 & data & 1)
		m_inp_mux = m_inp_mux << 1 | BIT(~data, 1);
	m_led_pwm->write_mx(m_inp_mux);

	// P64: 74164(2) CP
	// P65: 74164(2) DSB, output to LCD common
	if (~m_port6 & data & 0x10)
		m_lcd_com = m_lcd_com << 1 | BIT(data, 5);

	m_port6 = data;
	update_lcd();
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

// mode dial, rotary switch with 12 stops (duplicate halves)
// ready, options, info, game, set up, level
static const ioport_value mode_dial[6] = { 5, 1, 2, 4, 6, 3 };

// shuttle dial, free running dial with 6 magnets and 2 reed switches
static const ioport_value shuttle_dial[4] = { 0, 1, 3, 2 };

static INPUT_PORTS_START( blitz )
	PORT_START("IN.0")
	PORT_BIT(0x07, 0x00, IPT_POSITIONAL_V) PORT_POSITIONS(6) PORT_WRAPS PORT_REMAP_TABLE(mode_dial) PORT_SENSITIVITY(6) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_NAME("Mode Dial")
	PORT_BIT(0x18, 0x00, IPT_POSITIONAL_H) PORT_POSITIONS(4) PORT_WRAPS PORT_REMAP_TABLE(shuttle_dial) PORT_SENSITIVITY(12) PORT_KEYDELTA(1) PORT_CENTERDELTA(0) PORT_NAME("Shuttle Dial")
	PORT_BIT(0x60, IP_ACTIVE_HIGH, IPT_CUSTOM) // freq sel
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_UNKNOWN)

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, blitz_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void blitz_state::blitz(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->nvram_set_default_value(~0);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(blitz_state::standby));
	m_maincpu->write_port1().set(FUNC(blitz_state::p1_w));
	m_maincpu->write_port2().set(FUNC(blitz_state::p2_w));
	m_maincpu->read_port3().set(FUNC(blitz_state::p3_r));
	m_maincpu->write_port3().set(FUNC(blitz_state::p3_w));
	m_maincpu->read_port4().set(FUNC(blitz_state::power_r));
	m_maincpu->write_port4().set(FUNC(blitz_state::lcd_segs_w<0>));
	m_maincpu->write_port5().set(FUNC(blitz_state::lcd_segs_w<1>));
	m_maincpu->read_port6().set(FUNC(blitz_state::p6_r));
	m_maincpu->write_port6().set(FUNC(blitz_state::p6_w));
	m_maincpu->write_port6().append(FUNC(blitz_state::lcd_segs_w<2>));
	m_maincpu->write_port7().set(FUNC(blitz_state::lcd_segs_w<3>));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::MAGNETS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->init_cb().append(FUNC(blitz_state::init_board));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 22);
	m_lcd_pwm->output_x().set(FUNC(blitz_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 406/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(8, 8);
	m_led_pwm->set_bri_levels(0.25);

	config.set_default_layout(layout_saitek_blitz);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( kblitz )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 ) // serial S004184xx
	ROM_LOAD("90_saitek_86058151sa9_3258a03p.u1", 0x0000, 0x8000, CRC(636ccde5) SHA1(c333ea3260892458a9d0c37db849d7d290674f86) )

	ROM_REGION( 94712, "screen", 0 )
	ROM_LOAD("kblitz.svg", 0, 94712, CRC(b3bda86b) SHA1(9b64d3d6f6a275acd05a0f78899a5aee48d6b86b) )
ROM_END

ROM_START( kblitza )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 ) // serial S003078xx
	ROM_LOAD("h8_325_hd6473258p10_sa9_702g.u1", 0x0000, 0x8000, CRC(a5d17819) SHA1(e46c0b70aff31809ae02a5c5973935cf118f2324) )

	ROM_REGION( 94712, "screen", 0 )
	ROM_LOAD("kblitz.svg", 0, 94712, CRC(b3bda86b) SHA1(9b64d3d6f6a275acd05a0f78899a5aee48d6b86b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY, FULLNAME, FLAGS
SYST( 1990, kblitz,  0,      0,      blitz,   blitz, blitz_state, empty_init, "Saitek / Heuristic Software", "Kasparov Blitz (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, kblitza, kblitz, 0,      blitz,   blitz, blitz_state, empty_init, "Saitek / Heuristic Software", "Kasparov Blitz (set 2)", MACHINE_SUPPORTS_SAVE )
