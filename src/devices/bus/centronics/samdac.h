// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    SAMDAC Stereo DAC for SAM Coupe

***************************************************************************/

#ifndef MAME_BUS_CENTRONICS_SAMDAC_H
#define MAME_BUS_CENTRONICS_SAMDAC_H

#pragma once

#include "ctronics.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> centronics_samdac_device

class centronics_samdac_device : public device_t, public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_samdac_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// from centronics port
	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override;
	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { update_data(0, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { update_data(1, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { update_data(2, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { update_data(3, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { update_data(4, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { update_data(5, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { update_data(6, state); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data7 ) override { update_data(7, state); }

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

private:
	void update_data(int bit, int state);

	required_device_array<dac_byte_interface, 2> m_dac;

	int m_strobe;
	uint8_t m_data[2];
};

// device type definition
DECLARE_DEVICE_TYPE(CENTRONICS_SAMDAC, centronics_samdac_device)

#endif // MAME_BUS_CENTRONICS_SAMDAC_H
