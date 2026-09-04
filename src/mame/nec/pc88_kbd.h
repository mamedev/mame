// license:BSD-3-Clause
// copyright-holders:Angelo Salese

#ifndef MAME_NEC_PC88_KBD_H
#define MAME_NEC_PC88_KBD_H

#pragma once

#include "machine/keyboard.h"
//#include "diserial.h"

class pc8001_kbd_device : public device_t
{
public:
	pc8001_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	u8 read_direct(offs_t offset) { return m_io_keys[offset]->read() ^ 0xff; }

protected:
	pc8001_kbd_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void device_start() override { }
	virtual void device_reset() override { }

private:
	required_ioport_array<16> m_io_keys;
};

class pc8801_kbd_device : public pc8001_kbd_device
{
public:
	pc8801_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
};

class pc8801fh_kbd_device : public pc8001_kbd_device
{
public:
	pc8801fh_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	int read_id_r();
	auto read_id() { return m_read_id_cb.bind(); }

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	devcb_read_line m_read_id_cb;
};

class pc88va_kbd_device : public device_t
						, protected device_matrix_keyboard_interface<16>
{
public:
	pc88va_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	auto irq_cb() { return m_irq_cb.bind(); }
	u8 read_direct(offs_t offset) { return m_key_rows[offset]->read() ^ 0xff; }

	u8 read_code(offs_t offset) { return m_scan_code; }
	void write_command(offs_t offset, u8 data);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

	virtual void key_make(uint8_t row, uint8_t column) override;
	virtual void key_break(uint8_t row, uint8_t column) override;
	virtual void key_repeat(uint8_t row, uint8_t column) override;
	uint8_t translate(uint8_t row, uint8_t column);

private:
	devcb_write_line m_irq_cb;
	u8 m_scan_code;
};

DECLARE_DEVICE_TYPE(PC8001_KBD,   pc8001_kbd_device)
DECLARE_DEVICE_TYPE(PC8801_KBD,   pc8801_kbd_device)
DECLARE_DEVICE_TYPE(PC8801FH_KBD, pc8801fh_kbd_device)
DECLARE_DEVICE_TYPE(PC88VA_KBD,   pc88va_kbd_device)

#endif // MAME_NEC_PC88_KBD_H
