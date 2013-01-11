/***************************************************************************

    Chips & Technologies CS4031 chipset

    Chipset for 486 based PC/AT compatible systems. Consists of two
    individual chips:

    * F84031
        - DRAM controller
        - ISA-bus controller
        - VESA VL-BUS controller

    * F84035
        - 2x 8257 DMA controller
        - 2x 8259 interrupt controller
        - 8254 timer
        - MC14818 RTC

***************************************************************************/

#pragma once

#ifndef __CS4031_H__
#define __CS4031_H__

#include "emu.h"


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_CS4031_ADD(_tag, _cputag, _isatag, _biostag) \
	MCFG_DEVICE_ADD(_tag, CS4031, 0) \
	cs4031_device::static_set_cputag(*device, _cputag); \
	cs4031_device::static_set_isatag(*device, _isatag); \
	cs4031_device::static_set_biostag(*device, _biostag);


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> cs4031_device

class cs4031_device : public device_t
{
public:
	// construction/destruction
	cs4031_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( address_w );
	DECLARE_READ8_MEMBER( data_r );
	DECLARE_WRITE8_MEMBER( data_w );

	// inline configuration
	static void static_set_cputag(device_t &device, const char *tag);
	static void static_set_isatag(device_t &device, const char *tag);
	static void static_set_biostag(device_t &device, const char *tag);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();

private:
	void update_read_region(int index, const char *region, offs_t start, offs_t end);
	void update_write_region(int index, const char *region, offs_t start, offs_t end);
	void update_read_regions();
	void update_write_regions();

	// internal state
	address_space *m_space;
	UINT8 *m_isa;
	UINT8 *m_bios;
	UINT8 *m_ram;

	// address selection
	UINT8 m_address;
	bool m_address_valid;

	const char *m_cputag;
	const char *m_isatag;
	const char *m_biostag;


	UINT8 m_registers[0x20];
};


// device type definition
extern const device_type CS4031;


#endif  /* __CS4031_H__ */
