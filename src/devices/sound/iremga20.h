// license:BSD-3-Clause
// copyright-holders:Acho A. Tang,R. Belmont
/*********************************************************

    Irem GA20 PCM Sound Chip

*********************************************************/
#pragma once

#ifndef __IREMGA20_H__
#define __IREMGA20_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_IREMGA20_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, IREMGA20, _clock)
#define MCFG_IREMGA20_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, IREMGA20, _clock)


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

struct IremGA20_channel_def
{
	uint32_t rate;
	uint32_t size;
	uint32_t start;
	uint32_t pos;
	uint32_t frac;
	uint32_t end;
	uint32_t volume;
	uint32_t pan;
	uint32_t effect;
	uint32_t play;
};


// ======================> iremga20_device

class iremga20_device : public device_t,
						public device_sound_interface
{
public:
	iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~iremga20_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	void irem_ga20_w(address_space &space, offs_t offset, uint8_t data, uint8_t mem_mask = 0xff);
	uint8_t irem_ga20_r(address_space &space, offs_t offset, uint8_t mem_mask = 0xff);

private:
	void iremga20_reset();

private:
	required_region_ptr<uint8_t> m_rom;
	sound_stream *m_stream;
	uint16_t m_regs[0x40];
	IremGA20_channel_def m_channel[4];
};

extern const device_type IREMGA20;


#endif /* __IREMGA20_H__ */
