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

	// configuration
	auto irq_vector_callback() { return m_irq_vector_cb.bind(); }

	void write(offs_t offset, u16 data);
	u16 read(offs_t offset);
	DECLARE_INPUT_CHANGED_MEMBER(mouse_changed);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	static constexpr u16 IRQ_VECTOR_KEYBOARD = 0xb0;
	static constexpr u16 IRQ_VECTOR_MOUSE = 0xb4;
	static constexpr u16 IRQ_VECTOR_CHAOSNET = 0xb8;
	static constexpr u16 IRQ_VECTOR_CLOCK = 0xbc;

	static constexpr int CSR_REMOTE_MOUSE_ENABLE_BIT = 0;
	static constexpr int CSR_MOUSE_IRQ_ENABLE_BIT = 1;
	static constexpr int CSR_KEYBOARD_IRQ_ENABLE_BIT = 2;
	static constexpr int CSR_CLOCK_IRQ_ENABLE_BIT = 3;
	static constexpr int CSR_MOUSE_READY_BIT = 4;
	static constexpr u16 CSR_MOUSE_READY = 1 << CSR_MOUSE_READY_BIT;
	static constexpr int CSR_KEYBOARD_READY_BIT = 5;
	static constexpr u16 CSR_KEYBOARD_READY = 1 << CSR_KEYBOARD_READY_BIT;
	static constexpr int CSR_CLOCK_READY_BIT = 6;
	static constexpr u16 CSR_CLOCK_READY = 1 << CSR_CLOCK_READY_BIT;

	static constexpr int CHAOSNET_TIMER_IRQ_ENABLE_BIT = 0;
	static constexpr u16 CHAOSNET_TIMER_IRQ_ENABLE = 1 << CHAOSNET_TIMER_IRQ_ENABLE_BIT;
	static constexpr int CHAOSNET_LOOPBACK_BIT = 1;
	static constexpr u16 CHAOSNET_LOOKBACK = 1 << CHAOSNET_LOOPBACK_BIT;
	static constexpr int CHAOSNET_ANY_DESTINATION_BIT = 2;
	static constexpr u16 CHAOSNET_ANY_DESTINATION = 1 << CHAOSNET_ANY_DESTINATION_BIT;
	static constexpr int CHAOSNET_RESET_RECEIVE_BIT = 3;
	static constexpr int CHAOSNET_RECEIVE_IRQ_ENABLE_BIT = 4;
	static constexpr u16 CHAOSNET_RECEIVE_IRQ_ENABLE = 1 << CHAOSNET_RECEIVE_IRQ_ENABLE_BIT;
	static constexpr int CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT = 5;
	static constexpr u16 CHAOSNET_TRANSMIT_IRQ_ENABLE = 1 << CHAOSNET_TRANSMIT_IRQ_ENABLE_BIT;
	static constexpr int CHAOSNET_TRANSMIT_ABORTED_BIT = 6;
	static constexpr u16 CHAOSNET_TRANSMIT_ABORTED = 1 << CHAOSNET_TRANSMIT_ABORTED_BIT;
	static constexpr int CHAOSNET_TRANSMIT_DONE_BIT = 7;
	static constexpr u16 CHAOSNET_TRANSMIT_DONE = 1 << CHAOSNET_TRANSMIT_DONE_BIT;
	static constexpr int CHAOSNET_RESET_TRANSMIT_BIT = 8;
	static constexpr u16 CHAOSNET_LOST_COUNT = 0x1e00;
	static constexpr int CHAOSNET_RESET_BIT = 13;
	static constexpr int CHAOSNET_RECEIVE_DONE_BIT = 15;
	static constexpr u16 CHAOSNET_RECEIVE_DONE = 1 << CHAOSNET_RECEIVE_DONE_BIT;

	u8 mcu_bus_r();
	void mcu_bus_w(u8 data);
	void mcu_p1_w(u8 data);
	u8 mcu_p2_r();
	TIMER_CALLBACK_MEMBER(clock_callback);

	required_device<i8748_device> m_i8748;
	devcb_write16 m_irq_vector_cb;
	required_ioport_array<16> m_keyboard;
	required_ioport m_mouse_x;
	required_ioport m_mouse_y;
	required_ioport m_mouse_buttons;
	required_device<speaker_sound_device> m_speaker;
	emu_timer *m_clock_timer;
	u8 m_p1;
	u32 m_bus;
	u32 m_keyboard_data;
	u16 m_csr;
	u8 m_speaker_data;
	u32 m_microsecond_clock_buffer;
	u16 m_clock;
	u16 m_chaos_csr;
	u16 m_chaos_transmit;
	u16 m_chaos_receive;
};

#endif // MAME_MIT_CADR_IOB_H
