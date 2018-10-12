// license:BSD-3-Clause
// copyright-holders:Luca Elia

/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

#include "machine/gen_latch.h"
#include "screen.h"
#include "emupal.h"

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_vram(*this, "vram_%u", 0U),
		m_scrollram(*this, "scrollram_%u", 0U),
		m_priorityram(*this, "priorityram"),
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank") { }

	void magicbub(machine_config &config);
	void shocking(machine_config &config);

	void init_magicbub();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch; // magicbub

	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr_array<uint16_t, 2> m_scrollram;
	required_shared_ptr<uint16_t> m_priorityram;
	required_shared_ptr<uint16_t> m_spriteram;

	optional_memory_bank m_okibank;

	/* other video-related elements */
	tilemap_t     *m_tilemap[2];
	int         m_sprites_scrolldx;
	int         m_sprites_scrolldy;

	DECLARE_WRITE8_MEMBER(sound_bank_w);
	DECLARE_WRITE16_MEMBER(magicbub_sound_command_w);
	template<int Layer> DECLARE_WRITE16_MEMBER(vram_w);

	TILEMAP_MAPPER_MEMBER(tilemap_scan_pages);
	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );

	void main_map(address_map &map);
	void oki_map(address_map &map);
	void sound_map(address_map &map);
	void sound_port_map(address_map &map);
};
