// license:BSD-3-Clause
// copyright-holders: Roberto Fresca, Grull Osgo, Mirko Buffoni
/****************************************************************************************

  WEBAK MPU-12 PLATFORM
  ---------------------

  Driver by Roberto Fresca.


  Games running on this hardware:

  * Fruit Star Bonus (Ver 8.2.00ITL),                     199?, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.27PVIE),                      199?, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.20PIR),                       1997, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.36UNG-1100),                  1996, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.30UNG-200),                   1996, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.30UNG-25, set 1),             1996, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.30UNG-25, set 2),             1996, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.23PSTK, Steiermark),          1999, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 1),  1997, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 2),  1997, Webak Elektronik.
  * Golden Joker (Ver 16.06UNG-25, set 1),                1996, Webak Elektronik.
  * Golden Joker (Ver 16.06UNG-25, set 2),                1996, Webak Elektronik.


*****************************************************************************************

  General notes:

  "The first Webak video-based game, the Fruit Star Bonus, was a success 15 years ago
   when it was written on Assembler and was running on the MPU 12 at the time. Because
   this game has achieved such a great success to this day Webak has now transferred it
   on a 1:1 basis for the new Linux-Game box. Despite its age, it has experience a re-
   naissance in different countries, especially the Czech Republic."


*****************************************************************************************

  Hardware Notes:
  ---------------

  CPU:          1x M68B09P      \
  Program ROM:  1x P28F512 (IC2) > Inside the MPU12 epoxy block.
  PLD:          1x TIBPAL16V8   /

  Video:  1x MC68B45P CRTC.
  Sound:  1x AY-3-8910A.
  ROMS:   3x 27C512 (IC37-IC38-IC39) for graphics.
          1x Empty socket (IC4) for unknown purposes.

  Bipolar PROMs: 1x 82S147; 1x 82S131.

  RAM:    3x MB8464A-10L (8K x 8bits CMOS SRAM w/data retention).
                         (2 for VRAM, 1 for Working RAM).

  Xtal:   1x 8MHz.


*****************************************************************************************

  PCB Layout (all):

  .--------------------------------------------------------------------------------------.
  |        .---------.     .---------.                                                   |
  |        |74LS174N |     |74LS174N |              .------------.                       |
  |        '---------'     '---------'              |8          1|  .------------.       |
  |                                                 |    DIP1    |  | DM74LS245N |       |
  |        .------------.  .---------.              '------------'  '------------'       |
  |        |  N82S147AN |  |N82S131AN|                       2x17 connector              |
  |        '------------'  '---------'              .---------------------------------.  |
  |                                     .-------.   |o o o o o o o o o o o o o o o o o|  |
  |  .------------------.  .---------.  |  XTAL |   |o o o o o o o o o o o o o o o o o|  |
  |  |     Fruit 3      |  |74LS194AN|  | 8 Mhz |   '---------------------------------'  |
  |  |      27C512      |  '---------'  '-------'                                        |
  |  |              IC39|  .---------.  .--.        .--------------------.  .----------. |
  |  '------------------'  |74LS194AN|  |SN|        |                    |  |SN74LS02N | |
  |                        '---------'  |74|        |    EMPTY SOCKET    |  '----------' |
  |  .------------------.  .---------.  |LS| .--.   |                    |               |
  |  |     Fruit 2      |  |74LS194AN|  |00| |SN|   |                 IC4|               |
  |  |      27C512      |  '---------'  |N | |74|   '--------------------'     .------.  |
  |  |              IC38|               '--' |LS|                              |      |  |
  |  '------------------'  .---------.  .--. |00|     .------------------.     |      |  |
  |                        |74LS194AN|  |SN| |N |     |  MB8464A-10L-SK  |     |      |  |
  |  .------------------.  '---------'  |74| '--'     '------------------'     |AY-3  |  |
  |  |     Fruit 1      |               |LS|                                   |8910A |  |
  |  |      27C512      |  .---------.  |02| .-------------------------------. |      |  |
  |  |              IC37|  |74LS194AN|  |N | |EPOXY BLOCK                    | |      |  |
  |  '------------------'  '---------'  |  | |      .------.                 | |      |  |
  |                                     '--' |      |      |                 | |      |  |
  |                        .---------.  .--. |      |      |                 | |      |  |
  |  .-------------.       |74LS194AN|  |DM| |      |      |                 | |      |  |
  |  | DM74LS374N  |       '---------'  |74| |      |      |                 | |      |  |
  |  '-------------'                    |LS| |      |      |                 | |      |  |
  |                        .---------.  |74| |      | MCU  |                 | |      |  |
  |  .-------------.       |74LS174N |  |AN| |      |      |                 | |      |  |
  |  | DM74LS245N  |       '---------'  |  | |      |68B09P|                 | |      |  |
  |  '-------------'                    '--' |      |      |                 | |      |  |
  |                    .-------------.  .--. |      |      |                 | |      |  |
  |                    | DM74LS374N  |  |DM| |      |      |                 | |      |  |
  | .-------.          '-------------'  |74| |      |      |                 | '------'  |
  | |       |                           |LS| |      |      |                 |           |
  | |       | .----------------.  .--.  |24| |      |      |  .------.       |           |
  | |       | | MB8464A-10L-SK |  |SN|  |5N| |      |      |  |FLASH |       | .--.      |
  | |       | '----------------'  |74|  |  | |      |      |  |      |       | |LM|      |
  | |       |                     |LS|  |  | |      |      |  | P28F |       | |35|      |
  | |       | .---------.   .--.  |59|  |  | |      '------'  | 512  |       | |8N|      |
  | |  MC   | |74LS157N |   |MB|  |0N|  |  | |                |      |       | '--'      |
  | |       | '---------'   |84|  |  |  '--' |                |      |       |           |
  | |68B45P |               |64|  '--'       |                |      |       | .--------.|
  | |       | .---------.   |A-|        .--. |         .---.  |      |       | | LM380N ||
  | |       | |74LS157N |   |10|  .--.  |DM| |         |   |  |      |       | '--------'|
  | |       | '---------'   |L-|  |SN|  |74| |         |TIB|  |      |       |           |
  | |       |               |SK|  |74|  |LS| |         |PAL|  |      |       | .--.      |
  | |       | .---------.   |  |  |LS|  |24| |         |16 |  |   IC2|       | |SN|      |
  | |       | |74LS157N |   |  |  |59|  |5N| |         |L8 |  '------'       | |74|      |
  | |       | '---------'   |  |  |0N|  |  | |         |   |                 | |LS|      |
  | |       |               |  |  |  |  |  | |         |   |                 | |27|      |
  | |       | .---------.   |  |  '--'  |  | |         |   |                 | |3N|      |
  | |       | |74LS157N |   |  |        |  | |         '---'                 | |  |      |
  | '-------' '---------'   '--'        '--' |                               | |  |      |
  |                                          |                               | |  |      |
  |   .---------. .-------------.            '-------------------------------' '--'      |
  |   |74LS174N | | DM74LS245N  |                                                        |
  |   '---------' '-------------' .---.            .---------.  .---------.  .---------. |
  |                               |A00|            | 74LS138N|  | 74LS138N|  |74LS245N | |
  | .-----------. .-------------. '---'            '---------'  '---------'  '---------' |
  | |DM74LS245N | | DM74LS245N  |                                                        |
  | '-----------' '-------------' .--------. .------------. .------------.  .----------. |
  |                               |74LS174N| | SN74LS273N | | SN74LS273N |  |898-3-R220| |
  |  .----------. .----------.    '--------' '------------' '------------'  '----------' |
  |  |898-3-R220| |898-3-R220|                                                           |
  |  '----------' '----------'     .--------.   .--------.   .--------.                  |
  |                                |ULN2003A|   |ULN2003A|   |ULN2003A|                  |
  |                                '--------'   '--------'   '--------'                  |
  |                                                                                      |
  '------.- - - - - -2x22 edge connector- - - - - - -.-------.2x10 edge connector.-------'
         | | | | | | | | | | | | | | | | | | | | | | |       | | | | | | | | | | |
         '-------------------------------------------'       '-------------------'

  A00 = PCF1251P (Philips CMOS voltage monitor/supervisor)


*****************************************************************************************

  Fruit Star Bonus control panel layout:

  .------------------------------------------------------------------------------.
  | .--------. .--------. .--------. .--------. .--------. .--------. .--------. |
  | |  BET   | |  TAKE  | | STOP 1 | | STOP 2 | | STOP 3 | | STOP 4 | | START  | |
  | '--------' '--------' '--------' '--------' '--------' '--------' '--------' |
  '------------------------------------------------------------------------------'


*****************************************************************************************

  PINOUT:

                     (WEBAK CONNECTION Standard MPU12/MPU2000)

  +-----------------------------------------------------------------------------------+
  |           Component-Side               ||              Solder-Side                |
  +-----------------------------------------------------------------------------------+
  |       Function       | Direction | Nr. || Nr. | Direction |       Function        |
  +===================================================================================+
  | HOPPER-OUT           |  OUTPUT   | 10  ||  J  |  OUTPUT   |                       |
  | REMOTE-PL            |  OUTPUT   | 09  ||  I  |  OUTPUT   |                       |
  | REMOTE-CLOCK         |  OUTPUT   | 08  ||  H  |  OUTPUT   | REMOTE-DOUT           |
  | Lamp HOPPER-OUT      |  OUTPUT   | 07  ||  G  |  INPUT    | REMOTE-IN             |
  | REMOTE-SELECT        |  INPUT    | 06  ||  F  |  INPUT    |                       |
  |                      |  INPUT    | 05  ||  E  |  INPUT    | Bookkeeping 3         |
  |                      |  INPUT    | 04  ||  D  |  INPUT    | Button "Select Game"  |
  |                      |           | 03  ||  C  |           |                       |
  | EX64-SELECT          |  OUTPUT   | 02  ||  B  |  OUTPUT   | Lamp "Select Game"    |
  | EXTRA-Lamp           |  OUTPUT   | 01  ||  A  |  OUTPUT   | reserved              |
  +-----------------------------------------------------------------------------------+
  +-----------------------------------------------------------------------------------+
  | GND                  |  SUPPLY   | 22  ||  Z  |  SUPPLY   | GND                   |
  | GND                  |  SUPPLY   | 21  ||  Y  |  SUPPLY   | GND                   |
  | GND                  |  SUPPLY   | 20  ||  X  |  SUPPLY   | GND                   |
  | +5V                  |  SUPPLY   | 19  ||  W  |  SUPPLY   | +5V                   |
  | +12V                 |  SUPPLY   | 18  ||  V  |  SUPPLY   | +12V                  |
  | LAMP - HOLD 1        |  OUTPUT   | 17  ||  U  |  OUTPUT   | LAMP - START          |
  | LAMP - HOLD 2        |  OUTPUT   | 16  ||  T  |  OUTPUT   | LAMP - HOLD 5         |
  | LAMP - CANCEL        |  OUTPUT   | 15  ||  S  |  OUTPUT   | LAMP - HOLD 4         |
  | COIN INPUT 1         |  INPUT    | 14  ||  R  |  OUTPUT   | LAMP - HOLD 3/Printer |
  | Mech. Counter-IN     |  OUTPUT   | 13  ||  P  |  INPUT    | BOOKKEEPING 1         |
  | Mech. Counter-OUT    |  OUTPUT   | 12  ||  N  |  INPUT    | Button HOLD 1         |
  | Mech. Counter-3      |  OUTPUT   | 11  ||  M  |  INPUT    | Button CANCEL         |
  | Button HOLD 5        |  INPUT    | 10  ||  L  |  INPUT    | Button START          |
  | Bookkeeping 2        |  INPUT    | 09  ||  K  |  INPUT    | Bookkeeping A (Waiter)|
  | Button HOLD 2        |  INPUT    | 08  ||  J  |  INPUT    | Button HOLD 4         |
  | COIN INPUT 3         |  INPUT    | 07  ||  H  |  INPUT    | Button HOLD 3/Printer |
  | HOPPER COUNT         |  INPUT    | 06  ||  F  |  INPUT    | EXTRA Button          |
  | Button HOPPER OUT    |  INPUT    | 05  ||  E  |  INPUT    | COIN INPUT 2          |
  | Monitor GREEN        |TTLOUT-Anal| 04  ||  D  |TTLOUT-Anal| Monitor RED           |
  | Monitor SYNC         |TTLOUT-Anal| 03  ||  C  |TTLOUT-Anal| Monitor BLUE          |
  | SPEAKER              |OUT-Analog | 02  ||  B  |  SUPPLY   | Monitor GND           |
  | CREDIT CLEAR         |  INPUT    | 01  ||  A  |  SUPPLY   | SPEAKER GND           |
  +-----------------------------------------------------------------------------------+


*****************************************************************************************

  DIP Switches & CPU box (Ver 8.27PVIE)
  -------------------------------------


  DIP 1:
  .---------------.
  | |#|#|#|#|#| | |
  |---------------|
  |#| | | | | |#|#|
  '---------------'
   1 2 3 4 5 6 7 8


  EPOXY BLOCK - COVER:
  .--------------------------------------------------------------------------------.
  |                                                      .-----------------------. |
  |      #######################################         |        VERSION        | |
  |  #  ##                                     ##  #     |       Fruit STK       | |
  |   # ##  #   #  #####  ####    ###   #   #  ## #      '-----------------------' |
  |    ###  #   #  #      #   #  #   #  #  #   ###                                 |
  |  #####  # # #  ###    ####   #####  ###    #####     .-----------------------. |
  |    ###  # # #  #      #   #  #   #  #  #   ###       | NO:      6***         | |
  |   # ##   # #   #####  ####   #   #  #   #  ## #      | --------------------- | |
  |  #  ##                                     ##  #     | DATE:                 | |
  |      #######################################         | --------------------- | |
  |                                                      | CUST:                 | |
  |              SCHWANENSTADT - AUSTRIA                 '-----------------------' |
  |              Tel.: 43 (7673) 4201-0                                            |
  |              Fax : 43 (7673) 4201-23                                           |
  |                                                                                |
  |      ####################################################################      |
  |  #  ##              ## ##  ####   #   #       ##   ####                 ##  #  |
  |   # ##              # # #  #   #  #   #      # #  #   #                 ## #   |
  |    ###              # # #  ####   #   #     #  #     #                  ###    |
  |   # ##              #   #  #      #   #        #    #                   ## #   |
  |  #  ##              #   #  #       ###         #  #####                 ##  #  |
  |      ####################################################################      |
  |                                                                                |
  '--------------------------------------------------------------------------------'


  DIP Switches & CPU box (Ver 8.20PIR)
  -------------------------------------


  DIP Switches bank:
  .---------------.
  | |#| |#| |#| | |
  |-+-+-+-+-+-+-+-|
  |#| |#| |#| |#|#|
  '---------------'
   1 2 3 4 5 6 7 8


  Epoxy block - cover:
  .--------------------------------------------------------------------------------.
  |                                                      .-----------------------. |
  |      #######################################         |        VERSION        | |
  |  #  ##                                     ##  #     |         Fruit         | |
  |   # ##  #   #  #####  ####    ###   #   #  ## #      '-----------------------' |
  |    ###  #   #  #      #   #  #   #  #  #   ###                                 |
  |  #####  # # #  ###    ####   #####  ###    #####     .-----------------------. |
  |    ###  # # #  #      #   #  #   #  #  #   ###       | NO:                   | |
  |   # ##   # #   #####  ####   #   #  #   #  ## #      | --------------------- | |
  |  #  ##                                     ##  #     | DATE:     12/97       | |
  |      #######################################         | --------------------- | |
  |                                                      | CUST:                 | |
  |              SCHWANENSTADT - AUSTRIA                 '-----------------------' |
  |              Tel.: 43 (7673) 4201-0                                            |
  |              Fax : 43 (7673) 4201-23                                           |
  |                                                                                |
  |      ####################################################################      |
  |  #  ##              ## ##  ####   #   #       ##   ####                 ##  #  |
  |   # ##              # # #  #   #  #   #      # #  #   #                 ## #   |
  |  # ###              # # #  ####   #   #     #  #     #                  ### #  |
  |   # ##              #   #  #      #   #        #    #                   ## #   |
  |  #  ##              #   #  #       ###         #  #####                 ##  #  |
  |      ####################################################################      |
  |                                                                                |
  '--------------------------------------------------------------------------------'


  DIP Switches & CPU box (Ver 8.30UNG-200 & 8.36UNG-1100)
  -------------------------------------------------------


  DIP Switches bank:
  .---------------.
  | |#| | | | | | |
  |-+-+-+-+-+-+-+-|
  |#| |#|#|#|#|#|#|
  '---------------'
   1 2 3 4 5 6 7 8


  Epoxy block - cover:
  .--------------------------------------------------------------------------------.
  |                                                UNG   .-----------------------. |
  |      #######################################         |        VERSION        | |
  |  #  ##                                     ##  #     |       Fruit 1100      | |
  |   # ##  #   #  #####  ####    ###   #   #  ## #      '-----------------------' |
  |    ###  #   #  #      #   #  #   #  #  #   ###                                 |
  |  #####  # # #  ###    ####   #####  ###    #####     .-----------------------. |
  |    ###  # # #  #      #   #  #   #  #  #   ###       | NO:       4***        | |
  |   # ##   # #   #####  ####   #   #  #   #  ## #      | --------------------- | |
  |  #  ##                                     ##  #     | DATE:     10/96       | |
  |      #######################################         | --------------------- | |
  |                                                      | CUST:                 | |
  |              SCHWANENSTADT - AUSTRIA                 '-----------------------' |
  |              Tel.: 43 (7673) 4201-0                                            |
  |              Fax : 43 (7673) 4201-23                                           |
  |                                                                                |
  |      ####################################################################      |
  |  #  ##              ## ##  ####   #   #       ##   ####                 ##  #  |
  |   # ##              # # #  #   #  #   #      # #  #   #                 ## #   |
  |  # ###              # # #  ####   #   #     #  #     #                  ### #  |
  |   # ##              #   #  #      #   #        #    #                   ## #   |
  |  #  ##              #   #  #       ###         #  #####                 ##  #  |
  |      ####################################################################      |
  |                                                                                |
  '--------------------------------------------------------------------------------'


*****************************************************************************************

  DIP Switches reverse-engineering...

  +-------+----------------------------------+------------+--------------+
  | DSW:  | FUNCTION                         |     ON     |     OFF      |
  +-------+----------------------------------+------------+--------------+
  +-------+----------------------------------+------------+--------------+
  | DSW#1 | Enable Remote Accounts Clear     |     ON     |     OFF      |
  +-------+----------------------------------+------------+--------------+
  | DSW#2 | Quick Start on Max Bet           |     ON     |     OFF      |
  +-------+----------------------------------+------------+--------------+
  | DSW#3 | Coin 2 Settings                  |   Coin 2   |    Change    |
  +-------+----------------------------------+------------+--------------+
  | DSW#4 | Key In                           |  Disable   |    Enable    |
  +-------+----------------------------------+------------+--------------+
  | DSW#5 | Max Bet Settings                 |   Allow    |     Deny     |
  +-------+----------------------------------+------------+--------------+
  | DSW#6 | Currency (only in parent)        |    EURO    | Credits Mode |
  +-------+----------------------------------+------------+--------------+
  | DSW#7 | Autostop                         |     ON     |     OFF      |
  +-------+----------------------------------+------------+--------------+
  | DSW#8 | Hopper                           |     ON     |     OFF      |
  +-------+----------------------------------+------------+--------------+


*****************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x0000 - 0x0FFF    ; NVRAM.
  0x1000 - 0x1000    ; Input #0.
  0x1004 - 0x1004    ; Input #1.
  0x1008 - 0x1008    ; Input #2.
  0x100C - 0x100C    ; Output #0 (lamps + meters).
  0x1010 - 0x1010    ; Output #1 (lamps).
  0x1014 - 0x1014    ; Output #2.
  0x1018 - 0x1018    ; Output #3.
  0x1100 - 0x1100    ; DIP Switches bank.
  0x1300 - 0x1300    ; AY-3-8910 (data).
  0x1301 - 0x1301    ; AY-3-8910 (adress).
  0x1400 - 0x1400    ; M6845 CRTC (adress).
  0x1401 - 0x1401    ; M6845 CRTC (data).
  0x1800 - 0x1FFF    ; Working RAM (also battery backed).
  0x2000 - 0x2FFF    ; Video RAM.
  0x3000 - 0x3FFF    ; Color RAM.
  0x8000 - 0xFFFF    ; ROM space.


  *** CRTC Initialization ***
  ----------------------------------------------------------------------------------------------------------------------
  register:  R00   R01   R02   R03   R04   R05   R06   R07   R08   R09   R10   R11   R12   R13   R14   R15   R16   R17
  ----------------------------------------------------------------------------------------------------------------------
  value:     0x7F  0x60  0x6A  0x88  0x22  0x08  0x20  0x20  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.


*****************************************************************************************


  DRIVER UPDATES:


  [2022-01-16 to 2022-03-04]

    After hard work....

  - Fixed screen matrix according to the CRTC values.
  - Improved memory map.
  - Hooked the AY-3-8910. Adjusted the clock.
  - Decoded bitplanes and unscrambled the color PROM addressing.
  - Calculated the color resnet.
  - Got the correct palette and game colors.
  - Working inputs from the scratch.
  - Completely reversed all the DIP switches.
  - Supported Button-lamps.
  - Added control panel layout.
  - Added two new hungarian clones.
  - Promoted all supported games to working.
  - Added hopper support.
  - Added default NVRAM to all sets.
  - Added more findings, diagrams and technical notes.


  [2022-01-09]

  - Added a new parent.
  - Improved gfxdecode. Added two extra bitplanes.
  - Added more ASCII layouts and technical notes.


  [2012-10-10]

  - Initial release.
  - Added technical notes.


  TODO:

  - Find the obscure serial remote credits scheme.


*****************************************************************************************/

