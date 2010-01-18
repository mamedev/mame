#pragma once

#ifndef __CDDA_H__
#define _CDDA_H_

void cdda_set_cdrom(running_device *device, void *file);
running_device *cdda_from_cdrom(running_machine *machine, void *file);

void cdda_start_audio(running_device *device, UINT32 startlba, UINT32 numblocks);
void cdda_stop_audio(running_device *device);
void cdda_pause_audio(running_device *device, int pause);

UINT32 cdda_get_audio_lba(running_device *device);
int cdda_audio_active(running_device *device);
int cdda_audio_paused(running_device *device);
int cdda_audio_ended(running_device *device);

DEVICE_GET_INFO( cdda );
#define SOUND_CDDA DEVICE_GET_INFO_NAME( cdda )

#endif /* __CDDA_H__ */
