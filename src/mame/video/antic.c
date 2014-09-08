/******************************************************************************
    Atari 400/800

    ANTIC video controller

    Juergen Buchmueller, June 1998
******************************************************************************/

#include "emu.h"
#include "cpu/m6502/m6502.h"
#include "includes/atari.h"

#ifdef MAME_DEBUG
#define VERBOSE 1
#else
#define VERBOSE 0
#endif

#define LOG(x)  do { if (VERBOSE) logerror x; } while (0)

// declaration of renderer functions
#define ANTIC_RENDERER(name) void name(address_space &space, VIDEO *video)

ANTIC_RENDERER( antic_mode_0_xx );
ANTIC_RENDERER( antic_mode_2_32 );
ANTIC_RENDERER( antic_mode_2_40 );
ANTIC_RENDERER( antic_mode_2_48 );
ANTIC_RENDERER( antic_mode_3_32 );
ANTIC_RENDERER( antic_mode_3_40 );
ANTIC_RENDERER( antic_mode_3_48 );
ANTIC_RENDERER( antic_mode_4_32 );
ANTIC_RENDERER( antic_mode_4_40 );
ANTIC_RENDERER( antic_mode_4_48 );
ANTIC_RENDERER( antic_mode_5_32 );
ANTIC_RENDERER( antic_mode_5_40 );
ANTIC_RENDERER( antic_mode_5_48 );
ANTIC_RENDERER( antic_mode_6_32 );
ANTIC_RENDERER( antic_mode_6_40 );
ANTIC_RENDERER( antic_mode_6_48 );
ANTIC_RENDERER( antic_mode_7_32 );
ANTIC_RENDERER( antic_mode_7_40 );
ANTIC_RENDERER( antic_mode_7_48 );
ANTIC_RENDERER( antic_mode_8_32 );
ANTIC_RENDERER( antic_mode_8_40 );
ANTIC_RENDERER( antic_mode_8_48 );
ANTIC_RENDERER( antic_mode_9_32 );
ANTIC_RENDERER( antic_mode_9_40 );
ANTIC_RENDERER( antic_mode_9_48 );
ANTIC_RENDERER( antic_mode_a_32 );
ANTIC_RENDERER( antic_mode_a_40 );
ANTIC_RENDERER( antic_mode_a_48 );
ANTIC_RENDERER( antic_mode_b_32 );
ANTIC_RENDERER( antic_mode_b_40 );
ANTIC_RENDERER( antic_mode_b_48 );
ANTIC_RENDERER( antic_mode_c_32 );
ANTIC_RENDERER( antic_mode_c_40 );
ANTIC_RENDERER( antic_mode_c_48 );
ANTIC_RENDERER( antic_mode_d_32 );
ANTIC_RENDERER( antic_mode_d_40 );
ANTIC_RENDERER( antic_mode_d_48 );
ANTIC_RENDERER( antic_mode_e_32 );
ANTIC_RENDERER( antic_mode_e_40 );
ANTIC_RENDERER( antic_mode_e_48 );
ANTIC_RENDERER( antic_mode_f_32 );
ANTIC_RENDERER( antic_mode_f_40 );
ANTIC_RENDERER( antic_mode_f_48 );
ANTIC_RENDERER( gtia_mode_1_32 );
ANTIC_RENDERER( gtia_mode_1_40 );
ANTIC_RENDERER( gtia_mode_1_48 );
ANTIC_RENDERER( gtia_mode_2_32 );
ANTIC_RENDERER( gtia_mode_2_40 );
ANTIC_RENDERER( gtia_mode_2_48 );
ANTIC_RENDERER( gtia_mode_3_32 );
ANTIC_RENDERER( gtia_mode_3_40 );
ANTIC_RENDERER( gtia_mode_3_48 );



/*************************************************************************
 * The priority tables tell which playfield, player or missile colors
 * have precedence about the others, depending on the contents of the
 * "prior" register. There are 64 possible priority selections.
 * The table is here to make it easier to build the 'illegal' priority
 * combinations that produce black or 'ILL' color.
 *************************************************************************/

/*************************************************************************
 * calculate player/missile priorities (GTIA prior at $D00D)
 * prior   color priorities in descending order
 * ------------------------------------------------------------------
 * bit 0   PL0    PL1    PL2    PL3    PF0    PF1    PF2    PF3/P4 BK
 *         all players in front of all playfield colors
 * bit 1   PL0    PL1    PF0    PF1    PF2    PF3/P4 PL2    PL3    BK
 *         pl 0+1 in front of pf 0-3 in front of pl 2+3
 * bit 2   PF0    PF1    PF2    PF3/P4 PL0    PL1    PL2    PL3    BK
 *         all playfield colors in front of all players
 * bit 3   PF0    PF1    PL0    PL1    PL2    PL3    PF2    PF3/P4 BK
 *         pf 0+1 in front of all players in front of pf 2+3
 * bit 4   missiles colors are PF3 (P4)
 *         missiles have the same priority as pf3
 * bit 5   PL0+PL1 and PL2+PL3 bits xored
 *         00: playfield, 01: PL0/2, 10: PL1/3 11: black (EOR)
 * bit 7+6 CTIA mod (00) or GTIA mode 1 to 3 (01, 10, 11)
 *************************************************************************/

/* player/missile #4 color is equal to playfield #3 */
#define PM4 PF3

/* bit masks for players and missiles */
#define P0 0x01
#define P1 0x02
#define P2 0x04
#define P3 0x08
#define M0 0x10
#define M1 0x20
#define M2 0x40
#define M3 0x80

/************************************************************************
 * Contents of the following table:
 *
 * PL0 -PL3  are the player/missile colors 0 to 3
 * P000-P011 are the 4 available color clocks for playfield color 0
 * P100-P111 are the 4 available color clocks for playfield color 1
 * P200-P211 are the 4 available color clocks for playfield color 2
 * P300-P311 are the 4 available color clocks for playfield color 3
 * ILL       is some undefined color. On my 800XL it looked light yellow ;)
 *
 * Each line holds the 8 bitmasks and resulting colors for player and
 * missile number 0 to 3 in their fixed priority order.
 * The 8 lines per block are for the 8 available playfield colors.
 * Yes, 8 colors because the text modes 2,3 and graphics mode F can
 * be combined with players. The result is the players color with
 * luminance of the modes foreground (ie. colpf1).
 * Any combination of players/missiles (256) is checked for the highest
 * priority player or missile and the resulting color is stored into
 * antic.prio_table. The second part (20-3F) contains the resulting
 * color values for the EOR mode, which is derived from the *visible*
 * player/missile colors calculated for the first part (00-1F).
 * The priorities of combining priority bits (which games use!) are:
 ************************************************************************/
