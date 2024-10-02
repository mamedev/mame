// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Robbbert
/******************************************************************************************************
PINBALL
Alvin G pinball machines.

Undumped PAL16L8 (U29) on PCA-002 sound card.
Undumped PAL16L8 (U104) on PCA-008 sound card.

Pinball games:
- Al's Garage Band Goes on a World Tour (1992)
- Dual-Pool (1993, unreleased)
- Monte Carlo
- Mystery Castle (1993)
- Pistol Poker (1993)
- Punchy the Clown (1993)
- Slam 'n Jam (1993, unreleased)
- Tropical Isle
Other games:
- A-MAZE-ING Baseball (1994, redemption)
- A G Football (1992, head to head)
- A G Soccer-ball (1991, head to head)
- Dinosaur Eggs (1993, redemption)
- The Death Dealing Adventure of Maxx Badazz Champion Kickboxer (1993, unreleased)
- USA Football (1994, redemption) No flippers, 12 gobble holes, need to get the ball in a lit hole, or into the outhole.
- USA Football (1992, head to head)

Here are the key codes to enable play:

Game                                     NUM    Start game                          End ball
-------------------------------------------------------------------------------------------------
A G Soccer-Ball                           01    Hold N, hit 8                       (timed game)
Al's Garage Band Goes On A World Tour     03    Hold ABC, hit 1                     ABC
USA Football (head to head)               05    Hold N and Quote, hit 8             (timed game)
Punchy the Clown                          06    Hold EF, hit 1                      (mnw)
Dinosaur Eggs                             07    (mnw)
Mystery Castle                            08    Hold CDE, hit 1                     (mnw)
Pistol Poker                              10    Hold CDE, hit 1                     (mnw)
USA Football (redemption)                 11    Hold A, hit 1                       (mnw)

Status:
- A G Soccer, USA Football (head to head): Playable.
- Other machines are sometimes workable, but will sooner or later run into the weeds and fail.
- punchy: display gets corrupt, can't be started
- punchy3: can sometimes run, but soon goes berserk
- dinoeggs: usually goes berzerk at start
- usafootf: sometimes can work, sometimes corrupt display and freeze at start
- dmd column and row offsets obtained from dmdcpu are wrong, even though it's coded from schematics
- wrldtour: mostly playable, but random key pressing can freeze it. End of game can freeze too.
- pstlpkr: sometimes can start a game, but can't end the ball. In the end it freezes.
- mystcast: usually can't even start up. Randomly can start a game, but then nothing.
- pstlpkr, mystcast: show 0 credits at startup, even if there's some from last run.
- usafootf: loses all saved credits at start.

ToDo:
- Mechanical sound
- Find out why the random freezing (happens most of the time)
- msystcasta: DMD CPU rom seems bad (rubbish 08FE-0B60)
- wrldtour, wrldtour2: instructions fly past too fast to be read. wrldtour3 is ok.
- pstlpkr, mystcast: no attract mode.
- mystcast: Uses PCA-020A, a modified DMD controller, to be emulated (schematic hasn't been updated)
- The schematic of PCA-021 (DMD display board) does not appear in any manual.

****************************************************************************************************/
#include "emu.h"
#include "genpin.h"
#include "cpu/m6502/m65c02.h"
#include "cpu/m6809/m6809.h"
#include "cpu/mcs51/mcs51.h"
#include "machine/6522via.h"
#include "machine/clock.h"
#include "machine/i8255.h"
#include "machine/input_merger.h"
#include "sound/bsmt2000.h"
#include "sound/okim6295.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "speaker.h"
#include "screen.h"
#include "alvg.lh"


namespace {

class alvg_state : public genpin_class
{
public:
	alvg_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ppi0(*this, "ppi0")
		, m_ppi1(*this, "ppi1")
		, m_ppi2(*this, "ppi2")
		, m_ppi3(*this, "ppi3")
		, m_via0(*this, "via0")
		, m_via1(*this, "via1")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki")
		, m_bsmt(*this, "bsmt")
		, m_vias(*this, "vias")
		, m_dmdcpu(*this, "dmdcpu")
		, m_dmd(*this, "dmd")
		, m_vram(*this, "vram")
	{ }

