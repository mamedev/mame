// license:BSD-3-Clause
// copyright-holders:David Haywood
#ifndef MAME_VIDEO_EDEVICES_H
#define MAME_VIDEO_EDEVICES_H

#pragma once

#include "screen.h"
#include "emupal.h"

class edevices_device : public device_t
{
public:
	edevices_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);
	edevices_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	template <typename T> void set_bg_videoram_tag(T &&tag) { m_bg_videoram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mlow_videoram_tag(T &&tag) { m_mlow_videoram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mhigh_videoram_tag(T &&tag) { m_mhigh_videoram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_tx_videoram_tag(T &&tag) { m_tx_videoram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_bg_scrollram_tag(T &&tag) { m_bg_scrollram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mlow_scrollram_tag(T &&tag) { m_mlow_scrollram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_mhigh_scrollram_tag(T &&tag) { m_mhigh_scrollram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_vidattrram_tag(T &&tag) { m_vidattrram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_spriteram_tag(T &&tag) { m_spriteram.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	template <typename T> void set_palette_tag(T &&tag) { m_palette.set_tag(std::forward<T>(tag)); }

	void set_spritexoffset(int offset)
	{
		m_spritexoffs = offset;
	}

	DECLARE_WRITE16_MEMBER(bg_videoram_w);
	DECLARE_WRITE16_MEMBER(mlow_videoram_w);
	DECLARE_WRITE16_MEMBER(mhigh_videoram_w);
	DECLARE_WRITE16_MEMBER(tx_videoram_w);
	DECLARE_WRITE16_MEMBER(sprites_commands_w);

	uint32_t draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual int get_priority(const uint16_t *source);
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

	uint16_t m_sprites_buffer[0x400];

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

	/* misc */
	int m_which;
	int m_spritexoffs;
};

class edevices_sforce_device : public edevices_device
{
public:
	edevices_sforce_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
protected:
	virtual int get_priority(const uint16_t *source) override;
};


DECLARE_DEVICE_TYPE(EDEVICES_VID, edevices_device)
DECLARE_DEVICE_TYPE(EDEVICES_SFORCE_VID, edevices_sforce_device)

#endif // MAME_VIDEO_EDEVICES_H
