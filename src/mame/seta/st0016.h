// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/* ST0016 - CPU (z80) + Sound + Video */

#ifndef MAME_SETA_ST0016_H
#define MAME_SETA_ST0016_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/st0016.h"
#include "screen.h"


class st0016_cpu_device : public z80_device, public device_gfx_interface, public device_video_interface, public device_mixer_interface
{
public:
	enum
	{
		AS_CHARAM = AS_OPCODES + 1
	};

	typedef device_delegate<u8 ()> dma_offs_delegate;

	st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, u32);

	template <typename... T> void set_dma_offs_callback(T &&... args) { m_dma_offs_cb.set(std::forward<T>(args)...); }

	void set_game_flag(u32 flag) { m_game_flag = flag; }

	void draw_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	const address_space_config m_io_space_config;
	const address_space_config m_space_config;
	const address_space_config m_charam_space_config;

	virtual space_config_vector memory_space_config() const override;

private:
	bool ismacs() const { return m_game_flag & 0x80; }
	bool ismacs1() const { return (m_game_flag & 0x180) == 0x180; }
	bool ismacs2() const { return (m_game_flag & 0x180) == 0x080; }

	static constexpr unsigned MAX_SPR_BANK   = 0x10;
	static constexpr unsigned MAX_CHAR_BANK  = 0x10000;
	static constexpr unsigned MAX_PAL_BANK   = 4;

	static constexpr unsigned SPR_BANK_SIZE  = 0x1000;
	static constexpr unsigned CHAR_BANK_SIZE = 0x20;
	static constexpr unsigned PAL_BANK_SIZE  = 0x200;

	static constexpr unsigned UNUSED_PEN = 1024;

	static constexpr unsigned SPR_BANK_MASK  = MAX_SPR_BANK - 1;
	static constexpr unsigned CHAR_BANK_MASK = MAX_CHAR_BANK - 1;
	static constexpr unsigned PAL_BANK_MASK  = MAX_PAL_BANK - 1;

	required_memory_region m_rom;
	memory_share_creator<u8> m_spriteram;
	memory_share_creator<u8> m_charram;
	memory_share_creator<u8> m_paletteram;

	u8 m_dma_offset;
	dma_offs_delegate m_dma_offs_cb;
	u32 m_game_flag;

	memory_access<21, 0, 0, ENDIANNESS_LITTLE>::specific m_charam_space;

	void sprite_bank_w(u8 data);
	void palette_bank_w(u8 data);
	void character_bank_w(offs_t offset, u8 data);
	u8 sprite_ram_r(offs_t offset);
	void sprite_ram_w(offs_t offset, u8 data);
	u8 sprite2_ram_r(offs_t offset);
	void sprite2_ram_w(offs_t offset, u8 data);
	u8 palette_ram_r(offs_t offset);
	void palette_ram_w(offs_t offset, u8 data);
	u8 charam_bank_r(offs_t offset);
	void charam_bank_w(offs_t offset, u8 data);
	u8 character_ram_r(offs_t offset);
	void character_ram_w(offs_t offset, u8 data);
	u8 vregs_r(offs_t offset);
	u8 dma_r();
	void vregs_w(offs_t offset, u8 data);

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void save_init();
	void draw_bgmap(bitmap_ind16 &bitmap,const rectangle &cliprect, int priority);

	void startup();

	u32 m_spr_bank, m_spr2_bank, m_pal_bank, m_char_bank;
	int m_spr_dx, m_spr_dy;

	u8 m_vregs[0xc0];
	u8 m_ramgfx;

	void cpu_internal_io_map(address_map &map) ATTR_COLD;
	void cpu_internal_map(address_map &map) ATTR_COLD;
	void charam_map(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ST0016_CPU, st0016_cpu_device)


#endif // MAME_SETA_ST0016_H
