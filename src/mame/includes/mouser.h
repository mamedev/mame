/*************************************************************************

    Mouser

*************************************************************************/

class mouser_state : public driver_device
{
public:
	mouser_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_spriteram;
	size_t     m_spriteram_size;

	/* misc */
	UINT8      m_sound_byte;
	UINT8      m_nmi_enable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(mouser_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mouser_sound_interrupt_w);
	DECLARE_READ8_MEMBER(mouser_sound_byte_r);
	DECLARE_WRITE8_MEMBER(mouser_sound_nmi_clear_w);
	DECLARE_WRITE8_MEMBER(mouser_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(mouser_flip_screen_y_w);
};

/*----------- defined in video/mouser.c -----------*/


PALETTE_INIT( mouser );
SCREEN_UPDATE_IND16( mouser );
