// license:BSD-3-Clause
// copyright-holders:Tomasz Slanina
#ifndef MAME_TAITO_NYCAPTOR_H
#define MAME_TAITO_NYCAPTOR_H

#pragma once

#include "taito68705.h"

#include "machine/gen_latch.h"
#include "machine/input_merger.h"
#include "sound/msm5232.h"

#include "emupal.h"
#include "tilemap.h"


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
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	/* memory pointers */
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_scrlram;
	required_shared_ptr<uint8_t> m_sharedram;
	required_shared_ptr<uint8_t> m_spriteram;

	/* video-related */
	tilemap_t *m_bg_tilemap = nullptr;
	std::vector<uint8_t> m_paletteram;
	std::vector<uint8_t> m_paletteram_ext;
	uint8_t m_gfxctrl = 0;
	uint8_t m_char_bank = 0;
	uint8_t m_palette_bank = 0;

	/* misc */
	int m_generic_control_reg = 0;
	int m_gametype = 0;
	int m_mask = 0;

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

	void sub_cpu_halt_w(uint8_t data);
	uint8_t nycaptor_b_r();
	uint8_t nycaptor_by_r();
	uint8_t nycaptor_bx_r();
	void sound_cpu_reset_w(uint8_t data);
	void nmi_disable_w(uint8_t data);
	void nmi_enable_w(uint8_t data);
	uint8_t nycaptor_generic_control_r();
	void nycaptor_generic_control_w(uint8_t data);
	uint8_t cyclshtg_mcu_status_r();
	uint8_t cyclshtg_mcu_r();
	void cyclshtg_mcu_w(uint8_t data);
	uint8_t cyclshtg_mcu_status_r1();
	void cyclshtg_generic_control_w(uint8_t data);
	uint8_t unk_r();

	uint8_t nycaptor_mcu_status_r1();
	uint8_t nycaptor_mcu_status_r2();
	uint8_t sound_status_r();
	void nycaptor_videoram_w(offs_t offset, uint8_t data);
	void nycaptor_palette_w(offs_t offset, uint8_t data);
	uint8_t nycaptor_palette_r(offs_t offset);
	void nycaptor_gfxctrl_w(uint8_t data);
	uint8_t nycaptor_gfxctrl_r();
	void nycaptor_scrlram_w(offs_t offset, uint8_t data);
	void unk_w(uint8_t data);
	TILE_GET_INFO_MEMBER(get_tile_info);
	uint32_t screen_update_nycaptor(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	int nycaptor_spot();
	void draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int pri);
	void bronx_master_map(address_map &map) ATTR_COLD;
	void bronx_slave_io_map(address_map &map) ATTR_COLD;
	void bronx_slave_map(address_map &map) ATTR_COLD;
	void cyclshtg_master_map(address_map &map) ATTR_COLD;
	void cyclshtg_slave_map(address_map &map) ATTR_COLD;
	void nycaptor_master_map(address_map &map) ATTR_COLD;
	void nycaptor_slave_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;
};

#endif // MAME_TAITO_NYCAPTOR_H
