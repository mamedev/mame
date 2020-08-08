// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "video/ms32_sprite.h"
#include "emupal.h"
#include "tilemap.h"

class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "sub"),
		m_sprite(*this, "sprite"),
		m_rocknms_sub_sprite(*this, "sub_sprite"),
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
		m_sub_paletteram(*this, "sub_paletteram"),
		m_leds(*this, "led%u", 0U)
	{ }

	void rockn2(machine_config &config);
	void tetrisp2(machine_config &config);
	void nndmseal(machine_config &config);
	void rocknms(machine_config &config);
	void rockn(machine_config &config);

	void init_rockn2();
	void init_rockn1();
	void init_rockn();
	void init_rockn3();
	void init_rocknms();

	DECLARE_CUSTOM_INPUT_MEMBER(rocknms_main2sub_status_r);

	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(stepstag_get_tile_info_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_rot);

protected:
	void rockn_systemregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_systemregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 rockn_adpcmbank_r();
	void rockn_adpcmbank_w(u16 data);
	void rockn2_adpcmbank_w(u16 data);
	u16 rockn_soundvolume_r();
	void rockn_soundvolume_w(u16 data);
	void nndmseal_sound_bank_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tetrisp2_ip_1_word_r();
	u16 rockn_nvram_r(offs_t offset);
	u16 rocknms_main2sub_r();
	void rocknms_main2sub_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub2main_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tetrisp2_coincounter_w(u16 data);
	void nndmseal_coincounter_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void nndmseal_b20000_w(u16 data);
	void tetrisp2_systemregs_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tetrisp2_nvram_r(offs_t offset);
	void tetrisp2_nvram_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tetrisp2_palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_palette_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tetrisp2_priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_priority_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	u16 tetrisp2_priority_r(offs_t offset);
	void tetrisp2_vram_bg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tetrisp2_vram_fg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void tetrisp2_vram_rot_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_vram_bg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_vram_fg_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void rocknms_sub_vram_rot_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	TILE_GET_INFO_MEMBER(get_tile_info_fg);

	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_rot);

	DECLARE_VIDEO_START(tetrisp2);
	DECLARE_VIDEO_START(nndmseal);
	DECLARE_VIDEO_START(rockntread);
	DECLARE_VIDEO_START(rocknms);
	u32 screen_update_tetrisp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_rockntread(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_rocknms_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_rocknms_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(rockn_timer_level4_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_sub_level4_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_level1_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_sub_level1_callback);
	void init_rockn_timer();

	void nndmseal_map(address_map &map);
	void rockn1_map(address_map &map);
	void rockn2_map(address_map &map);
	void rocknms_main_map(address_map &map);
	void rocknms_sub_map(address_map &map);
	void tetrisp2_map(address_map &map);

	virtual void machine_start() override { m_leds.resolve(); }

	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_subcpu;

	required_device<ms32_sprite_device> m_sprite;
	optional_device<ms32_sprite_device> m_rocknms_sub_sprite;

	required_shared_ptr<u16> m_spriteram;
	optional_shared_ptr<u16> m_spriteram2;

	u16 m_systemregs[0x10];
	required_shared_ptr<u16> m_vram_fg;
	required_shared_ptr<u16> m_vram_bg;
	required_shared_ptr<u16> m_vram_rot;
	required_shared_ptr<u16> m_nvram;
	required_shared_ptr<u16> m_scroll_fg;
	required_shared_ptr<u16> m_scroll_bg;
	required_shared_ptr<u16> m_rotregs;
	std::unique_ptr<u8[]> m_priority;
	optional_shared_ptr<u16> m_rocknms_sub_priority;
	optional_shared_ptr<u16> m_rocknms_sub_vram_rot;
	optional_shared_ptr<u16> m_rocknms_sub_vram_fg;
	optional_shared_ptr<u16> m_rocknms_sub_vram_bg;
	optional_shared_ptr<u16> m_rocknms_sub_scroll_fg;
	optional_shared_ptr<u16> m_rocknms_sub_scroll_bg;
	optional_shared_ptr<u16> m_rocknms_sub_rotregs;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<gfxdecode_device> m_sub_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_sub_palette;
	required_shared_ptr<u16> m_paletteram;
	optional_shared_ptr<u16> m_sub_paletteram;
	output_finder<45> m_leds;

	u16 m_rocknms_sub_systemregs[0x10];
	u16 m_rockn_protectdata;
	u16 m_rockn_adpcmbank;
	u16 m_rockn_soundvolume;
	emu_timer *m_rockn_timer_l4;
	emu_timer *m_rockn_timer_sub_l4;
	emu_timer *m_rockn_timer_l1;
	emu_timer *m_rockn_timer_sub_l1;
	int m_bank_lo;
	int m_bank_hi;
	u16 m_rocknms_main2sub;
	u16 m_rocknms_sub2main;
	int m_flipscreen_old;
	tilemap_t *m_tilemap_bg;
	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_rot;
	tilemap_t *m_tilemap_sub_bg;
	tilemap_t *m_tilemap_sub_fg;
	tilemap_t *m_tilemap_sub_rot;
};

