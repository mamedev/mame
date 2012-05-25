/*************************************************************************

    Goal! '92

*************************************************************************/

class goal92_state : public driver_device
{
public:
	goal92_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_bg_data(*this, "bg_data"),
		m_fg_data(*this, "fg_data"),
		m_tx_data(*this, "tx_data"),
		m_spriteram(*this, "spriteram"),
		m_scrollram(*this, "scrollram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_bg_data;
	required_shared_ptr<UINT16> m_fg_data;
	required_shared_ptr<UINT16> m_tx_data;
	required_shared_ptr<UINT16> m_spriteram;
	required_shared_ptr<UINT16> m_scrollram;
//  UINT16 *    m_paletteram; // this currently use generic palette handling
	UINT16 *    m_buffered_spriteram;

	/* video-related */
	tilemap_t     *m_bg_layer;
	tilemap_t     *m_fg_layer;
	tilemap_t     *m_tx_layer;
	UINT16      m_fg_bank;

	/* misc */
	int         m_msm5205next;
	int         m_adpcm_toggle;

	/* devices */
	device_t *m_audiocpu;
	DECLARE_WRITE16_MEMBER(goal92_sound_command_w);
	DECLARE_READ16_MEMBER(goal92_inputs_r);
	DECLARE_WRITE8_MEMBER(adpcm_data_w);
	DECLARE_READ16_MEMBER(goal92_fg_bank_r);
	DECLARE_WRITE16_MEMBER(goal92_fg_bank_w);
	DECLARE_WRITE16_MEMBER(goal92_text_w);
	DECLARE_WRITE16_MEMBER(goal92_background_w);
	DECLARE_WRITE16_MEMBER(goal92_foreground_w);
	DECLARE_WRITE8_MEMBER(adpcm_control_w);
};





/*----------- defined in video/goal92.c -----------*/


VIDEO_START( goal92 );
SCREEN_UPDATE_IND16( goal92 );
SCREEN_VBLANK( goal92 );
