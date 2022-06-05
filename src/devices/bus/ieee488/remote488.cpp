// license:BSD-3-Clause
// copyright-holders: F. Ulivi
/*********************************************************************

    remote488.cpp

    This device allows the interfacing of a local IEEE-488 bus with
    external devices (i.e. outside of MAME environment).
    It's based on a simple text based protocol. The protocol relies
    on a bidirectional stream-based connection (a bitbanger).
    Main design features of the protocol:
    - Transparent to data & commands exchanged
    - Transparent to remote/local position of controller & controlled
      devices
    - Can be cross-connected: by routing the output direction of a
      running MAME instance into the input direction of another instance
      and viceversa, the two buses form a single logical bus.

    Protocol exchanges messages that are composed of a single uppercase
    letter, a colon, a byte value expressed as 2 hex digits and a terminator
    character.
    Valid terminator characters are ',' or ';' or whitespace. Extra
    whitespace before and after the message is ignored.
    The letter encodes the type of the message and the byte has different
    meaning according to type of message. Not all message types carry
    a meaningful byte value (but for uniformity the byte is always sent).
    Example of a message: "D:55 " (byte with 0x55 value exchanged on the
    bus without EOI being asserted).
    Example of a sequence of messages: D:AA,D:55,E:00 or
    D:AA\nD:55\nE:00.

    The following table summarizes the various message types, the directions
    wrt the remotizer in which they are meaningful, whether the byte
    carries a value and the purpose of the message.

    | Type | Direction | Has byte? | Meaning                         |
    |------+-----------+-----------+---------------------------------|
    | D    | I/O       | Yes       | Non-EOI DAB or command byte     |
    | E    | I/O       | Yes       | EOI DAB                         |
    | J    | O         | No        | Echo request                    |
    | K    | I         | No        | Echo reply                      |
    | P    | I         | Yes       | Set byte for next parallel poll |
    | Q    | O         | No        | Request 'P' msg                 |
    | R    | I/O       | Yes       | Set bus control signals to 0    |
    | S    | I/O       | Yes       | Set bus control signals to 1    |
    | X    | I/O       | No        | Checkpoint in byte string       |
    | Y    | I/O       | Yes       | Checkpoint reached              |

    - D messages

    This type of msg exchanges non-EOI data bytes and commands.
    In the output direction the remotizer implements an acceptor so
    that the proper 3-way handshake is implemented with the
    source. The acceptor has no delays between state transitions so
    that it can keep up with any speed of the source.
    In the input direction a source is implemented to handshake with
    local acceptor(s). The FSM in the source has no delays so that it can
    operate at whatever speed the slowest acceptor on the bus can sustain.
    Bytes are removed from the input buffer of the stream as the local
    acceptors acknowledge them. No input message is processed by the
    remotizer when it's waiting for 3-way handshake with acceptors to
    complete.
    Byte values are expressed in positive logic (which is the opposite
    of what's sent on the bus). To discriminate between DAB and command
    bytes the receiver of this message has to check its state of ATN
    signal (see R&S messages).
    A simple infrastructure for synchronization between near and far
    end is provided. The sender of a string of bytes can insert a
    "checkpoint" msg in the string. This msg is turned into a
    "checkpoint reached" msg by the receiver and sent back. This msg
    reports to the sender that the receiver has processed data up to
    the last checkpoint.
    The acceptor in the remotizer sends a checkpoint in two cases: when a
    byte with EOI is accepted and sent to the far end or when there's
    a pause in the byte string longer than 10 ms. This is just for
    safety, normal byte strings are always terminated by EOI by the
    sender. The remotizer acceptor stops accepting data after sending
    a checkpoint. It resumes when the "checkpoint reached" msg is sent
    back. Bytes of I/F commands never use checkpointing.
    The source handshake (SH) in the remotizer accepts data bytes from
    the far end and puts them on the local bus. A simple heuristic is
    implemented to recognize the situation when the far end has sent
    more data than a local acceptor is willing to take. Whenever ATN
    is asserted by a local device (the controller) all input bytes are
    discarded up to the next checkpoint. The "checkpoint reached" msg
    then reports that data has been completely/partially flushed by
    setting its byte to 1. In normal cases byte is set to 0.
    Usage of checkpointing is optional. The minimum the far end should
    implement is sending back a "checkpoint reached" msg whenever a
    checkpoint is received.

    - E messages

    This type of message carries DAB that are sourced on the bus with
    EOI asserted. These messages have the same logic of 'D' messages
    (see above).  These messages never carry command bytes (because
    the condition where both ATN & EOI are asserted is used for
    parallel polling).

    - J & K messages

    This message pair is used to probe the connection with the
    external devices because bitbanger streams do not report in any
    way if they are connected to outside or not. Any output byte is
    silently discarded if the stream is not connected.
    From the remotizer point of view the connection is up when a
    message of any kind comes in from the outside. When there's no
    traffic to exchange, the remotizer sends a J message every 0.5 s.
    The remote device should reply with a K msg to every J msg it
    sees.  Connection status is set to "down" when three consecutive J
    msgs are sent out without being replied to.

    - P & Q messages

    The 'P' msg is used by an external device to set the response to
    be placed on the bus when a local controller does a parallel
    poll. Message byte is encoded in positive logic (i.e. any bit set
    to 1 is forced to 0 on data bus during parallel poll).  The
    response set by P msg is used during parallel polls until changed
    by another P msg. A P msg is never sent out by the remotizer.
    The Q msg is sent by the remotizer whenever a parallel poll is
    performed locally. Its purpose is to solicit a P msg from the
    external device. The external device may choose to send P msgs in
    reply to Q msgs or to ignore them and send P msgs asynchronously.
    Parallel poll is usually a very fast operation (a few microseconds)
    so it's possible, due to latency, that the reply to a Q msg is
    applied on the bus many polls later. The remotizer doesn't repeat
    the transmission of a Q msg until a P msg is received to avoid
    generating a lot of traffic if the local controller rapidly
    repeats parallel polls.

    - R & S messages

    These messages are used to align the state of bus control lines
    with the external devices. Each control line is mapped to a bit in
    the accompanying byte, according to this table.

    | Bit | Signal |
    |-----+--------|
    | 0   | ATN    |
    | 1   | IFC    |
    | 2   | REN    |
    | 3   | SRQ    |

    A "R" message clears (sets to 0, i.e. asserts) all signals that
    are set to "1" in the byte, all others retain their value.
    A "S" message sets signals to 1, i.e. de-asserts them.
    The remotizer may respond with a "R" message if an incoming "S"
    message is attempting to set to 1 a signal that is locally forced
    to 0.

    - X & Y messages

    See "D" messages above. "X" message is sent to request the
    receiver to acknowledge the reception by sending back a "Y"
    message.  When used, the "X" message should be sent at the end of
    a string of bytes by the sender. The reception of "Y" message
    confirms that the receiver has seen (though not necessarily
    accepted) the whole string.
    The "X" -> "Y" sequence is used in the local AH for flow control:
    once "X" is sent out, the acceptor pauses accepting bytes until a
    "Y" is received.  For this reason the far end should always
    implement, at the very least, the sending back of "Y" whenever "X"
    is seen.
    A "X" message carries no info in its byte. A "Y" message reports
    whether the preceding byte string was entirely accepted by the
    receiver or not (with a 00 or 01 value in the associated byte,
    respectively).

    Limits & potential issues:
    - There are no checks for violations of IEEE-488 protocol. It's
      possible, for example, to put the bus in an illegal state by
      sending a 'E' msg when the local controller is asserting ATN
      (thus creating a phantom parallel poll).
    - The heuristic to discard byte strings in the SH could be a bit
      too simple and do the wrong thing in few rare cases.
    - It's difficult to achieve accurate synchronization between the
      local emulation time and the external time.

    TODOs/possible enhancements:
    - Implement handling of incoming Q msgs (needed when parallel poll
      is being performed by a remote controller)
    - Enhancement: implement a msg for accurate time synchronization

*********************************************************************/

