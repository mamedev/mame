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
#include "cpu/lr35902/lr35902.h"


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


class dmg_ppu_device :  public device_t,
						public device_video_interface
{
public:
	dmg_ppu_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source, UINT32 vram_size);
	dmg_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	static void static_set_lr35902_tag(device_t &device, const char *tag) { downcast<dmg_ppu_device &>(device).m_lr35902.set_tag(tag); }

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	DECLARE_READ8_MEMBER(vram_r);
	DECLARE_WRITE8_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(oam_r);
	DECLARE_WRITE8_MEMBER(oam_w);
	virtual DECLARE_READ8_MEMBER(video_r);
	virtual DECLARE_WRITE8_MEMBER(video_w);

	// FIXME: remove it when proper sgb support is added
	void set_sgb_hack(bool val) { m_sgb_border_hack = val ? 1 : 0; }

	virtual void update_state();

protected:
	enum {
		// bits in LCD Control register
		ENABLED = 0x80,
		WINDOW_ENABLED = 0x20,
		LARGE_SPRITES = 0x04,
		SPRITES_ENABLED = 0x02,
		BACKGROUND_ENABLED = 0x01,
		// bits in LCD Status register
		LY_LYC_INT_ENABLED = 0x40,
		MODE_2_INT_ENABLED = 0x20,
		MODE_1_INT_ENABLED = 0x10,
		MODE_0_INT_ENABLED = 0x08,
		LY_LYC_FLAG = 0x04
	};

	inline void plot_pixel(int x, int y, UINT16 color);

	void select_sprites();
	void calculate_window_cycles();
	virtual void update_sprites();
	virtual void update_scanline(UINT32 cycles_to_go);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	void common_start();
	void common_reset();

	required_device<lr35902_cpu_device> m_lr35902;

	address_space *m_program_space;

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

	static constexpr unsigned NR_GB_VID_REGS = 0x40;

	UINT8   m_vid_regs[NR_GB_VID_REGS];
	UINT8   m_bg_zbuf[160];

	UINT16  m_cgb_bpal[32];   /* CGB current background palette table */
	UINT16  m_cgb_spal[32];   /* CGB current sprite palette table */

	UINT16  m_gb_bpal[4];     /* Background palette */
	UINT16  m_gb_spal0[4];    /* Sprite 0 palette */
	UINT16  m_gb_spal1[4];    /* Sprite 1 palette */

	/* WIP Things used to render current line */
	struct {
		// Background/window data
		UINT8 tile_cycle;
		UINT8 tile_count;
		UINT8 y;
		UINT16 pattern_address;
		UINT8 pattern;
		UINT16 tile_address;
		UINT8 plane0;
		UINT8 plane1;
		UINT16 shift_register;

		// Sprite data
		struct {
			bool enabled;
			UINT8 x;
			UINT8 y;
			UINT8 pattern;
			UINT8 flags;
			UINT8 tile_plane_0;
			UINT8 tile_plane_1;
		} sprite[10];
		UINT8 sprite_delay_cycles;
		// other internal data
		bool starting;       // Inital fetches when (re)starting the rendering engine.
		UINT8 sequence_counter;
		bool drawing;
		bool start_drawing;
		UINT8 scrollx_delay;
		UINT8 scrollx_to_apply;
		UINT8 pixels_drawn;
		UINT16 window_compare_position;
		bool window_active;
		UINT8 scrollx;
		// To keep track of when changes to WNDPOSY/WNDPOSX should kick in
		UINT8 window_start_y[16];
		UINT8 window_start_x[16];
		int window_start_y_index;
		// To keep track of when changes to LCDCONT should kick in for window
		UINT8 window_enable[16];
		int window_enable_index;
		bool window_should_trigger;
	} m_line;
	bool m_frame_window_active;

	int m_current_line;       /* Current line */
	int m_cmp_line;           /* Compare line */
	int m_sprCount;           /* Number of sprites on current line */
	int m_sprite[10];         /* References to sprites to draw on current line */
	int m_previous_line;      /* Previous line we've drawn in */
	int m_start_x;            /* Pixel to start drawing from (inclusive) */
	int m_end_x;              /* Pixel to end drawing (exclusive) */
	int m_mode;               /* Keep track of internal STAT mode */
	int m_state;              /* Current state of the video state machine */
	int m_sprite_cycles;
	int m_window_cycles;
	int m_scrollx_adjust;
	int m_oam_locked;
	int m_oam_locked_reading;
	int m_vram_locked;
	int m_pal_locked;
	int m_hdma_enabled;
	int m_hdma_possible;
	int m_hdma_cycles_to_start;
	UINT16 m_hdma_length;
	struct layer_struct m_layer[2];
	emu_timer *m_lcd_timer;
	int m_oam_dma_start_cycles;
	int m_oam_dma_cycles_left;
	UINT16 m_oam_dma_source_address;
	int m_gbc_mode;
	UINT8 m_window_x;
	UINT8 m_window_y;
	UINT8 m_old_curline;
	// Interrupt related
	bool m_stat_mode0_int;
	bool m_stat_mode1_int;
	bool m_stat_mode2_int;
	bool m_stat_lyc_int;
	bool m_stat_lyc_int_prev;
	bool m_stat_write_int;
	bool m_stat_int;

	std::unique_ptr<UINT8[]>   m_vram;     // Pointer to VRAM
	std::unique_ptr<UINT8[]>   m_oam;      // Pointer to OAM memory
	bool    m_oam_dma_processing;
	UINT8   m_gb_tile_no_mod;
	UINT32  m_gb_chrgen_offs;     // GB Character generator
	UINT32  m_gb_bgdtab_offs;     // GB Background character table
	UINT32  m_gb_wndtab_offs;     // GB Window character table
	UINT32  m_gbc_chrgen_offs;    // CGB Character generator
	UINT32  m_gbc_bgdtab_offs;    // CGB Background character table
	UINT32  m_gbc_wndtab_offs;    // CGB Window character table
	int     m_vram_bank;

	attotime m_last_updated;
	UINT64 m_cycles_left;
	int m_next_state;
	bool m_updating_state;
	bool m_enable_experimental_engine;

	virtual void videoptr_restore();
	virtual bool stat_write(UINT8 new_data);
	void increment_scanline();
	void lcd_switch_on(UINT8 new_data);
	void update_oam_dma_state(UINT64 cycles);
	void check_stat_irq();
	void clear_line_state();
	void update_line_state(UINT64 cycles);
	void check_start_of_window();

