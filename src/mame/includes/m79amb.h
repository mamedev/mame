#include "sound/discrete.h"


/*----------- defined in audio/m79amb.c -----------*/

DISCRETE_SOUND_EXTERN( m79amb );

DECLARE_WRITE8_DEVICE_HANDLER( m79amb_8000_w );
DECLARE_WRITE8_DEVICE_HANDLER( m79amb_8003_w );
