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


DEFINE_DEVICE_TYPE(VCS_CD_ADAPTER, vcs_cd_adapter_device, "vcs_cdgames", "CD Games Pack adapter")


vcs_cd_adapter_device::vcs_cd_adapter_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, VCS_CD_ADAPTER, tag, owner, clock),
	device_vcs_control_port_interface(mconfig, *this),
	m_cd(*this, "cdrom"),
	m_edge_timer{ nullptr, nullptr },
	m_comparator{ { 512 }, { 512 } },
	m_joy(0xff)
{
}

void vcs_cd_adapter_device::device_add_mconfig(machine_config &config)
{
	CD_PLAYER(config, m_cd);
	m_cd->set_interface("cdrom");
	m_cd->set_sample_callback(FUNC(vcs_cd_adapter_device::sample_w));
}

void vcs_cd_adapter_device::device_start()
{
	m_edge_timer[0] = timer_alloc(FUNC(vcs_cd_adapter_device::line_edge), this);
	m_edge_timer[1] = timer_alloc(FUNC(vcs_cd_adapter_device::line_edge), this);

	m_comparator[0].register_save(*this, 0);
	m_comparator[1].register_save(*this, 1);
	save_item(NAME(m_joy));
}

void vcs_cd_adapter_device::sample_w(s16 left, s16 right)
{
	static constexpr uint8_t LINE[2] = { LINE_FIRE, LINE_UP };
	s16 const sample[2] = { left, right };

	for (int i = 0; i < 2; i++)
	{
		double position;
		if (m_comparator[i].update(sample[i], position))
			m_edge_timer[i]->adjust(attotime(0, attoseconds_t(position * HZ_TO_ATTOSECONDS(cd_player_device::SAMPLE_RATE))), (m_comparator[i].state() ? EDGE_HIGH : 0) | LINE[i]);
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