static const UINT8 _pm_colors[32][8*2*8] = {
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 00
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 01
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 02
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2,   0,P2,   0,M3,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 03
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 04
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 05
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 06
		M0,   0,P0, ILL,M1,   0,P1, ILL,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 07
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 08
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 09
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0A
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2,   0,P2,   0,M3,   0,P3,   0,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0B
		M0, ILL,P0, ILL,M1, ILL,P1, ILL,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0C
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0D
		M0,   0,P0,   0,M1,   0,P1,   0,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0E
		M0,   0,P0,   0,M1,   0,P1,   0,M2,   0,P2,   0,M3,   0,P3,   0,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		M0, PL0,P0, PL0,M1, PL1,P1, PL1,M2, PL2,P2, PL2,M3, PL3,P3, PL3,  // 0F
		M0,   0,P0,   0,M1,   0,P1,   0,M2, PL2,P2, PL2,M3, PL3,P3, PL3,
		M0,   0,P0,   0,M1,   0,P1,   0,M2, ILL,P2, ILL,M3, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,P0,P000,M1,P100,P1,P100,M2,P200,P2,P200,M3,P300,P3,P300,
		M0,P001,P0,P001,M1,P101,P1,P101,M2,P201,P2,P201,M3,P301,P3,P301,
		M0,P010,P0,P010,M1,P110,P1,P110,M2,P210,P2,P210,M3,P310,P3,P310,
		M0,P011,P0,P011,M1,P111,P1,P111,M2,P211,P2,P211,M3,P311,P3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 10
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 11
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,  // 12
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,   0,P3,   0,
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,M0,P400,M1,P400,M2,P400,M3,P400,P2,P200,P3,P300,
		P0,P001,P1,P101,M0,P401,M1,P401,M2,P401,M3,P401,P2,P201,P3,P301,
		P0,P010,P1,P110,M0,P410,M1,P410,M2,P410,M3,P410,P2,P210,P3,P310,
		P0,P011,P1,P111,M0,P411,M1,P411,M2,P411,M3,P411,P2,P211,P3,P311
	},
	{
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,  // 13
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, PL2,P3, PL3,
		P0, PL0,P1, PL1,M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,M0,P400,M1,P400,M2,P400,M3,P400,P2,P200,P3,P300,
		P0,P001,P1,P101,M0,P401,M1,P401,M2,P401,M3,P401,P2,P201,P3,P301,
		P0,P010,P1,P110,M0,P410,M1,P410,M2,P410,M3,P410,P2,P210,P3,P310,
		P0,P011,P1,P111,M0,P411,M1,P411,M2,P411,M3,P411,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, PL0,P1, PL1,P2, PL2,P3, PL3,  // 14
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P400,M1,P400,M2,P400,M3,P400,P0,P000,P1,P100,P2,P200,P3,P300,
		M0,P401,M1,P401,M2,P401,M3,P401,P0,P001,P1,P101,P2,P201,P3,P301,
		M0,P410,M1,P410,M2,P410,M3,P410,P0,P010,P1,P110,P2,P210,P3,P310,
		M0,P411,M1,P411,M2,P411,M3,P411,P0,P011,P1,P111,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, PL0,P1, PL1, PL2,P3, PL3,  // 15
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, ILL,P1, ILL, PL2,P3, PL3,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0,   0,P1,   0, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0, 0,    0, 0,   0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P2,P0,P000,P1,P100,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P2,P0,P001,P1,P101,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P2,P0,P010,P1,P110,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P2,P0,P011,P1,P111,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, PL0,P1, PL1,P2, PL2,P3, PL3,  // 16
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0, ILL,P1, ILL,P2,   0,P3,   0,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P0,   0,P1,   0,P2,   0,P3,   0,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P0,P000,P1,P100,P2,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P0,P001,P1,P101,P2,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P0,P010,P1,P110,P2,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P0,P011,P1,P111,P2,P211,P3,P311
	},
	{
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, PL0,P1, PL1, PL2,P3, PL3,  // 17
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0, ILL,P1, ILL, PL2,P3, PL3,
		M0, PM4,M1, PM4,M2, PM4,M3, PM4,P2,P0,   0,P1,   0, ILL,P3, ILL,
		0,   0, 0,   0, 0,   0, 0,   0, 0, 0,    0, 0,   0,   0, 0,   0,
		M0,P000,M1,P100,M2,P200,M3,P300,P2,P0,P000,P1,P100,P200,P3,P300,
		M0,P001,M1,P101,M2,P201,M3,P301,P2,P0,P001,P1,P101,P201,P3,P301,
		M0,P010,M1,P110,M2,P210,M3,P310,P2,P0,P010,P1,P110,P210,P3,P310,
		M0,P011,M1,P111,M2,P211,M3,P311,P2,P0,P011,P1,P111,P211,P3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 18
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 19
		P0, ILL,P1, ILL,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P000,M1,P100,M2,P200,M3,P300,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P001,M1,P101,M2,P201,M3,P301,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P010,M1,P110,M2,P210,M3,P310,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P011,M1,P111,M2,P211,M3,P311
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1A
		P0, ILL,P1, ILL,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1B
		P0, ILL,P1, ILL,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0, PL0,P1, PL1,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1C
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1D
		P0,   0,P1,   0,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1E
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	},
	{
		P0, PL0,P1, PL1,P2, PL2,P3, PL3,M0, PM4,M1, PM4,M2, PM4,M3, PM4,  // 1F
		P0,   0,P1,   0,P2,   0,P3,   0,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		P0,   0,P1,   0,P2, ILL,P3, ILL,M0, PM4,M1, PM4,M2, PM4,M3, PM4,
		0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0, 0,   0,
		P0,P000,P1,P100,P2,P200,P3,P300,M0,P400,M1,P400,M2,P400,M3,P400,
		P0,P001,P1,P101,P2,P201,P3,P301,M0,P401,M1,P401,M2,P401,M3,P401,
		P0,P010,P1,P110,P2,P210,P3,P310,M0,P410,M1,P410,M2,P410,M3,P410,
		P0,P011,P1,P111,P2,P211,P3,P311,M0,P411,M1,P411,M2,P411,M3,P411
	}
};

