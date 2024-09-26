// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    Sega 16-bit common hardware

***************************************************************************/
#ifndef MAME_SEGA_SEGAIC16_H
#define MAME_SEGA_SEGAIC16_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

typedef device_delegate<void (int, uint16_t*, uint16_t*, uint16_t*, uint16_t*)> segaic16_video_pagelatch_delegate;


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> sega_16bit_common_base

class sega_16bit_common_base : public driver_device
{
public:
	// palette helpers
	void paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void hangon_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void philko_paletteram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

protected:
	// construction/destruction
	sega_16bit_common_base(const machine_config &mconfig, device_type type, const char *tag);

	// internal helpers
	void palette_init();

public: // -- stupid system16.cpp
	// memory pointers
	required_shared_ptr<u16> m_paletteram;
protected:

	// internal state
	u32      m_palette_entries;          // number of palette entries
	u8       m_palette_normal[32];       // RGB translations for normal pixels
	u8       m_palette_shadow[32];       // RGB translations for shadowed pixels
	u8       m_palette_hilight[32];      // RGB translations for hilighted pixels
	required_device<palette_device> m_palette;
};


class segaic16_video_device :   public device_t,
								public device_video_interface
{
public:
	/* tilemap systems */
	static constexpr unsigned MAX_TILEMAPS       = 1;

	static constexpr unsigned TILEMAP_HANGON     = 0;
	static constexpr unsigned TILEMAP_16A        = 1;
	static constexpr unsigned TILEMAP_16B        = 2;
	static constexpr unsigned TILEMAP_16B_ALT    = 3;

	static constexpr unsigned TILEMAP_FOREGROUND = 0;
	static constexpr unsigned TILEMAP_BACKGROUND = 1;
	static constexpr unsigned TILEMAP_TEXT       = 2;


	/* rotation systems */
	static constexpr unsigned MAX_ROTATE         = 1;
	static constexpr unsigned ROTATE_YBOARD      = 0;


	struct tilemap_callback_info
	{
		uint16_t *                  rambase = nullptr;              // base of RAM for this tilemap page
		const uint8_t *             bank = nullptr;                 // pointer to bank array
		uint16_t                    banksize = 0;                   // size of banks
	};

	struct tilemap_info
	{
		using reset_func = void (*)(screen_device &screen, tilemap_info *info);
		using draw_layer_func = void (*)(screen_device &screen, tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);

		uint8_t                     index = 0;                          // index of this structure
		uint8_t                     type = 0;                           // type of tilemap (see segaic16.h for details)
		uint8_t                     numpages = 0;                       // number of allocated pages
		uint8_t                     flip = 0;                           // screen flip?
		uint8_t                     rowscroll = 0, colscroll = 0;       // are rowscroll/colscroll enabled (if external enables are used)
		uint8_t                     bank[8] = {0,0,0,0,0,0,0,0};        // indexes of the tile banks
		uint16_t                    banksize = 0;                       // number of tiles per bank
		uint16_t                    latched_xscroll[4] = {0,0,0,0};     // latched X scroll values
		uint16_t                    latched_yscroll[4] = {0,0,0,0};     // latched Y scroll values
		uint16_t                    latched_pageselect[4] = {0,0,0,0};  // latched page select values
		int32_t                     xoffs = 0;                          // X scroll offset
		tilemap_t *                 tilemaps[16]{};                     // up to 16 tilemap pages
		tilemap_t *                 textmap = nullptr;                  // a single text tilemap
		tilemap_callback_info       tmap_info[16];                      // callback info for 16 tilemap pages
		tilemap_callback_info       textmap_info;                       // callback info for a single textmap page
		reset_func                  reset = nullptr;                    // reset callback
		draw_layer_func             draw_layer = nullptr;
		uint16_t *                  textram = nullptr;                  // pointer to textram pointer
		uint16_t *                  tileram = nullptr;                  // pointer to tileram pointer
		emu_timer *                 latch_timer = nullptr;              // timer for latching 16b tilemap scroll values
	};


	struct rotate_info
	{
		uint8_t                     index = 0;                          // index of this structure
		uint8_t                     type = 0;                           // type of rotate system (see segaic16.h for details)
		uint16_t                    colorbase = 0;                      // base color index
		int32_t                     ramsize = 0;                        // size of rotate RAM
		uint16_t *                  rotateram = nullptr;                // pointer to rotateram pointer
		std::unique_ptr<uint16_t[]> buffer;                             // buffered data
	};

	template <typename T> segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&decode_tag)
		: segaic16_video_device(mconfig, tag, owner, clock)
	{
		m_gfxdecode.set_tag(std::forward<T>(decode_tag));
	}

	segaic16_video_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template <typename... T> void set_pagelatch_cb(T &&... args) { m_pagelatch_cb.set(std::forward<T>(args)...); }

	uint8_t m_display_enable;
	optional_shared_ptr<uint16_t> m_tileram;
	optional_shared_ptr<uint16_t> m_textram;
	optional_shared_ptr<uint16_t> m_rotateram;

	void tilemap_set_colscroll(int which, int enable);
	void tilemap_set_rowscroll(int which, int enable);
	void tilemap_set_flip(int which, int flip);
	void tilemap_set_bank(int which, int banknum, int offset);
	void tilemap_reset(screen_device &screen);
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int map, int priority, int priority_mark);
//  void tilemap_16b_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void tilemap_16a_draw_layer(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, int which, int flags, int priority);
//  void draw_virtual_tilemap(screen_device &screen, struct tilemap_info *info, bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t pages, uint16_t xscroll, uint16_t yscroll, uint32_t flags, uint32_t priority);
//  void tilemap_16b_reset(screen_device &screen, struct tilemap_info *info);

	segaic16_video_pagelatch_delegate m_pagelatch_cb;
	void tilemap_16b_fill_latch(int i, uint16_t* latched_pageselect, uint16_t* latched_yscroll, uint16_t* latched_xscroll, uint16_t* textram);
	TIMER_CALLBACK_MEMBER( tilemap_16b_latch_values );

	struct rotate_info m_rotate[MAX_ROTATE];
	struct tilemap_info m_bg_tilemap[MAX_TILEMAPS];

	void set_display_enable(int enable);
	void tilemap_init(int which, int type, int colorbase, int xoffs, int numbanks);
	void rotate_init(int which, int type, int colorbase);

	uint16_t tileram_r(offs_t offset);
	uint16_t textram_r(offs_t offset);
	void tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void textram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void rotate_draw(int which, bitmap_ind16 &bitmap, const rectangle &cliprect, bitmap_ind8 &priority_bitmap, bitmap_ind16 &srcbitmap);

	uint16_t rotate_control_r();

	TILE_GET_INFO_MEMBER( tilemap_16b_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_text_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_alt_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16b_alt_text_info );

	TILE_GET_INFO_MEMBER( tilemap_16a_tile_info );
	TILE_GET_INFO_MEMBER( tilemap_16a_text_info );

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	// internal state
	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(SEGAIC16VID, segaic16_video_device)

#endif // MAME_SEGA_SEGAIC16_H
