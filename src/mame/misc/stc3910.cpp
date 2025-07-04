// license:BSD-3-Clause
// copyright-holders:Nigel Barnes
/**********************************************************************

    STC 3910 Executel

    TODO:
    - requires tape to be fully usable.
    - sound, beeps at startup.

**********************************************************************/

#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/i8085/i8085.h"
#include "cpu/mcs48/mcs48.h"
#include "machine/clock.h"
#include "machine/mdcr.h"
#include "machine/mm58174.h"
#include "machine/saa5070.h"
#include "video/mr9735.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class executel_state : public driver_device
{
public:
	executel_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rombank(*this, "rombank")
		, m_keyb(*this, "keyb")
		, m_tape(*this, "tape")
		, m_videoram(*this, "videoram")
		, m_kbd(*this, "KEY%u", 0)
		, m_mdcr(*this, "mdcr")
		, m_hfree_led(*this, "hfree_led")
		, m_vdata_led(*this, "vdata_led")
	{ }

	void executel(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_memory_bank m_rombank;
	required_device<i8741a_device> m_keyb;
	required_device<i8741a_device> m_tape;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport_array<16> m_kbd;
	required_device<mdcr_device> m_mdcr;
	output_finder<> m_hfree_led;
	output_finder<> m_vdata_led;

	void mem_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;

	uint8_t kbd_p1_r();
	uint8_t kbd_p2_r();
	void kbd_p2_w(uint8_t data);

	uint8_t tape_p1_r();
	uint8_t tape_p2_r();
	void tape_p2_w(uint8_t data);

	void bank_w(uint8_t data);

	uint8_t m_keycol;
};


void executel_state::mem_map(address_map &map)
{
	map(0x0000, 0x3fff).rom().region("maincpu", 0);
	map(0x4000, 0x7fff).bankr("rombank");
	map(0x8000, 0xfbff).ram();
	map(0xfc00, 0xffff).ram().share("videoram");
}

void executel_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	//map(0x30, 0x30) // printer
	map(0x70, 0x71).rw(m_keyb, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xb0, 0xbf).rw("rtc", FUNC(mm58174_device::read), FUNC(mm58174_device::write));
	map(0xc0, 0xc0).w(FUNC(executel_state::bank_w));
	map(0xd0, 0xd1).rw(m_tape, FUNC(upi41_cpu_device::upi41_master_r), FUNC(upi41_cpu_device::upi41_master_w));
	map(0xe0, 0xef).rw("viop", FUNC(saa5070_device::read_direct), FUNC(saa5070_device::write_direct));
}


