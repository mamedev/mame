// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * symbfac2.h
 *
 *  Created on: 2/08/2014
 */

#ifndef MAME_BUS_CPC_SYMBFAC2_H
#define MAME_BUS_CPC_SYMBFAC2_H

#pragma once

#include "cpcexp.h"
#include "bus/ata/ataintf.h"
#include "machine/ds128x.h"
#include "machine/nvram.h"

class cpc_symbiface2_device  : public device_t,
								public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_symbiface2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	uint8_t ide_cs0_r(offs_t offset);
	void ide_cs0_w(offs_t offset, uint8_t data);
	uint8_t ide_cs1_r(offs_t offset);
	void ide_cs1_w(offs_t offset, uint8_t data);
	uint8_t mouse_r();
	uint8_t rom_rewrite_r();
	void rom_rewrite_w(uint8_t data);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_x);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_y);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_change_buttons);

	enum
	{
		PS2_MOUSE_IDLE = 0,
		PS2_MOUSE_X,
		PS2_MOUSE_Y,
		PS2_MOUSE_BUTTONS,
		PS2_MOUSE_SCROLL
	};

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;
	required_device<ata_interface_device> m_ide;
	required_device<ds12885_device> m_rtc;
	required_device<nvram_device> m_nvram;

	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_buttons;

	std::vector<uint8_t> m_rom_space;

	bool m_iohigh;
	uint16_t m_ide_data;

	uint8_t m_mouse_state;
	uint8_t m_input_x;
	uint8_t m_input_y;

	// stores backup pointers so that mapping can be restored
	uint8_t* m_4xxx_ptr_r;
	uint8_t* m_4xxx_ptr_w;
	uint8_t* m_6xxx_ptr_r;
	uint8_t* m_6xxx_ptr_w;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_SYMBIFACE2, cpc_symbiface2_device)


#endif // MAME_BUS_CPC_SYMBFAC2_H
