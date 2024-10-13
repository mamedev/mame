// license:BSD-3-Clause
// copyright-holders:David Haywood

/*

PCB contains the following major components


ESS VideoDrive
ES3207FP B390
TTV32098A

ESS VideoDrive
ES3210F Q390
TTT22869A

SAMSUNG C039A
S1L9223A01-Q0

SAMSUNG C031
S5L9284D01-Q0

ASD  AE43BH4016I-35
50G00290919D

F 037B
KA9259D

HOLTEK
GT27C020-70
A039K1523-2

PT6312LQ
PTC  0014Z

74F125D
C034205
fnn0040L

74F125D
C034205
fnn0040L

*/

#include "emu.h"

#include "bus/ata/atapicdr.h"
#include "cpu/mipsx/mipsx.h"

#include "softlist_dev.h"


namespace {

class kisssite_state : public driver_device

{
public:
	kisssite_state(machine_config const &mconfig, device_type type, char const *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{
	}

	void kisssite(machine_config &config) ATTR_COLD;

protected:
	virtual void machine_reset() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;

	void mem(address_map &map) ATTR_COLD;
};


void kisssite_state::machine_reset()
{
	m_maincpu->set_state_int(STATE_GENPC, 0x1cffff80); // might actually be 0x7fffff80 with a ROM mirror
}

void kisssite_state::mem(address_map &map)
{
	map(0x00000000, 0x0007ffff).ram();
	map(0x1c000000, 0x1c03ffff).mirror(0x00fc0000).rom().region("maincpu", 0);
	// registers at 0x20000000-0x2000ffff, ES6008 datasheet could be helpful
}

void kisssite_state::kisssite(machine_config &config)
{
	MIPSX(config, m_maincpu, 60'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &kisssite_state::mem);

	CDROM(config, "cdrom").set_interface("cdrom");
	SOFTWARE_LIST(config, "cd_list").set_original("kisssite_cd");
}

INPUT_PORTS_START(kisssite)
INPUT_PORTS_END

ROM_START(kisssite)
	ROM_REGION32_BE(0x040000, "maincpu", 0 )
	ROM_LOAD("ht27c020.u10", 0x000000, 0x040000, CRC(ccedce2b) SHA1(28dd3dfd0b8de0c5aa1c37d193ffc479d46563a1) )
ROM_END

} // anonymous namespace

SYST(200?, kisssite, 0,         0,      kisssite, kisssite, kisssite_state, empty_init, "Tomy", "Kiss-Site", MACHINE_IS_SKELETON)
