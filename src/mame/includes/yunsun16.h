/*************************************************************************

    Yun Sung 16 Bit Games

*************************************************************************/

class yunsun16_state : public driver_device
{
public:
	yunsun16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_vram_0(*this, "vram_0"),
		m_vram_1(*this, "vram_1"),
		m_scrollram_0(*this, "scrollram_0"),
		m_scrollram_1(*this, "scrollram_1"),
		m_priorityram(*this, "priorityram"),
		m_spriteram(*this, "spriteram"){ }

	/* memory pointers */
	required_shared_ptr<UINT16> m_vram_0;
	required_shared_ptr<UINT16> m_vram_1;
	required_shared_ptr<UINT16> m_scrollram_0;
	required_shared_ptr<UINT16> m_scrollram_1;
	required_shared_ptr<UINT16> m_priorityram;
//  UINT16 *    m_paletteram; // currently this uses generic palette handling
	required_shared_ptr<UINT16> m_spriteram;

	/* other video-related elements */
	tilemap_t     *m_tilemap_0;
	tilemap_t     *m_tilemap_1;
	int         m_sprites_scrolldx;
	int         m_sprites_scrolldy;

	/* devices */
	cpu_device *m_audiocpu;
	DECLARE_WRITE16_MEMBER(yunsun16_sound_bank_w);
	DECLARE_WRITE16_MEMBER(magicbub_sound_command_w);
	DECLARE_WRITE16_MEMBER(yunsun16_vram_0_w);
	DECLARE_WRITE16_MEMBER(yunsun16_vram_1_w);
	DECLARE_DRIVER_INIT(magicbub);
	TILEMAP_MAPPER_MEMBER(yunsun16_tilemap_scan_pages);
	TILE_GET_INFO_MEMBER(get_tile_info_0);
	TILE_GET_INFO_MEMBER(get_tile_info_1);
	virtual void machine_start();
	virtual void machine_reset();
	virtual void video_start();
	UINT32 screen_update_yunsun16(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
