/**********************************************************************************


    SNOOKER 10 / SANDII'

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Snooker 10 (Ver 1.11), Sandii', 1998.
    * Apple 10 (Ver 1.21),   Sandii', 1998.
    * Ten Balls (Ver 1.05),  unknown, 1997.


***********************************************************************************


    The hardware is generally composed by:


    CPU:    1x 65SC02 at 2MHz.

    Sound:  1x AD-65 or U6295 (OKI6295 compatible)
            1x LM358N
            1x TDA2003

    HD-PLD: 2x AMD MACH231-15-JC/1-18JI/1
            (2x Lattice ispLSI1024-60LJ for earlier revisions)

    RAM:    1x 76C88AL-15, SRAM 8Kx8
    NVRAM:  1x 76C88AL-15, SRAM 8Kx8 (battery backed)
    ROMs:   4x 27C256
            (3x 27C256 for earlier revisions)
            1x 27C020

    PROMs:  1x 82S147 or similar. (512 bytes)

    Clock:  1x Crystal: 16MHz.

    Other:  1x 28x2 edge connector.
            1x 15 legs connector.
            1x trimmer (volume).
            1x 8 DIP switches.
            1x 3.5 Volt, 55-80 mAh battery.


***************************************************************************************


    All the supported games have been coded using some Funworld games as point to start,
    changing hardware accesses, program logics, graphics, plus protection and some I/O
    through the 2x high density PLDs.

    Color palettes are normally stored in format GGBBBRRR inside a bipolar color PROM
    (old hardware), or repeated 64 times inside a regular 27c256 ROM (new hardware).

    - bits -
    7654 3210
    ---- -xxx   Red component.
    --xx x---   Blue component.
    xx-- ----   Green component.

    Same as Funworld video hardware, this one was designed to manage 4096 tiles with a
    size of 8x4 pixels each. Also support 4bpp graphics and the palette limitation is
    8 bits for color codes (256 x 16 colors). It means the hardware was designed for more
    elaborated graphics than these games...

    Color PROMs from current games are 512 bytes lenght, but they only use the first 256 bytes.

    The sound is composed by 4-bit ADPCM samples. All the supported games have the same sound ROM.
    All the sounds/samples were ripped from the Gottlieb pinball 'Cue Ball Wizard'(1992).


    ***** Game Notes *****

    To initialize NVRAM (for all games), keep pressed ESTATISTICA (key '9') + MANAGEMENT (key '0'),
    then press RESET (key F3), and finally release both (ESTATISTICA + MANAGEMENT) keys.

    Enter ESTATISTICA (key '9'), to enter the stats mode. Press CANCELLA/PLAY (key 'N')
    for 5 seconds to reset all values to zero. Press START (key '1') to exit the mode.

    Enter MANAGEMENT (key '0'), to enter the management mode. Press CANCELLA/PLAY (key 'N')
    to access the PROGRAMAZZIONE (program mode), where you can change the game parameters.
    Press START (key '1') to exit both modes.

    To clear credits (and stats), just re-initialize the NVRAM.

    Only for the new hardware revision (snookr10 & apple10), pressing STOP 1 (key 'Z') in the
    stats mode, make the hidden Input Test mode to appear. Press RESET (F3) to exit the mode.


    --- Super Game ---

    If you have some points accumulated and need to grab the tokens/tickets, you must to play
    a bonus game called SUPER GAME to get the points out. To enter the bonus game, you must
    press STOP5 in the attract mode. The payout system is through this game.

    5 themed items will be shown (apples, balls, etc... depending of the game).
    The joker will start to move from item to item quickly, but decreasing the speed gradually.
    To beat the game, you need to push the start button in the exact moment when the joker is
    located exactly in the center of the screen (item 3).

    Depending of the DIP switches settings, you can grab the prize manually pressing the SCARICA
    (payout) button, and then TICKET or HOPPER buttons. Press TICKET button to print a 100 points
    ticket. Press HOPPER button to get tokens x10 points.

    You have 1 attempt for each 100 earned points. If you lose the game, you lose the points.



    ***** Issues / Protection *****

    * Apple 10

    - Tile matrix and color palette are totally encrypted/scrambled.

    You can see the following table, where 'Normal tile #' is the tile number called to be drawn, and
    'Scrambled tile #' is the phisical tile position in the matrix:

    Normal | Scrambled
    tile # |  tile #  
    -------+----------
     0x00  |   0x00   \
     0x01  |   0x80    |
     0x02  |   0x40    | Big "0"
     0x03  |   0xC0    |
     0x04  |   0x20    |
     0x05  |   0xA0   /
    -------+----------
     0x06  |   0x60   \
     0x07  |   0xE0    |
     0x08  |   0x10    | Big "1"
     0x09  |   0x90    |
     0x0A  |   0x50    |
     0x0B  |   0xD0   /
    -------+----------
     0x0C  |   0x30   \
     0x0D  |   0xB0    |
     0x0E  |   0x70    | Big "2"
     0x0F  |   0xF0    |
     0x10  |   0x08    |
     0x11  |   0x88   /
    -------+----------
     0x12  |   0x48   \
     0x13  |   0xC8    |
     0x14  |   0x28    | Big "3"
     0x15  |   0xA8    |
     0x16  |   0x68    |
     0x17  |   0xE8   /
    -------+----------
     0x18  |   0x18   \
     0x19  |   0x98    |
     0x1A  |   0x58    | Big "4"
     0x1B  |   0xD8    |
     0x1C  |   0x38    |
     0x1D  |   0xB8   /
    -------+----------
     0x1E  |   0x78   \
     0x1F  |   0xF8    |
     0x20  |   0x04    | Big "5"
     0x21  |   0x84    |
     0x22  |   0x44    |
     0x23  |   0xC4   /
    -------+----------
     0x24  |   0x24   \
     0x25  |   0xA4    |
     0x26  |   0x64    | Big "6"
     0x27  |   0xE4    |
     0x28  |   0x14    |
     0x29  |   0x94   /
    -------+----------
     0x2A  |   0x54   \
     0x2B  |   0xD4    |
     0x2C  |   0x34    | Big "7"
     0x2D  |   0xB4    |
     0x2E  |   0x74    |
     0x2F  |   0xF4   /
    -------+----------

    So we extract the following decryption table: 

    0 <-> 0;  1 <-> 8;  2 <-> 4;  3 <-> C 
    4 <-> 2;  5 <-> A;  6 <-> 6;  7 <-> E 
    8 <-> 1;  9 <-> 9;  A <-> 5;  B <-> D 
    C <-> 3;  D <-> B;  E <-> 7;  F <-> F 

    ...and then swap nibbles.

    Also note that the values are inverted/mirrored bits of the original ones. 

    0x01 (0001) <-> 0x08 (1000) 
    0x02 (0010) <-> 0x04 (0100) 
    0x03 (0011) <-> 0x0C (1100) 
    0x04 (0100) <-> 0x04 (0010) 
    0x05 (0101) <-> 0x0A (1010) 
    ...and so on.

    To properly decrypt the thing 'on the fly' as the hardware does, I applied a bitswap into TILE_GET_INFO.
    This method rearrange the tile number for each tile called to be drawn.

    The final algorhithm:
                                                                   digit #3
                                                                   +-------+ swapped digits 1 & 2
                                                                   |       |  +-------+------+
        tile_offset = BITSWAP16((tile_offset & 0xfff),15,14,13,12, 8,9,10,11, 0,1,2,3, 4,5,6,7)
                                                                   | | |  |   | | | | || | | | 
                                                                   inverted   inverted|inverted
                                                                   bitorder   bitorder|bitorder

    Colors are scrambled in the following way: 

      Normal   |  Scrambled
      offset   |   offset
   ------------+------------
    0x00-0x0F  |  0x00-0x0F
    0x10-0x1F  |  0x80-0x8F
    0x20-0x2F  |  0x40-0x4F
    0x30-0x3F  |  0xC0-0xCF
    0x40-0x4F  |  0x20-0x2F
    0x50-0x5F  |  0xA0-0xAF
    0x60-0x6F  |  0x60-0x6F
    0x70-0x7F  |  0xE0-0xEF
   ------------+------------
    0x80-0x8F  |  0x10-0x1F
    0x90-0x9F  |  0x90-0x9F
    0xA0-0xAF  |  0x50-0x5F
    0xB0-0xBF  |  0xD0-0xDF
    0xC0-0xCF  |  0x30-0x3F
    0xD0-0xDF  |  0xB0-0xBF
    0xE0-0xEF  |  0x70-0x7F
    0xF0-0xFF  |  0xF0-0xFF


    So, the algorhythm to properly decrypt the color codes is very different here:

                                          1st nibble
                                      inverted bitorder
                                           | | | |
        color_index = BITSWAP8(color_index,4,5,6,7,2,3,0,1)
                                                   <-> <->
                                                  2nd nibble
                                                swappeed pairs

    Scary, huh?... ;)


***********************************************************************************


    Memory Maps
    -----------

    (Old hardware)

    $0000 - $0FFF   NVRAM          ;R/W, all registers and settings.
    $1000 - $1000   OKI6295        ;R/W, sound.
    $4000 - $4000   Input Port 0   ;R
    $4001 - $4001   Input Port 1   ;R
    $4002 - $4002   Input Port 2   ;R
    $4003 - $4003   Input Port 3   ;R  , DIP switches.
    $5000 - $5000   Output Port A  ;W  , lamps.
    $5001 - $5001   Output Port B  ;W  , lamps.
    $6000 - $6FFF   VideoRAM
    $7000 - $7FFF   ColorRAM
    $8000 - $FFFF   ROM Space


    (New hardware)

    $0000 - $0FFF   NVRAM          ;R/W, all registers and settings.
    $1000 - $1000   OKI6295        ;R/W, sound.
    $3000 - $3000   Input Port 0   ;R
    $3001 - $3001   Input Port 1   ;R
    $3002 - $3002   Input Port 2   ;R
    $3003 - $3003   Input Port 3   ;R  , DIP switches.
    $3004 - $3004   ???            ;R  , the code is constantly polling bit 7.
    $5000 - $5000   Output Port A  ;W  , lamps.
    $5001 - $5001   Output Port B  ;W  , lamps.
    $6000 - $6FFF   VideoRAM
    $7000 - $7FFF   ColorRAM
    $8000 - $FFFF   ROM Space


***********************************************************************************


    *** Driver Updates ***


    [2008/04/28]
    - Created a new machine driver for tenballs due to different memory map.
    - Worked all the input ports from the scratch.
    - Fixed the sound ROM_REGION.
    - Added the oki6295 emulation to all games.
    - Hooked output ports.
    - Documented and calculated all bits related to lamps.
    - Adjusted palette lenght to 256 colors.
    - Totally decrypted the apple10 color matrix. Now colors are perfect.
    - Created a new machine driver for apple10 due to encryption.
    - Reverse engineering the code to complete the DIP switches.
    - Added diplocations to DIP switches.
    - Promoted snookr10, apple10 and tenballs to 'WORKING' state.
    - Added game instructions & notes.
    - Updated encryption & technical notes.

    [2008/04/24]
    - Decrypted the apple10 tile matrix.
    - Partially decrypted the apple10 color codes.
    - Added encryption notes.
    - Updated technical notes.

    [2008/04/18]
    - Initial release. Support for snookr10, apple10 and tenballs.
    - Added technical/general notes.


    *** TO DO ***

    - Figure out the unknown reads to offset $3004, bit 7.
    - Get CPU and sound clocks properly measured.
    - Get the OKI6295 pin 7 measured to know the real status.
    - Hook lamps.


***********************************************************************************/


