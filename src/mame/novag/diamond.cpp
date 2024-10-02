// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Diamond (model 9303) / Diamond II (model 38602)

Hardware notes:

Diamond:
- PCB label: 100165 REV B
- Hitachi H8/325 MCU (mode 2), 26.601712MHz XTAL
- 32KB EPROM (TC57256AD-12), 128KB SRAM (HM628128ALP-7)
- LCD with 6 7segs and custom segments (same as Novag VIP)
- RJ-12 port for Novag Super System (always 9600 baud)
- piezo, 16 LEDs, button sensors chessboard

Diamond II:
- PCB label: 100208 REV B
- Hitachi H8/325 MCU (mode 2), 32MHz XTAL
- 128KB EPROM (27C010), 128KB SRAM (KM681000BLG-7)
- Sapphire II LCD instead of VIP, the rest is the same as Diamond

Diamond II MCU and EPROM are the same as Sapphire II. MCU pin P62 determines
which hardware it runs on, see sapphire.cpp for Sapphire II.

TODO:
- Novag Super System peripherals don't work due to serial clock drift, baud rate
  differs a bit between host and client, m6801 serial emulation issue (to work
  around it, underclock diamond to exactly 26.4192MHz, diamond2 to 31.9488MHz)
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

BTANB:
- diamond has the same AT level bug as sapphire (it works fine in diamond2)

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/h8/h8325.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_diamond.lh"
#include "novag_diamond2.lh"


namespace {

class diamond_state : public driver_device
{
public:
	diamond_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_memory(*this, "memory"),
		m_nvram(*this, "nvram", 0x20000, ENDIANNESS_BIG),
		m_rambank(*this, "rambank"),
		m_rombank(*this, "rombank"),
		m_board(*this, "board"),
		m_led_pwm(*this, "led_pwm"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void diamond(machine_config &config);
	void diamond2(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_switch);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override { set_power(true); }

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	memory_view m_memory;
	memory_share_creator<u8> m_nvram;
	required_memory_bank m_rambank;
	optional_memory_bank m_rombank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_led_pwm;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<3> m_inputs;
	output_finder<4, 10+6> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u8 m_led_select = 0;
	u8 m_lcd_sclk = 0;
	u32 m_lcd_data = 0;
	u8 m_lcd_segs2 = 0;

	void diamond_map(address_map &map) ATTR_COLD;
	void diamond2_map(address_map &map) ATTR_COLD;

	// I/O handlers
	void standby(int state);
	void set_power(bool power);
	u8 power_r();

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void d1_lcd_data_w(u8 data);
	void d2_lcd_data_w(u8 data);

	u8 read_buttons();
	u8 read_board();
	u8 input_r();
	u8 input2_r();
	void cs_w(u8 data);
	void bank_w(u8 data);
	void update_leds();
	template <int N> void leds_w(u8 data);
	void p4_w(u8 data);
	void p5_w(u8 data);
};

void diamond_state::machine_start()
{
	m_out_lcd.resolve();

	if (m_rombank)
		m_rombank->configure_entries(0, 4, memregion("eprom")->base(), 0x8000);

	m_rambank->configure_entries(0, 4, m_nvram, 0x8000);
	m_memory.select(0);

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_led_select));
	save_item(NAME(m_lcd_sclk));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_segs2));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void diamond_state::standby(int state)
{
	// clear display
	if (state)
	{
		m_lcd_pwm->clear();
		m_led_pwm->clear();
	}
}

void diamond_state::set_power(bool power)
{
	// power switch is tied to IRQ2
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, power ? ASSERT_LINE : CLEAR_LINE);
	m_power = power;
}

INPUT_CHANGED_MEMBER(diamond_state::power_switch)
{
	if (newval)
		set_power(bool(param));
}

u8 diamond_state::power_r()
{
	// P66: power switch (IRQ2)
	return m_power ? 0xbf : 0xff;
}


// LCD

void diamond_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void diamond_state::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		const u8 shift = m_lcd_pwm->width() & 0x18;

		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_data >> (shift + (i * 2)) & 3);
		u16 segs = m_lcd_data & ((1 << shift) - 1);
		segs |= m_lcd_segs2 << shift; // diamond

		m_lcd_pwm->write_row(i, (com == 0) ? segs : (com == 2) ? ~segs : 0);
	}
}

void diamond_state::d1_lcd_data_w(u8 data)
{
	// P60,P61: same as diamond2
	d2_lcd_data_w(data);

	// P62: 3*14015B R
	if (data & 4)
		m_lcd_data = 0;

	// 2 more LCD segments after common
	m_lcd_segs2 = m_lcd_data >> 16 & 3;
	update_lcd();
}

