// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
#ifndef MAME_SOUND_K005289_H
#define MAME_SOUND_K005289_H

#pragma once


// ======================> k005289_device

class k005289_device : public device_t,
						public device_sound_interface
{
public:
	k005289_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock);

	void control_A_w(u8 data);
	void control_B_w(u8 data);
	void ld1_w(offs_t offset, u8 data);
	void ld2_w(offs_t offset, u8 data);
	void tg1_w(u8 data);
	void tg2_w(u8 data);

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, std::vector<read_stream_view> const &inputs, std::vector<write_stream_view> &outputs) override;

private:
	required_region_ptr<u8> m_sound_prom;
	sound_stream *m_stream;

	struct voice_t
	{
		voice_t() { reset(); }

		void reset()
		{
			counter = 0;
			frequency = 0;
			pitch = 0;
			waveform = 0;
			volume = 0;
		}

		s16 counter;
		u16 frequency;
		u16 pitch;
		u16 waveform;
		u8 volume;
	};
	voice_t m_voice[2];
};

DECLARE_DEVICE_TYPE(K005289, k005289_device)

#endif // MAME_SOUND_K005289_H
