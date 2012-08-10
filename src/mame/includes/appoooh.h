

class appoooh_state : public driver_device
{
public:
	appoooh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_spriteram(*this, "spriteram"),
		m_fg_videoram(*this, "fg_videoram"),
		m_fg_colorram(*this, "fg_colorram"),
		m_spriteram_2(*this, "spriteram_2"),
		m_bg_videoram(*this, "bg_videoram"),
		m_bg_colorram(*this, "bg_colorram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_fg_videoram;
	required_shared_ptr<UINT8> m_fg_colorram;
	required_shared_ptr<UINT8> m_spriteram_2;
	required_shared_ptr<UINT8> m_bg_videoram;
	required_shared_ptr<UINT8> m_bg_colorram;

	/* video-related */
	tilemap_t  *m_fg_tilemap;
	tilemap_t  *m_bg_tilemap;
	int m_scroll_x;
	int m_priority;

	/* sound-related */
	UINT32   m_adpcm_data;
	UINT32   m_adpcm_address;

	/* devices */
	device_t *m_adpcm;

	UINT8 m_nmi_mask;
	DECLARE_WRITE8_MEMBER(appoooh_adpcm_w);
	DECLARE_WRITE8_MEMBER(appoooh_scroll_w);
	DECLARE_WRITE8_MEMBER(appoooh_fg_videoram_w);
	DECLARE_WRITE8_MEMBER(appoooh_fg_colorram_w);
	DECLARE_WRITE8_MEMBER(appoooh_bg_videoram_w);
	DECLARE_WRITE8_MEMBER(appoooh_bg_colorram_w);
	DECLARE_WRITE8_MEMBER(appoooh_out_w);
	DECLARE_DRIVER_INIT(robowres);
	DECLARE_DRIVER_INIT(robowresb);
};

#define CHR1_OFST   0x00  /* palette page of char set #1 */
#define CHR2_OFST   0x10  /* palette page of char set #2 */


/* ----------- defined in video/appoooh.c -----------*/

PALETTE_INIT( appoooh );
PALETTE_INIT( robowres );
VIDEO_START( appoooh );
SCREEN_UPDATE_IND16( appoooh );
SCREEN_UPDATE_IND16( robowres );