	void alvg(machine_config &config);
	void group1(machine_config &config);
	void group2(machine_config &config);
	void group3(machine_config &config);
	void pca002(machine_config &config);  // Gen 1 sound board
	void pca003(machine_config &config);  // Alphanumeric display
	void pca008(machine_config &config);  // Gen 2 sound board
	void pca020(machine_config &config);  // DMD controller

private:
	void main_map(address_map &map) ATTR_COLD;
	void pca002_map(address_map &map) ATTR_COLD;
	void pca003_map(address_map &map) ATTR_COLD;
	void pca008_map(address_map &map) ATTR_COLD;
	void pca020_io_map(address_map &map) ATTR_COLD;
	void pca020_mem_map(address_map &map) ATTR_COLD;
	void machine_start() override ATTR_COLD;
	void machine_reset() override ATTR_COLD;
	void display_w(offs_t, u8);
	void ppi0_pa_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[i] = BIT(data, i); }
	void ppi0_pb_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[8U+i] = BIT(data, i); }
	void ppi0_pc_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[16U+i] = BIT(data, i); }
	void ppi1_pa_w(u8 data) { for (u8 i = 0; i < 8; i++) m_io_outputs[24U+i] = BIT(data, i); }
	void ppi1_pb_w(u8 data) { m_row = (m_row & 0xff00) | data; }
	void ppi1_pc_w(u8 data) { m_row = (m_row & 0xff) | (data << 8); }
	void ppi2_pa_w(u8 data) { m_lamp_data = (m_lamp_data & 0xff00) | data; }
	void ppi2_pb_w(u8 data) { m_lamp_data = (m_lamp_data & 0xff) | (data << 8); }
	void ppi2_pc_w(u8 data);
	void ppi3_pa_w(u8 data);
	void ppi3_pb_w(u8 data);
	void ppi3_pc_w(u8 data);
	void vias_pb_w(u8 data);
	u8 via0_pa_r();
	u8 via0_pb_r() { return m_io_keyboard[12]->read() | (BIT(m_dmd_stat, 0) << 7); }
	void via1_pa_w(u8 data);
	void via1_pb_w(u8 data);
	u16 m_row = 0U;
	u16 m_lamp_data = 0U;
	u8 m_strobe = 0U;

	// bsmt
	u8 rdstat_r() { return m_bsmt_rdcode; }
	u8 rdcode_r() { m_audiocpu->set_input_line(0, CLEAR_LINE); return m_cpu_to_bsmt; }
	void wrcode_w(u8 data) { m_bsmt_to_cpu = data;  m_via1->write_ca1(BIT(data, 0)); }
	void bsmt_w(offs_t offset, u8 data);
	void watch_w(u8 data) { if (!BIT(data, 6)) m_bsmt->reset(); }
	void bsmt_ready_w() { m_bsmt_rdcode |= 0x80; }
	u8 m_bsmt_addr = 0U;
	u8 m_bsmt_data = 0U;
	u8 m_bsmt_to_cpu = 0U;
	u8 m_cpu_to_bsmt = 0U;
	u8 m_bsmt_rdcode = 0xffU;

	// dmd
	u8 dmd_portin_r() { m_dmdcpu->set_input_line(MCS51_INT0_LINE, CLEAR_LINE); return m_dmd_cmd; }
	void dmd_portout_w(u8 data) { m_dmd_stat = data & 15; m_via1->write_pb(bitswap<8>(data, 7, 7, 1, 2, 3, 7, 7, 7)); }
	void dmd_codepage_w(u8 data) { m_dmd->set_entry(data & 15); }
	void dmd_rowstart_w(u8 data) { m_dmd_row = data; } // not working - always FF
	void dmd_colstart_w(u8 data) { m_dmd_col = data; } // not working - always 0
	void dmd_w(u8 data) { m_dmd_cmd = data; if (m_dmdcpu) m_dmdcpu->set_input_line(MCS51_INT0_LINE, ASSERT_LINE); }
	void dmd_port1_w(u8 data) { m_dmd_page = data & 15; }
	u8 dmd_port1_r() { return m_dmd_page; }
	void dmd_port3_w(u8 data) { m_dmd_plan = BIT(data, 0); }
	u8 m_dmd_cmd = 0xffU;
	u8 m_dmd_stat = 0xffU;
	u8 m_dmd_page = 0U;
	u8 m_dmd_row = 0U;
	u8 m_dmd_col = 0U;
	bool m_dmd_plan = false;
	u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi0;
	required_device<i8255_device> m_ppi1;
	required_device<i8255_device> m_ppi2;
	optional_device<i8255_device> m_ppi3;
	required_device<via6522_device> m_via0;
	required_device<via6522_device> m_via1;
	required_ioport_array<13> m_io_keyboard;
	output_finder<40> m_digits;
	output_finder<128> m_io_outputs;   // 32 solenoids + 96 lamps
	optional_device<cpu_device> m_audiocpu;
	optional_device<okim6295_device> m_oki;
	optional_device<bsmt2000_device> m_bsmt;
	optional_device<via6522_device> m_vias;
	optional_device<mcs51_cpu_device> m_dmdcpu;
	optional_memory_bank m_dmd;
	optional_shared_ptr<u8> m_vram;
};


