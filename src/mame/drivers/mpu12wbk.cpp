// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/****************************************************************************************

  WEBAK MPU-12 PLATFORM
  ---------------------

  Preliminary driver by Roberto Fresca.


  Games running on this hardware:

  * Fruit Star Bonus (Ver 8.27PVIE).  199?, Webak Elektronik.
  * Fruit Star Bonus (Ver 8.20PIR).   1997, Webak Elektronik.


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

  Xtal:   1x 8MHz.


*****************************************************************************************

  PCB Layout (Ver 8.27PVIE) :

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

  A00 = PCF1251P


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


  ººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººººº


  PCB Layout (Ver 8.20PIR) :

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

  A00 = PCF1251P



  DIP 1:
  +---------------+
  | |#|#|#|#|#| | |
  |---------------|
  |#| | | | | |#|#|
  +---------------+
   1 2 3 4 5 6 7 8



  EPOXY BLOCK - COVER:
  +--------------------------------------------------------------------------------+
  |                                                      +-----------------------+ |
  |      #######################################         |        VERSION        | |
  |  #  ##                                     ##  #     |       Fruit STK       | |
  |   # ##  #   #  #####  ####    ###   #   #  ## #      +-----------------------+ |
  |    ###  #   #  #      #   #  #   #  #  #   ###                                 |
  |  #####  # # #  ###    ####   #####  ###    #####     +-----------------------+ |
  |    ###  # # #  #      #   #  #   #  #  #   ###       | NO:      6***         | |
  |   # ##   # #   #####  ####   #   #  #   #  ## #      | --------------------- | |
  |  #  ##                                     ##  #     | DATE:                 | |
  |      #######################################         | --------------------- | |
  |                                                      | CUST:                 | |
  |              SCHWANENSTADT - AUSTRIA                 +-----------------------+ |
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
  +--------------------------------------------------------------------------------+


  PINOUT:

                     (WEBAK CONNECTION Standard MPU12/MPU2000)

  +----------------------------------------------------------------------------------+
  |           Component-Side               ||              Solder-Side               |
  +----------------------------------------------------------------------------------+
  |       Function       | Direction | Nr. || Nr. | Direction |       Function       |
  +==================================================================================+
  | HOPPER-OUT           |  OUTPUT   | 10  ||  J  |  OUTPUT   |                      |
  | REMOTE-PL            |  OUTPUT   | 09  ||  I  |  OUTPUT   |                      |
  | REMOTE-CLOCK         |  OUTPUT   | 08  ||  H  |  OUTPUT   | REMOTE-DOUT          |
  | Lamp HOPPER-OUT      |  OUTPUT   | 07  ||  G  |  INPUT    | REMOTE-IN            |
  | REMOTE-SELECT        |  INPUT    | 06  ||  F  |  INPUT    |                      |
  |                      |  INPUT    | 05  ||  E  |  INPUT    | Bookkeeping 3        |
  |                      |  INPUT    | 04  ||  D  |  INPUT    | Button "Select Game" |
  |                      |           | 03  ||  C  |           |                      |
  | EX64-SELECT          |  OUTPUT   | 02  ||  B  |  OUTPUT   | Lamp "Select Game"   |
  | EXTRA-Lamp           |  Output   | 01  ||  A  |  OUTPUT   | reserved             |
  +----------------------------------------------------------------------------------+
  +----------------------------------------------------------------------------------+
  | GND                  |  SUPPLY   | 22  ||  Z  |  SUPPLY   | GND                  |
  | GND                  |  SUPPLY   | 21  ||  Y  |  SUPPLY   | GND                  |
  | GND                  |  SUPPLY   | 20  ||  X  |  SUPPLY   | GND                  |
  | +5V                  |  SUPPLY   | 19  ||  W  |  SUPPLY   | +5V                  |
  | +12V                 |  SUPPLY   | 18  ||  V  |  SUPPLY   | +12V                 |
  | LAMP - HOLD 1        |  OUTPUT   | 17  ||  U  |  OUTPUT   | LAMP - START         |
  | LAMP - HOLD 2        |  OUTPUT   | 16  ||  T  |  OUTPUT   | LAMP - HOLD 5        |
  | LAMP - CANCEL        |  OUTPUT   | 15  ||  S  |  OUTPUT   | LAMP - HOLD 4        |
  | COIN - INPUT 1       |  INPUT    | 14  ||  R  |  OUTPUT   | LAMP - HOLD 3/Printer|
  | Mech. Counter-IN     |  OUTPUT   | 13  ||  P  |  INPUT    | BOOKKEEPING 1        |
  | Mech. Counter-OUT    |  OUTPUT   | 12  ||  N  |  INPUT    | Button HOLD 1        |
  | Mech. Counter-3      |  OUTPUT   | 11  ||  M  |  INPUT    | Button CANCEL        |
  | Button HOLD 5        |  INPUT    | 10  ||  L  |  INPUT    | Button START         |
  | Bookkeeping 2        |  INPUT    | 09  ||  K  |  INPUT    | Bookkeeping A(Waiter)|
  | Button HOLD 2        |  INPUT    | 08  ||  J  |  INPUT    | Button HOLD 4        |
  | Coin INPUT 3         |  INPUT    | 07  ||  H  |  INPUT    | Button HOLD 3/Printer|
  | HOPPER COUNT         |  INPUT    | 06  ||  F  |  INPUT    | EXTRA Button         |
  | Button HOPPER OUT    |  INPUT    | 05  ||  E  |  INPUT    | Coin - INPUT 2       |
  | Monitor GREEN        |TTLOUT-Anal| 04  ||  D  |TTLOUT-Anal| Monitor RED          |
  | Monitor SYNC         |TTLOUT-Anal| 03  ||  C  |TTLOUT-Anal| Monitor BLUE         |
  | SPEAKER              |OUT-Analog | 02  ||  B  |  SUPPLY   | Monitor GND          |
  | CREDIT CLEAR         |  INPUT    | 01  ||  A  |  SUPPLY   | SPEAKER GND          |
  +----------------------------------------------------------------------------------+


