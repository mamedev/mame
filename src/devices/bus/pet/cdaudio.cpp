// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rainbow Arts CD audio to datassette port adapter

    Passive adapter from a CD player's line output to the datassette
    port, used with the Rainbow Arts 1st CD-Edition: an RCA jack on the
    left channel and a coupling capacitor feeding an MC14069UB inverter
    chain that squares the audio onto READ. MOTOR is not connected and
    SENSE is tied low, so PLAY always reads as pressed. The CD player is
    operated by hand.

**********************************************************************/

#include "emu.h"
#include "cdaudio.h"

#include <algorithm>
#include <cmath>


DEFINE_DEVICE_TYPE(PET_CD_ADAPTER, cd_audio_adapter_device, "pet_cdaudio", "CD audio datassette adapter")


cd_audio_adapter_device::cd_audio_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_CD_ADAPTER, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_pet_datassette_port_interface(mconfig, *this),
	device_cdda_player_interface(mconfig, *this),
	m_edge_timer(nullptr),
	m_comparator(512),
	m_read(1)
{
}

void cd_audio_adapter_device::device_add_mconfig(machine_config &config)
{
	add_cd_player(config);
}

void cd_audio_adapter_device::device_start()
{
	stream_alloc(2, 0, SAMPLE_RATE_INPUT_ADAPTIVE, STREAM_SYNCHRONOUS);
	m_edge_timer = timer_alloc(FUNC(cd_audio_adapter_device::read_edge), this);

	m_comparator.register_save(*this);
	save_item(NAME(m_read));
}

void cd_audio_adapter_device::sound_stream_update(sound_stream &stream)
{
	// every edge is delayed by the same comparator latency so it can land at its interpolated zero crossing
	for (int i = 0; i < stream.samples(); i++)
	{
		double position;
		if (m_comparator.update(std::lround(stream.get(0, i) * 32768.0f), position))
		{
			double const delay = std::max(position + i + 1 - stream.samples(), 0.0);
			m_edge_timer->adjust(attotime(0, attoseconds_t(delay * HZ_TO_ATTOSECONDS(stream.sample_rate()))), m_comparator.state() ? 0 : 1);
		}
	}
}

TIMER_CALLBACK_MEMBER(cd_audio_adapter_device::read_edge)
{
	m_read = param;
	m_slot->read_w(m_read);
}
