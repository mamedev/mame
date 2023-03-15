// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Megadrive / Genesis VDP */
#ifndef MAME_VIDEO_315_5313_H
#define MAME_VIDEO_315_5313_H

#pragma once

#include "video/315_5124.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"

class sega315_5313_device : public sega315_5313_mode4_device, public device_gfx_interface
{
public:
	static constexpr unsigned PALETTE_PER_FRAME = 64 * 313 * 2; // 313 total scanlines for PAL systems, *2 for interlaced

	template <typename T>
	sega315_5313_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock, T &&cpu_tag)
		: sega315_5313_device(mconfig, tag, owner, clock)
	{
		m_cpu68k.set_tag(std::forward<T>(cpu_tag));
	}

	sega315_5313_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	typedef device_delegate<void (int x, u32 priority, u32 &lineptr)> md_32x_scanline_delegate;
	typedef device_delegate<void (int scanline, int irq6)> md_32x_interrupt_delegate;
	typedef device_delegate<void (int scanline)> md_32x_scanline_helper_delegate;

	auto snd_irq() { return m_sndirqline_callback.bind(); }
	auto lv6_irq() { return m_lv6irqline_callback.bind(); }
	auto lv4_irq() { return m_lv4irqline_callback.bind(); }

	void set_lcm_scaling(bool lcm_scaling) { m_lcm_scaling = lcm_scaling; }
	void set_alt_timing(int use_alt_timing) { m_use_alt_timing = use_alt_timing; }
	void set_pal_write_base(int palwrite_base) { m_palwrite_base = palwrite_base; }
	template <typename T> void set_ext_palette(T &&tag) { m_ext_palette.set_tag(std::forward<T>(tag)); }

	// Temporary solution while 32x VDP mixing and scanline interrupting is moved outside MD VDP
	template <typename... T> void set_md_32x_scanline(T &&... args) { m_32x_scanline_func.set(std::forward<T>(args)...); }
	template <typename... T> void set_md_32x_interrupt(T &&... args) { m_32x_interrupt_func.set(std::forward<T>(args)...); }
	template <typename... T> void set_md_32x_scanline_helper(T &&... args) { m_32x_scanline_helper_func.set(std::forward<T>(args)...); }

	int m_use_alt_timing; // use MAME scanline timer instead, render only one scanline to a single line buffer, to be rendered by a partial update call.. experimental

	int m_palwrite_base; // if we want to write to the actual MAME palette..

	u16 vdp_r(offs_t offset, u16 mem_mask = ~0);
	void vdp_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void vram_w(offs_t offset, u16 data, u16 mem_mask = ~0)
	{
		offset &= 0x7fff;
		COMBINE_DATA(&m_vram[offset]);
		gfx(0)->mark_dirty(offset / ((8*8*4) / 16));
		gfx(1)->mark_dirty(offset / ((8*16*4) / 16));
		gfx(2)->mark_dirty(offset / ((8*8*4) / 16));
		gfx(3)->mark_dirty(offset / ((8*16*4) / 16));
		gfx(4)->mark_dirty(offset / ((8*8*4) / 16));
		gfx(5)->mark_dirty(offset / ((8*16*4) / 16));
	}

	device_palette_interface *gfx_palette() { return m_gfx_palette; }
	device_palette_interface *gfx_palette_shadow() { return m_gfx_palette_shadow; }
	device_palette_interface *gfx_palette_hilight() { return m_gfx_palette_hilight; }

	int get_scanline_counter();

	TIMER_CALLBACK_MEMBER(render_scanline);
	void vdp_handle_scanline_callback(int scanline);
	TIMER_CALLBACK_MEMBER(irq6_on_timer_callback);
	TIMER_CALLBACK_MEMBER(irq4_on_timer_callback);
	void vdp_handle_eof();
	void device_reset_old();
	void vdp_clear_irq6_pending() { m_irq6_pending = 0; }
	void vdp_clear_irq4_pending() { m_irq4_pending = 0; }

	// set some VDP variables at start (shall be moved to a device interface?)
	void set_scanline_counter(int scanline) { m_scanline_counter = scanline; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }
	void set_framerate(int rate) { m_framerate = rate; }
	void set_vdp_pal(bool pal) { m_vdp_pal = pal ? 1 : 0; }
	void set_use_cram(int cram) { m_use_cram = cram; }
	void set_dma_delay(int delay) { m_dma_delay = delay; }
	double get_framerate() { return has_screen() ? screen().frame_period().as_hz() : double(m_framerate); }
	int get_imode() { return m_imode; }

	void vdp_clear_bitmap()
	{
		if (m_render_bitmap)
			m_render_bitmap->fill(0);
	}

	std::unique_ptr<bitmap_rgb32> m_render_bitmap;
	std::unique_ptr<u32[]> m_render_line;
	std::unique_ptr<u16[]> m_render_line_raw;

	TIMER_DEVICE_CALLBACK_MEMBER(megadriv_scanline_timer_callback_alt_timing);
	TIMER_DEVICE_CALLBACK_MEMBER(megadriv_scanline_timer_callback);
	timer_device* m_megadriv_scanline_timer;

	inline u16 vdp_get_word_from_68k_mem(u32 source);

protected:
	virtual void device_post_load() override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;

	// called when we hit 240 and 241 (used to control the z80 irq line on genesis, or the main irq on c2)
	devcb_write_line m_sndirqline_callback;
	devcb_write_line m_lv6irqline_callback;
	devcb_write_line m_lv4irqline_callback;

	md_32x_scanline_delegate m_32x_scanline_func;
	md_32x_interrupt_delegate m_32x_interrupt_func;
	md_32x_scanline_helper_delegate m_32x_scanline_helper_func;

	virtual int screen_hpos() override;
