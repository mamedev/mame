// license:BSD-3-Clause
// copyright-holders:AJR
/***************************************************************************

    Logimus Midipak

    This MIDI extension interface basically consists of a EF68B50P plus
    some glue logic. An onboard jumper (JP1) can connect the 6850's IRQ
    output to either IRQ or FIRQ; currently only the IRQ connection is
    supported.

***************************************************************************/

#include "emu.h"
#include "midipak.h"

#include "bus/midi/midi.h"
#include "machine/6850acia.h"
#include "machine/clock.h"

// device type definition
DEFINE_DEVICE_TYPE(LOGIMUS_MIDIPAK, logimus_midipak_device, "logimus_midipak", "Logimus Midipak")

//-------------------------------------------------
//  logimus_midipak_device - constructor
//-------------------------------------------------

logimus_midipak_device::logimus_midipak_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, LOGIMUS_MIDIPAK, tag, owner, clock)
	, thomson_extension_interface(mconfig, *this)
{
}

void logimus_midipak_device::rom_map(address_map &map)
{
}

void logimus_midipak_device::io_map(address_map &map)
{
	map(0x32, 0x33).rw("acia", FUNC(acia6850_device::read), FUNC(acia6850_device::write));
}

void logimus_midipak_device::device_add_mconfig(machine_config &config)
{
	acia6850_device &acia(ACIA6850(config, "acia"));
	acia.txd_handler().set("mdout", FUNC(midi_port_device::write_txd));
	acia.irq_handler().set(FUNC(logimus_midipak_device::irq_w));
	acia.write_cts(0);
	acia.write_dcd(0);

	clock_device &midiclock(CLOCK(config, "midiclock", DERIVED_CLOCK(1, 1)));
	midiclock.signal_handler().set("acia", FUNC(acia6850_device::write_rxc));
	midiclock.signal_handler().append("acia", FUNC(acia6850_device::write_txc));

	MIDI_PORT(config, "mdin", midiin_slot, "midiin").rxd_handler().set("acia", FUNC(acia6850_device::write_rxd));
	MIDI_PORT(config, "mdout", midiout_slot, "midiout");
}

void logimus_midipak_device::device_start()
{
}
