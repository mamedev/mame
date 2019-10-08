// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Derrick Renaud
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/
#ifndef MAME_AUDIO_MW8080BW_H
#define MAME_AUDIO_MW8080BW_H

#pragma once

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"


class seawolf_audio_device : public device_t
{
public:
	seawolf_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device<samples_device> m_samples;
	u8 m_prev;
};


class gunfight_audio_device : public device_t
{
public:
	gunfight_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device_array<samples_device, 2> m_samples;
};



class gmissile_audio_device : public device_t
{
public:
	gmissile_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device_array<samples_device, 2> m_samples;
	output_finder<> m_l_exp;
	output_finder<> m_r_exp;
	u8 m_p1;
};


class m4_audio_device : public device_t
{
public:
	m4_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device_array<samples_device, 2> m_samples;
	u8 m_p1;
	u8 m_p2;
};


class clowns_audio_device : public device_t
{
public:
	auto ctrl_sel_out() { return m_ctrl_sel_out.bind(); }

	clowns_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device<samples_device> m_samples;
	required_device<discrete_sound_device> m_discrete;
	devcb_write_line m_ctrl_sel_out;
	u8 m_p1;
	u8 m_p2;
};


class spcenctr_audio_device : public device_t
{
public:
	spcenctr_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	TIMER_CALLBACK_MEMBER(strobe_callback);

	required_device<sn76477_device> m_sn;
	required_device<discrete_sound_device> m_discrete;
	output_finder<> m_lamp;
	output_finder<> m_strobe;
	emu_timer *m_strobe_timer;
	u8 m_strobe_enable;
};


class phantom2_audio_device : public device_t
{
public:
	phantom2_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	required_device<samples_device> m_samples;
	output_finder<> m_exp;
	u8 m_p1;
	u8 m_p2;
};


DECLARE_DEVICE_TYPE(SEAWOLF_AUDIO,  seawolf_audio_device)
DECLARE_DEVICE_TYPE(GUNFIGHT_AUDIO, gunfight_audio_device)
DECLARE_DEVICE_TYPE(GMISSILE_AUDIO, gmissile_audio_device)
DECLARE_DEVICE_TYPE(M4_AUDIO,       m4_audio_device)
DECLARE_DEVICE_TYPE(CLOWNS_AUDIO,   clowns_audio_device)
DECLARE_DEVICE_TYPE(SPCENCTR_AUDIO, spcenctr_audio_device)
DECLARE_DEVICE_TYPE(PHANTOM2_AUDIO, phantom2_audio_device)

#endif // MAME_AUDIO_MW8080BW_H
