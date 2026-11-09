// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    MP3@64 MPEG audio decoder card

    MAS3507D behind a CPLD that provides the parallel data port and a
    bit-banged I2C interface.

    4  (W) MP3 data port
    4  (R) status, bit 7 set when the decoder wants more data
    5  (R/W) I2C SDA, bit 0
    6  (R/W) I2C SCL, bit 0

***************************************************************************/

#ifndef MAME_BUS_AMIGA_CLOCKPORT_MP3AT64_H
#define MAME_BUS_AMIGA_CLOCKPORT_MP3AT64_H

#pragma once

#include "clockport.h"
#include "sound/mas3507d.h"


class amiga_clockport_mp3at64_device : public device_t, public device_amiga_clockport_interface
{
public:
	amiga_clockport_mp3at64_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_amiga_clockport_interface implementation
	virtual uint8_t spare_r(offs_t offset) override;
	virtual void spare_w(offs_t offset, uint8_t data) override;

private:
	void demand_w(int state);

	required_device<mas3507d_device> m_mas;

	bool m_demand;
};

DECLARE_DEVICE_TYPE(AMIGA_CLOCKPORT_MP3AT64, amiga_clockport_mp3at64_device)

#endif // MAME_BUS_AMIGA_CLOCKPORT_MP3AT64_H
