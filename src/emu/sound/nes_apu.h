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

typedef struct _nes_interface nes_interface;
struct _nes_interface
{
	const char *cpu_tag;  /* CPU tag */
};

READ8_DEVICE_HANDLER( nes_psg_r );
WRITE8_DEVICE_HANDLER( nes_psg_w );

DEVICE_GET_INFO( nesapu );
#define SOUND_NES DEVICE_GET_INFO_NAME( nesapu )

#endif /* __NES_APU_H__ */
