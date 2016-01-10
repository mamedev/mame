// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/**********************************************************************************


    SNOOKER 10 / SANDII'

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Snooker 10 (Ver 1.11),       Sandii', 1998.
    * Apple 10 (Ver 1.21),         Sandii', 1998.
    * Ten Balls (Ver 1.05),        unknown, 1997.
    * Crystals Colours (Ver 1.01), JCD srl, 1998.


***********************************************************************************


    The hardware is generally composed by:


    CPU:    1x 65SC02 at 2MHz.

    Sound:  1x AD-65 or U6295 (OKI6295 compatible) at 1MHz, pin7 HIGH.
            1x LM358N
            1x TDA2003

    HD-PLD: 2x AMD MACH231-15-JC/1-18JI/1     or...
            2x XILINX XC9572 - PO100ASJ9745   or...
            2x Lattice ispLSI1024-60LJ for earlier revisions

    RAM:    1x 76C88AL-15, SRAM 8Kx8
    NVRAM:  1x 76C88AL-15, SRAM 8Kx8 (battery backed)
    ROMs:   4x 27C256
            (3x 27C256 for earlier revisions)
            1x 27C020

    PROMs:  1x 82S147 or similar. (512 bytes)

    Clock:  1x Crystal: 16MHz.

    Other:  1x 28x2 edge connector.
            1x 15-pins connector.
            1x trimmer (volume).
            1x 8 DIP switches.
            1x 3.5 Volt, 55-80 mAh battery.


***************************************************************************************


    All supported games have been coded using some italian C.M.C games as point to start,
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

    Enter ESTATISTICA (key '9'), to enter the stats mode. Press PLAY/CANCELLA (key 'N')
    for 5 seconds to reset all values to zero. Press START (key '1') to exit the mode.

    Enter MANAGEMENT (key '0'), to enter the management mode. Press PLAY/CANCELLA (key 'N')
    to access the PROGRAMAZZIONE (program mode), where you can change the game parameters.
    Press START (key '1') to exit both modes.

    To clear credits (and stats), just re-initialize the NVRAM.

    Only for the new hardware revision (snookr10 & apple10), pressing STOP 1 (key 'Z') into the
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


    NOTE: Bit 7 of input port 0x3004 is tied to bit 7 of input port 0x3003 (DIP switch 1).
          This allow to use the PAYOUT button to trigger the Supper Game instead of STOP 5.


***************************************************************************************


    Encryption schemes...
    ---------------------

    * Apple 10

    - Tile matrix and color data (video & color RAM) are totally encrypted/scrambled.

    You can see the following table, where 'Normal tile #' is the tile number called
    to be drawn, and 'Scrambled tile #' is the phisical tile position in the matrix:

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

    The final algorithm is:
                                                                 digit #3
                                                                 +-------+ swapped digits 1 & 2
                                                                 |       |  +-------+------+
      tile_offset = BITSWAP16((tile_offset & 0xfff),15,14,13,12, 8,9,10,11, 0,1,2,3, 4,5,6,7)
                                                                 | | |  |   | | | | || | | |
                                                                 inverted   inverted|inverted
                                                                 bitorder   bitorder|bitorder

    Colors are scrambled in the following way:

      Normal  | Scrambled
      offset  |  offset
    ----------+-----------
    0x00-0x0F | 0x00-0x0F
    0x10-0x1F | 0x80-0x8F
    0x20-0x2F | 0x40-0x4F
    0x30-0x3F | 0xC0-0xCF
    0x40-0x4F | 0x20-0x2F
    0x50-0x5F | 0xA0-0xAF
    0x60-0x6F | 0x60-0x6F
    0x70-0x7F | 0xE0-0xEF
    ----------+-----------
    0x80-0x8F | 0x10-0x1F
    0x90-0x9F | 0x90-0x9F
    0xA0-0xAF | 0x50-0x5F
    0xB0-0xBF | 0xD0-0xDF
    0xC0-0xCF | 0x30-0x3F
    0xD0-0xDF | 0xB0-0xBF
    0xE0-0xEF | 0x70-0x7F
    0xF0-0xFF | 0xF0-0xFF

    And each one of these segments are internally scrambled too.

    So, the algorithm to properly decrypt the color codes is very different here:

                                        1st nibble
                                    inverted bitorder
                                         | | | |
      color_index = BITSWAP8(color_index,4,5,6,7,2,3,0,1)
                                                 <-> <->
                                                2nd nibble
                                              swappeed pairs

    Scary, huh?... ;-)


