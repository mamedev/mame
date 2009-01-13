#pragma once

#ifndef __TIAINTF_H__
#define __TIAINTF_H__

WRITE8_HANDLER( tia_sound_w );

SND_GET_INFO( tia );
#define SOUND_TIA SND_GET_INFO_NAME( tia )

#endif /* __TIAINTF_H__ */
