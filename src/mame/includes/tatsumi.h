// license:BSD-3-Clause
// copyright-holders:Bryan McPhail

#include "sound/okim6295.h"
#include "cpu/m68000/m68000.h"
#include "machine/gen_latch.h"

class tatsumi_state : public driver_device
{
public:
	tatsumi_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_videoram(*this, "videoram"),
		m_cyclwarr_cpua_ram(*this, "cw_cpua_ram"),
		m_cyclwarr_cpub_ram(*this, "cw_cpub_ram"),
		m_apache3_g_ram(*this, "apache3_g_ram"),
		m_roundup5_d0000_ram(*this, "ru5_d0000_ram"),
		m_roundup5_e0000_ram(*this, "ru5_e0000_ram"),
		m_roundup5_unknown0(*this, "ru5_unknown0"),
		m_roundup5_unknown1(*this, "ru5_unknown1"),
		m_roundup5_unknown2(*this, "ru5_unknown2"),
		m_68k_ram(*this, "68k_ram"),
		m_apache3_z80_ram(*this, "apache3_z80_ram"),
		m_sprite_control_ram(*this, "sprite_ctlram"),
		m_cyclwarr_videoram0(*this, "cw_videoram0"),
		m_cyclwarr_videoram1(*this, "cw_videoram1"),
		m_roundup_r_ram(*this, "roundup_r_ram"),
		m_roundup_p_ram(*this, "roundup_p_ram"),
		m_roundup_l_ram(*this, "roundup_l_ram"),
		m_spriteram(*this, "spriteram") { }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<m68000_base_device> m_subcpu;
	optional_device<cpu_device> m_subcpu2;
	required_device<okim6295_device> m_oki;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<generic_latch_8_device> m_soundlatch;

	optional_shared_ptr<uint16_t> m_videoram;
	optional_shared_ptr<uint16_t> m_cyclwarr_cpua_ram;
	optional_shared_ptr<uint16_t> m_cyclwarr_cpub_ram;
	optional_shared_ptr<uint16_t> m_apache3_g_ram;
	optional_shared_ptr<uint16_t> m_roundup5_d0000_ram;
	optional_shared_ptr<uint16_t> m_roundup5_e0000_ram;
	optional_shared_ptr<uint16_t> m_roundup5_unknown0;
	optional_shared_ptr<uint16_t> m_roundup5_unknown1;
	optional_shared_ptr<uint16_t> m_roundup5_unknown2;
	optional_shared_ptr<uint16_t> m_68k_ram;
	optional_shared_ptr<uint8_t> m_apache3_z80_ram;
	required_shared_ptr<uint16_t> m_sprite_control_ram;
	optional_shared_ptr<uint16_t> m_cyclwarr_videoram0;
	optional_shared_ptr<uint16_t> m_cyclwarr_videoram1;
	optional_shared_ptr<uint16_t> m_roundup_r_ram;
	optional_shared_ptr<uint16_t> m_roundup_p_ram;
	optional_shared_ptr<uint16_t> m_roundup_l_ram;
	required_shared_ptr<uint16_t> m_spriteram;

	uint16_t m_bigfight_a20000[8];
	uint16_t m_bigfight_a60000[2];
	uint16_t m_bigfight_a40000[2];
	uint8_t *m_rom_sprite_lookup1;
	uint8_t *m_rom_sprite_lookup2;
	uint8_t *m_rom_clut0;
	uint8_t *m_rom_clut1;
	uint16_t m_control_word;
	uint16_t m_apache3_rotate_ctrl[12];
	uint16_t m_last_control;
	uint8_t m_apache3_adc;
	int m_apache3_rot_idx;
	tilemap_t *m_tx_layer;
	tilemap_t *m_layer0;
	tilemap_t *m_layer1;
	tilemap_t *m_layer2;
	tilemap_t *m_layer3;
	bitmap_rgb32 m_temp_bitmap;
	std::unique_ptr<uint8_t[]> m_apache3_road_x_ram;
	uint8_t m_apache3_road_z;
	std::unique_ptr<uint16_t[]> m_roundup5_vram;
	uint16_t m_bigfight_bank;
	uint16_t m_bigfight_last_bank;
	uint8_t m_roundupt_crt_selected_reg;
	uint8_t m_roundupt_crt_reg[64];
	std::unique_ptr<uint8_t[]> m_shadow_pen_array;
	uint16_t cyclwarr_sprite_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cyclwarr_sprite_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bigfight_a20000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bigfight_a40000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void bigfight_a60000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cyclwarr_sound_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t apache3_bank_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void apache3_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void apache3_z80_ctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t apache3_v30_v20_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void apache3_v30_v20_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t apache3_z80_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void apache3_z80_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t apache3_adc_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void apache3_adc_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void apache3_rotate_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t roundup_v30_z80_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void roundup_v30_z80_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roundup5_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roundup5_d0000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roundup5_e0000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cyclwarr_control_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cyclwarr_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tatsumi_v30_68000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tatsumi_v30_68000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tatsumi_sprite_control_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void apache3_road_z_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void apache3_road_x_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint16_t roundup5_vram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void roundup5_vram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roundup5_text_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t cyclwarr_videoram0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t cyclwarr_videoram1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void cyclwarr_videoram0_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void cyclwarr_videoram1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void roundup5_crt_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_roundup5();
	void init_apache3();
	void init_cyclwarr();
	void get_text_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_bigfight_0(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_bigfight_1(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_apache3();
	void video_start_apache3();
	void video_start_roundup5();
	void video_start_cyclwarr();
	void video_start_bigfight();
	uint32_t screen_update_apache3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_roundup5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_cyclwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_bigfight(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void roundup5_interrupt(device_t &device);
	uint8_t tatsumi_hack_ym2151_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	uint8_t tatsumi_hack_oki_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void apache3_68000_reset(int state);
	void tatsumi_reset();
};