-----------------------------------------------------------------------------------

    * Crystals Color

    - Tile matrix and color data (video & color RAM) are totally encrypted/scrambled.

    You can see the following table, where 'Normal tile #' is the tile number called
    to be drawn, and 'Scrambled tile #' is the phisical tile position in the matrix:

    Normal | Scrambled
    tile # |  tile #
    -------+-----------
     0x00  |   0x000   \
     0x01  |   0x800    |
     0x02  |   0x100    | Big "0"
     0x03  |   0x900    |
     0x04  |   0x004    |
     0x05  |   0x804   /
    -------+-----------
     0x06  |   0x104   \
     0x07  |   0x904    |
     0x08  |   0x008    | Big "1"
     0x09  |   0x808    |
     0x0A  |   0x108    |
     0x0B  |   0x908   /
    -------+-----------
     0x0C  |   0x00C   \
     0x0D  |   0x80C    |
     0x0E  |   0x10C    | Big "2"
     0x0F  |   0x90C    |
     0x10  |   0x010    |
     0x11  |   0x810   /
    -------+-----------
     0x12  |   0x110   \
     0x13  |   0x910    |
     0x14  |   0x014    | Big "3"
     0x15  |   0x814    |
     0x16  |   0x114    |
     0x17  |   0x914   /
    -------+-----------
     0x18  |   0x018   \
     0x19  |   0x818    |
     0x1A  |   0x118    | Big "4"
     0x1B  |   0x918    |
     0x1C  |   0x01C    |
     0x1D  |   0x81C   /
    -------+-----------
     0x1E  |   0x11C   \
     0x1F  |   0x91C    |
     0x20  |   0x200    | Big "5"
     0x21  |   0xA00    |
     0x22  |   0x300    |
     0x23  |   0xB00   /
    -------+-----------
     0x24  |   0x204   \
     0x25  |   0xA04    |
     0x26  |   0x304    | Big "6"
     0x27  |   0xB04    |
     0x28  |   0x208    |
     0x29  |   0xA08   /
    -------+-----------
     0x2A  |   0x308   \
     0x2B  |   0xB08    |
     0x2C  |   0x20C    | Big "7"
     0x2D  |   0xA0C    |
     0x2E  |   0x30C    |
     0x2F  |   0xB0C   /
    -------+-----------


    Will see some binary patterns...


    TILE    NORMAL POS     SCRAMBLED POS
    -----+---------------+---------------
     00:  0000 0000 0000 : 0000 0000 0000
     01:  0000 0000 0001 : 1000 0000 0000
     02:  0000 0000 0010 : 0001 0000 0000
     03:  0000 0000 0011 : 1001 0000 0000
     04:  0000 0000 0100 : 0000 0000 0100
     05:  0000 0000 0101 : 1000 0000 0100
     06:  0000 0000 0110 : 0001 0000 0100
     07:  0000 0000 0111 : 1001 0000 0100

     08:  0000 0000 1000 : 0000 0000 1000
     09:  0000 0000 1001 : 1000 0000 1000
     0A:  0000 0000 1010 : 0001 0000 1000
     0B:  0000 0000 1011 : 1001 0000 1000
     0C:  0000 0000 1100 : 0000 0000 1100
     0D:  0000 0000 1101 : 1000 0000 1100
     0E:  0000 0000 1110 : 0001 0000 1100
     0F:  0000 0000 1111 : 1001 0000 1100

     10:  0000 0001 0000 : 0000 0001 0000
     11:  0000 0001 0001 : 1000 0001 0000
     12:  0000 0001 0010 : 0001 0001 0000
     13:  0000 0001 0011 : 1001 0001 0000
     14:  0000 0001 0100 : 0000 0001 0100
     15:  0000 0001 0101 : 1000 0001 0100
     16:  0000 0001 0110 : 0001 0001 0100
     17:  0000 0001 0111 : 1001 0001 0100

     18:  0000 0001 1000 : 0000 0001 1000
     19:  0000 0001 1001 : 1000 0001 1000
     1A:  0000 0001 1010 : 0001 0001 1000
     1B:  0000 0001 1011 : 1001 0001 1000
     1C:  0000 0001 1100 : 0000 0001 1100
     1D:  0000 0001 1101 : 1000 0001 1100
     1E:  0000 0001 1110 : 0001 0001 1100
     1F:  0000 0001 1111 : 1001 0001 1100

     20:  0000 0010 0000 : 0010 0000 0000
     21:  0000 0010 0001 : 1010 0000 0000
     22:  0000 0010 0010 : 0011 0000 0000
     23:  0000 0010 0011 : 1011 0000 0000
     24:  0000 0010 0100 : 0010 0000 0100
     25:  0000 0010 0101 : 1010 0000 0100
     26:  0000 0010 0110 : 0011 0000 0100
     27:  0000 0010 0111 : 1011 0000 0100

     28:  0000 0010 1000 : 0010 0000 1000
     29:  0000 0010 1001 : 1010 0000 1000
     2A:  0000 0010 1010 : 0011 0000 1000
     2B:  0000 0010 1011 : 1011 0000 1000
     2C:  0000 0010 1100 : 0010 0000 1100
     2D:  0000 0010 1101 : 1010 0000 1100
     2E:  0000 0010 1110 : 0011 0000 1100
     2F:  0000 0010 1111 : 1011 0000 1100

     40:  0000 0100 0000 : 0000 0100 0000
     80:  0000 1000 0000 : 0000 1000 0000
     C0:  0000 1100 0000 : 0000 1100 0000
    100:  0001 0000 0000 : 0000 0000 0010


    We have enough data to arrange the following bitswap:


    FEDCBA9876543210 -> normal tile index.
        ||||||||||||
        |||||||||||+--> B -
        ||||||||||+---> 8   \
        |||||||||+----> 2    |
        ||||||||+-----> 3    |
        |||||||+------> 4    |
        ||||||+-------> 9    |> FEDC0A517694328B -> encrypted tile index.
        |||||+--------> 6    |
        ||||+---------> 7    |
        |||+----------> 1    |
        ||+-----------> 5    |
        |+------------> A   /
        +-------------> 0 -


    - Color RAM: Colors are scrambled in the following way:

      Normal  | Scrambled
      offset  |  offset
    ----------+-----------
    0x00-0x1F | 0x00-0x1F
    0x20-0x3F | 0x40-0x5F
    0x40-0x5F | 0x20-0x3F
    0x60-0x7F | 0x60-0x7F
    ----------+-----------
    0x80-0x9F | 0x80-0x9F
    0xA0-0xBF | 0xC0-0xDF
    0xC0-0xDF | 0xA0-0xBF
    0xE0-0xFF | 0xE0-0xFF


    The color codes are swapped in blocks of 32 bytes. 0x20<->0x40 and 0xA0<->0xC0,
    giving us the following binary representation:

        Normal     | Scrambled
        offset     | offset
    ---------------+-----------
     00: 0000 0000 : 0000 0000
     20: 0010 0000 : 0100 0000 <-- changed
     40: 0100 0000 : 0010 0000 <-- changed
     60: 0110 0000 : 0110 0000
     80: 1000 0000 : 1000 0000
     A0: 1010 0000 : 1100 0000 <-- changed
     C0: 1100 0000 : 1010 0000 <-- changed
     E0: 1110 0000 : 1110 0000


    So, the algorithm to properly decrypt the color codes is the following one:

      color_index = BITSWAP8(color_index,7,5,6,4,3,2,1,0)
                                           | |
                                         swapped

    Happy end here :)


