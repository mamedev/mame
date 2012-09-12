/*************************************************************************

    Munch Mobile

*************************************************************************/

class munchmo_state : public driver_device
{
public:
	munchmo_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_sprite_xpos(*this, "sprite_xpos"),
		m_sprite_tile(*this, "sprite_tile"),
		m_sprite_attr(*this, "sprite_attr"),
		m_videoram(*this, "videoram"),
		m_status_vram(*this, "status_vram"),
		m_vreg(*this, "vreg"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_sprite_xpos;
	required_shared_ptr<UINT8> m_sprite_tile;
	required_shared_ptr<UINT8> m_sprite_attr;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_status_vram;
	required_shared_ptr<UINT8> m_vreg;

	/* video-related */
	bitmap_ind16     *m_tmpbitmap;
	int          m_palette_bank;
	int          m_flipscreen;

	/* misc */
	int          m_nmi_enable;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	DECLARE_WRITE8_MEMBER(mnchmobl_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_soundlatch_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_ack_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_palette_bank_w);
	DECLARE_WRITE8_MEMBER(mnchmobl_flipscreen_w);
	DECLARE_READ8_MEMBER(munchmo_ay1reset_r);
	DECLARE_READ8_MEMBER(munchmo_ay2reset_r);
};


/*----------- defined in video/munchmo.c -----------*/


PALETTE_INIT( mnchmobl );
VIDEO_START( mnchmobl );
SCREEN_UPDATE_IND16( mnchmobl );
