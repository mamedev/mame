// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic, Jean-François DEL NERO
/***************************************************************************

    Apollo 7 Squale

    The following hardware description is an extract of the Squale hardware analysis
    presented on this page : http://hxc2001.free.fr/Squale.
    This is a work in progress and is subject to changes

    PCB Ref Qty Manufacturer Ref    Description / Datasheet
    ============================================================================================
    U1              1   EF6809P             8-BIT MICROPROCESSOR UNIT (MPU)
    U2              1   27C32 / 27C64       EPROM
    U72,U75         1   EF6821P             PERIPHERAL INTERFACE ADAPTER (PIA)
    U69             1   EF6850              ASYNCHRONOUS COMMUNICATIONS INTERFACE ADAPTER (ACIA)
    U59             1   EF9365P             GRAPHIC DISPLAY PROCESSOR (GDP)
    U65             1   AY-3-8910A          PROGRAMMABLE SOUND GENERATOR
    U16,U17,U18,U19,
    U20,U21,U22,U23 8   MK4564              65,536 x 1-BIT DYNAMIC RAM
    U38,U39,U40,U41,
    U42,U43,U44,U45,
    U46,U47,U48,U49,
    U50,U51,U52,U53 16  TMS4116             16,384-BIT DYNAMIC RAM
    U68             1   EFB7510             SINGLE CHIP ASYNCHRONOUS FSK MODEM


    Memory map
    ==========

    Devices                                     Adresses
    =========================================================
    EPROM                                       0xF100-0xFFFF
    Extension Port                              0xF080-0xF0FF
    FREE                                        0xF070-0xF07F
    AY-3-8910A                                  0xF060-0xF06F
    ACIA EF6850 (Modem + K7)                    0xF050-0xF05F
    Restricted area                             0xF04C-0xF04F
    PIO EF6821 (Printer + Cartridge)            0xF048-0xF04B
    PIO EF6821 (Keyboard)                       0xF044-0xF047
    FREE                                        0xF040-0xF043
    VID_RD2                                     0xF030-0xF03F
    VID_RD1                                     0xF020-0xF02F
    REG1                                        0xF010-0xF01F
    Video Controller EF9365                     0xF000-0xF00F
    System RAM                                  0x0000-0xEFFF


    Notes:
    1) For 8KB versions of the monitor, the bank switching is done with the bit 7 of REG1.
    2) VID_RD1 : [7..0] = I0,R0,G0,B0,I1,R1,G1,B1 (I=Intensity,R=Red,G=Green,B=Blue)
    3) VID_RD2 : [7..0] = I2,R2,G2,B2,I3,R3,G3,B3 (I=Intensity,R=Red,G=Green,B=Blue)
    3) REG1 : [7..0] = EPROM Bank,-,Modem,K7,I,R,G,B (I=Intensity,R=Red,V=Green,B=Blue)

****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "video/ef9365.h"
#include "machine/6821pia.h"
#include "machine/6850acia.h"
#include "sound/ay8910.h"

#define MAIN_CLOCK           XTAL_14MHz
#define AY_CLOCK             MAIN_CLOCK / 8     /* 1.75 Mhz */
#define CPU_CLOCK            MAIN_CLOCK / 4     /* 3.50 Mhz */

class squale_state : public driver_device
{
public:
	squale_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_acia(*this,  "ef6850")
		, m_ay8910(*this,  "ay8910")
		, m_pia_u72(*this, "pia_u72")
		, m_pia_u75(*this, "pia_u75")
		, m_ef9365(*this, "ef9365")
		, m_maincpu(*this, "maincpu")
	{ }

	DECLARE_WRITE8_MEMBER(ctrl_w);

	virtual void machine_start();
	virtual void machine_reset();

	TIMER_DEVICE_CALLBACK_MEMBER(squale_scanline);

private:
	required_device<acia6850_device> m_acia;
	required_device<ay8910_device> m_ay8910;
	required_device<pia6821_device> m_pia_u72;
	required_device<pia6821_device> m_pia_u75;
	required_device<ef9365_device> m_ef9365;
	required_device<cpu_device> m_maincpu;
};

/*************************
*      Misc Handlers     *
*************************/

WRITE8_MEMBER( squale_state::ctrl_w )
{
	#ifdef DBGMODE
	printf("write ctrl reg : 0x%X\n",data);
	#endif

	membank("rom_bank")->set_entry(data >> 7);

	m_ef9365->static_set_color_filler(data & 0xF);
}

