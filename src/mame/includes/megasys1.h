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
	DECLARE_READ16_MEMBER(ip_select_r);
	DECLARE_WRITE16_MEMBER(ip_select_w);
	DECLARE_READ16_MEMBER(protection_peekaboo_r);
	DECLARE_WRITE16_MEMBER(protection_peekaboo_w);
	DECLARE_READ16_MEMBER(megasys1A_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(megasys1A_mcu_hs_w);
	DECLARE_READ16_MEMBER(edfbl_input_r);
	DECLARE_READ16_MEMBER(iganinju_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(iganinju_mcu_hs_w);
	DECLARE_READ16_MEMBER(soldamj_spriteram16_r);
	DECLARE_WRITE16_MEMBER(soldamj_spriteram16_w);
	DECLARE_READ16_MEMBER(stdragon_mcu_hs_r);
	DECLARE_WRITE16_MEMBER(stdragon_mcu_hs_w);
	DECLARE_READ16_MEMBER(monkelf_input_r);
	DECLARE_WRITE16_MEMBER(megasys1_scrollram_0_w);
	DECLARE_WRITE16_MEMBER(megasys1_scrollram_1_w);
	DECLARE_WRITE16_MEMBER(megasys1_scrollram_2_w);
	DECLARE_WRITE16_MEMBER(megasys1_vregs_A_w);
	DECLARE_READ16_MEMBER(megasys1_vregs_C_r);
	DECLARE_WRITE16_MEMBER(megasys1_vregs_C_w);
	DECLARE_WRITE16_MEMBER(megasys1_vregs_D_w);
	void megasys1_set_vreg_flag(int which, int data);
};


/*----------- defined in video/megasys1.c -----------*/

VIDEO_START( megasys1 );
SCREEN_UPDATE_IND16( megasys1 );
SCREEN_VBLANK( megasys1 );

PALETTE_INIT( megasys1 );



