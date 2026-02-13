// license:BSD-3-Clause
// copyright-holders:

/*
System 6(MG) MAIN PCB
8910960104 (8910970104)

VRenderZERO+ MagicEyes SoC
M27C801 program ROM
15.34098 XTAL (OSC 2)
14.31818 XTAL (OSC 3)
3x T436416A SRAM (near SoC)
CY62256VLL-70 SRAM
TC58DVM82A1FT00 NAND
R4543 RTC
Altera MAX EPM3128AT100-10 with KC004B sticker
2x empty spaces marked for CXD1178Q
ADM3222 ARU transceiver
*/


#include "emu.h"

#include "cpu/se3208/se3208.h"
#include "machine/nvram.h"
#include "machine/rtc4543.h"
#include "machine/vrender0.h"

#include "emupal.h"
#include "speaker.h"


namespace {

class namcos6_state : public driver_device
{
public:
	namcos6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vr0soc(*this, "vr0soc")
	{ }

	void namcos6(machine_config &config) ATTR_COLD;

private:
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;

	void program_map(address_map &map) ATTR_COLD;
};


void namcos6_state::program_map(address_map &map)
{
	map(0x00000000, 0x000fffff).rom().nopw();
	// map(0x01600000, 0x01607fff).ram(); // possibly?
	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));
	// map(0x01802000, 0x01802003).rw(FUNC(namcos6_state::piolddr_r), FUNC(namcos6_state::piolddr_w));
	// map(0x01802004, 0x01802007).rw(FUNC(namcos6_state::pioldat_r), FUNC(namcos6_state::pioldat_w));
	// map(0x01802008, 0x0180200b).r(FUNC(namcos6_state::pioedat_r));
	map(0x02000000, 0x027fffff).ram();
	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
}


static INPUT_PORTS_START( namcos6 )
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
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
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END


void namcos6_state::namcos6(machine_config &config)
{
	SE3208(config, m_maincpu, 14.318181_MHz_XTAL * 3); // multiplier not verified
	m_maincpu->set_addrmap(AS_PROGRAM, &namcos6_state::program_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	RTC4543(config, "rtc", 32.768_kHz_XTAL);

	VRENDER0_SOC(config, m_vr0soc, 14.318181_MHz_XTAL * 6); // multiplier not verified
	m_vr0soc->set_host_space_tag(m_maincpu, AS_PROGRAM);
	m_vr0soc->int_callback().set_inputline(m_maincpu, se3208_device::SE3208_INT);
	m_vr0soc->set_external_vclk(14.318181_MHz_XTAL * 2); // not verified

	SPEAKER(config, "speaker", 2).front();
	m_vr0soc->add_route(0, "speaker", 1.0, 0);
	m_vr0soc->add_route(1, "speaker", 1.0, 1);
}


ROM_START( ddflower )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD( "dfl1_prg0.6e", 0x000000, 0x100000, CRC(74106ef7) SHA1(d98360b6eb14f5e43ed8386f48e8be4954fa28f7) ) // 1xxxxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x1080010, "nand", 0 )
	ROM_LOAD( "tc58dvm82a1ft00.7c", 0x0000000, 0x1080010, CRC(51d1ac3c) SHA1(dccecee9c1313a02fa9360501d6e15e961da2909) ) // 1xxxxxxxxxxxxxxxxxxxxxxx = 0xFF
ROM_END

} // anonymous namespace


GAME( 2003, ddflower, 0, namcos6, namcos6, namcos6_state, empty_init, ROT0, "Namco", "Doki Doki! Flower (DFL1, Ver. A)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
