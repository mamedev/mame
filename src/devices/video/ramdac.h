// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Generic Palette RAMDAC device

***************************************************************************/

#pragma once

#ifndef __ramdacDEV_H__
#define __ramdacDEV_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_RAMDAC_ADD(_tag, _map, _palette_tag) \
	MCFG_DEVICE_ADD(_tag, RAMDAC, 0) \
	MCFG_DEVICE_ADDRESS_MAP(AS_0, _map) \
	ramdac_device::static_set_palette_tag(*device, "^" _palette_tag);

#define MCFG_RAMDAC_SPLIT_READ(_split) \
	ramdac_device::set_split_read(*device, _split);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> ramdac_device

class ramdac_device :   public device_t,
						public device_memory_interface
{
public:
	// construction/destruction
	ramdac_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// static configuration
	static void static_set_palette_tag(device_t &device, const char *tag);
	static void set_split_read(device_t &device, int split) { downcast<ramdac_device &>(device).m_split_read_reg = split; }

	// I/O operations
	DECLARE_READ8_MEMBER( index_r );
	DECLARE_READ8_MEMBER( pal_r );
	DECLARE_WRITE8_MEMBER( index_w );
	DECLARE_WRITE8_MEMBER( index_r_w );
	DECLARE_WRITE8_MEMBER( pal_w );
	DECLARE_WRITE8_MEMBER( mask_w );

	DECLARE_READ8_MEMBER( ramdac_pal_r );
	DECLARE_WRITE8_MEMBER( ramdac_rgb666_w );
	DECLARE_WRITE8_MEMBER( ramdac_rgb888_w );

	virtual const address_space_config *memory_space_config(address_spacenum spacenum = AS_0) const override;

protected:
	// device-level overrides
	virtual void device_validity_check(validity_checker &valid) const override;
	virtual void device_start() override;
	virtual void device_reset() override;
	inline UINT8 readbyte(offs_t address);
	inline void writebyte(offs_t address, UINT8 data);
	inline void reg_increment(UINT8 inc_type);

private:
	UINT8 m_pal_index[2];
	UINT8 m_pal_mask;
	UINT8 m_int_index[2];
	std::unique_ptr<UINT8[]> m_palram;

	const address_space_config      m_space_config;
	required_device<palette_device> m_palette;

	UINT8 m_split_read_reg; // read register index is separated, seen in rltennis
};


// device type definition
extern const device_type RAMDAC;


#endif
