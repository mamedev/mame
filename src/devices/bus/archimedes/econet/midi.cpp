// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    The Serial Port MIDI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/TheSerialPort_MIDI.html

    The Serial Port Sampler and MIDI Interface

    http://chrisacorns.computinghistory.org.uk/32bit_UpgradesH2Z/TheSerialPort_SamplerMIDI.html

    TODO:
    - add sampler using ZN439E

**********************************************************************/

#include "emu.h"
#include "midi.h"

#include "bus/midi/midi.h"
#include "machine/mc6854.h"

#include "speaker.h"


namespace {

// ======================> arc_serial_midi_device

class arc_serial_midi_device : public device_t, public device_archimedes_econet_interface
{
public:
	// construction/destruction
	arc_serial_midi_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: arc_serial_midi_device(mconfig, ARC_SERIAL_MIDI, tag, owner, clock)
	{
	}

protected:
	arc_serial_midi_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, u32 clock)
		: device_t(mconfig, type, tag, owner, clock)
		, device_archimedes_econet_interface(mconfig, *this)
		, m_adlc(*this, "mc6854")
	{
	}

	// device_t overrides
	virtual void device_start() override ATTR_COLD { }

	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

	virtual u8 read(offs_t offset) override
	{
		return m_adlc->read(offset);
	}

	virtual void write(offs_t offset, u8 data) override
	{
		m_adlc->write(offset, data);
	}

private:
	required_device<mc6854_device> m_adlc;
};


// ======================> arc_serial_sampler_device

class arc_serial_sampler_device: public arc_serial_midi_device
{
public:
	// construction/destruction
	arc_serial_sampler_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
		: arc_serial_midi_device(mconfig, ARC_SERIAL_SAMPLER, tag, owner, clock)
		, m_mic(*this, "mic")
	{
	}

	static constexpr feature_type unemulated_features() { return feature::CAPTURE; }

protected:
	// optional information overrides
	virtual void device_add_mconfig(machine_config &config) override ATTR_COLD;

private:
	required_device<microphone_device> m_mic;
};


//-------------------------------------------------
//  device_add_mconfig - add device configuration
//-------------------------------------------------

void arc_serial_midi_device::device_add_mconfig(machine_config &config)
{
	MC6854(config, m_adlc);
	m_adlc->out_txd_cb().set("mdout", FUNC(midi_port_device::write_txd));
	m_adlc->out_irq_cb().set(DEVICE_SELF_OWNER, FUNC(archimedes_econet_slot_device::efiq_w));

	midi_port_device &mdin(MIDI_PORT(config, "mdin", midiin_slot, "midiin"));
	mdin.rxd_handler().set(m_adlc, FUNC(mc6854_device::set_rx));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

void arc_serial_sampler_device::device_add_mconfig(machine_config &config)
{
	arc_serial_midi_device::device_add_mconfig(config);

	MICROPHONE(config, m_mic, 1).front_center();
	//m_mic->add_route(0, "adc", 1.0);

	// ZN439E
}

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(ARC_SERIAL_MIDI, device_archimedes_econet_interface, arc_serial_midi_device, "arc_serial_midi", "The Serial Port MIDI Interface");
DEFINE_DEVICE_TYPE_PRIVATE(ARC_SERIAL_SAMPLER, device_archimedes_econet_interface, arc_serial_sampler_device, "arc_serial_sampler", "The Serial Port Sampler and MIDI Interface");
