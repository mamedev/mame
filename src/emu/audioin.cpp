// ============================================================================
// audioin.cpp
// host os sound input device such as microphone, line in or virtual audio cable.
//
// BSD3, jariseon 2020
// based on speaker and device_mixer_interface by Aaron Giles
// ============================================================================

#include "emu.h"
#include "audioin.h"
#include "osdepend.h"


DEFINE_DEVICE_TYPE(AUDIOIN, audioin_device, "audioin", "AudioInput")
#define MAX_NSAMPLES 48000  // todo: how to get max buffersize ?


// ===-------------------------------------------------------------------------
// constructor
// ===-------------------------------------------------------------------------

audioin_device::audioin_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, AUDIOIN, tag, owner, clock)
	, device_sound_interface(mconfig, *this)
	, m_input_stream(nullptr)
{
	m_audioin = new s16[MAX_NSAMPLES * 2]; // interleaved stereo
}


// ===-------------------------------------------------------------------------
// destructor
// ===-------------------------------------------------------------------------

audioin_device::~audioin_device()
{
  delete [] m_audioin;
}


// ===-------------------------------------------------------------------------
// perform startup prior to the device startup
// ===-------------------------------------------------------------------------

void audioin_device::interface_pre_start()
{
  device_sound_interface::interface_pre_start();
  m_input_stream = stream_alloc(0, 2, device().machine().sample_rate());
}

// ===-------------------------------------------------------------------------
// device startup
// ===-------------------------------------------------------------------------

void audioin_device::device_start()
{
}

// ===-------------------------------------------------------------------------
// update stream sample rate after loading a save state
// ===-------------------------------------------------------------------------

void audioin_device::interface_post_load()
{
  if (m_input_stream)
    m_input_stream->set_sample_rate(device().machine().sample_rate());
  device_sound_interface::interface_post_load();
}


// ===-------------------------------------------------------------------------
// capture audio input from osd
// ===-------------------------------------------------------------------------

void audioin_device::sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int nsamples)
{
  if (m_input_stream == nullptr || nsamples < 0)
    return;
  
  memset(outputs[0], 0, nsamples * sizeof(s16));
  memset(outputs[1], 0, nsamples * sizeof(s16));
  
  // unsure if it is cool to access the osd layer directly from device level
  // seems a logical place for this though
  int N = std::min(nsamples, MAX_NSAMPLES);
  device().machine().osd().capture_audio_stream(m_audioin, nsamples);

  // de-interleave
  s16* p = m_audioin;
  for (int n = 0; n < N; n++) {
    outputs[0][n] = *p++;
    outputs[1][n] = *p++;
  }
}