*****************************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x1400 - 0x1401    ; M6845 CRTC.  OK
  0x1800 - 0x1801    ; ????.
  0x1E00 - 0x1E01    ; AY-3-8910?.
  0x???? - 0x????    ; Video RAM.
  0x???? - 0x????    ; Color RAM.
  0x2000 - 0x3FFF    ; RAM.         OK
  0x8000 - 0xFFFF    ; ROM space.   OK


  *** CRTC Initialization ***
  ----------------------------------------------------------------------------------------------------------------------
  register:  R00   R01   R02   R03   R04   R05   R06   R07   R08   R09   R10   R11   R12   R13   R14   R15   R16   R17
  ----------------------------------------------------------------------------------------------------------------------
  value:     0x7F  0x60  0x6A  0x88  0x22  0x08  0x20  0x20  0x00  0x07  0x01  0x01  0x00  0x00  0x00  0x00  0x00  0x00.


*****************************************************************************************


  DRIVER UPDATES:


  [2022-01-09]

  - Added a new parent.
  - Improved gfxdecode. Added two extra bitplanes.
  - Added more ASCII layouts and technical notes.


  [2012-10-10]

  - Initial release.
  - Added technical notes.


  TODO:

  - A lot of work.


*****************************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
//#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"


#define MASTER_CLOCK    XTAL(8'000'000)


class mpu12wbk_state : public driver_device
{
public:
	mpu12wbk_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode")
	{ }

	void mpu12wbk(machine_config &config);

	void init_mpu12wbk();

protected:
	virtual void video_start() override;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_shared_ptr<uint8_t> m_colorram;
	tilemap_t *m_bg_tilemap;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;

	void mpu12wbk_videoram_w(offs_t offset, uint8_t data);
	void mpu12wbk_colorram_w(offs_t offset, uint8_t data);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void mpu12wbk_palette(palette_device &palette) const;
	uint32_t screen_update_mpu12wbk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void mpu12wbk_map(address_map &map);
};


/*************************
*     Video Hardware     *
*************************/


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
    ---- ----   bank select.
    ---- ----   color code.
*/
//  int attr = m_colorram[tile_index];
//  int code = m_videoram[tile_index] | ((attr & 0xc0) << 2);
//  int color = (attr & 0x0f);

//  tileinfo.set(0, code, color, 0);
	tileinfo.set(0, 0 ,0 ,0);
}


void mpu12wbk_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(mpu12wbk_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


uint32_t mpu12wbk_state::screen_update_mpu12wbk(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


void mpu12wbk_state::mpu12wbk_palette(palette_device &palette) const
{
}


/*************************
*      Machine Init      *
*************************/


/*****************************
*    Read/Write  Handlers    *
*****************************/


/*************************
* Memory Map Information *
*************************/

void mpu12wbk_state::mpu12wbk_map(address_map &map)
{
	map(0x1400, 0x1400).w("crtc", FUNC(mc6845_device::address_w));                      // OK
	map(0x1401, 0x1401).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));     // OK
	map(0x1e00, 0x1e01).rw("ay8910", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));  // hmmmmm....
	map(0x2000, 0x23ff).ram().w(FUNC(mpu12wbk_state::mpu12wbk_videoram_w)).share("videoram");             // FIXME
	map(0x2400, 0x27ff).ram().w(FUNC(mpu12wbk_state::mpu12wbk_colorram_w)).share("colorram");             // FIXME
	map(0x2800, 0x3fff).ram();                                                             // RAM (from 2000-3fff)
	map(0x6000, 0x6000).portr("SW1");    // dummy, placeholder
	map(0x6001, 0x6001).portr("SW2");    // dummy, placeholder
	map(0x6002, 0x6002).portr("IN0");    // dummy, placeholder
	map(0x6003, 0x6003).portr("IN1");    // dummy, placeholder
	map(0x6004, 0x6004).portr("IN2");    // dummy, placeholder
	map(0x6005, 0x6005).portr("IN3");    // dummy, placeholder

	map(0x8000, 0xffff).rom();     // OK
}

/*

unknown writes:

1400-1401  CRTC

1800-1801 R (input?)
1e00-1e01 RW (psg?)

*/


