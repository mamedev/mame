// license:BSD-3-Clause
// copyright-holders:Kevin Horton, Jonathan Gevaryahu, Sandro Ronco, hap
/*******************************************************************************

Fidelity Voice Chess Challenger series hardware
- Voice Chess Challenger (VCC) (2 revisions)
- Advanced Voice Chess Challenger (UVC)
- Grandmaster Voice Chess Challenger (Fidelity Deutschland product)
- Decorator Challenger (FCC)

Grandmaster and FCC are verified to be the same PCB + ROMs as UVC. So even though
they have a large wooden chessboard attached instead of a small plastic one, from
MAME's perspective there's nothing to emulate on top of UVC.

TODO:
- add low-pass filters to sound? but when using flt_rc, it does not sound like
  recordings from a real VCC, maybe use a netlist or is it overkill? (same goes
  for newer Fidelity chess computers with this speech chip)

BTANB:
- with the English voice ROM, the letter D is barely distinguishable from E,
  Fidelity never updated the ROM later, and it sounds fine with other languages

================================================================================

RE notes by Kevin Horton

The CPU is a Z80 running at 4MHz. The TSI chip runs at around 25KHz, using a
470K / 100pf RC network. This system is very very basic, and is composed of just
the Z80, 4 ROMs, the TSI chip, and an 8255.

The Z80's interrupt inputs are all pulled to VCC, so no interrupts are used.

Reset is connected to a power-on reset circuit and a button on the keypad (marked RE).

The TSI chip connects to a 4K ROM. All of the 'Voiced' Chess Challengers
use this same ROM  (three or four). The later chess boards use a slightly different
part number, but the contents are identical.

The speech chip analog out (pin 11) goes to a PNP transistor, followed by two
cascaded low-pass filters (18K+5nf and 18K+20nf), an LM386N amplifier, and a
speaker. Newer Fidelity chess computers with this chip have a similar configuration,
with an additional volume filter before the LM386N.

Memory map (VCC):
-----------------
0000-0FFF: 4K 2332 ROM VCC1 or 101-32013
1000-1FFF: 4K 2332 ROM VCC2
2000-2FFF: 4K 2332 ROM VCC3
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

Memory map (UVC):
-----------------
0000-1FFF: 8K 2364 ROM 101-64017
2000-2FFF: 4K 2332 ROM 101-32010 or VCC3
4000-5FFF: 1K RAM (2114 SRAM x2)
6000-FFFF: empty

Port map:
---------
00-03: 8255 port chip, mirrored over the 00-FF range; program accesses F4-F7

8255 connections:
-----------------
PA.0 - segment G, TSI A0 (W)
PA.1 - segment F, TSI A1 (W)
PA.2 - segment E, TSI A2 (W)
PA.3 - segment D, TSI A3 (W)
PA.4 - segment C, TSI A4 (W)
PA.5 - segment B, TSI A5 (W)
PA.6 - segment A, language latch Data (W)
PA.7 - TSI START line, language latch clock (W, see below)

PB.0 - dot commons (W)
PB.1 - NC
PB.2 - digit 0, bottom dot (W)
PB.3 - digit 1, top dot (W)
PB.4 - digit 2 (W)
PB.5 - digit 3 (W)
PB.6 - enable language jumpers (W, see below)
PB.7 - TSI BUSY line (R)

(button rows pulled up to 5V through 2.2K resistors)
PC.0 - button row 0, German language jumper (R)
PC.1 - button row 1, French language jumper (R)
PC.2 - button row 2, Spanish language jumper (R)
PC.3 - button row 3, special language jumper (R)
PC.4 - button column A (W)
PC.5 - button column B (W)
PC.6 - button column C (W)
PC.7 - button column D (W)

Language jumpers:
-----------------
When PB.6 is pulled low, the language jumpers can be read. There are four.
They connect to the button rows. When enabled, the row(s) will read low if
the jumper is present. English only VCC's do not have the 367 or any pads stuffed.
The jumpers are labeled: French, German, Spanish, and special.

Language latch:
---------------
There's an unstuffed 7474 on the board that connects to PA.6 and PA.7. It allows
one to latch the state of A12 to the speech ROM. The English version has the chip
missing, and a jumper pulling "A12" to ground. This line is really a negative
enable.

To make the VCC multi-language, one would install the 74367 (note: it must be a 74367
or possibly a 74LS367. A 74HC367 would not work since they rely on the input current
to keep the inputs pulled up), solder a piggybacked ROM to the existing English
speech ROM, and finally install a 7474 dual flipflop.

This way, the game can then detect which secondary language is present, and then
it can automatically select the correct ROM(s). I have to test whether it will do
automatic determination and give you a language option on power up or something.

*******************************************************************************/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "sound/s14001a.h"
#include "video/pwm.h"

