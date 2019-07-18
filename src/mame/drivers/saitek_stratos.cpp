// license:BSD-3-Clause
// copyright-holders:Olivier Galibert, hap
/***************************************************************************

Scisys Kasparov Stratos Chess Computer

TODO:
- add LCD (188:88, 88:88, and 7*7 DMD bottom-left)
- corona: different addressmap, 64 leds
- add endgame rom (softwarelist?)
- clean up driver
- does nvram work? maybe both ram chips battery-backed
- add soft power off with STOP button(writes 0 to control_w), power-on with GO button

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6502/r65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
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
		m_lcd_busy(*this, "lcd_busy"),
		m_board(*this, "board"),
		m_display(*this, "display"),
		m_dac(*this, "dac"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	void stratos(machine_config &config);
	void corona(machine_config &config);
	void tking(machine_config &config);
	void tkingd(machine_config &config);

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

	TIMER_DEVICE_CALLBACK_MEMBER(helios_tick);

	std::unique_ptr<uint8_t[]> nvram_data;
	uint8_t control, m_select;
	uint32_t ind_leds;
	bool m_lcd_written;

	u8 m_lcd_address;
	u8 m_lcd_ram[0x100];
	u8 m_lcd_latch;

	u8 m_lcd_data;
	void show_leds();
	virtual void machine_reset() override;

	required_device<m65c02_device> m_maincpu;
	required_device<nvram_device> nvram;
	required_memory_bank bank_8000;
	required_memory_bank bank_4000;
	required_memory_bank nvram_bank;
	required_device<timer_device> m_lcd_busy;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport_array<8> m_inputs;
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

TIMER_DEVICE_CALLBACK_MEMBER(stratos_state::helios_tick)
{


	m_maincpu->set_input_line(M65C02_IRQ_LINE, HOLD_LINE);
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
		if (m_lcd_busy->enabled())
			data |= 0x20;

		// battery low?
		data |= 0x80;
	}

	if (sel < 8)
		data |= m_inputs[sel]->read() << 5;


	return data;
}

WRITE8_MEMBER(stratos_state::control_w)
{
	control = data;
	bank_8000->set_entry(data >> 0 & 1);
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
	m_lcd_data = data;
	m_lcd_written = true;

	m_lcd_busy->adjust(attotime::from_usec(50)); // ?
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

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(350));

	TIMER(config, "helios").configure_periodic(FUNC(stratos_state::helios_tick), attotime::from_hz(5.67_MHz_XTAL / 0x10000));
	TIMER(config, "lcd_busy").configure_generic(timer_device::expired_delegate());

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(2+4, 8+1);
	config.set_default_layout(layout_saitek_stratos);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}

void stratos_state::corona(machine_config &config)
{
	stratos(config);

	m_board->set_type(sensorboard_device::MAGNETS);
}

void stratos_state::tking(machine_config &config)
{
	stratos(config);

	/* basic machine hardware */
	R65C02(config.replace(), m_maincpu, 5_MHz_XTAL); // R65C02P4
	m_maincpu->set_addrmap(AS_PROGRAM, &stratos_state::stratos_mem);

	TIMER(config.replace(), "helios").configure_periodic(FUNC(stratos_state::helios_tick), attotime::from_hz(5_MHz_XTAL / 0x10000));
}

void stratos_state::tkingd(machine_config &config)
{
	tking(config);

	// seems much more responsive
	m_board->set_delay(attotime::from_msec(200));
}


ROM_START( stratos )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("w1y01f_728m_u3.u3", 0x0000, 0x8000, CRC(b58a7256) SHA1(75b3a3a65f4ca8d52aa5b17a06319bff59d9014f) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END

ROM_START( stratosl )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("w1y01f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("bw1_819n_u4.u4", 0x8000, 0x8000, CRC(cb0de631) SHA1(f78d40213be21775966cbc832d64acd9b73de632) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END

ROM_START( tking )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("y01f_713d_u3.u3", 0x0000, 0x8000, CRC(b8c6d853) SHA1(98923f44bbbd2ea17c269850971d3df229e6057e) )
	ROM_LOAD("y01f_712a_u4.u4", 0x8000, 0x8000, CRC(7d3f8f7b) SHA1(8be5d8d988ff0577ccfec0a773bfd94599f2534f) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END

ROM_START( tkingl )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("w1y01f_728l_u3.u3", 0x0000, 0x8000, CRC(19a22058) SHA1(a5ca54d870c70b7ce9c7be2951800bf49cc57527) )
	ROM_LOAD("y01f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END

ROM_START( tkingp )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("w1y01f_728p_u3.u3", 0x0000, 0x8000, CRC(ad77f83e) SHA1(598fdb1e40267d9d43a3d8f287723070b9afa349) )
	ROM_LOAD("y01f-b_819o_u4.u4", 0x8000, 0x8000, CRC(336040d4) SHA1(aca662b8cc4d6bafd61ca158c768ba8896117169) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END

ROM_START( corona )
	ROM_REGION( 0x10000, "roms_8000", 0 )
	ROM_LOAD("w2_708g_u2-a.u2", 0x0000, 0x8000, CRC(52568bb4) SHA1(83fe91787e17bbefc2b3ec651ddb11c88990060d) )
	ROM_LOAD("bw2_708a_u3-b.u3", 0x8000, 0x8000, CRC(32848f73) SHA1(a447543e3eb4757f9afed26fde77b66985eb96a7) )

	ROM_REGION( 0x8000, "roms_4000", 0 )
	ROM_FILL( 0x0000, 0x8000, 0xff )
ROM_END


/*    YEAR  NAME      PARENT  CMP MACHINE  INPUT    CLASS          INIT          COMPANY, FULLNAME, FLAGS */
CONS( 1986, stratos,  0,       0, stratos, stratos, stratos_state, init_stratos, "SciSys", "Kasparov Stratos (rev. M)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1986, stratosl, stratos, 0, stratos, stratos, stratos_state, init_stratos, "SciSys", "Kasparov Stratos (rev. L)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )

CONS( 1990, tking,   0,        0, tkingd,  stratos, stratos_state, init_stratos, "Saitek", "Kasparov Turbo King (rev. D)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK ) // aka Turbo King II
CONS( 1988, tkingl,  tking,    0, tking,   stratos, stratos_state, init_stratos, "Saitek", "Kasparov Turbo King (rev. L)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
CONS( 1988, tkingp,  tking,    0, tking,   stratos, stratos_state, init_stratos, "Saitek", "Kasparov Turbo King (rev. P)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )

CONS( 1988, corona,  0,        0, corona,  stratos, stratos_state, init_stratos, "Saitek", "Kasparov Corona (rev. G)", MACHINE_NOT_WORKING | MACHINE_CLICKABLE_ARTWORK )
