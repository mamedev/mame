// license:BSD-3-Clause
// copyright-holders:Brad Oliver, Nicola Salmoria
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> redbaron_sound_device

class redbaron_sound_device : public device_t,
								public device_sound_interface
{
public:
	redbaron_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER( sounds_w );

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	std::unique_ptr<INT16[]> m_vol_lookup;

	INT16 m_vol_crash[16];

	sound_stream *m_channel;
	int m_latch;
	int m_poly_counter;
	int m_poly_shift;

	int m_filter_counter;

	int m_crash_amp;
	int m_shot_amp;
	int m_shot_amp_counter;

	int m_squeal_amp;
	int m_squeal_amp_counter;
	int m_squeal_off_counter;
	int m_squeal_on_counter;
	int m_squeal_out;
};

extern const device_type REDBARON;
