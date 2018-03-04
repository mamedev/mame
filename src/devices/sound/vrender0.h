// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef MAME_SOUND_VRENDER0_H
#define MAME_SOUND_VRENDER0_H

#pragma once


//**************************************************************************
//  INTERFACE CONFIGURATION MACROS
//**************************************************************************

#define MCFG_SOUND_VRENDER0_ADD(_tag, _clock) \
	MCFG_DEVICE_ADD(_tag, VRENDER0, _clock)
#define MCFG_SOUND_VRENDER0_REPLACE(_tag, _clock) \
	MCFG_DEVICE_REPLACE(_tag, VRENDER0, _clock)

#define MCFG_VR0_REGBASE(_base) \
	downcast<vrender0_device &>(*device).set_reg_base(_base);

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> vrender0_device

class vrender0_device : public device_t,
						public device_sound_interface
{
public:
	vrender0_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// configuration
	void set_reg_base(int base) { m_reg_base = base; }

	DECLARE_READ32_MEMBER( vr0_snd_read );
	DECLARE_WRITE32_MEMBER( vr0_snd_write );

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

DECLARE_DEVICE_TYPE(VRENDER0, vrender0_device)

#endif // MAME_SOUND_VRENDER0_H
