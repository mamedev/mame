// license:BSD-3-Clause
// copyright-holders:David Haywood
#pragma once
#ifndef __K056832_H__
#define __K056832_H__

#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

#include "video/k055555.h"// still needs k055555_get_palette_index


typedef device_delegate<void (int layer, int *code, int *color, int *flags)> k056832_cb_delegate;
#define K056832_CB_MEMBER(_name)   void _name(int layer, int *code, int *color, int *flags)

#define MCFG_K056832_CB(_class, _method) \
	k056832_device::set_k056832_callback(*device, k056832_cb_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

#define MCFG_K056832_CONFIG(_gfx_reg, _bpp, _big, _djmain_hack, _k055555) \
	k056832_device::set_config(*device, "^" _gfx_reg, _bpp, _big, _djmain_hack, _k055555);


#define K056832_PAGE_COUNT 16

/* bit depths for the 56832 */
#define K056832_BPP_4   0
#define K056832_BPP_5   1
#define K056832_BPP_6   2
#define K056832_BPP_8   3
#define K056832_BPP_4dj 4
#define K056832_BPP_8LE 5
#define K056832_BPP_8TASMAN 6

#define K056832_DRAW_FLAG_MIRROR      0x00800000
#define K056382_DRAW_FLAG_FORCE_XYSCROLL        0x00800000


class k056832_device : public device_t, public device_gfx_interface
{
public:
	k056832_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~k056832_device()
	{
		m_k055555 = nullptr;
	}

	static void set_k056832_callback(device_t &device, k056832_cb_delegate callback) { downcast<k056832_device &>(device).m_k056832_cb = callback; }
	static void set_config(device_t &device, const char *gfx_reg, int bpp, int big, int djmain_hack, const char *k055555)
	{
		k056832_device &dev = downcast<k056832_device &>(device);
		dev.m_rombase.set_tag(gfx_reg);
		dev.m_bpp = bpp;
		dev.m_big = big;
		dev.m_djmain_hack = djmain_hack;
		dev.m_k055555_tag = k055555;
	}

	void SetExtLinescroll();    /* Lethal Enforcers */

	uint16_t ram_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ram_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t ram_half_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void ram_half_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k_5bpp_rom_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint32_t k_5bpp_rom_long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t k_6bpp_rom_long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint16_t rom_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t mw_rom_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t bishi_rom_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t old_rom_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rom_word_8000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff); // "VRAM" registers
	void b_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t ram_code_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ram_code_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ram_attr_lo_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t ram_attr_hi_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ram_code_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ram_code_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ram_attr_lo_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void ram_attr_hi_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void b_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
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

	uint32_t ram_long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	uint32_t rom_long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void ram_long_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	uint32_t unpaged_ram_long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void unpaged_ram_long_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void long_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);
	void b_long_w(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	uint16_t word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);        // VACSET
	uint16_t b_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);      // VSCCS  (board dependent)
	uint32_t long_r(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);        // VACSET


protected:
	// device-level overrides
	virtual void device_start() override;

private:
	// internal state
	tilemap_t   *m_tilemap[K056832_PAGE_COUNT];
	bitmap_ind16  *m_pixmap[K056832_PAGE_COUNT];

	std::vector<uint16_t> m_videoram;

	uint16_t    m_regs[0x20];   // 157/832 regs group 1
	uint16_t    m_regsb[4]; // 157/832 regs group 2, board dependent

	required_region_ptr<uint8_t> m_rombase;   // pointer to tile gfx data

	int       m_num_gfx_banks;    // depends on size of graphics ROMs
	int       m_cur_gfx_banks;        // cached info for K056832_regs[0x1a]

	k056832_cb_delegate   m_k056832_cb;

	int                m_gfx_num;
	int                m_bpp;
	int                m_big;
	int                m_djmain_hack;

	const char         *m_k055555_tag;    // tbyahhoo uses the k056832 together with a k055555


	// ROM readback involves reading 2 halves of a word
	// from the same location in a row.  Reading the
	// RAM window resets this state so you get the first half.
	int       m_rom_half;

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




	k055555_device *m_k055555;  /* used to choose colorbase */

	void get_tile_info(  tile_data &tileinfo, int tile_index, int pageIndex );

	void get_tile_info0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info2(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info3(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info4(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info5(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info6(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info7(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info8(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info9(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infoa(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infob(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infoc(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infod(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infoe(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_infof(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	void mark_page_dirty(int page);
	void update_page_layout();
	void change_rambank();
	void change_rombank();
	void postload();
	int rom_read_b(int offset, int blksize, int blksize2, int zerosec);

	template<class _BitmapClass>
	int update_linemap(screen_device &screen, _BitmapClass &bitmap, int page, int flags);

	template<class _BitmapClass>
	void tilemap_draw_common(screen_device &screen, _BitmapClass &bitmap, const rectangle &cliprect, int layer, uint32_t flags, uint32_t priority);

	void create_gfx();
	void create_tilemaps();
	void finalize_init();

public:
	void m_tilemap_draw(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect, int num, uint32_t flags, uint32_t priority);

private:
	int altK056832_update_linemap(screen_device &screen, bitmap_rgb32 &bitmap, int page, int flags);
};

extern const device_type K056832;




#define MCFG_K056832_PALETTE(_palette_tag) \
	MCFG_GFX_PALETTE(_palette_tag)


#endif
