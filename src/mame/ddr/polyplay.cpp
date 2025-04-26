// license:BSD-3-Clause
// copyright-holders:Martin Buchholz
// thanks-to:James Wallace, Martin Buchholz, Juergen Oppermann, Volker Hann, Jan-Ole Christian
/***************************************************************************

      Poly-Play
      (c) 1985 by VEB Polytechnik Karl-Marx-Stadt

      driver by Martin Buchholz (buchholz@mail.uni-greifswald.de)

      Very special thanks to the following people, each one of them spent
      some of their spare time to make this driver working:
      - Juergen Oppermann and Volker Hann for electronical assistance,
        repair work and ROM dumping.
      - Jan-Ole Christian from the Videogamemuseum in Berlin, which houses
        one of the last existing Poly-Play arcade automatons. He also
        provided me with schematics and service manuals.


TODO:
  - get other rom versions and games


NOTES:
  The hardware is based on the K1520 PC system, with video output coming through a standard Colormat TV.

  There are at least two revisions:

  - Revision 1 -
  2319-84-01   FAZ     Graphics card (Color and Sound Logic)
  2319-84-02   ABS     Graphics card (1KiB ROM for Charset)
  012-7100     ZRE     CPU-Board K2521 (3KiB ROM, 1KiB RAM)
  012-7040     PFS-1   ROM Board K3820 #1 - Games 1 to 4 (16KiB ROM)
  012-7040     PFS-2   ROM Board K3820 #2 - Games 5 to 8 (16KiB ROM)

  - Revision 2 -
  2319-84-01   FAZ     Graphics card (Color and Sound Logic)
  2319-84-02   ABS     Graphics card (1KiB ROM for Charset)
  02 899-0101  ZRE-PP  CPU-Board (56KiB ROM, 64KiB* RAM)

  Basically revision 2 combines the ZRE, PFS-1 and PFS-2 onto a single board.


memory map:
  - Revision 1 -
  0000 - 0fff ZRE    OS ROM                         (0000 - 03ff)
                     Game ROM used for Abfahrtslauf (0400 - 07ff)
                     Menu Screen ROM                (0800 - 0cff)
                     Work RAM                       (0d00 - 0fff)

  1000 - 4fff PFS-1  Abfahrtslauf          (1000 - 1bff)
                     Hirschjagd            (1c00 - 27ff)
                     Hase und Wolf         (2800 - 3fff)
                     Schmetterlingsfang    (4000 - 4fff)

  5000 - 8fff PFS-2  Schiessbude           (5000 - 5fff)
                     Autorennen            (6000 - 73ff)
                     opto-akust. Merkspiel (7400 - 7fff)
                     Wasserrohrbruch       (8000 - 8fff)

  e800 - ebff ABS   Character ROM (chr 00..7f) 1 bit per pixel
  ec00 - f7ff ABS   Character RAM (chr 80..ff) 3 bit per pixel
  f800 - ffff FAZ   Video RAM

  - Revision 2 -
  0000 - 0fff ZRE-PP OS and Menu ROM       (0000 - 1fff)
                     Game ROM #1           (2000 - 3fff)
                     Game ROM #2           (4000 - 5fff)
                     Game ROM #3           (6000 - 7fff)
                     Game ROM #4           (8000 - 9fff)
                     Game ROM #5           (a000 - bfff)
                     Game ROM #6           (c000 - dfff)

  e800 - ebff ABS   Character ROM (chr 00..7f) 1 bit per pixel
  ec00 - f7ff ABS   Character RAM (chr 80..ff) 3 bit per pixel
  f800 - ffff FAZ   Video RAM


i/o ports:
  - Revision 1 -
  80 - 83 ZRE    UB857D  (Z80 CTC)
  84 - 87 ZRE    UB855D  (Z80 PIO)

  - Revision 2 -
  80 - 83 ZRE-PP UB857D  (Z80 CTC)
  84 - 87 ZRE-PP UB855D  (Z80 PIO)
  88 - 8B ZRE-PP UB8560D (Z80 SIO)

  read:
  83        CTC COUNT 3 (IN1)
            used as hardware random number generator

  84        PIO PORT A (IN0)
            bit 0 = fire button
            bit 1 = right
            bit 2 = left
            bit 3 = up
            bit 4 = down
            bit 5 = unused*
            bit 6 = bookkeeping (Summe Spiele)
            bit 7 = coin sensor (+IRQ to make the game acknowledge it)

  85        PIO PORT B
            bit 0-2 = light organ
            bit 3-4 = control panel (not connected)
            bit 5-7 = sound parameter (not used on production units?)

  86        PIO CTRL A

  87        PIO CTRL B

  write:
  80        CTC COUNT 1 - Sound Channel 1
  81        CTC COUNT 2 - Sound Channel 2
  82        CTC COUNT 3 - generates 40 Hz timer for timeout in game title screens
  83        CTC COUNT 4 - generates main 75 Hz timer interrupt

The Poly-Play has a simple bookmarking system which can be activated
setting Bit 6 of PORTA (Summe Spiele) to low. It reads a double word
from 0c00 and displays it on the screen.
I currently haven't figured out how the I/O port handling for the book-
mark system works.

Uniquely the Poly-Play has a light organ which totally confuses you whilst
playing the automaton. Bits 0-2 of PORTB control the organ.

***************************************************************************/

