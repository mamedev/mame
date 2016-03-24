// license:BSD-3-Clause
// copyright-holders:Chris Hardy
#include "sound/msm5205.h"

class hyprolyb_adpcm_device : public device_t,
									public device_sound_interface
{
public:
	hyprolyb_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~hyprolyb_adpcm_device() {}

	DECLARE_WRITE8_MEMBER( write );
	DECLARE_READ8_MEMBER( busy_r );

	WRITE8_MEMBER( msm_data_w );
	READ8_MEMBER( msm_vck_r );
	READ8_MEMBER( ready_r );
	READ8_MEMBER( data_r );

	void vck_callback( int st );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	private:
	// internal state
	msm5205_device *m_msm;
	address_space *m_space;
	UINT8    m_adpcm_ready; // only bootlegs
	UINT8    m_adpcm_busy;
	UINT8    m_vck_ready;
};

MACHINE_CONFIG_EXTERN( hyprolyb_adpcm );

extern const device_type HYPROLYB_ADPCM;