#include "emu.h"
#include "remote488.h"

// Debugging
#define LOG_PARSER_MASK (1U << 1)
#define LOG_PARSER(...) LOGMASKED(LOG_PARSER_MASK, __VA_ARGS__)

#define VERBOSE (LOG_GENERAL)

#include "logmacro.h"

// Bit manipulation
namespace {
	template<typename T> constexpr T BIT_MASK(unsigned n)
	{
		return (T)1U << n;
	}

	template<typename T> void BIT_CLR(T& w , unsigned n)
	{
		w &= ~BIT_MASK<T>(n);
	}

	template<typename T> void BIT_SET(T& w , unsigned n)
	{
		w |= BIT_MASK<T>(n);
	}

	template<typename T> void COPY_BIT(bool bit , T& w , unsigned n)
	{
		if (bit) {
			BIT_SET(w , n);
		} else {
			BIT_CLR(w , n);
		}
	}
}

// Message types
constexpr char MSG_SIGNAL_CLEAR       = 'R'; // I/O: Clear signal(s)
constexpr char MSG_SIGNAL_SET         = 'S'; // I/O: Set signal(s)
constexpr char MSG_DATA_BYTE          = 'D'; // I/O: Cmd/data byte (no EOI)
constexpr char MSG_END_BYTE           = 'E'; // I/O: Data byte (with EOI)
constexpr char MSG_PP_DATA            = 'P'; // I:   Parallel poll data
constexpr char MSG_PP_REQUEST         = 'Q'; // O:   Request PP data
constexpr char MSG_ECHO_REQ           = 'J'; // O:   Heartbeat msg: echo request
constexpr char MSG_ECHO_REPLY         = 'K'; // I:   Heartbeat msg: echo reply
constexpr char MSG_CHECKPOINT         = 'X'; // I/O: Checkpoint in byte stream
constexpr char MSG_CHECKPOINT_REACHED = 'Y'; // I/O: Checkpoint reached