private:
	// vdp code defines
	const u8 CODE_DMA()         { return m_vdp_code & 0x20; }
	const u8 CODE_VRAM_COPY()   { return m_vdp_code & 0x10; }
	const u8 CODE_VRAM_READ()   { return (m_vdp_code & 0x0f) == 0x00; }
	const u8 CODE_VRAM_WRITE()  { return (m_vdp_code & 0x0f) == 0x01; }
	const u8 CODE_CRAM_WRITE()  { return (m_vdp_code & 0x0f) == 0x03; }
	const u8 CODE_VSRAM_READ()  { return (m_vdp_code & 0x0f) == 0x04; }
	const u8 CODE_VSRAM_WRITE() { return (m_vdp_code & 0x0f) == 0x05; }
	const u8 CODE_CRAM_READ()   { return (m_vdp_code & 0x0f) == 0x08; }
	//const u8 CODE_VRAM_READ_BYTE() { return (m_vdp_code & 0x0f) == 0x0c; } // undocumented, unhandled

	// nametable
	struct nametable_t {
		gfx_element *gfx;
		const u8* addr;
		bool xflip;
		bool yflip;
		u16 colour;
		u16 pri;
	};
	void get_window_tilebase(int &tile_base, u16 base, int vcolumn, int window_hsize, int hcolumn);
	void get_vcolumn_tilebase(int &vcolumn, int &tile_base, u16 base, int vscroll, int scanline, int vsize, int hsize, int hcolumn);
	void get_nametable(gfx_element *tile_gfx, u16 tile_base, nametable_t &tile, int vcolumn);
	inline void draw_tile(nametable_t tile, int start, int end, int &dpos, bool is_fg);

	inline u8 get_hres();
	int m_command_pending; // 2nd half of command pending..
	u16 m_command_part1;
	u16 m_command_part2;
	u8  m_vdp_code;
	u16 m_vdp_address;
	u8  m_vram_fill_pending;
	u16 m_vram_fill_length;
	int m_irq4counter;
	int m_imode_odd_frame;
	int m_sprite_collision;
	int m_irq6_pending;
	int m_irq4_pending;
	int m_scanline_counter;
	int m_vblank_flag;

	int m_imode;

	bool m_lcm_scaling;
	int m_visible_scanlines;
	int m_irq6_scanline;
	int m_z80irq_scanline;
	int m_total_scanlines;
	// this is only set at init: 262 for PAL, 313 for NTSC
	int m_base_total_scanlines;

	int m_framerate;
	int m_vdp_pal;
	int m_use_cram; // c2 uses it's own palette ram, so it sets this to 0
	int m_dma_delay;    // SVP and SegaCD have some 'lag' in DMA transfers

	std::unique_ptr<u16[]> m_regs;
	std::unique_ptr<u16[]> m_vram;
	std::unique_ptr<u16[]> m_cram;
	std::unique_ptr<u16[]> m_vsram;
	/* The VDP keeps a 0x400 byte on-chip cache of the Sprite Attribute Table
	   to speed up processing, Castlevania Bloodlines abuses this on the upside down level */
	std::unique_ptr<u16[]> m_internal_sprite_attribute_table;

	// these are used internally by the VDP to schedule when after the start of a scanline
	// to trigger the various interrupts / rendering to our bitmap, bit of a hack really
	emu_timer* m_irq6_on_timer;
	emu_timer* m_irq4_on_timer;
	emu_timer* m_render_timer;

	u16 vdp_vram_r(void);
	u16 vdp_vsram_r(void);
	u16 vdp_cram_r(void);

	void insta_68k_to_cram_dma(u32 source, u16 length);
	void insta_68k_to_vsram_dma(u32 source, u16 length);
	void insta_68k_to_vram_dma(u32 source, int length);
	void insta_vram_copy(u32 source, u16 length);

	void vdp_vram_write(u16 data);
	void vdp_cram_write(u16 data);
	void write_cram_value(int offset, int data);
	void vdp_vsram_write(u16 data);

	void vdp_address_inc();
	void vdp_set_register(int regnum, u8 value);

	void handle_dma_bits();

	u16 get_hposition();
	u16 megadriv_read_hv_counters();

	u16 ctrl_port_r();
	u16 data_port_r();
	void data_port_w(int data);
	void ctrl_port_w(int data);
	void update_code_and_address(void);


	void render_spriteline_to_spritebuffer(int scanline);
	void render_videoline_to_videobuffer(int scanline);
	void render_videobuffer_to_screenbuffer(int scanline);

	/* variables used during emulation - not saved */
	std::unique_ptr<u8[]> m_sprite_renderline;
	std::unique_ptr<u8[]> m_highpri_renderline;
	std::unique_ptr<u32[]> m_video_renderline;
	std::unique_ptr<u16[]> m_palette_lookup;

	address_space *m_space68k;
	required_device<m68000_base_device> m_cpu68k;
	optional_device<palette_device> m_ext_palette;

	// debug functions
	required_device<palette_device> m_gfx_palette;
	required_device<palette_device> m_gfx_palette_shadow;
	required_device<palette_device> m_gfx_palette_hilight;
};


DECLARE_DEVICE_TYPE(SEGA315_5313, sega315_5313_device)

#endif // MAME_VIDEO_315_5313_H
