// license:BSD-3-Clause
// copyright-holders:flama12333
/*************************************************************************

Led board pcb labeled as BAOSHUN
IC4 IC6 D82C55AC-2
IC9 M5L879P-5

Main board labeled T5CARD Rev.0
?? RD1F35 - upd7810
U4 D27512J-1
U3 HM6116L-70
U12 - Sticker removed. EPM7032LC44-12
U20 D27C040
U19 7295
U?13 UM3567

*/

#include "emu.h"

#include "cpu/upd7810/upd7810.h"
#include "machine/nvram.h"
#include "machine/ticket.h"
#include "machine/i8255.h"
#include "machine/i8279.h"
#include "sound/okim6295.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "speaker.h"

#include "yqzhaoupd.lh"

namespace {

class yqzhaoupd_state : public driver_device
{
public:
	yqzhaoupd_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_digits(*this, "digit%u", 0U)
		, m_leds(*this, "led%u", 0U)
		, m_inputs(*this, { "KEYS1", "KEYS2", "unknown1", "unknown2" })
		, m_hopper(*this, "hopper")
		, m_oki(*this, "oki")

	{ }

void yqzhaoupd(machine_config &config);



protected:
	virtual void machine_start() override;

private:
	void display_7seg_data_w(uint8_t data);
	void multiplex_7seg_w(uint8_t data);
	void program_map(address_map &map);
	void data_map(address_map &map);

	void ppi1_porta_w(uint8_t data) ATTR_COLD;
	void ppi1_portb_w(uint8_t data) ATTR_COLD;
	void ppi1_portc_w(uint8_t data) ATTR_COLD;
	void ppi2_porta_w(uint8_t data) ATTR_COLD;
	void ppi2_portb_w(uint8_t data) ATTR_COLD;
	void ppi2_portc_w(uint8_t data) ATTR_COLD;
	void port_a_port_w(u8 data);
	void port_b_port_w(u8 data);
	void port_c_port_w(u8 data);
	void port_d_port_w(u8 data);
	void port_f_port_w(u8 data);

  uint8_t keyboard_r();
	uint8_t m_selected_7seg_module = 0;

	output_finder<32> m_digits;
	output_finder<48> m_leds;
	required_ioport_array<4> m_inputs;
	required_device<hopper_device> m_hopper;
	required_device<okim6295_device> m_oki;

};

static INPUT_PORTS_START( yqzhaoupd )
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
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME( "Single" )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_NAME( "Shift Right" )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON7 ) PORT_NAME( "Shift Left" )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_GAMBLE_PAYOUT )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_BUTTON8  ) PORT_NAME( "unknown" )

	PORT_START("unknown1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "unknown1:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "unknown1:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "unknown1:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "unknown1:4")
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "unknown1:5")
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "unknown1:6")
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "unknown1:7")
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "unknown1:8")

	PORT_START("unknown2")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "unknown:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "unknown:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "unknown:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "unknown:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "unknown:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "unknown:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "unknown:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "unknown:8" )

PORT_START("IN0") // Port A
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "IN0:1")
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "IN0:2")
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "IN0:3")
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "IN0:4")
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_CUSTOM)  PORT_READ_LINE_DEVICE_MEMBER("hopper", FUNC(hopper_device::line_r))
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_COIN1  ) PORT_IMPULSE(3)
	PORT_BIT(0x40, IP_ACTIVE_LOW,  IPT_GAMBLE_KEYIN)
	PORT_BIT(0x80, IP_ACTIVE_LOW,  IPT_MEMORY_RESET)
	PORT_START("IN1")  // Port B
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "IN1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "IN1:2" )
	PORT_DIPNAME( 0x04, 0x00, "Betting points" )  PORT_DIPLOCATION("IN1:3") // 押分
	PORT_DIPSETTING(    0x00, "Bet 5 points on 1 item" ) // 5分押1个 - 下
	PORT_DIPSETTING(    0x04, "Bet 1 points on 1 item" ) // 1分押1个 - 上
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "IN1:4" )
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_NAME("K3") // K0
	PORT_BIT(0x20, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_NAME("K2") // K1
	PORT_BIT(0x40, IP_ACTIVE_LOW, IPT_BUTTON3) PORT_NAME("K1") // K2
	PORT_BIT(0x80, IP_ACTIVE_LOW, IPT_BUTTON4) PORT_NAME("K0") // K1

PORT_START("IN2") // Port C
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "IN2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "IN2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "IN2:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "IN2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "IN2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "IN2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "IN2:7" ) // Must be set on. error 80 if disabled
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "IN2:8" ) // Must be set on error 80 if disabled

