/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

class redalert_state : public driver_device
{
public:
	redalert_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8 ay8910_latch_1;
	UINT8 ay8910_latch_2;
	UINT8 *bitmap_videoram;
	UINT8 *bitmap_color;
	UINT8 *charmap_videoram;
	UINT8 *video_control;
	UINT8 *bitmap_colorram;
	UINT8 control_xor;
};


/*----------- defined in audio/redalert.c -----------*/

WRITE8_HANDLER( redalert_audio_command_w );
WRITE8_HANDLER( redalert_voice_command_w );

WRITE8_HANDLER( demoneye_audio_command_w );

MACHINE_CONFIG_EXTERN( redalert_audio );
MACHINE_CONFIG_EXTERN( ww3_audio );
MACHINE_CONFIG_EXTERN( demoneye_audio );


/*----------- defined in video/redalert.c -----------*/

WRITE8_HANDLER( redalert_bitmap_videoram_w );

MACHINE_CONFIG_EXTERN( ww3_video );
MACHINE_CONFIG_EXTERN( panther_video );
MACHINE_CONFIG_EXTERN( redalert_video );
MACHINE_CONFIG_EXTERN( demoneye_video );
