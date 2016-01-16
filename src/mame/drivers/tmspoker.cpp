// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

    Unknown TMS9980 Poker Game  <--- We need the real name & manufacturer! :)
    --------------------------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Unknown TMS9980 Poker Game,  198?,  Unknown.


*******************************************************************************


  Hardware Notes:
  ---------------

  PCB Layout:

  .--------------------------------------------------------------------------------------------------------.
  |          .------------------.  .-------------------.                                                   |
  | .-----.  |                  |  |                   |                                                   '-.
  | | [8] |  |    TMS9980ANL    |  |      MC6845P      |                                                   --|
  | |     |  |                  |  |                   |                                                   --|
  | |     |  '------------------'  '-------------------'                                             2x22  --|
  | | TMS |                                                                                          Edge  --|
  | |2532 |                                                                                          Conn  --|
  | |     |  .---------.                                                                                   --|
  | '-----'  |    A    | .-------.  .-------.  .-------.                                                   --|
  |          |         | |   B   |  |   D   |  |   B   |                                                   --|
  |          '---------' '-------'  '-------'  '-------'                                                   --|
  | .-----.  .---------.                                                                                   --|
  | | [0] |  |    A    | .-------.  .-------.  .-------.   .-------.                                       --|
  | |     |  |         | |   G   |  |   B   |  |   B   |   |   V   |                                       --|
  | |     |  '---------' '-------'  '-------'  '-------'   '-------'                                       --|
  | | TMS |                                                                                                --|
  | |2532 |  .-------.   .-------.             .-------.   .-------.  .-------.                            .-'
  | |     |  |   C   |   |   E   |             |   M   |   |   P   |  |   U   |                            |
  | '-----'  '-------'   '-------'             '-------'   '-------'  '-------'                            |
  |          .-------.   .-------.  .-------.  .-------.   .-------.  .-------.                            |
  |     .-.  |   C   |   |   H   |  |   K   |  |   N   |   |   P   |  |   U   |                            |
  |  SW | |  '-------'   '-------'  '-------'  '-------'   '-------'  '-------' .--------------.           |
  |    C==|  .-------.   .-------.  .-------.                         .-------. |              |           |
  |     '-'  |   C   |   |   F   |  |   F   |                         |   U   | |   SN76477N   |           |
  |          '-------'   '-------'  '-------'             .----------.'-------' |              |           |
  |          .-------.   .-------.    6MHz     .-------.  |   [3]    |          '--------------'           |
  |          |   I   |   |   J   |   .----.    |   O   |  | TMS2516  |                        .-------.    |
  |          '-------'   '-------'    XTAL     '-------'  '----------'                        |   U   |    |
  |                      .-------.  .-------.  .-------.   .-------.                          '-------'    |
  |                      |   H   |  |   J   |  |   P   |   |   Q   |                                       |
  |                      '-------'  '-------'  '-------'   '-------'           .--.                        |
  |                      .-------.  .-------.  .-------.   .-------.           |  |                        |
  |                      |   L   |  |   W   |  |   P   |   |   Q   |           |S |                        |
  |                      '-------'  '-------'  '-------'   '-------'           |  |                        |
  |                                        _     .------.   .-------.     _    '--'                        |
  |                                      /47k\   | 3.6V |   |   R   |   /1k \                              |
  |                                     | POT |  |      |   '-------'  | POT |                             |
  |                                      \ _ /   | BATT |               \ _ /                              |
  |                                              '------'                                                  |
  |                                                                                                        |
  '--------------------------------------------------------------------------------------------------------'

  1x Xtal @ 6 MHz.

  A = MWS5101EL2        M = P2114AL-4
  B = SN74LS367AN       N = L2114-30B
  C = SN74LS138N        O = 8223A / TBP28L22N
  D = SN74LS357N        P = SN74LS175N
  E = SN74LS74N         Q = DM74195N
  F = SN74LS04N         R = LM380N
  G = ON74148N          S = MC1723CP / L8201
  H = SN74LS02N         T = SN74L57N
  I = CD40938E          U = SN74LS259N
  J = SN74LS00N         V = SN74LS251J
  K = SN74LS245N        W = DM74157N
  L = ON7407N


*******************************************************************************


  *** Game Notes ***



