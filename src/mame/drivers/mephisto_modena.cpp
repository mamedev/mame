// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/**************************************************************************************************

    Mephisto Modena

**************************************************************************************************/


#include "emu.h"
#include "cpu/m6502/m65c02.h"
#include "machine/nvram.h"
#include "machine/mmboard.h"
#include "machine/timer.h"
#include "sound/dac.h"
#include "sound/volt_reg.h"
#include "speaker.h"

#include "mephisto_modena.lh"


class mephisto_modena_state : public driver_device
{
public:
	mephisto_modena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_dac(*this, "dac")
		, m_keys(*this, "KEY")
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u.%u", 0U, 0U)
	{ }

	void modena(machine_config &config);

protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<dac_bit_interface> m_dac;
	required_ioport m_keys;
	output_finder<4> m_digits;
	output_finder<3, 8> m_leds;

	void modena_mem(address_map &map);

	DECLARE_READ8_MEMBER(input_r);
	DECLARE_WRITE8_MEMBER(digits_w);
	DECLARE_WRITE8_MEMBER(io_w);
	DECLARE_WRITE8_MEMBER(led_w);

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_on)  { m_maincpu->set_input_line(M6502_NMI_LINE, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_off) { m_maincpu->set_input_line(M6502_NMI_LINE, CLEAR_LINE);  }

	uint8_t m_digits_idx;
	uint8_t m_io_ctrl;
};


READ8_MEMBER(mephisto_modena_state::input_r)
{
	if (m_board->mux_r(space, offset) == 0xff)
		return m_keys->read();
	else
		return m_board->input_r(space, offset) ^ 0xff;
}

WRITE8_MEMBER(mephisto_modena_state::led_w)
{
	m_board->mux_w(space, offset, data);

	for (int sel = 0; sel < 3; sel++)
	{
		if (BIT(m_io_ctrl, sel+1))
		{
			for (int i = 0; i < 8; i++)
				m_leds[sel][i] = BIT(data, i) ? 0 : 1;
		}
	}
}

WRITE8_MEMBER(mephisto_modena_state::io_w)
{
	m_io_ctrl = data;
	m_dac->write(BIT(data, 6));
}

WRITE8_MEMBER(mephisto_modena_state::digits_w)
{
	m_digits[m_digits_idx] = data ^ ((m_io_ctrl & 0x10) ? 0xff : 0x00);
	m_digits_idx = (m_digits_idx + 1) & 3;
}

void mephisto_modena_state::modena_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4000).w(FUNC(mephisto_modena_state::digits_w));
	map(0x5000, 0x5000).w(FUNC(mephisto_modena_state::led_w));
	map(0x6000, 0x6000).w(FUNC(mephisto_modena_state::io_w));
	map(0x7000, 0x7fff).r(FUNC(mephisto_modena_state::input_r));
	map(0x8000, 0xffff).rom().region("maincpu", 0);
}


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


void mephisto_modena_state::machine_start()
{
	m_digits.resolve();
	m_leds.resolve();

	save_item(NAME(m_digits_idx));
	save_item(NAME(m_io_ctrl));
}

void mephisto_modena_state::machine_reset()
{
	m_digits_idx = 0;
	m_io_ctrl = 0;
}


void mephisto_modena_state::modena(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(4'194'304)); // W65C02SP
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_modena_state::modena_mem);
	timer_device &nmi_on(TIMER(config, "nmi_on"));
	nmi_on.configure_periodic(FUNC(mephisto_modena_state::nmi_on), attotime::from_hz(XTAL(4'194'304) / (1 << 13)));
	nmi_on.set_start_delay(attotime::from_hz(XTAL(4'194'304) / (1 << 13)) - attotime::from_usec(975)); // active for 975us
	TIMER(config, "nmi_off").configure_periodic(FUNC(mephisto_modena_state::nmi_off), attotime::from_hz(XTAL(4'194'304) / (1 << 13)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_BUTTONS_BOARD(config, m_board);
	m_board->set_disable_leds(true);
	config.set_default_layout(layout_mephisto_modena);

	/* sound hardware */
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, m_dac).add_route(ALL_OUTPUTS, "speaker", 0.25);
	VOLTAGE_REGULATOR(config, "vref").add_route(0, "dac", 1.0, DAC_VREF_POS_INPUT);
}


ROM_START(modena)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_SYSTEM_BIOS( 0, "v1", "v1" )
	ROMX_LOAD("modena 12aug1992.bin", 0x0000, 0x8000, CRC(dd7b4920) SHA1(4606b9d1f8a30180aabedfc0ed3cca0c96618524), ROM_BIOS(0))
	ROM_SYSTEM_BIOS( 1, "v1alt", "v1alt" )
	ROMX_LOAD("27c256,457f.bin", 0x0000, 0x8000, CRC(2889082c) SHA1(b63f0d856793b4f87471837e2219ce2a42fe18de), ROM_BIOS(1))
ROM_END


/***************************************************************************
    Game driver(s)
***************************************************************************/

/*    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT   CLASS                   INIT        COMPANY             FULLNAME                     FLAGS */
CONS( 1992, modena,   0,      0,      modena,   modena, mephisto_modena_state,  empty_init, "Hegener + Glaser", "Mephisto Modena",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
