/*************************************************************************

    Slapshot / Operation Wolf 3

*************************************************************************/

struct slapshot_tempsprite
{
	int gfx;
	int code,color;
	int flipx,flipy;
	int x,y;
	int zoomx,zoomy;
	int primask;
};

class slapshot_state : public driver_device
{
public:
	slapshot_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    color_ram;
	UINT16 *    spriteram;
	UINT16 *    spriteext;
	UINT16 *    spriteram_buffered, *spriteram_delayed;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      spriteext_size;
	size_t      spriteram_size;

	/* video-related */
	struct      slapshot_tempsprite *spritelist;
	INT32       sprites_disabled, sprites_active_area, sprites_master_scrollx, sprites_master_scrolly;
	int         sprites_flipscreen;
	int         prepare_sprites;

	UINT16      spritebank[8];

	/* misc */
	INT32      banknum;

	/* devices */
	device_t *maincpu;
	device_t *audiocpu;
	device_t *tc0140syt;
	device_t *tc0480scp;
	device_t *tc0360pri;
	device_t *tc0640fio;
};


/*----------- defined in video/slapshot.c -----------*/

VIDEO_START( slapshot );
VIDEO_UPDATE( slapshot );
VIDEO_EOF( taito_no_buffer );
