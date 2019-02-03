// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_INCLUDES_NYCAPTOR_H
#define MAME_INCLUDES_NYCAPTOR_H

#pragma once

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5232.h"
#include "machine/taito68705interface.h"
#include "emupal.h"

class nycaptor_state : public driver_device
{
public:
	nycaptor_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"),
		m_sharedram(*this, "sharedram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_subcpu(*this, "sub"),
		m_bmcu(*this, "bmcu"),
		m_msm(*this, "msm"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_soundlatch(*this, "soundlatch"),
		m_soundlatch2(*this, "soundlatch2"),
		m_soundnmi(*this, "soundnmi")
	{ }

	void nycaptor(machine_config &config);
	void cyclshtg(machine_config &config);
	void bronx(machine_config &config);

	void init_cyclshtg();
	void init_colt();
	void init_bronx();
	void init_nycaptor();

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrlram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t m_gfxctrl;
	uint8_t m_char_bank;
	uint8_t m_palette_bank;

	/* misc */
	int m_generic_control_reg;
	int m_gametype;
	int m_mask;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<cpu_device> m_subcpu;
	optional_device<taito68705_mcu_device> m_bmcu;
	required_device<msm5232_device> m_msm;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<generic_latch_8_device> m_soundlatch2;
	required_device<input_merger_device> m_soundnmi;

	DECLARE_WRITE8_MEMBER(sub_cpu_halt_w);
	DECLARE_READ8_MEMBER(nycaptor_b_r);
	DECLARE_READ8_MEMBER(nycaptor_by_r);
	DECLARE_READ8_MEMBER(nycaptor_bx_r);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(nycaptor_generic_control_r);
	DECLARE_WRITE8_MEMBER(nycaptor_generic_control_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_r);
	DECLARE_WRITE8_MEMBER(cyclshtg_mcu_w);
	DECLARE_READ8_MEMBER(cyclshtg_mcu_status_r1);
	DECLARE_WRITE8_MEMBER(cyclshtg_generic_control_w);
	DECLARE_READ8_MEMBER(unk_r);

	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r1);
	DECLARE_READ8_MEMBER(nycaptor_mcu_status_r2);
	DECLARE_READ8_MEMBER(sound_status_r);
	DECLARE_WRITE8_MEMBER(nycaptor_videoram_w);
	DECLARE_WRITE8_MEMBER(nycaptor_palette_w);
	DECLARE_READ8_MEMBER(nycaptor_palette_r);
	DECLARE_WRITE8_MEMBER(nycaptor_gfxctrl_w);
	DECLARE_READ8_MEMBER(nycaptor_gfxctrl_r);
	DECLARE_WRITE8_MEMBER(nycaptor_scrlram_w);
	DECLARE_WRITE8_MEMBER(unk_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update_nycaptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nycaptor_spot();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void bronx_master_map(address_map &map);
	void bronx_slave_io_map(address_map &map);
	void bronx_slave_map(address_map &map);
	void cyclshtg_master_map(address_map &map);
	void cyclshtg_slave_map(address_map &map);
	void nycaptor_master_map(address_map &map);
	void nycaptor_slave_map(address_map &map);
	void sound_map(address_map &map);
};

#endif // MAME_INCLUDES_NYCAPTOR_H
