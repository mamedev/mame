// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    "Unknown VRender0+ Bowling Game" (c) 200x A1 Amusement One

    TODO:
    - checkout the actual SanDisk device

***************************************************************************/

#include "emu.h"
#include "cpu/se3208/se3208.h"
#include "machine/vrender0.h"
#include "sound/vrender0.h"
#include "video/vrender0.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class v0bowl_state : public driver_device
{
public:
	v0bowl_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_vr0soc(*this, "vr0soc")
	{}

	void v0bowl(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

private:
	void v0bowl_map(address_map &map) ATTR_COLD;

	// devices
	required_device<se3208_device> m_maincpu;
	required_device<vrender0soc_device> m_vr0soc;
};


void v0bowl_state::v0bowl_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom().nopw().region("ipl", 0);

//  map(0x01500000, 0x01500003).portr("IN0");
//  map(0x01500004, 0x01500007).portr("IN1");
//  map(0x01500008, 0x0150000b).portr("IN2");

	map(0x01800000, 0x01ffffff).m(m_vr0soc, FUNC(vrender0soc_device::regs_map));

	map(0x02000000, 0x027fffff).ram().share("workram");

	map(0x03000000, 0x04ffffff).m(m_vr0soc, FUNC(vrender0soc_device::audiovideo_map));
//  map(0x05000000, 0x0500000f) SanDisk
}


static INPUT_PORTS_START( v0bowl )
	/* dummy active high structure */
	PORT_START("SYSA")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )

	/* dummy active low structure */
	PORT_START("DSWA")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
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

void v0bowl_state::machine_start()
{
	// ...
}

void v0bowl_state::machine_reset()
{
	// ...
}

void v0bowl_state::v0bowl(machine_config &config)
{
	// TODO: clock to be tuned up
	SE3208(config, m_maincpu, 14318180 * 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &v0bowl_state::v0bowl_map);
	m_maincpu->iackx_cb().set(m_vr0soc, FUNC(vrender0soc_device::irq_callback));

	VRENDER0_SOC(config, m_vr0soc, 14318180 * 3);
	m_vr0soc->set_host_cpu_tag(m_maincpu);
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

ROM_START( v0bowl )
	ROM_REGION32_LE( 0x1000000, "ipl", ROMREGION_ERASEFF )
	ROM_LOAD( "bootrom.u16",  0x000000, 0x080000, CRC(f8f4cf22) SHA1(881d8eeb6f50b2e7933e7c3c93adcdd0c1e93e77) )

	ROM_REGION32_LE( 0x20000000, "flash", ROMREGION_ERASEFF ) // SanDisk CF
	// zipped FAT16, filesystem was made with DOS 3.31 but the folders have Windows Thumbs.db files as well.
	ROM_LOAD( "bowling.001",  0x000000, 0x1e8be000, BAD_DUMP CRC(aebb2b01) SHA1(cd11f74f6512350ac10f822937b8769f552aabf3) )
ROM_END

} // anonymous namespace


GAME( 200?, v0bowl,  0,   v0bowl,  v0bowl, v0bowl_state, empty_init, ROT0, "A1 Amusement One",      "unknown VRender0+ bowling game", MACHINE_IS_SKELETON ) // Return Bowl?
