// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Imsai MPU-B. One of the earliest single-board computers on a S100 card.

    2013-09-11 Skeleton driver.

    Chips used: i8085, i8251, i8253, 3622 fusable prom. XTAL 6MHz

    Press any key to start the monitor program.

    ToDo:
    - Banking
    - Dipswitches

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
//#include "bus/s100/s100.h"
#include "machine/i8251.h"
#include "machine/pit8253.h"
#include "machine/terminal.h"


class imsai_state : public driver_device
{
public:
	imsai_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_terminal(*this, "terminal")
		, m_pit(*this, "pit")
	{ }

	void kbd_put(u8 data);
	DECLARE_READ8_MEMBER(keyin_r);
	DECLARE_READ8_MEMBER(status_r);
	DECLARE_WRITE8_MEMBER(control_w);

	void imsai(machine_config &config);
	void imsai_io(address_map &map);
	void imsai_mem(address_map &map);
private:
	uint8_t m_term_data;
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	required_device<pit8253_device> m_pit;
};


void imsai_state::imsai_mem(address_map &map)
{
	map.unmap_value_high();
	map(0x0000, 0x07ff).rom().region("prom", 0);
	map(0xd000, 0xd0ff).ram();
	map(0xd100, 0xd103).rw(m_pit, FUNC(pit8253_device::read), FUNC(pit8253_device::write));
	map(0xd800, 0xdfff).rom().region("prom", 0);
}

void imsai_state::imsai_io(address_map &map)
{
	map.unmap_value_high();
	map.global_mask(0xff);
	map(0x02, 0x02).r(this, FUNC(imsai_state::keyin_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x03, 0x03).r(this, FUNC(imsai_state::status_r));
	map(0x04, 0x04).rw("uart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x05, 0x05).rw("uart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x12, 0x12).rw("uart", FUNC(i8251_device::data_r), FUNC(i8251_device::data_w));
	map(0x13, 0x13).rw("uart", FUNC(i8251_device::status_r), FUNC(i8251_device::control_w));
	map(0x14, 0x14).r(this, FUNC(imsai_state::keyin_r)).w(m_terminal, FUNC(generic_terminal_device::write));
	map(0x15, 0x15).r(this, FUNC(imsai_state::status_r));
	map(0xf3, 0xf3).w(this, FUNC(imsai_state::control_w));
}

/* Input ports */
static INPUT_PORTS_START( imsai )
INPUT_PORTS_END

READ8_MEMBER( imsai_state::keyin_r )
{
	uint8_t ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( imsai_state::status_r )
{
	return (m_term_data) ? 3 : 1;
}

void imsai_state::kbd_put(u8 data)
{
	m_term_data = data;
}

WRITE8_MEMBER( imsai_state::control_w )
{
}

void imsai_state::machine_reset()
{
	m_term_data = 0;
}

MACHINE_CONFIG_START(imsai_state::imsai)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu",I8085A, XTAL(6'000'000))
	MCFG_DEVICE_PROGRAM_MAP(imsai_mem)
	MCFG_DEVICE_IO_MAP(imsai_io)

	/* video hardware */
	MCFG_DEVICE_ADD("terminal", GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(PUT(imsai_state, kbd_put))

	/* Devices */
	MCFG_DEVICE_ADD("uart", I8251, 0)

	MCFG_DEVICE_ADD("pit", PIT8253, 0)
	MCFG_PIT8253_CLK0(XTAL(6'000'000) / 3) /* Timer 0: baud rate gen for 8251 */
	MCFG_PIT8253_OUT0_HANDLER(WRITELINE("uart", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("uart", i8251_device, write_rxc))
	MCFG_PIT8253_CLK1(XTAL(6'000'000) / 3) /* Timer 1: user */
	MCFG_PIT8253_CLK2(XTAL(6'000'000) / 3) /* Timer 2: user */
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( imsai )
	ROM_REGION( 0x800, "prom", 0 ) // 2716 or 2708 program PROM
	ROM_LOAD( "vdb-80.rom",   0x0000, 0x0800, CRC(0afc4683) SHA1(a5419aaee00badf339d7c627f50ef8b2538e42e2) )

	ROM_REGION( 0x200, "decode", 0 ) // 512x4 address decoder ROM
	ROM_LOAD( "3622.u31", 0x000, 0x200, NO_DUMP )

	ROM_REGION( 0x20, "status", 0 ) // PROM for decoding 8085 status signals
	ROM_LOAD( "74s288.u38", 0x00, 0x20, NO_DUMP )
ROM_END

/* Driver */

//    YEAR  NAME   PARENT  COMPAT  MACHINE  INPUT  CLASS        INIT        COMPANY  FULLNAME  FLAGS
COMP( 1978, imsai, 0,      0,      imsai,   imsai, imsai_state, empty_init, "Imsai", "MPU-B",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
