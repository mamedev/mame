// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Serial/Parallel Interface emulation

**********************************************************************/

#ifndef MAME_BUS_ADAMNET_SPI_H
#define MAME_BUS_ADAMNET_SPI_H

#pragma once

#include "adamnet.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6801.h"
#include "machine/scn_pci.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_spi_device

class adam_spi_device :  public device_t,
							public device_adamnet_card_interface
{
public:
	static constexpr flags_type emulation_flags() { return flags::NOT_WORKING; }

	// construction/destruction
	adam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6803_cpu_device> m_maincpu;
	required_device<scn2661a_device> m_epci;
	required_device<rs232_port_device> m_rs232;
	required_device<centronics_device> m_centronics;

	uint8_t m_busy;

	void busy_w(int state);
	uint8_t p1_r();
	void p1_w(uint8_t data);
	uint8_t p2_r();
	void p2_w(uint8_t data);

	void adam_spi_mem(address_map &map) ATTR_COLD;
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_SPI, adam_spi_device)

#endif // MAME_BUS_ADAMNET_SPI_H
