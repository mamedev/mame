// license:BSD-3-Clause
// copyright-holders:Alexandre Derumier
/******************************************************************************

    Motorola MCD251 MPEG-1 full motion video decoder.

    The decoder of the CD-i Digital Video Cartridge (VMPEG boards), where it
    sits beside the full motion audio decoder and both are fed the same MPEG-1
    system stream, each selecting its own elementary stream by number.

    Only a technical summary is published, at
    http://www.icdia.co.uk/docs/mcd251ts.pdf, so what the registers do was
    worked out from the CDi_MiSTer FPGA core by Andre Zeps, in particular its
    notes in doc/dvc.md: https://github.com/MiSTer-devel/CDi_MiSTer

    The summary's register maps do fix the addresses, and the ones it lists
    are at the offsets used here: the temporal picture registers T_PWI,
    T_PHE and T_PRPA at 0x02, 0x04 and 0x06, and the video control
    registers Yo, Xo, Ya, Xa, Yd, Xd, Wh, Ww, Yw and Xw from 0x6C to 0x7E.
    The display control buffers it shows at 0x30 and 0x44 continue at 0x58,
    which is the one the driver reads for the picture on screen.

    The chip drives 4 Mbit of picture DRAM itself and lets the host reach it
    through a second chip select, which on a VMPEG board is the 512KB window
    the cartridge exposes.

    The chip's own MPEG decoding is undocumented, so mpeg_video stands in for
    it.  What the chip does around it is emulated: the stream demultiplexing,
    the picture stores and their display order, the decoder clock and the
    display pacing, the status registers and the interrupts.

    The host reads and writes the registers as words, the picture buffers and
    the reconstruction are internal, and the decoded picture is handed to the
    display through video_pixel() in the chip's own pixel coordinates.

*******************************************************************************/

#ifndef MAME_VIDEO_MCD251_H
#define MAME_VIDEO_MCD251_H

#pragma once

#include "machine/mpeg_demux.h"
#include "video/mpeg_video.h"

#include <memory>
#include <vector>


// ======================> mcd251_device

class mcd251_device : public device_t
{
public:
	mcd251_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
	virtual ~mcd251_device();

	// interrupt request, and the transfer request that asks the host to move
	// the next part of the system stream into the chip
	auto irq_callback() { return m_irq_cb.bind(); }
	auto drq_callback() { return m_drq_cb.bind(); }

	// the programmable decoder clock tick, which the host and the audio
	// decoder beside it are paced by
	auto timer_callback() { return m_timer_cb.bind(); }

	// the display standard the chip drives, which it reports to the host
	void set_pal(bool pal) { m_pal = pal; }

	// host interface, word registers
	uint16_t regs_r(offs_t offset, uint16_t mem_mask = ~0);
	void regs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// The 4 Mbit of picture DRAM the chip drives, which the host reaches
	// through the chip's second chip select.
	uint16_t dram_r(offs_t offset, uint16_t mem_mask = ~0);
	void dram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// MPEG-1 system stream in
	void write_data(uint8_t data);

	// the transfer that fed those bytes has finished, dma false when the host
	// wrote them one word at a time
	void end_of_transfer(bool dma);

	// is this chip asking for the interrupt, and with which vector
	bool irq_pending() const { return (m_fmv_isr & m_fmv_ier) != 0; }
	uint8_t vector() const { return uint8_t(m_fmv_ivec >> 3); }

	// vertical timing from the display: the chip picks the next picture on
	// the vsync trailing edge and starts displaying it one frame later
	void vblank_w(int state);

	// The displayed picture, in the chip's pixel coordinates.  Returns true
	// and fills argb when the chip is driving this pixel.
	bool video_pixel(int x, int y, uint32_t &argb) const;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// interrupt status/enable register bits
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

	// SYSCMD bits
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

	// VIDCMD bits
	enum : uint16_t
	{
		VIDCMD_REGSUPD  = 1 << 3,
		VIDCMD_SCROLL   = 1 << 2,
		VIDCMD_VIDON    = 1 << 5,
		VIDCMD_HIDE     = 1 << 8,
		VIDCMD_SHOW     = 1 << 9,
		VIDCMD_SHOWNEXT = 1 << 10
	};

	// what the status registers report about a picture
	struct picture_info
	{
		uint16_t width = 0, height = 0;
		uint8_t video_status = 0;
		uint32_t timecode = 0;
		uint8_t  frameperiod_rawhdr = 0;
		bool first_intra_of_gop = false;
		bool first_intra_of_seq = false;
	};

	// one decoded picture waiting to be displayed
	struct picture
	{
		std::unique_ptr<uint32_t []> rgb;
		picture_info info;
	};

	// a picture as the MPEG decoder reconstructs it, planar YCbCr 4:2:0
	struct picture_store
	{
		std::unique_ptr<uint8_t []> ycbcr;
		picture_info info;
	};

	TIMER_CALLBACK_MEMBER(tim_tick);
	TIMER_CALLBACK_MEMBER(start_video_tick);
	TIMER_CALLBACK_MEMBER(pause_video_tick);
	TIMER_CALLBACK_MEMBER(frame_period_tick);

	void video_decode_pending();
	void video_picture_header(int width, int height);
	void video_picture_decoded();
	void video_sequence_end();
	void video_flush_stalled();
	void release_held_reference();
	void queue_picture(const picture_store &store);
	mpeg_video::picture_buffers video_buffers() const;

	void latch_display_frame();
	void restart_frame_timer();
	void clear_video_fifo();
	void reset_video_decoder();
	void update_intreq();
	void log_fmv_isr(const char *where);

	// the free running decoder clock, and the one the host has offset
	uint32_t raw_dclk() const;
	uint32_t fmv_dclk() const;

	static uint32_t pack_timecode(uint32_t gop_timecode);

	// 4 Mbit of picture DRAM, 256K x 16
	std::unique_ptr<uint16_t []> m_dram;

	devcb_write_line m_irq_cb;
	devcb_write_line m_drq_cb;
	devcb_write_line m_timer_cb;

	bool m_pal = true;

	// ---- register file ----
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

	// ---- stream plumbing ----
	mpeg_demux m_demux;

	int64_t m_next_picture_dts = 0;
	std::vector<int64_t> m_dts_fifo;

	// decoded picture queue and the picture currently on screen
	std::vector<picture> m_picture_fifo;
	picture m_display;
	bool m_display_valid = false;

	// what the status registers report for the picture being displayed
	picture_info m_display_info;

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

	// picture start code scanner over the elementary stream, so PICS_IN_FIFO
	// can report pictures that have arrived but not yet decoded
	uint32_t m_video_startcode_shift = 0;

	// MPEG video decoding.  The decoder reconstructs pictures in coded order
	// into three stores, two reference pictures and a B picture, and they are
	// put in display order here.
	std::unique_ptr<mpeg_video> m_video;
	std::vector<uint8_t> m_video_es;    // elementary stream the decoder has not taken
	bool m_video_needs_data = false;    // the decoder stopped for want of stream data
	picture_store m_store[3];
	int m_store_newest = -1;            // the reference pictures
	int m_store_older = -1;
	bool m_reference_held = false;      // the newest one still waits for display
	int m_store_decoding = -1;          // the picture being decoded, with its references
	int m_store_forward = -1;
	int m_store_backward = -1;
	bool m_pending_seq = false;         // headers the next intra picture reports
	bool m_pending_gop = false;

	bool m_irq_state = false;
};

DECLARE_DEVICE_TYPE(MCD251, mcd251_device)

#endif // MAME_VIDEO_MCD251_H
