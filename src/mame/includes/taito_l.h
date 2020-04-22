// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#ifndef MAME_INCLUDES_TAITO_L_H
#define MAME_INCLUDES_TAITO_L_H

#pragma once

#include "machine/74157.h"
#include "machine/bankdev.h"
#include "machine/tc009xlvc.h"
#include "machine/timer.h"
#include "machine/upd4701.h"
#include "sound/msm5205.h"
#include "sound/2203intf.h"
#include "emupal.h"
#include "tilemap.h"


class taitol_state : public driver_device
{
public:
	taitol_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_main_cpu(*this, "maincpu")
		, m_vdp(*this, "tc0090lvc")
		, m_upd4701(*this, "upd4701")
		, m_main_prg(*this, "tc0090lvc")
	{
	}

	DECLARE_MACHINE_START(taito_l);
	DECLARE_MACHINE_RESET(taito_l);
	IRQ_CALLBACK_MEMBER(irq_callback);

	DECLARE_WRITE8_MEMBER(coin_control_w);

protected:
	/* misc */
	int m_last_irq_level;
	int m_main_high;

	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(rombankswitch_w);

	DECLARE_WRITE8_MEMBER(mcu_control_w);
	DECLARE_READ8_MEMBER(mcu_control_r);
	DECLARE_WRITE_LINE_MEMBER(screen_vblank_taitol);
	TIMER_DEVICE_CALLBACK_MEMBER(vbl_interrupt);

	void l_system_video(machine_config &config);

	void common_banks_map(address_map &map);

	virtual void state_register();
	virtual void taito_machine_reset();

	required_device<cpu_device>                       m_main_cpu;
	required_device<tc0090lvc_device>                 m_vdp;
	optional_device<upd4701_device>                   m_upd4701;
	required_memory_region                            m_main_prg;
};


class taitol_2cpu_state : public taitol_state
{
public:
	taitol_2cpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitol_state(mconfig, type, tag)
		, m_audio_cpu(*this, "audiocpu")
		, m_audio_prg(*this, "audiocpu")
		, m_audio_bnk(*this, "audiobank")
	{
	}

	DECLARE_WRITE8_MEMBER(sound_bankswitch_w);

	void kurikint(machine_config &config);
	void evilston(machine_config &config);
	void raimais(machine_config &config);

protected:
	virtual void state_register() override;
	virtual void taito_machine_reset() override;

	void evilston_2_map(address_map &map);
	void evilston_map(address_map &map);
	void kurikint_2_map(address_map &map);
	void kurikint_map(address_map &map);
	void raimais_2_map(address_map &map);
	void raimais_3_map(address_map &map);
	void raimais_map(address_map &map);

	required_device<cpu_device> m_audio_cpu;
	required_memory_region      m_audio_prg;
	optional_memory_bank        m_audio_bnk;
};


class fhawk_state : public taitol_2cpu_state
{
public:
	fhawk_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitol_2cpu_state(mconfig, type, tag)
		, m_slave_prg(*this, "slave")
		, m_slave_bnk(*this, "slavebank")
		, m_slave_rombank(0)
		, m_slave_high(0)
	{
	}

	DECLARE_WRITE8_MEMBER(slave_rombank_w);
	DECLARE_READ8_MEMBER(slave_rombank_r);
	DECLARE_WRITE8_MEMBER(portA_w);

	void fhawk(machine_config &config);

protected:
	virtual void state_register() override;
	virtual void taito_machine_reset() override;

	void fhawk_2_map(address_map &map);
	void fhawk_3_map(address_map &map);
	void fhawk_map(address_map &map);

	required_memory_region      m_slave_prg;
	required_memory_bank        m_slave_bnk;

	u8  m_slave_rombank;
	u8  m_slave_high;
};


class champwr_state : public fhawk_state
{
public:
	champwr_state(const machine_config &mconfig, device_type type, const char *tag)
		: fhawk_state(mconfig, type, tag)
		, m_msm(*this, "msm")
		, m_adpcm_rgn(*this, "adpcm")
		, m_adpcm_pos(0)
		, m_adpcm_data(-1)
	{
	}

	DECLARE_WRITE_LINE_MEMBER(msm5205_vck);

	DECLARE_WRITE8_MEMBER(msm5205_lo_w);
	DECLARE_WRITE8_MEMBER(msm5205_hi_w);
	DECLARE_WRITE8_MEMBER(msm5205_start_w);
	DECLARE_WRITE8_MEMBER(msm5205_stop_w);
	DECLARE_WRITE8_MEMBER(msm5205_volume_w);

	void champwr(machine_config &config);

protected:
	virtual void state_register() override;
	virtual void taito_machine_reset() override;

	void champwr_2_map(address_map &map);
	void champwr_3_map(address_map &map);
	void champwr_map(address_map &map);

	required_device<msm5205_device> m_msm;
	required_region_ptr<u8>         m_adpcm_rgn;

	int m_adpcm_pos;
	int m_adpcm_data;
};


class taitol_1cpu_state : public taitol_state
{
public:
	taitol_1cpu_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitol_state(mconfig, type, tag)
		, m_ymsnd(*this, "ymsnd")
		, m_mux(*this, {"dswmux", "inmux"})
	{
	}

	DECLARE_READ8_MEMBER(extport_select_and_ym2203_r);

	void init_plottinga();

	DECLARE_MACHINE_RESET(plotting);
	DECLARE_MACHINE_RESET(puzznic);
	DECLARE_MACHINE_RESET(palamed);
	DECLARE_MACHINE_RESET(cachat);

	void base(machine_config &config);
	void add_muxes(machine_config &config);
	void palamed(machine_config &config);
	void plotting(machine_config &config);
	void puzznici(machine_config &config);
	void cachat(machine_config &config);
	void puzznic(machine_config &config);

protected:
	virtual void state_register() override;
	virtual void taito_machine_reset() override;

	void cachat_map(address_map &map);
	void palamed_map(address_map &map);
	void plotting_map(address_map &map);
	void puzznic_map(address_map &map);
	void puzznici_map(address_map &map);

	required_device<ym2203_device>  m_ymsnd;
	optional_device_array<ls157_x2_device, 2> m_mux;
};


class horshoes_state : public taitol_1cpu_state
{
public:
	horshoes_state(const machine_config &mconfig, device_type type, const char *tag)
		: taitol_1cpu_state(mconfig, type, tag)
	{
	}

	void horshoes(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	void horshoes_tile_cb(u32 &code);

	DECLARE_WRITE8_MEMBER(bankg_w);
	int m_horshoes_gfxbank;

	void horshoes_map(address_map &map);
};

#endif // MAME_INCLUDES_TAITO_L_H
