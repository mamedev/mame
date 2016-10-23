// license:BSD-3-Clause
// copyright-holders:Aaron Giles
#include "sound/discrete.h"

#define GAL_AUDIO   "discrete"

class galaxian_sound_device : public device_t,
									public device_sound_interface
{
public:
	galaxian_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~galaxian_sound_device() {}

	void sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void pitch_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void vol_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void noise_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void background_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void fire_enable_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	void lfo_freq_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

protected:
	// device-level overrides
	virtual void device_config_complete() override;
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	// internal state
	uint8_t m_lfo_val;
	discrete_device *m_discrete;
};

extern const device_type GALAXIAN;

MACHINE_CONFIG_EXTERN( mooncrst_audio );
MACHINE_CONFIG_EXTERN( galaxian_audio );
