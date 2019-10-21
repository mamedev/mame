// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/***************************************************************************

    Amiga 2000 Keyboard

 ***************************************************************************/

#ifndef MAME_BUS_AMIGA_KEYBOARD_A2000_H
#define MAME_BUS_AMIGA_KEYBOARD_A2000_H

#pragma once

#include "keyboard.h"


namespace bus { namespace amiga { namespace keyboard {

//**************************************************************************
//  TYPE DECLARATIONS
//**************************************************************************

// ======================> a2000_kbd_g80_device

class a2000_kbd_g80_device : public device_t, public device_amiga_keyboard_interface
{
public:
	// from host
	virtual DECLARE_WRITE_LINE_MEMBER(kdat_w) override;

protected:
	// construction/destruction
	a2000_kbd_g80_device(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock);

	// MCU I/O
	DECLARE_READ8_MEMBER(mcu_bus_r);
	DECLARE_WRITE8_MEMBER(mcu_p1_w);
	DECLARE_WRITE8_MEMBER(mcu_p2_w);

	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;

	virtual void device_start() override;
	virtual void device_reset() override;

	void program_map(address_map &map);
	void ext_map(address_map &map);

private:
	required_ioport_array<13>   m_rows;
	required_device<cpu_device> m_mcu;

	uint16_t                    m_row_drive;
	bool                        m_host_kdat, m_mcu_kdat, m_mcu_kclk;
};

// ======================> a2000_kbd_g80_us_device

class a2000_kbd_g80_us_device : public a2000_kbd_g80_device
{
public:
	a2000_kbd_g80_us_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a2000_kbd_g80_de_device

class a2000_kbd_g80_de_device : public a2000_kbd_g80_device
{
public:
	a2000_kbd_g80_de_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a2000_kbd_g80_se_device

class a2000_kbd_g80_se_device : public a2000_kbd_g80_device
{
public:
	a2000_kbd_g80_se_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a2000_kbd_g80_dk_device

class a2000_kbd_g80_dk_device : public a2000_kbd_g80_device
{
public:
	a2000_kbd_g80_dk_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a2000_kbd_g80_gb_device

class a2000_kbd_g80_gb_device : public a2000_kbd_g80_device
{
public:
	a2000_kbd_g80_gb_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

} } } // namespace bus::amiga::keyboard


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

DECLARE_DEVICE_TYPE_NS(A2000_KBD_G80_US, bus::amiga::keyboard, a2000_kbd_g80_us_device)
DECLARE_DEVICE_TYPE_NS(A2000_KBD_G80_DE, bus::amiga::keyboard, a2000_kbd_g80_de_device)
DECLARE_DEVICE_TYPE_NS(A2000_KBD_G80_SE, bus::amiga::keyboard, a2000_kbd_g80_se_device)
DECLARE_DEVICE_TYPE_NS(A2000_KBD_G80_DK, bus::amiga::keyboard, a2000_kbd_g80_dk_device)
DECLARE_DEVICE_TYPE_NS(A2000_KBD_G80_GB, bus::amiga::keyboard, a2000_kbd_g80_gb_device)

#endif // MAME_BUS_AMIGA_KEYBOARD_A2000_H
