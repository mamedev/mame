// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2swyft.h

    IAI SwyftCard

*********************************************************************/

#ifndef __A2BUS_SWYFT__
#define __A2BUS_SWYFT__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_swyft_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_swyft_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	a2bus_swyft_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual const rom_entry *device_rom_region() const;

protected:
	virtual void device_start();
	virtual void device_reset();

	virtual UINT8 read_c0nx(address_space &space, UINT8 offset);
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data);
	virtual UINT8 read_inh_rom(address_space &space, UINT16 offset);
	virtual UINT16 inh_start() { return 0xd000; }
	virtual UINT16 inh_end() { return 0xffff; }
	virtual int inh_type();

private:
	UINT8 *m_rom;
	int m_rombank;
	int m_inh_state;
};

// device type definition
extern const device_type A2BUS_SWYFT;

#endif  /* __A2BUS_SWYFT__ */
