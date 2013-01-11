#pragma once

#ifndef __LK201_H__
#define __LK201_H__

#include "emu.h"

//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define LK201_TAG   "lk201"

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LK201_ADD() \
	MCFG_DEVICE_ADD(LK201_TAG, LK201, 0)

#define MCFG_LK201_REPLACE() \
	MCFG_DEVICE_REPLACE(LK201_TAG, LK201, 0)

#define MCFG_LK201_REMOVE() \
	MCFG_DEVICE_REMOVE(LK201_TAG)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> lk201_device

class lk201_device :  public device_t
{
public:
	// construction/destruction
	lk201_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_READ8_MEMBER( ddr_r );
	DECLARE_WRITE8_MEMBER( ddr_w );
	DECLARE_READ8_MEMBER( ports_r );
	DECLARE_WRITE8_MEMBER( ports_w );
	DECLARE_READ8_MEMBER( sci_r );
	DECLARE_WRITE8_MEMBER( sci_w );
	DECLARE_READ8_MEMBER( spi_r );
	DECLARE_WRITE8_MEMBER( spi_w );

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
	virtual void device_config_complete();
	virtual machine_config_constructor device_mconfig_additions() const;
	virtual const rom_entry *device_rom_region() const;

	required_device<cpu_device> m_maincpu;

private:
	UINT8 ddrs[3];
	UINT8 ports[3];

	void send_port(address_space &space, UINT8 offset, UINT8 data);
};

// device type definition
extern const device_type LK201;

#endif
