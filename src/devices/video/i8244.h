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
	i8244_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration helpers
	auto irq_cb() { return m_irq_func.bind(); }
	i8244_device &set_screen_size(int width, int height, int cropx, int cropy);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	int vblank();
	int hblank();
	void i8244_palette(palette_device &palette) const;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	union vdc_t {
		uint8_t reg[0x100];
		struct {
			// 0x00
			struct {
				uint8_t y,x,color,unused;
			} sprites[4];

			// 0x10
			struct {
				uint8_t y,x,ptr,color;
			} foreground[12];

			// 0x40
			struct {
				struct {
					uint8_t y,x,ptr,color;
				} single[4];
			} quad[4];

			// 0x80
			uint8_t shape[4][8];

			// 0xa0
			uint8_t control;
			uint8_t status;
			uint8_t collision;
			uint8_t color;
			uint8_t y;
			uint8_t x;
			uint8_t unused;
			uint8_t shift1;
			uint8_t shift2;
			uint8_t shift3;
			uint8_t sound;
			uint8_t unused2[5+0x10];

			// 0xc0
			uint8_t hgrid[2][0x10];
			uint8_t vgrid[0x10];
			uint8_t unused3[0x10];
		} s;
	};

	i8244_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	// device_sound_interface overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	virtual void set_default_params();

	//void render_scanline(int vpos);
	int get_y_beam();
	int get_x_beam();
	offs_t fix_register_mirrors( offs_t offset );
	bool invalid_register( offs_t offset, bool rw );

	/* timers */
	static constexpr device_timer_id TIMER_VBLANK_START = 0;
	static constexpr device_timer_id TIMER_HBLANK_START = 1;

	// callbacks
	devcb_write_line m_irq_func;

	required_region_ptr<uint8_t> m_charset;

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
	uint8_t m_collision_map[0x200];

	uint8_t m_x_beam_pos = 0;
	uint8_t m_y_beam_pos = 0;
	uint8_t m_y_latch = 0;
	uint8_t m_control_status = 0;
	uint8_t m_collision_status = 0;
	bool m_pos_hold = false;
	bool m_y_hold = false;

	bool m_sh_written = false;
	bool m_sh_pending = false;
	u8 m_sh_prescaler = 0 ;
	int m_sh_output = 0;
	u8 m_sh_duty = 0;
};


class i8245_device :  public i8244_device
{
public:
	// construction/destruction
	i8245_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void set_default_params() override;
};


// device type definition
DECLARE_DEVICE_TYPE(I8244, i8244_device)
DECLARE_DEVICE_TYPE(I8245, i8245_device)

#endif // MAME_VIDEO_I8244_H
