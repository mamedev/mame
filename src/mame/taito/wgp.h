// license:BSD-3-Clause
// copyright-holders:David Graves
/*************************************************************************

    World Grand Prix

*************************************************************************/
#ifndef MAME_TAITO_WGP_H
#define MAME_TAITO_WGP_H

#pragma once

#include "taitosnd.h"
#include "taitoio.h"
#include "tc0100scn.h"
#include "emupal.h"
#include "tilemap.h"


class wgp_state : public driver_device
{
public:
	wgp_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spritemap(*this, "spritemap"),
		m_spriteram(*this, "spriteram"),
		m_pivram(*this, "pivram"),
		m_piv_ctrlram(*this, "piv_ctrlram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_tc0100scn(*this, "tc0100scn"),
		m_tc0140syt(*this, "tc0140syt"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_z80bank(*this, "z80bank"),
		m_steer(*this, "STEER"),
		m_unknown(*this, "UNKNOWN"),
		m_fake(*this, "FAKE")
	{ }

	void wgp2(machine_config &config);
	void wgp(machine_config &config);

	void init_wgp();
	void init_wgp2();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;
	virtual void device_post_load() override;

	TIMER_CALLBACK_MEMBER(trigger_cpu_b_int6);

private:
	void coins_w(u8 data);
	void cpua_ctrl_w(u16 data);
	u16 lan_status_r();
	void rotate_port_w(offs_t offset, u16 data);
	u8 accel_r();
	u8 steer_r();
	u8 steer_offset_r();
	u8 accel_offset_r();
	u8 brake_r();
	u8 unknown_r();
	void sound_bankswitch_w(u8 data);
	void pivram_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void piv_ctrl_word_w(offs_t offset, u16 data, u16 mem_mask = ~0);

	DECLARE_VIDEO_START(wgp2);
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(cpub_interrupt);

	void cpu2_map(address_map &map) ATTR_COLD;
	void main_map(address_map &map) ATTR_COLD;
	void z80_sound_map(address_map &map) ATTR_COLD;

	template<unsigned Offset> TILE_GET_INFO_MEMBER(get_piv_tile_info);

	void core_vh_start(int piv_xoffs, int piv_yoffs);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int y_offs);
	void piv_layer_draw(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int layer, int flags, u32 priority);
	void parse_control();

	/* memory pointers */
	required_shared_ptr<u16> m_spritemap;
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_pivram;
	required_shared_ptr<u16> m_piv_ctrlram;

	/* video-related */
	tilemap_t *m_piv_tilemap[3]{};
	u16       m_piv_ctrl_reg = 0;
	u16       m_piv_zoom[3]{};
	u16       m_piv_scrollx[3]{};
	u16       m_piv_scrolly[3]{};
	u16       m_rotate_ctrl[8]{};
	int       m_piv_xoffs = 0;
	int       m_piv_yoffs = 0;
	u8        m_dislayer[4]{};

	/* misc */
	u16       m_cpua_ctrl = 0;
	u16       m_port_sel = 0;
	emu_timer *m_cpub_int6_timer = nullptr;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	required_device<tc0100scn_device> m_tc0100scn;
	required_device<tc0140syt_device> m_tc0140syt;
	required_device<tc0220ioc_device> m_tc0220ioc;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_memory_bank m_z80bank;
	optional_ioport m_steer;
	optional_ioport m_unknown;
	optional_ioport m_fake;
};

#endif // MAME_TAITO_WGP_H
