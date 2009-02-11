#pragma once

#ifndef __ASTROCDE_H__
#define __ASTROCDE_H__

WRITE8_DEVICE_HANDLER( astrocade_sound_w );

DEVICE_GET_INFO( astrocade );
#define SOUND_ASTROCADE DEVICE_GET_INFO_NAME( astrocade )

#endif /* __ASTROCDE_H__ */