/************************************************************************
 * prio_init
 * Initialize player/missile priority lookup tables
 ************************************************************************/
void prio_init()
{
	int i, j, pm, p, c;
	const UINT8 * prio;
	
	/* 32 priority bit combinations */
	for( i = 0; i < 32; i++ )
	{
		/* 8 playfield colors */
		for( j = 0; j < 8; j++ )
		{
			prio = &_pm_colors[i][j*16];
			/* 256 player/missile combinations to build */
			for( pm = 0; pm < 256; pm++ )
			{
				c = PFD; /* assume playfield color */
				for( p = 0; (c == PFD) && (p < 16); p += 2 )
				{
					if (((prio[p] & pm) == prio[p]) && (prio[p+1]))
						c = prio[p+1];
				}
				antic.prio_table[i][(j << 8) + pm] = c;
				if( (c==PL0 || c==P000 || c==P001 || c==P010 || c==P011) &&
				   (pm & (P0+P1))==(P0+P1))
					c = EOR;
				if( (c==PL2 || c==P200 || c==P201 || c==P210 || c==P211) &&
				   (pm & (P2+P3))==(P2+P3))
					c = EOR;
				antic.prio_table[32 + i][(j << 8) + pm] = c;
			}
		}
	}
}

/************************************************************************
 * cclk_init
 * Initialize "color clock" lookup tables
 ************************************************************************/
