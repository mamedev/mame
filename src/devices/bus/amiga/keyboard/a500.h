// license: GPL-2.0+
// copyright-holders: Dirk Best
/***************************************************************************

    Amiga 500 Keyboard

    Assembly part numbers:

    - 312502-01  U.S./Canada
    - 312502-02  Germany/Austria
    - 312502-03  France/Belgium
    - 312502-04  Italy
    - 312502-05  Sweden/Finland
    - 312502-06  Spain
    - 312502-07  Denmark
    - 312502-08  Switzerland
    - 312502-09  Norway
    - 312502-12  UK

    Amiga 1000 (for reference, to be moved):

    - 327063-01  U.S./Canada
    - 327063-02  UK
    - 327063-03  Germany
    - 327063-04  France
    - 327063-05  Italy

***************************************************************************/

#ifndef MAME_BUS_AMIGA_KEYBOARD_A500_H
#define MAME_BUS_AMIGA_KEYBOARD_A500_H

#pragma once

#include "keyboard.h"
#include "cpu/m6502/m6502.h"


namespace bus { namespace amiga { namespace keyboard {

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> a500_kbd_device

class a500_kbd_device : public device_t, public device_amiga_keyboard_interface
{
public:

	DECLARE_INPUT_CHANGED_MEMBER(check_reset);

protected:
	a500_kbd_device(
			const machine_config &mconfig,
			const char *tag,
			device_t *owner,
			uint32_t clock,
			device_type type);

	// device-level overrides
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// from host
	virtual DECLARE_WRITE_LINE_MEMBER(kdat_w) override;

private:
	enum
	{
		// counter modes
		COUNTER_INTERVAL = 0x00,
		COUNTER_PULSE = 0x01,
		COUNTER_EVENT = 0x02,
		COUNTER_PWM = 0x03,

		// interrupt enables
		PA1_INT_ENABLED     = 0x04,
		PA0_INT_ENABLED     = 0x08,
		COUNTER_INT_ENABLED = 0x10,

		// status
		PA1_NEGATIVE_EDGE = 0x20,
		PA0_POSITIVE_EDGE = 0x40,
		COUNTER_OVERFLOW  = 0x80
	};

	void update_irqs();

	required_device<m6502_device> m_mpu;
	required_ioport m_special;
	required_ioport_array<15> m_rows;

	emu_timer *m_timer;
	emu_timer *m_watchdog;
	emu_timer *m_reset;

	int m_host_kdat;
	int m_mpu_kdat;
	int m_kclk;

	uint8_t m_port_c;
	uint8_t m_port_d;
	uint16_t m_latch;
	uint16_t m_counter;
	uint8_t m_control;

	// 6500/1 internal
	DECLARE_READ8_MEMBER(port_a_r);
	DECLARE_WRITE8_MEMBER(port_a_w);
	DECLARE_WRITE8_MEMBER(port_b_w);
	DECLARE_WRITE8_MEMBER(port_c_w);
	DECLARE_WRITE8_MEMBER(port_d_w);
	DECLARE_WRITE8_MEMBER(latch_w);
	DECLARE_READ8_MEMBER(counter_r);
	DECLARE_WRITE8_MEMBER(transfer_latch_w);
	DECLARE_WRITE8_MEMBER(clear_pa0_detect);
	DECLARE_WRITE8_MEMBER(clear_pa1_detect);
	DECLARE_READ8_MEMBER(control_r);
	DECLARE_WRITE8_MEMBER(control_w);

	void mpu6500_map(address_map &map);
};

// ======================> a500_kbd_us_device

class a500_kbd_us_device : public a500_kbd_device
{
public:
	a500_kbd_us_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_de_device

class a500_kbd_de_device : public a500_kbd_device
{
public:
	a500_kbd_de_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_fr_device

class a500_kbd_fr_device : public a500_kbd_device
{
public:
	a500_kbd_fr_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_it_device

class a500_kbd_it_device : public a500_kbd_device
{
public:
	a500_kbd_it_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_se_device

class a500_kbd_se_device : public a500_kbd_device
{
public:
	a500_kbd_se_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_es_device

class a500_kbd_es_device : public a500_kbd_device
{
public:
	a500_kbd_es_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_dk_device

class a500_kbd_dk_device : public a500_kbd_device
{
public:
	a500_kbd_dk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_ch_device

class a500_kbd_ch_device : public a500_kbd_device
{
public:
	a500_kbd_ch_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_no_device

class a500_kbd_no_device : public a500_kbd_device
{
public:
	a500_kbd_no_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

// ======================> a500_kbd_gb_device

class a500_kbd_gb_device : public a500_kbd_device
{
public:
	a500_kbd_gb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual ioport_constructor device_input_ports() const override;
};

} } } // namespace bus::amiga::keyboard


// device type definition
DECLARE_DEVICE_TYPE_NS(A500_KBD_US, bus::amiga::keyboard, a500_kbd_us_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_DE, bus::amiga::keyboard, a500_kbd_de_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_FR, bus::amiga::keyboard, a500_kbd_fr_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_IT, bus::amiga::keyboard, a500_kbd_it_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_SE, bus::amiga::keyboard, a500_kbd_se_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_ES, bus::amiga::keyboard, a500_kbd_es_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_DK, bus::amiga::keyboard, a500_kbd_dk_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_CH, bus::amiga::keyboard, a500_kbd_ch_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_NO, bus::amiga::keyboard, a500_kbd_no_device)
DECLARE_DEVICE_TYPE_NS(A500_KBD_GB, bus::amiga::keyboard, a500_kbd_gb_device)

#endif // MAME_BUS_AMIGA_KEYBOARD_A500_H