TIMER_DEVICE_CALLBACK_MEMBER( squale_state::squale_scanline )
{
	m_ef9365->update_scanline((UINT16)param);
}

static ADDRESS_MAP_START(squale_mem, AS_PROGRAM, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0xefff) AM_RAM
	AM_RANGE(0xf000,0xf00f) AM_DEVREADWRITE("ef9365", ef9365_device, data_r, data_w)
	AM_RANGE(0xf010,0xf01f) AM_WRITE( ctrl_w )
	AM_RANGE(0xf044,0xf047) AM_DEVREADWRITE("pia_u75", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xf048,0xf04b) AM_DEVREADWRITE("pia_u72", pia6821_device, read_alt, write_alt)
	AM_RANGE(0xf050,0xf05f) AM_DEVREADWRITE("ef6850", acia6850_device, data_r, data_w)
	AM_RANGE(0xf060,0xf06f) AM_DEVREADWRITE("ay8910", ay8910_device, data_r, address_data_w)
	AM_RANGE(0xf100,0xffff) AM_ROMBANK("rom_bank");
ADDRESS_MAP_END

static ADDRESS_MAP_START( squale_io, AS_IO, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( squale )
INPUT_PORTS_END

void squale_state::machine_start()
{
	int i;

	membank("rom_bank")->configure_entry(0, memregion("maincpu")->base() + 0x100);
	membank("rom_bank")->configure_entry(1, memregion("maincpu")->base() + 0x1100);
	membank("rom_bank")->set_entry( 0 );

	// Generate Squale hardware palette
	for(i=0;i<8;i++)
	{
		m_ef9365->static_set_color_entry(i,(((i&4)>>2)^1) * 255,(((i&2)>>1)^1) * 255, ((i&1)^1) * 255 );
	}

	for(i=0;i<8;i++)
	{
		m_ef9365->static_set_color_entry(i + 8,(((i&4)>>2)^1) * 127,(((i&2)>>1)^1) * 127, ((i&1)^1) * 127 );
	}
}

void squale_state::machine_reset()
{
}

static MACHINE_CONFIG_START( squale, squale_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809, CPU_CLOCK)
	MCFG_CPU_PROGRAM_MAP(squale_mem)
	MCFG_CPU_IO_MAP(squale_io)

	/* Cartridge pia */
	MCFG_DEVICE_ADD("pia_u72", PIA6821, 0)
	// TODO : Add port I/O handler

	/* Keyboard pia */
	MCFG_DEVICE_ADD("pia_u75", PIA6821, 0)
	// TODO : Add port I/O handler

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("ay8910", AY8910, AY_CLOCK)
	// TODO : Add port I/O handler
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.50)

	MCFG_DEVICE_ADD ("ef6850", ACIA6850, 0)

	/* screen */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_UPDATE_DEVICE("ef9365", ef9365_device, screen_update)

	MCFG_SCREEN_SIZE(336, 270)
	MCFG_SCREEN_VISIBLE_AREA(00, 336-1, 00, 270-1)
	MCFG_PALETTE_ADD("palette", 16)

	MCFG_DEVICE_ADD("ef9365", EF9365, 0)
	MCFG_EF9365_PALETTE("palette")
	MCFG_TIMER_DRIVER_ADD_SCANLINE("squale_sl", squale_state, squale_scanline, "screen", 0, 10)

MACHINE_CONFIG_END

/* ROM definition */
ROM_START( squale )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_DEFAULT_BIOS("v201")

	ROM_SYSTEM_BIOS(0, "v201", "Version 2.1")
	ROMX_LOAD( "sqmon_2r1.bin", 0x0000, 0x2000, CRC(ed57c707) SHA1(c8bd33a6fb07fe7f881f2605ad867b7e82366bfc), ROM_BIOS(1) )

	// place ROM v1.2 signature here.

	ROM_REGION( 0x1E0, "ef9365", 0 )
	ROM_LOAD( "charset_ef9365.rom", 0x0000, 0x01E0, CRC(8d3053be) SHA1(0f9a64d217a0f7f04ee0720d49c5b680ad0ae359) )
ROM_END

/* Driver */

/*    YEAR   NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1984, squale, 0,      0,       squale,    squale,driver_device,   0,     "Apollo 7", "Squale",    MACHINE_IS_SKELETON )
