// license:BSD-3-Clause
// copyright-holders:Curt Coder
/***************************************************************************

    DigiMAX ShortBus card

    MAX506 quad DAC, A0-A1 select the channel. The registers appear at
    $DE40-$DE47 or $DE48-$DE57 by jumper.

***************************************************************************/

#ifndef MAME_BUS_C64_SHORTBUS_DIGIMAX_H
#define MAME_BUS_C64_SHORTBUS_DIGIMAX_H

#pragma once

#include "shortbus.h"
#include "sound/dac.h"


class ide64_shortbus_digimax_device : public device_t, public device_ide64_shortbus_interface
{
public:
	ide64_shortbus_digimax_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_ide64_shortbus_interface implementation
	virtual void shortbus_w(offs_t offset, uint8_t data, int io1, int io2, int csel) override;

private:
	required_device_array<dac_8bit_r2r_device, 4> m_dac;
	required_ioport m_base;
};

DECLARE_DEVICE_TYPE(IDE64_SHORTBUS_DIGIMAX, ide64_shortbus_digimax_device)

#endif // MAME_BUS_C64_SHORTBUS_DIGIMAX_H
