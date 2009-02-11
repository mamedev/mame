#pragma once

#ifndef __CDDA_H__
#define _CDDA_H_

void cdda_set_cdrom(const device_config *device, void *file);
const device_config *cdda_from_cdrom(running_machine *machine, void *file);

void cdda_start_audio(const device_config *device, UINT32 startlba, UINT32 numblocks);
void cdda_stop_audio(const device_config *device);
void cdda_pause_audio(const device_config *device, int pause);

UINT32 cdda_get_audio_lba(const device_config *device);
int cdda_audio_active(const device_config *device);
int cdda_audio_paused(const device_config *device);
int cdda_audio_ended(const device_config *device);

DEVICE_GET_INFO( cdda );
#define SOUND_CDDA DEVICE_GET_INFO_NAME( cdda )

#endif /* __CDDA_H__ */
