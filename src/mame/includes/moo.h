// license:BSD-3-Clause
// copyright-holders:R. Belmont, Acho A. Tang
/*************************************************************************

    Wild West C.O.W.boys of Moo Mesa / Bucky O'Hare

*************************************************************************/
#include "sound/okim6295.h"
#include "sound/k054539.h"
#include "machine/k053252.h"
#include "video/k053251.h"
#include "video/k054156_k054157_k056832.h"
#include "video/k053246_k053247_k055673.h"
#include "video/k054000.h"
#include "video/k054338.h"
#include "video/konami_helper.h"

class moo_state : public driver_device
{
public:
	moo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_workram(*this, "workram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_soundcpu(*this, "soundcpu"),
		m_oki(*this, "oki"),
		m_k054539(*this, "k054539"),
		m_k053246(*this, "k053246"),
		m_k053251(*this, "k053251"),
		m_k053252(*this, "k053252"),
		m_k056832(*this, "k056832"),
		m_k054338(*this, "k054338"),
		m_palette(*this, "palette"),
		m_screen(*this, "screen") { }

	/* memory pointers */
	optional_shared_ptr<UINT16> m_workram;
	required_shared_ptr<UINT16> m_spriteram;

	/* video-related */
	int         m_sprite_colorbase;
	int         m_layer_colorbase[4];
	int         m_layerpri[3];
	int         m_alpha_enabled;
	UINT16      m_zmask;

	/* misc */
	UINT16      m_protram[16];
	UINT16      m_cur_control2;

	/* devices */
	required_device<cpu_device> m_maincpu;
	optional_device<cpu_device> m_soundcpu;
	optional_device<okim6295_device> m_oki;
	optional_device<k054539_device> m_k054539;
	required_device<k053247_device> m_k053246;
	required_device<k053251_device> m_k053251;
	optional_device<k053252_device> m_k053252;
	required_device<k056832_device> m_k056832;
	required_device<k054338_device> m_k054338;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;

	emu_timer *m_dmaend_timer;
	DECLARE_READ16_MEMBER(control2_r);
	DECLARE_WRITE16_MEMBER(control2_w);
	DECLARE_WRITE16_MEMBER(sound_cmd1_w);
	DECLARE_WRITE16_MEMBER(sound_cmd2_w);
	DECLARE_WRITE16_MEMBER(sound_irq_w);
	DECLARE_READ16_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);
	DECLARE_WRITE16_MEMBER(moo_prot_w);
	DECLARE_WRITE16_MEMBER(moobl_oki_bank_w);
	DECLARE_MACHINE_START(moo);
	DECLARE_MACHINE_RESET(moo);
	DECLARE_VIDEO_START(moo);
	DECLARE_VIDEO_START(bucky);
	UINT32 screen_update_moo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(moo_interrupt);
	INTERRUPT_GEN_MEMBER(moobl_interrupt);
	TIMER_CALLBACK_MEMBER(dmaend_callback);
	void moo_objdma();
	K056832_CB_MEMBER(tile_callback);
	K053246_CB_MEMBER(sprite_callback);
};
