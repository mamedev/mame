// license:BSD-3-Clause
// copyright-holders:Fabio Priuli,Acho A. Tang, R. Belmont
#ifndef MAME_VIDEO_K007342_H
#define MAME_VIDEO_K007342_H

#pragma once

#include "tilemap.h"


class k007342_device : public device_t
{
public:
	using tile_delegate = device_delegate<void (int layer, int bank, int *code, int *color, int *flags)>;

	k007342_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	//  configuration
	template <typename T> void set_gfxdecode_tag(T &&tag) { m_gfxdecode.set_tag(std::forward<T>(tag)); }
	void set_gfxnum(int gfxnum) { m_gfxnum = gfxnum; }
	template <typename... T> void set_tile_callback(T &&... args) { m_callback.set(std::forward<T>(args)...); }

	uint8_t read(offs_t offset);
	void write(offs_t offset, uint8_t data);
	uint8_t scroll_r(offs_t offset);
	void scroll_w(offs_t offset, uint8_t data);
	void vreg_w(offs_t offset, uint8_t data);

	void tilemap_update();
	void tilemap_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int num, int flags, uint32_t priority);
	int is_int_enabled();

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
private:
	// internal state
	std::unique_ptr<uint8_t[]>    m_ram;
	std::unique_ptr<uint8_t[]>    m_scroll_ram;
	uint8_t    *m_videoram_0;
	uint8_t    *m_videoram_1;
	uint8_t    *m_colorram_0;
	uint8_t    *m_colorram_1;

	tilemap_t  *m_tilemap[2]{};
	int      m_flipscreen, m_int_enabled;
	uint8_t    m_regs[8];
	uint16_t   m_scrollx[2];
	uint8_t    m_scrolly[2];
	required_device<gfxdecode_device> m_gfxdecode;
	tile_delegate m_callback;
	int m_gfxnum;

	TILEMAP_MAPPER_MEMBER(scan);
	TILE_GET_INFO_MEMBER(get_tile_info0);
	TILE_GET_INFO_MEMBER(get_tile_info1);
	void get_tile_info( tile_data &tileinfo, int tile_index, int layer, uint8_t *cram, uint8_t *vram );
};

DECLARE_DEVICE_TYPE(K007342, k007342_device)

// function definition for a callback
#define K007342_CALLBACK_MEMBER(_name)     void _name(int layer, int bank, int *code, int *color, int *flags)

#endif // MAME_VIDEO_K007342_H
