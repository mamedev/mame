// license:BSD-3-Clause
// copyright-holders:Dave Rand
/***************************************************************************

    Auto-baud ADM-3A terminal

    A "just works" serial terminal for the RS-232 slot:

    * Auto-baud / auto-framing: rather than relying on DIP switches matching
      the host, it watches the incoming line, captures the transition timing
      into a small ring, derives the bit cell from the shortest interval,
      snaps to the nearest standard rate and then re-decodes (replays) the
      captured characters so nothing typed before the lock is lost.  Because
      the emulated line has no jitter, the shortest interval is exactly one
      bit cell, so it can lock on the first character or two.  The terminal
      is assumed to be 7-bit ASCII, so the assembled byte is masked to 7 bits
      and any parity bit folds harmlessly into the (discarded) eighth bit;
      that lets a single 8N1 receiver decode 7E1/7O1/8N1 streams alike.

    * ADM-3A display: enough cursor handling to run vi.  The base
      generic_terminal already implements BS (left), CR, LF (down/scroll),
      ^K (up) and ^^ (home); this adds ESC= direct cursor addressing, ^L
      (non-destructive cursor right), ^Z (clear screen) and the ADM-31
      ESC T / ESC Y clear-to-end-of-line / -screen niceties.

    Off by default; selected with -rs232 auto3a.

***************************************************************************/

#include "emu.h"
#include "auto3a.h"

#include "machine/terminal.h"

#include <algorithm>
#include <cmath>
#include <vector>


namespace {

class auto3a_terminal_device : public generic_terminal_device,
	public device_buffered_serial_interface<16U>,
	public device_rs232_port_interface
{
public:
	auto3a_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void input_txd(int state) override;

	void update_serial(int state);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual void term_write(uint8_t data) override;
	virtual void tra_callback() override;
	virtual void send_key(uint8_t code) override;

private:
	virtual void received_byte(uint8_t byte) override;

	TIMER_CALLBACK_MEMBER(lock_timeout);

	void autobaud_edge(int state);
	void lock_and_replay();
	void reset_capture();
	static int snap_baud(double rate);

	// capacity of the pre-lock transition ring; large enough that a whole
	// power-on banner is captured and locked at the trailing idle rather than
	// hitting the cap mid-stream
	static constexpr unsigned CAP = 1024;
	// quiet time after the last transition that triggers a lock attempt;
	// longer than a whole character even at 110 baud (~90ms) so a gap is
	// unambiguously idle and not a run of same-value bits mid-character
	static constexpr int LOCK_QUIET_MS = 250;

	required_ioport m_rs232_txbaud;
	required_ioport m_rs232_databits;
	required_ioport m_rs232_parity;
	required_ioport m_rs232_stopbits;

	emu_timer *m_lock_timer;

	// auto-baud capture state
	bool      m_locked;
	bool      m_capturing;
	uint8_t   m_prev_level;
	attotime  m_prev_time;
	double    m_edge_dur[CAP];
	uint8_t   m_edge_level[CAP];
	uint32_t  m_edge_count;

	// ADM-3A escape-sequence state
	uint8_t   m_esc_state;
	uint8_t   m_esc_row;
};

auto3a_terminal_device::auto3a_terminal_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: generic_terminal_device(mconfig, SERIAL_TERMINAL_AUTO3A, tag, owner, clock, TERMINAL_WIDTH, TERMINAL_HEIGHT)
	, device_buffered_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_rs232_txbaud(*this, "RS232_TXBAUD")
	, m_rs232_databits(*this, "RS232_DATABITS")
	, m_rs232_parity(*this, "RS232_PARITY")
	, m_rs232_stopbits(*this, "RS232_STOPBITS")
	, m_lock_timer(nullptr)
	, m_locked(false)
	, m_capturing(false)
	, m_prev_level(1)
	, m_prev_time(attotime::zero)
	, m_edge_count(0)
	, m_esc_state(0)
	, m_esc_row(0)
{
	std::fill(std::begin(m_edge_dur), std::end(m_edge_dur), 0.0);
	std::fill(std::begin(m_edge_level), std::end(m_edge_level), 0);
}

static INPUT_PORTS_START(auto3a)
	PORT_INCLUDE(generic_terminal)

