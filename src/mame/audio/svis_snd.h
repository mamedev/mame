// license:GPL-2.0+
// copyright-holders:Peter Trauner
/*****************************************************************************
 *
 *  svis_snd.h
 *
 ****************************************************************************/

#ifndef SVIS_SND_H_
#define SVIS_SND_H_


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

enum SVISION_NOISE_Type
{
	SVISION_NOISE_Type7Bit,
	SVISION_NOISE_Type14Bit
};

struct SVISION_NOISE
{
	SVISION_NOISE() :
		on(0),
		right(0),
		left(0),
		play(0),
		type(SVISION_NOISE_Type7Bit),
		state(0),
		volume(0),
		count(0),
		step(0.0),
		pos(0.0),
		value(0)
	{
		memset(reg, 0, sizeof(UINT8)*3);
	}

	UINT8 reg[3];
	int on, right, left, play;
	SVISION_NOISE_Type type;
	int state;
	int volume;
	int count;
	double step, pos;
	int value; // currently simple random function
};

struct SVISION_DMA
{
	SVISION_DMA() :
		on(0),
		right(0),
		left(0),
		ca14to16(0),
		start(0),
		size(0),
		pos(0.0),
		step(0.0),
		finished(0)
	{
		memset(reg, 0, sizeof(UINT8)*5);
	}

	UINT8 reg[5];
	int on, right, left;
	int ca14to16;
	int start,size;
	double pos, step;
	int finished;
};

struct SVISION_CHANNEL
{
	SVISION_CHANNEL() :
		on(0),
		waveform(0),
		volume(0),
		pos(0),
		size(0),
		count(0)
	{
		memset(reg, 0, sizeof(UINT8)*4);
	}

	UINT8 reg[4];
	int on;
	int waveform, volume;
	int pos;
	int size;
	int count;
};

typedef device_delegate<void (void)> svision_snd_irq_delegate;
#define SVISION_SND_IRQ_MEMBER(_name)   void _name(void)

#define SVISION_SND_IRQ_CB(_class, _method) \
	svision_sound_device::set_irq_callback(*device, svision_snd_irq_delegate(&_class::_method, #_class "::" #_method, downcast<_class *>(owner)));

// ======================> svision_sound_device

class svision_sound_device : public device_t,
								public device_sound_interface
{
public:
	svision_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~svision_sound_device() { }

	// static configuration
	static void set_irq_callback(device_t &device, svision_snd_irq_delegate callback) { downcast<svision_sound_device &>(device).m_irq_cb = callback; }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( sounddma_w );
	DECLARE_WRITE8_MEMBER( noise_w );

public:
	int *dma_finished();
	void sound_decrement();
	void soundport_w(int which, int offset, int data);

private:
	svision_snd_irq_delegate m_irq_cb;

	sound_stream *m_mixer_channel;
	SVISION_DMA m_dma;
	SVISION_NOISE m_noise;
	SVISION_CHANNEL m_channel[2];
};

extern const device_type SVISION_SND;


#endif /* SVIS_SND_H_ */
