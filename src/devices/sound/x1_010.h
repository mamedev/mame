// license:BSD-3-Clause
// copyright-holders:Luca Elia
#ifndef MAME_SOUND_X1_010_H
#define MAME_SOUND_X1_010_H

#pragma once

class x1_010_device : public device_t, public device_sound_interface
{
public:
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_address_xor(int addr) { m_xor = addr; }

	DECLARE_READ8_MEMBER ( read );
	DECLARE_WRITE8_MEMBER( write );

	DECLARE_READ16_MEMBER ( word_r );
	DECLARE_WRITE16_MEMBER( word_w );

	void enable_w(int data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	static constexpr unsigned NUM_CHANNELS = 16;

	// internal state

	/* Variables only used here */
	required_region_ptr<int8_t> m_region;    // ROM
	int m_rate;                              // Output sampling rate (Hz)
	int m_xor;                               // address XOR
	sound_stream *  m_stream;                // Stream handle
	int m_sound_enable;                      // sound output enable/disable
	uint8_t   m_reg[0x2000];                 // X1-010 Register & wave form area
	uint8_t   m_HI_WORD_BUF[0x2000];         // X1-010 16bit access ram check avoidance work
	uint32_t  m_smp_offset[NUM_CHANNELS];
	uint32_t  m_env_offset[NUM_CHANNELS];

	uint32_t m_base_clock;
};

DECLARE_DEVICE_TYPE(X1_010, x1_010_device)

#define MCFG_X1_010_ADDRESS_XOR(_addr) \
	downcast<x1_010_device &>(*device).set_address_xor(_addr);

#endif // MAME_SOUND_X1_010_H
