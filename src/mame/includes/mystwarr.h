// license:BSD-3-Clause
// copyright-holders:R. Belmont, Phil Stroffolino, Acho A. Tang, Nicola Salmoria
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
		m_gx_workram(*this,"gx_workram"),
		m_spriteram(*this,"spriteram")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k055673_device> m_k055673;
	required_shared_ptr<UINT16> m_gx_workram;
	optional_shared_ptr<UINT16> m_spriteram;
	std::unique_ptr<UINT8[]> m_decoded;

	UINT8 m_mw_irq_control;
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
	UINT16 m_clip;

	UINT8 m_sound_ctrl;
	UINT8 m_sound_nmi_clk;

	DECLARE_READ16_MEMBER(eeprom_r);
	DECLARE_WRITE16_MEMBER(mweeprom_w);
	DECLARE_READ16_MEMBER(dddeeprom_r);
	DECLARE_WRITE16_MEMBER(mmeeprom_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_msb_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_msb_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_READ16_MEMBER(sound_status_msb_r);
	DECLARE_WRITE16_MEMBER(irq_ack_w);
	DECLARE_READ16_MEMBER(k053247_scattered_word_r);
	DECLARE_WRITE16_MEMBER(k053247_scattered_word_w);
	DECLARE_READ16_MEMBER(k053247_martchmp_word_r);
	DECLARE_WRITE16_MEMBER(k053247_martchmp_word_w);
	DECLARE_READ16_MEMBER(mccontrol_r);
	DECLARE_WRITE16_MEMBER(mccontrol_w);
	DECLARE_WRITE8_MEMBER(sound_ctrl_w);

	DECLARE_WRITE16_MEMBER(ddd_053936_enable_w);
	DECLARE_WRITE16_MEMBER(ddd_053936_clip_w);
	DECLARE_READ16_MEMBER(gai_053936_tilerom_0_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_0_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_1_r);
	DECLARE_READ16_MEMBER(gai_053936_tilerom_2_r);
	DECLARE_READ16_MEMBER(ddd_053936_tilerom_2_r);
	TILE_GET_INFO_MEMBER(get_gai_936_tile_info);
	TILE_GET_INFO_MEMBER(get_ult_936_tile_info);
	DECLARE_MACHINE_START(mystwarr);
	DECLARE_MACHINE_RESET(mystwarr);
	DECLARE_VIDEO_START(mystwarr);
	DECLARE_MACHINE_RESET(viostorm);
	DECLARE_VIDEO_START(viostorm);
	DECLARE_MACHINE_RESET(metamrph);
	DECLARE_VIDEO_START(metamrph);
	DECLARE_MACHINE_RESET(dadandrn);
	DECLARE_VIDEO_START(dadandrn);
	DECLARE_MACHINE_RESET(gaiapols);
	DECLARE_VIDEO_START(gaiapols);
	DECLARE_MACHINE_RESET(martchmp);
	DECLARE_VIDEO_START(martchmp);
	UINT32 screen_update_mystwarr(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_metamrph(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_dadandrn(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_martchmp(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(ddd_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
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
};