#include "emu.h"
#include "polyplay.h"

#include "machine/z80daisy.h"

#include "screen.h"
#include "speaker.h"

#include "polyplay.lh"


static const z80_daisy_config daisy_chain_zre[] =
{
	{ Z80CTC_TAG },
	{ Z80PIO_TAG },
	{ nullptr }
};

static const z80_daisy_config daisy_chain_zrepp[] =
{
	{ Z80CTC_TAG },
	{ Z80PIO_TAG },
	{ Z80SIO_TAG },
	{ nullptr }
};

INTERRUPT_GEN_MEMBER(polyplay_state::nmi_handler)
{
	m_maincpu->pulse_input_line(INPUT_LINE_NMI, attotime::zero);
}

/* I/O Port handling */
void polyplay_state::ctc_zc2_w(int state)
{
	//osd_printf_verbose("ctc_z2_w: %02x\n", state);
}

uint8_t polyplay_state::pio_porta_r()
{
	return m_in0_port->read();
}

void polyplay_state::pio_porta_w(uint8_t data)
{
	//osd_printf_verbose("pio_porta_w: %02x\n", data);
}

uint8_t polyplay_state::pio_portb_r()
{
	uint8_t result = 0x00;
	//osd_printf_verbose("pio_portb_r: %02x\n", result);
	return result;
}

void polyplay_state::pio_portb_w(uint8_t data)
{
	uint8_t const lightState = data & 0x07;
	//uint8_t soundState = data & 0xe0;

	// there is a DS8205D attached to bit 0 and 1
	for (unsigned i = 0; 4 > i; ++i)
		m_lamps[i] = (lightState == i) ? 1 : 0;
}

INPUT_CHANGED_MEMBER(polyplay_state::input_changed)
{
	m_z80pio->port_a_write(m_in0_port->read());
}

/* memory mapping */
void polyplay_state::polyplay_mem_zre(address_map &map)
{
	map(0x0000, 0x0bff).rom();
	map(0x0c00, 0x0fff).ram();
	map(0x1000, 0x8fff).rom();
	map(0xe800, 0xebff).rom().region("gfx1", 0);
	map(0xec00, 0xf7ff).ram().w(FUNC(polyplay_state::polyplay_characterram_w)).share("characterram");
	map(0xf800, 0xffff).ram().share("videoram");
}

