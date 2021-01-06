// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_BUS_TI8X_TISPEAKER_H
#define MAME_DEVICES_BUS_TI8X_TISPEAKER_H

#pragma once

#include "ti8x.h"
#include "sound/spkrdev.h"


namespace bus::ti8x {

class stereo_speaker_device : public device_t, public device_ti8x_link_port_interface
{
public:
	stereo_speaker_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override;
	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override;

	required_device<speaker_sound_device> m_left_speaker;
	required_device<speaker_sound_device> m_right_speaker;
};


class mono_speaker_device : public device_t, public device_ti8x_link_port_interface
{
public:
	mono_speaker_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual void device_add_mconfig(machine_config &config) override;
	virtual void device_start() override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override;
	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override;

	required_device<speaker_sound_device> m_speaker;

private:
	bool m_tip_state, m_ring_state;
};

} // namespace bus::ti8x


DECLARE_DEVICE_TYPE_NS(TI8X_SPEAKER_STEREO, bus::ti8x, stereo_speaker_device)
DECLARE_DEVICE_TYPE_NS(TI8X_SPEAKER_MONO,   bus::ti8x, mono_speaker_device)

#endif // MAME_DEVICES_BUS_TI8X_TISPEAKER_H
