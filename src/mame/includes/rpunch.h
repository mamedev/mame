// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "sound/upd7759.h"

class rpunch_state : public driver_device
{
public:
	rpunch_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_upd7759(*this, "upd"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_videoram(*this, "videoram"),
		m_bitmapram(*this, "bitmapram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<upd7759_device> m_upd7759;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_videoram;
	required_shared_ptr<uint16_t> m_bitmapram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint8_t m_sound_data;
	uint8_t m_sound_busy;
	uint8_t m_ym2151_irq;
	uint8_t m_upd_rom_bank;
	int m_sprite_palette;
	int m_sprite_xoffs;
	uint16_t m_videoflags;
	uint8_t m_crtc_register;
	uint8_t m_bins;
	uint8_t m_gins;
	tilemap_t *m_background[2];
	emu_timer *m_crtc_timer;
	void ym2151_irq_gen(int state);
	void sound_command_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t sound_command_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint16_t sound_busy_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rpunch_videoram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rpunch_videoreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rpunch_scrollreg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rpunch_crtc_data_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rpunch_crtc_register_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rpunch_ins_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value hi_bits_r(ioport_field &field, void *param);
	void upd_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void upd_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_rabiolep();
	void init_svolley();
	void get_bg0_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_bg1_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	virtual void machine_start() override;
	virtual void machine_reset() override;

	void video_start_rpunch();
	void video_start_svolley();


	uint32_t screen_update_rpunch(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void sound_command_w_callback(void *ptr, int32_t param);
	void crtc_interrupt_gen(void *ptr, int32_t param);
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int start, int stop);
	void draw_bitmap(bitmap_ind16 &bitmap, const rectangle &cliprect);
};