#include "emu.h"
#include "video/resnet.h"
#include "cpu/m6809/m6809.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"
#include "machine/ticket.h"

#include "fruitstb.lh"
#include "goldnjkr.lh"


namespace {

#define MASTER_CLOCK    XTAL(8'000'000)


class mpu12wbk_state : public driver_device
{
public:
	mpu12wbk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_hopper(*this, "hopper"),
		m_lamps(*this, "lamp%u", 0U)
	{ }

	void mpu12wbk(machine_config &config);

	void init_mpu12wbk();

protected:
	virtual void video_start() override ATTR_COLD;
	virtual void machine_start() override { m_lamps.resolve(); }

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	void mpu12wbk_palette(palette_device &palette) const;
	tilemap_t *m_bg_tilemap = nullptr;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ticket_dispenser_device> m_hopper;
	output_finder<15> m_lamps;

	void mp12wbk_outport(offs_t offset, uint8_t data);
	void mpu12wbk_videoram_w(offs_t offset, uint8_t data);
	void mpu12wbk_colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void crtc_vs(int state);

	uint32_t screen_update_mpu12wbk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mpu12wbk_map(address_map &map) ATTR_COLD;
	uint32_t m_frames = 0;
};


/********************************
*        Video Hardware         *
********************************/

/*
  Bipolar PROMs wiring + RESNET...

    82s131                 82s147
  ----------       ----------------------
  3  2  1  0       7  6  5  4  3  2  1  0                     Bits    RESNET (Ohms)
  |  |  |  |       |  |  |  |  |  |  |  |                     3210
  |  '------------------------------------------- Color R ->  ---x  ----ZZZ(2200)----+---- RED
  |     |  |       |  |  |  |  |  |  |  '-------- Color R ->  --x-  ----ZZZ(1000)----+
  |     |  |       |  |  |  |  |  |  '----------- Color R ->  -x--  ----ZZZ(470)-----+
  |     |  |       |  |  |  |  |  '-------------- Color R ->  x---  ----ZZZ(220)-----+
  |     |  |       |  |  |  |  |
  '---------------------------------------------- Color B ->  ---x  ----ZZZ(2200)----+---- BLUE
        |  |       |  |  |  |  '----------------- Color B ->  --x-  ----ZZZ(1000)----+
        |  |       |  |  |  '-------------------- Color B ->  -x--  ----ZZZ(470)-----+
        |  |       |  |  '----------------------- Color B ->  x---  ----ZZZ(220)-----+
        |  |       |  |
        '---------------------------------------- Color G ->  ---x  ----ZZZ(2200)----+---- GREEN
           |       |  '-------------------------- Color G ->  --x-  ----ZZZ(1000)----+
           |       '----------------------------- Color G ->  -x--  ----ZZZ(470)-----+
           '------------------------------------- Color G ->  x---  ----ZZZ(220)-----+
*/

void mpu12wbk_state::mpu12wbk_palette(palette_device &palette) const
{
	uint8_t const *const color_prom = memregion("proms")->base();
	uint8_t const *const color_prom2 = memregion("proms2")->base();

	static constexpr int resistances[4] = { 2200, 1000, 470, 220 };

	// compute the color output resistor weights
	double scaler[4];
	compute_resistor_weights(0, 255, -1.0,
			4,  &resistances[0], scaler, 0, 0,
			0,  nullptr, nullptr, 0, 0,
			0,  nullptr, nullptr, 0, 0);

	for (int i = 0; i < palette.entries(); i++)
	{
		int data = (color_prom2[i] << 8) | color_prom[i];  // 4bit + 8bit
		data = bitswap<12>(data, 8, 7, 6, 9, 5, 4, 3, 11, 2, 1, 0, 10);

		int bit0, bit1, bit2, bit3;

		// red component
		bit0 = BIT(data, 0);
		bit1 = BIT(data, 1);
		bit2 = BIT(data, 2);
		bit3 = BIT(data, 3);
		int const r = combine_weights(scaler, bit0, bit1, bit2, bit3);

		// blue component
		bit0 = BIT(data, 4);
		bit1 = BIT(data, 5);
		bit2 = BIT(data, 6);
		bit3 = BIT(data, 7);
		int const b = combine_weights(scaler, bit0, bit1, bit2, bit3);

		// green component
		bit0 = BIT(data, 8);
		bit1 = BIT(data, 9);
		bit2 = BIT(data, 10);
		bit3 = BIT(data, 11);
		int const g = combine_weights(scaler, bit0, bit1, bit2, bit3);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void mpu12wbk_state::mpu12wbk_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void mpu12wbk_state::mpu12wbk_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(mpu12wbk_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---x xxxx   bank select.
    xxx- ----   color code.
*/
	int attr = m_colorram[tile_index];
	int code = m_videoram[tile_index] | ((attr & 0x1f) << 8);
	int color = (attr) >> 5;
	tileinfo.set(0, code, color, 0);
}


void mpu12wbk_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mpu12wbk_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 8, 96, 32);
}

uint32_t mpu12wbk_state::screen_update_mpu12wbk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


/********************************
*    Interrupt Control          *
********************************/
void mpu12wbk_state::crtc_vs(int state)
{
	if( m_frames++ > 120)
	{
		m_maincpu->set_input_line(INPUT_LINE_NMI, state);
		//logerror("VBlank:time:%s\n", machine().time().as_string());
	}
}


/********************************
*    Memory Map Information     *
********************************/

void mpu12wbk_state::mpu12wbk_map(address_map &map)
{
	map(0x0000, 0x0fff).ram().share("nvram");                                                               // Backed battery MB8464A-10L
	map(0x1300, 0x1300).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::data_w));             // OK
	map(0x1301, 0x1301).w("ay8910", FUNC(ay8910_device::address_w));                                        // OK
	map(0x1400, 0x1400).w("crtc", FUNC(mc6845_device::address_w));                                          // OK
	map(0x1401, 0x1401).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));       // OK
	map(0x1800, 0x1fff).ram().share("nvram2");                                                              // Backed battery MB8464A-10L
	map(0x2000, 0x2fff).ram().w(FUNC(mpu12wbk_state::mpu12wbk_videoram_w)).share("videoram");               // 1x MB8464A-10L
	map(0x3000, 0x3fff).ram().w(FUNC(mpu12wbk_state::mpu12wbk_colorram_w)).share("colorram");               // 1x MB8464A-10L

	map(0x1000, 0x1000).portr("IN0");   // OK
	map(0x1004, 0x1004).portr("IN1");   // OK
	map(0x1008, 0x1008).portr("IN2");   // OK

	map(0x100c, 0x1018).w(FUNC(mpu12wbk_state::mp12wbk_outport));    // OK

	map(0x1100, 0x1100).portr("SW1");   // OK

	map(0x8000, 0xffff).rom();          // OK
};


