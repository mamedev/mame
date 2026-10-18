// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard device

#include "emu.h"
#include "ngen_kb.h"
#include "machine/keyboard.ipp"

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
	// Host commands are not understood; the earlier HLE only logged them.
	(void)byte;
}


bool ngen_keyboard_device::translate(uint8_t row, uint8_t column, uint8_t &data) const
{
	if (row >= 4U || column >= 16U)
		return false;

	ioport_field *const field = m_key_rows[row]->field(ioport_value(1) << column);
	if (!field)
		return false;

	u16 const modifiers(m_modifiers->read());
	unsigned which = 0;
	if (modifiers & 0x01U)
	{
		// Control-character variants are the third character mapping on
		// the generic keyboard fields where one is defined.
		which = 2;
	}
	else
	{
		bool const alphabetic = (row == 1U && column >= 1U && column <= 10U)
				|| (row == 2U && column >= 1U && column <= 9U)
				|| (row == 3U && column >= 2U && column <= 8U);
		bool const shifted = bool(modifiers & 0x02U) != (bool(modifiers & 0x04U) && alphabetic);
		which = shifted ? 1U : 0U;
	}

	std::vector<char32_t> codes(field->keyboard_codes(which));
	if (codes.empty() && (modifiers & 0x01U))
	{
		// Generic keyboard control mappings retain ordinary characters for
		// keys without a dedicated control character.  These punctuation
		// positions are the exceptions in its ANSI/JIS control tables.
		uint8_t control = 0xffU;
		bool const jis = BIT(m_config->read(), 0);
		if (row == 0U && column == 0U)
			control = jis ? 0x1cU : 0x00U;
		else if (row == 0U && column == 11U)
			control = 0x1fU;
		else if (row == 0U && column == 12U)
			control = 0x1eU;
		else if (row == 1U && column == 11U)
			control = jis ? 0x00U : 0x1bU;
		else if (row == 1U && column == 12U)
			control = 0x1bU;
		else if (row == 1U && column == 13U)
			control = jis ? 0x1dU : 0x1cU;
		else if (row == 3U && column == 1U)
			control = 0x1cU;
		else if (row == 3U && column == 11U)
			control = 0x1fU;

		if (control != 0xffU)
		{
			data = control | ((modifiers & 0x08U) ? 0x80U : 0x00U);
			return true;
		}

		codes = field->keyboard_codes(0);
	}

	if (codes.empty() || codes.front() > 0xffU)
		return false;

	data = uint8_t(codes.front());
	if (modifiers & 0x08U)
		data |= 0x80U;
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
