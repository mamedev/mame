// license:BSD-3-Clause
// copyright-holders:Luca Elia
#include "machine/nmk112.h"
#include "screen.h"

class powerins_state : public driver_device
{
public:
	powerins_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_vctrl_0(*this, "vctrl_0"),
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_spriteram(*this, "spriteram") { }


	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_vctrl_0;
	required_shared_ptr<uint16_t> m_vram_0;
	required_shared_ptr<uint16_t> m_vram_1;
	required_shared_ptr<uint16_t> m_spriteram;

	tilemap_t *m_tilemap_0;
	tilemap_t *m_tilemap_1;
	int m_tile_bank;

	DECLARE_WRITE8_MEMBER(powerinsa_okibank_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(tilebank_w);
	DECLARE_WRITE16_MEMBER(vram_0_w);
	DECLARE_WRITE16_MEMBER(vram_1_w);
	DECLARE_READ8_MEMBER(powerinsb_fake_ym2203_r);

	DECLARE_MACHINE_START(powerinsa);

	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	TILEMAP_MAPPER_MEMBER(get_memory_offset_0);

	virtual void video_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void powerinsa(machine_config &config);
	void powerinsb(machine_config &config);
	void powerins(machine_config &config);
	void powerins_map(address_map &map);
	void powerins_sound_io_map(address_map &map);
	void powerins_sound_map(address_map &map);
	void powerinsa_map(address_map &map);
	void powerinsa_oki_map(address_map &map);
	void powerinsb_sound_io_map(address_map &map);
};
