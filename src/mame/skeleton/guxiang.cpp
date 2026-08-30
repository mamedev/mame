// license::BSD-3-Clause
// copyright-holders:
/*************************************************************************
故乡 / GuXiang

Hw info: (Partially)
sub-board
c3 pal16l8acn
c4 wd1017-pl
c5 hd74ls373p

Led Board
sw1 - 8 dip switch
24  Roulette leds Winning features a dual-color red and green light and has 3 pins.
12 multiplier leds
2x 7 segment display 2 digits for the bonus
2x 7 segment display 2 digits for the credits
4x 7 segment display color green? - 1 digits
4x 7 segment display color green? - 1 digits
2x 7 segment display 2 digits
8x 7 segment display 2 digits for the bet.
U8? hm6116l-90
U1 u29 hd74hc00p
u10 w27c512-45z
U17 hd74ls02p
U15 hd74ls373p?
??? hd74ls08p
U2  hd74ls04p
u11 hd? 74lS38p
?? lm380n
U9 um3567 - opll
u46 unmarked.
u44 hd74ls374p
u23 u33 d8279c-2
u35 u40 d8255ac-2

u14 u47 Microchip AY38910A/P
u28 u2? u31 u32 mc14511bcp
u21 u22 u25 u33 mc14028bcp
u19 U36 U38 u26 u24 u41 u43 u42 ?? ?? ?? pa2003c
U5  U6  75462
u? labeled 8051 near of socket. unknown model (8052 microcontroller)
X1 10.73863 KDS 5G
X2 Cei 3.579545 MHz
There is data on a small PCB with a 16V8 chip that has not been dumped.
To reset it, press K0 and K3 simultaneously to power on, and the lights will start flashing.

guxiang is a scrambled ROM that must be flashed to 27512.
The 16v8 decryption from the backpack micro-card needs to be passed to the CPU for execution.

Guxianga.
no Hw info Provided
u? w78e065 (8052 microcontroller)
There are two versions of Gu Xiang’s PCB. The early version used confusing code, with the ROM stored in the 27C512, with a 16v8 misalignment method.
Later, it was cracked and pirated, resulting in uncracked code appearing on the market. The ROM was stored in microcontrollers like the W78E065, with hardware encryption.
To reset set the dip switch 8 on then restart,  will shown number, set off the dip switch 8 and restart.
*/

#include "emu.h"

#include "cpu/mcs51/i80c52.h"

#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "speaker.h"

#include "guxiang.lh"

namespace {

class guxiang_state : public driver_device
{
public:
	guxiang_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "SW1", "PUSHBUTTONS" })
		, m_p1(*this, "P1")
		, m_hopper(*this, "hopper")

	{ }

	void guxiang(machine_config &config);
	void init_dec();

protected:
	virtual void machine_start() override;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);

	void display_7seg_data2_w(uint8_t data);
	void multiplex_7seg2_w(uint8_t data);

	void program_map(address_map &map);
	void data_map(address_map &map);

	void ppi1_porta_w(uint8_t data) ATTR_COLD;
	void ppi1_portb_w(uint8_t data) ATTR_COLD;
	void ppi1_portc_w(uint8_t data) ATTR_COLD;
	void ppi2_porta_w(uint8_t data) ATTR_COLD;
	void ppi2_portb_w(uint8_t data) ATTR_COLD;
	void ppi2_portc_w(uint8_t data) ATTR_COLD;
	void ay1_port_a_w(uint8_t data);
	void ay1_port_b_w(uint8_t data);
	void ay2_port_a_w(uint8_t data);
	void ay2_port_b_w(uint8_t data);

	uint8_t p1_port_r();
	void p1_port_w(uint8_t data);

	uint8_t keyboard_r();
	uint8_t m_selected_7seg_module = 0;
	uint8_t m_selected_7seg_module2 = 0;

	uint8_t m_p1_out = 0xff;

	output_finder<64> m_digits;
	output_finder<80> m_leds;
	required_ioport_array<4> m_inputs;
	required_ioport m_p1;
	required_device<hopper_device> m_hopper;

};
static INPUT_PORTS_START( guxiang )

	PORT_START("KEYS1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 8" ) PORT_CODE( KEYCODE_8_PAD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 7" ) PORT_CODE( KEYCODE_7_PAD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 6" ) PORT_CODE( KEYCODE_6_PAD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 5" ) PORT_CODE( KEYCODE_5_PAD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 4" ) PORT_CODE( KEYCODE_4_PAD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 3" ) PORT_CODE( KEYCODE_3_PAD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 2" ) PORT_CODE( KEYCODE_2_PAD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_NAME( "Bet 1" ) PORT_CODE( KEYCODE_1_PAD)


	PORT_START("KEYS2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_GAMBLE_HIGH )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_GAMBLE_LOW )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Unknown" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Shift Right" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Shift Left" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1  ) PORT_IMPULSE(3)

	PORT_START("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Maximum Bet?" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "SW1:8")

	PORT_START("PUSHBUTTONS")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K0")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K1")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K2")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K3")
	PORT_BIT(0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "P1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P1:6" )

	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_CUSTOM)  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_MEMORY_RESET)

	PORT_START("P3")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "P3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "P3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "P3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "P3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "P3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "P3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "P3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "P3:8" )