void alvg_state::main_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x0000, 0x1fff).ram().share("nvram");
	map(0x2000, 0x2003).mirror(0x3f0).rw(m_ppi0, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U12
	map(0x2400, 0x2403).mirror(0x3f0).rw(m_ppi1, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U13
	map(0x2800, 0x2803).mirror(0x3f0).rw(m_ppi2, FUNC(i8255_device::read), FUNC(i8255_device::write)); // U14
	map(0x2c00, 0x2fff).w(FUNC(alvg_state::dmd_w));
	map(0x3800, 0x380f).mirror(0x3f0).m("via1", FUNC(via6522_device::map)); // U8
	map(0x3c00, 0x3c0f).mirror(0x3f0).m("via0", FUNC(via6522_device::map)); // U7
}

void alvg_state::pca003_map(address_map &map)
{
	main_map(map);
	map(0x2c00, 0x2c00).mirror(0x37f).w(FUNC(alvg_state::display_w));
	map(0x2c80, 0x2c83).mirror(0x37c).lrw8(NAME([this] (offs_t offset) -> u8 { return m_ppi3->read(offset^3); }), NAME([this] (offs_t offset, u8 data) { m_ppi3->write(offset^3, data); })); // IC1 on display board
}

void alvg_state::pca002_map(address_map &map)
{
	map(0x0000, 0xffff).rom();
	map(0x2000, 0x2001).mirror(0xffe).rw("ymsnd", FUNC(ym3812_device::read), FUNC(ym3812_device::write));
	map(0x3000, 0x37ff).mirror(0x800).ram();
	map(0x4000, 0x4fff).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0x5000, 0x500f).mirror(0xff0).m("vias", FUNC(via6522_device::map));
	map(0x6000, 0x6fff).noprw(); // watchdog
}

void alvg_state::pca008_map(address_map &map) // mapping done by U104
{
	map(0x0000, 0xffff).rom();
	map(0x0100, 0x0100).rw(FUNC(alvg_state::rdstat_r),FUNC(alvg_state::watch_w));
	map(0x0800, 0x0800).rw(FUNC(alvg_state::rdcode_r),FUNC(alvg_state::wrcode_w));
	map(0x1000, 0x10ff).w(FUNC(alvg_state::bsmt_w));
	map(0x2000, 0x3fff).ram();
}

void alvg_state::pca020_mem_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0x8000, 0xffff).bankr("dmd");
}

void alvg_state::pca020_io_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).ram().share("vram");
	map(0x8000, 0x8fff).r(FUNC(alvg_state::dmd_portin_r));
	map(0x9000, 0x9fff).w(FUNC(alvg_state::dmd_portout_w));
	map(0xa000, 0xafff).w(FUNC(alvg_state::dmd_codepage_w));
	map(0xb000, 0xbfff).w(FUNC(alvg_state::dmd_rowstart_w));
	map(0xc000, 0xcfff).w(FUNC(alvg_state::dmd_colstart_w));
}

static INPUT_PORTS_START( alvg )
	PORT_START("X0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN4 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("Left Flipper")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("Right Flipper")

	PORT_START("X1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP10") // Tilt on Mystery Castle, Punchy, Pistol Poker
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP11")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP12")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP13")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP14")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("Test+")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("Test-")

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP17")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP18")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP19")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP20")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP21")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP22")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP23")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP24")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP25")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP26")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP27")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP28")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP29")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP30")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP31")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP32")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP33")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP34")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP35")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP36")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP37")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP38")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP39")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP40")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP41")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP42")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP43")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP44")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP45")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP46")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP47")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP48")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP49")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_UP) PORT_NAME("INP50")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP51")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP52")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP53")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP54")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP55")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP56")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_END) PORT_NAME("INP57")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP58")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP59")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP60")
	// From here, these inputs only used by Pistol Poker
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP61")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("INP62")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("INP63")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("INP65")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("INP66")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("INP68")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("INP69")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("INP70")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("INP71")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CODE(KEYCODE_9_PAD) PORT_NAME("INP72")

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP73")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP74")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP75")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP76")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP77")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP78")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP79")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME("INP80")

	PORT_START("X10")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X11")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X12") // DIAGS
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("Enter")
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test") // Manual incorrectly says 0x20
INPUT_PORTS_END

void alvg_state::ppi2_pc_w(u8 data)
{
	for (u8 i = 0; i < 12; i++)
		if (BIT(m_lamp_data, i))
			for (u8 j = 0; j < 8; j++)
				m_io_outputs[24U+8*i+j] = BIT(data, j);
}

void alvg_state::ppi3_pa_w(u8 data)
{
	u16 t = m_digits[m_strobe] & 0xff00;
	m_digits[m_strobe] = t | data;
}

void alvg_state::ppi3_pb_w(u8 data)
{
	u16 t = m_digits[m_strobe] & 0xff;
	m_digits[m_strobe] = t | (bitswap<8>(data, 7, 6, 3, 2, 5, 4, 1, 0) << 8);
}

void alvg_state::ppi3_pc_w(u8 data)
{
	u16 t = m_digits[m_strobe+20] & 0xff00;
	m_digits[m_strobe+20] = t | data;
}

void alvg_state::display_w(offs_t offset, u8 data)
{
	u16 t = m_digits[m_strobe+20] & 0xff;
	m_digits[m_strobe+20] = t | (bitswap<8>(data, 7, 6, 3, 2, 5, 4, 1, 0) << 8);
}

void alvg_state::vias_pb_w(u8 data)
{
	m_via1->write_ca1(BIT(data, 1));
}

void alvg_state::via1_pa_w(u8 data)
{
	m_cpu_to_bsmt = data;
}

