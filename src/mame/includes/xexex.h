// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
/*************************************************************************

    Xexex

*************************************************************************/

#include "video/k053250.h"
#include "sound/flt_vol.h"
#include "sound/k054539.h"
#include "machine/gen_latch.h"
#include "machine/k053252.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054338.h"
#include "video/k053251.h"
#include "video/konami_helper.h"
#include "screen.h"

class xexex_state : public driver_device
{
public:
	xexex_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_k054539(*this, "k054539"),
		m_filter1l(*this, "filter1l"),
		m_filter1r(*this, "filter1r"),
		m_filter2l(*this, "filter2l"),
		m_filter2r(*this, "filter2r"),
		m_tilemap(*this, "tilemap"),
		m_sprites(*this, "sprites"),
		m_k053250(*this, "k053250"),
		m_mixer(*this, "mixer"),
		m_video_timings(*this, "video_timings"),
		m_blender(*this, "blender"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundlatch3(*this, "soundlatch3") { }

	/* memory pointers */
	required_shared_ptr<uint16_t> m_workram;
	required_shared_ptr<uint16_t> m_spriteram;

	/* video-related */
	int        m_layer_colorbase[4];
	int        m_sprite_colorbase;
	int        m_layerpri[4];
	int        m_cur_alpha;

	/* misc */
	uint16_t     m_cur_control2;
	int32_t      m_strip_0x1a;
	int        m_suspension_active;
	int        m_resume_trigger;
	emu_timer  *m_dmadelay_timer;
	int        m_frame;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<k054539_device> m_k054539;
	required_device<filter_volume_device> m_filter1l;
	required_device<filter_volume_device> m_filter1r;
	required_device<filter_volume_device> m_filter2l;
	required_device<filter_volume_device> m_filter2r;
	required_device<k054156_054157_device> m_tilemap;
	required_device<k053246_053247_device> m_sprites;
	required_device<k053250_device> m_k053250;
	required_device<k053251_device> m_mixer;
	required_device<k053252_device> m_video_timings;
	required_device<k054338_device> m_blender;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<generic_latch_8_device> m_soundlatch3;

	DECLARE_READ16_MEMBER(spriteram_mirror_r);
	DECLARE_WRITE16_MEMBER(spriteram_mirror_w);
	DECLARE_READ16_MEMBER(xexex_waitskip_r);
	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_DRIVER_INIT(xexex);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	TIMER_DEVICE_CALLBACK_MEMBER(xexex_interrupt);
	void xexex_postload();
	void xexex_objdma( int limiter );
	void parse_control2(  );
	//	K056832_CB_MEMBER(tile_callback);
	K054539_CB_MEMBER(ym_set_mixing);

	void blender_init(bitmap_ind16 **bitmaps);
	void blender_update(bitmap_ind16 **bitmaps, const rectangle &cliprect);
};