#define MASTER_CLOCK	XTAL_16MHz

#include "driver.h"
#include "sound/okim6295.h"


/* from video */
WRITE8_HANDLER( snookr10_videoram_w );
WRITE8_HANDLER( snookr10_colorram_w );
PALETTE_INIT( snookr10 );
PALETTE_INIT( apple10 );
VIDEO_START( snookr10 );
VIDEO_START( apple10 );
VIDEO_UPDATE( snookr10 );


/**********************
* Read/Write Handlers *
*  - Output Ports -   *
***********************

    LAMPS components:

    START  = bit1 & bit3
    CANCEL = bit0 & bit3
    STOP1  = bit1
    STOP2  = bit1 & bit2
    STOP3  = bit1
    STOP4  = bit1 & bit4
    STOP5  = bit0 & bit4
*/

static WRITE8_HANDLER( output_port_0_w )
/*-----------------------------
    PORT 0x5000 ;OUTPUT PORT A
-------------------------------
    BIT 0 = 
    BIT 1 = Lamps, bit0
    BIT 2 =
    BIT 3 = Lamps, bit1
    BIT 4 =
    BIT 5 = Lamps, bit2
    BIT 6 =
    BIT 7 = Lamps, bit3
------------------------------*/
{
//	(data >> 1) & 1;	/* lamps, bit0 */
//	(data >> 3) & 1;	/* lamps, bit1 */
//	(data >> 5) & 1;	/* lamps, bit2 */
//	(data >> 7) & 1;	/* lamps, bit3 */
}

