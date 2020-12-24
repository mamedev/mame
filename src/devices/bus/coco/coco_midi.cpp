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

#define MC6850_TAG "mc6850"

namespace
{
    // ======================> coco_midi_device

    class coco_dragon_midi_device :
            public device_t,
            public device_cococart_interface
    {
    public:
        // construction/destruction
        coco_dragon_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

    protected:
        // device-level overrides
        virtual void device_reset() override;

        // optional information overrides
        virtual void device_add_mconfig(machine_config &config) override;

        required_device<acia6850_device> m_acia;

        // callbacks
        DECLARE_WRITE_LINE_MEMBER( acia_irq_w );

   private:
        DECLARE_WRITE_LINE_MEMBER( write_acia_clock );
        DECLARE_WRITE_LINE_MEMBER( midi_in );

        required_device<midi_port_device> m_mdthru;
    };

    class coco_midi_device : public coco_dragon_midi_device
    {
    	public:
    		using coco_dragon_midi_device::coco_dragon_midi_device;

    	protected:
			virtual void device_start() override;
    };

    class dragon_midi_device : public coco_dragon_midi_device
    {
    	public:
    		using coco_dragon_midi_device::coco_dragon_midi_device;

    	protected:
			virtual void device_start() override;
    };
};

DEFINE_DEVICE_TYPE_PRIVATE(COCO_MIDI, device_cococart_interface, coco_midi_device, "coco_midi", "CoCo MIDI PAK");
DEFINE_DEVICE_TYPE_PRIVATE(DRAGON_MIDI, device_cococart_interface, dragon_midi_device, "dragon_midi", "Dragon MIDI PAK");

void coco_dragon_midi_device::device_add_mconfig(machine_config &config)
{
    ACIA6850(config, m_acia).txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
    m_acia->irq_handler().set(FUNC(coco_dragon_midi_device::acia_irq_w));

    MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set(FUNC(coco_dragon_midi_device::midi_in));
    MIDI_PORT(config, m_mdthru, midiout_slot, "midiout");
    MIDI_PORT(config, "mdout", midiout_slot, "midiout");

    clock_device &acia_clock(CLOCK(config, "acia_clock", 31250*16));
    acia_clock.signal_handler().set(FUNC(coco_dragon_midi_device::write_acia_clock));
}

coco_dragon_midi_device::coco_dragon_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
        : device_t(mconfig, COCO_MIDI, tag, owner, clock)
        , device_cococart_interface(mconfig, *this )
        , m_acia(*this, MC6850_TAG)
        , m_mdthru(*this, "mdthru")
{
}

void coco_midi_device::device_start()
{
    install_readwrite_handler(0xff6e, 0xff6f,
            read8sm_delegate(m_acia, FUNC(acia6850_device::read)),
            write8sm_delegate(m_acia, FUNC(acia6850_device::write)));
}

void dragon_midi_device::device_start()
{
    install_readwrite_handler(0xff74, 0xff75,
            read8sm_delegate(m_acia, FUNC(acia6850_device::read)),
            write8sm_delegate(m_acia, FUNC(acia6850_device::write)));
}

void coco_dragon_midi_device::device_reset()
{
    m_acia->reset();
}

WRITE_LINE_MEMBER( coco_dragon_midi_device::write_acia_clock )
{
    m_acia->write_txc(state);
    m_acia->write_rxc(state);
}

WRITE_LINE_MEMBER(coco_dragon_midi_device::midi_in)
{
    // MIDI in signals is sent to both the 6850 and the MIDI thru output port
    m_acia->write_rxd(state);
    m_mdthru->write_txd(state);
}

WRITE_LINE_MEMBER(coco_dragon_midi_device::acia_irq_w)
{
	set_line_value(line::CART, state != 0);
}
