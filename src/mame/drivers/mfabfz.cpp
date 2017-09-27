// license:BSD-3-Clause
// copyright-holders:rfka01, Robbbert
/***************************************************************************

Mikrocomputer fuer Ausbildung
Berufsfoerdungszentrum Essen
Information found on Wikipedia:
- System is a backbone upon which all functions are available on plug-in cards
- 32k RAM, 32k ROM
- Serial is via CPU SID/SOD pins, but can be replaced by 8251 on RS232 card
- Protocol is COM1, 4800, 8N1
- Timer card uses 8253
- PIO card uses 8255 and has a printer interface
- Cassette interface uses RS232 card
- Optional floppy (both sizes); and a EPROM burner
- OS: MAT85

Manuals have no schematics and no mention of the 8253 or 8255. The bios
doesn't try communicating with them either.

Commands:
A     Assembler
B     Set Breakpoint
D     Disassembler
G     Go
H     Help
I     Inport
M     Print/Modify memory (A=ascii, B=bit, H=hex)
N     Turn on tracer & step to next instruction
O     Outport
P     Display memory contents in various formats
R     Set initial register contents
T     Trace interval

Pressing enter will change the prompt from KMD > to KMD+> and pressing
space will change it back.

BIOS 0 uses the UART and works
BIOS 1,2 use SID/SOD and don't work

****************************************************************************/

#include "emu.h"
#include "cpu/i8085/i8085.h"
#include "machine/clock.h"
#include "machine/i8251.h"
#include "bus/rs232/rs232.h"


class mfabfz_state : public driver_device
{
public:
	mfabfz_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_rs232(*this, "rs232")
	{ }

	DECLARE_WRITE_LINE_MEMBER(sod_w);
	DECLARE_READ_LINE_MEMBER(sid_r);

private:
	virtual void machine_reset() override;
	required_device<cpu_device> m_maincpu;
	required_device<rs232_port_device> m_rs232;
};


WRITE_LINE_MEMBER( mfabfz_state::sod_w )
{
	m_rs232->write_txd(state);
}

READ_LINE_MEMBER( mfabfz_state::sid_r )
{
	return m_rs232->rxd_r();
}


static ADDRESS_MAP_START(mfabfz_mem, AS_PROGRAM, 8, mfabfz_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("roms", 0)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START(mfabfz_io, AS_IO, 8, mfabfz_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0xbe, 0xbe) AM_DEVREADWRITE("uart1", i8251_device, data_r, data_w)
	AM_RANGE(0xbf, 0xbf) AM_DEVREADWRITE("uart1", i8251_device, status_r, control_w)
	AM_RANGE(0xfe, 0xfe) AM_DEVREADWRITE("uart2", i8251_device, data_r, data_w)
	AM_RANGE(0xff, 0xff) AM_DEVREADWRITE("uart2", i8251_device, status_r, control_w)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( mfabfz )
INPUT_PORTS_END


void mfabfz_state::machine_reset()
{
}


static MACHINE_CONFIG_START( mfabfz )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8085A, XTAL_4MHz / 2)
	MCFG_CPU_PROGRAM_MAP(mfabfz_mem)
	MCFG_CPU_IO_MAP(mfabfz_io)
	MCFG_I8085A_SID(READLINE(mfabfz_state, sid_r))
	MCFG_I8085A_SOD(WRITELINE(mfabfz_state, sod_w))
	/* video hardware */

	// uart1 - terminal - clock hardware unknown
	MCFG_DEVICE_ADD("uart1_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(DEVWRITELINE("uart1", i8251_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(DEVWRITELINE("uart1", i8251_device, write_rxc))

	MCFG_DEVICE_ADD("uart1", I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("uart1", i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE("uart1", i8251_device, write_dsr))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("uart1", i8251_device, write_cts))

	// uart2 - cassette - clock comes from 2MHz through a divider consisting of 4 chips and some jumpers.
	MCFG_DEVICE_ADD("uart2", I8251, 0)
MACHINE_CONFIG_END

/* ROM definition */

ROM_START( mfabfz )
	ROM_REGION( 0x8000, "roms", 0 )
	ROM_SYSTEM_BIOS( 0, "v18t", "V1.8T" ) // 1986, works
	ROMX_LOAD( "mfa_mat32k_vers.1.8-t_ic0.bin", 0x0000, 0x8000, CRC(6cba989e) SHA1(81611b6250a5319e5d28af5ce3a1e261af8315ae), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v18", "V1.8" ) // 1982, not working
	ROMX_LOAD( "mfa_mat32k_vers.1.8-s_ic0.bin", 0x0000, 0x8000, CRC(021d7dff) SHA1(aa34b3a8bac52fc7746d35f5ffc6328734788cc2), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v18a", "V1.8(small roms)" ) // 1982, not working
	ROMX_LOAD( "mfa_mat_1_0000.bin", 0x0000, 0x0800, CRC(73b588ea) SHA1(2b9570fe44c3c19d6aa7c7c11ecf390fa5d48998), ROM_BIOS(3) )
	ROMX_LOAD( "mfa_mat_2_0800.bin", 0x0800, 0x0800, CRC(13f5be91) SHA1(2b9d64600679bab319a37381fc84e874c3b2a877), ROM_BIOS(3) )
	ROMX_LOAD( "mfa_mat_3_1000.bin", 0x1000, 0x0800, CRC(c9b91bb4) SHA1(ef829964f507b1f6bbcf3c557c274fe728636efe), ROM_BIOS(3) )
	ROMX_LOAD( "mfa_mat_4_1800.bin", 0x1800, 0x0800, CRC(649cd7f0) SHA1(e92f29c053234b36f22d525fe92e61bf24476f14), ROM_BIOS(3) )
ROM_END


/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT     CLASS,        INIT   COMPANY                                FULLNAME                       FLAGS */
COMP( 1979, mfabfz, 0,      0,       mfabfz,    mfabfz,   mfabfz_state,   0, "Berufsfoerdungszentrum Essen", "Mikrocomputer fuer Ausbildung", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
