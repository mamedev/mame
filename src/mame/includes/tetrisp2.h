// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"

class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_spriteram(*this, "spriteram"),
		m_spriteram2(*this, "spriteram2"),
		m_vram_fg(*this, "vram_fg"),
		m_vram_bg(*this, "vram_bg"),
		m_vram_rot(*this, "vram_rot"),
		m_nvram(*this, "nvram"),
		m_scroll_fg(*this, "scroll_fg"),
		m_scroll_bg(*this, "scroll_bg"),
		m_rotregs(*this, "rotregs"),
		m_rocknms_sub_priority(*this, "sub_priority"),
		m_rocknms_sub_vram_rot(*this, "sub_vram_rot"),
		m_rocknms_sub_vram_fg(*this, "sub_vram_fg"),
		m_rocknms_sub_vram_bg(*this, "sub_vram_bg"),
		m_rocknms_sub_scroll_fg(*this, "sub_scroll_fg"),
		m_rocknms_sub_scroll_bg(*this, "sub_scroll_bg"),
		m_rocknms_sub_rotregs(*this, "sub_rotregs"),
		m_gfxdecode(*this, "gfxdecode"),
		m_sub_gfxdecode(*this, "sub_gfxdecode"),
		m_palette(*this, "palette"),
		m_sub_palette(*this, "sub_palette"),
		m_paletteram(*this, "paletteram"),
		m_sub_paletteram(*this, "sub_paletteram")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;

	required_shared_ptr<uint16_t> m_spriteram;
	optional_shared_ptr<uint16_t> m_spriteram2;

	uint16_t m_systemregs[0x10];
	required_shared_ptr<uint16_t> m_vram_fg;
	required_shared_ptr<uint16_t> m_vram_bg;
	required_shared_ptr<uint16_t> m_vram_rot;
	required_shared_ptr<uint16_t> m_nvram;
	required_shared_ptr<uint16_t> m_scroll_fg;
	required_shared_ptr<uint16_t> m_scroll_bg;
	required_shared_ptr<uint16_t> m_rotregs;
	std::unique_ptr<uint8_t[]> m_priority;
	optional_shared_ptr<uint16_t> m_rocknms_sub_priority;
	optional_shared_ptr<uint16_t> m_rocknms_sub_vram_rot;
	optional_shared_ptr<uint16_t> m_rocknms_sub_vram_fg;
	optional_shared_ptr<uint16_t> m_rocknms_sub_vram_bg;
	optional_shared_ptr<uint16_t> m_rocknms_sub_scroll_fg;
	optional_shared_ptr<uint16_t> m_rocknms_sub_scroll_bg;
	optional_shared_ptr<uint16_t> m_rocknms_sub_rotregs;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<gfxdecode_device> m_sub_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_sub_palette;
	required_shared_ptr<uint16_t> m_paletteram;
	optional_shared_ptr<uint16_t> m_sub_paletteram;

	uint16_t m_rocknms_sub_systemregs[0x10];
	uint16_t m_rockn_protectdata;
	uint16_t m_rockn_adpcmbank;
	uint16_t m_rockn_soundvolume;
	emu_timer *m_rockn_timer_l4;
	emu_timer *m_rockn_timer_sub_l4;
	int m_bank_lo;
	int m_bank_hi;
	uint16_t m_rocknms_main2sub;
	uint16_t m_rocknms_sub2main;
	int m_flipscreen_old;
	tilemap_t *m_tilemap_bg;
	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_rot;
	tilemap_t *m_tilemap_sub_bg;
	tilemap_t *m_tilemap_sub_fg;
	tilemap_t *m_tilemap_sub_rot;
	void rockn_systemregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_systemregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rockn_adpcmbank_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rockn_adpcmbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rockn2_adpcmbank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t rockn_soundvolume_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rockn_soundvolume_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nndmseal_sound_bank_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tetrisp2_ip_1_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rockn_nvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t rocknms_main2sub_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void rocknms_main2sub_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub2main_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_coincounter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nndmseal_coincounter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void nndmseal_b20000_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_systemregs_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tetrisp2_nvram_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tetrisp2_nvram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_priority_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t tetrisp2_priority_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void tetrisp2_vram_bg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_vram_fg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void tetrisp2_vram_rot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_vram_bg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_vram_fg_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void rocknms_sub_vram_rot_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	ioport_value rocknms_main2sub_status_r(ioport_field &field, void *param);
	void init_rockn2();
	void init_rockn1();
	void init_rockn();
	void init_rockn3();
	void init_rocknms();
	void get_tile_info_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_rot(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_rocknms_sub_bg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_rocknms_sub_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_tile_info_rocknms_sub_rot(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void stepstag_get_tile_info_fg(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void video_start_tetrisp2();
	void video_start_nndmseal();
	void video_start_rockntread();
	void video_start_rocknms();
	uint32_t screen_update_tetrisp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rockntread(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rocknms_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_rocknms_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void rockn_timer_level4_callback(void *ptr, int32_t param);
	void rockn_timer_sub_level4_callback(void *ptr, int32_t param);
	void rockn_timer_level1_callback(void *ptr, int32_t param);
	void rockn_timer_sub_level1_callback(void *ptr, int32_t param);
	void init_rockn_timer();
};

class stepstag_state : public tetrisp2_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, const char *tag)
		: tetrisp2_state(mconfig, type, tag),
			m_spriteram3(*this, "spriteram3"),
			m_soundlatch(*this, "soundlatch") { }

	required_shared_ptr<uint16_t> m_spriteram3;
	required_device<generic_latch_16_device> m_soundlatch;
	uint16_t stepstag_coins_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unknown_read_0xc00000(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unknown_read_0xffff00(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t unk_a42000_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void stepstag_soundlatch_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stepstag_leds_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void stepstag_palette_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void init_stepstag();
	void video_start_stepstag();
	uint32_t screen_update_stepstag_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stepstag_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_stepstag_mid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int mypal(int x);
};
