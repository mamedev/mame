// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#ifndef MAME_SOUND_MIXER_H
#define MAME_SOUND_MIXER_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

DECLARE_DEVICE_TYPE(MIXER, mixer_device)

// ======================> mixer_device

class mixer_device : public device_t, public device_mixer_interface
{
public:
	// internal constructor
	mixer_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

private:
	u8 m_dummy = 0; // needed for save-state support
};

#endif // MAME_SOUND_MIXER_H
