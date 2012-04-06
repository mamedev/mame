/*************************************************************************

    Angel Kids

*************************************************************************/

class angelkds_state : public driver_device
{
public:
	angelkds_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_paletteram;
	UINT8 *    m_spriteram;
	UINT8 *    m_txvideoram;
	UINT8 *    m_bgtopvideoram;
	UINT8 *    m_bgbotvideoram;

	tilemap_t    *m_tx_tilemap;
	tilemap_t    *m_bgbot_tilemap;
	tilemap_t    *m_bgtop_tilemap;
	int        m_txbank;
	int        m_bgbotbank;
	int        m_bgtopbank;

	UINT8      m_sound[4];
	UINT8      m_sound2[4];
	UINT8      m_layer_ctrl;

	/* devices */
	device_t *m_subcpu;
	DECLARE_WRITE8_MEMBER(angelkds_cpu_bank_write);
	DECLARE_READ8_MEMBER(angelkds_input_r);
	DECLARE_WRITE8_MEMBER(angelkds_main_sound_w);
	DECLARE_READ8_MEMBER(angelkds_main_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_sub_sound_w);
	DECLARE_READ8_MEMBER(angelkds_sub_sound_r);
	DECLARE_WRITE8_MEMBER(angelkds_txvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_txbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgtopscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotvideoram_w);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotbank_write);
	DECLARE_WRITE8_MEMBER(angelkds_bgbotscroll_write);
	DECLARE_WRITE8_MEMBER(angelkds_layer_ctrl_write);
	DECLARE_WRITE8_MEMBER(angelkds_paletteram_w);
};


/*----------- defined in video/angelkds.c -----------*/




VIDEO_START( angelkds );
SCREEN_UPDATE_IND16( angelkds );
