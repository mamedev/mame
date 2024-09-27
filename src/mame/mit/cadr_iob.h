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

	void write(offs_t offset, u32 data);
	u32 read(offs_t offset);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_add_mconfig(machine_config &config) override;
	virtual const tiny_rom_entry *device_rom_region() const override;
	virtual ioport_constructor device_input_ports() const override;

private:
	static constexpr u16 IRQ_VECTOR_KEYBOARD = 0xb0;
	static constexpr u16 IRQ_VECTOR_CLOCK = 0xbc;

	u8 mcu_bus_r();
	void mcu_bus_w(u8 data);
	void mcu_p1_w(u8 data);
	u8 mcu_p2_r();
	TIMER_CALLBACK_MEMBER(clock_callback);

	required_device<i8748_device> m_i8748;
	devcb_write16 m_irq_vector_cb;
	required_ioport_array<16> m_keyboard;
	required_device<speaker_sound_device> m_speaker;
	emu_timer *m_clock_timer;
	u8 m_p1;
	u32 m_bus;
	u32 m_keyboard_data;
	u16 m_csr;
	u8 m_speaker_data;
	u32 m_microsecond_clock_buffer;
	u16 m_clock;
};

#endif // MAME_MIT_CADR_IOB_H
