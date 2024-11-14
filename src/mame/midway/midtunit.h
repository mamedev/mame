// license:BSD-3-Clause
// copyright-holders:Alex Pasadyn, Zsolt Vasvari, Ernesto Corvi, Aaron Giles
// thanks-to:Kurt Mahan
/*************************************************************************

    Driver for Midway T-unit games.

**************************************************************************/
#ifndef MAME_MIDWAY_MIDTUNIT_H
#define MAME_MIDWAY_MIDTUNIT_H

#pragma once

#include "midtunit_v.h"

#include "dcs.h"
#include "williamssound.h"

#include "cpu/tms34010/tms34010.h"

#include "emupal.h"


class midtunit_base_state : public driver_device
{
protected:
	midtunit_base_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_video(*this, "video"),
		m_palette(*this, "palette"),
		m_nvram(*this, "nvram")
	{ }

	void tunit_core(machine_config &config);

	void machine_start() override ATTR_COLD;

	void cmos_enable_w(uint16_t data);
	void cmos_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t cmos_r(offs_t offset);

	void main_map(address_map &map) ATTR_COLD;

	required_device<tms340x0_device> m_maincpu;
	required_device<midtunit_video_device> m_video;
	required_device<palette_device> m_palette;

	required_shared_ptr<uint16_t> m_nvram;

	// CMOS-related variables
	uint8_t    m_cmos_write_enable = 0;
};

class midtunit_adpcm_state : public midtunit_base_state
{
public:
	midtunit_adpcm_state(const machine_config &mconfig, device_type type, const char *tag) :
		midtunit_base_state(mconfig, type, tag),
		m_adpcm_sound(*this, "adpcm")
	{ }

	void tunit_adpcm(machine_config &config);

	void init_mktunit();
	void init_mkturbo();
	void init_nbajamte();
	void init_nbajam();
	void init_jdreddp();

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	uint16_t sound_state_r();
	uint16_t sound_r();
	void sound_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t mk_prot_r(offs_t offset);
	void mk_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);
	uint16_t mkturbo_prot_r();
	uint16_t nbajam_prot_r();
	void nbajam_prot_w(offs_t offset, uint16_t data);
	void jdredd_prot_w(offs_t offset, uint16_t data);
	uint16_t jdredd_prot_r(offs_t offset);

	void init_nbajam_common(int te_protection);

	void main_adpcm_map(address_map &map) ATTR_COLD;

	required_device<williams_adpcm_sound_device> m_adpcm_sound;

	// sound-related variables
	uint8_t    m_fake_sound_state = 0;

	// protection
	uint8_t    m_mk_prot_index = 0;
	std::unique_ptr<uint8_t[]> m_hidden_ram;

	const uint32_t *m_nbajam_prot_table = nullptr;
	uint16_t   m_nbajam_prot_queue[5] = {};
	uint8_t    m_nbajam_prot_index = 0;

	const uint8_t *m_jdredd_prot_table = nullptr;
	uint8_t    m_jdredd_prot_index = 0;
	uint8_t    m_jdredd_prot_max = 0;
};

class mk2_state : public midtunit_base_state
{
public:
	mk2_state(const machine_config &mconfig, device_type type, const char *tag) :
		midtunit_base_state(mconfig, type, tag),
		m_dcs(*this, "dcs")
	{ }

	void mk2(machine_config &config);

	void init_mk2();

protected:
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;

private:
	uint16_t dcs_state_r();
	uint16_t dcs_r();
	void dcs_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	uint16_t mk2_prot_const_r();
	uint16_t mk2_prot_r();
	uint16_t mk2_prot_shift_r();
	void mk2_prot_w(offs_t offset, uint16_t data, uint16_t mem_mask = ~0);

	void mk2_map(address_map &map) ATTR_COLD;

	required_device<dcs_audio_device> m_dcs;

	// protection
	uint16_t   m_mk2_prot_data = 0;
};

#endif // MAME_MIDWAY_MIDTUNIT_H
