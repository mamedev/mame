// license:GPL2+
// copyright-holders:Felipe Sanches
/***************************************************************************

	KN5000 control panel HLE

	Emulates the two Mitsubishi M37471M2196S MCUs on the control panel.

	Protocol Summary:
	-----------------
	- Commands are 2-byte sequences from main CPU
	- Command byte encoding: bits 7-5 = panel (001=left, 111=right),
	  bits 4-0 = command type (0=basic query, 2=analog register query,
	  3=extended read, 5=data mode)
	- Response packets have type encoded in bits 5-3:
	    Type 0,1: Button state (bits 7:6 select panel: 00=right, 11=left)
	    Type 2: Encoder (absolute 8-bit ADC value, NOT delta)
	    Type 3-5: Sync/ACK
	    Type 6,7: Multi-byte data

	See: https://felipesanches.github.io/kn5000-docs/control-panel-protocol/

***************************************************************************/

#include "emu.h"
#include "kn5000_cpanel.h"

#define LOG_COMMANDS (1U << 1)
#define LOG_SERIAL   (1U << 2)
#define LOG_BUTTONS  (1U << 3)
#define LOG_LEDS     (1U << 4)

#define VERBOSE 0
#include "logmacro.h"

DEFINE_DEVICE_TYPE(KN5000_CPANEL, kn5000_cpanel_device, "kn5000_cpanel", "KN5000 Control Panel HLE")

kn5000_cpanel_device::kn5000_cpanel_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, KN5000_CPANEL, tag, owner, clock),
	m_timer(nullptr),
	m_idle_detect_timer(nullptr),
	m_self_clock_timer(nullptr),
	m_baud_rate(0),
	m_rx_clock_count(8),
	m_rx_shift_register(0),
	m_rxd(1),
	m_sioclk_state(0),
	m_tx_clock_count(0),
	m_tx_shift_register(0xff),
	m_tx_skip_first_falling(false),
	m_cmd_index(0),
	m_initialized(false),
	m_self_clocking(false),
	m_inta_asserted(false),
	m_accept_next_byte(false),
	m_tx_output_enabled(true),
	m_next_accept(false),
	m_next_tx_output_enabled(true),
	m_rx_waiting_for_start(true),
	m_self_clock_bytes_sent(0),
	m_txd_cb(*this),
	m_sclk_out_cb(*this),
	m_inta_cb(*this),
	m_cpl_leds(*this, "cpl_led_%u", 0U),
	m_cpr_leds(*this, "cpr_led_%u", 0U)
{
	std::fill(std::begin(m_cmd_buffer), std::end(m_cmd_buffer), 0);
	std::fill(std::begin(m_last_button_state), std::end(m_last_button_state), 0);
	std::fill(std::begin(m_pending_button_state), std::end(m_pending_button_state), 0);
	std::fill(std::begin(m_cpl_ports), std::end(m_cpl_ports), nullptr);
	std::fill(std::begin(m_cpr_ports), std::end(m_cpr_ports), nullptr);
}

void kn5000_cpanel_device::device_start()
{
	m_timer = timer_alloc(FUNC(kn5000_cpanel_device::timer_callback), this);
	m_idle_detect_timer = timer_alloc(FUNC(kn5000_cpanel_device::idle_detect_callback), this);
	m_self_clock_timer = timer_alloc(FUNC(kn5000_cpanel_device::self_clock_callback), this);
	m_button_scan_timer = timer_alloc(FUNC(kn5000_cpanel_device::button_scan_callback), this);

	// Resolve LED outputs
	m_cpl_leds.resolve();
	m_cpr_leds.resolve();

	// Save state
	save_item(NAME(m_baud_rate));
	save_item(NAME(m_rx_clock_count));
	save_item(NAME(m_rx_shift_register));
	save_item(NAME(m_rxd));
	save_item(NAME(m_sioclk_state));
	save_item(NAME(m_tx_clock_count));
	save_item(NAME(m_tx_shift_register));
	save_item(NAME(m_tx_skip_first_falling));
	save_item(NAME(m_cmd_buffer));
	save_item(NAME(m_cmd_index));
	save_item(NAME(m_initialized));
	save_item(NAME(m_self_clocking));
	save_item(NAME(m_inta_asserted));
	save_item(NAME(m_accept_next_byte));
	save_item(NAME(m_tx_output_enabled));
	save_item(NAME(m_next_accept));
	save_item(NAME(m_next_tx_output_enabled));
	save_item(NAME(m_rx_waiting_for_start));
	save_item(NAME(m_self_clock_bytes_sent));
	save_item(NAME(m_last_button_state));
	save_item(NAME(m_pending_button_state));

	// Initial state - line idle high
	m_txd_cb(1);
}