*******************************************************************************

  --------------------
  ***  Memory Map  ***
  --------------------

  0x0000 - 0x0FFF    ; ROM space.
  0x???? - 0x????    ; Video RAM.


*******************************************************************************

  TMS9980A memory map:


  0000-0003 ---> Reset
  0004-0007 ---> Level 1
  0008-000B ---> Level 2
  000C-000F ---> Level 3
  0010-0013 ---> Level 4

  3FFC-3FFF ---> Load


*******************************************************************************

  Diagrams:
  ---------
                               TMS9980ANL
                       .-----------\ /-----------.
                    ---|01 /HOLD        /MEMEN 40|---
                    ---|02 HOLDA         READY 39|---
                    ---|03 IAQ             /WE 38|---
  SN74LS367 (pin2) ----|04 A13/CRUOUT   CRUCLK 37|---
  SN74LS367 (pin4) ----|05 A12             VDD 36|---
  SN74LS367 (pin10) ---|06 A11             VSS 35|--- VSS
  SN74LS367 (pin12) ---|07 A10            CKIN 34|--- SN74LS04 (pin10)
  SN74LS367 (pin14) ---|08 A9               D7 33|--- TMS2532 (Q1)
  TMS2532 (A5) --------|09 A8               D6 32|--- TMS2532 (Q2)
  TMS2532 (A6) --------|10 A7               D5 31|--- TMS2532 (Q3)
  TMS2532 (A7) --------|11 A6               D4 30|--- TMS2532 (Q4)
  TMS2532 (A8) --------|12 A5               D3 29|--- TMS2532 (Q5)
  TMS2532 (A9) --------|13 A4               D2 28|--- TMS2532 (Q6)
  TMS2532 (A10) -------|14 A3               D1 27|--- TMS2532 (Q7)
  SN74LS138 (pin1) ----|15 A2               D0 26|--- TMS2532 (Q8)
  SN74LS138 (pin2) ----|16 A1             INT0 25|---
  SN74LS138 (pin3) ----|17 A0             INT1 24|---
                    ---|18 DBIN           INT2 23|---
                    ---|19 CRUIN    /CLKPHASE3 22|---
                VCC ---|20 VCC             VBB 21|---
                       '-------------------------'


                             TMS2532JL
                       .--------\ /--------.
  TMS9980 (A6) --------|01 A7        VCC 24|--- VCC
  TMS9980 (A7) --------|02 A6         A8 23|--- TMS9980 (A5)
  TMS9980 (A8) --------|03 A5         A9 22|--- TMS9980 (A4)
  SN74LS367 (pin13) ---|04 A4        VPP 21|---
  SN74LS367 (pin11) ---|05 A3    PD /PGM 20|--- SELECTOR SWITCH
  SN74LS367 (pin9) ----|06 A2        A10 19|--- TMS9980 (A3)
  SN74LS367 (pin5) ----|07 A1        A11 18|--- SN74LS138 (pin14)
  SN74LS367 (pin3) ----|08 A0         Q8 17|--- TMS9980 (D0)
  TMS9980 (D7) --------|09 Q1         Q7 16|--- TMS9980 (D1)
  TMS9980 (D6) --------|10 Q2         Q6 15|--- TMS9980 (D2)
  TMS9980 (D5) --------|11 Q3         Q5 14|--- TMS9980 (D3)
                VSS ---|12 VSS        Q4 13|--- TMS9980 (D4)
                       '-------------------'


  Both program ROMs are sharing the same adressing space.
  The switch placed below the program ROMs is connected to each PD /PGM line,
  so you can switch between 2 different programs.

  Both programs seems to be encrypted.

  - Data lines are clearly inverted (Q1-Q8 --> D7-D0).

  - Address lines are scrambled:

    CPU    pin  device  pin   2532
    ------------------------------
    A13 - (02)SN74LS367(03) - A00 \
    A12 - (04)SN74LS367(05) - A01  |
    A11 - (10)SN74LS367(09) - A02  > Passing through buffers.
    A10 - (12)SN74LS367(11) - A03  |
    A09 - (14)SN74LS367(13) - A04 /
    A08 --------------------- A05
    A07 --------------------- A06
    A06 --------------------- A07
    A05 --------------------- A08
    A04 --------------------- A09
    A03 --------------------- A10
    A02 - (01)SN74LS138 \
    A01 - (02)SN74LS138  > 8 demuxed lines...
    A00 - (03)SN74LS138 /


