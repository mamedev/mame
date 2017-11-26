// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

        SWTPC 6800 Computer System

        10/12/2009 Skeleton driver.

        http://www.swtpc.com/mholley/swtpc_6800.htm

    MIKBUG is made for a PIA (parallel) interface.
    SWTBUG is made for a ACIA (serial) interface at the same address.
    MIKBUG will actually read the bits as they arrive and assemble a byte.
    Its delay loops are based on an underclocked XTAL.

    Note: All commands must be in uppercase. See the SWTBUG manual.

    ToDo:
        - Support selectable baud rate clocks (110, 150, 300, 600, 1200)
        - Implement interface bus slots to allow selection of either MP-S
          (ACIA) or MP-C or MP-L (PIA) for I/O slot #1 or seven other
          fixed-address I/O slots
        - Add DC-4 (WD1797 FDC) interface board as I/O slot option
        - Emulate MP-A2 revision of CPU board, with four 2716 ROM sockets
          and allowance for extra RAM boards at A000-BFFF and C000-DFFF


Commands:
B Breakpoint
C Clear screen
D Disk boot
E End of tape
F Find a byte
G Goto
J Jump
L Ascii Load
M Memory change (enter to quit, - to display next byte)
O Optional Port
P Ascii Punch
R Register dump
Z Goto Prom (0xC000)

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/mc14411.h"
#include "machine/ram.h"
#include "bus/rs232/rs232.h"

class swtpc_state : public driver_device
{
public:
	swtpc_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_ram(*this, "ram")
		, m_brg(*this, "brg")
	{ }

	virtual void machine_start() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<ram_device> m_ram;
	required_device<mc14411_device> m_brg;
};

static ADDRESS_MAP_START(mem_map, AS_PROGRAM, 8, swtpc_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x8004, 0x8004) AM_MIRROR(2) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0x8005, 0x8005) AM_MIRROR(2) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0xa000, 0xa07f) AM_RAM // MCM6810
	AM_RANGE(0xe000, 0xe3ff) AM_MIRROR(0x1c00) AM_ROM AM_REGION("mcm6830", 0)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( swtpc )
INPUT_PORTS_END

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_1200 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END


void swtpc_state::machine_start()
{
	m_brg->rsa_w(0);
	m_brg->rsb_w(1);

	m_maincpu->space(AS_PROGRAM).install_ram(0, m_ram->size() - 1, m_ram->pointer());
}

static MACHINE_CONFIG_START( swtpc )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6800, XTAL_1_8432MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mem_map)

	/* video hardware */
	MCFG_DEVICE_ADD("brg", MC14411, XTAL_1_8432MHz)
	MCFG_MC14411_F7_CB(DEVWRITELINE("acia", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("acia", acia6850_device, write_rxc))
	// F8, F9, F11, F13 also present on system bus

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("2K")
	MCFG_RAM_EXTRA_OPTIONS("4K,8K,12K,16K,20K,24K,28K,32K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( swtpcm, swtpc )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK(XTAL_1_7971MHz / 2)

	MCFG_DEVICE_MODIFY("brg")
	MCFG_DEVICE_CLOCK(XTAL_1_7971MHz)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( swtpc )
	ROM_REGION( 0x0400, "mcm6830", 0 )
	ROM_LOAD("swtbug.bin", 0x0000, 0x0400, CRC(f9130ef4) SHA1(089b2d2a56ce9526c3e78ce5d49ce368b9eabc0c))
ROM_END

ROM_START( swtpcm )
	ROM_REGION( 0x0400, "mcm6830", 0 )
	ROM_LOAD("mikbug.bin", 0x0000, 0x0400, CRC(e7f4d9d0) SHA1(5ad585218f9c9c70f38b3c74e3ed5dfe0357621c))
ROM_END

/* Driver */

//    YEAR  NAME    PARENT  COMPAT  MACHINE  INPUT  STATE        INIT  COMPANY                                     FULLNAME      FLAGS
COMP( 1977, swtpc,  0,      0,      swtpc,   swtpc, swtpc_state, 0,    "Southwest Technical Products Corporation", "SWTPC 6800 Computer System (with SWTBUG)", MACHINE_NO_SOUND_HW )
COMP( 1975, swtpcm, swtpc,  0,      swtpcm,  swtpc, swtpc_state, 0,    "Southwest Technical Products Corporation", "SWTPC 6800 Computer System (with MIKBUG)", MACHINE_NO_SOUND_HW | MACHINE_NOT_WORKING )
