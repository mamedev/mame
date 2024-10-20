// license:BSD-3-Clause
// copyright-holders:Curt Coder, Olivier Galibert
#ifndef MAME_ATARI_ATARIST_V_H
#define MAME_ATARI_ATARIST_V_H

#pragma once

#define ATARIST_HBSTART_PAL     128*4
#define ATARIST_HBEND_PAL       0
#define ATARIST_HBSTART_NTSC    127*4
#define ATARIST_HBEND_NTSC      0
#define ATARIST_HTOT_PAL        129*4
#define ATARIST_HTOT_NTSC       128*4

#define ATARIST_HBDEND_PAL      14*4
#define ATARIST_HBDSTART_PAL    94*4
#define ATARIST_HBDEND_NTSC     13*4
#define ATARIST_HBDSTART_NTSC   93*4

#define ATARIST_VBEND_PAL       0
#define ATARIST_VBEND_NTSC      0
#define ATARIST_VBSTART_PAL     312
#define ATARIST_VBSTART_NTSC    262
#define ATARIST_VTOT_PAL        313
#define ATARIST_VTOT_NTSC       263

#define ATARIST_VBDEND_PAL      63
#define ATARIST_VBDSTART_PAL    263
#define ATARIST_VBDEND_NTSC     34
#define ATARIST_VBDSTART_NTSC   234

class stx_video_device : public device_t, public device_palette_interface, public device_video_interface
{
public:
	stx_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_ram_space(T &&tag, int spacenum) { m_ram_space.set_tag(tag, spacenum); }
	auto de_callback() { return m_de_callback.bind(); }

	uint8_t shifter_base_r(offs_t offset);
	uint8_t shifter_counter_r(offs_t offset);
	uint8_t shifter_sync_r();
	uint16_t shifter_palette_r(offs_t offset);
	uint8_t shifter_mode_r();

	void shifter_base_w(offs_t offset, uint8_t data);
	void shifter_sync_w(uint8_t data);
	void shifter_palette_w(offs_t offset, uint16_t data);
	void shifter_mode_w(uint8_t data);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

protected:
	stx_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual uint32_t palette_entries() const noexcept override { return 16; }

private:
	inline pen_t shift_mode_0();
	inline pen_t shift_mode_1();
	inline pen_t shift_mode_2();
	TIMER_CALLBACK_MEMBER(shifter_tick);
	inline void shifter_load();
	inline void draw_pixel(int x, int y, u32 pen);
	TIMER_CALLBACK_MEMBER(glue_tick);
	void set_screen_parameters();

	required_address_space m_ram_space;
	required_device<cpu_device> m_maincpu; // HACK

	devcb_write_line m_de_callback;

	bitmap_rgb32 m_bitmap;

	// timers
	emu_timer *m_glue_timer = nullptr;
	emu_timer *m_shifter_timer = nullptr;

protected:
	// shifter state
	uint32_t m_shifter_base = 0U;
	uint32_t m_shifter_ofs = 0U;
	uint8_t m_shifter_sync = 0U;
	uint8_t m_shifter_mode = 0U;
	uint16_t m_shifter_palette[16]{};
	uint16_t m_shifter_rr[4]{};
	uint16_t m_shifter_ir[4]{};
	int m_shifter_bitplane = 0;
	int m_shifter_shift = 0;
	int m_shifter_h = 0;
	int m_shifter_v = 0;
	int m_shifter_de = 0;
	int m_shifter_x_start = 0;
	int m_shifter_x_end = 0;
	int m_shifter_y_start = 0;
	int m_shifter_y_end = 0;
	int m_shifter_hblank_start = 0;
	int m_shifter_vblank_start = 0;
};

class ste_video_device : public stx_video_device
{
public:
	ste_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t shifter_base_low_r();
	void shifter_base_low_w(uint8_t data);
	uint8_t shifter_counter_r(offs_t offset);
	void shifter_counter_w(offs_t offset, uint8_t data);
	void shifter_palette_w(offs_t offset, uint16_t data);
	uint8_t shifter_lineofs_r();
	void shifter_lineofs_w(uint8_t data);
	uint8_t shifter_pixelofs_r();
	void shifter_pixelofs_w(uint8_t data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual uint32_t palette_entries() const noexcept override { return 512; }

private:
	// shifter state
	uint8_t m_shifter_lineofs = 0U;
	uint8_t m_shifter_pixelofs = 0U;
};

DECLARE_DEVICE_TYPE(STX_VIDEO, stx_video_device)
DECLARE_DEVICE_TYPE(STE_VIDEO, ste_video_device)
//DECLARE_DEVICE_TYPE(STBOOK_VIDEO, stbook_video_device)
//DECLARE_DEVICE_TYPE(TT_VIDEO, tt_video_device)

#endif // MAME_ATARI_ATARIST_V_H
