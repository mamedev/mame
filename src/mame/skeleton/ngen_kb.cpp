// license:BSD-3-Clause
// copyright-holders:Barry Rodewald, Vas Crabb
// Convergent NGEN keyboard device

#include "emu.h"
#include "ngen_kb.h"
#include "machine/keyboard.ipp"

#define VERBOSE (0)
#include "logmacro.h"

namespace {

// Retain the previous generic keyboard character mapping while separating the
// NGEN serial device from serial_keyboard_device.  These tables match the
// generic_keyboard input ports (machine/keyboard.cpp), not the physical NGEN
// scan codes.  The hardware protocol remains to be implemented separately.
constexpr u8 TRANSLATION_TABLE[][2][4][16] = {
	{
		{   // ANSI
			{ '`',   '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   '-',   '=',   0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',   'o',   'p',   '[',   ']',   '\\',  0xffU, 0xffU },
			{ 0xffU, 'a',   's',   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',   '\'',  0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, '\\',  'z',   'x',   'c',   'v',   'b',   'n',   'm',   ',',   '.',   '/',   0xffU, 0xffU, 0xffU, ' '   }
		},
		{   // JIS
			{ '\\',  '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   '-',   '^',   0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 'q',   'w',   'e',   'r',   't',   'y',   'u',   'i',   'o',   'p',   '@',   '[',   ']',   0xffU, 0xffU },
			{ 0xffU, 'a',   's',   'd',   'f',   'g',   'h',   'j',   'k',   'l',   ';',   ':',   0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, '\\',  'z',   'x',   'c',   'v',   'b',   'n',   'm',   ',',   '.',   '/',   0xffU, 0xffU, 0xffU, ' '   }
		}
	},
	{
		{   // ANSI shift
			{ '~',   '!',   '@',   '#',   '$',   '%',   '^',   '&',   '*',   '(',   ')',   '_',   '+',   0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',   'O',   'P',   '{',   '}',   '|',   0xffU, 0xffU },
			{ 0xffU, 'A',   'S',   'D',   'F',   'G',   'H',   'J',   'K',   'L',   ':',   '"',   0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, '_',   'Z',   'X',   'C',   'V',   'B',   'N',   'M',   '<',   '>',   '?',   0xffU, 0xffU, 0xffU, ' '   }
		},
		{   // JIS shift
			{ '|',   '!',   '"',   '#',   '$',   '%',   '&',   '\'',  '(',   ')',   0xffU, '=',   '~',   0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 'Q',   'W',   'E',   'R',   'T',   'Y',   'U',   'I',   'O',   'P',   '`',   '{',   '}',   0xffU, 0xffU },
			{ 0xffU, 'A',   'S',   'D',   'F',   'G',   'H',   'J',   'K',   'L',   '+',   '*',   0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, '_',   'Z',   'X',   'C',   'V',   'B',   'N',   'M',   '<',   '>',   '?',   0xffU, 0xffU, 0xffU, ' '   }
		}
	},
	{
		{   // ANSI ctrl
			{ 0x00U, '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   0x1fU, 0x1eU, 0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 0x11U, 0x17U, 0x05U, 0x12U, 0x14U, 0x19U, 0x15U, 0x09U, 0x0fU, 0x10U, 0x1bU, 0x1dU, 0x1cU, 0xffU, 0xffU },
			{ 0xffU, 0x01U, 0x13U, 0x04U, 0x06U, 0x07U, 0x08U, 0x0aU, 0x0bU, 0x0cU, ';',   '\'',  0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, 0x1cU, 0x1aU, 0x18U, 0x03U, 0x16U, 0x02U, 0x0eU, 0x0dU, ',',   '.',   0x1fU, 0xffU, 0xffU, 0xffU, 0x00U }
		},
		{   // JIS ctrl
			{ 0x1cU, '1',   '2',   '3',   '4',   '5',   '6',   '7',   '8',   '9',   '0',   0x1fU, 0x1eU, 0x08U, 0x7fU, 0x1bU },
			{ 0x09U, 0x11U, 0x17U, 0x05U, 0x12U, 0x14U, 0x19U, 0x15U, 0x09U, 0x0fU, 0x10U, 0x00U, 0x1bU, 0x1dU, 0xffU, 0xffU },
			{ 0xffU, 0x01U, 0x13U, 0x04U, 0x06U, 0x07U, 0x08U, 0x0aU, 0x0bU, 0x0cU, ';',   ':',   0x0dU, 0xffU, 0xffU, 0x0aU },
			{ 0xffU, 0x1cU, 0x1aU, 0x18U, 0x03U, 0x16U, 0x02U, 0x0eU, 0x0dU, ',',   '.',   0x1fU, 0xffU, 0xffU, 0xffU, 0x00U }
		}
	}
};


constexpr bool CAPS_TABLE[4][16] = {
	{ false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false },
	{ false, true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false },
	{ false, false, true,  true,  true,  true,  true,  true,  true,  false, false, false, false, false, false, false }
};

} // anonymous namespace

DEFINE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device, "ngen_kb", "NGEN Keyboard (HLE)")

ngen_keyboard_device::ngen_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NGEN_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "GENKBD_ROW0", "GENKBD_ROW1", "GENKBD_ROW2", "GENKBD_ROW3")
	, m_tx_handler(*this)
	, m_config(*this, "GENKBD_CFG")
	, m_modifiers(*this, "GENKBD_MOD")
	, m_keys_down(0xffU)
{
}


static INPUT_PORTS_START( ngen_keyboard )
	PORT_INCLUDE(generic_keyboard)
INPUT_PORTS_END


ioport_constructor ngen_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ngen_keyboard);
}


