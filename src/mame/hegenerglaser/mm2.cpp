// license:BSD-3-Clause
// copyright-holders:Dirk Verwiebe, Cowering, hap
/*******************************************************************************

Mephisto MM II series chesscomputers
2007 Dirk V.

TODO:
- rebel5 unknown read from 0x4002, looks like leftover bookrom check
- need to emulate TurboKit properly as a slot device, also for mm5p (it's not as
  simple as a CPU overclock), TK20 EPROM is dumped for the common version (6502
  Mephisto/Fidelity/Novag/etc.) and for the SciSys Maestro/Analyst version
- correct rom labels (applies to the filenames with .bin extension)

================================================================================

For rebel5 and newer, the chess engine is by Ed Schröder. Older chesscomputers in
this driver were authored by Ulf Rathsman.

The MM II program was also licensed to Daimler-Benz, who gave away several custom
chesscomputers as a parting gift to retiring executives. The hardware is same as MM II.
see(1): http://chesseval.com/ChessEvalJournal/DaimlerBenz.htm
see(2): http://chesseval.com/RareBoard/DaimlerBenzBoard.htm

MM III was never released officially. Rebell 5,0 is commonly known as MM III, but the
real one (updated MM II engine) didn't get further than a prototype.

MM II (Nona program) wasn't a commercial release. After Mondial came out, Frans Morsch
ported his Nona program to MM II hardware, using Ed Schröder's interface (hence the
similarity with Rebel). According to research, this version competed in the 1985 Dutch
Open Computer Chess Championship.

MM IV TurboKit 18MHz - (mm4tk)
This is a replacement ROM combining the TurboKit initial ROM with the original MM IV.
The TurboKit powers up to its tiny ROM, copies itself to RAM, banks in normal ROM,
copies that to faster SRAM, then patches the checksum and the LED blink delays.

There is an undumped MM V TurboKit, which will be the exact same except for location
of the patches. The mm5tk just needs the normal mm5 ROM swapped out for that one to
blinks the LEDs a little slower.

Correction: The real TK20 TurboKit does not patch the ROM, so mm4tk (and a possible
mm5 version of this) is more likely a SteveUK hack.

The MM V prototype was the program that Ed Schröder participated with as "Rebel" at
the 1989 WMCCC in Portorose. It was used with the TK20 TurboKit.
For more information, see: http://chesseval.com/ChessEvalJournal/PrototypeMMV.htm

MM VI (Saitek, 1994) is on different hardware, H8 CPU.

================================================================================

MM IV + MM V hardware notes

Overview:
- CPU: R65C02P3/R65C02P4 or G65SC02P-4
- Clock: 4.9152 MHz
- NMI CLK: 600 Hz
- IRQ Line is set to VSS
- 8 KByte SRAM Sony CXK5864-15L

1-CD74HC4060E: 14 Bit Counter
1-CD74HC166E
1-CD74HC251E
1-SN74HC138N TI
1-SN74HC139N TI
1-74HC14AP Toshiba
1-74HC02AP Toshiba
1-74HC00AP Toshiba
1-CD74HC259E

LCD module:
PCB label HGS 10 122 01
1-CD4011
4-CD4015

$0000-$1fff S-RAM
$2000 LCD 4 Byte Shift Register writeonly right to left
every 2nd char xor'd by $FF

2c00-2c07 Keyboard (8to1 Multiplexer) 74HCT251
2*8 Matrix
Adr. 0x3407
==0 !=0
2c00 CL E5
2c01 POS F6
2c02 MEMO G7
2c03 INFO A1
2c04 LEV H8
2c05 ENT B2
2c06 >0 C3
2c07 <9 D4

$3400-$3407 LED 1-6, Buzzer, Keyboard select

$2400 // Chess Board
$2800 // Chess Board
$3000 // Chess Board

$4000-$7FFF Opening Module HG550
$8000-$FFFF ROM

*******************************************************************************/

#include "emu.h"

#include "mmboard.h"
#include "mmdisplay1.h"

#include "bus/generic/slot.h"
#include "bus/generic/carts.h"
#include "cpu/m6502/r65c02.h"
#include "machine/74259.h"
#include "sound/dac.h"

#include "softlist_dev.h"
#include "speaker.h"

// internal artwork
#include "mephisto_bup.lh"
#include "mephisto_mm2.lh"
#include "mephisto_mm5.lh"


namespace {

class mm2_state : public driver_device
{
public:
	mm2_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_outlatch(*this, "outlatch"),
		m_display(*this, "display"),
		m_keys(*this, "KEY.%u", 0),
		m_reset(*this, "RESET")
	{ }

