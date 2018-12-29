// license:BSD-3-Clause
// copyright-holders:Sergey Svishchev
#ifndef MAME_MACHINE_IE15_KBD_H
#define MAME_MACHINE_IE15_KBD_H

#pragma once

class ie15_keyboard_device : public device_t
{
public:
	enum
	{
		IE_KB_ACK   = 1,

		IE_KB_RED   = 0x01,
		IE_KB_SDV   = 0x02,
		IE_KB_DUP   = 0x08,
		IE_KB_LIN   = 0x10,
		IE_KB_DK    = 0x20,
		IE_KB_PCH   = 0x40,
		IE_KB_NR    = 0x80,

		IE_KB_RED_BIT   = 0,
		IE_KB_SDV_BIT   = 1,
		IE_KB_DUP_BIT   = 3,
		IE_KB_LIN_BIT   = 4,
		IE_KB_DK_BIT    = 5,
		IE_KB_PCH_BIT   = 6,
		IE_KB_NR_BIT    = 7,

		IE_KB_SI    = 0x0f,
		IE_KB_SO    = 0x0e
	};


	ie15_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	auto keyboard_cb() { return m_keyboard_cb.bind(); }

protected:
	ie15_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void send_key(uint16_t code) { m_keyboard_cb(offs_t(0), code); }

	required_ioport_array<4> m_io_kbd;
	required_ioport m_io_kbdc;

	emu_timer *m_timer;

private:
	virtual uint16_t keyboard_handler(uint16_t last_code, uint8_t *scan_line);
	uint8_t row_number(uint32_t code);
	uint16_t m_last_code;
	uint8_t m_scan_line;
	uint8_t m_ruslat;
	uint8_t *m_rom;

	devcb_write16 m_keyboard_cb;
};

DECLARE_DEVICE_TYPE(IE15_KEYBOARD, ie15_keyboard_device)

#endif // MAME_MACHINE_IE15_KBD_H
