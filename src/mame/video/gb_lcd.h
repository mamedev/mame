// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/*****************************************************************************
 *
 * video/gb_lcd.h
 *
 ****************************************************************************/

#ifndef __GB_LCD_H__
#define __GB_LCD_H__

#include "emu.h"


#define _NR_GB_VID_REGS     0x40


struct layer_struct {
	UINT8  enabled;
	UINT8  *bg_tiles;
	UINT8  *bg_map;
	UINT8  xindex;
	UINT8  xshift;
	UINT8  xstart;
	UINT8  xend;
	/* GBC specific */
	UINT8  *gbc_map;
	INT16  bgline;
};


class gb_lcd_device :   public device_t,
						public device_video_interface
{
public:
	gb_lcd_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	gb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);


	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(oam_r);
	DECLARE_WRITE8_MEMBER(oam_w);
	virtual DECLARE_READ8_MEMBER(video_r);
	virtual DECLARE_WRITE8_MEMBER(video_w);

	// FIXME: remove it when proper sgb support is added
	void set_sgb_hack(bool val) { m_sgb_border_hack = val ? 1 : 0; }

protected:
	inline void plot_pixel(bitmap_ind16 &bitmap, int x, int y, UINT32 color);

	void select_sprites();
	virtual void update_sprites();
	virtual void update_scanline();

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	void common_start();
	void common_reset();

	// pointer to the main system
	cpu_device *m_maincpu;

	// state variables
	bitmap_ind16 m_bitmap;

	UINT8 m_sgb_atf_data[4050];       /* (SGB) Attributes files */
	UINT32 m_sgb_atf;
	UINT16 m_sgb_pal_data[4096];
	UINT8 m_sgb_pal_map[20][18];
	UINT16 m_sgb_pal[128];
	std::unique_ptr<UINT8[]> m_sgb_tile_data;
	UINT8 m_sgb_tile_map[2048];
	UINT8 m_sgb_window_mask;

	// this is temporarily needed for a bunch of games which draw the border differently...
	int m_sgb_border_hack;

	int m_window_lines_drawn;

	UINT8   m_vid_regs[_NR_GB_VID_REGS];
	UINT8   m_bg_zbuf[160];

	UINT16  m_cgb_bpal[32];   /* CGB current background palette table */
	UINT16  m_cgb_spal[32];   /* CGB current sprite palette table */

	UINT8   m_gb_bpal[4];     /* Background palette */
	UINT8   m_gb_spal0[4];    /* Sprite 0 palette */
	UINT8   m_gb_spal1[4];    /* Sprite 1 palette */

	/* Things used to render current line */
	int m_current_line;       /* Current line */
	int m_cmp_line;           /* Compare line */
	int m_sprCount;           /* Number of sprites on current line */
	int m_sprite[10];         /* References to sprites to draw on current line */
	int m_previous_line;      /* Previous line we've drawn in */
	int m_start_x;            /* Pixel to start drawing from (inclusive) */
	int m_end_x;              /* Pixel to end drawing (exclusive) */
	int m_mode;               /* Keep track of internal STAT mode */
	int m_state;              /* Current state of the video state machine */
	int m_lcd_irq_line;
	int m_triggering_line_irq;
	int m_line_irq;
	int m_triggering_mode_irq;
	int m_mode_irq;
	int m_delayed_line_irq;
	int m_sprite_cycles;
	int m_scrollx_adjust;
	int m_oam_locked;
	int m_vram_locked;
	int m_pal_locked;
	int m_hdma_enabled;
	int m_hdma_possible;
	struct layer_struct m_layer[2];
	emu_timer *m_lcd_timer;
	int m_gbc_mode;

	std::unique_ptr<UINT8[]>   m_vram;     // Pointer to VRAM
	std::unique_ptr<UINT8[]>   m_oam;      // Pointer to OAM memory
	UINT8   m_gb_tile_no_mod;
	UINT32  m_gb_chrgen_offs;     // GB Character generator
	UINT32  m_gb_bgdtab_offs;     // GB Background character table
	UINT32  m_gb_wndtab_offs;     // GB Window character table
	UINT32  m_gbc_chrgen_offs;    // CGB Character generator
	UINT32  m_gbc_bgdtab_offs;    // CGB Background character table
	UINT32  m_gbc_wndtab_offs;    // CGB Window character table
	int     m_vram_bank;

	TIMER_CALLBACK_MEMBER(video_init_vbl);
	virtual TIMER_CALLBACK_MEMBER(lcd_timer_proc);
	virtual void videoptr_restore();
	void save_gb_video();
	void increment_scanline();
	void lcd_switch_on();
};


class mgb_lcd_device : public gb_lcd_device
{
public:
	mgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:

	// device-level overrides
	virtual void device_start() override;
};


class sgb_lcd_device : public gb_lcd_device
{
public:
	sgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void sgb_io_write_pal(int offs, UINT8 *data);

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void update_sprites() override;
	virtual void update_scanline() override;
	void refresh_border();
};


class cgb_lcd_device : public gb_lcd_device
{
public:
	cgb_lcd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(video_r) override;
	virtual DECLARE_WRITE8_MEMBER(video_w) override;

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void update_sprites() override;
	virtual void update_scanline() override;

	virtual TIMER_CALLBACK_MEMBER(lcd_timer_proc) override;
	virtual void videoptr_restore() override;
	void hdma_trans(UINT16 length);
};


extern const device_type GB_LCD_DMG;
extern const device_type GB_LCD_MGB;
extern const device_type GB_LCD_SGB;
extern const device_type GB_LCD_CGB;


#define MCFG_GB_LCD_DMG_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_DMG, 0 )

#define MCFG_GB_LCD_MGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_MGB, 0 )

#define MCFG_GB_LCD_SGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_SGB, 0 )

#define MCFG_GB_LCD_CGB_ADD(_tag ) \
		MCFG_DEVICE_ADD( _tag, GB_LCD_CGB, 0 )


#endif /* GB_LCD_H_ */
