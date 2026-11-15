// license:BSD-3-Clause
// copyright-holders:MagikalUnicorn

#ifndef MAME_TVGAMES_XAVIX_IO_LTV_TAM_IR_H
#define MAME_TVGAMES_XAVIX_IO_LTV_TAM_IR_H

#pragma once

#include "xavix_io_ir.h"

#include "imagedev/bitbngr.h"

#include <array>
#include <utility>

class xavix_tamagotchi_ir_device : public device_t, public xavix_ir_receiver_interface
{
public:
	xavix_tamagotchi_ir_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_event_callback(T &&... args) { m_event_out_cb.set(std::forward<T>(args)...); }

	int data_r() const { return m_rx_data; }
	void tx_w(int state);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// Stream frames have a two-byte big-endian byte count, followed by the payload.
	// This buffer limit belongs to the transport, not the optical protocol.
	static constexpr unsigned MAX_PACKET_BYTES = 256;
	static constexpr u32 CARRIER_HALF_TICKS = (0x8d + 1) << 1;
	static constexpr u32 LEADER_MARK_TICKS = 0x2d6 * CARRIER_HALF_TICKS;
	static constexpr u32 LEADER_SPACE_TICKS = (0xc8 + 1) << 8;
	static constexpr u32 DATA_MARK_TICKS = 0x24 * CARRIER_HALF_TICKS;
	static constexpr u32 ZERO_SPACE_TICKS = (0x3c + 1) << 8;
	static constexpr u32 ONE_SPACE_TICKS = (0x6f + 1) << 8;
	static constexpr u32 TRAILER_MARK_TICKS = 0x5c * CARRIER_HALF_TICKS;
	static constexpr u32 TX_SILENCE_TICKS = CARRIER_HALF_TICKS * 6;
	static constexpr u32 TX_LEADER_MIN_TICKS = LEADER_MARK_TICKS * 4 / 5;
	// Firmware loop overhead stretches the nominal 726-edge transmit leader to about 12.56 ms.
	static constexpr u32 TX_LEADER_MAX_TICKS = LEADER_MARK_TICKS * 3 / 2;
	static constexpr u32 TX_DATA_MARK_MIN_TICKS = DATA_MARK_TICKS * 2 / 3;
	static constexpr u32 TX_DATA_MARK_MAX_TICKS = DATA_MARK_TICKS * 3 / 2;
	static constexpr u32 TX_TRAILER_MIN_TICKS = DATA_MARK_TICKS * 2;
	static constexpr u32 TX_BIT_THRESHOLD_TICKS = (ZERO_SPACE_TICKS + ONE_SPACE_TICKS) / 2;
	static constexpr u32 TX_FRAME_GAP_MAX_TICKS = LEADER_SPACE_TICKS * 3 / 2;

	enum receive_phase : u8
	{
		IDLE,
		LEADER_MARK,
		LEADER_SPACE,
		DATA_MARK,
		DATA_SPACE,
		TRAILER_MARK
	};

	TIMER_CALLBACK_MEMBER(poll_stream);
	TIMER_CALLBACK_MEMBER(receive_tick);
	void reset_link();
	void start_receive_mark(receive_phase phase, u32 ticks);
	u32 receive_space_ticks() const;
	virtual void ir_burst_started(u64 gap_ticks) override;
	virtual void ir_burst_finished(u64 width_ticks) override;
	virtual void ir_idle_timeout() override;
	void append_transmit_bit(bool bit);
	void finish_transmit_packet();

	required_device<bitbanger_device> m_stream;
	util::notifier_subscription m_media_notifier;
	device_delegate<void ()> m_event_out_cb;
	emu_timer *m_poll_timer;
	emu_timer *m_receive_timer;

	std::array<u8, MAX_PACKET_BYTES> m_receive_packet;
	std::array<u8, MAX_PACKET_BYTES> m_transmit_packet;
	u16 m_stream_length;
	u16 m_stream_bytes;
	u8 m_stream_header_bytes;
	u8 m_receive_phase;
	u16 m_receive_length;
	u16 m_receive_bit;
	u8 m_transmit_shift;
	u8 m_transmit_bit;
	u16 m_transmit_bytes;
	u8 m_rx_data;
	bool m_transmit_frame;
	bool m_transmit_mark_pending;
};

DECLARE_DEVICE_TYPE(XAVIX_TAMAGOTCHI_IR, xavix_tamagotchi_ir_device)

#endif // MAME_TVGAMES_XAVIX_IO_LTV_TAM_IR_H
