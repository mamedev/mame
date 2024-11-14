// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Taito Yoke + Throttle Flight device

***************************************************************************/
#ifndef MAME_TAITO_TAITOIO_YOKE_H
#define MAME_TAITO_TAITOIO_YOKE_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class taitoio_yoke_device : public device_t
{
public:
	// construction/destruction
	taitoio_yoke_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// I/O operations
	u16 throttle_r();
	u16 stickx_r();
	u16 sticky_r();

	int slot_up_r();
	int slot_down_r();
	int handle_left_r();
	int handle_right_r();
	int handle_up_r();
	int handle_down_r();

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device-level overrides
//  virtual void device_validity_check(validity_checker &valid) const;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_ioport m_stick_x;
	required_ioport m_stick_y;
	required_ioport m_throttle;
};


// device type definition
DECLARE_DEVICE_TYPE(TAITOIO_YOKE, taitoio_yoke_device)

#endif // MAME_TAITO_TAITOIO_YOKE_H
