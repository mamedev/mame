// license:BSD-3-Clause
// copyright-holders:R. Belmont
/* C140.h */

#ifndef MAME_SOUND_C140_H
#define MAME_SOUND_C140_H

#pragma once

//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_C140_ADD(tag, clock) \
		MCFG_DEVICE_ADD((tag), C140, (clock))

#define MCFG_C140_REPLACE(tag, clock) \
		MCFG_DEVICE_REPLACE((tag), C140, (clock))

#define MCFG_C219_ADD(tag, clock) \
		MCFG_DEVICE_ADD((tag), C219, (clock))

#define MCFG_C219_REPLACE(tag, clock) \
		MCFG_DEVICE_REPLACE((tag), C219, (clock))


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> c140_device

class c140_device : public device_t,
					public device_sound_interface,
					public device_rom_interface
{
public:
	c140_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( c140_r );
	DECLARE_WRITE8_MEMBER( c140_w );

protected:
	c140_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int addrbit, int databit);

	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual void device_clock_changed() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
	
	// device_rom_interface overrides
	virtual void rom_bank_updated() override;
	
	virtual void voice_update();

	static constexpr unsigned MAX_VOICE = 24;

	struct C140_VOICE
	{
		C140_VOICE() { }
		
		int32_t    ptoffset     = 0;
		int32_t    pos          = 0;
		int32_t    key          = 0;
		//--work
		int32_t    lastdt       = 0;
		int32_t    prevdt       = 0;
		int32_t    dltdt        = 0;
		//--reg
		int32_t    rvol         = 0;
		int32_t    lvol         = 0;
		int32_t    frequency    = 0;
		int32_t    bank         = 0;
		int32_t    mode         = 0;

		int32_t    sample_start = 0;
		int32_t    sample_end   = 0;
		int32_t    sample_loop  = 0;
	};

	void init_voice( C140_VOICE *v );

	int m_sample_rate;
	sound_stream *m_stream;
	/* internal buffers */
	std::vector<int16_t> m_mixer_buffer_left;
	std::vector<int16_t> m_mixer_buffer_right;

	int m_prev_buffer_size;

	int m_baserate;
	uint8_t m_REG[0x200];

	int16_t m_pcmtbl[8];        //2000.06.26 CAB

	C140_VOICE m_voi[MAX_VOICE];
};

DECLARE_DEVICE_TYPE(C140, c140_device)

// ======================> c219_device

class c219_device : public c140_device
{
public:
	c219_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	DECLARE_READ8_MEMBER( c219_r );
	DECLARE_WRITE8_MEMBER( c219_w );

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	virtual void voice_update() override;

private:
	uint8_t m_c219bank[0x4];
};

DECLARE_DEVICE_TYPE(C219, c219_device)

#endif // MAME_SOUND_C140_H
