// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Tormod Tjaberg, Mirko Buffoni,Lee Taylor, Valerio Verrando, Zsolt Vasvari,Derrick Renaud
// thanks-to:Michael Strutts, Marco Cassili
/***************************************************************************

    Midway 8080-based black and white hardware

****************************************************************************/
#ifndef MAME_MIDW8080_MW8080BW_A_H
#define MAME_MIDW8080_MW8080BW_A_H

#pragma once

#include "sound/discrete.h"
#include "sound/samples.h"
#include "sound/sn76477.h"
#include "machine/netlist.h"


class midway_tone_generator_device_base : public device_t
{
public:
	void tone_generator_lo_w(u8 data);
	void tone_generator_hi_w(u8 data);

protected:
	midway_tone_generator_device_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);

	required_device<discrete_sound_device> m_discrete;
};


class seawolf_audio_device : public device_t
{
public:
	seawolf_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<netlist_mame_logic_input_device> m_left_shot;
	required_device<netlist_mame_logic_input_device> m_right_shot;
	required_device<netlist_mame_logic_input_device> m_left_hit;
	required_device<netlist_mame_logic_input_device> m_right_hit;
};


class boothill_audio_device : public midway_tone_generator_device_base
{
public:
	boothill_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};


class desertgu_audio_device : public midway_tone_generator_device_base
{
public:
	auto ctrl_sel_out() { return m_ctrl_sel_out.bind(); }

	desertgu_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_ctrl_sel_out;
	output_finder<> m_recoil;
	u8 m_p2;
};


class dplay_audio_device : public midway_tone_generator_device_base
{
public:
	dplay_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};


class gmissile_audio_device : public device_t
{
public:
	gmissile_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device_array<samples_device, 2> m_samples;
	u8 m_p1;
	u8 m_p2;
};


class clowns_audio_device : public midway_tone_generator_device_base
{
public:
	auto ctrl_sel_out() { return m_ctrl_sel_out.bind(); }

	clowns_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<samples_device> m_samples;
	devcb_write_line m_ctrl_sel_out;
	u8 m_p1;
	u8 m_p2;
};


class spacwalk_audio_device : public midway_tone_generator_device_base
{
public:
	auto ctrl_sel_out() { return m_ctrl_sel_out.bind(); }

	spacwalk_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	devcb_write_line m_ctrl_sel_out;
	u8 m_p1;
};


class dogpatch_audio_device : public midway_tone_generator_device_base
{
public:
	dogpatch_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void write(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
};


class spcenctr_audio_device : public device_t
{
public:
	spcenctr_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

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
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<samples_device> m_samples;
	output_finder<> m_exp;
	u8 m_p1;
	u8 m_p2;
};


class invaders_audio_device : public device_t
{
public:
	auto flip_screen_out() { return m_flip_screen_out.bind(); }

	invaders_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<sn76477_device> m_sn;
	required_device<discrete_sound_device> m_discrete;
	devcb_write_line m_flip_screen_out;
	u8 m_p2;
};


class invad2ct_audio_device : public device_t
{
public:
	invad2ct_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	void p1_w(u8 data);
	void p2_w(u8 data);
	void p3_w(u8 data);
	void p4_w(u8 data);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	required_device<discrete_sound_device> m_discrete;
	required_device_array<sn76477_device, 2> m_sn;
};


class zzzap_common_audio_device : public device_t
{
public:
	void p1_w(u8 data);
	void p2_w(u8 data);

protected:
	zzzap_common_audio_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock, void (*netlist)(netlist::nlparse_t &));

	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	void (*m_netlist)(netlist::nlparse_t &) = nullptr;
	required_device<netlist_mame_logic_input_device> m_pedal_bit0;
	required_device<netlist_mame_logic_input_device> m_pedal_bit1;
	required_device<netlist_mame_logic_input_device> m_pedal_bit2;
	required_device<netlist_mame_logic_input_device> m_pedal_bit3;
	required_device<netlist_mame_logic_input_device> m_hi_shift;
	required_device<netlist_mame_logic_input_device> m_lo_shift;
	required_device<netlist_mame_logic_input_device> m_boom;
	required_device<netlist_mame_logic_input_device> m_engine_sound_off;
	required_device<netlist_mame_logic_input_device> m_noise_cr_1;
	required_device<netlist_mame_logic_input_device> m_noise_cr_2;
};


class zzzap_audio_device : public zzzap_common_audio_device
{
public:
	zzzap_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
};


class lagunar_audio_device : public zzzap_common_audio_device
{
public:
	lagunar_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);
};


DECLARE_DEVICE_TYPE(SEAWOLF_AUDIO,  seawolf_audio_device)
DECLARE_DEVICE_TYPE(GUNFIGHT_AUDIO, gunfight_audio_device)
DECLARE_DEVICE_TYPE(BOOTHILL_AUDIO, boothill_audio_device)
DECLARE_DEVICE_TYPE(DESERTGU_AUDIO, desertgu_audio_device)
DECLARE_DEVICE_TYPE(DPLAY_AUDIO,    dplay_audio_device)
DECLARE_DEVICE_TYPE(GMISSILE_AUDIO, gmissile_audio_device)
DECLARE_DEVICE_TYPE(M4_AUDIO,       m4_audio_device)
DECLARE_DEVICE_TYPE(CLOWNS_AUDIO,   clowns_audio_device)
DECLARE_DEVICE_TYPE(SPACWALK_AUDIO, spacwalk_audio_device)
DECLARE_DEVICE_TYPE(DOGPATCH_AUDIO, dogpatch_audio_device)
DECLARE_DEVICE_TYPE(SPCENCTR_AUDIO, spcenctr_audio_device)
DECLARE_DEVICE_TYPE(PHANTOM2_AUDIO, phantom2_audio_device)
DECLARE_DEVICE_TYPE(INVADERS_AUDIO, invaders_audio_device)
DECLARE_DEVICE_TYPE(INVAD2CT_AUDIO, invad2ct_audio_device)
DECLARE_DEVICE_TYPE(ZZZAP_AUDIO,    zzzap_audio_device)
DECLARE_DEVICE_TYPE(LAGUNAR_AUDIO,  lagunar_audio_device)

#endif // MAME_MIDW8080_MW8080BW_A_H
