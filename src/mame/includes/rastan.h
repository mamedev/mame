/*************************************************************************

    Rastan

*************************************************************************/

class rastan_state : public driver_device
{
public:
	rastan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
//  UINT16 *    paletteram; // this currently uses generic palette handlers

	/* video-related */
	UINT16      m_sprite_ctrl;
	UINT16      m_sprites_flipscreen;

	/* misc */
	int         m_adpcm_pos;
	int         m_adpcm_data;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_audiocpu;
	device_t *m_pc090oj;
	device_t *m_pc080sn;
	DECLARE_WRITE8_MEMBER(rastan_msm5205_address_w);
	DECLARE_WRITE16_MEMBER(rastan_spritectrl_w);
	DECLARE_WRITE8_MEMBER(rastan_bankswitch_w);
	DECLARE_WRITE8_MEMBER(rastan_msm5205_start_w);
	DECLARE_WRITE8_MEMBER(rastan_msm5205_stop_w);
	virtual void machine_start();
	virtual void machine_reset();
	UINT32 screen_update_rastan(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