/********************************
*         Output Ports          *
********************************/

/*
  Fruit Star Bonus - Out Ports
  ----------------------------

  0x100C    D0  Meter Coin ???
            D1  Meter Coin Out
            D2  Meter Coin In
            D3  Lamp Stop Reel 2
            D4  x
            D5  x
            D6  x
            D7  x

  0x1010    D0  Lamp Stop Reel 3
            D1  Lamp Take
            D2  Lamp Stop Reel 4
            D3  Lamp Stop Reel 1
            D4  Start
            D5  Bet
            D6  Unused
            D7  Unused

  0x1014    D0  ?0
            D1  ?0
            D2  ?0
            D3  ?0
            D4  ?0
            D5  ?1
            D6  ?0
            D7  ?0

  0x1018    D0  ?1
            D1  ?1
            D2  ?1
            D3  ?1
            D4  ?1
            D5  x
            D6  ?1/0
            D7  x

*/

void mpu12wbk_state::mp12wbk_outport(offs_t offset, uint8_t data)
{
//  logerror("Offset: %x - Data:%x\n", offset, data);

	switch( offset )
	{
		// 0x100C
		case 0x00:  machine().bookkeeping().coin_counter_w(0, BIT(data, 0)); // M1 - Coins ???.
					machine().bookkeeping().coin_counter_w(1, BIT(data, 1)); // M2 - Coins Out.
					machine().bookkeeping().coin_counter_w(2, BIT(data, 2)); // M3 - Coins In.
					m_lamps[6] = BIT(data, 3);   // lamp6 (Stop 1) ok
					break;

		// 0x1010
		case 0x04:  m_lamps[0] = BIT(data, 0);    // lamp0 (Stop 3) ok
					m_lamps[1] = BIT(data, 1);    // lamp1 (Take)
					m_lamps[2] = BIT(data, 2);    // lamp2 (Stop 4) ok
					m_lamps[3] = BIT(data, 3);    // lamp3 (Stop 1) ok
					m_lamps[4] = BIT(data, 4);    // lamp4 (Start)  ok
					m_lamps[5] = BIT(data, 5);    // lamp5 (Bet)    ok
					break;

		// 0x1014
		case 0x08:  m_hopper->motor_w(BIT(data, 7));
					break;

		// 0x1018
		case 0x0c:  break;
	}
};


