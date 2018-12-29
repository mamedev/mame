// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC0091LVC device

***************************************************************************/

#ifndef MAME_MACHINE_TL009XLVC_H
#define MAME_MACHINE_TL009XLVC_H

#pragma once


class tc0091lvc_device : public device_t, public device_gfx_interface, public device_memory_interface
{
public:
	tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( vregs_r );
	DECLARE_WRITE8_MEMBER( vregs_w );

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	WRITE_LINE_MEMBER(screen_vblank);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual space_config_vector memory_space_config() const override;

private:
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, uint8_t global_flip);

	static rgb_t tc0091lvc_xBGRBBBBGGGGRRRR(uint32_t raw);
	DECLARE_WRITE8_MEMBER( vram_w );

	template<int Offset> TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void tc0091lvc_map8(address_map &map);

	uint8_t m_bg_scroll[2][4];

	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_tx_tilemap;

	std::unique_ptr<uint8_t[]> m_vregs;
	std::unique_ptr<uint8_t[]> m_sprram_buffer;

	address_space_config        m_space_config;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_bitmap_ram;
};

DECLARE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device)

#endif // MAME_MACHINE_TL009XLVC_H
