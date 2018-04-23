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


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

#define SVISION_SND_IRQ_MEMBER(_name)   void _name(void)

#define SVISION_SND_IRQ_CB(_class, _method) \
	downcast<svision_sound_device &>(*device).set_irq_callback(svision_sound_device::irq_delegate(&_class::_method, #_class "::" #_method, this));

// ======================> svision_sound_device

class svision_sound_device : public device_t, public device_sound_interface
{
public:
	typedef device_delegate<void ()> irq_delegate;

	svision_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	template<typename Object> void set_irq_callback(Object &&callback) { m_irq_cb = std::forward<Object>(callback); }

	DECLARE_WRITE8_MEMBER( sounddma_w );
	DECLARE_WRITE8_MEMBER( noise_w );

	int *dma_finished();
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

	irq_delegate m_irq_cb;

	sound_stream *m_mixer_channel;
	DMA m_dma;
	NOISE m_noise;
	CHANNEL m_channel[2];
};

DECLARE_DEVICE_TYPE(SVISION_SND, svision_sound_device)

#endif // MAME_AUDIO_SVIS_SND_H
