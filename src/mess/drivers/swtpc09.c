// license:BSD-3-Clause
// copyright-holders:Robert Justice
/**************************************************************************

    SWTPC S/09 Mess driver
    Robert Justice ,2009-2014

    Emulates four different fixed combinations of hardware
    1. swtpc09
       MP-09 with SBUG rom, MP-ID, MP-S2, DMF2.
       Will boot Flex operating system
    2. swtpc09i
       MP-09 with SBUG rom + HDrom, MP-ID, MP-S2, DMF2, PIAIDE.
       Will boot Flex operating system
       TODO: finish ide part and get this one working.
    3. swtpc09u
       MP-09 with UniBUG rom, MP-ID, MP-S2, DMF2.
       Will boot UniFlex operating system
    4. swtpc09d3
       MP-09 with UniBUG U3 rom, MP-ID, MP-S2, DMF3.
       Will boot UniFlex operating system
       TODO: add Harddisk support, DMF3 has WD1000 interface

***************************************************************************/

#include "emu.h"
#include "includes/swtpc09.h"
#include "formats/flex_dsk.h"


/**************************************************************************
 Address maps

 56K of RAM from 0x0000 to 0xdfff
 2K  of ROM from 0xf800 to 0xffff

 E000 - E003  S2   MC6850 ACIA1   (used for UniFlex console)
 E004 - E007  S2   MC6850 ACIA2   (used for Flex console)
 E080 - E08F  MPID MC6821 PIA
 E090 - E09F  MPID MC6840 PTM

 F000 - F01F  DMF2 MC6844 DMAC
 F020 - F023  DMF2 WD1791 FDC
 F024 - F03F  DMF2 Drive select register
 F040 - F041  DMF2 DMA Address register

 F800 - FFFF  ROM
 FFF0 - FFFF  DAT RAM (only for writes)


 for DMF3 version
 F000 - F01F  DMF3 MC6844 DMAC
 F020 - F023  DMF3 WD1791 FDC
 F024 - F024  DMF3 Drive select register
 F025 - F025  DMF3 DMA Address register
 F040 - F04F  DMF3 6522 VIA

***************************************************************************/

/* Address map is dynamically setup when DAT memory is written to  */
/* only ROM from FF00-FFFF and DAT memory at FFF0-FFFF (write only) is guaranteed always*/

static ADDRESS_MAP_START(swtpc09_mem, AS_PROGRAM, 8, swtpc09_state)
	AM_RANGE(0xff00, 0xffef) AM_ROM
	AM_RANGE(0xfff0, 0xffff) AM_ROM AM_WRITE(dat_w)
ADDRESS_MAP_END


/* Input ports */
static INPUT_PORTS_START( swtpc09 )
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( swtpc09_state::floppy_formats )
	FLOPPY_FLEX_FORMAT
FLOPPY_FORMATS_END

// todo: implement floppy controller cards as slot devices and do this properly
static SLOT_INTERFACE_START( swtpc09_floppies )
	SLOT_INTERFACE("sssd",   FLOPPY_525_SSSD)     // flex 40 trks ss sd 5.25
	SLOT_INTERFACE("sssd35", FLOPPY_525_SSSD_35T) // flex 35 trks ss sd 5.25
	SLOT_INTERFACE("ssdd",   FLOPPY_525_SSDD)     // flex 40 trks ss dd 5.25
	SLOT_INTERFACE("dd",     FLOPPY_525_DD)       // flex 40 trks ds dd 5.25
	SLOT_INTERFACE("8dssd",  FLOPPY_8_DSSD)       // UNIFlex 8 inch ds sd
	SLOT_INTERFACE("8dsdd",  FLOPPY_8_DSDD)       // UNIFlex 8 inch ds dd
	SLOT_INTERFACE("35hd",   FLOPPY_35_HD)        // flex 1.44mb disk from swtpc emu (emulator only?)
SLOT_INTERFACE_END

WRITE_LINE_MEMBER(swtpc09_state::write_acia_clock)
{
	m_acia->write_txc(state);
	m_acia->write_rxc(state);
}

/***************************************************************************
 Machine definitions
****************************************************************************/

/* Machine driver */
/* MPU09, MPID, MPS2 DMF2 */
static MACHINE_CONFIG_START( swtpc09, swtpc09_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 1000000)
	MCFG_CPU_PROGRAM_MAP(swtpc09_mem)

	/* video hardware */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(2000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(50, 0, 50)
	MCFG_PTM6840_OUT0_CB(WRITE8(swtpc09_state, ptm_o1_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(swtpc09_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(swtpc09_state, ptm_irq))

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_a_r))
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_ca1_r))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(swtpc09_state, pia0_irq_a))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(swtpc09_state, acia_interrupt))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(swtpc09_state, write_acia_clock))

	MCFG_FD1793_ADD("fdc", XTAL_1MHz)
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(swtpc09_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(swtpc09_state, fdc_drq_w))

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
MACHINE_CONFIG_END