PORT_START("IN3")  // Port D
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "IN3:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "IN3:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "IN3:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "IN3:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "IN3:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "IN3:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "IN3:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "IN3:8" )

PORT_START("IN4")  // Port F
	PORT_DIPUNKNOWN_DIPLOC( 0x01, 0x01, "IN4:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02, 0x02, "IN4:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04, 0x04, "IN4:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x08, 0x08, "IN4:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, 0x10, "IN4:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, 0x20, "IN4:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, 0x40, "IN4:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, 0x80, "IN4:8" )

INPUT_PORTS_END

void yqzhaoupd_state::ppi1_porta_w(uint8_t data)
{

	for (uint8_t i = 0; i < 8; i++)
		m_leds[i] = BIT(~data, i);
}

void yqzhaoupd_state::ppi1_portb_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 8] = BIT(~data, i);
}

void yqzhaoupd_state::ppi1_portc_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 16] = BIT(~data, i);
}

void yqzhaoupd_state::ppi2_porta_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
   m_leds[i + 24] = BIT(~data, i);
}

void yqzhaoupd_state::ppi2_portb_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
		m_leds[i + 32] = BIT(~data, i);
}
void yqzhaoupd_state::ppi2_portc_w(uint8_t data)
{
	for (int i = 0; i < 8; i++)
	m_leds[i + 40] = BIT(~data, i);
}

void yqzhaoupd_state::port_a_port_w(u8 data)
{
// write

//logerror("PA Write to %02x\n", data);
m_hopper->motor_w(BIT(data, 3));
}

void yqzhaoupd_state::port_b_port_w(u8 data)
{
// write
// logerror("PB Write to %02x\n", data);
m_oki->set_rom_bank(BIT(data, 0));
}

void yqzhaoupd_state::port_c_port_w(u8 data)
{
// write  ( repeat)
//logerror("PC Write to %02x\n", data);
}

void yqzhaoupd_state::port_d_port_w(u8 data)
{
// logerror("PD Write to %02x\n", data);
// No write
}
void yqzhaoupd_state::port_f_port_w(u8 data)
{
// No write
//logerror("PF Write to %02x\n", data);
}
void yqzhaoupd_state::multiplex_7seg_w(uint8_t data)
{
	m_selected_7seg_module = data;
}

uint8_t yqzhaoupd_state::keyboard_r()
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

void yqzhaoupd_state::display_7seg_data_w(uint8_t data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // Code was from marywu to decode 7 segment display.

	m_digits[2 * m_selected_7seg_module + 0] = patterns[data & 0x0f];
	m_digits[2 * m_selected_7seg_module + 1] = patterns[data >> 4];
}

void yqzhaoupd_state::program_map(address_map &map)
{
	map(0x0000, 0x8CFF).rom();
	map(0xc000, 0xc001).w("opll", FUNC(ym2413_device::write));
	map(0xc400, 0xc400).rw(m_oki, FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xc800, 0xcbff).ram().share("nvram");
	map(0xcc00, 0xcc03).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xd000, 0xd003).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0xd400, 0xd401).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

void yqzhaoupd_state::machine_start()
{
	save_item(NAME(m_selected_7seg_module));

}

