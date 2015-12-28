// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2thunderclock.h

    Implemention of the Thunderware Thunderclock Plus.

*********************************************************************/

#ifndef __A2BUS_THUNDERCLOCK__
#define __A2BUS_THUNDERCLOCK__

#include "emu.h"
#include "a2bus.h"
#include "machine/upd1990a.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_thunderclock_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_thunderclock_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_thunderclock_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual const rom_entry *device_rom_region() const override;

	DECLARE_WRITE_LINE_MEMBER( upd_dataout_w );

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_cnxx(address_space &space, UINT8 offset) override;
	virtual UINT8 read_c800(address_space &space, UINT16 offset) override;

	required_device<upd1990a_device> m_upd1990ac;

private:
	UINT8 *m_rom;
	int m_dataout;
};

// device type definition
extern const device_type A2BUS_THUNDERCLOCK;

#endif /* __A2BUS_THUNDERCLOCK__ */