***********************************************************************************


    * MEMORY MAPS *
    ---------------

    (Old hardware)

    $0000 - $07FF   NVRAM (battery backed 6116) ;R/W, all registers and settings.
    $1000 - $1000   OKI6295        ;R/W, sound.
    $4000 - $4000   Input Port 0   ;R
    $4001 - $4001   Input Port 1   ;R
    $4002 - $4002   Input Port 2   ;R
    $4003 - $4003   Input Port 3   ;R  , DIP switches.
    $5000 - $5000   Output Port 0  ;  W, lamps & counters.
    $5001 - $5001   Output Port 1  ;  W, lamps.
    $6000 - $6FFF   Video RAM
    $7000 - $7FFF   Color RAM
    $8000 - $FFFF   ROM Space


    (New hardware)

    $0000 - $07FF   NVRAM (battery backed 6116) ;R/W, all registers and settings.
    $1000 - $1000   OKI6295        ;R/W, sound.
    $3000 - $3000   Input Port 0   ;R
    $3001 - $3001   Input Port 1   ;R
    $3002 - $3002   Input Port 2   ;R
    $3003 - $3003   Input Port 3   ;R  , DIP switches.
    $3004 - $3004   Input Port 4   ;R  , bit 7 in parallel with DIP switch 1.
    $5000 - $5000   Output Port 0  ;  W, lamps & counters.
    $5001 - $5001   Output Port 1  ;  W, lamps.
    $6000 - $6FFF   Video RAM
    $7000 - $7FFF   Color RAM
    $8000 - $FFFF   ROM Space


    (Crystals hardware)

    $0000 - $07FF   NVRAM (battery backed 6116) ;R/W, all registers and settings.
    $1000 - $1000   Output Port 0  ;  W, lamps & counters.
    $1001 - $1001   Output Port 1  ;  W, lamps.
    $2000 - $2008   Special Input  ;R  , special ports meant for data channels...
    $3000 - $3000   Input Port 0   ;R
    $3001 - $3001   Input Port 1   ;R
    $3002 - $3002   Input Port 2   ;R
    $3003 - $3003   Input Port 3   ;R  , DIP switches.
    $5000 - $5000   OKI6295        ;R/W, sound.
    $6000 - $6FFF   Video RAM
    $7000 - $7FFF   Color RAM
    $8000 - $FFFF   ROM Space


