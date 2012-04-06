/*************************************************************************

    Munch Mobile

*************************************************************************/

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *      m_vreg;
	UINT8 *      m_status_vram;
	UINT8 *      m_sprite_xpos;
	UINT8 *      m_sprite_attr;
	UINT8 *      m_sprite_tile;
	UINT8 *      m_videoram;

	/* video-related */
	bitmap_ind16     *m_tmpbitmap;
	int          m_palette_bank;
	int          m_flipscreen;

	/* misc */
	int          m_nmi_enable;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	DECLARE_WRITE8_MEMBER(mnchmobl_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_soundlatch_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_palette_bank_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_flipscreen_w);
};


/*----------- defined in video/munchmo.c -----------*/


PALETTE_INIT( mnchmobl );
VIDEO_START( mnchmobl );
SCREEN_UPDATE_IND16( mnchmobl );
