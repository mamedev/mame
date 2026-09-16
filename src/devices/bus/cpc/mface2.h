// license:BSD-3-Clause
// copyright-holders:Kevin Thacker, Barry Rodewald
/*
 * mface2.h  --  Romantic Robot Multiface II expansion device for the Amstrad CPC/CPC+
 *
 *  Created on: 31/07/2011
 *
 *  I/O Ports:
 *    -  FEE8: Enables Multiface ROM and RAM
 *    -  FEEA: Disables Multiface ROM and RAM
 *
 *  When enabled, Multiface ROM is mapped to &0000, and RAM to &2000
 *
 *  When the Stop button is pressed, the Multiface II will generate an NMI
 *  (I guess the ROM/RAM is enabled when you do this also?)
 *
 *  It also monitors all I/O port writes, so that it can restore them when resuming the current application.
 */

#ifndef MAME_BUS_CPC_MFACE2_H
#define MAME_BUS_CPC_MFACE2_H

#pragma once

#include "cpcexp.h"

/* stop button has been pressed */
#define MULTIFACE_STOP_BUTTON_PRESSED   0x0001
/* ram/rom is paged into memory space */
#define MULTIFACE_RAM_ROM_ENABLED       0x0002
/* when visible OUT commands are performed! */
#define MULTIFACE_VISIBLE               0x0004


class cpc_multiface2_device :   public device_t,
								public device_cpc_expansion_card_interface
{
public:
	// construction/destruction
	cpc_multiface2_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	int multiface_hardware_enabled();
	void multiface_rethink_memory();
	void multiface_stop();
	int multiface_io_write(uint16_t offset, uint8_t data);
	void check_button_state();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	cpc_expansion_slot_device *m_slot;

	std::unique_ptr<uint8_t[]> m_multiface_ram;
	unsigned long m_multiface_flags;

	uint8_t m_romdis;
};

// device type definition
DECLARE_DEVICE_TYPE(CPC_MFACE2, cpc_multiface2_device)

#endif // MAME_BUS_CPC_MFACE2_H
