// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Namco 6-speed Gearbox device

***************************************************************************/
#ifndef MAME_NAMCO_NAMCOIO_GEARBOX_H
#define MAME_NAMCO_NAMCOIO_GEARBOX_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> namcoio_gearbox_device

class namcoio_gearbox_device : public device_t
{
public:
	// construction/destruction
	namcoio_gearbox_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	ioport_value in_r();
	int clutch_r();
	uint8_t m_gearbox_state = 0;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

protected:
	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(NAMCOIO_GEARBOX, namcoio_gearbox_device)

#endif // MAME_NAMCO_NAMCOIO_GEARBOX_H
