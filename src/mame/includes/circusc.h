// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria,Chris Hardy, Couriersud
/*************************************************************************

    Circus Charlie

*************************************************************************/

#include "sound/dac.h"
#include "sound/sn76496.h"
#include "sound/discrete.h"
class circusc_state : public driver_device
{
public:
	circusc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_spritebank(*this, "spritebank"),
		m_scroll(*this, "scroll"),
		m_colorram(*this, "colorram"),
		m_videoram(*this, "videoram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_spriteram(*this, "spriteram"),
		m_audiocpu(*this, "audiocpu"),
		m_sn_1(*this, "sn1"),
		m_sn_2(*this, "sn2"),
		m_dac(*this, "dac"),
		m_discrete(*this, "fltdisc"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<uint8_t> m_spritebank;
	required_shared_ptr<uint8_t> m_scroll;
	required_shared_ptr<uint8_t> m_colorram;
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram_2;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t        *m_bg_tilemap;

	/* sound-related */
	uint8_t          m_sn_latch;

	/* devices */
	required_device<cpu_device> m_audiocpu;
	required_device<sn76496_device> m_sn_1;
	required_device<sn76496_device> m_sn_2;
	required_device<dac_byte_interface> m_dac;
	required_device<discrete_device> m_discrete;

	uint8_t          m_irq_mask;
	uint8_t circusc_sh_timer_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void circusc_sh_irqtrigger_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void circusc_coin_counter_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void circusc_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void irq_mask_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void circusc_videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void circusc_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void circusc_flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_circusc();
	void get_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void palette_init_circusc(palette_device &palette);
	uint32_t screen_update_circusc(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vblank_irq(device_t &device);
	void draw_sprites( bitmap_ind16 &bitmap, const rectangle &cliprect );
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
