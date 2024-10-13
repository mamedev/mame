// license:BSD-3-Clause
// copyright-holders:Roberto Fresca
/******************************************************************************

    Unknown TMS9980 Poker Game  <--- We need the real name & confirm the manufacturer! :)
    --------------------------

    Driver by Roberto Fresca.


    Games running on this hardware:

    * Unknown TMS9980 Poker Game,  198?,  Jeutel?
    * Las Vegas, 198?, Jeutel (two sets)


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


  Both program ROMs are sharing the same addressing space.
  The switch placed below the program ROMs is connected to each PD /PGM line,
  so you can switch between 2 different programs.

  - Data lines are inverted according to TI convention (Q1-Q8 --> D7-D0).

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

#include "emu.h"
#include "cpu/tms9900/tms9980a.h"
#include "machine/74259.h"
#include "sound/sn76477.h"
#include "video/mc6845.h"
#include "emupal.h"
#include "screen.h"
#include "tilemap.h"


namespace {

class tmspoker_state : public driver_device
{
public:
	tmspoker_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_videoram(*this, "videoram"),
		m_gamebank(*this, "gamebank"),
		m_maincpu(*this, "maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_outlatch(*this, "outlatch%u", 0U),
		m_inputs(*this, "IN%u", 0U)
	{ }

	void tmspoker(machine_config &config);

	void init_bus();

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_shared_ptr<uint8_t> m_videoram;
	required_memory_bank m_gamebank;
	tilemap_t *m_bg_tilemap;
	required_device<cpu_device> m_maincpu;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<ls259_device, 4> m_outlatch;
	required_ioport_array<3> m_inputs;

	void videoram_w(offs_t offset, uint8_t data);
	[[maybe_unused]] void debug_w(uint8_t data);
	uint8_t inputs_r(offs_t offset);
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(interrupt);

	void cru_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};


/*************************
*     Video Hardware     *
*************************/

void tmspoker_state::videoram_w(offs_t offset, uint8_t data)
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

	tileinfo.set(0 /* bank */, code, 0 /* color */, 0);
}

void tmspoker_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(tmspoker_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 32, 32);
}

uint32_t tmspoker_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

void tmspoker_state::palette(palette_device &palette) const
{
}


/**************************
*  Read / Write Handlers  *
**************************/

void tmspoker_state::debug_w(uint8_t data)
{
	popmessage("written : %02X", data);
}

INTERRUPT_GEN_MEMBER(tmspoker_state::interrupt)
{
	m_maincpu->set_input_line(INT_9980A_LEVEL1, ASSERT_LINE);
	// MZ: The TMS9980A uses level-triggered interrupts, so this
	// interrupt must somehow be cleared later. Clearing the line
	// immediately is not effective, since the interrupt is not latched
	// and will be lost.
	// In addition, the TMS99xx processors do not offer an interrupt
	// acknowledge output, so this is possibly done by using a CRU port.
	// Needs further investigation of the ROM contents.
}


/************************
*     Start & Reset     *
************************/

void tmspoker_state::machine_start()
{
	m_gamebank->configure_entries(0, 2, memregion("maincpu")->base(), 0x1000);
}


void tmspoker_state::machine_reset()
{
	uint8_t seldsw = (ioport("SELDSW")->read() );

	popmessage("ROM Bank: %02X", seldsw);

	m_gamebank->set_entry(seldsw);
}


/*************************
* Memory Map Information *
*************************/
//59a

// MZ: The driver locks up in this loop until it gets an interrupt on level 2.
//     This is obviously missing.
//
//     0982:     CLR  @>2040               04E0 2040
//     0986:     MOV  @>2040,@>2040        C820 2040 2040
//     098C:     JEQ  >0986                13FC
//     098E:     RT                        045B
//

void tmspoker_state::prg_map(address_map &map)
{
	map.global_mask(0x3fff);
	map(0x0000, 0x0fff).bankr(m_gamebank);
	map(0x2800, 0x2800).nopr().w("crtc", FUNC(mc6845_device::address_w));
	map(0x2801, 0x2801).rw("crtc", FUNC(mc6845_device::register_r), FUNC(mc6845_device::register_w));
	map(0x3000, 0x33ff).ram().w(FUNC(tmspoker_state::videoram_w)).share(m_videoram);
	map(0x3800, 0x3fff).ram(); //NVRAM?
	map(0x2000, 0x20ff).ram(); //color RAM?
}


uint8_t tmspoker_state::inputs_r(offs_t offset)
{
	uint8_t q = m_outlatch[2]->output_state();
	for (int n = 0; n < 3; n++)
		if (BIT(q, 4 + n) && !BIT(m_inputs[n]->read(), offset))
			return 0;

	return 1;
}

void tmspoker_state::cru_map(address_map &map)
{
	map(0x0c80, 0x0c8f).w(m_outlatch[0], FUNC(ls259_device::write_d0));
	map(0x0c90, 0x0c9f).r(FUNC(tmspoker_state::inputs_r)).w(m_outlatch[1], FUNC(ls259_device::write_d0));
	map(0x0ca0, 0x0caf).w(m_outlatch[2], FUNC(ls259_device::write_d0));
	map(0x0cb0, 0x0cbf).w(m_outlatch[3], FUNC(ls259_device::write_d0));
}

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
	RGN_FRAC(1,1),  // 256 tiles
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 // every char takes 8 consecutive bytes
};


/******************************
* Graphics Decode Information *
******************************/

