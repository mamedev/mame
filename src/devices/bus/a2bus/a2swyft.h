// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    a2swyft.h

    IAI SwyftCard

*********************************************************************/

#ifndef __A2BUS_SWYFT__
#define __A2BUS_SWYFT__

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
	a2bus_swyft_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	a2bus_swyft_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);

	virtual const tiny_rom_entry *device_rom_region() const override;

protected:
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual uint8_t read_c0nx(address_space &space, uint8_t offset) override;
	virtual void write_c0nx(address_space &space, uint8_t offset, uint8_t data) override;
	virtual uint8_t read_inh_rom(address_space &space, uint16_t offset) override;
	virtual uint16_t inh_start() override { return 0xd000; }
	virtual uint16_t inh_end() override { return 0xffff; }
	virtual int inh_type() override;

private:
	uint8_t *m_rom;
	int m_rombank;
	int m_inh_state;
};

// device type definition
extern const device_type A2BUS_SWYFT;

#endif  /* __A2BUS_SWYFT__ */
