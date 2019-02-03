// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *  svis_snd.h
 *
 ****************************************************************************/

#ifndef MAME_AUDIO_SVIS_SND_H
#define MAME_AUDIO_SVIS_SND_H

#pragma once


// ======================> svision_sound_device

class svision_sound_device : public device_t, public device_sound_interface
{
public:
	template <typename T, typename U>
	svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock, T &&cpu_tag, U &&region_tag)
		: svision_sound_device(mconfig, tag, owner, clock)
	{
		m_maincpu.set_tag(std::forward<T>(cpu_tag));
		m_cartrom.set_tag(std::forward<U>(region_tag));
	}

	svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	auto irq_cb() { return m_irq_cb.bind(); }

	DECLARE_WRITE8_MEMBER( sounddma_w );
	DECLARE_WRITE8_MEMBER( noise_w );

	void sound_decrement();
	void soundport_w(int which, int offset, int data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	struct NOISE
	{
		enum class Type
		{
			Type7Bit = 0,
			Type14Bit = 1
		};

		NOISE() : reg{ 0, 0, 0 } { }

		uint8_t reg[3];
		int on = 0, right = 0, left = 0, play = 0;
		Type type = Type::Type7Bit;
		int state = 0;
		int volume = 0;
		int count = 0;
		double step = 0, pos = 0;
		int value = 0; // currently simple random function
	};

	struct DMA
	{
		DMA() : reg{ 0, 0, 0, 0, 0 } { }

		uint8_t reg[5];
		int on = 0, right = 0, left = 0;
		int ca14to16 = 0;
		int start = 0, size = 0;
		double pos = 0, step = 0;
		int finished = 0;
	};

	struct CHANNEL
	{
		CHANNEL() : reg{ 0, 0, 0, 0 } { }

		uint8_t reg[4];
		int on = 0;
		int waveform = 0, volume = 0;
		int pos = 0;
		int size = 0;
		int count = 0;
	};

	devcb_write_line m_irq_cb;

	required_device<cpu_device> m_maincpu;
	required_memory_bank m_cartrom;

	sound_stream *m_mixer_channel;
	DMA m_dma;
	NOISE m_noise;
	CHANNEL m_channel[2];
};

DECLARE_DEVICE_TYPE(SVISION_SND, svision_sound_device)

#endif // MAME_AUDIO_SVIS_SND_H
