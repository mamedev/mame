// license:GPL-2.0+
// copyright-holders:Peter Trauner
#ifndef ARCADIA_SOUND_H_
#define ARCADIA_SOUND_H_

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ARCADIA_SOUND_ADD(_tag) \
	MCFG_SOUND_ADD(_tag, ARCADIA_SOUND, 0)

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> arcadia_sound_device

class arcadia_sound_device : public device_t,
								public device_sound_interface
{
public:
	// construction/destruction
	arcadia_sound_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	DECLARE_WRITE8_MEMBER(write);
protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

	sound_stream *m_channel;
	UINT8 m_reg[3];
	int m_size, m_pos,m_tval,m_nval;
	unsigned m_mode, m_omode;
	unsigned m_volume;
	unsigned m_lfsr;
};

// device type definition
extern const device_type ARCADIA_SOUND;

#endif /* ARCADIA_SOUND_H_ */
