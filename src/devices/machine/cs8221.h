// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Chips & Technologies CS8221 chipset

    a.k.a. NEW ENHANCED AT (NEAT)

    Consists of four individual chips:

    * 82C211 - CPU/Bus controller
    * 82C212 - Page/Interleave and EMS Memory controller
    * 82C215 - Data/Address buffer
    * 82C206 - Integrated Peripherals Controller(IPC)

***************************************************************************/

#pragma once

#ifndef __CS8221_H__
#define __CS8221_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CS8221_ADD(_tag, _cputag, _isatag, _biostag) \
	MCFG_DEVICE_ADD(_tag, CS8221, 0) \
	cs8221_device::static_set_cputag(*device, _cputag); \
	cs8221_device::static_set_isatag(*device, _isatag); \
	cs8221_device::static_set_biostag(*device, _biostag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cs8221_device

class cs8221_device : public device_t
{
public:
	// construction/destruction
	cs8221_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	static void static_set_isatag(device_t &device, const char *tag);
	static void static_set_biostag(device_t &device, const char *tag);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	// internal state
	//address_space *m_space;
	//UINT8 *m_isa;
	//UINT8 *m_bios;
	//UINT8 *m_ram;

	// address selection
	UINT8 m_address;
	bool m_address_valid;

	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;


	UINT8 m_registers[0x10];
};


// device type definition
extern const device_type CS8221;


#endif  /* __CS8221_H__ */
