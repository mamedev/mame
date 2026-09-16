// license:BSD-3-Clause
// copyright-holders:AJR

#ifndef MAME_BUS_ISA_PCMIDI_H
#define MAME_BUS_ISA_PCMIDI_H

#pragma once

#include "isa.h"
#include "bus/midi/midi.h"
#include "cpu/z8/z8.h"
#include "machine/gen_latch.h"

class isa8_pcmidi_device : public device_t, public device_isa8_card_interface
{
public:
	isa8_pcmidi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

protected:
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;
	virtual ioport_constructor device_input_ports() const override ATTR_COLD;
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
	virtual const tiny_rom_entry *device_rom_region() const override ATTR_COLD;

private:
	void set_host_irq(bool state);
	u8 host_r(offs_t offset);
	void host_w(offs_t offset, u8 data);
	void status_w(u8 data);
	void mpu_p3_w(u8 data);

	void mpu_map(address_map &map) ATTR_COLD;

	required_device<z8_device> m_mpu;
	required_device<generic_latch_8_device> m_cmdlatch;
	required_device<generic_latch_8_device> m_statlatch;
	required_device<midi_port_device> m_midiout;
	required_ioport m_config;

	u8 m_mpu_p3;
	bool m_host_irq;
};

DECLARE_DEVICE_TYPE(ISA8_PCMIDI, isa8_pcmidi_device)

#endif // MAME_BUS_ISA_PCMIDI_H
