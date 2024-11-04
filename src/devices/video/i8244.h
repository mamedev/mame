// license:BSD-3-Clause
// copyright-holders:Peter Trauner, Wilbert Pol, hap
/***************************************************************************

    Intel 8244 (NTSC)/8245 (PAL) Graphics and sound chip

***************************************************************************/

#ifndef MAME_VIDEO_I8244_H
#define MAME_VIDEO_I8244_H

#pragma once

#include "emupal.h"


// pinout reference

/*
            ___   ___
    CLK  1 |*  \_/   | 28 Vcc
  _INTR  2 |         | 27 SND
    STB  3 |         | 26 ALE
     BG  4 |         | 25 D0
    CSY  5 |         | 24 D1
    M/S  6 |         | 23 D2
    HBL  7 |  P8244  | 22 D3
    VBL  8 |  P8245  | 21 D4
     CX  9 |         | 20 B
      L 10 |         | 19 G
    _CS 11 |         | 18 R
    _WR 12 |         | 17 D5
    _RD 13 |         | 16 D6
    Vss 14 |_________| 15 D7

*/

DECLARE_DEVICE_TYPE(I8244, i8244_device)
DECLARE_DEVICE_TYPE(I8245, i8245_device)


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
	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

	TIMER_CALLBACK_MEMBER(hblank_start);
	TIMER_CALLBACK_MEMBER(vblank_start);

	virtual void set_default_params();
	inline bool is_ntsc() { return m_vtotal == 263; }

	int get_y_beam();
	int get_x_beam();
	offs_t fix_register_mirrors(offs_t offset);

	void draw_grid(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_major(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_minor(int scanline, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void major_pixel(u8 index, int x, int y, u8 pixel, u16 color, bitmap_ind16 &bitmap, const rectangle &cliprect);

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
	u8 m_priority_map[0x200];

	u8 m_x_beam_pos;
	u8 m_y_beam_pos;
	u8 m_control_status;
	u8 m_collision_status;

	bool m_sh_written;
	bool m_sh_pending;
	u8 m_sh_prescaler;
	u8 m_sh_count;
	int m_sh_output;
	u8 m_sh_duty;
};


class i8245_device : public i8244_device
{
public:
	// construction/destruction
	i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static auto parent_rom_device_type() { return &I8244; }

protected:
	virtual void set_default_params() override;
};


#endif // MAME_VIDEO_I8244_H
