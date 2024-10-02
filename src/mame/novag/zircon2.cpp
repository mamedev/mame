// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Zircon II (model 9403)

Hardware notes (Aquamarine Risc II):
- PCB label: 100186 B
- Hitachi H8/325 MCU, 26.601712MHz XTAL
- 2*4-digit LCD panels (same as Mentor 16)
- piezo, 16+2 LEDs, 8*8 chessboard buttons

H8/325 B84 MCU is used in:
- Novag Zircon II
- Novag Jade II
- Novag Chess Wizard IQ V (Zircon II rebranded by Mitco Industries)
- Novag Aquamarine Risc II 26.6MHz (Siglo XXI version too)

Versions manufactured after around 1997 have a 16MHz H8/3214 (100186 C PCB), it's
a bit faster than the ~26.6MHz H8/325 due to the latter /2 divider. Newer revisions
of Chess Wizard IQ V and Aquamarine Risc II have it, presumably others too.

TODO:
- dump/add H8/3214 version
- is the first Novag Zircon/Jade on similar hardware?
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "cpu/h8/h8325.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_zircon2.lh"


namespace {

class zircon2_state : public driver_device
{
public:
	zircon2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U),
		m_out_digit(*this, "digit%u", 0U)
	{ }

	void zircon2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_switch);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { set_power(true); }

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<8, 8> m_out_lcd;
	output_finder<8> m_out_digit;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u16 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	// I/O handlers
	void standby(int state);
	void set_power(bool power);
	u8 power_r();

	void lcd_pwm_raw_w(offs_t offset, u8 data);
	void lcd_pwm_digit_w(offs_t offset, u64 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	void p1_w(u8 data);
	u8 p4_r();
	u8 p5_r();
	void p6_w(u8 data);
};

void zircon2_state::machine_start()
{
	m_out_lcd.resolve();
	m_out_digit.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void zircon2_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

void zircon2_state::set_power(bool power)
{
	// power switch is tied to IRQ0
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, power ? ASSERT_LINE : CLEAR_LINE);
	m_power = power;
}

INPUT_CHANGED_MEMBER(zircon2_state::power_switch)
{
	if (newval)
		set_power(bool(param));
}

u8 zircon2_state::power_r()
{
	// P64: power switch (IRQ0)
	return m_power ? 0xef : 0xff;
}


// LCD

void zircon2_state::lcd_pwm_raw_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void zircon2_state::lcd_pwm_digit_w(offs_t offset, u64 data)
{
	m_out_digit[offset] = data;
}

void zircon2_state::update_lcd()
{
	for (int digit = 0; digit < 8; digit++)
	{
		u8 data = 0;
		for (int i = 0; i < 4; i++)
		{
			// 4 commons per digit, 2 output pins per common (analog voltage level)
			const u8 com = population_count_32(m_lcd_com >> (i * 2) & 3);
			const u16 segs = (com == 0) ? m_lcd_segs : (com == 2) ? ~m_lcd_segs : 0;
			data = data << 2 | (segs >> (digit * 2) & 3);
		}

		m_lcd_pwm->write_row(digit, bitswap<8>(data,1,4,6,2,0,3,5,7));
	}
}

template <int N>
void zircon2_state::lcd_segs_w(u8 data)
{
	// P3x, P7x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}

void zircon2_state::lcd_com_w(u8 data)
{
	// P20-P27: LCD commons
	m_lcd_com = data;
	update_lcd();
}


// misc

void zircon2_state::p1_w(u8 data)
{
	// P10-P17: input mux, LED data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}

u8 zircon2_state::p4_r()
{
	u8 data = 0;

	// P40-P47: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i);

	return ~data;
}

u8 zircon2_state::p5_r()
{
	u8 data = 0;

	// P50,P51: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return ~data;
}

void zircon2_state::p6_w(u8 data)
{
	// P61,P62: speaker out
	m_dac->write(data >> 1 & 3);

	// P63,P65,P66: LED select
	m_led_pwm->write_my(bitswap<3>(~data,3,5,6));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( zircon2 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Change Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Take Back / Next Best")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("King / Easy")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Queen / Random")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Rook / Restore")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Bishop / Info")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Knight / Sound")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Pawn / Referee")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Trace Forward / Autoplay")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Verify / Set Up")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Training")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Hint")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Set Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Clear / Clear Board")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("New Game")

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, zircon2_state, power_switch, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, zircon2_state, power_switch, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void zircon2_state::zircon2(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 26.601712_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(zircon2_state::standby));
	m_maincpu->write_port1().set(FUNC(zircon2_state::p1_w));
	m_maincpu->write_port2().set(FUNC(zircon2_state::lcd_com_w));
	m_maincpu->write_port3().set(FUNC(zircon2_state::lcd_segs_w<0>));
	m_maincpu->read_port4().set(FUNC(zircon2_state::p4_r));
	m_maincpu->read_port5().set(FUNC(zircon2_state::p5_r));
	m_maincpu->read_port6().set(FUNC(zircon2_state::power_r));
	m_maincpu->write_port6().set(FUNC(zircon2_state::p6_w));
	m_maincpu->write_port7().set(FUNC(zircon2_state::lcd_segs_w<1>));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(8, 8);
	m_lcd_pwm->set_segmask(0xff, 0xff);
	m_lcd_pwm->output_x().set(FUNC(zircon2_state::lcd_pwm_raw_w));
	m_lcd_pwm->output_digit().set(FUNC(zircon2_state::lcd_pwm_digit_w));

	PWM_DISPLAY(config, m_led_pwm).set_size(3, 8);
	config.set_default_layout(layout_novag_zircon2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( zircon2 )
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("novag_9403-010057-6433258b84f", 0x0000, 0x8000, CRC(bb0e817d) SHA1(328b3e01b4bb52400bcd5111ce674308b65f5b86) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1995, zircon2, 0,      0,      zircon2, zircon2, zircon2_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Zircon II", MACHINE_SUPPORTS_SAVE )
