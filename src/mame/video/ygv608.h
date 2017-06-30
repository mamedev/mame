// license:BSD-3-Clause
// copyright-holders:Mark McDougall
/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 */

#ifndef MAME_VIDEO_YGV608_H
#define MAME_VIDEO_YGV608_H

#pragma once

#include "tilemap.h"
#include "screen.h"

class ygv608_device : public device_t, 
                      public device_gfx_interface,
					  public device_memory_interface
{
public:
	// construction/destruction
	ygv608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_ADDRESS_MAP(port_map, 8);

	// ports section
	DECLARE_READ8_MEMBER(pattern_name_table_r);
	DECLARE_READ8_MEMBER(sprite_data_r);
	DECLARE_READ8_MEMBER(scroll_data_r);
	DECLARE_READ8_MEMBER(palette_data_r);
	DECLARE_READ8_MEMBER(register_data_r);
//	DECLARE_READ8_MEMBER(register_select_r);
	DECLARE_READ8_MEMBER(status_port_r);
	DECLARE_READ8_MEMBER(system_control_r);
	DECLARE_WRITE8_MEMBER(pattern_name_table_w);
	DECLARE_WRITE8_MEMBER(sprite_data_w);
	DECLARE_WRITE8_MEMBER(scroll_data_w);
	DECLARE_WRITE8_MEMBER(palette_data_w);
	DECLARE_WRITE8_MEMBER(register_data_w);
	DECLARE_WRITE8_MEMBER(register_select_w);
	DECLARE_WRITE8_MEMBER(status_port_w);
	DECLARE_WRITE8_MEMBER(system_control_w);

	// register section
	DECLARE_READ8_MEMBER(pattern_name_table_x_r);
	DECLARE_WRITE8_MEMBER(pattern_name_table_x_w);
	DECLARE_READ8_MEMBER(pattern_name_table_y_r);
	DECLARE_WRITE8_MEMBER(pattern_name_table_y_w);
	DECLARE_READ8_MEMBER(sprite_address_r);
	DECLARE_WRITE8_MEMBER(sprite_address_w);
	DECLARE_READ8_MEMBER(scroll_address_r);
	DECLARE_WRITE8_MEMBER(scroll_address_w);
	DECLARE_READ8_MEMBER(palette_address_r);
	DECLARE_WRITE8_MEMBER(palette_address_w);
	DECLARE_READ8_MEMBER(sprite_bank_r);
	DECLARE_WRITE8_MEMBER(sprite_bank_w);
	DECLARE_READ8_MEMBER(screen_ctrl_mosaic_sprite_r);
	DECLARE_WRITE8_MEMBER(screen_ctrl_mosaic_sprite_w);	
	DECLARE_READ8_MEMBER(irq_mask_r);
	DECLARE_WRITE8_MEMBER(irq_mask_w);
	DECLARE_READ8_MEMBER(irq_ctrl_r);
	DECLARE_WRITE8_MEMBER(irq_ctrl_w);
	DECLARE_WRITE8_MEMBER(crtc_w);
	DECLARE_WRITE8_MEMBER(base_address_w);
	DECLARE_WRITE8_MEMBER(roz_ax_w);
	DECLARE_WRITE8_MEMBER(roz_dx_w);
	DECLARE_WRITE8_MEMBER(roz_dxy_w);
	DECLARE_WRITE8_MEMBER(roz_ay_w);
	DECLARE_WRITE8_MEMBER(roz_dy_w);
	DECLARE_WRITE8_MEMBER(roz_dyx_w);
	DECLARE_WRITE8_MEMBER(border_color_w);

	// TODO: is this even a real connection?
	void set_gfxbank(uint8_t gfxbank);

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);


	template <class Object> static devcb_base &static_set_vblank_callback(device_t &device, Object &&cb)
	{
		return downcast<ygv608_device &>(device).m_vblank_handler.set_callback(std::forward<Object>(cb));
	}
	template <class Object> static devcb_base &static_set_raster_callback(device_t &device, Object &&cb)
	{
		return downcast<ygv608_device &>(device).m_raster_handler.set_callback(std::forward<Object>(cb));
	}
	
protected:
	// device-level overrides
	virtual void device_start() override;

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_IO) const override;
	
	void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	address_space *m_iospace;
