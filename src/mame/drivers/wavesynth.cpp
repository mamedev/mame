// license:BSD-3-Clause
// copyright-holders: Olivier Galibert

// A "virtual" driver to turn waveblaster cards into a screen-less expander

// Currently KS0164 only, and built-in.  Evetually should be slot-based with
// multiple possible cards

#include "emu.h"
#include "speaker.h"
#include "bus/midi/midiinport.h"
#include "bus/midi/midioutport.h"

#include "sound/ks0164.h"

class wavesynth_state : public driver_device
{
public:
	wavesynth_state(const machine_config &mconfig, device_type type, const char *tag);

	void wavesynth(machine_config &config);


private:
	virtual void machine_start() override;

	required_device<ks0164_device> m_waveblaster;
};


wavesynth_state::wavesynth_state(const machine_config &mconfig, device_type type, const char *tag)
	: driver_device(mconfig, type, tag)
	, m_waveblaster(*this, "waveblaster")
{
}

void wavesynth_state::machine_start()
{
}

static INPUT_PORTS_START( wavesynth )
INPUT_PORTS_END


void wavesynth_state::wavesynth(machine_config &config)
{
	KS0164(config, m_waveblaster, 16.9344_MHz_XTAL);
	m_waveblaster->add_route(0, "lspeaker", 1.0);
	m_waveblaster->add_route(1, "rspeaker", 1.0);

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	auto &mdin(MIDI_PORT(config, "mdin"));
	midiin_slot(mdin);
	mdin.rxd_handler().set(m_waveblaster, FUNC(ks0164_device::midi_rx));

	auto &mdout(MIDI_PORT(config, "mdout"));
	midiout_slot(mdout);
	m_waveblaster->midi_tx().set(mdout, FUNC(midi_port_device::write_txd));
}

ROM_START( wavesynth )
	ROM_REGION( 0x100000, "waveblaster", 0)
	ROM_LOAD16_WORD_SWAP("ks0174-1m04.bin", 0, 0x100000, CRC(3cabaa2f) SHA1(1e894c0345eaf0ea713f36a75b065f7ee419c63c))
ROM_END

CONS( 2020, wavesynth, 0, 0, wavesynth, wavesynth, wavesynth_state, empty_init, "MAME", "Waveblaster-based expander", MACHINE_SUPPORTS_SAVE )
