/***************************************************************************

    Poly/Proteus (New Zealand)

    10/07/2011 Skeleton driver.

    http://www.cs.otago.ac.nz/homepages/andrew/poly/Poly.htm

    Andrew has supplied the roms for -bios 1

    It uses a 6809 for all main functions. There is a Z80 for CP/M, but all
    of the roms are 6809 code.

    The keyboard controller is one of those custom XR devices.
    Will use the terminal keyboard instead.

    ToDo:
    - Almost Everything!
    - Connect up the device ports & lines
    - Find out about graphics mode and how it is selected
    - There is a beeper or speaker connected to the 6840 - how?
    - Fix Keyboard so that the Enter key tells BASIC to do something
    - Fix If ^G is pressed, emulation freezes
    - Find out how to make 2nd teletext screen to display

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "video/saa5050.h"
#include "machine/keyboard.h"


class poly_state : public driver_device
{
public:
	poly_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, "maincpu"),
		  m_pia0(*this, "pia0"),
		  m_pia1(*this, "pia1"),
		  m_videoram(*this, "videoram")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_shared_ptr<UINT8> m_videoram;
	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(pia1_b_in);
	DECLARE_READ_LINE_MEMBER(pia1_cb1_in);
	DECLARE_READ8_MEMBER(videoram_r);
	UINT8 m_term_data;
	bool m_term_key;
	virtual void machine_reset();
};


static ADDRESS_MAP_START(poly_mem, AS_PROGRAM, 8, poly_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x9fff) AM_RAM
	AM_RANGE(0xa000,0xcfff) AM_ROM
	AM_RANGE(0xd000,0xdfff) AM_RAM
	AM_RANGE(0xe000,0xe003) AM_DEVREADWRITE("pia0", pia6821_device, read, write) //video control PIA 6821
	// AM_RANGE(0xe004,0xe006) optional RS232C interface
	AM_RANGE(0xe00c,0xe00f) AM_DEVREADWRITE("pia1", pia6821_device, read, write) //keyboard PIA 6821
	AM_RANGE(0xe020,0xe027) AM_DEVREADWRITE("ptm", ptm6840_device, read, write) //timer 6840
	// AM_RANGE(0xe030,0xe037) Data Link Controller 6854
	AM_RANGE(0xe040,0xe040) AM_NOP //Set protect flip-flop after 1 E-cycle
	AM_RANGE(0xe050,0xe05f) AM_RAM //Dynamic Address Translater (arranges memory banks)
	// AM_RANGE(0xe060,0xe060) Select Map 1
	// AM_RANGE(0xe070,0xe070) Select Map 2
	AM_RANGE(0xe800,0xebbf) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xebc0,0xebff) AM_RAM
	AM_RANGE(0xec00,0xefbf) AM_RAM // screen 2 AM_DEVREADWRITE_LEGACY("saa5050", saa5050_videoram_r, saa5050_videoram_w)
	AM_RANGE(0xefc0,0xefff) AM_RAM
	AM_RANGE(0xf000,0xffff) AM_ROM
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( poly )
INPUT_PORTS_END


void poly_state::machine_reset()
{
}

static const pia6821_interface poly_pia0_intf=
{
	DEVCB_NULL,						/* port A input */
	DEVCB_NULL,	/* port B input */
	DEVCB_NULL, /* CA1 input */
	DEVCB_NULL, /* CB1 input */
	DEVCB_NULL,						/* CA2 input */
	DEVCB_NULL,						/* CB2 input */
	DEVCB_NULL,	/* port A output */
	DEVCB_NULL,	/* port B output */
	DEVCB_NULL, /* CA2 output */
	DEVCB_NULL, /* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE),	/* IRQA output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)		/* IRQB output */
};

READ8_MEMBER( poly_state::pia1_b_in )
{
// return ascii key value, bit 7 is the strobe value
	return m_term_data;
}

READ_LINE_MEMBER( poly_state::pia1_cb1_in )
{
// return kbd strobe value
	return 0;
}


static const pia6821_interface poly_pia1_intf=
{
	DEVCB_NULL,		/* port A input */
	DEVCB_DRIVER_MEMBER(poly_state, pia1_b_in),		/* port B input */
	DEVCB_NULL,		/* CA1 input */
	DEVCB_DRIVER_LINE_MEMBER(poly_state, pia1_cb1_in),		/* CB1 input */
	DEVCB_NULL,		/* CA2 input */
	DEVCB_NULL,		/* CB2 input */
	DEVCB_NULL,		/* port A output */
	DEVCB_NULL,		/* port B output */
	DEVCB_NULL,		/* CA2 output */
	DEVCB_NULL,		/* CB2 output */
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE),
	DEVCB_CPU_INPUT_LINE("maincpu", M6809_IRQ_LINE)
};

