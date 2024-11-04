// license:BSD-3-Clause
// copyright-holders:R. Belmont
/*********************************************************************

    sider.h

    Implementation of the First Class Peripherals / Advanced Tech Services / Xebec Sider SASI Card

*********************************************************************/

#ifndef MAME_BUS_A2BUS_SIDER_H
#define MAME_BUS_A2BUS_SIDER_H

#pragma once

#include "a2bus.h"
#include "bus/nscsi/devices.h"
#include "machine/nscsi_bus.h"
#include "machine/nscsi_cb.h"

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class a2bus_sider_device:
	public device_t,
	public device_a2bus_card_interface
{
protected:
	a2bus_sider_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	// overrides of standard a2bus slot functions
	virtual uint8_t read_c0nx(uint8_t offset) override;
	virtual void write_c0nx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_cnxx(uint8_t offset) override;
	virtual void write_cnxx(uint8_t offset, uint8_t data) override;
	virtual uint8_t read_c800(uint16_t offset) override;
	virtual void write_c800(uint16_t offset, uint8_t data) override;

	required_device<nscsi_bus_device> m_sasibus;
	required_device<nscsi_callback_device> m_sasi;
	required_region_ptr<u8> m_rom;

private:
	u8 m_latch;
	u8 m_control;
};

class a2bus_sider2card_device: public a2bus_sider_device
{
public:
	a2bus_sider2card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

class a2bus_sider1card_device: public a2bus_sider_device
{
public:
	a2bus_sider1card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
};

// device type definition
DECLARE_DEVICE_TYPE(A2BUS_SIDER2, a2bus_sider2card_device)
DECLARE_DEVICE_TYPE(A2BUS_SIDER1, a2bus_sider1card_device)

#endif // MAME_BUS_A2BUS_SIDER_H