static void cclk_init()
{
	static const UINT8 _pf_21[4] =   {T00,T01,T10,T11};
	static const UINT8 _pf_1b[4] =   {G00,G01,G10,G11};
	static const UINT8 _pf_210b[4] = {PBK,PF0,PF1,PF2};
	static const UINT8 _pf_310b[4] = {PBK,PF0,PF1,PF3};
	int i;
	UINT8 * dst;
	
	/* setup color translation for the ANTIC modes */
	for( i = 0; i < 256; i++ )
	{
		/****** text mode (2,3) **********/
		dst = (UINT8 *)&antic.pf_21[0x000+i];
		*dst++ = _pf_21[(i>>6)&3];
		*dst++ = _pf_21[(i>>4)&3];
		*dst++ = _pf_21[(i>>2)&3];
		*dst++ = _pf_21[(i>>0)&3];
		
		/****** 4 color text (4,5) with pf2, D, E **********/
		dst = (UINT8 *)&antic.pf_x10b[0x000+i];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		dst = (UINT8 *)&antic.pf_x10b[0x100+i];
		*dst++ = _pf_310b[(i>>6)&3];
		*dst++ = _pf_310b[(i>>4)&3];
		*dst++ = _pf_310b[(i>>2)&3];
		*dst++ = _pf_310b[(i>>0)&3];
		
		/****** pf0 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x000+i*2];
		*dst++ = (i&0x80)?PF0:PBK;
		*dst++ = (i&0x40)?PF0:PBK;
		*dst++ = (i&0x20)?PF0:PBK;
		*dst++ = (i&0x10)?PF0:PBK;
		*dst++ = (i&0x08)?PF0:PBK;
		*dst++ = (i&0x04)?PF0:PBK;
		*dst++ = (i&0x02)?PF0:PBK;
		*dst++ = (i&0x01)?PF0:PBK;
		
		/****** pf1 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x200+i*2];
		*dst++ = (i&0x80)?PF1:PBK;
		*dst++ = (i&0x40)?PF1:PBK;
		*dst++ = (i&0x20)?PF1:PBK;
		*dst++ = (i&0x10)?PF1:PBK;
		*dst++ = (i&0x08)?PF1:PBK;
		*dst++ = (i&0x04)?PF1:PBK;
		*dst++ = (i&0x02)?PF1:PBK;
		*dst++ = (i&0x01)?PF1:PBK;
		
		/****** pf2 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x400+i*2];
		*dst++ = (i&0x80)?PF2:PBK;
		*dst++ = (i&0x40)?PF2:PBK;
		*dst++ = (i&0x20)?PF2:PBK;
		*dst++ = (i&0x10)?PF2:PBK;
		*dst++ = (i&0x08)?PF2:PBK;
		*dst++ = (i&0x04)?PF2:PBK;
		*dst++ = (i&0x02)?PF2:PBK;
		*dst++ = (i&0x01)?PF2:PBK;
		
		/****** pf3 color text (6,7), 9, B, C **********/
		dst = (UINT8 *)&antic.pf_3210b2[0x600+i*2];
		*dst++ = (i&0x80)?PF3:PBK;
		*dst++ = (i&0x40)?PF3:PBK;
		*dst++ = (i&0x20)?PF3:PBK;
		*dst++ = (i&0x10)?PF3:PBK;
		*dst++ = (i&0x08)?PF3:PBK;
		*dst++ = (i&0x04)?PF3:PBK;
		*dst++ = (i&0x02)?PF3:PBK;
		*dst++ = (i&0x01)?PF3:PBK;
		
		/****** 4 color graphics 4 cclks (8) **********/
		dst = (UINT8 *)&antic.pf_210b4[i*4];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		
		/****** 4 color graphics 2 cclks (A) **********/
		dst = (UINT8 *)&antic.pf_210b2[i*2];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>6)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>4)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>2)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		*dst++ = _pf_210b[(i>>0)&3];
		
		/****** high resolution graphics (F) **********/
		dst = (UINT8 *)&antic.pf_1b[i];
		*dst++ = _pf_1b[(i>>6)&3];
		*dst++ = _pf_1b[(i>>4)&3];
		*dst++ = _pf_1b[(i>>2)&3];
		*dst++ = _pf_1b[(i>>0)&3];
		
		/****** gtia mode 1 **********/
		dst = (UINT8 *)&antic.pf_gtia1[i];
		*dst++ = GT1+((i>>4)&15);
		*dst++ = GT1+((i>>4)&15);
		*dst++ = GT1+(i&15);
		*dst++ = GT1+(i&15);
		
		/****** gtia mode 2 **********/
		dst = (UINT8 *)&antic.pf_gtia2[i];
		*dst++ = GT2+((i>>4)&15);
		*dst++ = GT2+((i>>4)&15);
		*dst++ = GT2+(i&15);
		*dst++ = GT2+(i&15);
		
		/****** gtia mode 3 **********/
		dst = (UINT8 *)&antic.pf_gtia3[i];
		*dst++ = GT3+((i>>4)&15);
		*dst++ = GT3+((i>>4)&15);
		*dst++ = GT3+(i&15);
		*dst++ = GT3+(i&15);
		
	}
	
	/* setup used color tables */
	for( i = 0; i < 256; i++ )
	{
		/* used colors in text modes 2,3 */
		antic.uc_21[i] = (i) ? PF2 | PF1 : PF2;
		
		/* used colors in text modes 4,5 and graphics modes D,E */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x02: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x03: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x08: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x0c: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x20: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0x30: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_x10b[0x000+i] |= PF0; antic.uc_x10b[0x100+i] |= PF0; break;
			case 0x80: antic.uc_x10b[0x000+i] |= PF1; antic.uc_x10b[0x100+i] |= PF1; break;
			case 0xc0: antic.uc_x10b[0x000+i] |= PF2; antic.uc_x10b[0x100+i] |= PF3; break;
		}
		
		/* used colors in text modes 6,7 and graphics modes 9,B,C */
		if( i )
		{
			antic.uc_3210b2[0x000+i*2] |= PF0;
			antic.uc_3210b2[0x200+i*2] |= PF1;
			antic.uc_3210b2[0x400+i*2] |= PF2;
			antic.uc_3210b2[0x600+i*2] |= PF3;
		}
		
		/* used colors in graphics mode 8 */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_210b4[i*4] |= PF0; break;
			case 0x02: antic.uc_210b4[i*4] |= PF1; break;
			case 0x03: antic.uc_210b4[i*4] |= PF2; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_210b4[i*4] |= PF0; break;
			case 0x08: antic.uc_210b4[i*4] |= PF1; break;
			case 0x0c: antic.uc_210b4[i*4] |= PF2; break;
		}
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_210b4[i*4] |= PF0; break;
			case 0x20: antic.uc_210b4[i*4] |= PF1; break;
			case 0x30: antic.uc_210b4[i*4] |= PF2; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_210b4[i*4] |= PF0; break;
			case 0x80: antic.uc_210b4[i*4] |= PF1; break;
			case 0xc0: antic.uc_210b4[i*4] |= PF2; break;
		}
		
		/* used colors in graphics mode A */
		switch( i & 0x03 )
		{
			case 0x01: antic.uc_210b2[i*2] |= PF0; break;
			case 0x02: antic.uc_210b2[i*2] |= PF1; break;
			case 0x03: antic.uc_210b2[i*2] |= PF2; break;
		}
		switch( i & 0x0c )
		{
			case 0x04: antic.uc_210b2[i*2] |= PF0; break;
			case 0x08: antic.uc_210b2[i*2] |= PF1; break;
			case 0x0c: antic.uc_210b2[i*2] |= PF2; break;
		}
		switch( i & 0x30 )
		{
			case 0x10: antic.uc_210b2[i*2] |= PF0; break;
			case 0x20: antic.uc_210b2[i*2] |= PF1; break;
			case 0x30: antic.uc_210b2[i*2] |= PF2; break;
		}
		switch( i & 0xc0 )
		{
			case 0x40: antic.uc_210b2[i*2] |= PF0; break;
			case 0x80: antic.uc_210b2[i*2] |= PF1; break;
			case 0xc0: antic.uc_210b2[i*2] |= PF2; break;
		}
		
		/* used colors in graphics mode F */
		if( i )
			antic.uc_1b[i] |= PF1;
		
		/* used colors in GTIA graphics modes */
		/* GTIA 1 is 16 different luminances with hue of colbk */
		antic.uc_g1[i] = 0x00;
		/* GTIA 2 is all 9 colors (8..15 is colbk) */
		switch( i & 0x0f )
		{
			case 0x00: antic.uc_g2[i] = 0x10; break;
			case 0x01: antic.uc_g2[i] = 0x20; break;
			case 0x02: antic.uc_g2[i] = 0x40; break;
			case 0x03: antic.uc_g2[i] = 0x80; break;
			case 0x04: antic.uc_g2[i] = 0x01; break;
			case 0x05: antic.uc_g2[i] = 0x02; break;
			case 0x06: antic.uc_g2[i] = 0x04; break;
			case 0x07: antic.uc_g2[i] = 0x08; break;
			default:   antic.uc_g2[i] = 0x00;
		}
		
		/* GTIA 3 is 16 different hues with luminance of colbk */
		antic.uc_g3[i] = 0x00;
	}
}



ANTIC antic;

void antic_start(running_machine &machine)
{	
	/* save states */
	machine.save().save_pointer(NAME((UINT8 *) &antic.r), sizeof(antic.r));
	machine.save().save_pointer(NAME((UINT8 *) &antic.w), sizeof(antic.w));
}

