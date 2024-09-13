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
	ygv608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void port_map(address_map &map);

	// ports section
	uint8_t pattern_name_table_r();
	uint8_t sprite_data_r();
	uint8_t scroll_data_r();
	uint8_t palette_data_r();
	uint8_t register_data_r();
//  uint8_t register_select_r();
	uint8_t status_port_r();
	uint8_t system_control_r();
	void pattern_name_table_w(uint8_t data);
	void sprite_data_w(uint8_t data);
	void scroll_data_w(uint8_t data);
	void palette_data_w(uint8_t data);
	void register_data_w(uint8_t data);
	void register_select_w(uint8_t data);
	void status_port_w(uint8_t data);
	void system_control_w(uint8_t data);

	// register section
	uint8_t pattern_name_table_x_r();
	void pattern_name_table_x_w(uint8_t data);
	uint8_t pattern_name_table_y_r();
	void pattern_name_table_y_w(uint8_t data);
	uint8_t ram_access_ctrl_r();
	void ram_access_ctrl_w(uint8_t data);
	uint8_t sprite_address_r();
	void sprite_address_w(uint8_t data);
	uint8_t scroll_address_r();
	void scroll_address_w(uint8_t data);
	uint8_t palette_address_r();
	void palette_address_w(uint8_t data);
	uint8_t sprite_bank_r();
	void sprite_bank_w(uint8_t data);
	uint8_t screen_ctrl_7_r();
	void screen_ctrl_7_w(uint8_t data);
	uint8_t screen_ctrl_8_r();
	void screen_ctrl_8_w(uint8_t data);
	uint8_t screen_ctrl_9_r();
	void screen_ctrl_9_w(uint8_t data);
	uint8_t screen_ctrl_10_r();
	void screen_ctrl_10_w(uint8_t data);
	uint8_t screen_ctrl_11_r();
	void screen_ctrl_11_w(uint8_t data);
	uint8_t screen_ctrl_12_r();
	void screen_ctrl_12_w(uint8_t data);
	uint8_t irq_mask_r();
	void irq_mask_w(uint8_t data);
	uint8_t irq_ctrl_r(offs_t offset);
	void irq_ctrl_w(offs_t offset, uint8_t data);
	void crtc_w(offs_t offset, uint8_t data);
	void base_address_w(offs_t offset, uint8_t data);
	void roz_ax_w(offs_t offset, uint8_t data);
	void roz_dx_w(offs_t offset, uint8_t data);
	void roz_dxy_w(offs_t offset, uint8_t data);
	void roz_ay_w(offs_t offset, uint8_t data);
	void roz_dy_w(offs_t offset, uint8_t data);
	void roz_dyx_w(offs_t offset, uint8_t data);
	void border_color_w(uint8_t data);

	// TODO: is this even a real connection?
	void set_gfxbank(uint8_t gfxbank);

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	auto vblank_callback() { return m_vblank_handler.bind(); }
	auto raster_callback() { return m_raster_handler.bind(); }

	void regs_map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_post_load() override;

	virtual space_config_vector memory_space_config() const override;

	virtual uint32_t palette_entries() const noexcept override { return 256; }

	TIMER_CALLBACK_MEMBER(update_vblank_flag);
	TIMER_CALLBACK_MEMBER(update_raster_flag);

	address_space *m_iospace;