private:
	const address_space_config m_io_space_config;
	static constexpr unsigned SPRITE_ATTR_TABLE_SIZE = 256;

	struct YGV_PORTS {
		uint8_t na;           // P#0 - pattern name table data port (read/write)
		uint8_t p1;           // P#1 - sprite data port (read/write)
		uint8_t p2;           // P#2 - scroll data port (read/write)
		uint8_t p3;           // P#3 - colour palette data port (read/write)
		uint8_t p4;           // P#4 - register data port (read/write)
		uint8_t p5;           // P#5 - register select port (write only)
		uint8_t p6;           // P#6 - status port (read/write)
		uint8_t p7;           // P#7 - system control port (read/write)
	};

	struct YGV_REGS {
		uint8_t r0;           // R#0 - pattern name table access ptr (r/w)
		uint8_t r1;           // R#1 - pattern name table access ptr (r/w)
		uint8_t r2;           // R#2 - built in ram access control
		uint8_t saa;          // R#3 - sprite attribute table access ptr (r/w)
		uint8_t sca;          // R#4 - scroll table access ptr (r/w)
		uint8_t cc;           // R#5 - color palette access ptr (r/w)
		uint8_t sba;          // R#6 - sprite generator base address (r/w)

		// R#7 - R#11 - screen control (r/w)
		uint8_t r7;           // misc screen control (r/w)
		uint8_t r8;           // misc screen control (r/w)
		uint8_t r9;           // misc screen control (r/w)
		uint8_t r10;          // misc screen control (r/w)
		uint8_t r11;          // misc screen control (r/w)

		uint8_t r12;          // R#12 - color palette selection (r/w)
		uint8_t bdc;          // R#13 - border colour (wo)

		// R#14 - R#16 - interrupt control
		uint8_t r14;
		uint8_t il;
		uint8_t r16;

		// R#17 - R#24 - base address (wo)
		uint8_t r17;
		uint8_t r18;
		uint8_t r19;
		uint8_t r20;
		uint8_t r21;
		uint8_t r22;
		uint8_t r23;
		uint8_t r24;

		// R#25 - R#38 - enlargement, contraction and rotation parameters (wo)
		uint8_t ax0;
		uint8_t ax8;
		uint8_t ax16;

		uint8_t dx0;
		uint8_t dx8;
		uint8_t dxy0;
		uint8_t dxy8;

		uint8_t ay0;
		uint8_t ay8;
		uint8_t ay16;

		uint8_t dy0;
		uint8_t dy8;
		uint8_t dyx0;
		uint8_t dyx8;

		// R#39 - R#46 - display scan control (wo)
		uint8_t r39;
		uint8_t r40;
		uint8_t hdsp;
		uint8_t htl;
		uint8_t r43;
		uint8_t r44;
		uint8_t r45;
		uint8_t vtl;

		// R#47 - R#49 - rom transfer control (wo)
		uint8_t tb5;
		uint8_t tb13;
		uint8_t tn4;

	};

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
	void postload();
	void register_state_save();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_layer_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t *source_tilemap);

	uint8_t m_namcond1_gfxbank;

	tilemap_t *m_tilemap_A_cache_8[3];
	tilemap_t *m_tilemap_A_cache_16[3];
	tilemap_t *m_tilemap_B_cache_8[3];
	tilemap_t *m_tilemap_B_cache_16[3];
	tilemap_t *m_tilemap_A;
	tilemap_t *m_tilemap_B;
	bitmap_ind16 m_work_bitmap;

	void HandleYGV608Reset();
	void HandleRomTransfers(uint8_t type);
	void SetPreShortcuts(int reg, int data );
	void SetPostShortcuts(int reg);
	void ShowYGV608Registers();

	union {
		uint8_t       b[8];
		YGV_PORTS   s;
	} m_ports;

	union {
		uint8_t       b[50];
		YGV_REGS    s;
	} m_regs;

	/*
	*  Built in ram
	*/

	uint8_t m_pattern_name_table[4096];

	union {
		uint8_t           b[SPRITE_ATTR_TABLE_SIZE];
		SPRITE_ATTR     s[MAX_SPRITES];
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

	uint8_t m_screen_resize;  // screen requires resize
	uint8_t m_tilemap_resize; // tilemap requires resize
	
	/* These were statically allocated in the r/w routines */
	int p0_state_r,m_color_state_r;
	int p0_state_w,m_color_state_w;
	int pattern_name_base_r,pattern_name_base_w; 	 /* pattern name table base address */
	
	// === new variable handling starts here ===
	uint8_t m_register_address; /**< RN: Register address select */
	bool m_register_autoinc_r;  /**< RRAI: Register address auto-increment on read */
	bool m_register_autoinc_w;  /**< RWAI: Register address auto-increment on write */
	uint8_t m_screen_status; 	/**< CD: status port r/w */
	
	bool m_raster_irq_mask;		/**< IEP: raster irq mask (INT1 occurs if 1) */
	bool m_vblank_irq_mask;		/**< IEV: vblank irq mask (INT0 occurs if 1) */
	int m_raster_irq_hpos;		/**< IH: horizontal position where raster irq occurs x 32 */
	int m_raster_irq_vpos;		/**< IV: vertical position where raster irq occurs */
	bool m_raster_irq_mode; 	/**< FPM: if 1 vertical position becomes invalid for raster irqs (irqs occur for every line) */
	
	uint8_t m_scroll_address;	/**< SCA: scroll table access pointer */
	uint8_t m_palette_address;	/**< CC: color palette access pointer */
	uint8_t m_sprite_address;	/**< SAA: sprite attribute table access pointer */
	uint8_t m_sprite_bank;		/**< SBA: sprite generator base address (MA20 to MA13) */
	uint8_t m_xtile_ptr;		/**< PNX: X coordinate of pattern space */
	uint8_t m_ytile_ptr;		/**< PNY: Y coordinate of pattern space */
	bool m_xtile_autoinc;		/**< PNXA: Permits auto-increment in X coordinate */
	bool m_ytile_autoinc;		/**< PNXA: Permits auto-increment in Y coordinate */
	bool m_plane_select_access; /**< B/(A): A/B plane access select */
	
	uint8_t m_mosaic_aplane;	/**< MCA: mosaic factor applied to A plane */
	uint8_t m_mosaic_bplane;	/**< MCA: mosaic factor applied to B plane */
	bool m_sprite_disable;		/**< SPRD: disables the sprite plane display */
	bool m_sprite_aux_mode;		/**< SPAS: if 0 aux bits selects size, if 1 selects flipping */
	uint8_t m_sprite_aux_reg;	/**< SPA: auxiliary bits of sprite attribute table */
	uint8_t m_border_color;		/**< BDC: border color */
	
	// screen section
	devcb_write_line            m_vblank_handler;
	devcb_write_line            m_raster_handler;
	screen_device				*m_screen;
	emu_timer					*m_vblank_timer;
	emu_timer					*m_raster_timer;
	
	void screen_configure();		/**< Adjust screen parameters based off CRTC ones */
	attotime raster_sync_offset();	/**< Adjust timing based off raster & CRTC parameters */
	void vblank_irq_check();		/**< mask + pend check for vblank irq */
	void raster_irq_check();		/**< mask + pend check for raster irq */
	void pattern_name_autoinc_check();	/**< check autoinc for tile pointers */
	
	enum
	{
		VBLANK_TIMER,
		RASTER_TIMER
	};
	
	struct {
		int htotal;				/**< HTL: horizontal total number of dots x 2 */
		int vtotal;				/**< VTL: vertical total number of lines x 1 */
		int display_hstart;		/**< HDS: horizontal display starting position x 2*/
		int display_vstart;		/**< VDS: vertical display starting position x 1 */
		int display_width;		/**< HDW: horizontal display size x 16 */
		int display_height;		/**< VDW: vertical display size x 8 */
		int display_hsync;		/**< HSW: horizontal sync signal x 16 */
		int display_vsync;		/**< VSW: vertical sync signal x 1 */
		int border_width;		/**< HBW: horizontal border size x 16 */
		int border_height;		/**< VBW: vertical border size x 8 */
	}m_crtc;
	
	// rotation, zoom shortcuts
	uint32_t m_ax;				/**< AX */
	uint32_t m_dx;				/**< DX */
	uint32_t m_dxy;				/**< DXY */
	uint32_t m_ay;				/**< AY */
	uint32_t m_dy;				/**< DY */
	uint32_t m_dyx;				/**< DYX */

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


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_YGV608_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, YGV608, 0)

#define MCFG_YGV608_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)

#define MCFG_YGV608_VBLANK_HANDLER( _intcallb ) \
	devcb = &ygv608_device::static_set_vblank_callback( *device, DEVCB_##_intcallb );

#define MCFG_YGV608_RASTER_HANDLER( _intcallb ) \
	devcb = &ygv608_device::static_set_raster_callback( *device, DEVCB_##_intcallb );

	
#endif
