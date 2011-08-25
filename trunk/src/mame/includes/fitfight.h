
class fitfight_state : public driver_device
{
public:
	fitfight_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *  m_fof_100000;
	UINT16 *  m_fof_600000;
	UINT16 *  m_fof_700000;
	UINT16 *  m_fof_800000;
	UINT16 *  m_fof_900000;
	UINT16 *  m_fof_a00000;
	UINT16 *  m_fof_bak_tileram;
	UINT16 *  m_fof_mid_tileram;
	UINT16 *  m_fof_txt_tileram;
	UINT16 *  m_spriteram;
//  UINT16 *  m_paletteram;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t  *m_fof_bak_tilemap;
	tilemap_t  *m_fof_mid_tilemap;
	tilemap_t  *m_fof_txt_tilemap;

	/* misc */
	int      m_bbprot_kludge;
	UINT16   m_fof_700000_data;
};


/*----------- defined in video/fitfight.c -----------*/

WRITE16_HANDLER( fof_bak_tileram_w );
WRITE16_HANDLER( fof_mid_tileram_w );
WRITE16_HANDLER( fof_txt_tileram_w );

VIDEO_START(fitfight);
SCREEN_UPDATE(fitfight);
