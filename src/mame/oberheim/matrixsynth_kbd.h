// license:BSD-3-Clause
// copyright-holders:m1macrophage

// The velocity-sensitive keyboard and keyboard scanner, used by the Matrix-12
// and Matrix-6.
//
// The scanner tracks the state of keys and the velocity of key presses and
// releases, and pushes key press and release events (along with their velocity)
// to FIFOs polled by the synth's firmware.
//
// The circuits for the two synths are almost identical. They only differ in how
// the DIR (data in ready) and DOR (data out ready) signals are determined. See
// respective subclasses for details.

#ifndef MAME_OBERHEIM_MATRIXSYNTH_KBD_H
#define MAME_OBERHEIM_MATRIXSYNTH_KBD_H

#pragma once

#include "machine/40105.h"
#include "machine/timer.h"

DECLARE_DEVICE_TYPE(MATRIX12_KBD, matrix12_kbd_device)
DECLARE_DEVICE_TYPE(MATRIX6_KBD, matrix6_kbd_device)


class matrixsynth_kbd_device_base : public device_t
{
public:
	void kbdclr_w(int state);

	u8 kbd0_r(offs_t offset) { return fifo_r(offset, 0); }
	u8 kbd1_r(offs_t offset) { return fifo_r(offset, 2); }

	DECLARE_INPUT_CHANGED_MEMBER(key_changed);

protected:
	matrixsynth_kbd_device_base(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	void device_add_mconfig(machine_config &config) override ATTR_COLD;
	ioport_constructor device_input_ports() const override ATTR_COLD;
	void device_start() override ATTR_COLD;
	void device_reset() override ATTR_COLD;

	required_device_array<cmos_40105_device, 4> m_fifo;

private:
	virtual int dir_r() const = 0;
	u8 fifo_r(offs_t offset, int fifo_index) const;

	TIMER_DEVICE_CALLBACK_MEMBER(scan_key);
	TIMER_CALLBACK_MEMBER(update_key_sw);

	bool m_kbdclr;
	u8 m_key_addr;
	std::array<u8, 64> m_count_ram;  // 2114 RAM, A6-A9 tied to GND.
	std::array<u8, 64> m_status_ram;  // 2114 RAM, A6-A9 tied to GND.

	required_ioport m_vel_press;
	required_ioport m_vel_rel;
	std::array<emu_timer *, 64> m_vel_timers;
	std::array<bool, 64> m_sw1;
	std::array<bool, 64> m_sw2;
};


// Matrix-12 keyboard scanner.
class matrix12_kbd_device : public matrixsynth_kbd_device_base
{
public:
	matrix12_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	int dor_neg_r() const;

private:
	int dir_r() const override;
};


// Matrix-6 keyboard scanner.
class matrix6_kbd_device : public matrixsynth_kbd_device_base
{
public:
	matrix6_kbd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) ATTR_COLD;

	int dor_r() const;
	auto dor_cb() { return m_dor_cb.bind(); }

protected:
	void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	int dir_r() const override;

	devcb_write_line m_dor_cb;
};

#endif  // MAME_OBERHEIM_MATRIXSYNTH_KBD_H
