/*****************************************************************************
 *
 * sound/s2636.h
 *
 ****************************************************************************/

#ifndef S2636_SOUND_H_
#define S2636_SOUND_H_



DECLARE_LEGACY_SOUND_DEVICE(S2636_SOUND, s2636_sound);
void s2636_soundport_w (running_device *device, int mode, int data);


#endif /* VC4000_H_ */
