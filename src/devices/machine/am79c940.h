// license:BSD-3-Clause
// copyright-holders:R. Belmont

#ifndef MAME_MACHINE_AM79C940_H
#define MAME_MACHINE_AM79C940_H

#pragma once

#include "dinetwork.h"

class am79c940_device : public device_t, public device_network_interface
{
public:
	am79c940_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// A2 (0941) is deliberately excluded; it has significant errata that would be annoying to emulate
	enum class revision : u16 { B0 = 0x0940, C0 = 0x3940 };
	void set_revision(revision value) { m_revision = value; }

	auto irq_out() { return m_irq_out.bind(); }
	auto tx_drq_out() { return m_tx_drq_out.bind(); }
	auto rx_drq_out() { return m_rx_drq_out.bind(); }

	struct fifo_read_result
	{
		constexpr fifo_read_result() noexcept
			: data(0)
			, valid_mask(0)
			, bytes(0)
			, valid(false)
			, eof(false)
			, status(false)
			, frame_done(false)
		{
		}

		u16 data;
		u16 valid_mask;
		u8 bytes;
		bool valid;
		bool eof;
		bool status;
		bool frame_done;
	};

	bool tx_dma_w(u16 data, u16 mem_mask, bool eof);
	fifo_read_result rx_dma_r(u16 mem_mask = 0xffff);
	fifo_read_result bus_r(offs_t offset, u16 mem_mask = 0xffff);
	bool bus_w(offs_t offset, u16 data, u16 mem_mask = 0xffff, bool eof = false);

	void map(address_map &map) ATTR_COLD;
	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);

	static constexpr feature_type imperfect_features() { return feature::LAN; }

protected:
	am79c940_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;
	virtual int recv_start_cb(u8 *buf, int len) override;
	virtual void recv_complete_cb(int result) override;
	virtual void send_complete_cb(int result) override;

	// bookkeeping
	void count_missed_packet();
	void count_runt_packet();
	void count_receive_collision();

private:
	enum register_number : u8
	{
		RCVFIFO = 0x00, XMTFIFO, XMTFC, XMTFS, XMTRC, RCVFC, RCVFS, FIFOFC,
		IR, IMR, PR, BIUCC, FIFOCC, MACCC, PLSCC, PHYCC,
		CHIPID_LO, CHIPID_HI, IAC,
		LADRF = 0x14, PADR,
		MPC = 0x18,
		RNTPC = 0x1a, RCVCC,
		UTR = 0x1d
	};

	static constexpr u8 ENRCV		= 0x01;
	static constexpr u8 ADDRCHG		= 0x80;
	static constexpr u8 PHYADDR		= 0x04;
	static constexpr u8 LOGADDR		= 0x02;
	static constexpr u32 TX_WORDS	= 68;
	static constexpr u32 RX_WORDS	= 64;
	static constexpr u32 MAX_FRAME	= 4095;

	// FIFO entry flags: low two bits give the number of remaining data bytes.
	static constexpr u8 FIFO_EOF		= 0x04;
	static constexpr u8 FIFO_STATUS		= 0x08;
	static constexpr u8 FIFO_LAST		= 0x10;
	static constexpr u8 FIFO_OVERFLOW	= 0x20;
	static constexpr u8 FIFO_READY		= 0x40;
	static constexpr u8 FIFO_ACTIVE		= 0x80;

	void update_irq(bool force = false);
	void update_requests(bool force = false);
	void reset_tx_fifo(bool cancel_wire);
	void reset_rx_fifo();
	void pump_transmit();
	u8 transmit_status_r();
	bool receive_filter(const u8 *buf) const;
	fifo_read_result receive_r(u16 mem_mask, bool direct, bool status_only = false);
	void rx_push(u16 data, u8 flags);
	void rx_mark_ready();
	void rx_overflow();
	void rx_overflow_status();
	void rx_finish_address_change();
	void rx_append_status(u16 length, u8 flags);
	TIMER_CALLBACK_MEMBER(receive_tick);
	void increment_counter(unsigned reg, u8 event);
	u8 address_r(bool physical);
	void address_w(bool physical, u8 data);
	void advance_address(bool physical);
	bool address_selected(bool physical) const;

	revision m_revision;
	devcb_write_line m_irq_out, m_tx_drq_out, m_rx_drq_out;
	bool m_irq_state, m_tx_drq, m_rx_drq;

	u8 m_reg[32];
	u8 m_padr[6];
	u8 m_ladrf[8];

	u8 m_address_index;
	bool m_address_access;
	bool m_mpc_enabled;

	u8 m_tx_watermark;
	u8 m_rx_watermark;

	u16 m_tx_fifo[TX_WORDS];
	u8 m_tx_flags[TX_WORDS];
	u8 m_tx_controls[TX_WORDS];
	u8 m_tx_head, m_tx_tail, m_tx_count;
	u8 m_tx_frame_count;
	bool m_tx_odd_cycle;
	u8 m_tx_packet[MAX_FRAME + 4];
	u16 m_tx_length;
	bool m_tx_assembling;
	bool m_tx_sending;
	u8 m_tx_loopback;
	u8 m_tx_status[2];
	u8 m_tx_retry[2];
	u8 m_tx_status_head, m_tx_status_count;

	u16 m_rx_fifo[RX_WORDS];
	u8 m_rx_flags[RX_WORDS];
	u8 m_rx_head, m_rx_tail, m_rx_count;
	u8 m_rx_frame_count;
	u8 m_rx_current_words;
	u8 m_rx_packet[MAX_FRAME];
	u16 m_rx_length, m_rx_position, m_rx_wire_length;
	u16 m_rx_wire_position;
	u8 m_rx_error;
	u8 m_rx_runt_snapshot, m_rx_collision_snapshot;
	bool m_rx_busy;
	bool m_rx_readable;
	bool m_rx_request_started;
	bool m_rx_overflow;
	bool m_rx_overflowed;
	bool m_rx_overflow_pending;
	emu_timer *m_receive_timer;
};

DECLARE_DEVICE_TYPE(AM79C940, am79c940_device)

#endif // MAME_MACHINE_AM79C940_H
