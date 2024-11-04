// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Wyse gate array-based terminal keyboards

***************************************************************************/

#ifndef MAME_BUS_WYSEKBD_WYSEGAKB_H
#define MAME_BUS_WYSEKBD_WYSEGAKB_H

#pragma once

#include "wysekbd.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> wyse_gate_array_keyboard_device

class wyse_gate_array_keyboard_device : public device_t, public wyse_keyboard_interface
{
protected:
	// construction/destruction
	wyse_gate_array_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// wyse_keyboard_interface overrides
	virtual bool wysekbd_read_data() override;
	virtual void wysekbd_write_cmd(bool state) override;

	virtual u8 wysekbd_get_id() = 0;
	virtual void wysekbd_update_leds(u8 index);

private:
	// key matrix
	optional_ioport_array<13> m_r;

	// internal state
	u8 m_key_index;
	bool m_clock_state;
	attotime m_reset_time;
};

// ======================> wy85_keyboard_device

class wy85_keyboard_device : public wyse_gate_array_keyboard_device
{
public:
	// device type constructor
	wy85_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
};

// ======================> wy30_keyboard_device

class wy30_keyboard_device : public wyse_gate_array_keyboard_device
{
public:
	// device type constructor
	wy30_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
};

// ======================> wy60_ascii_keyboard_device

class wy60_ascii_keyboard_device : public wyse_gate_array_keyboard_device
{
public:
	// device type constructor
	wy60_ascii_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
};

// ======================> wyse_at_keyboard_device

class wyse_at_keyboard_device : public wyse_gate_array_keyboard_device
{
public:
	// device type constructor
	wyse_at_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	wyse_at_keyboard_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);

	virtual void device_resolve_objects() override ATTR_COLD;

	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
	virtual void wysekbd_update_leds(u8 index) override;

private:
	// LED outputs
	output_finder<> m_caps_led;
	output_finder<> m_num_led;
	output_finder<> m_scroll_led;
};

// ======================> wyse_316x_keyboard_device

class wyse_316x_keyboard_device : public wyse_gate_array_keyboard_device
{
public:
	// device type constructor
	wyse_316x_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;

private:
	required_ioport m_lclr;
};

// ======================> wyse_pce_keyboard_device

class wyse_pce_keyboard_device : public wyse_at_keyboard_device
{
public:
	// device type constructor
	wyse_pce_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
};

// ======================> wyse_pceint_keyboard_device

class wyse_pceint_keyboard_device : public wyse_at_keyboard_device
{
public:
	// device type constructor
	wyse_pceint_keyboard_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0U);

protected:
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual u8 wysekbd_get_id() override;
};

// device type declarations
DECLARE_DEVICE_TYPE(WY85_KEYBOARD, wy85_keyboard_device)
DECLARE_DEVICE_TYPE(WY30_KEYBOARD, wy30_keyboard_device)
DECLARE_DEVICE_TYPE(WY60_ASCII_KEYBOARD, wy60_ascii_keyboard_device)
DECLARE_DEVICE_TYPE(WYSE_AT_KEYBOARD, wyse_at_keyboard_device)
DECLARE_DEVICE_TYPE(WYSE_316X_KEYBOARD, wyse_316x_keyboard_device)
DECLARE_DEVICE_TYPE(WYSE_PCE_KEYBOARD, wyse_pce_keyboard_device)
DECLARE_DEVICE_TYPE(WYSE_PCEINT_KEYBOARD, wyse_pceint_keyboard_device)

#endif // MAME_BUS_WYSEKBD_WYSEGAKB_H
