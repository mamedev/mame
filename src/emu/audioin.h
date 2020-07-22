// ============================================================================
// audioin.h
// host os sound input device such as microphone, line in or virtual audio cable.
//
// BSD3, jariseon 2020
// based on speaker and device_mixer_interface by Aaron Giles
// ============================================================================

#ifndef MAME_EMU_AUDIOIN_H
#define MAME_EMU_AUDIOIN_H

#pragma once

DECLARE_DEVICE_TYPE(AUDIOIN, audioin_device)

class audioin_device : public device_t, public device_sound_interface
{
public:
	// construction/destruction
	audioin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock = 0);
	virtual ~audioin_device();

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;
  
  // optional operation overrides
  virtual void interface_pre_start() override;
  virtual void interface_post_load() override;

  // sound interface overrides
  virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples) override;

  // internal state
  sound_stream * m_input_stream;
  s16* m_audioin;
};


// device iterator
typedef device_type_iterator<audioin_device> audioin_device_iterator;


#endif // MAME_EMU_AUDIOIN_H
