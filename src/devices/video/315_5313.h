// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Sega Megadrive / Genesis VDP */

#pragma once

#include "video/315_5124.h"
#include "cpu/m68000/m68000.h"


/*  The VDP occupies addresses C00000h to C0001Fh.

 C00000h    -   Data port (8=r/w, 16=r/w)
 C00002h    -   Data port (mirror)
 C00004h    -   Control port (8=r/w, 16=r/w)
 C00006h    -   Control port (mirror)
 C00008h    -   HV counter (8/16=r/o)
 C0000Ah    -   HV counter (mirror)
 C0000Ch    -   HV counter (mirror)
 C0000Eh    -   HV counter (mirror)
 C00011h    -   SN76489 PSG (8=w/o)
 C00013h    -   SN76489 PSG (mirror)
 C00015h    -   SN76489 PSG (mirror)
 C00017h    -   SN76489 PSG (mirror)
*/

#define MEGADRIV_VDP_VRAM(address) m_vram[(address)&0x7fff]



/*

 $00 - Mode Set Register No. 1
 -----------------------------

 d7 - No effect
 d6 - No effect
 d5 - No effect
 d4 - IE1 (Horizontal interrupt enable)
 d3 - 1= Invalid display setting
 d2 - Palette select
 d1 - M3 (HV counter latch enable)
 d0 - Display disable

 */

#define MEGADRIVE_REG0_UNUSED          ((m_regs[0x00]&0xc0)>>6)
#define MEGADRIVE_REG0_BLANK_LEFT      ((m_regs[0x00]&0x20)>>5) // like SMS, not used by any commercial games?
#define MEGADRIVE_REG0_IRQ4_ENABLE     ((m_regs[0x00]&0x10)>>4)
#define MEGADRIVE_REG0_INVALID_MODE    ((m_regs[0x00]&0x08)>>3) // invalid display mode, unhandled
#define MEGADRIVE_REG0_SPECIAL_PAL     ((m_regs[0x00]&0x04)>>2) // strange palette mode, unhandled
#define MEGADRIVE_REG0_HVLATCH_ENABLE  ((m_regs[0x00]&0x02)>>1) // HV Latch, used by lightgun games
#define MEGADRIVE_REG0_DISPLAY_DISABLE ((m_regs[0x00]&0x01)>>0)

/*

 $01 - Mode Set Register No. 2
 -----------------------------

 d7 - TMS9918 / Genesis display select
 d6 - DISP (Display Enable)
 d5 - IE0 (Vertical Interrupt Enable)
 d4 - M1 (DMA Enable)
 d3 - M2 (PAL / NTSC)
 d2 - SMS / Genesis display select
 d1 - 0 (No effect)
 d0 - 0 (See notes)

*/

#define MEGADRIVE_REG01_TMS9918_SELECT  ((m_regs[0x01]&0x80)>>7)
#define MEGADRIVE_REG01_DISP_ENABLE     ((m_regs[0x01]&0x40)>>6)
#define MEGADRIVE_REG01_IRQ6_ENABLE     ((m_regs[0x01]&0x20)>>5)
#define MEGADRIVE_REG01_DMA_ENABLE      ((m_regs[0x01]&0x10)>>4)
#define MEGADRIVE_REG01_240_LINE        ((m_regs[0x01]&0x08)>>3)
#define MEGADRIVE_REG01_SMS_SELECT      ((m_regs[0x01]&0x04)>>2)
#define MEGADRIVE_REG01_UNUSED          ((m_regs[0x01]&0x02)>>1)
#define MEGADRIVE_REG01_STRANGE_VIDEO   ((m_regs[0x01]&0x01)>>0) // unhandled, does strange things to the display

#define MEGADRIVE_REG02_UNUSED1         ((m_regs[0x02]&0xc0)>>6)
#define MEGADRIVE_REG02_PATTERN_ADDR_A  ((m_regs[0x02]&0x38)>>3)
#define MEGADRIVE_REG02_UNUSED2         ((m_regs[0x02]&0x07)>>0)

#define MEGADRIVE_REG03_UNUSED1         ((m_regs[0x03]&0xc0)>>6)
#define MEGADRIVE_REG03_PATTERN_ADDR_W  ((m_regs[0x03]&0x3e)>>1)
#define MEGADRIVE_REG03_UNUSED2         ((m_regs[0x03]&0x01)>>0)

#define MEGADRIVE_REG04_UNUSED          ((m_regs[0x04]&0xf8)>>3)
#define MEGADRIVE_REG04_PATTERN_ADDR_B  ((m_regs[0x04]&0x07)>>0)

