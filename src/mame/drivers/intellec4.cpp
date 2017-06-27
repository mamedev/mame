// license:BSD-3-Clause
// copyright-holders:Vas Crabb
/*
 Intel INTELLEC® 4/MOD 40

 Set the terminal for 110 1/8/N/2 to talk to the monitor.
 It only likes to see uppercase letters, digits, comman and carriage return.
 */
#include "emu.h"

#include "bus/rs232/rs232.h"
#include "cpu/mcs40/mcs40.h"


namespace {

class intellec4_40_state : public driver_device
{
public:
	intellec4_40_state(machine_config const &mconfig, device_type type, char const *tag)
		: driver_device(mconfig, type, tag)
		, m_tty(*this, "tty")
	{
	}

	DECLARE_READ8_MEMBER(tty_r);
	DECLARE_WRITE8_MEMBER(tty_w);

private:
	required_device<rs232_port_device> m_tty;
};


READ8_MEMBER(intellec4_40_state::tty_r)
{
	return m_tty->rxd_r() ? 0x0 : 0x1;
}

WRITE8_MEMBER(intellec4_40_state::tty_w)
{
	m_tty->write_txd(BIT(data, 0));
}


ADDRESS_MAP_START(mod40_program, AS_PROGRAM, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_ROM AM_REGION("monitor", 0x0000) // 4289 + 4 * 1702A
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_data, AS_DATA, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x00ff) AM_RAM // 4 * 4002
ADDRESS_MAP_END

ADDRESS_MAP_START(mod40_io, AS_IO, 8, intellec4_40_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x000f) AM_MIRROR(0x0700) AM_READ(tty_r) // RDR when RC=00
	AM_RANGE(0x1000, 0x103f) AM_MIRROR(0x0800) AM_WRITE(tty_w) // WMP when RC=00
ADDRESS_MAP_END


MACHINE_CONFIG_START(intlc440)
	MCFG_CPU_ADD("maincpu", I4040, 5185000./7)
	MCFG_CPU_PROGRAM_MAP(mod40_program)
	MCFG_CPU_DATA_MAP(mod40_data)
	MCFG_CPU_IO_MAP(mod40_io)

	MCFG_RS232_PORT_ADD("tty", default_rs232_devices, "terminal")
MACHINE_CONFIG_END


INPUT_PORTS_START(intlc440)
INPUT_PORTS_END


ROM_START(intlc440)
	ROM_REGION(0x0400, "monitor", 0)
	ROM_DEFAULT_BIOS("v2.1")
	ROM_SYSTEM_BIOS(0, "v2.1", "MON 4 V2.1")
	ROMX_LOAD("mon_4-000-v_2.1.a1",      0x0000, 0x0100, CRC(8d1f56ff) SHA1(96bc19be9be4e92195fad82d7a3cadb763ab6e3f), ROM_BIOS(1))
	ROMX_LOAD("mon_4-100-v_2.1.a2",      0x0100, 0x0100, CRC(66562a4f) SHA1(040749c45e95dfc39b3397d0c31c8b4c11f0a5fc), ROM_BIOS(1))
	ROMX_LOAD("mon_4-200-v_2.1.a3",      0x0200, 0x0100, CRC(fe039c68) SHA1(1801cfcc7514412865c0fdc7d1800fcf583a2d2a), ROM_BIOS(1))
	ROMX_LOAD("mon_4-300-v_2.1.a4",      0x0300, 0x0100, CRC(3724d5af) SHA1(b764b3bb3541fbda875f7a7655f46aa54b332631), ROM_BIOS(1))
	ROM_SYSTEM_BIOS(1, "v2.1_1200", "MON 4 V2.1 1200 Baud hack")
	ROMX_LOAD("mon_4-000-v_2.1.a1",      0x0000, 0x0100, CRC(8d1f56ff) SHA1(96bc19be9be4e92195fad82d7a3cadb763ab6e3f), ROM_BIOS(2))
	ROMX_LOAD("i40_mon-1.a2",            0x0100, 0x0100, CRC(cd9fecd6) SHA1(9c4fb85118c881687fd4b324e5089df05d1e63d1), ROM_BIOS(2))
	ROMX_LOAD("i40_mon-2.a3",            0x0200, 0x0100, CRC(037de128) SHA1(3694636e1f4e23688b36ea9ee755a0c5888f4328), ROM_BIOS(2))
	ROMX_LOAD("1200_baud-i40_mon-f3.a4", 0x0300, 0x0100, CRC(f3198d79) SHA1(b7903073b69f487b6f78842c08694f12225d85f0), ROM_BIOS(2))
ROM_END

} // anonymous namespace

//    YEAR   NAME      PARENT  COMPAT  MACHINE   INPUT     STATE               INIT  COMPANY  FULLNAME            FLAGS
COMP( 1974?, intlc440, 0,      0,      intlc440, intlc440, intellec4_40_state, 0,    "Intel", "INTELLEC 4/MOD40", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW | MACHINE_SUPPORTS_SAVE )