// Timings
constexpr unsigned POLL_PERIOD_US   = 20;   // Poll period (µs)
constexpr unsigned HEARTBEAT_MS     = 500;  // Heartbeat ping period (ms)
constexpr unsigned MAX_MISSED_HB    = 3;    // Missed heartbeats to declare the connection dead
constexpr unsigned AH_TO_MS         = 10;   // Timeout in AH to report a byte string terminated (ms)

// device type definition
DEFINE_DEVICE_TYPE(REMOTE488, remote488_device, "remote488", "IEEE-488 Remotizer")

remote488_device::remote488_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig , REMOTE488 , tag , owner , clock),
	device_ieee488_interface(mconfig , *this),
	m_stream(*this , "stream")
{
}

void remote488_device::device_add_mconfig(machine_config &config)
{
	BITBANGER(config, m_stream, 0);
}

void remote488_device::ieee488_eoi(int state)
{
	update_pp();
}

void remote488_device::ieee488_dav(int state)
{
	update_ah_fsm();
}

void remote488_device::ieee488_nrfd(int state)
{
	update_sh_fsm();
}

void remote488_device::ieee488_ndac(int state)
{
	update_sh_fsm();
}

void remote488_device::ieee488_ifc(int state)
{
	update_signal(SIGNAL_IFC_BIT , state);
	if (!state) {
		LOG("IFC\n");
		flush_data();
		bus_reset();
	}
}

void remote488_device::ieee488_srq(int state)
{
	update_signal(SIGNAL_SRQ_BIT , state);
}

void remote488_device::ieee488_atn(int state)
{
	update_signal(SIGNAL_ATN_BIT , state);
	update_sh_fsm();
	update_ah_fsm();
	update_pp();
}