***********************************************************************************


    *** Driver Updates ***


    [2013/05/12]

    Crystals Colours:
    - Decrypted the tile matrix / video RAM.
    - Wrote proper memory map and inputs from the scratch.
    - Bypassed the program data channels protection.
    - Decrypted the colors / color RAM.
    - Added button-lamps support.
    - Created a default NVRAM.
    - Promoted to working state.
    - Added technical and encryption notes...

    Snooker 10:
    - Fixed NVRAM size.
    - Added default NVRAM.

    Apple 10:
    - Fixed NVRAM size.
    - Added default NVRAM.

    Ten Balls:
    - Fixed NVRAM size.
    - Added default NVRAM.


    [2013/05/04]

    - Added Crystals Colours (Ver 1.01).


    [2008/10/09]

    - Reworked the button-lamps matrix system.
    - Documented both output ports.
    - Connected coin in, key in and payout counters.
    - Improved the lamps layout to be more realistic.
    - Updated technical notes.


    [2008/06/09]

    - Fixed the tilemap size.
    - Fixed the screen size.
    - Updated technical notes.


    [2008/05/29]

    - Switched the color system to RESNET calculations.
    - Hooked the infamous bit7 of Input Port 0x3004 in parallel to DIP switch 1.
      This allow to use the PAYOUT button to trigger the Super Game instead of STOP 5.
    - Demultiplexed lamps matrix.
    - Added lamps support, but is still imperfect.
    - Updated technical notes.


    [2008/05/22]

    - Confirmed the CPU clock after some PCB measurements.
    - Changed the SND clock to 1MHz to match the PCB measurement.
    - Corrected the internal OKI6295 frequency turning the pin 7 state to HIGH.
    - Inputs: Eliminated all pulse limitations.
      Affected buttons have a rattled sound in the real thing too.
    - Updated technical notes.


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

    - Nothing... :)


***********************************************************************************/


#define MASTER_CLOCK    XTAL_16MHz

#include "emu.h"
#include "cpu/m6502/m65sc02.h"
#include "sound/okim6295.h"
#include "snookr10.lh"
#include "includes/snookr10.h"
#include "machine/nvram.h"


/**********************
* Read/Write Handlers *
*   - Input Ports -   *
**********************/

READ8_MEMBER(snookr10_state::dsw_port_1_r)
{
/*
   --------------------------------
    PORT 0x3004 ;INPUT PORT 4
   --------------------------------
    BIT 0 =
    BIT 1 =
    BIT 2 =
    BIT 3 =
    BIT 4 =
    BIT 5 =
    BIT 6 =
    BIT 7 = Complement of DS1, bit 7
   ---------------------------------
*/
return ioport("SW1")->read();
}


/**********************
* Read/Write Handlers *
*  - Output Ports -   *
**********************/

/*  Lamps are multiplexed using a 6 bit matrix.
    The first 4 bits are from Port A, and the
    remaining 2 are from Port B.

    LAMPS components:

    START  = bit5
    CANCEL = bit2
    STOP1  = bit0
    STOP2  = bit1
    STOP3  = bit0
    STOP4  = bit3
    STOP5  = bit4
*/

