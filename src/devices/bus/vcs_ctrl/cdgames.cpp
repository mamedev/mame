// license:BSD-3-Clause
// copyright-holders:Curt Coder
/**********************************************************************

    Codemasters CD Games Pack CD audio to joystick port adapter

    Two-channel adapter from a CD player's line output to a joystick
    port, used with The Codemasters CD Games Pack: the left channel
    is a clock on the fire button line and the right channel is data
    on the up line, each squared at its zero crossings. The CD player
    is operated by hand.

**********************************************************************/

#include "emu.h"
#include "cdgames.h"

#include <algorithm>
#include <cmath>


DEFINE_DEVICE_TYPE(VCS_CD_ADAPTER, vcs_cd_adapter_device, "vcs_cdgames", "CD Games Pack adapter")


vcs_cd_adapter_device::vcs_cd_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_CD_ADAPTER, tag, owner, clock),
	device_sound_interface(mconfig, *this),
	device_vcs_control_port_interface(mconfig, *this),
	device_cdda_player_interface(mconfig, *this),
	m_edge_timer{ nullptr, nullptr },
	m_comparator{ { 512 }, { 512 } },
	m_joy(0xff)
{
}

void vcs_cd_adapter_device::device_add_mconfig(machine_config &config)
{
	add_cd_player(config);
}

void vcs_cd_adapter_device::device_start()
{
	stream_alloc(2, 0, SAMPLE_RATE_INPUT_ADAPTIVE, STREAM_SYNCHRONOUS);
	m_edge_timer[0] = timer_alloc(FUNC(vcs_cd_adapter_device::line_edge), this);
	m_edge_timer[1] = timer_alloc(FUNC(vcs_cd_adapter_device::line_edge), this);

	m_comparator[0].register_save(*this, 0);
	m_comparator[1].register_save(*this, 1);
	save_item(NAME(m_joy));
}

void vcs_cd_adapter_device::sound_stream_update(sound_stream &stream)
{
	static constexpr uint8_t LINE[2] = { LINE_FIRE, LINE_UP };

	for (int i = 0; i < stream.samples(); i++)
	{
		for (int ch = 0; ch < 2; ch++)
		{
			double position;
			if (m_comparator[ch].update(std::lround(stream.get(ch, i) * 32768.0f), position))
			{
				double const delay = std::max(position + i + 1 - stream.samples(), 0.0);
				m_edge_timer[ch]->adjust(attotime(0, attoseconds_t(delay * HZ_TO_ATTOSECONDS(stream.sample_rate()))), (m_comparator[ch].state() ? EDGE_HIGH : 0) | LINE[ch]);
			}
		}
	}
}

TIMER_CALLBACK_MEMBER(vcs_cd_adapter_device::line_edge)
{
	uint8_t const line = param & 0xff;

	if (param & EDGE_HIGH)
		m_joy |= line;
	else
		m_joy &= ~line;

	if (line == LINE_FIRE)
		trigger_w(BIT(m_joy, 5));
}
