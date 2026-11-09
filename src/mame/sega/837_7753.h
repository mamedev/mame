// license:BSD-3-Clause
// copyright-holders:Tomás García-Merás Capote

#ifndef MAME_SEGA_837_7753_H
#define MAME_SEGA_837_7753_H

#pragma once

#include "machine/pit8253.h"
#include "machine/upd4701.h"

class sega_837_7753_device : public device_t
{
public:
	sega_837_7753_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);

	auto lamps_cb() { return m_lamps_cb.bind(); }

	// host interface on the main board I/O chip
	u8 data_r();
	void data_w(u8 data);
	void addr_w(u8 data);
	void strobe_w(int state);

	ioport_value bank_encoder_r();
	ioport_value motor_encoder_r();

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pit8254_device> m_pit1;
	required_device<upd4701_device> m_upd4701;
	required_shared_ptr<u8> m_ram;
	devcb_write8 m_lamps_cb;
	output_finder<> m_bank_position;
	output_finder<> m_bank_motor;

	u8 m_host_addr;
	u8 m_host_data;
	bool m_host_strobe;
	u8 m_drive_enable;
	u8 m_pwm_duty;
	double m_motor_speed;
	double m_motor_pos;

	void program_map(address_map &map) ATTR_COLD;

	IRQ_CALLBACK_MEMBER(irq_ack);
	void timer_irq_w(int state);
	u8 switches_r();
	void drive_enable_w(u8 data);
	void pit1_w(offs_t offset, u8 data);
	TIMER_CALLBACK_MEMBER(motion_tick);
	s32 bank_position() const;
};

DECLARE_DEVICE_TYPE(SEGA_837_7753, sega_837_7753_device)

#endif // MAME_SEGA_837_7753_H