void kn5000_cpanel_device::device_reset()
{
	m_baud_rate = 0;
	m_rx_clock_count = 8;
	m_tx_clock_count = 0;
	m_tx_skip_first_falling = false;
	m_cmd_index = 0;
	m_initialized = false;
	m_self_clocking = false;
	m_inta_asserted = false;
	m_accept_next_byte = false;
	m_tx_output_enabled = true;
	m_next_accept = false;
	m_next_tx_output_enabled = true;
	m_rx_waiting_for_start = true;
	m_self_clock_bytes_sent = 0;

	// Clear TX queue
	while (!m_tx_queue.empty())
		m_tx_queue.pop();

	m_idle_detect_timer->reset(attotime::never);
	m_self_clock_timer->reset(attotime::never);

	// Proactive button scan: real panel MCUs continuously monitor their
	// button matrices and push change notifications via INTA.  The
	// firmware's steady-state polling only queries one segment (E0 13 =
	// right panel segment 3), relying on MCU-initiated INTA for changes
	// on other segments.  7ms (~143 Hz) is responsive enough for UI use
	// and coprime with likely command cycle times to avoid phase-lock.
	m_button_scan_timer->adjust(attotime::from_msec(7), 0, attotime::from_msec(7));

	std::fill(std::begin(m_last_button_state), std::end(m_last_button_state), 0);
	std::fill(std::begin(m_pending_button_state), std::end(m_pending_button_state), 0);
}

void kn5000_cpanel_device::set_baudrate(uint16_t br)
{
	m_baud_rate = br;
	LOGMASKED(LOG_SERIAL, "Baud rate set to %d\n", br);

	if (br)
	{
		m_timer->adjust(attotime::from_hz(m_baud_rate), 0, attotime::from_hz(m_baud_rate));
	}
	else
	{
		m_timer->reset(attotime::never);
	}
}

void kn5000_cpanel_device::rxd(int state)
{
	m_rxd = state;
}

void kn5000_cpanel_device::tx_start(int state)
{
	// Called when CPU starts transmitting a new byte.
	// state=1: real byte (PFFC enabled, SCLK pin driven)
	// state=0: phantom byte (PFFC disabled, pin high-Z)
	LOGMASKED(LOG_SERIAL, "tx_start: state=%d (%s byte) rx_count=%d\n",
		state, state ? "real" : "phantom", m_rx_clock_count);

	// Defer the new accept/output state to the next byte boundary.
	// INTTX1 fires for byte N+1 before rising edge 8 completes byte N's
	// reception, so applying immediately would use the wrong accept state
	// for the byte currently being received.
	m_next_accept = (state != 0);
	m_next_tx_output_enabled = (state != 0);

	// Allow RX counting to begin.  Between bytes, the baud rate timer may
	// drive extra edges for internal RX completion; m_rx_waiting_for_start
	// prevents those from being counted as data bits.
	m_rx_waiting_for_start = false;

	if (m_rx_clock_count == 8)
	{
		// At byte boundary — safe to apply immediately
		m_accept_next_byte = m_next_accept;
		m_tx_output_enabled = m_next_tx_output_enabled;
	}

	// idle_detect is not cancelled here.  The sliding window in sioclk()
	// re-arms on every edge and fires after the LAST edge, which correctly
	// handles both the firmware's chained TX state machine (phantom bytes
	// after real commands) and polled serial modes (dummy bytes drain the
	// TX queue before idle_detect fires).
}

