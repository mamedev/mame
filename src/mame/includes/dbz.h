// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Pierpaolo Prazzoli
/*************************************************************************

    Dragonball Z

*************************************************************************/

#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k053936.h"
#include "video/k053251.h"
#include "video/konami_helper.h"

class dbz_state : public driver_device
{
public:
	dbz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_bg1_videoram(*this, "bg1_videoram"),
		m_bg2_videoram(*this, "bg2_videoram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_video_timings(*this, "video_timings"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_k053936_1(*this, "k053936_1"),
		m_k053936_2(*this, "k053936_2"),
		m_mixer(*this, "mixer"),
		m_gfxdecode(*this, "gfxdecode"),
		m_soundlatch(*this, "soundlatch") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_bg1_videoram;
	required_shared_ptr<uint16_t> m_bg2_videoram;

	/* video-related */
	tilemap_t    *m_bg1_tilemap;
	tilemap_t    *m_bg2_tilemap;
	int          m_layer_colorbase[6];
	int          m_layerpri[5];
	int          m_sprite_colorbase;

	/* misc */
	int           m_control;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k053252_device> m_video_timings;
	required_device<k054156_054157_device> m_tilemap;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053936_device> m_k053936_1;
	required_device<k053936_device> m_k053936_2;
	required_device<k053251_device> m_mixer;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<generic_latch_8_device> m_soundlatch;

	DECLARE_WRITE16_MEMBER(dbzcontrol_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_command_w);
	DECLARE_WRITE16_MEMBER(dbz_sound_cause_nmi);
	DECLARE_WRITE16_MEMBER(dbz_bg2_videoram_w);
	DECLARE_WRITE16_MEMBER(dbz_bg1_videoram_w);
	DECLARE_WRITE_LINE_MEMBER(dbz_irq2_ack_w);
	DECLARE_DRIVER_INIT(dbza);
	DECLARE_DRIVER_INIT(dbz);
	DECLARE_DRIVER_INIT(dbz2);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(dbz_scanline);
};
