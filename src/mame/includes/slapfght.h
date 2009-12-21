#include "cpu/z80/z80.h"


/* This it the best way to allow game specific kludges until the system is fully understood */
enum {
	GETSTUNK=0,  /* unknown for inclusion of possible new sets */
	GETSTAR,
	GETSTARJ,
	GTSTARB1,    /* "good" bootleg with same behaviour as 'getstarj' */
	GTSTARB2,    /* "lame" bootleg with lots of ingame bugs */
};


/*----------- defines -----------*/

/* due to code at 0x108d (GUARDIAN) or 0x1152 (GETSTARJ),
   register C is a unaltered copy of register A */

# define GS_SAVE_REGS  gs_a = cpu_get_reg(space->cpu, Z80_BC) >> 0; \
                       gs_d = cpu_get_reg(space->cpu, Z80_DE) >> 8; \
                       gs_e = cpu_get_reg(space->cpu, Z80_DE) >> 0;

# define GS_RESET_REGS gs_a = 0; \
                       gs_d = 0; \
                       gs_e = 0;


/*----------- defined in drivers/slapfght.c -----------*/

extern int getstar_id;


/*----------- defined in machine/slapfght.c -----------*/

MACHINE_RESET( slapfight );

READ8_HANDLER( slapfight_port_00_r );
WRITE8_HANDLER( slapfight_port_00_w );
WRITE8_HANDLER( slapfight_port_01_w );
WRITE8_HANDLER( slapfight_port_06_w );
WRITE8_HANDLER( slapfight_port_07_w );
WRITE8_HANDLER( slapfight_port_08_w );
WRITE8_HANDLER( slapfight_port_09_w );


READ8_HANDLER( getstar_e803_r );
WRITE8_HANDLER( getstar_e803_w );


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

extern UINT8 *slapfight_videoram;
extern UINT8 *slapfight_colorram;
extern UINT8 *slapfight_fixvideoram;
extern UINT8 *slapfight_fixcolorram;
extern UINT8 *slapfight_scrollx_lo,*slapfight_scrollx_hi,*slapfight_scrolly;

VIDEO_UPDATE( slapfight );
VIDEO_UPDATE( perfrman );
VIDEO_START( slapfight );
VIDEO_START( perfrman );

WRITE8_HANDLER( slapfight_flipscreen_w );
WRITE8_HANDLER( slapfight_fixram_w );
WRITE8_HANDLER( slapfight_fixcol_w );
WRITE8_HANDLER( slapfight_videoram_w );
WRITE8_HANDLER( slapfight_colorram_w );
WRITE8_HANDLER( slapfight_palette_bank_w );
