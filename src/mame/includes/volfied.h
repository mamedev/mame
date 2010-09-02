/*************************************************************************

    Volfied

*************************************************************************/

class volfied_state : public driver_device
{
public:
	volfied_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	/* memory pointers */
	UINT16 *    video_ram;
	UINT8  *    cchip_ram;
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      video_ctrl;
	UINT16      video_mask;

	/* c-chip */
	UINT8       current_bank;
	UINT8       current_flag;
	UINT8       cc_port;
	UINT8       current_cmd;

	/* devices */
	running_device *maincpu;
	running_device *audiocpu;
	running_device *pc090oj;
};


/*----------- defined in machine/volfied.c -----------*/

void volfied_cchip_init(running_machine *machine);
void volfied_cchip_reset(running_machine *machine);

READ16_HANDLER( volfied_cchip_ctrl_r );
READ16_HANDLER( volfied_cchip_ram_r );
WRITE16_HANDLER( volfied_cchip_ctrl_w );
WRITE16_HANDLER( volfied_cchip_bank_w );
WRITE16_HANDLER( volfied_cchip_ram_w );


/*----------- defined in video/volfied.c -----------*/

WRITE16_HANDLER( volfied_sprite_ctrl_w );
WRITE16_HANDLER( volfied_video_ram_w );
WRITE16_HANDLER( volfied_video_ctrl_w );
WRITE16_HANDLER( volfied_video_mask_w );

READ16_HANDLER( volfied_video_ram_r );
READ16_HANDLER( volfied_video_ctrl_r );

VIDEO_UPDATE( volfied );
VIDEO_START( volfied );