static INPUT_PORTS_START( executel )
	PORT_START("KEY0")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F7)                            PORT_CHAR(UCHAR_MAMEKEY(F7))         PORT_NAME("HANDS FREE")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F11)                           PORT_CHAR(UCHAR_MAMEKEY(F11))        PORT_NAME("CALL")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F10)                           PORT_CHAR(UCHAR_MAMEKEY(F10))        PORT_NAME("EXCH")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LSHIFT)                        PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY1")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F8)                            PORT_CHAR(UCHAR_MAMEKEY(F8))         PORT_NAME("REDIAL")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_1) PORT_CODE(KEYCODE_1_PAD)    PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_4) PORT_CODE(KEYCODE_4_PAD)    PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_7) PORT_CODE(KEYCODE_7_PAD)    PORT_CHAR('7') PORT_CHAR('\'')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_QUOTE)                         PORT_CHAR('*')

	PORT_START("KEY2")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F6)                            PORT_CHAR(UCHAR_MAMEKEY(F6))         PORT_NAME("VIEW DATA")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_2) PORT_CODE(KEYCODE_2_PAD)    PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_5) PORT_CODE(KEYCODE_5_PAD)    PORT_CHAR('5')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_8) PORT_CODE(KEYCODE_8_PAD)    PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_0) PORT_CODE(KEYCODE_0_PAD)    PORT_CHAR('0')

	PORT_START("KEY3")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F9)                            PORT_CHAR(UCHAR_MAMEKEY(F9))         PORT_NAME("AUTO DIAL")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_3) PORT_CODE(KEYCODE_3_PAD)    PORT_CHAR('3') PORT_CHAR(0xa3) // £
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_6) PORT_CODE(KEYCODE_6_PAD)    PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_9) PORT_CODE(KEYCODE_9_PAD)    PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSLASH)                     PORT_CHAR('#')

	PORT_START("KEY4")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F5)                            PORT_CHAR(UCHAR_MAMEKEY(F5))         PORT_NAME("CALC")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH_PAD)                     PORT_CHAR(UCHAR_MAMEKEY(SLASH_PAD))  PORT_NAME(u8"÷")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ASTERISK)                      PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))   PORT_NAME(u8"× AM")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_PLUS_PAD)                      PORT_CHAR('+')                       PORT_NAME("+")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS_PAD)                     PORT_CHAR('-')                       PORT_NAME("- PM")

	PORT_START("KEY5")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F1)                            PORT_CHAR(UCHAR_MAMEKEY(F1))         PORT_NAME("TIMER")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Q)                             PORT_CHAR('q') PORT_CHAR('Q')        PORT_NAME("Q JAN")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS)                        PORT_CHAR('=')
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_STOP)                          PORT_CHAR('.')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_BACKSPACE)                     PORT_CHAR(8)                         PORT_NAME("DELETE")

	PORT_START("KEY6")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_HOME)                          PORT_CHAR(UCHAR_MAMEKEY(HOME))       PORT_NAME("NEW")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_W)                             PORT_CHAR('w') PORT_CHAR('W')        PORT_NAME("W FEB")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_A)                             PORT_CHAR('a') PORT_CHAR('A')        PORT_NAME("A")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)                                                  PORT_CHAR('%')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_ENTER)                                                              PORT_NAME("CHANGE")

	PORT_START("KEY7")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F2)                            PORT_CHAR(UCHAR_MAMEKEY(F2))          PORT_NAME("REPTRY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_E)                             PORT_CHAR('e') PORT_CHAR('E')         PORT_NAME("E MAR")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_S)                             PORT_CHAR('s') PORT_CHAR('S')         PORT_NAME("S")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Z)                             PORT_CHAR('z') PORT_CHAR('Z')         PORT_NAME("Z")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SLASH)                         PORT_CHAR('?')

	PORT_START("KEY8")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_OPENBRACE)                     PORT_CHAR(UCHAR_MAMEKEY(OPENBRACE))  PORT_NAME("MONTH")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_R)                             PORT_CHAR('r') PORT_CHAR('R')        PORT_NAME("R APR")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_D)                             PORT_CHAR('d') PORT_CHAR('D')        PORT_NAME("D")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_X)                             PORT_CHAR('x') PORT_CHAR('X')        PORT_NAME("X")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_SPACE)                         PORT_CHAR(' ')                       PORT_NAME("SPACE")

	PORT_START("KEY9")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F4)                            PORT_CHAR(UCHAR_MAMEKEY(F4))         PORT_NAME("NOTES")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_T)                             PORT_CHAR('t') PORT_CHAR('T')        PORT_NAME("T MAY")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F)                             PORT_CHAR('f') PORT_CHAR('F')        PORT_NAME("F JUL")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_C)                             PORT_CHAR('c') PORT_CHAR('C')        PORT_NAME("C")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)

	PORT_START("KEY10")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_F3)                            PORT_CHAR(UCHAR_MAMEKEY(F3))         PORT_NAME("NAME")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_Y)                             PORT_CHAR('y') PORT_CHAR('Y')        PORT_NAME("Y JUN")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_G)                             PORT_CHAR('g') PORT_CHAR('G')        PORT_NAME("G AUG")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_V)                             PORT_CHAR('v') PORT_CHAR('V')        PORT_NAME("V")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED) // >

	PORT_START("KEY11")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DOWN)                          PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_U)                             PORT_CHAR('u') PORT_CHAR('U')        PORT_NAME("U")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_H)                             PORT_CHAR('h') PORT_CHAR('H')        PORT_NAME("H SEP")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_B)                             PORT_CHAR('b') PORT_CHAR('B')        PORT_NAME("B")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_LEFT)                          PORT_CHAR(UCHAR_MAMEKEY(LEFT))

	PORT_START("KEY12")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_CLOSEBRACE)                    PORT_CHAR(UCHAR_MAMEKEY(CLOSEBRACE)) PORT_NAME("DAY")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_I)                             PORT_CHAR('i') PORT_CHAR('I')        PORT_NAME("I")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_J)                             PORT_CHAR('j') PORT_CHAR('J')        PORT_NAME("J OCT")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_N)                             PORT_CHAR('n') PORT_CHAR('N')        PORT_NAME("N")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_UP)                            PORT_CHAR(UCHAR_MAMEKEY(UP))

	PORT_START("KEY13")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RIGHT)                         PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_O)                             PORT_CHAR('o') PORT_CHAR('O')        PORT_NAME("O")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_K)                             PORT_CHAR('k') PORT_CHAR('K')        PORT_NAME("K NOV")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_M)                             PORT_CHAR('m') PORT_CHAR('M')        PORT_NAME("M")
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_DEL)                                                                PORT_NAME("TODAY")

	PORT_START("KEY14")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_END)                           PORT_CHAR(UCHAR_MAMEKEY(END))        PORT_NAME("FINISH")
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_P)                             PORT_CHAR('p') PORT_CHAR('P')        PORT_NAME("P")
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_L)                             PORT_CHAR('l') PORT_CHAR('L')        PORT_NAME("L DEC")
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_COMMA)                         PORT_CHAR(',')
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_KEYBOARD) PORT_CODE(KEYCODE_RCONTROL)                      PORT_CHAR(UCHAR_MAMEKEY(RCONTROL))   PORT_NAME("COPY") // =

	PORT_START("KEY15")
	PORT_BIT(0x01, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x02, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x04, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x08, IP_ACTIVE_LOW, IPT_UNUSED)
	PORT_BIT(0x10, IP_ACTIVE_LOW, IPT_UNUSED)
