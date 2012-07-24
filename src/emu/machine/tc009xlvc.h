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

	DECLARE_WRITE8_MEMBER( tc0091lvc_bg0_scroll_w );
	DECLARE_WRITE8_MEMBER( tc0091lvc_bg1_scroll_w );

	UINT8 *m_palette_ram;
	UINT8 *m_vregs;
	UINT8 *m_bitmap_ram;
	
	UINT8 *m_pcg_ram;
	UINT8 *m_pcg1_ram;
	UINT8 *m_pcg2_ram;
	UINT8 *m_vram0;
	UINT8 *m_vram1;
	UINT8 *m_sprram;
	UINT8 *m_sprram_buffer;
	UINT8 *m_tvram;
	UINT8 m_bg0_scroll[4];
	UINT8 m_bg1_scroll[4];

	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *tx_tilemap;

	int m_gfx_index; // for RAM tiles

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect, UINT8 global_flip);
	void screen_eof(void);

protected:
	virtual void device_config_complete();
	virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start();
	virtual void device_reset();
	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const;
	address_space_config		m_space_config;
};

extern const device_type TC0091LVC;

#endif

