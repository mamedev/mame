// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Midway driving game gear shifter

    The cabinets have a four-position shifter that closes one switch per
    gear and none in neutral.  This turns gear buttons into those switches,
    as sticky or toggling buttons or sequential up/down controls, or passes
    them straight through for use with a real H-pattern shifter.

    The driver includes the midway_shifter input ports, which add the
    "Shifter Type" configuration as CONF, and declares the buttons, routing
    them here and enabling each set only for the shifter types that use it:

    1st-4th gear (gear_button, param 1-4):
        PORT_CONDITION("CONF", 0x04, EQUALS, 0x00)
    neutral (gear_button, param 0):
        PORT_CONDITION("CONF", 0x07, EQUALS, 0x01)
    shift down/up (shift_button, param 0/1):
        PORT_CONDITION("CONF", 0x04, EQUALS, 0x04)

    gears_r returns the switches with bit n set while gear n + 1 is
    engaged; the driver places them and sets their polarity.

***************************************************************************/

#ifndef MAME_WILLIAMS_MIDWAY_SHIFTER_H
#define MAME_WILLIAMS_MIDWAY_SHIFTER_H

#pragma once


class midway_shifter_device : public device_t
{
public:
	midway_shifter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// gear selected at power on, 0 for neutral
	midway_shifter_device &set_initial_gear(uint8_t gear) { m_initial_gear = gear; return *this; }

	ioport_value gears_r();

	DECLARE_INPUT_CHANGED_MEMBER(gear_button);
	DECLARE_INPUT_CHANGED_MEMBER(shift_button);

protected:
	virtual void device_start() override ATTR_COLD;

private:
	required_ioport m_conf;

	uint8_t m_initial_gear;
	uint8_t m_gear; // selected gear for the button and sequential types
	uint8_t m_held; // gear buttons held down, for the H-pattern type
};

DECLARE_DEVICE_TYPE(MIDWAY_SHIFTER, midway_shifter_device)

INPUT_PORTS_EXTERN(midway_shifter);

#endif // MAME_WILLIAMS_MIDWAY_SHIFTER_H
