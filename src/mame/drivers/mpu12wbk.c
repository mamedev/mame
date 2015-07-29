// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/****************************************************************************************

  WEBAK MPU-12 PLATFORM
  ---------------------

  Preliminary driver by Roberto Fresca.


  Games running on this hardware:

  * Fruit Star Bonus.   1997, Webak Elektronik.


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

  PCB Layout:

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


  [2012-10-10]

  - Initial release.
  - Added technical notes.


  TODO:

  - A lot of work.


*****************************************************************************************/


#define MASTER_CLOCK    XTAL_8MHz

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/mc6845.h"
#include "sound/ay8910.h"
//#include "machine/nvram.h"


class mpu12wbk_state : public driver_device
{
public:
	mpu12wbk_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_colorram(*this, "colorram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	required_shared_ptr<UINT8> m_colorram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(mpu12wbk_videoram_w);
	DECLARE_WRITE8_MEMBER(mpu12wbk_colorram_w);
	DECLARE_DRIVER_INIT(mpu12wbk);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void video_start();
	DECLARE_PALETTE_INIT(mpu12wbk);
	UINT32 screen_update_mpu12wbk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************
*     Video Hardware     *
*************************/


WRITE8_MEMBER(mpu12wbk_state::mpu12wbk_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


WRITE8_MEMBER(mpu12wbk_state::mpu12wbk_colorram_w)
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

//  SET_TILE_INFO_MEMBER(0, code, color, 0);
	SET_TILE_INFO_MEMBER(0, 0 ,0 ,0);
}


void mpu12wbk_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(mpu12wbk_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}


UINT32 mpu12wbk_state::screen_update_mpu12wbk(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}


PALETTE_INIT_MEMBER(mpu12wbk_state, mpu12wbk)
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

static ADDRESS_MAP_START( mpu12wbk_map, AS_PROGRAM, 8, mpu12wbk_state )
	AM_RANGE(0x1400, 0x1400) AM_DEVWRITE("crtc", mc6845_device, address_w)                      // OK
	AM_RANGE(0x1401, 0x1401) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)     // OK
	AM_RANGE(0x1e00, 0x1e01) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, address_data_w)  // hmmmmm....
	AM_RANGE(0x2000, 0x23ff) AM_RAM_WRITE(mpu12wbk_videoram_w) AM_SHARE("videoram")             // FIXME
	AM_RANGE(0x2400, 0x27ff) AM_RAM_WRITE(mpu12wbk_colorram_w) AM_SHARE("colorram")             // FIXME
	AM_RANGE(0x2800, 0x3fff) AM_RAM                                                             // RAM (from 2000-3fff)
	AM_RANGE(0x6000, 0x6000) AM_READ_PORT("SW1")    // dummy, placeholder
	AM_RANGE(0x6001, 0x6001) AM_READ_PORT("SW2")    // dummy, placeholder
	AM_RANGE(0x6002, 0x6002) AM_READ_PORT("IN0")    // dummy, placeholder
	AM_RANGE(0x6003, 0x6003) AM_READ_PORT("IN1")    // dummy, placeholder
	AM_RANGE(0x6004, 0x6004) AM_READ_PORT("IN2")    // dummy, placeholder
	AM_RANGE(0x6005, 0x6005) AM_READ_PORT("IN3")    // dummy, placeholder

	AM_RANGE(0x8000, 0xffff) AM_ROM     // OK
ADDRESS_MAP_END

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
//  8, 8,
//  RGN_FRAC(3,3),
//  1,      /* 1 bpp */
//  { 0 },
//  { 0, 1, 2, 3, 4, 5, 6, 7 },
//  { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
//  8*8 /* every char takes 8 consecutive bytes */

//  8, 8,
//  RGN_FRAC(1,3),
//  3,
//  { 0, RGN_FRAC(1,3), RGN_FRAC(2,3) },    /* bitplanes are separated */
//  { 0, 1, 2, 3, 4, 5, 6, 7 },
//  { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
//  8*8

	4,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(0,2), RGN_FRAC(0,2) + 4, RGN_FRAC(1,2), RGN_FRAC(1,2) + 4 },
	{ 3, 2, 1, 0 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*4*2

};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( mpu12wbk )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END

/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( mpu12wbk, mpu12wbk_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/2)  /* guess */
	MCFG_CPU_PROGRAM_MAP(mpu12wbk_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", mpu12wbk_state,  nmi_line_pulse)

//  MCFG_NVRAM_ADD_0FILL("nvram")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE((32+1)*8, (32+1)*8)                  /* From MC6845, registers 00 & 04. (value-1) */
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)    /* Driven by MC6845, registers 01 & 06 */
	MCFG_SCREEN_UPDATE_DRIVER(mpu12wbk_state, screen_update_mpu12wbk)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", mpu12wbk)
	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_INIT_OWNER(mpu12wbk_state, mpu12wbk)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/4) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(4)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay8910", AY8910, MASTER_CLOCK/8)        /* guess */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/
/*

  Program is inside a CPU epoxy block
  with a m6809 and PLD.

  Version 8.20PIR

*/
ROM_START( fruitstb )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "p28f512_box.ic2",    0x8000, 0x8000, CRC(95d4ddaa) SHA1(498f841b3cd12ac128954841dd463b62c335e038) )
	ROM_IGNORE(                     0x8000 ) // second half is filled with 0xff, vectors are at the end of the 1st half

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "fruit1.ic37",    0x00000, 0x10000, CRC(c1834a6d) SHA1(ece1e47641087be342d3c5c092d8a7233ae871f3) )
	ROM_LOAD( "fruit2.ic38",    0x10000, 0x10000, CRC(32d282a8) SHA1(792174d75dc7ec5f1e6f145539a5ec8e3953e1dd) )
//  ROM_LOAD( "fruit3.ic39",    0x20000, 0x10000, CRC(311a6d4e) SHA1(62cf670b605906f7f4225905118524ee30d0e85b) )  // and this one?

	ROM_REGION( 0x0400, "proms", 0 )
	ROM_LOAD( "82s131.ic47", 0x0000, 0x0200, CRC(54565d41) SHA1(8e412a3441c9c1e7f8309f2087389ac4250896e6) )
	ROM_LOAD( "82s147.ic46", 0x0200, 0x0200, CRC(ee576268) SHA1(8964526fa253f484d784aec46c4c31358bc1667b) )

ROM_END


/************************
*      Driver Init      *
************************/

DRIVER_INIT_MEMBER(mpu12wbk_state, mpu12wbk)
{
	// just in case...
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     STATE           INIT      ROT    COMPANY             FULLNAME                         FLAGS */
GAME( 1997, fruitstb, 0,      mpu12wbk, mpu12wbk, mpu12wbk_state, mpu12wbk, ROT0, "Webak Elektronik", "Fruit Star Bonus (Ver 8.20PIR)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
