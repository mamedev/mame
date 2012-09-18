/*****************************************************************************
 *
 * includes/aquarius.h
 *
 ****************************************************************************/

#ifndef __AQUARIUS__
#define __AQUARIUS__

class aquarius_state : public driver_device
{
public:
	aquarius_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) ,
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"){ }

	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_scrambler;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_tilemap;
	DECLARE_WRITE8_MEMBER(aquarius_videoram_w);
	DECLARE_WRITE8_MEMBER(aquarius_colorram_w);
	DECLARE_READ8_MEMBER(cassette_r);
	DECLARE_WRITE8_MEMBER(cassette_w);
	DECLARE_READ8_MEMBER(vsync_r);
	DECLARE_WRITE8_MEMBER(mapper_w);
	DECLARE_READ8_MEMBER(printer_r);
	DECLARE_WRITE8_MEMBER(printer_w);
	DECLARE_READ8_MEMBER(keyboard_r);
	DECLARE_WRITE8_MEMBER(scrambler_w);
	DECLARE_READ8_MEMBER(cartridge_r);
	DECLARE_READ8_MEMBER(floppy_r);
	DECLARE_WRITE8_MEMBER(floppy_w);
	DECLARE_DRIVER_INIT(aquarius);
	TILE_GET_INFO_MEMBER(aquarius_gettileinfo);
	virtual void video_start();
	virtual void palette_init();
	UINT32 screen_update_aquarius(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};
#endif /* AQUARIUS_H_ */
