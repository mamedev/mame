/***************************************************************************

    atarigen.h

    General functions for Atari raster games.

***************************************************************************/

#include "video/atarimo.h"
#include "video/atarirle.h"

#ifndef __MACHINE_ATARIGEN__
#define __MACHINE_ATARIGEN__


/***************************************************************************
    CONSTANTS
***************************************************************************/

#define ATARI_CLOCK_14MHz	XTAL_14_31818MHz
#define ATARI_CLOCK_20MHz	XTAL_20MHz
#define ATARI_CLOCK_32MHz	XTAL_32MHz
#define ATARI_CLOCK_50MHz	XTAL_50MHz



/***************************************************************************
    TYPES & STRUCTURES
***************************************************************************/

typedef void (*atarigen_int_func)(running_machine *machine);

typedef void (*atarigen_scanline_func)(screen_device &screen, int scanline);

typedef struct _atarivc_state_desc atarivc_state_desc;
struct _atarivc_state_desc
{
	UINT32 latch1;								/* latch #1 value (-1 means disabled) */
	UINT32 latch2;								/* latch #2 value (-1 means disabled) */
	UINT32 rowscroll_enable;					/* true if row-scrolling is enabled */
	UINT32 palette_bank;						/* which palette bank is enabled */
	UINT32 pf0_xscroll;						/* playfield 1 xscroll */
	UINT32 pf0_xscroll_raw;					/* playfield 1 xscroll raw value */
	UINT32 pf0_yscroll;						/* playfield 1 yscroll */
	UINT32 pf1_xscroll;						/* playfield 2 xscroll */
	UINT32 pf1_xscroll_raw;					/* playfield 2 xscroll raw value */
	UINT32 pf1_yscroll;						/* playfield 2 yscroll */
	UINT32 mo_xscroll;							/* sprite xscroll */
	UINT32 mo_yscroll;							/* sprite xscroll */
};


typedef struct _atarigen_screen_timer atarigen_screen_timer;
struct _atarigen_screen_timer
{
	screen_device *screen;
	emu_timer *			scanline_interrupt_timer;
	emu_timer *			scanline_timer;
	emu_timer *			atarivc_eof_update_timer;
};


class atarigen_state : public driver_device
{
public:
	atarigen_state(running_machine &machine, const driver_device_config_base &config)
		: driver_device(machine, config) { }

	UINT8				scanline_int_state;
	UINT8				sound_int_state;
	UINT8				video_int_state;

	const UINT16 *		eeprom_default;
	UINT16 *			eeprom;
	size_t				eeprom_size;

	UINT8				cpu_to_sound_ready;
	UINT8				sound_to_cpu_ready;

	UINT16 *			playfield;
	UINT16 *			playfield2;
	UINT16 *			playfield_upper;
	UINT16 *			alpha;
	UINT16 *			alpha2;
	UINT16 *			xscroll;
	UINT16 *			yscroll;

	UINT32 *			playfield32;
	UINT32 *			alpha32;

	tilemap_t *			playfield_tilemap;
	tilemap_t *			playfield2_tilemap;
	tilemap_t *			alpha_tilemap;
	tilemap_t *			alpha2_tilemap;

	UINT16 *			atarivc_data;
	UINT16 *			atarivc_eof_data;
	atarivc_state_desc	atarivc_state;

	/* internal state */
	atarigen_int_func		update_int_callback;

	UINT8					eeprom_unlocked;

	UINT8					slapstic_num;
	UINT16 *				slapstic;
	UINT8					slapstic_bank;
	void *					slapstic_bank0;
	offs_t					slapstic_last_pc;
	offs_t					slapstic_last_address;
	offs_t					slapstic_base;
	offs_t					slapstic_mirror;

	running_device *	sound_cpu;
	UINT8					cpu_to_sound;
	UINT8					sound_to_cpu;
	UINT8					timed_int;
	UINT8					ym2151_int;

	atarigen_scanline_func	scanline_callback;
	UINT32					scanlines_per_callback;

	UINT32					actual_vc_latch0;
	UINT32					actual_vc_latch1;
	UINT8					atarivc_playfields;

	UINT32					playfield_latch;
	UINT32					playfield2_latch;

