// license:BSD-3-Clause
// copyright-holders:Mark McDougall
/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 */

#ifndef MAME_NAMCO_YGV608_H
#define MAME_NAMCO_YGV608_H

#pragma once

#include "tilemap.h"
#include "screen.h"

class ygv608_device : public device_t,
					  public device_gfx_interface,
					  public device_memory_interface,
					  public device_palette_interface,
					  public device_video_interface
{
public:
	// construction/destruction
	ygv608_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	// configurations
	using ygv608_gfxbank_delegate = device_delegate<u32 (u32 addr)>;

	auto vblank_callback() { return m_vblank_handler.bind(); }
	auto raster_callback() { return m_raster_handler.bind(); }
	template <typename... T> void set_gfxbank_callback(T &&... args) { m_gfxbank_cb.set(std::forward<T>(args)...); }

	void set_tilemap_dirty();

	// ports section
	u8 pattern_name_table_r();
	u8 sprite_data_r();
	u8 scroll_data_r();
	u8 palette_data_r();
	u8 register_data_r();
//  u8 register_select_r();
	u8 status_port_r();
	u8 system_control_r();
	void pattern_name_table_w(u8 data);
	void sprite_data_w(u8 data);
	void scroll_data_w(u8 data);
	void palette_data_w(u8 data);
	void register_data_w(u8 data);
	void register_select_w(u8 data);
	void status_port_w(u8 data);
	void system_control_w(u8 data);

	// register section
	u8 pattern_name_table_x_r();
	void pattern_name_table_x_w(u8 data);
	u8 pattern_name_table_y_r();
	void pattern_name_table_y_w(u8 data);
	u8 ram_access_ctrl_r();
	void ram_access_ctrl_w(u8 data);
	u8 sprite_address_r();
	void sprite_address_w(u8 data);
	u8 scroll_address_r();
	void scroll_address_w(u8 data);
	u8 palette_address_r();
	void palette_address_w(u8 data);
	u8 sprite_bank_r();
	void sprite_bank_w(u8 data);
	u8 screen_ctrl_7_r();
	void screen_ctrl_7_w(u8 data);
	u8 screen_ctrl_8_r();
	void screen_ctrl_8_w(u8 data);
	u8 screen_ctrl_9_r();
	void screen_ctrl_9_w(u8 data);
	u8 screen_ctrl_10_r();
	void screen_ctrl_10_w(u8 data);
	u8 screen_ctrl_11_r();
	void screen_ctrl_11_w(u8 data);
	u8 screen_ctrl_12_r();
	void screen_ctrl_12_w(u8 data);
	u8 irq_mask_r();
	void irq_mask_w(u8 data);
	u8 irq_ctrl_r(offs_t offset);
	void irq_ctrl_w(offs_t offset, u8 data);
	void crtc_w(offs_t offset, u8 data);
	void base_address_w(offs_t offset, u8 data);
	void roz_ax_w(offs_t offset, u8 data);
	void roz_dx_w(offs_t offset, u8 data);
	void roz_dxy_w(offs_t offset, u8 data);
	void roz_ay_w(offs_t offset, u8 data);
	void roz_dy_w(offs_t offset, u8 data);
	void roz_dyx_w(offs_t offset, u8 data);
	void border_color_w(u8 data);

	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void regs_map(address_map &map) ATTR_COLD;
	void port_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

	virtual space_config_vector memory_space_config() const override;

	virtual u32 palette_entries() const noexcept override { return 256; }

	TIMER_CALLBACK_MEMBER(update_vblank_flag);
	TIMER_CALLBACK_MEMBER(update_raster_flag);

	address_space *m_iospace;

private:
	const address_space_config m_io_space_config;
	static constexpr unsigned SPRITE_ATTR_TABLE_SIZE = 256;

	static constexpr unsigned MAX_SPRITES = SPRITE_ATTR_TABLE_SIZE >> 2;

	struct SPRITE_ATTR {
		u8 sy;       // y dot position 7:0
		u8 sx;       // x dot position 7:0
		u8 attr;     // 0xf0 = color, 0x0c = size, reverse, 0x02 = x hi bit, 0x01 = y hi bit
		u8 sn;    // pattern name (0-255)
	};

	TILEMAP_MAPPER_MEMBER(get_tile_offset);
	TILE_GET_INFO_MEMBER(get_tile_info_A_8);
	TILE_GET_INFO_MEMBER(get_tile_info_B_8);
	TILE_GET_INFO_MEMBER(get_tile_info_A_16);
	TILE_GET_INFO_MEMBER(get_tile_info_B_16);

	void register_state_save();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_layer_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ba_select);
	void draw_layer_scroll(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int ba);
	void draw_mosaic(bitmap_ind16 &bitmap, const rectangle &cliprect, int n);

	void handle_reset();
	void handle_rom_transfers(u8 type);

	void screen_configure();        /**< Adjust screen parameters based off CRTC ones */
	attotime raster_sync_offset();  /**< Adjust timing based off raster & CRTC parameters */
	void vblank_irq_check();        /**< mask + pend check for vblank irq */
	void raster_irq_check();        /**< mask + pend check for raster irq */
	void pattern_name_autoinc_check();  /**< check autoinc for tile pointers */
	void pattern_mode_setup();      /**< refresh pattern mode at register 7/8 change*/
	int get_col_division(int screen_x); /**< scroll-table byte index for a display column (dots) */
	int get_row_division(int screen_y); /**< scroll-table byte index for a display row (dots) */
	int scroll_table_shift(u8 div_size) const; /**< SLH/SLV -> screen-dot shift */

	// inline helpers
	// for raw to ROZ conversion
	u32 roz_convert_raw24(u32 &raw_reg, u8 offset, u8 data);
	u32 roz_convert_raw16(u16 &raw_reg, u8 offset, u8 data);

	ygv608_gfxbank_delegate m_gfxbank_cb;

	tilemap_t *m_tilemap_cache_8[2][3];
	tilemap_t *m_tilemap_cache_16[2][3];
	tilemap_t *m_tilemap[2];
	bitmap_ind16 m_work_bitmap;


	/*
	*  Built in ram
	*/

	u8 m_pattern_name_table[4096];

	union {
		u8 b[SPRITE_ATTR_TABLE_SIZE];
		SPRITE_ATTR s[MAX_SPRITES];
	} m_sprite_attribute_table;

	u8 m_scroll_data_table[2][256];
	u8 m_colour_palette[256][3];

	/*
	*  Shortcut variables
	*/

	u32 m_bits16;          // bits per pattern (8/16)
	u32 m_page_x, m_page_y;  // pattern page size
	u32 m_pny_shift;       // y coord multiplier
	u8 m_na8_mask;       // mask on/off na11/9:8
	s32 m_col_shift;                // screen-dot -> table shift for SLV (column Y)
	s32 m_row_shift;                // screen-dot -> table shift for SLH (row X)


	// base address shortcuts
	u32 m_base_addr[2][8];
	u32 m_base_y_shift;    // for extracting pattern y coord 'base'

	bool m_screen_resize;  // screen requires resize
	bool m_tilemap_resize; // tilemap requires resize

	/* These were statically allocated in the r/w routines */
	s32 m_color_state_r;
	s32 m_color_state_w;
	s32 m_p0_state;
	s32 m_pattern_name_base_r,m_pattern_name_base_w;     /* pattern name table base address */

	// === new variable handling starts here ===
	u8 m_screen_status;    /**< port #6: status port r/w */
	u8 m_dma_status;       /**< port #7: system control port r/w */

	u8 m_register_address; /**< RN: Register address select */
	bool m_register_autoinc_r;  /**< RRAI: Register address auto-increment on read */
	bool m_register_autoinc_w;  /**< RWAI: Register address auto-increment on write */

	bool m_raster_irq_mask;     /**< IEP: raster irq mask (INT1 occurs if 1) */
	bool m_vblank_irq_mask;     /**< IEV: vblank irq mask (INT0 occurs if 1) */
	s32 m_raster_irq_hpos;      /**< IH: horizontal position where raster irq occurs x 32 */
	s32 m_raster_irq_vpos;      /**< IV: vertical position where raster irq occurs */
	bool m_raster_irq_mode;     /**< FPM: if 1 vertical position becomes invalid for raster irqs (irqs occur for every line) */

	u8 m_scroll_address;   /**< SCA: scroll table access pointer */
	u8 m_palette_address;  /**< CC: color palette access pointer */
	u8 m_sprite_address;   /**< SAA: sprite attribute table access pointer */
	u8 m_sprite_bank;      /**< SBA: sprite generator base address (MA20 to MA13) */
	u8 m_xtile_ptr;        /**< PNX: X coordinate of pattern space */
	u8 m_ytile_ptr;        /**< PNY: Y coordinate of pattern space */
	bool m_xtile_autoinc;       /**< PNXA: Permits auto-increment in X coordinate */
	bool m_ytile_autoinc;       /**< PNXA: Permits auto-increment in Y coordinate */
	bool m_plane_select_access; /**< B/(A): A/B plane access select */

	u8 m_mosaic_plane[2];    /**< MCA/B: mosaic factor applied to A/B plane */
	bool m_sprite_disable;      /**< SPRD: disables the sprite plane display */
	bool m_sprite_aux_mode;     /**< SPAS: if 0 aux bits selects size, if 1 selects flipping */
	u8 m_sprite_aux_reg;   /**< SPA: auxiliary bits of sprite attribute table */
	u8 m_border_color;     /**< BDC: border color */

	bool m_saar;                    /**< SAAR: Address autoinc after reading sprite attribute table */
	bool m_saaw;                    /**< SAAW: Address autoinc after writing sprite attribute table */
	bool m_scar;                    /**< SCAR: Address autoinc after reading scroll data table */
	bool m_scaw;                    /**< SCAW: Address autoinc after writing scroll data table */
	bool m_cpar;                    /**< CPAR: Address autoinc after reading color palette */
	bool m_cpaw;                    /**< CPAW: Address autoinc after writing color palette */
	bool m_ba_plane_scroll_select;          /**< B/(A) P#2 gains access to scroll data table in A/B plane */

	bool m_dspe;                    /**< DSPE: display permission of pattern plane(s) (screen blanked if 0) */
	u8 m_md;                   /**< MDx: mode for pattern planes */
	bool m_zron;                    /**< ZRON: enable ROZ features */
	bool m_flip;                    /**< FLIP: enable flip for attribute bits 11 & 10 */
	bool m_dckm;                    /**< DCKM: dot clock frequency select */

	bool m_page_size;               /**< PGS: page size setter */
	u8 m_h_display_size;       /**< HDS: horizontal display domain */
	u8 m_v_display_size;       /**< VDS: vertical display domain */
	bool m_roz_wrap_disable;        /**< RLRT: ROZ wraparound disable */
	bool m_scroll_wrap_disable;     /**< RLSC: scroll wraparound disable */

	u8 m_pattern_size;         /**< PTS: pattern size of pattern plane */
	u8 m_h_div_size;           /**< SLH: size of horizontal division in screen division scrolling */
	u8 m_v_div_size;           /**< SLV: size of vertical division in screen division scrolling */

	bool m_plane_trans_enable[2];     /**< CTPA/B: enable transparency for plane A/B */
	u8 m_priority_mode;        /**< PRM: priority mode select */
	bool m_cbdr;                    /**< CBDR: color bus terminals CB7 to 0 and SPRT */
	bool m_yse;                     /**< YSE: permission control of trasparency timing output of YS terminal */
	u8 m_scm;                  /**< SCM: output frequency of clock signal output from terminal FSC */

	u8 m_plane_color_fetch[2];   /**< A/BPF: A/B plane color fetch mode */
	u8 m_sprite_color_fetch;   /**< SPF: sprite color fetch mode */

	// screen section
	devcb_write_line m_vblank_handler;
	devcb_write_line m_raster_handler;
	emu_timer *m_vblank_timer;
	emu_timer *m_raster_timer;

	enum
	{
		VBLANK_TIMER,
		RASTER_TIMER
	};

	struct {
		s32 htotal = 0;             /**< HTL: horizontal total number of dots x 2 */
		s32 vtotal = 0;             /**< VTL: vertical total number of lines x 1 */
		s32 display_hstart = 0;     /**< HDS: horizontal display starting position x 2*/
		s32 display_vstart = 0;     /**< VDS: vertical display starting position x 1 */
		s32 display_width = 0;      /**< HDW: horizontal display size x 16 */
		s32 display_height = 0;     /**< VDW: vertical display size x 8 */
		s32 display_hsync = 0;      /**< HSW: horizontal sync signal x 16 */
		s32 display_vsync = 0;      /**< VSW: vertical sync signal x 1 */
		s32 border_width = 0;       /**< HBW: horizontal border size x 16 */
		s32 border_height = 0;      /**< VBW: vertical border size x 8 */
	} m_crtc;

	// rotation, zoom shortcuts
	u32 m_ax;              /**< AX */
	u32 m_dx;              /**< DX */
	u32 m_dxy;             /**< DXY */
	u32 m_ay;              /**< AY */
	u32 m_dy;              /**< DY */
	u32 m_dyx;             /**< DYX */

	// raw register versions of above
	u32 m_raw_ax;
	u16 m_raw_dx;
	u16 m_raw_dxy;
	u32 m_raw_ay;
	u16 m_raw_dy;
	u16 m_raw_dyx;

};

// device type definition
DECLARE_DEVICE_TYPE(YGV608, ygv608_device)


#endif // MAME_NAMCO_YGV608_H
