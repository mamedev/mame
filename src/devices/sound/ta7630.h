// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    TA7630P

***************************************************************************/

#ifndef MAME_SOUND_TA7630_H
#define MAME_SOUND_TA7630_H

#pragma once

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ta7630_device

class ta7630_device : public device_t
	/*, public device_sound_interface*/
{
public:
	// construction/destruction
	ta7630_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// filter setters
	void set_device_volume(device_sound_interface *device,uint8_t value);
	void set_channel_volume(device_sound_interface *device, uint8_t ch,uint8_t value);

protected:
	// device-level overrides
	//virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;

private:
	double m_vol_ctrl[16]; // table for volume gains
};


// device type definition
DECLARE_DEVICE_TYPE(TA7630, ta7630_device)



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************


#endif // MAME_SOUND_TA7630_H
