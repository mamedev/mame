// license:BSD-3-Clause
// copyright-holders:Robbbert
// PINBALL
/***************************************************************************************************************
Skeleton driver for MAC S.A. and CICPlay pinballs. ROM definitions taken from PinMAME.

There's a number of undumped PAL-type devices, so had to guess a few things.
Unknown chips: SIVP47, MAC P01, MAC D01, MAC A01 (=MAC A.A4, MAC A.A5).
Schematic has errors and omissions (no input or output matrix, no wiring between boards).
Although the coin slots are not working, you can add credits by changing 0xC011 in the memory view.

Status:
- Some games are playable
-- macgalxy (Outhole is Z and X)
-- macjungl (Outhole is E)
-- glxplay2 (Outhole unknown)
- Other machines can generally accept credits and start, but then cannot score.
- Display has intolerable flicker.

ToDo
- MSM sound
- Inputs not responding
- DIPs to be checked
- Fix flicker on displays
- CIC games need a different layout

***************************************************************************************************************/

#include "emu.h"
#include "genpin.h"
#include "cpu/z80/z80.h"
#include "machine/clock.h"
#include "machine/i8279.h"
#include "sound/ay8910.h"
#include "sound/msm5205.h"
#include "speaker.h"
#include "macp.lh"

namespace {

class macp_state : public genpin_class
{
public:
	macp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ay8910(*this, "ay%d", 0U)
		, m_io_keyboard(*this, "X%d", 0U)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void mac16k(machine_config &config);
	void mac32k(machine_config &config);
	void mem16k_map(address_map &map) ATTR_COLD;
	void io16k_map(address_map &map) ATTR_COLD;
	void mem32k_map(address_map &map) ATTR_COLD;
	void io32k_map(address_map &map) ATTR_COLD;

private:
	void clock_tick(int state);
	void irq_w(int state);
	void scanlines_w(u8);
	void digit_w(u8);
	void ay0_a_w(u8);
	void ay0_b_w(u8);
	void ay1_a_w(u8);
	void ay1_b_w(u8);
	u8 ay1_b_r();
	u8 kbd_r();
	u8 m_t_c = 0U;
	u8 m_digit = 0U;
	u8 m_ay1_b = 0U;
	u8 m_ay1_a = 0U;
	u8 m_relay_ctrl = 0U;
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	required_device<z80_device> m_maincpu;
	required_device_array<ay8910_device, 2> m_ay8910;
	required_ioport_array<8> m_io_keyboard;
	output_finder<40> m_digits;
	output_finder<85> m_io_outputs;  // 12+4+1 solenoids + 48+16 lamps
};

void macp_state::mem16k_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0x7fff);  // A15 only goes to the expansion socket and a PAL
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram().share("nvram");   // 6116
	map(0x6000, 0x6001).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

void macp_state::mem32k_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x7fff).rom();
	map(0xc000, 0xdfff).ram().share("nvram");  // DS1225 (8k RAM with inbuilt battery), only first 2k used.
}

void macp_state::io16k_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x08).w(m_ay8910[0], FUNC(ay8910_device::address_w));
	map(0x0a, 0x0a).w(m_ay8910[0], FUNC(ay8910_device::data_w));
	map(0x18, 0x18).w(m_ay8910[1], FUNC(ay8910_device::address_w));
	map(0x19, 0x19).r(m_ay8910[1], FUNC(ay8910_device::data_r));
	map(0x1a, 0x1a).w(m_ay8910[1], FUNC(ay8910_device::data_w));
}

void macp_state::io32k_map(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x08, 0x08).w(m_ay8910[0], FUNC(ay8910_device::address_w));
	map(0x0a, 0x0a).w(m_ay8910[0], FUNC(ay8910_device::data_w));
	map(0x28, 0x28).w(m_ay8910[1], FUNC(ay8910_device::address_w));
	map(0x29, 0x29).r(m_ay8910[1], FUNC(ay8910_device::data_r));
	map(0x2a, 0x2a).w(m_ay8910[1], FUNC(ay8910_device::data_w));
	map(0x40, 0x41).rw("i8279", FUNC(i8279_device::read), FUNC(i8279_device::write));
}

