// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    transwarp.h

    Implementation of the Applied Engineering TransWarp accelerator

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2TRANSWARP_H
#define MAME_BUS_A2BUS_A2TRANSWARP_H

#include "a2bus.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_transwarp_device:
	public device_t,
	public device_a2bus_card_interface
{
public:
	// construction/destruction
	a2bus_transwarp_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	a2bus_transwarp_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// overrides of device_t functions
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual bool take_c800() override;

private:
	bool m_bEnabled;
	bool m_bReadA2ROM;
	bool m_bIn1MHzMode;
	emu_timer *m_timer;

	required_device<cpu_device> m_ourcpu;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_dsw1, m_dsw2;

	TIMER_CALLBACK_MEMBER(clock_adjust_tick);

	uint8_t dma_r(offs_t offset);
	void dma_w(offs_t offset, uint8_t data);

	void m65c02_mem(address_map &map) ATTR_COLD;

	void hit_slot(int slot);
	void hit_slot_joy();
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_TRANSWARP, a2bus_transwarp_device)

#endif // MAME_BUS_A2BUS_TRANSWARP_H
