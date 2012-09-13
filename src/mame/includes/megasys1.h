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
		: driver_device(mconfig, type, tag) ,
		m_vregs(*this, "vregs"),
		m_objectram(*this, "objectram"),
		m_scrollram(*this, "scrollram"),
		m_ram(*this, "ram"){ }

	required_shared_ptr<UINT16> m_vregs;
	required_shared_ptr<UINT16> m_objectram;
	required_shared_ptr_array<UINT16,3> m_scrollram;
	required_shared_ptr<UINT16> m_ram;

	UINT16 *m_spriteram;
	UINT16 m_ip_select;
	UINT16 m_ip_select_values[5];
	UINT8 m_ignore_oki_status;
	UINT16 m_protection_val;
	int m_bank;
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
	DECLARE_READ8_MEMBER(oki_status_1_r);
	DECLARE_READ8_MEMBER(oki_status_2_r);
	DECLARE_WRITE16_MEMBER(okim6295_both_1_w);
	DECLARE_WRITE16_MEMBER(okim6295_both_2_w);
	DECLARE_DRIVER_INIT(64street);
	DECLARE_DRIVER_INIT(chimerab);
	DECLARE_DRIVER_INIT(peekaboo);
	DECLARE_DRIVER_INIT(soldam);
	DECLARE_DRIVER_INIT(astyanax);
	DECLARE_DRIVER_INIT(stdragon);
	DECLARE_DRIVER_INIT(hayaosi1);
	DECLARE_DRIVER_INIT(soldamj);
	DECLARE_DRIVER_INIT(phantasm);
	DECLARE_DRIVER_INIT(jitsupro);
	DECLARE_DRIVER_INIT(iganinju);
	DECLARE_DRIVER_INIT(cybattlr);
	DECLARE_DRIVER_INIT(rodlandj);
	DECLARE_DRIVER_INIT(avspirit);
	DECLARE_DRIVER_INIT(monkelf);
	DECLARE_DRIVER_INIT(edf);
	DECLARE_DRIVER_INIT(bigstrik);
	DECLARE_DRIVER_INIT(rodland);
	DECLARE_DRIVER_INIT(edfbl);
	DECLARE_DRIVER_INIT(stdragona);
	TILEMAP_MAPPER_MEMBER(megasys1_scan_8x8);
	TILEMAP_MAPPER_MEMBER(megasys1_scan_16x16);
	TILE_GET_INFO_MEMBER(megasys1_get_scroll_tile_info_8x8);
	TILE_GET_INFO_MEMBER(megasys1_get_scroll_tile_info_16x16);
	DECLARE_MACHINE_RESET(megasys1);
	DECLARE_VIDEO_START(megasys1);
	DECLARE_PALETTE_INIT(megasys1);
	DECLARE_MACHINE_RESET(megasys1_hachoo);
};


/*----------- defined in video/megasys1.c -----------*/


SCREEN_UPDATE_IND16( megasys1 );
SCREEN_VBLANK( megasys1 );





