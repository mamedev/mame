// license:GPL-2.0+
// copyright-holders:Felipe Sanches

// Tone generators of the KN2400, KN6000 and KN7000 families.

#ifndef MAME_MATSUSHITA_KN_TONEGEN_H
#define MAME_MATSUSHITA_KN_TONEGEN_H

#pragma once

#include <memory>

class kn_tonegen_base_device : public device_t, public device_sound_interface
{
public:
	static constexpr feature_type unemulated_features() { return feature::SOUND; }

	// The firmware latches a register address, then writes its data; each chip
	// has its own window.
	void tg_write(int chip, uint16_t addr, uint16_t data);

protected:
	kn_tonegen_base_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int chips);

	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	// device_sound_interface implementation
	virtual void sound_stream_update(sound_stream &stream) override;

private:
	const int m_chips;
	sound_stream *m_stream;
	std::unique_ptr<uint16_t []> m_regs;
};

// KN6000/KN6500: IC213, 64 voices
class kn6000_tonegen_device : public kn_tonegen_base_device
{
public:
	kn6000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// KN2400/KN2600: one tone generator
class kn2400_tonegen_device : public kn_tonegen_base_device
{
public:
	kn2400_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

// KN7000: IC201 (main) and IC205 (sub), 64 voices each
class kn7000_tonegen_device : public kn_tonegen_base_device
{
public:
	kn7000_tonegen_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);
};

DECLARE_DEVICE_TYPE(KN6000_TONEGEN, kn6000_tonegen_device)
DECLARE_DEVICE_TYPE(KN2400_TONEGEN, kn2400_tonegen_device)
DECLARE_DEVICE_TYPE(KN7000_TONEGEN, kn7000_tonegen_device)

#endif // MAME_MATSUSHITA_KN_TONEGEN_H
