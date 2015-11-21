// license:GPL-2.0+
// copyright-holders:Matthew Conte
/*****************************************************************************

  MAME/MESS NES APU CORE

  Based on the Nofrendo/Nosefart NES N2A03 sound emulation core written by
  Matthew Conte (matt@conte.com) and redesigned for use in MAME/MESS by
  Who Wants to Know? (wwtk@mail.com)

  This core is written with the advise and consent of Matthew Conte and is
  released under the GNU Public License.  This core is freely avaiable for
  use in any freeware project, subject to the following terms:

  Any modifications to this code must be duly noted in the source and
  approved by Matthew Conte and myself prior to public submission.

 *****************************************************************************

   NES_APU.H

   NES APU external interface.

 *****************************************************************************/

#pragma once

#ifndef __NES_APU_H__
#define __NES_APU_H__


/* AN EXPLANATION
 *
 * The NES APU is actually integrated into the Nintendo processor.
 * You must supply the same number of APUs as you do processors.
 * Also make sure to correspond the memory regions to those used in the
 * processor, as each is shared.
 */

#include "nes_defs.h"

/* GLOBAL CONSTANTS */
#define  SYNCS_MAX1     0x20
#define  SYNCS_MAX2     0x80

class nesapu_device : public device_t,
						public device_sound_interface
{
public:
	nesapu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nesapu_device() {}

	static void set_cpu_tag(device_t &device, const char *tag) { downcast<nesapu_device &>(device).m_cpu_tag = tag; }
	void set_tag_memory(const char *tag);

	DECLARE_READ8_MEMBER( read );
	DECLARE_WRITE8_MEMBER( write );

protected:
	// device-level overrides
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);

private:
	// internal state
	apu_t   m_APU;                   /* Actual APUs */
	float   m_apu_incsize;           /* Adjustment increment */
	uint32  m_samps_per_sync;        /* Number of samples per vsync */
	uint32  m_buffer_size;           /* Actual buffer size in bytes */
	uint32  m_real_rate;             /* Actual playback rate */
	uint8   m_noise_lut[NOISE_LONG]; /* Noise sample lookup table */
	uint32  m_vbl_times[0x20];       /* VBL durations in samples */
	uint32  m_sync_times1[SYNCS_MAX1]; /* Samples per sync table */
	uint32  m_sync_times2[SYNCS_MAX2]; /* Samples per sync table */
	sound_stream *m_stream;

	const char *m_cpu_tag;

	void create_syncs(unsigned long sps);
	int8 apu_square(square_t *chan);
	int8 apu_triangle(triangle_t *chan);
	int8 apu_noise(noise_t *chan);
	int8 apu_dpcm(dpcm_t *chan);
	inline void apu_regwrite(int address, uint8 value);
	inline uint8 apu_read(int address);
	inline void apu_write(int address, uint8 value);
};

extern const device_type NES_APU;

#define MCFG_NES_APU_CPU(_tag) \
	nesapu_device::set_cpu_tag(*device, _tag);


#endif /* __NES_APU_H__ */
