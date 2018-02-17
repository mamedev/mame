// license:GPL-2.0+
// copyright-holders:Brandon Munger
/**********************************************************************

    ROLM 9751 9005 System Monitor Input/Ouput Card emulation

**********************************************************************/

#ifndef MAME_MACHINE_SMIOC_H
#define MAME_MACHINE_SMIOC_H

#pragma once

#include "cpu/i86/i186.h"
#include "machine/am9517a.h"
#include "bus/rs232/rs232.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> smioc_device

class smioc_device : public device_t
{
public:
	/* Constructor and Destructor */
	smioc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	/* Device-level overrides */
	virtual void device_start() override;
	virtual void device_reset() override;
	/* Optional information overrides */
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;

private:
	/* Attached devices */
	required_device<cpu_device> m_smioccpu;

	required_device<am9517a_device> m_dma8237_1;
	required_device<am9517a_device> m_dma8237_2;
	required_device<am9517a_device> m_dma8237_3;
	required_device<am9517a_device> m_dma8237_4;
	required_device<am9517a_device> m_dma8237_5;

	required_device<rs232_port_device> m_rs232_p1;
	required_device<rs232_port_device> m_rs232_p2;
	required_device<rs232_port_device> m_rs232_p3;
	required_device<rs232_port_device> m_rs232_p4;
	required_device<rs232_port_device> m_rs232_p5;
	required_device<rs232_port_device> m_rs232_p6;
	required_device<rs232_port_device> m_rs232_p7;
	required_device<rs232_port_device> m_rs232_p8;

	required_shared_ptr<uint8_t> m_smioc_ram;

	void smioc_mem(address_map &map);
};

/* Device type */
DECLARE_DEVICE_TYPE(SMIOC, smioc_device)

#endif // MAME_MACHINE_SMIOC_H
