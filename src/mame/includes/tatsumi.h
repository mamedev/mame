// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#include "sound/okim6295.h"
#include "cpu/m68000/m68000.h"

class tatsumi_state : public driver_device
{
public:
	tatsumi_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_subcpu2(*this, "sub2"),
		m_oki(*this, "oki"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
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

	optional_shared_ptr<UINT16> m_videoram;
	optional_shared_ptr<UINT16> m_cyclwarr_cpua_ram;
	optional_shared_ptr<UINT16> m_cyclwarr_cpub_ram;
	optional_shared_ptr<UINT16> m_apache3_g_ram;
	optional_shared_ptr<UINT16> m_roundup5_d0000_ram;
	optional_shared_ptr<UINT16> m_roundup5_e0000_ram;
	optional_shared_ptr<UINT16> m_roundup5_unknown0;
	optional_shared_ptr<UINT16> m_roundup5_unknown1;
	optional_shared_ptr<UINT16> m_roundup5_unknown2;
	optional_shared_ptr<UINT16> m_68k_ram;
	optional_shared_ptr<UINT8> m_apache3_z80_ram;
	required_shared_ptr<UINT16> m_sprite_control_ram;
	optional_shared_ptr<UINT16> m_cyclwarr_videoram0;
	optional_shared_ptr<UINT16> m_cyclwarr_videoram1;
	optional_shared_ptr<UINT16> m_roundup_r_ram;
	optional_shared_ptr<UINT16> m_roundup_p_ram;
	optional_shared_ptr<UINT16> m_roundup_l_ram;
	required_shared_ptr<UINT16> m_spriteram;

	UINT16 m_bigfight_a20000[8];
	UINT16 m_bigfight_a60000[2];
	UINT16 m_bigfight_a40000[2];
	UINT8 *m_rom_sprite_lookup1;
	UINT8 *m_rom_sprite_lookup2;
	UINT8 *m_rom_clut0;
	UINT8 *m_rom_clut1;
	UINT16 m_control_word;
	UINT16 m_apache3_rotate_ctrl[12];
	UINT16 m_last_control;
	UINT8 m_apache3_adc;
	int m_apache3_rot_idx;
	tilemap_t *m_tx_layer;
	tilemap_t *m_layer0;
	tilemap_t *m_layer1;
	tilemap_t *m_layer2;
	tilemap_t *m_layer3;
	bitmap_rgb32 m_temp_bitmap;
	std::unique_ptr<UINT8[]> m_apache3_road_x_ram;
	UINT8 m_apache3_road_z;
	std::unique_ptr<UINT16[]> m_roundup5_vram;
	UINT16 m_bigfight_bank;
	UINT16 m_bigfight_last_bank;
	UINT8 m_roundupt_crt_selected_reg;
	UINT8 m_roundupt_crt_reg[64];
	std::unique_ptr<UINT8[]> m_shadow_pen_array;
	DECLARE_READ16_MEMBER(cyclwarr_sprite_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_sprite_w);
	DECLARE_WRITE16_MEMBER(bigfight_a20000_w);
	DECLARE_WRITE16_MEMBER(bigfight_a40000_w);
	DECLARE_WRITE16_MEMBER(bigfight_a60000_w);
	DECLARE_WRITE16_MEMBER(cyclwarr_sound_w);
	DECLARE_READ16_MEMBER(apache3_bank_r);
	DECLARE_WRITE16_MEMBER(apache3_bank_w);
	DECLARE_WRITE16_MEMBER(apache3_z80_ctrl_w);
	DECLARE_READ16_MEMBER(apache3_v30_v20_r);
	DECLARE_WRITE16_MEMBER(apache3_v30_v20_w);
	DECLARE_READ16_MEMBER(apache3_z80_r);
	DECLARE_WRITE16_MEMBER(apache3_z80_w);
	DECLARE_READ8_MEMBER(apache3_adc_r);
	DECLARE_WRITE8_MEMBER(apache3_adc_w);
	DECLARE_WRITE16_MEMBER(apache3_rotate_w);
	DECLARE_READ16_MEMBER(roundup_v30_z80_r);
	DECLARE_WRITE16_MEMBER(roundup_v30_z80_w);
	DECLARE_WRITE16_MEMBER(roundup5_control_w);
	DECLARE_WRITE16_MEMBER(roundup5_d0000_w);
	DECLARE_WRITE16_MEMBER(roundup5_e0000_w);
	DECLARE_READ16_MEMBER(cyclwarr_control_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_control_w);
	DECLARE_READ16_MEMBER(tatsumi_v30_68000_r);
	DECLARE_WRITE16_MEMBER(tatsumi_v30_68000_w);
	DECLARE_WRITE16_MEMBER(tatsumi_sprite_control_w);
	DECLARE_WRITE16_MEMBER(apache3_road_z_w);
	DECLARE_WRITE8_MEMBER(apache3_road_x_w);
	DECLARE_READ16_MEMBER(roundup5_vram_r);
	DECLARE_WRITE16_MEMBER(roundup5_vram_w);
	DECLARE_WRITE16_MEMBER(roundup5_text_w);
	DECLARE_READ16_MEMBER(cyclwarr_videoram0_r);
	DECLARE_READ16_MEMBER(cyclwarr_videoram1_r);
	DECLARE_WRITE16_MEMBER(cyclwarr_videoram0_w);
	DECLARE_WRITE16_MEMBER(cyclwarr_videoram1_w);
	DECLARE_WRITE16_MEMBER(roundup5_crt_w);
	DECLARE_DRIVER_INIT(roundup5);
	DECLARE_DRIVER_INIT(apache3);
	DECLARE_DRIVER_INIT(cyclwarr);
	TILE_GET_INFO_MEMBER(get_text_tile_info);
	TILE_GET_INFO_MEMBER(get_tile_info_bigfight_0);
	TILE_GET_INFO_MEMBER(get_tile_info_bigfight_1);
	DECLARE_MACHINE_RESET(apache3);
	DECLARE_VIDEO_START(apache3);
	DECLARE_VIDEO_START(roundup5);
	DECLARE_VIDEO_START(cyclwarr);
	DECLARE_VIDEO_START(bigfight);
	UINT32 screen_update_apache3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_roundup5(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_cyclwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_bigfight(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(roundup5_interrupt);
	DECLARE_READ8_MEMBER(tatsumi_hack_ym2151_r);
	DECLARE_READ8_MEMBER(tatsumi_hack_oki_r);
	DECLARE_WRITE_LINE_MEMBER(apache3_68000_reset);
	void tatsumi_reset();
};
