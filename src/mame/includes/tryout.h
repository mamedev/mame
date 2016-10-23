// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Bryan McPhail

#include "machine/gen_latch.h"

class tryout_state : public driver_device
{
public:
	tryout_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_gfx_control(*this, "gfx_control") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_spriteram;
	required_shared_ptr<uint8_t> m_spriteram2;
	required_shared_ptr<uint8_t> m_gfx_control;

	tilemap_t *m_fg_tilemap;
	tilemap_t *m_bg_tilemap;
	uint8_t m_vram_bank;
	std::unique_ptr<uint8_t[]> m_vram;
	std::unique_ptr<uint8_t[]> m_vram_gfx;

	void nmi_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void sound_irq_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t vram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void videoram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vram_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void flipscreen_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);

	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILEMAP_MAPPER_MEMBER(get_fg_memory_offset);
	TILEMAP_MAPPER_MEMBER(get_bg_memory_offset);

	virtual void machine_start() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(tryout);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
};
