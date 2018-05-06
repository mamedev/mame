// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC0091LVC device

***************************************************************************/

#ifndef MAME_MACHINE_TL009XLVC_H
#define MAME_MACHINE_TL009XLVC_H

#pragma once


class tc0091lvc_device : public device_t, public device_memory_interface
{
public:
	tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_gfxdecode_tag(const char *tag) { m_gfxdecode.set_tag(tag); }

	DECLARE_READ8_MEMBER( vregs_r );
	DECLARE_WRITE8_MEMBER( vregs_w );

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof();

protected:
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;

private:
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t global_flip);

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

	void tc0091lvc_map8(address_map &map);

	uint8_t *m_pcg1_ram;
	uint8_t *m_pcg2_ram;
	uint8_t *m_vram0;
	uint8_t *m_vram1;
	uint8_t *m_sprram;
	uint8_t *m_tvram;
	uint8_t m_bg0_scroll[4];
	uint8_t m_bg1_scroll[4];

	tilemap_t *bg0_tilemap;
	tilemap_t *bg1_tilemap;
	tilemap_t *tx_tilemap;

	int m_gfx_index; // for RAM tiles

	uint8_t m_palette_ram[0x200];
	uint8_t m_vregs[0x100];
	uint8_t m_bitmap_ram[0x20000];
	uint8_t m_pcg_ram[0x10000];
	uint8_t m_sprram_buffer[0x400];

	address_space_config        m_space_config;
	required_device<gfxdecode_device> m_gfxdecode;
};

DECLARE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device)


#define MCFG_TC0091LVC_GFXDECODE(gfxtag) \
	downcast<tc0091lvc_device &>(*device).set_gfxdecode_tag(gfxtag);

#endif // MAME_MACHINE_TL009XLVC_H