INPUT_PORTS_END

static INPUT_PORTS_START( guxianga )
	PORT_INCLUDE( guxiang )
	PORT_MODIFY("SW1")
	PORT_DIPNAME( 0x01, 0x00, "Maximum Bet?" )  PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x01, "2" )
	PORT_DIPSETTING(    0x00, "1" )

	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Demo_Sounds ) )  PORT_DIPLOCATION("SW1:2")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )

	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "SW1:7")
	PORT_DIPNAME( 0x80, 0x80, "Clear NVRAM" )  PORT_DIPLOCATION("SW1:8")
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )

INPUT_PORTS_END


uint8_t guxiang_state::p1_port_r()
{
	uint8_t const ioport_val = m_p1->read();
	uint8_t meter_fb = 0x00;

	if (!BIT(m_p1_out, 0))
		meter_fb = (BIT(m_p1_out, 1) << 4) | (BIT(m_p1_out, 2) << 5);

	return (ioport_val & 0xcf) | meter_fb;
}

void guxiang_state::p1_port_w(uint8_t data)
{

m_hopper->motor_w(BIT(data, 3));
m_p1_out = data;

}

void guxiang_state::ppi1_porta_w(uint8_t data)
{

	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(~data, i);
}

void guxiang_state::ppi1_portb_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(~data, i);
}

void guxiang_state::ppi1_portc_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(~data, i);
}

void guxiang_state::ppi2_porta_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 24] = BIT(~data, i);
}

void guxiang_state::ppi2_portb_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 32] = BIT(~data, i);
}
void guxiang_state::ppi2_portc_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 40] = BIT(~data, i);
}

void guxiang_state::ay1_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 48] = BIT(data, i);
}

void guxiang_state::ay1_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 56] = BIT(data, i);
}

void guxiang_state::ay2_port_a_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 64] = BIT(data, i);
}

void guxiang_state::ay2_port_b_w(uint8_t data)
{
	for (uint8_t i = 0; i < 8; i++)
		m_leds[i + 72] = BIT(data, i);
}

void guxiang_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t guxiang_state::keyboard_r()
{
switch (m_selected_7seg_module & 0x07)
	{
	case 0:

	case 1:

	case 2:

	case 3:
		return m_inputs[m_selected_7seg_module & 0x07]->read();

	default:
		return 0x00;
	}
}

void guxiang_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // Code was from marywu to decode 7 segment display.

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void guxiang_state::multiplex_7seg2_w(uint8_t data)
{
	m_selected_7seg_module2 = data;
}

void guxiang_state::display_7seg_data2_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // Code was from marywu to decode 7 segment display.

m_digits[32 + (2 * m_selected_7seg_module2) + 0] = patterns[data & 0x0f];
m_digits[32 + (2 * m_selected_7seg_module2) + 1] = patterns[data >> 4];
}

void guxiang_state::program_map(address_map &map)
{
	map(0x0000, 0xffff).rom();

}

void guxiang_state::data_map(address_map &map)
{
map(0x8000, 0x87ff).ram().share("nvram");
map(0x9000, 0x9001).rw("ay1", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
map(0x9002, 0x9003).rw("ay2", FUNC(ay8910_device::data_r), FUNC(ay8910_device::address_data_w));
map(0xa000, 0xa003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
map(0xb000, 0xb001).rw("i8279_1", FUNC(i8279_device::read), FUNC(i8279_device::write));
map(0xc000, 0xc001).w("opll", FUNC(ym2413_device::write));
map(0xd000, 0xd001).rw("i8279_2", FUNC(i8279_device::read), FUNC(i8279_device::write));
map(0xe000, 0xe003).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));

}

