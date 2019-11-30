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
#include "machine/mc2661.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> adam_spi_device

class adam_spi_device :  public device_t,
							public device_adamnet_card_interface
{
public:
	// construction/destruction
	adam_spi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

private:
	required_device<m6801_cpu_device> m_maincpu;

	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );

	void adam_spi_mem(address_map &map);
};


// device type definition
DECLARE_DEVICE_TYPE(ADAM_SPI, adam_spi_device)

#endif // MAME_BUS_ADAMNET_SPI_H
