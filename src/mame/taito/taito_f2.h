// license:BSD-3-Clause
// copyright-holders:David Graves, Bryan McPhail, Brad Oliver, Andrew Prime, Brian Troha, Nicola Salmoria
#ifndef MAME_TAITO_TAITO_F2_H
#define MAME_TAITO_TAITO_F2_H

#pragma once

#include "taitocchip.h"
#include "taitoio.h"
#include "tc0100scn.h"
#include "tc0110pcr.h"
#include "tc0280grd.h"
#include "tc0360pri.h"
#include "tc0480scp.h"

#include "machine/timer.h"
#include "sound/okim6295.h"

#include "emupal.h"
#include "screen.h"

class taitof2_state : public driver_device
{
public:
	taitof2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_tc0100scn(*this, "tc0100scn_%u", 1U)
		, m_tc0110pcr(*this, "tc0110pcr")
		, m_tc0360pri(*this, "tc0360pri")
		, m_tc0220ioc(*this, "tc0220ioc")
		, m_tc0510nio(*this, "tc0510nio")
		, m_gfxdecode(*this, "gfxdecode")
		, m_screen(*this, "screen")
		, m_palette(*this, "palette")
		, m_sprite_extension(*this, "sprite_ext")
		, m_spriteram(*this, "spriteram")
		, m_audiobank(*this, "audiobank")
	{ }

	void taito_f2_tc0220ioc(machine_config &config) ATTR_COLD;
	void taito_f2_tc0510nio(machine_config &config) ATTR_COLD;
	void taito_f2_te7750(machine_config &config) ATTR_COLD;
	void taito_f2_tc0110pcr(machine_config &config) ATTR_COLD;
	void taito_f2(machine_config &config) ATTR_COLD;
	void thundfox(machine_config &config) ATTR_COLD;
	void dinorex(machine_config &config) ATTR_COLD;
	void koshien(machine_config &config) ATTR_COLD;
	void qzchikyu(machine_config &config) ATTR_COLD;
	void yesnoj(machine_config &config) ATTR_COLD;
	void quizhq(machine_config &config) ATTR_COLD;
	void qcrayon2(machine_config &config) ATTR_COLD;
	void qtorimon(machine_config &config) ATTR_COLD;
	void solfigtr(machine_config &config) ATTR_COLD;
	void qzquest(machine_config &config) ATTR_COLD;
	void liquidk(machine_config &config) ATTR_COLD;
	void ssi(machine_config &config) ATTR_COLD;
	void growl(machine_config &config) ATTR_COLD;
	void ninjak(machine_config &config) ATTR_COLD;
	void finalb(machine_config &config) ATTR_COLD;
	void gunfront(machine_config &config) ATTR_COLD;
	void qcrayon(machine_config &config) ATTR_COLD;
	void qjinsei(machine_config &config) ATTR_COLD;
	void yuyugogo(machine_config &config) ATTR_COLD;

	void init_finalb() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

	TIMER_CALLBACK_MEMBER(trigger_int6);

	struct f2_tempsprite
	{
		u32 code = 0, color = 0;
		bool flipx = false, flipy = false;
		int x = 0, y = 0;
		int zoomx = 0, zoomy = 0;
		u64 primask = 0;
	};

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device_array<tc0100scn_device, 2> m_tc0100scn;
	optional_device<tc0110pcr_device> m_tc0110pcr;
	optional_device<tc0360pri_device> m_tc0360pri;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	optional_device<palette_device> m_palette;

	/* memory pointers */
	optional_shared_ptr<u16> m_sprite_extension;
	required_shared_ptr<u16> m_spriteram;

	optional_memory_bank m_audiobank;

	std::unique_ptr<u16[]>   m_spriteram_buffered;
	std::unique_ptr<u16[]>   m_spriteram_delayed;

	/* video-related */
	std::unique_ptr<f2_tempsprite[]> m_spritelist;
	int           m_sprite_type = 0;

	u16           m_spritebank[8]{};
//  u16           m_spritebank_eof[8]{};
	u16           m_spritebank_buffered[8]{};

	bool          m_sprites_disabled = false;
	s32           m_sprites_active_area = 0;
	s32           m_sprites_master_scrollx = 0;
	s32           m_sprites_master_scrolly = 0;
	/* remember flip status over frames because driftout can fail to set it */
	bool          m_sprites_flipscreen = false;

	/* On the left hand screen edge (assuming horiz screen, no
	   screenflip: in screenflip it is the right hand edge etc.)
	   there may be 0-3 unwanted pixels in both tilemaps *and*
	   sprites. To erase this we use f2_hide_pixels (0 to +3). */

