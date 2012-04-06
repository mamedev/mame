/*************************************************************************

    Mermaid

*************************************************************************/

class mermaid_state : public driver_device
{
public:
	mermaid_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT8 *    m_videoram;
	UINT8 *    m_colorram;
	UINT8 *    m_videoram2;
	UINT8 *    m_spriteram;
	UINT8 *    m_bg_scrollram;
	UINT8 *    m_fg_scrollram;
	UINT8 *    m_ay8910_enable;
	size_t     m_spriteram_size;

	/* video-related */
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	bitmap_ind16 m_helper;
	bitmap_ind16 m_helper2;
	int m_coll_bit0;
	int m_coll_bit1;
	int m_coll_bit2;
	int m_coll_bit3;
	int m_coll_bit6;
	int m_rougien_gfxbank1;
	int m_rougien_gfxbank2;

	/* sound-related */
	UINT32   m_adpcm_pos;
	UINT32   m_adpcm_end;
	UINT8    m_adpcm_idle;
	int      m_adpcm_data;
	UINT8    m_adpcm_trigger;
	UINT8    m_adpcm_rom_sel;
	UINT8    m_adpcm_play_reg;

	/* devices */
	device_t *m_maincpu;
	device_t *m_ay1;
	device_t *m_ay2;

	UINT8    m_nmi_mask;
	DECLARE_WRITE8_MEMBER(mermaid_ay8910_write_port_w);
	DECLARE_WRITE8_MEMBER(mermaid_ay8910_control_port_w);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(rougien_sample_rom_lo_w);
	DECLARE_WRITE8_MEMBER(rougien_sample_rom_hi_w);
	DECLARE_WRITE8_MEMBER(rougien_sample_playback_w);
	DECLARE_WRITE8_MEMBER(mermaid_videoram2_w);
	DECLARE_WRITE8_MEMBER(mermaid_videoram_w);
	DECLARE_WRITE8_MEMBER(mermaid_colorram_w);
	DECLARE_WRITE8_MEMBER(mermaid_flip_screen_x_w);
	DECLARE_WRITE8_MEMBER(mermaid_flip_screen_y_w);
	DECLARE_WRITE8_MEMBER(mermaid_bg_scroll_w);
	DECLARE_WRITE8_MEMBER(mermaid_fg_scroll_w);
	DECLARE_WRITE8_MEMBER(rougien_gfxbankswitch1_w);
	DECLARE_WRITE8_MEMBER(rougien_gfxbankswitch2_w);
	DECLARE_READ8_MEMBER(mermaid_collision_r);
};



/*----------- defined in video/mermaid.c -----------*/


PALETTE_INIT( mermaid );
PALETTE_INIT( rougien );
VIDEO_START( mermaid );
SCREEN_UPDATE_IND16( mermaid );
SCREEN_VBLANK( mermaid );
