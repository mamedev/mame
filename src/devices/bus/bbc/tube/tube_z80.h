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
	bbc_tube_z80_device(const machine_config& mconfig, device_type type, const char* tag, device_t* owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	IRQ_CALLBACK_MEMBER( irq_callback );

	required_device<z80_device> m_z80;
	required_device<tube_device> m_ula;
	required_memory_region m_rom;

	std::unique_ptr<uint8_t[]> m_ram;

	bool m_rom_enabled;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t opcode_r(offs_t offset);

	void tube_z80_fetch(address_map &map) ATTR_COLD;
	void tube_z80_io(address_map &map) ATTR_COLD;
	void tube_z80_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


class bbc_tube_z80w_device : public bbc_tube_z80_device
{
public:
	bbc_tube_z80w_device(const machine_config& mconfig, const char* tag, device_t* owner, uint32_t clock);

protected:
	// optional information overrides
	virtual const tiny_rom_entry* device_rom_region() const override;
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_Z80, bbc_tube_z80_device)
DECLARE_DEVICE_TYPE(BBC_TUBE_Z80W, bbc_tube_z80w_device)


#endif /* MAME_BUS_BBC_TUBE_Z80_H */
