#pragma once

#ifndef __SNKWAVE_H__
#define __SNKWAVE_H__

WRITE8_HANDLER( snkwave_w );

SND_GET_INFO( snkwave );
#define SOUND_SNKWAVE SND_GET_INFO_NAME( snkwave )

#endif /* __SNKWAVE_H__ */
