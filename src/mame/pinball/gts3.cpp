// license:BSD-3-Clause
// copyright-holders:Robbbert
/****************************************************************************************************

PINBALL
Gottlieb System 3
Alpha-Numeric Display

Setting up:
- Press num-0
- Press num-1
- Press 1 (to load factory defaults)
- Press 1 (to select English)
- Press num-0 until the credit settings appear (settings #9,10,11)
- On each of the 3 coin slots, press num1/num2 to set to 1 credit.
- Press F3 or 0
- Now you can play

Quick setup for surfnsaf,opthund,tt_game,ccruise:
- Press 0 A num1 num1 to load factory defaults
- Press 0 until you get to Game Pricing, press A to get Universal. Press 0 a few times then F3 to save.
- Now press 7 to insert a coin.
Further setup if needed:
- Use A and B to cycle up and down (instead of num1/num2)
- num1 selects default setting
- 0 to choose another setting
- F3 to save and reboot

Here are the key codes to enable play:

Game                 NUM  Start game                                       End ball
--------------------------------------------------------------------------------------------------
Lights Camera Action 720  1 then W                                         V
Silver Slugger       722  1 then jiggle ; and quote                        ;
Vegas                723  1 then B                                         A
Deadly Weapon        724  1 then Q                                         I then Q
Car Hop              725  1 then P and ]                                   P then ]
Title Fight          726  1 then [ and left                                [ (wait for message) then left
Hoops                727  1 then Q and Z                                   [ then Z (may have to jiggle ] and [, or try multiple times)
Cactus Jack          729  1 then ; and quote                               ;
Class of 1812        730  1 then ; and quote                               up
Nudge-it            N102  1 then B and M                                   unknown (Redemption machine)
Bell Ringer         N103  1 then A                                         unknown (Redemption machine)

** From here, num1 = start button, 7 = coin, 0 = test/setup
Surf'n Safari        731  num1 then num2 then P                            num2 then P
Operation: Thunder   732  num1 then G and O                                G then hold O for a few seconds
tt_game              7**  unknown                                          unknown
Caribbean Cruise    C102  1, enter, hold \ and right until message goes    \


Status:
- All games (except tt_game) are playable
- Various sounds are missing in some games, usually because the cpu concerned runs into the weeds.

ToDo:
- Display flickers a bit
- tt_game: roms missing therefore no sound

*****************************************************************************************************/

#include "emu.h"
#include "genpin.h"

#include "gottlieb_a.h"

#include "cpu/m6502/m65c02.h"
#include "machine/6522via.h"
#include "machine/input_merger.h"
#include "speaker.h"

#include "gts3.lh"

namespace {

class gts3_state : public genpin_class
{
public:
	gts3_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_u4(*this, "u4")
		, m_u5(*this, "u5")
		, m_p5_sound(*this, "p5sound")
		, m_p7_sound(*this, "p7sound")
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
	{ }

	void p0(machine_config &config);  // no sound card assigned yet
	void p5(machine_config &config);  // p5 sound card
	void p7(machine_config &config);  // p7 sound card
	DECLARE_INPUT_CHANGED_MEMBER(test_inp);

protected:
	virtual void machine_reset() override ATTR_COLD;
	virtual void machine_start() override ATTR_COLD;

private:
	void segbank_w(offs_t offset, u8 data);
	u8 u4a_r();
	u8 u4b_r();
	void lampret_w(u8);
	void solenoid_w(offs_t, u8);
	void u4b_w(u8 data);
	void u5a_w(u8 data);
	void nmi_w(int state);
	void mem_map(address_map &map) ATTR_COLD;

	bool m_dispclk = false;
	bool m_lampclk = false;
	u8 m_digit = 0U;
	u8 m_row = 0U; // for lamps and switches
	u8 m_segment[4]{};
	u8 m_u4b = 0U;

