// license:BSD-3-Clause
// copyright-holders:Luca Elia

#include "machine/gen_latch.h"
#include "sound/k007232.h"
#include "video/k051316.h"

class wecleman_state : public driver_device
{
public:
	wecleman_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videostatus(*this, "videostatus"),
		m_protection_ram(*this, "protection_ram"),
		m_blitter_regs(*this, "blitter_regs"),
		m_pageram(*this, "pageram"),
		m_txtram(*this, "txtram"),
		m_spriteram(*this, "spriteram"),
		m_roadram(*this, "roadram"),
		m_generic_paletteram_16(*this, "paletteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_k051316_1(*this, "k051316_1"),
		m_k051316_2(*this, "k051316_2"),
		m_k007232(*this, "k007232"),
		m_k007232_1(*this, "k007232_1"),
		m_k007232_2(*this, "k007232_2"),
		m_k007232_3(*this, "k007232_3"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch") { }

	optional_shared_ptr<uint16_t> m_videostatus;
	optional_shared_ptr<uint16_t> m_protection_ram;
	required_shared_ptr<uint16_t> m_blitter_regs;
	optional_shared_ptr<uint16_t> m_pageram;
	optional_shared_ptr<uint16_t> m_txtram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_shared_ptr<uint16_t> m_roadram;
	required_shared_ptr<uint16_t> m_generic_paletteram_16;

	int m_multiply_reg[2];
	int m_spr_color_offs;
	int m_prot_state;
	int m_selected_ip;
	int m_irqctrl;
	int m_bgpage[4];
	int m_fgpage[4];
	const int *m_gfx_bank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_txt_tilemap;
	int *m_spr_idx_list;
	int *m_spr_pri_list;
	int *m_t32x32pm;
	int m_gameid;
	int m_spr_offsx;
	int m_spr_offsy;
	int m_spr_count;
	uint16_t *m_rgb_half;
	int m_cloud_blend;
	int m_cloud_ds;
	int m_cloud_visible;
	int m_sound_hw_type;
	bool m_hotchase_sound_hs;
	pen_t m_black_pen;
	struct sprite *m_sprite_list;
	struct sprite **m_spr_ptr_list;
	uint16_t wecleman_protection_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void wecleman_protection_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void irqctrl_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void selected_ip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t selected_ip_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void blitter_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint8_t multiply_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void multiply_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void hotchase_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hotchase_sound_control_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wecleman_soundlatch_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wecleman_txtram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wecleman_pageram_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wecleman_videostatus_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void hotchase_paletteram16_SBGRBBBBGGGGRRRR_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wecleman_paletteram16_SSSSBBBBGGGGRRRR_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void wecleman_K00723216_bank_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void wecleman_volume_callback(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hotchase_1_k007232_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hotchase_1_k007232_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hotchase_2_k007232_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hotchase_2_k007232_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t hotchase_3_k007232_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void hotchase_3_k007232_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void init_wecleman();
	void init_hotchase();
	void wecleman_get_txt_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wecleman_get_bg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void wecleman_get_fg_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_reset_wecleman();
	void video_start_wecleman();
	void machine_reset_hotchase();
	void video_start_hotchase();
	uint32_t screen_update_wecleman(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_hotchase(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void hotchase_sound_timer(device_t &device);
	void wecleman_scanline(timer_device &timer, void *ptr, int32_t param);
	void hotchase_scanline(timer_device &timer, void *ptr, int32_t param);
	void draw_cloud(bitmap_rgb32 &bitmap,gfx_element *gfx,uint16_t *tm_base,int x0,int y0,int xcount,int ycount,int scrollx,int scrolly,int tmw_l2,int tmh_l2,int alpha,int pal_offset);
	void wecleman_unpack_sprites();
	void bitswap(uint8_t *src,size_t len,int _14,int _13,int _12,int _11,int _10,int _f,int _e,int _d,int _c,int _b,int _a,int _9,int _8,int _7,int _6,int _5,int _4,int _3,int _2,int _1,int _0);
	void hotchase_sprite_decode( int num16_banks, int bank_size );
	void get_sprite_info();
	void sortsprite(int *idx_array, int *key_array, int size);
	template<class _BitmapClass> void do_blit_zoom32(_BitmapClass &bitmap, const rectangle &cliprect, struct sprite *sprite);
	template<class _BitmapClass> void sprite_draw(_BitmapClass &bitmap, const rectangle &cliprect);
	void wecleman_draw_road(bitmap_rgb32 &bitmap, const rectangle &cliprect, int priority);
	void hotchase_draw_road(bitmap_ind16 &bitmap, const rectangle &cliprect);
	K051316_CB_MEMBER(hotchase_zoom_callback_1);
	K051316_CB_MEMBER(hotchase_zoom_callback_2);
	ioport_value hotchase_sound_status_r(ioport_field &field, void *param);
	void hotchase_sound_hs_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<k051316_device> m_k051316_1;
	optional_device<k051316_device> m_k051316_2;
	optional_device<k007232_device> m_k007232;
	optional_device<k007232_device> m_k007232_1;
	optional_device<k007232_device> m_k007232_2;
	optional_device<k007232_device> m_k007232_3;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
};
