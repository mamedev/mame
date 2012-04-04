typedef struct {
	UINT8 *context_ram;
	UINT8 bank;
	UINT8 *image_ram;
	UINT8 param[0x9];
} coprocessor_t;

class thief_state : public driver_device
{
public:
	thief_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 *m_videoram;
	UINT8 m_input_select;
	UINT8 m_read_mask;
	UINT8 m_write_mask;
	UINT8 m_video_control;
	coprocessor_t m_coprocessor;
	DECLARE_WRITE8_MEMBER(thief_input_select_w);
	DECLARE_READ8_MEMBER(thief_io_r);
};


/*----------- defined in video/thief.c -----------*/

READ8_HANDLER( thief_context_ram_r );
WRITE8_HANDLER( thief_context_ram_w );
WRITE8_HANDLER( thief_context_bank_w );
WRITE8_HANDLER( thief_video_control_w );
WRITE8_HANDLER( thief_color_map_w );
WRITE8_HANDLER( thief_color_plane_w );
READ8_HANDLER( thief_videoram_r );
WRITE8_HANDLER( thief_videoram_w );
WRITE8_HANDLER( thief_blit_w );
READ8_HANDLER( thief_coprocessor_r );
WRITE8_HANDLER( thief_coprocessor_w );

VIDEO_START( thief );
SCREEN_UPDATE_IND16( thief );
