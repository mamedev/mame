// license:BSD-3-Clause
// copyright-holders:Antoine Mine
/**********************************************************************

  Copyright (C) Antoine Mine' 2006

  Thomson 8-bit computers

**********************************************************************/

#ifndef MAME_THOMSON_TO_VIDEO_H
#define MAME_THOMSON_TO_VIDEO_H

#pragma once


/***************************** dimensions **************************/

/*
   TO7 video:
   one line (64 us) =
      56 left border pixels ( 7 us)
   + 320 active pixels (40 us)
   +  56 right border pixels ( 7 us)
   +     horizontal retrace (10 us)

   one image (20 ms) =
      47 top border lines (~3 ms)
   + 200 active lines (12.8 ms)
   +  47 bottom border lines (~3 ms)
   +     vertical retrace (~1 ms)

   TO9 and up introduced a half (160 pixels) and double (640 pixels)
   horizontal mode, but still in 40 us (no change in refresh rate).
*/


/* original screen dimension (may be different from emulated screen!) */
#define THOM_ACTIVE_WIDTH  320
#define THOM_BORDER_WIDTH   56
#define THOM_ACTIVE_HEIGHT 200
#define THOM_BORDER_HEIGHT  47
#define THOM_TOTAL_WIDTH   432
#define THOM_TOTAL_HEIGHT  294

/* Emulated screen dimension may be doubled to allow hi-res 640x200 mode.
   Emulated screen can have smaller borders.
 */


class thomson_video_device : public device_t, public device_video_interface
{
public:
	void set_is_mo(bool mo) { m_is_mo = mo; }
	void set_lightpen_decal(uint8_t decal) { m_lightpen_decal = decal; }
	void set_lightpen_steps(uint8_t nb) { m_lightpen_nb = nb; }

	template <typename... T> void set_vram_page_cb(T &&... args) { m_vram_page_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_lightpen_step_cb(T &&... args) { m_lightpen_step_cb.set(std::forward<T>(args)...); }
	auto init_cb() { return m_init_cb.bind(); }
	auto int_50hz_cb() { return m_int_50hz_cb.bind(); }

	void set_fixed_palette(const uint16_t *pal);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	bool lightpen_active() const { return m_lightpen; }
	void set_lightpen_active(bool lp) { m_lightpen = lp; }
	unsigned to7_lightpen_gpl();

	void set_border_color( unsigned index );
	void set_palette( unsigned index, uint16_t color );
	void mark_dirty(offs_t offset) { m_vmem_dirty[offset / 40] = true; }

	void to770_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4althalf_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page1_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page2_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlayhalf_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay3_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16alt_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to770_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mo5alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap4althalf_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page1_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void page2_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlayhalf_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void overlay3_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void bitmap16alt_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to9_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_to9_scandraw_16( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void to9_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );
	void mode80_to9_scandraw_8( uint8_t* vram, uint16_t* dst, uint16_t* pal, int org, int len );

protected:
	struct thom_vsignal {
		unsigned count;  /* pixel counter */
		unsigned init;   /* 1 -> active vertical windos, 0 -> border/VBLANK */
		unsigned inil;   /* 1 -> active horizontal window, 0 -> border/HBLANK */
		unsigned lt3;    /* bit 3 of us counter */
		unsigned line;   /* line counter */
	};

	thomson_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_config_complete() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	bool m_is_mo = false;

private:
	TIMER_CALLBACK_MEMBER( lightpen_step );
	TIMER_CALLBACK_MEMBER( scanline_start );
	TIMER_CALLBACK_MEMBER( set_init );
	TIMER_CALLBACK_MEMBER( synlt_50hz );

	void thom_vblank(screen_device &screen, bool state);

	required_ioport m_io_lightpen_x;
	required_ioport m_io_lightpen_y;
	required_ioport m_io_vconfig;

protected:
	uint8_t m_lightpen_step = 0;
	uint8_t m_lightpen = 0;

	/* We allow choosing dynamically:
	   - the border size
	   - whether we use 640 pixels or 320 pixels in an active row
	   (now this is automatically chosen by default for each frame)
	*/
	uint16_t m_bwidth = 0;
	uint16_t m_bheight = 0;
	/* border size */
	uint8_t  m_hires = 0;
	/* 0 = low res: 320x200 active area (faster)
	   1 = hi res:  640x200 active area (can represent all video modes)
	*/
	bool m_hires_better = false;
	/* 1 = a 640 mode was used in the last frame */
	/* we use our own video timing to precisely cope with VBLANK and HBLANK */
	emu_timer *m_video_timer = nullptr; /* time elapsed from beginning of frame */
	uint8_t m_lightpen_decal = 0;
	/* number of lightpen call-backs per frame */
	int m_lightpen_nb = 0;
	/* called m_lightpen_nb times */
	emu_timer *m_lightpen_timer = nullptr;
	emu_timer *m_scanline_timer = nullptr; /* scan-line update */
	uint16_t m_last_pal[16]{};   /* palette at last scanline start */
	uint16_t m_pal[16]{};        /* current palette */
	bool     m_pal_changed = false;    /* whether pal != old_pal */
	uint8_t  m_border_index = 0;   /* current border color index */
	/* the left and right border color for each row (including top and bottom
	   border rows); -1 means unchanged wrt last scanline
	*/
	int16_t m_border_l[THOM_TOTAL_HEIGHT+1]{};
	int16_t m_border_r[THOM_TOTAL_HEIGHT+1]{};
	/* active area, updated one scan-line at a time every 64us,
	   then blitted in screen_update
	*/
	uint16_t m_vbody[640*200]{};
	uint8_t m_vmode = 0; /* current vide mode */
	uint8_t m_vpage = 0; /* current video page */
	/* this stores the video mode & page at each GPL in the current line
	   (-1 means unchanged)
	*/
	int16_t m_vmodepage[41]{};
	bool m_vmodepage_changed = false;
	/* one dirty flag for each video memory line */
	bool m_vmem_dirty[205]{};
	/* set to 1 if undirty scanlines need to be redrawn due to other video state
	   changes */
	bool m_vstate_dirty = false;
	bool m_vstate_last_dirty = false;
	emu_timer *m_init_timer = nullptr;
	emu_timer *m_synlt_timer = nullptr;

	device_delegate<uint8_t* (int page)> m_vram_page_cb;
	device_delegate<void (int step)> m_lightpen_step_cb;
	devcb_write_line m_init_cb;
	devcb_write_line m_int_50hz_cb;

	bool update_screen_size();
	unsigned video_elapsed();
	thom_vsignal get_vsignal();
	void get_lightpen_pos( int*x, int* y );
	thom_vsignal get_lightpen_vsignal( int xdec, int ydec, int xdec2 );
	bool mode_is_hires( int mode ) const;
	void border_changed();
	void gplinfo_changed();
	void set_video_mode( unsigned mode );
	void set_video_page( unsigned page );
};

class to7_video_device : public thomson_video_device
{
public:
	to7_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_reset() override ATTR_COLD;
};

class to770_video_device : public thomson_video_device
{
public:
	to770_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	uint8_t gatearray_r(offs_t offset);
	void gatearray_w(offs_t offset, uint8_t data);

protected:
	virtual void device_reset() override ATTR_COLD;
};

class to9_video_device : public thomson_video_device
{
public:
	to9_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	uint8_t gatearray_r(offs_t offset);
	void gatearray_w(offs_t offset, uint8_t data);
	void video_mode_w(uint8_t data);
	void border_color_w(uint8_t data);

protected:
	to9_video_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_reset() override ATTR_COLD;

