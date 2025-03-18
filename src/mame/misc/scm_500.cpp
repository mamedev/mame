// license:BSD-3-Clause
// copyright-holders:David Haywood
/*

http://www.standardchange.com/frequently-asked-questions

"The System 500 series note acceptor was sold in change machines from 1987 to 1994"

"The System 500E series note acceptor replaced the System 500 note acceptor in 1995."

*/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"


namespace {

class scm_500_state : public driver_device
{
public:
	scm_500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{
	}

	void scm_500(machine_config &config);

private:
	void prog_map(address_map &map) ATTR_COLD;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	required_device<mcs51_cpu_device> m_maincpu;
};


void scm_500_state::prog_map(address_map &map)
{
	map(0x0000, 0xffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( scm_500 )
INPUT_PORTS_END

void scm_500_state::machine_start()
{
}

void scm_500_state::machine_reset()
{
}

void scm_500_state::scm_500(machine_config &config)
{
	I80C51GB(config, m_maincpu, 12'000'000); // unknown clock
	m_maincpu->set_addrmap(AS_PROGRAM, &scm_500_state::prog_map);
}

/*

this is the Eprom for the new $$ for the system 500  changers.

device is a 27c512

4_42 is latest version
for both system 500 and 500E


----------------------------------------------------------

4.26 and 4.27, 4.28  are older versions of program.

Std-chan is most current program.

500E-3.07 is for the 500E system,

*/

#define ROM_LOAD_BIOS(bios,name,offset,length,hash) \
	ROMX_LOAD(name, offset, length, hash, ROM_BIOS(bios))

ROM_START( scm_500 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS( 0, "usa_442",   "USA 4.42" )
	ROM_LOAD_BIOS( 0, "stndxgr_442", 0x00000, 0x10000, CRC(7e641cfa) SHA1(5c9ed9551f3c0496bc6c810375595e821a2eb75d)  ) // USA 04.42 (for 500 and 500E)
	ROM_SYSTEM_BIOS( 1, "usa_431",   "USA 4.31" )
	ROM_LOAD_BIOS( 1, "stndxgr_431", 0x00000, 0x10000, CRC(66a179d7) SHA1(3cdb06917616b57e3e3947007f6e2d923fb592a2)  ) // USA 04.31
	ROM_SYSTEM_BIOS( 2, "usa_427",   "USA 4.27" )
	ROM_LOAD_BIOS( 2, "stndxgr_427", 0x00000, 0x10000, CRC(908cda2f) SHA1(2a32922305b8201c25310679d5dd02f0951a4985)  ) // USA 04.27
	ROM_SYSTEM_BIOS( 3, "usa_426",   "USA 4.26" )
	ROM_LOAD_BIOS( 3, "stndxgr_426", 0x00000, 0x10000, CRC(fa459c77) SHA1(cafbde4256cf69ae422a700adfadeeaec4af53d3)  ) // USA 04.36
	ROM_SYSTEM_BIOS( 4, "usa_307",   "USA 3.07" )
	ROM_LOAD_BIOS( 4, "stndxgr_307", 0x00000, 0x10000, CRC(4d0d91c6) SHA1(85ff5d43ec331bcd4cde6aaf82f6143acc7e020c)  ) // USA 03.07 (could be 500E specific)
ROM_END

} // anonymous namespace


GAME( 1987, scm_500,  0,    scm_500, scm_500, scm_500_state, empty_init, ROT0, "Standard Change-Makers", "Standard Change-Makers System 500 / 500E", MACHINE_NO_SOUND | MACHINE_NOT_WORKING | MACHINE_MECHANICAL | MACHINE_REQUIRES_ARTWORK )
// 1995 - 500E - same basic hw?