#include "speaker.h"

// internal artwork
#include "fidel_vcc.lh"


namespace {

class vcc_state : public driver_device
{
public:
	vcc_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_ppi8255(*this, "ppi8255"),
		m_display(*this, "display"),
		m_speech(*this, "speech"),
		m_language(*this, "language"),
		m_inputs(*this, "IN.%u", 0)
	{ }

	// RE button is tied to Z80 RESET pin
	DECLARE_INPUT_CHANGED_MEMBER(reset_button) { m_maincpu->set_input_line(INPUT_LINE_RESET, newval ? ASSERT_LINE : CLEAR_LINE); }

	// machine configs
	void vcc(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;

private:
	// devices/pointers
	required_device<cpu_device> m_maincpu;
	required_device<i8255_device> m_ppi8255;
	required_device<pwm_display_device> m_display;
	required_device<s14001a_device> m_speech;
	required_region_ptr<u8> m_language;
	required_ioport_array<4> m_inputs;

	u8 m_led_select = 0;
	u8 m_7seg_data = 0;
	u8 m_inp_mux = 0;

	// address maps
	void main_map(address_map &map) ATTR_COLD;
	void main_io(address_map &map) ATTR_COLD;

	// I/O handlers
	void update_display();
	void ppi_porta_w(u8 data);
	u8 ppi_portb_r();
	void ppi_portb_w(u8 data);
	u8 ppi_portc_r();
	void ppi_portc_w(u8 data);
};

void vcc_state::machine_start()
{
	// register for savestates
	save_item(NAME(m_led_select));
	save_item(NAME(m_7seg_data));
	save_item(NAME(m_inp_mux));

	// game relies on RAM filled with FF at power-on
	for (int i = 0; i < 0x400; i++)
		m_maincpu->space(AS_PROGRAM).write_byte(i + 0x4000, 0xff);
}



/*******************************************************************************
    I/O
*******************************************************************************/

void vcc_state::update_display()
{
	// 4 7seg leds (note: sel d0 for extra leds)
	u8 outdata = (m_7seg_data & 0x7f) | (m_led_select << 7 & 0x80);
	m_display->matrix(m_led_select >> 2 & 0xf, outdata);
}

void vcc_state::ppi_porta_w(u8 data)
{
	// d0-d6: digit segment data, bits are xABCDEFG
	m_7seg_data = bitswap<8>(data,7,0,1,2,3,4,5,6);
	update_display();

	// d6: language latch data
	// d7: language latch clock (latch on high)
	if (data & 0x80)
		m_speech->set_rom_bank(BIT(data, 6));

	// d0-d5: S14001A C0-C5
	// d7: S14001A start pin
	m_speech->data_w(data & 0x3f);
	m_speech->start_w(BIT(data, 7));
}

u8 vcc_state::ppi_portb_r()
{
	// d7: S14001A busy pin
	return (m_speech->busy_r()) ? 0x80 : 0x00;
}

void vcc_state::ppi_portb_w(u8 data)
{
	// d0,d2-d5: digit/led select
	// _d6: enable language jumpers
	m_led_select = data;
	update_display();
}

u8 vcc_state::ppi_portc_r()
{
	u8 data = 0;

	// d0-d3: multiplexed inputs (active low)
	for (int i = 0; i < 4; i++)
		if (BIT(m_inp_mux, i))
			data |= m_inputs[i]->read();

	// also language jumpers (hardwired)
	// 0(no jumper): English, 1: German, 2: French, 4: Spanish, 8: Special(unused)
	if (~m_led_select & 0x40)
		data |= *m_language;

	return ~data & 0xf;
}

void vcc_state::ppi_portc_w(u8 data)
{
	// d4-d7: input mux (inverted)
	m_inp_mux = ~data >> 4 & 0xf;
}



/*******************************************************************************
    Address Maps
*******************************************************************************/

void vcc_state::main_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x2fff).rom();
	map(0x4000, 0x43ff).mirror(0x1c00).ram();
}

