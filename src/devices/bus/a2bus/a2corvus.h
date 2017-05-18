// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2corvus.h

    Implementation of the Corvus flat-cable hard disk interface
    for the Apple II.

*********************************************************************/

#ifndef __A2BUS_CORVUS__
#define __A2BUS_CORVUS__

#include "a2bus.h"
#include "machine/corvushd.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_corvus_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_corvus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	a2bus_corvus_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const tiny_rom_entry *device_rom_region() const override;

	required_device<corvus_hdc_t> m_corvushd;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(address_space &space, uint8_t offset) override;
	virtual uint8_t read_c800(address_space &space, uint16_t offset) override;

private:
	uint8_t *m_rom;
};

// device type definition
extern const device_type A2BUS_CORVUS;

#endif /* __A2BUS_CORVUS__ */
