// license:BSD-3-Clause
// copyright-holders:superctr
/****************************************************************************

    Roland SC-88 sub CPU (M38881M2, undumped): high level emulation of its
    system bus interface and of the MIDI protocol it speaks with the main
    CPU.

****************************************************************************/

#ifndef MAME_ROLAND_SC88_SUB_H
#define MAME_ROLAND_SC88_SUB_H

#pragma once

#include "diserial.h"

#include <deque>


// UART receiver, one per MIDI source
class sc88_sub_rx_device : public device_t, public device_serial_interface
{
public:
	sc88_sub_rx_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	auto byte_callback() { return m_byte_cb.bind(); }
	using device_serial_interface::rx_w;
protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void rcv_complete() override;
private:
	devcb_write8 m_byte_cb;
};

DECLARE_DEVICE_TYPE(SC88_SUB_RX, sc88_sub_rx_device)


class sc88_sub_device : public device_t
{
public:
	sc88_sub_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// INTR pin: raised when a message is waiting in the IPC error registers
	auto int_callback() { return m_int_cb.bind(); }
	// switch matrix rows selected by the strobes on PB0-PB3
	template <unsigned N> auto keys_callback() { return m_keys_cb[N].bind(); }
	// serial data for the MIDI OUT connector
	auto tx_callback() { return m_tx_cb.bind(); }

	// MIDI IN A (rear) and B (front), and the RS-422/RS-232 computer port
	template <unsigned N> void rxd_w(int state) { m_rx[N]->rx_w(state); }

	// /RST, driven from the main CPU's P4-0 on the SC-88VL
	void reset_w(int state);

	// system bus window (A0-A7)
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	struct message
	{
		u8 code, flags, d1, d2;
		std::vector<u8> block;
	};

	struct source
	{
		u8 status = 0;
		u8 data[2]{};
		u8 count = 0;
		std::vector<u8> sysex;
		bool in_sysex = false;
	};

	template <unsigned N> void rx_byte(u8 data) { midi_byte(N, data); }
	void midi_byte(int src, u8 data);
	void send_sysex(int src, source &s);
	void queue(u8 code, u8 flags, u8 d1, u8 d2, std::vector<u8> &&block = {});
	static u8 ring_advance(u8 offset, u8 count);
	void deliver();
	TIMER_CALLBACK_MEMBER(deliver_timer);
	TIMER_CALLBACK_MEMBER(tx_timer);
	u8 port_r();
	void port_w(u8 data);

	devcb_write_line m_int_cb;
	devcb_read8::array<4> m_keys_cb;
	devcb_write_line m_tx_cb;
	required_device_array<sc88_sub_rx_device, 3> m_rx;

	u8 m_dpram[0xd8]{};
	u8 m_ipcm[4]{};
	u8 m_ipcer[4]{};
	u8 m_flags[0x1b]{};
	u8 m_sem = 0;
	u8 m_spcon = 0;
	u8 m_pa = 0, m_pa_dir = 0, m_pb = 0, m_pb_dir = 0;
	bool m_int_state = false;
	bool m_in_reset = false;

	source m_src[3];
	std::deque<message> m_queue;
	bool m_busy = false;
	emu_timer *m_deliver_timer = nullptr;

	u8 m_tx_rd = 0;
	u8 m_tx_left = 0;
	u8 m_tx_end = 0;
	emu_timer *m_tx_timer = nullptr;
	u16 m_tx_shift = 0;
	int m_tx_bits = 0;
};

DECLARE_DEVICE_TYPE(SC88_SUB, sc88_sub_device)

#endif // MAME_ROLAND_SC88_SUB_H
