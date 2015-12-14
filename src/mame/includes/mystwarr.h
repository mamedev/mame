// license:BSD-3-Clause
// copyright-holders:R. Belmont, Phil Stroffolino, Acho A. Tang, Nicola Salmoria

#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "sound/k054539.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k054338.h"
#include "video/k055555.h"

class mystwarr_state : public driver_device
{
public:
	mystwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_soundcpu(*this,"soundcpu"),
		m_video_timings(*this, "video_timings"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_mixer(*this, "mixer"),
		m_blender(*this, "blender"),
		m_k054539_1(*this,"k054539_1"),
		m_k054539_2(*this,"k054539_2"),
		m_workram(*this,"workram"),
		m_spriteram(*this,"spriteram"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_soundcpu;
	required_device<k053252_device> m_video_timings;
	required_device<k054156_054157_device> m_tilemap;
	required_device<k053246_055673_device> m_sprites;
	required_device<k055555_device> m_mixer;
	required_device<k054338_device> m_blender;
	required_device<k054539_device> m_k054539_1;
	required_device<k054539_device> m_k054539_2;
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;
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

	uint16_t m_prot_data[0x20];

	uint8_t m_sound_ctrl;
	uint8_t m_sound_nmi_clk;

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

	DECLARE_WRITE16_MEMBER(K053990_martchmp_word_w);

	DECLARE_READ16_MEMBER(K055550_word_r);
	DECLARE_WRITE16_MEMBER(K055550_word_w);

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
	DECLARE_MACHINE_RESET(viostorm);
	DECLARE_MACHINE_RESET(metamrph);
	DECLARE_MACHINE_RESET(dadandrn);
	DECLARE_MACHINE_RESET(gaiapols);
	DECLARE_MACHINE_RESET(martchmp);
	INTERRUPT_GEN_MEMBER(ddd_interrupt);
	DECLARE_WRITE_LINE_MEMBER(k054539_nmi_gen);
	TIMER_DEVICE_CALLBACK_MEMBER(mystwarr_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(metamrph_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(mchamp_interrupt);

	void blender_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
	void mixer_init(bitmap_ind16 **bitmaps);
	void mixer_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
};
