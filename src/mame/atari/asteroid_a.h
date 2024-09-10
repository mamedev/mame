// license:BSD-3-Clause
// copyright-holders:K.Wilkins
/***********************************************************************

    Lunar Lander / Asteroids audio hardware

***********************************************************************/
#ifndef MAME_ATARI_ASTEROID_AUDIO_H
#define MAME_ATARI_ASTEROID_AUDIO_H

#include "machine/74259.h"
#include "sound/discrete.h"
#include "sound/pokey.h"

#pragma once


/*************************************
 *
 *  Audio hardware
 *  (Lunar Lander)
 *
 *************************************/
DECLARE_DEVICE_TYPE(LLANDER_AUDIO, llander_audio_device)

class llander_audio_device
	: public device_t
	, public device_mixer_interface
{
public:
	// construction/destruction
	llander_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// memory handlers
	void sound_w(u8 data);
	void noise_reset_w(u8 data);

protected:
	// device_t implementation
	void virtual device_add_mconfig(machine_config &config) override;
	void virtual device_start() override { save_item(NAME(m_dummy)); }

	// devices, etc.
	required_device<discrete_device> m_discrete;

private:
	// HACK: bypass savestate requirements
	bool m_dummy = false;
};


/*************************************
 *
 *  Audio hardware
 *  (Asteroids)
 *
 *************************************/
DECLARE_DEVICE_TYPE(ASTEROID_AUDIO, asteroid_audio_device)

class asteroid_audio_device
	: public device_t
	, public device_mixer_interface
{
public:
	// construction/destruction
	asteroid_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock = 0);

	// memory handlers
	void sound_w(offs_t offset, u8 data);
	void noise_reset_w(u8 data);
	void explode_w(u8 data);
	void thump_w(u8 data);

protected:
	// construction/destruction
	asteroid_audio_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock = 0);

	// device_t implementation
	void virtual device_add_mconfig(machine_config &config) override;
	void virtual device_start() override { save_item(NAME(m_dummy)); }

	// devices, etc.
	required_device<discrete_device> m_discrete;
	optional_device<ls259_device>    m_latch;

private:
	// HACK: bypass savestate requirements
	bool m_dummy = false;
};


/*************************************
 *
 *  Audio hardware
 *  (Asteroids Deluxe)
 *
 *************************************/
DECLARE_DEVICE_TYPE(ASTDELUX_AUDIO, astdelux_audio_device)

class astdelux_audio_device
	: public asteroid_audio_device
{
public:
	// construction/destruction
	astdelux_audio_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// device callbacks
	auto dsw() { return m_dswcb.bind(); }

	// line handlers
	void thrust_w(int state);

	// POKEY trampolines
	u8 pokey_r(offs_t offset) { return m_pokey->read(offset); }
	void pokey_w(offs_t offset, u8 data) { m_pokey->write(offset, data); }

protected:
	// device_t implementation
	void virtual device_add_mconfig(machine_config &config) override;

	// devices, etc.
	required_device<pokey_device> m_pokey;
	devcb_read8                   m_dswcb;

private:
	// not used here
	using asteroid_audio_device::sound_w;
	using asteroid_audio_device::thump_w;
};


#endif // MAME_ATARI_ASTEROID_AUDIO_H