private:
	const address_space_config m_io_space_config;
	static constexpr unsigned SPRITE_ATTR_TABLE_SIZE = 256;

	static constexpr unsigned MAX_SPRITES = SPRITE_ATTR_TABLE_SIZE >> 2;

	struct SPRITE_ATTR {
		uint8_t sy;       // y dot position 7:0
		uint8_t sx;       // x dot position 7:0
		uint8_t attr;     // 0xf0 = color, 0x0c = size, reverse, 0x02 = x hi bit, 0x01 = y hi bit
		uint8_t sn;    // pattern name (0-255)
	};

	TILEMAP_MAPPER_MEMBER(get_tile_offset);
	TILE_GET_INFO_MEMBER(get_tile_info_A_8);
	TILE_GET_INFO_MEMBER(get_tile_info_B_8);
	TILE_GET_INFO_MEMBER(get_tile_info_A_16);
	TILE_GET_INFO_MEMBER(get_tile_info_B_16);

	void register_state_save();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_layer_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *source_tilemap);
	void draw_mosaic(bitmap_ind16 &bitmap, const rectangle &cliprect, int n);

	uint8_t m_namcond1_gfxbank;

	tilemap_t *m_tilemap_A_cache_8[3];
	tilemap_t *m_tilemap_A_cache_16[3];
	tilemap_t *m_tilemap_B_cache_8[3];
	tilemap_t *m_tilemap_B_cache_16[3];
	tilemap_t *m_tilemap_A;
	tilemap_t *m_tilemap_B;
	bitmap_ind16 m_work_bitmap;

	void HandleReset();
	void HandleRomTransfers(uint8_t type);


	/*
	*  Built in ram
	*/

	uint8_t m_pattern_name_table[4096];

	union {
		uint8_t b[SPRITE_ATTR_TABLE_SIZE];
		SPRITE_ATTR s[MAX_SPRITES];
	} m_sprite_attribute_table;

	uint8_t m_scroll_data_table[2][256];
	uint8_t m_colour_palette[256][3];

	/*
	*  Shortcut variables
	*/

	uint32_t m_bits16;          // bits per pattern (8/16)
	uint32_t m_page_x, m_page_y;  // pattern page size
	uint32_t m_pny_shift;       // y coord multiplier
	uint8_t m_na8_mask;       // mask on/off na11/9:8
	int m_col_shift;                // shift in scroll table column index


	// base address shortcuts
	uint32_t m_base_addr[2][8];
	uint32_t m_base_y_shift;    // for extracting pattern y coord 'base'

	bool m_screen_resize;  // screen requires resize
	bool m_tilemap_resize; // tilemap requires resize

	/* These were statically allocated in the r/w routines */
	int m_color_state_r;
	int m_color_state_w;
	int m_p0_state;
	int m_pattern_name_base_r,m_pattern_name_base_w;     /* pattern name table base address */

	// === new variable handling starts here ===
	uint8_t m_screen_status;    /**< port #6: status port r/w */
	uint8_t m_dma_status;       /**< port #7: system control port r/w */

	uint8_t m_register_address; /**< RN: Register address select */
	bool m_register_autoinc_r;  /**< RRAI: Register address auto-increment on read */
	bool m_register_autoinc_w;  /**< RWAI: Register address auto-increment on write */

	bool m_raster_irq_mask;     /**< IEP: raster irq mask (INT1 occurs if 1) */
	bool m_vblank_irq_mask;     /**< IEV: vblank irq mask (INT0 occurs if 1) */
	int m_raster_irq_hpos;      /**< IH: horizontal position where raster irq occurs x 32 */
	int m_raster_irq_vpos;      /**< IV: vertical position where raster irq occurs */
	bool m_raster_irq_mode;     /**< FPM: if 1 vertical position becomes invalid for raster irqs (irqs occur for every line) */

	uint8_t m_scroll_address;   /**< SCA: scroll table access pointer */
	uint8_t m_palette_address;  /**< CC: color palette access pointer */
	uint8_t m_sprite_address;   /**< SAA: sprite attribute table access pointer */
	uint8_t m_sprite_bank;      /**< SBA: sprite generator base address (MA20 to MA13) */
	uint8_t m_xtile_ptr;        /**< PNX: X coordinate of pattern space */
	uint8_t m_ytile_ptr;        /**< PNY: Y coordinate of pattern space */
	bool m_xtile_autoinc;       /**< PNXA: Permits auto-increment in X coordinate */
	bool m_ytile_autoinc;       /**< PNXA: Permits auto-increment in Y coordinate */
	bool m_plane_select_access; /**< B/(A): A/B plane access select */

	uint8_t m_mosaic_aplane;    /**< MCA: mosaic factor applied to A plane */
	uint8_t m_mosaic_bplane;    /**< MCA: mosaic factor applied to B plane */
	bool m_sprite_disable;      /**< SPRD: disables the sprite plane display */
	bool m_sprite_aux_mode;     /**< SPAS: if 0 aux bits selects size, if 1 selects flipping */
	uint8_t m_sprite_aux_reg;   /**< SPA: auxiliary bits of sprite attribute table */
	uint8_t m_border_color;     /**< BDC: border color */

	bool m_saar;                    /**< SAAR: Address autoinc after reading sprite attribute table */
	bool m_saaw;                    /**< SAAW: Address autoinc after writing sprite attribute table */
	bool m_scar;                    /**< SCAR: Address autoinc after reading scroll data table */
	bool m_scaw;                    /**< SCAW: Address autoinc after writing scroll data table */
	bool m_cpar;                    /**< CPAR: Address autoinc after reading color palette */
	bool m_cpaw;                    /**< CPAW: Address autoinc after writing color palette */
	bool m_ba_plane_scroll_select;          /**< B/(A) P#2 gains access to scroll data table in A/B plane */

	bool m_dspe;                    /**< DSPE: display permission of pattern plane(s) (screen blanked if 0) */
	uint8_t m_md;                   /**< MDx: mode for pattern planes */
	bool m_zron;                    /**< ZRON: enable ROZ features */
	bool m_flip;                    /**< FLIP: enable flip for attribute bits 11 & 10 */
	bool m_dckm;                    /**< DCKM: dot clock frequency select */

	bool m_page_size;               /**< PGS: page size setter */
	uint8_t m_h_display_size;       /**< HDS: horizontal display domain */
	uint8_t m_v_display_size;       /**< VDS: vertical display domain */
	bool m_roz_wrap_disable;        /**< RLRT: ROZ wraparound disable */
	bool m_scroll_wrap_disable;     /**< RLSC: ROZ wraparound disable */

	uint8_t m_pattern_size;         /**< PTS: pattern size of pattern plane */
	uint8_t m_h_div_size;           /**< SLH: size of horizontal division in screen division scrolling */
	uint8_t m_v_div_size;           /**< SLV: size of vertical division in screen division scrolling */

	bool m_planeA_trans_enable;     /**< CTPA: enable transparency for plane A */
	bool m_planeB_trans_enable;     /**< CTPA: enable transparency for plane B */
	uint8_t m_priority_mode;        /**< PRM: priority mode select */
	bool m_cbdr;                    /**< CBDR: color bus terminals CB7 to 0 and SPRT */
	bool m_yse;                     /**< YSE: permission control of trasparency timing output of YS terminal */
	uint8_t m_scm;                  /**< SCM: output frequency of clock signal output from terminal FSC */

	uint8_t m_planeA_color_fetch;   /**< APF: A plane color fetch mode */
	uint8_t m_planeB_color_fetch;   /**< BPF: B plane color fetch mode */
	uint8_t m_sprite_color_fetch;   /**< SPF: sprite color fetch mode */

	// screen section
	devcb_write_line m_vblank_handler;
	devcb_write_line m_raster_handler;
	emu_timer *m_vblank_timer;
	emu_timer *m_raster_timer;

	void screen_configure();        /**< Adjust screen parameters based off CRTC ones */
	attotime raster_sync_offset();  /**< Adjust timing based off raster & CRTC parameters */
	void vblank_irq_check();        /**< mask + pend check for vblank irq */
	void raster_irq_check();        /**< mask + pend check for raster irq */
	void pattern_name_autoinc_check();  /**< check autoinc for tile pointers */
	void pattern_mode_setup();      /**< refresh pattern mode at register 7/8 change*/
	int get_col_division(int raw_col); /**< calculate column scroll */
	int get_row_division(int raw_row); /**< calculate row scroll */

	enum
	{
		VBLANK_TIMER,
		RASTER_TIMER
	};

	struct {
		int htotal = 0;             /**< HTL: horizontal total number of dots x 2 */
		int vtotal = 0;             /**< VTL: vertical total number of lines x 1 */
		int display_hstart = 0;     /**< HDS: horizontal display starting position x 2*/
		int display_vstart = 0;     /**< VDS: vertical display starting position x 1 */
		int display_width = 0;      /**< HDW: horizontal display size x 16 */
		int display_height = 0;     /**< VDW: vertical display size x 8 */
		int display_hsync = 0;      /**< HSW: horizontal sync signal x 16 */
		int display_vsync = 0;      /**< VSW: vertical sync signal x 1 */
		int border_width = 0;       /**< HBW: horizontal border size x 16 */
		int border_height = 0;      /**< VBW: vertical border size x 8 */
	} m_crtc;

	// rotation, zoom shortcuts
	uint32_t m_ax;              /**< AX */
	uint32_t m_dx;              /**< DX */
	uint32_t m_dxy;             /**< DXY */
	uint32_t m_ay;              /**< AY */
	uint32_t m_dy;              /**< DY */
	uint32_t m_dyx;             /**< DYX */

	// raw register versions of above
	uint32_t m_raw_ax;
	uint16_t m_raw_dx;
	uint16_t m_raw_dxy;
	uint32_t m_raw_ay;
	uint16_t m_raw_dy;
	uint16_t m_raw_dyx;

	// inline helpers
	// for raw to ROZ conversion
	uint32_t roz_convert_raw24(uint32_t *raw_reg, uint8_t offset, uint8_t data);
	uint32_t roz_convert_raw16(uint16_t *raw_reg, uint8_t offset, uint8_t data);
};

// device type definition
DECLARE_DEVICE_TYPE(YGV608, ygv608_device)


#endif // MAME_NAMCO_YGV608_H
