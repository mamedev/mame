// license:GPL-2.0+
// copyright-holders:Felipe Sanches
/***************************************************************************

  Argox Rabbit Printer
  model: 0S-214

  Skeleton driver by Felipe Correa da Silva Sanches <juca@members.fsf.org>

  OVERALL HARDWARE DESCRIPTION:

  There's a sticker labeled "V4.21"
  The board is labeled "ARGOX INFORMATION 48.20401.002 DATE:2003/03/20 REV:4.2"

  There's a soldered IC at U10 which is labeled "A511 093060006 55-20401-003 E" and "OK"
  I guess it may be another flash ROM

  External interfaces:
  * RS232 serial interface
  * Centronics port

  Connectors:
  * 4-pin labeled "PEELER" (unused)
  * 4-pin labeled "RIBBON"
  * 4-pin labeled "MEDIA"
  * 4-pin labeled "MOTOR"
  * 6-pin labeled "MOTOR" (unpopulated)
  * 4-pin labeled "CUTTER" (unused)
  * 18-pin unlabeled JP18 (with 4 unused pins) Connects to the printing subassembly
  * 9-pin labeled "KEYPAD" (unpopulated)
  * 6-pin labeled "LED/KEY" (connects to FEED button, POWER LED and READY LED)

  Jumpers:
  * set of 2 jumpers (JP1 and JP2) with a jumper inserted at JP2
  * set of 6 unlabelled with jumpers inserted at position #0 and #3

  DIP sockets:
  * u8 and u9 hold the FLASH ROM chips
  * U19 is an unpopulated DIP16 socket

***************************************************************************/

#include "emu.h"
#include "cpu/h8/h83002.h"


namespace {

class os214_state : public driver_device
{
public:
	os214_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu")
	{ }

	void os214(machine_config &config);

	void init_os214();

private:
	required_device<cpu_device> m_maincpu;
	void os214_prg_map(address_map &map) ATTR_COLD;
};

void os214_state::os214_prg_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();
}

void os214_state::os214(machine_config &config)
{
	/* basic machine hardware */
	H83002(config, m_maincpu, XTAL(16'000'000)); /* X1 xtal value is correct,
	                                                   but there can be some clock divider perhaps ? */
	m_maincpu->set_addrmap(AS_PROGRAM, &os214_state::os214_prg_map);
}

void os214_state::init_os214()
{
}

ROM_START( os214 )
	ROM_REGION( 0x080000, "maincpu", 0 )
	ROM_LOAD16_BYTE( "u9_s2a2-4.03_argox_am.u9", 0x000000, 0x040000, CRC(3bd8b2b1) SHA1(546f9fd8d7e1f589f6e594a332a3429041b49eea) )
	ROM_LOAD16_BYTE( "u8_s2a2-4.03_argox_am.u8", 0x000001, 0x040000, CRC(d49f52af) SHA1(0ca5a70c6c3995f275226af26db965f6ba7ed123) )
ROM_END

} // anonymous namespace


//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME                         FLAGS
COMP( 1996, os214, 0,      0,      os214,   0,     os214_state, init_os214, "Argox", "Rabbit Printer (model OS-214)", MACHINE_IS_SKELETON)
