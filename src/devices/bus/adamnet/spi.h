// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Coleco Adam Serial/Parallel Interface emulation

**********************************************************************/

#pragma once

#ifndef __ADAM_SPI__
#define __ADAM_SPI__

#include "emu.h"
#include "adamnet.h"
#include "bus/centronics/ctronics.h"
#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6800.h"
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
	adam_spi_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const override;
	virtual machine_config_constructor device_mconfig_additions() const override;

	// not really public
	DECLARE_READ8_MEMBER( p2_r );
	DECLARE_WRITE8_MEMBER( p2_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// device_adamnet_card_interface overrides
	virtual void adamnet_reset_w(int state) override;

	required_device<cpu_device> m_maincpu;
};


// device type definition
extern const device_type ADAM_SPI;



#endif