	atarigen_screen_timer	screen_timer[2];
};



/***************************************************************************
    FUNCTION PROTOTYPES
***************************************************************************/

/*---------------------------------------------------------------
    OVERALL INIT
---------------------------------------------------------------*/

void atarigen_init(running_machine *machine);


/*---------------------------------------------------------------
    INTERRUPT HANDLING
---------------------------------------------------------------*/

void atarigen_interrupt_reset(atarigen_state *state, atarigen_int_func update_int);
void atarigen_update_interrupts(running_machine *machine);

void atarigen_scanline_int_set(screen_device &screen, int scanline);
INTERRUPT_GEN( atarigen_scanline_int_gen );
WRITE16_HANDLER( atarigen_scanline_int_ack_w );
WRITE32_HANDLER( atarigen_scanline_int_ack32_w );

INTERRUPT_GEN( atarigen_sound_int_gen );
WRITE16_HANDLER( atarigen_sound_int_ack_w );
WRITE32_HANDLER( atarigen_sound_int_ack32_w );

INTERRUPT_GEN( atarigen_video_int_gen );
WRITE16_HANDLER( atarigen_video_int_ack_w );
WRITE32_HANDLER( atarigen_video_int_ack32_w );


/*---------------------------------------------------------------
    EEPROM HANDLING
---------------------------------------------------------------*/

void atarigen_eeprom_reset(atarigen_state *state);

WRITE16_HANDLER( atarigen_eeprom_enable_w );
WRITE16_HANDLER( atarigen_eeprom_w );
READ16_HANDLER( atarigen_eeprom_r );
READ16_HANDLER( atarigen_eeprom_upper_r );

WRITE32_HANDLER( atarigen_eeprom_enable32_w );
WRITE32_HANDLER( atarigen_eeprom32_w );
READ32_HANDLER( atarigen_eeprom_upper32_r );

NVRAM_HANDLER( atarigen );


/*---------------------------------------------------------------
    SLAPSTIC HANDLING
---------------------------------------------------------------*/

void atarigen_slapstic_init(running_device *device, offs_t base, offs_t mirror, int chipnum);
void atarigen_slapstic_reset(atarigen_state *state);

WRITE16_HANDLER( atarigen_slapstic_w );
READ16_HANDLER( atarigen_slapstic_r );


/*---------------------------------------------------------------
    SOUND I/O
---------------------------------------------------------------*/

void atarigen_sound_io_reset(running_device *device);

INTERRUPT_GEN( atarigen_6502_irq_gen );
READ8_HANDLER( atarigen_6502_irq_ack_r );
WRITE8_HANDLER( atarigen_6502_irq_ack_w );

void atarigen_ym2151_irq_gen(running_device *device, int irq);

WRITE16_HANDLER( atarigen_sound_w );
READ16_HANDLER( atarigen_sound_r );
WRITE16_HANDLER( atarigen_sound_upper_w );
READ16_HANDLER( atarigen_sound_upper_r );

WRITE32_HANDLER( atarigen_sound_upper32_w );
READ32_HANDLER( atarigen_sound_upper32_r );

void atarigen_sound_reset(running_machine *machine);
WRITE16_HANDLER( atarigen_sound_reset_w );
WRITE8_HANDLER( atarigen_6502_sound_w );
READ8_HANDLER( atarigen_6502_sound_r );


/*---------------------------------------------------------------
    SOUND HELPERS
---------------------------------------------------------------*/

void atarigen_set_ym2151_vol(running_machine *machine, int volume);
void atarigen_set_ym2413_vol(running_machine *machine, int volume);
void atarigen_set_pokey_vol(running_machine *machine, int volume);
void atarigen_set_tms5220_vol(running_machine *machine, int volume);
void atarigen_set_oki6295_vol(running_machine *machine, int volume);


/*---------------------------------------------------------------
    VIDEO CONTROLLER
---------------------------------------------------------------*/

void atarivc_reset(screen_device &screen, UINT16 *eof_data, int playfields);

void atarivc_w(screen_device &screen, offs_t offset, UINT16 data, UINT16 mem_mask);
UINT16 atarivc_r(screen_device &screen, offs_t offset);

