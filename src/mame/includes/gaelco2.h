#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"

class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram")
		{ }

	UINT16 *m_snowboar_protection;
	UINT16 *m_vregs;
	int m_clr_gun_int;
	UINT8 m_analog_ports[2];
	UINT16 *m_videoram;
	tilemap_t *m_pant[2];
	int m_dual_monitor;

	required_device<m68000_device> m_maincpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	DECLARE_READ16_MEMBER(p1_gun_x);
	DECLARE_READ16_MEMBER(p1_gun_y);
	DECLARE_READ16_MEMBER(p2_gun_x);
	DECLARE_READ16_MEMBER(p2_gun_y);
	DECLARE_READ16_MEMBER(dallas_kludge_r);
	DECLARE_WRITE16_MEMBER(gaelco2_coin_w);
	DECLARE_WRITE16_MEMBER(gaelco2_coin2_w);
	DECLARE_WRITE16_MEMBER(wrally2_coin_w);
	DECLARE_WRITE16_MEMBER(touchgo_coin_w);
	DECLARE_WRITE16_MEMBER(bang_clr_gun_int_w);
	DECLARE_WRITE16_MEMBER(wrally2_adc_clk);
	DECLARE_WRITE16_MEMBER(wrally2_adc_cs);
	DECLARE_READ16_MEMBER(snowboar_protection_r);
	DECLARE_WRITE16_MEMBER(snowboar_protection_w);
	DECLARE_WRITE16_MEMBER(gaelco2_vram_w);
	DECLARE_WRITE16_MEMBER(gaelco2_palette_w);
};


/*----------- defined in machine/gaelco2.c -----------*/

DRIVER_INIT( alighunt );
DRIVER_INIT( touchgo );
DRIVER_INIT( snowboar );
DRIVER_INIT( bang );
TIMER_DEVICE_CALLBACK( bang_irq );
CUSTOM_INPUT( wrally2_analog_bit_r );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_cs_w );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_sk_w );
WRITE16_DEVICE_HANDLER( gaelco2_eeprom_data_w );

/*----------- defined in video/gaelco2.c -----------*/

SCREEN_UPDATE_IND16( gaelco2 );
VIDEO_START( gaelco2 );
SCREEN_UPDATE_IND16( gaelco2_left );
SCREEN_UPDATE_IND16( gaelco2_right );
VIDEO_START( gaelco2_dual );
