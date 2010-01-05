
typedef struct _xmen_state xmen_state;
struct _xmen_state
{
	/* memory pointers */
//  UINT16 *   paletteram;    // currently this uses generic palette handling

	/* video-related */
	int        layer_colorbase[3], sprite_colorbase;
	int        layerpri[3];

	/* for xmen6p */
	bitmap_t   *screen_right;
	bitmap_t   *screen_left;
	UINT16 *   xmen6p_spriteramleft;
	UINT16 *   xmen6p_spriteramright;
	UINT16 *   xmen6p_tilemapleft;
	UINT16 *   xmen6p_tilemapright;
	UINT16 *   k053247_ram;
	UINT16     current_frame;

	/* misc */
	UINT8       sound_curbank;

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k054539;
	const device_config *k052109;
	const device_config *k053246;
	const device_config *k053251;
	const device_config *lscreen;
	const device_config *rscreen;
};

/*----------- defined in video/xmen.c -----------*/

void xmen_tile_callback(running_machine *machine, int layer,int bank,int *code,int *color,int *flags,int *priority);
void xmen_sprite_callback(running_machine *machine, int *code,int *color,int *priority_mask);

VIDEO_START( xmen6p );
VIDEO_UPDATE( xmen );
VIDEO_UPDATE( xmen6p );
VIDEO_EOF( xmen6p );
