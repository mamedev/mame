// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn 80286 2nd Processor

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_80286_H
#define MAME_BUS_BBC_TUBE_80286_H

#include "tube.h"
#include "cpu/i86/i286.h"
#include "machine/ram.h"
#include "machine/tube.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_80286_device

class bbc_tube_80286_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_80286_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
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

	required_device<i80286_cpu_device> m_i80286;
	required_device<tube_device> m_ula;
	required_device<ram_device> m_ram;
	required_memory_region m_bootstrap;

	uint8_t m_irq_latch;
	uint8_t disable_boot_rom();

	void tube_80286_io(address_map &map) ATTR_COLD;
	void tube_80286_mem(address_map &map) ATTR_COLD;

	void prst_w(int state);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_80286, bbc_tube_80286_device)


#endif /* MAME_BUS_BBC_TUBE_80286_H */