INLINE void atarivc_update_pf_xscrolls(atarigen_state *state)
{
	state->atarivc_state.pf0_xscroll = state->atarivc_state.pf0_xscroll_raw + ((state->atarivc_state.pf1_xscroll_raw) & 7);
	state->atarivc_state.pf1_xscroll = state->atarivc_state.pf1_xscroll_raw + 4;
}


/*---------------------------------------------------------------
    PLAYFIELD/ALPHA MAP HELPERS
---------------------------------------------------------------*/

WRITE16_HANDLER( atarigen_alpha_w );
WRITE32_HANDLER( atarigen_alpha32_w );
WRITE16_HANDLER( atarigen_alpha2_w );
void atarigen_set_playfield_latch(atarigen_state *state, int data);
void atarigen_set_playfield2_latch(atarigen_state *state, int data);
WRITE16_HANDLER( atarigen_playfield_w );
WRITE32_HANDLER( atarigen_playfield32_w );
WRITE16_HANDLER( atarigen_playfield_large_w );
WRITE16_HANDLER( atarigen_playfield_upper_w );
WRITE16_HANDLER( atarigen_playfield_dual_upper_w );
WRITE16_HANDLER( atarigen_playfield_latched_lsb_w );
WRITE16_HANDLER( atarigen_playfield_latched_msb_w );
WRITE16_HANDLER( atarigen_playfield2_w );
WRITE16_HANDLER( atarigen_playfield2_latched_msb_w );


/*---------------------------------------------------------------
    VIDEO HELPERS
---------------------------------------------------------------*/

void atarigen_scanline_timer_reset(screen_device &screen, atarigen_scanline_func update_graphics, int frequency);
int atarigen_get_hblank(screen_device &screen);
void atarigen_halt_until_hblank_0(screen_device &screen);
WRITE16_HANDLER( atarigen_666_paletteram_w );
WRITE16_HANDLER( atarigen_expanded_666_paletteram_w );
WRITE32_HANDLER( atarigen_666_paletteram32_w );


/*---------------------------------------------------------------
    MISC HELPERS
---------------------------------------------------------------*/

void atarigen_swap_mem(void *ptr1, void *ptr2, int bytes);
void atarigen_blend_gfx(running_machine *machine, int gfx0, int gfx1, int mask0, int mask1);



/***************************************************************************
    GENERAL ATARI NOTES
**************************************************************************##

    Atari 68000 list:

    Driver      Pr? Up? VC? PF? P2? MO? AL? BM? PH?
    ----------  --- --- --- --- --- --- --- --- ---
    arcadecl.c       *               *       *
    atarig1.c        *       *      rle  *
    atarig42.c       *       *      rle  *
    atarigt.c                *      rle  *
    atarigx2.c               *      rle  *
    atarisy1.c   *   *       *       *   *              270->260
    atarisy2.c   *   *       *       *   *              150->120
    badlands.c       *       *       *                  250->260
    batman.c     *   *   *   *   *   *   *       *      200->160 ?
    blstroid.c       *       *       *                  240->230
    cyberbal.c       *       *       *   *              125->105 ?
    eprom.c          *       *       *   *              170->170
    gauntlet.c   *   *       *       *   *       *      220->250
    klax.c       *   *       *       *                  480->440 ?
    offtwall.c       *   *   *       *                  260->260
    rampart.c        *               *       *          280->280
    relief.c     *   *   *   *   *   *                  240->240
    shuuz.c          *   *   *       *                  410->290 fix!
    skullxbo.c       *       *       *   *              150->145
    thunderj.c       *   *   *   *   *   *       *      180->180
    toobin.c         *       *       *   *              140->115 fix!
    vindictr.c   *   *       *       *   *       *      200->210
    xybots.c     *   *       *       *   *              235->238
    ----------  --- --- --- --- --- --- --- --- ---

    Pr? - do we have verifiable proof on priorities?
    Up? - have we updated to use new MO's & tilemaps?
    VC? - does it use the video controller?
    PF? - does it have a playfield?
    P2? - does it have a dual playfield?
    MO? - does it have MO's?
    AL? - does it have an alpha layer?
    BM? - does it have a bitmap layer?
    PH? - does it use the palette hack?

***************************************************************************/


#endif
