#include "sound/discrete.h"


/* Discrete Sound Input Nodes */
#define NORAUTP_SND_EN					NODE_01
#define NORAUTP_FREQ_DATA				NODE_02


class norautp_state : public driver_device
{
public:
	norautp_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_np_vram;
	UINT16 m_np_addr;
	DECLARE_READ8_MEMBER(test_r);
	DECLARE_READ8_MEMBER(vram_data_r);
	DECLARE_WRITE8_MEMBER(vram_data_w);
	DECLARE_WRITE8_MEMBER(vram_addr_w);
	DECLARE_READ8_MEMBER(test2_r);
	DECLARE_WRITE8_MEMBER(mainlamps_w);
	DECLARE_WRITE8_MEMBER(soundlamps_w);
	DECLARE_WRITE8_MEMBER(counterlamps_w);
	DECLARE_DRIVER_INIT(ssa);
	DECLARE_DRIVER_INIT(enc);
	DECLARE_DRIVER_INIT(deb);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_norautp(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

/*----------- defined in audio/norautp.c -----------*/
DISCRETE_SOUND_EXTERN( norautp );
DISCRETE_SOUND_EXTERN( dphl );
DISCRETE_SOUND_EXTERN( kimble );
