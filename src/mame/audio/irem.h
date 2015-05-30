// license:BSD-3-Clause
// copyright-holders:Couriersud
#include "sound/ay8910.h"
#include "sound/msm5205.h"

class irem_audio_device : public device_t,
									public device_sound_interface
{
public:
	irem_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~irem_audio_device() {}

	DECLARE_WRITE8_MEMBER( cmd_w );
	DECLARE_WRITE8_MEMBER( m6803_port1_w );
	DECLARE_WRITE8_MEMBER( m6803_port2_w );
	DECLARE_READ8_MEMBER( m6803_port1_r );
	DECLARE_READ8_MEMBER( m6803_port2_r );
	DECLARE_WRITE8_MEMBER( ay8910_0_portb_w );
	DECLARE_WRITE8_MEMBER( ay8910_1_porta_w );
	DECLARE_WRITE8_MEMBER( sound_irq_ack_w );
	DECLARE_WRITE8_MEMBER( m52_adpcm_w );
	DECLARE_WRITE8_MEMBER( m62_adpcm_w );

	void adpcm_int(int st);

protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	UINT8           m_port1;
	UINT8           m_port2;

	ay8910_device *m_ay1;
	ay8910_device *m_ay2;
	msm5205_device *m_adpcm1;
	msm5205_device *m_adpcm2;
};

MACHINE_CONFIG_EXTERN( m52_sound_c_audio );
MACHINE_CONFIG_EXTERN( m52_large_audio );
MACHINE_CONFIG_EXTERN( m62_audio );

extern const device_type IREM_AUDIO;