void guxiang_state::init_dec()
{
	uint8_t *rom = memregion("maincpu")->base();
	const uint32_t size = memregion("maincpu")->bytes();

	std::vector<uint8_t> buffer(rom, rom + size);

	static const uint8_t page_map[8] =
	{
		0, 2, 4, 6, 1, 3, 5, 7
	};

	for (uint32_t i = 0; i < size; i++)
	{
		uint32_t addr = i;

		// page inside each 0x800-byte block
		uint32_t page = (i >> 8) & 7;

		addr =
			(i & ~0x0700) |
			(page_map[page] << 8) |
			(i & 0xff);

		rom[i] = buffer[addr];
	}
}

void guxiang_state::machine_start()
{
	save_item(NAME(m_selected_7seg_module));
	save_item(NAME(m_p1_out));

}

void guxiang_state::guxiang(machine_config &config)
{

	/* basic machine hardware */
	i80c52_device &maincpu(I80C52(config, "maincpu", XTAL(10'738'635)));
	maincpu.set_addrmap(AS_PROGRAM, &guxiang_state::program_map);
	maincpu.set_addrmap(AS_DATA, &guxiang_state::data_map);
	maincpu.port_in_cb<1>().set(FUNC(guxiang_state::p1_port_r));
	maincpu.port_out_cb<1>().set(FUNC(guxiang_state::p1_port_w));
	maincpu.port_in_cb<3>().set_ioport("P3");

	HOPPER(config, m_hopper, attotime::from_msec(100));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
	i8279_device &kbdc1(I8279(config, "i8279_1", XTAL(10'738'635) / 6)); // divisor not verified
	kbdc1.out_sl_callback().set(FUNC(guxiang_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc1.in_rl_callback().set(FUNC(guxiang_state::keyboard_r));          // keyboard Return Lines
	kbdc1.out_disp_callback().set(FUNC(guxiang_state::display_7seg_data_w));


	i8279_device &kbdc2(I8279(config, "i8279_2", XTAL(10'738'635) / 6)); // divisor not verified
	kbdc2.out_sl_callback().set(FUNC(guxiang_state::multiplex_7seg2_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc2.out_disp_callback().set(FUNC(guxiang_state::display_7seg_data2_w));

	/* Programmable Peripheral Interface */
	i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(guxiang_state::ppi1_porta_w));
	ppi1.out_pb_callback().set(FUNC(guxiang_state::ppi1_portb_w));
	ppi1.out_pc_callback().set(FUNC(guxiang_state::ppi1_portc_w));

 //  I8255A(config, "ppi2");
	i8255_device &ppi2(I8255A(config, "ppi2"));

	ppi2.out_pa_callback().set(FUNC(guxiang_state::ppi2_porta_w));
	ppi2.out_pb_callback().set(FUNC(guxiang_state::ppi2_portb_w));
	ppi2.out_pc_callback().set(FUNC(guxiang_state::ppi2_portc_w));

	/* Video */
	config.set_default_layout(layout_guxiang);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();


	ay8910_device &ay1(AY8910(config, "ay1", XTAL(10'738'635) / 6));
	ay1.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay1.port_a_write_callback().set(FUNC(guxiang_state::ay1_port_a_w));
	ay1.port_b_write_callback().set(FUNC(guxiang_state::ay1_port_b_w));

	ay8910_device &ay2(AY8910(config, "ay2", XTAL(10'738'635) / 6));
	ay2.add_route(ALL_OUTPUTS, "mono", 0.50);
	ay2.port_a_write_callback().set(FUNC(guxiang_state::ay2_port_a_w));
	ay2.port_b_write_callback().set(FUNC(guxiang_state::ay2_port_b_w));

	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono",0.50);

}

ROM_START( guxiang)
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w27c512.u10", 0x00000, 0x10000, CRC(BA39B626) SHA1(e7a6dea3286c2c6177c2d38eaab851e833621b8e) ) // w27c512-45z
	ROM_REGION( 0x200, "plds", 0 )
	ROM_LOAD( "pal16l8acn.c3",   0x0000, 0x104, NO_DUMP )
ROM_END

ROM_START( guxianga )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "w78e065.bin", 0x00000, 0x10000, CRC(F772DCB4) SHA1(08e3efc853e7fb7de5b1fd039367487344f2270d) )


ROM_END


} // anonymous namespace


//    YEAR    NAME         PARENT   MACHINE      INPUT      STATE            INIT        ROT      COMPANY        FULLNAME                                FLAGS
GAME( 1991,  guxiang,      0,       guxiang,     guxiang,  guxiang_state,     init_dec,   ROT0,    "<unknown>",  "GuXiang (Encrypted) ",                  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
GAME( 1991,  guxianga,     0,       guxiang,     guxianga,  guxiang_state,     empty_init, ROT0,    "<unknown>",  "GuXiang (Unencrypted) ",                MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
