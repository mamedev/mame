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

#ifndef MAME_MACHINE_CS8221_H
#define MAME_MACHINE_CS8221_H

#pragma once



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
	cs8221_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	static void static_set_isatag(device_t &device, const char *tag);
	static void static_set_biostag(device_t &device, const char *tag);

	void map(address_map &map);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

private:

	// internal state
	//address_space *m_space;
	//uint8_t *m_isa;
	//uint8_t *m_bios;
	//uint8_t *m_ram;

	// address selection
	uint8_t m_address;
	bool m_address_valid;

	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;

	uint8_t m_registers[0x10];

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );
};


// device type definition
DECLARE_DEVICE_TYPE(CS8221, cs8221_device)

#endif // MAME_MACHINE_CS8221_H