/********************************
*         Input Ports           *
********************************/

/*
  Fruit Star  - Button Input Function
  -----------------------------------

  ---- 0x1000 ----

  D0    KeyOut
  D1    Coin2 / Change (Partial KeyOut).    (DSW#3 ON for Coin2 Mode. OFF for Change Mode)
  D2    Payout (Hopper)
  D3    -
  D4    Hopper coin out
  D5    Coin3
  D6    KeyIn1  ->  Stop1   ->  100  Credits
                ->  Stop2   ->  1000 Credits
                ->  Stop3   ->  100  Credits
                ->  Take    ->  Clear
                ->  Stop4   ->  Accounting

  D7    Service ->  Stop1   ->  Program     ->  Stop1   ->  Coin In 2           (0-20, 30, 40, 50, 60, 70, 80, 90, 100)
                                            ->  Stop2   ->  Coin In 3           (0-20, 30, 40, 50, 60, 70, 80, 90, 100)
                                            ->  Stop3   ->  Max Bet (DSW#5 ON)  (1, 2, 3, 4, 5, 10, 20)
                                            ->  Stop4   ->  Coin Out            (x1, x2, x3, x4)
                                            ->  Bet     ->  Coin In 1           (0-20, 30, 40, 50, 60, 70, 80, 90, 100)
                                            ->  Start   ->  End

                ->  Stop2   ->  Print Accounting 2
                ->  Stop4   ->  Screen Test / Lamp Test
                ->  Start   ->  End
                ->  Stop3   ->  Hopper Fill
                ->  Take    ->  5 Seconds -> Data Clear
                ->  Coin1   ->  Button Echo


  ---- 0x1004 ----

  D0    Stop2
  D1    Stop1
  D2    Stop3
  D3    Stop4
  D4    Start/Double
  D5    Take
  D6    Bet / Half Gamble
  D7    KeyIn2  ->  Stop1   ->  100  Credits
                ->  Stop2   ->  1000 Credits
                ->  Stop3   ->  100  Credits
                ->  Take    ->  Clear


  ---- 0x1008 ----

  D0    Coin1
  D1    Unknown
  D2    Operator Accounting
  D3    Unknown
  D4    Unknown
  D5    Unknown
  D6    Unknown
  D7    Unknown

*/

