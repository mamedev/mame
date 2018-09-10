// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_EDEVICES_H
#define MAME_VIDEO_EDEVICES_H

#pragma once


class edevices_device : public device_t
{
public:
	edevices_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_tags(const char *bgram,const char *mlowram,const char *mhighram,const char *txram,const char *bgscrram,const char *mlowscrram,const char *mhighscrram,const char *attrram,const char *sprram,const char *gfx,const char *pal)
	{
		m_bg_videoram.set_tag(bgram);
		m_mlow_videoram.set_tag(mlowram);
		m_mhigh_videoram.set_tag(mhighram);
		m_tx_videoram.set_tag(txram);
		m_bg_scrollram.set_tag(bgscrram);
		m_mlow_scrollram.set_tag(mlowscrram);
		m_mhigh_scrollram.set_tag(mhighscrram);
		m_vidattrram.set_tag(attrram);
		m_spriteram.set_tag(sprram);
		m_gfxdecode.set_tag(gfx);
		m_palette.set_tag(pal);
	};

	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(tx_videoram_w);
	DECLARE_WRITE16_MEMBER(sprites_commands_w);

	uint32_t draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;
private:

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_mlow_tilemap;
	tilemap_t *m_mhigh_tilemap;
	tilemap_t *m_tx_tilemap;

	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_mlow_tile_info);
	TILE_GET_INFO_MEMBER(get_mhigh_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	/* misc */
	int m_which;

	uint16_t m_sprites_buffer[0x800];

	required_shared_ptr<uint16_t> m_bg_videoram;
	required_shared_ptr<uint16_t> m_mlow_videoram;
	required_shared_ptr<uint16_t> m_mhigh_videoram;
	required_shared_ptr<uint16_t> m_tx_videoram;
	required_shared_ptr<uint16_t> m_bg_scrollram;
	required_shared_ptr<uint16_t> m_mlow_scrollram;
	required_shared_ptr<uint16_t> m_mhigh_scrollram;
	required_shared_ptr<uint16_t> m_vidattrram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};

DECLARE_DEVICE_TYPE(EDEVICES_VID, edevices_device)

#endif // MAME_VIDEO_EDEVICES_H
