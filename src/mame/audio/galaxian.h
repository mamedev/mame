// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_AUDIO_GALAXIAN_H
#define MAME_AUDIO_GALAXIAN_H

#pragma once

#include "sound/discrete.h"

class galaxian_sound_device : public device_t
{
public:
	galaxian_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_WRITE8_MEMBER( sound_w );
	DECLARE_WRITE8_MEMBER( pitch_w );
	DECLARE_WRITE8_MEMBER( vol_w );
	DECLARE_WRITE8_MEMBER( noise_enable_w );
	DECLARE_WRITE8_MEMBER( background_enable_w );
	DECLARE_WRITE8_MEMBER( fire_enable_w );
	DECLARE_WRITE8_MEMBER( lfo_freq_w );

protected:
	galaxian_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_add_mconfig(machine_config &config) override;

	required_device<discrete_device> m_discrete;

private:
	// internal state
	uint8_t m_lfo_val;
};

class mooncrst_sound_device : public galaxian_sound_device
{
public:
	mooncrst_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override;
};

DECLARE_DEVICE_TYPE(GALAXIAN_SOUND, galaxian_sound_device)
DECLARE_DEVICE_TYPE(MOONCRST_SOUND, mooncrst_sound_device)

#endif // MAME_AUDIO_GALAXIAN_H