void polyplay_state::polyplay_mem_zrepp(address_map &map)
{
	map(0x0000, 0xbfff).rom();

	map(0xc000, 0xcfff).ram();

	map(0xd000, 0xd7ff).rom().region("gfx1", 0);

	map(0xea00, 0xebff).ram();
	map(0xec00, 0xf7ff).ram().w(FUNC(polyplay_state::polyplay_characterram_w)).share("characterram");

	map(0xf800, 0xffff).ram().share("videoram");
}

/* port mapping */
void polyplay_state::polyplay_io_zre(address_map &map)
{
	map.global_mask(0xff);
	map(0x80, 0x83).rw(m_z80ctc, FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));
	map(0x84, 0x87).rw(m_z80pio, FUNC(z80pio_device::read), FUNC(z80pio_device::write));
}

void polyplay_state::polyplay_io_zrepp(address_map &map)
{
	polyplay_io_zre(map);
	map(0x88, 0x8b).rw(m_z80sio, FUNC(z80sio_device::cd_ba_r), FUNC(z80sio_device::cd_ba_w));
}

static INPUT_PORTS_START( polyplay )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0) PORT_8WAY
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNUSED ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0) PORT_NAME("Bookkeeping Info") PORT_CODE(KEYCODE_F2)
	PORT_BIT( 0x80, IP_ACTIVE_HIGH, IPT_COIN1 ) PORT_CHANGED_MEMBER(DEVICE_SELF, FUNC(polyplay_state::input_changed), 0)
INPUT_PORTS_END

/* graphic structures */
static const gfx_layout charlayout_1_bit =
{
	8,8,    /* 8*8 characters */
	128,    /* 128 characters */
	1,      /* 1 bit per pixel */
	{ 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static const gfx_layout charlayout_3_bit =
{
	8,8,    /* 8*8 characters */
	128,    /* 128 characters */
	3,      /* 3 bit per pixel */
	{ 0, 128*8*8, 128*8*8 + 128*8*8 },    /* offset for each bitplane */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8 /* every char takes 8 consecutive bytes */
};

static GFXDECODE_START( gfx_polyplay )
	GFXDECODE_ENTRY( "gfx1",  0x0000, charlayout_1_bit, 0, 1 )
	GFXDECODE_RAM(   nullptr, 0xec00, charlayout_3_bit, 2, 1 )
GFXDECODE_END


/* the machine driver */
void polyplay_state::polyplay_zre(machine_config &config)
{
	/* basic machine hardware */
	Z80(config, m_maincpu, POLYPLAY_MAIN_CLOCK / 4); /* UB880D */
	m_maincpu->set_daisy_config(daisy_chain_zre);
	m_maincpu->set_addrmap(AS_PROGRAM, &polyplay_state::polyplay_mem_zre);
	m_maincpu->set_addrmap(AS_IO, &polyplay_state::polyplay_io_zre);
	m_maincpu->set_periodic_int(FUNC(polyplay_state::nmi_handler), attotime::from_hz(100)); /* A302 - zero cross detection from AC (50Hz) */

	/* devices */
	Z80CTC(config, m_z80ctc, POLYPLAY_MAIN_CLOCK / 4); /* UB857D */
	m_z80ctc->intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80ctc->zc_callback<0>().set(FUNC(polyplay_state::ctc_zc0_w));
	m_z80ctc->zc_callback<1>().set(FUNC(polyplay_state::ctc_zc1_w));
	//m_z80ctc->zc_callback<2>().set(FUNC(polyplay_state::ctc_zc2_w));

	Z80PIO(config, m_z80pio, POLYPLAY_MAIN_CLOCK / 4); /* UB855D */
	m_z80pio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
	m_z80pio->in_pa_callback().set(FUNC(polyplay_state::pio_porta_r));
	m_z80pio->out_pa_callback().set(FUNC(polyplay_state::pio_porta_w));
	m_z80pio->in_pb_callback().set(FUNC(polyplay_state::pio_portb_r));
	m_z80pio->out_pb_callback().set(FUNC(polyplay_state::pio_portb_w));

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(50);
	screen.set_size(64*8, 32*8);
	screen.set_visarea(0*8, 64*8-1, 0*8, 32*8-1);
	screen.set_screen_update(FUNC(polyplay_state::screen_update_polyplay));
	screen.set_palette(m_palette);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_polyplay);
	PALETTE(config, m_palette, FUNC(polyplay_state::polyplay_palette), 10);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	SPEAKER_SOUND(config, m_speaker1).add_route(ALL_OUTPUTS, "mono", 0.5);
	SPEAKER_SOUND(config, m_speaker2).add_route(ALL_OUTPUTS, "mono", 0.5);
}

void polyplay_state::polyplay_zrepp(machine_config &config)
{
	polyplay_zre(config);

	/* basic machine hardware */
	m_maincpu->set_daisy_config(daisy_chain_zrepp);
	m_maincpu->set_addrmap(AS_PROGRAM, &polyplay_state::polyplay_mem_zrepp);
	m_maincpu->set_addrmap(AS_IO, &polyplay_state::polyplay_io_zrepp);

	/* devices */
	Z80SIO(config, m_z80sio, POLYPLAY_MAIN_CLOCK / 4); /* UB8560D */
	m_z80sio->out_int_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);
}