void diamond_state::d2_lcd_data_w(u8 data)
{
	// P60: 3*14015B C (chained)
	if (data & 1 && !m_lcd_sclk)
	{
		// P61: 14015B D, outputs to LCD
		m_lcd_data = m_lcd_data << 1 | BIT(data, 1);
		update_lcd();
	}
	m_lcd_sclk = data & 1;
}


// misc

u8 diamond_state::read_buttons()
{
	u8 data = 0;

	for (int i = 0; i < 3; i++)
		if (m_inp_mux & m_inputs[i]->read())
			data |= 1 << i;

	return ~data;
}

u8 diamond_state::read_board()
{
	u8 data = 0;

	// priority encoded (either a 74148 on d1, or 2*7421 on d2)
	for (int i = 0; i < 8; i++)
		if (BIT(m_inp_mux, i))
			data |= (count_leading_zeros_32(m_board->read_rank(i)) - 24) ^ 8;

	return ~data;
}

u8 diamond_state::input_r()
{
	// P71-P73: multiplexed inputs
	return (read_buttons() & read_board()) << 1 | 0xf1;
}

u8 diamond_state::input2_r()
{
	// P27: chessboard active (P47 on d2)
	return read_board() << 4 | 0x7f;
}

void diamond_state::cs_w(u8 data)
{
	// P41: ROM/RAM CS (P63 on d2)
	m_memory.select(data & 1);
}

void diamond_state::bank_w(u8 data)
{
	// P64,P65: ROM/RAM bankswitch
	const u8 bank = data >> 4 & 3;
	m_rambank->set_entry(bank);

	// only diamond2 has ROM banks
	if (m_rombank)
		m_rombank->set_entry(bank);

	// other: LCD (see above)
}

void diamond_state::update_leds()
{
	m_led_pwm->matrix(m_led_select, m_inp_mux);
}

template <int N>
void diamond_state::leds_w(u8 data)
{
	// P63/P41,P70: select LEDs
	const u8 mask = 1 << N;
	m_led_select = (m_led_select & ~mask) | ((data & 1) ? 0 : mask);
	update_leds();
}

void diamond_state::p4_w(u8 data)
{
	// P40: speaker out
	m_dac->write(data & 1);

	// P42-P45: input mux low
	m_inp_mux = (m_inp_mux & 0xf0) | (~data >> 2 & 0xf);
	update_leds();
}

