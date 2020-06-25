// license:BSD-3-Clause
// copyright-holders:Sandro Ronco, hap
// thanks-to:yoyo_chessboard
/**************************************************************************************************

Mephisto Modena

The chess engine is by Frans Morsch, same one as Sphinx Dominator 2.05.
Hold Pawn + Knight buttons at boot for test mode.

**************************************************************************************************/

#include "emu.h"

#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/sensorboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "video/pwm.h"

#include "speaker.h"

#include "mephisto_modena.lh"


class modena_state : public driver_device
{
public:
	modena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_display(*this, "display")
		, m_dac(*this, "dac")
		, m_keys(*this, "KEY")
		, m_digits(*this, "digit%u", 0U)
	{ }

	void modena(machine_config &config);

protected:
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<sensorboard_device> m_board;
	required_device<pwm_display_device> m_display;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_keys;
	output_finder<4> m_digits;

	void modena_mem(address_map &map);

	u8 input_r();
	void digits_w(u8 data);
	void io_w(u8 data);
	void led_w(u8 data);
	void update_display();

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_on)  { m_maincpu->set_input_line(M6502_NMI_LINE, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_off) { m_maincpu->set_input_line(M6502_NMI_LINE, CLEAR_LINE);  }

	u8 m_board_mux = 0xff;
	u8 m_digits_idx = 0;
	u8 m_io_ctrl = 0;
};

void modena_state::machine_start()
{
	m_digits.resolve();

	save_item(NAME(m_digits_idx));
	save_item(NAME(m_io_ctrl));
}



/******************************************************************************
    I/O
******************************************************************************/

void modena_state::update_display()
{
	m_display->matrix(m_io_ctrl >> 1 & 7, ~m_board_mux);
}

u8 modena_state::input_r()
{
	u8 data = 0;

	// read buttons
	if (~m_io_ctrl & 1)
		data |= m_keys->read();

	// read chessboard sensors
	for (int i=0; i<8; i++)
		if (!BIT(m_board_mux, i))
			data |= m_board->read_rank(i);

	return data;
}

void modena_state::led_w(u8 data)
{
	// d0-d7: chessboard mux, led data
	m_board_mux = data;
	update_display();
}

void modena_state::io_w(u8 data)
{
	// d0: button select
	// d1-d3: led select
	// d4: lcd polarity
	// d6: speaker out
	m_io_ctrl = data;
	update_display();
	m_dac->write(BIT(data, 6));
}

void modena_state::digits_w(u8 data)
{
	m_digits[m_digits_idx] = data ^ ((m_io_ctrl & 0x10) ? 0xff : 0x00);
	m_digits_idx = (m_digits_idx + 1) & 3;
}



/******************************************************************************
    Address Maps
******************************************************************************/

void modena_state::modena_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4000).w(FUNC(modena_state::digits_w));
	map(0x5000, 0x5000).w(FUNC(modena_state::led_w));
	map(0x6000, 0x6000).w(FUNC(modena_state::io_w));
	map(0x7000, 0x7000).r(FUNC(modena_state::input_r));
	map(0x7f00, 0x7fff).nopr(); // dummy read on 6502 absolute X page wrap
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}



/******************************************************************************
    Input Ports
******************************************************************************/

static INPUT_PORTS_START( modena )
	PORT_START("KEY")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("BOOK")      PORT_CODE(KEYCODE_B)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("INFO")      PORT_CODE(KEYCODE_I)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("MEMORY")    PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("POSITION")  PORT_CODE(KEYCODE_O)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("LEVEL")     PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("FUNCTION")  PORT_CODE(KEYCODE_F)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("ENTER")     PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_F1) // combine for NEW GAME
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("CLEAR")     PORT_CODE(KEYCODE_BACKSPACE) PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_F1) // "
INPUT_PORTS_END



/******************************************************************************
    Machine Configs
******************************************************************************/

void modena_state::modena(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, 4.194304_MHz_XTAL); // W65C02SP or RP65C02G
	m_maincpu->set_addrmap(AS_PROGRAM, &modena_state::modena_mem);

	timer_device &nmi_on(TIMER(config, "nmi_on"));
	const attotime nmi_period = attotime::from_hz(4.194304_MHz_XTAL / (1 << 13));
	nmi_on.configure_periodic(FUNC(modena_state::nmi_on), nmi_period);
	nmi_on.set_start_delay(nmi_period - attotime::from_usec(975)); // active for 975us
	TIMER(config, "nmi_off").configure_periodic(FUNC(modena_state::nmi_off), nmi_period);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	SENSORBOARD(config, m_board).set_type(sensorboard_device::BUTTONS);
	m_board->init_cb().set(m_board, FUNC(sensorboard_device::preset_chess));
	m_board->set_delay(attotime::from_msec(150));
	m_board->set_nvram_enable(true);

	/* video hardware */
	PWM_DISPLAY(config, m_display).set_size(3, 8);
	config.set_default_layout(layout_mephisto_modena);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}



/******************************************************************************
    ROM Definitions
******************************************************************************/

ROM_START( modena )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("modena_12aug1992_441d.u3", 0x0000, 0x8000, CRC(dd7b4920) SHA1(4606b9d1f8a30180aabedfc0ed3cca0c96618524) )
ROM_END

ROM_START( modenaa )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("27c256_457f.u3", 0x0000, 0x8000, CRC(2889082c) SHA1(b63f0d856793b4f87471837e2219ce2a42fe18de) )
ROM_END

ROM_START( modenab )
	ROM_REGION( 0x8000, "maincpu", 0 )
	ROM_LOAD("modena_4929_270192.u3", 0x0000, 0x8000, CRC(99212677) SHA1(f0565e5441fb38df201176d01793c953886b0303) )
ROM_END



/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY             FULLNAME                   FLAGS */
CONS( 1992, modena,   0,      0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 1)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1992, modenaa,  modena, 0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 2)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
CONS( 1992, modenab,  modena, 0,      modena,  modena, modena_state, empty_init, "Hegener + Glaser", "Mephisto Modena (set 3)", MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
