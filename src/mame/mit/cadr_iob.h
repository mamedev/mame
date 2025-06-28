// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
#ifndef MAME_MIT_CADR_IOB_H
#define MAME_MIT_CADR_IOB_H

#pragma once


#include "cpu/mcs48/mcs48.h"
#include "sound/spkrdev.h"


DECLARE_DEVICE_TYPE(CADR_IOB, cadr_iob_device)


class cadr_iob_device : public device_t
{
public:
	cadr_iob_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	static constexpr feature_type unemulated_features() { return feature::LAN; }

	// configuration
	auto irq_vector_callback() { return m_irq_vector_cb.bind(); }

	void write(offs_t offset, u16 data);
	u16 read(offs_t offset);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_changed);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;

private:
	static constexpr u16 CHAOS_BUFFER_SIZE = 512 / 2;

	u8 mcu_bus_r();
	void mcu_bus_w(u8 data);
	void mcu_p1_w(u8 data);
	u8 mcu_p2_r();
	TIMER_CALLBACK_MEMBER(clock_callback);
	TIMER_CALLBACK_MEMBER(transmit_callback);
	void chaos_transmit_start();

	required_device<i8748_device> m_i8748;
	devcb_write16 m_irq_vector_cb;
	required_ioport_array<16> m_keyboard;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_buttons;
	required_ioport m_my_chaos_address;
	required_device<speaker_sound_device> m_speaker;
	emu_timer *m_clock_timer;
	emu_timer *m_transmit_timer;
	u8 m_p1;
	u32 m_bus;
	u32 m_keyboard_data;
	u16 m_csr;
	u8 m_speaker_data;
	u32 m_microsecond_clock_buffer;
	u16 m_clock;
	u16 m_chaos_csr;
	u16 m_chaos_transmit_buffer[CHAOS_BUFFER_SIZE];
	u16 m_chaos_transmit_pointer;
	u16 m_chaos_receive_buffer[CHAOS_BUFFER_SIZE];
	u16 m_chaos_receive_size;
	u16 m_chaos_receive_pointer;
	u16 m_chaos_receive_bit_count;
};

#endif // MAME_MIT_CADR_IOB_H
