// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Sinclair QL ROM cartridge port emulation

**********************************************************************

                              A     B
                              *  1  *   VDD
                        A12   *  2  *   A14
                         A7   *  3  *   A13
                         A6   *  4  *   A8
                         A5   *  5  *   A9
                       SLOT      6      SLOT
                         A4   *  7  *   A11
                         A3   *  8  *   ROMOEH
                         A2   *  9  *   A10
                         A1   * 10  *   A15
                         A0   * 11  *   D7
                         D0   * 12  *   D6
                         D1   * 13  *   D5
                         D2   * 14  *   D4
                        GND   * 15  *   D3

**********************************************************************/

#ifndef MAME_BUS_QL_ROM_H
#define MAME_BUS_QL_ROM_H

#pragma once

#include "imagedev/cartrom.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> device_ql_rom_cartridge_card_interface

class ql_rom_cartridge_slot_device;

class device_ql_rom_cartridge_card_interface : public device_interface
{
	friend class ql_rom_cartridge_slot_device;

public:
	// construction/destruction
	virtual ~device_ql_rom_cartridge_card_interface();

	virtual void romoeh_w(int state) { m_romoeh = state; }
	virtual uint8_t read(offs_t offset, uint8_t data) { return data; }
	virtual void write(offs_t offset, uint8_t data) { }

protected:
	device_ql_rom_cartridge_card_interface(const machine_config &mconfig, device_t &device);

	virtual void interface_post_start() override;

	std::unique_ptr<uint8_t[]> m_rom;

	ql_rom_cartridge_slot_device *const m_slot;

	int m_romoeh;
};


// ======================> ql_rom_cartridge_slot_device

class ql_rom_cartridge_slot_device : public device_t,
								public device_single_card_slot_interface<device_ql_rom_cartridge_card_interface>,
								public device_cartrom_image_interface
{
public:
	// construction/destruction
	template <typename T>
	ql_rom_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, T &&opts, const char *dflt)
		: ql_rom_cartridge_slot_device(mconfig, tag, owner, 0)
	{
		option_reset();
		opts(*this);
		set_default_option(dflt);
		set_fixed(false);
	}
	ql_rom_cartridge_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	// computer interface
	uint8_t read(offs_t offset, uint8_t data) { if (m_card) data = m_card->read(offset, data); return data; }
	void write(offs_t offset, uint8_t data) { if (m_card) m_card->write(offset, data); }
	void romoeh_w(int state) { if (m_card) m_card->romoeh_w(state); }

protected:
	// device_t implementation
	virtual void device_resolve_objects() override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;

	// device_image_interface implementation
	virtual std::pair<std::error_condition, std::string> call_load() override;

	virtual bool is_reset_on_load() const noexcept override { return true; }
	virtual const char *image_interface() const noexcept override { return "ql_cart"; }
	virtual const char *file_extensions() const noexcept override { return "rom,bin"; }

	// device_slot_interface implementation
	virtual std::string get_default_card_software(get_default_card_software_hook &hook) const override;

	device_ql_rom_cartridge_card_interface *m_card;
};


// device type definition
DECLARE_DEVICE_TYPE(QL_ROM_CARTRIDGE_SLOT, ql_rom_cartridge_slot_device)

void ql_rom_cartridge_cards(device_slot_interface &device);

#endif // MAME_BUS_QL_ROM_H