	required_device<m65c02_device> m_maincpu;
	required_device<via6522_device> m_u4;
	required_device<via6522_device> m_u5;
	optional_device<gottlieb_sound_p5_device> m_p5_sound;
	optional_device<gottlieb_sound_p7_device> m_p7_sound;
	required_ioport_array<12> m_io_keyboard;
	output_finder<40> m_digits;
	output_finder<128> m_io_outputs;   // 32 solenoids + 96 lamps
};


void gts3_state::mem_map(address_map &map)
{
	map(0x0000, 0x1fff).ram().share("nvram"); // sch shows a 6164 labelled as 6116LP, parts list says 6116
	map(0x2000, 0x200f).mirror(0x1f80).m(m_u4, FUNC(via6522_device::map));
	map(0x2010, 0x201f).mirror(0x1f80).m(m_u5, FUNC(via6522_device::map));
	map(0x2020, 0x2023).mirror(0x1f8c).w(FUNC(gts3_state::segbank_w));
	map(0x2030, 0x2033).mirror(0x1f8c).w(FUNC(gts3_state::solenoid_w)).nopr(); // writing here causes a read?? lca @8CE7
	map(0x2040, 0x2040).mirror(0x1f80).w(FUNC(gts3_state::lampret_w));
	map(0x2041, 0x207f).mirror(0x1f80).nopw();   // AUX: purpose unknown
	map(0x4000, 0xffff).rom();
}

static INPUT_PORTS_START( gts3 )
	PORT_START("TTS")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test") PORT_CHANGED_MEMBER(DEVICE_SELF, gts3_state, test_inp, 1)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Slam Tilt") PORT_CODE(KEYCODE_0)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Tilt") PORT_CODE(KEYCODE_9)

	PORT_START("X0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_COIN3)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_COIN2)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_START)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Left Advance")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Right Advance")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_A) PORT_NAME("INP06")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_B) PORT_NAME("INP07")

	PORT_START("X1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_C) PORT_NAME("INP10")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_D) PORT_NAME("INP11")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_E) PORT_NAME("INP12")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_F) PORT_NAME("INP13")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_G) PORT_NAME("INP14")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_H) PORT_NAME("INP15")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_I) PORT_NAME("INP16")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_J) PORT_NAME("INP17")

	PORT_START("X2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_K) PORT_NAME("INP20")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_L) PORT_NAME("INP21")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_M) PORT_NAME("INP22")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_N) PORT_NAME("INP23")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_O) PORT_NAME("INP24")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_P) PORT_NAME("INP25")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Q) PORT_NAME("INP26")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_R) PORT_NAME("INP27")

	PORT_START("X3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_S) PORT_NAME("INP30")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_T) PORT_NAME("INP31")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_U) PORT_NAME("INP32")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_V) PORT_NAME("INP33")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_W) PORT_NAME("INP34")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_X) PORT_NAME("INP35")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Y) PORT_NAME("INP36")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_Z) PORT_NAME("INP37")

	PORT_START("X4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP40")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP41")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP42")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP43")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP44")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP45")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP46")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP47")

	PORT_START("X5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP50")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS) PORT_NAME("INP51")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_EQUALS) PORT_NAME("INP52")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_BACKSPACE) PORT_NAME("INP53")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_UP) PORT_NAME("INP54")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DOWN) PORT_NAME("INP55")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LEFT) PORT_NAME("INP56")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RIGHT) PORT_NAME("INP57")

	PORT_START("X6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SPACE) PORT_NAME("INP60")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LSHIFT) PORT_NAME("INP61")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RSHIFT) PORT_NAME("INP62")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_LCONTROL) PORT_NAME("INP63")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_RCONTROL) PORT_NAME("INP64")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PGUP) PORT_NAME("INP65")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PGDN) PORT_NAME("INP66")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_HOME) PORT_NAME("INP67")

	PORT_START("X7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_END) PORT_NAME("INP70")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL) PORT_NAME("INP71")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_DEL_PAD) PORT_NAME("INP72")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ENTER_PAD) PORT_NAME("INP73")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_PLUS_PAD) PORT_NAME("INP74")
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_MINUS_PAD) PORT_NAME("INP75")
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_ASTERISK) PORT_NAME("INP76")
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_KEYPAD) PORT_CODE(KEYCODE_SLASH_PAD) PORT_NAME("INP77")

	PORT_START("X8")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X9")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X10")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("X11")
	PORT_BIT(0xff, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END

INPUT_CHANGED_MEMBER( gts3_state::test_inp )
{
	m_u4->write_ca1(newval);
}

// This trampoline needed; WRITELINE("maincpu", m65c02_device, nmi_line) does not work
void gts3_state::nmi_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_NMI, (state) ? CLEAR_LINE : HOLD_LINE);
}