void antic_vstart(running_machine &machine)
{	
	LOG(("atari antic_vh_start\n"));
	memset(&antic, 0, sizeof(antic));
	
	antic.bitmap = auto_bitmap_ind16_alloc(machine, machine.first_screen()->width(), machine.first_screen()->height());
	
	antic.cclk_expand = auto_alloc_array(machine, UINT32, 21 * 256);
	
	antic.pf_21       = &antic.cclk_expand[ 0 * 256];
	antic.pf_x10b     = &antic.cclk_expand[ 1 * 256];
	antic.pf_3210b2   = &antic.cclk_expand[ 3 * 256];
	antic.pf_210b4    = &antic.cclk_expand[11 * 256];
	antic.pf_210b2    = &antic.cclk_expand[15 * 256];
	antic.pf_1b       = &antic.cclk_expand[17 * 256];
	antic.pf_gtia1    = &antic.cclk_expand[18 * 256];
	antic.pf_gtia2    = &antic.cclk_expand[19 * 256];
	antic.pf_gtia3    = &antic.cclk_expand[20 * 256];
	
	antic.used_colors = auto_alloc_array(machine, UINT8, 21 * 256);
	
	memset(antic.used_colors, 0, 21 * 256 * sizeof(UINT8));
	
	antic.uc_21       = &antic.used_colors[ 0 * 256];
	antic.uc_x10b     = &antic.used_colors[ 1 * 256];
	antic.uc_3210b2   = &antic.used_colors[ 3 * 256];
	antic.uc_210b4    = &antic.used_colors[11 * 256];
	antic.uc_210b2    = &antic.used_colors[15 * 256];
	antic.uc_1b       = &antic.used_colors[17 * 256];
	antic.uc_g1       = &antic.used_colors[18 * 256];
	antic.uc_g2       = &antic.used_colors[19 * 256];
	antic.uc_g3       = &antic.used_colors[20 * 256];
	
	LOG(("atari cclk_init\n"));
	cclk_init();
	
	for (int i = 0; i < 64; i++)
		antic.prio_table[i] = auto_alloc_array(machine, UINT8, 8*256);
	
	LOG(("atari prio_init\n"));
	prio_init();
	
	for (int i = 0; i < machine.first_screen()->height(); i++)
		antic.video[i] = auto_alloc_clear(machine, VIDEO);
}

/**************************************************************
 *
 * Reset ANTIC
 *
 **************************************************************/

void antic_reset(void)
{
	/* reset the ANTIC read / write registers */
	memset(&antic.r, 0, sizeof(antic.r));
	memset(&antic.w, 0, sizeof(antic.w));
	antic.r.antic00 = 0xff;
	antic.r.antic01 = 0xff;
	antic.r.antic02 = 0xff;
	antic.r.antic03 = 0xff;
	antic.r.antic04 = 0xff;
	antic.r.antic05 = 0xff;
	antic.r.antic06 = 0xff;
	antic.r.antic07 = 0xff;
	antic.r.antic08 = 0xff;
	antic.r.antic09 = 0xff;
	antic.r.antic0a = 0xff;
	antic.r.penh    = 0x00;
	antic.r.penv    = 0x00;
	antic.r.antic0e = 0xff;
	antic.r.nmist   = 0x1f;
}

/**************************************************************
 *
 * Read ANTIC hardware registers
 *
 **************************************************************/
READ8_MEMBER ( atari_common_state::atari_antic_r )
{
	UINT8 data = 0xff;

	switch (offset & 15)
	{
	case  0: /* nothing */
		data = antic.r.antic00;
		break;
	case  1: /* nothing */
		data = antic.r.antic01;
		break;
	case  2: /* nothing */
		data = antic.r.antic02;
		break;
	case  3: /* nothing */
		data = antic.r.antic03;
		break;
	case  4: /* nothing */
		data = antic.r.antic04;
		break;
	case  5: /* nothing */
		data = antic.r.antic05;
		break;
	case  6: /* nothing */
		data = antic.r.antic06;
		break;
	case  7: /* nothing */
		data = antic.r.antic07;
		break;
	case  8: /* nothing */
		data = antic.r.antic08;
		break;
	case  9: /* nothing */
		data = antic.r.antic09;
		break;
	case 10: /* WSYNC read */
		space.machine().device("maincpu")->execute().spin_until_trigger(TRIGGER_HSYNC);
		antic.w.wsync = 1;
		data = antic.r.antic0a;
		break;
	case 11: /* vert counter (scanline / 2) */
		data = antic.r.vcount = antic.scanline >> 1;
		break;
	case 12: /* light pen horz pos */
		data = antic.r.penh;
		break;
	case 13: /* light pen vert pos */
		data = antic.r.penv;
		break;
	case 14: /* NMI enable */
		data = antic.r.antic0e;
		break;
	case 15: /* NMI status */
		data = antic.r.nmist;
		break;
	}
	return data;
}

/**************************************************************
 *
 * Write ANTIC hardware registers
 *
 **************************************************************/

WRITE8_MEMBER ( atari_common_state::atari_antic_w )
{
	int temp;

	switch (offset & 15)
	{
	case  0:
		if( data == antic.w.dmactl )
			break;
		LOG(("ANTIC 00 write DMACTL $%02X\n", data));
		antic.w.dmactl = data;
		switch (data & 3)
		{
			case 0: antic.pfwidth =  0; break;
			case 1: antic.pfwidth = 32; break;
			case 2: antic.pfwidth = 40; break;
			case 3: antic.pfwidth = 48; break;
		}
		break;
	case  1:
		if( data == antic.w.chactl )
			break;
		LOG(("ANTIC 01 write CHACTL $%02X\n", data));
		antic.w.chactl = data;
		antic.chand = (data & 1) ? 0x00 : 0xff;
		antic.chxor = (data & 2) ? 0xff : 0x00;
		break;
	case  2:
		LOG(("ANTIC 02 write DLISTL $%02X\n", data));
		antic.w.dlistl = data;
		temp = (antic.w.dlisth << 8) + antic.w.dlistl;
		antic.dpage = temp & DPAGE;
		antic.doffs = temp & DOFFS;
		break;
	case  3:
		LOG(("ANTIC 03 write DLISTH $%02X\n", data));
		antic.w.dlisth = data;
		temp = (antic.w.dlisth << 8) + antic.w.dlistl;
		antic.dpage = temp & DPAGE;
		antic.doffs = temp & DOFFS;
		break;
	case  4:
		if( data == antic.w.hscrol )
			break;
		LOG(("ANTIC 04 write HSCROL $%02X\n", data));
		antic.w.hscrol = data & 15;
		break;
	case  5:
		if( data == antic.w.vscrol )
			break;
		LOG(("ANTIC 05 write VSCROL $%02X\n", data));
		antic.w.vscrol = data & 15;
		break;
	case  6:
		if( data == antic.w.pmbasl )
			break;
		LOG(("ANTIC 06 write PMBASL $%02X\n", data));
		/* antic.w.pmbasl = data; */
		break;
	case  7:
		if( data == antic.w.pmbash )
			break;
		LOG(("ANTIC 07 write PMBASH $%02X\n", data));
		antic.w.pmbash = data;
		antic.pmbase_s = (data & 0xfc) << 8;
		antic.pmbase_d = (data & 0xf8) << 8;
		break;
	case  8:
		if( data == antic.w.chbasl )
			break;
		LOG(("ANTIC 08 write CHBASL $%02X\n", data));
		/* antic.w.chbasl = data; */
		break;
	case  9:
		if( data == antic.w.chbash )
			break;
		LOG(("ANTIC 09 write CHBASH $%02X\n", data));
		antic.w.chbash = data;
		break;
	case 10: /* WSYNC write */
		LOG(("ANTIC 0A write WSYNC  $%02X\n", data));
		space.machine().device("maincpu")->execute().spin_until_trigger(TRIGGER_HSYNC);
		antic.w.wsync = 1;
		break;
	case 11:
		if( data == antic.w.antic0b )
			break;
		LOG(("ANTIC 0B write ?????? $%02X\n", data));
		antic.w.antic0b = data;
		break;
	case 12:
		if( data == antic.w.antic0c )
			break;
		LOG(("ANTIC 0C write ?????? $%02X\n", data));
		antic.w.antic0c = data;
		break;
	case 13:
		if( data == antic.w.antic0d )
			break;
		LOG(("ANTIC 0D write ?????? $%02X\n", data));
		antic.w.antic0d = data;
		break;
	case 14:
		if( data == antic.w.nmien )
			break;
		LOG(("ANTIC 0E write NMIEN  $%02X\n", data));
		antic.w.nmien  = data;
		break;
	case 15:
		LOG(("ANTIC 0F write NMIRES $%02X\n", data));
		antic.r.nmist = 0x1f;
		antic.w.nmires = data;
		break;
	}
}


