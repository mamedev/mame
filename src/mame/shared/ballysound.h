// license:BSD-3-Clause
// copyright-holders:Mike Harris
/***************************************************************************

    bally.h

    Functions to emulate the various Bally pinball sound boards.

***************************************************************************/
#ifndef MAME_SHARED_BALLYSOUND_H
#define MAME_SHARED_BALLYSOUND_H

#pragma once


#include "cpu/m6800/m6800.h"
#include "cpu/m6800/m6801.h"
#include "cpu/m6809/m6809.h"
#include "machine/6821pia.h"
#include "machine/timer.h"
#include "sound/ay8910.h"
#include "sound/dac.h"
#include "sound/discrete.h"
#include "sound/flt_rc.h"
#include "sound/hc55516.h"
#include "sound/tms5220.h"



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE(BALLY_AS2888,           bally_as2888_device)
DECLARE_DEVICE_TYPE(BALLY_AS3022,           bally_as3022_device)
DECLARE_DEVICE_TYPE(BALLY_SOUNDS_PLUS,      bally_sounds_plus_device)
DECLARE_DEVICE_TYPE(BALLY_CHEAP_SQUEAK,     bally_cheap_squeak_device)
DECLARE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK,    bally_squawk_n_talk_device)
DECLARE_DEVICE_TYPE(BALLY_SQUAWK_N_TALK_AY, bally_squawk_n_talk_ay_device)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bally_as2888_device

class bally_as2888_device : public device_t, public device_mixer_interface
{
public:
	bally_as2888_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 0) :
		bally_as2888_device(mconfig, BALLY_AS2888, tag, owner)
	{ }

	// read/write
	void sound_select(uint8_t data);
	void sound_int(int state);

	void as2888_map(address_map &map) ATTR_COLD;

protected:
	bally_as2888_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner) :
		device_t(mconfig, type, tag, owner, 0),
		device_mixer_interface(mconfig, *this),
		m_snd_prom(*this, "sound"),
		m_discrete(*this, "discrete"),
		m_timer_s_freq(*this, "timer_s_freq"),
		m_snd_sustain_timer(*this, "timer_as2888")
	{ }

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

private:
	uint8_t m_sound_select = 0;
	uint8_t m_snd_sel = 0;
	uint8_t m_snd_tone_gen = 0;
	uint8_t m_snd_div = 1;
	required_region_ptr<uint8_t> m_snd_prom;
	required_device<discrete_sound_device> m_discrete;
	required_device<timer_device> m_timer_s_freq;
	required_device<timer_device> m_snd_sustain_timer;


	// internal communications
	TIMER_CALLBACK_MEMBER(sound_select_sync);
	TIMER_CALLBACK_MEMBER(sound_int_sync);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_s);
	TIMER_DEVICE_CALLBACK_MEMBER(timer_as2888);
};


// ======================> bally_as3022_device

class bally_as3022_device : public device_t, public device_mixer_interface
{
public:
	bally_as3022_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545) :
		bally_as3022_device(mconfig, BALLY_AS3022, tag, owner, clock)
	{ }

	// read/write
	DECLARE_INPUT_CHANGED_MEMBER(sw1);
	void sound_select(uint8_t data);
	void sound_int(int state);

	void as3022_map(address_map &map) ATTR_COLD;

protected:
	bally_as3022_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock) :
		device_t(mconfig, type, tag, owner, clock),
		device_mixer_interface(mconfig, *this),
		m_cpu(*this, "cpu"),
		m_pia(*this, "pia"),
		m_ay_filters(*this, "ay_filter%u", 0),
		m_ay(*this, "ay"),
		m_mc3417_filter(*this, "mc3417_filter"),
		m_mc3417(*this, "mc3417")
	{ }

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// devices
	// The schematics list an optional 555, but it never seemed to be used
	required_device<m6808_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia;
	required_device_array<filter_rc_device, 3> m_ay_filters;
	required_device<ay8910_device> m_ay;
	optional_device<filter_rc_device> m_mc3417_filter;
	optional_device<mc3417_device> m_mc3417;

	// overwridden by children
	void pia_portb_w(uint8_t data);

