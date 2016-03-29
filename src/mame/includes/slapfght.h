// license:BSD-3-Clause
// copyright-holders:K.Wilkins,Stephane Humbert
/***************************************************************************

  Toaplan Slap Fight hardware

***************************************************************************/

#include "cpu/z80/z80.h"
#include "video/bufsprite.h"


class slapfght_state : public driver_device
{
public:
	slapfght_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_audiocpu(*this, "audiocpu"),
		m_mcu(*this, "mcu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_screen(*this, "screen"),
		m_palette(*this, "palette"),
		m_spriteram(*this, "spriteram"),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_fixvideoram(*this, "fixvideoram"),
		m_fixcolorram(*this, "fixcolorram")
	{ }

	// devices, memory pointers
	required_device<cpu_device> m_maincpu;
	required_device<cpu_device> m_audiocpu;
	optional_device<cpu_device> m_mcu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<screen_device> m_screen;
	required_device<palette_device> m_palette;
	required_device<buffered_spriteram8_device> m_spriteram;

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	optional_shared_ptr<UINT8> m_fixvideoram;
	optional_shared_ptr<UINT8> m_fixcolorram;

	/* This it the best way to allow game specific kludges until the system is fully understood */
	enum getstar_id
	{
		GETSTUNK = 0, /* unknown for inclusion of possible new sets */
		GETSTAR,
		GETSTARJ,
		GETSTARB1,    /* "good" bootleg with same behaviour as 'getstarj' */
		GETSTARB2     /* "lame" bootleg with lots of ingame bugs */
	} m_getstar_id;

	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_fix_tilemap;
	UINT8 m_palette_bank;
	UINT8 m_scrollx_lo;
	UINT8 m_scrollx_hi;
	UINT8 m_scrolly;
	bool m_main_irq_enabled;
	bool m_sound_nmi_enabled;

	bool m_mcu_sent;
	bool m_main_sent;
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_ddrA;
	UINT8 m_portB_in;
	UINT8 m_portB_out;
	UINT8 m_ddrB;
	UINT8 m_portC_in;
	UINT8 m_portC_out;
	UINT8 m_ddrC;

	int m_getstar_status;
	int m_getstar_sequence_index;
	int m_getstar_status_state;
	UINT8 m_getstar_cmd;
	UINT8 m_gs_a;
	UINT8 m_gs_d;
	UINT8 m_gs_e;
	UINT8 m_tigerhb_cmd;

	DECLARE_WRITE8_MEMBER(sound_reset_w);
	DECLARE_WRITE8_MEMBER(irq_enable_w);
	DECLARE_WRITE8_MEMBER(prg_bank_w);
	DECLARE_READ8_MEMBER(vblank_r);
	DECLARE_WRITE8_MEMBER(sound_nmi_enable_w);
	DECLARE_WRITE8_MEMBER(videoram_w);
	DECLARE_WRITE8_MEMBER(colorram_w);
	DECLARE_WRITE8_MEMBER(fixram_w);
	DECLARE_WRITE8_MEMBER(fixcol_w);
	DECLARE_WRITE8_MEMBER(scrollx_lo_w);
	DECLARE_WRITE8_MEMBER(scrollx_hi_w);
	DECLARE_WRITE8_MEMBER(scrolly_w);
	DECLARE_WRITE8_MEMBER(flipscreen_w);
	DECLARE_WRITE8_MEMBER(palette_bank_w);

	DECLARE_WRITE8_MEMBER(tigerh_mcu_w);
	DECLARE_READ8_MEMBER(tigerh_mcu_r);
	DECLARE_READ8_MEMBER(tigerh_mcu_status_r);
	DECLARE_READ8_MEMBER(tigerh_68705_portA_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portA_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrA_w);
	DECLARE_READ8_MEMBER(tigerh_68705_portB_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portB_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrB_w);
	DECLARE_READ8_MEMBER(tigerh_68705_portC_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portC_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrC_w);

	DECLARE_READ8_MEMBER(slapfight_68705_portA_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portA_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrA_w);
	DECLARE_READ8_MEMBER(slapfight_68705_portB_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portB_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrB_w);
	DECLARE_READ8_MEMBER(slapfight_68705_portC_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portC_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrC_w);

	DECLARE_READ8_MEMBER(getstar_mcusim_r);
	DECLARE_WRITE8_MEMBER(getstar_mcusim_w);
	DECLARE_READ8_MEMBER(getstar_mcusim_status_r);
	DECLARE_READ8_MEMBER(getstarb1_prot_r);
	DECLARE_READ8_MEMBER(tigerhb1_prot_r);
	DECLARE_WRITE8_MEMBER(tigerhb1_prot_w);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	DECLARE_MACHINE_RESET(getstar);

	void init_banks();
	DECLARE_DRIVER_INIT(getstarj);
	DECLARE_DRIVER_INIT(getstar);
	DECLARE_DRIVER_INIT(getstarb1);
	DECLARE_DRIVER_INIT(slapfigh);
	DECLARE_DRIVER_INIT(getstarb2);

	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	DECLARE_VIDEO_START(perfrman);
	DECLARE_VIDEO_START(slapfight);

	void draw_perfrman_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect, int layer);
	void draw_slapfight_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	INTERRUPT_GEN_MEMBER(vblank_irq);
	INTERRUPT_GEN_MEMBER(sound_nmi);
};
