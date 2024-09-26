// license:BSD-3-Clause
// copyright-holders:R. Belmont, O. Galibert
/*********************************************************************

    a2iwm.h

    Apple II IWM Controller Card, new floppy

*********************************************************************/

#ifndef MAME_BUS_A2BUS_A2IWM_H
#define MAME_BUS_A2BUS_A2IWM_H

#pragma once

#include "a2bus.h"
#include "imagedev/floppy.h"
#include "machine/iwm.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


class a2bus_iwm_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	// construction/destruction
	a2bus_iwm_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(u8 offset) override;
	virtual void write_c0nx(u8 offset, u8 data) override;

	required_device<iwm_device> m_iwm;
	required_device_array<floppy_connector, 2> m_floppy;

private:
	void devsel_w(u8 data);
	void phases_w(u8 data);
};

class a2bus_iwm_int_device: public a2bus_iwm_device
{
public:
	a2bus_iwm_int_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
};

class a2bus_iwm_card_device: public a2bus_iwm_device
{
public:
	a2bus_iwm_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

	virtual uint8_t read_cnxx(uint8_t offset) override;

private:
	uint8_t *m_rom;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_IWM, a2bus_iwm_int_device)
DECLARE_DEVICE_TYPE(A2BUS_IWM_CARD, a2bus_iwm_card_device)

#endif  // MAME_BUS_A2BUS_A2IWM_H