static const ptm6840_interface poly_ptm_intf =
{
	XTAL_10MHz/10, // not correct
	{ 0, 0, 0 },
	{ DEVCB_NULL,
	  DEVCB_NULL,
	  DEVCB_NULL },
	DEVCB_NULL
};

READ8_MEMBER( poly_state::videoram_r )
{
	return m_videoram[offset];
}

static SAA5050_INTERFACE( poly_saa5050_intf )
{
	DEVCB_DRIVER_MEMBER(poly_state, videoram_r),
	40, 24, 40  /* x, y, size */
};

// temporary hack
WRITE8_MEMBER( poly_state::kbd_put )
{
	m_term_data = data;
	//m_term_key = 1;
	address_space &mem = m_maincpu->space(AS_PROGRAM);
	mem.write_byte(0xebec, data); // this has to be 0xecf1 for bios 1
	mem.write_byte(0xebd0, 1); // any non-zero here
}

static ASCII_KEYBOARD_INTERFACE( keyboard_intf )
{
	DEVCB_DRIVER_MEMBER(poly_state, kbd_put)
};

static MACHINE_CONFIG_START( poly, poly_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_4MHz)
	MCFG_CPU_PROGRAM_MAP(poly_mem)


	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(40 * 12, 24 * 20)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 12 - 1, 0, 24 * 20 - 1)
	MCFG_SCREEN_UPDATE_DEVICE("saa5050", saa5050_device, screen_update)

	/* Devices */
	MCFG_SAA5050_ADD("saa5050", 6000000, poly_saa5050_intf)
	MCFG_PIA6821_ADD( "pia0", poly_pia0_intf )
	MCFG_PIA6821_ADD( "pia1", poly_pia1_intf )
	MCFG_PTM6840_ADD("ptm", poly_ptm_intf)

	// temporary hack
	MCFG_ASCII_KEYBOARD_ADD(KEYBOARD_TAG, keyboard_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( poly1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "BIOS0", "Standalone")
	ROMX_LOAD( "v3bas1.bin", 0xa000, 0x1000, CRC(2c5276cb) SHA1(897cb9c2456ddb0f316a8c3b8aa56706056cc1dd), ROM_BIOS(1) )
	// next 3 roms could be at the wrong location
	ROMX_LOAD( "v3bas2.bin", 0xb000, 0x1000, CRC(30f99447) SHA1(a26170113a968ccd8df7db1b0f256a2198054037), ROM_BIOS(1) )
	ROMX_LOAD( "v3bas3.bin", 0xc000, 0x1000, CRC(89ea5b27) SHA1(e37a61d3dd78fb40bc43c70af9714ce3f75fd895), ROM_BIOS(1) )
	ROMX_LOAD( "v3bas4.bin", 0x9000, 0x1000, CRC(c16c1209) SHA1(42f3b0bce32aafab14bc0500fb13bd456730875c), ROM_BIOS(1) )
	// boot rom
	ROMX_LOAD( "plrt16v3e9.bin", 0xf000, 0x1000, CRC(453c10a0) SHA1(edfbc3d83710539c01093e89fe1b47dfe1e68acd), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(1, "BIOS1", "Terminal")
	// supplied by Andrew Trotman, author of Poly1 emulator (PolyROM v3.4)
	ROMX_LOAD( "v2bas1.bin", 0xa000, 0x1000, CRC(f8c5adc4) SHA1(b1a16d7d996909185495b15a52afa697324e1f8d), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas2.bin", 0xb000, 0x1000, CRC(a2b0fa4d) SHA1(05ab723eb2e2b09325380a1a72da5ade401847d1), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas3.bin", 0xc000, 0x1000, CRC(04a58be5) SHA1(729fa02c76783213e40bb179e60c09d4b439ab90), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas4.bin", 0x9000, 0x1000, CRC(328fe790) SHA1(43dca92092b27627603d3588f91cf9eca24ed29f), ROM_BIOS(2) )
	ROMX_LOAD( "slrt14_00f9.bin", 0xf000, 0x1000, CRC(6559a2ce) SHA1(7c38f449ca122342732123b56992ed0c446406c2), ROM_BIOS(2) )
ROM_END

/* Driver */

/*    YEAR   NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1981,  poly1,  0,      0,       poly,      poly, driver_device,    0,      "Polycorp",  "Poly-1 Educational Computer", GAME_NOT_WORKING | GAME_NO_SOUND )
