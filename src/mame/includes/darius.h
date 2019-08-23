// license:GPL-2.0+
// copyright-holders:David Graves, Jarek Burczynski
/*************************************************************************

    Darius

*************************************************************************/
#ifndef MAME_INCLUDES_DARIUS_H
#define MAME_INCLUDES_DARIUS_H

#pragma once

#include "sound/flt_vol.h"
#include "sound/msm5205.h"
#include "video/pc080sn.h"
#include "emupal.h"
#include "tilemap.h"

#define VOL_MAX    (3 * 2 + 2)
#define PAN_MAX    (2 + 2 + 1)   /* FM 2port + PSG 2port + DA 1port */

class darius_state : public driver_device
{
public:
	darius_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_spriteram(*this, "spriteram"),
		m_fg_ram(*this, "fg_ram"),
		m_audiobank(*this, "audiobank"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_msm(*this, "msm"),
		m_cpub(*this, "cpub"),
		m_adpcm(*this, "adpcm"),
		m_pc080sn(*this, "pc080sn"),
		m_filter_l{{*this, "filter0.%ul", 0U},
					{*this, "filter1.%ul", 0U}},
		m_filter_r{{*this, "filter0.%ur", 0U},
					{*this, "filter1.%ur", 0U}},
		m_msm5205_l(*this, "msm5205.l"),
		m_msm5205_r(*this, "msm5205.r"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	void darius(machine_config &config);

protected:
	virtual void device_post_load() override;
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<u16> m_spriteram;
	required_shared_ptr<u16> m_fg_ram;
	required_memory_bank m_audiobank;

	/* video-related */
	tilemap_t  *m_fg_tilemap;

	/* misc */
	u16     m_cpua_ctrl;
	u16     m_coin_word;
	u8      m_adpcm_command;
	u8      m_nmi_enable;
	u32     m_def_vol[0x10];
	u8      m_vol[VOL_MAX];
	u8      m_pan[PAN_MAX];

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<msm5205_device> m_msm;
	required_device<cpu_device> m_cpub;
	required_device<cpu_device> m_adpcm;
	required_device<pc080sn_device> m_pc080sn;

	required_device_array<filter_volume_device, 4> m_filter_l[2];
	required_device_array<filter_volume_device, 4> m_filter_r[2];
	required_device<filter_volume_device> m_msm5205_l;
	required_device<filter_volume_device> m_msm5205_r;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	void cpua_ctrl_w(u16 data);
	u16 coin_r();
	void coin_w(u16 data);
	void sound_bankswitch_w(u8 data);
	void adpcm_command_w(u8 data);
	void fm0_pan_w(u8 data);
	void fm1_pan_w(u8 data);
	void psg0_pan_w(u8 data);
	void psg1_pan_w(u8 data);
	void da_pan_w(u8 data);
	u8 adpcm_command_r();
	u8 readport2();
	u8 readport3();
	void adpcm_nmi_disable(u8 data);
	void adpcm_nmi_enable(u8 data);
	void fg_layer_w(offs_t offset, u16 data, u16 mem_mask = ~0);
	void write_portA0(u8 data);
	void write_portA1(u8 data);
	void write_portB0(u8 data);
	void write_portB1(u8 data);
	void adpcm_data_w(u8 data);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	u32 screen_update_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_middle(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	u32 screen_update_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void draw_sprites(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int x_offs, int y_offs);
	u32 update_screen(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect, int xoffs);
	void parse_control();   // assumes Z80 sandwiched between 68Ks
	void update_fm0();
	void update_fm1();
	void update_psg0(int port);
	void update_psg1(int port);
	void update_da();
	DECLARE_WRITE_LINE_MEMBER(adpcm_int);
	void darius_cpub_map(address_map &map);
	void darius_map(address_map &map);
	void darius_sound2_io_map(address_map &map);
	void darius_sound2_map(address_map &map);
	void darius_sound_map(address_map &map);
};

#endif // MAME_INCLUDES_DARIUS_H
