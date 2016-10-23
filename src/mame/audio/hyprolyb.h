// license:BSD-3-Clause
// copyright-holders:Chris Hardy

#include "machine/gen_latch.h"
#include "sound/msm5205.h"

class hyprolyb_adpcm_device : public device_t,
									public device_sound_interface
{
public:
	hyprolyb_adpcm_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~hyprolyb_adpcm_device() {}

	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t busy_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

	void msm_data_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask);
	uint8_t msm_vck_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t ready_r(address_space &space, offs_t offset, uint8_t mem_mask);
	uint8_t data_r(address_space &space, offs_t offset, uint8_t mem_mask);

	void vck_callback( int st );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	private:
	// internal state
	required_device<generic_latch_8_device> m_soundlatch2;
	msm5205_device *m_msm;
	address_space *m_space;
	uint8_t    m_adpcm_ready; // only bootlegs
	uint8_t    m_adpcm_busy;
	uint8_t    m_vck_ready;
};

MACHINE_CONFIG_EXTERN( hyprolyb_adpcm );

extern const device_type HYPROLYB_ADPCM;
