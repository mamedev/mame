// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    ramcard16k.h

    Implemention of the Apple II 16K RAM card (aka "language card")

*********************************************************************/

#ifndef __A2BUS_RAMCARD16K__
#define __A2BUS_RAMCARD16K__

#include "emu.h"
#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_ramcard_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_ramcard_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);
	a2bus_ramcard_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	// overrides of standard a2bus slot functions
	virtual UINT8 read_c0nx(address_space &space, UINT8 offset) override;
	virtual void write_c0nx(address_space &space, UINT8 offset, UINT8 data) override;
	virtual UINT8 read_inh_rom(address_space &space, UINT16 offset) override;
	virtual void write_inh_rom(address_space &space, UINT16 offset, UINT8 data) override;
	virtual UINT16 inh_start() override { return 0xd000; }
	virtual UINT16 inh_end() override { return 0xffff; }
	virtual int inh_type() override;

private:
	void do_io(int offset);

	int m_inh_state;
	int m_last_offset;
	int m_dxxx_bank;
	UINT8 m_ram[16*1024];
};

// device type definition
extern const device_type A2BUS_RAMCARD16K;

#endif /* __A2BUS_RAMCARD16K__ */
