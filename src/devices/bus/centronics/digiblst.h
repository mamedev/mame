// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * digiblst.h
 *
 *  Digiblaster - a DIY printer port DAC for the Amstrad CPC
 *  Printed in the German magazine CPC Amstrad International issue 8-9/1991
 *  Uses Strobe (inverted on the CPC) for the 8th bit (CPCs only have 7-bit printer ports)
 *
 *  Code borrows from the Covox Speech Thing device.
 *
 *  Created on: 23/08/2014
 */

#ifndef DIGIBLST_H_
#define DIGIBLST_H_

#pragma once

#include "ctronics.h"
#include "sound/dac.h"

// ======================> centronics_covox_device

class centronics_digiblaster_device : public device_t,
	public device_centronics_peripheral_interface
{
public:
	// construction/destruction
	centronics_digiblaster_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_data0 ) override { if (state) m_data |= 0x01; else m_data &= ~0x01; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data1 ) override { if (state) m_data |= 0x02; else m_data &= ~0x02; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data2 ) override { if (state) m_data |= 0x04; else m_data &= ~0x04; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data3 ) override { if (state) m_data |= 0x08; else m_data &= ~0x08; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data4 ) override { if (state) m_data |= 0x10; else m_data &= ~0x10; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data5 ) override { if (state) m_data |= 0x20; else m_data &= ~0x20; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_data6 ) override { if (state) m_data |= 0x40; else m_data &= ~0x40; update_dac(); }
	virtual DECLARE_WRITE_LINE_MEMBER( input_strobe ) override { if (state) m_data &= ~0x80; else m_data |= 0x80; update_dac(); }

private:
	required_device<dac_device> m_dac;

	void update_dac();

	UINT8 m_data;
};

// device type definition
extern const device_type CENTRONICS_DIGIBLASTER;


#endif /* DIGIBLST_H_ */
