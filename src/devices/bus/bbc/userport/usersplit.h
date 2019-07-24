// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Watford Electronics User Port Splitter

**********************************************************************/


#ifndef MAME_BUS_BBC_USERPORT_USERSPLIT_H
#define MAME_BUS_BBC_USERPORT_USERSPLIT_H

#pragma once

#include "userport.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_usersplit_device

class bbc_usersplit_device :
	public device_t,
	public device_bbc_userport_interface
{
public:
	// construction/destruction
	bbc_usersplit_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_INPUT_CHANGED_MEMBER(userport_changed) { m_selected = newval; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t pb_r() override;
	virtual void pb_w(uint8_t data) override;

private:
	DECLARE_WRITE_LINE_MEMBER(cb1a_w);
	DECLARE_WRITE_LINE_MEMBER(cb2a_w);
	DECLARE_WRITE_LINE_MEMBER(cb1b_w);
	DECLARE_WRITE_LINE_MEMBER(cb2b_w);

	required_device_array<bbc_userport_slot_device, 2> m_userport;
	uint8_t m_selected;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_USERSPLIT, bbc_usersplit_device)


#endif // MAME_BUS_BBC_USERPORT_USERSPLIT_H
