/***************************************************************************

  Fast Freddie/Jump Coaster hardware
  driver by Zsolt Vasvari

***************************************************************************/

#include "includes/galaxold.h"

class fastfred_state : public galaxold_state
{
public:
	fastfred_state(const machine_config &mconfig, device_type type, const char *tag)
		: galaxold_state(mconfig, type, tag),
		  m_videoram(*this, "videoram"),
		  m_spriteram(*this, "spriteram"),
		  m_attributesram(*this, "attributesram"),
		  m_background_color(*this, "bgcolor"),
		  m_imago_fg_videoram(*this, "imago_fg_vram") { }

	UINT8 m_imago_sprites[0x800*3];
	UINT16 m_imago_sprites_address;
	UINT8 m_imago_sprites_bank;
	int m_hardware_type;
	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_spriteram;
	required_shared_ptr<UINT8> m_attributesram;
	optional_shared_ptr<UINT8> m_background_color;
	optional_shared_ptr<UINT8> m_imago_fg_videoram;
	UINT16 m_charbank;
	UINT8 m_colorbank;
	tilemap_t *m_bg_tilemap;
	tilemap_t *m_fg_tilemap;
	tilemap_t *m_web_tilemap;

	UINT8 m_nmi_mask;
	UINT8 m_sound_nmi_mask;
	DECLARE_READ8_MEMBER(fastfred_custom_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom1_io_r);
	DECLARE_READ8_MEMBER(flyboy_custom2_io_r);
	DECLARE_READ8_MEMBER(jumpcoas_custom_io_r);
	DECLARE_READ8_MEMBER(boggy84_custom_io_r);
	DECLARE_WRITE8_MEMBER(imago_dma_irq_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_bank_w);
	DECLARE_WRITE8_MEMBER(imago_sprites_dma_w);
	DECLARE_READ8_MEMBER(imago_sprites_offset_r);
	DECLARE_WRITE8_MEMBER(nmi_mask_w);
	DECLARE_WRITE8_MEMBER(sound_nmi_mask_w);
	DECLARE_DRIVER_INIT(fastfred);
	DECLARE_DRIVER_INIT(flyboy);
	DECLARE_DRIVER_INIT(flyboyb);
	DECLARE_DRIVER_INIT(imago);
	DECLARE_DRIVER_INIT(boggy84);
	DECLARE_DRIVER_INIT(jumpcoas);
	DECLARE_DRIVER_INIT(boggy84b);
};


/*----------- defined in video/fastfred.c -----------*/

PALETTE_INIT( fastfred );
VIDEO_START( fastfred );
WRITE8_HANDLER( fastfred_videoram_w );
WRITE8_HANDLER( fastfred_attributes_w );
WRITE8_HANDLER( fastfred_charbank1_w );
WRITE8_HANDLER( fastfred_charbank2_w );
WRITE8_HANDLER( fastfred_colorbank1_w );
WRITE8_HANDLER( fastfred_colorbank2_w );
WRITE8_HANDLER( fastfred_flip_screen_x_w );
WRITE8_HANDLER( fastfred_flip_screen_y_w );
SCREEN_UPDATE_IND16( fastfred );

VIDEO_START( imago );
SCREEN_UPDATE_IND16( imago );
WRITE8_HANDLER( imago_fg_videoram_w );
WRITE8_HANDLER( imago_charbank_w );