private:
	UINT32 m_oam_size;
	UINT32 m_vram_size;
};


class mgb_ppu_device : public dmg_ppu_device
{
public:
	mgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:

	// device-level overrides
	virtual void device_start() override;
};


class sgb_ppu_device : public dmg_ppu_device
{
public:
	sgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	void sgb_io_write_pal(int offs, UINT8 *data);

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void update_sprites() override;
	virtual void update_scanline(UINT32 cycles_to_go) override;
	void refresh_border();
};


class cgb_ppu_device : public dmg_ppu_device
{
public:
	cgb_ppu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	virtual DECLARE_READ8_MEMBER(video_r) override;
	virtual DECLARE_WRITE8_MEMBER(video_w) override;

protected:

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void update_sprites() override;
	virtual void update_scanline(UINT32 cycles_to_go) override;

	virtual void update_state() override;
	virtual void videoptr_restore() override;
	virtual bool stat_write(UINT8 new_data) override;
	void update_hdma_state(UINT64 cycles);
	void hdma_trans(UINT16 length);
	void hdma_trans_execute();
};


extern const device_type DMG_PPU;
extern const device_type MGB_PPU;
extern const device_type SGB_PPU;
extern const device_type CGB_PPU;


#define MCFG_DMG_PPU_ADD(_tag, _cpu_tag ) \
		MCFG_DEVICE_ADD( _tag, DMG_PPU, 0 ) \
		dmg_ppu_device::static_set_lr35902_tag(*device, "^" _cpu_tag);

#define MCFG_MGB_PPU_ADD(_tag, _cpu_tag ) \
		MCFG_DEVICE_ADD( _tag, MGB_PPU, 0 ) \
		dmg_ppu_device::static_set_lr35902_tag(*device, "^" _cpu_tag);

#define MCFG_SGB_PPU_ADD(_tag, _cpu_tag ) \
		MCFG_DEVICE_ADD( _tag, SGB_PPU, 0 ) \
		dmg_ppu_device::static_set_lr35902_tag(*device, "^" _cpu_tag);

#define MCFG_CGB_PPU_ADD(_tag, _cpu_tag ) \
		MCFG_DEVICE_ADD( _tag, CGB_PPU, 0 ) \
		dmg_ppu_device::static_set_lr35902_tag(*device, "^" _cpu_tag);


#endif /* GB_LCD_H_ */
