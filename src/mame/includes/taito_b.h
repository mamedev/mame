// license:GPL-2.0+
// copyright-holders:Jarek Burczynski
#ifndef MAME_INCLUDES_TAITO_B_H
#define MAME_INCLUDES_TAITO_B_H

#pragma once

#include "machine/mb87078.h"
#include "machine/taitoio.h"
#include "video/hd63484.h"
#include "video/tc0180vcu.h"
#include "emupal.h"
#include "screen.h"

class taitob_state : public driver_device
{
public:
	taitob_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_ym(*this, "ymsnd"),
		m_hd63484(*this, "hd63484"),
		m_tc0180vcu(*this, "tc0180vcu"),
		m_tc0640fio(*this, "tc0640fio"),
		m_tc0220ioc(*this, "tc0220ioc"),
		m_tc0510nio(*this, "tc0510nio"),
		m_mb87078(*this, "mb87078"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_audiobank(*this, "audiobank"),
		m_eepromout_io(*this, "EEPROMOUT"),
		m_trackx_io(*this, "TRACKX%u", 1U),
		m_tracky_io(*this, "TRACKY%u", 1U)
	{ }

	void spacedx(machine_config &config);
	void rambo3(machine_config &config);
	void ashura(machine_config &config);
	void silentd(machine_config &config);
	void tetrista(machine_config &config);
	void spacedxo(machine_config &config);
	void rambo3p(machine_config &config);
	void rastsag2(machine_config &config);
	void qzshowby(machine_config &config);
	void sbm(machine_config &config);
	void tetrist(machine_config &config);
	void pbobble(machine_config &config);
	void masterw(machine_config &config);
	void ryujin(machine_config &config);
	void viofight(machine_config &config);
	void crimec(machine_config &config);
	void selfeena(machine_config &config);

	void init_taito_b();

protected:
	DECLARE_WRITE8_MEMBER(player_12_coin_ctrl_w);

	void sound_map(address_map &map);

	DECLARE_WRITE8_MEMBER(bankswitch_w);
	template<int Player> DECLARE_READ16_MEMBER(tracky_hi_r);
	template<int Player> DECLARE_READ16_MEMBER(tracky_lo_r);
	template<int Player> DECLARE_READ16_MEMBER(trackx_hi_r);
	template<int Player> DECLARE_READ16_MEMBER(trackx_lo_r);
	DECLARE_READ16_MEMBER(eep_latch_r);
	DECLARE_WRITE16_MEMBER(eeprom_w);
	DECLARE_READ16_MEMBER(player_34_coin_ctrl_r);
	DECLARE_WRITE16_MEMBER(player_34_coin_ctrl_w);
	DECLARE_WRITE16_MEMBER(spacedxo_tc0220ioc_w);
	DECLARE_WRITE8_MEMBER(mb87078_gain_changed);
	virtual void video_start() override;
	uint32_t screen_update_taitob(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void crimec_map(address_map &map);
	void masterw_map(address_map &map);
	void masterw_sound_map(address_map &map);
	void pbobble_map(address_map &map);
	void qzshowby_map(address_map &map);
	void rambo3_map(address_map &map);
	void rastsag2_map(address_map &map);

	void sbm_map(address_map &map);
	void selfeena_map(address_map &map);
	void silentd_map(address_map &map);
	void spacedx_map(address_map &map);
	void spacedxo_map(address_map &map);
	void tetrist_map(address_map &map);
	void tetrista_map(address_map &map);
	void viofight_map(address_map &map);
	void viofight_sound_map(address_map &map);

	virtual void machine_start() override;
	virtual void machine_reset() override;

	/* video-related */
	std::unique_ptr<bitmap_ind16> m_pixel_bitmap;

	uint16_t        m_pixel_scroll[3];

	int            m_b_fg_color_base;

	/* misc */
	uint16_t        m_eep_latch;
	uint16_t        m_coin_word;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	required_device<device_t> m_ym;
	optional_device<hd63484_device> m_hd63484;
	required_device<tc0180vcu_device> m_tc0180vcu;
	optional_device<tc0640fio_device> m_tc0640fio;
	optional_device<tc0220ioc_device> m_tc0220ioc;
	optional_device<tc0510nio_device> m_tc0510nio;
	optional_device<mb87078_device> m_mb87078;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;

	required_memory_bank m_audiobank;
	optional_ioport m_eepromout_io;
	optional_ioport_array<2> m_trackx_io;
	optional_ioport_array<2> m_tracky_io;
};

class taitob_c_state : public taitob_state
{
public:
	using taitob_state::taitob_state;
	static constexpr feature_type unemulated_features() { return feature::CAMERA; }
	void realpunc(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(realpunc_sensor);

protected:
	DECLARE_WRITE16_MEMBER(realpunc_output_w);
	DECLARE_WRITE16_MEMBER(realpunc_video_ctrl_w);

	void realpunc_map(address_map &map);
	void realpunc_hd63484_map(address_map &map);

	virtual void video_start() override;
	uint32_t screen_update_realpunc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	std::unique_ptr<bitmap_ind16> m_realpunc_bitmap;
	uint16_t        m_realpunc_video_ctrl;
};

class hitice_state : public taitob_state
{
public:
	hitice_state(const machine_config &mconfig, device_type type, const char *tag) :
		taitob_state(mconfig, type, tag),
		m_pixelram(*this, "pixelram")
	{ }

	void hitice(machine_config &config);

protected:
	virtual void video_start() override;
	virtual void video_reset() override;

private:
	DECLARE_WRITE16_MEMBER(pixelram_w);
	DECLARE_WRITE16_MEMBER(pixel_scroll_w);

	void hitice_map(address_map &map);

	void clear_pixel_bitmap();

	required_shared_ptr<uint16_t> m_pixelram;
};

#endif // MAME_INCLUDES_TAITO_B_H
