/**********************************************************************

Legion
(c)1986 Nichibutsu

Terra Force
(c)1987 Nichibutsu

Kodure Ookami
(c)1987 Nichibutsu

Crazy Climber 2
(c)1988 Nichibutsu

Armed Formation
(c)1988 Nichibutsu

Based on the notes below I suspect several of the supported sets are
actually bootlegs.

68000 + Z80

TODO:
- simulate the mcu/blitter (particularly needed in terrafu and legion)
   -- or figure out which chip it is, decap it, and emulate it.




Stephh's notes (based on the games M68000 code and some tests) :

1a) 'legion'

  - The ROM test (code at 0x00046e) checks range 0x000102-0x01ffff (!)
    and reports an error if the checksum isn't correct.

  - 3 Dip Switches which are told to be unused have an effect if
    [0x062d53] != 0x00 (check code at 0x00d9b2).
    As this value is ALWAYS set to 0x00 via code at 0x0001d4, I've
    added a #define LEGION_HACK you need to change to 1 if you want
    get the benefits of these Dip Switches.
    Here is what these "unused" Dip Switch do :

      * 1st unused Dip Switch (DSW0 bit 6) remains unused
      * 2nd unused Dip Switch (DSW0 bit 7) determines if the "Invulnerability"
        Dip Switches will be tested
      * 3rd unused Dip Switch (DSW1 bit 6) gives invulnerability or not
        to player 1 (DSW0 bit 7 must be ON too !)
      * 4th unused Dip Switch (DSW1 bit 7) gives invulnerability or not
        to player 2 (DSW0 bit 7 must be ON too !)

    Also note bit 1 of 0x062d53 is also tested but I haven't been able
    to find its purpose (any more infos are welcome)


1b) 'legiono'

  - The ROM test (code at 0x000466) checks range 0x000102-0x03ffff
    but NEVER reports an error if the checksum isn't correct due
    to the instruction at 0x000480 (see where it branches) :

    000466: 7000                     moveq   #$0, D0
    000468: 41FA FC98                lea     (-$368,PC), A0; ($102)
    00046C: D058                     add.w   (A0)+, D0
    00046E: B1FC 0004 0000           cmpa.l  #$40000, A0
    000474: 66F6                     bne     46c
    000476: 33C0 0006 2CAE           move.w  D0, $62cae.l
    00047C: B078 0100                cmp.w   $100.w, D0
    000480: 6600 0002                bne     484
    000484: 41FA FF86                lea     (-$7a,PC), A0; ($40c)

  - 3 Dip Switches which are told to be unused have an effect if
    [0x062d53] != 0x00 (check code at 0x00d7ea).
    As this value is ALWAYS set to 0x00 via code at 0x0001d4, I've
    added a #define LEGION_HACK you need to change to 1 if you want
    get the benefits of these Dip Switches.
    Here is what these "unused" Dip Switch do :

      * 1st unused Dip Switch (DSW0 bit 6) remains unused
      * 2nd unused Dip Switch (DSW0 bit 7) determines if the "Invulnerability"
        Dip Switches will be tested
      * 3rd unused Dip Switch (DSW1 bit 6) gives invulnerability or not
        to player 1 (DSW0 bit 7 must be ON too !)
      * 4th unused Dip Switch (DSW1 bit 7) gives invulnerability or not
        to player 2 (DSW0 bit 7 must be ON too !)

    Also note bit 1 of 0x062d53 is also tested but I haven't been able
    to find its purpose (any more infos are welcome)


2a) 'terraf'

  - The ROM test (code at 0x000292) ALWAYS displays "OK", but memory is
    in fact NEVER scanned ! Original behaviour or is the game a bootleg ?

    000292: 45F8 0000                lea     $0.w, A2
    000296: 303C 7FFF                move.w  #$7fff, D0
    00029A: 7200                     moveq   #$0, D1
    00029C: D29A                     add.l   (A2)+, D1
    00029E: 0C81 0000 0000           cmpi.l  #$0, D1
    0002A4: 4E71                     nop
    0002A6: 23FC 004F 004B 0006 83AA move.l  #$4f004b, $683aa.l
    0002B0: 4EF9 0000 0124           jmp     $124.l
    ...
    0002C2: 4EF9 0000 0124           jmp     $124.l


2b) 'terrafu'

  - The ROM test (code at 0x000292) NEVER displays "OK", but memory is
    in fact NEVER scanned ! Original behaviour or is the game a bootleg ?

    000292: 45F8 0000                lea     $0.w, A2
    000296: 303C 7FFF                move.w  #$7fff, D0
    00029A: 7200                     moveq   #$0, D1
    00029C: D29A                     add.l   (A2)+, D1
    00029E: 0C81 0000 0000           cmpi.l  #$0, D1
    0002A4: 661C                     bne     2c2
    0002A6: 23FC 004F 004B 0006 83AA move.l  #$4f004b, $683aa.l
    0002B0: 4EF9 0000 0124           jmp     $124.l
    ...
    0002C2: 4EF9 0000 0124           jmp     $124.l


3)  'kodure'

  - The ROM test (code at 0x004fac) checks range 0x000000-0x05ffff
    and reports an error if the checksum isn't correct.


4)  'cclimbr2'

  - The ROM test (code at 0x012f6e) checks ranges 0x000100-0x014fff,
    and 0x020000-0x024fff, and reports an error if the checksum isn't
    correct.


5)  'armedf'

  - The ROM test (code at 0x00df5e) checks ranges 0x000100-0x014fff,
    0x020000-0x024fff and 0x040000-0x04ffff, and reports an error if
    the checksum isn't correct.

***********************************************************************/

/*

    2003-06-01  Added cocktail support to all games

    2005-04-02  Sebastien Chevalier : various update to video on terrafu, plus some typos here and there


    DIP locations verified for:
    -cclimbr2
    -legion
    -terraf

*/

#include "emu.h"
#include "deprecat.h"
#include "cpu/m68000/m68000.h"
#include "cpu/z80/z80.h"
#include "sound/dac.h"
#include "sound/3812intf.h"
#include "includes/armedf.h"

#define LEGION_HACK	0


/*************************************
 *
 *  Memory handlers
 *
 *************************************/

static WRITE16_HANDLER( io_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();

	COMBINE_DATA(&state->vreg);
	/* bits 0 and 1 of armedf_vreg are coin counters */
	/* bit 12 seems to handle screen flipping */
	flip_screen_set(space->machine, state->vreg & 0x1000);
}

static WRITE16_HANDLER( terraf_io_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();

	COMBINE_DATA(&state->vreg);
	/* bits 0 and 1 of armedf_vreg are coin counters */
	/* bit 12 seems to handle screen flipping */
	flip_screen_set(space->machine, state->vreg & 0x1000);

	if ((state->vreg & 0x4000) && !(state->vreg & 0x0100))
	{
		int i;
		for (i = 0x10; i < 0x1000; i++)
		{
			state->text_videoram[i] = 0x20;
		}
		tilemap_mark_all_tiles_dirty(state->tx_tilemap);
		//logerror("vreg WIPE TX\n");
	}
	//logerror("VReg = %04x\n", state->vreg);

}