void remote488_device::ieee488_ren(int state)
{
	update_signal(SIGNAL_REN_BIT , state);
}

void remote488_device::device_start()
{
	m_poll_timer = timer_alloc(FUNC(remote488_device::process_input_msgs), this);
	m_hb_timer = timer_alloc(FUNC(remote488_device::heartbeat_tick), this);
	m_ah_timer = timer_alloc(FUNC(remote488_device::checkpoint_timeout_tick), this);
}

void remote488_device::device_reset()
{
	m_no_propagation = true;
	m_in_signals = 0xff;
	m_bus->atn_w(this , 1);
	m_bus->eoi_w(this , 1);
	m_bus->ifc_w(this , 1);
	m_bus->ren_w(this , 1);
	m_bus->srq_w(this , 1);
	m_out_signals = 0xff;
	m_no_propagation = false;

	// Fake disconnection
	m_connected = true;
	set_connection(false);

	m_ibf = false;
	m_flush_bytes = false;
	m_waiting_checkpoint = false;
	bus_reset();
}

void remote488_device::bus_reset()
{
	m_sh_state = REM_SH_SIDS;
	m_ah_state = REM_AH_ACRS;
	m_rx_state = REM_RX_WAIT_CH;
	m_poll_timer->adjust(attotime::from_usec(POLL_PERIOD_US) , 0 , attotime::from_usec(POLL_PERIOD_US));
	m_pp_data = 0;
	m_pp_requested = false;
	update_ah_fsm();
	update_sh_fsm();
	update_pp();
}

TIMER_CALLBACK_MEMBER(remote488_device::process_input_msgs)
{
	uint8_t data;
	char msg_ch;
	while ((msg_ch = recv_update(data)) != 0) {
		set_connection(true);
		LOG("%.6f Rx %c %02x\n" , machine().time().as_double() , msg_ch , data);

		switch (msg_ch) {
		case MSG_SIGNAL_CLEAR:
			update_signals_from_rem(0 , data);
			break;

		case MSG_SIGNAL_SET:
			update_signals_from_rem(data , 0);
			break;

		case MSG_DATA_BYTE:
		case MSG_END_BYTE:
			if (m_flush_bytes) {
				LOG("Flushed\n");
				m_poll_timer->adjust(attotime::zero);
			} else {
				m_poll_timer->reset();
				recvd_data_byte(data , msg_ch == MSG_END_BYTE);
			}
			return;

		case MSG_PP_DATA:
			m_pp_data = data;
			m_pp_requested = false;
			update_pp_dio();
			break;

		case MSG_ECHO_REQ:
			send_update(MSG_ECHO_REPLY , 0);
			break;

		case MSG_CHECKPOINT:
			send_update(MSG_CHECKPOINT_REACHED , m_flush_bytes);
			m_flush_bytes = false;
			break;

		case MSG_CHECKPOINT_REACHED:
			if (m_waiting_checkpoint) {
				m_waiting_checkpoint = false;
				update_ah_fsm();
			}
			break;

		default:
			break;
		}
	}
	if (!m_poll_timer->enabled()) {
		m_poll_timer->adjust(attotime::from_usec(POLL_PERIOD_US) , 0 , attotime::from_usec(POLL_PERIOD_US));
	}
}

TIMER_CALLBACK_MEMBER(remote488_device::heartbeat_tick)
{
	if (m_connected && m_connect_cnt && --m_connect_cnt == 0) {
		set_connection(false);
	}
	send_update(MSG_ECHO_REQ , 0);
}

TIMER_CALLBACK_MEMBER(remote488_device::checkpoint_timeout_tick)
{
	if (!m_waiting_checkpoint) {
		LOG("Checkpoint T/O\n");
		ah_checkpoint();
	}
}