INPUT_PORTS_END


void executel_state::machine_start()
{
	m_hfree_led.resolve();
	m_vdata_led.resolve();

	m_rombank->configure_entries(0, 3, memregion("maincpu")->base() + 0x4000, 0x4000);
}

void executel_state::machine_reset()
{
	m_rombank->set_entry(0);

	m_keycol = 0;
}


uint8_t executel_state::kbd_p1_r()
{
	return m_kbd[m_keycol]->read();
}

uint8_t executel_state::kbd_p2_r()
{
	return 0xff;
}

void executel_state::kbd_p2_w(uint8_t data)
{
	m_keycol = data & 0x0f;

	//if (m_keycol == 0x0f) set CTS

	m_hfree_led = BIT(data, 4);
	m_vdata_led = BIT(data, 5);
}


uint8_t executel_state::tape_p1_r()
{
	uint8_t data = 0x00;

	data |=  m_mdcr->bet() << 0;
	data |= !m_mdcr->cip() << 1;
	data |= !m_mdcr->wen() << 2;

	return data;
}

uint8_t executel_state::tape_p2_r()
{
	return 0xff;
}

void executel_state::tape_p2_w(uint8_t data)
{
	m_mdcr->wda(!BIT(data, 2));
	m_mdcr->wdc(!BIT(data, 3));

	m_maincpu->set_input_line(I8085_RST55_LINE, BIT(data, 4, 2) ? 0 : 1);

	m_mdcr->rev(!BIT(data, 6));
	m_mdcr->fwd(!BIT(data, 7));
}


void executel_state::bank_w(uint8_t data)
{
	//logerror("bank_w: %d\n", data);
	m_rombank->set_entry(data & 3);
}