/* ROM loading and mapping */
ROM_START( polyplay )
	ROM_REGION( 0x10000, Z80CPU_TAG, 0 )
	ROM_LOAD( "cpu_0000.37",       0x0000, 0x0400, CRC(87884c5f) SHA1(849c6b3f40496c694a123d6eec268a7128c037f0) )
	ROM_LOAD( "cpu_0400.36",       0x0400, 0x0400, CRC(d5c84829) SHA1(baa8790e77db66e1e543b3a0e5390cc71256de2f) )
	ROM_LOAD( "cpu_0800.35",       0x0800, 0x0400, CRC(5f36d08e) SHA1(08ecf8143e818a9844b4f168e68629d6d4481a8a) )
	ROM_LOAD( "2_-_1000.14",       0x1000, 0x0400, CRC(950dfcdb) SHA1(74170d5c99d1ea61fe37d1fe023dca96efb1ca69) )
	ROM_LOAD( "2_-_1400.10",       0x1400, 0x0400, CRC(829f74ca) SHA1(4df9d3c24e1bc4c2c953dce9530e43a00ecf67fc) )
	ROM_LOAD( "2_-_1800.6",        0x1800, 0x0400, CRC(b69306f5) SHA1(66d7c3cf76782a5b6eafa3e1513ecc9a9df0e0e1) )
	ROM_LOAD( "2_-_1c00.2",        0x1c00, 0x0400, CRC(aede2280) SHA1(0a01394ab70d07d666e955c87a08cb4d4945767e) )
	ROM_LOAD( "2_-_2000.15",       0x2000, 0x0400, CRC(6c7ad0d8) SHA1(df959d1e43fde96b5e21e3c53b397209a98ea423) )
	ROM_LOAD( "2_-_2400.11",       0x2400, 0x0400, CRC(bc7462f0) SHA1(01ca680c74b92b9ba5a85f98e0933ef1e754bfc1) )
	ROM_LOAD( "2_-_2800.7",        0x2800, 0x0400, CRC(9ccf1958) SHA1(6bdf04d7796074af7327fab6717b52736540f97c) )
	ROM_LOAD( "2_-_2c00.3",        0x2c00, 0x0400, CRC(21827930) SHA1(71d27d68f6973a59996102381f8754d9b353c65a) )
	ROM_LOAD( "2_-_3000.16",       0x3000, 0x0400, CRC(b3b3c0ec) SHA1(a94cd9794d59ea2f9ddd8bef86e6e3a269b276ad) )
	ROM_LOAD( "2_-_3400.12",       0x3400, 0x0400, CRC(bd416cd0) SHA1(57391cc4a417468455b45014969067629fd629b8) )
	ROM_LOAD( "2_-_3800.8",        0x3800, 0x0400, CRC(1c470b7c) SHA1(f7c71ee1752ecd4f30a35f14ee392b37febefb9c) )
	ROM_LOAD( "2_-_3c00.4",        0x3c00, 0x0400, CRC(b8354a19) SHA1(58ea7798ecc1be987b1217f4078c7cb366622dd3) )
	ROM_LOAD( "2_-_4000.17",       0x4000, 0x0400, CRC(1e01041e) SHA1(ff63e4bb924d1c26e445a28c5f8cbc696b4b9f5a) )
	ROM_LOAD( "2_-_4400.13",       0x4400, 0x0400, CRC(fe4d8959) SHA1(233f97956f4c819558d5d38034d92edc0e86a0de) )
	ROM_LOAD( "2_-_4800.9",        0x4800, 0x0400, CRC(c45f1d9d) SHA1(f3373f1f5a3c6099fd38e65f66e024ef042a984c) )
	ROM_LOAD( "2_-_4c00.5",        0x4c00, 0x0400, CRC(26950ad6) SHA1(881f5f0f4806ba6f21d0b28a70fc43363d51419b) )
	ROM_LOAD( "1_-_5000.30",       0x5000, 0x0400, CRC(9f5e2ba1) SHA1(58c696afbda8932f5e401b0a82b2de5cdfc2d1fb) )
	ROM_LOAD( "1_-_5400.26",       0x5400, 0x0400, CRC(b5f9a780) SHA1(eb785b7668f6af0a9df84cbd1905173869377e6c) )
	ROM_LOAD( "1_-_5800.22",       0x5800, 0x0400, CRC(d973ad12) SHA1(81cc5e19e83f2e5b10b885583c250a2ff66bafe5) )
	ROM_LOAD( "1_-_5c00.18",       0x5c00, 0x0400, CRC(9c22ea79) SHA1(e25ed745589a83e297dba936a6e5979f1b31b2d5) )
	ROM_LOAD( "1_-_6000.31",       0x6000, 0x0400, CRC(245c49ca) SHA1(12e5a032327fb45b2a240aff11b0c5d1798932f4) )
	ROM_LOAD( "1_-_6400.27",       0x6400, 0x0400, CRC(181e427e) SHA1(6b65409cd8410e632093662f5de2989dd9134620) )
	ROM_LOAD( "1_-_6800.23",       0x6800, 0x0400, CRC(8a6c1f97) SHA1(bf9d4dda8ac933a4a700f52540dcd1197f0a64eb) )
	ROM_LOAD( "1_-_6c00.19",       0x6c00, 0x0400, CRC(77901dc9) SHA1(b1132e06011aa8f7a95c43f447cd422f01139bb1) )
	ROM_LOAD( "1_-_7000.32",       0x7000, 0x0400, CRC(83ffbe57) SHA1(1e06408f7b4c9a4e5cadab58f6efbc03a5bedc1e) )
	ROM_LOAD( "1_-_7400.28",       0x7400, 0x0400, CRC(e2a66531) SHA1(1c9eb54e9c8a13f26335d8fb79fe5e39c28b3255) )
	ROM_LOAD( "1_-_7800.24",       0x7800, 0x0400, CRC(1d0803ef) SHA1(15a1996f9262f26cf531f329e086b10b3c25ce92) )
	ROM_LOAD( "1_-_7c00.20",       0x7c00, 0x0400, CRC(17dfa7e4) SHA1(afb471dc6cb2faccfb4305540f75162fcee3d622) )
	ROM_LOAD( "1_-_8000.33",       0x8000, 0x0400, CRC(6ee02375) SHA1(fbf797b655639ee442804a30fd3a06bbf261999a) )
	ROM_LOAD( "1_-_8400.29",       0x8400, 0x0400, CRC(9db09598) SHA1(8eb385542a617b23caad3ce7bbdd9714c1dd684f) )
	ROM_LOAD( "1_-_8800.25",       0x8800, 0x0400, CRC(ca2f963f) SHA1(34295f02bfd1bca141d650bbbbc1989e01c67b2f) )
	ROM_LOAD( "1_-_8c00.21",       0x8c00, 0x0400, CRC(0c7dec2d) SHA1(48d776b97c1eca851f89b0c5df4d5765d9aa0319) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "char.1",            0x0000, 0x0400, CRC(5242dd6b) SHA1(ba8f317df62fe4360757333215ce3c8223c68c4e) )
