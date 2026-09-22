// license:GPL2+
// copyright-holders:Felipe Sanches

// Shared control panel: scan matrix, LEDs and the serial link.

#include "emu.h"
#include "kn_cpanel.h"

kn_cpanel_base_device::kn_cpanel_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, type, tag, owner, clock),
	m_panel_pos(0),
	m_panel_p1(0),
	m_panel_p2(0),
	m_panel_resp_len(0),
	m_panel_resp_pos(0),
	m_panel_evt(nullptr),
	m_panel_timer(nullptr),
	m_vol_apcseq_prev(0),
	m_vol_apcseq_synced(false),
	m_dial_prev(0),
	m_dial_synced(false),
	m_tempoknob_prev(0),
	m_tempoknob_synced(false),
	m_tempoknob_field(nullptr),
	m_atn_cb(*this),
	m_rxd_cb(*this),
	m_dial(*this, finder_base::DUMMY_TAG),
	m_volapcseq(*this, finder_base::DUMMY_TAG),
	m_tempoknob(*this, finder_base::DUMMY_TAG)
{
	std::fill(std::begin(m_panel_resp), std::end(m_panel_resp), 0);
	std::fill(std::begin(m_btn_prev), std::end(m_btn_prev), 0);
}

void kn_cpanel_base_device::device_start()
{
	m_panel_evt = timer_alloc(FUNC(kn_cpanel_base_device::panel_event), this);
	m_panel_timer = timer_alloc(FUNC(kn_cpanel_base_device::panel_scan), this);

	save_item(NAME(m_panel_pos));
	save_item(NAME(m_panel_p1));
	save_item(NAME(m_panel_p2));
	save_item(NAME(m_panel_resp));
	save_item(NAME(m_panel_resp_len));
	save_item(NAME(m_panel_resp_pos));
	save_item(NAME(m_btn_prev));
	save_item(NAME(m_vol_apcseq_prev));
	save_item(NAME(m_vol_apcseq_synced));
	save_item(NAME(m_dial_prev));
	save_item(NAME(m_dial_synced));
	save_item(NAME(m_tempoknob_prev));
	save_item(NAME(m_tempoknob_synced));
}

void kn_cpanel_base_device::device_reset()
{
	m_panel_pos = 0;
	// Drop any in-flight reply and its deferred delivery -- no stale byte can leak
	// after a soft reset (identical at cold boot, where both are already zero).
	m_panel_resp_len = m_panel_resp_pos = 0;
	m_panel_evt->reset();
	std::fill(std::begin(m_btn_prev), std::end(m_btn_prev), 0);
	m_vol_apcseq_synced = false;   // re-record the pot on the first post-reset scan (no frame)
	m_dial_synced = false;         // re-record the DATA dial on the first post-reset scan (no frame)
	m_tempoknob_synced = false;

	// Periodic button/analog scan at 250 Hz (real panel sub-CPUs continuously
	// monitor their matrices and push change notifications via the ATN line).
	m_panel_timer->adjust(attotime::from_hz(250), 0, attotime::from_hz(250));
}

void kn_cpanel_base_device::tx_byte(uint8_t data)
{
	switch (m_panel_pos)
	{
	case 2: m_panel_p1 = data; break;
	case 4: m_panel_p2 = data; break;
	}
	if (++m_panel_pos >= 7)
	{
		m_panel_pos = 0;
		switch (m_panel_p1)
		{
		case 0x1f: case 0x1d: case 0x1e: case 0x20: case 0xe0: case 0x29: case 0xdd:
		{
			static constexpr uint8_t sync_reply[2] = { 0x18, 0x00 };
			panel_queue(sync_reply, 2);
			break;
		}
		default:
			panel_led_frame(m_panel_p1, m_panel_p2);
			break;
		}
	}
}

// The panel raises ATN, the main CPU's group-0x1A ISR switches the link to RX and
// clocks the bytes in one group-0x10 interrupt at a time.
void kn_cpanel_base_device::panel_queue(const uint8_t *bytes, int n)
{
	if (m_panel_resp_pos == m_panel_resp_len)
		m_panel_resp_pos = m_panel_resp_len = 0;          // queue fully drained: reset
	if (m_panel_resp_len + n > int(sizeof(m_panel_resp)))
		return;                                           // overflow: drop (panel would too)
	const bool was_idle = (m_panel_resp_pos == m_panel_resp_len);
	for (int i = 0; i < n; i++)
		m_panel_resp[m_panel_resp_len++] = bytes[i];
	if (was_idle)
		m_panel_evt->adjust(attotime::from_usec(60), 1);  // ATN edge 1
}