void diamond_state::p5_w(u8 data)
{
	// P52-P55: input mux high
	m_inp_mux = (m_inp_mux & 0x0f) | (~data << 2 & 0xf0);
	update_leds();
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void diamond_state::diamond_map(address_map &map)
{
	map(0x8000, 0xffff).view(m_memory);
	m_memory[0](0x8000, 0xffff).rom().region("eprom", 0);
	m_memory[1](0x8000, 0xffff).bankrw(m_rambank);

	map(0xff90, 0xff9f).unmaprw(); // reserved for H8 registers
	map(0xffb0, 0xffff).unmaprw(); // "
}

void diamond_state::diamond2_map(address_map &map)
{
	diamond_map(map);
	m_memory[0](0x8000, 0xffff).bankr(m_rombank);
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( diamond )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("Go")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("Info")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("Hint")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("Training")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Next Best")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("Set Level")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Clear / Clear Board")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("New Game")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Take Back / Auto/Demo")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Trace Forward / Autoclock")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3) PORT_NAME("Easy / Replay")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Random / Human")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Restore / Video")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Book Select / Print Moves")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Print Game")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / Print Board")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("Color")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Verify / Setup")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("Pro-op Print / King")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("Pro-op Priority / Queen")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("Pro-op Delete / Rook")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("Pro-op Save / Bishop")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Load Game / Knight")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Save Game / Pawn")

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, diamond_state, power_switch, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, diamond_state, power_switch, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( diamond2 )
	PORT_INCLUDE( diamond )

	PORT_MODIFY("IN.0")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("Option 1/2")

	PORT_MODIFY("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) PORT_NAME("Next Best / Take Back / Print Board")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) PORT_NAME("Trace Forward / Print Game")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) PORT_NAME("Random / Video")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) PORT_NAME("Restore / Human")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) PORT_NAME("Book Select / Auto/Demo")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) PORT_NAME("Sound / Auto Clock")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) PORT_NAME("Referee / Print Moves")

	PORT_MODIFY("IN.2")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("Verify / Setup / Rating")

	PORT_START("BATT")
	PORT_CONFNAME( 0x80, 0x80, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x80, DEF_STR( Normal ) )
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void diamond_state::diamond(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 26.601712_MHz_XTAL);
	m_maincpu->set_mode(2);
	m_maincpu->set_addrmap(AS_PROGRAM, &diamond_state::diamond_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append(FUNC(diamond_state::standby));
	m_maincpu->write_sci_tx<0>().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_maincpu->read_port2().set(FUNC(diamond_state::input2_r));
	m_maincpu->write_port4().set(FUNC(diamond_state::p4_w));
	m_maincpu->write_port4().append(FUNC(diamond_state::cs_w)).bit(1);
	m_maincpu->write_port5().set(FUNC(diamond_state::p5_w));
	m_maincpu->read_port6().set(FUNC(diamond_state::power_r));
	m_maincpu->write_port6().set(FUNC(diamond_state::bank_w));
	m_maincpu->write_port6().append(FUNC(diamond_state::leds_w<1>)).bit(3);
	m_maincpu->write_port6().append(FUNC(diamond_state::d1_lcd_data_w));
	m_maincpu->read_port7().set(FUNC(diamond_state::input_r));
	m_maincpu->write_port7().set(FUNC(diamond_state::leds_w<0>)).bit(0);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(250));
	//m_board->set_nvram_enable(true);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(diamond_state::lcd_pwm_w));
	m_lcd_pwm->set_bri_levels(0.05);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/3, 606/3);
	screen.set_visarea_full();

	PWM_DISPLAY(config, m_led_pwm).set_size(2, 8);
	config.set_default_layout(layout_novag_diamond);

	// rs232 (configure after video)
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_maincpu, FUNC(h8325_device::sci_rx_w<0>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void diamond_state::diamond2(machine_config &config)
{
	diamond(config);

	// basic machine hardware
	m_maincpu->set_clock(32_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &diamond_state::diamond2_map);
	m_maincpu->read_port2().set_ioport("BATT").invert();
	m_maincpu->read_port4().set(FUNC(diamond_state::input2_r));
	m_maincpu->write_port4().set(FUNC(diamond_state::p4_w));
	m_maincpu->write_port4().append(FUNC(diamond_state::leds_w<1>)).bit(1); // pin swapped with cs_w

	// P62 input is forced low (0 = Diamond II, 1 = Sapphire II)
	m_maincpu->read_port6().set(FUNC(diamond_state::power_r)).mask(0xfb);
	m_maincpu->write_port6().set(FUNC(diamond_state::bank_w));
	m_maincpu->write_port6().append(FUNC(diamond_state::cs_w)).bit(3);
	m_maincpu->write_port6().append(FUNC(diamond_state::d2_lcd_data_w));

	// video hardware
	m_lcd_pwm->set_width(16);

	screen_device &screen(*subdevice<screen_device>("screen"));
	screen.set_size(1920/3, 671/3);
	screen.set_visarea_full();

	config.set_default_layout(layout_novag_diamond2);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( diamond ) // ID = DIAMOND 1.01
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("novag_9303-010052_6433258b47p.u6", 0x0000, 0x8000, CRC(cbc544ee) SHA1(43bc09a296f23f23f6a2656a12a9f8d4519fec10) )

	ROM_REGION16_BE( 0x8000, "eprom", 0 )
	ROM_LOAD("bk301_26601.u4", 0x0000, 0x8000, CRC(648ebe8f) SHA1(2883f962a0bf17426fd809b9f2c01ce3dec0df1b) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

ROM_START( diamond2 ) // ID = DIAMOND II 1.02
	ROM_REGION16_BE( 0x8000, "maincpu", 0 )
	ROM_LOAD("ihp_7600109_hd6433258c67f.u1", 0x0000, 0x8000, CRC(10970123) SHA1(8f72e756915de6569c3936140c775d24730e9065) )

	ROM_REGION16_BE( 0x20000, "eprom", 0 )
	ROM_LOAD("32dsii_060597.u3", 0x00000, 0x20000, CRC(c1be39c6) SHA1(e33ee655bd342fed736c2b2093a89752e695aff3) )

	ROM_REGION( 72533, "screen", 0 )
	ROM_LOAD("sapphire2.svg", 0, 72533, CRC(34944b61) SHA1(4a0536ac07790cced9f9bf15522b17ebc375ff8a) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS          INIT        COMPANY, FULLNAME, FLAGS
SYST( 1994, diamond,  0,      0,      diamond,  diamond,  diamond_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Diamond", MACHINE_SUPPORTS_SAVE )

SYST( 1997, diamond2, 0,      0,      diamond2, diamond2, diamond_state, empty_init, "Novag Industries / Intelligent Heuristic Programming", "Diamond II", MACHINE_SUPPORTS_SAVE )
