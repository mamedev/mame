// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Diag264 Cassette Loop Back Connector emulation

**********************************************************************/

#pragma once

#ifndef __DIAG264_CASSETTE_LOOPBACK__
#define __DIAG264_CASSETTE_LOOPBACK__

#include "emu.h"
#include "cass.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> diag264_cassette_loopback_device

class diag264_cassette_loopback_device :  public device_t,
											public device_pet_datassette_port_interface
{
public:
	// construction/destruction
	diag264_cassette_loopback_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_pet_datassette_port_interface overrides
	virtual int datassette_read() override;
	virtual void datassette_write(int state) override;
	virtual int datassette_sense() override;
	virtual void datassette_motor(int state) override;

private:
	int m_read;
	int m_sense;
};


// device type definition
extern const device_type DIAG264_CASSETTE_LOOPBACK;



#endif