static GFXDECODE_START( gfx_tmspoker )
	GFXDECODE_ENTRY( "chars", 0, charlayout, 0, 16 )
GFXDECODE_END


/*************************
*    Machine Drivers     *
*************************/

void tmspoker_state::tmspoker(machine_config &config)
{
	// CPU TMS9980A; no line connections
	TMS9980A(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &tmspoker_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &tmspoker_state::cru_map);
	m_maincpu->set_vblank_int("screen", FUNC(tmspoker_state::interrupt));

	LS259(config, m_outlatch[0]);
	LS259(config, m_outlatch[1]);
	LS259(config, m_outlatch[2]);
	LS259(config, m_outlatch[3]);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(32*8, 32*8);
	screen.set_visarea(0*8, 32*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(tmspoker_state::screen_update));

	GFXDECODE(config, m_gfxdecode, "palette", gfx_tmspoker);
	PALETTE(config, "palette", FUNC(tmspoker_state::palette), 256);

	mc6845_device &crtc(MC6845(config, "crtc", 6_MHz_XTAL / 4)); // guess
	crtc.set_screen("screen");
	crtc.set_show_border_area(false);
	crtc.set_char_width(8);
}


/*************************
*        Rom Load        *
*************************/

#define TMSPOKER_BIOS \
	ROM_REGION( 0x0800, "chars", 0 ) \
	ROM_LOAD( "3.bin",  0x0000, 0x0800, CRC(55458dae) SHA1(bf96d1b287292ff89bc2dbd9451a88a2ab941f3e) ) \
	ROM_REGION( 0x0100, "proms", 0 ) \
	ROM_LOAD( "tibp22.bin", 0x0000, 0x0100, CRC(74ec6bbd) SHA1(3ab696506de03c69d9f40d49c5fc6d3ac6601acf) )

ROM_START( tmspoker )
	ROM_REGION( 0x4000, "maincpu", 0 ) // TMS9980 selectable code
	ROM_LOAD( "0.bin",  0x0800, 0x0800, CRC(a20ae6cb) SHA1(d47780119b4ebb16dc759a50dfc880ddbc6a1112) )  // Program 1
	ROM_CONTINUE(       0x0000, 0x0800 )
	ROM_LOAD( "8.bin",  0x1800, 0x0800, CRC(0c0a7159) SHA1(92cc3dc32a5bf4a7fa197e72c3931e583c96ef33) )  // Program 2
	ROM_CONTINUE(       0x1000, 0x0800 )

	TMSPOKER_BIOS
ROM_END

/*
'tmspoker2' and 'tmspoker3' were removable carts, having the PCB a slot instead of the ROM sockets found on 'tmspoker' (but the
PCB was the same on every other component). Each game cart had 2 2716 EPROMs, and the main PCB was silkscreened as "POKER".
Both 'tmspoker2' and 'tmspoker3' seems like different sets of the same game, having only 22 different bytes, where the value always
change by adding 8, and only on addresses ending by 'E'.

         Sea Pk
0000012E: 2A 22
0000013E: 2C 24
0000014E: 3E 36
0000015E: FA F2
0000017E: E8 E0
0000018E: 4C 44
000001AE: 19 11
000001BE: 2A 22
0000020E: 0B 03
0000022E: 2C 24
0000026E: 2C 24
0000027E: 2C 24
000002CE: 2D 25
000002DE: 2C 24
000002EE: 2F 27
0000031E: 2C 24
0000034E: 2C 24
0000039E: DA D2
000003BE: 2D 25
0000044E: 2C 24
0000045E: 2D 25
0000046E: 2C 24
*/

ROM_START( jlasvegas )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "sea_1_2716.bin", 0x0800, 0x0800, CRC(e49ce839) SHA1(07e2cc5129825c3c241890b7d75ddb7a7af7fef1) )
	ROM_LOAD( "sea_2_2716.bin", 0x1800, 0x0800, CRC(d675a0a6) SHA1(b9d2efcb33f9ffde394cf3360fb863cd8a7e946d) )

	TMSPOKER_BIOS
ROM_END

ROM_START( jlasvegasa )
	ROM_REGION( 0x4000, "maincpu", 0 )
	ROM_LOAD( "pk4_2716.bin", 0x0800, 0x0800, CRC(e49ce839) SHA1(07e2cc5129825c3c241890b7d75ddb7a7af7fef1) )
	ROM_LOAD( "pk5_2716.bin", 0x1800, 0x0800, CRC(f9756009) SHA1(88bfb5482510751a2f7e6bb7824342f2469aeea1) )

	TMSPOKER_BIOS
ROM_END


/***************************
*       Driver Init        *
***************************/

void tmspoker_state::init_bus()
{
	// still need to decode the addressing lines
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

} // Anonymous namespace


/*************************
*      Game Drivers      *
*************************/

//    YEAR  NAME        PARENT     MACHINE   INPUT     STATE           INIT      ROT   COMPANY    FULLNAME                      FLAGS
GAME( 198?, tmspoker,   0,         tmspoker, tmspoker, tmspoker_state, init_bus, ROT0, "Jeutel?", "unknown TMS9980 poker game", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 198?, jlasvegas,  0,         tmspoker, tmspoker, tmspoker_state, init_bus, ROT0, "Jeutel",  "Las Vegas (Jeutel, set 1)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 198?, jlasvegasa, jlasvegas, tmspoker, tmspoker, tmspoker_state, init_bus, ROT0, "Jeutel",  "Las Vegas (Jeutel, set 2)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
