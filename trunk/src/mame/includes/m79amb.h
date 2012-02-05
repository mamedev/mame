#include "sound/discrete.h"


/*----------- defined in audio/m79amb.c -----------*/

DISCRETE_SOUND_EXTERN( m79amb );

WRITE8_DEVICE_HANDLER( m79amb_8000_w );
WRITE8_DEVICE_HANDLER( m79amb_8003_w );
