/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

#ifndef __KANEKO16_H__
#define __KANEKO16_H__

#include "machine/nvram.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"
#include "machine/kaneko_calc3.h"


#define MCU_RESPONSE(d) memcpy(&state->m_mcu_ram[mcu_offset], d, sizeof(d))


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

	DECLARE_WRITE16_MEMBER(toybox_mcu_com0_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com1_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com2_w);
	DECLARE_WRITE16_MEMBER(toybox_mcu_com3_w);
	DECLARE_READ16_MEMBER(toybox_mcu_status_r);

	void toybox_mcu_com_w(offs_t offset, UINT16 data, UINT16 mem_mask, int _n_);

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

class kaneko16_shogwarr_state : public kaneko16_state
{
public:
	kaneko16_shogwarr_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag),
		m_calc3_prot(*this, "calc3_prot")
	{
	}

	optional_device<kaneko_calc3_device> m_calc3_prot;

	DECLARE_WRITE16_MEMBER(shogwarr_oki_bank_w);
	DECLARE_WRITE16_MEMBER(brapboys_oki_bank_w);
};


/*----------- defined in machine/kaneko16.c -----------*/



void toybox_mcu_init(running_machine &machine);

void bloodwar_mcu_run(running_machine &machine);
void bonkadv_mcu_run(running_machine &machine);
void gtmr_mcu_run(running_machine &machine);
void calc3_mcu_run(running_machine &machine);

void toxboy_handle_04_subcommand(running_machine& machine, UINT8 mcu_subcmd, UINT16*mcu_ram);

DRIVER_INIT( decrypt_toybox_rom );
DRIVER_INIT( decrypt_toybox_rom_alt );


/*----------- defined in drivers/kaneko16.c -----------*/

MACHINE_RESET( kaneko16 );

/*----------- defined in video/kaneko16.c -----------*/

PALETTE_INIT( berlwall );

VIDEO_START( kaneko16 );
VIDEO_START( berlwall );

SCREEN_UPDATE_IND16( kaneko16 );
SCREEN_UPDATE_IND16( berlwall );


#endif
