// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"

class suna16_state : public driver_device
{
public:
	suna16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_bank1(*this, "bank1"),
		m_bank2(*this, "bank2")


	{ }

	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_spriteram2;

	optional_memory_bank m_bank1;
	optional_memory_bank m_bank2;


	std::unique_ptr<uint16_t[]> m_paletteram;
	int m_color_bank;
	uint8_t m_prot;

	// common
	void soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t paletteram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void paletteram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// bestbest specific
	void bestbest_flipscreen_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bestbest_coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t bestbest_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void bestbest_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bestbest_ay8910_port_a_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// bssoccer specific
	void bssoccer_leds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bssoccer_pcm_1_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void bssoccer_pcm_2_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// uballoon specific
	void uballoon_leds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void uballoon_pcm_1_bankswitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t uballoon_prot_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void uballoon_prot_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void bssoccer_interrupt(timer_device &timer, void *ptr, int32_t param);

	void init_uballoon();
	virtual void video_start() override;
	void machine_start_bestbest();
	void machine_start_bssoccer();
	void machine_start_uballoon();
	void machine_reset_uballoon();

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bestbest(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, uint16_t *sprites, int gfx);
};
