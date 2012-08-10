

class freekick_state : public driver_device
{
public:
	freekick_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t    *m_freek_tilemap;

	/* misc */
	int        m_inval;
	int        m_outval;
	int        m_cnt;	// used by oigas
	int        m_romaddr;
	int        m_spinner;
	int        m_nmi_en;
	int        m_ff_data;
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(coin_w);
	DECLARE_WRITE8_MEMBER(spinner_select_w);
	DECLARE_READ8_MEMBER(spinner_r);
	DECLARE_WRITE8_MEMBER(pbillrd_bankswitch_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_WRITE8_MEMBER(oigas_5_w);
	DECLARE_READ8_MEMBER(oigas_3_r);
	DECLARE_READ8_MEMBER(oigas_2_r);
	DECLARE_READ8_MEMBER(freekick_ff_r);
	DECLARE_WRITE8_MEMBER(freekick_ff_w);
	DECLARE_WRITE8_MEMBER(freek_videoram_w);
	DECLARE_WRITE8_MEMBER(snd_rom_addr_l_w);
	DECLARE_WRITE8_MEMBER(snd_rom_addr_h_w);
	DECLARE_READ8_MEMBER(snd_rom_r);
	DECLARE_DRIVER_INIT(gigas);
	DECLARE_DRIVER_INIT(gigasb);
	DECLARE_DRIVER_INIT(pbillrds);
};


/*----------- defined in video/freekick.c -----------*/

VIDEO_START(freekick);
SCREEN_UPDATE_IND16(gigas);
SCREEN_UPDATE_IND16(pbillrd);
SCREEN_UPDATE_IND16(freekick);
