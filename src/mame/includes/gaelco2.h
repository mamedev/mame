#include "cpu/m68000/m68000.h"
#include "video/bufsprite.h"

class gaelco2_state : public driver_device
{
public:
	gaelco2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this,"maincpu"),
		m_spriteram(*this,"spriteram"),
		m_vregs(*this, "vregs"),
		m_snowboar_protection(*this, "snowboar_prot"){ }

	required_device<m68000_device> m_maincpu;
	required_device<buffered_spriteram16_device> m_spriteram;
	required_shared_ptr<UINT16> m_vregs;
	optional_shared_ptr<UINT16> m_snowboar_protection;

	int m_clr_gun_int;
	UINT8 m_analog_ports[2];
	UINT16 *m_videoram;
	tilemap_t *m_pant[2];
	int m_dual_monitor;

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
	DECLARE_CUSTOM_INPUT_MEMBER(wrally2_analog_bit_r);
	DECLARE_DRIVER_INIT(touchgo);
	DECLARE_DRIVER_INIT(snowboar);
	DECLARE_DRIVER_INIT(alighunt);
	DECLARE_DRIVER_INIT(bang);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen0);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen1);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen0_dual);
	TILE_GET_INFO_MEMBER(get_tile_info_gaelco2_screen1_dual);
	DECLARE_VIDEO_START(gaelco2);
	DECLARE_VIDEO_START(gaelco2_dual);
	UINT32 screen_update_gaelco2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gaelco2_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_gaelco2_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defined in machine/gaelco2.c -----------*/

TIMER_DEVICE_CALLBACK( bang_irq );
DECLARE_WRITE16_DEVICE_HANDLER( gaelco2_eeprom_cs_w );
DECLARE_WRITE16_DEVICE_HANDLER( gaelco2_eeprom_sk_w );
DECLARE_WRITE16_DEVICE_HANDLER( gaelco2_eeprom_data_w );
