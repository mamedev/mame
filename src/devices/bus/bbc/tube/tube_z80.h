// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC04 Z80 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC04_Z802ndproc.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_Z80_H
#define MAME_BUS_BBC_TUBE_Z80_H

#include "tube.h"
#include "cpu/z80/z80.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_z80_device

class bbc_tube_z80_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_z80_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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
	IRQ_CALLBACK_MEMBER( irq_callback );
	DECLARE_WRITE_LINE_MEMBER( nmi_w );

	required_device<cpu_device> m_z80;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	bool m_rom_enabled;

	DECLARE_READ8_MEMBER( mem_r );
	DECLARE_WRITE8_MEMBER( mem_w );
	DECLARE_READ8_MEMBER( opcode_r );

	void tube_z80_fetch(address_map &map);
	void tube_z80_io(address_map &map);
	void tube_z80_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_Z80, bbc_tube_z80_device)


#endif /* MAME_BUS_BBC_TUBE_Z80_H */
