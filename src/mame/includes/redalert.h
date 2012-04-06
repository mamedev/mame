/***************************************************************************

    Irem Red Alert hardware

    If you have any questions about how this driver works, don't hesitate to
    ask.  - Mike Balfour (mab22@po.cwru.edu)

****************************************************************************/

class redalert_state : public driver_device
{
public:
	redalert_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_ay8910_latch_1;
	UINT8 m_ay8910_latch_2;
	UINT8 *m_bitmap_videoram;
	UINT8 *m_bitmap_color;
	UINT8 *m_charmap_videoram;
	UINT8 *m_video_control;
	UINT8 *m_bitmap_colorram;
	UINT8 m_control_xor;
	DECLARE_READ8_MEMBER(redalert_interrupt_clear_r);
	DECLARE_WRITE8_MEMBER(redalert_interrupt_clear_w);
	DECLARE_READ8_MEMBER(panther_interrupt_clear_r);
	DECLARE_READ8_MEMBER(panther_unk_r);
	DECLARE_WRITE8_MEMBER(redalert_bitmap_videoram_w);
};


/*----------- defined in audio/redalert.c -----------*/

WRITE8_HANDLER( redalert_audio_command_w );
WRITE8_HANDLER( redalert_voice_command_w );

WRITE8_HANDLER( demoneye_audio_command_w );

MACHINE_CONFIG_EXTERN( redalert_audio );
MACHINE_CONFIG_EXTERN( ww3_audio );
MACHINE_CONFIG_EXTERN( demoneye_audio );


/*----------- defined in video/redalert.c -----------*/


MACHINE_CONFIG_EXTERN( ww3_video );
MACHINE_CONFIG_EXTERN( panther_video );
MACHINE_CONFIG_EXTERN( redalert_video );
MACHINE_CONFIG_EXTERN( demoneye_video );
