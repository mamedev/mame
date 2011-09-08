/***************************************************************************

                            -= Jaleco Mega System 1 =-

                    driver by   Luca Elia (l.elia@tin.it)


    This file contains definitions used across multiple megasys1
    and non megasys1 Jaleco games:

    * Scrolling layers handling
    * Code decryption handling

***************************************************************************/


class megasys1_state : public driver_device
{
public:
	megasys1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT16 *m_spriteram;
	UINT16 m_ip_select;
	UINT16 m_ip_select_values[5];
	UINT8 m_ignore_oki_status;
	UINT16 m_protection_val;
	int m_bank;
	UINT16 *m_scrollram[3];
	UINT16 *m_objectram;
	UINT16 *m_vregs;
	UINT16 *m_ram;
	int m_scrollx[3];
	int m_scrolly[3];
	int m_active_layers;
	int m_bits_per_color_code;
	int m_scroll_flag[3];
	int m_sprite_bank;
	int m_screen_flag;
	int m_sprite_flag;
	int m_8x8_scroll_factor[3];
	int m_16x16_scroll_factor[3];
	tilemap_t *m_tmap[3];
	tilemap_t *m_tilemap[3][2][4];
	int m_hardware_type_z;
	UINT16 *m_buffer_objectram;
	UINT16 *m_buffer2_objectram;
	UINT16 *m_buffer_spriteram16;
	UINT16 *m_buffer2_spriteram16;
	int m_layers_order[16];

	int m_mcu_hs;
	UINT16 m_mcu_hs_ram[0x10];
};


/*----------- defined in video/megasys1.c -----------*/

VIDEO_START( megasys1 );
SCREEN_UPDATE( megasys1 );
SCREEN_EOF( megasys1 );

PALETTE_INIT( megasys1 );

READ16_HANDLER( megasys1_vregs_C_r );

WRITE16_HANDLER( megasys1_vregs_A_w );
WRITE16_HANDLER( megasys1_vregs_C_w );
WRITE16_HANDLER( megasys1_vregs_D_w );

WRITE16_HANDLER( megasys1_scrollram_0_w );
WRITE16_HANDLER( megasys1_scrollram_1_w );
WRITE16_HANDLER( megasys1_scrollram_2_w );
