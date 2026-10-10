// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Front panel shared by the KN6000 and KN7000 families: the sub-CPUs that scan
// the buttons and drive the LEDs, and their serial link to the main CPU. The
// sub-CPUs' programs are not dumped; their behaviour on the link is simulated.

#ifndef MAME_MATSUSHITA_KN_CPANEL_H
#define MAME_MATSUSHITA_KN_CPANEL_H

#pragma once

#include <array>
#include <span>
#include <utility>

class kn_cpanel_base_device : public device_t
{
public:
	// The analog controls live in the driver's input ports
	template <typename T> void set_dial_port(T &&tag) { m_dial.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_volapcseq_port(T &&tag) { m_volapcseq.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_tempoknob_port(T &&tag) { m_tempoknob.set_tag(std::forward<T>(tag)); }

	auto atn() { return m_atn_cb.bind(); }   // ATN line, to an external interrupt pin
	auto rxd() { return m_rxd_cb.bind(); }   // a reply byte for the main CPU's receiver

	void tx_byte(u8 data);                   // a byte from the main CPU
	void rx_enable(int state);               // the main CPU's receiver is enabled

protected:
	static constexpr unsigned MAX_PORTS = 22;
	static constexpr unsigned MAX_SEGS = 0x40;

	// The model's button matrix: its scan ports, the segment each one reports,
	// and the wire address of each segment
	kn_cpanel_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock,
			const std::array<char const *, MAX_PORTS> &scan_tags, std::span<const u8> port_seg, std::span<const u8> seg_wire_addr);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// The model's LEDs: each frame writes eight of them, bit n to LED 8 * reg + n
	virtual void panel_led_frame(u8 addr, u8 data) = 0;
	template <unsigned N> static void set_led_reg(output_finder<N> &leds, unsigned reg, u8 data)
	{
		for (int bit = 0; bit < 8; bit++)
			leds[reg * 8 + bit] = BIT(data, bit);
	}

private:
	optional_ioport_array<MAX_PORTS> m_scan;
	const std::span<const u8> m_port_seg;
	const std::span<const u8> m_seg_wire_addr;
	optional_ioport m_dial;
	optional_ioport m_volapcseq;
	optional_ioport m_tempoknob;
	devcb_write_line m_atn_cb;
	devcb_write8 m_rxd_cb;

	emu_timer *m_atn_timer;                  // param: the ATN level to drive
	emu_timer *m_rx_timer;
	emu_timer *m_panel_timer;

	u8 m_panel_pos;                          // position within the 7-byte frame from the main CPU
	u8 m_panel_p1;
	u8 m_panel_p2;
	u8 m_panel_resp[64];                     // replies and button events for the main CPU
	u8 m_panel_resp_len;
	u8 m_panel_resp_pos;
	u8 m_btn_prev[MAX_SEGS];
	u8 m_vol_apcseq_prev;
	bool m_vol_apcseq_synced;
	u8 m_dial_prev;
	bool m_dial_synced;
	u8 m_tempoknob_prev;
	bool m_tempoknob_synced;

	bool panel_queue(std::span<const u8> bytes);
	TIMER_CALLBACK_MEMBER(atn_event);
	TIMER_CALLBACK_MEMBER(rx_event);
	TIMER_CALLBACK_MEMBER(panel_scan);
};

#endif // MAME_MATSUSHITA_KN_CPANEL_H
