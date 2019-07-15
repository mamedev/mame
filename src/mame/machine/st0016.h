// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina, David Haywood
/* ST0016 - CPU (z80) + Sound + Video */

#ifndef MAME_MACHINE_ST0016_H
#define MAME_MACHINE_ST0016_H

#pragma once

#include "cpu/z80/z80.h"
#include "sound/st0016.h"
#include "screen.h"


class st0016_cpu_device : public z80_device, public device_gfx_interface
{
public:
	typedef device_delegate<uint8_t ()> dma_offs_delegate;

	st0016_cpu_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t);

	template <typename... T> void set_dma_offs_callback(T &&... args) { m_dma_offs_cb = dma_offs_delegate(std::forward<T>(args)...); }

	DECLARE_WRITE8_MEMBER(st0016_sprite_bank_w);
	DECLARE_WRITE8_MEMBER(st0016_palette_bank_w);
	DECLARE_WRITE8_MEMBER(st0016_character_bank_w);
	DECLARE_READ8_MEMBER(st0016_sprite_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_sprite_ram_w);
	DECLARE_READ8_MEMBER(st0016_sprite2_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_sprite2_ram_w);
	DECLARE_READ8_MEMBER(st0016_palette_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_palette_ram_w);
	DECLARE_READ8_MEMBER(st0016_character_ram_r);
	DECLARE_WRITE8_MEMBER(st0016_character_ram_w);
	DECLARE_READ8_MEMBER(st0016_vregs_r);
	DECLARE_READ8_MEMBER(st0016_dma_r);
	DECLARE_WRITE8_MEMBER(st0016_vregs_w);

	void set_st0016_game_flag(uint32_t flag) { m_game_flag = flag; }

	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void st0016_save_init();
	void draw_bgmap(bitmap_ind16 &bitmap,const rectangle &cliprect, int priority);

	void startup();
	uint32_t update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void st0016_draw_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	std::unique_ptr<uint8_t[]> st0016_spriteram;
	std::unique_ptr<uint8_t[]> st0016_paletteram;


	int32_t st0016_spr_bank,st0016_spr2_bank,st0016_pal_bank,st0016_char_bank;
	int spr_dx,spr_dy;

	uint8_t st0016_vregs[0xc0];
	int st0016_ramgfx;
	std::unique_ptr<uint8_t[]> m_charram;

	void st0016_cpu_internal_io_map(address_map &map);
	void st0016_cpu_internal_map(address_map &map);
protected:
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


	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;

	const address_space_config m_io_space_config;
	const address_space_config m_space_config;


	virtual space_config_vector memory_space_config() const override;

	required_device<screen_device> m_screen;
	required_memory_region m_rom;

private:
	uint8_t m_dma_offset;
	dma_offs_delegate m_dma_offs_cb;
	uint32_t m_game_flag;

	DECLARE_READ8_MEMBER(soundram_read);
};


// device type definition
DECLARE_DEVICE_TYPE(ST0016_CPU, st0016_cpu_device)


#endif // MAME_MACHINE_ST0016_H
