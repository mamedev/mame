// license:BSD-3-Clause
// copyright-holders:Jonathan Gevaryahu
/******************************************************************************
*
*  Votrax Type 'N Talk Driver
*  By Jonathan Gevaryahu AKA Lord Nightmare
*  with loads of help (dumping, code disassembly, schematics, and other
*  documentation) from Kevin 'kevtris' Horton
*
*  The Votrax TNT was sold from some time in early 1981 until at least 1983.
*
*  2011-02-27 The TNT communicates with its host via serial (RS-232) by
*             using a 6850 ACIA. It will echo whatever is sent, back to the
*             host. In order that we can examine it, I have connected the
*             'terminal' so we can enter data, and see what gets sent back.
*             I've also connected the 'votrax' device, but it is far from
*             complete (the sounds are not understandable).
*
*             ACIA status code is set to E2 for no data and E3 for data.
*
*             On the CPU, A12 and A15 are not connected. A13 and A14 are used
*             to select devices. A0-A9 address RAM. A0-A11 address ROM.
*             A0 switches the ACIA between status/command, and data in/out.
*
*  Special codes: The Type 'N Talk will take notice of certain codes. They
*                 are: 08, 0D, 1B, 20. I didn't investigate what the codes do.
*
*  ToDo:
*  - Votrax device needs considerable improvement in sound quality.
*
*
******************************************************************************/

/* Core includes */
#include "bus/rs232/rs232.h"
#include "cpu/m6800/m6800.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "sound/votrax.h"
#include "votrtnt.lh"


class votrtnt_state : public driver_device
{
public:
	votrtnt_state(const machine_config &mconfig, device_type type, std::string tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_votrax(*this, "votrax"),
		m_acia(*this, "acia")
	{
	}

	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);

private:
	required_device<cpu_device> m_maincpu;
	required_device<votrax_sc01_device> m_votrax;
	required_device<acia6850_device> m_acia;
};


/******************************************************************************
 Address Maps
******************************************************************************/

/*  a15 a14 a13 a12   a11 a10  a9  a8    a7  a6  a5  a4    a3  a2  a1  a0
      x   0   0   x     x   x   *   *     *   *   *   *     *   *   *   *    RW  RAM (2x 2114 1kx4 SRAM, wired in parallel)
      x   0   1   x     x   x   x   x     x   x   x   x     x   x   x   0    RW  6850 Status(R)/Control(W)
      x   0   1   x     x   x   x   x     x   x   x   x     x   x   x   1    RW  6850 Data(R)/Data(W)
      x   1   0   x     x   x   x   x     x   x   x   x     x   x   x   x    W   SC-01 Data(W)
      x   1   1   x     *   *   *   *     *   *   *   *     *   *   *   *    R   ROM (2332 4kx8 Mask ROM, inside potted brick)
*/

static ADDRESS_MAP_START(6802_mem, AS_PROGRAM, 8, votrtnt_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000, 0x03ff) AM_MIRROR(0x9c00) AM_RAM /* RAM, 2114*2 (0x400 bytes) mirrored 4x */
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0x9ffe) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0x9ffe) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	AM_RANGE(0x4000, 0x4000) AM_MIRROR(0x9fff) AM_DEVWRITE("votrax", votrax_sc01_device, write) /* low 6 bits write to 6 bit input of sc-01-a; high 2 bits are ignored (but by adding a buffer chip could be made to control the inflection bits of the sc-01-a which are normally grounded on the TNT); upon any access to this area (even reads, which count effectively as a 'write of open bus value') the /STB line of the sc-01 is pulsed low using a 74123 monostable multivibrator with a capacitor of 120pf and a resistor to vcc of 22Kohm */
	AM_RANGE(0x6000, 0x6fff) AM_MIRROR(0x9000) AM_ROM /* ROM in potted block */
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/
/** TODO: actually hook this up to the ACIA */
static INPUT_PORTS_START(votrtnt)
	PORT_START("DSW1") /* not connected to cpu, each switch is connected directly to the output of a 4040 counter dividing the cpu m1? clock to feed the 6850 ACIA. Setting more than one switch on (downward is on, upward is off) is a bad idea, as it will short together outputs of the 4040, possibly damaging it. see tnt_schematic.jpg */
	PORT_DIPNAME( 0xFF, 0x80, "Baud Rate" ) PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x02, "150" )
	PORT_DIPSETTING(    0x04, "300" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPSETTING(    0x10, "1200" )
	PORT_DIPSETTING(    0x20, "2400" )
	PORT_DIPSETTING(    0x40, "4800" )
	PORT_DIPSETTING(    0x80, "9600" )
INPUT_PORTS_END


WRITE_LINE_MEMBER(votrtnt_state::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}


/******************************************************************************
 Machine Drivers
******************************************************************************/

static MACHINE_CONFIG_START( votrtnt, votrtnt_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6802, XTAL_2_4576MHz)  /* 2.4576MHz XTAL, verified; divided by 4 inside the m6802*/
	MCFG_CPU_PROGRAM_MAP(6802_mem)

	/* video hardware */
	//MCFG_DEFAULT_LAYOUT(layout_votrtnt)

	/* serial hardware */
	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(votrtnt_state, write_acia_clock))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_DEVICE_ADD("votrax", VOTRAX_SC01, 720000) /* 720kHz? needs verify */
	MCFG_VOTRAX_SC01_REQUEST_CB(INPUTLINE("maincpu", INPUT_LINE_IRQ0))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrtnt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cn49752n.bin", 0x6000, 0x1000, CRC(a44e1af3) SHA1(af83b9e84f44c126b24ee754a22e34ca992a8d3d)) /* 2332 mask rom inside potted brick */
ROM_END


/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME       PARENT      COMPAT  MACHINE     INPUT   CLASS         INIT      COMPANY    FULLNAME      FLAGS */
COMP( 1980, votrtnt,   0,          0,      votrtnt,   votrtnt, driver_device, 0,     "Votrax", "Type 'N Talk", MACHINE_NOT_WORKING )