WRITE8_MEMBER(snookr10_state::output_port_0_w)
{
/*
   ----------------------------
    PORT 0x5000 ;OUTPUT PORT A
   ----------------------------
    BIT 0 = Coin counter.
    BIT 1 = Lamps matrix, m_bit0.
    BIT 2 = Payout x10.
    BIT 3 = Lamps matrix, m_bit1.
    BIT 4 = Key in.
    BIT 5 = Lamps matrix, m_bit2.
    BIT 6 =
    BIT 7 = Lamps matrix, m_bit3.
   ----------------------------
*/
	m_outportl = data;

	m_bit0 = (data >> 1) & 1;
	m_bit1 = (data >> 3) & 1;
	m_bit2 = (data >> 5) & 1;
	m_bit3 = (data >> 7) & 1;
	m_bit4 = m_outporth & 1;
	m_bit5 = (m_outporth >> 1) & 1;

	output().set_lamp_value(0, m_bit5);   /* Lamp 0 - START  */
	output().set_lamp_value(1, m_bit2);   /* Lamp 1 - CANCEL */
	output().set_lamp_value(2, m_bit0);   /* Lamp 2 - STOP1  */
	output().set_lamp_value(3, m_bit1);   /* Lamp 3 - STOP2  */
	output().set_lamp_value(4, m_bit0);   /* Lamp 4 - STOP3  */
	output().set_lamp_value(5, m_bit3);   /* Lamp 5 - STOP4  */
	output().set_lamp_value(6, m_bit4);   /* Lamp 6 - STOP5  */

	machine().bookkeeping().coin_counter_w(0, data & 0x01);  /* Coin in */
	machine().bookkeeping().coin_counter_w(1, data & 0x10);  /* Key in */
	machine().bookkeeping().coin_counter_w(2, data & 0x04);  /* Payout x10 */

//  logerror("high: %04x - low: %X \n", m_outporth, m_outportl);
//  popmessage("written : %02X", data);
}

WRITE8_MEMBER(snookr10_state::output_port_1_w)
{
/*
   ----------------------------
    PORT 0x5001 ;OUTPUT PORT B
   ----------------------------
    BIT 0 = Lamps matrix, m_bit4
    BIT 1 = Lamps matrix, m_bit5
    BIT 2 =
    BIT 3 =
    BIT 4 =
    BIT 5 =
    BIT 6 =
    BIT 7 =
   ----------------------------
*/
	m_outporth = data << 8;

	m_bit0 = (m_outportl >> 1) & 1;
	m_bit1 = (m_outportl >> 3) & 1;
	m_bit2 = (m_outportl >> 5) & 1;
	m_bit3 = (m_outportl >> 7) & 1;
	m_bit4 = data & 1;
	m_bit5 = (data >> 1) & 1;

	output().set_lamp_value(0, m_bit5);   /* Lamp 0 - START  */
	output().set_lamp_value(1, m_bit2);   /* Lamp 1 - CANCEL */
	output().set_lamp_value(2, m_bit0);   /* Lamp 2 - STOP1  */
	output().set_lamp_value(3, m_bit1);   /* Lamp 3 - STOP2  */
	output().set_lamp_value(4, m_bit0);   /* Lamp 4 - STOP3  */
	output().set_lamp_value(5, m_bit3);   /* Lamp 5 - STOP4  */
	output().set_lamp_value(6, m_bit4);   /* Lamp 6 - STOP5  */
}


READ8_MEMBER(snookr10_state::port2000_8_r)
{
/*
    Ports 2000-2008 expect data, otherwise the program trigger
    a sort of service-text mode, sets 300000 points and starts
    to decrease the amount one by one, triggering a sound each
    time the amount is decremented.
*/
	return 0xff;
}

/*************************
* Memory map information *
*************************/