void gts3_state::lampret_w(u8 data)
{
	if (m_row < 12)
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[32+m_row*8+i] = BIT(data, i);
}

void gts3_state::solenoid_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[offset*8+i] = BIT(data, i);
	// Mechanical sounds
	if (offset == 3)
	{
		if (data & 0x18)
			m_samples->start(0, 9);  // outhole
		if (data & 0x20)
			m_samples->start(1, 6);  // knocker
	}
}

void gts3_state::segbank_w(offs_t offset, u8 data)
{
	uint32_t seg1,seg2;
	m_segment[offset] = data;
	seg1 = m_segment[offset&2] | (m_segment[offset|1] << 8);
	seg2 = bitswap<32>(seg1,16,16,16,16,16,16,16,16,16,16,16,16,16,16,15,14,9,7,13,11,10,6,8,12,5,4,3,3,2,1,0,0);
	if (m_digit < 20)
		m_digits[m_digit+(BIT(offset, 1) ? 0 : 20)] = seg2;
}

void gts3_state::u4b_w(u8 data)
{
	m_u4b = data & 0xe7;
	bool clk_bit = BIT(data, 6);
	if ((!m_dispclk) && clk_bit) // 0->1 is valid
	{
		if (BIT(data, 5))
			m_digit = 0;
		else
			m_digit++;
	}
	m_dispclk = clk_bit;

	clk_bit = BIT(data, 1);
	if ((!m_lampclk) && clk_bit) // 0->1 is valid
	{
		if (BIT(data, 0))
			m_row = 0;
		else
			m_row++;
	}
	m_lampclk = clk_bit;
}

u8 gts3_state::u4a_r()
{
	if (m_row < 12)
		return m_io_keyboard[m_row]->read();
	else
		return 0xff;
}

u8 gts3_state::u4b_r()
{
	return m_u4b | (ioport("TTS")->read() & 0x18);
}

void gts3_state::u5a_w(u8 data)
{
	if (m_p5_sound)
		m_p5_sound->write(data);
	else
	if (m_p7_sound)
		m_p7_sound->write(data);
}

void gts3_state::machine_start()
{
	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_dispclk));
	save_item(NAME(m_lampclk));
	save_item(NAME(m_segment));
	save_item(NAME(m_row));
	save_item(NAME(m_u4b));
}

void gts3_state::machine_reset()
{
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_digit = 0;
	m_dispclk = 0;
}