void kn5000_cpanel_device::sioclk(int state)
{
	if (m_sioclk_state == state)
		return;

	m_sioclk_state = state;

	// Sliding idle_detect window: retrigger on every edge while response
	// data is pending.  Fires 50µs after the last external clock edge,
	// which is after the firmware's phantom bytes complete but before
	// its WaitTXReady delay (~375µs) checks the INTA line.
	if (!m_self_clocking && (m_tx_clock_count > 0 || !m_tx_queue.empty()))
	{
		m_idle_detect_timer->adjust(attotime::from_usec(50));
	}

	LOGMASKED(LOG_SERIAL, "sioclk state=%d rxd=%d rx_count=%d tx_count=%d\n",
		state, m_rxd, m_rx_clock_count, m_tx_clock_count);

	if (state)
	{
		// Rising edge: sample RXD bit from CPU.
		// Skip during self-clocking (CPU's TXD holds stale data) and
		// while waiting for tx_start (baud rate timer drives extra edges
		// between bytes that would desync byte boundaries).
		if (!m_self_clocking && !m_rx_waiting_for_start && m_rx_clock_count > 0)
		{
			m_rx_shift_register >>= 1;
			m_rx_shift_register |= (m_rxd << 7);
			m_rx_clock_count--;

			LOGMASKED(LOG_SERIAL, "RX bit: %d, shift_reg=%02X, count=%d\n",
				m_rxd, m_rx_shift_register, m_rx_clock_count);

			if (m_rx_clock_count == 0)
			{
				// Full byte received
				m_rx_clock_count = 8;
				process_received_byte(m_rx_shift_register);

				// Apply deferred tx_start flags at byte boundary
				m_accept_next_byte = m_next_accept;
				m_tx_output_enabled = m_next_tx_output_enabled;

				// Wait for next tx_start before counting another byte.
				// This prevents orphan edges from starting a new byte.
				m_rx_waiting_for_start = true;
			}
		}
	}
	else
	{
		// Falling edge: output TXD bit for CPU to sample on next rising edge.
		// Suppress output during phantom byte edges (PFFC off) to keep
		// response data intact for later INTA-driven delivery.
		if (!m_tx_output_enabled)
		{
			// Phantom byte edge — hold response data
		}
		else if (m_tx_skip_first_falling)
		{
			// Skip this falling edge - bit 0 was pre-output and we need to give
			// CPU a rising edge to sample it before we output bit 1
			LOGMASKED(LOG_SERIAL, "skipping first falling edge (bit 0 already on line)\n");
			m_tx_skip_first_falling = false;
		}
		else if (m_tx_clock_count > 0)
		{
			if (m_tx_clock_count == 8)
			{
				// First bit of a chained byte (loaded from queue) — output bit 0 without shifting
				LOGMASKED(LOG_SERIAL, "TX bit 0 (chained): %d, shift_reg=%02X\n",
					m_tx_shift_register & 1, m_tx_shift_register);
				m_txd_cb(m_tx_shift_register & 1);
				m_tx_clock_count--;
			}
			else
			{
				// Normal operation: shift out the next bit
				m_tx_shift_register >>= 1;
				LOGMASKED(LOG_SERIAL, "TX bit: %d, shift_reg=%02X, count=%d\n",
					m_tx_shift_register & 1, m_tx_shift_register, m_tx_clock_count);
				m_txd_cb(m_tx_shift_register & 1);
				m_tx_clock_count--;
			}

			if (m_tx_clock_count == 0)
			{
				// Track bytes sent during self-clocking for packet-pause.
				// Must count HERE (tx_clock_count==0, before queue refill
				// sets it to 8) — self_clock_callback's rising-edge check
				// would never see 0 for intermediate bytes.
				if (m_self_clocking)
					m_self_clock_bytes_sent++;

				// Byte sent, check for more
				if (!m_tx_queue.empty())
				{
					m_tx_shift_register = m_tx_queue.front();
					m_tx_queue.pop();
					m_tx_clock_count = 8;  // Full 8 bits — don't pre-output yet

					LOGMASKED(LOG_SERIAL, "TX next byte queued: %02X (no pre-output)\n",
						m_tx_shift_register);

					// Don't pre-output: bit 7 of previous byte is still on the line
					// and needs to be sampled by CPU on the next rising edge.
					// Bit 0 of new byte will be output on the next falling edge.
				}
				else
				{
					// Transmission complete — leave the last bit on the line.
					// Do NOT call m_txd_cb(1) here: that would overwrite bit 7
					// of the last byte before the CPU samples it on the next
					// rising edge. The line will be updated when send_byte()
					// pre-outputs the next byte's bit 0.
					LOGMASKED(LOG_SERIAL, "TX done, holding last bit\n");
				}
			}
		}
	}
}

void kn5000_cpanel_device::send_byte(uint8_t data)
{
	LOGMASKED(LOG_SERIAL, "send_byte(%02X) tx_count=%d queue_size=%zu sioclk_state=%d\n",
		data, m_tx_clock_count, m_tx_queue.size(), m_sioclk_state);

	if (m_tx_clock_count == 0 && !m_tx_skip_first_falling)
	{
		// Start sending immediately
		m_tx_shift_register = data;
		m_tx_clock_count = 7;  // 7 more bits to send after pre-outputting bit 0

		// Pre-output first bit immediately so CPU can sample it on the first rising edge
		m_txd_cb(m_tx_shift_register & 1);
		LOGMASKED(LOG_SERIAL, "TX start: byte=%02X, pre-output bit=%d\n",
			data, data & 1);

		// Only skip the first falling edge if clock is currently HIGH.
		// If clock is HIGH: next edge = falling (skip it)
		// If clock is LOW: next edge = rising (CPU samples), then falling outputs bit 1 (no skip)
		m_tx_skip_first_falling = (m_sioclk_state == 1);
		LOGMASKED(LOG_SERIAL, "TX skip_first_falling=%d\n", m_tx_skip_first_falling);
	}
	else
	{
		// Queue for later
		m_tx_queue.push(data);
		LOGMASKED(LOG_SERIAL, "TX queued: byte=%02X\n", data);
	}
}

