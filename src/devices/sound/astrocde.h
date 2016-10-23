// license:BSD-3-Clause
// copyright-holders:Aaron Giles,Frank Palazzolo
#pragma once

#ifndef __ASTROCDE_H__
#define __ASTROCDE_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ASTROCADE_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, ASTROCADE, _clock)
#define MCFG_ASTROCADE_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, ASTROCADE, _clock)



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> astrocade_device

class astrocade_device : public device_t,
							public device_sound_interface
{
public:
	astrocade_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~astrocade_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void astrocade_sound_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

private:
	void state_save_register();

private:
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

extern const device_type ASTROCADE;


#endif /* __ASTROCDE_H__ */