TIMER_CALLBACK_MEMBER(kn_cpanel_base_device::panel_event)
{
	if (param == 1)
		m_atn_cb(1);
	else if (param == 2 && m_panel_resp_pos < m_panel_resp_len)
	{
		m_rxd_cb(m_panel_resp[m_panel_resp_pos++]);
		if (m_panel_resp_pos < m_panel_resp_len)
			m_panel_evt->adjust(attotime::from_usec(120), 2);   // next byte
	}
}

// Main CPU enabled SIO ch0 RX (config bit14, set by the group-0x1A ISR's pass 2):
// the panel now sends its queued reply, one byte per group-0x10 interrupt.
void kn_cpanel_base_device::rx_enable()
{
	if (m_panel_resp_pos < m_panel_resp_len)
		m_panel_evt->adjust(attotime::from_usec(60), 2);
}

// The group-0x1A ISR's pass 1 re-armed EXTMD for the opposite edge (11b -> 10b)
// and expects the panel's second ATN edge to arrive after it returns.
void kn_cpanel_base_device::atn_rearm()
{
	m_panel_evt->adjust(attotime::from_usec(60), 1);
}

TIMER_CALLBACK_MEMBER(kn_cpanel_base_device::panel_scan)
{
	{
		const uint8_t data = uint8_t(255 - (m_volapcseq.read_safe(0) * 255 + 50) / 100);
		if (!m_vol_apcseq_synced)
		{
			m_vol_apcseq_prev = data;
			m_vol_apcseq_synced = true;
		}
		else if (data != m_vol_apcseq_prev)
		{
			m_vol_apcseq_prev = data;
			const uint8_t pkt[2] = { 0xd2, data };
			panel_queue(pkt, 2);
		}
	}

	{
		const uint8_t pos = m_dial.read_safe(0);
		if (!m_dial_synced)
		{
			m_dial_prev = pos;
			m_dial_synced = true;
		}
		else if (pos != m_dial_prev)
		{
			m_dial_prev = pos;
			const uint8_t pkt[2] = { 0x10, pos };
			panel_queue(pkt, 2);
		}
	}

	{
		if (m_tempoknob_field == nullptr && m_tempoknob.found())
			for (ioport_field &f : m_tempoknob->fields())
				if (f.type() == IPT_ADJUSTER) { m_tempoknob_field = &f; break; }
		const uint8_t adj = m_tempoknob_field ? uint8_t(m_tempoknob_field->live().value) : m_tempoknob.read_safe(0);
		if (!m_tempoknob_synced)
		{
			m_tempoknob_prev = adj;
			m_tempoknob_synced = true;
		}
		else if (adj != m_tempoknob_prev)
		{
			// The layout knob is an INFINITE rotary encoder: a full-circle drag wraps the 0..100 adjuster
			// past its ends. Take the direction the SHORT way round (a jump of >50 = a wrap).
			int delta = int(adj) - int(m_tempoknob_prev);
			if (delta > 50) delta -= 101;
			else if (delta < -50) delta += 101;
			const int step = (delta > 0) ? 1 : -1;                          // one detent toward the adjuster
			m_tempoknob_prev = uint8_t((int(m_tempoknob_prev) + step + 101) % 101);
			const uint8_t pkt[2] = { 0x17, uint8_t(int8_t(step)) };         // +/-1 = ~1 BPM/detent; firmware ADDS it
			panel_queue(pkt, 2);
		}
	}

	uint8_t seg_state[MAX_SEGS] = { 0 };
	const int nports = num_scan_ports();
	for (int p = 0; p < nports; p++)
	{
		const uint8_t v = scan_port_read(p);
		if (!v)
			continue;
		seg_state[port_seg(p)] |= v;   // identity: scan column -> normSeg, bits unchanged
	}
	const int nsegs = num_segs();
	for (int seg = 0; seg < nsegs; seg++)
	{
		const uint8_t addr = seg_wire_addr(seg);
		if (addr == 0xff)   // no wire path
			continue;
		if (seg_state[seg] == m_btn_prev[seg])
			continue;
		m_btn_prev[seg] = seg_state[seg];
		const uint8_t pkt[2] = { addr, seg_state[seg] };
		panel_queue(pkt, 2);
	}
}
