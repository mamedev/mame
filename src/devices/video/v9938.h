// license:BSD-3-Clause
// copyright-holders:Aaron Giles, Nathan Woods
/***************************************************************************

    v9938 / v9958 emulation

***************************************************************************/

#ifndef MAME_VIDEO_V9938_H
#define MAME_VIDEO_V9938_H

#pragma once

#include "screen.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(V9938, v9938_device)
DECLARE_DEVICE_TYPE(V9958, v9958_device)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> v99x8_device

class v99x8_device :    public device_t,
						public device_memory_interface,
						public device_palette_interface,
						public device_video_interface
{
public:
	auto int_cb() { return m_int_callback.bind(); }
	template <class T> void set_screen_ntsc(T &&screen)
	{
		set_screen(std::forward<T>(screen));
		m_pal_config = false;
	}
	template <class T> void set_screen_pal(T &&screen)
	{
		set_screen(std::forward<T>(screen));
		m_pal_config = true;
	}

	bitmap_rgb32 &get_bitmap() { return m_bitmap; }
	void colorbus_x_input(int mx_delta);
	void colorbus_y_input(int my_delta);
	void colorbus_button_input(bool button1, bool button2);

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);

	uint8_t vram_r();
	uint8_t status_r();
	void palette_w(uint8_t data);
	void vram_w(uint8_t data);
	void command_w(uint8_t data);
	void register_w(uint8_t data);

	void set_vram_size(uint32_t vram_size) { m_vram_size = vram_size; }

	/* RESET pin */
	void reset_line(int state) { if (state==ASSERT_LINE) device_reset(); }

	static constexpr int HTOTAL = 684;
	static constexpr int HVISIBLE = 544;
	static constexpr int VTOTAL_NTSC = 262;
	static constexpr int VTOTAL_PAL = 313;
	static constexpr int VVISIBLE_NTSC = 26 + 192 + 25;
	static constexpr int VVISIBLE_PAL = 53 + 192 + 49;
	// Looking at some youtube videos of real units on real monitors
	// there appear to be small vertical timing differences. Some (LCD)
	// monitors show the full borders, other CRT monitors seem to
	// display ~5 lines less at the top and bottom of the screen.
	static constexpr int VERTICAL_ADJUST = 5;
	static constexpr int TOP_ERASE = 13;
	static constexpr int VERTICAL_SYNC = 3;

protected:
	// construction/destruction
	v99x8_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int model);

	const address_space_config      m_space_config;
	address_space*                  m_vram_space;

	const int m_model;

	bool m_pal_config;

	// device overrides
	virtual void device_config_complete() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	// device_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	virtual void palette_init() = 0;
	virtual u32 palette_entries() const noexcept override { return 16 + 256; }

	TIMER_CALLBACK_MEMBER(update_line);

	void configure_pal_ntsc();
	void set_screen_parameters();