#define MEGADRIVE_REG05_UNUSED          ((m_regs[0x05]&0x80)>>7)
#define MEGADRIVE_REG05_SPRITE_ADDR     ((m_regs[0x05]&0x7f)>>0)

/* 6? */

#define MEGADRIVE_REG07_UNUSED          ((m_regs[0x07]&0xc0)>>6)
#define MEGADRIVE_REG07_BGCOLOUR        ((m_regs[0x07]&0x3f)>>0)

/* 8? */
/* 9? */

#define MEGADRIVE_REG0A_HINT_VALUE      ((m_regs[0x0a]&0xff)>>0)

#define MEGADRIVE_REG0B_UNUSED          ((m_regs[0x0b]&0xf0)>>4)
#define MEGADRIVE_REG0B_IRQ2_ENABLE     ((m_regs[0x0b]&0x08)>>3)
#define MEGADRIVE_REG0B_VSCROLL_MODE    ((m_regs[0x0b]&0x04)>>2)
#define MEGADRIVE_REG0B_HSCROLL_MODE    ((m_regs[0x0b]&0x03)>>0)

#define MEGADRIVE_REG0C_RS0             ((m_regs[0x0c]&0x80)>>7)
#define MEGADRIVE_REG0C_UNUSED1         ((m_regs[0x0c]&0x40)>>6)
#define MEGADRIVE_REG0C_SPECIAL         ((m_regs[0x0c]&0x20)>>5)
#define MEGADRIVE_REG0C_UNUSED2         ((m_regs[0x0c]&0x10)>>4)
#define MEGADRIVE_REG0C_SHADOW_HIGLIGHT ((m_regs[0x0c]&0x08)>>3)
#define MEGADRIVE_REG0C_INTERLEAVE      ((m_regs[0x0c]&0x06)>>1)
#define MEGADRIVE_REG0C_RS1             ((m_regs[0x0c]&0x01)>>0)

#define MEGADRIVE_REG0D_UNUSED          ((m_regs[0x0d]&0xc0)>>6)
#define MEGADRIVE_REG0D_HSCROLL_ADDR    ((m_regs[0x0d]&0x3f)>>0)

/* e? */

#define MEGADRIVE_REG0F_AUTO_INC        ((m_regs[0x0f]&0xff)>>0)

#define MEGADRIVE_REG10_UNUSED1        ((m_regs[0x10]&0xc0)>>6)
#define MEGADRIVE_REG10_VSCROLL_SIZE   ((m_regs[0x10]&0x30)>>4)
#define MEGADRIVE_REG10_UNUSED2        ((m_regs[0x10]&0x0c)>>2)
#define MEGADRIVE_REG10_HSCROLL_SIZE   ((m_regs[0x10]&0x03)>>0)

#define MEGADRIVE_REG11_WINDOW_RIGHT   ((m_regs[0x11]&0x80)>>7)
#define MEGADRIVE_REG11_UNUSED         ((m_regs[0x11]&0x60)>>5)
#define MEGADRIVE_REG11_WINDOW_HPOS      ((m_regs[0x11]&0x1f)>>0)

#define MEGADRIVE_REG12_WINDOW_DOWN    ((m_regs[0x12]&0x80)>>7)
#define MEGADRIVE_REG12_UNUSED         ((m_regs[0x12]&0x60)>>5)
#define MEGADRIVE_REG12_WINDOW_VPOS      ((m_regs[0x12]&0x1f)>>0)

#define MEGADRIVE_REG13_DMALENGTH1     ((m_regs[0x13]&0xff)>>0)

#define MEGADRIVE_REG14_DMALENGTH2      ((m_regs[0x14]&0xff)>>0)

#define MEGADRIVE_REG15_DMASOURCE1      ((m_regs[0x15]&0xff)>>0)
#define MEGADRIVE_REG16_DMASOURCE2      ((m_regs[0x16]&0xff)>>0)

#define MEGADRIVE_REG17_DMASOURCE3      ((m_regs[0x17]&0xff)>>0)
#define MEGADRIVE_REG17_DMATYPE         ((m_regs[0x17]&0xc0)>>6)
#define MEGADRIVE_REG17_UNUSED          ((m_regs[0x17]&0x3f)>>0)


#define MCFG_SEGA315_5313_IS_PAL(_bool) \
	sega315_5313_device::set_signal_type(*device, _bool);

