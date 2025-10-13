// license:BSD-3-Clause
// copyright-holders:R. Belmont
#ifndef MAME_ENSONIQ_ESQPANEL_H
#define MAME_ENSONIQ_ESQPANEL_H

#pragma once

#include "esqvfd.h"
#include "esqlcd.h"

#include "diserial.h"

#include <iostream>
#include <set>
#include <vector>


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> esqpanel_device

class esqpanel_device : public device_t, public device_serial_interface
{
public:
	auto write_tx() { return m_write_tx.bind(); }
	auto write_analog() { return m_write_analog.bind(); }
	auto read_analog() { return m_read_analog.bind(); }

	void xmit_char(uint8_t data);

	void set_char(int row, int column, uint8_t c, uint8_t attr);
	virtual void set_analog_value(offs_t offset, uint16_t value);
	uint16_t get_analog_value(offs_t offset);
	virtual void set_button(uint8_t button, bool pressed);

protected:
	// construction/destruction
	esqpanel_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock);

	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_reset_after_children() override ATTR_COLD;
	virtual void device_stop() override ATTR_COLD;

	// serial overrides
	virtual void rcv_complete() override;    // Rx completed receiving byte
	virtual void tra_complete() override;    // Tx completed sending byte
	virtual void tra_callback() override;    // Tx send bit

	virtual void send_to_display(uint8_t data) = 0;

	std::set<int> m_pressed_buttons;
	std::vector<uint8_t> m_light_states;

	bool m_eps_mode = false;
	bool m_expect_calibration_second_byte = false;
	bool m_expect_light_second_byte = false;

	template <typename Format, typename... Params>
	void logerror(Format &&fmt, Params &&... args) const
	{
		util::stream_format(
				std::cerr,
				"[%s] %s",
				tag(),
				util::string_format(std::forward<Format>(fmt), std::forward<Params>(args)...));
	}

private:
	static const int XMIT_RING_SIZE = 16;

	devcb_write_line m_write_tx;
	devcb_write16 m_write_analog;
	devcb_read16 m_read_analog;
	uint8_t m_xmitring[XMIT_RING_SIZE];
	int m_xmit_read, m_xmit_write = 0;
	bool m_tx_busy = false;
};

class esqpanel1x22_device : public esqpanel_device {
public:
	esqpanel1x22_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void send_to_display(uint8_t data) override { m_vfd->write_char(data); }

	required_device<esq1x22_device> m_vfd;
};

class esqpanel2x40_device : public esqpanel_device {
public:
	esqpanel2x40_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void send_to_display(uint8_t data) override { m_vfd->write_char(data); }

	required_device<esq2x40_device> m_vfd;
};

class esqpanel2x40_vfx_device : public esqpanel_device {
public:
	esqpanel2x40_vfx_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

	DECLARE_INPUT_CHANGED_MEMBER(button_change);
	DECLARE_INPUT_CHANGED_MEMBER(analog_value_change);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override;

	virtual void send_to_display(uint8_t data) override { }
	virtual void rcv_complete() override;    // Rx completed receiving byte

	required_device<esq2x40_vfx_device> m_vfd;

	static constexpr uint8_t AT_NORMAL      = 0x00;
	static constexpr uint8_t AT_UNDERLINE   = 0x01;
	static constexpr uint8_t AT_BLINK       = 0x02;

	TIMER_CALLBACK_MEMBER(update_blink);

	ioport_value get_adjuster_value(required_ioport &ioport);
	void set_adjuster_value(required_ioport &ioport, const ioport_value & value);

private:
	int m_cursx = 0, m_cursy = 0;
	int m_savedx = 0, m_savedy = 0;
	uint8_t m_curattr = 0;

	emu_timer *m_blink_timer = nullptr;
	uint8_t m_blink_phase;

	void update_lights();

	output_finder<> m_lights;
	output_finder<> m_variant;

	required_ioport m_buttons_0;
	required_ioport m_buttons_32;
	required_ioport m_analog_data_entry;
	required_ioport m_analog_volume;

};

class esqpanel2x40_sq1_device : public esqpanel_device {
public:
	esqpanel2x40_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void send_to_display(uint8_t data) override { m_vfd->write_char(data); }

	required_device<esq2x40_sq1_device> m_vfd;
};

// --- SQ1 - Parduz --------------------------------------------------------------------------------------------------------------------------
class esqpanel2x16_sq1_device : public esqpanel_device {
public:
	esqpanel2x16_sq1_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock = 0);

protected:
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual void send_to_display(uint8_t data) override { m_vfd->write_char(data); }

	required_device<esq2x16_sq1_device> m_vfd;
};

DECLARE_DEVICE_TYPE(ESQPANEL1X22,     esqpanel1x22_device)
DECLARE_DEVICE_TYPE(ESQPANEL2X40,     esqpanel2x40_device)
DECLARE_DEVICE_TYPE(ESQPANEL2X40_VFX, esqpanel2x40_vfx_device)
DECLARE_DEVICE_TYPE(ESQPANEL2X40_SQ1, esqpanel2x40_sq1_device)
DECLARE_DEVICE_TYPE(ESQPANEL2X16_SQ1, esqpanel2x16_sq1_device)

#endif // MAME_ENSONIQ_ESQPANEL_H
