// license:BSD-3-Clause
// copyright-holders:Aaron Giles
/***************************************************************************

    speaker.h

    Speaker output sound device.

***************************************************************************/
#ifndef MAME_EMU_SPEAKER_H
#define MAME_EMU_SPEAKER_H

#pragma once


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

// device type definition
DECLARE_DEVICE_TYPE(SPEAKER, speaker_device)



//**************************************************************************
//  DEVICE CONFIGURATION MACROS
//**************************************************************************

// add/remove speakers
#define MCFG_SPEAKER_ADD(_tag, _x, _y, _z) \
	MCFG_DEVICE_ADD(_tag, SPEAKER, 0) \
	speaker_device::static_set_position(*device, _x, _y, _z);
#define MCFG_SPEAKER_STANDARD_MONO(_tag) \
	MCFG_SPEAKER_ADD(_tag, 0.0, 0.0, 1.0)

#define MCFG_SPEAKER_STANDARD_STEREO(_tagl, _tagr) \
	MCFG_SPEAKER_ADD(_tagl, -0.2, 0.0, 1.0) \
	MCFG_SPEAKER_ADD(_tagr, 0.2, 0.0, 1.0)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> speaker_device

class speaker_device : public device_t, public device_mixer_interface
{
public:
	// construction/destruction
	speaker_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);
	virtual ~speaker_device();

	// inline configuration helpers
	static void static_set_position(device_t &device, double x, double y, double z);

	// internally for use by the sound system
	void mix(s32 *leftmix, s32 *rightmix, int &samples_this_update, bool suppress);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// inline configuration state
	double              m_x;
	double              m_y;
	double              m_z;

	// internal state
#ifdef MAME_DEBUG
	s32                 m_max_sample;           // largest sample value we've seen
	s32                 m_clipped_samples;      // total number of clipped samples
	s32                 m_total_samples;        // total number of samples
#endif
};


// speaker device iterator
typedef device_type_iterator<speaker_device> speaker_device_iterator;


#endif  /* MAME_EMU_SPEAKER_H */
