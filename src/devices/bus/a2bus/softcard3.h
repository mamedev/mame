// license:BSD-3-Clause
// copyright-holders:Rob Justice, R. Belmont
/*********************************************************************

    softcard3.h

    Implementation of the Microsoft SoftCard /// Z-80 card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_SOFTCARD3_H
#define MAME_BUS_A2BUS_SOFTCARD3_H

#include "a2bus.h"
#include "machine/timer.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_softcard3_device:
	public device_t,
	public device_a2bus_card_interface
{
public:

	// construction/destruction
	a2bus_softcard3_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	void z80_io_w(offs_t offset, uint8_t data);

protected:
	a2bus_softcard3_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual bool take_c800() override;
	virtual u8 read_inh_rom(u16 offset) override;
	virtual bool inh_check(uint16_t offset, bool bIsWrite) override;
	TIMER_DEVICE_CALLBACK_MEMBER(timercallback);

private:
	required_device<cpu_device> m_z80;
	required_region_ptr<u8> m_prom;
	required_device<timer_device> m_timer;
	bool m_bEnabled;
	bool m_reset;
	bool m_enable_fffx;

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	void z80_io(address_map &map) ATTR_COLD;
	void z80_mem(address_map &map) ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SOFTCARD3, a2bus_softcard3_device)

#endif // MAME_BUS_A2BUS_SOFTCARD3_H
