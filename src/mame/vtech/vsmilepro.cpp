// license:BSD-3-Clause
// copyright-holders:Fabio Priuli
/***************************************************************************

    Skeleton driver for VTech V.Smile Pro CD System

    30/05/2016

    Some information about the hardware can be found at

    http://www.x86-secret.com/dossier-64-VTech_V_Smile_Pro.html


    In particular
    - It uses a LSI Zevio 1020 CPU + peripherals which comprises ARM926EJ-S CPU,
      ZSP400 DSP, 3D graphics processor & 2D graphics processor
    - The CD controller is a Sony CXD3059AR

****************************************************************************/

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "imagedev/cdromimg.h"

#include "cdrom.h"
#include "softlist.h"


namespace {

class vsmilpro_state : public driver_device
{
public:
	vsmilpro_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void vsmilpro(machine_config &config);

private:
	void vsmilpro_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
};

void vsmilpro_state::vsmilpro_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).rom();
}


// Input ports
static INPUT_PORTS_START( vsmilpro )
INPUT_PORTS_END

void vsmilpro_state::vsmilpro(machine_config &config)
{
	// basic machine hardware
	ARM9(config, m_maincpu, 150000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vsmilpro_state::vsmilpro_map);

	CDROM(config, "cdrom").set_interface("vsmile_vdisk");

	SOFTWARE_LIST(config, "cd_list").set_original("vsmile_cd");
}

// ROM definition
ROM_START( vsmilpro )
	ROM_REGION( 0x200000, "maincpu", 0 )
	ROM_LOAD( "70004.bin", 0x000000, 0x200000, CRC(b9161eac) SHA1(8d75fdeda8c4e228a0b1efd35011f9f667f9fb23) )
ROM_END

} // anonymous namespace


// Driver

//    YEAR  NAME      PARENT  COMPAT  MACHINE   INPUT     CLASS           INIT        COMPANY  FULLNAME       FLAGS
COMP( 2007, vsmilpro, 0,      0,      vsmilpro, vsmilpro, vsmilpro_state, empty_init, "VTech", "V.Smile Pro", MACHINE_IS_SKELETON )
