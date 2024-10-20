// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    CMS 6809 2nd Processor

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_CMS6809_H
#define MAME_BUS_BBC_TUBE_CMS6809_H

#include "tube.h"
#include "cpu/m6809/m6809.h"
#include "machine/6522via.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_cms6809_device

class bbc_tube_cms6809_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_cms6809_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<cpu_device> m_maincpu;
	required_device_array<via6522_device, 2> m_via;

	void tube_cms6809_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_CMS6809, bbc_tube_cms6809_device)


#endif /* MAME_BUS_BBC_TUBE_CMS6809_H */
