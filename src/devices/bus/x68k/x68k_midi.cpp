// license:BSD-3-Clause
// copyright-holders:Barry Rodewald
/*
 * x68k_midi.c
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

MACHINE_CONFIG_START(x68k_midi_device::device_add_mconfig)
	MCFG_DEVICE_ADD("midi", YM3802, XTAL(1'000'000))  // clock is unknown
	MCFG_YM3802_TXD_HANDLER(WRITELINE("mdout",midi_port_device,write_txd))
	MCFG_YM3802_IRQ_HANDLER(WRITELINE(*this, x68k_midi_device,irq_w))
	MCFG_MIDI_PORT_ADD("mdin", midiin_slot, "midiin")
	MCFG_MIDI_PORT_ADD("mdout", midiout_slot, "midiout")
//  MCFG_MIDI_PORT_ADD("mdthru", midiout_slot, "midiout")
	// TODO: Add serial data handlers

MACHINE_CONFIG_END


x68k_midi_device::x68k_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, X68K_MIDI, tag, owner, clock)
	, device_x68k_expansion_card_interface(mconfig, *this)
	, m_slot(nullptr)
	, m_midi(*this, "midi")
{
}

void x68k_midi_device::device_start()
{
	device_t* cpu = machine().device("maincpu");
	address_space& space = cpu->memory().space(AS_PROGRAM);
	m_slot = dynamic_cast<x68k_expansion_slot_device *>(owner());
	space.install_readwrite_handler(0xeafa00,0xeafa0f,read8_delegate(FUNC(x68k_midi_device::x68k_midi_reg_r),this),write8_delegate(FUNC(x68k_midi_device::x68k_midi_reg_w),this),0x00ff00ff);
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
	set_vector(MIDI_IRQ_VECTOR | (m_midi->vector() & 0x1f));
	m_slot->irq4_w(state);  // selectable between IRQ2 and IRQ4
}
