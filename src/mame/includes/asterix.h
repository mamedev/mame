/*************************************************************************

    Asterix

*************************************************************************/

typedef struct _asterix_state asterix_state;
struct _asterix_state
{
	/* memory pointers */
//  UINT16 *    paletteram;    // currently this uses generic palette handling

	/* video-related */
	int         sprite_colorbase;
	int         layer_colorbase[4], layerpri[3];
	UINT16      spritebank;
	int         tilebanks[4];
	int         spritebanks[4];

	/* misc */
	UINT8       cur_control2;
	UINT16      prot[2];

	/* devices */
	const device_config *maincpu;
	const device_config *audiocpu;
	const device_config *k053260;
	const device_config *k056832;
	const device_config *k053244;
	const device_config *k053251;
};



/*----------- defined in video/asterix.c -----------*/

VIDEO_UPDATE( asterix );
WRITE16_HANDLER( asterix_spritebank_w );

extern void asterix_tile_callback(running_machine *machine, int layer, int *code, int *color, int *flags);
extern void asterix_sprite_callback(running_machine *machine, int *code, int *color, int *priority_mask);
