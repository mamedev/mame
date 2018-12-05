// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/
#ifndef MAME_INCLUDES_MIDTUNIT_H
#define MAME_INCLUDES_MIDTUNIT_H

#pragma once

#include "audio/dcs.h"
#include "audio/williams.h"
#include "video/midtunit.h"

#include "cpu/tms34010/tms34010.h"
#include "emupal.h"


class midtunit_state : public driver_device
{
public:
	midtunit_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "video"),
		m_dcs(*this, "dcs"),
		m_palette(*this, "palette"),
		m_gfxrom(*this, "gfxrom"),
		m_cvsd_sound(*this, "cvsd"),
		m_adpcm_sound(*this, "adpcm"),
		m_nvram(*this, "nvram")
	{ }

	void tunit_core(machine_config &config);
	void tunit_adpcm(machine_config &config);
	void tunit_dcs(machine_config &config);

	void init_mktunit();
	void init_mkturbo();
	void init_nbajamte();
	void init_nbajam();
	void init_jdreddp();
	void init_mk2();

protected:
	void machine_reset() override;

	required_device<tms340x0_device> m_maincpu;
	required_device<midtunit_video_device> m_video;
	optional_device<dcs_audio_device> m_dcs;
	required_device<palette_device> m_palette;
	required_memory_region m_gfxrom;

private:
	optional_device<williams_cvsd_sound_device> m_cvsd_sound;
	optional_device<williams_adpcm_sound_device> m_adpcm_sound;

	required_shared_ptr<uint16_t> m_nvram;

	DECLARE_WRITE16_MEMBER(midtunit_cmos_enable_w);
	DECLARE_WRITE16_MEMBER(midtunit_cmos_w);
	DECLARE_READ16_MEMBER(midtunit_cmos_r);
	DECLARE_READ16_MEMBER(midtunit_sound_state_r);
	DECLARE_READ16_MEMBER(midtunit_sound_r);
	DECLARE_WRITE16_MEMBER(midtunit_sound_w);
	DECLARE_READ16_MEMBER(mk_prot_r);
	DECLARE_WRITE16_MEMBER(mk_prot_w);
	DECLARE_READ16_MEMBER(mkturbo_prot_r);
	DECLARE_READ16_MEMBER(mk2_prot_const_r);
	DECLARE_READ16_MEMBER(mk2_prot_r);
	DECLARE_READ16_MEMBER(mk2_prot_shift_r);
	DECLARE_WRITE16_MEMBER(mk2_prot_w);
	DECLARE_READ16_MEMBER(nbajam_prot_r);
	DECLARE_WRITE16_MEMBER(nbajam_prot_w);
	DECLARE_WRITE16_MEMBER(jdredd_prot_w);
	DECLARE_READ16_MEMBER(jdredd_prot_r);

	void register_state_saving();
	void init_tunit_generic(int sound);
	void init_nbajam_common(int te_protection);

	/* CMOS-related variables */
	uint8_t    m_cmos_write_enable;

	/* sound-related variables */
	uint8_t    m_chip_type;
	uint8_t    m_fake_sound_state;

	/* protection */
	uint8_t    m_mk_prot_index;
	uint16_t   m_mk2_prot_data;

	const uint32_t *m_nbajam_prot_table;
	uint16_t   m_nbajam_prot_queue[5];
	uint8_t    m_nbajam_prot_index;

	const uint8_t *m_jdredd_prot_table;
	uint8_t    m_jdredd_prot_index;
	uint8_t    m_jdredd_prot_max;

	void main_map(address_map &map);
};

#endif // MAME_INCLUDES_MIDTUNIT_H
