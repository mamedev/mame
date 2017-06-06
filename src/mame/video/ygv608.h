// license:BSD-3-Clause
// copyright-holders:Mark McDougall
#ifndef MAME_VIDEO_YGV608_H
#define MAME_VIDEO_YGV608_H

#pragma once

#include "tilemap.h"
/*
 *    Yamaha YGV608 - PVDC2 Pattern mode Video Display Controller 2
 *    - Mark McDougall
 */

class ygv608_device : public device_t, public device_gfx_interface
{
public:
	// construction/destruction
	ygv608_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE16_MEMBER( write );
	DECLARE_READ16_MEMBER( read );


	void set_gfxbank(uint8_t gfxbank);

	uint32_t update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER( timed_interrupt );

	// to be removed
	DECLARE_READ16_MEMBER( debug_trigger_r );
protected:
	// device-level overrides
	virtual void device_start() override;

private:
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
	void HandleRomTransfers();
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

	// rotation, zoom shortcuts
	uint32_t m_ax, m_dx, m_dxy, m_ay, m_dy, m_dyx;

	// base address shortcuts
	uint32_t m_base_addr[2][8];
	uint32_t m_base_y_shift;    // for extracting pattern y coord 'base'

	uint8_t m_screen_resize;  // screen requires resize
	uint8_t m_tilemap_resize; // tilemap requires resize
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

#endif