void kn5000_cpanel_device::process_received_byte(uint8_t data)
{
	LOGMASKED(LOG_SERIAL, "RX byte: %02X (cmd_index=%d, accept=%d)\n",
		data, m_cmd_index, m_accept_next_byte);

	// Reject phantom bytes (PFFC-off phases in firmware TX state machine).
	// The accept flag is managed by tx_start's deferred mechanism, not
	// consumed here.
	if (!m_accept_next_byte)
	{
		LOGMASKED(LOG_SERIAL, "skipping unsolicited/phantom byte %02X\n", data);
		return;
	}

	// Ignore 0xFF when it appears as a command byte (first byte of pair).
	// In synchronous serial mode, the CPU must send dummy bytes to clock in
	// response data. These dummy 0xFF bytes are not valid commands — without
	// this filter they would be paired with the next real command byte,
	// misaligning the 2-byte command parser and potentially triggering
	// unintended LED commands or sync responses.
	if (m_cmd_index == 0 && data == 0xFF)
	{
		LOGMASKED(LOG_SERIAL, "ignoring dummy byte 0xFF as command\n");
		return;
	}

	m_cmd_buffer[m_cmd_index++] = data;

	if (m_cmd_index >= 2)
	{
		// Full 2-byte command received
		process_command();
		m_cmd_index = 0;
	}
}

void kn5000_cpanel_device::process_command()
{
	uint8_t cmd = m_cmd_buffer[0];
	uint8_t param = m_cmd_buffer[1];

	LOGMASKED(LOG_COMMANDS, "Command: %02X %02X\n", cmd, param);

	switch (cmd)
	{
	// Initialization commands — respond with sync.
	// These are sent one at a time during boot (not in rapid batches
	// like LED commands), so response accumulation isn't an issue.
	// The firmware checks for the sync response; without it, the
	// "ERROR in CPU data transmission" dialog appears.
	case 0x1f:  // Init sequence (left)
	case 0x1d:
	case 0x1e:
	case 0xdd:  // Setup mode
		LOGMASKED(LOG_COMMANDS, "Init command\n");
		send_sync_packet();
		m_initialized = true;
		break;

	// Query commands
	// The param byte encodes a segment index in bits 3-0.  Bit 4 is a mode
	// flag used by the panel MCUs (e.g., 0x10 = segment 0 scan mode, 0x13 =
	// segment 3 scan mode).  The HLE extracts the segment via param & 0x0F.
	// Param 0x00 (no flag, segment 0) is a sync/ping — everything else with
	// a valid segment (0-11) returns button data.
	// Query commands — command type in bits 4-0:
	//   0x20 (type 0): Basic left panel query (ping, poll segment, status)
	//   0x25 (type 5): Left panel data mode (bulk button state request,
	//                  used only in CPanel_ReadAllButtons during boot)
	case 0x20:
	case 0x25:
	{
		int segment = param & 0x0f;
		if (param == 0x00)
		{
			send_sync_packet();
		}
		else if (segment <= 0x0b)
		{
			// Segment 0x0B is a hardware status register — return
			// WITHOUT panel flag so firmware stores at offset 11.
			send_button_packet(segment, (segment <= 0x0a));
		}
		else
		{
			send_sync_packet();
		}
		break;
	}

	//   0xE0 (type 0): Basic right panel query (ping, steady-state poll)
	case 0xe0:
	{
		int segment = param & 0x0f;
		if (param == 0x00)
		{
			send_sync_packet();
		}
		else if (segment <= 0x0b)
		{
			send_button_packet(segment, false);  // right panel
		}
		else
		{
			send_sync_packet();
		}
		break;
	}

	//   0xE2 (type 2): Right panel analog register query — requests
	//                  encoder/analog controller values.  The real MCU
	//                  responds with Type 2 encoder packets (absolute
	//                  ADC values for modwheel, volume, etc.).
	//                  Used in CPanel_ReadAllButtons with params 0x04, 0x11.
	// TODO: When analog controller inputs are implemented, respond with
	// Type 2 encoder packets here instead of sync.
	case 0xe2:
		send_sync_packet();
		break;

	//   0xE3 (type 3): Right panel extended read — requests button state
	//                  with additional status.  Used in CPanel_InitButtonState
	//                  with param 0x10.
	case 0xe3:
	{
		int segment = param & 0x0f;
		if (segment <= 0x0b)
		{
			send_button_packet(segment, false);  // right panel
		}
		else
		{
			send_sync_packet();
		}
		break;
	}

	case 0x2b:  // Init button state array (left)
		send_all_button_states(true);
		break;

	case 0xeb:  // Init button state array (right)
		send_all_button_states(false);
		break;

	// LED control commands - right panel (no response — real MCUs process silently)
	case 0x00:
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x08:
	case 0x0a:
	case 0x0b:
	case 0x0c:
		process_led_command(cmd, param);
		break;

	// LED control commands - left panel (no response)
	case 0xc0:
	case 0xc1:
	case 0xc2:
	case 0xc3:
	case 0xc4:
	case 0xc8:
		process_led_command(cmd, param);
		break;

	default:
		// Unknown command — do not respond.  Real panel MCUs process
		// many commands (LCD, encoders, etc.) silently.
		LOGMASKED(LOG_COMMANDS, "Unknown command %02X %02X (no response)\n", cmd, param);
		break;
	}

	// Arm idle detection for INTA-based response delivery.  The sliding
	// window in sioclk() retriggers on every edge and fires 50µs after
	// the last edge, handling both firmware phantom bytes and polled modes.
	if (!m_self_clocking && (m_tx_clock_count > 0 || !m_tx_queue.empty()))
	{
		m_idle_detect_timer->adjust(attotime::from_usec(50));
	}
}

