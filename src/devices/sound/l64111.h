// license:BSD-3-Clause
// copyright-holders:Matt Phelps
/***************************************************************************

    LSI Logic L64111 MPEG audio decoder

***************************************************************************/

#ifndef MAME_SOUND_L64111_H
#define MAME_SOUND_L64111_H

#pragma once

#include "mpeg_audio.h"

class l64111_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type imperfect_features() { return feature::SOUND; }

	l64111_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0) ATTR_COLD;

	// outputs
	auto intr() { return m_intr_cb.bind(); }     // INTR, active low
	auto femty() { return m_femty_cb.bind(); }   // input FIFO empty, active low
	auto fale() { return m_fale_cb.bind(); }     // input FIFO almost empty, active low
	auto falf() { return m_falf_cb.bind(); }     // input FIFO almost full, active low
	auto starto() { return m_starto_cb.bind(); } // STARTO: decoding stored frames

	// controller interface: registers at A[4:0], data D[7:0]
	void map(address_map &map) ATTR_COLD;
	void data_w(u16 data); // Data register (A[4:0] = 0) write with the full D[15:0] bus

	int femty_r();
	int fale_r();
	int falf_r();
	int starto_r();

	void reset_w(int state);  // RESET, active high
	void starti_w(int state); // STARTI
	void hdmute_w(int state); // HDMUTE, active high

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override ATTR_COLD;
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// Control Register 1 and 2 bits
	static constexpr u8 CR1_ANC_RST = 0x80, CR1_SER_PAR = 0x40, CR1_BYPASS = 0x20, CR1_BUF_RST = 0x04,
			CR1_SOFT_RST = 0x02, CR1_STARTI = 0x01;
	static constexpr u8 CR2_SOFT_MUTE = 0x20, CR2_SEL_16 = 0x10, CR2_AUDIO_ONLY = 0x08;
	static constexpr u8 CR3_ID_IGNORE = 0x80;

	// Interrupt Mask / Status Register 1 (the same bit positions)
	static constexpr u8 S1_ANC_VALID = 0x80, S1_ANC_OVRFL = 0x40, S1_ANC_HFF = 0x20, S1_ERR_BUF_OVFL = 0x08;

	// Status Register 2, and Interrupt Mask 2: one bit higher, with NEW FRAME OUT and LAST FRAME separate
	static constexpr u8 S2_SYNTAX_ERR = 0x40, S2_PTS = 0x20, S2_SYNC_AUD = 0x10, S2_SYNC_SYS = 0x08,
			S2_FRAME_DETECT = 0x04, S2_CRC_ERR = 0x02, S2_NEW_LAST_FRAME = 0x01;
	static constexpr u8 M2_SYNTAX_ERR = 0x80, M2_PTS = 0x40, M2_SYNC_AUD = 0x20, M2_SYNC_SYS = 0x10,
			M2_FRAME_DETECT = 0x08, M2_CRC_ERR = 0x04, M2_NEW_FRAME = 0x02, M2_LAST_FRAME = 0x01;

	// system stream parser states
	enum : u8 { SYS_SCAN, SYS_PACK, SYS_LENGTH, SYS_SKIP, SYS_PACKET_HEADER, SYS_PACKET_DATA };

	static constexpr u32 FIFO_BYTES = 128;
	static constexpr u32 FIFO_FLAG_LEVEL = 32;   // FALE / FALF thresholds
	static constexpr u32 PREPARSE_CLOCKS = 16;   // 15 Mbit/s at SYSCLK 30 MHz
	static constexpr u32 FRAME_BYTES_MAX = 1792; // largest Layer I/II frame, rounded up
	static constexpr u32 CB_SLOTS = 128;         // channel buffer: 128 Layer I or 64 Layer II frames
	static constexpr u32 SAMPLES_MAX = 1152;
	static constexpr u32 ANC_BYTES = 16;

	// register handlers
	bool reg_read_ready();
	bool reg_write_ready(const char *name, u8 data);
	u8 reserved_r(offs_t offset);
	void reserved_w(offs_t offset, u8 data);
	void data8_w(u8 data);
	u8 cr1_r();
	void cr1_w(u8 data);
	u8 cr2_r();
	void cr2_w(u8 data);
	u8 cr3_r();
	void cr3_w(u8 data);
	u8 status1_r();
	void mask1_w(u8 data);
	u8 status2_r();
	void mask2_w(u8 data);
	u8 tcr_r();
	void tcr_w(u8 data);
	u8 torh_r();
	void torh_w(u8 data);
	u8 torl_r();
	void torl_w(u8 data);
	u8 pd_r(offs_t offset);
	u8 pts_r(offs_t offset);
	u8 anc_r();
	u8 cb_status_r();
	u8 cb_write_r();
	void cb_write_w(u8 data);
	u8 cb_read_r();
	void cb_read_w(u8 data);

	void chip_reset() ATTR_COLD;
	void decoder_reset();
	void parser_reset();
	void sync();
	void sync_fifo();
	void deliver_events();
	void schedule_flags();
	void update_outputs();
	void update_poll();
	void system_parse(u8 data);
	void packet_header(u8 data);
	void audio_parse(u8 data);
	void store_frame();
	bool next_frame();
	void silent_frame();
	void ancillary(u32 bit_pos, u32 bit_end);
	void interrupt(bool enabled);
	void update_intr();
	void update_rate();
	void syntax_error();
	u32 cb_frames() const { return (m_cb_layer == 1) ? CB_SLOTS : CB_SLOTS / 2; }
	bool decoder_enabled() const { return !m_reset_pin && (m_cr1 & CR1_STARTI) && m_starti_pin; }
	TIMER_CALLBACK_MEMBER(poll);
	TIMER_CALLBACK_MEMBER(flags_tick);

	devcb_write_line m_intr_cb, m_femty_cb, m_fale_cb, m_falf_cb, m_starto_cb;
	sound_stream *m_stream;
	std::unique_ptr<mpeg_audio> m_decoder;
	emu_timer *m_poll_timer;
	emu_timer *m_flags_timer;

	// registers
	u8 m_cr1, m_cr2, m_cr3;
	u8 m_mask1, m_mask2;
	u8 m_status1, m_status2; // latched event bits
	u8 m_tcr, m_torh, m_torl;
	u8 m_pd[3];
	u8 m_pts[5];

	// pins and outputs
	bool m_reset_pin, m_starti_pin, m_hdmute_pin;
	bool m_intr_pending;
	int m_intr_state, m_femty_state, m_fale_state, m_falf_state, m_starto_state;

	// input data FIFO
	u8 m_fifo[FIFO_BYTES];
	u32 m_fifo_read, m_fifo_count;
	attotime m_fifo_time; // time up to which the pre-parser has drained the FIFO
	bool m_overflow_logged;

	// pre-parser: system layer
	u8 m_sys_state;
	u32 m_sys_shift, m_sys_shift_count;
	u8 m_sys_code;
	u32 m_sys_left, m_sys_length_bytes;
	u8 m_sys_hdr[10];
	u32 m_sys_hdr_len, m_sys_hdr_need;
	u32 m_pack_count;

	// pre-parser: audio frames
	u8 m_frame[FRAME_BYTES_MAX];
	u32 m_frame_len, m_frame_need;
	u32 m_sync_count;

	// channel buffer (whole frames, not the external DRAM's layout)
	u8 m_cb[CB_SLOTS][FRAME_BYTES_MAX];
	u16 m_cb_len[CB_SLOTS];
	u32 m_cb_write, m_cb_read, m_cb_count;
	u8 m_cb_layer;

	// ancillary data FIFO
	u8 m_anc[ANC_BYTES];
	u32 m_anc_read, m_anc_count;
	u32 m_anc_acc, m_anc_bits;
	bool m_anc_overflow;

	// decoder and PCM output
	u8 m_decode[FRAME_BYTES_MAX];
	s16 m_pcm[SAMPLES_MAX * 2];
	u32 m_pcm_pos, m_pcm_count, m_pcm_channels;
	bool m_started, m_underrun;
	// events raised during the stream update, delivered from timer or CPU context
	bool m_ev_last_frame, m_ev_new_frame, m_ev_anc_valid, m_ev_anc_hff, m_ev_anc_overflow;
};

DECLARE_DEVICE_TYPE(L64111, l64111_device)

#endif // MAME_SOUND_L64111_H