	PORT_RS232_BAUD("RS232_TXBAUD",   RS232_BAUD_9600, "TX Baud (until lock)", auto3a_terminal_device, update_serial)
	PORT_RS232_DATABITS("RS232_DATABITS", RS232_DATABITS_8, "Data Bits (until lock)", auto3a_terminal_device, update_serial)
	PORT_RS232_PARITY("RS232_PARITY", RS232_PARITY_NONE, "Parity (until lock)", auto3a_terminal_device, update_serial)
	PORT_RS232_STOPBITS("RS232_STOPBITS", RS232_STOPBITS_1, "Stop Bits (until lock)", auto3a_terminal_device, update_serial)
INPUT_PORTS_END

ioport_constructor auto3a_terminal_device::device_input_ports() const
{
	return INPUT_PORTS_NAME(auto3a);
}

void auto3a_terminal_device::device_start()
{
	generic_terminal_device::device_start();

	m_lock_timer = timer_alloc(FUNC(auto3a_terminal_device::lock_timeout), this);

	save_item(NAME(m_locked));
	save_item(NAME(m_capturing));
	save_item(NAME(m_prev_level));
	save_item(NAME(m_prev_time));
	save_item(NAME(m_edge_dur));
	save_item(NAME(m_edge_level));
	save_item(NAME(m_edge_count));
	save_item(NAME(m_esc_state));
	save_item(NAME(m_esc_row));
}

// the DIP-selected frame/rate is only the pre-lock default; auto-baud overrides
// the receive side (and matches the transmit side) once it locks
void auto3a_terminal_device::update_serial(int state)
{
	clear_fifo();

	int const startbits = 1;
	int const databits = convert_databits(m_rs232_databits->read());
	parity_t const parity = convert_parity(m_rs232_parity->read());
	stop_bits_t const stopbits = convert_stopbits(m_rs232_stopbits->read());

	set_data_frame(startbits, databits, parity, stopbits);

	int const txbaud = convert_baud(m_rs232_txbaud->read());
	set_tra_rate(txbaud);
	set_rcv_rate(txbaud);

	output_rxd(1);

	// TODO: make this configurable
	output_dcd(0);
	output_dsr(0);
	output_cts(0);

	receive_register_reset();
	transmit_register_reset();
}

void auto3a_terminal_device::device_reset()
{
	generic_terminal_device::device_reset();

	m_locked = false;
	m_esc_state = 0;
	m_esc_row = 0;
	reset_capture();

	update_serial(0);
}

void auto3a_terminal_device::reset_capture()
{
	m_capturing = false;
	m_edge_count = 0;
	m_prev_level = 1;
	m_lock_timer->reset();
}

//-------------------------------------------------
//  auto-baud
//-------------------------------------------------

void auto3a_terminal_device::input_txd(int state)
{
	// lock onto the first traffic and stay locked: a real terminal talks to a
	// fixed-rate host, so once the rate/framing is known every later character
	// is decoded live by the receiver
	if (m_locked)
		device_buffered_serial_interface::rx_w(state);
	else
		autobaud_edge(state);
}

void auto3a_terminal_device::autobaud_edge(int state)
{
	state = state ? 1 : 0;
	attotime const now = machine().time();

	if (!m_capturing)
	{
		// ignore the idle mark; start capturing at the first start bit (1->0)
		if (state)
			return;
		m_capturing = true;
		m_prev_level = state;
		m_prev_time = now;
		m_lock_timer->adjust(attotime::from_msec(LOCK_QUIET_MS));
		return;
	}

	if (state == m_prev_level)
		return;

	// record the interval that just finished (held at m_prev_level)
	if (m_edge_count < CAP)
	{
		m_edge_dur[m_edge_count] = (now - m_prev_time).as_double();
		m_edge_level[m_edge_count] = m_prev_level;
		m_edge_count++;
	}
	m_prev_level = state;
	m_prev_time = now;

	if (m_edge_count >= CAP)
		lock_and_replay();          // safety cap on a gapless stream
	else
		m_lock_timer->adjust(attotime::from_msec(LOCK_QUIET_MS));
}

TIMER_CALLBACK_MEMBER(auto3a_terminal_device::lock_timeout)
{
	if (!m_locked && m_capturing)
		lock_and_replay();
}

int auto3a_terminal_device::snap_baud(double rate)
{
	static const int rates[] =
			{ 110, 150, 300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 115200 };

	int best = rates[0];
	double bestdiff = std::fabs(rate - rates[0]);
	for (int r : rates)
	{
		double const d = std::fabs(rate - r);
		if (d < bestdiff)
		{
			bestdiff = d;
			best = r;
		}
	}
	return best;
}

void auto3a_terminal_device::lock_and_replay()
{
	// shortest captured interval == one bit cell (no jitter in emulation)
	double cell = 0.0;
	for (uint32_t i = 0; i < m_edge_count; i++)
		if (m_edge_dur[i] > 0.0 && (cell == 0.0 || m_edge_dur[i] < cell))
			cell = m_edge_dur[i];

	if (m_edge_count == 0 || cell <= 0.0)
	{
		reset_capture();
		return;
	}

	int const baud = snap_baud(1.0 / cell);

	// expand the captured intervals back into a bit stream, then re-frame it:
	// start(0) + 8 data bits LSB-first + stop, masked to 7-bit ASCII
	std::vector<uint8_t> bits;
	for (uint32_t i = 0; i < m_edge_count; i++)
	{
		int n = int(m_edge_dur[i] / cell + 0.5);
		if (n < 1)
			n = 1;
		bits.insert(bits.end(), n, m_edge_level[i]);
	}
	bits.insert(bits.end(), 12, 1);     // trailing idle to supply final stop bit(s)

	// Decode the captured characters (8 data bits LSB-first) and, in the same
	// pass, classify the framing so the transmitter can answer with the correct
	// parity.  The payload is 7-bit ASCII, so the 8th bit is either the ASCII
	// MSB (always 0 -> no parity, 8 data) or the parity of the low 7 bits
	// (-> 7 data, even/odd).  Across a whole burst this is unambiguous.
	std::vector<uint8_t> chars;
	unsigned ones = 0, zeros = 0, even_match = 0, odd_match = 0;
	for (size_t i = 0; i + 9 < bits.size(); )
	{
		if (bits[i])                    // mark / idle / stop
		{
			i++;
			continue;
		}
		unsigned byte = 0;              // bits[i] == 0 -> start bit
		for (int b = 0; b < 8; b++)
			if (bits[i + 1 + b])
				byte |= (1u << b);
		uint8_t const lo7 = byte & 0x7f;
		uint8_t const hi  = (byte >> 7) & 1;
		unsigned pc = 0;
		for (int b = 0; b < 7; b++)
			pc += (lo7 >> b) & 1;
		uint8_t const evenbit = pc & 1; // even-parity bit value for the low 7 bits
		if (hi == evenbit)       even_match++;
		if (hi == (evenbit ^ 1)) odd_match++;
		if (hi) ones++; else zeros++;
		chars.push_back(lo7);
		i += 9;                         // consume start + 8 data; stop re-read as mark
	}

	unsigned const nchars = unsigned(chars.size());
	int data_bits = 8;
	parity_t parity = PARITY_NONE;
	bool const varied = ones && zeros;  // need both bit values to tell even from odd
	if (nchars && zeros != nchars)
	{
		if (varied && even_match == nchars)      { data_bits = 7; parity = PARITY_EVEN; }
		else if (varied && odd_match == nchars)  { data_bits = 7; parity = PARITY_ODD; }
		else if (ones == nchars)                 { data_bits = 7; parity = PARITY_MARK; }
	}

	// lock both directions to the detected rate + framing, so typed characters
	// are transmitted with the right parity.  Receive still masks to 7 bits, so
	// a parity bit (when present) never reaches the display.
	set_data_frame(1, data_bits, parity, STOP_BITS_1);
	set_rcv_rate(baud);
	set_tra_rate(baud);
	receive_register_reset();
	transmit_register_reset();
	m_locked = true;

	for (uint8_t c : chars)
		received_byte(c);

	reset_capture();
}

//-------------------------------------------------
//  serial plumbing
//-------------------------------------------------

void auto3a_terminal_device::send_key(uint8_t code)
{
	transmit_byte(code);
}

void auto3a_terminal_device::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void auto3a_terminal_device::received_byte(uint8_t byte)
{
	term_write(byte & 0x7f);
}

//-------------------------------------------------
//  ADM-3A display
//-------------------------------------------------

void auto3a_terminal_device::term_write(uint8_t data)
{
	switch (m_esc_state)
	{
	case 0:
		switch (data)
		{
		case 0x1b:  // ESC - start a sequence
			m_esc_state = 1;
			return;
		case 0x1a:  // ^Z - clear screen and home (reuse base form-feed = clear)
			generic_terminal_device::term_write(0x0c);
			return;
		case 0x0c:  // ^L - non-destructive cursor right (ADM-3A; base FF clears)
			if (m_x_pos < m_width - 1)
				m_x_pos++;
			return;
		default:
			// printable, plus BS/CR/LF/^K(up)/^^(home)/bell/tab handled by base
			generic_terminal_device::term_write(data);
			return;
		}

	case 1: // after ESC
		switch (data)
		{
		case '=':   // direct cursor address: ESC = <row+0x20> <col+0x20>
			m_esc_state = 2;
			return;
		case 'T':   // ADM-31 clear to end of line
			std::fill(m_buffer.get() + (m_y_pos * m_width + m_x_pos),
					m_buffer.get() + ((m_y_pos + 1) * m_width), 0x20);
			m_esc_state = 0;
			return;
		case 'Y':   // ADM-31 clear to end of screen
			std::fill(m_buffer.get() + (m_y_pos * m_width + m_x_pos),
					m_buffer.get() + (m_width * m_height), 0x20);
			m_esc_state = 0;
			return;
		default:    // unknown - swallow
			m_esc_state = 0;
			return;
		}

	case 2: // cursor address - row
		m_esc_row = (data >= 0x20) ? (data - 0x20) : 0;
		if (m_esc_row >= m_height)
			m_esc_row = m_height - 1;
		m_esc_state = 3;
		return;

	case 3: // cursor address - column
		{
			uint8_t col = (data >= 0x20) ? (data - 0x20) : 0;
			if (col >= m_width)
				col = m_width - 1;
			m_x_pos = col;
			m_y_pos = m_esc_row;
		}
		m_esc_state = 0;
		return;
	}
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(SERIAL_TERMINAL_AUTO3A, device_rs232_port_interface, auto3a_terminal_device, "auto3a_terminal", "Auto-baud ADM-3A Terminal")