static const atari_renderer_func renderer[2][19][5] = {
	/*   no playfield    narrow          normal          wide         */
	{
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_2_32,antic_mode_2_40,antic_mode_2_48},
		{antic_mode_0_xx,antic_mode_3_32,antic_mode_3_40,antic_mode_3_48},
		{antic_mode_0_xx,antic_mode_4_32,antic_mode_4_40,antic_mode_4_48},
		{antic_mode_0_xx,antic_mode_5_32,antic_mode_5_40,antic_mode_5_48},
		{antic_mode_0_xx,antic_mode_6_32,antic_mode_6_40,antic_mode_6_48},
		{antic_mode_0_xx,antic_mode_7_32,antic_mode_7_40,antic_mode_7_48},
		{antic_mode_0_xx,antic_mode_8_32,antic_mode_8_40,antic_mode_8_48},
		{antic_mode_0_xx,antic_mode_9_32,antic_mode_9_40,antic_mode_9_48},
		{antic_mode_0_xx,antic_mode_a_32,antic_mode_a_40,antic_mode_a_48},
		{antic_mode_0_xx,antic_mode_b_32,antic_mode_b_40,antic_mode_b_48},
		{antic_mode_0_xx,antic_mode_c_32,antic_mode_c_40,antic_mode_c_48},
		{antic_mode_0_xx,antic_mode_d_32,antic_mode_d_40,antic_mode_d_48},
		{antic_mode_0_xx,antic_mode_e_32,antic_mode_e_40,antic_mode_e_48},
		{antic_mode_0_xx,antic_mode_f_32,antic_mode_f_40,antic_mode_f_48},
		{antic_mode_0_xx, gtia_mode_1_32, gtia_mode_1_40, gtia_mode_1_48},
		{antic_mode_0_xx, gtia_mode_2_32, gtia_mode_2_40, gtia_mode_2_48},
		{antic_mode_0_xx, gtia_mode_3_32, gtia_mode_3_40, gtia_mode_3_48},
	},
	/*   with hscrol enabled playfield width is +32 color clocks      */
	/*   no playfield    narrow->normal  normal->wide    wide->wide   */
	{
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx,antic_mode_0_xx},
		{antic_mode_0_xx,antic_mode_2_40,antic_mode_2_48,antic_mode_2_48},
		{antic_mode_0_xx,antic_mode_3_40,antic_mode_3_48,antic_mode_3_48},
		{antic_mode_0_xx,antic_mode_4_40,antic_mode_4_48,antic_mode_4_48},
		{antic_mode_0_xx,antic_mode_5_40,antic_mode_5_48,antic_mode_5_48},
		{antic_mode_0_xx,antic_mode_6_40,antic_mode_6_48,antic_mode_6_48},
		{antic_mode_0_xx,antic_mode_7_40,antic_mode_7_48,antic_mode_7_48},
		{antic_mode_0_xx,antic_mode_8_40,antic_mode_8_48,antic_mode_8_48},
		{antic_mode_0_xx,antic_mode_9_40,antic_mode_9_48,antic_mode_9_48},
		{antic_mode_0_xx,antic_mode_a_40,antic_mode_a_48,antic_mode_a_48},
		{antic_mode_0_xx,antic_mode_b_40,antic_mode_b_48,antic_mode_b_48},
		{antic_mode_0_xx,antic_mode_c_40,antic_mode_c_48,antic_mode_c_48},
		{antic_mode_0_xx,antic_mode_d_40,antic_mode_d_48,antic_mode_d_48},
		{antic_mode_0_xx,antic_mode_e_40,antic_mode_e_48,antic_mode_e_48},
		{antic_mode_0_xx,antic_mode_f_40,antic_mode_f_48,antic_mode_f_48},
		{antic_mode_0_xx, gtia_mode_1_40, gtia_mode_1_48, gtia_mode_1_48},
		{antic_mode_0_xx, gtia_mode_2_40, gtia_mode_2_48, gtia_mode_2_48},
		{antic_mode_0_xx, gtia_mode_3_40, gtia_mode_3_48, gtia_mode_3_48},
	}
};

void antic_render(address_space &space, int param1, int param2, int param3)
{
	VIDEO *video = antic.video[antic.scanline];
	(*renderer[param1][param2][param3])(space, video);
}

/*************  ANTIC mode 00: *********************************
 * generate 1-8 empty scanlines
 ***************************************************************/
ANTIC_RENDERER( antic_mode_0_xx )
{
	PREPARE();
	memset(dst, PBK, HWIDTH*4);
	POST();
}

