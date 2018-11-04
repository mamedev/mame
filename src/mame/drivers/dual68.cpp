// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        Dual Systems 68000

        09/12/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "cpu/i8085/i8085.h"
//#include "bus/s100/s100.h"
#include "machine/i8251.h"
#include "machine/terminal.h"


class dual68_state : public driver_device
{
public:
	dual68_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_p_ram(*this, "ram")
	{ }

	void kbd_put(u8 data);
	DECLARE_WRITE16_MEMBER(terminal_w);

	void dual68(machine_config &config);
	void dual68_mem(address_map &map);
	void sio4_io(address_map &map);
	void sio4_mem(address_map &map);
private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint16_t> m_p_ram;
	//uint8_t m_term_data;
};



WRITE16_MEMBER( dual68_state::terminal_w )
{
	m_terminal->write(space, 0, data >> 8);
}

ADDRESS_MAP_START(dual68_state::dual68_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000000, 0x0000ffff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0x00080000, 0x00081fff) AM_ROM AM_REGION("user1",0)
	AM_RANGE(0x007f0000, 0x007f0001) AM_WRITE(terminal_w)
	AM_RANGE(0x00800000, 0x00801fff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

ADDRESS_MAP_START(dual68_state::sio4_mem)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x07ff) AM_ROM
	AM_RANGE(0x0800, 0xffff) AM_RAM
ADDRESS_MAP_END

ADDRESS_MAP_START(dual68_state::sio4_io)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x22, 0x22) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0x23, 0x23) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0x2a, 0x2a) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
	AM_RANGE(0x2b, 0x2b) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0x32, 0x32) AM_DEVREADWRITE("uart3", i8251_device, status_r, control_w)
	AM_RANGE(0x33, 0x33) AM_DEVREADWRITE("uart3", i8251_device, data_r, data_w)
	AM_RANGE(0x3a, 0x3a) AM_DEVREADWRITE("uart4", i8251_device, status_r, control_w)
	AM_RANGE(0x3b, 0x3b) AM_DEVREADWRITE("uart4", i8251_device, data_r, data_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dual68 )
INPUT_PORTS_END


void dual68_state::machine_reset()
{
	uint8_t* user1 = memregion("user1")->base();

	memcpy((uint8_t*)m_p_ram.target(),user1,0x2000);

	m_maincpu->reset();
}

void dual68_state::kbd_put(u8 data)
{
	//m_term_data = data;
}

MACHINE_CONFIG_START(dual68_state::dual68)
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL(16'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(dual68_mem)

	MCFG_CPU_ADD("siocpu", I8085A, XTAL(16'000'000) / 8)
	MCFG_CPU_PROGRAM_MAP(sio4_mem)
	MCFG_CPU_IO_MAP(sio4_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(dual68_state, kbd_put))

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	MCFG_DEVICE_ADD("uart2", I8251, 0)
	MCFG_DEVICE_ADD("uart3", I8251, 0)
	MCFG_DEVICE_ADD("uart4", I8251, 0)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dual68 )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "2 * 4KB" )
	ROMX_LOAD( "dual_cpu68000_1.bin", 0x0001, 0x1000, CRC(d1785c08) SHA1(73c1f68875f1d8eb5e92f4347f509c61103da90f),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD( "dual_cpu68000_2.bin", 0x0000, 0x1000, CRC(b9f1ba3c) SHA1(8fd02936ad06d5a22d435d96f06e2442fc7d00ec),ROM_SKIP(1) | ROM_BIOS(1))

	ROM_SYSTEM_BIOS( 1, "v2", "2 * 2KB" )
	ROMX_LOAD( "dual.u2.bin", 0x0001, 0x0800, CRC(e9c44fcd) SHA1(d5cc609d6f5e6745d5f0af1aa6dc66012333ed60),ROM_SKIP(1) | ROM_BIOS(2))
	ROMX_LOAD( "dual.u3.bin", 0x0000, 0x0800, CRC(827b049f) SHA1(8209f8ab3d1068e5bab51e7eb12be46d4ea28354),ROM_SKIP(1) | ROM_BIOS(2))

	ROM_REGION( 0x10000, "siocpu", ROMREGION_ERASEFF )
	ROM_LOAD( "dual_sio4.bin", 0x0000, 0x0800, CRC(6b0a1965) SHA1(5d2dc6c6a315293ded4b9fc95c8ac1599bf31dd3))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT   STATE         INIT  COMPANY                     FULLNAME              FLAGS
COMP( 1981, dual68,  0,     0,       dual68,    dual68, dual68_state, 0,    "Dual Systems Corporation", "Dual Systems 68000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
