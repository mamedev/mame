// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_INCLUDES_POWERINS_H
#define MAME_INCLUDES_POWERINS_H

#include "emupal.h"
#include "screen.h"

class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vctrl_0(*this, "vctrl_0"),
		m_vram(*this, "vram_%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank")
	{ }

	void powerinsa(machine_config &config);
	void powerinsb(machine_config &config);
	void powerins(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_vctrl_0;
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_okibank;

	std::unique_ptr<uint16_t[]> m_spritebuffer[2];

	tilemap_t *m_tilemap[2];
	int m_tile_bank;

	DECLARE_WRITE8_MEMBER(powerinsa_okibank_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(tilebank_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(vram_w);
	DECLARE_READ8_MEMBER(powerinsb_fake_ym2203_r);

	DECLARE_MACHINE_START(powerinsa);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILEMAP_MAPPER_MEMBER(get_memory_offset_0);

	DECLARE_WRITE_LINE_MEMBER(screen_vblank);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void powerins_map(address_map &map);
	void powerins_sound_io_map(address_map &map);
	void powerins_sound_map(address_map &map);
	void powerinsa_map(address_map &map);
	void powerinsa_oki_map(address_map &map);
	void powerinsb_sound_io_map(address_map &map);
};

#endif // MAME_INCLUDES_POWERINS_H