/* MPU09, MPID, MPS2 DC4 PIAIDE*/
static MACHINE_CONFIG_START( swtpc09i, swtpc09_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 1000000)
	MCFG_CPU_PROGRAM_MAP(swtpc09_mem)

	/* video hardware */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(2000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(50, 0, 50)
	MCFG_PTM6840_OUT0_CB(WRITE8(swtpc09_state, ptm_o1_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(swtpc09_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(swtpc09_state, ptm_irq))

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_a_r))
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_ca1_r))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(swtpc09_state, pia0_irq_a))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(swtpc09_state, acia_interrupt))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(swtpc09_state, write_acia_clock))

	MCFG_FD1793_ADD("fdc", XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)

	MCFG_DEVICE_ADD("piaide", PIA6821, 0)

/* old start to adding ide support, needs major updating */
/* this is to support an add on card driving IDE from a PIA */
//  MCFG_HARDDISK_ADD("harddisk")
//  MCFG_IDE_CONTROLLER_ADD("ide", NULL)
//  MCFG_IDE_CONTROLLER_REGIONS("harddisk", NULL)
//  MCFG_IDE_CONTROLLER_ADD( "ide", ide_intf, "hdd", NULL, false )  /* FIXME */ bebox
MACHINE_CONFIG_END


/* MPU09, MPID, MPS2 DMF3 */
static MACHINE_CONFIG_START( swtpc09d3, swtpc09_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M6809, 2000000)
	MCFG_CPU_PROGRAM_MAP(swtpc09_mem)

	/* video hardware */
	MCFG_RS232_PORT_ADD("rs232", default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(DEVWRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("ptm", PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(2000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(50, 0, 50)
	MCFG_PTM6840_OUT0_CB(WRITE8(swtpc09_state, ptm_o1_callback))
	MCFG_PTM6840_OUT2_CB(WRITE8(swtpc09_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(swtpc09_state, ptm_irq))

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_a_r))
	MCFG_PIA_READPA_HANDLER(READ8(swtpc09_state, pia0_ca1_r))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(swtpc09_state, pia0_irq_a))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(DEVWRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(swtpc09_state, acia_interrupt))
	MCFG_ACIA6850_IRQ_HANDLER(DEVWRITELINE("maincpu", m6809_device, irq_line))

	MCFG_DEVICE_ADD("acia_clock", CLOCK, 153600)
	MCFG_CLOCK_SIGNAL_HANDLER(WRITELINE(swtpc09_state, write_acia_clock))

	MCFG_FD1793_ADD("fdc", XTAL_1MHz)

	MCFG_FLOPPY_DRIVE_ADD("fdc:0", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)

	MCFG_DEVICE_ADD("via", VIA6522, XTAL_4MHz / 4)
	MCFG_VIA6522_READPA_HANDLER(READ8(swtpc09_state, dmf3_via_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(swtpc09_state, dmf3_via_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(swtpc09_state, dmf3_via_write_porta))
	//MCFG_VIA6522_CA1_HANDLER(WRITELINE(swtpc09_state, dmf3_via_write_ca1))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(swtpc09_state, dmf3_via_irq))
MACHINE_CONFIG_END


/* ROM definition */
ROM_START( swtpc09 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD ( "sbugh1-8.bin", 0xf800, 0x0800, CRC(10a045a7) SHA1(de547b77653951c7424a069520d52c5b0432e98d) )
ROM_END

ROM_START( swtpc09i )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD ( "hd-rom.bin", 0xe800, 0x0800, CRC(b898b4d7) SHA1(2806633eda7da4e9a243fc534f15526ee928b6bc) )
	ROM_LOAD ( "sbugh1-8.bin", 0xf800, 0x0800, CRC(10a045a7) SHA1(de547b77653951c7424a069520d52c5b0432e98d) )
ROM_END

ROM_START( swtpc09u )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD ( "unibug.bin", 0xf800, 0x0800, CRC(92e1cbf2) SHA1(db00f17ee9accdbfa1775fe0162d3556159b8e70) )
ROM_END

ROM_START( swtpc09d3 )
	ROM_REGION( 0x100000, "maincpu", 0 )
	ROM_LOAD ( "uos3.bin", 0xf800, 0x0800, CRC(e95eb3e0) SHA1(3e971d3b7e143bc87e4b506e18a8c928c089c25a) )
ROM_END

/* Driver */

/*    YEAR  NAME       PARENT  COMPAT    MACHINE   INPUT   INIT     COMPANY  FULLNAME   FLAGS */
COMP( 1980, swtpc09,         0,      0,   swtpc09,   swtpc09, swtpc09_state,   swtpc09, "SWTPC", "swtpc S/09 Sbug", MACHINE_NO_SOUND_HW)
COMP( 1980, swtpc09i,   swtpc09,      0,  swtpc09i,   swtpc09, swtpc09_state,  swtpc09i, "SWTPC", "swtpc S/09 Sbug + piaide", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW)
COMP( 1980, swtpc09u,   swtpc09,      0,   swtpc09,   swtpc09, swtpc09_state,  swtpc09u, "SWTPC", "swtpc S/09 UNIBug + DMF2", MACHINE_NO_SOUND_HW)
COMP( 1980, swtpc09d3,  swtpc09,      0, swtpc09d3,   swtpc09, swtpc09_state, swtpc09d3, "SWTPC", "swtpc S/09 UNIBug + DMF3", MACHINE_NO_SOUND_HW)