void vcc_state::main_io(address_map &map)
{
	map.global_mask(0x03);
	map(0x00, 0x03).rw(m_ppi8255, FUNC(i8255_device::read), FUNC(i8255_device::write));
}



/*******************************************************************************
    Input Ports
*******************************************************************************/

static INPUT_PORTS_START( vcc )
	PORT_START("IN.0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("LV") PORT_CODE(KEYCODE_L)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("A1") PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD) PORT_CODE(KEYCODE_A)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("E5") PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD) PORT_CODE(KEYCODE_E)

	PORT_START("IN.1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("Speaker") PORT_CODE(KEYCODE_SPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("DM") PORT_CODE(KEYCODE_M)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("B2") PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD) PORT_CODE(KEYCODE_B)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("F6") PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD) PORT_CODE(KEYCODE_F)

	PORT_START("IN.2")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("CL") PORT_CODE(KEYCODE_DEL) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PB") PORT_CODE(KEYCODE_P)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("C3") PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD) PORT_CODE(KEYCODE_C)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("G7") PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD) PORT_CODE(KEYCODE_G)

	PORT_START("IN.3")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("EN") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("PV") PORT_CODE(KEYCODE_V)
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("D4") PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD) PORT_CODE(KEYCODE_D)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("H8") PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD) PORT_CODE(KEYCODE_H)

	PORT_START("RESET") // is not on matrix IN.0 d0
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_KEYPAD) PORT_NAME("RE") PORT_CODE(KEYCODE_R) PORT_CHANGED_MEMBER(DEVICE_SELF, vcc_state, reset_button, 0)
INPUT_PORTS_END



/*******************************************************************************
    Machine Configs
*******************************************************************************/

void vcc_state::vcc(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 4_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &vcc_state::main_map);
	m_maincpu->set_addrmap(AS_IO, &vcc_state::main_io);

	I8255(config, m_ppi8255);
	m_ppi8255->out_pa_callback().set(FUNC(vcc_state::ppi_porta_w));
	m_ppi8255->tri_pa_callback().set_constant(0);
	m_ppi8255->in_pb_callback().set(FUNC(vcc_state::ppi_portb_r));
	m_ppi8255->out_pb_callback().set(FUNC(vcc_state::ppi_portb_w));
	m_ppi8255->tri_pb_callback().set_constant(0);
	m_ppi8255->in_pc_callback().set(FUNC(vcc_state::ppi_portc_r));
	m_ppi8255->out_pc_callback().set(FUNC(vcc_state::ppi_portc_w));

	// video hardware
	PWM_DISPLAY(config, m_display).set_size(4, 8);
	m_display->set_segmask(0xf, 0x7f);
	config.set_default_layout(layout_fidel_vcc);

	// sound hardware
	SPEAKER(config, "speaker").front_center();
	S14001A(config, m_speech, 25000); // R/C circuit, around 25khz
	m_speech->add_route(ALL_OUTPUTS, "speaker", 0.75);
}



/*******************************************************************************
    ROM Definitions
*******************************************************************************/

