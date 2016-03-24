// license:BSD-3-Clause
// copyright-holders:Robbbert
/***************************************************************************

    Poly/Proteus (New Zealand)

    10/07/2011 Skeleton driver.

    http://www.cs.otago.ac.nz/homepages/andrew/poly/Poly.htm

    Andrew has supplied the roms for -bios 1

    It uses a 6809 for all main functions. There is a Z80 for CP/M, but all
    of the roms are 6809 code.

    The keyboard controller is one of those custom XR devices.
    Will use the terminal keyboard instead.

    With bios 1, after entering your userid and password, you get a black
    screen. This is normal, because it joins to a network which isn't there.

    ToDo:
    - Almost Everything!
    - Connect up the device ports & lines
    - Find out about graphics mode and how it is selected
    - Fix Keyboard so that the Enter key tells BASIC to do something
    - Find out how to make 2nd teletext screen to display
    - Banking

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/z80/z80.h"
#include "machine/6821pia.h"
#include "machine/6840ptm.h"
#include "machine/6850acia.h"
#include "machine/clock.h"
#include "machine/mc6854.h"
#include "video/saa5050.h"
#include "machine/keyboard.h"
#include "sound/speaker.h"

#define KEYBOARD_TAG "keyboard"

class poly_state : public driver_device
{
public:
	poly_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_pia0(*this, "pia0"),
		m_pia1(*this, "pia1"),
		m_acia(*this, "acia"),
		m_ptm(*this, "ptm"),
		m_speaker(*this, "speaker"),
		m_videoram(*this, "videoram")
	{
	}

	DECLARE_WRITE8_MEMBER(kbd_put);
	DECLARE_READ8_MEMBER(pia1_b_in);
	DECLARE_READ8_MEMBER(videoram_r);
	DECLARE_WRITE_LINE_MEMBER(write_acia_clock);
	DECLARE_WRITE8_MEMBER( ptm_o2_callback );
	DECLARE_WRITE8_MEMBER( ptm_o3_callback );

protected:
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<pia6821_device> m_pia0;
	required_device<pia6821_device> m_pia1;
	required_device<acia6850_device> m_acia;
	required_device<ptm6840_device> m_ptm;
	required_device<speaker_sound_device> m_speaker;
	required_shared_ptr<UINT8> m_videoram;
	UINT8 m_term_data;
};


static ADDRESS_MAP_START(poly_mem, AS_PROGRAM, 8, poly_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0x9fff) AM_RAM
	AM_RANGE(0xa000,0xcfff) AM_ROM
	AM_RANGE(0xd000,0xdfff) AM_RAM
	AM_RANGE(0xe000,0xe003) AM_DEVREADWRITE("pia0", pia6821_device, read, write) //video control PIA 6821
	AM_RANGE(0xe004,0xe004) AM_DEVREADWRITE("acia", acia6850_device, status_r, control_w)
	AM_RANGE(0xe005,0xe005) AM_DEVREADWRITE("acia", acia6850_device, data_r, data_w)
	//AM_RANGE(0xe006, 0xe006) // baud rate controller (0=9600,2=4800,4=2400,6=1200,8=600,A=300)
	AM_RANGE(0xe00c,0xe00f) AM_DEVREADWRITE("pia1", pia6821_device, read, write) //keyboard PIA 6821
	AM_RANGE(0xe020,0xe027) AM_DEVREADWRITE("ptm", ptm6840_device, read, write) //timer 6840
	AM_RANGE(0xe030,0xe037) AM_DEVREADWRITE("adlc", mc6854_device, read, write) //Data Link Controller 6854
	AM_RANGE(0xe040,0xe040) AM_NOP //Set protect flip-flop after 1 E-cycle
	AM_RANGE(0xe050,0xe05f) AM_RAM //Dynamic Address Translater (arranges memory banks)
	// AM_RANGE(0xe060,0xe060) Select Map 1
	// AM_RANGE(0xe070,0xe070) Select Map 2
	AM_RANGE(0xe800,0xebbf) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0xebc0,0xebff) AM_RAM
	AM_RANGE(0xec00,0xefbf) AM_RAM // screen 2 AM_SHARE("videoram")
	AM_RANGE(0xefc0,0xefff) AM_RAM
	AM_RANGE(0xf000,0xffff) AM_ROM
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( poly )
INPUT_PORTS_END


void poly_state::machine_reset()
{
}

READ8_MEMBER( poly_state::pia1_b_in )
{
// return ascii key value, bit 7 is the strobe value
	UINT8 data = m_term_data;
	m_term_data &= 0x7f;
	return data;
}

READ8_MEMBER( poly_state::videoram_r )
{
	return m_videoram[offset];
}

WRITE8_MEMBER( poly_state::kbd_put )
{
	m_term_data = data | 0x80;

	m_pia1->cb1_w(1);
	m_pia1->cb1_w(0);
}

WRITE_LINE_MEMBER( poly_state::write_acia_clock )
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

WRITE8_MEMBER( poly_state::ptm_o2_callback )
{
	m_ptm->set_c1(data);
}

WRITE8_MEMBER( poly_state::ptm_o3_callback )
{
	m_speaker->level_w(data);
}

static MACHINE_CONFIG_START( poly, poly_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809E, XTAL_12MHz / 3) // 12.0576MHz
	MCFG_CPU_PROGRAM_MAP(poly_mem)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_SIZE(40 * 12, 24 * 20)
	MCFG_SCREEN_VISIBLE_AREA(0, 40 * 12 - 1, 0, 24 * 20 - 1)
	MCFG_SCREEN_UPDATE_DEVICE("saa5050", saa5050_device, screen_update)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("speaker", SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	/* Devices */
	MCFG_DEVICE_ADD("saa5050", SAA5050, 6000000)
	MCFG_SAA5050_D_CALLBACK(READ8(poly_state, videoram_r))
	MCFG_SAA5050_SCREEN_SIZE(40, 24, 40)

	MCFG_DEVICE_ADD("pia0", PIA6821, 0)
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_DEVICE_ADD("pia1", PIA6821, 0)
	MCFG_PIA_READPB_HANDLER(READ8(poly_state, pia1_b_in))
	// CB1 kbd strobe
	MCFG_PIA_IRQA_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))
	MCFG_PIA_IRQB_HANDLER(DEVWRITELINE("maincpu", m6809e_device, irq_line))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(XTAL_12MHz / 3)
	MCFG_PTM6840_EXTERNAL_CLOCKS(0, 0, 0)
	MCFG_PTM6840_OUT1_CB(WRITE8(poly_state, ptm_o2_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(poly_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(INPUTLINE("maincpu", M6809_IRQ_LINE))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	//MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	//MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))

	//MCFG_DEVICE_ADD("acia_clock", CLOCK, 1)
	//MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(poly_state, write_acia_clock))

	MCFG_DEVICE_ADD("adlc", MC6854, 0)

	MCFG_DEVICE_ADD(KEYBOARD_TAG, GENERIC_KEYBOARD, 0)
	MCFG_GENERIC_KEYBOARD_CB(WRITE8(poly_state, kbd_put))
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( poly1 )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_SYSTEM_BIOS(0, "bios0", "Standalone")
	ROMX_LOAD( "v3bas1.bin", 0xa000, 0x1000, CRC(2c5276cb) SHA1(897cb9c2456ddb0f316a8c3b8aa56706056cc1dd), ROM_BIOS(1) )
	ROMX_LOAD( "v3bas2.bin", 0xb000, 0x1000, CRC(30f99447) SHA1(a26170113a968ccd8df7db1b0f256a2198054037), ROM_BIOS(1) )
	ROMX_LOAD( "v3bas3.bin", 0xc000, 0x1000, CRC(89ea5b27) SHA1(e37a61d3dd78fb40bc43c70af9714ce3f75fd895), ROM_BIOS(1) )
	ROMX_LOAD( "v3bas4.bin", 0x9000, 0x1000, CRC(c16c1209) SHA1(42f3b0bce32aafab14bc0500fb13bd456730875c), ROM_BIOS(1) )
	// boot rom
	ROMX_LOAD( "plrt16v3e9.bin", 0xf000, 0x1000, CRC(453c10a0) SHA1(edfbc3d83710539c01093e89fe1b47dfe1e68acd), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS(1, "bios1", "Terminal")
	// supplied by Andrew Trotman, author of Poly1 emulator (PolyROM v3.4)
	ROMX_LOAD( "v2bas1.bin", 0xa000, 0x1000, CRC(f8c5adc4) SHA1(b1a16d7d996909185495b15a52afa697324e1f8d), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas2.bin", 0xb000, 0x1000, CRC(a2b0fa4d) SHA1(05ab723eb2e2b09325380a1a72da5ade401847d1), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas3.bin", 0xc000, 0x1000, CRC(04a58be5) SHA1(729fa02c76783213e40bb179e60c09d4b439ab90), ROM_BIOS(2) )
	ROMX_LOAD( "v2bas4.bin", 0x9000, 0x1000, CRC(328fe790) SHA1(43dca92092b27627603d3588f91cf9eca24ed29f), ROM_BIOS(2) )
	ROMX_LOAD( "slrt14_00f9.bin", 0xf000, 0x1000, CRC(6559a2ce) SHA1(7c38f449ca122342732123b56992ed0c446406c2), ROM_BIOS(2) )
ROM_END

/* Driver */

/*    YEAR   NAME    PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY     FULLNAME       FLAGS */
COMP( 1981,  poly1,  0,      0,       poly,      poly, driver_device,    0,   "Polycorp", "Poly-1 Educational Computer", MACHINE_NOT_WORKING )
