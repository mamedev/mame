// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Philips CD-i Digital Video Cartridge (DVC) - VMPEG

    MPEG-1 audio (FMA) and video (FMV) decoder cartridge.

    The register level behaviour was worked out using the reverse engineering
    of the CDi_MiSTer FPGA core by Andre Zeps as the hardware reference (see
    cdidvc.cpp).  The MPEG-1 elementary stream decoding is done with pl_mpeg,
    since the cartridge's decoder chips are undocumented.

    Memory layout as seen by the SCC68070:

        0xd00000..0xdfffff  1MB  additional system RAM
        0xe00000..0xe3ffff       register file (only A[15:1] is decoded, so
                                 the 64KB register window mirrors four times)
        0xe40000..0xe7ffff  256K OS-9 driver ROM
        0xe80000..0xefffff  512K MPEG decoder RAM, hidden from the bus until
                            the driver has poked the register file enough
                            times, so the OS RAM crawler does not find it

    FMA registers live at 0xe03000, FMV registers at 0xe04000.

*******************************************************************************/

#ifndef MAME_PHILIPS_CDIDVC_H
#define MAME_PHILIPS_CDIDVC_H

#pragma once

#include "machine/scc68070.h"
#include "mcd212.h"

#include "screen.h"

#include <memory>
#include <vector>


// ======================> cdi_dvc_device

class cdi_dvc_device : public device_t, public device_sound_interface, public mcd212_ext_video_source
{
public:
	cdi_dvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~cdi_dvc_device();

	auto intreq_callback() { return m_intreq_cb.bind(); }

	template <typename... T> void set_screen(T &&... args) { m_screen.set_tag(std::forward<T>(args)...); }
	template <typename... T> void set_scc(T &&... args) { m_scc.set_tag(std::forward<T>(args)...); }
	void set_pal(bool pal) { m_pal = pal; }

	// SCC68070 bus
	uint16_t regs_r(offs_t offset, uint16_t mem_mask = ~0);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t rom_r(offs_t offset);
	uint16_t ram_r(offs_t offset, uint16_t mem_mask = ~0);
	void ram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint8_t intack_r();

	// the 512KB decoder RAM must not answer the bus before the driver enables it
	bool mpeg_ram_enabled() const { return m_mpeg_ram_enabled; }

	// mcd212_ext_video_source implementation.  Returns true and fills argb when
	// the DVC is driving this pixel, false when the MCD212 should use its own
	// backdrop colour.
	virtual bool ext_video_pixel(int x, int y, uint32_t &argb) const override;

