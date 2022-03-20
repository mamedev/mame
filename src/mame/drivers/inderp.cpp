// license:BSD-3-Clause
// copyright-holders:Robbbert
// PINBALL
/***************************************************************************************************************************
Skeleton driver for early Inder pinballs on "Indertronic B-1" hardware.
Hardware listing and ROM definitions from PinMAME.

Inder (early games, "Indertronic B-1")
--------------------------------------

   Hardware:
   ---------
CPU:     M6504, R/C combo with 100kOhms and 10pF for clock, manual says 4 microseconds min. instruction execution
    INT: ? (somewhere in between 200 .. 250 Hz seems likely)
IO:      DMA only
DISPLAY: 6-digit, both 9-segment & 7-segment panels with direct segment access
SOUND:   simple tones, needs comparison with real machine

Status:
- Machines run, but with issues
- If you quit while a game is in progress, it's still there when you start again.

ToDo:
- Sound
- It seems a bit sluggish
- When it says Inder/TOPAZ/High-score, the status display tries to show that too, but
  it is corrupt. Perhaps it should be blank at that time.
- Layout for Centaur - it's a one-player game.
- Dips to identify, they don't seem to be correct.
- Where are the test buttons connected to? They don't work.

 ************************************************************************************************************************/

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6502/m6504.h"
#include "machine/clock.h"
#include "inderp.lh"

namespace {

class inderp_state : public genpin_class
{
public:
	inderp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_io_keyboard(*this, "X%d", 0)
		, m_digits(*this, "digit%d", 0U)
		, m_io_outputs(*this, "out%d", 0U)
		{ }