	DECLARE_INPUT_CHANGED_MEMBER(reset_button);

	void rebel5(machine_config &config);
	void mm4(machine_config &config);
	void mm4tk(machine_config &config);
	void mm5(machine_config &config);
	void mm5p(machine_config &config);
	void mm2(machine_config &config);
	void mm2nona(machine_config &config);
	void bup(machine_config &config);

protected:
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<hc259_device> m_outlatch;
	required_device<mephisto_display1_device> m_display;
	required_ioport_array<2> m_keys;
	required_ioport m_reset;

	void bup_mem(address_map &map);
	void mm2_mem(address_map &map);
	void mm4_mem(address_map &map);
	void mm5p_mem(address_map &map);
	void rebel5_mem(address_map &map);

	void lcd_irqack_w(u8 data);
	u8 keys_r(offs_t offset);
};

void mm2_state::machine_reset()
{
	m_maincpu->set_input_line(0, CLEAR_LINE);
	m_display->reset();
}

INPUT_CHANGED_MEMBER(mm2_state::reset_button)
{
	// RES buttons in serial tied to CPU RESET
	if (m_reset->read() == 3)
	{
		m_maincpu->pulse_input_line(INPUT_LINE_RESET, attotime::zero);
		machine_reset();
	}
}



/*******************************************************************************
    I/O
*******************************************************************************/

void mm2_state::lcd_irqack_w(u8 data)
{
	m_display->data_w(data);

	// accessing here also clears irq
	m_maincpu->set_input_line(0, CLEAR_LINE);
}

u8 mm2_state::keys_r(offs_t offset)
{
	// lcd strobe is shared with keypad select
	return ~(BIT(m_keys[m_outlatch->q7_r()]->read(), offset) << 7);
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void mm2_state::bup_mem(address_map &map)
{
	map(0x0000, 0x0fff).ram();
	map(0x1000, 0x1007).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x1800, 0x1807).r(FUNC(mm2_state::keys_r));
	map(0x2000, 0x2000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x2800, 0x2800).w(FUNC(mm2_state::lcd_irqack_w));
	map(0x3000, 0x3000).w("board", FUNC(mephisto_board_device::led_w));
	map(0x3800, 0x3800).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x8000, 0xffff).rom();
}

void mm2_state::mm2_mem(address_map &map)
{
	bup_mem(map);
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom)); // opening library
}

void mm2_state::rebel5_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2007).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x3000, 0x3007).r(FUNC(mm2_state::keys_r));
	map(0x4000, 0x4000).r("board", FUNC(mephisto_board_device::input_r));
	map(0x5000, 0x5000).w(FUNC(mm2_state::lcd_irqack_w));
	map(0x6000, 0x6000).w("board", FUNC(mephisto_board_device::led_w));
	map(0x7000, 0x7000).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x8000, 0xffff).rom();
}

void mm2_state::mm5p_mem(address_map &map)
{
	map(0x0000, 0x1fff).ram();
	map(0x2000, 0x2000).mirror(0x03ff).w(m_display, FUNC(mephisto_display1_device::data_w));
	map(0x2400, 0x2400).mirror(0x03ff).w("board", FUNC(mephisto_board_device::led_w)).nopr();
	map(0x2800, 0x2800).mirror(0x03ff).w("board", FUNC(mephisto_board_device::mux_w));
	map(0x2c00, 0x2c07).mirror(0x03f8).r(FUNC(mm2_state::keys_r));
	map(0x3000, 0x3000).mirror(0x03ff).r("board", FUNC(mephisto_board_device::input_r));
	map(0x3400, 0x3407).mirror(0x03f8).w("outlatch", FUNC(hc259_device::write_d7)).nopr();
	map(0x3800, 0x3800).mirror(0x03ff).nopw(); // N/C
	map(0x4000, 0xffff).rom();
}

