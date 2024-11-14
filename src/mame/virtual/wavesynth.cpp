// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// A "virtual" driver to turn waveblaster cards into a screen-less expander

#include "emu.h"
#include "speaker.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"
#include "bus/waveblaster/waveblaster.h"


namespace {

class wavesynth_state : public driver_device
{
public:
	wavesynth_state(const machine_config &mconfig, device_type type, const char *tag);

	void wavesynth(machine_config &config);


private:
	virtual void machine_start() override ATTR_COLD;

	required_device<waveblaster_connector> m_waveblaster;
};


wavesynth_state::wavesynth_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_waveblaster(*this, "waveblaster")
{
}

void wavesynth_state::machine_start()
{
}

void wavesynth_state::wavesynth(machine_config &config)
{
	WAVEBLASTER_CONNECTOR(config, m_waveblaster, waveblaster_intf, "omniwave");
	m_waveblaster->add_route(0, "lspeaker", 1.0);
	m_waveblaster->add_route(1, "rspeaker", 1.0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_waveblaster, FUNC(waveblaster_connector::midi_rx));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_waveblaster->midi_tx().set(mdout, FUNC(midi_port_device::write_txd));
}

static INPUT_PORTS_START( wavesynth )
INPUT_PORTS_END

ROM_START( wavesynth )
ROM_END


} // anonymous namespace


CONS( 2020, wavesynth, 0, 0, wavesynth, wavesynth, wavesynth_state, empty_init, "MAME", "Waveblaster-based expander", MACHINE_SUPPORTS_SAVE )
