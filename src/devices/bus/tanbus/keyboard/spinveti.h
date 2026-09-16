// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ETI Space Invasion Key Unit

*********************************************************************/

#ifndef MAME_BUS_TANBUS_KEYBOARD_SPINVETI_H
#define MAME_BUS_TANBUS_KEYBOARD_SPINVETI_H

#pragma once

#include "keyboard.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class microtan_kbd_spinveti : public device_t, public device_microtan_kbd_interface
{
public:
	// construction/destruction
	microtan_kbd_spinveti(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(trigger_reset);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t read() override;
	virtual void write(uint8_t data) override;

private:
	required_ioport m_pb;
	required_ioport m_sw;
};


// device type definition
DECLARE_DEVICE_TYPE(MICROTAN_KBD_SPINVETI, microtan_kbd_spinveti)


#endif // MAME_BUS_TANBUS_KEYBOARD_SPINVETI_H