void alvg_state::via1_pb_w(u8 data)
{
	if (m_vias)
		m_vias->write_ca2(BIT(data, 1));

	if (m_bsmt)
		m_audiocpu->set_input_line(0, ASSERT_LINE);

	if ((data & 0x38)==0)
	{
		m_strobe++;
		if (m_strobe > 19)
			m_strobe = 0;
	}
	if (BIT(data, 5))
		m_strobe = 0;
}

u8 alvg_state::via0_pa_r()
{
	u8 data = 0xff;
	for (u8 i = 0; i < 12; i++)
		if (!BIT(m_row, i))
			data &= m_io_keyboard[i]->read();

	return data;
}

void alvg_state::bsmt_w(offs_t offset, u8 data)
{
	if (BIT(offset, 0))
	{
		m_bsmt->write_reg(m_bsmt_addr);
		m_bsmt->write_data((m_bsmt_data << 8) | data);
		m_bsmt_rdcode &= 0x7f;
	}
	else
	{
		m_bsmt_addr = offset >> 1;
		m_bsmt_data = data;
	}
}

u32 alvg_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	u16 sy = 0;
	u16 ma = m_dmd_page << 11;
	//if (m_dmd_plan)
		//ma += BIT(m_dmd_row, 5, 2) << 9;
	// m_dmd_row and m_dmd_col should give finer tuning, but getting bad data
	//ma += ((m_dmd_row & 0x1f) << 4) | BIT(m_dmd_col, 3, 4);

	for (u16 y = 0; y < 32; y++)
	{
		u16 *p = &bitmap.pix(sy++);

		for (u8 x = 0; x < 16; x++) // 128 dots across
		{
			// mystcast has 5-level brightness from 0,0x200,0x400,0x600 which need to be combined
			u8 gfx = m_vram[ma];

			*p++ = BIT(gfx, 7);
			*p++ = BIT(gfx, 6);
			*p++ = BIT(gfx, 5);
			*p++ = BIT(gfx, 4);
			*p++ = BIT(gfx, 3);
			*p++ = BIT(gfx, 2);
			*p++ = BIT(gfx, 1);
			*p++ = BIT(gfx, 0);
			ma++;
		}
	}
	return 0;
}

void alvg_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_row));
	save_item(NAME(m_lamp_data));
	save_item(NAME(m_strobe));
	save_item(NAME(m_bsmt_addr));
	save_item(NAME(m_bsmt_data));
	save_item(NAME(m_bsmt_to_cpu));
	save_item(NAME(m_cpu_to_bsmt));
	save_item(NAME(m_bsmt_rdcode));
	save_item(NAME(m_dmd_cmd));
	save_item(NAME(m_dmd_stat));
	save_item(NAME(m_dmd_page));
	save_item(NAME(m_dmd_row));
	save_item(NAME(m_dmd_col));
	save_item(NAME(m_dmd_plan));

	if (m_dmd)
		m_dmd->configure_entries(0, 16, memregion("dmd")->base(), 0x8000);
}

void alvg_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	m_strobe = 0U;
	if (m_dmd)
		m_dmd->set_entry(0);
}

void alvg_state::pca002(machine_config &config)
{
	MC6809(config, m_audiocpu, XTAL(8'000'000)); // 68B09, 8 MHz crystal, internal divide by 4 to produce E/Q outputs
	m_audiocpu->set_addrmap(AS_PROGRAM, &alvg_state::pca002_map);

	MOS6522(config, m_vias, XTAL(8'000'000) / 4);  // uses E clock from audiocpu; port A = read sound code; port B = ticket machine
	m_vias->writepb_handler().set(FUNC(alvg_state::vias_pb_w));
	m_vias->irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);

	SPEAKER(config, "mono").front_center();

	ym3812_device &ymsnd(YM3812(config, "ymsnd", XTAL(8'000'000) / 2));
	ymsnd.irq_handler().set_inputline(m_audiocpu, M6809_FIRQ_LINE);
	ymsnd.add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki, XTAL(8'000'000) / 8, okim6295_device::PIN7_HIGH);
	m_oki->add_route(ALL_OUTPUTS, "mono", 0.50);

	m_via1->writepa_handler().set(m_vias, FUNC(via6522_device::write_pa));
}

void alvg_state::pca003(machine_config &config)
{
	m_maincpu->set_addrmap(AS_PROGRAM, &alvg_state::pca003_map);

	/* Video */
	config.set_default_layout(layout_alvg);

	I8255A(config, m_ppi3); // U14
	m_ppi3->out_pa_callback().set(FUNC(alvg_state::ppi3_pa_w));
	m_ppi3->out_pb_callback().set(FUNC(alvg_state::ppi3_pb_w));
	m_ppi3->out_pc_callback().set(FUNC(alvg_state::ppi3_pc_w));
}

