// license:BSD-3-Clause
// copyright-holders:tim lindner
/***************************************************************************

    coco_midi.cpp

    Emulation of Rutherford Research's Midi Pak and also compatible with
    Go4Retro's MIDI Maestro.

***************************************************************************/

#include "emu.h"
#include "coco_midi.h"

#include "machine/6850acia.h"
#include "machine/clock.h"
#include "bus/midi/midi.h"

namespace {

// ======================> coco_midi_device

class coco_midi_device :
		public device_t,
		public device_cococart_interface
{
	public:
		// construction/destruction
		coco_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		coco_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
		virtual void device_start() override ATTR_COLD;

		// optional information overrides
		virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;
		required_device<acia6850_device> m_acia;
		void acia_irq_w(int state);

	private:
		required_device<midi_port_device> m_mdthru;
};

class dragon_midi_device : public coco_midi_device
{
	public:
		// construction/destruction
		dragon_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	protected:
		dragon_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock);
		virtual void device_start() override ATTR_COLD;
};

coco_midi_device::coco_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_cococart_interface(mconfig, *this )
		, m_acia(*this, "mc6850")
		, m_mdthru(*this, "mdthru")
{
}

coco_midi_device::coco_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: coco_midi_device(mconfig, COCO_MIDI, tag, owner, clock)
{
}

void coco_midi_device::device_add_mconfig(machine_config &config)
{
	ACIA6850(config, m_acia).txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	m_acia->irq_handler().set(FUNC(coco_midi_device::acia_irq_w));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_acia, FUNC(acia6850_device::write_rxd));
	mdin.rxd_handler().append(m_mdthru, FUNC(midi_port_device::write_txd));
	MIDI_PORT(config, m_mdthru, midiout_slot, "midiout");
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");

	clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
	acia_clock.signal_handler().set(m_acia, FUNC(acia6850_device::write_txc));
	acia_clock.signal_handler().append(m_acia, FUNC(acia6850_device::write_rxc));
}

void coco_midi_device::device_start()
{
	install_readwrite_handler(0xff6e, 0xff6f,
			read8sm_delegate(m_acia, FUNC(acia6850_device::read)),
			write8sm_delegate(m_acia, FUNC(acia6850_device::write)));
}

void coco_midi_device::acia_irq_w(int state)
{
	set_line_value(line::CART, state == 0);
}

dragon_midi_device::dragon_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
	: coco_midi_device(mconfig, type, tag, owner, clock)
{
}

dragon_midi_device::dragon_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: dragon_midi_device(mconfig, DRAGON_MIDI, tag, owner, clock)
{
}

void dragon_midi_device::device_start()
{
	install_readwrite_handler(0xff74, 0xff75,
			read8sm_delegate(m_acia, FUNC(acia6850_device::read)),
			write8sm_delegate(m_acia, FUNC(acia6850_device::write)));
}

} // anonymous namespace

DEFINE_DEVICE_TYPE_PRIVATE(COCO_MIDI, device_cococart_interface, coco_midi_device, "coco_midi", "CoCo MIDI PAK")
DEFINE_DEVICE_TYPE_PRIVATE(DRAGON_MIDI, device_cococart_interface, dragon_midi_device, "dragon_midi", "Dragon MIDI PAK")
