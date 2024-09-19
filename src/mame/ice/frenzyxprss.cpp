// license:BSD-3-Clause
// copyright-holders:
/*
    Skeleton driver for ICE Frenzy Express.
    PC-based configuration running Windows 98 SE SP3:
    - 694T Pro Ver 5 motherboard (Via VT82C686B + Via VT82C694T).
    - Intel Celeron CPU 1000A/256/100/1.475 Q208A083-0620 SL5ZF.
    - 256MB PC133 RAM (one single M366S3253CTS-C7A module).
    - InsideTNC IV011A AGP graphics card.
    - Crystal CS4281-CM EP based PCI sound card.
    - "FE 107 I/O" ISA card.

   FE 107 I/O
       ________    ________    ________    ________
   ___|       |___|       |___|       |___|       |___
  |   |_______|   |_______|   |_______|   |_______|  |
  |                                                  |
  |  __________   __________  __________             |
  | |DM74LS245N  |ADC0838CCN |DM74LS245N             |
  |                 _____________                    |
  |  _________     | ACTEL      |         __________ |
  | |DM74LS14N     | A40MX04-F  |        |DM74LS245N |
  |   _______      | PL84 0007  |                    |
  |  | Xtal |      |            |                    |
  |  4.9152MHz     |____________|                    |
  |   ___                                            |
  |  |___|<-Empty socket for 93C46                   |
  |_             ____                        ________|
    |_|_|_|_|_|_|   |_|_|_|_|_|_|_|_|_|_|_|_|
              ISA SLOT
*/

#include "emu.h"
#include "cpu/i386/i386.h"
#include "machine/pci.h"

namespace {

class frenzyxprss_state : public driver_device
{
public:
	frenzyxprss_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu")
	{ }

	void frenzyxprss(machine_config &config);

private:
	required_device<cpu_device> m_maincpu;

	void frenzyxprss_map(address_map &map) ATTR_COLD;
};


void frenzyxprss_state::frenzyxprss_map(address_map &map)
{
	map(0x00000000, 0x0009ffff).ram();
	map(0x000e0000, 0x000fffff).rom().region("bios", 0x20000);
	map(0xfffc0000, 0xffffffff).rom().region("bios", 0);
}

static INPUT_PORTS_START( frenzyxprss )
INPUT_PORTS_END

void frenzyxprss_state::frenzyxprss(machine_config &config)
{
	PENTIUM3(config, m_maincpu, 100'000'000); // Intel Celeron SL5ZF 1GHz
	m_maincpu->set_addrmap(AS_PROGRAM, &frenzyxprss_state::frenzyxprss_map);

	PCI_ROOT(config, "pci", 0);
	// ...
}

/***************************************************************************

  Game drivers

***************************************************************************/

ROM_START( frenzyxprss )
	ROM_REGION32_LE( 0x40000, "bios", 0 )
	ROM_SYSTEM_BIOS( 0, "750", "2002-04-19" )
	ROMX_LOAD( "a6309vms_2002-04-19.750.u24", 0x00000, 0x40000, CRC(a227ff2a) SHA1(eea6b336082bf8091f120b6c4cc9bb61c3c3c234), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "740", "2002-02-28" )
	ROMX_LOAD( "a6309vms_2002-02-28.740.u24", 0x00000, 0x40000, CRC(316df0fd) SHA1(94995f35356e136c51abba3be05fe97b2c1baf7b), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "73x", "2002-02-07" )
	ROMX_LOAD( "a6309vms_2002-02-07.73x.u24", 0x00000, 0x40000, CRC(82c3b24c) SHA1(4b145def75e62fc64ebecf2ad666c9ab580b5d38), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 3, "730", "2002-01-07" )
	ROMX_LOAD( "a6309vms_2002-01-07.730.u24", 0x00000, 0x40000, CRC(3c226a0b) SHA1(c8ccab6eb8acc775732055eebf914b274d314c37), ROM_BIOS(3) )
	ROM_SYSTEM_BIOS( 4, "720", "2001-11-03" )
	ROMX_LOAD( "a6309vms_2001-11-03.720.u24", 0x00000, 0x40000, CRC(7bd0ced9) SHA1(ddd3bdde983c7b3746fc9a7ee8d9dea9988089ce), ROM_BIOS(4) )
	ROM_SYSTEM_BIOS( 5, "710", "2001-09-19" )
	ROMX_LOAD( "a6309vms_2001-09-19.710.u24", 0x00000, 0x40000, CRC(bb2c094e) SHA1(2c2def2b5b22d7f66661742f23d7a0fc23cd8cff), ROM_BIOS(5) )
	ROM_SYSTEM_BIOS( 6, "700", "2001-07-11" )
	ROMX_LOAD( "a6309vms_2001-07-11.700.u24", 0x00000, 0x40000, CRC(72081fd3) SHA1(99556f6d7b638f229c466245eed82eb47a2c2304), ROM_BIOS(6) )
	ROM_DEFAULT_BIOS("73x") // The one dumped from the actual machine

	DISK_REGION( "ide:0:hdd" )
	DISK_IMAGE( "fexpress_cf_version_2.1", 0, SHA1(583607be83048ca10b1837a8982ba379256a3cf2) )
ROM_END

} // Anonymous namespace


GAME(2001, frenzyxprss, 0, frenzyxprss, frenzyxprss, frenzyxprss_state, empty_init, ROT0, "ICE / Uniana", "Frenzy Express", MACHINE_IS_SKELETON)