void gts3_state::p0(machine_config &config)
{
	M65C02(config, m_maincpu, XTAL(4'000'000) / 2);
	m_maincpu->set_addrmap(AS_PROGRAM, &gts3_state::mem_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	config.set_default_layout(layout_gts3);

	R65C22(config, m_u4, XTAL(4'000'000) / 2);
	m_u4->irq_handler().set("irq", FUNC(input_merger_device::in_w<0>));
	m_u4->readpa_handler().set(FUNC(gts3_state::u4a_r));
	m_u4->readpb_handler().set(FUNC(gts3_state::u4b_r));
	m_u4->writepb_handler().set(FUNC(gts3_state::u4b_w));
	//m_u4->ca2_handler().set(FUNC(gts3_state::u4ca2_w));
	m_u4->cb2_handler().set(FUNC(gts3_state::nmi_w));

	R65C22(config, m_u5, XTAL(4'000'000) / 2);
	m_u5->irq_handler().set("irq", FUNC(input_merger_device::in_w<1>));
	m_u5->writepa_handler().set(FUNC(gts3_state::u5a_w));
	//m_u5->readpb_handler().set(FUNC(gts3_state::u5b_r));
	//m_u5->writepb_handler().set(FUNC(gts3_state::u5b_w));
	//m_u5->ca2_handler().set(FUNC(gts3_state::u5ca2_w));
	//m_u5->cb1_handler().set(FUNC(gts3_state::u5cb1_w));
	//m_u5->cb2_handler().set(FUNC(gts3_state::u5cb2_w));

	INPUT_MERGER_ANY_HIGH(config, "irq").output_handler().set_inputline("maincpu", m65c02_device::IRQ_LINE);

	/* Sound */
	genpin_audio(config);
	SPEAKER(config, "mono").front_center();
}

void gts3_state::p5(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN5(config, m_p5_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}

void gts3_state::p7(machine_config &config)
{
	p0(config);
	GOTTLIEB_SOUND_PIN7(config, m_p7_sound).add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*-------------------------------------------------------------------
/ Bell Ringer (N103) Redemption Machine
/-------------------------------------------------------------------*/
ROM_START(bellring)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("br_gprom.bin", 0x0000, 0x10000, CRC(a9a59b36) SHA1(ca6d0e54a5c85ef72485975c632660831a3b8c82))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("br_drom1.bin", 0x8000, 0x8000, CRC(99f38229) SHA1(f63d743e63e88728e8d53320b21b2fda1b6385f8))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("br_yrom1.bin", 0x8000, 0x8000, CRC(d5aab379) SHA1(b3995f8aa2e54f91f2a0fd010c807fbfbf9ae847))
ROM_END

/*-------------------------------------------------------------------
/ Cactus Jack's (#729)
/-------------------------------------------------------------------*/
ROM_START(cactjack)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(5661ab06) SHA1(12b7066110feab0aef36ff7bdc74690fc8da4ed3))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(78c099e1) SHA1(953111237fdc3e20562d823eb2b6430e5a4afe4d))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(c890475f) SHA1(1cf6ed0dbd003a76a5cf889f62b489c0a62e9d25))
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x80000, 0x20000)
	ROM_RELOAD(0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(aba8fd98) SHA1(81b8af4d2d8e40b5b44f114c095371afe5539549))
	ROM_RELOAD(0x60000, 0x20000)
	ROM_RELOAD(0xa0000, 0x20000)
	ROM_RELOAD(0xe0000, 0x20000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4554ed0d) SHA1(df0a9225f961e0ee876c3e63ad54c6e4eac080ae))
ROM_END

/*-------------------------------------------------------------------
/ Car Hop (#725)
/-------------------------------------------------------------------*/
ROM_START(carhop)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(164b2c9c) SHA1(49cf7e3a3acb5de8dbfd2ad22f8bd9a352ff2899))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9dec74e7) SHA1(8234bdca5536d30dc1eabcb3a5505d2fd824ce0f))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(831ee812) SHA1(57056cde36b17cb7d7f34275b1bb5dc3d52bde4e))
ROM_END

/*-------------------------------------------------------------------
/ Caribbean Cruise (#C102)
/ One-sided cocktail designed and built by Gottlieb for International Concepts
/-------------------------------------------------------------------*/
ROM_START(ccruise)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(668b5757) SHA1(8ff955e8598ffdc68eab7fd69c6a67c4eed13f0f))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000,  0x8000, CRC(4480257e) SHA1(50b93d4496816ef7cdf007ac75c72c6aaa956aba))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(f8cec60c) SHA1(e52f3a5890a3bb5eb6c932c3d0ed471ed76909c9))
	ROM_RELOAD(0x40000,  0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(6e424e53) SHA1(90a9bf5ce84680972f9d12eb386215494c584b9b))
ROM_END

/*-------------------------------------------------------------------
/ Class of 1812 (#730)
/-------------------------------------------------------------------*/
ROM_START(clas1812)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(564349bf) SHA1(458eb2ece924a20d309dce7117c94e75b4a21fd7))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(3863a9df) SHA1(1759abbfcb127a6909f70845f41daf3ac8e80cef))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(357b0069) SHA1(870b0b84c6b3754f89b4e4e0b4594613ef589204))
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x80000, 0x20000)
	ROM_RELOAD(0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(5be02ff7) SHA1(51af73a26bbed0915ec57cde8f9cac552978b2dc))
	ROM_RELOAD(0x60000, 0x20000)
	ROM_RELOAD(0xa0000, 0x20000)
	ROM_RELOAD(0xe0000, 0x20000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(4ecf6ecb) SHA1(92469ccdedcc8e61edcddaedd688ef990a9ad5ad))
