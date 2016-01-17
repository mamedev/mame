// license:BSD-3-Clause
// copyright-holders:Luca Elia
class tetrisp2_state : public driver_device
{
public:
	tetrisp2_state(const machine_config &mconfig, device_type type, std::string tag)
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

	required_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_spriteram2;

	UINT16 m_systemregs[0x10];
	required_shared_ptr<UINT16> m_vram_fg;
	required_shared_ptr<UINT16> m_vram_bg;
	required_shared_ptr<UINT16> m_vram_rot;
	required_shared_ptr<UINT16> m_nvram;
	required_shared_ptr<UINT16> m_scroll_fg;
	required_shared_ptr<UINT16> m_scroll_bg;
	required_shared_ptr<UINT16> m_rotregs;
	std::unique_ptr<UINT8[]> m_priority;
	optional_shared_ptr<UINT16> m_rocknms_sub_priority;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_rot;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_fg;
	optional_shared_ptr<UINT16> m_rocknms_sub_vram_bg;
	optional_shared_ptr<UINT16> m_rocknms_sub_scroll_fg;
	optional_shared_ptr<UINT16> m_rocknms_sub_scroll_bg;
	optional_shared_ptr<UINT16> m_rocknms_sub_rotregs;
	required_device<gfxdecode_device> m_gfxdecode;
	optional_device<gfxdecode_device> m_sub_gfxdecode;
	required_device<palette_device> m_palette;
	optional_device<palette_device> m_sub_palette;
	required_shared_ptr<UINT16> m_paletteram;
	optional_shared_ptr<UINT16> m_sub_paletteram;

	UINT16 m_rocknms_sub_systemregs[0x10];
	UINT16 m_rockn_protectdata;
	UINT16 m_rockn_adpcmbank;
	UINT16 m_rockn_soundvolume;
	emu_timer *m_rockn_timer_l4;
	emu_timer *m_rockn_timer_sub_l4;
	int m_bank_lo;
	int m_bank_hi;
	UINT16 m_rocknms_main2sub;
	UINT16 m_rocknms_sub2main;
	int m_flipscreen_old;
	tilemap_t *m_tilemap_bg;
	tilemap_t *m_tilemap_fg;
	tilemap_t *m_tilemap_rot;
	tilemap_t *m_tilemap_sub_bg;
	tilemap_t *m_tilemap_sub_fg;
	tilemap_t *m_tilemap_sub_rot;
	DECLARE_WRITE16_MEMBER(rockn_systemregs_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_systemregs_w);
	DECLARE_READ16_MEMBER(rockn_adpcmbank_r);
	DECLARE_WRITE16_MEMBER(rockn_adpcmbank_w);
	DECLARE_WRITE16_MEMBER(rockn2_adpcmbank_w);
	DECLARE_READ16_MEMBER(rockn_soundvolume_r);
	DECLARE_WRITE16_MEMBER(rockn_soundvolume_w);
	DECLARE_WRITE16_MEMBER(nndmseal_sound_bank_w);
	DECLARE_READ16_MEMBER(tetrisp2_ip_1_word_r);
	DECLARE_READ16_MEMBER(rockn_nvram_r);
	DECLARE_READ16_MEMBER(rocknms_main2sub_r);
	DECLARE_WRITE16_MEMBER(rocknms_main2sub_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub2main_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_coincounter_w);
	DECLARE_WRITE16_MEMBER(nndmseal_coincounter_w);
	DECLARE_WRITE16_MEMBER(nndmseal_b20000_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_systemregs_w);
	DECLARE_READ16_MEMBER(tetrisp2_nvram_r);
	DECLARE_WRITE16_MEMBER(tetrisp2_nvram_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_palette_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_palette_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_priority_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_priority_w);
	DECLARE_READ16_MEMBER(tetrisp2_priority_r);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_bg_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_fg_w);
	DECLARE_WRITE16_MEMBER(tetrisp2_vram_rot_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_bg_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_fg_w);
	DECLARE_WRITE16_MEMBER(rocknms_sub_vram_rot_w);
	DECLARE_CUSTOM_INPUT_MEMBER(rocknms_main2sub_status_r);
	DECLARE_DRIVER_INIT(rockn2);
	DECLARE_DRIVER_INIT(rockn1);
	DECLARE_DRIVER_INIT(rockn);
	DECLARE_DRIVER_INIT(rockn3);
	DECLARE_DRIVER_INIT(rocknms);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_rot);
	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_bg);
	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_fg);
	TILE_GET_INFO_MEMBER(get_tile_info_rocknms_sub_rot);
	TILE_GET_INFO_MEMBER(stepstag_get_tile_info_fg);
	DECLARE_VIDEO_START(tetrisp2);
	DECLARE_VIDEO_START(nndmseal);
	DECLARE_VIDEO_START(rockntread);
	DECLARE_VIDEO_START(rocknms);
	UINT32 screen_update_tetrisp2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rockntread(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rocknms_left(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_rocknms_right(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	TIMER_CALLBACK_MEMBER(rockn_timer_level4_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_sub_level4_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_level1_callback);
	TIMER_CALLBACK_MEMBER(rockn_timer_sub_level1_callback);
	void init_rockn_timer();
};

class stepstag_state : public tetrisp2_state
{
public:
	stepstag_state(const machine_config &mconfig, device_type type, std::string tag)
		: tetrisp2_state(mconfig, type, tag),
			m_spriteram3(*this, "spriteram3") { }

	required_shared_ptr<UINT16> m_spriteram3;
	DECLARE_READ16_MEMBER(stepstag_coins_r);
	DECLARE_READ16_MEMBER(unknown_read_0xc00000);
	DECLARE_READ16_MEMBER(unknown_read_0xffff00);
	DECLARE_READ16_MEMBER(unk_a42000_r);
	DECLARE_WRITE16_MEMBER(stepstag_soundlatch_word_w);
	DECLARE_WRITE16_MEMBER(stepstag_leds_w);
	DECLARE_WRITE16_MEMBER( stepstag_palette_w );
	DECLARE_DRIVER_INIT(stepstag);
	DECLARE_VIDEO_START(stepstag);
	UINT32 screen_update_stepstag_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stepstag_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_stepstag_mid(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	inline int mypal(int x);
};