ROM_END

ROM_START( polyplay2 )
	ROM_REGION( 0x10000, Z80CPU_TAG, 0 )
	ROM_LOAD( "2_1.rom",           0x0000, 0x2000, CRC(d728ca42) SHA1(7359a59ba7f205b3ac95c8a0946093f24bdaa4da) )
	ROM_LOAD( "2_2.rom",           0x2000, 0x2000, CRC(52316236) SHA1(88c159621d40953240c5d2f1d6dcebb8f5e1ee81) )
	ROM_LOAD( "2_3.rom",           0x4000, 0x2000, CRC(e199d303) SHA1(fea2c3c659222553f36b3e922c6ac017f7111025) )
	ROM_LOAD( "2_4.rom",           0x6000, 0x2000, CRC(1324c2e2) SHA1(c0102d5abbb2f4dbb5b8dff5ad515bc3f2af9166) )
	ROM_LOAD( "2_5.rom",           0x8000, 0x2000, CRC(2e959fe9) SHA1(1f0a80bb26d0ae332d4a4313ec22ab7eefcf1e34) )
	ROM_LOAD( "2_6.rom",           0xA000, 0x2000, CRC(85774542) SHA1(420349a0d812bc0052a77cbc6c5ab84262e58b32) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "1_1.bin",           0x0000, 0x0800, CRC(4f028af9) SHA1(319537234069b43cfffafe567cc599ae05f24e23) )
