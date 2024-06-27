// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag (Perfect Technology*) Star Diamond

*: Novag Industries dissolved in 2000. The Novag brand continued for a few years
under Perfect Technology, Ltd., established by the daughter of Novag's founder.
The main programmer (David Kittinger) also moved to the new company.

Although there may be newer Novag products with (old) software by David Kittinger,
Star Diamond was the last chess computer that he personally worked on.

Hardware notes:
- PCB label: TF-05 94V0Δ
- Hitachi H8S/2312 12312VTE25V, 25MHz XTAL
- 512KB Flash ROM (SST 39VF400A), only 192KB used
- 256KB RAM (2*Hynix HY62V8100B)
- LCD with 6 7segs and custom segments
- RJ-12 port for Novag Super System (always 57600 baud)
- piezo, 16 LEDs, button sensors chessboard

TODO:
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/h8/h8s2319.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_sdiamond.lh"


namespace {

class sdiamond_state : public driver_device
{
public:
	sdiamond_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void sdiamond(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_switch);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { set_power(true); }

private:
	// devices/pointers
	required_device<h8s2312_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<4> m_inputs;
	output_finder<4, 16> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u8 m_lcd_sclk = 0;
	u16 m_lcd_segs = 0;
	u8 m_lcd_com = 0;

	void main_map(address_map &map);

	// I/O handlers
	void standby(int state);
	void set_power(bool power);

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void lcd_segs_w(u8 data);
	void lcd_com_w(offs_t offset, u8 data, u8 mem_mask);

	void p1_w(u8 data);
	u8 p2_r();
	u8 p4_r();
	u8 pf_r();
	void pg_w(u8 data);
};

void sdiamond_state::machine_start()
{
	m_out_lcd.resolve();

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_sclk));
	save_item(NAME(m_lcd_segs));
	save_item(NAME(m_lcd_com));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void sdiamond_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

void sdiamond_state::set_power(bool power)
{
	// power switch is tied to NMI
	m_maincpu->set_input_line(INPUT_LINE_NMI, power ? ASSERT_LINE : CLEAR_LINE);
	m_power = power;
}

INPUT_CHANGED_MEMBER(sdiamond_state::power_switch)
{
	if (newval)
		set_power(bool(param));
}


// LCD

void sdiamond_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void sdiamond_state::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		// LCD common is 0/1/Hi-Z
		const u16 data = BIT(m_lcd_com, i + 4) ? (BIT(m_lcd_com, i) ? ~m_lcd_segs : m_lcd_segs) : 0;
		m_lcd_pwm->write_row(i, data);
	}
}

void sdiamond_state::lcd_com_w(offs_t offset, u8 data, u8 mem_mask)
{
	// P20-P23: LCD common
	m_lcd_com = mem_mask << 4 | (data & 0xf);
	update_lcd();
}

void sdiamond_state::lcd_segs_w(u8 data)
{
	// P35: 2*14015B C (chained)
	if (data & 0x20 && !m_lcd_sclk)
	{
		// P34: 14015B D, outputs to LCD segments
		m_lcd_segs = m_lcd_segs << 1 | BIT(data, 4);
		update_lcd();
	}

	m_lcd_sclk = BIT(data, 5);
}


// misc

void sdiamond_state::p1_w(u8 data)
{
	// P10-P17: input mux, led data
	m_inp_mux = ~data;
	m_led_pwm->write_mx(~data);
}

u8 sdiamond_state::p2_r()
{
	u8 data = 0;

	// P26: power switch
	if (!m_power)
		data |= 0x40;

	// P27: battery status
	data |= m_inputs[3]->read() << 7;
	return data | 0xf;
}

u8 sdiamond_state::p4_r()
{
	u8 data = 0;

	// P40-P47: read chessboard
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= m_board->read_rank(i ^ 7);

	return ~data;
}

u8 sdiamond_state::pf_r()
{
	u8 data = 0;

	// PF0-PF2: read buttons
	for (int i = 0; i < 3; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return ~data;
}

void sdiamond_state::pg_w(u8 data)
{
	// PG0,PG1: led select
	m_led_pwm->write_my(~data & 3);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sdiamond_state::main_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
	map(0x400000, 0x43ffff).ram().share("nvram");
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sdiamond )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Next Best / Take Back / Print Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Trace Forward / Print Game")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Easy / Replay")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Random / Video")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Restore / Human")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Book Select / Auto/Demo")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Auto Clock")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / Print Moves")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Verify / Setup / Rating")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Pro-op Print / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-op Priority / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Pro-op Delete / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Pro-op Save / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Load Game / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Save Game / Pawn")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Info")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Training")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Option 1/2")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Set Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Clear / Clear Board")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")

	PORT_START("IN.3")
	PORT_CONFNAME( 0x01, 0x00, "Battery Status" )
	PORT_CONFSETTING(    0x01, "Low" )
	PORT_CONFSETTING(    0x00, DEF_STR( Normal ) )

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, sdiamond_state, power_switch, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, sdiamond_state, power_switch, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sdiamond_state::sdiamond(machine_config &config)
{
	// basic machine hardware
	H8S2312(config, m_maincpu, 25_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &sdiamond_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8s2312_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(sdiamond_state::standby));
	m_maincpu->write_sci_tx<0>().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_maincpu->write_port1().set(FUNC(sdiamond_state::p1_w));
	m_maincpu->read_port2().set(FUNC(sdiamond_state::p2_r));
	m_maincpu->write_port2().set(FUNC(sdiamond_state::lcd_com_w));
	m_maincpu->write_port3().set(FUNC(sdiamond_state::lcd_segs_w));
	m_maincpu->read_port4().set(FUNC(sdiamond_state::p4_r));
	m_maincpu->read_portf().set(FUNC(sdiamond_state::pf_r));
	m_maincpu->write_portf().set(m_dac, FUNC(dac_1bit_device::write)).bit(6);
	m_maincpu->write_portg().set(FUNC(sdiamond_state::pg_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 16);
	m_lcd_pwm->output_x().set(FUNC(sdiamond_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/5, 671/5);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_novag_sdiamond);

	// rs232 (configure after video)
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_maincpu, FUNC(h8s2312_device::sci_rx_w<0>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sdiamond ) // ID = H8S/SD V1.04
	ROM_REGION16_BE( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP("39vf400a.ic3", 0x00000, 0x80000, CRC(ee9a4fee) SHA1(b86e5efa5b7b9ddbe9fe1dabfe8cbc2bc40809b8) )

	ROM_REGION( 72533, "screen", 0 )
	ROM_LOAD("sdiamond.svg", 0, 72533, CRC(34944b61) SHA1(4a0536ac07790cced9f9bf15522b17ebc375ff8a) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 2003, sdiamond, 0,      0,      sdiamond, sdiamond, sdiamond_state, empty_init, "Perfect Technology", "Star Diamond (v1.04)", MACHINE_SUPPORTS_SAVE )
