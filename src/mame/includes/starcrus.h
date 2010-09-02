class starcrus_state : public driver_device
{
public:
	starcrus_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	bitmap_t *ship1_vid;
	bitmap_t *ship2_vid;
	bitmap_t *proj1_vid;
	bitmap_t *proj2_vid;

	int s1_x;
	int s1_y;
	int s2_x;
	int s2_y;
	int p1_x;
	int p1_y;
	int p2_x;
	int p2_y;

	int p1_sprite;
	int p2_sprite;
	int s1_sprite;
	int s2_sprite;

	int engine1_on;
	int engine2_on;
	int explode1_on;
	int explode2_on;
	int launch1_on;
	int launch2_on;

	int collision_reg;

	int engine_sound_playing;
	int explode_sound_playing;
	int launch1_sound_playing;
	int launch2_sound_playing;
};


/*----------- defined in video/starcrus.c -----------*/

WRITE8_HANDLER( starcrus_s1_x_w );
WRITE8_HANDLER( starcrus_s1_y_w );
WRITE8_HANDLER( starcrus_s2_x_w );
WRITE8_HANDLER( starcrus_s2_y_w );
WRITE8_HANDLER( starcrus_p1_x_w );
WRITE8_HANDLER( starcrus_p1_y_w );
WRITE8_HANDLER( starcrus_p2_x_w );
WRITE8_HANDLER( starcrus_p2_y_w );
WRITE8_HANDLER( starcrus_ship_parm_1_w );
WRITE8_HANDLER( starcrus_ship_parm_2_w );
WRITE8_HANDLER( starcrus_proj_parm_1_w );
WRITE8_HANDLER( starcrus_proj_parm_2_w );
READ8_HANDLER( starcrus_coll_det_r );
VIDEO_START( starcrus );
VIDEO_UPDATE( starcrus );