ROM_END

ROM_START( polyplay2c )
	ROM_REGION( 0x10000, Z80CPU_TAG, 0 )
	ROM_LOAD( "2_1.bin",           0x0000, 0x2000, CRC(bf22f44f) SHA1(dd412b45f49ceffe336d905043c7af2447c577a0) )
	ROM_LOAD( "2_2.bin",           0x2000, 0x2000, CRC(e54b8be8) SHA1(23037102eab60fd03c349ad154b2498139e84dd4) )
	ROM_LOAD( "2_3.bin",           0x4000, 0x2000, CRC(ac43ec6b) SHA1(1662e10d80d47c1e3c54d4ef232c0d671fecce96) )
	ROM_LOAD( "2_4.bin",           0x6000, 0x2000, CRC(703f3d46) SHA1(5ab571cb63ffdbce86b6e452988402e5c1098009) )
	ROM_LOAD( "2_5.bin",           0x8000, 0x2000, CRC(d5ef6ed8) SHA1(317f251dc85b412b1390651d39a95ce57dd92999) )

	ROM_REGION( 0x800, "gfx1", 0 )
	ROM_LOAD( "1_1.bin",           0x0000, 0x0800, CRC(4f028af9) SHA1(319537234069b43cfffafe567cc599ae05f24e23) )
ROM_END

/* game driver */
GAMEL( 1986, polyplay,   0,         polyplay_zre,   polyplay, polyplay_state, empty_init, ROT0, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Play (ZRE)",            0, layout_polyplay )
GAMEL( 1989, polyplay2,  0,         polyplay_zrepp, polyplay, polyplay_state, empty_init, ROT0, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Play (ZRE-PP)",         0, layout_polyplay )
GAMEL( 1989, polyplay2c, polyplay2, polyplay_zrepp, polyplay, polyplay_state, empty_init, ROT0, "VEB Polytechnik Karl-Marx-Stadt", "Poly-Play (ZRE-PP - Czech)", 0, layout_polyplay )
