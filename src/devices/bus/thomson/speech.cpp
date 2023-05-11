// license:BSD-3-Clause
// copyright-holders:AJR

#include "emu.h"
#include "speech.h"

#include "sound/mea8000.h"
#include "speaker.h"

// device type definition
DEFINE_DEVICE_TYPE(THOMSON_SPEECH, thomson_speech_device, "thomson_speech", "Cedic-Nathan Speech Synthesizer")

//-------------------------------------------------
//  thomson_speech_device - constructor
//-------------------------------------------------

thomson_speech_device::thomson_speech_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, THOMSON_SPEECH, tag, owner, clock)
	, thomson_extension_interface(mconfig, *this)
{
}

void thomson_speech_device::rom_map(address_map &map)
{
}

void thomson_speech_device::io_map(address_map &map)
{
	map(0x3e, 0x3f).rw("mea8000", FUNC(mea8000_device::read), FUNC(mea8000_device::write));
}

void thomson_speech_device::device_add_mconfig(machine_config &config)
{
	mea8000_device &mea8000(MEA8000(config, "mea8000", 4_MHz_XTAL));
	mea8000.add_route(ALL_OUTPUTS, "speaker", 1.0);

	// FIXME: actually drives main speaker through the sound line on the bus
	SPEAKER(config, "speaker").front_center();
}

void thomson_speech_device::device_start()
{
}
