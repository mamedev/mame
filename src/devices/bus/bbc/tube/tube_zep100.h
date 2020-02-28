// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Torch Z80 Communicator (ZEP100)

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Torch_Z802ndproc.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_ZEP100_H
#define MAME_BUS_BBC_TUBE_ZEP100_H

#include "tube.h"
#include "cpu/z80/z80.h"
#include "machine/6522via.h"
#include "machine/i8255.h"
#include "machine/ram.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_zep100_device

class bbc_tube_zep100_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_zep100_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	bbc_tube_zep100_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

	bool m_rom_enabled;

private:
	required_device<z80_device> m_z80;
	required_device<via6522_device> m_via;
	required_device<i8255_device> m_ppi;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	uint8_t m_port_b;

	uint8_t mem_r(offs_t offset);
	void mem_w(offs_t offset, uint8_t data);
	uint8_t io_r(offs_t offset);
	void io_w(offs_t offset, uint8_t data);

	void via_pb_w(uint8_t data);
	uint8_t ppi_pb_r();
	void ppi_pc_w(uint8_t data);

	void tube_zep100_io(address_map &map);
	void tube_zep100_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_ZEP100, bbc_tube_zep100_device)


#endif /* MAME_BUS_BBC_TUBE_ZEP100_H */
