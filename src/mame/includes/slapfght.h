#include "cpu/z80/z80.h"


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
		: driver_device(mconfig, type, tag) { }

	int m_getstar_id;
	UINT8 *m_slapfight_videoram;
	UINT8 *m_slapfight_colorram;
	UINT8 *m_slapfight_fixvideoram;
	UINT8 *m_slapfight_fixcolorram;
	UINT8 *m_slapfight_scrollx_lo;
	UINT8 *m_slapfight_scrollx_hi;
	UINT8 *m_slapfight_scrolly;
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
};


/*----------- defines -----------*/

/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ),
   register C is a unaltered copy of register A */

#define GS_SAVE_REGS  state->m_gs_a = cpu_get_reg(&space->device(), Z80_BC) >> 0; \
                       state->m_gs_d = cpu_get_reg(&space->device(), Z80_DE) >> 8; \
                       state->m_gs_e = cpu_get_reg(&space->device(), Z80_DE) >> 0;

#define GS_RESET_REGS state->m_gs_a = 0; \
                       state->m_gs_d = 0; \
                       state->m_gs_e = 0;


/*----------- defined in machine/slapfght.c -----------*/

MACHINE_RESET( slapfight );

READ8_HANDLER( slapfight_port_00_r );
WRITE8_HANDLER( slapfight_port_00_w );
WRITE8_HANDLER( slapfight_port_01_w );
WRITE8_HANDLER( slapfight_port_06_w );
WRITE8_HANDLER( slapfight_port_07_w );
WRITE8_HANDLER( slapfight_port_08_w );
WRITE8_HANDLER( slapfight_port_09_w );

READ8_HANDLER ( slapfight_68705_portA_r );
WRITE8_HANDLER( slapfight_68705_portA_w );
READ8_HANDLER ( slapfight_68705_portB_r );
WRITE8_HANDLER( slapfight_68705_portB_w );
READ8_HANDLER ( slapfight_68705_portC_r );
WRITE8_HANDLER( slapfight_68705_portC_w );
WRITE8_HANDLER( slapfight_68705_ddrA_w );
WRITE8_HANDLER( slapfight_68705_ddrB_w );
WRITE8_HANDLER( slapfight_68705_ddrC_w );
WRITE8_HANDLER( slapfight_mcu_w );
READ8_HANDLER ( slapfight_mcu_r );
READ8_HANDLER ( slapfight_mcu_status_r );

READ8_HANDLER( getstar_e803_r );
WRITE8_HANDLER( getstar_e803_w );

READ8_HANDLER( perfrman_port_00_r );

READ8_HANDLER ( tigerh_68705_portA_r );
WRITE8_HANDLER( tigerh_68705_portA_w );
READ8_HANDLER ( tigerh_68705_portB_r );
WRITE8_HANDLER( tigerh_68705_portB_w );
READ8_HANDLER ( tigerh_68705_portC_r );
WRITE8_HANDLER( tigerh_68705_portC_w );
WRITE8_HANDLER( tigerh_68705_ddrA_w );
WRITE8_HANDLER( tigerh_68705_ddrB_w );
WRITE8_HANDLER( tigerh_68705_ddrC_w );
WRITE8_HANDLER( tigerh_mcu_w );
READ8_HANDLER ( tigerh_mcu_r );
READ8_HANDLER ( tigerh_mcu_status_r );

READ8_HANDLER( tigerhb_e803_r );
WRITE8_HANDLER( tigerhb_e803_w );


WRITE8_HANDLER( getstar_sh_intenable_w );
INTERRUPT_GEN( getstar_interrupt );


/*----------- defined in video/slapfght.c -----------*/

SCREEN_UPDATE( slapfight );
SCREEN_UPDATE( perfrman );
VIDEO_START( slapfight );
VIDEO_START( perfrman );

WRITE8_HANDLER( slapfight_flipscreen_w );
WRITE8_HANDLER( slapfight_fixram_w );
WRITE8_HANDLER( slapfight_fixcol_w );
WRITE8_HANDLER( slapfight_videoram_w );
WRITE8_HANDLER( slapfight_colorram_w );
WRITE8_HANDLER( slapfight_palette_bank_w );