private:
	bool m_bc1 = false;
	bool m_bdir = false;
	uint8_t m_sound_select = 0;
	uint8_t m_ay_data = 0;

	// internal communications
	TIMER_CALLBACK_MEMBER(sound_select_sync);
	TIMER_CALLBACK_MEMBER(sound_int_sync);
	uint8_t pia_porta_r();
	void pia_porta_w(uint8_t data);
	void pia_cb2_w(int state);
	void pia_irq_w(int state);
	uint8_t ay_io_r();

	void update_ay_bus();
};


// ======================> bally_sounds_plus_device
class bally_sounds_plus_device : public bally_as3022_device
{
public:
	bally_sounds_plus_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545) :
		bally_as3022_device(mconfig, BALLY_SOUNDS_PLUS, tag, owner, clock)
	{ }

	void sounds_plus_map(address_map &map) ATTR_COLD;

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	// internal communications
	void vocalizer_pia_portb_w(uint8_t data);
};


// ======================> bally_cheap_squeak_device

class bally_cheap_squeak_device : public device_t, public device_mixer_interface
{
public:
	bally_cheap_squeak_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545);

	auto sound_ack_w_handler() { return m_sound_ack_w_handler.bind(); }

	// read/write
	DECLARE_INPUT_CHANGED_MEMBER(sw1);
	void sound_select(uint8_t data);
	void sound_int(int state);

	void cheap_squeak_map(address_map &map) ATTR_COLD;

protected:
	bally_cheap_squeak_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// devices
	required_device<m6803_cpu_device> m_cpu;
	required_device<dac_byte_interface> m_dac;

private:
	uint8_t m_sound_select = 0;
	bool m_sound_int = 0;
	bool m_sound_ack = 0;

	devcb_write_line m_sound_ack_w_handler;
	output_finder<1> m_leds;

	// internal communications
	TIMER_CALLBACK_MEMBER(sound_select_sync);
	TIMER_CALLBACK_MEMBER(sound_int_sync);
	void out_p1_cb(uint8_t data);
	uint8_t in_p2_cb();
	void out_p2_cb(uint8_t data);

	void update_led();
};

// ======================> bally_squawk_n_talk_device

// This board comes in different configurations, with or without the DAC and/or AY8910.

class bally_squawk_n_talk_device : public device_t, public device_mixer_interface
{
public:
	bally_squawk_n_talk_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545);

	// read/write
	DECLARE_INPUT_CHANGED_MEMBER(sw1);
	void sound_select(uint8_t data);
	void sound_int(int state);

	void squawk_n_talk_map(address_map &map) ATTR_COLD;

protected:
	bally_squawk_n_talk_device(
			const machine_config &mconfig,
			device_type type,
			const char *tag,
			device_t *owner,
			uint32_t clock);

	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// devices
	required_device<m6802_cpu_device> m_cpu;
	required_device<pia6821_device> m_pia1;
	required_device<pia6821_device> m_pia2;
	required_device<filter_rc_device> m_dac_filter;
	required_device<dac_byte_interface> m_dac;
	required_device<filter_rc_device> m_speech_filter;
	required_device<tms5200_device> m_tms5200;

	uint8_t m_sound_select = 0;

	uint8_t pia2_porta_r();

private:
	// internal communications
	TIMER_CALLBACK_MEMBER(sound_select_sync);
	TIMER_CALLBACK_MEMBER(sound_int_sync);
	void pia1_portb_w(uint8_t data);
	void pia_irq_w(int state);
};


class bally_squawk_n_talk_ay_device : public bally_squawk_n_talk_device
{
public:
	bally_squawk_n_talk_ay_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock = 3'579'545);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	uint8_t pia2_porta_r();

private:
	bool m_bc1 = false;
	bool m_bdir = false;
	uint8_t m_ay_data = 0;

	required_device_array<filter_rc_device, 3> m_ay_filters;
	required_device<ay8910_device> m_ay;

	void pia2_porta_w(uint8_t data);
	void pia2_portb_w(uint8_t data);
	void pia2_cb2_w(int state);
	uint8_t ay_io_r();

	void update_ay_bus();
};


#endif // MAME_SHARED_BALLYSOUND_H
