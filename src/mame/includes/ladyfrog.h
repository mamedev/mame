/*************************************************************************

    Lady Frog

*************************************************************************/

class ladyfrog_state : public driver_device
{
public:
	ladyfrog_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_scrlram(*this, "scrlram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	UINT8 *    m_spriteram;
	required_shared_ptr<UINT8> m_scrlram;
//      UINT8 *    m_paletteram;    // currently this uses generic palette handling
//      UINT8 *    m_paletteram2;   // currently this uses generic palette handling

	/* video-related */
	tilemap_t    *m_bg_tilemap;
	int        m_tilebank;
	int        m_palette_bank;
	int        m_spritetilebase;

	/* misc */
	int        m_sound_nmi_enable;
	int        m_pending_nmi;
	int        m_snd_flag;
	UINT8      m_snd_data;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_READ8_MEMBER(from_snd_r);
	DECLARE_WRITE8_MEMBER(to_main_w);
	DECLARE_WRITE8_MEMBER(sound_cpu_reset_w);
	DECLARE_WRITE8_MEMBER(sound_command_w);
	DECLARE_WRITE8_MEMBER(nmi_disable_w);
	DECLARE_WRITE8_MEMBER(nmi_enable_w);
	DECLARE_READ8_MEMBER(snd_flag_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_spriteram_w);
	DECLARE_READ8_MEMBER(ladyfrog_spriteram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_videoram_w);
	DECLARE_READ8_MEMBER(ladyfrog_videoram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_palette_w);
	DECLARE_READ8_MEMBER(ladyfrog_palette_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_gfxctrl_w);
	DECLARE_WRITE8_MEMBER(ladyfrog_gfxctrl2_w);
	DECLARE_READ8_MEMBER(ladyfrog_gfxctrl_r);
	DECLARE_READ8_MEMBER(ladyfrog_scrlram_r);
	DECLARE_WRITE8_MEMBER(ladyfrog_scrlram_w);
	DECLARE_WRITE8_MEMBER(unk_w);
	TILE_GET_INFO_MEMBER(get_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	DECLARE_VIDEO_START(toucheme);
	DECLARE_VIDEO_START(ladyfrog_common);
	UINT32 screen_update_ladyfrog(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in video/ladyfrog.c -----------*/