static WRITE8_HANDLER( output_port_1_w )
/*-----------------------------
    PORT 0x5001 ;OUTPUT PORT B
-------------------------------
    BIT 0 = Lamps, bit4
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 =
    BIT 5 =
    BIT 6 =
    BIT 7 =
------------------------------*/
{
//	(data & 1);			/* lamps, bit4 */
}


/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( snookr10_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x1000, 0x1000) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x3000, 0x3000) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x3001, 0x3001) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x3002, 0x3002) AM_READ(input_port_2_r)	/* IN2 */
	AM_RANGE(0x3003, 0x3003) AM_READ(input_port_3_r)	/* DS1 */
	AM_RANGE(0x3004, 0x3004) AM_READNOP /* unknown reads comparing bit 7 */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(output_port_0_w)	/* OUT0 */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(output_port_1_w)	/* OUT1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM	AM_WRITE(snookr10_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tenballs_map, ADDRESS_SPACE_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x0fff) AM_RAM AM_BASE(&generic_nvram) AM_SIZE(&generic_nvram_size)
	AM_RANGE(0x1000, 0x1000) AM_READWRITE(OKIM6295_status_0_r, OKIM6295_data_0_w)
	AM_RANGE(0x4000, 0x4000) AM_READ(input_port_0_r)	/* IN0 */
	AM_RANGE(0x4001, 0x4001) AM_READ(input_port_1_r)	/* IN1 */
	AM_RANGE(0x4002, 0x4002) AM_READ(input_port_2_r)	/* IN2 */
	AM_RANGE(0x4003, 0x4003) AM_READ(input_port_3_r)	/* DS1 */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(output_port_0_w)	/* OUT0 */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(output_port_1_w)	/* OUT1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_BASE(&videoram)
	AM_RANGE(0x7000, 0x7fff) AM_RAM	AM_WRITE(snookr10_colorram_w) AM_BASE(&colorram)
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

/*  Do the stops & cancel buttons need PORT_IMPULSE
    to just trigger only 1 sound each time they are pressed? */

