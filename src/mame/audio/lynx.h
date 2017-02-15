// license:GPL-2.0+
// copyright-holders:Peter Trauner

#ifndef __LYNXSOUND_H__
#define __LYNXSOUND_H__

struct LYNX_AUDIO {
	struct {
		int8_t volume;
		uint8_t feedback;
		int8_t output;
		uint8_t shifter;
		uint8_t bakup;
		uint8_t control1;
		uint8_t counter;
		uint8_t control2;
	} reg;
	uint8_t attenuation;
	uint16_t mask; // 12-bit
	uint16_t shifter; // 12-bit
	float ticks;
	int count;
};

typedef device_delegate<void (void)> lynx_sound_timer_delegate;


class lynx_sound_device : public device_t,
									public device_sound_interface
{
public:
	lynx_sound_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, uint32_t clock, const char *shortname, const char *source);
	lynx_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER(read);
	DECLARE_WRITE8_MEMBER(write);
	void count_down(int nr);
	static void set_timer_delegate(device_t &device, lynx_sound_timer_delegate cb);

protected:
	// device-level overrides
	virtual void device_start() override;

	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	void reset_channel(LYNX_AUDIO *channel);
	void shift(int chan_nr);
	void execute(int chan_nr);
	void init();
	void register_save();

	sound_stream *m_mixer_channel;
	lynx_sound_timer_delegate   m_timer_delegate;   // this calls lynx_timer_count_down from the driver state

	float m_usec_per_sample;
	std::unique_ptr<int[]> m_shift_mask;
	std::unique_ptr<int[]> m_shift_xor;
	uint8_t m_attenuation_enable;
	uint8_t m_master_enable;
	LYNX_AUDIO m_audio[4];
};


class lynx2_sound_device : public lynx_sound_device
{
public:
	lynx2_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
};


extern const device_type LYNX_SND;
extern const device_type LYNX2_SND;


#define MCFG_LYNX_SND_SET_TIMER( _class, _method) \
	lynx_sound_device::set_timer_delegate(*device, lynx_sound_timer_delegate(&_class::_method, #_class "::" #_method, nullptr, (_class *)nullptr));


#endif
