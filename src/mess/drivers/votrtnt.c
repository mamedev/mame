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
*  - Reconnect the ACIA when serial coms become viable in MESS.
*
******************************************************************************/

/* Core includes */
#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "votrtnt.lh"

/* Components */
#include "sound/votrax.h"
#include "machine/6850acia.h"

/* For testing */
#include "machine/terminal.h"



class votrtnt_state : public driver_device
{
public:
	votrtnt_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG),
	//m_acia(*this, "acia_0"),
	m_votrax(*this, "votrax")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<generic_terminal_device> m_terminal;
	//required_device<> m_acia;
	required_device<votrax_sc01_device> m_votrax;
	DECLARE_READ8_MEMBER(votrtnt_acia_status_r);
	DECLARE_READ8_MEMBER(votrtnt_acia_data_r);
	DECLARE_WRITE8_MEMBER(votrtnt_kbd_put);
	UINT8 m_term_data;
	UINT8 m_term_status;
	virtual void machine_reset();
};

READ8_MEMBER( votrtnt_state::votrtnt_acia_status_r )
{
	return m_term_status;
}

READ8_MEMBER( votrtnt_state::votrtnt_acia_data_r )
{
	m_term_status = 0xe2;
	return m_term_data;
}


/******************************************************************************
 Address Maps
******************************************************************************/

static ADDRESS_MAP_START(6802_mem, AS_PROGRAM, 8, votrtnt_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0x6fff)
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_MIRROR(0xc00)/* RAM, 2114*2 (0x400 bytes) mirrored 4x */
	//AM_RANGE(0x2000, 0x2000) AM_NOP AM_MIRROR(0xffe)//AM_DEVREADWRITE("acia_0",aciastat_r,aciactrl_w)/* 6850 ACIA */
	//AM_RANGE(0x2001, 0x2001) AM_NOP AM_MIRROR(0xffe)//AM_DEVREADWRITE("acia_0",aciadata_r,aciadata_w)/* 6850 ACIA */
	AM_RANGE(0x2000, 0x2000) AM_MIRROR(0xffe) AM_READ(votrtnt_acia_status_r) AM_WRITENOP// temp testing
	AM_RANGE(0x2001, 0x2001) AM_MIRROR(0xffe) AM_READ(votrtnt_acia_data_r) AM_DEVWRITE(TERMINAL_TAG, generic_terminal_device, write) // temp testing
	AM_RANGE(0x4000, 0x5fff) AM_DEVWRITE("votrax", votrax_sc01_device, write) /* low 6 bits write to 6 bit input of sc-01-a; high 2 bits are ignored (but by adding a buffer chip could be made to control the inflection bits of the sc-01-a which are normally grounded on the tnt) */
	AM_RANGE(0x6000, 0x6fff) AM_ROM /* ROM in potted block */
ADDRESS_MAP_END


/******************************************************************************
 Input Ports
******************************************************************************/

static INPUT_PORTS_START(votrtnt)
	PORT_START("DSW1") /* not connected to cpu, each switch is connected directly to the output of a 4040 counter dividing the cpu m1? clock to feed the 6850 ACIA. Setting more than one switch on is a bad idea. see tnt_schematic.jpg */
	PORT_DIPNAME( 0xFF, 0x00, "Baud Rate" ) PORT_DIPLOCATION("SW1:1,2,3,4,5,6,7,8")
	PORT_DIPSETTING(    0x01, "75" )
	PORT_DIPSETTING(    0x02, "150" )
	PORT_DIPSETTING(    0x04, "300" )
	PORT_DIPSETTING(    0x08, "600" )
	PORT_DIPSETTING(    0x10, "1200" )
	PORT_DIPSETTING(    0x20, "2400" )
	PORT_DIPSETTING(    0x40, "4800" )
	PORT_DIPSETTING(    0x80, "9600" )
INPUT_PORTS_END

void votrtnt_state::machine_reset()
{
	m_term_data = 0;
	m_term_status = 0xe2;
}

WRITE8_MEMBER( votrtnt_state::votrtnt_kbd_put )
{
	m_term_data = data;
	m_term_status = 0xe3;
}

static GENERIC_TERMINAL_INTERFACE( votrtnt_terminal_intf )
{
	DEVCB_DRIVER_MEMBER(votrtnt_state, votrtnt_kbd_put)
};

static struct votrax_sc01_interface votrtnt_votrax_interface =
{
	DEVCB_CPU_INPUT_LINE("maincpu", INPUT_LINE_IRQ0)
};

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
	//MCFG_ACIA6850_ADD("acia_0", acia_intf)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_VOTRAX_SC01_ADD("votrax", 1700000, votrtnt_votrax_interface ) /* 1.70 MHz? needs verify */
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 1.00)

	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, votrtnt_terminal_intf)
MACHINE_CONFIG_END



/******************************************************************************
 ROM Definitions
******************************************************************************/

ROM_START(votrtnt)
	ROM_REGION(0x10000, "maincpu", 0)
	ROM_LOAD("cn49752n.bin", 0x6000, 0x1000, CRC(a44e1af3) SHA1(af83b9e84f44c126b24ee754a22e34ca992a8d3d)) /* 2332 mask rom inside potted brick */
ROM_END



/******************************************************************************
 Stuff that belongs in machine/votrtnt.c
******************************************************************************/
/*
static ACIA6850_INTERFACE( acia_intf )
{
    9600, // rx clock, actually based on dipswitches
    9600, // tx clock, actually based on dipswitches
    DEVCB_NULL, // rx callback
    DEVCB_NULL, // tx callback
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL,
    DEVCB_NULL
};*/

/******************************************************************************
 Drivers
******************************************************************************/

/*    YEAR  NAME        PARENT      COMPAT  MACHINE     INPUT   INIT      COMPANY                     FULLNAME                            FLAGS */
COMP( 1980, votrtnt,   0,          0,      votrtnt,   votrtnt, driver_device, 0,     "Votrax",        "Type 'N Talk", GAME_NOT_WORKING )