#define MCFG_SEGA315_5313_INT_CB(_devcb) \
	devcb = &sega315_5313_device::set_int_callback(*device, DEVCB_##_devcb);

#define MCFG_SEGA315_5313_PAUSE_CB(_devcb) \
	devcb = &sega315_5313_device::set_pause_callback(*device, DEVCB_##_devcb);

#define MCFG_SEGA315_5313_SND_IRQ_CALLBACK(_write) \
	devcb = &sega315_5313_device::set_sndirqline_callback(*device, DEVCB_##_write);

#define MCFG_SEGA315_5313_LV6_IRQ_CALLBACK(_write) \
	devcb = &sega315_5313_device::set_lv6irqline_callback(*device, DEVCB_##_write);

#define MCFG_SEGA315_5313_LV4_IRQ_CALLBACK(_write) \
	devcb = &sega315_5313_device::set_lv4irqline_callback(*device, DEVCB_##_write);

#define MCFG_SEGA315_5313_ALT_TIMING(_data) \
	sega315_5313_device::set_alt_timing(*device, _data);

#define MCFG_SEGA315_5313_PAL_WRITE_BASE(_data) \
	sega315_5313_device::set_palwrite_base(*device, _data);

#define MCFG_SEGA315_5313_PALETTE(_palette_tag) \
	sega315_5313_device::static_set_palette_tag(*device, "^" _palette_tag);


// Temporary solution while 32x VDP mixing and scanline interrupting is moved outside MD VDP
typedef device_delegate<void (int x, UINT32 priority, UINT16 &lineptr)> md_32x_scanline_delegate;
typedef device_delegate<void (int scanline, int irq6)> md_32x_interrupt_delegate;
typedef device_delegate<void (int scanline)> md_32x_scanline_helper_delegate;

#define MCFG_SEGA315_5313_32X_SCANLINE_CB(_class, _method) \
	sega315_5313_device::set_md_32x_scanline(*device, md_32x_scanline_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SEGA315_5313_32X_INTERRUPT_CB(_class, _method) \
	sega315_5313_device::set_md_32x_interrupt(*device, md_32x_interrupt_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_SEGA315_5313_32X_SCANLINE_HELPER_CB(_class, _method) \
	sega315_5313_device::set_md_32x_scanline_helper(*device, md_32x_scanline_helper_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));


class sega315_5313_device : public sega315_5124_device
{
public:
	sega315_5313_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	template<class _Object> static devcb_base &set_sndirqline_callback(device_t &device, _Object object) { return downcast<sega315_5313_device &>(device).m_sndirqline_callback.set_callback(object); }
	template<class _Object> static devcb_base &set_lv6irqline_callback(device_t &device, _Object object) { return downcast<sega315_5313_device &>(device).m_lv6irqline_callback.set_callback(object); }
	template<class _Object> static devcb_base &set_lv4irqline_callback(device_t &device, _Object object) { return downcast<sega315_5313_device &>(device).m_lv4irqline_callback.set_callback(object); }
	static void set_alt_timing(device_t &device, int use_alt_timing);
	static void set_palwrite_base(device_t &device, int palwrite_base);
	static void static_set_palette_tag(device_t &device, const char *tag);

	static void set_md_32x_scanline(device_t &device, md_32x_scanline_delegate callback) { downcast<sega315_5313_device &>(device).m_32x_scanline_func = callback; }
	static void set_md_32x_interrupt(device_t &device, md_32x_interrupt_delegate callback) { downcast<sega315_5313_device &>(device).m_32x_interrupt_func = callback; }
	static void set_md_32x_scanline_helper(device_t &device, md_32x_scanline_helper_delegate callback) { downcast<sega315_5313_device &>(device).m_32x_scanline_helper_func = callback; }

	int m_use_alt_timing; // use MAME scanline timer instead, render only one scanline to a single line buffer, to be rendered by a partial update call.. experimental

	int m_palwrite_base; // if we want to write to the actual MAME palette..

	DECLARE_READ16_MEMBER( vdp_r );
	DECLARE_WRITE16_MEMBER( vdp_w );

	int get_scanline_counter();

	void render_scanline();
	void vdp_handle_scanline_callback(int scanline);
	void vdp_handle_irq6_on_timer_callback(int param);
	void vdp_handle_irq4_on_timer_callback(int param);
	void vdp_handle_eof();
	void device_reset_old();
	void vdp_clear_irq6_pending(void) { m_irq6_pending = 0; };
	void vdp_clear_irq4_pending(void) { m_irq4_pending = 0; };

	// set some VDP variables at start (shall be moved to a device interface?)
	void set_scanline_counter(int scanline) { m_scanline_counter = scanline; }
	void set_total_scanlines(int total) { m_base_total_scanlines = total; }
	void set_framerate(int rate) { m_framerate = rate; }
	void set_vdp_pal(bool pal) { m_vdp_pal = pal ? 1 : 0; }
	void set_use_cram(int cram) { m_use_cram = cram; }
	void set_dma_delay(int delay) { m_dma_delay = delay; }
	int get_framerate() { return m_framerate; }
	int get_imode() { return m_imode; }


	void vdp_clear_bitmap(void)
	{
		if (m_render_bitmap)
			m_render_bitmap->fill(0);
	}

	bitmap_ind16* m_render_bitmap;
	UINT16* m_render_line;
	UINT16* m_render_line_raw;

	TIMER_DEVICE_CALLBACK_MEMBER( megadriv_scanline_timer_callback_alt_timing );
	TIMER_DEVICE_CALLBACK_MEMBER( megadriv_scanline_timer_callback );
	timer_device* m_megadriv_scanline_timer;

	inline UINT16 vdp_get_word_from_68k_mem(UINT32 source);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// called when we hit 240 and 241 (used to control the z80 irq line on genesis, or the main irq on c2)
	devcb_write_line m_sndirqline_callback;
	devcb_write_line m_lv6irqline_callback;
	devcb_write_line m_lv4irqline_callback;

	md_32x_scanline_delegate m_32x_scanline_func;
	md_32x_interrupt_delegate m_32x_interrupt_func;
	md_32x_scanline_helper_delegate m_32x_scanline_helper_func;

private:

	int m_command_pending; // 2nd half of command pending..
	UINT16 m_command_part1;
	UINT16 m_command_part2;
	UINT8  m_vdp_code;
	UINT16 m_vdp_address;
	UINT8 m_vram_fill_pending;
	UINT16 m_vram_fill_length;
	int m_irq4counter;
	int m_imode_odd_frame;
	int m_sprite_collision;
	int m_irq6_pending;
	int m_irq4_pending;
	int m_scanline_counter;
	int m_vblank_flag;

	int m_imode;

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

	UINT16* m_regs;
	UINT16* m_vram;
	UINT16* m_cram;
	UINT16* m_vsram;
	/* The VDP keeps a 0x400 byte on-chip cache of the Sprite Attribute Table
	   to speed up processing, Castlevania Bloodlines abuses this on the upside down level */
	UINT16* m_internal_sprite_attribute_table;

	// these are used internally by the VDP to schedule when after the start of a scanline
	// to trigger the various interrupts / rendering to our bitmap, bit of a hack really
	emu_timer* m_irq6_on_timer;
	emu_timer* m_irq4_on_timer;
	emu_timer* m_render_timer;

	UINT16 vdp_vram_r(void);
	UINT16 vdp_vsram_r(void);
	UINT16 vdp_cram_r(void);

	void insta_68k_to_cram_dma(UINT32 source,UINT16 length);
	void insta_68k_to_vsram_dma(UINT32 source,UINT16 length);
	void insta_68k_to_vram_dma(UINT32 source,int length);
	void insta_vram_copy(UINT32 source, UINT16 length);

	void vdp_vram_write(UINT16 data);
	void vdp_cram_write(UINT16 data);
	void write_cram_value(int offset, int data);
	void vdp_vsram_write(UINT16 data);

	void vdp_set_register(int regnum, UINT8 value);

	void handle_dma_bits();

	UINT16 get_hposition();
	UINT16 megadriv_read_hv_counters();

	UINT16 ctrl_port_r();
	UINT16 data_port_r();
	void data_port_w(int data);
	void ctrl_port_w(int data);
	void update_code_and_address(void);


	void render_spriteline_to_spritebuffer(int scanline);
	void render_videoline_to_videobuffer(int scanline);
	void render_videobuffer_to_screenbuffer(int scanline);

	/* variables used during emulation - not saved */
	UINT8* m_sprite_renderline;
	UINT8* m_highpri_renderline;
	UINT32* m_video_renderline;
	UINT16* m_palette_lookup;
	UINT16* m_palette_lookup_sprite; // for C2
	UINT16* m_palette_lookup_shadow;
	UINT16* m_palette_lookup_highlight;

	address_space *m_space68k;
	m68000_base_device* m_cpu68k;
};


extern const device_type SEGA315_5313;