ROM_END

/*-------------------------------------------------------------------
/ Deadly Weapon (#724)
/-------------------------------------------------------------------*/
ROM_START(deadweap)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(07d84b32) SHA1(25d8772a5c8655b3406df94563076719b07129cd))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(f55dd7ec) SHA1(fe306c40bf3d98e4076d0d8a935c3671469d4cff))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(93369ed3) SHA1(3340478ffc00cf9991beabd4f0ecd89d0c88965e))
ROM_END

/*-------------------------------------------------------------------
/ Hoops (#727)
/-------------------------------------------------------------------*/
ROM_START(hoops)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(78391273) SHA1(dbf91597ce2910e526fb5e82355ad862706b4975))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(e72c00eb) SHA1(5b9f85083b38d916afb0f9b72b061501504725ff))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(9718b958) SHA1(bac806267bab4852c0f3fdb48f8d872992f61ace))
ROM_END

/*-------------------------------------------------------------------
/ Lights, Camera, Action (#720)
/-------------------------------------------------------------------*/
ROM_START(lca)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x8000, 0x8000, CRC(52957d70) SHA1(0c24d824b1aa966eb3af3db3ff02870ba463dcd6))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

ROM_START(lca2)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom2.bin", 0x8000, 0x8000, CRC(937a8426) SHA1(6bc2d1b0c3dc273577376654ba72b60febe32529))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(a258d72d) SHA1(eeb4768c8b2f57509a088d3ac8d35aa34f2cfc2c))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20919ebb) SHA1(a2ea79863b41a04aa23ea596932825408cca64e3))
ROM_END

/*-------------------------------------------------------------------
/ Nudge-It (N102) Redemption Machine
/-------------------------------------------------------------------*/
ROM_START(nudgeit)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(3d9e0309) SHA1(caaa28482e7f260668aa05b39b551acb8e4cc41a))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ae0c4b1d) SHA1(c8aa409c9b54fd8ecf70eb2926f4e98fc5eb11fe))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(65fc2e60) SHA1(6377c220753d9e4b5c76d445056409526d95772f))
ROM_END

/*-------------------------------------------------------------------
/ Operation: Thunder (#732)
/-------------------------------------------------------------------*/
ROM_START(opthund)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(96a128c2) SHA1(4032c5191b167a0498371207666a1f73155b7a74))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(db28be69) SHA1(6c505c34c8bdccc43dd8f310f01dd3a6b49e8059))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(0fbb130a) SHA1(a171c20f861dac5918c5b410e2a2bdd6e7c0553b))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(0f7632b3) SHA1(a122a062448139d5c1a9daa7d827c3073aa194f7))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(169816d1) SHA1(d23b1d8d1b841ca065a485e80805ecc6342ce57b))
ROM_END

/*-------------------------------------------------------------------
/ Silver Slugger (#722)
/-------------------------------------------------------------------*/
ROM_START(silvslug)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(a6c524e2) SHA1(dc12dd8e814a37aada021f84c58475efe72cb846))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(eac3e1cc) SHA1(2725457231854e4f3d54fbba745b8fc6f55b1688))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(20bc9797) SHA1(5d17b5f0423d8854fb7c4816d53a223ecc7c50c6))
ROM_END

/*-------------------------------------------------------------------
/ Surf'n Safari (#731)
/-------------------------------------------------------------------*/
ROM_START(surfnsaf)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(ac3393bd) SHA1(f9c533b937b5ca5698b805ed6ed573cb22383d9d))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(ec8fc963) SHA1(247e76d87beb3339e7d55292f9eadd2351621cfa))

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x40000, CRC(38b569b2) SHA1(93be47916a92541d097233b60a42eb7ca587ce52))
	ROM_RELOAD(0x40000, 0x40000)
	ROM_LOAD("arom2.bin", 0x80000, 0x40000, CRC(224c2021) SHA1(6b426097a2870b3b32d786be6e66ba6be9f54c29))
	ROM_RELOAD(0xc0000, 0x40000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(a0480418) SHA1(a982564d5dbf52275c2e7223687b07cf4ca0a115))
