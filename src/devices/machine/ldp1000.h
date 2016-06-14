// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

	Sony LDP-1000 laserdisc emulation.

***************************************************************************/

#pragma once

#ifndef __LDP1000DEV_H__
#define __LDP1000DEV_H__

#include "laserdsc.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_LASERDISC_LDP1000_ADD(_tag) \
	MCFG_DEVICE_ADD(_tag, SONY_LDP1000, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// device type definition
extern const device_type SONY_LDP1000;

// ======================> sony_ldp1000_device

class sony_ldp1000_device : public laserdisc_device
{
public:
	// construction/destruction
	sony_ldp1000_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// I/O operations
	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( read );

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual const rom_entry *device_rom_region() const override;
	
	virtual void player_vsync(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual INT32 player_update(const vbi_metadata &vbi, int fieldnum, const attotime &curtime) override;
	virtual void player_overlay(bitmap_yuy16 &bitmap) override { }

};






//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************



#endif
