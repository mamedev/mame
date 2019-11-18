// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_midi.cpp
 *
 * X68000 MIDI interface - YM3802
 *
 */

#include "emu.h"
#include "bus/midi/midi.h"
#include "x68k_midi.h"

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE(X68K_MIDI, x68k_midi_device, "x68k_midi", "X68000 MIDI Interface")

void x68k_midi_device::device_add_mconfig(machine_config &config)
{
	YM3802(config, m_midi, XTAL(1'000'000));  // clock is unknown
	m_midi->txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_midi->irq_handler().set(FUNC(x68k_midi_device::irq_w));
	MIDI_PORT(config, "mdin", midiin_slot, "midiin");
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
//  MIDI_PORT(config, "mdthru", midiout_slot, "midiout");
	// TODO: Add serial data handlers
}


x68k_midi_device::x68k_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X68K_MIDI, tag, owner, clock)
	, device_x68k_expansion_card_interface(mconfig, *this)
	, m_slot(nullptr)
	, m_midi(*this, "midi")
{
}

void x68k_midi_device::device_start()
{
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	m_slot->space().install_readwrite_handler(0xeafa00,0xeafa0f, read8_delegate(*this, FUNC(x68k_midi_device::x68k_midi_reg_r)), write8_delegate(*this, FUNC(x68k_midi_device::x68k_midi_reg_w)), 0x00ff00ff);
}

void x68k_midi_device::device_reset()
{
}

READ8_MEMBER(x68k_midi_device::x68k_midi_reg_r)
{
	return m_midi->read(space, offset);
}

WRITE8_MEMBER(x68k_midi_device::x68k_midi_reg_w)
{
	m_midi->write(space, offset, data);
}

void x68k_midi_device::irq_w(int state)
{
	m_slot->irq4_w(state);  // selectable between IRQ2 and IRQ4
}

uint8_t x68k_midi_device::iack4()
{
	return MIDI_IRQ_VECTOR | (m_midi->vector() & 0x1f);
}
