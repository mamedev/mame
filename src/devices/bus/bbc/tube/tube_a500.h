// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Acorn A500 2nd processor

    http://chrisacorns.computinghistory.org.uk/8bit_Upgrades/Acorn_A5002ndProc.html

**********************************************************************/


#ifndef MAME_BUS_BBC_TUBE_A500_H
#define MAME_BUS_BBC_TUBE_A500_H

#include "tube.h"
#include "cpu/arm/arm.h"
#include "machine/tube.h"
#include "machine/input_merger.h"
#include "machine/acorn_ioc.h"
#include "machine/acorn_memc.h"
#include "machine/acorn_vidc.h"
#include "machine/archimedes_keyb.h"
#include "screen.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> bbc_tube_a500_device

class bbc_tube_a500_device :
	public device_t,
	public device_bbc_tube_interface
{
public:
	// construction/destruction
	bbc_tube_a500_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	virtual uint8_t host_r(offs_t offset) override;
	virtual void host_w(offs_t offset, uint8_t data) override;

private:
	required_device<arm_cpu_device> m_maincpu;
	required_device<acorn_ioc_device> m_ioc;
	required_device<acorn_memc_device> m_memc;
	required_device<acorn_vidc10_device> m_vidc;
	required_device<tube_device> m_ula;
	required_device<input_merger_device> m_irqs;
	required_device<input_merger_device> m_fiqs;

	void arm_mem(address_map &map);
	void a500_map(address_map &map);

	DECLARE_WRITE_LINE_MEMBER(prst_w);
};


// device type definition
DECLARE_DEVICE_TYPE(BBC_TUBE_A500, bbc_tube_a500_device)


#endif /* MAME_BUS_BBC_TUBE_A500_H */