static INPUT_PORTS_START( macp )
	PORT_START("X0")
	PORT_DIPNAME( 0x01, 0x01, "DIP A1")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "DIP A2")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "DIP A3")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "DIP A4")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "DIP A5")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "DIP A6")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "DIP A7")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))

	PORT_START("X1")
	PORT_DIPNAME( 0x01, 0x01, "DIP B1")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x01, DEF_STR(Off))
	PORT_DIPNAME( 0x02, 0x02, "DIP B2")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x02, DEF_STR(Off))
	PORT_DIPNAME( 0x04, 0x04, "DIP B3")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x04, DEF_STR(Off))
	PORT_DIPNAME( 0x08, 0x08, "DIP B4")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x08, DEF_STR(Off))
	PORT_DIPNAME( 0x10, 0x10, "DIP B5")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x10, DEF_STR(Off))
	PORT_DIPNAME( 0x20, 0x20, "DIP B6")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x20, DEF_STR(Off))
	PORT_DIPNAME( 0x40, 0x40, "DIP B7")
	PORT_DIPSETTING(    0x00, DEF_STR(On))
	PORT_DIPSETTING(    0x40, DEF_STR(Off))

	PORT_START("X2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0) PORT_NAME("Slam Tilt")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("Reset RAM")

	PORT_START("X3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP30")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP31")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP32")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP33")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP34")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP35")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP36")

	PORT_START("X4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP40")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP41")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP42")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP43")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP44")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP45")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP46")

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP50")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP51")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP52")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP53")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP54")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP55")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP56")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP60")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP61")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("INP62")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP63")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP64")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP65")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP66")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_SLASH) PORT_NAME("INP70")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP71")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP72")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP73")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP74")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP75")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_NAME("INP76")
INPUT_PORTS_END

static INPUT_PORTS_START( cicplay )
	PORT_INCLUDE( macp )
INPUT_PORTS_END

void macp_state::scanlines_w(u8 data)
{
	m_digit = data & 15;
}

void macp_state::digit_w(u8 data)
{
	static const uint8_t patterns[16] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7c, 0x07, 0x7f, 0x67, 0, 0, 0, 0, 0, 0 }; // guess, because custom PAL
	m_digits[m_digit] =    patterns[BIT(data, 0, 4)];
	m_digits[m_digit+20] = patterns[BIT(data, 4, 4)];
	// This isn't right obviously, but if score > 999999 and bit 7 of PAL, then illuminate some extra lamps on the backboard
	u8 i,j = BIT(data, 0, 4);
	if (j > 9)
		for (i = 0; i < 8; i++)
			m_io_outputs[69+i] = BIT(data, i);
	j = BIT(data, 4, 4);
	if (j > 9)
		for (i = 0; i < 8; i++)
			m_io_outputs[77+i] = BIT(data, i);
}

u8 macp_state::kbd_r()
{
	return (m_io_keyboard[m_digit & 7]->read() & 0x7f )| (BIT(m_ay1_b, 4) ? 0x80 : 0);
}

void macp_state::ay0_a_w(u8 data)
{
	// d4-7 solenoids
	for (u8 i = 4; i < 8; i++)
		m_io_outputs[4+i] = BIT(data, i);

	// d0-3 lamps
	data &= 15;
	if (data < 6)
		for (u8 i = 0; i < 8; i++)
			m_io_outputs[17+data*8+i] = BIT(m_ay1_a, i);
}

void macp_state::ay0_b_w(u8 data)
{
	// d0-7 solenoids
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[i] = BIT(data, i);
}

void macp_state::ay1_a_w(u8 data)
{
	m_ay1_a = data;
}

void macp_state::ay1_b_w(u8 data)
{
	m_ay1_b = data;
	// d0-3 solenoids: Coin Lock (probably lockout), Coin In (probably coin counter), Ext Play, Ext Ball.
	for (u8 i = 0; i < 4; i++)
		m_io_outputs[12+i] = BIT(data, i);
	// d4 goes to RL7 (kbd_r)
	// d5 another solenoid
	m_io_outputs[16] = BIT(data, 5);
	// Relay control: 0 = not set; 1 = K1 relay enables 12v supply; 2 = K2 relay enables 40v supply; 3= normal operation
	// K2 is pulsed by a watchdog circuit. The K2 relay needs both bits enabled before it will turn on.
	m_relay_ctrl = BIT(data, 5, 2);
}

u8 macp_state::ay1_b_r()
{
	//return 0x7f | ((m_relay_ctrl == 3) ? 0 : 0x80);  // BIT 7 low indicates 40volt supply is available
	return 0x7f | (m_relay_ctrl ? 0 : 0x80);  // Temporary for testing
}

