// license:GPL2+
// copyright-holders:Felipe Sanches

// Shared control-panel device: the CP serial protocol.

#ifndef MAME_MATSUSHITA_KN_CPANEL_H
#define MAME_MATSUSHITA_KN_CPANEL_H

#pragma once

class kn_cpanel_base_device : public device_t
{
public:
	// The analog controls the panel sub-CPUs scan live in the driver's ioports;
	// the device reads them through these tags.
	template <typename T> void set_dial_port(T &&tag) { m_dial.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_volapcseq_port(T &&tag) { m_volapcseq.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_tempoknob_port(T &&tag) { m_tempoknob.set_tag(std::forward<T>(tag)); }

	// Callbacks to the main CPU (its interrupt controller / SIO channel 0):
	auto atn() { return m_atn_cb.bind(); }   // panel ATN pulse -> main asserts INTC group 0x1A
	auto rxd() { return m_rxd_cb.bind(); }   // one reply byte -> main pushes it onto SIO0 RX + asserts group 0x10

	// From the main CPU (SIO / INTC bridge):
	void tx_byte(uint8_t data);   // main CPU wrote one panel TX byte (SIO ch0 +8)
	void rx_enable();             // main CPU set SIO ch0 RX-enable (config bit14): clock out the next reply byte
	void atn_rearm();             // group-0x1A ISR re-armed EXTMD (11b->10b): deliver ATN edge 2

	virtual bool chorus_led() const { return false; }
	virtual bool multi_led() const { return false; }

protected:
	kn_cpanel_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device_t overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// ---- per-model geometry, supplied by the derived panel device ----
	virtual int     num_scan_ports() const = 0;
	virtual uint8_t scan_port_read(int port) = 0;
	virtual uint8_t port_seg(int port) const = 0;
	// Reverse-normalization: normSeg -> wire ADDR (0xFF = this segment has no wire path).
	virtual int     num_segs() const = 0;
	virtual uint8_t seg_wire_addr(int seg) const = 0;
	// Decode one [ADDR][DATA] LED-register frame onto this model's LED outputs.
	virtual void    panel_led_frame(uint8_t addr, uint8_t data) = 0;

	static constexpr int MAX_SEGS = 0x40;

	TIMER_CALLBACK_MEMBER(panel_event);   // deferred ATN edge / RX-byte delivery
	TIMER_CALLBACK_MEMBER(panel_scan);    // periodic button + analog scan

	// Append panel->main reply bytes and kick the ATN delivery dance if idle.
	void panel_queue(const uint8_t *bytes, int n);

private:
	// Serial frame parse (7-byte TX frames from the main CPU).
	int     m_panel_pos;                   // position within the 7-byte TX frame
	uint8_t m_panel_p1, m_panel_p2;        // frame payload bytes (positions 2 and 4)

	// Response queue (panel -> main).
	uint8_t m_panel_resp[64];              // pending panel->main bytes (replies + button events)
	int     m_panel_resp_len, m_panel_resp_pos;

	// Timers.
	emu_timer *m_panel_evt;                // one-shot; param: 1=ATN edge, 2=deliver RX byte
	emu_timer *m_panel_timer;              // periodic button/analog scan

	// Input scan state.
	uint8_t m_btn_prev[MAX_SEGS];          // last scanned state, one per normSeg
	uint8_t m_vol_apcseq_prev;             // last DATA byte for the APC/SEQ pot
	bool    m_vol_apcseq_synced;           // false until the first scan records the pot (no startup frame)
	uint8_t m_dial_prev;                   // last IPT_DIAL position delivered for the DATA dial (wire 0x10)
	bool    m_dial_synced;                 // false until the first scan records the dial (no startup frame)
	uint8_t m_tempoknob_prev;              // last adjuster value seen (for the signed per-scan delta, wire 0x17)
	bool    m_tempoknob_synced;            // false until the first scan records the knob (no startup frame)
	ioport_field *m_tempoknob_field;       // the TEMPO_KNOB adjuster field (read RAW, bypassing analog interp)

	// Callbacks to the main CPU.
	devcb_write_line m_atn_cb;
	devcb_write8     m_rxd_cb;

	// Shared analog panel controls (set by the main driver via the set_*_port() helpers above).
	optional_ioport m_dial;                // DATA dial (rotary encoder, wire 0x10)
	optional_ioport m_volapcseq;           // APC/SEQ VOLUME slider (wire 0xD2)
	optional_ioport m_tempoknob;           // TEMPO/PROGRAM knob (relative encoder, wire 0x17)
};

#endif // MAME_MATSUSHITA_KN_CPANEL_H
