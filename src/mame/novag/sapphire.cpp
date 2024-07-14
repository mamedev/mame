// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Sapphire (model 9304)

Handheld chess computer. It's the successor to Novag Super VIP, whereas Ruby is
the successor to Novag VIP. The chess engine is by David Kittinger again.

Hardware notes:
- PCB label: 100168 REV A
- Hitachi H8/325 MCU (mode 2), 26.601712MHz XTAL
- 32KB EPROM (M27C256B-12F1), 128KB SRAM (KM681000ALG-10)
- LCD with 4 7segs and custom segments, same as Novag VIP
- RJ-12 port for Novag Super System (always 9600 baud)
- 24 buttons, piezo

TODO:
- Novag Super System peripherals don't work due to serial clock drift, baud rate
  differs a bit between host and client, m6801 serial emulation issue (to work
  around it, underclock sapphire to exactly 26.4192MHz)
- it does a cold boot at every reset, so nvram won't work properly unless MAME
  adds some kind of auxillary autosave state feature at power-off

BTANB:
- Average Time level (AT) does not work properly after a few moves, this is
  mentioned in the manual and it suggests to set user programmable time control

*******************************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/h8/h8325.h"
#include "machine/nvram.h"
#include "sound/dac.h"
#include "video/pwm.h"

#include "screen.h"
#include "speaker.h"

// internal artwork
#include "novag_sapphire.lh"


namespace {

class sapphire_state : public driver_device
{
public:
	sapphire_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_memory(*this, "memory"),
		m_nvram(*this, "nvram", 0x20000, ENDIANNESS_BIG),
		m_rambank(*this, "rambank"),
		m_lcd_pwm(*this, "lcd_pwm"),
		m_dac(*this, "dac"),
		m_rs232(*this, "rs232"),
		m_inputs(*this, "IN.%u", 0),
		m_out_lcd(*this, "s%u.%u", 0U, 0U)
	{ }

	void sapphire(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(power_switch);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override { set_power(true); }

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	memory_view m_memory;
	memory_share_creator<u8> m_nvram;
	required_memory_bank m_rambank;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<3> m_inputs;
	output_finder<4, 10> m_out_lcd;

	bool m_power = false;
	u8 m_inp_mux = 0;
	u8 m_lcd_sclk = 0;
	u16 m_lcd_data = 0;
	u8 m_lcd_segs2 = 0;

	void main_map(address_map &map);

	// I/O handlers
	void set_power(bool power);
	u8 power_r();

	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void lcd_data_w(u8 data);

	u8 read_inputs();
	void p4_w(u8 data);
	u8 p5_r();
	void p6_w(u8 data);
	u8 p7_r();
};

void sapphire_state::machine_start()
{
	m_out_lcd.resolve();

	m_rambank->configure_entries(0, 4, m_nvram, 0x8000);
	m_memory.select(0);

	// register for savestates
	save_item(NAME(m_power));
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_sclk));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_segs2));
}



/*******************************************************************************
    I/O
*******************************************************************************/

// power

void sapphire_state::set_power(bool power)
{
	// power switch is tied to IRQ2
	m_maincpu->set_input_line(INPUT_LINE_IRQ2, power ? ASSERT_LINE : CLEAR_LINE);
	m_power = power;
}

INPUT_CHANGED_MEMBER(sapphire_state::power_switch)
{
	if (newval)
		set_power(bool(param));
}

u8 sapphire_state::power_r()
{
	// P66: power switch (IRQ2)
	return m_power ? 0xbf : 0xff;
}


// LCD

void sapphire_state::lcd_pwm_w(offs_t offset, u8 data)
{
	m_out_lcd[offset & 0x3f][offset >> 6] = data;
}

void sapphire_state::update_lcd()
{
	for (int i = 0; i < 4; i++)
	{
		// LCD common is analog (voltage level)
		const u8 com = population_count_32(m_lcd_data >> (8 + (i * 2)) & 3);
		u16 segs = (m_lcd_data & 0xff) | (m_lcd_segs2 << 8 & 0x300);
		segs = (com == 0) ? segs : (com == 2) ? ~segs : 0;

		m_lcd_pwm->write_row(i, segs);
	}
}

void sapphire_state::lcd_data_w(u8 data)
{
	// P62: 2*14015B R
	if (data & 4)
		m_lcd_data = 0;

	// P60: 2*14015B C (chained)
	else if (data & 1 && !m_lcd_sclk)
	{
		// P61: 14015B D, outputs to LCD
		m_lcd_data = m_lcd_data << 1 | BIT(data, 1);
	}
	m_lcd_sclk = data & 1;

	// P64,P65: 2 more LCD segments
	m_lcd_segs2 = data >> 4 & 3;
	update_lcd();
}


// misc

