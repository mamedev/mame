// license:BSD-3-Clause
// copyright-holders:Ernesto Corvi, Jarek Parchanski, Nicola Salmoria, hap
/*************************************************************************

    Talbot - Champion Base Ball - Exciting Soccer

*************************************************************************/

#include "machine/alpha8201.h"
#include "machine/watchdog.h"


class champbas_state : public driver_device
{
public:
	champbas_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_alpha_8201(*this, "alpha_8201"),
		m_watchdog(*this, "watchdog"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_mainram(*this, "mainram"),
		m_vram(*this, "vram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_audiocpu;
	optional_device<alpha_8201_device> m_alpha_8201;
	required_device<watchdog_timer_device> m_watchdog;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint8_t> m_mainram;
	required_shared_ptr<uint8_t> m_vram;
	required_shared_ptr<uint8_t> m_spriteram;
	optional_shared_ptr<uint8_t> m_spriteram2;

	// internal state
	uint8_t m_irq_mask;
	tilemap_t *m_bg_tilemap;
	uint8_t m_gfx_bank;
	uint8_t m_palette_bank;

	// handlers
	void irq_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_switch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void mcu_start_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t champbja_protection_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	ioport_value watchdog_bit2(ioport_field &field, void *param);

	void vblank_irq(device_t &device);
	void exctsccr_sound_irq(timer_device &timer, void *ptr, int32_t param);

	void tilemap_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void gfxbank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void palette_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_exctsccr();
	void init_champbas();

	void palette_init_champbas(palette_device &palette);
	void palette_init_exctsccr(palette_device &palette);
	void video_start_champbas();
	void video_start_exctsccr();
	void champbas_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void exctsccr_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update_champbas(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_exctsccr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void champbas_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void exctsccr_draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override;
	virtual void machine_reset() override;
};
