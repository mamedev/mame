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

#define MCFG_C140_BANK_TYPE(type) \
	downcast<c140_device &>(*device).set_bank_type((c140_device::C140_TYPE::type));


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> c140_device

class c140_device : public device_t,
					public device_sound_interface
{
public:
	enum class C140_TYPE
	{
		SYSTEM2,
		SYSTEM21,
		ASIC219
	};

	c140_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_bank_type(C140_TYPE bank) { m_banking_type = bank; }

	DECLARE_READ8_MEMBER( c140_r );
	DECLARE_WRITE8_MEMBER( c140_w );

	void set_base(void *base);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
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
	long find_sample(long adrs, long bank, int voice);

	int m_sample_rate;
	sound_stream *m_stream;
	C140_TYPE m_banking_type;
	/* internal buffers */
	std::unique_ptr<int16_t[]> m_mixer_buffer_left;
	std::unique_ptr<int16_t[]> m_mixer_buffer_right;

	int m_baserate;
	optional_region_ptr<int8_t> m_rom_ptr;
	int8_t *m_pRom;
	uint8_t m_REG[0x200];

	int16_t m_pcmtbl[8];        //2000.06.26 CAB

	C140_VOICE m_voi[MAX_VOICE];
};

DECLARE_DEVICE_TYPE(C140, c140_device)

#endif // MAME_SOUND_C140_H
