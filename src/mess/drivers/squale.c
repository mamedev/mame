// license:BSD-3-Clause
// copyright-holders:Miodrag Milanovic
/***************************************************************************

    Apollo 7 Squale
	
	PCB Ref	Qty	Manufacturer Ref	Description / Datasheet
	============================================================================================
	U1				1	EF6809P				8-BIT MICROPROCESSOR UNIT (MPU)
	U2				1	27C32 / 27C64		EPROM
	U72,U75			1	EF6821P				PERIPHERAL INTERFACE ADAPTER (PIA)
	U69				1	EF6850				ASYNCHRONOUS COMMUNICATIONS INTERFACE ADAPTER (ACIA)
	U59				1	EF9365P				GRAPHIC DISPLAY PROCESSOR (GDP)
	U65				1	AY-3-8910A			PROGRAMMABLE SOUND GENERATOR
	U16,U17,U18,U19,
	U20,U21,U22,U23	8	MK4564				65,536 x 1-BIT DYNAMIC RAM
	U38,U39,U40,U41,
	U42,U43,U44,U45,
	U46,U47,U48,U49,
	U50,U51,U52,U53	16	TMS4116				16,384-BIT DYNAMIC RAM
	U68				1	EFB7510				SINGLE CHIP ASYNCHRONOUS FSK MODEM
	
	
	Memory map
	==========
	
	Périphériques								Adresses
	=========================================================
	EPROM 										0xF100-0xFFFF
	Extension Port								0xF080-0xF0FF
	FREE										0xF070-0xF07F
	AY-3-8910A									0xF060-0xF06F
	ACIA EF6850 (Modem + K7)					0xF050-0xF05F
    Restricted area								0xF04C-0xF04F
	PIO EF6821 (Printer + Cartridge)			0xF048-0xF04B
	PIO EF6821 (Keyboard)						0xF044-0xF047
	FREE										0xF040-0xF043
	VID_RD2										0xF030-0xF03F
	VID_RD1										0xF020-0xF02F
	REG1										0xF010-0xF01F
	Video Controller EF9365						0xF000-0xF00F
	System RAM 									0x0000-0xEFFF
	
	
	Notes:
	1) For 8KB versions of the monitor, the bank switching is done via bit 7 of REG1.
	2) VID_RD1 : [7..0] = I0,R0,G0,B0,I1,R1,G1,B1 (I=Intensity,R=Red,G=Green,B=Blue)
	3) VID_RD2 : [7..0] = I2,R2,G2,B2,I3,R3,G3,B3 (I=Intensity,R=Red,G=Green,B=Blue)
	3) REG1 : [7..0] = EPROM Bank,-,Modem,K7,I,R,G,B (I=Intensity,R=Red,V=Green,B=Blue)
****************************************************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"

class squale_state : public driver_device
{
public:
	squale_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

private:
	required_device<cpu_device> m_maincpu;
};


static ADDRESS_MAP_START(squale_mem, AS_PROGRAM, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x0000,0xefff) AM_RAM
	AM_RANGE(0xf100,0xffff) AM_ROM AM_REGION("maincpu", 0x100)
ADDRESS_MAP_END

static ADDRESS_MAP_START( squale_io, AS_IO, 8, squale_state)
	ADDRESS_MAP_UNMAP_HIGH
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( squale )
INPUT_PORTS_END


static MACHINE_CONFIG_START( squale, squale_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6809, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(squale_mem)
	MCFG_CPU_IO_MAP(squale_io)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( squale )
	ROM_REGION( 0x2000, "maincpu", 0 )
	ROM_LOAD( "sqm2r1.bin", 0x0000, 0x2000, CRC(ed57c707) SHA1(c8bd33a6fb07fe7f881f2605ad867b7e82366bfc) )
ROM_END

/* Driver */

/*    YEAR   NAME   PARENT  COMPAT   MACHINE    INPUT  CLASS           INIT    COMPANY   FULLNAME       FLAGS */
COMP( 1984, squale, 0,      0,       squale,    squale,driver_device,   0,     "Apollo 7", "Squale", 	MACHINE_IS_SKELETON )
