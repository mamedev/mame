// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    ETI Sound Card

**********************************************************************/


#ifndef MAME_BUS_TANBUS_ETISND_H
#define MAME_BUS_TANBUS_ETISND_H

#pragma once

#include "tanbus.h"
#include "machine/6821pia.h"
#include "sound/ay8910.h"
#include "sound/dac.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class tanbus_etisnd_device : public device_t, public device_tanbus_interface
{
public:
	// construction/destruction
	tanbus_etisnd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual uint8_t read(offs_t offset, int inhrom, int inhram, int be) override;
	virtual void write(offs_t offset, uint8_t data, int inhrom, int inhram, int be) override;

private:
	required_device_array<pia6821_device, 4> m_pia;
	required_device<ay8910_device> m_ay8910;

	void pia_pb_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(TANBUS_ETISND, tanbus_etisnd_device)


#endif // MAME_BUS_TANBUS_ETISND_H