static INPUT_PORTS_START( mpu12wbk )
	PORT_START("IN0")  // 0x1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                                          // DSW#2 OFF = Change; DSW#2 ON = Coin2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("0-4") PORT_CODE(KEYCODE_S)  // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Supervisor Key") PORT_CODE(KEYCODE_8) PORT_TOGGLE   // key in / other features
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service")        PORT_CODE(KEYCODE_0)               // all settings

	PORT_START("IN1")  // 0x1004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Stop 2")                   // button 4 in layout.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Stop 1")                   // button 3 in layout.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Stop 3 / Paytable")        // button 5 in layout.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Stop 4 / Paytable")        // button 6 in layout.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Start / Double (Doppeln)") // button 7 in layout.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )  PORT_NAME("Take (Loeschen-Nehmen)")   // button 2 in layout.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_BET )   PORT_NAME("Bet / Half Gamble")        // button 1 in layout.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Attendant Key")  PORT_CODE(KEYCODE_9) PORT_TOGGLE  // key in / in-out accounts

	PORT_START("IN2")  // 0x1008
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-2") PORT_CODE(KEYCODE_F)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Accounting") PORT_CODE(KEYCODE_E) PORT_TOGGLE  // in-out accounts
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-4") PORT_CODE(KEYCODE_G)  // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-5") PORT_CODE(KEYCODE_H)  // unknown
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-6") PORT_CODE(KEYCODE_J)  // unknown
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-7") PORT_CODE(KEYCODE_K)  // unknown
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-8") PORT_CODE(KEYCODE_L)  // unknown

PORT_START("SW1")  // 0x1100
	PORT_DIPNAME( 0x01, 0x01, "Enable Remote Accounts Clear" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x02, "Quick Start on Max Bet" )    PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x00, "Coin 2 Settings" )       PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, "Change" )
	PORT_DIPSETTING(    0x00, "Coin 2" )

	PORT_DIPNAME( 0x08, 0x08, "Key In" )                PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, "Enable" )
	PORT_DIPSETTING(    0x00, "Disable" )

	PORT_DIPNAME( 0x10, 0x00, "Max Bet Settings" )      PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, "Allow" )
	PORT_DIPSETTING(    0x00, "Deny" )

	PORT_DIPNAME( 0x20, 0x20, "Currency" )              PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, "Credits" )
	PORT_DIPSETTING(    0x00, "Euro Currency" )

	PORT_DIPNAME( 0x40, 0x40, "Autostop")               PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x80, "Hopper" )                PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


static INPUT_PORTS_START( goldnjkr )
	PORT_START("IN0")  // 0x1000
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_GAMBLE_KEYOUT )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )                                          // DSW#2 OFF = Change; DSW#2 ON = Coin2
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("0-4") PORT_CODE(KEYCODE_S)  // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_READ_LINE_DEVICE_MEMBER("hopper", ticket_dispenser_device, line_r)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Supervisor Key") PORT_CODE(KEYCODE_8) PORT_TOGGLE   // key in / other features
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Service")        PORT_CODE(KEYCODE_0)               // all settings

	PORT_START("IN1")  // 0x1004
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_POKER_HOLD3 )  PORT_NAME("Stop 3 / Half Gamble")       // button 4 in layout.
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_POKER_HOLD2 )  PORT_NAME("Stop 2 / Low")               // button 3 in layout.
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_POKER_HOLD4 )  PORT_NAME("Stop 4 / High")              // button 5 in layout.
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_POKER_HOLD5 )  PORT_NAME("Stop 5 / Black")             // button 6 in layout.
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )       PORT_NAME("Start / Deal")               // button 7 in layout.
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )  PORT_NAME("Take / Cancel")              // button 1 in layout.
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_POKER_HOLD1 )  PORT_NAME("Stop 1 / Red / Info")        // button 2 in layout.
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE )      PORT_NAME("Attendant Key")  PORT_CODE(KEYCODE_9) PORT_TOGGLE  // key in / in-out accounts

	PORT_START("IN2")  // 0x1008
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-2") PORT_CODE(KEYCODE_F)  // unknown
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("Operator Accounting") PORT_CODE(KEYCODE_E) PORT_TOGGLE  // in-out accounts
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-4") PORT_CODE(KEYCODE_G)  // unknown
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-5") PORT_CODE(KEYCODE_H)  // unknown
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-6") PORT_CODE(KEYCODE_J)  // unknown
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-7") PORT_CODE(KEYCODE_K)  // unknown
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER )   PORT_NAME("2-8") PORT_CODE(KEYCODE_L)  // unknown

