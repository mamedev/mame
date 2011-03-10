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
	slapfght_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	int getstar_id;
	UINT8 *slapfight_videoram;
	UINT8 *slapfight_colorram;
	UINT8 *slapfight_fixvideoram;
	UINT8 *slapfight_fixcolorram;
	UINT8 *slapfight_scrollx_lo;
	UINT8 *slapfight_scrollx_hi;
	UINT8 *slapfight_scrolly;
	int slapfight_status;
	int getstar_sequence_index;
	int getstar_sh_intenabled;
	int slapfight_status_state;
	UINT8 mcu_val;
	UINT8 getstar_cmd;
	UINT8 gs_a;
	UINT8 gs_d;
	UINT8 gs_e;
	UINT8 tigerhb_cmd;
	UINT8 from_main;
	UINT8 from_mcu;
	int mcu_sent;
	int main_sent;
	UINT8 portA_in;
	UINT8 portA_out;
	UINT8 ddrA;
	UINT8 portB_in;
	UINT8 portB_out;
	UINT8 ddrB;
	UINT8 portC_in;
	UINT8 portC_out;
	UINT8 ddrC;
	int flipscreen;
	int slapfight_palette_bank;
	tilemap_t *pf1_tilemap;
	tilemap_t *fix_tilemap;
};


/*----------- defines -----------*/

/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ),
   register C is a unaltered copy of register A */

#define GS_SAVE_REGS  state->gs_a = cpu_get_reg(space->cpu, Z80_BC) >> 0; \
                       state->gs_d = cpu_get_reg(space->cpu, Z80_DE) >> 8; \
                       state->gs_e = cpu_get_reg(space->cpu, Z80_DE) >> 0;

#define GS_RESET_REGS state->gs_a = 0; \
                       state->gs_d = 0; \
                       state->gs_e = 0;


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
