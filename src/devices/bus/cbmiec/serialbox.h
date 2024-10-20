// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Serial Box 64K Serial Port Buffer emulation

**********************************************************************/

#ifndef MAME_BUS_CBMIEC_SERIALBOX_H
#define MAME_BUS_CBMIEC_SERIALBOX_H

#pragma once

#include "cbmiec.h"
#include "cpu/m6502/m65c02.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cbm_serial_box_device

class cbm_serial_box_device : public device_t, public device_cbm_iec_interface
{
public:
	// construction/destruction
	cbm_serial_box_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_cbm_iec_interface overrides
	void cbm_iec_atn(int state) override;
	void cbm_iec_data(int state) override;
	void cbm_iec_reset(int state) override;

private:
	required_device<m65c02_device> m_maincpu;

	void serial_box_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(CBM_SERIAL_BOX, cbm_serial_box_device)


#endif // MAME_BUS_CBMIEC_SERIALBOX_H
