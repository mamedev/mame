// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Commodore C2N/1530/1531 Datassette emulation

**********************************************************************/

#ifndef MAME_BUS_PET_C2N_H
#define MAME_BUS_PET_C2N_H

#pragma once

#include "cass.h"
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
	c2n_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	c2n_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_pet_datassette_port_interface overrides
	virtual int datassette_read() override;
	virtual void datassette_write(int state) override;
	virtual int datassette_sense() override;
	virtual void datassette_motor(int state) override;

	TIMER_CALLBACK_MEMBER(read_tick);

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
	c1530_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// ======================> c1531_device

class c1531_device :  public c2n_device
{
public:
	// construction/destruction
	c1531_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};


// device type definition
DECLARE_DEVICE_TYPE(C2N,   c2n_device)
DECLARE_DEVICE_TYPE(C1530, c1530_device)
DECLARE_DEVICE_TYPE(C1531, c1531_device)

#endif // MAME_BUS_PET_C2N_H
