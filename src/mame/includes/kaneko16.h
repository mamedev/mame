/***************************************************************************

                            -= Kaneko 16 Bit Games =-

***************************************************************************/

#ifndef __KANEKO16_H__
#define __KANEKO16_H__

#include "machine/nvram.h"
#include "video/kaneko_tmap.h"
#include "video/kaneko_spr.h"
#include "machine/kaneko_calc3.h"
#include "machine/kaneko_toybox.h"




class kaneko16_state : public driver_device
{
public:
	kaneko16_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_spriteram(*this, "spriteram"),
		m_mainram(*this, "mainram"),
		m_view2_0(*this, "view2_0"),
		m_view2_1(*this, "view2_1"),
		m_kaneko_spr(*this, "kan_spr")
	{ }

	required_device<cpu_device> m_maincpu;
	optional_shared_ptr<UINT16> m_spriteram;
	optional_shared_ptr<UINT16> m_mainram;
	optional_device<kaneko_view2_tilemap_device> m_view2_0;
	optional_device<kaneko_view2_tilemap_device> m_view2_1;
	optional_device<kaneko16_sprite_device> m_kaneko_spr;

	UINT16 m_disp_enable;

	int VIEW2_2_pri;


	DECLARE_WRITE16_MEMBER(kaneko16_coin_lockout_w);
	DECLARE_WRITE16_MEMBER(kaneko16_soundlatch_w);
	DECLARE_WRITE16_MEMBER(kaneko16_eeprom_w);


	DECLARE_WRITE16_MEMBER(kaneko16_display_enable);

	DECLARE_READ16_MEMBER(kaneko16_ay1_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay1_YM2149_w);
	DECLARE_READ16_MEMBER(kaneko16_ay2_YM2149_r);
	DECLARE_WRITE16_MEMBER(kaneko16_ay2_YM2149_w);
	DECLARE_WRITE16_MEMBER(bakubrkr_oki_bank_sw);

	DECLARE_WRITE8_MEMBER(kaneko16_eeprom_reset_w);

	DECLARE_DRIVER_INIT(kaneko16);
	DECLARE_DRIVER_INIT(samplebank);


	DECLARE_MACHINE_RESET(gtmr);
	DECLARE_VIDEO_START(kaneko16);
	DECLARE_MACHINE_RESET(mgcrystl);
};

class kaneko16_gtmr_state : public kaneko16_state
{
public:
	kaneko16_gtmr_state(const machine_config &mconfig, device_type type, const char *tag)
		: kaneko16_state(mconfig, type, tag)
	{
	}

	DECLARE_WRITE16_MEMBER(bloodwar_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bloodwar_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(bonkadv_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_0_bank_w);
	DECLARE_WRITE16_MEMBER(gtmr_oki_1_bank_w);
	DECLARE_WRITE16_MEMBER(bloodwar_coin_lockout_w);
	DECLARE_READ16_MEMBER(gtmr_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_wheel_r);
	DECLARE_READ16_MEMBER(gtmr2_IN1_r);
	DECLARE_DRIVER_INIT(gtmr);

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

	DECLARE_DRIVER_INIT(berlwall);
	DECLARE_PALETTE_INIT(berlwall);
	DECLARE_VIDEO_START(berlwall);
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

	DECLARE_DRIVER_INIT(shogwarr);
	DECLARE_DRIVER_INIT(brapboys);
};

/*----------- defined in drivers/kaneko16.c -----------*/



/*----------- defined in video/kaneko16.c -----------*/






SCREEN_UPDATE_IND16( kaneko16 );
SCREEN_UPDATE_IND16( berlwall );


#endif