static ADDRESS_MAP_START( snookr10_map, AS_PROGRAM, 8, snookr10_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")   /* battery backed 6116 */
	AM_RANGE(0x1000, 0x1000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("IN0")        /* IN0 */
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("IN1")        /* IN1 */
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("IN2")        /* IN2 */
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("SW1")        /* DS1 */
	AM_RANGE(0x3004, 0x3004) AM_READ(dsw_port_1_r)      /* complement of DS1, bit 7 */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(output_port_0_w)  /* OUT0 */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(output_port_1_w)  /* OUT1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(snookr10_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( tenballs_map, AS_PROGRAM, 8, snookr10_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")   /* battery backed 6116 */
	AM_RANGE(0x1000, 0x1000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x4000, 0x4000) AM_READ_PORT("IN0")        /* IN0 */
	AM_RANGE(0x4001, 0x4001) AM_READ_PORT("IN1")        /* IN1 */
	AM_RANGE(0x4002, 0x4002) AM_READ_PORT("IN2")        /* IN2 */
	AM_RANGE(0x4003, 0x4003) AM_READ_PORT("SW1")        /* DS1 */
	AM_RANGE(0x5000, 0x5000) AM_WRITE(output_port_0_w)  /* OUT0 */
	AM_RANGE(0x5001, 0x5001) AM_WRITE(output_port_1_w)  /* OUT1 */
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(snookr10_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END

static ADDRESS_MAP_START( crystalc_map, AS_PROGRAM, 8, snookr10_state )
	AM_RANGE(0x0000, 0x07ff) AM_RAM AM_SHARE("nvram")   /* battery backed 6116 */
	AM_RANGE(0x1000, 0x1000) AM_WRITE(output_port_0_w)  /* OUT0 */
	AM_RANGE(0x1001, 0x1001) AM_WRITE(output_port_1_w)  /* OUT1 */
	AM_RANGE(0x2000, 0x2008) AM_READ(port2000_8_r)      /* unknown... protection or data channels? */
	AM_RANGE(0x3000, 0x3000) AM_READ_PORT("IN0")        /* IN0 */
	AM_RANGE(0x3001, 0x3001) AM_READ_PORT("IN1")        /* IN1 */
	AM_RANGE(0x3002, 0x3002) AM_READ_PORT("IN2")        /* IN2 */
	AM_RANGE(0x3003, 0x3003) AM_READ_PORT("SW1")        /* DS1 */
	AM_RANGE(0x5000, 0x5000) AM_DEVREADWRITE("oki", okim6295_device, read, write)
	AM_RANGE(0x6000, 0x6fff) AM_RAM_WRITE(snookr10_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x7000, 0x7fff) AM_RAM_WRITE(snookr10_colorram_w) AM_SHARE("colorram")
	AM_RANGE(0x8000, 0xffff) AM_ROM
ADDRESS_MAP_END


/*************************
*      Input ports       *
*************************/

/*  Eliminated all PORT_IMPULSE limitations.
    All Hold & Cancel buttons have a rattle sound in the real PCB. */

static INPUT_PORTS_START( snookr10 )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Remote x100") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 ) PORT_NAME("Stop 1")    /* Input Test in stats mode */
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL ) PORT_NAME("Cancella (Cancel) / Play / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) PORT_NAME("Start (Deal) / Raddoppio (Double-Up)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 ) PORT_NAME("Stop 5 / Risk (Half Gamble) / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK ) PORT_NAME("Estatistica (Stats)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 ) PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Hopper")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Scarica (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )                PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Super Game Settings" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "Play to Payout" )
	PORT_DIPSETTING(    0x00, "Direct Payout" )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )   PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - User Choice" )
	PORT_DIPSETTING(    0x20, "Manual - Coins" )
	PORT_DIPSETTING(    0x40, "Manual - Tickets" )
	PORT_DIPSETTING(    0x60, "Automatic" )
	PORT_DIPNAME( 0x80, 0x80, "Super Game Button" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "PAYOUT button" )
	PORT_DIPSETTING(    0x80, "STOP 5 button" )
INPUT_PORTS_END

/*  Eliminated all PORT_IMPULSE limitations.
    All Hold & Cancel buttons have a rattle sound in the real PCB. */

static INPUT_PORTS_START( apple10 )
	PORT_INCLUDE( snookr10 )

	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )                PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x0c, 0x00, DEF_STR( Coinage ) )          PORT_DIPLOCATION("SW1:5,6")
	PORT_DIPSETTING(    0x0c, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x08, DEF_STR( 1C_2C ) )
	PORT_DIPSETTING(    0x04, DEF_STR( 1C_5C ) )
	PORT_DIPSETTING(    0x00, "1 Coin / 10 Credits" )
	PORT_DIPNAME( 0x10, 0x10, "Super Game Settings" )       PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, "Play to Payout" )
	PORT_DIPSETTING(    0x00, "Direct Payout" )
	PORT_DIPNAME( 0x60, 0x60, "Super Game Payment Type" )   PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Manual - Coins 1" )
	PORT_DIPSETTING(    0x20, "Manual - Coins 2" )
	PORT_DIPSETTING(    0x40, "Disable Payment/Game" )
	PORT_DIPSETTING(    0x60, "Automatic" )
	PORT_DIPNAME( 0x80, 0x80, "Super Game Button" )         PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, "PAYOUT button" )
	PORT_DIPSETTING(    0x80, "STOP 5 button" )