	int m_style = 0;
};

class to8_video_device : public to9_video_device
{
public:
	to8_video_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	template <typename... T> void set_update_ram_bank_cb(T &&... args) { m_update_ram_bank_cb.set(std::forward<T>(args)...); }
	template <typename... T> void set_update_cart_bank_cb(T &&... args) { m_update_cart_bank_cb.set(std::forward<T>(args)...); }
	auto lightpen_intr_cb() { return m_lightpen_intr_cb.bind(); }

	uint8_t gatearray_r(offs_t offset);
	void gatearray_w(offs_t offset, uint8_t data);
	void sys2_w(uint8_t data);

	uint8_t reg_ram() const { return m_reg_ram; }
	uint8_t reg_cart() const { return m_reg_cart; }
	uint8_t reg_sys1() const { return m_reg_sys1; }
	uint8_t reg_sys2() const { return m_reg_sys2; }

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	void lightpen_cb( int step );

	device_delegate<void ()> m_update_ram_bank_cb;
	device_delegate<void ()> m_update_cart_bank_cb;
	devcb_write_line m_lightpen_intr_cb;

	uint8_t m_reg_ram = 0;
	uint8_t m_reg_cart = 0;
	uint8_t m_reg_sys1 = 0;
	uint8_t m_reg_sys2 = 0;
	uint8_t m_lightpen_intr = 0;
};

// device type definitions
DECLARE_DEVICE_TYPE(TO7_VIDEO, to7_video_device)
DECLARE_DEVICE_TYPE(TO770_VIDEO, to770_video_device)
DECLARE_DEVICE_TYPE(TO9_VIDEO, to9_video_device)
DECLARE_DEVICE_TYPE(TO8_VIDEO, to8_video_device)

#endif // MAME_THOMSON_TO_VIDEO_H
