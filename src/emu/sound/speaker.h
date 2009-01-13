/**********************************************************************

    speaker.h
    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/

#pragma once

#ifndef __SPEAKER_H__
#define __SPEAKER_H__

#ifdef __cplusplus
extern "C" {
#endif


typedef struct _speaker_interface speaker_interface;
struct _speaker_interface
{
	int num_level; 	/* optional: number of levels (if not two) */
	const INT16 *levels; 	/* optional: pointer to level lookup table */
};

void speaker_level_w (int which, int new_level);

SND_GET_INFO( speaker );
#define SOUND_SPEAKER SND_GET_INFO_NAME( speaker )

#ifdef __cplusplus
}
#endif


#endif /* __SPEAKER_H__ */