static INPUT_PORTS_START( snookr10 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Remote x100") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )	PORT_IMPULSE(2) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_Z)	/* Input Test in stats mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )	PORT_IMPULSE(2) PORT_NAME("Cancella (Cancel) / Play / Bet") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )		PORT_NAME("Start (Deal) / Raddoppio (Double-Up)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )	PORT_IMPULSE(2) PORT_NAME("Stop 5 / Risk (Half Gamble) / Super Game") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	PORT_NAME("Estatistica (Stats)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )	PORT_NAME("Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )	PORT_IMPULSE(2) PORT_NAME("Stop 4 / Alta (High)") PORT_CODE(KEYCODE_V)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )	PORT_IMPULSE(2) PORT_NAME("Stop 2 / Bassa (Low)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )	PORT_IMPULSE(2) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 )	PORT_NAME("Scarica (Payout)") PORT_CODE(KEYCODE_M)

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )				PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Super Game Settings" )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "Play to Payout" )
	PORT_DIPSETTING(    0x00, "Direct Payout" )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )	PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Automatic" )
	PORT_DIPNAME( 0x80, 0x80, "Super Game" )				PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( apple10 )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Remote x100") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )	PORT_IMPULSE(2) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_Z)	/* Input Test in stats mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )	PORT_IMPULSE(2) PORT_NAME("Cancella (Cancel) / Play / Bet") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )		PORT_NAME("Start (Deal) / Raddoppio (Double-Up)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )	PORT_IMPULSE(2) PORT_NAME("Stop 5 / Risk (Half Gamble) / Super Game") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	PORT_NAME("Estatistica (Stats)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )	PORT_NAME("Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )	PORT_IMPULSE(2) PORT_NAME("Stop 4 / Alta (High)") PORT_CODE(KEYCODE_V)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )	PORT_IMPULSE(2) PORT_NAME("Stop 2 / Bassa (Low)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )	PORT_IMPULSE(2) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 )	PORT_NAME("Scarica (Payout)") PORT_CODE(KEYCODE_M)

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START_TAG("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )				PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )			PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Super Game Settings" )		PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "Play to Payout" )
	PORT_DIPSETTING(    0x00, "Direct Payout" )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )	PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - Coins 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins 2" )
	PORT_DIPSETTING(    0x40, "Disable Payment/Game" )
	PORT_DIPSETTING(    0x60, "Automatic" )
	PORT_DIPNAME( 0x80, 0x80, "Super Game" )				PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( tenballs )
	PORT_START_TAG("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Remote x100") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON1 )	PORT_IMPULSE(2) PORT_NAME("Stop 1") PORT_CODE(KEYCODE_Z)	/* no Input Test in stats mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON6 )	PORT_IMPULSE(2) PORT_NAME("Cancella (Cancel) / Play / Bet") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )		PORT_NAME("Start (Deal) / Raddoppio (Double-Up)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON5 )	PORT_IMPULSE(2) PORT_NAME("Stop 5 / Risk (Half Gamble) / Super Game") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )	PORT_NAME("Estatistica (Stats)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 )	PORT_NAME("Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON4 )	PORT_IMPULSE(2) PORT_NAME("Stop 4 / Alta (High)") PORT_CODE(KEYCODE_V)

	PORT_START_TAG("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2 )	PORT_IMPULSE(2) PORT_NAME("Stop 2 / Bassa (Low)") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON3 )	PORT_IMPULSE(2) PORT_NAME("Stop 3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE )	PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON7 )	PORT_NAME("Scarica (Payout)") PORT_CODE(KEYCODE_M)

	PORT_START_TAG("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

    /* tenballs seems a prototype, most DIP
       switches seems to do nothing at all.
    */ 
	PORT_START_TAG("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )		PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
    /* coinage is always 1 coin - 10 credits */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    /* always play Super Game to payout */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    /* always manual payout */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
    /* Super Game always ON */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )	PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( snookr10 )
	GFXDECODE_ENTRY( REGION_GFX1, 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_DRIVER_START( snookr10 )

    /* basic machine hardware */
	MDRV_CPU_ADD_TAG("main", M65SC02, MASTER_CLOCK/8)	/* 2MHz */
	MDRV_CPU_PROGRAM_MAP(snookr10_map, 0)
	MDRV_CPU_VBLANK_INT("main", nmi_line_pulse)

	MDRV_NVRAM_HANDLER(generic_0fill)

    /* video hardware */

	MDRV_SCREEN_ADD("main", RASTER)
	MDRV_SCREEN_REFRESH_RATE(60)
	MDRV_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MDRV_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MDRV_SCREEN_SIZE((128+1)*4, (30+1)*8)
	MDRV_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 30*8-1)

	MDRV_GFXDECODE(snookr10)

	MDRV_PALETTE_LENGTH(0x100)
	MDRV_PALETTE_INIT(snookr10)
	MDRV_VIDEO_START(snookr10)
	MDRV_VIDEO_UPDATE(snookr10)

	/* sound hardware */
	MDRV_SPEAKER_STANDARD_MONO("mono")
	MDRV_SOUND_ADD(OKIM6295, MASTER_CLOCK/12)	/* guess */
	MDRV_SOUND_CONFIG(okim6295_interface_region_1_pin7low)	/* pin7 not checked */
	MDRV_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.8)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( apple10 )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(snookr10)
	MDRV_CPU_MODIFY("main")

    /* video hardware */
	MDRV_PALETTE_INIT(apple10)
	MDRV_VIDEO_START(apple10)

