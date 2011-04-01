#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define NORAUTP_SND_EN					NODE_01
#define NORAUTP_FREQ_DATA				NODE_02


class norautp_state : public driver_device
{
public:
	norautp_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT16 *m_np_vram;
	UINT16 m_np_addr;
};


/*----------- defined in audio/norautp.c -----------*/

DISCRETE_SOUND_EXTERN( norautp );
DISCRETE_SOUND_EXTERN( dphl );
DISCRETE_SOUND_EXTERN( kimble );
