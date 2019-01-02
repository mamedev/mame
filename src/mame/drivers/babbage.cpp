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

    ToDo:
    - Blank the display if digits aren't being refreshed

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/timer.h"
#include "machine/z80ctc.h"
#include "machine/z80pio.h"
#include "machine/z80daisy.h"
#include "babbage.lh"

#define MAIN_CLOCK 25e5

class babbage_state : public driver_device
{
public:
	babbage_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_pio_1(*this, "z80pio_1")
		, m_pio_2(*this, "z80pio_2")
		, m_ctc(*this, "z80ctc")
		, m_keyboard(*this, "X%u", 0)
		, m_digits(*this, "digit%u", 0U)
	{ }

	void babbage(machine_config &config);

protected:
	DECLARE_READ8_MEMBER(pio2_a_r);
	DECLARE_WRITE8_MEMBER(pio1_b_w);
	DECLARE_WRITE8_MEMBER(pio2_b_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z0_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z1_w);
	DECLARE_WRITE_LINE_MEMBER(ctc_z2_w);
	TIMER_DEVICE_CALLBACK_MEMBER(keyboard_callback);

	void babbage_io(address_map &map);
	void babbage_map(address_map &map);

private:
	uint8_t m_segment;
	uint8_t m_key;
	uint8_t m_prev_key;
	bool m_step;
	virtual void machine_start() override { m_digits.resolve(); }
	required_device<z80_device> m_maincpu;
	required_device<z80pio_device> m_pio_1;
	required_device<z80pio_device> m_pio_2;
	required_device<z80ctc_device> m_ctc;
	required_ioport_array<4> m_keyboard;
	output_finder<33> m_digits;
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
	map(0x10, 0x13).rw(m_pio_1, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
	map(0x20, 0x23).rw(m_pio_2, FUNC(z80pio_device::read_alt), FUNC(z80pio_device::write_alt));
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

WRITE_LINE_MEMBER( babbage_state::ctc_z0_w )
{
}

WRITE_LINE_MEMBER( babbage_state::ctc_z1_w )
{
}

WRITE_LINE_MEMBER( babbage_state::ctc_z2_w )
{
}

/* Z80-PIO Interface */

// The 8 LEDs
// bios never writes here - you need to set PIO for output yourself - see test program above
WRITE8_MEMBER( babbage_state::pio1_b_w )
{
	char ledname[8];
	for (int i = 0; i < 8; i++)
	{
		sprintf(ledname,"led%d",i);
		output().set_value(ledname, BIT(data, i));
	}
}

READ8_MEMBER( babbage_state::pio2_a_r )
{
	m_maincpu->set_input_line(0, CLEAR_LINE); // release interrupt
	return m_key;
}

WRITE8_MEMBER( babbage_state::pio2_b_w )
{
	if (BIT(data, 7))
	{
		m_step = false;
	}
	else if (!m_step)
	{
		m_segment = data;
		m_step = true;
	}
	else
	{
		m_digits[data] = m_segment;
	}
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
		inp = m_keyboard[i]->read();

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
		m_pio_2->strobe(0, 0);
	}
	else
		m_pio_2->strobe(0, 1);
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

	/* Devices */
	Z80CTC(config, m_ctc, MAIN_CLOCK);
	m_ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_ctc->zc_callback<0>().set(FUNC(babbage_state::ctc_z0_w));
	m_ctc->zc_callback<1>().set(FUNC(babbage_state::ctc_z1_w));
	m_ctc->zc_callback<2>().set(FUNC(babbage_state::ctc_z2_w));

	Z80PIO(config, m_pio_1, MAIN_CLOCK);
	m_pio_1->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio_1->out_pb_callback().set(FUNC(babbage_state::pio1_b_w));

	Z80PIO(config, m_pio_2, MAIN_CLOCK);
	m_pio_2->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_pio_2->in_pa_callback().set(FUNC(babbage_state::pio2_a_r));
	m_pio_2->out_pb_callback().set(FUNC(babbage_state::pio2_b_w));

	TIMER(config, "keyboard_timer", 0).configure_periodic(timer_device::expired_delegate(FUNC(babbage_state::keyboard_callback), this), attotime::from_hz(30));
}


/***************************************************************************

    Game driver

***************************************************************************/

ROM_START(babbage)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("mon.rom",    0x0000, 0x0200, CRC(469bd607) SHA1(8f3489a0f96de6a03b05c1ee01b89d9848f4b152) )
ROM_END


//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY               FULLNAME       FLAGS
COMP( 1986, babbage, 0,      0,      babbage, babbage, babbage_state, empty_init, "Mr Takafumi Aihara", "Babbage-2nd", MACHINE_NO_SOUND_HW )