*******************************************************************************


  DRIVER UPDATES:


  [2011-10-10]

  - Initial release.
  - Added technical notes.
  - Added ROM banking.
  - Data lines decoded.



  TODO:

  - A lot...


*******************************************************************************/


#define MASTER_CLOCK    XTAL_6MHz   /* confirmed */

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "video/mc6845.h"
#include "sound/sn76477.h"


class tmspoker_state : public driver_device
{
public:
	tmspoker_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode") { }

	required_shared_ptr<UINT8> m_videoram;
	tilemap_t *m_bg_tilemap;
	DECLARE_WRITE8_MEMBER(tmspoker_videoram_w);
	//DECLARE_WRITE8_MEMBER(debug_w);
	DECLARE_READ8_MEMBER(unk_r);
	DECLARE_DRIVER_INIT(bus);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	DECLARE_PALETTE_INIT(tmspoker);
	UINT32 screen_update_tmspoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(tmspoker_interrupt);
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
};


/*************************
*     Video Hardware     *
*************************/

WRITE8_MEMBER(tmspoker_state::tmspoker_videoram_w)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}


TILE_GET_INFO_MEMBER(tmspoker_state::get_bg_tile_info)
{
/*  - bits -
    7654 3210
    ---- ----   bank select.
    ---- ----   color code.
    ---- ----   seems unused.
*/
	int code = m_videoram[tile_index];

	SET_TILE_INFO_MEMBER(0 /* bank */, code, 0 /* color */, 0);
}

void tmspoker_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(m_gfxdecode, tilemap_get_info_delegate(FUNC(tmspoker_state::get_bg_tile_info),this), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

UINT32 tmspoker_state::screen_update_tmspoker(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

PALETTE_INIT_MEMBER(tmspoker_state, tmspoker)
{
}


/**************************
*  Read / Write Handlers  *
**************************/

//WRITE8_MEMBER(tmspoker_state::debug_w)
//{
//  popmessage("written : %02X", data);
//}

INTERRUPT_GEN_MEMBER(tmspoker_state::tmspoker_interrupt)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL1, ASSERT_LINE); //_and_vector(0, ASSERT_LINE, 3);//2=nmi  3,4,5,6
	m_maincpu->set_input_line(INT_9980A_LEVEL1, CLEAR_LINE);  // MZ: do we need this?
}


/************************
*     Start & Reset     *
************************/

void tmspoker_state::machine_start()
{
	UINT8 *ROM = memregion("maincpu")->base();
	membank("bank1")->configure_entries(0, 2, &ROM[0], 0x1000);
}


void tmspoker_state::machine_reset()
{
	UINT8 seldsw = (ioport("SELDSW")->read() );

	popmessage("ROM Bank: %02X", seldsw);

	membank("bank1")->set_entry(seldsw);
}


/*************************
* Memory Map Information *
*************************/
//59a
static ADDRESS_MAP_START( tmspoker_map, AS_PROGRAM, 8, tmspoker_state )
	ADDRESS_MAP_GLOBAL_MASK(0x3fff)
	AM_RANGE(0x0000, 0x0fff) AM_ROMBANK("bank1")
	AM_RANGE(0x2800, 0x2800) AM_DEVWRITE("crtc", mc6845_device, address_w)
	AM_RANGE(0x2801, 0x2801) AM_DEVREADWRITE("crtc", mc6845_device, register_r, register_w)
	AM_RANGE(0x3000, 0x33ff) AM_WRITE(tmspoker_videoram_w) AM_SHARE("videoram")
	AM_RANGE(0x3800, 0x3fff) AM_RAM //NVRAM?
	AM_RANGE(0x2000, 0x20ff) AM_RAM //color RAM?
ADDRESS_MAP_END


READ8_MEMBER(tmspoker_state::unk_r)
{
	printf("%x\n",offset);
	return 0;//0xff;//mame_rand(machine);
}

static ADDRESS_MAP_START( tmspoker_cru_map, AS_IO, 8, tmspoker_state )
	AM_RANGE(0x0000, 0xffff) AM_READ(unk_r)
ADDRESS_MAP_END

/* I/O byte R/W


   -----------------

   unknown writes:


*/