void alvg_state::pca008(machine_config &config)
{
	MC6809(config, m_audiocpu, XTAL(8'000'000)); // 68B09, 8 MHz crystal, internal divide by 4 to produce E/Q outputs
	m_audiocpu->set_addrmap(AS_PROGRAM, &alvg_state::pca008_map);
	m_via1->writepa_handler().set(FUNC(alvg_state::via1_pa_w));

	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	BSMT2000(config, m_bsmt, XTAL(24'000'000));
	m_bsmt->set_ready_callback(FUNC(alvg_state::bsmt_ready_w));
	m_bsmt->add_route(0, "lspeaker", 1.2);
	m_bsmt->add_route(1, "rspeaker", 1.2);

	CLOCK(config, "fclock", 2'000'000 / 4096).signal_handler().set_inputline(m_audiocpu, 1);
}

void alvg_state::pca020(machine_config &config)
{
	/* basic machine hardware */
	I8031(config, m_dmdcpu, XTAL(12'000'000));
	m_dmdcpu->set_addrmap(AS_PROGRAM, &alvg_state::pca020_mem_map);
	m_dmdcpu->set_addrmap(AS_IO, &alvg_state::pca020_io_map);
	m_dmdcpu->port_out_cb<1>().set(FUNC(alvg_state::dmd_port1_w));
	m_dmdcpu->port_in_cb<1>().set(FUNC(alvg_state::dmd_port1_r));
	m_dmdcpu->port_out_cb<3>().set(FUNC(alvg_state::dmd_port3_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_LCD, rgb_t::amber()));
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate */
	screen.set_size(128, 32);
	screen.set_visarea(0, 127, 0, 31);
	screen.set_screen_update(FUNC(alvg_state::screen_update));
	screen.set_palette("palette");
	PALETTE(config, "palette", palette_device::MONOCHROME);

	CLOCK(config, "dclock", 100).signal_handler().set_inputline(m_dmdcpu, MCS51_INT1_LINE); // unknown frequency, need to be measured.
}