void yqzhaoupd_state::yqzhaoupd(machine_config &config)
{

	/* basic machine hardware */
	upd78c11_device &maincpu(UPD78C11(config, "maincpu", XTAL( 10'694'250)));
	maincpu.set_addrmap(AS_PROGRAM, &yqzhaoupd_state::program_map);
	maincpu.pa_in_cb().set_ioport("IN0");
	maincpu.pb_in_cb().set_ioport("IN1");
	maincpu.pc_in_cb().set_ioport("IN2");
	maincpu.pd_in_cb().set_ioport("IN3");
	maincpu.pf_in_cb().set_ioport("IN4");
  maincpu.pa_out_cb().set(FUNC(yqzhaoupd_state::port_a_port_w));
  maincpu.pb_out_cb().set(FUNC(yqzhaoupd_state::port_b_port_w));
  maincpu.pc_out_cb().set(FUNC(yqzhaoupd_state::port_c_port_w));
  maincpu.pd_out_cb().set(FUNC(yqzhaoupd_state::port_d_port_w));
  maincpu.pf_out_cb().set(FUNC(yqzhaoupd_state::port_f_port_w));
  
  HOPPER(config, m_hopper, attotime::from_msec(100));
	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* Keyboard & display interface */
	i8279_device &kbdc(I8279(config, "i8279", XTAL(10'694'250) / 6));     // divisor not verified
	kbdc.out_sl_callback().set(FUNC(yqzhaoupd_state::multiplex_7seg_w));   // select  block of 7seg modules by multiplexing the SL scan lines
	kbdc.in_rl_callback().set(FUNC(yqzhaoupd_state::keyboard_r));          // keyboard Return Lines
	kbdc.out_disp_callback().set(FUNC(yqzhaoupd_state::display_7seg_data_w));

	/* Programmable Peripheral Interface */
  i8255_device &ppi1(I8255A(config, "ppi1"));
	ppi1.out_pa_callback().set(FUNC(yqzhaoupd_state::ppi1_porta_w));
	ppi1.out_pb_callback().set(FUNC(yqzhaoupd_state::ppi1_portb_w));
	ppi1.out_pc_callback().set(FUNC(yqzhaoupd_state::ppi1_portc_w));

  i8255_device &ppi2(I8255A(config, "ppi2"));
	ppi2.out_pa_callback().set(FUNC(yqzhaoupd_state::ppi2_porta_w));
	ppi2.out_pb_callback().set(FUNC(yqzhaoupd_state::ppi2_portb_w));
	ppi2.out_pc_callback().set(FUNC(yqzhaoupd_state::ppi2_portc_w));

	/* Video */
	config.set_default_layout(layout_yqzhaoupd);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	ym2413_device &opll(YM2413(config, "opll", 3.579545_MHz_XTAL));
	opll.add_route(ALL_OUTPUTS, "mono", 0.50);
	OKIM6295(config, m_oki, 1_MHz_XTAL, okim6295_device::PIN7_LOW).add_route(ALL_OUTPUTS, "mono", 0.50); // verified
}

ROM_START( yqzhaoupd )

	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "78c11cpu.512", 0x0000, 0x10000, CRC(BA5EEE0E) SHA1(ecd7c96ae4339e64bd57fa18601c727938564a13) )

	ROM_REGION( 0x80000, "oki", 0 )
	ROM_LOAD( "shengyan-u20.040", 0x00000, 0x80000, CRC(19134993) SHA1(8e3ecee0035ee32e98a5b8be6a6d17a3ddea4dc1) )
ROM_END

} // anonymous namespace


//    YEAR    NAME         PARENT   MACHINE       INPUT      STATE            INIT        ROT    COMPANY        FULLNAME                               FLAGS
GAME( ????,  yqzhaoupd,     0,       yqzhaoupd,     yqzhaoupd,  yqzhaoupd_state,  empty_init, ROT0,  "Unknown",  "You qian zhen hao",                  MACHINE_IMPERFECT_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL )
