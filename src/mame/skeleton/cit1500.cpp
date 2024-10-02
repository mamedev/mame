// license:BSD-3-Clause
// copyright-holders:AJR
/****************************************************************************

    Skeleton driver for public pay phones by Alcatel/Telefónica/Citesa.

Components (trmavia):

  Parts side:
CIT1500 (80-pin 78K/0 MCU): NEC CITESA ICS1 7CE52872AA 0143H8L01
CIT1200: ST HCF4067 Analog Multiplexer/demultiplexer
CIT500: Philips TEA1067AT Telephone Transmission Circuit With Dialler Interface
CIT1400 (display?) NEC D7225GB 0119EY002
  Subboard 49:
(visible side) FX604D4 V23 modem
(back side) CMX673D4 Call progress tone detector
  Reverse side:
LCD display
DS1302S trickle-charge timekeeping
Philips 3312CT DTMF/modem/musical-tone generator
Philips 74HC573D (behind the LCD)

Components (teletup):

  Parts side:
CIT1500 (80-pin 78K/0 MCU): NEC CITESA ICS1 7CE52872AA 0143H8L01
CIT1200: ST HCF4067 Analog Multiplexer/demultiplexer
CIT500: Philips TEA1067AT Telephone Transmission Circuit With Dialler Interface
CIT1400: (display?) NEC D7225GB 0119EY002
CIT900: CMX673D4 Call progress tone detector
  Reverse side:
LCD display
Philips 3312CT DTMF/modem/musical-tone generator
DS1302S trickle-charge timekeeping

****************************************************************************/

#include "emu.h"
#include "cpu/upd78k/upd78k0.h"
//#include "machine/ds1302.h"
//#include "video/upd7225.h"
//#include "screen.h"


namespace {

class cit1500_state : public driver_device
{
public:
	cit1500_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_cit1500(*this, "cit1500")
	{
	}

	void cit1500(machine_config &config);

private:
	void mem_map(address_map &map) ATTR_COLD;

	required_device<upd78053_device> m_cit1500;
};


void cit1500_state::mem_map(address_map &map)
{
	map(0x6000, 0xfa7f).rom().region("cit1600", 0x6000); // TODO: banked
}


static INPUT_PORTS_START(cit1500)
INPUT_PORTS_END


void cit1500_state::cit1500(machine_config &config)
{
	UPD78053(config, m_cit1500, 5_MHz_XTAL, 32.768_kHz_XTAL);
	m_cit1500->set_addrmap(AS_PROGRAM, &cit1500_state::mem_map);

	//UPD7225(config, "cit1400");

	//SCREEN(config, "screen", SCREEN_TYPE_LCD);

	//DS1302(config, "ds1302", 32.768_kHz_XTAL);
}


ROM_START(trmavia)
	ROM_REGION(0x20000, "cit1600", 0) // external program ROM
	ROM_LOAD("3cq-1039-e-183cf58-3a-28-06-01_m27w101.cit1600", 0x00000, 0x20000, CRC(db6bff63) SHA1(1b33ad98112d2946edc9444c8e6df4b4da778377))

	ROM_REGION(0x6000, "cit1500", 0) // 24K internal mask ROM
	ROM_LOAD("citesa_ics1_7ce52872aa_0143h8l01", 0x0000, 0x6000, NO_DUMP)
	ROM_COPY("cit1600", 0x06000, 0x0000, 2) // hack to provide at least a reset vector

	ROM_REGION(0x1000, "cit1300", 0)
	ROM_LOAD("tpe-2c-eaaa_24lc32a.cit1300", 0x0000, 0x1000, CRC(141d94e0) SHA1(f76a920a4eb6476999b5b59a1bf9eaa7b616cd0b))
ROM_END

ROM_START(teletup)
	ROM_REGION(0x20000, "cit1600", 0) // external program ROM
	ROM_LOAD("3cq-1043-ae-1616d87-3d-16-11-00_27v101.cit1600", 0x00000, 0x20000, CRC(656c3ac5) SHA1(abcda259826d91a8f8c66a64066bd3da017a61af))

	ROM_REGION(0x6000, "cit1500", 0) // 24K internal mask ROM
	ROM_LOAD("citesa_ics1_7ce52872aa_0143h8l01", 0x0000, 0x6000, NO_DUMP)
	ROM_COPY("cit1600", 0x06000, 0x0000, 2) // hack to provide at least a reset vector

	ROM_REGION(0x1000, "cit1300", 0)
	ROM_LOAD("tue2a-xbaa_br24c32.cit1300", 0x0000, 0x1000, CRC(dd447884) SHA1(8ae677f46cd39015355a38a7f1147a7abc412674))
ROM_END

} // anonymous namespace


SYST(2001, trmavia, 0, 0, cit1500, cit1500, cit1500_state, empty_init, "Alcatel / Telefonica", "TRMA VIA", MACHINE_IS_SKELETON)
SYST(2000, teletup, 0, 0, cit1500, cit1500, cit1500_state, empty_init, "Alcatel / Telefonica", "TeleTUP", MACHINE_IS_SKELETON)
