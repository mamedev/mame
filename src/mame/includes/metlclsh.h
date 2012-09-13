/*************************************************************************

    Metal Clash

*************************************************************************/

class metlclsh_state : public driver_device
{
public:
	metlclsh_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_fgram(*this, "fgram"),
		m_spriteram(*this, "spriteram"),
		m_bgram(*this, "bgram"),
		m_scrollx(*this, "scrollx"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_fgram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_bgram;
	required_shared_ptr<UINT8> m_scrollx;
	UINT8 *        m_otherram;
//      UINT8 *        m_paletteram;    // currently this uses generic palette handling
//      UINT8 *        m_paletteram2;    // currently this uses generic palette handling

	/* video-related */
	tilemap_t      *m_bg_tilemap;
	tilemap_t      *m_fg_tilemap;
	UINT8          m_write_mask;
	UINT8          m_gfxbank;

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_subcpu;
	DECLARE_WRITE8_MEMBER(metlclsh_cause_irq);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi);
	DECLARE_WRITE8_MEMBER(metlclsh_cause_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_irq2);
	DECLARE_WRITE8_MEMBER(metlclsh_ack_nmi2);
	DECLARE_WRITE8_MEMBER(metlclsh_flipscreen_w);
	DECLARE_WRITE8_MEMBER(metlclsh_rambank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_gfxbank_w);
	DECLARE_WRITE8_MEMBER(metlclsh_bgram_w);
	DECLARE_WRITE8_MEMBER(metlclsh_fgram_w);
	DECLARE_INPUT_CHANGED_MEMBER(coin_inserted);
	TILEMAP_MAPPER_MEMBER(metlclsh_bgtilemap_scan);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	TILE_GET_INFO_MEMBER(get_fg_tile_info);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
};


/*----------- defined in video/metlclsh.c -----------*/



SCREEN_UPDATE_IND16( metlclsh );
