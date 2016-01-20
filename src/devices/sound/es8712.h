// license:BSD-3-Clause
// copyright-holders:Quench
/* An interface for the ES8712 ADPCM chip */

#pragma once

#ifndef __ES8712_H__
#define __ES8712_H__

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ES8712_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ES8712, _clock)
#define MCFG_ES8712_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ES8712, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> es8712_device

class es8712_device : public device_t,
						public device_sound_interface
{
public:
	es8712_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~es8712_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( es8712_w );

public:
	void play();
	void set_bank_base(int base);
	void set_frequency(int frequency);

private:
	void compute_tables();
	void generate_adpcm(stream_sample_t *buffer, int samples);
	void es8712_state_save_register();

private:
	required_region_ptr<UINT8> m_rom;

	UINT8 m_playing;          /* 1 if we're actively playing */

	UINT32 m_base_offset;     /* pointer to the base memory location */
	UINT32 m_sample;          /* current sample number */
	UINT32 m_count;           /* total samples to play */

	UINT32 m_signal;          /* current ADPCM signal */
	UINT32 m_step;            /* current ADPCM step */

	UINT32 m_start;           /* starting address for the next loop */
	UINT32 m_end;             /* ending address for the next loop */
	UINT8  m_repeat;          /* Repeat current sample when 1 */

	INT32 m_bank_offset;
	sound_stream *m_stream;   /* which stream are we playing on? */
};

extern const device_type ES8712;


#endif /* __ES8712_H__ */
