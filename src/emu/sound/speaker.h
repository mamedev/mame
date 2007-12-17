/**********************************************************************

    speaker.h
    Sound driver to emulate a simple speaker,
    driven by one or more output bits

**********************************************************************/
#ifndef SPEAKER_H
#define SPEAKER_H

#ifdef __cplusplus
extern "C" {
#endif

struct Speaker_interface
{
	int num_level; 	/* optional: number of levels (if not two) */
	INT16 *levels; 	/* optional: pointer to level lookup table */
};

void speaker_level_w (int which, int new_level);

#ifdef __cplusplus
}
#endif


#endif

