// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Netronics Giant Board emulation

**********************************************************************/

#ifndef MAME_BUS_ELF2_GIANT_H
#define MAME_BUS_ELF2_GIANT_H

#pragma once

#include "exp.h"

#include "imagedev/cassette.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> elf2_giant_board_device

class elf2_giant_board_device : public device_t, public device_elf2_expansion_card_interface
{
public:
	elf2_giant_board_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	// device_elf2_expansion_card_interface implementation
	virtual int elf2_ef2_r() override;
	virtual void elf2_q_w(int state) override;

private:
	required_device<cassette_image_device> m_cassette;
	required_region_ptr<uint8_t> m_rom;
	required_ioport m_jumpers;

	bool m_tape_polarity;
};


DECLARE_DEVICE_TYPE(ELF2_GIANT_BOARD, elf2_giant_board_device)

#endif // MAME_BUS_ELF2_GIANT_H
