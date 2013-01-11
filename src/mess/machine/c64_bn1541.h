/**********************************************************************

    SpeedDOS / Burst Nibbler 1541/1571 Parallel Cable emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __C64_BN1541__
#define __C64_BN1541__


#include "emu.h"
#include "machine/c64user.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_c64_floppy_parallel_interface

class device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	device_c64_floppy_parallel_interface(const machine_config &mconfig, device_t &device);
	virtual ~device_c64_floppy_parallel_interface();

	virtual void parallel_data_w(UINT8 data) = 0;
	virtual void parallel_strobe_w(int state) = 0;

	device_c64_floppy_parallel_interface *m_other;

protected:
	UINT8 m_parallel_data;
};


// ======================> c64_bn1541_device

class c64_bn1541_device : public device_t,
							public device_c64_user_port_interface,
							public device_c64_floppy_parallel_interface
{
public:
	// construction/destruction
	c64_bn1541_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	// device-level overrides
	virtual void device_config_complete() { m_shortname = "c64_bn1541"; }
	virtual void device_start();

	// device_c64_user_port_interface overrides
	virtual UINT8 c64_pb_r(address_space &space, offs_t offset);
	virtual void c64_pb_w(address_space &space, offs_t offset, UINT8 data);
	virtual void c64_pc2_w(int level);

	// device_c64_floppy_parallel_interface overrides
	virtual void parallel_data_w(UINT8 data);
	virtual void parallel_strobe_w(int state);
};


// device type definition
extern const device_type C64_BN1541;


#endif