void kn5000_cpanel_device::send_sync_packet()
{
	// Type 3 sync packet: bits 5-3 = 011 = 0x18
	send_byte(0x18);
	send_byte(0x00);
}

uint8_t kn5000_cpanel_device::read_button_segment(int segment, bool is_left_panel)
{
	if (segment < 0 || segment > 10)
		return 0;

	ioport_port *port = is_left_panel ? m_cpl_ports[segment] : m_cpr_ports[segment];
	if (port)
	{
		return port->read() & 0xff;
	}
	return 0;
}

void kn5000_cpanel_device::send_button_packet(int segment, bool is_left_panel)
{
	// Button packet header: bits 7:6 = panel (00=right, 11=left),
	// bits 5:3 = type (0/1 for buttons), bits 3:0 = segment index.
	// The firmware dispatches via a ROM lookup table that only maps
	// bits 7:6=00 and bits 7:6=11 to valid event indices.

	uint8_t state = read_button_segment(segment, is_left_panel);

	uint8_t header = (segment & 0x0f);
	if (is_left_panel)
		header |= 0xC0;  // Left panel: bits 7:6 = 11

	send_byte(header);
	send_byte(state);

	LOGMASKED(LOG_BUTTONS, "Button packet: seg=%d left=%d state=%02X\n",
		segment, is_left_panel, state);

	// Track state for change detection
	int state_idx = is_left_panel ? (segment + 11) : segment;
	m_last_button_state[state_idx] = state;
	m_pending_button_state[state_idx] = state;
}

void kn5000_cpanel_device::send_all_button_states(bool is_left_panel)
{
	// Send all 11 segments for the requested panel
	for (int seg = 0; seg <= 10; seg++)
	{
		send_button_packet(seg, is_left_panel);
	}
}


