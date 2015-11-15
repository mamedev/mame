// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore C2N/1530/1531 Datassette emulation

**********************************************************************/

#pragma once

#ifndef __C2N__
#define __C2N__

#include "emu.h"
#include "cass.h"
#include "formats/cbm_tap.h"
#include "imagedev/cassette.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c2n_device

class c2n_device :  public device_t,
					public device_pet_datassette_port_interface
{
public:
	// construction/destruction
	c2n_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	c2n_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

	// device_pet_datassette_port_interface overrides
	virtual int datassette_read();
	virtual void datassette_write(int state);
	virtual int datassette_sense();
	virtual void datassette_motor(int state);

private:
	required_device<cassette_image_device> m_cassette;

	bool m_motor;

	// timers
	emu_timer *m_read_timer;
};


// ======================> c1530_device

class c1530_device :  public c2n_device
{
public:
	// construction/destruction
	c1530_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// ======================> c1531_device

class c1531_device :  public c2n_device
{
public:
	// construction/destruction
	c1531_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
};


// device type definition
extern const device_type C2N;
extern const device_type C1530;
extern const device_type C1531;



#endif
