/*************************************************************************

    Super Slams

*************************************************************************/

class suprslam_state : public driver_device
{
public:
	suprslam_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *    m_screen_videoram;
	UINT16 *    m_bg_videoram;
	UINT16 *    m_sp_videoram;
	UINT16 *    m_spriteram;
//  UINT16 *    m_paletteram; // this currently uses generic palette handling

	/* video-related */
	tilemap_t     *m_screen_tilemap;
	tilemap_t     *m_bg_tilemap;
	UINT16      m_screen_bank;
	UINT16      m_bg_bank;
	UINT16      *m_spr_ctrl;
	UINT16      *m_screen_vregs;

	/* misc */
	int         m_pending_command;

	/* devices */
	device_t *m_audiocpu;
	device_t *m_k053936;
	DECLARE_WRITE16_MEMBER(sound_command_w);
	DECLARE_READ16_MEMBER(pending_command_r);
	DECLARE_WRITE8_MEMBER(pending_command_clear_w);
	DECLARE_WRITE8_MEMBER(suprslam_sh_bankswitch_w);
	DECLARE_WRITE16_MEMBER(suprslam_screen_videoram_w);
	DECLARE_WRITE16_MEMBER(suprslam_bg_videoram_w);
	DECLARE_WRITE16_MEMBER(suprslam_bank_w);
};


/*----------- defined in video/suprslam.c -----------*/


VIDEO_START( suprslam );
SCREEN_UPDATE_IND16( suprslam );
