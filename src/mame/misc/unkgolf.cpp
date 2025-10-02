// license:BSD-3-Clause
// copyright-holders:

/*
HSG-001A PCB

This is probably a golf game (or part of it), given the ROM labels.

Main components:
Sharp LH0080A Z80A-CPU-D
Sharp LH0082A Z80A-CTC-D
HM6264ALP-15 RAM
4x TMP82C55AP-2 (with 2 more empty spaces)
OKI M6376
20 Mhz XTAL
4.096 Mhz XTAL
8-DIP bank
1 push button
*/

#include "emu.h"

#include "cpu/z80/z80.h"
#include "machine/i8255.h"
#include "machine/z80ctc.h"
#include "machine/z80daisy.h"
#include "sound/okim6376.h"

#include "speaker.h"


namespace {

class unkgolf_state : public driver_device
{
public:
	unkgolf_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_oki(*this, "oki")
	{}

	void unkgolf(machine_config &config) ATTR_COLD;

private:
	required_device<z80_device> m_maincpu;
	required_device<okim6376_device> m_oki;

	void program_map(address_map &map) ATTR_COLD;
	void io_map(address_map &map) ATTR_COLD;
	void ppi1_w(uint8_t data);
	void ppi4_w(uint8_t data);

};


void unkgolf_state::program_map(address_map &map)
{
	map(0x0000, 0x3fff).rom();
	map(0x4000, 0x47ff).ram();
}

void unkgolf_state::io_map(address_map &map)
{
	map.global_mask(0xff);

	map(0x00, 0x03).rw("ctc", FUNC(z80ctc_device::read), FUNC(z80ctc_device::write));

	map(0x10, 0x13).rw("ppi0", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x20, 0x23).rw("ppi1", FUNC(i8255_device::read), FUNC(i8255_device::write));

	map(0x30, 0x30).w(m_oki, FUNC(okim6376_device::write));  // seems wrong

	map(0x40, 0x43).rw("ppi2", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x50, 0x53).rw("ppi3", FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x60, 0x63).rw("ppi4", FUNC(i8255_device::read), FUNC(i8255_device::write));  // indirect input/output through 61h. no sense at all since was set to output.
//  map(0x70, 0x73).rw("ppi5", FUNC(i8255_device::read), FUNC(i8255_device::write));  // initialized, but not used.

/*
  .-----.-----------.---------.---------.---------.-------.-------.-------.------.
  | PPI | Map Range | control | PA mode | PB mode | PortA | PortB | PortC | Used |
  '-----+-----------'---------'---------'---------'-------'-------'-------'------'
   PPI0    10h-13h     0x90       M0        M0       IN*     OUT     OUT*   YES
   PPI1    20h-23h     0x80       M0        M0       OUT*    OUT     OUT    YES
   PPI2    40h-43h     0x9B       M0        M0       IN*     IN*     IN*    YES
   PPI3    50h-53h     0x9B       M0        M0       IN*     IN*     IN*    YES
   PPI4    60h-63h     0x80       M0        M0       OUT     OUT**   OUT    NO
   PPI5    70h-73h     0x80       M0        M0       OUT     OUT     OUT    NO

   *   Activity.
   **  Indirect input/output.

*/
}

void unkgolf_state::ppi1_w(uint8_t data)
{
	logerror("PPI1 OUT: %02X\n", data);
}

void unkgolf_state::ppi4_w(uint8_t data)
{
	logerror("PPI4 OUT: %02X\n", data);
}


static INPUT_PORTS_START( unkgolf )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1) PORT_NAME("IN0-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2) PORT_NAME("IN0-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3) PORT_NAME("IN0-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4) PORT_NAME("IN0-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5) PORT_NAME("IN0-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6) PORT_NAME("IN0-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7) PORT_NAME("IN0-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8) PORT_NAME("IN0-8")

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Q) PORT_NAME("IN1-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_W) PORT_NAME("IN1-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_E) PORT_NAME("IN1-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_R) PORT_NAME("IN1-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_T) PORT_NAME("IN1-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Y) PORT_NAME("IN1-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_U) PORT_NAME("IN1-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_I) PORT_NAME("IN1-8")

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_A) PORT_NAME("IN2-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_S) PORT_NAME("IN2-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_D) PORT_NAME("IN2-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_F) PORT_NAME("IN2-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_G) PORT_NAME("IN2-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_H) PORT_NAME("IN2-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_J) PORT_NAME("IN2-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_K) PORT_NAME("IN2-8")

	PORT_START("IN3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_Z) PORT_NAME("IN3-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_X) PORT_NAME("IN3-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_C) PORT_NAME("IN3-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_V) PORT_NAME("IN3-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_B) PORT_NAME("IN3-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_N) PORT_NAME("IN3-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_M) PORT_NAME("IN3-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_L) PORT_NAME("IN3-8")

	PORT_START("IN4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_1_PAD) PORT_NAME("IN4-1")
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_2_PAD) PORT_NAME("IN4-2")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_3_PAD) PORT_NAME("IN4-3")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_4_PAD) PORT_NAME("IN4-4")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_5_PAD) PORT_NAME("IN4-5")
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_6_PAD) PORT_NAME("IN4-6")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_7_PAD) PORT_NAME("IN4-7")
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_OTHER ) PORT_CODE(KEYCODE_8_PAD) PORT_NAME("IN4-8")

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("DSW")
	PORT_DIPNAME( 0x01, 0x01, "DSW" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

INPUT_PORTS_END


static const z80_daisy_config daisy_chain[] =
{
	{ "ctc" },
	{ nullptr }
};


void unkgolf_state::unkgolf(machine_config &config)
{
	Z80(config, m_maincpu, 20_MHz_XTAL / 5); // divisor unknown
	m_maincpu->set_daisy_config(daisy_chain);
	m_maincpu->set_addrmap(AS_PROGRAM, &unkgolf_state::program_map);
	m_maincpu->set_addrmap(AS_IO, &unkgolf_state::io_map);

	z80ctc_device& ctc(Z80CTC(config, "ctc", 20_MHz_XTAL / 5)); // divisor unknown
	ctc.intr_callback().set_inputline(m_maincpu, INPUT_LINE_IRQ0);

	i8255_device &ppi0(I8255(config, "ppi0"));
	// (10-13) Mode 0 - Ports A set as input, Ports B, high C & low C as output.
	ppi0.in_pa_callback().set_ioport("DSW");

	i8255_device &ppi1(I8255(config, "ppi1"));
	// (20-23) Mode 0 - Ports A, B, high C & low C set as output.
	ppi1.out_pa_callback().set(FUNC(unkgolf_state::ppi1_w));

	i8255_device &ppi2(I8255(config, "ppi2"));
	// (40-43) Mode 0 - Ports A, B, high C & low C set as input.
	ppi2.in_pa_callback().set_ioport("IN0");
	ppi2.in_pb_callback().set_ioport("IN1");
	ppi2.in_pc_callback().set_ioport("IN2");

	i8255_device &ppi3(I8255(config, "ppi3"));
	// (50-53) Mode 0 - Ports A, B, high C & low C set as input.
	ppi3.in_pa_callback().set_ioport("IN3");
	ppi3.in_pb_callback().set_ioport("IN4");
	ppi3.in_pc_callback().set_ioport("IN5");

	i8255_device &ppi4(I8255(config, "ppi4"));
	// (60-63) Mode 0 - Ports A, B, high C & low C set as output.
	ppi4.out_pa_callback().set(FUNC(unkgolf_state::ppi4_w));


	// sound
	SPEAKER(config, "mono").front_center();
	OKIM6376(config, "oki", 4.096_MHz_XTAL / 4).add_route(ALL_OUTPUTS, "mono", 1.0);  // divisor unknown
}


ROM_START( unkgolf )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "pinest_4096h.u3", 0x00000, 0x10000, CRC(0f97fd6c) SHA1(d0f8ebf3414929498a8a014252ce61974e3b5d77) ) // 1ST AND 2ND HALF IDENTICAL, handwritten label

	ROM_REGION( 0x100000, "oki", 0 ) // handwritten labels
	ROM_LOAD( "golf11", 0x00000, 0x80000, CRC(48234f9e) SHA1(bd2d0c17b532fe105485d64a04c76b7a9d6b2f26) )
	ROM_LOAD( "golf12", 0x80000, 0x80000, CRC(2ee904e5) SHA1(b7565f5a1eb677e0d05aa43f302a0c50be48b708) ) // 1xxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 19??, unkgolf, 0, unkgolf, unkgolf, unkgolf_state, empty_init, ROT0, "<unknown>", "unknown golf game", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
