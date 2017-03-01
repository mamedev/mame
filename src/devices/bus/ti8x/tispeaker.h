// license:BSD-3-Clause
// copyright-holders:Vas Crabb
#ifndef MAME_DEVICES_BUS_TI8X_TISPEAKER_H
#define MAME_DEVICES_BUS_TI8X_TISPEAKER_H

#pragma once

#include "ti8x.h"
#include "sound/spkrdev.h"


extern device_type const TI8X_SPEAKER_STEREO;
extern device_type const TI8X_SPEAKER_MONO;


namespace bus { namespace ti8x {

class stereo_speaker_device : public device_t, public device_ti8x_link_port_interface
{
public:
	stereo_speaker_device(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock);

protected:
	virtual machine_config_constructor device_mconfig_additions() const override;
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
	virtual machine_config_constructor device_mconfig_additions() const override;
	virtual void device_start() override;

	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override;
	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override;

	required_device<speaker_sound_device> m_speaker;

private:
	bool m_tip_state, m_ring_state;
};

} } // namespace bus::ti8x

#endif // MAME_DEVICES_BUS_TI8X_TISPEAKER_H
