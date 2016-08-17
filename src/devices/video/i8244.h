// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/***************************************************************************

    i8244.h

    Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip

***************************************************************************/

#pragma once

#ifndef __I8244_H__
#define __I8244_H__

#include "emu.h"


/***************************************************************************
    DEVICE CONFIGURATION MACROS
***************************************************************************/

#define MCFG_I8244_ADD(_tag, _clock, _screen_tag, _irq_cb, _postprocess_cb) \
	MCFG_DEVICE_ADD(_tag, I8244, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_I8244_IRQ_CB(_irq_cb) \
	MCFG_I8244_POSTPROCESS_CB(_postprocess_cb)
#define MCFG_I8244_IRQ_CB(_devcb) \
	devcb = &i8244_device::set_irq_cb(*device, DEVCB_##_devcb);
#define MCFG_I8244_POSTPROCESS_CB(_devcb) \
	devcb = &i8244_device::set_postprocess_cb(*device, DEVCB_##_devcb);
#define MCFG_I8245_ADD(_tag, _clock, _screen_tag, _irq_cb, _postprocess_cb) \
	MCFG_DEVICE_ADD(_tag, I8245, _clock) \
	MCFG_VIDEO_SET_SCREEN(_screen_tag) \
	MCFG_I8244_IRQ_CB(_irq_cb) \
	MCFG_I8244_POSTPROCESS_CB(_postprocess_cb )

/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

union vdc_t {
	UINT8 reg[0x100];
	struct {
		struct {
			UINT8 y,x,color,res;
		} sprites[4];
		struct {
			UINT8 y,x,ptr,color;
		} foreground[12];
		struct {
			struct {
				UINT8 y,x,ptr,color;
			} single[4];
		} quad[4];
		UINT8 shape[4][8];
		UINT8 control;
		UINT8 status;
		UINT8 collision;
		UINT8 color;
		UINT8 y;
		UINT8 x;
		UINT8 res;
		UINT8 shift1;
		UINT8 shift2;
		UINT8 shift3;
		UINT8 sound;
		UINT8 res2[5+0x10];
		UINT8 hgrid[2][0x10];
		UINT8 vgrid[0x10];
	} s;
};


// ======================> i8244_device

class i8244_device :  public device_t
					, public device_sound_interface
					, public device_video_interface
{
public:
	// construction/destruction
	i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	i8244_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, int lines, const char *shortname, const char *source);

	// static configuration helpers
	static void set_screen_tag(device_t &device, const char *screen_tag) { downcast<i8244_device &>(device).m_screen_tag = screen_tag; }
	template<class _Object> static devcb_base &set_irq_cb(device_t &device, _Object object) { return downcast<i8244_device &>(device).m_irq_func.set_callback(object); }
	template<class _Object> static devcb_base &set_postprocess_cb(device_t &device, _Object object) { return downcast<i8244_device &>(device).m_postprocess_func.set_callback(object); }

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	DECLARE_READ_LINE_MEMBER(vblank);
	DECLARE_READ_LINE_MEMBER(hblank);
	DECLARE_PALETTE_INIT(i8244);

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	inline bitmap_ind16 *get_bitmap() { return &m_tmp_bitmap; }

	// Global constants
	static const int START_ACTIVE_SCAN = 42;
	static const int BORDER_SIZE       = 10;
	static const int END_ACTIVE_SCAN   = 42 + 10 + 320 + 10;
	static const int START_Y           = 1;
	static const int SCREEN_HEIGHT     = 243;
	static const int LINE_CLOCKS       = 455;
	static const int LINES             = 262;

protected:
	// device-level overrides
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
	static const UINT8 VDC_CONTROL_REG_STROBE_XY = 0x02;

	/* timers */
	static const device_timer_id TIMER_LINE = 0;
	static const device_timer_id TIMER_HBLANK = 1;

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
	UINT16 m_sh_count;
	UINT8 m_x_beam_pos;
	UINT8 m_y_beam_pos;
	UINT8 m_control_status;
	UINT8 m_collision_status;
	int m_iff;
};


class i8245_device :  public i8244_device
{
public:
	// construction/destruction
	i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static const int LINES = 312;
};


// device type definition
extern const device_type I8244;
extern const device_type I8245;


#endif  /* __I8244_H__ */
