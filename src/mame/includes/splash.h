// license:BSD-3-Clause
// copyright-holders:Manuel Abadia, David Haywood

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class splash_state : public driver_device
{
public:
	splash_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_msm1(*this, "msm1"),
		m_msm2(*this, "msm2"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_pixelram(*this, "pixelram"),
		m_videoram(*this, "videoram"),
		m_vregs(*this, "vregs"),
		m_spriteram(*this, "spriteram"),
		m_protdata(*this, "protdata"),
		m_bitmap_mode(*this, "bitmap_mode"),

		m_funystrp_val(0),
		m_funystrp_ff3cc7_val(0),
		m_funystrp_ff3cc8_val(0)
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<msm5205_device> m_msm;
	optional_device<msm5205_device> m_msm1;
	optional_device<msm5205_device> m_msm2;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;

	required_shared_ptr<uint16_t> m_pixelram;
	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_vregs;
	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_protdata;
	optional_shared_ptr<uint16_t> m_bitmap_mode;

	// driver init configuration
	int m_bitmap_type;
	int m_sprite_attr2_shift;

	tilemap_t *m_bg_tilemap[2];

	// splash specific
	int m_adpcm_data;

	//roldfrog specific
	int m_ret;
	int m_vblank_irq;
	int m_sound_irq;

	// funystrp specific
	uint8_t m_funystrp_val;
	uint8_t m_funystrp_ff3cc7_val;
	uint8_t m_funystrp_ff3cc8_val;
	int m_msm_data1;
	int m_msm_data2;
	int m_msm_toggle1;
	int m_msm_toggle2;
	int m_msm_source;
	int m_snd_interrupt_enable1;
	int m_snd_interrupt_enable2;

	// common
	void vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void coin_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	// splash specific
	void splash_msm5205_int(int state);
	void splash_sh_irqtrigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void splash_adpcm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	// roldfrog specific
	void roldf_sh_irqtrigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t roldfrog_bombs_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void roldfrog_vblank_ack_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t roldfrog_unk_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void ym_irq(int state);

	// funystrp specific
	uint16_t spr_read(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void spr_write(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t int_source_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void msm1_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm1_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm2_interrupt_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void msm2_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void adpcm_int1(int state);
	void adpcm_int2(int state);
	void funystrp_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t funystrp_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void funystrp_sh_irqtrigger_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	//roldfrog and funystrp specific
	void sound_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void init_splash10();
	void init_roldfrog();
	void init_splash();
	void init_rebus();
	void init_funystrp();
	virtual void video_start() override;
	void machine_start_splash();
	void machine_start_roldfrog();
	void machine_start_funystrp();
	void machine_reset_splash();
	void machine_reset_funystrp();

	void get_tile_info_tilemap0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_tilemap1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_funystrp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);
	void funystrp_draw_sprites(bitmap_ind16 &bitmap,const rectangle &cliprect);

	void roldfrog_interrupt(device_t &device);
	void roldfrog_update_irq(  );
};
