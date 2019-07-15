// license:BSD-3-Clause
// copyright-holders:Robbbert
// PINBALL
// Skeleton driver for early Inder pinballs on "Indertronic B-1" hardware.
// Known pinballs to be dumped: Skateboard (1980)
// Hardware listing and ROM definitions from PinMAME.

/*
 Inder (early games, "Indertronic B-1")
 --------------------------------------

   Hardware:
   ---------
CPU:     M6502, R/C combo with 100kOhms and 10pF for clock, manual says 4 microseconds min. instruction execution
    INT: ? (somewhere in between 200 .. 250 Hz seems likely)
IO:      DMA only
DISPLAY: 6-digit, both 9-segment & 7-segment panels with direct segment access
SOUND:   simple tones, needs comparison with real machine
 */

#include "emu.h"
#include "machine/genpin.h"
#include "cpu/m6502/m6504.h"
#include "machine/clock.h"

class inderp_state : public genpin_class
{
public:
	inderp_state(const machine_config &mconfig, device_type type, const char *tag)
		: genpin_class(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_inputs(*this, "SW.%u", 0)
		{ }

	void inderp(machine_config &config);

private:
	DECLARE_WRITE_LINE_MEMBER(clock_tick);
	DECLARE_WRITE8_MEMBER(inputs_w);
	DECLARE_READ8_MEMBER(inputs_r);

	void maincpu_map(address_map &map);

	required_device<cpu_device> m_maincpu;
	required_ioport_array<10> m_inputs;
	u8 m_inbank;
	u8 m_irqcnt;
};

void inderp_state::maincpu_map(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x00ff).mirror(0x500).ram(); // 2x 5101/2101, battery-backed
	map(0x0200, 0x02ff).mirror(0x400); // outputs CI-110 (displays)
	map(0x0300, 0x030f).mirror(0x4c0).rw(FUNC(inderp_state::inputs_r), FUNC(inderp_state::inputs_w)); // outputs, one per address CI-118; inputs: whole range CI-159
	map(0x0310, 0x0310).mirror(0x4cf); // outputs, D0-D2, BOB0-7 (solenoids) CI-121
	map(0x0320, 0x0320).mirror(0x4cf); // outputs, D0-D3, (sound) CI-122
	map(0x0330, 0x0337).mirror(0x4c8); // outputs, D0-D7, one per address to 16 sets of 4-bit outputs (lamps)
	map(0x0800, 0x1fff).rom().region("roms", 0);
}

