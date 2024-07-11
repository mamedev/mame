// license:BSD-3-Clause
// copyright-holders:hap
// thanks-to:Berger
/*******************************************************************************

Novag Sapphire

TODO:
- currently hardlocks MAME, suspect problem with h8_sci
- everything else

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
//#include "novag_sapphire.lh"


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

protected:
	virtual void machine_start() override;

private:
	// devices/pointers
	required_device<h8325_device> m_maincpu;
	memory_view m_memory;
	memory_share_creator<u8> m_nvram;
	required_memory_bank m_rambank;
	required_device<pwm_display_device> m_lcd_pwm;
	required_device<dac_1bit_device> m_dac;
	required_device<rs232_port_device> m_rs232;
	required_ioport_array<2> m_inputs;
	output_finder<4, 10> m_out_lcd;

	u8 m_inp_mux = 0;
	u8 m_lcd_sclk = 0;
	u16 m_lcd_data = 0;
	u8 m_lcd_segs2 = 0;

	void main_map(address_map &map);

	// I/O handlers
	void lcd_pwm_w(offs_t offset, u8 data);
	void update_lcd();
	void lcd_data_w(u8 data);

	u8 p1_r();
	void p1_w(u8 data);

	u8 p2_r();
	void p2_w(u8 data);

	u8 p3_r();
	void p3_w(u8 data);

	u8 p4_r();
	void p4_w(u8 data);

	u8 p5_r();
	void p5_w(u8 data);

	u8 p6_r();
	void p6_w(u8 data);

	u8 p7_r();
	void p7_w(u8 data);
};

void sapphire_state::machine_start()
{
	m_out_lcd.resolve();

	m_rambank->configure_entries(0, 4, m_nvram, 0x8000);
	m_memory.select(0);

	// register for savestates
	save_item(NAME(m_inp_mux));
	save_item(NAME(m_lcd_sclk));
	save_item(NAME(m_lcd_data));
	save_item(NAME(m_lcd_segs2));
}



/*******************************************************************************
    I/O
*******************************************************************************/

/*

[:maincpu:port1] ddr_w ff
[:maincpu:port3] ddr_w ff
[:maincpu:port2] ddr_w 7f
[:maincpu:port4] ddr_w 3f
[:maincpu:port6] ddr_w 3f

01 01 01 00 00000000
01 01 00 01 00000000
01 00 01 01 00000000
00 01 01 01 00000000

01 01 01 11 11111111
01 01 11 01 11111111
01 11 01 01 11111111
11 01 01 01 11111111

*/

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

u8 sapphire_state::p1_r()
{
	//printf("r1 ");
	return 0xff;
}

void sapphire_state::p1_w(u8 data)
{
	//printf("w1_%X ",data);
}

u8 sapphire_state::p2_r()
{
	//printf("r2 ");
	return 0xff ^ 0x80;
}

void sapphire_state::p2_w(u8 data)
{
	//printf("w2_%X ",data);
}

u8 sapphire_state::p3_r()
{
	//printf("r3 ");
	return 0xff;
}

void sapphire_state::p3_w(u8 data)
{
	//printf("w3_%X ",data);
}

u8 sapphire_state::p4_r()
{
	//printf("r4 ");
	return 0xff ^ 0xc0;
}

void sapphire_state::p4_w(u8 data)
{
	//printf("w4_%X ",data);

	// P40: speaker out
	m_dac->write(data & 1);

	// P41,P42: RAM bank
	m_rambank->set_entry(data >> 1 & 3);
}

u8 sapphire_state::p5_r()
{
	//printf("r5 ");
	return 0xff;
}

void sapphire_state::p5_w(u8 data)
{
	//printf("w5_%X ",data);
}

u8 sapphire_state::p6_r()
{
	//printf("r6 ");
	return 0xff ^ 0x40;
}

void sapphire_state::p6_w(u8 data)
{
	// P63: RAM/ROM CS
	m_memory.select(BIT(data, 3));
}

u8 sapphire_state::p7_r()
{
	//printf("r7 ");
	return 0xff;
}

void sapphire_state::p7_w(u8 data)
{
	//printf("w7_%X ",data);
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
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I)
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
	m_maincpu->write_sci_tx<0>().set(m_rs232, FUNC(rs232_port_device::write_txd));

	m_maincpu->read_port1().set(FUNC(sapphire_state::p1_r));
	m_maincpu->write_port1().set(FUNC(sapphire_state::p1_w));

	m_maincpu->read_port2().set(FUNC(sapphire_state::p2_r));
	m_maincpu->write_port2().set(FUNC(sapphire_state::p2_w));

	m_maincpu->read_port3().set(FUNC(sapphire_state::p3_r));
	m_maincpu->write_port3().set(FUNC(sapphire_state::p3_w));

	m_maincpu->read_port4().set(FUNC(sapphire_state::p4_r));
	m_maincpu->write_port4().set(FUNC(sapphire_state::p4_w));

	m_maincpu->read_port5().set(FUNC(sapphire_state::p5_r));
	m_maincpu->write_port5().set(FUNC(sapphire_state::p5_w));

	m_maincpu->read_port6().set(FUNC(sapphire_state::p6_r));
	m_maincpu->write_port6().set(FUNC(sapphire_state::p6_w));
	m_maincpu->write_port6().append(FUNC(sapphire_state::lcd_data_w));

	m_maincpu->read_port7().set(FUNC(sapphire_state::p7_r));
	m_maincpu->write_port7().set(FUNC(sapphire_state::p7_w));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	PWM_DISPLAY(config, m_lcd_pwm).set_size(4, 10);
	m_lcd_pwm->output_x().set(FUNC(sapphire_state::lcd_pwm_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_SVG));
	screen.set_refresh_hz(60);
	screen.set_size(1920/2.5, 606/2.5);
	screen.set_visarea_full();

	//config.set_default_layout(layout_novag_sapphire);

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

ROM_START( sapphire )
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
SYST( 1994, sapphire, 0,      0,      sapphire, sapphire, sapphire_state, empty_init, "Novag Industries", "Sapphire (Novag)", MACHINE_SUPPORTS_SAVE | MACHINE_NOT_WORKING )
