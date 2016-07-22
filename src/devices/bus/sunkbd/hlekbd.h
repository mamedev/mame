// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_SUNKBD_HLEKBD_H
#define MAME_DEVICES_SUNKBD_HLEKBD_H

#pragma once

#include "sunkbd.h"
#include "sound/beep.h"


extern device_type const SUN_TYPE3_HLE_KEYBOARD;
extern device_type const SUN_TYPE4_HLE_KEYBOARD;
extern device_type const SUN_TYPE5_HLE_KEYBOARD;
extern device_type const SUN_TYPE5_UK_HLE_KEYBOARD;


namespace bus { namespace sunkbd {
class hle_device_base : public device_t, public device_serial_interface, public device_sun_keyboard_port_interface
{
public:
	virtual machine_config_constructor device_mconfig_additions() const override;

	virtual DECLARE_WRITE_LINE_MEMBER( input_txd ) override;

protected:
	// constructor/destructor
	hle_device_base(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			UINT32 clock);
	hle_device_base(
			machine_config const &mconfig,
			device_type type, char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);
	virtual ~hle_device_base() override;

	// device overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

	// device_serial_interface overrides
	virtual void tra_callback() override;
	virtual void tra_complete() override;
	virtual void rcv_complete() override;

	required_ioport m_dips;
	required_ioport m_key_inputs[8];

private:
	// device_serial_interface uses 10'000 range
	enum {
		SCAN_TIMER_ID = 20'000,
		CLICK_TIMER_ID
	};

	// TODO: ensure these don't clash with diagnostic LEDs on host computer
	enum : int {
		LED_NUM = 0,
		LED_COMPOSE,
		LED_SCROLL,
		LED_CAPS
	};

	enum : UINT8 {
		BEEPER_BELL = 0x01U,
		BEEPER_CLICK = 0x02U
	};

	enum : UINT8 {
		RX_IDLE,
		RX_LED
	};

	enum : UINT8 {
		COMMAND_RESET = 0x01U,
		COMMAND_BELL_ON = 0x02U,
		COMMAND_BELL_OFF = 0x03U,
		COMMAND_CLICK_ON = 0x0aU,
		COMMAND_CLICK_OFF = 0x0bU,
		COMMAND_LED = 0x0eU,
		COMMAND_LAYOUT = 0x0fU
	};

	virtual UINT8 ident_byte() = 0;

	void scan_row();
	void send_byte(UINT8 code);

	emu_timer                       *m_scan_timer;
	emu_timer                       *m_click_timer;
	required_device<beep_device>    m_beeper;

	UINT16  m_current_keys[8];
	UINT8   m_next_row;

	UINT8   m_fifo[16];
	UINT8   m_head, m_tail;
	UINT8   m_empty;

	UINT8   m_rx_state;

	UINT8   m_keyclick;
	UINT8   m_beeper_state;
};


class hle_type3_device : public hle_device_base
{
public:
	hle_type3_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			UINT32 clock);
	hle_type3_device(
			machine_config const &mconfig,
			device_type type, char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);

	virtual ioport_constructor device_input_ports() const override;

private:
	virtual UINT8 ident_byte() override;
};


class hle_type4_device : public hle_device_base
{
public:
	hle_type4_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			UINT32 clock);
	hle_type4_device(
			machine_config const &mconfig,
			device_type type, char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);

	virtual ioport_constructor device_input_ports() const override;

private:
	virtual UINT8 ident_byte() override;
};


class hle_type5_device : public hle_device_base
{
public:
	hle_type5_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			UINT32 clock);
	hle_type5_device(
			machine_config const &mconfig,
			device_type type, char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);

	virtual ioport_constructor device_input_ports() const override;

private:
	virtual UINT8 ident_byte() override;
};


class hle_type5_uk_device : public hle_device_base
{
public:
	hle_type5_uk_device(
			machine_config const &mconfig,
			char const *tag,
			device_t *owner,
			UINT32 clock);
	hle_type5_uk_device(
			machine_config const &mconfig,
			device_type type, char const *name,
			char const *tag,
			device_t *owner,
			UINT32 clock,
			char const *shortname,
			char const *source);

	virtual ioport_constructor device_input_ports() const override;

private:
	virtual UINT8 ident_byte() override;
};

} } // namespace bus::sunkbd

#endif // MAME_DEVICES_SUNKBD_HLEKBD_H
