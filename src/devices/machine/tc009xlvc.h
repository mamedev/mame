// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************

    TC009xLVC device

***************************************************************************/

#ifndef MAME_MACHINE_TC009XLVC_H
#define MAME_MACHINE_TC009XLVC_H

#pragma once

#include "cpu/z80/z80.h"
#include "tilemap.h"

class tc0090lvc_device : public z80_device, public device_gfx_interface
{
public:
	typedef device_delegate<void (u32 &code)> tc009xlvc_cb_delegate;

	tc0090lvc_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	// configuration
	template <typename... T> void set_tile_callback(T &&... args) { m_tile_cb.set(std::forward<T>(args)...); }
	void set_tilemap_xoffs(int xoffs, int flipped_xoffs) { m_tilemap_xoffs = xoffs; m_tilemap_flipped_xoffs = flipped_xoffs; }
	void set_tilemap_yoffs(int yoffs, int flipped_yoffs) { m_tilemap_yoffs = yoffs; m_tilemap_flipped_yoffs = flipped_yoffs; }

	// memory handlers
	u8 rom_r(offs_t offset) { return m_rom[offset & (m_rom.length() - 1)]; }

	// internal functions
	u8 vregs_r(offs_t offset) { return m_vregs[offset]; }
	void vregs_w(offs_t offset, u8 data);
	u8 irq_vector_r(offs_t offset) { return m_irq_vector[offset]; }
	void irq_vector_w(offs_t offset, u8 data) { m_irq_vector[offset] = data; }
	u8 irq_enable_r() { return m_irq_enable; }
	void irq_enable_w(u8 data) { m_irq_enable = data; }
	u8 ram_bank_r(offs_t offset) { return m_ram_bank[offset]; }
	void ram_bank_w(offs_t offset, u8 data) { m_ram_bank[offset] = data; }
	u8 rom_bank_r() { return m_rom_bank; }
	void rom_bank_w(u8 data) { m_rom_bank = data; }

	// getters
	u8 irq_vector(offs_t offset) { return m_irq_vector[offset]; }
	u8 irq_enable() { return m_irq_enable; }

	// tilemap handlers
	void mark_all_layer_dirty();

	virtual u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	virtual void screen_eof();

	void cpu_map(address_map &map) ATTR_COLD;

protected:
	tc0090lvc_device(const machine_config &mconfig, device_type &type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_post_load() override;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// device_config_memory_interface overrides
	virtual space_config_vector memory_space_config() const override;

	// address space configurations
	const address_space_config m_program_space_config;
	const address_space_config m_vram_space_config;
	const address_space_config m_io_space_config;

	memory_access<20, 0, 0, ENDIANNESS_LITTLE>::specific m_vram_space;

	// vram space handlers
	const inline offs_t vram_addr(offs_t offset) { return (m_ram_bank[BIT(offset, 12, 2)] << 12) | (offset & 0xfff); }

	u8 banked_vram_r(offs_t offset) { return m_vram_space.read_byte(vram_addr(offset)); }
	void banked_vram_w(offs_t offset, u8 data) { m_vram_space.write_byte(vram_addr(offset), data); }

	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void update_scroll(u8 *ram);

	void vram_w(offs_t offset, u8 data);

	template<unsigned Offset> TILE_GET_INFO_MEMBER(get_tile_info);
	TILE_GET_INFO_MEMBER(get_tx_tile_info);

	void vram_map(address_map &map) ATTR_COLD;

	u8 m_bg_scroll[2][4];

	tilemap_t *m_bg_tilemap[2];
	tilemap_t *m_tx_tilemap;

	DECLARE_GFXDECODE_MEMBER(gfxinfo);

	int m_tilemap_xoffs, m_tilemap_yoffs;
	int m_tilemap_flipped_xoffs, m_tilemap_flipped_yoffs;

	u32 tilebank(u8 slot) const { return m_vregs[slot & 3]; }

	// other bits unknown
	bool screen_enable() const { return BIT(m_vregs[4], 5); }
	bool global_flip() const { return BIT(m_vregs[4], 4); }
	bool bg0_pri() const { return BIT(m_vregs[4], 3); }
	bool bitmap_mode() const { return (m_vregs[4] & 0x7) == 7; }

	u8 m_irq_vector[3];
	u8 m_irq_enable;
	u8 m_ram_bank[4];
	u8 m_rom_bank;
	std::unique_ptr<u8[]> m_vregs;
	std::unique_ptr<u8[]> m_sprram_buffer;

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