class stepstag_state : public tetrisp2_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, const char *tag) :
		tetrisp2_state(mconfig, type, tag),
		m_vj_sprite_l(*this, "sprite_l"),
		m_vj_sprite_m(*this, "sprite_m"),
		m_vj_sprite_r(*this, "sprite_r"),
		m_spriteram1(*this, "spriteram1"),
		m_spriteram3(*this, "spriteram3"),
		m_vj_palette_l(*this, "lpalette"),
		m_vj_palette_m(*this, "mpalette"),
		m_vj_palette_r(*this, "rpalette"),
		m_vj_paletteram_l(*this, "paletteram1"),
		m_vj_paletteram_m(*this, "paletteram2"),
		m_vj_paletteram_r(*this, "paletteram3"),
		m_soundlatch(*this, "soundlatch")
	{ }

	void stepstag(machine_config &config);
	void vjdash(machine_config &config);

	void init_stepstag();

	DECLARE_VIDEO_START(stepstag);

private:
	u16 stepstag_coins_r();
	u16 vj_upload_idx;
	bool vj_upload_fini;
	void stepstag_b00000_w(u16 data);
	void stepstag_b20000_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_main2pc_w(u16 data);
	u16 unknown_read_0xc00000();
	u16 unknown_read_0xffff00();
	u16 stepstag_pc2main_r();
	void stepstag_soundlatch_word_w(u16 data);
	void stepstag_neon_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_step_leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_button_leds_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_palette_left_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_palette_mid_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void stepstag_palette_right_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	u32 screen_update_stepstag_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_stepstag_mid(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	u32 screen_update_stepstag_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_stepstag_main(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
//  inline int mypal(int x);

	void stepstag_map(address_map &map);
	void stepstag_sub_map(address_map &map);
	void vjdash_map(address_map &map);

	optional_device<ms32_sprite_device> m_vj_sprite_l;
	optional_device<ms32_sprite_device> m_vj_sprite_m;
	optional_device<ms32_sprite_device> m_vj_sprite_r;
	required_shared_ptr<u16> m_spriteram1;
	required_shared_ptr<u16> m_spriteram3;
	optional_device<palette_device> m_vj_palette_l;
	optional_device<palette_device> m_vj_palette_m;
	optional_device<palette_device> m_vj_palette_r;
	optional_shared_ptr<u16> m_vj_paletteram_l;
	optional_shared_ptr<u16> m_vj_paletteram_m;
	optional_shared_ptr<u16> m_vj_paletteram_r;
	required_device<generic_latch_16_device> m_soundlatch;
	void convert_yuv422_to_rgb888(palette_device *paldev, u16 *palram,u32 offset);
};
