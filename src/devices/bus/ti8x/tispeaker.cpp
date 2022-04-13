// license:BSD-3-Clause
// copyright-holders:Vas Crabb

#include "emu.h"
#include "tispeaker.h"

#include "sound/spkrdev.h"

#include "speaker.h"


namespace {

class stereo_speaker_device : public device_t, public device_ti8x_link_port_interface
{
public:
	stereo_speaker_device(
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

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		SPEAKER(config, "outl").front_left();
		SPEAKER(config, "outr").front_right();

		SPEAKER_SOUND(config, m_left_speaker, 0).add_route(ALL_OUTPUTS, "outl", 0.50);

		SPEAKER_SOUND(config, m_right_speaker, 0).add_route(ALL_OUTPUTS, "outr", 0.50);
	}

	virtual void device_start() override
	{
	}


	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override
	{
		m_left_speaker->level_w(state);
	}

	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override
	{
		m_right_speaker->level_w(state);
	}


	required_device<speaker_sound_device> m_left_speaker;
	required_device<speaker_sound_device> m_right_speaker;
};


class mono_speaker_device : public device_t, public device_ti8x_link_port_interface
{
public:
	mono_speaker_device(
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

protected:
	virtual void device_add_mconfig(machine_config &config) override
	{
		SPEAKER(config, "mono").front_center();

		SPEAKER_SOUND(config, m_speaker, 0).add_route(ALL_OUTPUTS, "mono", 0.50);
	}

	virtual void device_start() override
	{
		save_item(NAME(m_tip_state));
		save_item(NAME(m_ring_state));

		m_tip_state = m_ring_state = true;
	}


	virtual DECLARE_WRITE_LINE_MEMBER(input_tip) override
	{
		m_tip_state = bool(state);
		m_speaker->level_w((m_tip_state || m_ring_state) ? 1 : 0);
	}

	virtual DECLARE_WRITE_LINE_MEMBER(input_ring) override
	{
		m_ring_state = bool(state);
		m_speaker->level_w((m_tip_state || m_ring_state) ? 1 : 0);
	}


	required_device<speaker_sound_device> m_speaker;

private:
	bool m_tip_state, m_ring_state;
};

} // anonymous namespace


DEFINE_DEVICE_TYPE_PRIVATE(TI8X_SPEAKER_STEREO, device_ti8x_link_port_interface, stereo_speaker_device, "ti8x_stspkr", "TI-8x Speaker (Stereo)")
DEFINE_DEVICE_TYPE_PRIVATE(TI8X_SPEAKER_MONO,   device_ti8x_link_port_interface, mono_speaker_device,   "ti8x_mspkr",  "TI-8x Speaker (Mono)")
