/***************************************************************************

    Battle Lane Vol. 5

***************************************************************************/

class battlane_state : public driver_device
{
public:
	battlane_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_tileram(*this, "tileram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT8> m_tileram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t     *m_bg_tilemap;
	bitmap_ind8 m_screen_bitmap;
	int         m_video_ctrl;
	int         m_cpu_control;	/* CPU interrupt control register */

	/* devices */
	cpu_device *m_maincpu;
	cpu_device *m_subcpu;
	DECLARE_WRITE8_MEMBER(battlane_cpu_command_w);
	DECLARE_WRITE8_MEMBER(battlane_palette_w);
	DECLARE_WRITE8_MEMBER(battlane_scrollx_w);
	DECLARE_WRITE8_MEMBER(battlane_scrolly_w);
	DECLARE_WRITE8_MEMBER(battlane_tileram_w);
	DECLARE_WRITE8_MEMBER(battlane_spriteram_w);
	DECLARE_WRITE8_MEMBER(battlane_bitmap_w);
	DECLARE_WRITE8_MEMBER(battlane_video_ctrl_w);
	TILE_GET_INFO_MEMBER(get_tile_info_bg);
	TILEMAP_MAPPER_MEMBER(battlane_tilemap_scan_rows_2x2);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_battlane(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
