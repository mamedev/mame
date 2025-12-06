// license:BSD-3-Clause
// copyright-holders:Hiromitsu Shioya, Olivier Galibert
/*********************************************************/
/*    SEGA 8bit PCM                                      */
/*********************************************************/

#ifndef MAMESOUND_HOHNERPCM_H
#define MAMESOUND_HOHNERPCM_H

#pragma once

#include "dirom.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class hohnerpcm_device : public device_t,
					     public device_sound_interface,
					     public device_rom_interface<16>
{
public:
	hohnerpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void write(offs_t offset, uint8_t data);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream) override;

	// device_rom_interface overrides
	virtual void rom_bank_pre_change() override;

private:
    struct Channel
    {
        uint16_t address;
        uint8_t volume;
        uint8_t active;
    };
    Channel m_channels[6];
	sound_stream* m_stream;
};

DECLARE_DEVICE_TYPE(HOHNERPCM, hohnerpcm_device)

#endif // MAMESOUND_HOHNERPCM_H