u8 sapphire_state::read_inputs()
{
	u8 data = 0;

	for (int i = 0; i < 3; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	return ~data;
}

void sapphire_state::p4_w(u8 data)
{
	// P40: speaker out
	m_dac->write(data & 1);

	// P41,P42: RAM bank
	m_rambank->set_entry(data >> 1 & 3);

	// P43-P45: input mux
	m_inp_mux = ~data >> 3 & 7;
}

u8 sapphire_state::p5_r()
{
	// P52-P55: read buttons (low)
	return read_inputs() << 2 | 0xc3;
}

void sapphire_state::p6_w(u8 data)
{
	// P63: RAM/ROM CS
	m_memory.select(BIT(data, 3));
}

u8 sapphire_state::p7_r()
{
	// P70-P73: read buttons (high)
	return read_inputs() >> 4 | 0xf0;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void sapphire_state::main_map(address_map &map)
{
	map(0x8000, 0xffff).view(m_memory);
	m_memory[0](0x8000, 0xffff).rom().region("maincpu", 0x8000);
	m_memory[1](0x8000, 0xffff).bankrw(m_rambank);

	map(0xff90, 0xff9f).unmaprw(); // reserved for H8 registers
	map(0xffb0, 0xffff).unmaprw(); // "
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( sapphire )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("NG")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_NAME("C/CB")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("Option 1/2 / Random")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("Level")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("Hint / Analyze")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("Ver/Set")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("Right / Easy")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("GO")

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("Pawn / Load Game / ProDelete")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("Knight / Save Game / ProSave")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("Bishop / Training / ProPrior")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("Rook / Referee / ProPrint")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("Queen / Sound / BkSelect")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("King / Info / Restore")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("Left / Next Best")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("Color")

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("H 8")
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("G 7")
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("F 6")
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("E 5")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("D 4")
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("C 3")
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("B 2")
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("A 1")

	PORT_START("BATT")
	PORT_CONFNAME( 0x80, 0x80, "Battery Status" )
	PORT_CONFSETTING(    0x00, "Low" )
	PORT_CONFSETTING(    0x80, DEF_STR( Normal ) )
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("LOCK")
	PORT_CONFNAME( 0x80, 0x00, "Keyboard Lock")
	PORT_CONFSETTING(    0x00, DEF_STR( Off ) )
	PORT_CONFSETTING(    0x80, DEF_STR( On ) )
	PORT_BIT(0x7f, IP_ACTIVE_HIGH, IPT_UNUSED)

	PORT_START("POWER")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_POWER_ON) PORT_CHANGED_MEMBER(DEVICE_SELF, sapphire_state, power_switch, 1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_POWER_OFF) PORT_CHANGED_MEMBER(DEVICE_SELF, sapphire_state, power_switch, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void sapphire_state::sapphire(machine_config &config)
{
	// basic machine hardware
	H8325(config, m_maincpu, 26.601712_MHz_XTAL);
	m_maincpu->set_mode(2);
	m_maincpu->set_addrmap(AS_PROGRAM, &sapphire_state::main_map);
	m_maincpu->nvram_enable_backup(true);
	m_maincpu->standby_cb().set(m_maincpu, FUNC(h8325_device::nvram_set_battery));
	m_maincpu->standby_cb().append([this](int state) { if (state) m_lcd_pwm->clear(); });
	m_maincpu->write_sci_tx<0>().set(m_rs232, FUNC(rs232_port_device::write_txd));
	m_maincpu->read_port2().set_ioport("BATT").invert();
	m_maincpu->read_port4().set_ioport("LOCK").invert();
	m_maincpu->write_port4().set(FUNC(sapphire_state::p4_w));
	m_maincpu->read_port5().set(FUNC(sapphire_state::p5_r));
	m_maincpu->read_port6().set(FUNC(sapphire_state::power_r));
	m_maincpu->write_port6().set(FUNC(sapphire_state::p6_w));
	m_maincpu->write_port6().append(FUNC(sapphire_state::lcd_data_w));
	m_maincpu->read_port7().set(FUNC(sapphire_state::p7_r));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(sapphire_state::lcd_pwm_w));
	m_lcd_pwm->set_bri_levels(0.05);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2.5, 606/2.5);
	screen.set_visarea_full();

	config.set_default_layout(layout_novag_sapphire);

	// rs232 (configure after video)
	RS232_PORT(config, m_rs232, default_rs232_devices, nullptr);
	m_rs232->rxd_handler().set(m_maincpu, FUNC(h8325_device::sci_rx_w<0>));

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( sapphire ) // ID = SAPPHIRE V1.01
	ROM_REGION16_BE( 0x10000, "maincpu", 0 )
	ROM_LOAD("novag_9304-010053_6433258b46f.u1", 0x0000, 0x8000, CRC(bfc39f4b) SHA1(dc96440c070e903772f4485757443dd690e92120) )
	ROM_LOAD("bk301_26601.u2", 0x8000, 0x8000, CRC(648ebe8f) SHA1(2883f962a0bf17426fd809b9f2c01ce3dec0df1b) )

	ROM_REGION( 36256, "screen", 0 )
	ROM_LOAD("nvip.svg", 0, 36256, CRC(3373e0d5) SHA1(25bfbf0405017388c30f4529106baccb4723bc6b) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY, FULLNAME, FLAGS
SYST( 1994, sapphire, 0,      0,      sapphire, sapphire, sapphire_state, empty_init, "Novag Industries", "Sapphire (Novag)", MACHINE_SUPPORTS_SAVE )
