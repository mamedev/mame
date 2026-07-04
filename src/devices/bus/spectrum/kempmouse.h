// license:BSD-3-Clause
// copyright-holders: Oleksandr Kovalchuk
/**********************************************************************

    Kempston Mouse Interface

**********************************************************************/

#ifndef MAME_BUS_SPECTRUM_KEMPMOUSE_H
#define MAME_BUS_SPECTRUM_KEMPMOUSE_H

#pragma once

#include "exp.h"

class spectrum_kempmouse_device :
	public device_t,
	public device_spectrum_expansion_interface
{
public:
	spectrum_kempmouse_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_start() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual uint8_t iorq_r(offs_t offset) override;

private:
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_buttons;
};


DECLARE_DEVICE_TYPE(SPECTRUM_KEMPMOUSE, spectrum_kempmouse_device)

#endif // MAME_BUS_SPECTRUM_KEMPMOUSE_H
