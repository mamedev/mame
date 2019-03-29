// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn ANC05 32016 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC05_320162ndproc.html

    Acorn ANC06 Cambridge Co-Processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_ANC06_CamCoPro.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_32016_H
#define MAME_BUS_BBC_TUBE_32016_H

#include "tube.h"
#include "cpu/ns32000/ns32000.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_32016_device

class bbc_tube_32016_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_32016_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<ns32016_cpu_device> m_maincpu;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_rom;

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);

	void tube_32016_mem(address_map &map);

	bool m_rom_enabled;
};



// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_32016, bbc_tube_32016_device)


#endif /* MAME_BUS_BBC_TUBE_32016_H */
