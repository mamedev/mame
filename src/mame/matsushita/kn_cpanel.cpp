// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Front panel shared by the KN6000 and KN7000 families: scan matrix, LEDs and
// the serial link.

#include "emu.h"
#include "kn_cpanel.h"

#include <algorithm>
#include <iterator>


kn_cpanel_base_device::kn_cpanel_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
		const std::array<char const *, MAX_PORTS> &scan_tags, std::span<const u8> port_seg, std::span<const u8> seg_wire_addr)
	: device_t(mconfig, type, tag, owner, clock)
	, m_scan(*this, scan_tags)
	, m_port_seg(port_seg)
	, m_seg_wire_addr(seg_wire_addr)
	, m_dial(*this, finder_base::DUMMY_TAG)
	, m_volapcseq(*this, finder_base::DUMMY_TAG)
	, m_tempoknob(*this, finder_base::DUMMY_TAG)
	, m_atn_cb(*this)
	, m_rxd_cb(*this)
	, m_atn_timer(nullptr)
	, m_rx_timer(nullptr)
	, m_panel_timer(nullptr)
	, m_panel_pos(0)
	, m_panel_p1(0)
	, m_panel_p2(0)
	, m_panel_resp{}
	, m_panel_resp_len(0)
	, m_panel_resp_pos(0)
	, m_btn_prev{}
	, m_vol_apcseq_prev(0)
	, m_vol_apcseq_synced(false)
	, m_dial_prev(0)
	, m_dial_synced(false)
	, m_tempoknob_prev(0)
	, m_tempoknob_synced(false)
{
}

void kn_cpanel_base_device::device_start()
{
	m_atn_timer = timer_alloc(FUNC(kn_cpanel_base_device::atn_event), this);
	m_rx_timer = timer_alloc(FUNC(kn_cpanel_base_device::rx_event), this);
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
	m_panel_resp_len = 0;
	m_panel_resp_pos = 0;
	m_atn_timer->adjust(attotime::never);
	m_rx_timer->adjust(attotime::never);
	m_atn_cb(0);
	std::fill(std::begin(m_btn_prev), std::end(m_btn_prev), 0);
	m_vol_apcseq_synced = false;
	m_dial_synced = false;
	m_tempoknob_synced = false;

	// The sub-CPUs scan continuously and report changes
	m_panel_timer->adjust(attotime::from_hz(250), 0, attotime::from_hz(250));
}

// The main CPU sends 7-byte frames with the payload at positions 2 and 4. A
// sync code is answered; anything else is an [ADDR][DATA] LED register write.
void kn_cpanel_base_device::tx_byte(u8 data)
{
	switch (m_panel_pos)
	{
	case 2:
		m_panel_p1 = data;
		break;
	case 4:
		m_panel_p2 = data;
		break;
	}

	if (++m_panel_pos < 7)
		return;

	m_panel_pos = 0;
	switch (m_panel_p1)
	{
	case 0x1d: case 0x1e: case 0x1f: case 0x20: case 0x29: case 0xdd: case 0xe0:
	{
		static constexpr u8 SYNC_REPLY[2] = { 0x18, 0x00 };
		panel_queue(SYNC_REPLY);
		break;
	}
	default:
		panel_led_frame(m_panel_p1, m_panel_p2);
		break;
	}
}

// To send, the panel pulses ATN. The main CPU's interrupt handler runs once for
// each level of the pulse, then enables its receiver and takes the bytes one
// receive interrupt at a time.
bool kn_cpanel_base_device::panel_queue(std::span<const u8> bytes)
{
	if (m_panel_resp_pos == m_panel_resp_len)
		m_panel_resp_pos = m_panel_resp_len = 0;
	if (m_panel_resp_len + bytes.size() > std::size(m_panel_resp))
	{
		logerror("reply queue full, %u bytes dropped\n", bytes.size());
		return false;
	}

	const bool was_idle = m_panel_resp_pos == m_panel_resp_len;
	for (const u8 b : bytes)
		m_panel_resp[m_panel_resp_len++] = b;
	if (was_idle)
		m_atn_timer->adjust(attotime::from_usec(60), 1);
	return true;
}

TIMER_CALLBACK_MEMBER(kn_cpanel_base_device::atn_event)
{
	m_atn_cb(param);
	if (param)
		m_atn_timer->adjust(attotime::from_usec(100), 0);
}

TIMER_CALLBACK_MEMBER(kn_cpanel_base_device::rx_event)
{
	if (m_panel_resp_pos < m_panel_resp_len)
	{
		m_rxd_cb(m_panel_resp[m_panel_resp_pos++]);
		if (m_panel_resp_pos < m_panel_resp_len)
			m_rx_timer->adjust(attotime::from_usec(120));
	}
}

void kn_cpanel_base_device::rx_enable(int state)
{
	if (state && m_panel_resp_pos < m_panel_resp_len)
		m_rx_timer->adjust(attotime::from_usec(60));
}

TIMER_CALLBACK_MEMBER(kn_cpanel_base_device::panel_scan)
{
	// APC/SEQ VOLUME, sent as wire address 0xd2
	const u8 vol = u8(255 - (m_volapcseq.read_safe(0) * 255 + 50) / 100);
	if (!m_vol_apcseq_synced)
	{
		m_vol_apcseq_prev = vol;
		m_vol_apcseq_synced = true;
	}
	else if (vol != m_vol_apcseq_prev)
	{
		const u8 pkt[2] = { 0xd2, vol };
		if (panel_queue(pkt))
			m_vol_apcseq_prev = vol;
	}

	// DATA dial, sent as its position at wire address 0x10
	const u8 pos = m_dial.read_safe(0);
	if (!m_dial_synced)
	{
		m_dial_prev = pos;
		m_dial_synced = true;
	}
	else if (pos != m_dial_prev)
	{
		const u8 pkt[2] = { 0x10, pos };
		if (panel_queue(pkt))
			m_dial_prev = pos;
	}

	// TEMPO/PROGRAM knob, an endless encoder sent as +1 or -1 steps at wire
	// address 0x17. A change of more than half the 0-100 range is taken as a wrap
	// past the ends, so the step goes the short way round.
	const u8 adj = m_tempoknob.read_safe(0);
	if (!m_tempoknob_synced)
	{
		m_tempoknob_prev = adj;
		m_tempoknob_synced = true;
	}
	else if (adj != m_tempoknob_prev)
	{
		int delta = int(adj) - int(m_tempoknob_prev);
		if (delta > 50)
			delta -= 101;
		else if (delta < -50)
			delta += 101;
		const int step = (delta > 0) ? 1 : -1;
		const u8 pkt[2] = { 0x17, u8(step) };
		if (panel_queue(pkt))
			m_tempoknob_prev = u8((int(m_tempoknob_prev) + step + 101) % 101);
	}

	// Buttons: each scan column that changed is sent as [address][switch bits]
	u8 seg_state[MAX_SEGS] = { 0 };
	for (unsigned p = 0; p < m_port_seg.size(); p++)
		seg_state[m_port_seg[p]] |= m_scan[p]->read();
	for (unsigned seg = 0; seg < m_seg_wire_addr.size(); seg++)
	{
		if (seg_state[seg] == m_btn_prev[seg])
			continue;
		const u8 pkt[2] = { m_seg_wire_addr[seg], seg_state[seg] };
		if (panel_queue(pkt))
			m_btn_prev[seg] = seg_state[seg];
	}
}
