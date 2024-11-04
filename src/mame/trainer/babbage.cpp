// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Babbage-2nd skeleton driver (19/OCT/2011)

    http://takeda-toshiya.my.coocan.jp/babbage/index.html

    Pasting:
        0-F : as is
        INC : ^
        AD : -
        DA : =
        GO : X

    Here is a test program to turn on the LEDs.
    Copy the text and Paste into the emulator.
    =3E^=0F^=D3^=13^=3E^=07^=D3^=13^=3E^=00^=D3^=12^=76^-1000X


***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "video/pwm.h"
#include "babbage.lh"


namespace {

#define MAIN_CLOCK 25e5

class babbage_state : public driver_device
{
public:
	babbage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio(*this, "z80pio_%u", 1U)
		, m_ctc(*this, "z80ctc")
		, m_display(*this, "display")
		, m_io_keyboard(*this, "X%u", 0U)
		, m_leds(*this, "led%u", 0U)
	{ }

	void babbage(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t pio2_a_r();
	void pio1_b_w(uint8_t data);
	void pio2_b_w(uint8_t data);
	void ctc_z0_w(int state);
	void ctc_z1_w(int state);
	void ctc_z2_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);

	void babbage_io(address_map &map) ATTR_COLD;
	void babbage_map(address_map &map) ATTR_COLD;

	uint8_t m_seg = 0U;
	uint8_t m_key = 0U;
	uint8_t m_prev_key = 0U;
	bool m_step = 0;

	required_device<z80_device> m_maincpu;
	required_device_array<z80pio_device, 2> m_pio;
	required_device<z80ctc_device> m_ctc;
	required_device<pwm_display_device> m_display;
	required_ioport_array<4> m_io_keyboard;
	output_finder<8> m_leds;
};



/***************************************************************************

    Address Map

***************************************************************************/

void babbage_state::babbage_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x07ff).rom();
	map(0x1000, 0x17ff).ram();
}

void babbage_state::babbage_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x03).rw(m_ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x10, 0x13).rw(m_pio[0], FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x20, 0x23).rw(m_pio[1], FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
}



/**************************************************************************

    Keyboard Layout

***************************************************************************/

// no idea of the actual key matrix
static INPUT_PORTS_START( babbage )
	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("0") PORT_CODE(KEYCODE_0) PORT_CHAR('0')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("4") PORT_CODE(KEYCODE_4) PORT_CHAR('4')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("8") PORT_CODE(KEYCODE_8) PORT_CHAR('8')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("C") PORT_CODE(KEYCODE_C) PORT_CHAR('C')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_X) PORT_CHAR('X')

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("1") PORT_CODE(KEYCODE_1) PORT_CHAR('1')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("5") PORT_CODE(KEYCODE_5) PORT_CHAR('5')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("9") PORT_CODE(KEYCODE_9) PORT_CHAR('9')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("D") PORT_CODE(KEYCODE_D) PORT_CHAR('D')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("AD") PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-')

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("2") PORT_CODE(KEYCODE_2) PORT_CHAR('2')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("6") PORT_CODE(KEYCODE_6) PORT_CHAR('6')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("A") PORT_CODE(KEYCODE_A) PORT_CHAR('A')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("E") PORT_CODE(KEYCODE_E) PORT_CHAR('E')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("DA") PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=')

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("3") PORT_CODE(KEYCODE_3) PORT_CHAR('3')
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("7") PORT_CODE(KEYCODE_7) PORT_CHAR('7')
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("B") PORT_CODE(KEYCODE_B) PORT_CHAR('B')
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("F") PORT_CODE(KEYCODE_F) PORT_CHAR('F')
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("INC") PORT_CODE(KEYCODE_UP) PORT_CHAR('^')
INPUT_PORTS_END

/* Z80-CTC Interface */

void babbage_state::ctc_z0_w(int state)
{
}

void babbage_state::ctc_z1_w(int state)
{
}

void babbage_state::ctc_z2_w(int state)
{
}

/* Z80-PIO Interface */

// The 8 LEDs
// bios never writes here - you need to set PIO for output yourself - see test program above
void babbage_state::pio1_b_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i] = BIT(data, i);
}

uint8_t babbage_state::pio2_a_r()
{
	if (!machine().side_effects_disabled())
		m_maincpu->set_input_line(0, CLEAR_LINE); // release interrupt
	return m_key;
}

void babbage_state::pio2_b_w(uint8_t data)
{
	if (BIT(data, 7))
		m_step = false;
	else
	if (!m_step)
	{
		m_seg = data;
		m_step = true;
	}
	else
		m_display->matrix(data, m_seg);
}

static const z80_daisy_config babbage_daisy_chain[] =
{// need to check the order
	{ "z80pio_1" },
	{ "z80pio_2" },
	{ "z80ctc" },
	{ nullptr }
};

TIMER_DEVICE_CALLBACK_MEMBER(babbage_state::keyboard_callback)
{
	u8 inp, data = 0xff;

	for (u8 i = 0; i < 4; i++)
	{
		inp = m_io_keyboard[i]->read();

		for (u8 j = 0; j < 5; j++)
			if (BIT(inp, j))
				data = (j << 2) | i;
	}

	/* make sure only one keystroke */
	if (data != m_prev_key)
		m_prev_key = data;
	else
		data = 0xff;

	/* while key is down, activate strobe. When key released, deactivate strobe which causes an interrupt */
	if (data < 0xff)
	{
		m_key = data;
		m_pio[1]->strobe(0, 0);
	}
	else
		m_pio[1]->strobe(0, 1);
}

void babbage_state::machine_start()
{
	m_leds.resolve();

	save_item(NAME(m_seg));
	save_item(NAME(m_key));
	save_item(NAME(m_prev_key));
	save_item(NAME(m_step));
}

/***************************************************************************

    Machine driver

***************************************************************************/

void babbage_state::babbage(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, MAIN_CLOCK); //2.5MHz
	m_maincpu->set_addrmap(AS_PROGRAM, &babbage_state::babbage_map);
	m_maincpu->set_addrmap(AS_IO, &babbage_state::babbage_io);
	m_maincpu->set_daisy_config(babbage_daisy_chain);

	/* video hardware */
	config.set_default_layout(layout_babbage);
	PWM_DISPLAY(config, m_display).set_size(6, 8);
	m_display->set_segmask(0x3f, 0xff);

	/* Devices */
	Z80CTC(config, m_ctc, MAIN_CLOCK);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(babbage_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(babbage_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(babbage_state::ctc_z2_w));

	Z80PIO(config, m_pio[0], MAIN_CLOCK);
	m_pio[0]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[0]->out_pb_callback().set(FUNC(babbage_state::pio1_b_w));

	Z80PIO(config, m_pio[1], MAIN_CLOCK);
	m_pio[1]->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio[1]->in_pa_callback().set(FUNC(babbage_state::pio2_a_r));
	m_pio[1]->out_pb_callback().set(FUNC(babbage_state::pio2_b_w));

	TIMER(config, "keyboard_timer", 0).configure_periodic(FUNC(babbage_state::keyboard_callback), attotime::from_hz(30));
}


/***************************************************************************

    Game driver

***************************************************************************/

ROM_START(babbage)
	ROM_REGION(0x0800, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("mon.rom",    0x0000, 0x0200, CRC(469bd607) SHA1(8f3489a0f96de6a03b05c1ee01b89d9848f4b152) )
ROM_END

} // anonymous namespace


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME       FLAGS
COMP( 1986, babbage, 0,      0,      babbage, babbage, babbage_state, empty_init, "Mr Takafumi Aihara", "Babbage-2nd", MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
