// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "sound/vlm5030.h"
#include "cpu/m6800/m6800.h"

class trackfld_audio_device : public device_t,
									public device_sound_interface
{
public:
	trackfld_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~trackfld_audio_device() {}

	DECLARE_WRITE8_MEMBER(konami_sh_irqtrigger_w );
	DECLARE_READ8_MEMBER(trackfld_sh_timer_r );
	DECLARE_READ8_MEMBER(trackfld_speech_r );
	DECLARE_WRITE8_MEMBER(trackfld_sound_w );
	DECLARE_READ8_MEMBER(hyperspt_sh_timer_r );
	DECLARE_WRITE8_MEMBER(hyperspt_sound_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	int      m_last_addr;
	int      m_last_irq;

	cpu_device *m_audiocpu;
	vlm5030_device *m_vlm;
};

extern const device_type TRACKFLD_AUDIO;
