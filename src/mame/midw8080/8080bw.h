// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    8080-based black and white hardware

****************************************************************************/
#ifndef MAME_MIDW8080_8080BW_H
#define MAME_MIDW8080_8080BW_H

#pragma once

#include "8080bw_a.h"
#include "mw8080bw.h"

#include "machine/eepromser.h"
#include "machine/timer.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "sound/spkrdev.h"

#include "emupal.h"
#include "screen.h"


/* for games in 8080bw.cpp */
#define CABINET_PORT_TAG                  "CAB"


// TODO: turn the "Space Invaders samples" audio into a device and get rid of this class
class invaders_clone_state : public invaders_state
{
public:
	ioport_value sicv_in2_control_r();
	ioport_value invadpt2_in1_control_r();
	ioport_value invadpt2_in2_control_r();

protected:
	invaders_clone_state(const machine_config &mconfig, device_type type, const char *tag) :
		invaders_state(mconfig, type, tag),
		m_sn(*this, "snsnd"),
		m_samples(*this, "samples")
	{
	}

	void invaders_samples_audio(machine_config &config);

	optional_device<sn76477_device> m_sn;
	optional_device<samples_device> m_samples;
};


class sisv_state : public invaders_clone_state // only using invaders_clone_state for the custom input handler
{
public:
	sisv_state(const machine_config &mconfig, device_type type, const char *tag) :
		invaders_clone_state(mconfig, type, tag)
	{
	}
};


class invaders_clone_palette_state : public invaders_clone_state
{
protected:
	invaders_clone_palette_state(const machine_config &mconfig, device_type type, const char *tag) :
		invaders_clone_state(mconfig, type, tag),
		m_palette(*this, "palette")
	{
	}

	void set_pixel( bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, int color );
	void set_8_pixels(bitmap_rgb32 &bitmap, uint8_t y, uint8_t x, uint8_t data, int fore_color, int back_color);
	void clear_extra_columns( bitmap_rgb32 &bitmap, int color );

	optional_device<palette_device> m_palette; // TODO: make this required when things are untangled more
};


class _8080bw_state : public invaders_clone_palette_state
{
public:
	_8080bw_state(const machine_config &mconfig, device_type type, const char *tag)
		: invaders_clone_palette_state(mconfig, type, tag)
		, m_schaser_effect_555_timer(*this, "schaser_sh_555")
		, m_speaker(*this, "speaker")
		, m_colorram(*this, "colorram")
	{ }

	void indianbtbr(machine_config &config);
	void spcewarla(machine_config &config);
	void escmars(machine_config &config);
	void lrescue(machine_config &config);
	void lrescuem2(machine_config &config);
	void polaris(machine_config &config);
	void attackfc(machine_config &config);
	void attackfcu(machine_config &config);
	void astropal(machine_config &config);
	void sflush(machine_config &config);
	void invadpt2(machine_config &config);
	void lupin3a(machine_config &config);
	void indianbt(machine_config &config);
	void starw1(machine_config &config);
	void cosmo(machine_config &config);
	void spcewars(machine_config &config);
	void cosmicmo(machine_config &config);
	void ballbomb(machine_config &config);
	void crashrd(machine_config &config);
	void schasercv(machine_config &config);
	void lupin3(machine_config &config);
	void spacerng(machine_config &config);
	void steelwkr(machine_config &config);
	void schaser(machine_config &config);

	void init_attackfc();

	int cosmicmo_cab_r();
	int sflush_80_r();

protected:
	virtual void video_start() override { m_color_map = m_screen_red = 0; }

	void invadpt2_sh_port_1_w(uint8_t data);
	void invadpt2_sh_port_2_w(uint8_t data);

	DECLARE_MACHINE_START(extra_8080bw_vh);
	DECLARE_MACHINE_START(extra_8080bw_sh);
	DECLARE_MACHINE_START(extra_8080bw);

