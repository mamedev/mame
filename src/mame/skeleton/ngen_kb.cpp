// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard device
/*
    The keyboard is only partially emulated.  The implemented K1 letter,
    Return and GO keys use their documented key-post codes.  The K1 sends
    snapshots of depressed keys, with bit 7 set on the final key-post in a
    scan, and 0xc0 when no keys are down.  Typematic behaviour is inherited
    from the serial keyboard base.
*/

#include "emu.h"

#include "ngen_kb.h"

#define LOG_KEYBOARD (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

ngen_keyboard_device::ngen_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: serial_keyboard_device(mconfig, NGEN_KEYBOARD, tag, owner, 0)
	, m_keys_down(0U)
	, m_last_reset(0U)
	, m_id_timer(nullptr)
	, m_id_bytes_remaining(0U)
	, m_id_sequence_complete(false)
	, m_scan_requested(false)
	, m_last_shifted(false)
{
}


namespace {

// The K1 keyboard reports key positions, not translated ASCII characters.
// Its alphabetic key positions happen to use the lower-case ASCII values.
uint8_t ngen_key_code(uint8_t row, uint8_t column)
{
	if ((row == 0U) && (column >= 1U) && (column <= 9U))
		return '1' + (column - 1U);
	if ((row == 0U) && (column == 10U))
		return '0';
	if ((row == 0U) && (column == 11U))
		return '-';
	if ((row == 0U) && (column == 12U))
		return '=';
	if ((row == 0U) && (column == 13U))
		return 0x08U; // K1 Back Space.
	if ((row == 0U) && (column == 14U))
		return 0x22U; // K1 Delete Char.

	static constexpr uint8_t const ROW1_CODES[10] = { 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p' };
	static constexpr uint8_t const ROW2_CODES[9] = { 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l' };
	static constexpr uint8_t const ROW3_CODES[7] = { 'z', 'x', 'c', 'v', 'b', 'n', 'm' };

	if ((row == 1U) && (column == 0U))
		return 0x09U; // K1 Tab.
	if ((row == 1U) && (column >= 1U) && (column <= std::size(ROW1_CODES)))
		return ROW1_CODES[column - 1U];
	if ((row == 1U) && (column == 11U))
		return 0x5bU; // K1 left bracket.
	if ((row == 1U) && (column == 12U))
		return 0x5dU; // K1 right bracket.
	if ((row == 1U) && (column == 14U))
		return 0x01U; // K1 Cursor Up.
	if ((row == 1U) && (column == 15U))
		return 0x0bU; // K1 Cursor Down.
	if ((row == 2U) && (column >= 1U) && (column <= std::size(ROW2_CODES)))
		return ROW2_CODES[column - 1U];
	if ((row == 2U) && (column == 13U))
		return 0x0eU; // K1 Cursor Left.
	if ((row == 2U) && (column == 14U))
		return 0x0fU; // K1 Cursor Right.
	if ((row == 2U) && (column == 12U))
		return 0x0aU; // K1 Return key-post from the keyboard table.
	if ((row == 2U) && (column == 10U))
		return ';'; // Shift is reported separately, producing the colon key.
	if ((row == 2U) && (column == 11U))
		return '\'';
	if ((row == 3U) && (column == 12U))
		return 0x1bU; // K1 GO key-post from the documented key table.
	if ((row == 3U) && (column == 9U))
		return ',';
	if ((row == 3U) && (column == 10U))
		return '.';
	if ((row == 3U) && (column == 11U))
		return '/';
	if ((row == 3U) && (column == 15U))
		return ' ';
	if ((row == 3U) && (column >= 2U) && (column <= 8U))
		return ROW3_CODES[column - 2U];

	return 0xffU;
}

} // anonymous namespace


void ngen_keyboard_device::write(uint8_t data)
{
	// 0x92 enables keyboard scanning after a soft reset.
	// 0x8c requests the ROM checksum result; 0x9e enables echo mode.
	// Codes 0xAx and 0xBx appear to control the keyboard LEDs, lower nibbles controlling the state of the LEDs
	// When setting an error code via the LEDs, 0xB0 then 0xAE is sent (presumably for error code 0xE0),
	// so that means that 0xAx controls the Overtype, Lock, F1 and F2 LEDs, and 0xBx controls the F3, F8, F9 and F10 LEDs.
	LOGMASKED(LOG_KEYBOARD, "KB: received character %02x\n", data);
	if (data == 0x92U)
		m_echo = false;
	else if (m_echo)
	{
		send_key(data);
		return;
	}

	switch(data)
	{
	case 0x92U:  // soft reset
		m_last_reset = 0x01U;
		m_scan_requested = true;
		// Defer the first all-keys-up scan until the power-on ID sequence ends.
		if (m_id_sequence_complete)
			send_key(0xc0U);
		break;
	case 0x8cU:  // ROM checksum test
		send_key(0xf0U);
		break;
	case 0x9eU:  // echo test
		m_echo = true;
		break;
	}
}

static INPUT_PORTS_START( ngen_keyboard )
	PORT_INCLUDE(generic_keyboard)

	// The generic host keyboard has no GO key.  F13 is a single-key stand-in
	// that avoids MAME's built-in function-key controls and modifier chords,
	// which can be intercepted by the host OS.
	// This unused matrix position maps to the K1 GO key post in ngen_key_code().
	PORT_MODIFY("GENKBD_ROW3")
	PORT_BIT(0x1000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("GO") PORT_CODE(KEYCODE_F13)

	// The generic PC keyboard has no cursor-key fields in its four-row matrix.
	// Use otherwise-unused positions so the host arrow keys can send the K1
	// cursor key-posts without colliding with alphanumeric keys.
	PORT_MODIFY("GENKBD_ROW1")
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Up") PORT_CODE(KEYCODE_UP)
	PORT_BIT(0x8000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Down") PORT_CODE(KEYCODE_DOWN)

	PORT_MODIFY("GENKBD_ROW2")
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Left") PORT_CODE(KEYCODE_LEFT)
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Cursor Right") PORT_CODE(KEYCODE_RIGHT)

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_1200, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_BAUD_1200, "1200")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_2, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(serial_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")
INPUT_PORTS_END


ioport_constructor ngen_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ngen_keyboard);
}

void ngen_keyboard_device::device_start()
{
	serial_keyboard_device::device_start();
	m_id_timer = timer_alloc(FUNC(ngen_keyboard_device::id_sequence_tick), this);

	save_item(NAME(m_keys_down));
	save_item(NAME(m_last_reset));
	save_item(NAME(m_id_bytes_remaining));
	save_item(NAME(m_id_sequence_complete));
	save_item(NAME(m_scan_requested));
	save_item(NAME(m_last_shifted));
	save_item(NAME(m_echo));

	set_rcv_rate(1200);
}

void ngen_keyboard_device::device_reset()
{
	serial_keyboard_device::device_reset();

	m_keys_down = 0U;
	m_last_reset = 0U;
	m_id_bytes_remaining = 120U;
	m_id_sequence_complete = false;
	m_scan_requested = false;
	m_last_shifted = false;
	m_echo = false;

	// A K1 sends 120 FE synchronisation bytes, then B0 04 to identify the
	// dual-ported US K1 layout.  Pace the bytes at the serial rate to avoid
	// overflowing the buffered transmitter.
	m_id_timer->adjust(attotime::from_msec(10), 0, attotime::from_msec(10));
}

TIMER_CALLBACK_MEMBER(ngen_keyboard_device::id_sequence_tick)
{
	if (m_id_bytes_remaining)
	{
		send_key(0xfeU);
		--m_id_bytes_remaining;
		return;
	}

	send_key(0xb0U);
	send_key(0x04U);
	m_id_timer->reset();
	m_id_sequence_complete = true;
	if (m_scan_requested)
		send_key(0xc0U);
}

void ngen_keyboard_device::rcv_complete()
{
	receive_register_extract();
	write(get_received_char());
}

void ngen_keyboard_device::key_make(uint8_t row, uint8_t column)
{
	// Log host key presses separately from write(), which logs commands sent by the computer to the keyboard.
	LOGMASKED(LOG_KEYBOARD, "KB: host key down row %u column %u\n", row, column);
	uint8_t const code = ngen_key_code(row, column);
	if (code == 0xffU)
	{
		LOGMASKED(LOG_KEYBOARD, "KB: no NGEN key code for row %u column %u\n", row, column);
		return;
	}

	// Modifier changes do not arrive through key_make/key_break because the
	// generic keyboard exposes them on a separate input port.  If Shift was
	// released since the previous key event, explicitly clear the guest's held
	// key state before reporting this new key.
	uint16_t const modifiers = m_modifiers->read();
	bool const shifted = bool(BIT(modifiers, 1)) != bool(BIT(modifiers, 2));
	if (shifted && !m_last_shifted && m_scan_requested && m_id_sequence_complete)
		send_key(0xc8U); // Report Shift before the first key pressed with it.
	if (m_last_shifted && !shifted && m_scan_requested && m_id_sequence_complete)
		send_key(0xc0U);
	m_last_shifted = shifted;

	// The K1 sends a snapshot of depressed keys.  The high bit terminates a
	// scan after its last key; it does not indicate that a key was released.
	m_keys_down |= uint64_t(1) << ((row << 4) | column);
	if (m_scan_requested && m_id_sequence_complete)
		send_key_scan();
	m_last_reset = 0U;
}

void ngen_keyboard_device::key_repeat(uint8_t row, uint8_t column)
{
	uint8_t const code = ngen_key_code(row, column);
	if ((code != 0xffU) && m_scan_requested && m_id_sequence_complete)
		send_key_scan();
}

void ngen_keyboard_device::key_break(uint8_t row, uint8_t column)
{
	LOGMASKED(LOG_KEYBOARD, "KB: host key up row %u column %u\n", row, column);
	uint8_t const code = ngen_key_code(row, column);
	if (code == 0xffU)
		return;

	uint16_t const modifiers = m_modifiers->read();
	bool const shifted = bool(BIT(modifiers, 1)) != bool(BIT(modifiers, 2));
	if (shifted && !m_last_shifted && m_scan_requested && m_id_sequence_complete)
		send_key(0xc8U);
	if (m_last_shifted && !shifted && m_scan_requested && m_id_sequence_complete)
		send_key(0xc0U);
	m_last_shifted = shifted;

	// Key-up is represented by a later scan that omits the released key.  If
	// the scan is empty, the keyboard sends the special all-keys-up byte C0.
	m_keys_down &= ~(uint64_t(1) << ((row << 4) | column));
	if (m_scan_requested && m_id_sequence_complete)
		send_key_scan();
}

void ngen_keyboard_device::send_key_scan()
{
	uint8_t codes[64];
	unsigned count = 0U;

	// Shift is a separate K1 key-post.  Include it in the same scan as any
	// shifted character rather than sending an ASCII uppercase character.
	uint16_t const modifiers = m_modifiers->read();
	bool const shifted = bool(BIT(modifiers, 1)) != bool(BIT(modifiers, 2));

	for (unsigned row = 0; row < 4U; ++row)
	{
		for (unsigned column = 0; column < 16U; ++column)
		{
			if (m_keys_down & (uint64_t(1) << ((row << 4) | column)))
			{
				uint8_t const code = ngen_key_code(row, column);
				if (code != 0xffU)
					codes[count++] = code;
			}
		}
	}
	if (shifted)
		codes[count++] = 0x48U; // Left Shift key-post.

	if (!count)
	{
		send_key(0xc0U); // K1 all-keys-up scan.
		return;
	}

	// The K1 scans its key-post table in ascending code order.  In particular,
	// Shift (0x48) follows punctuation such as semicolon (0x3b); preserving that
	// order matters because the final byte terminates the whole scan.
	std::sort(codes, codes + count);

	// Bit 7 marks the final key-post in this scan.  Earlier bytes in a
	// multi-key scan have bit 7 clear.
	for (unsigned index = 0; index < count; ++index)
		send_key(codes[index] | ((index + 1U == count) ? 0x80U : 0x00U));
}

void ngen_keyboard_device::send_key(uint8_t code)
{
	// This is the byte the emulated keyboard transmits to the computer after translating a host key press.
	LOGMASKED(LOG_KEYBOARD, "KB: transmit key code %02x\n", code);
	serial_keyboard_device::send_key(code);
}

DEFINE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device, "ngen_kb", "NGEN Keyboard")
