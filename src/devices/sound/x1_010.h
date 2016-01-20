// license:BSD-3-Clause
// copyright-holders:Luca Elia
#pragma once

#ifndef __X1_010_H__
#define __X1_010_H__

#define SETA_NUM_CHANNELS 16

class x1_010_device : public device_t,
						public device_sound_interface
{
public:
	x1_010_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~x1_010_device() {}

	// static configuration
	static void set_address(device_t &device, int addr) { downcast<x1_010_device &>(device).m_adr = addr; }

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
	// internal state

	/* Variables only used here */
	required_region_ptr<INT8> m_region;       // ROM
	int m_rate;                               // Output sampling rate (Hz)
	int m_adr;                                // address
	sound_stream *  m_stream;                 // Stream handle
	int m_sound_enable;                       // sound output enable/disable
	UINT8   m_reg[0x2000];                // X1-010 Register & wave form area
	UINT8   m_HI_WORD_BUF[0x2000];            // X1-010 16bit access ram check avoidance work
	UINT32  m_smp_offset[SETA_NUM_CHANNELS];
	UINT32  m_env_offset[SETA_NUM_CHANNELS];

	UINT32 m_base_clock;
};

extern const device_type X1_010;


#define MCFG_X1_010_ADDRESS(_addr) \
	x1_010_device::set_address(*device, _addr);


#endif /* __X1_010_H__ */