void kn5000_cpanel_device::process_led_command(uint8_t row, uint8_t data)
{
	LOGMASKED(LOG_LEDS, "LED command: row=%02X data=%02X\n", row, data);

	switch (row)
	{
	// Right panel LED rows
	case 0x00:
		m_cpr_leds[1] = BIT(data, 0);  // D101 - EFFECT: SUSTAIN
		m_cpr_leds[2] = BIT(data, 1);  // D102 - EFFECT: DIGITAL EFFECT
		m_cpr_leds[3] = BIT(data, 2);  // D103 - EFFECT: DSP EFFECT
		m_cpr_leds[4] = BIT(data, 3);  // D104 - EFFECT: DIGITAL REVERB
		m_cpr_leds[5] = BIT(data, 4);  // D105 - EFFECT: ACOUSTIC ILLUSION
		m_cpr_leds[6] = BIT(data, 5);  // D106 - SEQUENCER: PLAY
		m_cpr_leds[7] = BIT(data, 6);  // D107 - SEQUENCER: EASY REC
		m_cpr_leds[8] = BIT(data, 7);  // D108 - SEQUENCER: MENU
		break;

	case 0x01:
		m_cpr_leds[9] = BIT(data, 0);   // D109 - PIANO
		m_cpr_leds[10] = BIT(data, 1);  // D110 - GUITAR
		m_cpr_leds[11] = BIT(data, 2);  // D111 - STRINGS & VOCAL
		m_cpr_leds[12] = BIT(data, 3);  // D112 - BRASS
		m_cpr_leds[13] = BIT(data, 4);  // D113 - FLUTE
		m_cpr_leds[14] = BIT(data, 5);  // D114 - SAX & REED
		m_cpr_leds[15] = BIT(data, 6);  // D115 - MALLET & ORCH PERC
		m_cpr_leds[16] = BIT(data, 7);  // D116 - WORLD PERC
		break;

	case 0x02:
		m_cpr_leds[17] = BIT(data, 0);  // D117 - ORGAN & ACCORDION
		m_cpr_leds[18] = BIT(data, 1);  // D118 - ORCHESTRAL PAD
		m_cpr_leds[19] = BIT(data, 2);  // D119 - SYNTH
		m_cpr_leds[20] = BIT(data, 3);  // D120 - BASS
		m_cpr_leds[21] = BIT(data, 4);  // D121 - DIGITAL DRAWBAR
		m_cpr_leds[22] = BIT(data, 5);  // D122 - ACCORDION REGISTER
		m_cpr_leds[23] = BIT(data, 6);  // D123 - GM SPECIAL
		m_cpr_leds[24] = BIT(data, 7);  // D124 - DRUM KITS
		break;

	case 0x03:
		m_cpr_leds[25] = BIT(data, 0);  // D125 - PANEL MEMORY 1
		m_cpr_leds[26] = BIT(data, 1);  // D126 - PANEL MEMORY 2
		m_cpr_leds[27] = BIT(data, 2);  // D127 - PANEL MEMORY 3
		m_cpr_leds[28] = BIT(data, 3);  // D128 - PANEL MEMORY 4
		m_cpr_leds[29] = BIT(data, 4);  // D129 - PANEL MEMORY 5
		m_cpr_leds[30] = BIT(data, 5);  // D130 - PANEL MEMORY 6
		m_cpr_leds[31] = BIT(data, 6);  // D131 - PANEL MEMORY 7
		m_cpr_leds[32] = BIT(data, 7);  // D132 - PANEL MEMORY 8
		break;

	case 0x04:
		m_cpr_leds[33] = BIT(data, 0);  // D133 - PART SELECT: LEFT
		m_cpr_leds[34] = BIT(data, 1);  // D134 - PART SELECT: RIGHT 2
		m_cpr_leds[35] = BIT(data, 2);  // D135 - PART SELECT: RIGHT 1
		m_cpr_leds[36] = BIT(data, 3);  // D136 - ENTERTAINER
		m_cpr_leds[37] = BIT(data, 4);  // D137 - CONDUCTOR: LEFT
		m_cpr_leds[38] = BIT(data, 5);  // D138 - CONDUCTOR: RIGHT 2
		m_cpr_leds[39] = BIT(data, 6);  // D139 - CONDUCTOR: RIGHT 1
		m_cpr_leds[40] = BIT(data, 7);  // D140 - TECHNI CHORD
		break;

	case 0x08:
		m_cpr_leds[49] = BIT(data, 0);  // D149 - MENU: SOUND
		m_cpr_leds[50] = BIT(data, 1);  // D150 - MENU: CONTROL
		m_cpr_leds[51] = BIT(data, 2);  // D151 - MENU: MIDI
		m_cpr_leds[52] = BIT(data, 3);  // D152 - MENU: DISK
		break;

	case 0x0a:
		m_cpr_leds[57] = BIT(data, 0);  // D157 - MEMORY A
		m_cpr_leds[58] = BIT(data, 1);  // D158 - MEMORY B
		break;

	case 0x0b:
		m_cpr_leds[61] = BIT(data, 0);  // D161 - SYNCHRO & BREAK
		m_cpr_leds[62] = BIT(data, 1);  // D162 - R1/R2 OCTAVE MINUS
		m_cpr_leds[63] = BIT(data, 2);  // D163 - R1/R2 OCTAVE PLUS
		m_cpr_leds[64] = BIT(data, 3);  // D164 - BANK VIEW
		break;

	case 0x0c:
		m_cpr_leds[65] = BIT(data, 0);  // D165 - START/STOP 1 BEAT
		m_cpr_leds[66] = BIT(data, 1);  // D166 - START/STOP 2 BEAT
		m_cpr_leds[67] = BIT(data, 2);  // D167 - START/STOP 3 BEAT
		m_cpr_leds[68] = BIT(data, 3);  // D168 - START/STOP 4 BEAT
		break;

	// Left panel LED rows
	case 0xc0:
		m_cpl_leds[1] = BIT(data, 0);  // D101 - COMPOSER: MEMORY
		m_cpl_leds[2] = BIT(data, 1);  // D102 - COMPOSER: MENU
		m_cpl_leds[3] = BIT(data, 2);  // D103 - SOUND ARRANGER: SET
		m_cpl_leds[4] = BIT(data, 3);  // D104 - SOUND ARRANGER: ON/OFF
		m_cpl_leds[5] = BIT(data, 4);  // D105 - MUSIC STYLIST
		m_cpl_leds[6] = BIT(data, 5);  // D106 - FADE IN
		m_cpl_leds[7] = BIT(data, 6);  // D107 - FADE OUT
		m_cpl_leds[8] = BIT(data, 7);  // D108 - DISPLAY HOLD
		break;

	case 0xc1:
		m_cpl_leds[9] = BIT(data, 0);   // D109 - U.S. TRAD
		m_cpl_leds[10] = BIT(data, 1);  // D110 - COUNTRY
		m_cpl_leds[11] = BIT(data, 2);  // D111 - LATIN
		m_cpl_leds[12] = BIT(data, 3);  // D112 - MARCH & WALTZ
		m_cpl_leds[13] = BIT(data, 4);  // D113 - PARTY TIME
		m_cpl_leds[14] = BIT(data, 5);  // D114 - SHOW TIME & TRAD DANCE
		m_cpl_leds[15] = BIT(data, 6);  // D115 - WORLD
		m_cpl_leds[16] = BIT(data, 7);  // D116 - CUSTOM
		break;

	case 0xc2:
		m_cpl_leds[17] = BIT(data, 0);  // D117 - STANDARD ROCK
		m_cpl_leds[18] = BIT(data, 1);  // D118 - R & ROLL & BLUES
		m_cpl_leds[19] = BIT(data, 2);  // D119 - POP & BALLAD
		m_cpl_leds[20] = BIT(data, 3);  // D120 - FUNK & FUSION
		m_cpl_leds[21] = BIT(data, 4);  // D121 - SOUL & MODERN DANCE
		m_cpl_leds[22] = BIT(data, 5);  // D122 - BIG BAND & SWING
		m_cpl_leds[23] = BIT(data, 6);  // D123 - JAZZ COMBO
		m_cpl_leds[24] = BIT(data, 7);  // D124 - MANUAL SEQUENCE PADS: MENU
		break;

	case 0xc3:
		m_cpl_leds[25] = BIT(data, 0);  // D125 - VARIATION & MSA 1
		m_cpl_leds[26] = BIT(data, 1);  // D126 - VARIATION & MSA 2
		m_cpl_leds[27] = BIT(data, 2);  // D127 - VARIATION & MSA 3
		m_cpl_leds[28] = BIT(data, 3);  // D128 - VARIATION & MSA 4
		m_cpl_leds[29] = BIT(data, 4);  // D129 - MUSIC STYLE ARRANGER
		m_cpl_leds[30] = BIT(data, 5);  // D130 - AUTO PLAY CHORD
		break;

	case 0xc4:
		m_cpl_leds[33] = BIT(data, 0);  // D133 - FILL IN 1
		m_cpl_leds[34] = BIT(data, 1);  // D134 - FILL IN 2
		m_cpl_leds[35] = BIT(data, 2);  // D135 - INTRO & ENDING 1
		m_cpl_leds[36] = BIT(data, 3);  // D136 - INTRO & ENDING 2
		m_cpl_leds[37] = BIT(data, 4);  // D137 - SPLIT POINT INDICATOR (LEFT)
		m_cpl_leds[38] = BIT(data, 5);  // D138 - SPLIT POINT INDICATOR (CENTER)
		m_cpl_leds[39] = BIT(data, 6);  // D139 - SPLIT POINT INDICATOR (RIGHT)
		m_cpl_leds[40] = BIT(data, 7);  // D140 - TEMPO/PROGRAM
		break;

	case 0xc8:
		m_cpl_leds[49] = BIT(data, 0);  // D149 - OTHER PARTS/TR
		break;
	}
}