static WRITE16_HANDLER( kodure_io_w )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();

	COMBINE_DATA(&state->vreg);
	/* bits 0 and 1 of armedf_vreg are coin counters */
	/* bit 12 seems to handle screen flipping */
	flip_screen_set(space->machine, state->vreg & 0x1000);

	/* This is a temporary condition specification. */
	if (!(state->vreg & 0x0080))
	{
		int i;
		for (i = 0; i < 0x1000; i++)
		{
			armedf_text_videoram_w(space,i, ' ', 0xffff);
		}
	}
}

static WRITE16_HANDLER( sound_command_w )
{
	if (ACCESSING_BITS_0_7)
		soundlatch_w(space, 0, ((data & 0x7f) << 1) | 1);
}

#ifdef UNUSED_FUNCTION
static WRITE16_HANDLER( legion_command_c )
{
	armedf_state *state = space->machine->driver_data<armedf_state>();

	COMBINE_DATA(&state->legion_cmd[offset]);
	//logerror("Legion CMD %04x=%04x", offset, data);
}
#endif



/*************************************
 *
 *  Address maps
 *
 *************************************/

static ADDRESS_MAP_START( terraf_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x04ffff) AM_ROM
	AM_RANGE(0x060000, 0x0603ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x060400, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(terraf_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c006, 0x07c007) AM_WRITE(terraf_fg_scrollx_w)			/* not use in terrafu, 0x07c008 neither */
	AM_RANGE(0x07c008, 0x07c009) AM_WRITE(terraf_fg_scrolly_w)			/* written twice, lsb and msb */
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP					/* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(armedf_mcu_cmd)				/* MCU Command ? */
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(terraf_fg_scroll_msb_arm_w)	/* written between two consecutive writes to 7c008 */
ADDRESS_MAP_END

/* the same for now */
static ADDRESS_MAP_START( terrafb_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x04ffff) AM_ROM
	AM_RANGE(0x060000, 0x0603ff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x060400, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(terraf_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c006, 0x07c007) AM_WRITE(terraf_fg_scrollx_w)			/* not use in terrafu, 0x07c008 neither */
	AM_RANGE(0x07c008, 0x07c009) AM_WRITE(terraf_fg_scrolly_w)			/* written twice, lsb and msb */
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP					/* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(armedf_mcu_cmd)				/* MCU Command ? */
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITE(terraf_fg_scroll_msb_arm_w)	/* written between two consecutive writes to 7c008 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( kodure_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(kodure_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x0c0000, 0x0c0001) AM_WRITENOP /* watchdog? */
	AM_RANGE(0xffd000, 0xffd001) AM_WRITENOP /* ? */
ADDRESS_MAP_END

static ADDRESS_MAP_START( cclimbr2_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x06ca00, 0x06cbff) AM_WRITEONLY
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITENOP /* ? */
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( legion_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x06ca00, 0x06cbff) AM_WRITEONLY
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(terraf_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(armedf_mcu_cmd)	/* MCU Command ? */
	AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP		/* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( legiono_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x03ffff) AM_ROM
	AM_RANGE(0x040000, 0x04003f) AM_WRITEONLY AM_BASE_MEMBER(armedf_state, legion_cmd)
	AM_RANGE(0x040040, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x061000, 0x063fff) AM_RAM
	AM_RANGE(0x064000, 0x064fff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06a9ff) AM_RAM
	AM_RANGE(0x06c000, 0x06c9ff) AM_RAM
	AM_RANGE(0x06ca00, 0x06cbff) AM_WRITEONLY
	AM_RANGE(0x070000, 0x070fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x074000, 0x074fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x078000, 0x078001) AM_READ_PORT("P1")
	AM_RANGE(0x078002, 0x078003) AM_READ_PORT("P2")
	AM_RANGE(0x078004, 0x078005) AM_READ_PORT("DSW1")
	AM_RANGE(0x078006, 0x078007) AM_READ_PORT("DSW2")
	AM_RANGE(0x07c000, 0x07c001) AM_WRITE(terraf_io_w)
	AM_RANGE(0x07c002, 0x07c003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x07c004, 0x07c005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x07c00a, 0x07c00b) AM_WRITE(sound_command_w)
	//AM_RANGE(0x07c00e, 0x07c00f) AM_WRITE(armedf_mcu_cmd) /* MCU Command ? */
	//AM_RANGE(0x07c00c, 0x07c00d) AM_WRITENOP      /* Watchdog ? cycle 0000 -> 0100 -> 0200 back to 0000 */
ADDRESS_MAP_END

static ADDRESS_MAP_START( armedf_map, ADDRESS_SPACE_PROGRAM, 16 )
	AM_RANGE(0x000000, 0x05ffff) AM_ROM
	AM_RANGE(0x060000, 0x060fff) AM_RAM AM_BASE_SIZE_GENERIC(spriteram)
	AM_RANGE(0x061000, 0x065fff) AM_RAM
	AM_RANGE(0x066000, 0x066fff) AM_RAM_WRITE(armedf_bg_videoram_w) AM_BASE_MEMBER(armedf_state, bg_videoram)
	AM_RANGE(0x067000, 0x067fff) AM_RAM_WRITE(armedf_fg_videoram_w) AM_BASE_MEMBER(armedf_state, fg_videoram)
	AM_RANGE(0x068000, 0x069fff) AM_RAM_WRITE(armedf_text_videoram_w) AM_BASE_MEMBER(armedf_state, text_videoram)
	AM_RANGE(0x06a000, 0x06afff) AM_RAM_WRITE(paletteram16_xxxxRRRRGGGGBBBB_word_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0x06b000, 0x06bfff) AM_RAM
	AM_RANGE(0x06c000, 0x06c001) AM_READ_PORT("P1")
	AM_RANGE(0x06c002, 0x06c003) AM_READ_PORT("P2")
	AM_RANGE(0x06c004, 0x06c005) AM_READ_PORT("DSW1")
	AM_RANGE(0x06c006, 0x06c007) AM_READ_PORT("DSW2")
	AM_RANGE(0x06c000, 0x06c7ff) AM_RAM
	AM_RANGE(0x06d000, 0x06d001) AM_WRITE(io_w)
	AM_RANGE(0x06d002, 0x06d003) AM_WRITE(armedf_bg_scrollx_w)
	AM_RANGE(0x06d004, 0x06d005) AM_WRITE(armedf_bg_scrolly_w)
	AM_RANGE(0x06d006, 0x06d007) AM_WRITE(armedf_fg_scrollx_w)
	AM_RANGE(0x06d008, 0x06d009) AM_WRITE(armedf_fg_scrolly_w)
	AM_RANGE(0x06d00a, 0x06d00b) AM_WRITE(sound_command_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xf7ff) AM_ROM
	AM_RANGE(0xf800, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( cclimbr2_soundmap, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0xbfff) AM_ROM
	AM_RANGE(0xc000, 0xffff) AM_RAM
ADDRESS_MAP_END


static ADDRESS_MAP_START( terrafb_extraz80_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x3fff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( terrafb_extraz80_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
ADDRESS_MAP_END


static READ8_HANDLER( soundlatch_clear_r )
{
	soundlatch_clear_w(space, 0, 0);
	return 0;
}

static ADDRESS_MAP_START( sound_portmap, ADDRESS_SPACE_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x0, 0x1) AM_DEVWRITE("ymsnd", ym3812_w)
	AM_RANGE(0x2, 0x2) AM_DEVWRITE("dac1", dac_signed_w)
	AM_RANGE(0x3, 0x3) AM_DEVWRITE("dac2", dac_signed_w)
	AM_RANGE(0x4, 0x4) AM_READ(soundlatch_clear_r)
	AM_RANGE(0x6, 0x6) AM_READ(soundlatch_r)
ADDRESS_MAP_END


/*************************************
 *
 *  Input ports
 *
 *************************************/

#define NIHON_SINGLE_JOYSTICK(_n_) \
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(_n_) \
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(_n_) \
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )

static INPUT_PORTS_START( armedf )
	PORT_START("P1")
	NIHON_SINGLE_JOYSTICK(1)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0xf000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("P2")
	NIHON_SINGLE_JOYSTICK(2)
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE( 0x0200, IP_ACTIVE_LOW )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_TILT )
	PORT_BIT( 0xf800, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Lives ) )				PORT_DIPLOCATION("SW1:1,2")
	PORT_DIPSETTING(    0x03, "3" )
	PORT_DIPSETTING(    0x02, "4" )
	PORT_DIPSETTING(    0x01, "5" )
	PORT_DIPSETTING(    0x00, "6" )
	/* SW1:3,4 defined in manual/test-mode as:
    PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
    PORT_DIPSETTING(    0x04, "20k" )
    PORT_DIPSETTING(    0x00, "40k" )
    PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
    PORT_DIPSETTING(    0x08, "60k" )
    PORT_DIPSETTING(    0x00, "80k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )			PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "20k then every 80k" )
	PORT_DIPSETTING(    0x08, "40k then every 60k" )
	PORT_DIPSETTING(    0x00, "40k then every 80k" )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Demo_Sounds ) )			PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Cabinet ) )				PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Difficulty ) )			PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Coin_A ) )				PORT_DIPLOCATION("SW2:1,2")
	PORT_DIPSETTING(    0x01, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x03, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x02, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Free_Play ) )
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )				PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0x30, 0x00, DEF_STR( Allow_Continue ) )		PORT_DIPLOCATION("SW2:5,6")	// not in the "test mode"
	PORT_DIPSETTING(    0x30, DEF_STR( No ) )
	PORT_DIPSETTING(    0x20, "3 Times" )
	PORT_DIPSETTING(    0x10, "5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Flip_Screen ) )			PORT_DIPLOCATION("SW2:7") // not in the "test mode"
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( legion )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("DSW1")
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Bonus_Life ) )			PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "30k Then Every 100k" )
	PORT_DIPSETTING(    0x00, "50k Only" )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Demo_Sounds ) )			PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Flip_Screen ) )			PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x20, 0x20, "SW1:6" )				/* Listed as "Unused" */
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW1:7" )				/* Listed as "Unused" */
#if LEGION_HACK // see notes
	PORT_DIPNAME( 0x80, 0x80, "Allow Invulnerability (Cheat)" )	PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )				/* Listed as "Unused" */
#endif

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x10, "Coin Slots" )					PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x10, "Common" )
	PORT_DIPSETTING(    0x00, "Individual" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Difficulty ) )			PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