INPUT_PORTS_END

/*  Eliminated all PORT_IMPULSE limitations.
    All Hold & Cancel buttons have a rattle sound in the real PCB. */

static INPUT_PORTS_START( tenballs )
	PORT_INCLUDE( snookr10 )

	/* tenballs seems a prototype, most DIP
	   switches seems to do nothing at all.
	*/
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	/* coinage is always 1 coin - 10 credits */
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* always play Super Game to payout */
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* always manual payout */
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	/* Super Game always ON */
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static INPUT_PORTS_START( crystalc )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE )        PORT_NAME("Remote x100") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )    PORT_NAME("Stop 1")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_CANCEL )   PORT_NAME("Cancella (Cancel) / Play / Bet")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )         PORT_NAME("Start (Deal) / Raddoppio (Double-Up)")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )    PORT_NAME("Stop 5 / Risk (Half Gamble) / Super Game")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_BOOK )    PORT_NAME("Statistica (Stats)")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_SERVICE ) PORT_NAME("Management")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )    PORT_NAME("Stop 4 / Alta (High)")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 ) PORT_NAME("Stop 2 / Bassa (Low)")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD3 ) PORT_NAME("Stop 3")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Ticket") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )       PORT_NAME("Hopper") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT ) PORT_NAME("Scarica (Payout)")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )    // SG vs real value in paytable.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("SW1")
	PORT_DIPNAME( 0x03, 0x00, "Pool Value" )        PORT_DIPLOCATION("SW1:7,8")
	PORT_DIPSETTING(    0x03, "100" )
	PORT_DIPSETTING(    0x02, "200" )
	PORT_DIPSETTING(    0x01, "500" )
	PORT_DIPSETTING(    0x00, "1000" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x60, 0x60, "Payment Type" )      PORT_DIPLOCATION("SW1:3,2")
	PORT_DIPSETTING(    0x00, "Ticket + Coins" )
	PORT_DIPSETTING(    0x20, "Ticket only" )
	PORT_DIPSETTING(    0x40, "Coins only" )
	PORT_DIPSETTING(    0x60, "Ticket + Coins" )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
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
	GFXDECODE_ENTRY( "gfx1", 0x0000, charlayout, 0, 16 )
GFXDECODE_END


/**************************
*     Machine Drivers     *
**************************/

