// license:BSD-3-Clause
// copyright-holders:R. Belmont, Phil Stroffolino, Acho A. Tang, Nicola Salmoria

#include "machine/gen_latch.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/k055555.h"
#include "video/k054000.h"
#include "video/k053246_k053247_k055673.h"

class mystwarr_state : public konamigx_state
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: konamigx_state(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k055673(*this, "k055673"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3"),
		m_gx_workram(*this,"gx_workram"),
		m_spriteram(*this,"spriteram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k055673_device> m_k055673;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;
	required_shared_ptr<uint16_t> m_gx_workram;
	optional_shared_ptr<uint16_t> m_spriteram;
	std::unique_ptr<uint8_t[]> m_decoded;

	uint8_t m_mw_irq_control;
	int m_cur_sound_region;
	int m_layer_colorbase[6];
	int m_oinprion;
	int m_cbparam;
	int m_sprite_colorbase;
	int m_sub1_colorbase;
	int m_last_psac_colorbase;
	int m_gametype;
	int m_roz_enable;
	int m_roz_rombank;
	tilemap_t *m_ult_936_tilemap;
	uint16_t m_clip;

	uint8_t m_sound_ctrl;
	uint8_t m_sound_nmi_clk;

	uint16_t eeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mweeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t dddeeprom_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mmeeprom_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd1_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd1_msb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd2_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_cmd2_msb_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_irq_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t sound_status_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t sound_status_msb_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void irq_ack_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k053247_scattered_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k053247_scattered_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t k053247_martchmp_word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void k053247_martchmp_word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t mccontrol_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void mccontrol_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void sound_ctrl_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	void ddd_053936_enable_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	void ddd_053936_clip_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);
	uint16_t gai_053936_tilerom_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ddd_053936_tilerom_0_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ddd_053936_tilerom_1_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t gai_053936_tilerom_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	uint16_t ddd_053936_tilerom_2_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void get_gai_936_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void get_ult_936_tile_info(tilemap_t &tilemap, tile_data &tileinfo, tilemap_memory_index tile_index);
	void machine_start_mystwarr();
	void machine_reset_mystwarr();
	void video_start_mystwarr();
	void machine_reset_viostorm();
	void video_start_viostorm();
	void machine_reset_metamrph();
	void video_start_metamrph();
	void machine_reset_dadandrn();
	void video_start_dadandrn();
	void machine_reset_gaiapols();
	void video_start_gaiapols();
	void machine_reset_martchmp();
	void video_start_martchmp();
	uint32_t screen_update_mystwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_metamrph(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dadandrn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_martchmp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void ddd_interrupt(device_t &device);
	void k054539_nmi_gen(int state);
	void mystwarr_interrupt(timer_device &timer, void *ptr, int32_t param);
	void metamrph_interrupt(timer_device &timer, void *ptr, int32_t param);
	void mchamp_interrupt(timer_device &timer, void *ptr, int32_t param);
	K056832_CB_MEMBER(mystwarr_tile_callback);
	K056832_CB_MEMBER(game5bpp_tile_callback);
	K056832_CB_MEMBER(game4bpp_tile_callback);
	K055673_CB_MEMBER(mystwarr_sprite_callback);
	K055673_CB_MEMBER(metamrph_sprite_callback);
	K055673_CB_MEMBER(gaiapols_sprite_callback);
	K055673_CB_MEMBER(martchmp_sprite_callback);
	void decode_tiles();
};