	s32           m_hide_pixels = 0;
	s32           m_flip_hide_pixels = 0; /* Different in some games */

	s32           m_game = 0;

	u8            m_tilepri[6]{}; // todo - move into taitoic.c
	u8            m_spritepri[6]{}; // todo - move into taitoic.c
	u8            m_spriteblendmode = 0; // todo - move into taitoic.c

	bool          m_prepare_sprites = false;

	/* misc */
	emu_timer     *m_int6_timer = nullptr;
	std::unique_ptr<u8[]> m_decoded_gfx;

	void coin_nibble_w(u8 data);
	void growl_coin_word_w(u8 data);
	void _4p_coin_word_w(u8 data);
	void sound_bankswitch_w(u8 data);
	void sprite_extension_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void spritebank_w(offs_t offset, u16 data);
	void koshien_spritebank_w(u16 data);

	DECLARE_VIDEO_START(finalb);
	DECLARE_VIDEO_START(megab);
	DECLARE_VIDEO_START(thundfox);
	DECLARE_VIDEO_START(ssi);
	DECLARE_VIDEO_START(gunfront);
	DECLARE_VIDEO_START(growl);
	DECLARE_VIDEO_START(koshien);
	DECLARE_VIDEO_START(yuyugogo);
	DECLARE_VIDEO_START(ninjak);
	DECLARE_VIDEO_START(solfigtr);
	DECLARE_VIDEO_START(qzchikyu);
	DECLARE_VIDEO_START(yesnoj);
	DECLARE_VIDEO_START(dinorex);
	DECLARE_VIDEO_START(quiz);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_pri(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_thundfox(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_ssi(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_yesnoj(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_vblank_no_buffer(int state);
	void screen_vblank_partial_buffer_delayed(int state);
	void screen_vblank_partial_buffer_delayed_thundfox(int state);
	void screen_vblank_full_buffer_delayed(int state);
	void screen_vblank_partial_buffer_delayed_qzchikyu(int state);
	INTERRUPT_GEN_MEMBER(interrupt);
	void core_vh_start(int sprite_type, int hide, int flip_hide);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u64 *primasks, int uses_tc360_mixer);
	void update_spritebanks();
	void handle_sprite_buffering();
	void update_sprites_active_area();
	void taito_f2_tc360_spritemixdraw(screen_device &screen, bitmap_ind16 &dest_bmp, const rectangle &clip, gfx_element *gfx,
	u32 code, u32 color, int flipx, int flipy, int sx, int sy, int scalex, int scaley, u64 primask = 0, bool use_mixer = false);

	void dinorex_map(address_map &map) ATTR_COLD;
	void finalb_map(address_map &map) ATTR_COLD;
	void growl_map(address_map &map) ATTR_COLD;
	void gunfront_map(address_map &map) ATTR_COLD;
	void koshien_map(address_map &map) ATTR_COLD;
	void liquidk_map(address_map &map) ATTR_COLD;
	void ninjak_map(address_map &map) ATTR_COLD;
	void qcrayon2_map(address_map &map) ATTR_COLD;
	void qcrayon_map(address_map &map) ATTR_COLD;
	void qjinsei_map(address_map &map) ATTR_COLD;
	void qtorimon_map(address_map &map) ATTR_COLD;
	void quizhq_map(address_map &map) ATTR_COLD;
	void qzchikyu_map(address_map &map) ATTR_COLD;
	void qzquest_map(address_map &map) ATTR_COLD;
	void solfigtr_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
	void ssi_map(address_map &map) ATTR_COLD;
	void thundfox_map(address_map &map) ATTR_COLD;
	void yesnoj_map(address_map &map) ATTR_COLD;
	void yuyugogo_map(address_map &map) ATTR_COLD;
};

// with C-Chip
class megablst_state : public taitof2_state
{
public:
	megablst_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitof2_state(mconfig, type, tag)
		, m_cchip(*this, "cchip")
		, m_cchip_irq_clear(*this, "cchip_irq_clear")
	{ }

	void megab(machine_config &config) ATTR_COLD;

private:
	required_device<taito_cchip_device> m_cchip;
	required_device<timer_device> m_cchip_irq_clear;

	INTERRUPT_GEN_MEMBER(megab_interrupt);
	TIMER_DEVICE_CALLBACK_MEMBER(cchip_irq_clear_cb);

	void megab_map(address_map &map) ATTR_COLD;
};

// with Mahjong inputs and tilemap bank
class mjnquest_state : public taitof2_state
{
public:
	mjnquest_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitof2_state(mconfig, type, tag)
		, m_io_in(*this, "IN%u", 0U)
		, m_io_dsw(*this, "DSW%c", 'A')
	{ }

	void mjnquest(machine_config &config) ATTR_COLD;

	void init_mjnquest() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_ioport_array<7> m_io_in;
	required_ioport_array<2> m_io_dsw;

	s32 m_mjnquest_input = 0;
	u8 m_gfxbank = 0;

	u16 dsw_r(offs_t offset);
	u16 input_r();
	void inputselect_w(u16 data);
	void gfxbank_w(u8 data);
	TC0100SCN_CB_MEMBER(tmap_cb);

	void mjnquest_map(address_map &map) ATTR_COLD;
};

// with ROZ layer
class dondokod_state : public taitof2_state
{
public:
	dondokod_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitof2_state(mconfig, type, tag)
		, m_tc0280grd(*this, "tc0280grd")
		, m_tc0430grw(*this, "tc0430grw")
	{ }

	void dondokod(machine_config &config) ATTR_COLD;
	void driftout(machine_config &config) ATTR_COLD;
	void pulirula(machine_config &config) ATTR_COLD;

protected:
	/* devices */
	optional_device<tc0280grd_device> m_tc0280grd;
	optional_device<tc0430grw_device> m_tc0430grw;

	s32 m_pivot_xdisp = 0;  /* Needed in games with a pivot layer */
	s32 m_pivot_ydisp = 0;

	DECLARE_VIDEO_START(dondokod);
	DECLARE_VIDEO_START(driftout);
	DECLARE_VIDEO_START(pulirula);
	u32 screen_update_pri_roz(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_roz_layer(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, u8 priority, u8 priority_mask = 0xff);

	void dondokod_map(address_map &map) ATTR_COLD;
	void driftout_map(address_map &map) ATTR_COLD;
	void pulirula_map(address_map &map) ATTR_COLD;
};

// with Paddle
class cameltry_state : public dondokod_state
{
public:
	cameltry_state(const machine_config &mconfig, device_type type, const char *tag)
		: dondokod_state(mconfig, type, tag)
		, m_io_paddle(*this, "PADDLE%u", 1U)
	{ }

	void cameltrya(machine_config &config) ATTR_COLD;
	void cameltry(machine_config &config) ATTR_COLD;
	void driftoutct(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_ioport_array<2> m_io_paddle;

	s32 m_last[2]{};

	u16 paddle_r(offs_t offset);
	void cameltrya_porta_w(u8 data);

	void cameltry_map(address_map &map) ATTR_COLD;
	void cameltrya_map(address_map &map) ATTR_COLD;
	void cameltrya_sound_map(address_map &map) ATTR_COLD;
	void driftoutct_map(address_map &map) ATTR_COLD;
};

// Bootleg with different sound system
class driveout_state : public dondokod_state
{
public:
	driveout_state(const machine_config &mconfig, device_type type, const char *tag)
		: dondokod_state(mconfig, type, tag)
		, m_okibank(*this, "okibank")
	{ }

	void driveout(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	required_memory_bank m_okibank;

	/* misc */
	bool m_nibble = false;
	s32  m_sound_latch = 0;

	u8 sound_command_r();
	void oki_bank_w(u8 data);
	void sound_command_w(offs_t offset, u8 data);

	void driveout_map(address_map &map) ATTR_COLD;
	void driveout_oki_map(address_map &map) ATTR_COLD;
	void driveout_sound_map(address_map &map) ATTR_COLD;
};

// with TC0480SCP tilemap chip
class footchmp_state : public taitof2_state
{
public:
	footchmp_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitof2_state(mconfig, type, tag)
		, m_tc0480scp(*this, "tc0480scp")
	{ }

	void deadconx(machine_config &config) ATTR_COLD;
	void deadconxj(machine_config &config) ATTR_COLD;
	void footchmp(machine_config &config) ATTR_COLD;
	void footchmpbl(machine_config &config) ATTR_COLD;
	void hthero(machine_config &config) ATTR_COLD;
	void metalb(machine_config &config) ATTR_COLD;

protected:
	required_device<tc0480scp_device> m_tc0480scp;

	DECLARE_VIDEO_START(deadconx);
	DECLARE_VIDEO_START(footchmp);
	u32 screen_update_deadconx(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_metalb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void deadconx_map(address_map &map) ATTR_COLD;
	void footchmp_map(address_map &map) ATTR_COLD;
	void metalb_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_TAITO_F2_H