/*************  ANTIC mode 01: *********************************
 * display list jump, eventually wait for vsync
 ***************************************************************/


/*************  ANTIC mode 02: *********************************
 * character mode 8x8:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODE2(s) COPY4(dst, antic.pf_21[video->data[s]])

ANTIC_RENDERER( antic_mode_2_32 )
{
	PREPARE_TXT2(space, 32);
	REP32(MODE2);
	POST_TXT(32);
}
ANTIC_RENDERER( antic_mode_2_40 )
{
	PREPARE_TXT2(space, 40);
	REP40(MODE2);
	POST_TXT(40);
}
ANTIC_RENDERER( antic_mode_2_48 )
{
	PREPARE_TXT2(space, 48);
	REP48(MODE2);
	POST_TXT(48);
}

/*************  ANTIC mode 03: *********************************
 * character mode 8x10:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODE3(s) COPY4(dst, antic.pf_21[video->data[s]])

ANTIC_RENDERER( antic_mode_3_32 )
{
	PREPARE_TXT3(space, 32);
	REP32(MODE3);
	POST_TXT(32);
}
ANTIC_RENDERER( antic_mode_3_40 )
{
	PREPARE_TXT3(space, 40);
	REP40(MODE3);
	POST_TXT(40);
}
ANTIC_RENDERER( antic_mode_3_48 )
{
	PREPARE_TXT3(space, 48);
	REP48(MODE3);
	POST_TXT(48);
}

/*************  ANTIC mode 04: *********************************
 * character mode 8x8:4 multi color (32/40/48 byte per line)
 ***************************************************************/
#define MODE4(s) COPY4(dst, antic.pf_x10b[video->data[s]])

ANTIC_RENDERER( antic_mode_4_32 )
{
	PREPARE_TXT45(space, 32,0);
	REP32(MODE4);
	POST_TXT(32);
}
ANTIC_RENDERER( antic_mode_4_40 )
{
	PREPARE_TXT45(space, 40,0);
	REP40(MODE4);
	POST_TXT(40);
}
ANTIC_RENDERER( antic_mode_4_48 )
{
	PREPARE_TXT45(space, 48,0);
	REP48(MODE4);
	POST_TXT(48);
}

/*************  ANTIC mode 05: *********************************
 * character mode 8x16:4 multi color (32/40/48 byte per line)
 ***************************************************************/
#define MODE5(s) COPY4(dst, antic.pf_x10b[video->data[s]])

ANTIC_RENDERER( antic_mode_5_32 )
{
	PREPARE_TXT45(space, 32,1);
	REP32(MODE5);
	POST_TXT(32);
}
ANTIC_RENDERER( antic_mode_5_40 )
{
	PREPARE_TXT45(space, 40,1);
	REP40(MODE5);
	POST_TXT(40);
}
ANTIC_RENDERER( antic_mode_5_48 )
{
	PREPARE_TXT45(space, 48,1);
	REP48(MODE5);
	POST_TXT(48);
}

/*************  ANTIC mode 06: *********************************
 * character mode 16x8:5 single color (16/20/24 byte per line)
 ***************************************************************/
#define MODE6(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_6_32 )
{
	PREPARE_TXT67(space, 16,0);
	REP16(MODE6);
	POST_TXT(16);
}
ANTIC_RENDERER( antic_mode_6_40 )
{
	PREPARE_TXT67(space, 20,0);
	REP20(MODE6);
	POST_TXT(20);
}
ANTIC_RENDERER( antic_mode_6_48 )
{
	PREPARE_TXT67(space, 24,0);
	REP24(MODE6);
	POST_TXT(24);
}

/*************  ANTIC mode 07: *********************************
 * character mode 16x16:5 single color (16/20/24 byte per line)
 ***************************************************************/
#define MODE7(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_7_32 )
{
	PREPARE_TXT67(space, 16,1);
	REP16(MODE7);
	POST_TXT(16);
}
ANTIC_RENDERER( antic_mode_7_40 )
{
	PREPARE_TXT67(space, 20,1);
	REP20(MODE7);
	POST_TXT(20);
}
ANTIC_RENDERER( antic_mode_7_48 )
{
	PREPARE_TXT67(space, 24,1);
	REP24(MODE7);
	POST_TXT(24);
}

/*************  ANTIC mode 08: *********************************
 * graphics mode 8x8:4 (8/10/12 byte per line)
 ***************************************************************/
#define MODE8(s) COPY16(dst, antic.pf_210b4[video->data[s]],antic.pf_210b4[video->data[s]+1],antic.pf_210b4[video->data[s]+2],antic.pf_210b4[video->data[s]+3])

ANTIC_RENDERER( antic_mode_8_32 )
{
	PREPARE_GFX8(space, 8);
	REP08(MODE8);
	POST_GFX(8);
}
ANTIC_RENDERER( antic_mode_8_40 )
{
	PREPARE_GFX8(space, 10);
	REP10(MODE8);
	POST_GFX(10);
}
ANTIC_RENDERER( antic_mode_8_48 )
{
	PREPARE_GFX8(space, 12);
	REP12(MODE8);
	POST_GFX(12);
}

/*************  ANTIC mode 09: *********************************
 * graphics mode 4x4:2 (8/10/12 byte per line)
 ***************************************************************/
#define MODE9(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_9_32 )
{
	PREPARE_GFX9BC(space, 16);
	REP16(MODE9);
	POST_GFX(16);
}
ANTIC_RENDERER( antic_mode_9_40 )
{
	PREPARE_GFX9BC(space, 20);
	REP20(MODE9);
	POST_GFX(20);
}
ANTIC_RENDERER( antic_mode_9_48 )
{
	PREPARE_GFX9BC(space, 24);
	REP24(MODE9);
	POST_GFX(24);
}

/*************  ANTIC mode 0A: *********************************
 * graphics mode 4x4:4 (16/20/24 byte per line)
 ***************************************************************/
#define MODEA(s) COPY8(dst, antic.pf_210b2[video->data[s]], antic.pf_210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_a_32 )
{
	PREPARE_GFXA(space, 16);
	REP16(MODEA);
	POST_GFX(16);
}
ANTIC_RENDERER( antic_mode_a_40 )
{
	PREPARE_GFXA(space, 20);
	REP20(MODEA);
	POST_GFX(20);
}
ANTIC_RENDERER( antic_mode_a_48 )
{
	PREPARE_GFXA(space, 24);
	REP24(MODEA);
	POST_GFX(24);
}

