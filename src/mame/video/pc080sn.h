// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#ifndef MAME_VIDEO_PC080SN_H
#define MAME_VIDEO_PC080SN_H

#pragma once

class pc080sn_device : public device_t
{
public:
	pc080sn_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }
	void set_gfx_region(int gfxregion) { m_gfxnum = gfxregion; }
	void set_yinvert(int y_inv) { m_y_invert = y_inv; }
	void set_dblwidth(int dblwidth) { m_dblwidth = dblwidth; }
	void set_offsets(int x_offset, int y_offset)
	{
		m_x_offset = x_offset;
		m_y_offset = y_offset;
	}

	DECLARE_READ16_MEMBER( word_r );
	DECLARE_WRITE16_MEMBER( word_w );
	DECLARE_WRITE16_MEMBER( xscroll_word_w );
	DECLARE_WRITE16_MEMBER( yscroll_word_w );
	DECLARE_WRITE16_MEMBER( ctrl_word_w );

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);

	void common_get_pc080sn_bg_tile_info( tile_data &tileinfo, int tile_index, uint16_t *ram, int gfxnum );
	void common_get_pc080sn_fg_tile_info( tile_data &tileinfo, int tile_index, uint16_t *ram, int gfxnum );

	void set_scroll(int tilemap_num, int scrollx, int scrolly);
	void set_trans_pen(int tilemap_num, int pen);
	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, uint32_t priority);
	void tilemap_draw_offset(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, uint32_t priority, int xoffs, int yoffs);
	void topspeed_custom_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, uint32_t priority, uint16_t *color_ctrl_ram);

	/* For Topspeed */
	void tilemap_draw_special(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, uint32_t priority, uint16_t *ram);

	void restore_scroll();

	protected:
	// device-level overrides
	virtual void device_start() override;

	private:
	// internal state
	uint16_t         m_ctrl[8];

	std::unique_ptr<uint16_t[]>         m_ram;
	uint16_t         *m_bg_ram[2];
	uint16_t         *m_bgscroll_ram[2];

	int            m_bgscrollx[2], m_bgscrolly[2];

	tilemap_t      *m_tilemap[2];

	int            m_gfxnum;
	int            m_x_offset, m_y_offset;
	int            m_y_invert;
	int            m_dblwidth;

	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(PC080SN, pc080sn_device)


#define MCFG_PC080SN_GFX_REGION(_region) \
	downcast<pc080sn_device &>(*device).set_gfx_region(_region);

#define MCFG_PC080SN_OFFSETS(_xoffs, _yoffs) \
	downcast<pc080sn_device &>(*device).set_offsets(_xoffs, _yoffs);

#define MCFG_PC080SN_YINVERT(_yinv) \
	downcast<pc080sn_device &>(*device).set_yinvert(_yinv);

#define MCFG_PC080SN_DBLWIDTH(_dbl) \
	downcast<pc080sn_device &>(*device).set_dblwidth(_dbl);

#define MCFG_PC080SN_GFXDECODE(_gfxtag) \
	downcast<pc080sn_device &>(*device).set_gfxdecode_tag(_gfxtag);

#endif // MAME_VIDEO_PC080SN_H
