// license:BSD-3-Clause
// copyright-holders:Wilbert Pol, hap
/***************************************************************************

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
	i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration helpers
	auto irq_cb() { return m_irq_func.bind(); }
	i8244_device &set_screen_size(int width, int height, int cropx = 0, int cropy = 0);

	u8 read(offs_t offset);
	void write(offs_t offset, u8 data);
	void write_cx(int x, bool cx); // CX pin on current scanline

	int vblank();
	int hblank();
	void i8244_palette(palette_device &palette) const;

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	union vdc_t {
		u8 reg[0x100];
		struct {
			// 0x00
			struct {
				u8 y,x,color,unused;
			} sprites[4];

			// 0x10
			struct {
				u8 y,x,ptr,color;
			} foreground[12];

			// 0x40
			struct {
				struct {
					u8 y,x,ptr,color;
				} single[4];
			} quad[4];

			// 0x80
			u8 shape[4][8];

			// 0xa0
			u8 control;
			u8 status;
			u8 collision;
			u8 color;
			u8 y;
			u8 x;
			u8 unused;
			u8 shift1;
			u8 shift2;
			u8 shift3;
			u8 sound;
			u8 unused2[5+0x10];

			// 0xc0
			u8 hgrid[2][0x10];
			u8 vgrid[0x10];
			u8 unused3[0x10];
		} s;
	};

	i8244_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	virtual void set_default_params();

	int get_y_beam();
	int get_x_beam();
	offs_t fix_register_mirrors(offs_t offset);
	bool invalid_register(offs_t offset, bool rw);

	/* timers */
	static constexpr device_timer_id TIMER_VBLANK_START = 0;
	static constexpr device_timer_id TIMER_HBLANK_START = 1;

	// callbacks
	devcb_write_line m_irq_func;

	required_region_ptr<u8> m_charset;

	emu_timer *m_hblank_timer;
	emu_timer *m_vblank_timer;
	sound_stream *m_stream;

	void sound_update();

	int m_htotal;
	int m_vtotal;
	int m_width;
	int m_height;
	int m_cropx;
	int m_cropy;

	int m_vblank_start;
	int m_vblank_end;
	int m_hblank_start;
	int m_hblank_end;
	int m_bgate_start;

	vdc_t m_vdc;
	u8 m_collision_map[0x200];

	u8 m_x_beam_pos = 0;
	u8 m_y_beam_pos = 0;
	u8 m_control_status = 0;
	u8 m_collision_status = 0;

	bool m_sh_written = false;
	bool m_sh_pending = false;
	u8 m_sh_prescaler = 0;
	u8 m_sh_count = 0;
	int m_sh_output = 0;
	u8 m_sh_duty = 0;
};


class i8245_device :  public i8244_device
{
public:
	// construction/destruction
	i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void set_default_params() override;
};


// device type definition
DECLARE_DEVICE_TYPE(I8244, i8244_device)
DECLARE_DEVICE_TYPE(I8245, i8245_device)

#endif // MAME_VIDEO_I8244_H
