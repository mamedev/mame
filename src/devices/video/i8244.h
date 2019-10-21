// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    i8244.h

    Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip

***************************************************************************/

#ifndef MAME_VIDEO_I8244_H
#define MAME_VIDEO_I8244_H

#pragma once

#include "emupal.h"


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/


// ======================> i8244_device

class i8244_device :  public device_t
					, public device_sound_interface
					, public device_video_interface
{
public:
	// construction/destruction
	i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_cb() { return m_irq_func.bind(); }
	auto postprocess_cb() { return m_postprocess_func.bind(); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ_LINE_MEMBER(vblank);
	DECLARE_READ_LINE_MEMBER(hblank);
	void i8244_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }

	// Global constants
	static constexpr int START_ACTIVE_SCAN = 42;
	static constexpr int BORDER_SIZE       = 10;
	static constexpr int END_ACTIVE_SCAN   = 42 + 10 + 320 + 10;
	static constexpr int START_Y           = 1;
	static constexpr int SCREEN_HEIGHT     = 243;
	static constexpr int LINE_CLOCKS       = 455;
	static constexpr int LINES             = 262;

protected:
	union vdc_t {
		uint8_t reg[0x100];
		struct {
			struct {
				uint8_t y,x,color,res;
			} sprites[4];
			struct {
				uint8_t y,x,ptr,color;
			} foreground[12];
			struct {
				struct {
					uint8_t y,x,ptr,color;
				} single[4];
			} quad[4];
			uint8_t shape[4][8];
			uint8_t control;
			uint8_t status;
			uint8_t collision;
			uint8_t color;
			uint8_t y;
			uint8_t x;
			uint8_t res;
			uint8_t shift1;
			uint8_t shift2;
			uint8_t shift3;
			uint8_t sound;
			uint8_t res2[5+0x10];
			uint8_t hgrid[2][0x10];
			uint8_t vgrid[0x10];
		} s;
	};

	i8244_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int lines);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void render_scanline(int vpos);
	int get_y_beam();
	int get_x_beam();
	offs_t fix_register_mirrors( offs_t offset );

	// Local constants
	static constexpr uint8_t VDC_CONTROL_REG_STROBE_XY = 0x02;

	/* timers */
	static constexpr device_timer_id TIMER_LINE = 0;
	static constexpr device_timer_id TIMER_HBLANK = 1;

	// callbacks
	devcb_write_line m_irq_func;
	devcb_write16 m_postprocess_func;

	bitmap_ind16 m_tmp_bitmap;
	emu_timer *m_line_timer;
	emu_timer *m_hblank_timer;
	sound_stream *m_stream;

	int m_start_vpos;
	int m_start_vblank;
	int m_screen_lines;

	vdc_t m_vdc;
	uint16_t m_sh_count;
	uint8_t m_x_beam_pos;
	uint8_t m_y_beam_pos;
	uint8_t m_control_status;
	uint8_t m_collision_status;
	int m_iff;
};


class i8245_device :  public i8244_device
{
public:
	// construction/destruction
	i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	static constexpr int LINES = 312;
};


// device type definition
DECLARE_DEVICE_TYPE(I8244, i8244_device)
DECLARE_DEVICE_TYPE(I8245, i8245_device)

#endif // MAME_VIDEO_I8244_H
