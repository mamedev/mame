/*************************************************************************

    Atari Cloud 9 (prototype) hardware

*************************************************************************/

class cloud9_state : public driver_device
{
public:
	cloud9_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config),
		  nvram_stage(*this, "nvram") { }

	/* memory pointers */
	UINT8 *     videoram;
	UINT8 *     spriteram;
	UINT8 *     paletteram;
	required_shared_ptr<UINT8> nvram_stage;    // currently this uses generic nvram handlers
	UINT8		nvram[0x100];      // currently this uses generic nvram handlers

	/* video-related */
	const UINT8 *syncprom;
	const UINT8 *wpprom;
	const UINT8 *priprom;
	bitmap_t    *spritebitmap;
	double      rweights[3], gweights[3], bweights[3];
	UINT8       video_control[8];
	UINT8       bitmode_addr[2];

	/* misc */
	int         vblank_start;
	int         vblank_end;
	emu_timer   *irq_timer;
	UINT8       irq_state;

	/* devices */
	running_device *maincpu;
};


/*----------- defined in video/cloud9.c -----------*/

VIDEO_START( cloud9 );
VIDEO_UPDATE( cloud9 );

WRITE8_HANDLER( cloud9_video_control_w );

WRITE8_HANDLER( cloud9_paletteram_w );
WRITE8_HANDLER( cloud9_videoram_w );

READ8_HANDLER( cloud9_bitmode_r );
WRITE8_HANDLER( cloud9_bitmode_w );
WRITE8_HANDLER( cloud9_bitmode_addr_w );