void remote488_device::set_connection(bool state)
{
	if (state) {
		if (!m_connected) {
			// Just connected
			m_connected = true;
			LOG("Connected!\n");
			// Align signal state on both sides
			uint8_t tmp = m_out_signals & ((1 << SIGNAL_COUNT) - 1);
			if (tmp) {
				send_update(MSG_SIGNAL_SET , tmp);
			}
			tmp = ~m_out_signals & ((1 << SIGNAL_COUNT) - 1);
			if (tmp) {
				send_update(MSG_SIGNAL_CLEAR , tmp);
			}
		}
		m_hb_timer->adjust(attotime::from_msec(HEARTBEAT_MS) , 0 , attotime::from_msec(HEARTBEAT_MS));
		m_connect_cnt = MAX_MISSED_HB;
	} else {
		if (m_connected) {
			// Disconnected
			m_connected = false;
			LOG("Connection lost!\n");
			update_ah_fsm();
			m_hb_timer->adjust(attotime::from_msec(HEARTBEAT_MS) , 0 , attotime::from_msec(HEARTBEAT_MS));
		}
	}
}

void remote488_device::recvd_data_byte(uint8_t data , bool eoi)
{
	m_ib = data;
	m_ib_eoi = eoi;
	m_ibf = true;
	if (is_local_atn_active()) {
		flush_data();
	}
	update_sh_fsm();
	update_ah_fsm();
}

void remote488_device::flush_data()
{
	if (m_ibf) {
		LOG("Flushing enabled\n");
		m_flush_bytes = true;
		m_ibf = false;
		m_poll_timer->adjust(attotime::zero);
	}
}

void remote488_device::update_signals_from_rem(uint8_t to_set , uint8_t to_clear)
{
	uint8_t diff = m_in_signals;
	m_in_signals |= to_set;
	m_in_signals &= ~to_clear;
	diff ^= m_in_signals;

	//LOG("REM SIG %02x %02x\n" , m_in_signals , diff);
	m_no_propagation = true;

	if (BIT(diff , SIGNAL_ATN_BIT)) {
		m_bus->atn_w(this , BIT(m_in_signals , SIGNAL_ATN_BIT));
	}
	if (BIT(diff , SIGNAL_IFC_BIT)) {
		m_bus->ifc_w(this , BIT(m_in_signals , SIGNAL_IFC_BIT));
	}
	if (BIT(diff , SIGNAL_REN_BIT)) {
		m_bus->ren_w(this , BIT(m_in_signals , SIGNAL_REN_BIT));
	}
	if (BIT(diff , SIGNAL_SRQ_BIT)) {
		m_bus->srq_w(this , BIT(m_in_signals , SIGNAL_SRQ_BIT));
	}

	m_no_propagation = false;
}

void remote488_device::update_signal(signal_bit bit , int state)
{
	if (!m_no_propagation) {
		uint8_t tmp = m_out_signals;
		COPY_BIT(state , tmp , bit);
		update_state(tmp);
	}
}

void remote488_device::update_state(uint8_t new_signals)
{
	uint8_t to_set = new_signals & ~m_out_signals;
	uint8_t to_clear = ~new_signals & m_out_signals;

	m_out_signals = new_signals;

	if (is_local_atn_active()) {
		flush_data();
	}

	if (to_set) {
		send_update(MSG_SIGNAL_SET , to_set);
	}
	if (to_clear) {
		send_update(MSG_SIGNAL_CLEAR , to_clear);
	}
}

void remote488_device::send_update(char type , uint8_t data)
{
	std::string buff = util::string_format("%c:%02x\n" , type , data);
	LOG("%.6f %s" , machine().time().as_double() , buff);
	for (char c : buff) {
		m_stream->output(c);
	}
}

bool remote488_device::a2hex(char c , uint8_t& out)
{
	if (c >= '0' && c <= '9') {
		out = c - '0';
		return true;
	} else if (c >= 'a' && c <= 'f') {
		out = c - 'a' + 10;
		return true;
	} else if (c >= 'A' && c <= 'F') {
		out = c - 'A' + 10;
		return true;
	} else {
		return false;
	}
}