void ngen_keyboard_device::device_start()
{
	save_item(NAME(m_keys_down));
}


void ngen_keyboard_device::device_reset()
{
	clear_fifo();
	receive_register_reset();
	transmit_register_reset();

	// The NGEN firmware uses an 8251 for the keyboard link.  The current
	// HLE retains the serial parameters used by the previous keyboard model;
	// the keyboard controller's exact framing is not documented here.
	set_data_frame(1, 8, PARITY_NONE, STOP_BITS_2);
	set_rcv_rate(19'200);
	set_tra_rate(19'200);
	m_tx_handler(1);

	reset_key_state();
	start_processing(attotime::from_hz(2'400));
	typematic_stop();

	m_keys_down = 0xffU;
}


void ngen_keyboard_device::tra_callback()
{
	m_tx_handler(transmit_register_get_data_bit());
}


void ngen_keyboard_device::received_byte(uint8_t byte)
{
	// Host commands are not understood yet; retain optional protocol logging.
	LOG("Received from host: %02x\n", byte);
}


bool ngen_keyboard_device::translate(uint8_t row, uint8_t column, uint8_t &data) const
{
	if (row >= 4U || column >= 16U)
		return false;

	unsigned const layout(BIT(m_config->read(), 0));
	u16 const modifiers(m_modifiers->read());
	bool const shift(BIT(modifiers, 1) != (BIT(modifiers, 2) && CAPS_TABLE[row][column]));
	unsigned const map(BIT(modifiers, 0) ? 2U : shift ? 1U : 0U);
	u8 const result(TRANSLATION_TABLE[map][layout][row][column]);
	if (result == 0xffU)
		return false;

	data = result | (BIT(modifiers, 3) ? 0x80U : 0x00U);
	return true;
}


attotime ngen_keyboard_device::typematic_delay() const
{
	return attotime::from_msec(250 * (1 + ((m_config->read() >> 1) & 0x03U)));
}


attotime ngen_keyboard_device::typematic_period() const
{
	unsigned const rate(~(m_config->read() >> 3) & 0x1fU);
	return attotime::from_ticks((8U + (rate & 0x07U)) * (1U << ((rate >> 3) & 0x03U)), 240U);
}


void ngen_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	uint8_t data;
	if (translate(row, column, data))
		transmit_byte(data);

	m_keys_down = uint8_t((row << 4) | column);

	// Reuse the generic keyboard's configurable delay and repeat rate.
	typematic_start(row, column, typematic_delay(), typematic_period());
}


void ngen_keyboard_device::key_break(uint8_t row, uint8_t column)
{
	if (m_keys_down == uint8_t((row << 4) | column))
	{
		m_keys_down = 0xffU;
		// Preserve the byte emitted by the earlier HLE when its most
		// recently pressed key is released; this is not asserted to be a
		// confirmed hardware scan-code convention.
		transmit_byte(0xc0U);
	}

	if (typematic_is(row, column))
		typematic_stop();
}


void ngen_keyboard_device::key_repeat(uint8_t row, uint8_t column)
{
	uint8_t data;
	if (translate(row, column, data))
		transmit_byte(data);
}
