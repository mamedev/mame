// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
// Convergent NGEN keyboard device
/*
    This is a partial K1 keyboard emulation.  Host keys are translated to
    K1 key-post codes; each scan reports the depressed-key snapshot, with bit
    7 set on its final key-post and 0xc0 used when no keys are down.  The
    keyboard scans the host matrix through MAME's matrix interface and sends
    bytes through its buffered serial interface.
*/

#include "emu.h"

#include "ngen_kb.h"
#include "machine/keyboard.ipp"

#define LOG_KEYBOARD (1U << 1)
#define VERBOSE (0)
#include "logmacro.h"

ngen_keyboard_device::ngen_keyboard_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock)
	: device_t(mconfig, NGEN_KEYBOARD, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, device_matrix_keyboard_interface(mconfig, *this, "GENKBD_ROW0", "GENKBD_ROW1", "GENKBD_ROW2", "GENKBD_ROW3")
	, m_modifiers(*this, "GENKBD_MOD")
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
	, m_keys_down(0U)
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
	if ((row == 3U) && (column == 0U))
		return 0x00U; // K1 Help key-post.
	if ((row == 3U) && (column == 12U))
		return 0x1bU; // K1 GO key-post from the documented key table.
	if ((row == 3U) && (column == 13U))
		return 0x0cU; // K1 Next Page key-post.
	if ((row == 3U) && (column == 14U))
		return 0x11U; // K1 Scroll Up key-post.
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


void ngen_keyboard_device::input_txd(int state)
{
	device_buffered_serial_interface::rx_w(state);
}


void ngen_keyboard_device::update_serial(int state)
{
	reset();
}


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
	// Unused matrix positions provide host bindings for the dedicated K1 Help
	// and GO key-posts in ngen_key_code().
	PORT_MODIFY("GENKBD_ROW3")
	PORT_BIT(0x0001U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Help") PORT_CODE(KEYCODE_F14)
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

	// The generic PC keyboard lacks the NGEN display-pad keys.  Map Page Down
	// and Page Up onto unused K1 matrix positions for Next Page and Scroll Up.
	PORT_MODIFY("GENKBD_ROW3")
	PORT_BIT(0x2000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Next Page") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT(0x4000U, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_NAME("Scroll Up") PORT_CODE(KEYCODE_PGUP)

	PORT_START("RS232_TXBAUD")
	PORT_CONFNAME(0xff, RS232_BAUD_1200, "TX Baud") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(ngen_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_BAUD_1200, "1200")

	PORT_START("RS232_DATABITS")
	PORT_CONFNAME(0xff, RS232_DATABITS_8, "Data Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(ngen_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_DATABITS_8, "8")

	PORT_START("RS232_PARITY")
	PORT_CONFNAME(0xff, RS232_PARITY_NONE, "Parity") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(ngen_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_PARITY_NONE, "None")

	PORT_START("RS232_STOPBITS")
	PORT_CONFNAME(0xff, RS232_STOPBITS_2, "Stop Bits") PORT_WRITE_LINE_DEVICE_MEMBER(DEVICE_SELF, FUNC(ngen_keyboard_device::update_serial))
	PORT_CONFSETTING( RS232_STOPBITS_2, "2")
INPUT_PORTS_END


ioport_constructor ngen_keyboard_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(ngen_keyboard);
}

void ngen_keyboard_device::device_start()
{
	m_id_timer = timer_alloc(FUNC(ngen_keyboard_device::id_sequence_tick), this);

	save_item(NAME(m_keys_down));
	save_item(NAME(m_id_bytes_remaining));
	save_item(NAME(m_id_sequence_complete));
	save_item(NAME(m_scan_requested));
	save_item(NAME(m_last_shifted));
	save_item(NAME(m_echo));

}

void ngen_keyboard_device::device_reset()
{
	clear_fifo();
	receive_register_reset();
	transmit_register_reset();
	set_data_frame(
			1,
			convert_databits(m_rs232_databits->read()),
			convert_parity(m_rs232_parity->read()),
			convert_stopbits(m_rs232_stopbits->read()));
	set_rcv_rate(convert_baud(m_rs232_txbaud->read()));
	set_tra_rate(convert_baud(m_rs232_txbaud->read()));
	output_rxd(1);
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

	reset_key_state();
	start_processing(attotime::from_hz(2'400));
	typematic_stop();

	m_keys_down = 0U;
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

void ngen_keyboard_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}


void ngen_keyboard_device::received_byte(uint8_t byte)
{
	write(byte);
}


void ngen_keyboard_device::will_scan_row(uint8_t)
{
	// Modifiers are separate input ports rather than matrix positions.  Watch
	// them during the regular matrix scan so Shift changes are posted even when
	// no ordinary key is pressed or released at the same time.
	uint16_t const modifiers = m_modifiers->read();
	bool const shifted = bool(BIT(modifiers, 1)) != bool(BIT(modifiers, 2));
	if (shifted != m_last_shifted)
	{
		m_last_shifted = shifted;
		if (m_scan_requested && m_id_sequence_complete)
			send_key_scan();
	}
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

	// The K1 sends a snapshot of depressed keys.  The high bit terminates a
	// scan after its last key; it does not indicate that a key was released.
	m_keys_down |= uint64_t(1) << ((row << 4) | column);
	if (m_scan_requested && m_id_sequence_complete)
		send_key_scan();
	typematic_start(row, column, attotime::from_msec(750), attotime::from_msec(50));
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
	if (typematic_is(row, column))
		typematic_stop();

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

	// Keep the currently modelled key posts in a stable order.  The physical
	// K1 matrix scan order still needs to be represented for multi-key chords.
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
	transmit_byte(code);
}

DEFINE_DEVICE_TYPE(NGEN_KEYBOARD, ngen_keyboard_device, "ngen_kb", "NGEN Keyboard")