bool remote488_device::is_msg_type(char c)
{
	// Recognize type of input messages
	return c == MSG_SIGNAL_CLEAR ||
		c == MSG_SIGNAL_SET ||
		c == MSG_DATA_BYTE ||
		c == MSG_END_BYTE ||
		c == MSG_PP_DATA ||
		c == MSG_ECHO_REPLY ||
		c == MSG_CHECKPOINT ||
		c == MSG_CHECKPOINT_REACHED;
}

bool remote488_device::is_terminator(char c)
{
	// Match message terminator characters
	return c == ',' ||
		c == ';';
}

bool remote488_device::is_space(char c)
{
	// Match whitespace characters
	return c == ' ' ||
		c == '\t' ||
		c == '\r' ||
		c == '\n';
}

char remote488_device::recv_update(uint8_t& data)
{
	char c;
	unsigned i;

	// Do not iterate too much..
	for (i = 0; i < 8 && m_stream->input(&c , 1); i++) {
		int prev_state = m_rx_state;
		switch (m_rx_state) {
		case REM_RX_WAIT_CH:
			if (is_msg_type(c)) {
				m_rx_ch = c;
				m_rx_state = REM_RX_WAIT_COLON;
			} else if (!is_space(c)) {
				m_rx_state = REM_RX_WAIT_WS;
			}
			break;

		case REM_RX_WAIT_COLON:
			if (c == ':') {
				m_rx_state = REM_RX_WAIT_1ST_HEX;
			} else {
				m_rx_state = REM_RX_WAIT_WS;
			}
			break;

		case REM_RX_WAIT_1ST_HEX:
			if (a2hex(c , m_rx_data)) {
				m_rx_state = REM_RX_WAIT_2ND_HEX;
			} else {
				m_rx_state = REM_RX_WAIT_WS;
			}
			break;

		case REM_RX_WAIT_2ND_HEX:
			{
				uint8_t tmp;
				if (a2hex(c , tmp)) {
					m_rx_data = (m_rx_data << 4) | tmp;
					m_rx_state = REM_RX_WAIT_SEP;
				} else {
					m_rx_state = REM_RX_WAIT_WS;
				}
			}
			break;

		case REM_RX_WAIT_SEP:
			if (is_terminator(c) || is_space(c)) {
				m_rx_state = REM_RX_WAIT_CH;
				LOG_PARSER("PARSE %02x %d->%d\n" , c , prev_state , m_rx_state);
				data = m_rx_data;
				return m_rx_ch;
			} else {
				m_rx_state = REM_RX_WAIT_WS;
			}
			break;

		case REM_RX_WAIT_WS:
			if (is_terminator(c) || is_space(c)) {
				m_rx_state = REM_RX_WAIT_CH;
			}
			break;

		default:
			m_rx_state = REM_RX_WAIT_CH;
			break;
		}
		LOG_PARSER("PARSE %02x %d->%d\n" , c , prev_state , m_rx_state);
	}
	return 0;
}

bool remote488_device::is_local_atn_active() const
{
	return !BIT(m_out_signals , SIGNAL_ATN_BIT) && BIT(m_in_signals , SIGNAL_ATN_BIT);
}

void remote488_device::ah_checkpoint()
{
	m_waiting_checkpoint = true;
	m_ah_timer->reset();
	send_update(MSG_CHECKPOINT , 0);
}

