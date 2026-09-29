// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Rainbow Arts CD audio to datassette port adapter

    Passive adapter from a CD player's line output to the datassette
    port, used with the Rainbow Arts 1st CD-Edition: an RCA jack and
    coupling capacitor feeding an MC14069UB inverter chain that
    squares the audio onto READ. MOTOR is not connected and SENSE is
    tied low, so PLAY always reads as pressed. The CD player is
    operated by hand.

**********************************************************************/

#include "emu.h"
#include "cdaudio.h"


DEFINE_DEVICE_TYPE(PET_CD_ADAPTER, cd_audio_adapter_device, "pet_cdaudio", "CD audio datassette adapter")


cd_audio_adapter_device::cd_audio_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, PET_CD_ADAPTER, tag, owner, clock),
	device_pet_datassette_port_interface(mconfig, *this),
	m_cd(*this, "cdrom"),
	m_edge_timer(nullptr),
	m_comparator(512),
	m_read(1)
{
}

void cd_audio_adapter_device::device_add_mconfig(machine_config &config)
{
	CD_PLAYER(config, m_cd);
	m_cd->set_interface("cdrom");
	m_cd->set_sample_callback(FUNC(cd_audio_adapter_device::sample_w));
}

void cd_audio_adapter_device::device_start()
{
	m_edge_timer = timer_alloc(FUNC(cd_audio_adapter_device::read_edge), this);

	m_comparator.register_save(*this);
	save_item(NAME(m_read));
}

void cd_audio_adapter_device::sample_w(s16 left, s16 right)
{
	// every edge is delayed by the same comparator latency so it can land at its interpolated zero crossing
	double position;
	if (m_comparator.update((s32(left) + s32(right)) / 2, position))
		m_edge_timer->adjust(attotime(0, attoseconds_t(position * HZ_TO_ATTOSECONDS(cd_player_device::SAMPLE_RATE))), m_comparator.state() ? 0 : 1);
}

TIMER_CALLBACK_MEMBER(cd_audio_adapter_device::read_edge)
{
	m_read = param;
	m_slot->read_w(m_read);
}