/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( mpu12wbk )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-1") PORT_CODE(KEYCODE_1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-2") PORT_CODE(KEYCODE_2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-3") PORT_CODE(KEYCODE_3)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-4") PORT_CODE(KEYCODE_4)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-5") PORT_CODE(KEYCODE_5)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-6") PORT_CODE(KEYCODE_6)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-7") PORT_CODE(KEYCODE_7)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("0-8") PORT_CODE(KEYCODE_8)

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-1") PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-2") PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-3") PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-4") PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-5") PORT_CODE(KEYCODE_T)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-6") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-7") PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("1-8") PORT_CODE(KEYCODE_I)

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-1") PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-2") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-3") PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-4") PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-5") PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-6") PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-7") PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("2-8") PORT_CODE(KEYCODE_K)

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-1") PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-2") PORT_CODE(KEYCODE_X)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-3") PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-4") PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-5") PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-6") PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-7") PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_NAME("3-8") PORT_CODE(KEYCODE_L)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x01, "SW1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("SW2")
	PORT_DIPNAME( 0x01, 0x01, "SW2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	4,8,
	RGN_FRAC(1,3),
	6,
	{ RGN_FRAC(0,3), RGN_FRAC(0,3) + 4, RGN_FRAC(1,3), RGN_FRAC(1,3) + 4, RGN_FRAC(2,3), RGN_FRAC(2,3) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2

};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_mpu12wbk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void mpu12wbk_state::mpu12wbk(machine_config &config)
{
	/* basic machine hardware */
	MC6809(config, m_maincpu, MASTER_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &mpu12wbk_state::mpu12wbk_map);

//  NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size((32+1)*8, (32+1)*8);             // From MC6845, registers 00 & 04. (value-1)
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);    // Driven by MC6845, registers 01 & 06
	screen.set_screen_update(FUNC(mpu12wbk_state::screen_update_mpu12wbk));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_mpu12wbk);
	PALETTE(config, "palette", FUNC(mpu12wbk_state::mpu12wbk_palette), 512);

	mc6845_device &crtc(MC6845(config, "crtc", MASTER_CLOCK / 4));  // clock guessed
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(4);
	crtc.out_vsync_callback().set_inputline(m_maincpu, INPUT_LINE_NMI);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	AY8910(config, "ay8910", MASTER_CLOCK / 8).add_route(ALL_OUTPUTS, "mono", 1.00);   // clock guessed
}


/*************************
*        Rom Load        *
*************************/

/*
  Program is inside a CPU epoxy block
  with a m6809 and PLD.

  Version 8.27PVIE
*/
ROM_START( fruitstb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_827_box.ic2",  0x8000, 0x8000, CRC(c41fb59d) SHA1(7f8cde7a33aae83baae2564b8913c43502e7b5b7) )
	ROM_IGNORE(                       0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "1.ic37",  0x00000, 0x10000, CRC(eaa231ad) SHA1(6c6d708a05b23b8b3f1f5844a587ae6d06940486) )
	ROM_LOAD( "2.ic38",  0x10000, 0x10000, CRC(0a7126aa) SHA1(28a05ca824922bb362b86f25f300833db30a91ae) )
	ROM_LOAD( "3.ic39",  0x20000, 0x10000, CRC(81ad7752) SHA1(b5be715c9c1e0c20f63bfb2f9dcbadafe758a098) )  // two extra bitplanes.

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
	ROM_LOAD( "82s147.ic46",  0x0200, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

ROM_END

/*
  Program is inside a CPU epoxy block
  with a m6809 and PLD.

  Version 8.20PIR
*/
ROM_START( fruitstba )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_820_box.ic2",  0x8000, 0x8000, CRC(95d4ddaa) SHA1(498f841b3cd12ac128954841dd463b62c335e038) )
	ROM_IGNORE(                       0x8000 )  // second half is filled with 0xff, vectors are at the end of the 1st half.

	ROM_REGION( 0x30000, "gfx1", 0 )
	ROM_LOAD( "fruit1.ic37",  0x00000, 0x10000, CRC(c1834a6d) SHA1(ece1e47641087be342d3c5c092d8a7233ae871f3) )
	ROM_LOAD( "fruit2.ic38",  0x10000, 0x10000, CRC(32d282a8) SHA1(792174d75dc7ec5f1e6f145539a5ec8e3953e1dd) )
	ROM_LOAD( "fruit3.ic39",  0x20000, 0x10000, CRC(311a6d4e) SHA1(62cf670b605906f7f4225905118524ee30d0e85b) )           // two extra bitplanes.

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s131.ic47",  0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
	ROM_LOAD( "82s147.ic46",  0x0200, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

ROM_END


/************************
*      Driver Init      *
************************/

void mpu12wbk_state::init_mpu12wbk()
{
	// just in case...
}


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME       PARENT    MACHINE   INPUT     STATE           INIT           ROT    COMPANY             FULLNAME                          FLAGS
GAME( 199?, fruitstb,  0,        mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.27PVIE)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1997, fruitstba, fruitstb, mpu12wbk, mpu12wbk, mpu12wbk_state, init_mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.20PIR)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