	void inderp(machine_config &config);
	void init_1player();

private:
	DECLARE_WRITE_LINE_MEMBER(clock_tick);
	u8 m_key_row = 0U;
	u16 m_t_c = 0U;
	u8 m_segment[5]{};
	bool m_1player = false;
	void inputs_w(offs_t offset, u8 data);
	u8 inputs_r();
	void solenoids_w(u8);
	void sound_w(u8);
	void lamps_w(offs_t, u8);
	void display_w(offs_t, u8);
	u16 seg8to14(u16 data);
	void mem_map(address_map &map);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	required_device<m6504_device> m_maincpu;
	required_ioport_array<11> m_io_keyboard;
	output_finder<48> m_digits;
	output_finder<72> m_io_outputs;  // 8 solenoids + 64 lamps
};

void inderp_state::mem_map(address_map &map)
{
	map.global_mask(0x1fff);
	map.unmap_value_high();
	map(0x0000, 0x00ff).mirror(0x100).ram().share("nvram"); // 2x 5101/2101, battery-backed
	map(0x0200, 0x03ff).nopr();  // cpu does an unwanted read when doing a write.
	map(0x0200, 0x02ff).w(FUNC(inderp_state::display_w)); // outputs CI-110 (displays)
	map(0x0300, 0x030f).mirror(0xc0).rw(FUNC(inderp_state::inputs_r), FUNC(inderp_state::inputs_w)); // outputs, one per address CI-118; inputs: whole range CI-159
	map(0x0310, 0x0310).mirror(0xcf).w(FUNC(inderp_state::solenoids_w)); // outputs, D0-D2, BOB0-7 (solenoids) CI-121
	map(0x0320, 0x0320).mirror(0xcf).w(FUNC(inderp_state::sound_w)); // outputs, D0-D3, (sound) CI-122
	map(0x0330, 0x0337).mirror(0xc8).w(FUNC(inderp_state::lamps_w)); // outputs, D0-D7, one per address to 16 sets of 4-bit outputs (lamps)
	map(0x0400, 0x1fff).rom();
}

// dsw: don't know bit order yet
static INPUT_PORTS_START( inderp )
	PORT_START("X0")
	PORT_DIPNAME( 0x07, 0x00, "Coins")
	PORT_DIPSETTING(    0x00, "choice 1")
	PORT_DIPSETTING(    0x01, "choice 2")
	PORT_DIPSETTING(    0x02, "choice 3")
	PORT_DIPSETTING(    0x03, "choice 4")
	PORT_DIPSETTING(    0x04, "choice 5")
	PORT_DIPSETTING(    0x05, "choice 6")
	PORT_DIPSETTING(    0x06, "choice 7")
	PORT_DIPSETTING(    0x07, "choice 8")
	PORT_DIPNAME( 0x08, 0x08, "Balls")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x08, "3")
	PORT_DIPNAME( 0x10, 0x00, "Sound")
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPSETTING(    0x10, DEF_STR(No))
	PORT_DIPNAME( 0x40, 0x40, "Show high score in attract mode")
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPSETTING(    0x40, DEF_STR(No))
	PORT_DIPNAME( 0x80, 0x80, "Free games for beating high score")
	PORT_DIPSETTING(    0x00, "2")
	PORT_DIPSETTING(    0x80, "1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X1")
	PORT_DIPNAME( 0x03, 0x00, "High Score") //"Handicap"
	PORT_DIPSETTING(    0x00, "700,000")
	PORT_DIPSETTING(    0x01, "750,000")
	PORT_DIPSETTING(    0x02, "850,000")
	PORT_DIPSETTING(    0x03, "950,000")
	PORT_DIPNAME( 0x0c, 0x00, "Score for 2nd extra")
	PORT_DIPSETTING(    0x00, "800,000")
	PORT_DIPSETTING(    0x04, "850,000")
	PORT_DIPSETTING(    0x08, "900,000")
	PORT_DIPSETTING(    0x0c, "Off")
	PORT_DIPNAME( 0x30, 0x00, "Score for 1st extra")
	PORT_DIPSETTING(    0x00, "650,000")
	PORT_DIPSETTING(    0x10, "700,000")
	PORT_DIPSETTING(    0x20, "750,000")
	PORT_DIPSETTING(    0x30, "800,000")
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X2")
	PORT_DIPNAME( 0x01, 0x00, "Rotate roulette when ball passes 5 upper aisles")
	PORT_DIPSETTING(    0x00, DEF_STR(Yes))
	PORT_DIPSETTING(    0x01, DEF_STR(No))
	PORT_DIPNAME( 0x0c, 0x00, "Credit limit")
	PORT_DIPSETTING(    0x0c, "5")
	PORT_DIPSETTING(    0x08, "10")
	PORT_DIPSETTING(    0x04, "15")
	PORT_DIPSETTING(    0x00, "20")
	PORT_BIT( 0xf2, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("X3")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X4")
	PORT_BIT( 0xff, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("X5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 ) // "Monedero A"
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_COIN2 ) // "Monedero B"
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN3 ) // "Monedero C"
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 ) // "Pulsador Partidas"
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_9) PORT_NAME("Tilt") // "Falta"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("X6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_A) PORT_NAME("INP60")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_B) PORT_NAME("INP61")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_C) PORT_NAME("INP62")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_D) PORT_NAME("INP63")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_E) PORT_NAME("INP64")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_F) PORT_NAME("INP65")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_G) PORT_NAME("INP66")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_H) PORT_NAME("INP67")

	PORT_START("X7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_I) PORT_NAME("INP70")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_J) PORT_NAME("INP71")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_K) PORT_NAME("INP72")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_L) PORT_NAME("INP73")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_M) PORT_NAME("INP74")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_N) PORT_NAME("INP75")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_O) PORT_NAME("INP76")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_P) PORT_NAME("INP77")

	PORT_START("X8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Q) PORT_NAME("INP80")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_R) PORT_NAME("INP81")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_S) PORT_NAME("INP82")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_T) PORT_NAME("INP83")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_U) PORT_NAME("INP84")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_V) PORT_NAME("INP85")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_W) PORT_NAME("INP86")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Y) PORT_NAME("INP87")

	PORT_START("X9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_Z) PORT_NAME("INP90")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_STOP) PORT_NAME("INP91")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COMMA) PORT_NAME("INP92")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_COLON) PORT_NAME("INP93")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_QUOTE) PORT_NAME("INP94")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_ENTER) PORT_NAME("INP95")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_NAME("INP96")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_NAME("INP97")

	// don't know where these buttons fit in
	PORT_START("X10")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("Reset") // "Puesta a cero"
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("Accounting info") // "Test economico"
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYPAD ) PORT_CODE(KEYCODE_0_PAD) PORT_NAME("Test") // "Test tecnico"
	PORT_BIT( 0x1f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

u8 inderp_state::inputs_r()
{
	if (m_key_row < 11)
		return m_io_keyboard[m_key_row]->read();
	else
		return 0xff;
}

// 0-2 select a dipsw bank; 3-9 select banks of mechanical inputs; A-F not connected
void inderp_state::inputs_w(offs_t offset, u8 data)
{
	m_key_row = offset;
}

void inderp_state::solenoids_w(u8 data)
{
	for (u8 i = 0; i < 8; i++)
	{
		m_io_outputs[i] = (i == data) ? 1 : 0;
		switch (i)
		{
			case 0:
			case 1:
			case 2:
				if (i==data)
					m_samples->start(0, 0);  // bumpers
				break;
			case 3:
			case 4:
				if (i==data)
					m_samples->start(0, 7);  // slings
				break;
			case 7:
				if (i==data)
					m_samples->start(0, 5);  // outhole
				break;
			default:
				break;
		}
	}
}

void inderp_state::sound_w(u8 data)
{
	//printf("%X ",data);
	// Bits 0-2 adjust the frequency of a tone oscillator.
	// Bit 3 "Pulso Musicales"
}

void inderp_state::lamps_w(offs_t offset, u8 data)
{
	for (u8 i = 0; i < 8; i++)
		m_io_outputs[offset*8+8+i] = BIT(data, i);
}

// Note that Centaur uses Player-2 display for the player, and Player-3 display for the status.
void inderp_state::display_w(offs_t offset, u8 data)
{
	u8 status_seg = m_1player ? 2 : 4;
	if (offset < 5)
	{
		m_segment[offset] = (offset == status_seg) ? data & 0x7f : data;
	}
	else
	{
		u8 digit = BIT(offset, 3, 3);
		if (m_1player)
		{
			m_digits[digit] = m_segment[1];
			if ((digit == 4) || (digit == 5))
				m_digits[digit+40] = m_segment[2];
			else
			if ((digit == 2) || (digit == 3))
				m_digits[digit+38] = m_segment[2];
		}
		else
		{
			for (u8 i = 0; i < 4; i++)
				m_digits[digit+i*10] = seg8to14(m_segment[i]);
			m_digits[digit+40] = m_segment[4];
		}
	}
}

u16 inderp_state::seg8to14(u16 data)
{
	// convert custom 8seg digit to MAME 14seg digit
	return bitswap<10>(data,7,7,6,6,5,4,3,2,1,0);
}

WRITE_LINE_MEMBER( inderp_state::clock_tick )
{
	m_t_c++;
	if (m_t_c == 0x173)
		m_t_c = 0x170;
	if (m_t_c == 0x170)
		m_maincpu->set_input_line(M6504_IRQ_LINE, HOLD_LINE);
}

void inderp_state::machine_start()
{
	genpin_class::machine_start();

	m_digits.resolve();
	m_io_outputs.resolve();

	save_item(NAME(m_key_row));
	save_item(NAME(m_t_c));
	save_item(NAME(m_segment));
	save_item(NAME(m_1player));
}

void inderp_state::machine_reset()
{
	genpin_class::machine_reset();
	for (u8 i = 0; i < m_io_outputs.size(); i++)
		m_io_outputs[i] = 0;

	m_t_c = 0;
}

void inderp_state::init_1player()
{
	m_1player = true;
}

void inderp_state::inderp(machine_config &config)
{
	/* basic machine hardware */
	M6504(config, m_maincpu, 2000000); // need to be measured
	m_maincpu->set_addrmap(AS_PROGRAM, &inderp_state::mem_map);

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 200)); // crosspoint detector
	cpoint_clock.signal_handler().set(FUNC(inderp_state::clock_tick));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	/* video hardware */
	config.set_default_layout(layout_inderp);

	/* sound hardware */
	//discrete ?
	genpin_audio(config);
}


ROM_START(centauri)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("cent2.ci104",  0x0800, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3.ci103",  0x1000, 0x0400, CRC(f87abd63) SHA1(c3f48ffd46fad076fd064cbc0fdcc31641f5b1b6))
	ROM_RELOAD(0x1400, 0x0400)
	ROM_LOAD("cent4.ci102",  0x1800, 0x0400, CRC(b69e95b6) SHA1(2f053a5848110d084239e1fc960198b247b3b98e))
	ROM_RELOAD(0x1c00, 0x0400)
ROM_END

ROM_START(centauri2)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("cent2.ci104",  0x0800, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3a.ci103", 0x1000, 0x0400, CRC(7b8215b1) SHA1(7cb6c18ad88060b56785bbde398bff157d8417cd))
	ROM_RELOAD(0x1400, 0x0400)
	ROM_LOAD("cent4a.ci102", 0x1800, 0x0400, CRC(7ee64ea6) SHA1(b751b757faab7e3bb56625e4d72c3aeeb84a3f28))
	ROM_RELOAD(0x1c00, 0x0400)
ROM_END

ROM_START(topaz)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("topaz0.bin", 0x0400, 0x0400, CRC(d047aee0) SHA1(b2bc2e9fb088006fd3b7eb080feaa1eac479af58))
	ROM_RELOAD(0x1400, 0x0400)
	ROM_LOAD("topaz1.bin", 0x0800, 0x0400, CRC(72a423c2) SHA1(e3ba5d581739fc0871901f861a7692fd86e0f6aa))
	ROM_RELOAD(0x1800, 0x0400)
	ROM_LOAD("topaz2.bin", 0x0c00, 0x0400, CRC(b8d2e7c6) SHA1(e19bec04fab15536fea51c4298c6a4cb3817630c))
	ROM_RELOAD(0x1c00, 0x0400)
ROM_END

ROM_START(skatebrd)
	ROM_REGION(0x2000, "maincpu", 0)
	ROM_LOAD("skate2.bin",  0x0800, 0x0800, CRC(ee9b4c4c) SHA1(1a8b2ef8dfead18bfc62e85474dab2838b73ce08))
	ROM_LOAD("skate3.bin",  0x1000, 0x0400, CRC(58e181fe) SHA1(f54c8099100d0c96dc2ddbae8db9293f8581d459))
	ROM_RELOAD(0x1400, 0x0400)
	ROM_LOAD("skate4.bin",  0x1800, 0x0400, CRC(fcdccffe) SHA1(a2db53f7bc555d705aa894e62307590fd74067dd))
	ROM_RELOAD(0x1c00, 0x0400)
ROM_END

} // Anonymous namespace

GAME( 1979, centauri,  0,        inderp, inderp, inderp_state, init_1player, ROT0, "Inder", "Centaur (Inder)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, centauri2, centauri, inderp, inderp, inderp_state, init_1player, ROT0, "Inder", "Centaur (Inder, alternate set)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, topaz,     0,        inderp, inderp, inderp_state, empty_init,   ROT0, "Inder", "Topaz (Inder)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1980, skatebrd,  0,        inderp, inderp, inderp_state, empty_init,   ROT0, "Inder", "Skate Board (Inder)",     MACHINE_IS_SKELETON_MECHANICAL )