// dsw: don't know bit order yet
static INPUT_PORTS_START( inderp )
	PORT_START("SW.0")
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
	PORT_DIPSETTING(    0x08, "5")
	PORT_DIPSETTING(    0x00, "3")
	PORT_DIPNAME( 0x10, 0x10, "Sound")
	PORT_DIPSETTING(    0x10, DEF_STR(Yes))
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPNAME( 0x40, 0x40, "Show high score in attract mode")
	PORT_DIPSETTING(    0x40, DEF_STR(Yes))
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPNAME( 0x80, 0x80, "Free games for beating high score")
	PORT_DIPSETTING(    0x80, "2")
	PORT_DIPSETTING(    0x00, "1")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW.1")
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

	PORT_START("SW.2")
	PORT_DIPNAME( 0x01, 0x00, "Rotate roulette when ball passes 5 upper aisles")
	PORT_DIPSETTING(    0x01, DEF_STR(Yes))
	PORT_DIPSETTING(    0x00, DEF_STR(No))
	PORT_DIPNAME( 0x0c, 0x00, "Credit limit")
	PORT_DIPSETTING(    0x00, "5")
	PORT_DIPSETTING(    0x04, "10")
	PORT_DIPSETTING(    0x08, "15")
	PORT_DIPSETTING(    0x0c, "20")
	PORT_BIT( 0xf2, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW.3")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_COIN1 ) // "Monedero A"
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_COIN2 ) // "Monedero B"
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_COIN3 ) // "Monedero C"
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_START1 ) // "Pulsador Partidas"
	PORT_BIT( 0x30, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_TILT ) // "Falta"
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("Outhole")

	PORT_START("SW.4")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Q)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_W)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_E)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_R)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_U)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_I)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_O)

	PORT_START("SW.5")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_A)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_D)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_F)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_G)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_H)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_J)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_K)

	PORT_START("SW.6")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_Z)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_C)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_V)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_B)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_N)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_M)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COMMA)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_STOP)

	PORT_START("SW.7")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_L)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_QUOTE)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_EQUALS)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSPACE)
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_OPENBRACE)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_CLOSEBRACE)
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_BACKSLASH)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_OTHER ) PORT_CODE(KEYCODE_COLON)

	// don't know where these buttons fit in
	PORT_START("SW.8")
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SERVICE3 ) PORT_NAME("Reset") // "Puesta a cero"
	PORT_BIT( 0x40, IP_ACTIVE_HIGH, IPT_SERVICE2 ) PORT_NAME("Accounting info") // "Test economico"
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_SERVICE1 ) PORT_NAME("Test") // "Test tecnico"
	PORT_BIT( 0x1f, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("SW.9")
	PORT_BIT( 0xff, IP_ACTIVE_HIGH, IPT_UNUSED )
INPUT_PORTS_END

READ8_MEMBER( inderp_state::inputs_r )
{
	if (m_inbank < 10)
		return m_inputs[m_inbank]->read();
	else
		return 0;
}

// 0-2 select a dipsw bank; 3-9 select banks of mechanical inputs; A-F not connected
WRITE8_MEMBER( inderp_state::inputs_w )
{
	m_inbank = offset;
}

WRITE_LINE_MEMBER( inderp_state::clock_tick )
{
	m_irqcnt++;
	if (m_irqcnt & 3)
		m_maincpu->set_input_line(M6504_IRQ_LINE, CLEAR_LINE);
	else
		m_maincpu->set_input_line(M6504_IRQ_LINE, ASSERT_LINE);
}

void inderp_state::inderp(machine_config &config)
{
	/* basic machine hardware */
	M6504(config, m_maincpu, 434000); // possible calculation of frequency-derived time constant 100k res and 10pf cap
	m_maincpu->set_addrmap(AS_PROGRAM, &inderp_state::maincpu_map);

	clock_device &cpoint_clock(CLOCK(config, "cpoint_clock", 200)); // crosspoint detector
	cpoint_clock.signal_handler().set(FUNC(inderp_state::clock_tick));

	/* video hardware */
	//config.set_default_layout()

	/* sound hardware */
	//discrete ?
	genpin_audio(config);
}


ROM_START(centauri)
	ROM_REGION(0x1800, "roms", 0)
	ROM_LOAD("cent2.ci104",  0x0000, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3.ci103",  0x0800, 0x0400, CRC(f87abd63) SHA1(c3f48ffd46fad076fd064cbc0fdcc31641f5b1b6))
	ROM_RELOAD(0x0c00, 0x0400)
	ROM_LOAD("cent4.ci102",  0x1000, 0x0400, CRC(b69e95b6) SHA1(2f053a5848110d084239e1fc960198b247b3b98e))
	ROM_RELOAD(0x1400, 0x0400)
ROM_END

ROM_START(centauri2)
	ROM_REGION(0x1800, "roms", 0)
	ROM_LOAD("cent2.ci104",  0x0000, 0x0800, CRC(4f1ad0bc) SHA1(0622c16520275e629eac27ae2575e4e433de56ed))
	ROM_LOAD("cent3a.ci103", 0x0800, 0x0400, CRC(7b8215b1) SHA1(7cb6c18ad88060b56785bbde398bff157d8417cd))
	ROM_RELOAD(0x0c00, 0x0400)
	ROM_LOAD("cent4a.ci102", 0x1000, 0x0400, CRC(7ee64ea6) SHA1(b751b757faab7e3bb56625e4d72c3aeeb84a3f28))
	ROM_RELOAD(0x1400, 0x0400)
ROM_END

ROM_START(topaz)
	ROM_REGION(0x1800, "roms", 0)
	ROM_LOAD("topaz0.bin", 0x0400, 0x0400, CRC(d047aee0) SHA1(b2bc2e9fb088006fd3b7eb080feaa1eac479af58))
	ROM_LOAD("topaz1.bin", 0x0800, 0x0400, CRC(72a423c2) SHA1(e3ba5d581739fc0871901f861a7692fd86e0f6aa))
	ROM_LOAD("topaz2.bin", 0x0c00, 0x0400, CRC(b8d2e7c6) SHA1(e19bec04fab15536fea51c4298c6a4cb3817630c))
ROM_END

ROM_START(skatebrd)
	ROM_REGION(0x1800, "roms", 0)
	ROM_LOAD("skate2.bin",  0x0000, 0x0800, CRC(ee9b4c4c) SHA1(1a8b2ef8dfead18bfc62e85474dab2838b73ce08))
	ROM_LOAD("skate3.bin",  0x0800, 0x0400, CRC(58e181fe) SHA1(f54c8099100d0c96dc2ddbae8db9293f8581d459))
	ROM_RELOAD(0x0c00, 0x0400)
	ROM_LOAD("skate4.bin",  0x1000, 0x0400, CRC(fcdccffe) SHA1(a2db53f7bc555d705aa894e62307590fd74067dd))
	ROM_RELOAD(0x1400, 0x0400)
ROM_END

GAME( 1979, centauri,  0,        inderp, inderp, inderp_state, empty_init, ROT0, "Inder", "Centaur (Inder)",         MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, centauri2, centauri, inderp, inderp, inderp_state, empty_init, ROT0, "Inder", "Centaur (alternate set)", MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1979, topaz,     0,        inderp, inderp, inderp_state, empty_init, ROT0, "Inder", "Topaz (Inder)",           MACHINE_IS_SKELETON_MECHANICAL )
GAME( 1980, skatebrd,  0,        inderp, inderp, inderp_state, empty_init, ROT0, "Inder", "Skate Board (Inder)",     MACHINE_IS_SKELETON_MECHANICAL )
