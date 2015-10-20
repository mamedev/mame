// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2dx1.h

    Implementation of the Decillionix DX-1 sampler card

*********************************************************************/

#ifndef __A2BUS_DX1__
#define __A2BUS_DX1__

#include "emu.h"
#include "a2bus.h"
#include "sound/dac.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_dx1_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_dx1_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_dx1_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const;

	required_device<dac_device> m_dac;

protected:
	virtual void device_start();
	virtual void device_reset();

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
	virtual bool take_c800();

private:
	UINT8 m_volume, m_lastdac;
};

// device type definition
extern const device_type A2BUS_DX1;

#endif /* __A2BUS_DX1__ */
