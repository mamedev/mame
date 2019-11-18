// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
/**********************************************************************

    Iskra-1030 XX-key keyboard emulation

*********************************************************************/

#ifndef MAME_BUS_PC_KBD_ISKR1030_H
#define MAME_BUS_PC_KBD_ISKR1030_H

#pragma once

#include "pc_kbdc.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/rescap.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> iskr_1030_keyboard_device

class iskr_1030_keyboard_device : public device_t, public device_pc_kbd_interface
{
public:
	// construction/destruction
	iskr_1030_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// optional information overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual ioport_constructor device_input_ports() const override;

	// device_pc_kbd_interface overrides
	virtual DECLARE_WRITE_LINE_MEMBER( clock_write ) override;
	virtual DECLARE_WRITE_LINE_MEMBER( data_write ) override;

private:
	DECLARE_READ8_MEMBER( p1_r );
	DECLARE_WRITE8_MEMBER( p1_w );
	DECLARE_WRITE8_MEMBER( p2_w );
	DECLARE_READ_LINE_MEMBER( t1_r );

	DECLARE_READ8_MEMBER( ram_r );
	DECLARE_WRITE8_MEMBER( ram_w );

	void iskr_1030_keyboard_io(address_map &map);

	required_device<i8048_device> m_maincpu;
	required_ioport m_md00;
	required_ioport m_md01;
	required_ioport m_md02;
	required_ioport m_md03;
	required_ioport m_md04;
	required_ioport m_md05;
	required_ioport m_md06;
	required_ioport m_md07;
	required_ioport m_md08;
	required_ioport m_md09;
	required_ioport m_md10;
	required_ioport m_md11;
	required_ioport m_md12;
	required_ioport m_md13;
	required_ioport m_md14;
	required_ioport m_md15;
	required_ioport m_md16;
	required_ioport m_md17;
	required_ioport m_md18;
	required_ioport m_md19;
	required_ioport m_md20;
	required_ioport m_md21;
	required_ioport m_md22;
	required_ioport m_md23;

	std::vector<uint8_t> m_ram;
	uint8_t m_bus;
	uint8_t m_p1;
	uint8_t m_p2;
	int m_q;
};


// device type definition
DECLARE_DEVICE_TYPE(PC_KBD_ISKR_1030, iskr_1030_keyboard_device)

#endif // MAME_BUS_PC_KBD_ISKR1030_H
