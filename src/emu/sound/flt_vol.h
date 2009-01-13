#pragma once

#ifndef __FLT_VOL_H__
#define __FLT_VOL_H__

void flt_volume_set_volume(int num, float volume);

SND_GET_INFO( filter_volume );
#define SOUND_FILTER_VOLUME SND_GET_INFO_NAME( filter_volume )

#endif /* __FLT_VOL_H__ */