ROM_END

/*-------------------------------------------------------------------
/ Title Fight (#726)
/-------------------------------------------------------------------*/
ROM_START(tfight)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(43b3193a) SHA1(bd185fe67c147a6acca8e78da4b77c384124fc46))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(9514739f) SHA1(2794549f549d68e064a9a962a4e91fff7dcf0160))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(8591d421) SHA1(74402cf8b419e0cb05069851b0d5616e66b2f0a9))
ROM_END

/*-------------------------------------------------------------------
/ Vegas (#723)
/-------------------------------------------------------------------*/
ROM_START(vegas)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(48189981) SHA1(95144af4b222158becd4d5748d15b7b6c6021bd2))

	ROM_REGION(0x10000, "p5sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, CRC(46eb5755) SHA1(94ec2d0cf41f68a8c3d7505186b11b4abb4803db))

	ROM_REGION(0x10000, "p5sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, CRC(af1095f1) SHA1(06609085cd74b969e4f2ec962c427c5c42ebc6ff))
ROM_END


/************************************/
/* NOT OFFICIALLY LISTED            */
/************************************/

/*-------------------------------------------------------------------
/ Unnamed game? by Toptronic HGmbH, Germany
/ Shows as "GAME NAME" in display, (#7**)
/-------------------------------------------------------------------*/
ROM_START(tt_game)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASEFF)
	ROM_LOAD("gprom.bin", 0x0000, 0x10000, CRC(e7944b75) SHA1(b73f2e0004556c8aa88baef0cddcdefb5b905b8d))

	ROM_REGION(0x10000, "p7sound:audiocpu", ROMREGION_ERASEFF)
	ROM_LOAD("drom1.bin", 0x8000, 0x8000, NO_DUMP)

	ROM_REGION(0x100000, "p7sound:oki", ROMREGION_ERASEFF)
	ROM_LOAD("arom1.bin", 0x00000, 0x20000, CRC(b0983d90) SHA1(72e6a71f20fd5849543ca13813f062a3fc1d7dcf))
	ROM_RELOAD(0x40000, 0x20000)
	ROM_RELOAD(0x80000, 0x20000)
	ROM_RELOAD(0xc0000, 0x20000)
	ROM_LOAD("arom2.bin", 0x20000, 0x20000, CRC(3e31ce58) SHA1(a2ef72d7b2bb821d1f62dce7212e31a1df3e7791))
	ROM_RELOAD(0x60000, 0x20000)
	ROM_RELOAD(0xa0000, 0x20000)
	ROM_RELOAD(0xe0000, 0x20000)

	ROM_REGION(0x10000, "p7sound:speechcpu", ROMREGION_ERASEFF)
	ROM_LOAD("yrom1.bin", 0x8000, 0x8000, NO_DUMP)
ROM_END

} // anonymous namespace

GAME(1989,  lca,      0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Lights...Camera...Action!",         MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  lca2,     lca, p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Lights...Camera...Action! (rev.2)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  silvslug, 0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Silver Slugger",                    MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  vegas,    0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Vegas",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  deadweap, 0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Deadly Weapon",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  tfight,   0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Title Fight",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  nudgeit,  0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Nudge-It",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1990,  bellring, 0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Bell Ringer",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1991,  carhop,   0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Car Hop",                           MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1991,  hoops,    0,   p5, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Hoops",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1991,  cactjack, 0,   p7, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Cactus Jack's",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1991,  clas1812, 0,   p7, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Class of 1812",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1991,  surfnsaf, 0,   p7, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Surf'n Safari",                     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1992,  opthund,  0,   p7, gts3, gts3_state, empty_init, ROT0, "Gottlieb", "Operation: Thunder",                MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(19??,  tt_game,  0,   p7, gts3, gts3_state, empty_init, ROT0, "Toptronic", "unknown Toptronic pinball game",   MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME(1989,  ccruise,  0,   p7, gts3, gts3_state, empty_init, ROT0, "International Concepts","Caribbean Cruise",     MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
