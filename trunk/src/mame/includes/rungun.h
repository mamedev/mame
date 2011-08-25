/*************************************************************************

    Run and Gun / Slam Dunk

*************************************************************************/

class rungun_state : public driver_device
{
public:
	rungun_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_936_videoram;
//  UINT16 *    m_paletteram;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t   *m_ttl_tilemap;
	tilemap_t   *m_936_tilemap;
	UINT16      m_ttl_vram[0x1000];
	int         m_ttl_gfx_index;
	int         m_sprite_colorbase;

	/* misc */
	UINT16      m_sysreg[0x20];
	int         m_z80_control;
	int         m_sound_status;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_k054539_1;
	device_t *m_k054539_2;
	device_t *m_k053936;
	device_t *m_k055673;
	device_t *m_k053252;
};




/*----------- defined in video/rungun.c -----------*/

extern void rng_sprite_callback(running_machine &machine, int *code, int *color, int *priority_mask);

READ16_HANDLER( rng_ttl_ram_r );
WRITE16_HANDLER( rng_ttl_ram_w );
WRITE16_HANDLER( rng_936_videoram_w );

VIDEO_START( rng );
SCREEN_UPDATE( rng );
