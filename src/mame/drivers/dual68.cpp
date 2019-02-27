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
#include "machine/mc2661.h"
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

	void dual68(machine_config &config);

private:
	void kbd_put(u8 data);
	DECLARE_WRITE16_MEMBER(terminal_w);

	void dual68_mem(address_map &map);
	void sio4_io(address_map &map);
	void sio4_mem(address_map &map);
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_shared_ptr<uint16_t> m_p_ram;
	//uint8_t m_term_data;
};



WRITE16_MEMBER( dual68_state::terminal_w )
{
	m_terminal->write(data >> 8);
}

void dual68_state::dual68_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x00000000, 0x0000ffff).ram().share("ram");
	map(0x00080000, 0x00081fff).rom().region("user1", 0);
	map(0x007f0000, 0x007f0001).w(FUNC(dual68_state::terminal_w));
	map(0x00800000, 0x00801fff).rom().region("user1", 0);
}

void dual68_state::sio4_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom();
	map(0x0800, 0xffff).ram();
}

void dual68_state::sio4_io(address_map &map)
{
	map.unmap_value_high();
	map(0x20, 0x23).rw("usart1", FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0x28, 0x2b).rw("usart2", FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0x30, 0x33).rw("usart3", FUNC(mc2661_device::read), FUNC(mc2661_device::write));
	map(0x38, 0x3b).rw("usart4", FUNC(mc2661_device::read), FUNC(mc2661_device::write));
}

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
	MCFG_DEVICE_ADD("maincpu", M68000, 16_MHz_XTAL / 2) // MC68000L8
	MCFG_DEVICE_PROGRAM_MAP(dual68_mem)

	MCFG_DEVICE_ADD("siocpu", I8085A, 9.8304_MHz_XTAL) // NEC D8085AC-2
	MCFG_DEVICE_PROGRAM_MAP(sio4_mem)
	MCFG_DEVICE_IO_MAP(sio4_io)

	/* video hardware */
	GENERIC_TERMINAL(config, m_terminal, 0);
	m_terminal->set_keyboard_callback(FUNC(dual68_state::kbd_put));

	MCFG_DEVICE_ADD("usart1", MC2661, 9.8304_MHz_XTAL / 2) // SCN2661B
	MCFG_DEVICE_ADD("usart2", MC2661, 9.8304_MHz_XTAL / 2) // SCN2661B
	MCFG_DEVICE_ADD("usart3", MC2661, 9.8304_MHz_XTAL / 2) // SCN2661B
	MCFG_DEVICE_ADD("usart4", MC2661, 9.8304_MHz_XTAL / 2) // SCN2661B
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dual68 )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_SYSTEM_BIOS( 0, "v1", "2 * 4KB" )
	ROMX_LOAD("dual_cpu68000_1.bin", 0x0001, 0x1000, CRC(d1785c08) SHA1(73c1f68875f1d8eb5e92f4347f509c61103da90f),ROM_SKIP(1) | ROM_BIOS(0))
	ROMX_LOAD("dual_cpu68000_2.bin", 0x0000, 0x1000, CRC(b9f1ba3c) SHA1(8fd02936ad06d5a22d435d96f06e2442fc7d00ec),ROM_SKIP(1) | ROM_BIOS(0))

	ROM_SYSTEM_BIOS( 1, "v2", "2 * 2KB" )
	ROMX_LOAD("dual.u2.bin", 0x0001, 0x0800, CRC(e9c44fcd) SHA1(d5cc609d6f5e6745d5f0af1aa6dc66012333ed60),ROM_SKIP(1) | ROM_BIOS(1))
	ROMX_LOAD("dual.u3.bin", 0x0000, 0x0800, CRC(827b049f) SHA1(8209f8ab3d1068e5bab51e7eb12be46d4ea28354),ROM_SKIP(1) | ROM_BIOS(1))

	ROM_REGION( 0x10000, "siocpu", ROMREGION_ERASEFF )
	ROM_LOAD("dual_sio4.bin", 0x0000, 0x0800, CRC(6b0a1965) SHA1(5d2dc6c6a315293ded4b9fc95c8ac1599bf31dd3))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT   CLASS         INIT        COMPANY                     FULLNAME              FLAGS
COMP( 1981, dual68, 0,      0,      dual68,  dual68, dual68_state, empty_init, "Dual Systems Corporation", "Dual Systems 68000", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
