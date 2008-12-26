#pragma once

#ifndef __MULTIPCM_H__
#define __MULTIPCM_H__

WRITE8_HANDLER( multi_pcm_reg_0_w );
READ8_HANDLER( multi_pcm_reg_0_r);
WRITE8_HANDLER( multi_pcm_reg_1_w );
READ8_HANDLER( multi_pcm_reg_1_r);

void multi_pcm_set_bank(int which, UINT32 leftoffs, UINT32 rightoffs);

SND_GET_INFO( multipcm );

#endif /* __MULTIPCM_H__ */