static MACHINE_CONFIG_START( snookr10, snookr10_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, MASTER_CLOCK/8)    /* 2 MHz (1.999 MHz measured) */
	MCFG_CPU_PROGRAM_MAP(snookr10_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", snookr10_state,  nmi_line_pulse)

	MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(96*4, 30*8)
	MCFG_SCREEN_VISIBLE_AREA(0*4, 96*4-1, 0*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(snookr10_state, screen_update_snookr10)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", snookr10)
	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(snookr10_state, snookr10)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_OKIM6295_ADD("oki", MASTER_CLOCK/16, OKIM6295_PIN7_HIGH)   /* 1 MHz (995.5 kHz measured); pin7 checked HIGH on PCB */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.8)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( apple10, snookr10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")

	/* video hardware */
	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(snookr10_state, apple10)
	MCFG_VIDEO_START_OVERRIDE(snookr10_state, apple10)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( tenballs, snookr10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(tenballs_map)

MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( crystalc, snookr10 )

	/* basic machine hardware */
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_PROGRAM_MAP(crystalc_map)

	MCFG_PALETTE_MODIFY("palette")
	MCFG_PALETTE_INIT_OWNER(snookr10_state, crystalc)
	MCFG_VIDEO_START_OVERRIDE(snookr10_state, crystalc)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( snookr10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(216ccb2d) SHA1(d86270cd03a08f6fd3e7b327b8173f66da28e5e8) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(a70d9c48) SHA1(3fa90190323526553866662afda4dbe1c94abeff) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(3009faaa) SHA1(d1cda455b270cb9afa65b9701735a3a1f2a48df2) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "4.u18", 0x00000, 0x40000, CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "snooker_10_nvram.bin", 0x0000, 0x0800, CRC(d383363b) SHA1(88f0d9e53b0bd276d01cefd817f0edcd753322dc) )

	/* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, "proms", 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(f3d7d640) SHA1(f78060f4603e316fa3c2ec4ba6d7edf261cf6d8a) )
ROM_END

ROM_START( apple10 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "1.u2", 0x8000, 0x8000, CRC(7d538566) SHA1(2e805157010c366ab1f2313a2bedb071c1dde733) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "2.u22", 0x0000, 0x8000, CRC(42b016f4) SHA1(59d1b77f8cb706a3878813111c6a71514c413784) )
	ROM_LOAD( "3.u25", 0x8000, 0x8000, CRC(afc535dc) SHA1(ed2d65f3154c6d80b7b22bfef1f30232e4496128) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "4.u18", 0x00000, 0x40000, CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "apple_10_nvram.bin", 0x0000, 0x0800, CRC(00069b55) SHA1(378ca25dc3173252548bc27e80909e1b2b4d58db) )

	/* this should be changed because the palette is stored in a normal ROM instead of a color PROM */
	ROM_REGION( 0x8000, "proms", 0 )
	ROM_LOAD( "5.u27", 0x0000, 0x8000, CRC(3510d705) SHA1(2190c8199d29bf89e3007eb771cc6b0e2b58f6cd) )
ROM_END

ROM_START( tenballs )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "4.u2", 0x8000, 0x8000, CRC(2f334862) SHA1(61d57995451b6bc7de23900c460c3e073993899c) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "3.u16", 0x0000, 0x8000, CRC(9eb88a08) SHA1(ab52924103e2b14c598a21c3d77b053da37a0212) )
	ROM_LOAD( "2.u15", 0x8000, 0x8000, CRC(a5091583) SHA1(c0775d9b77cb634d3702b6c08cdf73c867b6169a) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "1.u28", 0x00000, 0x40000, CRC(17090d56) SHA1(3a4c247f96c80f8cf4c1389b273880c5ea6fc39d) )

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "ten_balls_nvram.bin", 0x0000, 0x0800, CRC(42a5803f) SHA1(2c8c9ec0f26a947cf9cfa2e91e9127725becdef5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.u17", 0x0000, 0x0200, CRC(20234dcc) SHA1(197937bbec0201888467e250bdba49e39aa4204a) )
ROM_END

ROM_START( crystalc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "crystals_colours_1.u2", 0x8000, 0x8000, CRC(ee7a6f62) SHA1(bb3f438ca563eeb4b365d344066b10b9aedb77bb) )

	ROM_REGION( 0x10000, "gfx1", 0 )
	ROM_LOAD( "crystals_colours_2.u16", 0x0000, 0x8000, CRC(527c07f6) SHA1(53ceb1d0267e1d76fa1a9325c9a1d2e8e034958d) )
	ROM_LOAD( "crystals_colours_3.u15", 0x8000, 0x8000, CRC(e1003ab7) SHA1(a78bb2e1dc9d578d6a38072e2087f382cffa9f99) )

	ROM_REGION( 0x40000, "oki", 0 ) /* ADPCM samples */
	ROM_LOAD( "crystals_colours_4.u28", 0x00000, 0x40000, CRC(ecc6b575) SHA1(f6032e89b30aebeab9ad721608277430084256bc) )

	ROM_REGION( 0x0800, "nvram", 0 )    /* default NVRAM */
	ROM_LOAD( "crystals_colours_nvram.bin", 0x0000, 0x0800, CRC(907d8828) SHA1(ffd302996bee81277c2280fc212d910e0801d81d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29pc.u17", 0x0000, 0x0200, CRC(1af53eab) SHA1(1cac837aed75a77a0b93a294b07f2a8de6fcffef) )
ROM_END


/*************************
*      Game Drivers      *
*************************/

/*     YEAR  NAME      PARENT    MACHINE   INPUT     STATE          INIT ROT    COMPANY      FULLNAME                      FLAGS   LAYOUT */
GAMEL( 1998, snookr10, 0,        snookr10, snookr10, driver_device, 0,   ROT0, "Sandii'",   "Snooker 10 (Ver 1.11)",       0,      layout_snookr10 )
GAMEL( 1998, apple10,  0,        apple10,  apple10,  driver_device, 0,   ROT0, "Sandii'",   "Apple 10 (Ver 1.21)",         0,      layout_snookr10 )
GAMEL( 1997, tenballs, snookr10, tenballs, tenballs, driver_device, 0,   ROT0, "<unknown>", "Ten Balls (Ver 1.05)",        0,      layout_snookr10 )
GAMEL( 1998, crystalc, 0,        crystalc, crystalc, driver_device, 0,   ROT0, "JCD srl",   "Crystals Colours (Ver 1.01)", 0,      layout_snookr10 )
