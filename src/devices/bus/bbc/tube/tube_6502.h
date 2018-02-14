// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC01 6502 2nd Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC01_65022ndproc.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_6502_H
#define MAME_BUS_BBC_TUBE_6502_H

#include "tube.h"
#include "cpu/m6502/m65c02.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_6502_device

class bbc_tube_6502_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_6502_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	required_device<cpu_device> m_m6502;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	bool m_rom_enabled;

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

	void tube_6502_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_6502, bbc_tube_6502_device)


#endif /* MAME_BUS_BBC_TUBE_6502_H */