void executel_state::executel(machine_config &config)
{
	I8085A(config, m_maincpu, 6_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &executel_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &executel_state::io_map);
	//m_maincpu->in_sid_func().set(FUNC(executel_state::sid_r));
	//m_maincpu->out_sod_func().set(FUNC(executel_state::sod_w));

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(6_MHz_XTAL, 384, 0, 240, 313, 0, 240);
	screen.set_screen_update("vdg", FUNC(mr9735_002_device::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, I8085_RST65_LINE);

	PALETTE(config, "palette", palette_device::RGB_3BIT);

	mr9735_002_device &vdg(MR9735_002(config, "vdg", 6_MHz_XTAL));
	vdg.data_cb().set([this](offs_t offset) { return ~m_videoram[offset & 0x3ff]; });
	vdg.set_interlace(false);

	CLOCK(config, "1200hz", 1200).signal_handler().set_inputline("maincpu", I8085_RST75_LINE); // 1.2kHz output from MR9735

	saa5070_device &viop(SAA5070(config, "viop", 6_MHz_XTAL / 6));
	//viop.readpa_handler().set([this]() { logerror("viop_pa_r: 00\n"); return 0x00; });
	//viop.writepa_handler().set([this](uint8_t data) { logerror("viop_pa_w: %02x\n", data); });
	//viop.readpb_handler().set([this]() { logerror("viop_pb_r: 00\n"); return 0x00; });
	//viop.writepb_handler().set([this](uint8_t data) { logerror("viop_pb_w: %02x\n", data); });
	viop.txdata_handler().set("modem", FUNC(rs232_port_device::write_txd));

	rs232_port_device &rs232(RS232_PORT(config, "modem", default_rs232_devices, nullptr)); // SAA5070 (internal modem)
	rs232.rxd_handler().set("viop", FUNC(saa5070_device::write_rxdata));
	rs232.dcd_handler().set("viop", FUNC(saa5070_device::write_cardet));

	MM58174(config, "rtc", 32.768_kHz_XTAL);

	I8741A(config, m_keyb, 3.579545_MHz_XTAL);
	m_keyb->p1_in_cb().set(FUNC(executel_state::kbd_p1_r));
	m_keyb->p2_in_cb().set(FUNC(executel_state::kbd_p2_r));
	m_keyb->p2_out_cb().set(FUNC(executel_state::kbd_p2_w));
	m_keyb->t0_in_cb().set_constant(1);
	m_keyb->t1_in_cb().set_constant(1);

	I8741A(config, m_tape, 3.579545_MHz_XTAL);
	m_tape->p1_in_cb().set(FUNC(executel_state::tape_p1_r));
	m_tape->p2_in_cb().set(FUNC(executel_state::tape_p2_r));
	m_tape->p2_out_cb().set(FUNC(executel_state::tape_p2_w));
	m_tape->t0_in_cb().set(m_mdcr, FUNC(mdcr_device::rda)).invert();
	m_tape->t1_in_cb().set(m_mdcr, FUNC(mdcr_device::rdc)).invert();

	SPEAKER(config, "mono").front_center();

	MDCR(config, m_mdcr);
}


ROM_START(executel)
	ROM_REGION(0x10000, "maincpu", ROMREGION_ERASE00)

	ROM_SYSTEM_BIOS(0, "v111", "V1.11")
	ROMX_LOAD("711_30624_0633_01_12d.bin", 0x0000, 0x2000, CRC(d9a6eabe) SHA1(e3231192665982be7487ecd71049a70fe6e53789), ROM_BIOS(0))
	ROMX_LOAD("711_30625_0533_01_13d.bin", 0x2000, 0x2000, CRC(3865774f) SHA1(6ad4ebcc0677f05563213d96a48674ec61a9ed9f), ROM_BIOS(0))
	ROMX_LOAD("711_30626_0833_01_14d.bin", 0x4000, 0x2000, CRC(6fb2d323) SHA1(2e995c268b834f87803e23c3cb4f87fd834158b2), ROM_BIOS(0))
	ROMX_LOAD("711_30627_0633_01_15d.bin", 0x6000, 0x2000, CRC(20aeac5f) SHA1(0ff9615a1ea7f51dce61f307d7179ae134e904e0), ROM_BIOS(0))

	ROM_SYSTEM_BIOS(1, "v19", "V1.9") // unreleased US version 'Buckingham'
	ROMX_LOAD("v1-9_us_1.bin", 0x0000, 0x2000, CRC(8b2a568b) SHA1(5c5396864f5effd4bff3ad1d2168da93604b9a17), ROM_BIOS(1))
	ROMX_LOAD("v1-9_us_2.bin", 0x2000, 0x2000, CRC(347fa3ea) SHA1(1a01c026070bb70837e2b51f32f761d25d3dcb43), ROM_BIOS(1))
	ROMX_LOAD("v1-9_us_3.bin", 0x4000, 0x2000, CRC(445a7954) SHA1(f4d06a3d43425728cb6ead6b1ce581b171f43476), ROM_BIOS(1))
	ROMX_LOAD("v1-9_us_4.bin", 0x6000, 0x2000, CRC(a3cbb691) SHA1(65399c979bc94b7666fc6f4a81cc640532f7bfb9), ROM_BIOS(1))

	ROM_SYSTEM_BIOS(2, "v21", "V2.1")
	ROMX_LOAD("711_31191_0033_12d.bin", 0x0000, 0x4000, CRC(5cdcf42f) SHA1(442348578b965b9c5d63348a0c9088b2de69c9d7), ROM_BIOS(2))
	ROMX_LOAD("711_31192_0033_13d.bin", 0x4000, 0x4000, CRC(d40ce85d) SHA1(9d04318ec69a853c5c316e2a151beecae96df817), ROM_BIOS(2))
	ROMX_LOAD("711_31193_0033_14d.bin", 0x8000, 0x4000, CRC(338f17db) SHA1(3019ec6ea94f379d375490c0387bbacc62878832), ROM_BIOS(2))

	ROM_SYSTEM_BIOS(3, "v41", "V4.1")
	ROMX_LOAD("executel_6-4-83_v4-1_1.bin", 0x0000, 0x2000, CRC(bb3290e4) SHA1(75276a1953ba1443cfc5dd6ee461300c337b87e6), ROM_BIOS(3))
	ROMX_LOAD("executel_6-4-83_v4-1_2.bin", 0x2000, 0x2000, CRC(72763244) SHA1(2946ea80923c8d91a6d3f99e89c7b50781d7c327), ROM_BIOS(3))
	ROMX_LOAD("executel_6-4-83_v4-1_3.bin", 0x4000, 0x2000, CRC(351f1288) SHA1(c3d71a5f2667a577e6b00e90d5fb0849097d4451), ROM_BIOS(3))
	ROMX_LOAD("executel_6-4-83_v4-1_4.bin", 0x6000, 0x2000, CRC(2e054911) SHA1(d6e7f30a5110c7dc23b1ef0d24d67cc8d14e8d72), ROM_BIOS(3))

	ROM_SYSTEM_BIOS(4, "v6", "V6")
	ROMX_LOAD("12d_v6.bin", 0x0000, 0x2000, CRC(a8fd2a66) SHA1(146432d45e6c33e134887ebfd127e124d807fc32), ROM_BIOS(4))
	ROM_IGNORE(0x2000)
	ROMX_LOAD("13d_v6.bin", 0x2000, 0x2000, CRC(bb0d431e) SHA1(9e6878c2f8ba6b2363e6d69686026affb513397d), ROM_BIOS(4))
	ROM_IGNORE(0x2000)
	ROMX_LOAD("14d_v6.bin", 0x4000, 0x2000, CRC(e78d766b) SHA1(8eb192cd70dcf9c62743054e5466c92a4ca60f26), ROM_BIOS(4))
	ROM_IGNORE(0x2000)
	ROMX_LOAD("15d_v6.bin", 0x6000, 0x2000, CRC(bb0d431e) SHA1(9e6878c2f8ba6b2363e6d69686026affb513397d), ROM_BIOS(4))
	ROM_IGNORE(0x2000)

	ROM_REGION(0x400, "keyb", 0)
	ROM_LOAD("d8741a_kbd.bin", 0x0000, 0x0400, CRC(1cb00127) SHA1(28e61ea9a924e0caf4f3a107c5a3667adbbbfcbc))

	ROM_REGION(0x400, "tape", 0)
	ROM_LOAD("d8741a_tape.bin", 0x0000, 0x0400, CRC(06e350e9) SHA1(b5e84f8cab8daefe0bbcb2a1c6da6de8d29525a5))

	ROM_REGION(0x100, "prom", 0) // used for ROM enable
	ROM_LOAD("dm74s287n.bin", 0x0000, 0x0100, CRC(b7086783) SHA1(452c7b1d20bf00276b2f876ef09eb28bc4a34c8c))
ROM_END

} // anonymous namespace


//    YEAR  NAME       PARENT  COMPAT  MACHINE     INPUT      CLASS           INIT        COMPANY                         FULLNAME              FLAGS
COMP( 1984, executel,  0,      0,      executel,   executel,  executel_state, empty_init, "STC Telecommunications Ltd.",  "STC 3910 Executel",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
