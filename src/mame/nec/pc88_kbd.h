// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88_KBD_H
#define MAME_NEC_PC88_KBD_H

#pragma once

#include "machine/keyboard.h"
#include "diserial.h"

class pc8001_kbd_device : public device_t
{
public:
	pc8001_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	u8 read_direct(offs_t offset) { return m_io_keys[offset]->read(); }

protected:
	pc8001_kbd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override { }
	virtual void device_reset() override { }

private:
	required_ioport_array<16> m_io_keys;
};

class pc8801_kbd_device : public pc8001_kbd_device
{
public:
	pc8801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class pc8801fh_kbd_device : public pc8001_kbd_device
{
public:
	pc8801fh_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};


DECLARE_DEVICE_TYPE(PC8001_KBD,   pc8001_kbd_device)
DECLARE_DEVICE_TYPE(PC8801_KBD,   pc8801_kbd_device)
DECLARE_DEVICE_TYPE(PC8801FH_KBD, pc8801fh_kbd_device)


#endif // MAME_NEC_PC88_KBD_H