/*************  ANTIC mode 0B: *********************************
 * graphics mode 2x2:2 (16/20/24 byte per line)
 ***************************************************************/
#define MODEB(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_b_32 )
{
	PREPARE_GFX9BC(space, 16);
	REP16(MODEB);
	POST_GFX(16);
}
ANTIC_RENDERER( antic_mode_b_40 )
{
	PREPARE_GFX9BC(space, 20);
	REP20(MODEB);
	POST_GFX(20);
}
ANTIC_RENDERER( antic_mode_b_48 )
{
	PREPARE_GFX9BC(space, 24);
	REP24(MODEB);
	POST_GFX(24);
}

/*************  ANTIC mode 0C: *********************************
 * graphics mode 2x1:2 (16/20/24 byte per line)
 ***************************************************************/
#define MODEC(s) COPY8(dst, antic.pf_3210b2[video->data[s]], antic.pf_3210b2[video->data[s]+1])

ANTIC_RENDERER( antic_mode_c_32 )
{
	PREPARE_GFX9BC(space, 16);
	REP16(MODEC);
	POST_GFX(16);
}
ANTIC_RENDERER( antic_mode_c_40 )
{
	PREPARE_GFX9BC(space, 20);
	REP20(MODEC);
	POST_GFX(20);
}
ANTIC_RENDERER( antic_mode_c_48 )
{
	PREPARE_GFX9BC(space, 24);
	REP24(MODEC);
	POST_GFX(24);
}

/*************  ANTIC mode 0D: *********************************
 * graphics mode 2x2:4 (32/40/48 byte per line)
 ***************************************************************/
#define MODED(s) COPY4(dst, antic.pf_x10b[video->data[s]])

ANTIC_RENDERER( antic_mode_d_32 )
{
	PREPARE_GFXDE(space, 32);
	REP32(MODED);
	POST_GFX(32);
}
ANTIC_RENDERER( antic_mode_d_40 )
{
	PREPARE_GFXDE(space, 40);
	REP40(MODED);
	POST_GFX(40);
}
ANTIC_RENDERER( antic_mode_d_48 )
{
	PREPARE_GFXDE(space, 48);
	REP48(MODED);
	POST_GFX(48);
}

/*************  ANTIC mode 0E: *********************************
 * graphics mode 2x1:4 (32/40/48 byte per line)
 ***************************************************************/
#define MODEE(s) COPY4(dst, antic.pf_x10b[video->data[s]])

ANTIC_RENDERER( antic_mode_e_32 )
{
	PREPARE_GFXDE(space, 32);
	REP32(MODEE);
	POST_GFX(32);
}
ANTIC_RENDERER( antic_mode_e_40 )
{
	PREPARE_GFXDE(space, 40);
	REP40(MODEE);
	POST_GFX(40);
}
ANTIC_RENDERER( antic_mode_e_48 )
{
	PREPARE_GFXDE(space, 48);
	REP48(MODEE);
	POST_GFX(48);
}

/*************  ANTIC mode 0F: *********************************
 * graphics mode 1x1:2 (32/40/48 byte per line)
 ***************************************************************/
#define MODEF(s) COPY4(dst, antic.pf_1b[video->data[s]])

ANTIC_RENDERER( antic_mode_f_32 )
{
	PREPARE_GFXF(space, 32);
	REP32(MODEF);
	POST_GFX(32);
}
ANTIC_RENDERER( antic_mode_f_40 )
{
	PREPARE_GFXF(space, 40);
	REP40(MODEF);
	POST_GFX(40);
}
ANTIC_RENDERER( antic_mode_f_48 )
{
	PREPARE_GFXF(space, 48);
	REP48(MODEF);
	POST_GFX(48);
}

/*************  ANTIC mode 0F : GTIA mode 1 ********************
 * graphics mode 8x1:16 (32/40/48 byte per line)
 ***************************************************************/
#define GTIA1(s) COPY4(dst, antic.pf_gtia1[video->data[s]])

ANTIC_RENDERER( gtia_mode_1_32 )
{
	PREPARE_GFXG1(space, 32);
	REP32(GTIA1);
	POST_GFX(32);
}
ANTIC_RENDERER( gtia_mode_1_40 )
{
	PREPARE_GFXG1(space, 40);
	REP40(GTIA1);
	POST_GFX(40);
}
ANTIC_RENDERER( gtia_mode_1_48 )
{
	PREPARE_GFXG1(space, 48);
	REP48(GTIA1);
	POST_GFX(48);
}

/*************  ANTIC mode 0F : GTIA mode 2 ********************
 * graphics mode 8x1:16 (32/40/48 byte per line)
 ***************************************************************/
#define GTIA2(s) COPY4(dst, antic.pf_gtia2[video->data[s]])

ANTIC_RENDERER( gtia_mode_2_32 )
{
	PREPARE_GFXG2(space, 32);
	REP32(GTIA2);
	POST_GFX(32);
}
ANTIC_RENDERER( gtia_mode_2_40 )
{
	PREPARE_GFXG2(space, 40);
	REP40(GTIA2);
	POST_GFX(40);
}
ANTIC_RENDERER( gtia_mode_2_48 )
{
	PREPARE_GFXG2(space, 48);
	REP48(GTIA2);
	POST_GFX(48);
}

/*************  ANTIC mode 0F : GTIA mode 3 ********************
 * graphics mode 8x1:16 (32/40/48 byte per line)
 ***************************************************************/
#define GTIA3(s) COPY4(dst, antic.pf_gtia3[video->data[s]])

ANTIC_RENDERER( gtia_mode_3_32 )
{
	PREPARE_GFXG3(space, 32);
	REP32(GTIA3);
	POST_GFX(32);
}
ANTIC_RENDERER( gtia_mode_3_40 )
{
	PREPARE_GFXG3(space, 40);
	REP40(GTIA3);
	POST_GFX(40);
}
ANTIC_RENDERER( gtia_mode_3_48 )
{
	PREPARE_GFXG3(space, 48);
	REP48(GTIA3);
	POST_GFX(48);
}
