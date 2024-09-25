// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Mentor 16 (model 892)

NOTE: Turn the power switch off before exiting MAME, otherwise NVRAM won't save
properly.

Hardware notes:
- PCB label: 100103
- Hitachi HD6301Y0P @ 8MHz
- 2*4-digit LCD panels
- piezo, 16+4 LEDs, 8*8 chessboard buttons

TODO:
- if/when MAME supports an exit callback, hook up power-off switch to that
- is Novag Amigo the same ROM? MCU label is also "892A", but QFP, ROM serial M44

BTANB:
- piezo sounds glitchy/fast (which is weird when compared against other Novag
  chesscomputers), verified with 2 videos

*******************************************************************************/

#include "emu.h"

#include "cpu/m6800/m6801.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "novag_mentor16.lh"


namespace {

class mentor16_state : public driver_device
{
public:
	mentor16_state(const machine_config &mconfig, device_type type, const char *tag) :
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

	void mentor16(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_off) { if (newval) m_power = false; }

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { m_power = true; }

private:
	// devices/pointers
	required_device<hd6301y0_cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_2bit_ones_complement_device> m_dac;
	required_ioport_array<2> m_inputs;
	output_finder<4, 16> m_out_lcd;
	output_finder<8> m_out_digit;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u16 m_lcd_segs = 0;
	u8 m_lcd_com = 0;
	emu_timer *m_piezo_delay;
	u8 m_piezo_data = 0;

	// I/O handlers
	void standby(int state);

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	template <int N> void lcd_segs_w(u8 data);
	void lcd_com_w(u8 data);

	void update_piezo(s32 param);
	void p2_w(u8 data);
	u8 p5_r();
	u8 p6_r();
	void p6_w(u8 data);
};

void mentor16_state::machine_start()
{
	m_piezo_delay = timer_alloc(FUNC(mentor16_state::update_piezo), this);

	m_out_lcd.resolve();
	m_out_digit.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
	save_item(NAME(m_piezo_data));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// LCD

void mentor16_state::lcd_pwm_w(offs_t offset, u8 data)
{
	// output raw segment data
	m_out_lcd[offset & 0x3f][offset >> 6] = data;

	// convert to 7seg digit
	int d = offset >> 6 & 0xe;
	m_out_digit[d >> 1] =
			m_out_lcd[0][d | 1] << 0 |
			m_out_lcd[1][d | 1] << 1 |
			m_out_lcd[2][d | 1] << 2 |
			m_out_lcd[3][d | 0] << 3 |
			m_out_lcd[2][d | 0] << 4 |
			m_out_lcd[0][d | 0] << 5 |
			m_out_lcd[1][d | 0] << 6;
}

void mentor16_state::update_lcd()
{
	m_lcd_pwm->matrix(m_lcd_com >> 4, (m_lcd_com & 1) ? ~m_lcd_segs : m_lcd_segs);
}

template <int N>
void mentor16_state::lcd_segs_w(u8 data)
{
	// P1x, P4x: LCD segments
	const u8 shift = 8 * N;
	m_lcd_segs = (m_lcd_segs & ~(0xff << shift)) | (data << shift);
	update_lcd();
}

void mentor16_state::lcd_com_w(u8 data)
{
	// P30: 4066 Y0-Y3, P34-P37: 4066 E0-E3
	// 4066 Z0-Z3: LCD commons
	m_lcd_com = data;
	update_lcd();
}


// misc

void mentor16_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_led_pwm->clear();
		m_lcd_pwm->clear();
	}
}

void mentor16_state::update_piezo(s32 param)
{
	m_piezo_data = param & 3;
	m_dac->write(m_piezo_data);
}

void mentor16_state::p2_w(u8 data)
{
	// P20-P27: input mux, led data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}

u8 mentor16_state::p5_r()
{
	u8 data = 0;

	// P50-P57: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7);

	return ~data;
}

u8 mentor16_state::p6_r()
{
	u8 data = 0;

	// P60,P61: read buttons
	for (int i = 0; i < 2; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	// P62,P63: piezo
	data |= m_piezo_data << 2;

	// P67: power state
	data |= (m_power) ? 0x80 : 0x00;
	return data ^ 0xf3;
}

void mentor16_state::p6_w(u8 data)
{
	// P62,P63: piezo (relies on short delay at rising edge)
	update_piezo(data >> 2 & m_piezo_data);
	m_piezo_delay->adjust(attotime::from_usec(5), data >> 2 & 3);

	// P64-P66: led select
	m_led_pwm->write_my(~data >> 4 & 7);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mentor16 )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Player/Player / King")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Random / Queen")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Sound / Bishop")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Solve Mate / Knight")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Depth Search / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Autoplay / Pawn")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Set Level")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_CODE(KEYCODE_N) PORT_NAME("New Game")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Set Up / Verify")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Change Color")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Clear Board")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Acc. Time")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Move Time")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Hint / Show Moves")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Go")

	PORT_START("POWER") // needs to be triggered for nvram to work
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, mentor16_state, power_off, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mentor16_state::mentor16(machine_config &config)
{
	// basic machine hardware
	HD6301Y0(config, m_maincpu, 8_MHz_XTAL);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(hd6301y0_cpu_device::nvram_set_battery));
	m_maincpu->standby_cb().set(FUNC(mentor16_state::standby));
	m_maincpu->out_p1_cb().set(FUNC(mentor16_state::lcd_segs_w<0>));
	m_maincpu->out_p2_cb().set(FUNC(mentor16_state::p2_w));
	m_maincpu->out_p3_cb().set(FUNC(mentor16_state::lcd_com_w));
	m_maincpu->out_p4_cb().set(FUNC(mentor16_state::lcd_segs_w<1>));
	m_maincpu->in_p5_cb().set(FUNC(mentor16_state::p5_r));
	m_maincpu->in_p6_cb().set(FUNC(mentor16_state::p6_r));
	m_maincpu->in_p6_override_mask(0x0c); // reads back from live piezo
	m_maincpu->out_p6_cb().set(FUNC(mentor16_state::p6_w));

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 16);
	m_lcd_pwm->output_x().set(FUNC(mentor16_state::lcd_pwm_w));

	PWM_DISPLAY(config, m_led_pwm).set_size(3, 8);
	config.set_default_layout(layout_novag_mentor16);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_2BIT_ONES_COMPLEMENT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.125);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( mentor16 )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD("novag_892a_31y0rk62p.u1", 0x0000, 0x4000, CRC(78176c7b) SHA1(fbd1b29efa5d80411754bf7cd6b80f388e065321) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1989, mentor16, 0,      0,      mentor16, mentor16, mentor16_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Mentor 16", MACHINE_SUPPORTS_SAVE )
