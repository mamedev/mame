// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC0091LVC device

***************************************************************************/

#pragma once

#ifndef __ramdacDEV_H__
#define __ramdacDEV_H__

#include "emu.h"

class tc0091lvc_device : public device_t,
							public device_memory_interface
{
public:
	tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_gfxdecode_tag(device_t &device, const char *tag);

	DECLARE_READ8_MEMBER( vregs_r );
	DECLARE_WRITE8_MEMBER( vregs_w );

	DECLARE_READ8_MEMBER( tc0091lvc_paletteram_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_paletteram_w );
	DECLARE_READ8_MEMBER( tc0091lvc_bitmap_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_bitmap_w );
	DECLARE_READ8_MEMBER( tc0091lvc_pcg1_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_pcg1_w );
	DECLARE_READ8_MEMBER( tc0091lvc_pcg2_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_pcg2_w );
	DECLARE_READ8_MEMBER( tc0091lvc_vram0_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_vram0_w );
	DECLARE_READ8_MEMBER( tc0091lvc_vram1_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_vram1_w );
	DECLARE_READ8_MEMBER( tc0091lvc_spr_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_spr_w );
	DECLARE_READ8_MEMBER( tc0091lvc_tvram_r );
	DECLARE_WRITE8_MEMBER( tc0091lvc_tvram_w );

	TILE_GET_INFO_MEMBER(get_bg0_tile_info);
	TILE_GET_INFO_MEMBER(get_bg1_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	UINT8 *m_pcg1_ram;
	UINT8 *m_pcg2_ram;
	UINT8 *m_vram0;
	UINT8 *m_vram1;
	UINT8 *m_sprram;
	UINT8 *m_tvram;
	UINT8 m_bg0_scroll[4];
	UINT8 m_bg1_scroll[4];

	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *tx_tilemap;

	int m_gfx_index; // for RAM tiles

	UINT8 m_palette_ram[0x200];
	UINT8 m_vregs[0x100];
	UINT8 m_bitmap_ram[0x20000];
	UINT8 m_pcg_ram[0x10000];
	UINT8 m_sprram_buffer[0x400];

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 global_flip);
	void screen_eof(void);

protected:
	virtual void device_config_complete() override;
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;
	address_space_config        m_space_config;
	required_device<gfxdecode_device> m_gfxdecode;
};

extern const device_type TC0091LVC;

#define MCFG_TC0091LVC_GFXDECODE(_gfxtag) \
	tc0091lvc_device::static_set_gfxdecode_tag(*device, "^" _gfxtag);


#endif