private:
	// internal helpers
	pen_t pen16(int index) const { return uint32_t(pen_color(index)); }
	pen_t pen256(int index) const { return uint32_t(pen_color(index + 16)); }
	void set_pen16(int index, pen_t pen) { set_pen_color(index, rgb_t(pen).set_a(index != 0 ? 0xff : 0x00)); }
	void set_pen256(int index, pen_t pen) { set_pen_color(index + 16, rgb_t(pen).set_a(index != 0 ? 0xff : 0x00)); }

	inline int position_offset(uint8_t value) { value &= 0x0f; return (value < 8) ? -value : 16 - value; }
	void reset_palette();
	void vram_write(int offset, int data);
	int vram_read(int offset);
	void check_int();
	void register_write(int reg, int data);

	void default_border(uint32_t *ln);
	void graphic7_border(uint32_t *ln);
	void graphic5_border(uint32_t *ln);
	void mode_text1(uint32_t *ln, int line);
	void mode_text2(uint32_t *ln, int line);
	void mode_multi(uint32_t *ln, int line);
	void mode_graphic1(uint32_t *ln, int line);
	void mode_graphic23(uint32_t *ln, int line);
	void mode_graphic4(uint32_t *ln, int line);
	void mode_graphic5(uint32_t *ln, int line);
	void mode_graphic6(uint32_t *ln, int line);
	void mode_graphic7(uint32_t *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yae(_PixelType *ln, int line);
//  template<typename _PixelType, int _Width> void mode_yjk(_PixelType *ln, int line);
	void mode_unknown(uint32_t *ln, int line);
	void default_draw_sprite(uint32_t *ln, uint8_t *col);
	void graphic5_draw_sprite(uint32_t *ln, uint8_t *col);
	void graphic7_draw_sprite(uint32_t *ln, uint8_t *col);

	void sprite_mode1(int line, uint8_t *col);
	void sprite_mode2(int line, uint8_t *col);
	void set_mode();
	void refresh_32(int line);
	void refresh_line(int line);

	void interrupt_start_vblank();

	int VDPVRMP(uint8_t M, int MX, int X, int Y);

	uint8_t VDPpoint5(int MXS, int SX, int SY);
	uint8_t VDPpoint6(int MXS, int SX, int SY);
	uint8_t VDPpoint7(int MXS, int SX, int SY);
	uint8_t VDPpoint8(int MXS, int SX, int SY);

	uint8_t VDPpoint(uint8_t SM, int MXS, int SX, int SY);

	void VDPpsetlowlevel(int addr, uint8_t CL, uint8_t M, uint8_t OP);

	void VDPpset5(int MXD, int DX, int DY, uint8_t CL, uint8_t OP);
	void VDPpset6(int MXD, int DX, int DY, uint8_t CL, uint8_t OP);
	void VDPpset7(int MXD, int DX, int DY, uint8_t CL, uint8_t OP);
	void VDPpset8(int MXD, int DX, int DY, uint8_t CL, uint8_t OP);

	void VDPpset(uint8_t SM, int MXD, int DX, int DY, uint8_t CL, uint8_t OP);

	int get_vdp_timing_value(const int *);

	void srch_engine();
	void line_engine();
	void lmmv_engine();
	void lmmm_engine();
	void lmcm_engine();
	void lmmc_engine();
	void hmmv_engine();
	void hmmm_engine();
	void ymmm_engine();
	void hmmc_engine();

	inline bool v9938_second_field();

	void cpu_to_vdp(uint8_t V);
	uint8_t vdp_to_cpu();
	void report_vdp_command(uint8_t Op);
	uint8_t command_unit_w(uint8_t Op);
	void update_command();

	void memmap(address_map &map) ATTR_COLD;

	// general
	int m_offset_x, m_offset_y, m_visible_y, m_mode;
	// palette
	int m_pal_write_first, m_cmd_write_first;
	uint8_t m_pal_write, m_cmd_write;
	uint8_t m_pal_reg[32], m_stat_reg[10], m_cont_reg[48], m_read_ahead;
	uint8_t m_v9958_sp_mode;

	// memory
	uint16_t m_address_latch;
	int m_vram_size;

	// interrupt
	uint8_t m_int_state;
	devcb_write_line   m_int_callback;
	int m_scanline;
	// blinking
	int m_blink, m_blink_count;
	// mouse
	int16_t m_mx_delta, m_my_delta;
	// mouse & lightpen
	uint8_t m_button_state;
	// render bitmap
	bitmap_rgb32 m_bitmap;
	// Command unit
	struct {
		int SX,SY;
		int DX,DY;
		int TX,TY;
		int NX,NY;
		int MX;
		int ASX,ADX,ANX;
		uint8_t CL;
		uint8_t LO;
		uint8_t CM;
		uint8_t MXS, MXD;
	} m_mmc;
	int  m_vdp_ops_count;
	void (v99x8_device::*m_vdp_engine)();

	struct v99x8_mode
	{
		uint8_t m;
		void (v99x8_device::*visible_32)(uint32_t*, int);
		void (v99x8_device::*border_32)(uint32_t*);
		void (v99x8_device::*sprites)(int, uint8_t*);
		void (v99x8_device::*draw_sprite_32)(uint32_t*, uint8_t*);
	};
	static const v99x8_mode s_modes[];
	emu_timer *m_line_timer;
	uint8_t m_pal_ntsc;
	int m_scanline_start;
	int m_vblank_start;
	int m_scanline_max;
	int m_height;
protected:
	static uint32_t s_pal_indYJK[0x20000];
};


class v9938_device : public v99x8_device
{
public:
	v9938_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void palette_init() override;
};

class v9958_device : public v99x8_device
{
public:
	v9958_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void palette_init() override;
};


#endif // MAME_DEVICES_VIDEO_V9938_H