#if LEGION_HACK // see notes
	PORT_DIPNAME( 0x40, 0x40, "P1 Invulnerability (Cheat)")		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, "P2 Invulnerability (Cheat)")		PORT_DIPLOCATION("SW2:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
#else
	PORT_DIPUNUSED_DIPLOC( 0x40, 0x40, "SW2:7" )				/* Listed as "Unused" */
#endif
INPUT_PORTS_END


static INPUT_PORTS_START( terraf )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
    PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
    PORT_DIPSETTING(    0x04, "20k" )
    PORT_DIPSETTING(    0x00, "50k" )
    PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
    PORT_DIPSETTING(    0x08, "60k" )
    PORT_DIPSETTING(    0x00, "90k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )			PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "20k then every 60k" )
	PORT_DIPSETTING(    0x04, "20k then every 90k" )
	PORT_DIPSETTING(    0x08, "50k then every 60k" )
	PORT_DIPSETTING(    0x00, "50k then every 90k" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Demo_Sounds ) )			PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )


	PORT_MODIFY("DSW2")
	PORT_DIPUNUSED_DIPLOC( 0x10, 0x10, "SW2:5" )				/* Listed as "Unused" */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )			PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x00, DEF_STR( Allow_Continue ) )		PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, DEF_STR( No ) )
	PORT_DIPSETTING(    0x80, "Only 3 Times" )
	PORT_DIPSETTING(    0x40, "Only 5 Times" )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( kodure )
	PORT_INCLUDE( terraf )

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
    PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                PORT_DIPLOCATION("SW1:3")
    PORT_DIPSETTING(    0x04, DEF_STR( None ) )
    PORT_DIPSETTING(    0x00, "50k" )
    PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                PORT_DIPLOCATION("SW1:4")
    PORT_DIPSETTING(    0x08, "60k" )
    PORT_DIPSETTING(    0x00, "90k" )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )			PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x08, "50k then every 60k" )
	PORT_DIPSETTING(    0x00, "50k then every 90k" )
	PORT_DIPSETTING(    0x0c, "Every 60k" )
	PORT_DIPSETTING(    0x04, "Every 90k" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )			PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:7" )				/* Listed as "Unused" */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Coin_B ) )				PORT_DIPLOCATION("SW2:3,4")
	PORT_DIPSETTING(    0x00, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_3C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_6C ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Allow_Continue ) )		PORT_DIPLOCATION("SW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Yes ) )
INPUT_PORTS_END

static INPUT_PORTS_START( cclimbr2 )
	PORT_INCLUDE( armedf )

	PORT_MODIFY("P1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(1)

	PORT_MODIFY("P2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICKLEFT_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_JOYSTICKRIGHT_RIGHT ) PORT_8WAY PORT_PLAYER(2)

	PORT_MODIFY("DSW1")
	/* SW1:3,4 defined in manual/test-mode as:
    PORT_DIPNAME( 0x04, 0x04, "1st Bonus Life" )                    PORT_DIPLOCATION("SW1:3")
    PORT_DIPSETTING(    0x04, "30k" )
    PORT_DIPSETTING(    0x00, "60k" )
    PORT_DIPNAME( 0x08, 0x08, "2nd Bonus Life" )                    PORT_DIPLOCATION("SW1:4")
    PORT_DIPSETTING(    0x08, "70k" )
    PORT_DIPSETTING(    0x00, DEF_STR( None ) )*/
	PORT_DIPNAME( 0x0c, 0x0c, DEF_STR( Bonus_Life ) )				PORT_DIPLOCATION("SW1:3,4")
	PORT_DIPSETTING(    0x0c, "30K and 100k" )
	PORT_DIPSETTING(    0x08, "60k and 130k" )
	PORT_DIPSETTING(    0x04, "30k only" )
	PORT_DIPSETTING(    0x00, "60k only" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Difficulty ) )				PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Normal ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW1:8" )					/* Listed as "Unused" */

	PORT_MODIFY("DSW2")
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Allow_Continue ) )			PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( No ) )
	PORT_DIPSETTING(    0x10, "3 Times" )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Flip_Screen ) )				PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, "Partial Invulnerability (Cheat)")	PORT_DIPLOCATION("SW2:7") /* Listed as "Unused" */
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "SW2:8" )					/* Listed as "Unused" */
INPUT_PORTS_END



