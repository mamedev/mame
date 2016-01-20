// license:BSD-3-Clause
// copyright-holders: Miodrag Milanovic, Robbbert
/***************************************************************************

        Systec Z80

        More data :
            http://www.hd64180-cpm.de/html/systecz80.html

        30/08/2010 Skeleton driver

        Systec Platine 1

        SYSTEC 155.1L
        EPROM 2764 CP/M LOADER 155 / 9600 Baud
        Z8400APS CPU
        Z8420APS PIO
        Z8430APS CTC
        Z8470APS DART

        Systec Platine 2

        SYSTEC 100.3B
        MB8877A FDC Controller
        FDC9229BT SMC 8608
        Z8410APS DMA
        Z8420APS PIO

        MB8877A Compatible FD1793

        2011-12-22 connected to a terminal [Robbbert]


****************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/terminal.h"

#define TERMINAL_TAG "terminal"

class systec_state : public driver_device
{
public:
	systec_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_terminal(*this, TERMINAL_TAG)
	{
	}

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	DECLARE_READ8_MEMBER(systec_c4_r);
	DECLARE_READ8_MEMBER(systec_c6_r);
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 m_term_data;
	virtual void machine_reset() override;
};

READ8_MEMBER( systec_state::systec_c4_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ8_MEMBER( systec_state::systec_c6_r )
{
	return 0x04 | (m_term_data ? 1 : 0);
}

WRITE8_MEMBER( systec_state::kbd_put )
{
	m_term_data = data;
}

static ADDRESS_MAP_START(systec_mem, AS_PROGRAM, 8, systec_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0xffff) AM_RAM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(systec_io, AS_IO, 8, systec_state)
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xc4, 0xc4) AM_READ(systec_c4_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write)
	AM_RANGE(0xc6, 0xc6) AM_READ(systec_c6_r)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( systec )
INPUT_PORTS_END

void systec_state::machine_reset()
{
	UINT8 *m_p_maincpu = memregion("maincpu")->base();
	UINT8 *m_p_roms = memregion("roms")->base();
	memcpy(m_p_maincpu, m_p_roms, 0x2000);
}

static MACHINE_CONFIG_START( systec, systec_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_16MHz / 4)
	MCFG_CPU_PROGRAM_MAP(systec_mem)
	MCFG_CPU_IO_MAP(systec_io)


	/* video hardware */
	MCFG_DEVICE_ADD(TERMINAL_TAG, GENERIC_TERMINAL, 0)
	MCFG_GENERIC_TERMINAL_KEYBOARD_CB(WRITE8(systec_state, kbd_put))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( systec )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_REGION( 0x10000, "roms", 0 )
	ROM_LOAD( "systec.bin",   0x0000, 0x2000, CRC(967108ab) SHA1(a414db032ca7db0f9fdbe22aa68a099a93efb593))
ROM_END

/* Driver */

/*   YEAR  NAME    PARENT  COMPAT   MACHINE  INPUT    INIT   COMPANY    FULLNAME       FLAGS */
COMP(19??, systec, 0,      0,       systec,  systec, driver_device,  0,    "Systec", "Systec Z80", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
