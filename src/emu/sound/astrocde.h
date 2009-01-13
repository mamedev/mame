#pragma once

#ifndef __ASTROCDE_H__
#define __ASTROCDE_H__

WRITE8_HANDLER( astrocade_sound1_w );
WRITE8_HANDLER( astrocade_sound2_w );

SND_GET_INFO( astrocade );
#define SOUND_ASTROCADE SND_GET_INFO_NAME( astrocade )

#endif /* __ASTROCDE_H__ */
