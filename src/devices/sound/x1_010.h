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
	x1_010_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~x1_010_device() {}

	// static configuration
	static void set_address(device_t &device, int addr) { downcast<x1_010_device &>(device).m_adr = addr; }

	uint8_t read(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);
	void write(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);

	uint16_t word_r(address_space &space, offs_t offset, uint16_t mem_mask = 0xffff);
	void word_w(address_space &space, offs_t offset, uint16_t data, uint16_t mem_mask = 0xffff);

	void enable_w(int data);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;
private:
	// internal state

	/* Variables only used here */
	required_region_ptr<int8_t> m_region;       // ROM
	int m_rate;                               // Output sampling rate (Hz)
	int m_adr;                                // address
	sound_stream *  m_stream;                 // Stream handle
	int m_sound_enable;                       // sound output enable/disable
	uint8_t   m_reg[0x2000];                // X1-010 Register & wave form area
	uint8_t   m_HI_WORD_BUF[0x2000];            // X1-010 16bit access ram check avoidance work
	uint32_t  m_smp_offset[SETA_NUM_CHANNELS];
	uint32_t  m_env_offset[SETA_NUM_CHANNELS];

	uint32_t m_base_clock;
};

extern const device_type X1_010;


#define MCFG_X1_010_ADDRESS(_addr) \
	x1_010_device::set_address(*device, _addr);


#endif /* __X1_010_H__ */