void mm2_state::mm4_mem(address_map &map)
{
	mm5p_mem(map);
	map(0x4000, 0x7fff).r("cartslot", FUNC(generic_slot_device::read_rom));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( mm2 )
	PORT_START("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Right / White / 0") PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD) PORT_CODE(KEYCODE_RIGHT)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Left / Black / 9") PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD) PORT_CODE(KEYCODE_LEFT)

	PORT_START("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E / 5 / Queen") PORT_CODE(KEYCODE_E) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F / 6 / King") PORT_CODE(KEYCODE_F) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G / 7") PORT_CODE(KEYCODE_G) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A / 1 / Pawn") PORT_CODE(KEYCODE_A) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H / 8") PORT_CODE(KEYCODE_H) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B / 2 / Knight") PORT_CODE(KEYCODE_B) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C / 3 / Bishop") PORT_CODE(KEYCODE_C) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D / 4 / Rook") PORT_CODE(KEYCODE_D) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)

	PORT_START("RESET")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 1") PORT_CODE(KEYCODE_Z) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mm2_state, reset_button, 0)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RES 2") PORT_CODE(KEYCODE_X) PORT_CODE(KEYCODE_F1) PORT_CHANGED_MEMBER(DEVICE_SELF, mm2_state, reset_button, 0)
INPUT_PORTS_END

