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

#include "devlegcy.h"


/* AN EXPLANATION
 *
 * The NES APU is actually integrated into the Nintendo processor.
 * You must supply the same number of APUs as you do processors.
 * Also make sure to correspond the memory regions to those used in the
 * processor, as each is shared.
 */

typedef struct _nes_interface nes_interface;
struct _nes_interface
{
	const char *cpu_tag;  /* CPU tag */
};

READ8_DEVICE_HANDLER( nes_psg_r );
WRITE8_DEVICE_HANDLER( nes_psg_w );

class nesapu_device : public device_t,
                                  public device_sound_interface
{
public:
	nesapu_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	~nesapu_device() { global_free(m_token); }

	// access to legacy token
	void *token() const { assert(m_token != NULL); return m_token; }
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();

	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
private:
	// internal state
	void *m_token;
};

extern const device_type NES;


#endif /* __NES_APU_H__ */