/*************************
*      Input Ports       *
*************************/

static INPUT_PORTS_START( tmspoker )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("DSW3")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("DSW4")
	PORT_DIPNAME( 0x01, 0x01, DEF_STR( Unknown ) )
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

	PORT_START("SELDSW")
	PORT_DIPNAME( 0x01, 0x00, "Game Selector" )
	PORT_DIPSETTING(    0x00, "Game 1" )
	PORT_DIPSETTING(    0x01, "Game 2" )

INPUT_PORTS_END


/*************************
*    Graphics Layouts    *
*************************/

static const gfx_layout charlayout =
{
	8, 8,
	RGN_FRAC(1,1),  /* 256 tiles */
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( tmspoker )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

static MACHINE_CONFIG_START( tmspoker, tmspoker_state )

	// CPU TMS9980A; no line connections
	MCFG_TMS99xx_ADD("maincpu", TMS9980A, MASTER_CLOCK/4, tmspoker_map, tmspoker_cru_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", tmspoker_state,  tmspoker_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(32*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 32*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(tmspoker_state, screen_update_tmspoker)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", tmspoker)

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_INIT_OWNER(tmspoker_state, tmspoker)

	MCFG_MC6845_ADD("crtc", MC6845, "screen", MASTER_CLOCK/4) /* guess */
	MCFG_MC6845_SHOW_BORDER_AREA(false)
	MCFG_MC6845_CHAR_WIDTH(8)

MACHINE_CONFIG_END


/*************************
*        Rom Load        *
*************************/

ROM_START( tmspoker )
	ROM_REGION( 0x4000, "maincpu", 0 ) /* TMS9980 selectable code */

	ROM_LOAD( "0.bin",  0x0000, 0x1000, CRC(a20ae6cb) SHA1(d47780119b4ebb16dc759a50dfc880ddbc6a1112) )  /* Program 1 */
	ROM_LOAD( "8.bin",  0x1000, 0x1000, CRC(0c0a7159) SHA1(92cc3dc32a5bf4a7fa197e72c3931e583c96ef33) )  /* Program 2 */

	ROM_REGION( 0x0800, "gfx1", 0 )
	ROM_LOAD( "3.bin",  0x0000, 0x0800, CRC(55458dae) SHA1(bf96d1b287292ff89bc2dbd9451a88a2ab941f3e) )

	ROM_REGION( 0x0100, "proms", 0 )
	ROM_LOAD( "tibp22.bin", 0x0000, 0x0100, CRC(74ec6bbd) SHA1(3ab696506de03c69d9f40d49c5fc6d3ac6601acf) )
ROM_END


/***************************
*       Driver Init        *
***************************/

DRIVER_INIT_MEMBER(tmspoker_state,bus)
{
	/* decode the TMS9980 ROMs */

	// MZ: Does not make sense to swap the bit order, so I commented it out.
	// Only when unswapped do the commands make sense; otherwise there is a lot
	// of invalid opcodes, and the RESET vector at 0000 is invalid either.

/*  offs_t offs;
    UINT8 *rom = memregion("maincpu")->base();
    const size_t len = memregion("maincpu")->bytes();

    for (offs = 0; offs < len; offs++)
    {
        rom[offs] = BITSWAP8(rom[offs],0,1,2,3,4,5,6,7);
    }
*/

	/* still need to decode the addressing lines */
	/* text found in the ROM (A at 6, B at 8, etc: consistent with gfx rom byte offsets) suggests
	   that the lower address lines are good already:

	ROM offset    TEXT
	-----------------------
	$0914-$0919   POINTS
	$091e-$0920   BET
	$0924-$0927   GAME
	$092c-$092f   OVER
	$0934-$0938   RESET
	$093c-$0941   WINNER
	$0946-$0949   TEST

	the same 53 bytes blob of data ("POINTS" to "TEST", text control codes included) is also located at offset $190c-$1941
	*/
}


/*************************
*      Game Drivers      *
*************************/

/*    YEAR  NAME      PARENT  MACHINE   INPUT     INIT  ROT    COMPANY      FULLNAME                     FLAGS */
GAME( 198?, tmspoker, 0,      tmspoker, tmspoker, tmspoker_state, bus,  ROT0, "<unknown>", "unknown TMS9980 Poker Game", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
