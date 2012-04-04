/*************************************************************************

    Rainbow Islands

*************************************************************************/

class rbisland_state : public driver_device
{
public:
	rbisland_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_spriteram;
//  UINT16 *    paletteram;    // currently this uses generic palette handling
	size_t      m_spriteram_size;

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	UINT8       m_jumping_latch;

	/* c-chip */
	UINT8       *m_CRAM[8];
	int         m_extra_version;
	UINT8       m_current_bank;

	/* devices */
	device_t *m_maincpu;
	device_t *m_audiocpu;
	device_t *m_pc080sn;
	device_t *m_pc090oj;
	DECLARE_WRITE16_MEMBER(jumping_sound_w);
	DECLARE_READ8_MEMBER(jumping_latch_r);
};


/*----------- defined in machine/rainbow.c -----------*/

void rbisland_cchip_init(running_machine &machine, int version);
READ16_HANDLER( rbisland_cchip_ctrl_r );
READ16_HANDLER( rbisland_cchip_ram_r );
WRITE16_HANDLER( rbisland_cchip_ctrl_w );
WRITE16_HANDLER( rbisland_cchip_bank_w );
WRITE16_HANDLER( rbisland_cchip_ram_w );


/*----------- defined in video/rainbow.c -----------*/

SCREEN_UPDATE_IND16( rainbow );
VIDEO_START( jumping );
SCREEN_UPDATE_IND16( jumping );

WRITE16_HANDLER( jumping_spritectrl_w );
WRITE16_HANDLER( rbisland_spritectrl_w );
