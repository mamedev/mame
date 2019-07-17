// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

Scisys Kasparov Stratos Chess Computer

TODO:
- add LCD, maybe Hughes serial chip? (188:88, 88:88, and 7*7 DMD bottom-left)
- add Turbo King/Corona (same hardware family)
- add endgame rom (softwarelist?)
- clean up driver
- does nvram work? maybe both ram chips battery-backed
- add soft power off with STOP button(writes 0 to control_w), power-on with GO button

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"
#include "video/pwm.h"

// internal artwork
#include "saitek_stratos.lh" // clickable

class stratos_state : public driver_device
{
public:
	stratos_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		nvram(*this, "nvram"),
		bank_8000(*this, "bank_8000"),
		bank_4000(*this, "bank_4000"),
		nvram_bank(*this, "nvram_bank"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void stratos(machine_config &config);

	void init_stratos();

private:
	DECLARE_WRITE8_MEMBER(p2000_w);
	DECLARE_READ8_MEMBER(p2200_r);
	DECLARE_WRITE8_MEMBER(p2200_w);
	DECLARE_WRITE8_MEMBER(p2400_w);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);
	DECLARE_READ8_MEMBER(lcd_r);
	DECLARE_WRITE8_MEMBER(lcd_w);

	void stratos_mem(address_map &map);

	std::unique_ptr<uint8_t[]> nvram_data;
	uint8_t control, m_select;
	uint32_t ind_leds;
	void show_leds();
	virtual void machine_reset() override;

	required_device<m65c02_device> m_maincpu;
	required_device<nvram_device> nvram;
	required_memory_bank bank_8000;
	required_memory_bank bank_4000;
	required_memory_bank nvram_bank;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_inputs;

	bool m_lcd_busy;
};

void stratos_state::init_stratos()
{
	nvram_data = std::make_unique<uint8_t[]>(0x2000);
	nvram->set_base(nvram_data.get(), 0x2000);

	bank_8000 ->configure_entries(0, 2, memregion("roms_8000")->base(), 0x8000);
	bank_4000 ->configure_entries(0, 2, memregion("roms_4000")->base(), 0x4000);
	nvram_bank->configure_entries(0, 2, nvram_data.get(),               0x1000);
}

void stratos_state::machine_reset()
{
	control = 0x00;
	m_select = 0x00;
	bank_8000 ->set_entry(0);
	bank_4000 ->set_entry(0);
	nvram_bank->set_entry(0);
}

void stratos_state::show_leds()
{
	m_display->matrix_partial(2, 4, ~m_select >> 4 & 0xf, 1 << (m_select & 0xf), false);
	m_display->matrix_partial(0, 2, 1 << (control >> 5 & 1), (~ind_leds & 0xff) | (~control << 6 & 0x100));
}


WRITE8_MEMBER(stratos_state::p2000_w)
{
	m_dac->write(0); // guessed
	m_select = data;

	show_leds();
}

READ8_MEMBER(stratos_state::p2200_r)
{
	return ~m_board->read_file(m_select & 0xf);
}

WRITE8_MEMBER(stratos_state::p2200_w)
{
	m_dac->write(1);
}

WRITE8_MEMBER(stratos_state::p2400_w)
{
	ind_leds = data;

	show_leds();
}

READ8_MEMBER(stratos_state::control_r)
{
	u8 data = 0;
	u8 sel = m_select & 0xf;

	if (sel == 8)
	{
		// lcd busy flag?
		data = m_lcd_busy ? 0x20: 0;
		m_lcd_busy = false;
	}

	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;

	return data;
}

WRITE8_MEMBER(stratos_state::control_w)
{
	control = data;
	bank_8000->set_entry(data & 1);
	bank_4000->set_entry(data >> 1 & 1); // ?
	nvram_bank->set_entry((data >> 1) & 1);

	show_leds();
}


READ8_MEMBER(stratos_state::lcd_r)
{
	return 0;
}

WRITE8_MEMBER(stratos_state::lcd_w)
{
	m_lcd_busy = true; // ?

	// TODO..
}

void stratos_state::stratos_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).w(FUNC(stratos_state::p2000_w));
	map(0x2200, 0x2200).rw(FUNC(stratos_state::p2200_r), FUNC(stratos_state::p2200_w));
	map(0x2400, 0x2400).w(FUNC(stratos_state::p2400_w));
	map(0x2600, 0x2600).rw(FUNC(stratos_state::control_r), FUNC(stratos_state::control_w));
	map(0x2800, 0x37ff).bankrw("nvram_bank");
	map(0x3800, 0x3800).rw(FUNC(stratos_state::lcd_r), FUNC(stratos_state::lcd_w));
	map(0x4000, 0x7fff).bankr("bank_4000");
	map(0x8000, 0xffff).bankr("bank_8000");
}

static INPUT_PORTS_START( stratos )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_1) // setup
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_2) // level
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_3)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_4) // sound
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_5) // stop?
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_6) // new game?

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_7) // rook
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_8) // pawn
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_9) // bishop

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) // queen
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_W) // knight
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_E) // king

	PORT_START("IN.4")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_R) // play
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_T) // tab/color
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) // -

	PORT_START("IN.5")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_U) // +
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_I) // function
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_O)

	PORT_START("IN.6")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_A) // library
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_S) // info
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_D)

	PORT_START("IN.7")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_F)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_G) // analysis
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_CODE(KEYCODE_H) // normal
INPUT_PORTS_END

void stratos_state::stratos(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 5.67_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &stratos_state::stratos_mem);
	m_maincpu->set_periodic_int(FUNC(stratos_state::irq0_line_hold), attotime::from_hz(5.67_MHz_XTAL / 0x1000));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(100));

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 8+1);
	m_display->set_bri_levels(0.05); // leds supposed to flicker
	m_display->set_bri_maximum(0.1); // "

	config.set_default_layout(layout_saitek_stratos);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

ROM_START( stratos )
	ROM_REGION(0x10000, "roms_8000", 0)
	ROM_LOAD("w1_728m_u3.u3",  0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f))
	ROM_LOAD("bw1_918n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632))

	ROM_REGION(0x10000, "roms_4000", 0)
	ROM_FILL(0x00000, 0x10000, 0xff)
ROM_END

/*    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT          COMPANY    FULLNAME            FLAGS */
CONS( 1986, stratos, 0,      0,      stratos, stratos, stratos_state, init_stratos, "SciSys",  "Kasparov Stratos", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
