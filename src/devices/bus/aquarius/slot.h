// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    Aquarius Cartridge Port emulation

**********************************************************************/

#ifndef MAME_BUS_AQUARIUS_SLOT_H
#define MAME_BUS_AQUARIUS_SLOT_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> aquarius_cartridge_slot_device

class device_aquarius_cartridge_interface;

class aquarius_cartridge_slot_device : public device_t,
	public device_cartrom_image_interface,
	public device_single_card_slot_interface<device_aquarius_cartridge_interface>
{
public:
	// construction/destruction
	template <typename T>
	aquarius_cartridge_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: aquarius_cartridge_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	aquarius_cartridge_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

	// callbacks
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "aquarius_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	// reading and writing
	uint8_t mreq_r(offs_t offset);
	void mreq_w(offs_t offset, uint8_t data);
	uint8_t mreq_ce_r(offs_t offset);
	void mreq_ce_w(offs_t offset, uint8_t data);
	uint8_t iorq_r(offs_t offset);
	void iorq_w(offs_t offset, uint8_t data);

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;

	device_aquarius_cartridge_interface *m_cart;

private:
	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;
};


// ======================> device_aquarius_cartridge_interface

class device_aquarius_cartridge_interface : public device_interface
{
public:
	// reading and writing
	virtual uint8_t mreq_r(offs_t offset) { return 0xff; }
	virtual void mreq_w(offs_t offset, uint8_t data) { }
	virtual uint8_t mreq_ce_r(offs_t offset) { return 0xff; }
	virtual void mreq_ce_w(offs_t offset, uint8_t data) { }
	virtual uint8_t iorq_r(offs_t offset) { return 0xff; }
	virtual void iorq_w(offs_t offset, uint8_t data) { }

	void rom_alloc(uint32_t size);
	uint8_t* get_rom_base() { return m_rom; }
	uint32_t get_rom_size() { return m_rom_size; }

protected:
	// construction/destruction
	device_aquarius_cartridge_interface(const machine_config &mconfig, device_t &device);

	aquarius_cartridge_slot_device *m_slot;

private:
	// internal state
	uint8_t *m_rom;
	uint32_t m_rom_size;
};


// device type definition
DECLARE_DEVICE_TYPE(AQUARIUS_CARTRIDGE_SLOT, aquarius_cartridge_slot_device)

void aquarius_cartridge_devices(device_slot_interface &device);


#endif // MAME_BUS_AQUARIUS_SLOT_H
