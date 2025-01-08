// license:BSD-3-Clause
// copyright-holders:R. Belmont, Phil Stroffolino, Acho A. Tang, Nicola Salmoria

#include "konamigx.h"
#include "k053246_k053247_k055673.h"

#include "sound/k054539.h"
#include "machine/k053252.h"
#include "k055555.h"
#include "k054000.h"
#include "machine/k054321.h"
#include "machine/timer.h"
#include "tilemap.h"

class mystwarr_state : public konamigx_state
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag) :
		konamigx_state(mconfig, type, tag),
		m_k054321(*this, "k054321"),
		m_gx_workram(*this, "gx_workram"),
		m_spriteram(*this, "spriteram"),
		m_okibank(*this, "okibank")
	{ }

	void martchmp(machine_config &config);
	void mystwarr(machine_config &config);
	void dadandrn(machine_config &config);
	void viostorm(machine_config &config);
	void viostormbl(machine_config &config);
	void gaiapols(machine_config &config);
	void metamrph(machine_config &config);

private:
	optional_device<k054321_device> m_k054321;
	required_shared_ptr<uint16_t> m_gx_workram;
	optional_shared_ptr<uint16_t> m_spriteram;
	optional_memory_bank m_okibank; // for viostormabbl
	std::unique_ptr<uint8_t[]> m_decoded;

	uint8_t m_mw_irq_control = 0;
	int m_cur_sound_region = 0;
	int m_layer_colorbase[6]{};
	int m_oinprion = 0;
	int m_cbparam = 0;
	int m_sprite_colorbase = 0;
	int m_sub1_colorbase = 0;
	int m_last_psac_colorbase = 0;
	int m_gametype = 0;
	int m_roz_enable = 0;
	int m_roz_rombank = 0;
	tilemap_t *m_ult_936_tilemap = nullptr;
	uint16_t m_clip = 0;

	uint8_t m_sound_ctrl = 0;
	uint8_t m_sound_nmi_clk = 0;

	uint16_t eeprom_r(offs_t offset, uint16_t mem_mask = ~0);
	void mweeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t dddeeprom_r(offs_t offset, uint16_t mem_mask = ~0);
	void mmeeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_irq_w(uint16_t data);
	void irq_ack_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t k053247_scattered_word_r(offs_t offset);
	void k053247_scattered_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t k053247_martchmp_word_r(offs_t offset);
	void k053247_martchmp_word_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void mceeprom_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mccontrol_r();
	void mccontrol_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void sound_ctrl_w(uint8_t data);

	void ddd_053936_enable_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	void ddd_053936_clip_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t gai_053936_tilerom_0_r(offs_t offset);
	uint16_t ddd_053936_tilerom_0_r(offs_t offset);
	uint16_t ddd_053936_tilerom_1_r(offs_t offset);
	uint16_t gai_053936_tilerom_2_r(offs_t offset);
	uint16_t ddd_053936_tilerom_2_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_gai_936_tile_info);
	TILE_GET_INFO_MEMBER(get_ult_936_tile_info);
	DECLARE_MACHINE_START(mystwarr);
	DECLARE_MACHINE_RESET(mystwarr);
	DECLARE_VIDEO_START(mystwarr);
	DECLARE_MACHINE_RESET(viostorm);
	DECLARE_VIDEO_START(viostorm);
	DECLARE_MACHINE_START(viostormbl);
	DECLARE_MACHINE_RESET(metamrph);
	DECLARE_VIDEO_START(metamrph);
	DECLARE_MACHINE_RESET(dadandrn);
	DECLARE_VIDEO_START(dadandrn);
	DECLARE_MACHINE_RESET(gaiapols);
	DECLARE_VIDEO_START(gaiapols);
	DECLARE_MACHINE_RESET(martchmp);
	DECLARE_VIDEO_START(martchmp);
	uint32_t screen_update_mystwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_metamrph(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_dadandrn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_martchmp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ddd_interrupt);
	void k054539_nmi_gen(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(mystwarr_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(metamrph_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mchamp_interrupt);
	K056832_CB_MEMBER(mystwarr_tile_callback);
	K056832_CB_MEMBER(game5bpp_tile_callback);
	K056832_CB_MEMBER(game4bpp_tile_callback);
	K055673_CB_MEMBER(mystwarr_sprite_callback);
	K055673_CB_MEMBER(metamrph_sprite_callback);
	K055673_CB_MEMBER(gaiapols_sprite_callback);
	K055673_CB_MEMBER(martchmp_sprite_callback);
	void decode_tiles();
	void dadandrn_map(address_map &map) ATTR_COLD;
	void gaiapols_map(address_map &map) ATTR_COLD;
	void martchmp_map(address_map &map) ATTR_COLD;
	void martchmp_sound_map(address_map &map) ATTR_COLD;
	void metamrph_map(address_map &map) ATTR_COLD;
	void mystwarr_map(address_map &map) ATTR_COLD;
	void mystwarr_sound_map(address_map &map) ATTR_COLD;
	void oki_map(address_map &map) ATTR_COLD; // for viostormabbl
	void viostorm_map(address_map &map) ATTR_COLD;
	void viostormbl_map(address_map &map) ATTR_COLD;
};