TIMER_CALLBACK_MEMBER(kn5000_cpanel_device::timer_callback)
{
	// Timer drives the serial clock when we have data to send
	if (m_tx_clock_count > 0 || !m_tx_queue.empty())
	{
		// Toggle clock to shift out data
		m_sclk_out_cb(1);
		m_sclk_out_cb(0);
	}
}

TIMER_CALLBACK_MEMBER(kn5000_cpanel_device::idle_detect_callback)
{
	// External SCLK has been idle — assert INTA and self-clock the response.
	// On real hardware, the panel MCU drives SCLK after detecting idle.

	if (m_tx_clock_count > 0 || !m_tx_queue.empty())
	{
		// Enable TX output — response data was frozen during phantom bytes
		m_tx_output_enabled = true;

		if (m_inta_asserted)
		{
			// Multi-packet: pulse INTA to re-trigger the interrupt
			LOGMASKED(LOG_SERIAL, "re-triggering INTA for next packet (%zu bytes queued)\n",
				m_tx_queue.size());
			m_inta_cb(0);
			m_inta_cb(1);
		}
		else
		{
			LOGMASKED(LOG_SERIAL, "SCLK idle, asserting INTA\n");
			m_inta_asserted = true;
			m_inta_cb(1);
		}

		// Reset byte counter for this INTA cycle
		m_self_clock_bytes_sent = 0;

		// Brief delay lets the CPU's INTA ISR enable receive mode
		m_self_clocking = true;
		m_self_clock_timer->adjust(attotime::from_usec(20), 0, attotime::from_hz(250000));
	}
}

