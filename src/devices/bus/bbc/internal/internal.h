// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    BBC Micro internal expansion boards

**********************************************************************/

#ifndef MAME_BUS_BBC_INTERNAL_INTERNAL_H
#define MAME_BUS_BBC_INTERNAL_INTERNAL_H

#pragma once

#include "machine/ram.h"
#include "bus/bbc/rom/slot.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class bbc_internal_slot_device;

// ======================> device_bbc_internal_interface

class device_bbc_internal_interface : public device_interface
{
public:
	virtual bool overrides_ram() { return false; }
	virtual bool overrides_rom() { return false; }
	virtual bool overrides_mos() { return false; }
	virtual uint8_t ram_r(offs_t offset) { return 0xff; }
	virtual void ram_w(offs_t offset, uint8_t data) { }
	virtual uint8_t romsel_r(offs_t offset) { return 0xfe; }
	virtual void romsel_w(offs_t offset, uint8_t data) { }
	virtual uint8_t paged_r(offs_t offset) { return 0xff; }
	virtual void paged_w(offs_t offset, uint8_t data) { }
	virtual uint8_t mos_r(offs_t offset) { return 0xff; }
	virtual void mos_w(offs_t offset, uint8_t data) { }
	virtual void latch_fe60_w(uint8_t data) { }

	virtual void irq6502_w(int state) { }

protected:
	device_bbc_internal_interface(const machine_config &mconfig, device_t &device);

	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_mb_ram;
	optional_device_array<bbc_romslot_device, 16> m_mb_rom;
	required_memory_region m_region_swr;
	required_memory_region m_region_mos;

	bbc_internal_slot_device *m_slot;
};

// ======================> bbc_internal_slot_device

class bbc_internal_slot_device : public device_t, public device_single_card_slot_interface<device_bbc_internal_interface>
{
public:
	// construction/destruction
	template <typename T>
	bbc_internal_slot_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock, T &&slot_options, const char *default_option)
		: bbc_internal_slot_device(mconfig, tag, owner, clock)
	{
		option_reset();
		slot_options(*this);
		set_default_option(default_option);
		set_fixed(false);
	}

	bbc_internal_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// inline configuration
	auto irq_handler() { return m_irq_handler.bind(); }
	auto nmi_handler() { return m_nmi_handler.bind(); }

	void irq_w(int state) { m_irq_handler(state); }
	void nmi_w(int state) { m_nmi_handler(state); }

	virtual bool overrides_ram() { return m_card ? m_card->overrides_ram() : false; }
	virtual bool overrides_rom() { return m_card ? m_card->overrides_rom() : false; }
	virtual bool overrides_mos() { return m_card ? m_card->overrides_mos() : false; }
	virtual uint8_t ram_r(offs_t offset);
	virtual void ram_w(offs_t offset, uint8_t data);
	virtual uint8_t romsel_r(offs_t offset);
	virtual void romsel_w(offs_t offset, uint8_t data);
	virtual uint8_t paged_r(offs_t offset);
	virtual void paged_w(offs_t offset, uint8_t data);
	virtual uint8_t mos_r(offs_t offset);
	virtual void mos_w(offs_t offset, uint8_t data);
	virtual void latch_fe60_w(uint8_t data);

	virtual void irq6502_w(int state);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	devcb_write_line m_irq_handler;
	devcb_write_line m_nmi_handler;

	uint8_t m_romsel_fe30;
	uint8_t m_romsel_fe62;

	device_bbc_internal_interface *m_card;
};

// device type definition
DECLARE_DEVICE_TYPE(BBC_INTERNAL_SLOT, bbc_internal_slot_device)

void bbcb_internal_devices(device_slot_interface &device);
void bbcbp_internal_devices(device_slot_interface &device);
void bbcm_internal_devices(device_slot_interface &device);


#endif // MAME_BUS_BBC_INTERNAL_INTERNAL_H