ROM_START( vcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19256n_101-32013", 0x0000, 0x1000, CRC(257bb5ab) SHA1(f7589225bb8e5f3eac55f23e2bd526be780b38b5) )
	ROM_LOAD("cn19174n_vcc_2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc_3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 0, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 4, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( vcca )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("cn19173n_vcc_1", 0x0000, 0x1000, CRC(6fab0464) SHA1(b917cfb488cfd73ed776d3838289623585530181) )
	ROM_LOAD("cn19174n_vcc_2", 0x1000, 0x1000, CRC(f33095e7) SHA1(692fcab1b88c910b74d04fe4d0660367aee3f4f0) )
	ROM_LOAD("cn19175n_vcc_3", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) )

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 0, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 4, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) )
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

ROM_START( avcc )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD("101-64017", 0x0000, 0x2000, CRC(f1133abf) SHA1(09dd85051c4e7d364d43507c1cfea5c2d08d37f4) ) // MOS // 101-64017 // 3880
	ROM_LOAD("101-32010", 0x2000, 0x1000, CRC(624f0cd5) SHA1(7c1a4f4497fe5882904de1d6fecf510c07ee6fc6) ) // NEC P9Z021 // D2332C 228 // 101-32010, == cn19175n_vcc3 on vcc

	// speech ROM
	ROM_DEFAULT_BIOS("en")
	ROM_SYSTEM_BIOS(0, "en", "English")
	ROM_SYSTEM_BIOS(1, "de", "German")
	ROM_SYSTEM_BIOS(2, "fr", "French")
	ROM_SYSTEM_BIOS(3, "sp", "Spanish")

	ROM_REGION( 1, "language", 0 )
	ROMX_FILL(0, 1, 0, ROM_BIOS(0) )
	ROMX_FILL(0, 1, 1, ROM_BIOS(1) )
	ROMX_FILL(0, 1, 2, ROM_BIOS(2) )
	ROMX_FILL(0, 1, 4, ROM_BIOS(3) )

	ROM_REGION( 0x2000, "speech", 0 )
	ROMX_LOAD("101-32107", 0x0000, 0x1000, CRC(f35784f9) SHA1(348e54a7fa1e8091f89ac656b4da22f28ca2e44d), ROM_BIOS(0) ) // NEC P9Y019 // D2332C 229 // 101-32107
	ROM_RELOAD(            0x1000, 0x1000)
	ROMX_LOAD("101-64101", 0x0000, 0x2000, CRC(6c85e310) SHA1(20d1d6543c1e6a1f04184a2df2a468f33faec3ff), ROM_BIOS(1) )
	ROMX_LOAD("101-64105", 0x0000, 0x2000, CRC(fe8c5c18) SHA1(2b64279ab3747ee81c86963c13e78321c6cfa3a3), ROM_BIOS(2) )
	ROMX_LOAD("101-64106", 0x0000, 0x2000, CRC(8766e128) SHA1(78c7413bf240159720b131ab70bfbdf4e86eb1e9), ROM_BIOS(3) )
ROM_END

} // anonymous namespace



/*******************************************************************************
    Drivers
*******************************************************************************/

//    YEAR  NAME  PARENT  COMPAT  MACHINE  INPUT  CLASS      INIT        COMPANY, FULLNAME, FLAGS
SYST( 1979, vcc,  0,      0,      vcc,     vcc,   vcc_state, empty_init, "Fidelity Electronics", "Voice Chess Challenger (set 1)", MACHINE_SUPPORTS_SAVE )
SYST( 1979, vcca, vcc,    0,      vcc,     vcc,   vcc_state, empty_init, "Fidelity Electronics", "Voice Chess Challenger (set 2)", MACHINE_SUPPORTS_SAVE )

SYST( 1980, avcc, vcc,    0,      vcc,     vcc,   vcc_state, empty_init, "Fidelity Electronics", "Advanced Voice Chess Challenger", MACHINE_SUPPORTS_SAVE )