MACHINE_DRIVER_END

static MACHINE_DRIVER_START( tenballs )

	/* basic machine hardware */
	MDRV_IMPORT_FROM(snookr10)
	MDRV_CPU_MODIFY("main")
	MDRV_CPU_PROGRAM_MAP(tenballs_map, 0)

MACHINE_DRIVER_END


/*************************
*        Rom Load        *
*************************/

ROM_START( snookr10 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(216ccb2d) SHA1(d86270cd03a08f6fd3e7b327b8173f66da28e5e8) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(a70d9c48) SHA1(3fa90190323526553866662afda4dbe1c94abeff) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(3009faaa) SHA1(d1cda455b270cb9afa65b9701735a3a1f2a48df2) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "4.u18", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

    /* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, REGION_PROMS, 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(f3d7d640) SHA1(f78060f4603e316fa3c2ec4ba6d7edf261cf6d8a) )
ROM_END

ROM_START( apple10 )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(7d538566) SHA1(2e805157010c366ab1f2313a2bedb071c1dde733) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(42b016f4) SHA1(59d1b77f8cb706a3878813111c6a71514c413784) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(afc535dc) SHA1(ed2d65f3154c6d80b7b22bfef1f30232e4496128) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "4.u18", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

    /* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, REGION_PROMS, 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(3510d705) SHA1(2190c8199d29bf89e3007eb771cc6b0e2b58f6cd) )
ROM_END

ROM_START( tenballs )
	ROM_REGION( 0x10000, REGION_CPU1, 0 )
	ROM_LOAD( "4.u2", 0x8000, 0x8000, CRC(2f334862) SHA1(61d57995451b6bc7de23900c460c3e073993899c) )

	ROM_REGION( 0x10000, REGION_GFX1, ROMREGION_DISPOSE )
	ROM_LOAD( "3.u16", 0x0000, 0x8000, CRC(9eb88a08) SHA1(ab52924103e2b14c598a21c3d77b053da37a0212) )
	ROM_LOAD( "2.u15", 0x8000, 0x8000, CRC(a5091583) SHA1(c0775d9b77cb634d3702b6c08cdf73c867b6169a) )

	ROM_REGION( 0x40000, REGION_SOUND1, 0 )	/* ADPCM samples */
	ROM_LOAD( "1.u28", 0x00000, 0x40000 , CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x0200, REGION_PROMS, 0 )
	ROM_LOAD( "82s147.u17", 0x0000, 0x0200, CRC(20234dcc) SHA1(197937bbec0201888467e250bdba49e39aa4204a) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT    MACHINE   INPUT     INIT ROT    COMPANY    FULLNAME                FLAGS  */
GAME( 1998, snookr10, 0,        snookr10, snookr10, 0,   ROT0, "Sandii'", "Snooker 10 (Ver 1.11)", 0 )
GAME( 1998, apple10,  0,        apple10,  apple10,  0,   ROT0, "Sandii'", "Apple 10 (Ver 1.21)",   0 )
GAME( 1997, tenballs, snookr10, tenballs, tenballs, 0,   ROT0, "unknown", "Ten Balls (Ver 1.05)",  0 )
