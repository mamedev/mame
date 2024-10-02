// license:BSD-3-Clause
// copyright-holders:Luca Elia
/*************************************************************************

                      -= IGS Lord Of Gun =-

*************************************************************************/
#include "sound/okim6295.h"
#include "machine/eepromser.h"
#include "machine/gen_latch.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"

struct lordgun_gun_data
{
	int     scr_x,  scr_y;
	uint16_t  hw_x,   hw_y;
};

class lordgun_state : public driver_device
{
public:
	lordgun_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_eeprom(*this, "eeprom"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_priority_ram(*this, "priority_ram"),
		m_scrollram(*this, "scrollram"),
		m_spriteram(*this, "spriteram"),
		m_vram(*this, "vram.%u", 0),
		m_scroll_x(*this, "scroll_x.%u", 0),
		m_scroll_y(*this, "scroll_y.%u", 0) { }

	void aliencha(machine_config &config);
	void lordgun(machine_config &config);

	void init_aliencha();
	void init_lordgun();

private:
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<okim6295_device> m_oki;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;

	required_shared_ptr<uint16_t> m_priority_ram;
	required_shared_ptr<uint16_t> m_scrollram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr_array<uint16_t, 4> m_vram;
	required_shared_ptr_array<uint16_t, 4> m_scroll_x;
	required_shared_ptr_array<uint16_t, 4> m_scroll_y;

	uint8_t m_old = 0U;
	uint8_t m_aliencha_dip_sel = 0U;
	uint16_t m_priority = 0U;
	int m_whitescreen = 0;
	lordgun_gun_data m_gun[2]{};
	tilemap_t *m_tilemap[4]{};
	std::unique_ptr<bitmap_ind16> m_bitmaps[5]{};

	uint16_t m_protection_data = 0U;
	void lordgun_protection_w(offs_t offset, uint16_t data);
	uint16_t lordgun_protection_r(offs_t offset);
	void aliencha_protection_w(offs_t offset, uint16_t data);
	uint16_t aliencha_protection_r(offs_t offset);

	void priority_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t lordgun_gun_0_x_r();
	uint16_t lordgun_gun_0_y_r();
	uint16_t lordgun_gun_1_x_r();
	uint16_t lordgun_gun_1_y_r();
	void soundlatch_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	template<int Layer> void vram_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void fake_w(uint8_t data);
	void fake2_w(uint8_t data);
	void lordgun_eeprom_w(uint8_t data);
	void aliencha_eeprom_w(uint8_t data);
	uint8_t aliencha_dip_r();
	void aliencha_dip_w(uint8_t data);
	void lordgun_okibank_w(uint8_t data);

	template<int Layer> TILE_GET_INFO_MEMBER(get_tile_info);

	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void lorddgun_calc_gun_scr(int i);
	void lordgun_update_gun(int i);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void aliencha_map(address_map &map) ATTR_COLD;
	void aliencha_soundio_map(address_map &map) ATTR_COLD;
	void common_map(address_map &map) ATTR_COLD;
	void lordgun_map(address_map &map) ATTR_COLD;
	void lordgun_soundio_map(address_map &map) ATTR_COLD;
	void soundmem_map(address_map &map) ATTR_COLD;
	void ymf278_map(address_map &map) ATTR_COLD;
};

/*----------- defined in video/lordgun.c -----------*/
float lordgun_crosshair_mapper(const ioport_field *field, float linear_value);
