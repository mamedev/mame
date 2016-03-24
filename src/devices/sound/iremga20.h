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
	UINT32 rate;
	UINT32 size;
	UINT32 start;
	UINT32 pos;
	UINT32 frac;
	UINT32 end;
	UINT32 volume;
	UINT32 pan;
	UINT32 effect;
	UINT32 play;
};


// ======================> iremga20_device

class iremga20_device : public device_t,
						public device_sound_interface
{
public:
	iremga20_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~iremga20_device() { }

protected:
	// device-level overrides
	virtual void device_start() override;
	virtual void device_reset() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_WRITE8_MEMBER( irem_ga20_w );
	DECLARE_READ8_MEMBER( irem_ga20_r );

private:
	void iremga20_reset();

private:
	UINT8 *m_rom;
	INT32 m_rom_size;
	sound_stream *m_stream;
	UINT16 m_regs[0x40];
	IremGA20_channel_def m_channel[4];
};

extern const device_type IREMGA20;


#endif /* __IREMGA20_H__ */