void macp_state::irq_w(int state)
{
	m_maincpu->set_input_line(INPUT_LINE_IRQ0, state ? ASSERT_LINE : CLEAR_LINE);
}

void macp_state::clock_tick(int state)
{
	if (state)
	{
		if (m_t_c > 0xf0)
			m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
		else
			m_t_c++;
	}
}

void macp_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_digit));
	save_item(NAME(m_t_c));
	save_item(NAME(m_ay1_a));
	save_item(NAME(m_ay1_b));
}

void macp_state::machine_reset()
{
	genpin_class::machine_reset();

	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;
	m_t_c = 0;
	m_relay_ctrl = 0;
}

void macp_state::mac16k(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, 3'800'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &macp_state::mem16k_map);
	m_maincpu->set_addrmap(AS_IO, &macp_state::io16k_map);

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 50)); // crosspoint detector
	cpoint_clock.signal_handler().set(FUNC(macp_state::clock_tick));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	config.set_default_layout(layout_macp);

	//I8279
	i8279_device &kbdc(I8279(config, "i8279", 3'800'000)); // based on divider
	kbdc.out_sl_callback().set(FUNC(macp_state::scanlines_w));    // scan SL lines
	kbdc.out_disp_callback().set(FUNC(macp_state::digit_w));      // display A&B
	kbdc.in_rl_callback().set(FUNC(macp_state::kbd_r));           // kbd RL lines
	kbdc.out_irq_callback().set(FUNC(macp_state::irq_w));

	/* sound hardware */
	genpin_audio(config);

	//2x AY8910
	SPEAKER(config, "lspkr").front_left();
	AY8910(config, m_ay8910[0], 3'800'000/2);
	m_ay8910[0]->set_resistors_load(10e3,10e3,10e3);
	m_ay8910[0]->port_a_write_callback().set(FUNC(macp_state::ay0_a_w));
	m_ay8910[0]->port_b_write_callback().set(FUNC(macp_state::ay0_b_w));
	m_ay8910[0]->add_route(ALL_OUTPUTS, "lspkr", 0.50);

	SPEAKER(config, "rspkr").front_right();
	AY8910(config, m_ay8910[1], 3'800'000/2);
	m_ay8910[1]->set_resistors_load(10e3,10e3,10e3);
	m_ay8910[1]->port_a_write_callback().set(FUNC(macp_state::ay1_a_w));
	m_ay8910[1]->port_b_write_callback().set(FUNC(macp_state::ay1_b_w));
	m_ay8910[1]->port_b_read_callback().set(FUNC(macp_state::ay1_b_r));
	m_ay8910[1]->add_route(ALL_OUTPUTS, "rspkr", 0.50);
}

void macp_state::mac32k(machine_config &config)
{
	mac16k(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &macp_state::mem32k_map);
	m_maincpu->set_addrmap(AS_IO, &macp_state::io32k_map);
	// MSM5205
}

ROM_START(macgalxy)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("galaxy1.bin", 0x0000, 0x2000, CRC(00c71e67) SHA1(c1ad1dacae2b90f516c732bfdf8244908f67e15a))
	ROM_LOAD("galaxy2.bin", 0x2000, 0x2000, CRC(f0efb723) SHA1(697b3c9f3ebedca1087354eda5dfe9719d497045))
ROM_END

ROM_START(macgalxyb)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("system_iv_fabricante_m-73_macsa_galaxy_m5_6.bin",    0x0000, 0x2000, CRC(38319685) SHA1(54e979bba16d79d3589b3ae3f9e8e29071ef5b36))
	ROM_LOAD("modelo_a-01202_fabricante_m-73_macsa_galaxy_05.bin", 0x2000, 0x2000, CRC(c1f40cbf) SHA1(566d962f55367379346155f2078b92b4e1e6a79d))
ROM_END

ROM_START(macjungl)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("jungle1.bin", 0x0000, 0x2000, CRC(461a3e1b) SHA1(96981b4d8db0412c474169eaf5e5386be5006ffe))
	ROM_LOAD("jungle2.bin", 0x2000, 0x2000, CRC(26b53e6e) SHA1(e588787b2381c0e6a42590f0e7d18d2a74ebf5f0))
ROM_END

