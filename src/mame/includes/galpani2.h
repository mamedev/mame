#include "includes/kaneko16.h"

class galpani2_state : public kaneko16_state
{
public:
	galpani2_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_subcpu(*this,"sub")
		{ }

	UINT16 *m_bg8[2];
	UINT16 *m_palette[2];
	UINT16 *m_bg8_scrollx[2];
	UINT16 *m_bg8_scrolly[2];
	UINT16 *m_bg15;
	UINT16 m_eeprom_word;
	UINT16 *m_ram;
	UINT16 *m_ram2;
	UINT16 m_old_mcu_nmi1;
	UINT16 m_old_mcu_nmi2;
	UINT16 *m_rombank;
	bitmap_t *m_bg8_bitmap[2];
	bitmap_t *m_bg15_bitmap;

	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_subcpu;
};


/*----------- defined in video/galpani2.c -----------*/

PALETTE_INIT( galpani2 );
VIDEO_START( galpani2 );
SCREEN_UPDATE( galpani2 );

WRITE16_HANDLER( galpani2_palette_0_w );
WRITE16_HANDLER( galpani2_palette_1_w );

WRITE16_HANDLER( galpani2_bg8_0_w );
WRITE16_HANDLER( galpani2_bg8_1_w );

WRITE16_HANDLER( galpani2_bg15_w );