TIMER_CALLBACK_MEMBER(kn5000_cpanel_device::self_clock_callback)
{
	// Toggle SCLK to shift out response data
	int new_state = m_sioclk_state ^ 1;
	m_sclk_out_cb(new_state);

	// Check completion after rising edges (CPU samples last bit)
	if (new_state == 1)
	{
		if (m_tx_queue.empty() && m_tx_clock_count == 0)
		{
			// All response data sent — stop self-clocking and deassert INTA.
			LOGMASKED(LOG_SERIAL, "self-clock TX complete (%d bytes), deasserting INTA\n",
				m_self_clock_bytes_sent);
			m_self_clocking = false;
			m_self_clock_timer->reset(attotime::never);

			if (m_inta_asserted)
			{
				m_inta_asserted = false;
				m_inta_cb(0);
			}

			// Button scan timer runs periodically (set in device_reset),
			// no need to reschedule from here.
		}
		else if (m_self_clock_bytes_sent >= 2)
		{
			// Pause after each 2-byte packet — firmware processes one
			// packet per INTA cycle.  Keep INTA asserted to block
			// WaitTXReady while more data is queued.
			LOGMASKED(LOG_SERIAL, "self-clock pausing after 2-byte packet, %zu bytes queued\n",
				m_tx_queue.size());
			m_self_clocking = false;
			m_self_clock_timer->reset(attotime::never);

			// Schedule next packet after 20µs (enough for INTA ISR)
			m_idle_detect_timer->adjust(attotime::from_usec(20));
		}
	}
}

TIMER_CALLBACK_MEMBER(kn5000_cpanel_device::button_scan_callback)
{
	// Periodic button matrix scan (~143 Hz).  Real panel MCUs continuously
	// scan and push change notifications via INTA, independent of polling.
	// Per-segment confirmation: changes must be stable for 2 consecutive
	// scans (14ms) before being reported, filtering transient glitches.

	if (!m_initialized)
		return;

	// Don't queue changes while a serial transaction is in progress
	if (m_self_clocking || m_inta_asserted)
		return;
	if (!m_rx_waiting_for_start)
		return;

	bool changed = false;

	// Scan right panel segments 0-10
	for (int seg = 0; seg <= 10; seg++)
	{
		if (!m_cpr_ports[seg])
			continue;
		uint8_t state = m_cpr_ports[seg]->read() & 0xff;
		if (state != m_last_button_state[seg])
		{
			// State differs from last confirmed — check if pending agrees
			if (state == m_pending_button_state[seg])
			{
				// Stable for 2 scans: confirmed change
				LOGMASKED(LOG_BUTTONS, "confirmed right seg %d change (%02X->%02X)\n",
					seg, m_last_button_state[seg], state);
				send_button_packet(seg, false);
				changed = true;
			}
			else
			{
				// First observation — record as pending, wait for confirmation
				LOGMASKED(LOG_BUTTONS, "pending right seg %d change (%02X->%02X)\n",
					seg, m_last_button_state[seg], state);
				m_pending_button_state[seg] = state;
			}
		}
		else
		{
			// Matches confirmed state — reset pending
			m_pending_button_state[seg] = state;
		}
	}

	// Scan left panel segments 0-10
	for (int seg = 0; seg <= 10; seg++)
	{
		if (!m_cpl_ports[seg])
			continue;
		int idx = seg + 11;
		uint8_t state = m_cpl_ports[seg]->read() & 0xff;
		if (state != m_last_button_state[idx])
		{
			if (state == m_pending_button_state[idx])
			{
				LOGMASKED(LOG_BUTTONS, "confirmed left seg %d change (%02X->%02X)\n",
					seg, m_last_button_state[idx], state);
				send_button_packet(seg, true);
				changed = true;
			}
			else
			{
				LOGMASKED(LOG_BUTTONS, "pending left seg %d change (%02X->%02X)\n",
					seg, m_last_button_state[idx], state);
				m_pending_button_state[idx] = state;
			}
		}
		else
		{
			m_pending_button_state[idx] = state;
		}
	}

	if (changed)
	{
		LOGMASKED(LOG_BUTTONS, "confirmed button change, triggering INTA delivery\n");
		m_idle_detect_timer->adjust(attotime::from_usec(50));
	}
}
