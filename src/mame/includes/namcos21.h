/**
 * @file namcos21.h
 */

#define NAMCOS21_POLY_FRAME_WIDTH 496
#define NAMCOS21_POLY_FRAME_HEIGHT 480

class namcos21_state : public driver_device
{
public:
	namcos21_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 *videoram;
};


/*----------- defined in drivers/namcos21.c -----------*/

extern void namcos21_kickstart(running_machine *, int);


/*----------- defined in video/namcos21.c -----------*/

extern void namcos21_ClearPolyFrameBuffer( void );
extern void namcos21_DrawQuad( int sx[4], int sy[4], int zcode[4], int color );

extern READ16_HANDLER(winrun_gpu_color_r);
extern WRITE16_HANDLER(winrun_gpu_color_w);

extern READ16_HANDLER(winrun_gpu_videoram_r);
extern WRITE16_HANDLER(winrun_gpu_videoram_w);

extern READ16_HANDLER(winrun_gpu_register_r);
extern WRITE16_HANDLER(winrun_gpu_register_w);

extern VIDEO_START( namcos21 ) ;
extern VIDEO_UPDATE( namcos21 );