	uint32_t screen_update_invadpt2(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	/* devices/memory pointers */
	optional_device<timer_device> m_schaser_effect_555_timer;
	optional_device<speaker_sound_device> m_speaker;
	optional_shared_ptr<uint8_t> m_colorram;

	/* misc game specific */
	uint8_t m_color_map = 0;
	uint8_t m_screen_red = 0;

private:
	std::unique_ptr<uint8_t[]> m_scattered_colorram;

	/* sound-related */
	uint8_t       m_port_1_last_extra = 0;
	uint8_t       m_port_2_last_extra = 0;

	attotime m_schaser_effect_555_time_remain;
	int32_t m_schaser_effect_555_time_remain_savable = 0;
	int m_schaser_effect_555_is_low = 0;
	int m_schaser_explosion = 0;
	int m_schaser_last_effect = 0;

	uint8_t m_polaris_cloud_speed = 0;
	uint8_t m_polaris_cloud_pos = 0;
	uint8_t m_schaser_background_disable = 0;
	uint8_t m_schaser_background_select = 0;

	uint8_t indianbt_r();
	uint8_t polaris_port00_r();
	void steelwkr_sh_port_3_w(uint8_t data);
	void spacerng_sh_port_2_w(uint8_t data);
	void spcewars_sh_port_w(uint8_t data);
	void lrescue_sh_port_1_w(uint8_t data);
	void lrescue_sh_port_2_w(uint8_t data);
	void cosmo_sh_port_2_w(uint8_t data);
	void ballbomb_01_w(uint8_t data);
	void ballbomb_sh_port_1_w(uint8_t data);
	void ballbomb_sh_port_2_w(uint8_t data);
	void indianbt_sh_port_1_w(uint8_t data);
	void indianbt_sh_port_2_w(uint8_t data);
	void indianbtbr_sh_port_1_w(uint8_t data);
	void indianbtbr_sh_port_2_w(uint8_t data);
	uint8_t indianbt_01_r();
	void schaser_sh_port_1_w(uint8_t data);
	void schaser_sh_port_2_w(uint8_t data);
	uint8_t sflush_in0_r();
	void lupin3_00_w(uint8_t data);
	void lupin3_sh_port_1_w(uint8_t data);
	void lupin3_sh_port_2_w(uint8_t data);
	uint8_t schasercv_02_r();
	void schasercv_sh_port_1_w(uint8_t data);
	void schasercv_sh_port_2_w(uint8_t data);
	void crashrd_port03_w(uint8_t data);
	void crashrd_port05_w(uint8_t data);

	uint8_t schaser_scattered_colorram_r(offs_t offset);
	void schaser_scattered_colorram_w(offs_t offset, uint8_t data);

	DECLARE_MACHINE_START(sflush);
	DECLARE_MACHINE_START(schaser);
	DECLARE_MACHINE_START(schasercv);
	DECLARE_MACHINE_RESET(schaser);
	DECLARE_MACHINE_START(polaris);
	DECLARE_MACHINE_START(schaser_sh);
	DECLARE_MACHINE_RESET(schaser_sh);

	void sflush_palette(palette_device &palette) const;

	uint32_t screen_update_cosmo(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_schaser(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_schasercv(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_sflush(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_indianbt(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_lupin3(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_polaris(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	uint32_t screen_update_ballbomb(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void polaris_60hz_w(int state);
	TIMER_DEVICE_CALLBACK_MEMBER(schaser_effect_555_cb);
	void indianbt_sh_port_3_w(uint8_t data);
	void polaris_sh_port_1_w(uint8_t data);
	void polaris_sh_port_2_w(uint8_t data);
	void polaris_sh_port_3_w(uint8_t data);

	void schaser_reinit_555_time_remain();

	void astropal_io_map(address_map &map) ATTR_COLD;
	void attackfc_io_map(address_map &map) ATTR_COLD;
	void attackfcu_io_map(address_map &map) ATTR_COLD;
	void ballbomb_io_map(address_map &map) ATTR_COLD;
	void cosmicmo_io_map(address_map &map) ATTR_COLD;
	void cosmo_io_map(address_map &map) ATTR_COLD;
	void cosmo_map(address_map &map) ATTR_COLD;
	void crashrd_io_map(address_map &map) ATTR_COLD;
	void escmars_map(address_map &map) ATTR_COLD;
	void indianbt_io_map(address_map &map) ATTR_COLD;
	void indianbtbr_io_map(address_map &map) ATTR_COLD;
	void invadpt2_io_map(address_map &map) ATTR_COLD;
	void lrescue_io_map(address_map &map) ATTR_COLD;
	void lrescuem2_io_map(address_map &map) ATTR_COLD;
	void lupin3_io_map(address_map &map) ATTR_COLD;
	void polaris_io_map(address_map &map) ATTR_COLD;
	void schaser_io_map(address_map &map) ATTR_COLD;
	void schaser_map(address_map &map) ATTR_COLD;
	void schasercv_io_map(address_map &map) ATTR_COLD;
	void sflush_map(address_map &map) ATTR_COLD;
	void spacerng_io_map(address_map &map) ATTR_COLD;
	void spcewarla_io_map(address_map &map) ATTR_COLD;
	void spcewars_io_map(address_map &map) ATTR_COLD;
	void starw1_io_map(address_map &map) ATTR_COLD;
	void steelwkr_io_map(address_map &map) ATTR_COLD;
};


/*----------- defined in audio/8080bw.cpp -----------*/
extern const char *const lrescue_sample_names[];
extern const char *const lupin3_sample_names[];

DISCRETE_SOUND_EXTERN( ballbomb_discrete );
DISCRETE_SOUND_EXTERN( indianbt_discrete );
DISCRETE_SOUND_EXTERN( polaris_discrete );
DISCRETE_SOUND_EXTERN( schaser_discrete );


/*******************************************************/
/* Sidam Invasion                                      */
/*******************************************************/

class invasion_state : public invaders_state
{
public:
	invasion_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_state(mconfig, type, tag)
	{
	}

	void invasion(machine_config &config);

private:
	void io_map(address_map &map) ATTR_COLD;
};


/*******************************************************/
/* Darth Vader bootleg                                 */
/*******************************************************/

class darthvdr_state : public invaders_clone_state
{
public:
	darthvdr_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_clone_state(mconfig, type, tag)
	{
	}

	void darthvdr(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void darthvdr_00_w(uint8_t data);
	void darthvdr_08_w(uint8_t data);
	IRQ_CALLBACK_MEMBER(darthvdr_interrupt_vector);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t m_port_1_last = 0;
	uint8_t m_fleet_step = 0;
};


/*******************************************************/
/* Space Combat bootleg                                */
/*******************************************************/

class spacecom_state : public invaders_state
{
public:
	spacecom_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_state(mconfig, type, tag),
		m_palette(*this, "palette")
	{
	}

	void spacecom(machine_config &config);

	void init_spacecom();

private:
	uint32_t screen_update_spacecom(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_device<palette_device> m_palette;
};


/*******************************************************/
/* Zenitone-Microsec Invader's Revenge                 */
/*******************************************************/

class invrvnge_state : public _8080bw_state
{
public:
	invrvnge_state(machine_config const &mconfig, device_type type, char const *tag) :
		_8080bw_state(mconfig, type, tag),
		m_audiocpu(*this, "audiocpu")
	{
	}

	void invrvnge(machine_config &config);

	void init_invrvnge();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void port03_w(uint8_t data);
	void port05_w(uint8_t data);

	TIMER_DEVICE_CALLBACK_MEMBER(nmi_timer);

	void io_map(address_map &map) ATTR_COLD;
	void sound_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_audiocpu;

	uint8_t m_sound_data = 0;
	uint8_t m_timer_state = 1;
};


/*******************************************************/
/* Zilec Vortex                                        */
/*******************************************************/

class vortex_state : public invaders_state
{
public:
	vortex_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_state(mconfig, type, tag)
	{
	}

	void vortex(machine_config &config);

	void init_vortex();

private:
	uint32_t screen_update_vortex(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map) ATTR_COLD;
};


/*******************************************************/
/* Nichibutsu Rolling Crash / Moon Base                */
/*******************************************************/

class rollingc_state : public _8080bw_state // TODO: untangle the invadpt2 sounds from _8080bw_state
{
public:
	rollingc_state(machine_config const &mconfig, device_type type, char const *tag) :
		_8080bw_state(mconfig, type, tag)
	{
	}

	void rollingc(machine_config &config);

	ioport_value game_select_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void rollingc_sh_port_w(uint8_t data);

	uint8_t scattered_colorram_r(offs_t offset);
	void scattered_colorram_w(offs_t offset, uint8_t data);
	uint8_t scattered_colorram2_r(offs_t offset);
	void scattered_colorram2_w(offs_t offset, uint8_t data);

	void rollingc_palette(palette_device &palette) const;

	uint32_t screen_update_rollingc(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	std::unique_ptr<uint8_t []> m_scattered_colorram;
	std::unique_ptr<uint8_t []> m_scattered_colorram2;
	uint8_t m_port_3_last = 0;
};


/*************************************/
/* SNK Ozma Wars                     */
/*************************************/

class ozmawars_state : public _8080bw_state
{
public:
	ozmawars_state(const machine_config &mconfig, device_type type, const char *tag) :
		_8080bw_state(mconfig, type, tag)
	{
	}

	void ozmawars(machine_config &config);
	void ozmawars_samples_audio(machine_config &config);

private:
	void ozmawars_port03_w(uint8_t data);
	void ozmawars_port04_w(uint8_t data);
	void ozmawars_port05_w(uint8_t data);
	void ozmawars_io_map(address_map &map) ATTR_COLD;
	uint8_t m_port03 = 0;
	uint8_t m_port05 = 0;
	bool m_sound_enable = 0;
};


/*******************************************************/
/* Wing Yosaku to Donbei                               */
/*******************************************************/

class yosakdon_state : public invaders_clone_state
{
public:
	yosakdon_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_clone_state(mconfig, type, tag)
	{
	}

	void yosakdon(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	void sh_port_1_w(uint8_t data);
	void sh_port_2_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t m_port_1_last;
	uint8_t m_port_2_last;
};


/*******************************************************/
/* Omori Shuttle Invader                               */
/*******************************************************/

class shuttlei_state : public invaders_clone_state
{
public:
	shuttlei_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_clone_state(mconfig, type, tag),
		m_inputs(*this, "INPUTS"),
		m_p2(*this, "P2"),
		m_palette(*this, "palette")
	{
	}

	void shuttlei(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t port_ff_r();
	void port_ff_w(uint8_t data);
	void sh_port_1_w(uint8_t data);
	void sh_port_2_w(uint8_t data);

	uint32_t screen_update_shuttlei(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void main_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	required_ioport m_inputs;
	required_ioport m_p2;
	required_device<palette_device> m_palette;

	uint8_t m_port_1_last = 0;
};


/*******************************************************/
/* Model Racing Claybuster                             */
/*******************************************************/

class claybust_state : public invaders_state
{
public:
	claybust_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_state(mconfig, type, tag),
		m_gunx(*this, "GUNX"),
		m_guny(*this, "GUNY"),
		m_gun_on(*this, "claybust_gun")
	{
	}

	void claybust(machine_config &config);

	DECLARE_INPUT_CHANGED_MEMBER(gun_trigger);

	int gun_on_r();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t gun_lo_r();
	uint8_t gun_hi_r();
	TIMER_DEVICE_CALLBACK_MEMBER(gun_callback);

	void io_map(address_map &map) ATTR_COLD;

	required_ioport m_gunx;
	required_ioport m_guny;
	required_device<timer_device> m_gun_on;

	uint16_t m_gun_pos = 0;
};


/*******************************************************/
/* Cane (Model Racing)                                 */
/*******************************************************/

class cane_state : public mw8080bw_state
{
public:
	cane_state(machine_config const &mconfig, device_type type, char const *tag) :
		mw8080bw_state(mconfig, type, tag)
	{
	}

	void cane(machine_config &config);
	void cane_audio(machine_config &config);

protected:
	void cane_unknown_port0_w(uint8_t data);

private:
	void cane_io_map(address_map &map) ATTR_COLD;
	void cane_map(address_map &map) ATTR_COLD;
};

DISCRETE_SOUND_EXTERN( cane_discrete );


/*******************************************************/
/* Model Racing Orbite                                 */
/*******************************************************/

class orbite_state : public invaders_clone_palette_state
{
public:
	orbite_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_clone_palette_state(mconfig, type, tag),
		m_main_ram(*this, "main_ram")
	{
	}

	void orbite(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

	uint8_t orbite_scattered_colorram_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void orbite_scattered_colorram_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void orbite_io_map(address_map &map) ATTR_COLD;
	void orbite_map(address_map &map) ATTR_COLD;

	u32 screen_update_orbite(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_shared_ptr<uint8_t> m_main_ram;
	std::unique_ptr<uint8_t []> m_scattered_colorram;
};


/*******************************************************/
/* Braze Technologies Space Invaders Multigame hacks   */
/*******************************************************/

class invmulti_state : public invaders_state
{
public:
	invmulti_state(machine_config const &mconfig, device_type type, char const *tag) :
		invaders_state(mconfig, type, tag),
		m_banks(*this, "bank%u", 1U),
		m_eeprom(*this, "eeprom")
	{
	}

	void invmulti(machine_config &config);

	void init_invmulti();

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	uint8_t eeprom_r();
	void eeprom_w(uint8_t data);
	void bank_w(uint8_t data);

	void main_map(address_map &map) ATTR_COLD;

	required_memory_bank_array<2> m_banks;
	required_device<eeprom_serial_93cxx_device> m_eeprom;
};

#endif // MAME_MIDW8080_8080BW_H
