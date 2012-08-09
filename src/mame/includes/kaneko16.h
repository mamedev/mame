/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

#ifndef __KANEKO16_H__
#define __KANEKO16_H__

#include "machine/nvram.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"



typedef struct
{
	UINT16 x1p, y1p, x1s, y1s;
	UINT16 x2p, y2p, x2s, y2s;

	INT16 x12, y12, x21, y21;

	UINT16 mult_a, mult_b;
} calc1_hit_t;

typedef struct
{
	int x1p, y1p, z1p, x1s, y1s, z1s;
	int x2p, y2p, z2p, x2s, y2s, z2s;

	int x1po, y1po, z1po, x1so, y1so, z1so;
	int x2po, y2po, z2po, x2so, y2so, z2so;

	int x12, y12, z12, x21, y21, z21;

	int x_coll, y_coll, z_coll;

	int x1tox2, y1toy2, z1toz2;

	UINT16 mult_a, mult_b;

	UINT16 flags;
	UINT16 mode;
} calc3_hit_t;

typedef struct
{
	int mcu_status;
	int mcu_command_offset;
	UINT16 mcu_crc;
	UINT8 decryption_key_byte;
	UINT8 alternateswaps;
	UINT8 shift;
	UINT8 subtracttype;
	UINT8 mode;
	UINT8 blocksize_offset;
	UINT16 dataend;
	UINT16 database;
	int data_header[2];
	UINT32 writeaddress;
	UINT32 writeaddress_current;
	UINT16 dsw_addr;
	UINT16 eeprom_addr;
	UINT16 poll_addr;
	UINT16 checksumaddress;
} calc3_t;

class kaneko16_state : public driver_device
{
public:
	kaneko16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spriteram(*this, "spriteram"),
		m_mcu_ram(*this, "mcu_ram"),
		m_mainram(*this, "mainram"),
		m_view2_0(*this, "view2_0"),
		m_view2_1(*this, "view2_1"),
		m_kaneko_spr(*this, "kan_spr")
	{ }


	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_mcu_ram;
	optional_shared_ptr<UINT16> m_mainram;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;
	optional_device<kaneko_view2_tilemap_device> m_view2_1;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;

	UINT8 m_nvram_save[128];

	calc1_hit_t m_hit;
	calc3_hit_t m_hit3;
	calc3_t m_calc3;
	void (*m_toybox_mcu_run)(running_machine &machine);
	UINT16 m_toybox_mcu_com[4];
	UINT16 m_disp_enable;

	int VIEW2_2_pri;


	DECLARE_READ16_MEMBER(kaneko16_rnd_r);
	DECLARE_WRITE16_MEMBER(kaneko16_coin_lockout_w);
	DECLARE_WRITE16_MEMBER(kaneko16_soundlatch_w);
	DECLARE_WRITE16_MEMBER(kaneko16_eeprom_w);
	DECLARE_WRITE16_MEMBER(bloodwar_coin_lockout_w);
	DECLARE_READ16_MEMBER(gtmr_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_IN1_r);
	DECLARE_WRITE16_MEMBER(shogwarr_oki_bank_w);
	DECLARE_WRITE16_MEMBER(brapboys_oki_bank_w);
	DECLARE_READ16_MEMBER(galpanib_calc_r);
	DECLARE_WRITE16_MEMBER(galpanib_calc_w);
	DECLARE_WRITE16_MEMBER(bloodwar_calc_w);
	DECLARE_READ16_MEMBER(bloodwar_calc_r);
	DECLARE_WRITE16_MEMBER(calc3_mcu_ram_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com0_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com1_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com2_w);
	DECLARE_WRITE16_MEMBER(calc3_mcu_com3_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com0_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com1_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com2_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com3_w);
	DECLARE_READ16_MEMBER(toybox_mcu_status_r);
	void calc3_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_);
	void toybox_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_);
	DECLARE_WRITE16_MEMBER(shogwarr_calc_w);
	DECLARE_READ16_MEMBER(shogwarr_calc_r);
	DECLARE_WRITE16_MEMBER(kaneko16_display_enable);

	DECLARE_READ16_MEMBER(kaneko16_ay1_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay1_YM2149_w);
	DECLARE_READ16_MEMBER(kaneko16_ay2_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay2_YM2149_w);
	DECLARE_WRITE16_MEMBER(bakubrkr_oki_bank_sw);
	DECLARE_WRITE16_MEMBER(bloodwar_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bloodwar_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_1_bank_w);
	DECLARE_WRITE8_MEMBER(kaneko16_eeprom_reset_w);
};

class kaneko16_berlwall_state : public kaneko16_state
{
public:
	kaneko16_berlwall_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_bg15_reg(*this, "bg15_reg"),
		m_bg15_select(*this, "bg15_select")
	{
	}

	optional_shared_ptr<UINT16> m_bg15_reg;
	optional_shared_ptr<UINT16> m_bg15_select;

	bitmap_ind16 m_bg15_bitmap;

	DECLARE_READ16_MEMBER(kaneko16_bg15_select_r);
	DECLARE_WRITE16_MEMBER(kaneko16_bg15_select_w);
	DECLARE_READ16_MEMBER(kaneko16_bg15_reg_r);
	DECLARE_WRITE16_MEMBER(kaneko16_bg15_reg_w);

};

/*----------- defined in machine/kaneko16.c -----------*/



void calc3_mcu_init(running_machine &machine);

void toybox_mcu_init(running_machine &machine);

void bloodwar_mcu_run(running_machine &machine);
void bonkadv_mcu_run(running_machine &machine);
void gtmr_mcu_run(running_machine &machine);
void calc3_mcu_run(running_machine &machine);

void toxboy_handle_04_subcommand(running_machine& machine, UINT8 mcu_subcmd, UINT16*mcu_ram);

DRIVER_INIT( decrypt_toybox_rom );
DRIVER_INIT( decrypt_toybox_rom_alt );
DRIVER_INIT( calc3_scantables );



/*----------- defined in drivers/kaneko16.c -----------*/

MACHINE_RESET( kaneko16 );

/*----------- defined in video/kaneko16.c -----------*/





void kaneko16_draw_sprites(running_machine &machine, bitmap_ind16 &bitmap, const rectangle &cliprect);



PALETTE_INIT( berlwall );

VIDEO_START( kaneko16 );
VIDEO_START( berlwall );

SCREEN_UPDATE_IND16( kaneko16 );
SCREEN_UPDATE_IND16( berlwall );


#endif
