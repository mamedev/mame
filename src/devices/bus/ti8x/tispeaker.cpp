// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "tispeaker.h"

#include "speaker.h"


DEFINE_DEVICE_TYPE_NS(TI8X_SPEAKER_STEREO, bus::ti8x, stereo_speaker_device, "ti8x_stspkr", "TI-8x Speaker (Stereo)")
DEFINE_DEVICE_TYPE_NS(TI8X_SPEAKER_MONO,   bus::ti8x, mono_speaker_device,   "ti8x_mspkr",  "TI-8x Speaker (Mono)")


namespace bus { namespace ti8x {

namespace {

MACHINE_CONFIG_START(ti_speaker_stereo)
	MCFG_SPEAKER_STANDARD_STEREO("outl", "outr")

	MCFG_SOUND_ADD("lspkr", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "outl", 0.50)

	MCFG_SOUND_ADD("rspkr", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "outr", 0.50)
MACHINE_CONFIG_END


MACHINE_CONFIG_START(ti_speaker_mono)
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("spkr", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)
MACHINE_CONFIG_END

} // anonymous namespace


stereo_speaker_device::stereo_speaker_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_SPEAKER_STEREO, tag, owner, clock)
	, device_ti8x_link_port_interface(mconfig, *this)
	, m_left_speaker(*this, "lspkr")
	, m_right_speaker(*this, "rspkr")
{
}


machine_config_constructor stereo_speaker_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(ti_speaker_stereo);
}


void stereo_speaker_device::device_start()
{
}


WRITE_LINE_MEMBER(stereo_speaker_device::input_tip)
{
	m_left_speaker->level_w(state);
}


WRITE_LINE_MEMBER(stereo_speaker_device::input_ring)
{
	m_right_speaker->level_w(state);
}



mono_speaker_device::mono_speaker_device(
		machine_config const &mconfig,
		char const *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, TI8X_SPEAKER_MONO, tag, owner, clock)
	, device_ti8x_link_port_interface(mconfig, *this)
	, m_speaker(*this, "spkr")
	, m_tip_state(true)
	, m_ring_state(true)
{
}


machine_config_constructor mono_speaker_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME(ti_speaker_mono);
}


void mono_speaker_device::device_start()
{
	save_item(NAME(m_tip_state));
	save_item(NAME(m_ring_state));

	m_tip_state = m_ring_state = true;
}


WRITE_LINE_MEMBER(mono_speaker_device::input_tip)
{
	m_tip_state = bool(state);
	m_speaker->level_w((m_tip_state || m_ring_state) ? 1 : 0);
}


WRITE_LINE_MEMBER(mono_speaker_device::input_ring)
{
	m_ring_state = bool(state);
	m_speaker->level_w((m_tip_state || m_ring_state) ? 1 : 0);
}

} } // namespace bus::ti8x
