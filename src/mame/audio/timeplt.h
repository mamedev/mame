// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "sound/flt_rc.h"

class timeplt_audio_device : public device_t,
									public device_sound_interface
{
public:
	timeplt_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~timeplt_audio_device() {}

	DECLARE_WRITE8_MEMBER( sh_irqtrigger_w );
	DECLARE_WRITE8_MEMBER( filter_w );
	DECLARE_READ8_MEMBER( portB_r );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	UINT8    m_last_irq_state;
	cpu_device *m_soundcpu;

	device_t *m_filter_0_0;
	device_t *m_filter_0_1;
	device_t *m_filter_0_2;
	device_t *m_filter_1_0;
	device_t *m_filter_1_1;
	device_t *m_filter_1_2;

	void filter_w( device_t *device, int data );
};

MACHINE_CONFIG_EXTERN( timeplt_sound );
MACHINE_CONFIG_EXTERN( locomotn_sound );

extern const device_type TIMEPLT_AUDIO;
