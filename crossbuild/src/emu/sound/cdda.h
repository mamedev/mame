#ifndef _CDDA_H_
#define _CDDA_H_

void cdda_set_cdrom(int num, void *file);
int cdda_num_from_cdrom(void *file);

void cdda_start_audio(int num, UINT32 startlba, UINT32 numblocks);
void cdda_stop_audio(int num);
void cdda_pause_audio(int num, int pause);

UINT32 cdda_get_audio_lba(int num);
int cdda_audio_active(int num);
int cdda_audio_paused(int num);
int cdda_audio_ended(int num);

#endif