PORT_START("SW1")  // 0x1100
	PORT_DIPNAME( 0x01, 0x01, "DSW #1" )            PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x02, 0x02, "DSW #2" )            PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x04, 0x04, "DSW #3" )            PORT_DIPLOCATION("SW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x08, 0x08, "DSW #4" )            PORT_DIPLOCATION("SW1:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x10, 0x10, "DSW #5" )            PORT_DIPLOCATION("SW1:5")
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x20, 0x20, "DSW #6" )            PORT_DIPLOCATION("SW1:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x40, 0x40, "DSW #7")             PORT_DIPLOCATION("SW1:7")
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_DIPNAME( 0x80, 0x80, "DSW #8" )            PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/********************************
*       Graphics Layouts        *
********************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,3),
	6,
//  the first two bitplanes are inverted...
	{ RGN_FRAC(0,3) + 4, RGN_FRAC(0,3), RGN_FRAC(1,3), RGN_FRAC(1,3) + 4, RGN_FRAC(2,3), RGN_FRAC(2,3) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2

};


/********************************
*  Graphics Decode Information  *
********************************/

static GFXDECODE_START( gfx_mpu12wbk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 8 )
GFXDECODE_END


/********************************
*       Machine Drivers         *
********************************/

void mpu12wbk_state::mpu12wbk(machine_config &config)
{
	// basic machine hardware
	MC6809(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu12wbk_state::mpu12wbk_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);
	NVRAM(config, "nvram2", nvram_device::DEFAULT_ALL_0);

	HOPPER(config, m_hopper, attotime::from_msec(100));

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));

	// Default screen values analyzing the MC6845 setup / init.
	screen.set_size((48+1)*8, (48+1)*8);                // From MC6845, registers 00 & 04. (value-1)
	screen.set_visarea(0*8, 48*8-1, 0*8, 32*8-1);       // Driven by MC6845, registers 01 & 06
	screen.set_screen_update(FUNC(mpu12wbk_state::screen_update_mpu12wbk));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mpu12wbk);
	PALETTE(config, "palette", FUNC(mpu12wbk_state::mpu12wbk_palette), 512);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 4));  // clock guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	//crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);
	crtc.out_vsync_callback().set(FUNC(mpu12wbk_state::crtc_vs));

	// sound hardware
	SPEAKER(config, "mono").front_center();
	AY8910(config, "ay8910", MASTER_CLOCK / 4).add_route(ALL_OUTPUTS, "mono", 1.00);   // clock guessed

}


/********************************
*           Rom Load            *
********************************/

/*
  Fruit Star Bonus (Ver 8.2.00ITL)
  International version with English language.

  No MPU number, and no stickers in the CPU epoxy block.

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  BET button is also used as HOLDs cancel, and to trigger the service menu in the service mode.

*/