ROM_START(spctrain)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mbm27128.25", 0x0000, 0x4000, CRC(d65c5c36) SHA1(6f350b48daaecd36b3086e682ec6ee174f297a34))
ROM_END

ROM_START(spctraino)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1717-c2.bin", 0x0000, 0x2000, CRC(ca8be787) SHA1(4091921013429c6104da698625391c575e30b8e1))
	ROM_LOAD("1717-c8.bin", 0x2000, 0x2000, CRC(c7f499f5) SHA1(6564cab0c70fb66a95b24c05b427239b4b886f1e))
ROM_END

ROM_START(spcpnthr)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("sp_game.bin", 0x0000, 0x8000, CRC(0428563c) SHA1(45b9daf12f8384101450f1e529491812f73d88bd))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(mac_1808)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("mac_1808.cpu", 0x0000, 0x8000, CRC(29126585) SHA1(b24c4f0f17f3ef7de5348cb06ec3b305c6ca7373))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(macjungn)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("juego1.bin", 0x0000, 0x8000, CRC(0b2d9b64) SHA1(22602b79c8b178793b447783bca59dcb49e4525f))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(nbamac)
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("nba_mac.cpu", 0x0000, 0x8000, CRC(0c430988) SHA1(71126d9caf10ac27056b8bf28d300775062dc693))

	ROM_REGION(0x8000, "msm5205", 0)
	ROM_LOAD("mac_snd.bin", 0x0000, 0x8000, CRC(d7aedbac) SHA1(4b59028e08b2d7ff8f19596022ba6e830cf736e2))
ROM_END

ROM_START(glxplay)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1083-1.cpu", 0x0000, 0x2000, CRC(3df33169) SHA1(657720aab4cccf3364f013acb3f5dbc46fe0e05c))
	ROM_LOAD("1083-2.cpu", 0x2000, 0x2000, CRC(47b4f49e) SHA1(59853ac56bb9e2dc7b848dc46ebd27c21b9d2e82))
ROM_END

ROM_START(kidnap)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("kidnap_1.bin", 0x0000, 0x2000, CRC(4b8f9bb1) SHA1(16672c1a5e55ba5963fbd8834443dbead9bdff10) BAD_DUMP)
	ROM_LOAD("kidnap_2.bin", 0x2000, 0x2000, CRC(4333d9ba) SHA1(362bcc9caaf37ad7efc116c3bee9b99cbbfa0563))
ROM_END

ROM_START(glxplay2)
	ROM_REGION(0x4000, "maincpu", 0)
	ROM_LOAD("1382-1.cpu", 0x0000, 0x2000, CRC(da43b0b9) SHA1(b13b260c61b3bd0b7632aabcdbcf4cdd5cbe4b22))
	ROM_LOAD("1382-2.cpu", 0x2000, 0x2000, CRC(945c90fd) SHA1(8367992f8db8b402d82e4a3f02a35b796756ce0f))
ROM_END

} // Anonymous namespace

// MAC S.A. pinballs
GAME( 1986, macgalxy,         0, mac16k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC's Galaxy (yellow version, M.6)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, macgalxyb, macgalxy, mac16k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC's Galaxy (blue version, M.042)",  MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, macjungl,         0, mac16k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC Jungle",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, spctrain,         0, mac32k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "Space Train (Pinball)",               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, spctraino, spctrain, mac16k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "Space Train (Pinball, old hardware)", MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1988, spcpnthr,         0, mac32k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "Space Panther",                       MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 19??, mac_1808,         0, mac32k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "unknown game (MAC #1808)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1995, macjungn,         0, mac32k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "MAC Jungle (New version)",            MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1996, nbamac,           0, mac32k,  macp,    macp_state, empty_init, ROT0, "MAC S.A.", "NBA MAC",                             MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )

// CICPlay pinballs
GAME( 1985, glxplay,          0, mac16k,  cicplay, macp_state, empty_init, ROT0, "CICPlay", "Galaxy Play",                          MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1986, kidnap,           0, mac16k,  cicplay, macp_state, empty_init, ROT0, "CICPlay", "Kidnap",                               MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
GAME( 1987, glxplay2,         0, mac16k,  cicplay, macp_state, empty_init, ROT0, "CICPlay", "Galaxy Play 2",                        MACHINE_IS_SKELETON_MECHANICAL | MACHINE_SUPPORTS_SAVE )