	// vblank edges from the MCD212 screen drive the picture display events
	void screen_vblank(int state);
	void vblank_callback(screen_device &screen, bool state) { screen_vblank(state ? 1 : 0); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	// FMV interrupt status/enable register bits
	enum : uint16_t
	{
		FMV_IRQ_SEQ   = 1 << 0,  // sequence header decoded
		FMV_IRQ_GOP   = 1 << 1,  // group of pictures decoded
		FMV_IRQ_PIC   = 1 << 2,  // picture starts display
		FMV_IRQ_EOD   = 1 << 3,  // end of data
		FMV_IRQ_RFB   = 1 << 4,  // request for bits
		FMV_IRQ_NDAT  = 1 << 5,  // no data / underflow
		FMV_IRQ_OVF   = 1 << 6,  // overflow
		FMV_IRQ_DCL   = 1 << 7,
		FMV_IRQ_TIM   = 1 << 8,  // timer
		FMV_IRQ_ESI   = 1 << 9,  // end sequence indicator
		FMV_IRQ_EII   = 1 << 10, // end ISO indicator
		FMV_IRQ_VSYNC = 1 << 11,
		FMV_IRQ_PAI   = 1 << 12, // pause
		FMV_IRQ_VCUP  = 1 << 13, // video clip update
		FMV_IRQ_ERDD  = 1 << 14,
		FMV_IRQ_ERDV  = 1 << 15
	};

	// FMA status/interrupt register bits
	enum : uint16_t
	{
		FMA_EOI  = 1 << 0,  // ISO end detected
		FMA_CSU  = 1 << 1,  // stream changed
		FMA_UPD  = 1 << 2,  // frame header updated
		FMA_UNF  = 1 << 3,  // underflow
		FMA_DEC  = 1 << 4,  // decoding started
		FMA_ERR  = 1 << 5,
		FMA_POLL = 1 << 8
	};

	// FMV SYSCMD bits
	enum : uint16_t
	{
		SYSCMD_PLAY      = 1 << 3,
		SYSCMD_PAUSE     = 1 << 4,
		SYSCMD_CONTINUE  = 1 << 5,
		SYSCMD_STEP      = 1 << 6,
		SYSCMD_STOP      = 1 << 7,
		SYSCMD_CLEARFIFO = 1 << 8,
		SYSCMD_GOPSEARCH = 1 << 10,
		SYSCMD_DEC_ON    = 1 << 12,
		SYSCMD_DEC_OFF   = 1 << 13,
		SYSCMD_DMA       = 1 << 15
	};

	// FMV VIDCMD bits
	enum : uint16_t
	{
		VIDCMD_REGSUPD  = 1 << 3,
		VIDCMD_SCROLL   = 1 << 2,
		VIDCMD_VIDON    = 1 << 5,
		VIDCMD_HIDE     = 1 << 8,
		VIDCMD_SHOW     = 1 << 9,
		VIDCMD_SHOWNEXT = 1 << 10
	};

	// ---- MPEG-1 system stream demuxer ----
	enum demux_state : uint8_t
	{
		DEMUX_IDLE = 0, DEMUX_MAGIC0, DEMUX_MAGIC2, DEMUX_MAGIC_MATCH,
		DEMUX_PACK0, DEMUX_PACK1, DEMUX_PACK2, DEMUX_PACK3, DEMUX_PACK4, DEMUX_PACK5,
		DEMUX_PES0, DEMUX_PES1, DEMUX_PES2, DEMUX_PES3, DEMUX_PES4, DEMUX_PES5,
		DEMUX_PES6, DEMUX_PES7, DEMUX_PES8,
		DEMUX_PES_DTS0, DEMUX_PES_DTS1, DEMUX_PES_DTS2, DEMUX_PES_DTS3, DEMUX_PES_DTS4
	};

	struct demuxer
	{
		demux_state state = DEMUX_IDLE;
		bool packet_body = false;
		bool length_decreasing = false;
		uint16_t length = 0;
		bool dts_present = false;

		int64_t scr = 0, pts = 0, dts = 0;
		int64_t scr_temp = 0, pts_temp = 0, dts_temp = 0;

		// pulses, valid for the byte that completed the field
		bool scr_updated = false, pts_updated = false, dts_updated = false;
		bool program_end = false;

		void reset();
	};

	// one decoded picture waiting to be displayed
	struct picture
	{
		std::unique_ptr<uint32_t []> rgb;
		uint16_t width = 0, height = 0;
		uint8_t video_status = 0;
		uint32_t timecode = 0;
		uint8_t  frameperiod_rawhdr = 0;
		bool first_intra_of_gop = false;
		bool first_intra_of_seq = false;
	};

	TIMER_CALLBACK_MEMBER(tim_tick);
	TIMER_CALLBACK_MEMBER(start_video_tick);
	TIMER_CALLBACK_MEMBER(pause_video_tick);
	TIMER_CALLBACK_MEMBER(frame_period_tick);
	TIMER_CALLBACK_MEMBER(audio_tick);

	void demux_byte(demuxer &dmx, uint8_t data, uint8_t stream_filter);
	void feed_byte(uint8_t data);
	void run_dma(bool for_fma);

	void video_decode_pending();
	void video_flush_stalled();
	void check_sequence_end();
	// Frame is pl_mpeg's plm_frame_t, an anonymous struct the header cannot name
	template <typename Frame> picture make_picture(const Frame *frame);
	void queue_picture(picture &&pic);
	void audio_decode_pending();

	void latch_display_frame();
	void restart_frame_timer();
	void clear_video_fifo();
	void reset_video_decoder();
	void reset_audio_decoder();
	void update_intreq();
	void log_fmv_isr(const char *where);
	uint32_t fma_dclk() const;
	uint32_t fmv_dclk() const;

	static uint32_t pack_timecode(uint32_t gop_timecode);

	devcb_write_line m_intreq_cb;
	required_device<scc68070_device> m_scc;
	optional_region_ptr<uint16_t> m_rom;
	optional_device<screen_device> m_screen;

	address_space *m_memory_space = nullptr;
	sound_stream *m_stream = nullptr;

	bool m_pal = true;

	// ---- FMA register file (0xe03000) ----
	uint16_t m_fma_cmd = 0;
	uint8_t  m_fma_status = 0;
	uint16_t m_fma_isr = 0;
	uint16_t m_fma_ier = 0;
	uint16_t m_fma_ivec = 0;
	uint8_t  m_fma_stream = 0;
	uint8_t  m_fma_dspa = 0;
	bool     m_fma_dsp_enable = false;
	uint16_t m_fma_dclkl_latch = 0;
	uint32_t m_fma_audio_header = 0;
	bool     m_pending_fma_stream_change = false;

	// ---- FMV register file (0xe04000) ----
	uint16_t m_fmv_isr = 0;
	uint16_t m_fmv_ier = 0;
	uint16_t m_fmv_ivec = 0;
	uint16_t m_fmv_syscmd = 0;
	uint16_t m_fmv_vidcmd = 0;
	uint16_t m_fmv_sysscr = 0;
	uint16_t m_fmv_dec_cmd = 0;
	uint16_t m_fmv_vdi_cmd = 0;
	uint16_t m_fmv_frame_rate = 0;
	uint16_t m_fmv_tcnt = 56 - 1;
	uint8_t  m_fmv_stream = 0;
	bool     m_fmv_dsp_enable = false;
	bool     m_fmv_playback_active = false;
	bool     m_fmv_decoder_active = false;
	uint8_t  m_fmv_slow_motion = 0;

	uint16_t m_image_width = 0, m_image_height = 0, m_image_rt = 0;

	uint16_t m_video_ctrl_y_offset = 0, m_video_ctrl_x_offset = 0;
	uint16_t m_video_ctrl_y_active = 0, m_video_ctrl_x_active = 0;
	uint16_t m_video_ctrl_y_display = 0, m_video_ctrl_x_display = 0;
	uint16_t m_video_ctrl_window_width = 0, m_video_ctrl_window_height = 0;
	uint16_t m_video_ctrl_decoder_offset_y = 0, m_video_ctrl_decoder_offset_x = 0;

	// latched at VCUP time, these are what the display path actually uses
	uint16_t m_latched_display_offset_x = 0, m_latched_display_offset_y = 0;
	uint16_t m_latched_window_offset_x = 0, m_latched_window_offset_y = 0;
	uint16_t m_latched_window_width = 0, m_latched_window_height = 0;

	bool m_register_update_latch = false;
	bool m_register_update_scroll = false;
	bool m_show_video = false;

	// ---- timing ----
	attotime m_dclk_origin;
	int32_t  m_fmv_dclk_offset = 0;
	bool     m_syscr_written = false;
	emu_timer *m_tim_timer = nullptr;
	emu_timer *m_start_video_timer = nullptr;
	emu_timer *m_pause_video_timer = nullptr;
	emu_timer *m_frame_timer = nullptr;
	emu_timer *m_audio_timer = nullptr;

	bool m_mpeg_ram_enabled = false;
	uint8_t m_mpeg_ram_enable_cnt = 0;

	bool m_dma_active = false;
	bool m_dma_for_fma = false;

	// ---- stream plumbing ----
	demuxer m_fma_demux;
	demuxer m_fmv_demux;

	// audio playback start timer
	bool m_fma_scr_start_valid = false;
	int64_t m_fma_scr_start_time = 0;

	int64_t m_next_picture_dts = 0;
	std::vector<int64_t> m_dts_fifo;

	// decoded picture queue and the picture currently on screen
	std::vector<picture> m_picture_fifo;
	picture m_display;
	bool m_display_valid = false;

	// values the status registers report for the picture being displayed
	uint16_t m_display_width = 0, m_display_height = 0;
	uint8_t  m_display_video_status = 0;
	uint32_t m_display_timecode = 0;
	uint8_t  m_display_frameperiod_rawhdr = 0;
	bool m_display_first_intra_of_gop = false;
	bool m_display_first_intra_of_seq = false;

	// values the status registers report for the picture last decoded
	uint16_t m_decoder_width = 0, m_decoder_height = 0;
	uint32_t m_decoder_timecode = 0;
	uint16_t m_decoder_frameperiod_90khz = 0;
	uint8_t  m_decoder_frameperiod_rawhdr = 0;

	// frame pacing
	uint32_t m_frame_period_30mhz = 1200000;
	int32_t  m_display_dts_desync = 0;
	bool     m_desync_satisfied = false;
	bool     m_latch_until_vsync = false;
	bool     m_latch_until_vblank = false;
	bool     m_single_step_latch = false;

	uint32_t m_pictures_in_input_fifo = 0;
	uint16_t m_logged_fmv_isr = 0;  // for log_fmv_isr

	// picture start code scanner over the video elementary stream, so
	// PICS_IN_FIFO can report pictures that have arrived but not yet decoded
	uint32_t m_video_startcode_shift = 0;

	// audio.  A read head rather than erase(begin()), which would make the
	// stream callback quadratic in the queue depth.
	std::vector<int16_t> m_audio_samples[2];
	size_t m_audio_head = 0;
	uint32_t m_audio_sample_rate = 44100;
	size_t audio_available() const { return m_audio_samples[0].size() - m_audio_head; }
	void audio_clear() { m_audio_samples[0].clear(); m_audio_samples[1].clear(); m_audio_head = 0; }

	// elementary stream write batching, so we do not call into pl_mpeg once
	// per byte for a whole DMA transfer
	std::vector<uint8_t> m_es_batch;
	bool m_es_batch_for_fma = false;
	void flush_es_batch();
	uint64_t m_video_es_written = 0;  // bytes written to the video ES buffer
	bool m_video_needs_data = false;   // pl_mpeg stopped for want of stream data
	uint64_t m_flush_tried_at = 0;    // m_video_es_written at the last flush attempt

	// the raw MPEG audio frame header, reported at 0xe03014/0xe03016
	uint32_t m_audio_header_shift = 0;

	// pl_mpeg state, opaque here so pl_mpeg.h stays out of the header
	struct decoder_state;
	std::unique_ptr<decoder_state> m_dec;

	std::unique_ptr<uint16_t []> m_ram;
	bool m_intreq_state = false;
};

// ======================> cdi_dvc_slot_device

// The cartridge slot on the back of the player.  The cartridge routes its
// sound through the slot, which the machine routes to its speakers.
class cdi_dvc_slot_device : public device_t, public device_single_card_slot_interface<cdi_dvc_device>, public device_mixer_interface
{
public:
	template <typename T>
	cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: cdi_dvc_slot_device(mconfig, tag, owner, 0U)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}

	cdi_dvc_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_start() override ATTR_COLD;
};

void cdi_dvc_cards(device_slot_interface &device);

DECLARE_DEVICE_TYPE(CDI_DVC, cdi_dvc_device)
DECLARE_DEVICE_TYPE(CDI_DVC_SLOT, cdi_dvc_slot_device)

#endif // MAME_PHILIPS_CDIDVC_H
