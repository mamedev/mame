/***************************************************************************

        Intel iSBC series

        09/12/2009 Skeleton driver.

Notes:

isbc86 commands: BYTE WORD REAL EREAL ROMTEST. ROMTEST works, the others hang.

****************************************************************************/

#include "emu.h"
#include "cpu/i86/i86.h"
#include "cpu/i86/i286.h"
#include "machine/terminal.h"


class isbc_state : public driver_device
{
public:
	isbc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ16_MEMBER(isbc_terminal_status_r);
	DECLARE_READ16_MEMBER(isbc_terminal_r);
	DECLARE_WRITE8_MEMBER(kbd_put);
	UINT8 m_term_data;
	virtual void machine_reset();
};



READ16_MEMBER( isbc_state::isbc_terminal_status_r )
{
	return (m_term_data) ? 3 : 1;
}

READ16_MEMBER( isbc_state::isbc_terminal_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

static ADDRESS_MAP_START(rpc86_mem, AS_PROGRAM, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x0ffff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(rpc86_io, AS_IO, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc86_mem, AS_PROGRAM, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xfbfff) AM_RAM
	AM_RANGE(0xfc000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc86_io, AS_IO, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00d8, 0x00d9) AM_READ(isbc_terminal_r) AM_DEVWRITE8(TERMINAL_TAG, generic_terminal_device, write, 0xff)
	AM_RANGE(0x00da, 0x00db) AM_READ(isbc_terminal_status_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc286_mem, AS_PROGRAM, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xdffff) AM_RAM
	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc286_io, AS_IO, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc2861_mem, AS_PROGRAM, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0xeffff) AM_RAM
	AM_RANGE(0xf0000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(isbc2861_io, AS_IO, 16, isbc_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( isbc )
INPUT_PORTS_END


void isbc_state::machine_reset()
{
	m_term_data = 0;
}

WRITE8_MEMBER( isbc_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(isbc_state, kbd_put)
};

static MACHINE_CONFIG_START( isbc86, isbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(isbc86_mem)
	MCFG_CPU_IO_MAP(isbc86_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( rpc86, isbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I8086, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(rpc86_mem)
	MCFG_CPU_IO_MAP(rpc86_io)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

static const unsigned i286_address_mask = 0x00ffffff;

static MACHINE_CONFIG_START( isbc286, isbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(isbc286_mem)
	MCFG_CPU_IO_MAP(isbc286_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( isbc2861, isbc_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", I80286, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(isbc2861_mem)
	MCFG_CPU_IO_MAP(isbc2861_io)
	MCFG_CPU_CONFIG(i286_address_mask)

	/* video hardware */
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( isbc86 )
	ROM_REGION( 0x4000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "8612_2u.bin", 0x0001, 0x1000, CRC(84fa14cf) SHA1(783e1459ab121201fd49368d4bf769c1bab6447a))
	ROM_LOAD16_BYTE( "8612_2l.bin", 0x0000, 0x1000, CRC(922bda5f) SHA1(15743e69f3aba56425fa004d19b82ec20532fd72))
	ROM_LOAD16_BYTE( "8612_3u.bin", 0x2001, 0x1000, CRC(68d47c3e) SHA1(16c17f26b33daffa84d065ff7aefb581544176bd))
	ROM_LOAD16_BYTE( "8612_3l.bin", 0x2000, 0x1000, CRC(17f27ad2) SHA1(c3f379ac7d67dc4a0a7a611a0bc6323b8a3d4840))
ROM_END

ROM_START( isbc286 )
	ROM_REGION( 0x20000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "u79.bin", 0x00001, 0x10000, CRC(144182ea) SHA1(4620ca205a6ac98fe2636183eaead7c4bfaf7a72))
	ROM_LOAD16_BYTE( "u36.bin", 0x00000, 0x10000, CRC(22db075f) SHA1(fd29ea77f5fc0697c8f8b66aca549aad5b9db3ea))
	ROM_REGION( 0x4000, "isbc215", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "174581.001.bin", 0x0000, 0x2000, CRC(ccdbc7ab) SHA1(5c2ebdde1b0252124177221ba9cacdb6d925a24d))
	ROM_LOAD16_BYTE( "174581.002.bin", 0x0001, 0x2000, CRC(6190fa67) SHA1(295dd4e75f699aaf93227cc4876cee8accae383a))
ROM_END

ROM_START( isbc2861 )
	ROM_REGION( 0x10000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "174894-001.bin", 0x0000, 0x4000, CRC(79e4f7af) SHA1(911a4595d35e6e82b1149e75bb027927cd1c1658))
	ROM_LOAD16_BYTE( "174894-002.bin", 0x0001, 0x4000, CRC(66747d21) SHA1(4094b1f10a8bc7db8d6dd48d7128e14e875776c7))
	ROM_LOAD16_BYTE( "174894-003.bin", 0x8000, 0x4000, CRC(c98c7f17) SHA1(6e9a14aedd630824dccc5eb6052867e73b1d7db6))
	ROM_LOAD16_BYTE( "174894-004.bin", 0x8001, 0x4000, CRC(61bc1dc9) SHA1(feed5a5f0bb4630c8f6fa0d5cca30654a80b4ee5))
ROM_END

ROM_START( rpc86 )
	ROM_REGION( 0x4000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "145068-001.bin", 0x0001, 0x1000, CRC(0fa9db83) SHA1(4a44f8683c263c9ef6850cbe05aaa73f4d4d4e06))
	ROM_LOAD16_BYTE( "145069-001.bin", 0x2001, 0x1000, CRC(1692a076) SHA1(0ce3a4a867cb92340871bb8f9c3e91ce2984c77c))
	ROM_LOAD16_BYTE( "145070-001.bin", 0x0000, 0x1000, CRC(8c8303ef) SHA1(60f94daa76ab9dea6e309ac580152eb212b847a0))
	ROM_LOAD16_BYTE( "145071-001.bin", 0x2000, 0x1000, CRC(a49681d8) SHA1(e81f8b092cfa2d1737854b1fa270a4ce07d61a9f))
ROM_END
/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT COMPANY   FULLNAME       FLAGS */
COMP( 19??, rpc86,    0,       0,    rpc86,      isbc, driver_device,    0,   "Intel",   "RPC 86",GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 19??, isbc86,   0,       0,    isbc86,     isbc, driver_device,    0,   "Intel",   "iSBC 86/12A",GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 19??, isbc286,  0,       0,    isbc286,    isbc, driver_device,    0,   "Intel",   "iSBC 286",GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 19??, isbc2861, 0,       0,    isbc2861,   isbc, driver_device,    0,   "Intel",   "iSBC 286-10",GAME_NOT_WORKING | GAME_NO_SOUND)
