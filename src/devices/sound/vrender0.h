// license:BSD-3-Clause
// copyright-holders:ElSemi
#pragma once

#ifndef __VRENDER0_H__
#define __VRENDER0_H__


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_VRENDER0_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, VRENDER0, _clock)
#define MCFG_SOUND_VRENDER0_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, VRENDER0, _clock)

#define MCFG_VR0_REGBASE(_base) \
	vrender0_device::set_reg_base(*device, _base);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> vrender0_device

class vrender0_device : public device_t,
						public device_sound_interface
{
public:
	vrender0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);
	~vrender0_device() { }

	// static configuration
	static void set_reg_base(device_t &device, int base) { downcast<vrender0_device &>(device).m_reg_base = base; }

	uint32_t vr0_snd_read(address_space &space, offs_t offset, uint32_t mem_mask = 0xffffffff);
	void vr0_snd_write(address_space &space, offs_t offset, uint32_t data, uint32_t mem_mask = 0xffffffff);

	void set_areas(uint32_t *texture, uint32_t *frame);

protected:
	// device-level overrides
	virtual void device_start() override;

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

private:
	uint32_t *m_TexBase;
	uint32_t *m_FBBase;
	uint32_t m_SOUNDREGS[0x10000/4];
	sound_stream *m_stream;
	uint32_t m_reg_base;

	void VR0_RenderAudio(int nsamples, stream_sample_t *l, stream_sample_t *r);
};

extern const device_type VRENDER0;


#endif /* __VRENDER0_H__ */