void remote488_device::update_ah_fsm()
{
	bool changed = true;

	while (changed) {
		//LOG("AH %d DAV %d\n" , m_ah_state , m_bus->dav_r());
		int prev_state = m_ah_state;

		if (m_sh_state != REM_SH_SIDS || !m_connected) {
			m_ah_state = REM_AH_AIDS;
		} else {
			switch (m_ah_state) {
			case REM_AH_AIDS:
				m_ah_state = REM_AH_ACRS;
				break;

			case REM_AH_ACRS:
				if (!m_bus->dav_r()) {
					m_ah_state = REM_AH_ACDS;
				}
				break;

			case REM_AH_ACDS:
				if (m_bus->dav_r()) {
					m_ah_state = REM_AH_ACRS;
				} else if (!m_waiting_checkpoint) {
					uint8_t dio = ~m_bus->dio_r();

					if (!m_bus->eoi_r()) {
						send_update(MSG_END_BYTE , dio);
						ah_checkpoint();
					} else {
						send_update(MSG_DATA_BYTE , dio);
						if (!BIT(m_out_signals , SIGNAL_ATN_BIT)) {
							// I/F commands have no checkpoint
							m_ah_timer->reset();
						} else {
							m_ah_timer->adjust(attotime::from_msec(AH_TO_MS));
						}
					}
					m_ah_state = REM_AH_AWNS;
				}
				break;

			case REM_AH_AWNS:
				if (m_bus->dav_r()) {
					m_ah_state = REM_AH_ACRS;
				}
				break;

			default:
				m_ah_state = REM_AH_ACRS;
				break;
			}
		}
		changed = prev_state != m_ah_state;

		m_bus->ndac_w(this , m_ah_state == REM_AH_AIDS || m_ah_state == REM_AH_AWNS);
		m_bus->nrfd_w(this , m_ah_state == REM_AH_AIDS || m_ah_state == REM_AH_ACRS);
	}
}

void remote488_device::update_sh_fsm()
{
	bool changed = true;

	while (changed) {
		int prev_state = m_sh_state;
		//LOG("SH %d LATN %d NRFD %d NDAC %d\n" , m_sh_state , is_local_atn_active() , m_bus->nrfd_r() , m_bus->ndac_r());

		if (is_local_atn_active() || !m_ibf) {
			// Reset condition
			m_sh_state = REM_SH_SIDS;
		} else {
			switch (m_sh_state) {
			case REM_SH_SIDS:
				m_sh_state = REM_SH_SDYS;
				break;

			case REM_SH_SDYS:
				if (m_bus->nrfd_r()) {
					m_sh_state = REM_SH_STRS;
				}
				break;

			case REM_SH_STRS:
				if (m_bus->ndac_r()) {
					m_sh_state = REM_SH_SIDS;
					LOG("Sourced %02x\n" , m_ib);
					m_ibf = false;
					// Schedule an immediate poll for incoming messages
					// This allows sourcing string of bytes on the bus at the highest possible speed
					m_poll_timer->adjust(attotime::zero);
				}
				break;

			default:
				m_sh_state = REM_SH_SIDS;
				break;
			}
		}
		changed = prev_state != m_sh_state;

		if (m_sh_state == REM_SH_SDYS || m_sh_state == REM_SH_STRS) {
			m_bus->eoi_w(this , !m_ib_eoi);
			m_sh_dio = ~m_ib;
		} else {
			m_bus->eoi_w(this , 1);
			m_sh_dio = 0xff;
		}
		update_dio();
		m_bus->dav_w(this , m_sh_state == REM_SH_SIDS || m_sh_state == REM_SH_SDYS);
	}
}

bool remote488_device::is_local_pp_active() const
{
	return is_local_atn_active() && !m_bus->eoi_r();
}

void remote488_device::update_pp()
{
	if (is_local_pp_active() && m_connected && !m_pp_requested) {
		send_update(MSG_PP_REQUEST , 0);
		m_pp_requested = true;
	}
	update_pp_dio();
}

void remote488_device::update_pp_dio()
{
	if (is_local_pp_active()) {
		LOG("PP %02x\n" , m_pp_data);
		m_pp_dio = ~m_pp_data;
	} else {
		m_pp_dio = 0xff;
	}
	update_dio();
}

void remote488_device::update_dio()
{
	m_bus->dio_w(this , m_pp_dio & m_sh_dio);
}
