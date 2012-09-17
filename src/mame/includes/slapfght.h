#include "cpu/z80/z80.h"
#include "video/bufsprite.h"


/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	GETSTUNK=0,  /* unknown for inclusion of possible new sets */
	GETSTAR,
	GETSTARJ,
	GTSTARB1,    /* "good" bootleg with same behaviour as 'getstarj' */
	GTSTARB2,    /* "lame" bootleg with lots of ingame bugs */
};


class slapfght_state : public driver_device
{
public:
	slapfght_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_slapfight_videoram(*this, "videoram"),
		  m_slapfight_colorram(*this, "colorram"),
		  m_slapfight_fixvideoram(*this, "fixvideoram"),
		  m_slapfight_fixcolorram(*this, "fixcolorram"),
		  m_slapfight_scrollx_lo(*this, "scrollx_lo"),
		  m_slapfight_scrollx_hi(*this, "scrollx_hi"),
		  m_slapfight_scrolly(*this, "scrolly"),
		  m_spriteram(*this, "spriteram") { }

	int m_getstar_id;
	required_shared_ptr<UINT8> m_slapfight_videoram;
	required_shared_ptr<UINT8> m_slapfight_colorram;
	optional_shared_ptr<UINT8> m_slapfight_fixvideoram;
	optional_shared_ptr<UINT8> m_slapfight_fixcolorram;
	optional_shared_ptr<UINT8> m_slapfight_scrollx_lo;
	optional_shared_ptr<UINT8> m_slapfight_scrollx_hi;
	optional_shared_ptr<UINT8> m_slapfight_scrolly;
	int m_slapfight_status;
	int m_getstar_sequence_index;
	int m_getstar_sh_intenabled;
	int m_slapfight_status_state;
	UINT8 m_mcu_val;
	UINT8 m_getstar_cmd;
	UINT8 m_gs_a;
	UINT8 m_gs_d;
	UINT8 m_gs_e;
	UINT8 m_tigerhb_cmd;
	UINT8 m_from_main;
	UINT8 m_from_mcu;
	int m_mcu_sent;
	int m_main_sent;
	UINT8 m_portA_in;
	UINT8 m_portA_out;
	UINT8 m_ddrA;
	UINT8 m_portB_in;
	UINT8 m_portB_out;
	UINT8 m_ddrB;
	UINT8 m_portC_in;
	UINT8 m_portC_out;
	UINT8 m_ddrC;
	int m_flipscreen;
	int m_slapfight_palette_bank;
	tilemap_t *m_pf1_tilemap;
	tilemap_t *m_fix_tilemap;
	UINT8 m_irq_mask;
	required_device<buffered_spriteram8_device> m_spriteram;
	DECLARE_READ8_MEMBER(tigerh_status_r);
	DECLARE_READ8_MEMBER(gtstarb1_port_0_read);
	DECLARE_WRITE8_MEMBER(slapfight_port_00_w);
	DECLARE_WRITE8_MEMBER(slapfight_port_01_w);
	DECLARE_WRITE8_MEMBER(slapfight_port_06_w);
	DECLARE_WRITE8_MEMBER(slapfight_port_07_w);
	DECLARE_WRITE8_MEMBER(slapfight_port_08_w);
	DECLARE_WRITE8_MEMBER(slapfight_port_09_w);
	DECLARE_READ8_MEMBER(slapfight_port_00_r);
	DECLARE_READ8_MEMBER(slapfight_68705_portA_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portA_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrA_w);
	DECLARE_READ8_MEMBER(slapfight_68705_portB_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portB_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrB_w);
	DECLARE_READ8_MEMBER(slapfight_68705_portC_r);
	DECLARE_WRITE8_MEMBER(slapfight_68705_portC_w);
	DECLARE_WRITE8_MEMBER(slapfight_68705_ddrC_w);
	DECLARE_WRITE8_MEMBER(slapfight_mcu_w);
	DECLARE_READ8_MEMBER(slapfight_mcu_r);
	DECLARE_READ8_MEMBER(slapfight_mcu_status_r);
	DECLARE_READ8_MEMBER(getstar_e803_r);
	DECLARE_WRITE8_MEMBER(getstar_e803_w);
	DECLARE_WRITE8_MEMBER(getstar_sh_intenable_w);
	DECLARE_WRITE8_MEMBER(getstar_port_04_w);
	DECLARE_READ8_MEMBER(tigerh_68705_portA_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portA_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrA_w);
	DECLARE_READ8_MEMBER(tigerh_68705_portB_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portB_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrB_w);
	DECLARE_READ8_MEMBER(tigerh_68705_portC_r);
	DECLARE_WRITE8_MEMBER(tigerh_68705_portC_w);
	DECLARE_WRITE8_MEMBER(tigerh_68705_ddrC_w);
	DECLARE_WRITE8_MEMBER(tigerh_mcu_w);
	DECLARE_READ8_MEMBER(tigerh_mcu_r);
	DECLARE_READ8_MEMBER(tigerh_mcu_status_r);
	DECLARE_READ8_MEMBER(tigerhb_e803_r);
	DECLARE_WRITE8_MEMBER(tigerhb_e803_w);
	DECLARE_READ8_MEMBER(perfrman_port_00_r);
	DECLARE_WRITE8_MEMBER(slapfight_videoram_w);
	DECLARE_WRITE8_MEMBER(slapfight_colorram_w);
	DECLARE_WRITE8_MEMBER(slapfight_fixram_w);
	DECLARE_WRITE8_MEMBER(slapfight_fixcol_w);
	DECLARE_WRITE8_MEMBER(slapfight_flipscreen_w);
	DECLARE_WRITE8_MEMBER(slapfight_palette_bank_w);
	DECLARE_DRIVER_INIT(getstarj);
	DECLARE_DRIVER_INIT(getstar);
	DECLARE_DRIVER_INIT(gtstarb1);
	DECLARE_DRIVER_INIT(tigerhb);
	DECLARE_DRIVER_INIT(slapfigh);
	DECLARE_DRIVER_INIT(perfrman);
	DECLARE_DRIVER_INIT(gtstarb2);
	DECLARE_DRIVER_INIT(tigerh);
	TILE_GET_INFO_MEMBER(get_pf_tile_info);
	TILE_GET_INFO_MEMBER(get_pf1_tile_info);
	TILE_GET_INFO_MEMBER(get_fix_tile_info);
	DECLARE_MACHINE_RESET(slapfight);
	DECLARE_VIDEO_START(perfrman);
	DECLARE_VIDEO_START(slapfight);
	UINT32 screen_update_perfrman(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_slapfight(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};


/*----------- defines -----------*/

/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ),
   register C is a unaltered copy of register A */

#define GS_SAVE_REGS  m_gs_a = space.device().state().state_int(Z80_BC) >> 0; \
                      m_gs_d = space.device().state().state_int(Z80_DE) >> 8; \
                      m_gs_e = space.device().state().state_int(Z80_DE) >> 0;

#define GS_RESET_REGS m_gs_a = 0; \
                      m_gs_d = 0; \
                      m_gs_e = 0;


/*----------- defined in machine/slapfght.c -----------*/










INTERRUPT_GEN( getstar_interrupt );


/*----------- defined in video/slapfght.c -----------*/






