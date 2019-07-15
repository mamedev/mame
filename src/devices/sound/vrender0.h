// license:BSD-3-Clause
// copyright-holders:ElSemi
#ifndef MAME_SOUND_VRENDER0_H
#define MAME_SOUND_VRENDER0_H

#pragma once


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************


// ======================> vr0sound_device

class vr0sound_device : public device_t,
						public device_sound_interface
{
public:
	vr0sound_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

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

	void VR0_RenderAudio(int nsamples, stream_sample_t *l, stream_sample_t *r);
};

DECLARE_DEVICE_TYPE(SOUND_VRENDER0, vr0sound_device)

#endif // MAME_SOUND_VRENDER0_H
