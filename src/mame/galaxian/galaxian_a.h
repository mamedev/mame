// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_GALAXIAN_GALAXIAN_A_H
#define MAME_GALAXIAN_GALAXIAN_A_H

#pragma once

#include "sound/discrete.h"

class galaxian_sound_device : public device_t
{
public:
	galaxian_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void pitch_w(uint8_t data);
	void vol_w(offs_t offset, uint8_t data);
	void noise_enable_w(uint8_t data);
	void background_enable_w(offs_t offset, uint8_t data);
	void fire_enable_w(uint8_t data);
	void lfo_freq_w(offs_t offset, uint8_t data);
	void sound_w(offs_t offset, uint8_t data);

protected:
	galaxian_sound_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	required_device<discrete_device> m_discrete;

private:
	// internal state
	uint8_t m_lfo_val = 0;
};

class mooncrst_sound_device : public galaxian_sound_device
{
public:
	mooncrst_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class bongo_sound_device : public galaxian_sound_device
{
public:
	bongo_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

class sbhoei_sound_device : public galaxian_sound_device
{
public:
	sbhoei_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void noise1_enable_w(offs_t offset, uint8_t data);
	void noise2_enable_w(offs_t offset, uint8_t data);
	void noise3_enable_w(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
};

DECLARE_DEVICE_TYPE(GALAXIAN_SOUND, galaxian_sound_device)
DECLARE_DEVICE_TYPE(MOONCRST_SOUND, mooncrst_sound_device)
DECLARE_DEVICE_TYPE(SBHOEI_SOUND, sbhoei_sound_device)
DECLARE_DEVICE_TYPE(BONGO_SOUND, bongo_sound_device)

#endif // MAME_GALAXIAN_GALAXIAN_A_H