/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout char_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8, 20, 16, 28, 24,
			32+4, 32+0, 32+12, 32+8, 32+20, 32+16, 32+28, 32+24 },
	{ 0*64, 1*64, 2*64, 3*64, 4*64, 5*64, 6*64, 7*64,
			8*64, 9*64, 10*64, 11*64, 12*64, 13*64, 14*64, 15*64 },
	128*8
};

static const gfx_layout sprite_layout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, RGN_FRAC(1,2)+4, RGN_FRAC(1,2)+0, 12, 8, RGN_FRAC(1,2)+12, RGN_FRAC(1,2)+8,
			20, 16, RGN_FRAC(1,2)+20, RGN_FRAC(1,2)+16, 28, 24, RGN_FRAC(1,2)+28, RGN_FRAC(1,2)+24 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	64*8
};

static GFXDECODE_START( armedf )
	GFXDECODE_ENTRY( "gfx1", 0, char_layout,		 0*16,	32 )
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout,		64*16,	32 )
	GFXDECODE_ENTRY( "gfx3", 0, tile_layout,		96*16,	32 )
	GFXDECODE_ENTRY( "gfx4", 0, sprite_layout,	32*16,	32 )
GFXDECODE_END



/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_START( armedf )
{
	armedf_state *state = machine->driver_data<armedf_state>();

	state_save_register_global(machine, state->mcu_mode);
	state_save_register_global(machine, state->old_mcu_mode);
	state_save_register_global(machine, state->scroll_msb);
	state_save_register_global(machine, state->waiting_msb);
	state_save_register_global(machine, state->vreg);
	state_save_register_global(machine, state->fg_scrollx);
	state_save_register_global(machine, state->fg_scrolly);
	state_save_register_global(machine, state->bg_scrollx);
	state_save_register_global(machine, state->bg_scrolly);
}

static MACHINE_RESET( armedf )
{
	armedf_state *state = machine->driver_data<armedf_state>();

	state->mcu_mode = 0;
	state->old_mcu_mode = 0;
	state->scroll_msb = 0;
	state->waiting_msb = 0;
	state->vreg = 0;
	state->fg_scrollx = 0;
	state->fg_scrolly = 0;
	state->bg_scrollx = 0;
	state->bg_scrolly = 0;
}


