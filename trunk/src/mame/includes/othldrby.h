/*************************************************************************

    Othello Derby

*************************************************************************/

#define OTHLDRBY_VREG_SIZE   18

class othldrby_state : public driver_device
{
public:
	othldrby_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* memory pointers */
	UINT16 *     m_vram;
	UINT16 *     m_buf_spriteram;
	UINT16 *     m_buf_spriteram2;

	/* video-related */
	tilemap_t    *m_bg_tilemap[3];
	UINT16       m_vreg[OTHLDRBY_VREG_SIZE];
	UINT32       m_vram_addr;
	UINT32       m_vreg_addr;

	/* misc */
	int          m_toggle;
	DECLARE_READ16_MEMBER(othldrby_scanline_r);
	DECLARE_WRITE16_MEMBER(coinctrl_w);
	DECLARE_WRITE16_MEMBER(calendar_w);
	DECLARE_READ16_MEMBER(calendar_r);
	DECLARE_WRITE16_MEMBER(othldrby_videoram_addr_w);
	DECLARE_READ16_MEMBER(othldrby_videoram_r);
	DECLARE_WRITE16_MEMBER(othldrby_videoram_w);
	DECLARE_WRITE16_MEMBER(othldrby_vreg_addr_w);
	DECLARE_WRITE16_MEMBER(othldrby_vreg_w);
};


/*----------- defined in video/othldrby.c -----------*/


VIDEO_START( othldrby );
SCREEN_VBLANK( othldrby );
SCREEN_UPDATE_IND16( othldrby );
