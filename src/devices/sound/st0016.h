// license:BSD-3-Clause
// copyright-holders:R. Belmont, Tomasz Slanina, David Haywood
#pragma once

#ifndef __ST0016_H__
#define __ST0016_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_ST0016_SOUNDRAM_READ_CB(_devcb) \
	devcb = &st0016_device::set_soundram_callback(*device, DEVCB_##_devcb);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> st0016_device

class st0016_device : public device_t,
						public device_sound_interface
{
public:
	st0016_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock);
	~st0016_device() { }

	template<class _Object> static devcb_base &set_soundram_callback(device_t &device, _Object object) { return downcast<st0016_device &>(device).m_ram_read_cb.set_callback(object); }

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

public:
	DECLARE_READ8_MEMBER( st0016_snd_r );
	DECLARE_WRITE8_MEMBER( st0016_snd_w );

private:
	sound_stream *m_stream;
	devcb_read8 m_ram_read_cb;
	int m_vpos[8];
	int m_frac[8];
	int m_lponce[8];
	UINT8 m_regs[0x100];
};

extern const device_type ST0016;


#endif /* __ST0016_H__ */
