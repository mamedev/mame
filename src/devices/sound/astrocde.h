// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Frank Palazzolo
#ifndef MAME_SOUND_ASTROCDE_H
#define MAME_SOUND_ASTROCDE_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ASTROCADE_ADD(tag, clock) \
		MCFG_DEVICE_ADD((tag), ASTROCADE, (clock))

#define MCFG_ASTROCADE_REPLACE(tag, clock) \
		MCFG_DEVICE_REPLACE((tag), ASTROCADE, (clock))



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> astrocade_device

class astrocade_device : public device_t, public device_sound_interface
{
public:
	astrocade_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( astrocade_sound_w );

private:
	void state_save_register();

	sound_stream *m_stream;       /* sound stream */

	uint8_t       m_reg[8];         /* 8 control registers */

	uint8_t       m_master_count;   /* current master oscillator count */
	uint16_t      m_vibrato_clock;  /* current vibrato clock */

	uint8_t       m_noise_clock;    /* current noise generator clock */
	uint16_t      m_noise_state;    /* current noise LFSR state */

	uint8_t       m_a_count;        /* current tone generator A count */
	uint8_t       m_a_state;        /* current tone generator A state */

	uint8_t       m_b_count;        /* current tone generator B count */
	uint8_t       m_b_state;        /* current tone generator B state */

	uint8_t       m_c_count;        /* current tone generator C count */
	uint8_t       m_c_state;        /* current tone generator C state */

	uint8_t       m_bitswap[256];   /* bitswap table */
};

DECLARE_DEVICE_TYPE(ASTROCADE, astrocade_device)

#endif // MAME_SOUND_ASTROCDE_H