void alvg_state::alvg(machine_config &config)
{
	/* basic machine hardware */
	M65C02(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &alvg_state::main_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	MOS6522(config, m_via0, XTAL(4'000'000) / 2);  // U7, uses clock2 from maincpu; switch inputs
	m_via0->readpa_handler().set(FUNC(alvg_state::via0_pa_r));
	m_via0->readpb_handler().set(FUNC(alvg_state::via0_pb_r));
	m_via0->cb2_handler().set_inputline(m_maincpu, INPUT_LINE_NMI).invert();
	m_via0->irq_handler().set("cpuirq", FUNC(input_merger_device::in_w<1>));

	MOS6522(config, m_via1, XTAL(4'000'000) / 2);  // U8, uses clock2 from maincpu; port A = to sound; port B = serial to display
	m_via1->writepb_handler().set(FUNC(alvg_state::via1_pb_w));
	m_via1->irq_handler().set("cpuirq", FUNC(input_merger_device::in_w<2>));

	I8255A(config, m_ppi0); // U12
	m_ppi0->out_pa_callback().set(FUNC(alvg_state::ppi0_pa_w)); // Solenoids
	m_ppi0->out_pb_callback().set(FUNC(alvg_state::ppi0_pb_w)); // Solenoids
	m_ppi0->out_pc_callback().set(FUNC(alvg_state::ppi0_pc_w)); // Solenoids

	I8255A(config, m_ppi1); // U13
	m_ppi1->out_pa_callback().set(FUNC(alvg_state::ppi1_pa_w)); // Solenoids
	m_ppi1->out_pb_callback().set(FUNC(alvg_state::ppi1_pb_w)); // Switch rows
	m_ppi1->out_pc_callback().set(FUNC(alvg_state::ppi1_pc_w)); // Switch rows

	I8255A(config, m_ppi2); // U14
	m_ppi2->out_pa_callback().set(FUNC(alvg_state::ppi2_pa_w)); // Lamps
	m_ppi2->out_pb_callback().set(FUNC(alvg_state::ppi2_pb_w)); // Lamps
	m_ppi2->out_pc_callback().set(FUNC(alvg_state::ppi2_pc_w)); // Lamps

	genpin_audio(config);

	INPUT_MERGER_ANY_HIGH(config, "cpuirq").output_handler().set_inputline(m_maincpu, M65C02_IRQ_LINE);
}


void alvg_state::group1(machine_config &config)
{
	alvg(config);
	pca003(config);  // A-N display
	pca002(config);  // Gen 1 sound
}

void alvg_state::group2(machine_config &config)
{
	alvg(config);
	pca003(config);  // A-N display
	pca008(config);  // Gen 2 sound
}

void alvg_state::group3(machine_config &config)
{
	alvg(config);
	pca020(config);  // DMD
	pca008(config);  // Gen 2 sound
}

/*----------------------------------------------------------------------------
/ A.G. Soccer Ball - A.G. Football has identical ROMs but different playfield
/----------------------------------------------------------------------------*/
ROM_START(agsoccer)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("agscpu1r.18u", 0x0000, 0x10000, CRC(37affcf4) SHA1(017d47f54d5b34a4b71c2f5b84ba9bdb1c924299))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v24", 0x0000, 0x10000, CRC(4ba36e8d) SHA1(330dcb1eea8c311df0e57a3b74146601c26d63c0)) // label says 2.4, inside the ROM it says 2.5L though

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

ROM_START(agsoccera)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("agscpu1r.18u", 0x0000, 0x10000, CRC(37affcf4) SHA1(017d47f54d5b34a4b71c2f5b84ba9bdb1c924299))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v21", 0x0000, 0x10000, CRC(aa30bfe4) SHA1(518f7019639a0284461e83ad849bee0be5371580))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

ROM_START(agsoccer07)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("ags_cpu_r07u", 0x0000, 0x10000, CRC(009ef717) SHA1(d770ce8fd032f4f1d96b9792509cceebbfaebbd9))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("ags_snd.v14", 0x0000, 0x10000, CRC(2544e468) SHA1(d49e2fc91cbb80fdf96f436c614c6f305efafb6f))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("ags_voic.v12", 0x000000, 0x40000, CRC(bac70b18) SHA1(0a699eb95d7d6b071b2cd9d0bf73df355e2ffce8))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Al's Garage Band Goes On A World Tour
/-------------------------------------------------------------------*/
ROM_START(wrldtour)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu27c.512", 0x0000, 0x10000, CRC(c9572fb5) SHA1(47a3e8943ef4207011a33f4a03a6e722c937cc48))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("dot27c.512", 0x0000, 0x8000, CRC(c8bd48e7) SHA1(e2dc513dd42c05c2018e6d8c0b6f0b2c56e6e059))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
ROM_END

ROM_START(wrldtour2)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu02b.512", 0x0000, 0x10000, CRC(1658bf40) SHA1(7af9eedab4e7d0cedaf8bfdbc1f27b989a7171cd))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("dot02b.512", 0x0000, 0x8000, CRC(50e3d59d) SHA1(db6df3482fc485af6bde341750bf8072a296b8da))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
ROM_END

ROM_START(wrldtour3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu03.512", 0x0000, 0x10000, CRC(56dee967) SHA1(f7b1f69d96c72b0cf738bdf45701502f7306a4a0))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("soundc.512", 0x0000, 0x10000, CRC(b44bee01) SHA1(795d8500e5bd73ce23756bf1f5c96db1a3621a70))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("samp_0.c21", 0x000000, 0x40000, CRC(37beb831) SHA1(2b90d2be0a1bd7c59469846631d2b44bdf9f5f9d))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("samp_1.c21", 0x100000, 0x40000, CRC(621533c6) SHA1(ca0ed9e89c340cb3b08f9a9002af9997372c1cbf))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("samp_2.c21", 0x200000, 0x40000, CRC(454a5cca) SHA1(66b1a5832134365fd762fcba4cf4d666f60ebd65))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("samp_3.c21", 0x300000, 0x40000, CRC(1f4928f4) SHA1(9949ab96644984fab8037224f52ec28d7d7cc967))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("dot03.512", 0x0000, 0x8000, CRC(f8a084bb) SHA1(30eb344ad96b5605693d3a7c703c9ed5c1770ca4)) // empty
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("romdef1.c20", 0x00000, 0x40000, CRC(045b21c1) SHA1(134b7eb0f71506d12d9ded24999d530126c558fc))
	ROM_LOAD("romdef2.c20", 0x40000, 0x40000, CRC(23c32ee5) SHA1(429b3b069251bb8b681bbc6382ceb6b85125eb79))
ROM_END

/*-------------------------------------------------------------------
/ Dinosaur Eggs
/-------------------------------------------------------------------*/
ROM_START(dinoeggs)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("dinoeggs.512", 0x0000, 0x10000, CRC(4712f97f) SHA1(593351dcfd475e685c1e5eb2c1006769d3325c8b))
	//ROM_FILL(0x8119,1,0x0d) // stop jump into the weeds
	//ROM_FILL(0x40bd,1,0x80) // disable rom check

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps071.r02", 0x0000, 0x10000, CRC(288f116c) SHA1(5d03ce66bffe39ec02173525078ff07c5005ef18))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("eps072.r02", 0x000000, 0x40000, CRC(780a4364) SHA1(d8a972debee669f0fe66c7407fbed5ef9cd2ce01))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ Mystery Castle
/-------------------------------------------------------------------*/
ROM_START(mystcast)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("mcastle.cpu", 0x0000, 0x10000, CRC(936e6799) SHA1(aa29fb5f12f34c695d1556232744f65cd576a2b1))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mcastle.102", 0x0000, 0x10000, CRC(752822d0) SHA1(36461ef03cac5aefa0c03dfdc63c3d294a3b9c09))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("mcastle.du4", 0x0000, 0x8000, CRC(686e253a) SHA1(28aff34c120c61e231e2111dc396df515bcbbb89))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("mcastle.du5", 0x00000, 0x40000, CRC(9095c367) SHA1(9d3e9416f662ee2aad891eef059278c530448fcc))
	ROM_RELOAD( 0x40000, 0x40000)
ROM_END

ROM_START(mystcasta)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cpu_103.bin", 0x0000, 0x10000, CRC(70ab8ece) SHA1(2bf8cd042450968b7500552419a9af5df2589c13))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("mcastle.103", 0x0000, 0x10000, CRC(bd4849ac) SHA1(f477ea369539a65c0960be1f1c3b4c5503dd6b75))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("mcastle.sr0", 0x000000, 0x40000, CRC(0855cc73) SHA1(c46e08432bcff24594c33171f20669ba63828931))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("mcastle.sr1", 0x100000, 0x40000, CRC(3b5d76e0) SHA1(b2e1bca3c596eba89feda868fa56c71a6b22414c))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("mcastle.sr2", 0x200000, 0x40000, CRC(c3ffd277) SHA1(d16d1b22089b89bbf0db7d2b66c9745a56034322))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("mcastle.sr3", 0x300000, 0x40000, CRC(740858bb) SHA1(d2e9a0a178977dcc873368b042cea7052578df66))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("u4.bin", 0x0000, 0x8000, CRC(a6969efc) SHA1(82da976cb3d30d6fb1576e4c67febd7235f73f51))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("u5.bin", 0x00000, 0x40000, CRC(e5126980) SHA1(2c6d412c87bf27098dae4351958d84e8f9348423))
	ROM_LOAD("u6.bin", 0x40000, 0x40000, CRC(eb241633) SHA1(8e5db75b32ed2ea74088615bbe1403d4c8feafbd))
ROM_END

/*-------------------------------------------------------------------
/ Pistol Poker
/-------------------------------------------------------------------*/
ROM_START(pstlpkr)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("p_peteu2.512", 0x0000, 0x10000, CRC(490a1e2d) SHA1(907dd858ed948681e7366a64a0e7537ebe301d6b))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("p_pu102.512", 0x0000, 0x10000, CRC(b8fb806e) SHA1(c2dc19820ea22bbcf5808db2fb4be76a4033d6ea))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("p_parom0.c20", 0x000000, 0x40000, CRC(99986af2) SHA1(52fa7d2979f7f2d6d65ab6d4f7bbfbed16303991))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("p_parom1.c20", 0x100000, 0x40000, CRC(ae2af238) SHA1(221d3a0e3fb1daad261d723e873ef0727b88889e))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("p_parom2.c20", 0x200000, 0x40000, CRC(f39560a4) SHA1(cdfdf7b44ff4c3f9f4d39fbd8ecbf141d8568088))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("p_parom3.c20", 0x300000, 0x40000, CRC(19d5e4de) SHA1(fb59166ebf992e81b92a42898e351d8443adb1c3))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("p_peteu4.512", 0x0000, 0x8000, CRC(caa0cabd) SHA1(caff6ca4a9cce4e3d846502696c8838805673261))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("p_peteu5.c20", 0x00000, 0x40000, CRC(1d2cecd8) SHA1(6072a0f744fb9eef728fe7cf5e17d0007edbddd7))
	ROM_LOAD("p_peteu6.c20", 0x40000, 0x40000, CRC(3a56376c) SHA1(69febc17b8416c03a58e651447bbe1e14ff27e50))
ROM_END

ROM_START(pstlpkr1)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("u2-ddff.512", 0x0000, 0x10000, CRC(83fa0595) SHA1(d6ebb0e63fd964ccaee3979a7fc13b6adf7b837c))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("p_pu102.512", 0x0000, 0x10000, CRC(b8fb806e) SHA1(c2dc19820ea22bbcf5808db2fb4be76a4033d6ea))

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("p_parom0.c20", 0x000000, 0x40000, CRC(99986af2) SHA1(52fa7d2979f7f2d6d65ab6d4f7bbfbed16303991))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("p_parom1.c20", 0x100000, 0x40000, CRC(ae2af238) SHA1(221d3a0e3fb1daad261d723e873ef0727b88889e))
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
	ROM_LOAD("p_parom2.c20", 0x200000, 0x40000, CRC(f39560a4) SHA1(cdfdf7b44ff4c3f9f4d39fbd8ecbf141d8568088))
	ROM_RELOAD(0x240000, 0x40000)
	ROM_RELOAD(0x280000, 0x40000)
	ROM_RELOAD(0x2c0000, 0x40000)
	ROM_LOAD("p_parom3.c20", 0x300000, 0x40000, CRC(19d5e4de) SHA1(fb59166ebf992e81b92a42898e351d8443adb1c3))
	ROM_RELOAD(0x340000, 0x40000)
	ROM_RELOAD(0x380000, 0x40000)
	ROM_RELOAD(0x3c0000, 0x40000)

	ROM_REGION(0x8000, "dmdcpu", 0)
	ROM_LOAD("p_peteu4.512", 0x0000, 0x8000, CRC(caa0cabd) SHA1(caff6ca4a9cce4e3d846502696c8838805673261))
	ROM_CONTINUE(0x0000, 0x8000)

	ROM_REGION(0x80000, "dmd", 0)
	ROM_LOAD("p_peteu5.c20", 0x00000, 0x40000, CRC(1d2cecd8) SHA1(6072a0f744fb9eef728fe7cf5e17d0007edbddd7))
	ROM_LOAD("p_peteu6.c20", 0x40000, 0x40000, CRC(3a56376c) SHA1(69febc17b8416c03a58e651447bbe1e14ff27e50))
ROM_END

/*-------------------------------------------------------------------
/ Punchy The Clown
/-------------------------------------------------------------------*/
ROM_START(punchy)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("epc061.r02", 0x0000, 0x10000, CRC(732fca88) SHA1(dff0aa4b856bafb95b08dae675dd2ad59e1860e1))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps061.r02", 0x0000, 0x10000, CRC(cfde1b9a) SHA1(cbf9e67df6a6762843272493c2caa1413f70fb27))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("eps062.r02", 0x000000, 0x40000, CRC(7462a5cd) SHA1(05141bcc91b1a786444bff7fa8ba2a785dc0d376))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

ROM_START(punchy3)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("epc061.r03", 0x0000, 0x10000, CRC(8e91131c) SHA1(1bf1408e4e512b764048f4847cf8e4b7a0bf824d))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("eps061.r02", 0x0000, 0x10000, CRC(cfde1b9a) SHA1(cbf9e67df6a6762843272493c2caa1413f70fb27))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("eps062.r02", 0x000000, 0x40000, CRC(7462a5cd) SHA1(05141bcc91b1a786444bff7fa8ba2a785dc0d376))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

/*-------------------------------------------------------------------
/ U.S.A. Football
/-------------------------------------------------------------------*/
ROM_START(usafootb)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usa_cpu.bin", 0x0000, 0x10000, CRC(53b00873) SHA1(96812c4722026554a830c62eca64f09d25a0de82))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("usa_snd.bin", 0x0000, 0x10000, CRC(9d509cbc) SHA1(0be629945b5102adf75e88661e0f956e32ca77da))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("usa_vox.bin", 0x000000, 0x40000, CRC(baae0aa3) SHA1(7933bffcf1509ceeea58a4449268c10c9fac554c))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

ROM_START(usafootba)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usa_cpu1.bin", 0x0000, 0x10000, CRC(3b64a6e9) SHA1(65535bc17395416181bafddc61c0fac177eeba2f))

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("usa_snd.bin", 0x0000, 0x10000, CRC(9d509cbc) SHA1(0be629945b5102adf75e88661e0f956e32ca77da))

	ROM_REGION(0x400000, "oki", 0)
	ROM_LOAD("usa_vox.bin", 0x000000, 0x40000, CRC(baae0aa3) SHA1(7933bffcf1509ceeea58a4449268c10c9fac554c))
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
ROM_END

ROM_START(usafootf)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("usafootf_cpu1.u2", 0x0000, 0x10000, CRC(bdcee108) SHA1(5f241e3d56620cd7464889c9f0032416c190f0c8) )

	ROM_REGION(0x10000, "audiocpu", 0)
	ROM_LOAD("usafootf_snd.u102", 0x0000, 0x10000, CRC(330a7974) SHA1(0fc041a826403167e9c05046304cb4be30e89aaa) )

	ROM_REGION( 0x1000000, "bsmt", 0 )
	ROM_LOAD("usafootf.arom0", 0x000000, 0x40000, CRC(577509bb) SHA1(2d7705aaa5cf42f4fcffa259a325cec2018d0085) )
	ROM_RELOAD(0x040000, 0x40000)
	ROM_RELOAD(0x080000, 0x40000)
	ROM_RELOAD(0x0c0000, 0x40000)
	ROM_LOAD("usafootf.arom1", 0x100000, 0x40000, CRC(d71553bd) SHA1(4d1e1ca252a07e2c4f0e0fe68900b6c2438cc933) )
	ROM_RELOAD(0x140000, 0x40000)
	ROM_RELOAD(0x180000, 0x40000)
	ROM_RELOAD(0x1c0000, 0x40000)
ROM_END

} // Anonymous namespace


GAME( 1991, agsoccer,   0,        group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R18u, 2.5L sound)",          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, agsoccera,  agsoccer, group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R18u, 2.1 sound)",           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1991, agsoccer07, agsoccer, group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "A.G. Soccer Ball (R07u)",                      MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, wrldtour,   0,        group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour (R01c)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, wrldtour2,  wrldtour, group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour (R02b)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1992, wrldtour3,  wrldtour, group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Al's Garage Band Goes On A World Tour (R06a)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, usafootb,   0,        group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "U.S.A. Football (R06u)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, usafootba,  usafootb, group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "U.S.A. Football (R01u)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1994, usafootf,   0,        group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "U.S.A. Football (P08, redemption)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, mystcast,   0,        group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Mystery Castle (R02)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, mystcasta,  mystcast, group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Mystery Castle (R03)",                         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, pstlpkr,    0,        group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Pistol Poker (R02)",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, pstlpkr1,   pstlpkr,  group3, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Pistol Poker (R01)",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, punchy,     0,        group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Punchy The Clown (R02)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, punchy3,    punchy,   group1, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Punchy The Clown (R03)",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1993, dinoeggs,   0,        group2, alvg, alvg_state, empty_init, ROT0, "Alvin G", "Dinosaur Eggs (R02)",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
