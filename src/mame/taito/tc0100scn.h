// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_TAITO_TC0100SCN_H
#define MAME_TAITO_TC0100SCN_H

#pragma once

#include "emupal.h"
#include "tilemap.h"

enum {
	TC0100SCN_LAYOUT_DEFAULT = 0,
	TC0100SCN_LAYOUT_1BPP
};

enum {
	TC0620SCC_LAYOUT_DEFAULT = 0 // default TC0620SCC layout is 6bpp
};

#define TC0100SCN_CB_MEMBER(_name)   void _name(u32 *code, u16 *color)

class tc0100scn_base_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 *code, u16 *color)> tc0100scn_cb_delegate;

	// configuration
	void set_gfxlayout(int layout) { m_gfxlayout = layout; }
	template <typename... T> void set_tile_callback(T &&... args) { m_tc0100scn_cb.set(std::forward<T>(args)...); }
	void set_multiscr_xoffs(int xoffs) { m_multiscrn_xoffs = xoffs; }
	void set_multiscr_hack(int hack) { m_multiscrn_hack = hack; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}
	void set_offsets_flip(int x_offset, int y_offset)
	{
		m_flip_xoffs = x_offset;
		m_flip_yoffs = y_offset;
	}
	void set_offsets_fliptx(int x_offset, int y_offset)
	{
		m_flip_text_xoffs = x_offset;
		m_flip_text_yoffs = y_offset;
	}

	static constexpr unsigned SINGLE_VDU = 1024; // for set_multiscr_xoffs

	/* Function to set separate color banks for the three tilemapped layers.
	To change from the default (0,0,0) use after calling TC0100SCN_vh_start */
	void set_colbanks(int bg0, int bg1, int tx);

	u16 ram_r(offs_t offset);
	void ram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 ctrl_r(offs_t offset);
	void ctrl_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	void tilemap_update();
	int tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u8 priority, u8 pmask = 0xff);
	void tilemap_set_dirty();

	/* returns 0 or 1 depending on the lowest priority tilemap set in the internal
	register. Use this function to draw tilemaps in the correct order. */
	int bottomlayer();

protected:
	tc0100scn_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_post_load() override;

	int          m_gfxlayout;
private:
	// internal state
	tc0100scn_cb_delegate   m_tc0100scn_cb;

	u16          m_ctrl[8];

	std::unique_ptr<u16[]>    m_ram;
	u16 *        m_bgscroll_ram;
	u16 *        m_fgscroll_ram;
	u16 *        m_colscroll_ram;

	int          m_bgscrollx, m_bgscrolly, m_fgscrollx, m_fgscrolly;

	/* We keep two tilemaps for each of the 3 actual tilemaps: one at standard width, one double */
	tilemap_t    *m_tilemap[3][2]{};

	s32          m_bg_colbank[2], m_tx_colbank;
	int          m_dblwidth;
	bool         m_dirty;

	int          m_x_offset, m_y_offset;
	int          m_flip_xoffs, m_flip_yoffs;
	int          m_flip_text_xoffs, m_flip_text_yoffs;
	int          m_multiscrn_xoffs;
	int          m_multiscrn_hack;

	template<unsigned Offset, unsigned Colbank> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	template<unsigned Offset, unsigned Gfx> TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void tilemap_draw_fg(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, tilemap_t* tmap, int flags, u8 priority, u8 pmask = 0xff);
	void set_layer_ptrs();
	void restore_scroll();
};

class tc0100scn_device : public tc0100scn_base_device
{
public:
	tc0100scn_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo_default);
	DECLARE_GFXDECODE_MEMBER(gfxinfo_1bpp);
};

class tc0620scc_device : public tc0100scn_base_device
{
public:
	tc0620scc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	// decoding info
	DECLARE_GFXDECODE_MEMBER(gfxinfo_6bpp);
	std::unique_ptr<u8[]> m_decoded_gfx;
};

DECLARE_DEVICE_TYPE(TC0100SCN, tc0100scn_device)
DECLARE_DEVICE_TYPE(TC0620SCC, tc0620scc_device)

#endif // MAME_TAITO_TC0100SCN_H