static INPUT_PORTS_START( bup )
	PORT_INCLUDE( mm2 )

	PORT_MODIFY("KEY.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("1 / Pawn") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("2 / Knight") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("3 / Bishop") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("4 / Rook") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("5 / Queen") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("6 / King") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("White") PORT_CODE(KEYCODE_W)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Black") PORT_CODE(KEYCODE_B)

	PORT_MODIFY("KEY.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("ENT") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("BEST") PORT_CODE(KEYCODE_S)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("INFO") PORT_CODE(KEYCODE_I)
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MON") PORT_CODE(KEYCODE_N)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("POS") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LEV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("MEM") PORT_CODE(KEYCODE_M)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void mm2_state::rebel5(machine_config &config)
{
	// basic machine hardware
	R65C02(config, m_maincpu, 9.8304_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::rebel5_mem);

	const attotime irq_period = attotime::from_hz(9.8304_MHz_XTAL / 2 / 0x2000); // 600Hz
	m_maincpu->set_periodic_int(FUNC(mm2_state::irq0_line_assert), irq_period);

	HC259(config, m_outlatch);
	m_outlatch->q_out_cb<0>().set_output("led100");
	m_outlatch->q_out_cb<1>().set_output("led101");
	m_outlatch->q_out_cb<2>().set_output("led102");
	m_outlatch->q_out_cb<3>().set_output("led103");
	m_outlatch->q_out_cb<4>().set_output("led104");
	m_outlatch->q_out_cb<5>().set_output("led105");
	m_outlatch->q_out_cb<6>().set("dac", FUNC(dac_1bit_device::write));
	m_outlatch->q_out_cb<7>().set(m_display, FUNC(mephisto_display1_device::strobe_w)).invert();

	MEPHISTO_SENSORS_BOARD(config, "board");
	MEPHISTO_DISPLAY_MODULE1(config, m_display);
	config.set_default_layout(layout_mephisto_mm2);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	DAC_1BIT(config, "dac").add_route(ALL_OUTPUTS, "speaker", 0.25);
}

void mm2_state::mm5p(machine_config &config)
{
	rebel5(config);

	// basic machine hardware
	m_maincpu->set_clock(4.9152_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::mm5p_mem);

	const attotime nmi_period = attotime::from_hz(4.9152_MHz_XTAL / 0x2000); // 600Hz
	m_maincpu->set_periodic_int(FUNC(mm2_state::nmi_line_pulse), nmi_period);
}

void mm2_state::mm4(machine_config &config)
{
	mm5p(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::mm4_mem);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mephisto_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mm4");
}

void mm2_state::mm4tk(machine_config &config)
{
	mm4(config);
	m_maincpu->set_clock(36_MHz_XTAL / 2);
}

void mm2_state::mm5(machine_config &config)
{
	mm4(config);
	SOFTWARE_LIST(config.replace(), "cart_list").set_original("mephisto_mm5");

	config.set_default_layout(layout_mephisto_mm5); // does not apply to mm5p
}

void mm2_state::bup(machine_config &config)
{
	rebel5(config);

	// basic machine hardware
	m_maincpu->set_clock(7.3728_MHz_XTAL / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::bup_mem);

	const attotime irq_period = attotime::from_hz(7.3728_MHz_XTAL / 2 / 0x2000); // 450Hz from 4020 Q13
	m_maincpu->set_periodic_int(FUNC(mm2_state::irq0_line_assert), irq_period);

	m_outlatch->q_out_cb<7>().set(m_display, FUNC(mephisto_display1_device::strobe_w));

	config.set_default_layout(layout_mephisto_bup);
}

void mm2_state::mm2(machine_config &config)
{
	bup(config);

	// basic machine hardware
	m_maincpu->set_addrmap(AS_PROGRAM, &mm2_state::mm2_mem);

	config.set_default_layout(layout_mephisto_mm2);

	GENERIC_CARTSLOT(config, "cartslot", generic_plain_slot, "mephisto_cart");
	SOFTWARE_LIST(config, "cart_list").set_original("mephisto_mm2");
}

void mm2_state::mm2nona(machine_config &config)
{
	bup(config);

	m_outlatch->q_out_cb<7>().set(m_display, FUNC(mephisto_display1_device::strobe_w)).invert();
	config.set_default_layout(layout_mephisto_mm2);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( bup )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bup_1.bin", 0x8000, 0x4000, CRC(e1e9625a) SHA1(8a757e28b7afca2a092f8ff419087e06b07b743e) )
	ROM_LOAD("bup_2.bin", 0xc000, 0x4000, CRC(6db30b80) SHA1(df4b379c4e916dff6b4110ec9c3591a9620c3424) )
ROM_END

ROM_START( bupa )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("bupa_1.bin", 0x8000, 0x4000, CRC(e1e9625a) SHA1(8a757e28b7afca2a092f8ff419087e06b07b743e) )
	ROM_LOAD("bupa_2.bin", 0xc000, 0x4000, CRC(708338ea) SHA1(d617c4aa2161865a22b4b0646ba793f8a1fda863) )
ROM_END


ROM_START( mm2 ) // 10 Sep 1986
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("400", 0x8000, 0x8000, CRC(e8c1f431) SHA1(c32dfa66eefbf3e539438d2fe6e6916f78a128be) ) // HN27C256G-20
	// 2-EPROM version also exists: CRC32 e9adcb8f & d40cbfc2
ROM_END

ROM_START( mm2a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("300", 0x8000, 0x8000, CRC(60c777d4) SHA1(a77d678be60094073275558b4e8f0d34b43dd9ae) ) // D27C256D-20
	// 2-EPROM version also exists: CRC32 86a5a14f & a122f2c0
ROM_END

ROM_START( mm2b ) // 21 Apr 1986
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("200", 0x8000, 0x8000, CRC(9b69aaab) SHA1(98ee4879eef4d8b06553290f16ca661cf4181af8) )
	// 2-EPROM version also exists: CRC32 09cf6228 & 86d77724, ROM labels 8-b_21.4 & c-f_21.4
ROM_END

ROM_START( mm2c ) // serial 05780xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("hg86", 0x8000, 0x4000, CRC(e26d1f17) SHA1(3227bb1f8f22dd0d902a9a8be3c508b45c57d6cc) )
	ROM_LOAD("50",   0xc000, 0x4000, CRC(86d77724) SHA1(e46c59e87465a9a1784fbaf4743649b2c10006e0) )
ROM_END

ROM_START( mm2d ) // serial 05650xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mm2d_1.bin", 0x8000, 0x4000, CRC(b91dab77) SHA1(67762304afe51fb8f1eb91259567b2451bf9bbfd) )
	ROM_LOAD("mm2d_2.bin", 0xc000, 0x4000, CRC(01143cc1) SHA1(f78474b410dbecb209aa23ef81e9f894e8b54942) )
ROM_END

ROM_START( mm2e ) // 13 Sep 1985, serial 05569xx
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("hg_8b_13.9", 0x8000, 0x4000, CRC(e2daac82) SHA1(c9fa59ca92362f8ee770733073bfa2ab8c7904ad) )
	ROM_LOAD("c-f_6.9",   0xc000, 0x4000, CRC(5e296939) SHA1(badd2a377259cf738cd076d8fb245c3dc284c24d) )
ROM_END


ROM_START( mm2nona )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("l7", 0x8000, 0x4000, CRC(9a45e1d4) SHA1(26e4c9cd1afe9aea0e8cfc25bdc9138bd99d5992) )
	ROM_LOAD("r7", 0xc000, 0x4000, CRC(2285af5e) SHA1(bea22e32b65eea5fa7617d9d1b3a824a7affe678) )
ROM_END


ROM_START( rebel5 ) // 8 Feb 1987?
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rebel5.bin", 0x8000, 0x8000, CRC(17232752) SHA1(3cd6893c0071f3dc02785bf99f1950eed81eba39) )
ROM_END

ROM_START( rebel5a ) // 5 Dec 1986
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("rebell_5.12.86", 0x8000, 0x8000, CRC(8d02e1ef) SHA1(9972c75936613bd68cfd3fe62bd222e90e8b1083) )
ROM_END

ROM_START( rebel5b ) // 18 Aug 1986
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("reb_18.8.86", 0x8000, 0x8000, CRC(c8c95e81) SHA1(0fb83ade11d2a2a74c94d7bd6f71130ebbc77497) )
ROM_END


ROM_START( mm4 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("710", 0x8000, 0x8000, CRC(f68a4124) SHA1(d1d03a9aacc291d5cb720d2ee2a209eeba13a36c) )
ROM_END

ROM_START( mm4a )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("700", 0x8000, 0x8000, CRC(c97da840) SHA1(10bd2a391338ed2e417b35dcb6396ab4a4e360f0) )
ROM_END

ROM_START( mm4b )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("600", 0x8000, 0x8000, CRC(837d49b9) SHA1(9fb2dfaaeca2559ce582211137635c069180e95f) )
ROM_END

ROM_START( mm4tk ) // hack of 710
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mm4tk.bin", 0x8000, 0x8000, CRC(51cb36a4) SHA1(9e184b4e85bb721e794b88d8657ae8d2ff5a24af) )
ROM_END


ROM_START( mm5 ) // v5.1 (MEM->INFO to see version number)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mm5.bin", 0x8000, 0x8000, CRC(89c3d9d2) SHA1(77cd6f8eeb03c713249db140d2541e3264328048) )
ROM_END

ROM_START( mm5a ) // v5.0
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("mm5a.bin", 0x8000, 0x8000, CRC(fcfa7e6e) SHA1(afeac3a8c957ba58cefaa27b11df974f6f2066da) )
ROM_END

ROM_START( mm5p )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("buch.bin",     0x0000, 0x8000, CRC(534607c7) SHA1(d0347a5f8dc4cf6001f649aa13e7a7fe75bec5b9) ) // 1st half empty
	ROM_LOAD("programm.bin", 0x8000, 0x8000, CRC(ee22b974) SHA1(37267507be30ee84051bc94c3a63fb1298a00261) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME     PARENT  COMPAT  MACHINE   INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1985, bup,     0,      0,      bup,      bup,   mm2_state, empty_init, "Hegener + Glaser", u8"Mephisto Blitz- und Problemlösungs-Modul (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, bupa,    bup,    0,      bup,      bup,   mm2_state, empty_init, "Hegener + Glaser", u8"Mephisto Blitz- und Problemlösungs-Modul (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1985, mm2,     0,      0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 1, v4.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, mm2a,    mm2,    0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 2, v3.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, mm2b,    mm2,    0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 3, v2.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, mm2c,    mm2,    0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 4)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, mm2d,    mm2,    0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 5)", MACHINE_SUPPORTS_SAVE )
SYST( 1985, mm2e,    mm2,    0,      mm2,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (set 6)", MACHINE_SUPPORTS_SAVE )

SYST( 1985, mm2nona, 0,      0,      mm2nona,  mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM II (Nona program, DOCCC 1985 Leiden TM)", MACHINE_SUPPORTS_SAVE )

SYST( 1986, rebel5,  0,      0,      rebel5,   mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto Rebell 5,0 (set 1)", MACHINE_SUPPORTS_SAVE ) // aka MM III
SYST( 1986, rebel5a, rebel5, 0,      rebel5,   mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto Rebell 5,0 (set 2)", MACHINE_SUPPORTS_SAVE ) // "
SYST( 1986, rebel5b, rebel5, 0,      rebel5,   mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto Rebell 5,0 (set 3)", MACHINE_SUPPORTS_SAVE ) // "

SYST( 1987, mm4,     0,      0,      mm4,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM IV (v7.10)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, mm4a,    mm4,    0,      mm4,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM IV (v7.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, mm4b,    mm4,    0,      mm4,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM IV (v6.00)", MACHINE_SUPPORTS_SAVE )
SYST( 1987, mm4tk,   mm4,    0,      mm4tk,    mm2,   mm2_state, empty_init, "hack",             "Mephisto MM IV (TurboKit)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING )

SYST( 1990, mm5,     0,      0,      mm5,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM V (v5.1)", MACHINE_SUPPORTS_SAVE )
SYST( 1990, mm5a,    mm5,    0,      mm5,      mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM V (v5.0)", MACHINE_SUPPORTS_SAVE )
SYST( 1989, mm5p,    mm5,    0,      mm5p,     mm2,   mm2_state, empty_init, "Hegener + Glaser", "Mephisto MM V (WMCCC 1989 Portorose TM)", MACHINE_SUPPORTS_SAVE | MACHINE_IMPERFECT_TIMING ) // aka Rebel