ROM_START( fruitstb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_8200itl_box.ic2",  0x8000, 0x8000, CRC(5810a945) SHA1(9f11ae7b4ca620400f0e05871812e2d83d47185f) )
	ROM_IGNORE(                           0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "03.ic39",  0x00000, 0x10000, CRC(062cc82d) SHA1(153bb45bc3d7d7cc820b79941291f3fc74f22cfd) )
	ROM_LOAD( "02.ic38",  0x10000, 0x10000, CRC(bb974684) SHA1(aa8dff82ceb397904cb3a6887541c5b83a372f41) )
	ROM_LOAD( "01.ic37",  0x20000, 0x10000, CRC(26c8bf72) SHA1(59ffe2da97f3596fef2b071532c06295a71d6988) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstb_820_nvram.bin",  0x0000, 0x1000, CRC(b7c2537a) SHA1(883febf6d0122ad974af5751de83d1a823a5a4fb) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstb_820_nvram2.bin", 0x0000, 0x0800, CRC(ce18880f) SHA1(b56245154673f65bbc6de003ec5b93bbd85e480d) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.ic46",  0x0000, 0x0200, CRC(18d89004) SHA1(a09bead0eca1757a385e2b605473f56c05088fc4) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )

	ROM_REGION( 0x0200, "plds", 0 )
	ROM_LOAD( "gal16v8_box.ic1", 0x0000, 0x0117, CRC(4c7af826) SHA1(3c78dae1606fc95292306ae5f9bd3bff4172ccf1) ) // cracked
ROM_END

/*
  Fruit Star Bonus (Ver 8.27PVIE)

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

*/
ROM_START( fruitstba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_827_box.ic2",  0x8000, 0x8000, CRC(c41fb59d) SHA1(7f8cde7a33aae83baae2564b8913c43502e7b5b7) )
	ROM_IGNORE(                       0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "3.ic39",  0x00000, 0x10000, CRC(81ad7752) SHA1(b5be715c9c1e0c20f63bfb2f9dcbadafe758a098) )
	ROM_LOAD( "2.ic38",  0x10000, 0x10000, CRC(0a7126aa) SHA1(28a05ca824922bb362b86f25f300833db30a91ae) )
	ROM_LOAD( "1.ic37",  0x20000, 0x10000, CRC(eaa231ad) SHA1(6c6d708a05b23b8b3f1f5844a587ae6d06940486) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstba_827_nvram.bin",  0x0000, 0x1000, CRC(90326cbd) SHA1(cc6c105e3faec88dd3b2f81d1d853d30b154ecdf) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstba_827_nvram2.bin", 0x0000, 0x0800, CRC(cc5f35e4) SHA1(03a451b8f6ac3849732e3567d6b5111df418e5af) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.20PIR)

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

*/
ROM_START( fruitstbb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_820_box.ic2",  0x8000, 0x8000, CRC(95d4ddaa) SHA1(498f841b3cd12ac128954841dd463b62c335e038) )
	ROM_IGNORE(                       0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fruit3.ic39",  0x00000, 0x10000, CRC(311a6d4e) SHA1(62cf670b605906f7f4225905118524ee30d0e85b) )
	ROM_LOAD( "fruit2.ic38",  0x10000, 0x10000, CRC(32d282a8) SHA1(792174d75dc7ec5f1e6f145539a5ec8e3953e1dd) )
	ROM_LOAD( "fruit1.ic37",  0x20000, 0x10000, CRC(c1834a6d) SHA1(ece1e47641087be342d3c5c092d8a7233ae871f3) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbb_820_nvram.bin",  0x0000, 0x1000, CRC(90f4cf2e) SHA1(cb63a4cc8461ff993c00449dc4559c0fd8e70de6) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbb_820_nvram2.bin", 0x0000, 0x0800, CRC(12edfdef) SHA1(b20b992db0189dfa063cfac7444881a3251b5f92) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.36UNG-1100)

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  The game displays 8.30UNG-200 in the service mode)

  Even when the version is higher in number,
  program and graphics are older.

*/
ROM_START( fruitstbc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_830_836_box.ic2",  0x8000, 0x8000, CRC(e647adc3) SHA1(6042bf12c6dac567d70a2cf3fbea0086f4e33e8f) )
	ROM_CONTINUE(                         0x8000, 0x8000 )  // first half has program v8.30UNG-200. second half has program v8.36UNG-1100.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fru-neu_3.ic39",  0x00000, 0x10000, CRC(de5f03c8) SHA1(340ccc63c5c29d1c34e63cdf18e71e5c4a42967a) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.
	ROM_LOAD( "fru-neu_2.ic38",  0x10000, 0x10000, CRC(df1094e9) SHA1(3746c6564ba77cc2d8eb191ef08f29d7298ef75e) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.
	ROM_LOAD( "fru-neu_1.ic37",  0x20000, 0x10000, CRC(0da9fa11) SHA1(85539287a1930d0da0d1a58c1894b37e09a2a378) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbc_836_nvram.bin",  0x0000, 0x1000, CRC(4d43b56a) SHA1(0dabb3d5eeed4b48e248794317711fc8ff856201) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbc_836_nvram2.bin", 0x0000, 0x0800, CRC(6b3bd2f9) SHA1(f2fa7299e81c0b709f3e37a0eb497da5a714ffb4) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.30UNG-200)

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  Even when the version is higher in number,
  program and graphics are older.

*/
ROM_START( fruitstbd )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_830_836_box.ic2",  0x8000, 0x8000, CRC(e647adc3) SHA1(6042bf12c6dac567d70a2cf3fbea0086f4e33e8f) )
	ROM_IGNORE(                                   0x8000 )  // first half has program v8.30UNG-200. second half has program v8.36UNG-1100.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fru-neu_3.ic39",  0x00000, 0x10000, CRC(de5f03c8) SHA1(340ccc63c5c29d1c34e63cdf18e71e5c4a42967a) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.
	ROM_LOAD( "fru-neu_2.ic38",  0x10000, 0x10000, CRC(df1094e9) SHA1(3746c6564ba77cc2d8eb191ef08f29d7298ef75e) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.
	ROM_LOAD( "fru-neu_1.ic37",  0x20000, 0x10000, CRC(0da9fa11) SHA1(85539287a1930d0da0d1a58c1894b37e09a2a378) )
	ROM_IGNORE(                           0x10000 )  // duplicated halves.

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbd_830_nvram.bin",  0x0000, 0x1000, CRC(6483b841) SHA1(cb48063b842b04f5ab34dce515c564e0ee596deb) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbd_830_nvram2.bin", 0x0000, 0x0800, CRC(72e5fadb) SHA1(1960c3d9c9a06a98579fca86cf16e8e99ffc3df5) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "82s147.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus
  Ver 8.30UNG-25 -  MPU11: 9562

  Program flash ROM is inside a CPU epoxy block
  with a M6809 CPU and a 16L8 PLD.

  Set 1

*/
ROM_START( fruitstbh )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_box.ic2",  0x8000, 0x8000, CRC(92432c84) SHA1(67f62053767ed5ca6ae5fd22b239131aefb1f258) )
	ROM_CONTINUE(                 0x8000, 0x8000 )  // first half has program v8.30UNG-200. second half has program v8.36UNG-1100.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "f-neu3.ic39",  0x00000, 0x10000, CRC(809bd675) SHA1(2df1222260cfbc646c336599134ba0b8f7aa58ff) )
	ROM_LOAD( "f-neu2.ic38",  0x10000, 0x10000, CRC(6478e395) SHA1(838c52e1a7117b15b91d7098e20bbcbfd5e9bce8) )
	ROM_LOAD( "f-neu1.ic37",  0x20000, 0x10000, CRC(85641001) SHA1(f99240f2d9b947a525fdd7545c1a5d285806d374) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbh_830ung-25_nvram.bin",  0x0000, 0x1000, CRC(8fc5db4e) SHA1(8d85b799e70a867f67842a0cc1eb34358f200336) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbh_830ung-25_nvram2.bin", 0x0000, 0x0800, CRC(8473d004) SHA1(63a3b57f853589154ed2f5f255a2b28cbaedda84) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "28s42.ic46",   0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus
  Ver 8.30UNG-25 -  MPU11: 9562

  Program flash ROM is inside a CPU epoxy block
  with a M6809 CPU and a 16L8 PLD.

  Set 2

*/
ROM_START( fruitstbi )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_box.ic2",  0x8000, 0x8000, CRC(92432c84) SHA1(67f62053767ed5ca6ae5fd22b239131aefb1f258) )
	ROM_IGNORE(                           0x8000 )  // first half has program v8.30UNG-200. second half has program v8.36UNG-1100.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "f-neu3.ic39",  0x00000, 0x10000, CRC(809bd675) SHA1(2df1222260cfbc646c336599134ba0b8f7aa58ff) )
	ROM_LOAD( "f-neu2.ic38",  0x10000, 0x10000, CRC(6478e395) SHA1(838c52e1a7117b15b91d7098e20bbcbfd5e9bce8) )
	ROM_LOAD( "f-neu1.ic37",  0x20000, 0x10000, CRC(85641001) SHA1(f99240f2d9b947a525fdd7545c1a5d285806d374) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbi_830ung-25_nvram.bin",  0x0000, 0x1000, CRC(d639901f) SHA1(252601d2c8632ba39108b77a162a0c51523f5f5f) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbi_830ung-25_nvram2.bin", 0x0000, 0x0800, CRC(f6385903) SHA1(be81f09fb77a69a2198c3fb45ee7111a80d3511e) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "28s42.ic46",   0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.23PSTK, Steiermark)
  MPU11 Number: 6218.
  Date: 1/99?

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  STK => Steiermark, Austria.

*/
ROM_START( fruitstbe )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "am28f512_8.23pstk_box.ic2",  0x8000, 0x8000, CRC(7c719e75) SHA1(67218db219eb4b7c229c66cce45dbf55fb594ff9) )
	ROM_IGNORE(                                     0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )  // double sized roms.
	ROM_LOAD( "fu_3.ic39",  0x00000, 0x10000, CRC(70efbf69) SHA1(32bb019db5aaff4d3f3f98a30cdca422cca7b598) )
	ROM_IGNORE(                      0x10000 )    // identical halves.
	ROM_LOAD( "fu_2.ic38",  0x10000, 0x10000, CRC(82c196b8) SHA1(ea1a74a6b13dbea253a804b88f22bd124fb1a3e6) )
	ROM_IGNORE(                      0x10000 )    // identical halves.
	ROM_LOAD( "fu_1.ic37",  0x20000, 0x10000, CRC(11ee9747) SHA1(19931a8c99e4c521cd7aed42398a9557a0d7579a) )
	ROM_IGNORE(                      0x10000 )    // identical halves.

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbe_823_nvram.bin",  0x0000, 0x1000, CRC(4255e21c) SHA1(055f2e1e507993184a15ec3133fbb1d8fafd6c22) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbe_823_nvram2.bin", 0x0000, 0x0800, CRC(ad6d8b59) SHA1(79de1b8e44bb2fc8e8dbd3210c0d1d26e641da4b) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 1)
  MPU11 Number: 9650.

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  BGL => Burgenland, Austria.

*/
ROM_START( fruitstbf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_817bgl-3_box.ic2",  0x8000, 0x8000, CRC(9562ab0b) SHA1(e60bec86d9ad8f41f726efa29279b467a72d600d) )
	ROM_IGNORE(                                    0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fu_3.ic39",  0x00000, 0x10000, CRC(81ad7752) SHA1(b5be715c9c1e0c20f63bfb2f9dcbadafe758a098) )
	ROM_LOAD( "fu_2.ic38",  0x10000, 0x10000, CRC(0a7126aa) SHA1(28a05ca824922bb362b86f25f300833db30a91ae) )
	ROM_LOAD( "fu_1.ic37",  0x20000, 0x10000, CRC(eaa231ad) SHA1(6c6d708a05b23b8b3f1f5844a587ae6d06940486) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbf_817_nvram.bin",  0x0000, 0x1000, CRC(08dd5a3e) SHA1(f8f6775a0ece56ffd9c1c2e3384f2da405cc6c60) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbf_817_nvram2.bin", 0x0000, 0x0800, CRC(0907a8dd) SHA1(14b54a753aef94543ee97c5022f5664863eebb8a) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 2)
  MPU11 Number: 9651. (program settings shows 5960)
  Date: 3/97.

  Program flash ROM is inside a CPU epoxy block
  with M6809 CPU and one PLD.

  BGL => Burgenland, Austria.

  Program has 4 bytes different to the other set.

*/
ROM_START( fruitstbg )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "m28f512_8.17bgl-3_box.ic2",  0x8000, 0x8000, CRC(3a11823d) SHA1(7b7c21ad0f2f877274e0b99c8eb74398685705ae) )
	ROM_IGNORE(                                     0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fu3.ic39",  0x00000, 0x10000, CRC(81ad7752) SHA1(b5be715c9c1e0c20f63bfb2f9dcbadafe758a098) )
	ROM_LOAD( "fu2.ic38",  0x10000, 0x10000, CRC(0a7126aa) SHA1(28a05ca824922bb362b86f25f300833db30a91ae) )
	ROM_LOAD( "fu1.ic37",  0x20000, 0x10000, CRC(eaa231ad) SHA1(6c6d708a05b23b8b3f1f5844a587ae6d06940486) )

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbg_817_nvram.bin",  0x0000, 0x1000, CRC(ff3e8883) SHA1(a6603d825b8298cba8b16005485a57acdd6243c0) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "fruitstbg_817_nvram2.bin", 0x0000, 0x0800, CRC(6efbfb43) SHA1(0a6e36ea47683baa2b502ea9e17e1847cee65b45) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "am27s29.ic46",  0x0000, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END



/*
  Golden Joker
  Version 16.06UNG-25
  26-01-1996 - MPU Nº 4346

  Program flash ROM is inside a CPU epoxy block
  with a M6809 CPU and a 16L8 PLD.

  Set 1

*/
ROM_START( goldnjkr )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512.box",  0x8000, 0x8000, CRC(9eed5cf0) SHA1(d3f130e2229da1ec9429507b2b2ea52a9f74d3a6) )
	ROM_CONTINUE(             0x8000, 0x8000 )  // using second half

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "goldenjoker_zg3.ic39",  0x00000, 0x10000, CRC(7e5b66e6) SHA1(ed1f704adea6fc251b3b85b1c8660676c16225c2) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.
	ROM_LOAD( "goldenjoker_zg2.ic38",  0x10000, 0x10000, CRC(3db7032c) SHA1(2f60050b68003e81f945369ada98681f2aa371c1) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.
	ROM_LOAD( "goldenjoker_zg1.ic37",  0x20000, 0x10000, CRC(be0c62a9) SHA1(e4355cc9895ca03fb8de59fb0ee1765059f26b4e) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "goldnjkr_nvram.bin",  0x0000, 0x1000, CRC(89681d3a) SHA1(923e7bf29d68e0707275e6d66b6d8821e6c2584c) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "goldnjkr_nvram2.bin", 0x0000, 0x0800, CRC(f33601ca) SHA1(246555d62d5a5584d708a80036c244b33cd6002f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp28s42.ic46",  0x0000, 0x0200, CRC(18d89004) SHA1(a09bead0eca1757a385e2b605473f56c05088fc4) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "am27s13.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END

/*
  Golden Joker
  Version 16.06UNG-25
  26-01-1996 - MPU Nº 4346

  Program flash ROM is inside a CPU epoxy block
  with a M6809 CPU and a 16L8 PLD.

  Set 2

*/
ROM_START( goldnjkra )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512.box",  0x8000, 0x8000, CRC(9eed5cf0) SHA1(d3f130e2229da1ec9429507b2b2ea52a9f74d3a6) )
	ROM_IGNORE(                       0x8000 )  // using first half

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "goldenjoker_zg3.ic39",  0x00000, 0x10000, CRC(7e5b66e6) SHA1(ed1f704adea6fc251b3b85b1c8660676c16225c2) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.
	ROM_LOAD( "goldenjoker_zg2.ic38",  0x10000, 0x10000, CRC(3db7032c) SHA1(2f60050b68003e81f945369ada98681f2aa371c1) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.
	ROM_LOAD( "goldenjoker_zg1.ic37",  0x20000, 0x10000, CRC(be0c62a9) SHA1(e4355cc9895ca03fb8de59fb0ee1765059f26b4e) )
	ROM_IGNORE(                                 0x10000 )  // duplicated halves.

	ROM_REGION( 0x1000, "nvram", 0 )    // first 0x1000 of the battery backed MB8464A-10L
	ROM_LOAD( "goldnjkra_nvram.bin",  0x0000, 0x1000, CRC(89681d3a) SHA1(923e7bf29d68e0707275e6d66b6d8821e6c2584c) )

	ROM_REGION( 0x0800, "nvram2", 0 )    // last 0x0800 of the battery backed MB8464A-10L
	ROM_LOAD( "goldnjkra_nvram2.bin", 0x0000, 0x0800, CRC(f33601ca) SHA1(246555d62d5a5584d708a80036c244b33cd6002f) )

	ROM_REGION( 0x0200, "proms", 0 )
	ROM_LOAD( "tbp28s42.ic46",  0x0000, 0x0200, CRC(18d89004) SHA1(a09bead0eca1757a385e2b605473f56c05088fc4) )

	ROM_REGION( 0x0200, "proms2", 0 )
	ROM_LOAD( "am27s13.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
ROM_END


/********************************
*          Driver Init          *
********************************/

void mpu12wbk_state::init_mpu12wbk()
{
	// just in case...
}

} // anonymous namespace


/********************************
*         Game Drivers          *
********************************/

//     YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT    COMPANY             FULLNAME                                              FLAGS   LAYOUT
GAMEL( 199?, fruitstb,  0,        mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.2.00ITL)",                    0,      layout_fruitstb )
GAMEL( 199?, fruitstba, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.27PVIE)",                     0,      layout_fruitstb )
GAMEL( 1997, fruitstbb, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.20PIR)",                      0,      layout_fruitstb )
GAMEL( 1996, fruitstbc, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.36UNG-1100)",                 0,      layout_fruitstb )
GAMEL( 1996, fruitstbd, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.30UNG-200)",                  0,      layout_fruitstb )
GAMEL( 1996, fruitstbh, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.30UNG-25, set 1)",            0,      layout_fruitstb )
GAMEL( 1996, fruitstbi, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.30UNG-25, set 2)",            0,      layout_fruitstb )
GAMEL( 1999, fruitstbe, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.23PSTK, Steiermark)",         0,      layout_fruitstb )
GAMEL( 1997, fruitstbf, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 1)", 0,      layout_fruitstb )
GAMEL( 1997, fruitstbg, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.17BGL-3, Burgenland, set 2)", 0,      layout_fruitstb )
GAMEL( 1996, goldnjkr,  0,        mpu12wbk, goldnjkr, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Golden Joker (Ver 16.06UNG-25, set 1)",               0,      layout_goldnjkr )
GAMEL( 1996, goldnjkra, goldnjkr, mpu12wbk, goldnjkr, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Golden Joker (Ver 16.06UNG-25, set 2)",               0,      layout_goldnjkr )
