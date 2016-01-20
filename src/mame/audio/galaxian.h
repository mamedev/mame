// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "sound/discrete.h"

#define GAL_AUDIO   "discrete"

class galaxian_sound_device : public device_t,
									public device_sound_interface
{
public:
	galaxian_sound_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~galaxian_sound_device() {}

	DECLARE_WRITE8_MEMBER( sound_w );
	DECLARE_WRITE8_MEMBER( pitch_w );
	DECLARE_WRITE8_MEMBER( vol_w );
	DECLARE_WRITE8_MEMBER( noise_enable_w );
	DECLARE_WRITE8_MEMBER( background_enable_w );
	DECLARE_WRITE8_MEMBER( fire_enable_w );
	DECLARE_WRITE8_MEMBER( lfo_freq_w );

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	UINT8 m_lfo_val;
	discrete_device *m_discrete;
};

extern const device_type GALAXIAN;

MACHINE_CONFIG_EXTERN( mooncrst_audio );
MACHINE_CONFIG_EXTERN( galaxian_audio );
