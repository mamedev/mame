// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    laser128.h

    Helper to implement the Laser 128's built-in slot peripherals

*********************************************************************/

#ifndef __A2BUS_LASER128__
#define __A2BUS_LASER128__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_laser128_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_laser128_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_laser128_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;
	virtual void write_c800(address_space &space, UINT16 offset, UINT8 data) override;
	virtual bool take_c800() override;

private:
	UINT8 *m_rom;
	UINT8 m_slot7_ram[0x800];
	int m_slot7_bank, m_slot7_ram_bank;
};

// device type definition
extern const device_type A2BUS_LASER128;

#endif /* __A2BUS_LASER128__ */
