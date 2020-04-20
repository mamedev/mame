// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC009xLVC device

***************************************************************************/

#ifndef MAME_MACHINE_TC009XLVC_H
#define MAME_MACHINE_TC009XLVC_H

#pragma once

#include "machine/bankdev.h"
#include "tilemap.h"

class tc0090lvc_device : public device_t, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 &code)> tc009xlvc_cb_delegate;

	tc0090lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }
	void set_tilemap_xoffs(int xoffs, int flipped_xoffs) { m_tilemap_xoffs = xoffs; m_tilemap_flipped_xoffs = flipped_xoffs; }
	void set_tilemap_yoffs(int yoffs, int flipped_yoffs) { m_tilemap_yoffs = yoffs; m_tilemap_flipped_yoffs = flipped_yoffs; }

	// memory handlers
	u8 rom_r(offs_t offset) { return m_rom[offset & m_rom.mask()]; }

	// internal functions
	u8 vregs_r(offs_t offset) { return m_vregs[offset]; }
	void vregs_w(offs_t offset, u8 data);
	u8 irq_vector_r(offs_t offset) { return m_irq_vector[offset]; }
	void irq_vector_w(offs_t offset, u8 data) { m_irq_vector[offset] = data; }
	u8 irq_enable_r() { return m_irq_enable; }
	void irq_enable_w(u8 data) { m_irq_enable = data; }
	u8 ram_bank_r(offs_t offset) { return m_ram_bank[offset]; }
	void ram_bank_w(offs_t offset, u8 data);
	u8 rom_bank_r() { return m_rom_bank; }
	void rom_bank_w(u8 data) { m_rom_bank = data; }

	// getters
	u8 irq_vector(offs_t offset) { return m_irq_vector[offset]; }
	u8 irq_enable() { return m_irq_enable; }

	// tilemap handlers
	void mark_all_layer_dirty();

	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void screen_eof();

	void cpu_map(address_map &map);

protected:
	tc0090lvc_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, u32 clock);

	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_post_load() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_scroll(u8 *ram);

	void vram_w(offs_t offset, u8 data);

	template<unsigned Offset> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void banked_map(address_map &map);

	u8 m_bg_scroll[2][4];

	tilemap_t *bg_tilemap[2];
	tilemap_t *tx_tilemap;

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	int m_tilemap_xoffs, m_tilemap_yoffs;
	int m_tilemap_flipped_xoffs, m_tilemap_flipped_yoffs;

	u32 tilebank(u8 slot) { return m_vregs[slot & 3]; }

	// other bits unknown
	bool screen_enable() { return BIT(m_vregs[4], 5); }
	bool global_flip() { return BIT(m_vregs[4], 4); }
	bool bg0_pri() { return BIT(m_vregs[4], 3); }
	bool bitmap_mode() { return (m_vregs[4] & 0x7) == 7; }

	u8 m_irq_vector[3];
	u8 m_irq_enable;
	u8 m_ram_bank[4];
	u8 m_rom_bank;
	std::unique_ptr<u8[]> m_vregs;
	std::unique_ptr<u8[]> m_sprram_buffer;

	required_device_array<address_map_bank_device, 4> m_bankdev;
	required_shared_ptr<u8> m_vram;
	required_shared_ptr<u8> m_bitmap_ram;
	required_region_ptr<u8> m_rom;

	tc009xlvc_cb_delegate m_tile_cb;
};

class tc0091lvc_device : public tc0090lvc_device
{
public:
	tc0091lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect) override;
	virtual void screen_eof() override;
};

DECLARE_DEVICE_TYPE(TC0090LVC, tc0090lvc_device)
DECLARE_DEVICE_TYPE(TC0091LVC, tc0091lvc_device)

#endif // MAME_MACHINE_TC009XLVC_H
