class iqblock_state : public driver_device
{
public:
	iqblock_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu")
		{ }

	UINT8 *m_rambase;
	UINT8 *m_bgvideoram;
	UINT8 *m_fgvideoram;
	int m_videoenable;
	int m_video_type;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;

	required_device<cpu_device> m_maincpu;
	DECLARE_WRITE8_MEMBER(iqblock_prot_w);
	DECLARE_WRITE8_MEMBER(grndtour_prot_w);
	DECLARE_WRITE8_MEMBER(iqblock_irqack_w);
	DECLARE_READ8_MEMBER(extrarom_r);
	DECLARE_WRITE8_MEMBER(iqblock_fgvideoram_w);
	DECLARE_WRITE8_MEMBER(iqblock_bgvideoram_w);
	DECLARE_READ8_MEMBER(iqblock_bgvideoram_r);
	DECLARE_WRITE8_MEMBER(iqblock_fgscroll_w);
};


/*----------- defined in video/iqblock.c -----------*/


VIDEO_START( iqblock );
SCREEN_UPDATE_IND16( iqblock );
