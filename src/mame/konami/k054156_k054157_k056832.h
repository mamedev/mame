// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_KONAMI_K054156_K054157_K056832_H
#define MAME_KONAMI_K054156_K054157_K056832_H

#pragma once

#include "k055555.h" // still needs k055555_get_palette_index
#include "tilemap.h"

#define K056832_CB_MEMBER(_name)   void _name(int layer, int *code, int *color, int *flags, int *priority)

#define K056832_PAGE_COUNT 16

/* bit depths for the 56832 */
#define K056832_BPP_4   0
#define K056832_BPP_5   1
#define K056832_BPP_6   2
#define K056832_BPP_8   3
#define K056832_BPP_4dj 4
#define K056832_BPP_4PIRATESH 5
#define K056832_BPP_8LE 6
#define K056832_BPP_8TASMAN 7

#define K056832_DRAW_FLAG_MIRROR      0x00800000
#define K056382_DRAW_FLAG_FORCE_XYSCROLL        0x00800000


class k055555_device;

class k056832_device : public device_t, public device_gfx_interface
{
public:
	using tile_delegate = device_delegate<void (int layer, int *code, int *color, int *flags, int *priority)>;

	template <typename T> k056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&mixer_tag)
		: k056832_device(mconfig, tag, owner, clock)
	{
		m_k055555.set_tag(std::forward<T>(mixer_tag));
	}

	k056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename... T> void set_tile_callback(T &&... args) { m_k056832_cb.set(std::forward<T>(args)...); }

	void set_config(int bpp, int big, int djmain_hack)
	{
		m_bpp = bpp;
		m_big = big;
		m_djmain_hack = djmain_hack;
	}

	void SetExtLinescroll();    /* Lethal Enforcers */

	u16 ram_word_r(offs_t offset);
	void ram_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ram_half_word_r(offs_t offset);
	void ram_half_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 unpaged_ram_word_r(offs_t offset);
	void unpaged_ram_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 k_5bpp_rom_word_r(offs_t offset, u16 mem_mask = ~0);
	u32 k_5bpp_rom_long_r(offs_t offset, u32 mem_mask = ~0);
	u32 k_6bpp_rom_long_r(offs_t offset, u32 mem_mask = ~0);
	u16 rom_word_r(offs_t offset);
	u8 konmedal_rom_r(offs_t offset);
	u16 piratesh_rom_r(offs_t offset);
	u16 mw_rom_word_r(offs_t offset);
	u16 bishi_rom_word_r(offs_t offset);
	u16 old_rom_word_r(offs_t offset);
	u16 rom_word_8000_r(offs_t offset);
	void word_w(offs_t offset, u16 data, u16 mem_mask = ~0); // "VRAM" registers
	void b_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u8 ram_code_lo_r(offs_t offset);
	u8 ram_code_hi_r(offs_t offset);
	u8 ram_attr_lo_r(offs_t offset);
	u8 ram_attr_hi_r(offs_t offset);
	void ram_code_lo_w(offs_t offset, u8 data);
	void ram_code_hi_w(offs_t offset, u8 data);
	void ram_attr_lo_w(offs_t offset, u8 data);
	void ram_attr_hi_w(offs_t offset, u8 data);
	void write(offs_t offset, u8 data);
	void b_w(offs_t offset, u8 data);
	void mark_plane_dirty(int num);
	void mark_all_tilemaps_dirty();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, uint32_t flags, uint32_t priority);
	void tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int num, uint32_t flags, uint32_t priority);
	void tilemap_draw_dj(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int layer, uint32_t flags, uint32_t priority);
	void set_layer_association(int status);
	int  get_layer_association();
	void set_layer_offs(int layer, int offsx, int offsy);
	void set_lsram_page(int logical_page, int physical_page, int physical_offset);
	void linemap_enable(int enable);
	int  is_irq_enabled(int irqline);
	void read_avac(int *mode, int *data);
	int  read_register(int regnum);
	int get_current_rambank();
	int get_lookup(int bits); /* Asterix */
	void set_tile_bank(int bank); /* Asterix */

	int get_gfx_num() const { return m_gfx_num; }

	u16 word_r(offs_t offset);        // VACSET
	u16 b_word_r(offs_t offset);      // VSCCS  (board dependent)


protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_post_load() override;

private:
	// internal state
	tilemap_t   *m_tilemap[K056832_PAGE_COUNT]{};
	bitmap_ind16  *m_pixmap[K056832_PAGE_COUNT]{};

	std::vector<uint16_t> m_videoram;

	uint16_t    m_regs[0x20];   // 157/832 regs group 1
	uint16_t    m_regsb[4]; // 157/832 regs group 2, board dependent

	required_region_ptr<uint8_t> m_rombase;   // pointer to tile gfx data

	int       m_num_gfx_banks;    // depends on size of graphics ROMs
	int       m_cur_gfx_banks;        // cached info for K056832_regs[0x1a]

	tile_delegate      m_k056832_cb;

	int                m_gfx_num;
	int                m_bpp;
	int                m_big;
	int                m_djmain_hack;

	// ROM readback involves reading 2 halves of a word
	// from the same location in a row.  Reading the
	// RAM window resets this state so you get the first half.
	int       m_rom_half = 0;

	// locally cached values
	int       m_layer_assoc_with_page[K056832_PAGE_COUNT];
	int       m_layer_offs[8][2];
	int       m_lsram_page[8][2];
	int32_t     m_x[8]; // 0..3 left
	int32_t     m_y[8]; // 0..3 top
	int32_t     m_w[8]; // 0..3 width  -> 1..4 pages
	int32_t     m_h[8]; // 0..3 height -> 1..4 pages
	int32_t     m_dx[8];    // scroll
	int32_t     m_dy[8];    // scroll
	uint32_t    m_line_dirty[K056832_PAGE_COUNT][8];
	uint8_t     m_all_lines_dirty[K056832_PAGE_COUNT];
	uint8_t     m_page_tile_mode[K056832_PAGE_COUNT];
	int       m_last_colorbase[K056832_PAGE_COUNT];
	uint8_t     m_layer_tile_mode[8];
	int       m_default_layer_association;
	int       m_layer_association;
	int       m_active_layer;
	int       m_selected_page;
	int       m_selected_page_x4096;
	int       m_linemap_enabled;
	int       m_use_ext_linescroll;
	int       m_uses_tile_banks, m_cur_tile_bank;

	optional_device<k055555_device> m_k055555;  /* used to choose colorbase */

	void get_tile_info(  tile_data &tileinfo, int tile_index, int pageIndex );

	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	TILE_GET_INFO_MEMBER(get_tile_info2);
	TILE_GET_INFO_MEMBER(get_tile_info3);
	TILE_GET_INFO_MEMBER(get_tile_info4);
	TILE_GET_INFO_MEMBER(get_tile_info5);
	TILE_GET_INFO_MEMBER(get_tile_info6);
	TILE_GET_INFO_MEMBER(get_tile_info7);
	TILE_GET_INFO_MEMBER(get_tile_info8);
	TILE_GET_INFO_MEMBER(get_tile_info9);
	TILE_GET_INFO_MEMBER(get_tile_infoa);
	TILE_GET_INFO_MEMBER(get_tile_infob);
	TILE_GET_INFO_MEMBER(get_tile_infoc);
	TILE_GET_INFO_MEMBER(get_tile_infod);
	TILE_GET_INFO_MEMBER(get_tile_infoe);
	TILE_GET_INFO_MEMBER(get_tile_infof);

	void mark_page_dirty(int page);
	void update_page_layout();
	void change_rambank();
	void change_rombank();
	int rom_read_b(int offset, int blksize, int blksize2, int zerosec);

	template<class BitmapClass>
	int update_linemap(screen_device &screen, BitmapClass &bitmap, int page, int flags);

	template<class BitmapClass>
	void tilemap_draw_common(screen_device &screen, BitmapClass &bitmap, const rectangle &cliprect, int layer, uint32_t flags, uint32_t priority);

	void create_gfx();
	void create_tilemaps();
	void finalize_init();

public:
	void m_tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int num, uint32_t flags, uint32_t priority);

private:
	int altK056832_update_linemap(screen_device &screen, bitmap_rgb32 &bitmap, int page, int flags);
};

DECLARE_DEVICE_TYPE(K056832, k056832_device)

#endif // MAME_KONAMI_K054156_K054157_K056832_H

