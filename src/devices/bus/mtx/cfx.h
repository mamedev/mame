// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Memotech Compact Flash System

**********************************************************************/


#ifndef MAME_BUS_MTX_CFX_H
#define MAME_BUS_MTX_CFX_H

#include "exp.h"
#include "machine/i8255.h"
#include "bus/ata/ataintf.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class mtx_cfx_device : public device_t, public device_mtx_exp_interface
{
public:
	// construction/destruction
	mtx_cfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

private:
	required_memory_region m_flash;
	required_device<i8255_device> m_pia;
	required_device<ata_interface_device> m_ide;

	uint16_t m_ide_data;

	void portc_w(uint8_t data);
};


// device type definition
DECLARE_DEVICE_TYPE(MTX_CFX, mtx_cfx_device)

#endif // MAME_BUS_MTX_CFX_H
