// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Casper 68000 2nd Processor

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_CASPER_H
#define MAME_BUS_BBC_TUBE_CASPER_H

#include "tube.h"
#include "cpu/m68000/m68000.h"
#include "machine/6522via.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_casper_device

class bbc_tube_casper_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_casper_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual DECLARE_READ8_MEMBER( host_r ) override;
	virtual DECLARE_WRITE8_MEMBER( host_w ) override;

private:
	required_device<cpu_device> m_m68000;
	required_device<via6522_device> m_via6522_0;
	required_device<via6522_device> m_via6522_1;
	required_memory_region m_casper_rom;
	required_memory_region m_host_rom;

	void tube_casper_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_CASPER, bbc_tube_casper_device)


#endif /* MAME_BUS_BBC_TUBE_CASPER_H */
