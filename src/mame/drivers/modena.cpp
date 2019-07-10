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
#include "speaker.h"

#include "mephisto_modena.lh"


class mephisto_modena_state : public driver_device
{
public:
	mephisto_modena_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_board(*this, "board")
		, m_beeper(*this, "beeper")
		, m_keys(*this, "KEY")
		, m_digits(*this, "digit%u", 0U)
		, m_leds1(*this, "led%u", 100U)
		, m_leds2(*this, "led%u", 0U)
		, m_leds3(*this, "led%u", 8U)
	{ }

	DECLARE_READ8_MEMBER(modena_input_r);
	DECLARE_WRITE8_MEMBER(modena_digits_w);
	DECLARE_WRITE8_MEMBER(modena_io_w);
	DECLARE_WRITE8_MEMBER(modena_led_w);
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_on)  { m_maincpu->set_input_line(M6502_NMI_LINE, ASSERT_LINE); }
	TIMER_DEVICE_CALLBACK_MEMBER(nmi_off) { m_maincpu->set_input_line(M6502_NMI_LINE, CLEAR_LINE);  }

	void modena(machine_config &config);
	void modena_mem(address_map &map);
protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<mephisto_board_device> m_board;
	required_device<beep_device> m_beeper;
	required_ioport m_keys;
	output_finder<4> m_digits;
	output_finder<8> m_leds1;
	output_finder<8> m_leds2;
	output_finder<8> m_leds3;
	uint8_t m_digits_idx;
	uint8_t m_io_ctrl;
};


READ8_MEMBER(mephisto_modena_state::modena_input_r)
{
	if (m_board->mux_r(space, offset) == 0xff)
		return m_keys->read();
	else
		return m_board->input_r(space, offset) ^ 0xff;
}

WRITE8_MEMBER(mephisto_modena_state::modena_led_w)
{
	m_board->mux_w(space, offset, data);

	if (m_io_ctrl & 0x0e)
	{
		for(int i=0; i<8; i++)
		{
			if (BIT(m_io_ctrl, 1))
				m_leds1[i] = BIT(data, i) ? 0 : 1;
			if (BIT(m_io_ctrl, 2))
				m_leds2[i] = BIT(data, i) ? 0 : 1;
			if (BIT(m_io_ctrl, 3))
				m_leds3[i] = BIT(data, i) ? 0 : 1;
		}
	}
}

WRITE8_MEMBER(mephisto_modena_state::modena_io_w)
{
	m_io_ctrl = data;
	m_beeper->set_state(BIT(data, 6));
}

WRITE8_MEMBER(mephisto_modena_state::modena_digits_w)
{
	m_digits[m_digits_idx] = data ^ ((m_io_ctrl & 0x10) ? 0xff : 0x00);
	m_digits_idx = (m_digits_idx + 1) & 3;
}

void mephisto_modena_state::modena_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x4000, 0x4000).w(FUNC(mephisto_modena_state::modena_digits_w));
	map(0x5000, 0x5000).w(FUNC(mephisto_modena_state::modena_led_w));
	map(0x6000, 0x6000).w(FUNC(mephisto_modena_state::modena_io_w));
	map(0x7000, 0x7fff).r(FUNC(mephisto_modena_state::modena_input_r));
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
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("ENTER")     PORT_CODE(KEYCODE_ENTER)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD)     PORT_NAME("CLEAR")     PORT_CODE(KEYCODE_BACKSPACE)
INPUT_PORTS_END


void mephisto_modena_state::machine_start()
{
	m_digits.resolve();
	m_leds1.resolve();
	m_leds2.resolve();
	m_leds3.resolve();
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
	M65C02(config, m_maincpu, XTAL(4'194'304));          // W65C02SP
	m_maincpu->set_addrmap(AS_PROGRAM, &mephisto_modena_state::modena_mem);
	timer_device &nmi_on(TIMER(config, "nmi_on"));
	nmi_on.configure_periodic(FUNC(mephisto_modena_state::nmi_on), attotime::from_hz(XTAL(4'194'304) / (1 << 13)));
	nmi_on.set_start_delay(attotime::from_hz(XTAL(4'194'304) / (1 << 13)) - attotime::from_usec(975));  // active for 975us
	TIMER(config, "nmi_off").configure_periodic(FUNC(mephisto_modena_state::nmi_off), attotime::from_hz(XTAL(4'194'304) / (1 << 13)));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MEPHISTO_BUTTONS_BOARD(config, m_board);
	m_board->set_disable_leds(true);
	config.set_default_layout(layout_mephisto_modena);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	BEEP(config, m_beeper, 3250).add_route(ALL_OUTPUTS, "mono", 1.0);
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
CONS( 1992, modena,   0,      0,      modena,   modena, mephisto_modena_state,  empty_init, "Hegener & Glaser", "Mephisto Modena",           MACHINE_SUPPORTS_SAVE | MACHINE_CLICKABLE_ARTWORK )
