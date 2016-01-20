// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2corvus.h

    Implementation of the Corvus flat-cable hard disk interface
    for the Apple II.

*********************************************************************/

#ifndef __A2BUS_CORVUS__
#define __A2BUS_CORVUS__

#include "emu.h"
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
	a2bus_corvus_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_corvus_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	required_device<corvus_hdc_t> m_corvushd;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;

private:
	UINT8 *m_rom;
};

// device type definition
extern const device_type A2BUS_CORVUS;

#endif /* __A2BUS_CORVUS__ */