static MACHINE_CONFIG_START( terraf, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(terraf_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( terrafb, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(terrafb_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_CPU_ADD("extra", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(terrafb_extraz80_map)
	MDRV_CPU_IO_MAP(terrafb_extraz80_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.0)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( kodure, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(kodure_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000) /* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( armedf, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(armedf_map)
	MDRV_CPU_VBLANK_INT("screen", irq1_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(sound_map)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(57)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(12*8, (64-12)*8-1, 1*8, 31*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( cclimbr2, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(cclimbr2_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)		/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( legion, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(legion_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( legiono, armedf_state )

	/* basic machine hardware */
	MDRV_CPU_ADD("maincpu", M68000, 8000000) /* 8 MHz?? */
	MDRV_CPU_PROGRAM_MAP(legiono_map)
	MDRV_CPU_VBLANK_INT("screen", irq2_line_hold)

	MDRV_CPU_ADD("audiocpu", Z80, 3072000)	/* 3.072 MHz???? */
	MDRV_CPU_PROGRAM_MAP(cclimbr2_soundmap)
	MDRV_CPU_IO_MAP(sound_portmap)
	MDRV_CPU_VBLANK_INT_HACK(irq0_line_hold,128)

	MDRV_MACHINE_START(armedf)
	MDRV_MACHINE_RESET(armedf)

	/* video hardware */
	MDRV_VIDEO_ATTRIBUTES(VIDEO_BUFFERS_SPRITERAM)

	MDRV_SCREEN_ADD("screen", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE(64*8, 32*8)
	MDRV_SCREEN_VISIBLE_AREA(14*8, (64-14)*8-1, 2*8, 30*8-1 )
	MDRV_GFXDECODE(armedf)
	MDRV_PALETTE_LENGTH(2048)

	MDRV_VIDEO_EOF(armedf)
	MDRV_VIDEO_START(armedf)
	MDRV_VIDEO_UPDATE(armedf)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")

	MDRV_SOUND_ADD("ymsnd", YM3812, 4000000)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MDRV_SOUND_ADD("dac1", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)

	MDRV_SOUND_ADD("dac2", DAC, 0)
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.40)
MACHINE_CONFIG_END


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( legion )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "lg1.bin", 0x000001, 0x010000, CRC(c4aeb724) SHA1(b4c0383f3b1fa6b1d5bdab0f3a5293c89a82a474) )
	ROM_LOAD16_BYTE( "lg3.bin", 0x000000, 0x010000, CRC(777e4935) SHA1(225766940059b4c12e69332ea77eb618dbd1467b) )
	ROM_LOAD16_BYTE( "legion.1b", 0x020001, 0x010000, CRC(c306660a) SHA1(31c6b868ba07677b5110c577335873354bff596f) ) // lg2
	ROM_LOAD16_BYTE( "legion.1d", 0x020000, 0x010000, CRC(c2e45e1e) SHA1(95cc359145b1b03123262891feed358407ba105a) ) // lg4

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "legion.1h", 0x00000, 0x04000, CRC(2ca4f7f0) SHA1(7cf997af9dd74ced9d28c047069ccfb67d72e257) ) // lg9

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "lg8.bin", 0x00000, 0x08000, CRC(e0596570) SHA1(68ddc950efc55a16e6abc699e3bad18ea19d579f) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "legion.1e", 0x00000, 0x10000, CRC(a9d70faf) SHA1(8b8b60ae49c55e931d6838e863463f6b2bf7adb0) ) // lg5
	ROM_LOAD( "legion.1f", 0x18000, 0x08000, CRC(f018313b) SHA1(860bc9937202dc3a40c9fa7caad11c2c2aa19f5c) ) // lg6

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "legion.1l", 0x00000, 0x10000, CRC(29b8adaa) SHA1(10338ebe7324960683de1f796dd311ed662e42b4) ) // lg13

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "legion.1k", 0x000000, 0x010000, CRC(ff5a0db9) SHA1(9308deb363d3b7686cc69485ec14201dd68f9a97) ) // lg12
	ROM_LOAD( "legion.1j", 0x010000, 0x010000, CRC(bae220c8) SHA1(392ae0fb0351dcad7b0e8e0ed4a1dc6e07f493df) ) // lg11

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD ( "lg7.bin", 0x0000, 0x4000, CRC(533e2b58) SHA1(a13ea4a530038760ffa87713903c59a932452717) )

	ROM_REGION( 0x8000, "user1", 0 )	/* ? */
	ROM_LOAD( "legion.1i",        0x0000, 0x8000, CRC(79f4a827) SHA1(25e4c1b5b8466627244b7226310e67e4261333b6) ) // lg10
ROM_END

ROM_START( legiono )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "legion.1a", 0x000001, 0x010000, CRC(8c0cda1d) SHA1(14b93d4fb4381ebc6a4ccdb480089bf69c6f474b) )
	ROM_LOAD16_BYTE( "legion.1c", 0x000000, 0x010000, CRC(21226660) SHA1(ee48812d6ec9d4dccc58684164916f91b71aabf2) )
	ROM_LOAD16_BYTE( "legion.1b", 0x020001, 0x010000, CRC(c306660a) SHA1(31c6b868ba07677b5110c577335873354bff596f) )
	ROM_LOAD16_BYTE( "legion.1d", 0x020000, 0x010000, CRC(c2e45e1e) SHA1(95cc359145b1b03123262891feed358407ba105a) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "legion.1h", 0x00000, 0x04000, CRC(2ca4f7f0) SHA1(7cf997af9dd74ced9d28c047069ccfb67d72e257) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "legion.1g", 0x00000, 0x08000, CRC(c50b0125) SHA1(83b5e9707152d97777fb65fa8820ba34ec2fac8d) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "legion.1e", 0x00000, 0x10000, CRC(a9d70faf) SHA1(8b8b60ae49c55e931d6838e863463f6b2bf7adb0) )
	ROM_LOAD( "legion.1f", 0x18000, 0x08000, CRC(f018313b) SHA1(860bc9937202dc3a40c9fa7caad11c2c2aa19f5c) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "legion.1l", 0x00000, 0x10000, CRC(29b8adaa) SHA1(10338ebe7324960683de1f796dd311ed662e42b4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "legion.1k", 0x000000, 0x010000, CRC(ff5a0db9) SHA1(9308deb363d3b7686cc69485ec14201dd68f9a97) )
	ROM_LOAD( "legion.1j", 0x010000, 0x010000, CRC(bae220c8) SHA1(392ae0fb0351dcad7b0e8e0ed4a1dc6e07f493df) )

	/* should lg7.bin be loaded here too? The ROM wasn't included in this set */

	ROM_REGION( 0x8000, "user1", 0 )	/* ? */
	ROM_LOAD( "legion.1i",        0x0000, 0x8000, CRC(79f4a827) SHA1(25e4c1b5b8466627244b7226310e67e4261333b6) )
ROM_END

ROM_START( terraf )
	ROM_REGION( 0x50000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tf-014.6e", 0x00000, 0x10000, CRC(8e5f557f) SHA1(3462a58146c3f33bf8686adbd2ead25dae3804a8) )
	ROM_LOAD16_BYTE( "tf-011.6h", 0x00001, 0x10000, CRC(5320162a) SHA1(eaffafcaf146cdddb03f40f92ce23dfd096eb89e) )
	ROM_LOAD16_BYTE( "tf-013.4e", 0x20000, 0x10000, CRC(a86951e0) SHA1(804cc6f143993f5a9d5f3798e971d7abfe94c3a8) )
	ROM_LOAD16_BYTE( "tf-010.4h", 0x20001, 0x10000, CRC(58b5f43b) SHA1(9df77235c0b7ac5af4258c04bd90d0a86ccc86b0) )
	ROM_LOAD16_BYTE( "tf-012.3e", 0x40000, 0x08000, CRC(4f0e1d76) SHA1(b8636acde7547358663b94bdc8d49b5cc6b596eb) )
	ROM_LOAD16_BYTE( "tf-009.3h", 0x40001, 0x08000, CRC(d1014280) SHA1(5ee8d71d77b31b25cce2bf1953c0a5166313a857) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "tf-001.17k", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tf-003.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tf-002.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "tf-10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )	/* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

/*

CPU

Main cpu 68000,Z80B*
Sound cpu Z80
Sound ic ym3812
Osc 16 and 24 Mhz

Note:

The custom mcu of original was been replaced by a small daughterboard inversely
connected to the main cpu board.It has a Z80B*,which uses 2kb of ram (6116 SRAM x1),
and some standard TTL ics.

ROMs

tfb-1 to 3 and tfb-6 to 8 main program
tfb-10 Z80B* program (mcu hack)
the rest of roms (sound program and gfx data) are the same of existing set

All roms are 24128,24256,27512
Note

This romset comes from a bootleg/hacked pcb.Game differences from original are:
Company logo and copyright string removed.

*/


ROM_START( terrafb )
	ROM_REGION( 0x50000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tfb-8.bin", 0x00000, 0x10000, CRC(b11a6fa7) SHA1(7bb2b98be02d8913796a6d4fa20eed16226ce6b9) )
	ROM_LOAD16_BYTE( "tfb-3.bin", 0x00001, 0x10000, CRC(6c6aa7ed) SHA1(ee5fdeb5411034ce0fd1c883ee25bf1fe9a3ec52) )
	ROM_LOAD16_BYTE( "tfb-7.bin", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "tfb-2.bin", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "tfb-6.bin", 0x40000, 0x08000, CRC(1de681a1) SHA1(bddf404988226698d65e075b4c21de736a862df1) )
	ROM_LOAD16_BYTE( "tfb-1.bin", 0x40001, 0x08000, CRC(6a0b94c7) SHA1(55fb32ab859bf51a1c79bf962bb677fa557216ed) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "tf-001.17k", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x4000, "extra", 0 )	/* z80 program (replacement mcu/blitter) */
	ROM_LOAD( "tfb-10.bin", 0x0000, 0x4000, CRC(3f9aa367) SHA1(8278fb357b2d68869e39efa01ff19005807b41f8) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tf-003.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tf-002.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )


	ROM_REGION( 0x0100, "proms", 0 )	/* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END


ROM_START( terrafu )
	ROM_REGION( 0x50000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "tf-8.6e", 0x00000, 0x10000, CRC(fea6dd64) SHA1(682eae338ce14808f134897f594fae1c69e75a1a) )
	ROM_LOAD16_BYTE( "tf-3.6h", 0x00001, 0x10000, CRC(02f9d05a) SHA1(88985373bc3cffbc838e0b701ecd732a417975a1) )
	ROM_LOAD16_BYTE( "tf-7.4e", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "tf-2.4h", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "tf-6.3e", 0x40000, 0x08000, CRC(b91e9ba3) SHA1(33e5272d1691859a2bb1f340eb4bdfdd5d73a5d4) )
	ROM_LOAD16_BYTE( "tf-1.3h", 0x40001, 0x08000, CRC(d6e22375) SHA1(c84fc19700b65ee36b0c7d75cd7c97f86c7f719d) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "tf-001.17k", 0x00000, 0x10000, CRC(eb6b4138) SHA1(04c53bf46d87a156d3fad86f051985d0df79bd20) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "tf-003.7d", 0x00000, 0x10000, CRC(d74085a1) SHA1(3f6ba85dbd6e48a502c115b2d322a586fc4f56c9) ) /* sprites */
	ROM_LOAD( "tf-002.9d", 0x10000, 0x10000, CRC(148aa0c5) SHA1(8d8a565540e91b384a9c154522501921b7da4d4e) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "tf-10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )	/* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( terrafa )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "8.6e", 0x00000, 0x10000, CRC(fd58fa06) SHA1(f1f5fbd153be5fd5669aada66134baedfeac5d32) )
	ROM_LOAD16_BYTE( "3.6h", 0x00001, 0x10000, CRC(54823a7d) SHA1(bdf67890428710470a622ea48383b3fae8de8cbd) )
	ROM_LOAD16_BYTE( "7.4e", 0x20000, 0x10000, CRC(fde8de7e) SHA1(6b0d27ec49c8c0609c110ad97938bec8c077ad18) )
	ROM_LOAD16_BYTE( "2.4h", 0x20001, 0x10000, CRC(db987414) SHA1(0a1734794c626cf9083d7854c9000c5daadfc3fd) )
	ROM_LOAD16_BYTE( "6.3e", 0x40000, 0x10000, CRC(a5bb8c3b) SHA1(36730b1a4a686de7251b80eb084a00f64bd7b40d) )
	ROM_LOAD16_BYTE( "1.3h", 0x40001, 0x10000, CRC(d2de6d28) SHA1(0371b5bf8e6105aaf8847c4b6d0d2d7d105079c1) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "11.17k", 0x00000, 0x10000,  CRC(4407d475) SHA1(96e86c7ef4dc997812436f7d0ddea332b4e6cb2b) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "9.11e", 0x00000, 0x08000, CRC(bc6f7cbc) SHA1(20b8a34de4bfa0c2fdcd2f7743a0ab35141f4bf9) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "5.15h", 0x00000, 0x10000, CRC(25d23dfd) SHA1(da32895c1aca403209b7fb181fa4fa23a8e74d32) ) /* foreground tiles */
	ROM_LOAD( "4.13h", 0x10000, 0x10000, CRC(b9b0fe27) SHA1(983c48239ba1524b517f89f281f2b70564bea1e9) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "15.8a", 0x00000, 0x10000, CRC(2144d8e0) SHA1(ed89da11abf3d79753b478603009970c2600ab60) ) /* background tiles */
	ROM_LOAD( "14.6a", 0x10000, 0x10000, CRC(744f5c9e) SHA1(696223a087bb575c7cfaba11e682b221ada461e4) )

	ROM_REGION( 0x20000, "gfx4", 0 )
	ROM_LOAD( "12.7d", 0x00000, 0x10000, CRC(2d1f2ceb) SHA1(77544e1c4bda06feac135a96bb76af7c79278dc0) ) /* sprites */
	ROM_LOAD( "13.9d", 0x10000, 0x10000, CRC(1d2f92d6) SHA1(e842c6bf95a5958a6ca2c85e68b9bc3cc15211a4) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "tf-10.11c", 0x0000, 0x4000, CRC(ac705812) SHA1(65be46ee959d8478cb6dffb25e61f7742276997b) )

	ROM_REGION( 0x0100, "proms", 0 )	/* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( kodure )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "kodure8.6e", 0x00000, 0x10000, CRC(6bbfb1e6) SHA1(ffc8f835e28ff5f5b11f826b74ac2712c3018178) )
	ROM_LOAD16_BYTE( "kodure3.6h", 0x00001, 0x10000, CRC(f9178ec8) SHA1(68085b6030f4d1f89931263df68635b3b276e9f0) )
	ROM_LOAD16_BYTE( "kodure7.5e", 0x20000, 0x10000, CRC(a7ee09bb) SHA1(70ea2ab09b44e9188eb9500d9663d11e521caa1a) )
	ROM_LOAD16_BYTE( "kodure2.5h", 0x20001, 0x10000, CRC(236d820f) SHA1(e8784c0bbfe22e63a442a8eac18247b740f437a8) )
	ROM_LOAD16_BYTE( "kodure6.3e", 0x40000, 0x10000, CRC(9120e728) SHA1(af2ce368d66d01cbad136ae119b31b1701ad0595) )
	ROM_LOAD16_BYTE( "kodure1.3h", 0x40001, 0x10000, CRC(345fe7a5) SHA1(56ad809cf4a609447cce3e0181ff86e3f0e8966c) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "kodure11.17k", 0x00000, 0x10000, CRC(dba51e2d) SHA1(49e799d39d298cd3e01602ae5a2d123dfbfa9134) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "kodure9.11e", 0x00000, 0x08000, CRC(e041356e) SHA1(3e4c8564e7a8c940bbe72db11759903aa295287f) )	/* characters */

	ROM_REGION( 0x40000, "gfx2", 0 )
	ROM_LOAD( "kodure5.15h", 0x00000, 0x20000, CRC(0b510258) SHA1(e7ec89faa574b755605ddb67d6c908a9f5f2d6ac) )	/* foreground tiles */
	ROM_LOAD( "kodure4.14h", 0x20000, 0x10000, CRC(fb8e13e6) SHA1(f2eafcf6d7362dc62e808f582a7bd2970e5e1ad1) )

	ROM_REGION( 0x10000, "gfx3", 0 )
	ROM_LOAD( "kodure14.8a", 0x00000, 0x10000, CRC(94a9c3d0) SHA1(7a5d810ea370d158b2099c17f4d656fbd3deeac8) )	/* background tiles */

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "kodure12.8d", 0x00000, 0x20000, CRC(15f4021d) SHA1(b2ba6fda1a7bdaae97de4b0157b9b656b4385e08) )	/* sprites */
	ROM_LOAD( "kodure13.9d", 0x20000, 0x20000, CRC(b3b6c753) SHA1(9ad061cac9558320b5cfd1ac1ac8d7f1788270cc) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "kodure10.11c", 0x0000, 0x4000, CRC(f48be21d) SHA1(5d6db049f30cab98f672814a86a06609c1fa8fb4) )

	ROM_REGION( 0x0100, "proms", 0 )	/* Unknown use */
	ROM_LOAD( "n82s129an.11j", 0x0000, 0x0100, CRC(81244757) SHA1(6324f63e571f0f7a0bb9eb97f9994809db79493f) ) /* N82S129AN or compatible labled "TF" */
ROM_END

ROM_START( cclimbr2 )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "4.bin", 0x00000, 0x10000, CRC(7922ea14) SHA1(4395c1dfdeeba8173cce23b5453185c3ff976980) )
	ROM_LOAD16_BYTE( "1.bin", 0x00001, 0x10000, CRC(2ac7ed67) SHA1(5e9c3ca4f5b259ed7b08db8353be7d36ac947578) )
	ROM_LOAD16_BYTE( "6.bin", 0x20000, 0x10000, CRC(7905c992) SHA1(5e3ddb3b8127476b763578c3717ffe85aa6b342a) )
	ROM_LOAD16_BYTE( "5.bin", 0x20001, 0x10000, CRC(47be6c1e) SHA1(cbd928458087199e63020956c5b61925c3f055f4) )
	ROM_LOAD16_BYTE( "3.bin", 0x40000, 0x10000, CRC(1fb110d6) SHA1(a478096b4b075ff655d079e43151d6b8375f0caa) )
	ROM_LOAD16_BYTE( "2.bin", 0x40001, 0x10000, CRC(0024c15b) SHA1(0cd69a24139e878c09d4de37e4d102851765168f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "11.bin", 0x00000, 0x04000, CRC(fe0175be) SHA1(5c50fa07d8fa61d58a825bbc2cc5a7b85ff3e42e) )
	ROM_LOAD( "12.bin", 0x04000, 0x08000, CRC(5ddf18f2) SHA1(b66da5ad400d00b07160986e4841a309a3572bd1) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x00000, 0x08000, CRC(7f475266) SHA1(73d544731fcfd7266bca451880120c555d19ea5d) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x10000, CRC(cbdd3906) SHA1(0525599a5981f1e25ec6faf008e547da7a9ee2cb) ) /* foreground tiles */
	ROM_LOAD( "8.bin",  0x10000, 0x10000, CRC(b2a613c0) SHA1(1d92b85a0dd4b7e533677c454ec23359867defda) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "17.bin", 0x00000, 0x10000, CRC(e24bb2d7) SHA1(8f4b8410b77a50ae735d72f2f37e63784ebc10d9) ) /* background tiles */
	ROM_LOAD( "18.bin", 0x10000, 0x10000, CRC(56834554) SHA1(6d579c32fb57eb4eddc062cb2cc78b546f6607b2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(4bf838be) SHA1(6b1d7448caf406e47268a1276225bb0619b80cc9) ) /* sprites */
	ROM_LOAD( "16.bin", 0x10000, 0x10000, CRC(21a265c5) SHA1(a2b3a1e5aa545030d933c0f058f4f9a18e1af1c9) )
	ROM_LOAD( "13.bin", 0x20000, 0x10000, CRC(6b6ec999) SHA1(7749ce435f497732bd1b6958974cd95e960fc9fe) )
	ROM_LOAD( "14.bin", 0x30000, 0x10000, CRC(f426a4ad) SHA1(facccb21ca73c560d3a38e05e677782516d5b0c0) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(740d260f) SHA1(5b4487930c7a1fb0a796aec2243bec631b1b5104) )
ROM_END

ROM_START( cclimbr2a )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 64K*8 for 68000 code */
	ROM_LOAD16_BYTE( "4a.bin", 0x00000, 0x10000, CRC(e1d3192c) SHA1(6e0e6d731af401147f3c2566ddef3e169223eccd) )
	ROM_LOAD16_BYTE( "1a.bin", 0x00001, 0x10000, CRC(3ef84974) SHA1(04a6a2ac5b95d0a329ad7afc68b0ef480608c2d4) )
	ROM_LOAD16_BYTE( "6.bin",  0x20000, 0x10000, CRC(7905c992) SHA1(5e3ddb3b8127476b763578c3717ffe85aa6b342a) )
	ROM_LOAD16_BYTE( "5.bin",  0x20001, 0x10000, CRC(47be6c1e) SHA1(cbd928458087199e63020956c5b61925c3f055f4) )
	ROM_LOAD16_BYTE( "3.bin",  0x40000, 0x10000, CRC(1fb110d6) SHA1(a478096b4b075ff655d079e43151d6b8375f0caa) )
	ROM_LOAD16_BYTE( "2.bin",  0x40001, 0x10000, CRC(0024c15b) SHA1(0cd69a24139e878c09d4de37e4d102851765168f) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "11.bin", 0x00000, 0x04000, CRC(fe0175be) SHA1(5c50fa07d8fa61d58a825bbc2cc5a7b85ff3e42e) )
	ROM_LOAD( "12.bin", 0x04000, 0x08000, CRC(5ddf18f2) SHA1(b66da5ad400d00b07160986e4841a309a3572bd1) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "10.bin", 0x00000, 0x08000, CRC(7f475266) SHA1(73d544731fcfd7266bca451880120c555d19ea5d) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "7.bin",  0x00000, 0x10000, CRC(cbdd3906) SHA1(0525599a5981f1e25ec6faf008e547da7a9ee2cb) ) /* foreground tiles */
	ROM_LOAD( "8.bin",  0x10000, 0x10000, CRC(b2a613c0) SHA1(1d92b85a0dd4b7e533677c454ec23359867defda) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "17.bin", 0x00000, 0x10000, CRC(e24bb2d7) SHA1(8f4b8410b77a50ae735d72f2f37e63784ebc10d9) ) /* background tiles */
	ROM_LOAD( "18.bin", 0x10000, 0x10000, CRC(56834554) SHA1(6d579c32fb57eb4eddc062cb2cc78b546f6607b2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "15.bin", 0x00000, 0x10000, CRC(4bf838be) SHA1(6b1d7448caf406e47268a1276225bb0619b80cc9) ) /* sprites */
	ROM_LOAD( "16.bin", 0x10000, 0x10000, CRC(21a265c5) SHA1(a2b3a1e5aa545030d933c0f058f4f9a18e1af1c9) )
	ROM_LOAD( "13.bin", 0x20000, 0x10000, CRC(6b6ec999) SHA1(7749ce435f497732bd1b6958974cd95e960fc9fe) )
	ROM_LOAD( "14.bin", 0x30000, 0x10000, CRC(f426a4ad) SHA1(facccb21ca73c560d3a38e05e677782516d5b0c0) )

	ROM_REGION( 0x4000, "gfx5", 0 )	/* data for mcu/blitter */
	ROM_LOAD( "9.bin",  0x0000, 0x4000, CRC(740d260f) SHA1(5b4487930c7a1fb0a796aec2243bec631b1b5104) )
ROM_END

ROM_START( armedf )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "06.3d", 0x00000, 0x10000, CRC(0f9015e2) SHA1(0dc9c2cda0220bb9d03048c0eac62bc512d30622) )
	ROM_LOAD16_BYTE( "01.3f", 0x00001, 0x10000, CRC(816ff7c5) SHA1(1013883f2389aba4b8f397aacc0d681807a31cd5) )
	ROM_LOAD16_BYTE( "07.5d", 0x20000, 0x10000, CRC(5b3144a5) SHA1(98dd45049f242480c9ac0903fb38dfb2b8906be7) )
	ROM_LOAD16_BYTE( "02.4f", 0x20001, 0x10000, CRC(fa10c29d) SHA1(9c772ff6f749a542248c2ca260ac3460ae4f0cc6) )
	ROM_LOAD16_BYTE( "af_08.rom", 0x40000, 0x10000, CRC(d1d43600) SHA1(1a473b4958a02a33c0a02e7e72a70a9ee0c68c50) )
	ROM_LOAD16_BYTE( "af_03.rom", 0x40001, 0x10000, CRC(bbe1fe2d) SHA1(f47be23c7564b106d636d49d5f1da47daecd31df) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "af_10.rom", 0x00000, 0x10000, CRC(c5eacb87) SHA1(33af84b48fbda26729975b02cfb70f23c0bce6a2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "09.11c", 0x00000, 0x08000, CRC(5c6993d5) SHA1(3991851e5c4d2b3d72c372afdfbb710796874c15) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "af_04.rom", 0x00000, 0x10000, CRC(44d3af4f) SHA1(0c2cb54357c314e43cec6f959fe9d4a2c8bc8834) ) /* foreground tiles */
	ROM_LOAD( "af_05.rom", 0x10000, 0x10000, CRC(92076cab) SHA1(f47424817373a6735da2b2049b53da5b38178cec) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "af_14.rom", 0x00000, 0x10000, CRC(8c5dc5a7) SHA1(758140ddb9e60fb3950fe58bf53c7aea769a1a94) ) /* background tiles */
	ROM_LOAD( "af_13.rom", 0x10000, 0x10000, CRC(136a58a3) SHA1(5481e3ce404881a0470f8740f0de6e42283bedf2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "af_11.rom", 0x00000, 0x20000, CRC(b46c473c) SHA1(d8573225e2d8b133b3bdd4fa5a12d445c71d5e0e) ) /* sprites */
	ROM_LOAD( "af_12.rom", 0x20000, 0x20000, CRC(23cb6bfe) SHA1(34cb013827206bea71f5336b308ba92bee688506) )
ROM_END

ROM_START( armedff )
	ROM_REGION( 0x60000, "maincpu", 0 )	/* 68000 code */
	ROM_LOAD16_BYTE( "af_06.rom", 0x00000, 0x10000, CRC(c5326603) SHA1(135a8d96d792cf9b55e39e21160ee419be76d28a) )
	ROM_LOAD16_BYTE( "af_01.rom", 0x00001, 0x10000, CRC(458e9542) SHA1(22e4281eaac6b9b04af324cc96b5f3e4d1cefe43) )
	ROM_LOAD16_BYTE( "af_07.rom", 0x20000, 0x10000, CRC(cc8517f5) SHA1(93e4d3707a48551af89cadd0e016ddb65285a005) )
	ROM_LOAD16_BYTE( "af_02.rom", 0x20001, 0x10000, CRC(214ef220) SHA1(0c32349afc31fbcd825695679540a024f1e1acb2) )
	ROM_LOAD16_BYTE( "af_08.rom", 0x40000, 0x10000, CRC(d1d43600) SHA1(1a473b4958a02a33c0a02e7e72a70a9ee0c68c50) )
	ROM_LOAD16_BYTE( "af_03.rom", 0x40001, 0x10000, CRC(bbe1fe2d) SHA1(f47be23c7564b106d636d49d5f1da47daecd31df) )

	ROM_REGION( 0x10000, "audiocpu", 0 )	/* Z80 code (sound) */
	ROM_LOAD( "af_10.rom", 0x00000, 0x10000, CRC(c5eacb87) SHA1(33af84b48fbda26729975b02cfb70f23c0bce6a2) )

	ROM_REGION( 0x08000, "gfx1", 0 )
	ROM_LOAD( "af_09.rom", 0x00000, 0x08000, CRC(7025e92d) SHA1(e590682092c25bbfb674afeccbfc0e613c51d188) ) /* characters */

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "af_04.rom", 0x00000, 0x10000, CRC(44d3af4f) SHA1(0c2cb54357c314e43cec6f959fe9d4a2c8bc8834) ) /* foreground tiles */
	ROM_LOAD( "af_05.rom", 0x10000, 0x10000, CRC(92076cab) SHA1(f47424817373a6735da2b2049b53da5b38178cec) )

	ROM_REGION( 0x20000, "gfx3", 0 )
	ROM_LOAD( "af_14.rom", 0x00000, 0x10000, CRC(8c5dc5a7) SHA1(758140ddb9e60fb3950fe58bf53c7aea769a1a94) ) /* background tiles */
	ROM_LOAD( "af_13.rom", 0x10000, 0x10000, CRC(136a58a3) SHA1(5481e3ce404881a0470f8740f0de6e42283bedf2) )

	ROM_REGION( 0x40000, "gfx4", 0 )
	ROM_LOAD( "af_11.rom", 0x00000, 0x20000, CRC(b46c473c) SHA1(d8573225e2d8b133b3bdd4fa5a12d445c71d5e0e) ) /* sprites */
	ROM_LOAD( "af_12.rom", 0x20000, 0x20000, CRC(23cb6bfe) SHA1(34cb013827206bea71f5336b308ba92bee688506) )
ROM_END


/*************************************
 *
 *  Driver initialization
 *
 *************************************/

static DRIVER_INIT( terraf )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	state->scroll_type = 0;
}

static DRIVER_INIT( terrafu )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	state->scroll_type = 5;
}

static DRIVER_INIT( armedf )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	state->scroll_type = 1;
}


static DRIVER_INIT( kodure )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	state->scroll_type = 2;
}

static DRIVER_INIT( legion )
{
	armedf_state *state = machine->driver_data<armedf_state>();
#if LEGION_HACK
	/* This is a hack to allow you to use the extra features
         of 3 of the "Unused" Dip Switches (see notes above). */
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");
	RAM[0x0001d6 / 2] = 0x0001;
	/* To avoid checksum error */
	RAM[0x000488 / 2] = 0x4e71;
#endif

	state->scroll_type = 3;
}

static DRIVER_INIT( legiono )
{
	armedf_state *state = machine->driver_data<armedf_state>();
#if LEGION_HACK
	/* This is a hack to allow you to use the extra features
         of 3 of the "Unused" Dip Switches (see notes above). */
	UINT16 *RAM = (UINT16 *)memory_region(machine, "maincpu");
	RAM[0x0001d6/2] = 0x0001;
	/* No need to patch the checksum routine (see notes) ! */
#endif

	state->scroll_type = 6;
}

static DRIVER_INIT( cclimbr2 )
{
	armedf_state *state = machine->driver_data<armedf_state>();
	state->scroll_type = 4;
}


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

/*     YEAR, NAME,    PARENT,   MACHINE,  INPUT,    INIT,     MONITOR, COMPANY,         FULLNAME,                          FLAGS */
GAME( 1987, legion,   0,        legion,   legion,   legion,   ROT270, "Nichibutsu",     "Chouji Meikyuu Legion (ver 2.03)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1987, legiono,  legion,   legiono,  legion,   legiono,  ROT270, "Nichibutsu",     "Chouji Meikyuu Legion (ver 1.05)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1987, terraf,   0,        terraf,   terraf,   terraf,   ROT0,   "Nichibutsu",     "Terra Force (set 1)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1987, terrafb,  terraf,   terrafb,  terraf,   terrafu,  ROT0,   "bootleg",        "Terra Force (bootleg with additional Z80)", GAME_SUPPORTS_SAVE | GAME_NOT_WORKING )
GAME( 1987, terrafa,  terraf,   terraf,   terraf,   terrafu,  ROT0,   "Nichibutsu",     "Terra Force (set 2)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1987, terrafu,  terraf,   terraf,   terraf,   terrafu,  ROT0,   "Nichibutsu USA", "Terra Force (US)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1987, kodure,   0,        kodure,   kodure,   kodure,   ROT0,   "Nichibutsu",     "Kodure Ookami (Japan)", GAME_SUPPORTS_SAVE | GAME_IMPERFECT_GRAPHICS | GAME_UNEMULATED_PROTECTION )
GAME( 1988, cclimbr2, 0,        cclimbr2, cclimbr2, cclimbr2, ROT0,   "Nichibutsu",     "Crazy Climber 2 (Japan)", GAME_SUPPORTS_SAVE )
GAME( 1988, cclimbr2a,cclimbr2, cclimbr2, cclimbr2, cclimbr2, ROT0,   "Nichibutsu",     "Crazy Climber 2 (Japan, Harder)", GAME_SUPPORTS_SAVE )
GAME( 1988, armedf,   0,        armedf,   armedf,   armedf,   ROT270, "Nichibutsu",     "Armed Formation", GAME_SUPPORTS_SAVE )
GAME( 1988, armedff,  armedf,   armedf,   armedf,   armedf,   ROT270, "Nichibutsu (Fillmore license)", "Armed Formation (Fillmore license)", GAME_SUPPORTS_SAVE )
