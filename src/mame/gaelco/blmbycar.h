// license:BSD-3-Clause
// copyright-holders:Luca Elia
/***************************************************************************

    Blomby Car

***************************************************************************/

#include "gaelco_wrally_sprites.h"
#include "emupal.h"
#include "tilemap.h"

class blmbycar_state : public driver_device
{
public:
	blmbycar_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_sprites(*this, "sprites"),
		m_vram(*this, "vram_%u", 0U),
		m_scroll(*this, "scroll_%u", 0U),
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank"),
		m_pot_wheel_io(*this, "POT_WHEEL"),
		m_opt_wheel_io(*this, "OPT_WHEEL")
	{
	}

	void watrball(machine_config &config);
	void blmbycar(machine_config &config);

	void init_blmbycar();

private:
	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<gaelco_wrally_sprites_device> m_sprites;

	/* memory pointers */
	required_shared_ptr_array<uint16_t, 2> m_vram;
	required_shared_ptr_array<uint16_t, 2> m_scroll;
	required_shared_ptr<uint16_t> m_spriteram;

	required_memory_bank m_okibank;
	optional_ioport m_pot_wheel_io;
	optional_ioport m_opt_wheel_io;

	/* video-related */
	tilemap_t     *m_tilemap[2]{};

	/* input-related */
	uint8_t       m_pot_wheel = 0;    // blmbycar
	uint8_t       m_old_val = 0;  // blmbycar
	int         m_retvalue = 0; // waterball

	// common
	void okibank_w(uint8_t data);
	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	// blmbycar
	void blmbycar_pot_wheel_reset_w(uint8_t data);
	void blmbycar_pot_wheel_shift_w(uint8_t data);
	uint16_t blmbycar_pot_wheel_r();
	uint16_t blmbycar_opt_wheel_r();

	// waterball
	uint16_t waterball_unk_r();

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void video_start() override;
	DECLARE_MACHINE_START(blmbycar);
	DECLARE_MACHINE_RESET(blmbycar);
	DECLARE_MACHINE_START(watrball);
	DECLARE_MACHINE_RESET(watrball);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect );
	void blmbycar_map(address_map &map);
	void blmbycar_oki_map(address_map &map);
	void common_map(address_map &map);
	void watrball_map(address_map &map);
};
