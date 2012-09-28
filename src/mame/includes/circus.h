#include "sound/discrete.h"

class circus_state : public driver_device
{
public:
	circus_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_discrete(*this, "discrete"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;

	/* video-related */
	tilemap_t  *m_bg_tilemap;
	int m_clown_x;
	int m_clown_y;
	int m_clown_z;

	/* devices */
	cpu_device *m_maincpu;
	samples_device *m_samples;
	required_device<discrete_device> m_discrete;

	/* game id */
	int m_game_id;
	DECLARE_READ8_MEMBER(circus_paddle_r);
	DECLARE_WRITE8_MEMBER(circus_videoram_w);
	DECLARE_WRITE8_MEMBER(circus_clown_x_w);
	DECLARE_WRITE8_MEMBER(circus_clown_y_w);
	DECLARE_WRITE8_MEMBER(circus_clown_z_w);
	DECLARE_DRIVER_INIT(ripcord);
	DECLARE_DRIVER_INIT(circus);
	DECLARE_DRIVER_INIT(robotbwl);
	DECLARE_DRIVER_INIT(crash);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_circus(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_robotbwl(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_crash(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_ripcord(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TIMER_DEVICE_CALLBACK_MEMBER(crash_scanline);
};
/*----------- defined in audio/circus.c -----------*/

DISCRETE_SOUND_EXTERN( circus );
DISCRETE_SOUND_EXTERN( robotbwl );
DISCRETE_SOUND_EXTERN( crash );
extern const samples_interface circus_samples_interface;
extern const samples_interface crash_samples_interface;
extern const samples_interface ripcord_samples_interface;
extern const samples_interface robotbwl_samples_interface;
