// license:???
// copyright-holders:insideoutboy, Nicola Salmoria
/***************************************************************************

    Car Jamboree

***************************************************************************/

class carjmbre_state : public driver_device
{
public:
	carjmbre_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_spriteram(*this, "spriteram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette") { }

	/* memory pointers */
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;

	/* video-related */
	tilemap_t *m_cj_tilemap;
	UINT8   m_flipscreen;
	UINT16  m_bgcolor;

	UINT8   m_nmi_mask;
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(carjmbre_flipscreen_w);
	DECLARE_WRITE8_MEMBER(carjmbre_bgcolor_w);
	DECLARE_WRITE8_MEMBER(carjmbre_8806_w);
	DECLARE_WRITE8_MEMBER(carjmbre_videoram_w);
	TILE_GET_INFO_MEMBER(get_carjmbre_tile_info);
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(carjmbre);
	UINT32 screen_update_carjmbre(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
};
