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

void swtpc09_state::mp09_mem(address_map &map)
{
	map(0x0000, 0xfeff).rw(this, FUNC(swtpc09_state::main_r), FUNC(swtpc09_state::main_w));
	map(0xff00, 0xffff).rom().region("maincpu", 0xff00);
	map(0xff00, 0xff0f).mirror(0xf0).writeonly().share("dat");
}

void swtpc09_state::flex_dmf2_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram().share("mainram"); // by default everything is ram, 1MB ram emulated
	map(0xe000, 0xe003).mirror(0xf0000).noprw();
	map(0xe004, 0xe005).mirror(0xf0000).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe080, 0xe083).mirror(0xf000c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe090, 0xe097).mirror(0xf0008).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xe0a0, 0xefff).mirror(0xf0000).noprw();
	map(0xf000, 0xf01f).mirror(0xf0000).rw(this, FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xf020, 0xf023).mirror(0xf0000).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xf024, 0xf03f).mirror(0xf0000).rw(this, FUNC(swtpc09_state::dmf2_control_reg_r), FUNC(swtpc09_state::dmf2_control_reg_w));
	//AM_RANGE(0xf042, 0xf7ff) AM_MIRROR(0xf0000) AM_NOP
	map(0xf800, 0xffff).mirror(0xf0000).rom().region("maincpu", 0xf800);
}

void swtpc09_state::flex_dc4_piaide_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram().share("mainram"); // by default everything is ram, 1MB ram emulated
	map(0xe000, 0xe003).mirror(0xf0000).noprw();
	map(0xe004, 0xe005).mirror(0xf0000).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe014, 0xe014).mirror(0xf0000).w(this, FUNC(swtpc09_state::dc4_control_reg_w));
	map(0xe018, 0xe01b).mirror(0xf0000).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	//AM_RANGE(0xe01c, 0xe05f) AM_MIRROR(0xf0000) AM_NOP
	map(0xe060, 0xe063).mirror(0xf000c).rw(m_piaide, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	//AM_RANGE(0xe070, 0xe07f) AM_MIRROR(0xf0000) AM_NOP
	map(0xe080, 0xe083).mirror(0xf000c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe090, 0xe097).mirror(0xf0008).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	//AM_RANGE(0xe0a0, 0xe7ff) AM_MIRROR(0xf0000) AM_NOP
	map(0xe800, 0xefff).mirror(0xf0000).rom().region("maincpu", 0xe800); //piaide rom
	//AM_RANGE(0xf000, 0xf01f) AM_MIRROR(0xf0000) AM_READWRITE(m6844_r, m6844_w)
	//AM_RANGE(0xf020, 0xf023) AM_MIRROR(0xf0000) AM_DEVREADWRITE("fdc", fd1793_device, read, write)
	//AM_RANGE(0xf024, 0xf03f) AM_MIRROR(0xf0000) AM_READWRITE(dmf2_control_reg_r, dmf2_control_reg_w)
	//AM_RANGE(0xf040, 0xf041) AM_MIRROR(0xf0000) AM_READWRITE(dmf2_dma_address_reg_r, dmf2_dma_address_reg_w)
	map(0xf000, 0xf7ff).mirror(0xf0000).ram().share("s09ram"); // 2k ram for piaide on s09 board
	map(0xf800, 0xffff).mirror(0xf0000).rom().region("maincpu", 0xf800);
}

void swtpc09_state::uniflex_dmf2_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram().share("mainram"); // by default everything is ram, 1MB ram emulated
	map(0xe000, 0xe001).mirror(0xf0000).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe002, 0xe07f).mirror(0xf0000).noprw();
	map(0xe080, 0xe083).mirror(0xf000c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe090, 0xe097).mirror(0xf0008).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xe0a0, 0xefff).mirror(0xf0000).noprw();
	map(0xf000, 0xf01f).mirror(0xf0000).rw(this, FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xf020, 0xf023).mirror(0xf0000).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xf024, 0xf03f).mirror(0xf0000).rw(this, FUNC(swtpc09_state::dmf2_control_reg_r), FUNC(swtpc09_state::dmf2_control_reg_w));
	//AM_RANGE(0xf042, 0xf7ff) AM_MIRROR(0xf0000) AM_NOP
	map(0xf800, 0xffff).mirror(0xf0000).rom().region("maincpu", 0xf800);
}

void swtpc09_state::uniflex_dmf3_mem(address_map &map)
{
	map(0x00000, 0xfffff).ram().share("mainram"); // by default everything is ram, 1MB ram emulated
	map(0xe000, 0xe001).mirror(0xf0000).rw(m_acia, FUNC(acia6850_device::read), FUNC(acia6850_device::write));
	map(0xe002, 0xe07f).mirror(0xf0000).noprw();
	map(0xe080, 0xe083).mirror(0xf000c).rw(m_pia, FUNC(pia6821_device::read), FUNC(pia6821_device::write));
	map(0xe090, 0xe097).mirror(0xf0008).rw(m_ptm, FUNC(ptm6840_device::read), FUNC(ptm6840_device::write));
	map(0xe0a0, 0xefff).mirror(0xf0000).noprw();
	map(0xf000, 0xf01f).mirror(0xf0000).rw(this, FUNC(swtpc09_state::m6844_r), FUNC(swtpc09_state::m6844_w));
	map(0xf020, 0xf023).mirror(0xf0000).rw(m_fdc, FUNC(fd1793_device::read), FUNC(fd1793_device::write));
	map(0xf024, 0xf024).mirror(0xf0000).rw(this, FUNC(swtpc09_state::dmf3_control_reg_r), FUNC(swtpc09_state::dmf3_control_reg_w));
	map(0xf025, 0xf025).mirror(0xf0000).rw(this, FUNC(swtpc09_state::dmf3_dma_address_reg_r), FUNC(swtpc09_state::dmf3_dma_address_reg_w));
	//AM_RANGE(0xf030, 0xf03f) AM_MIRROR(0xf0000) AM_NOP
	map(0xf040, 0xf04f).mirror(0xf0000).rw(m_via, FUNC(via6522_device::read), FUNC(via6522_device::write));
	//AM_RANGE(0xf050, 0xf7ff) AM_MIRROR(0xf0000) AM_NOP
	map(0xf800, 0xffff).mirror(0xf0000).rom().region("maincpu", 0xf800);
}


/* Input ports */
static INPUT_PORTS_START( swtpc09 )
INPUT_PORTS_END

FLOPPY_FORMATS_MEMBER( swtpc09_state::floppy_formats )
	FLOPPY_FLEX_FORMAT
FLOPPY_FORMATS_END

// todo: implement floppy controller cards as slot devices and do this properly
static void swtpc09_floppies(device_slot_interface &device)
{
	device.option_add("sssd",   FLOPPY_525_SSSD);     // flex 40 trks ss sd 5.25
	device.option_add("sssd35", FLOPPY_525_SSSD_35T); // flex 35 trks ss sd 5.25
	device.option_add("ssdd",   FLOPPY_525_SSDD);     // flex 40 trks ss dd 5.25
	device.option_add("dd",     FLOPPY_525_DD);       // flex 40 trks ds dd 5.25
	device.option_add("8dssd",  FLOPPY_8_DSSD);       // UNIFlex 8 inch ds sd
	device.option_add("8dsdd",  FLOPPY_8_DSDD);       // UNIFlex 8 inch ds dd
	device.option_add("35hd",   FLOPPY_35_HD);        // flex 1.44mb disk from swtpc emu (emulator only?)
}


/***************************************************************************
 Machine definitions
****************************************************************************/

/* Machine driver */
/* MPU09, MPID, MPS2 DMF2 */
MACHINE_CONFIG_START(swtpc09_state::swtpc09_base)
	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", MC6809, XTAL(4'000'000))
	MCFG_DEVICE_PROGRAM_MAP(mp09_mem)

	MCFG_DEVICE_ADD("bankdev", ADDRESS_MAP_BANK, 0)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_BIG)
	MCFG_ADDRESS_MAP_BANK_DATA_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_ADDR_WIDTH(20)
	MCFG_DEVICE_PROGRAM_MAP(flex_dmf2_mem)

	MCFG_DEVICE_ADD("pia", PIA6821, 0)
	MCFG_PIA_READPA_HANDLER(READ8(*this, swtpc09_state, pia0_a_r))
	MCFG_PIA_READPA_HANDLER(READ8(*this, swtpc09_state, pia0_ca1_r))
	MCFG_PIA_IRQA_HANDLER(WRITELINE(*this, swtpc09_state, pia0_irq_a))

	MCFG_DEVICE_ADD("ptm", PTM6840, 2000000)
	MCFG_PTM6840_EXTERNAL_CLOCKS(50, 0, 50)
	MCFG_PTM6840_O1_CB(WRITELINE(*this, swtpc09_state, ptm_o1_callback))
	MCFG_PTM6840_O3_CB(WRITELINE(*this, swtpc09_state, ptm_o3_callback))
	MCFG_PTM6840_IRQ_CB(WRITELINE(*this, swtpc09_state, ptm_irq))

	MCFG_DEVICE_ADD("rs232", RS232_PORT, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(WRITELINE("acia", acia6850_device, write_rxd))
	MCFG_RS232_CTS_HANDLER(WRITELINE("acia", acia6850_device, write_cts))

	MCFG_DEVICE_ADD("acia", ACIA6850, 0)
	MCFG_ACIA6850_TXD_HANDLER(WRITELINE("rs232", rs232_port_device, write_txd))
	MCFG_ACIA6850_RTS_HANDLER(WRITELINE("rs232", rs232_port_device, write_rts))
	MCFG_ACIA6850_IRQ_HANDLER(WRITELINE(*this, swtpc09_state, acia_interrupt))

	MCFG_DEVICE_ADD("brg", MC14411, XTAL(1'843'200))
	MCFG_MC14411_F1_CB(WRITELINE("acia", acia6850_device, write_txc))
	MCFG_DEVCB_CHAIN_OUTPUT(WRITELINE("acia", acia6850_device, write_rxc))

	MCFG_FD1793_ADD("fdc", XTAL(1'000'000))
	MCFG_FLOPPY_DRIVE_ADD("fdc:0", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:1", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:2", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
	MCFG_FLOPPY_DRIVE_ADD("fdc:3", swtpc09_floppies, "dd", swtpc09_state::floppy_formats)
MACHINE_CONFIG_END

MACHINE_CONFIG_START(swtpc09_state::swtpc09)
	swtpc09_base(config);
	MCFG_DEVICE_MODIFY("fdc")
	MCFG_WD_FDC_INTRQ_CALLBACK(WRITELINE(*this, swtpc09_state, fdc_intrq_w))
	MCFG_WD_FDC_DRQ_CALLBACK(WRITELINE(*this, swtpc09_state, fdc_drq_w))
MACHINE_CONFIG_END

/* MPU09, MPID, MPS2 DC4 PIAIDE*/
MACHINE_CONFIG_START(swtpc09_state::swtpc09i)
	swtpc09_base(config);
	MCFG_DEVICE_MODIFY("bankdev")
	MCFG_DEVICE_PROGRAM_MAP(flex_dc4_piaide_mem)

	MCFG_DEVICE_ADD("piaide", PIA6821, 0)

/* old start to adding ide support, needs major updating */
/* this is to support an add on card driving IDE from a PIA */
//  MCFG_HARDDISK_ADD("harddisk")
//  MCFG_IDE_CONTROLLER_ADD("ide", nullptr)
//  MCFG_IDE_CONTROLLER_REGIONS("harddisk", nullptr)
//  MCFG_IDE_CONTROLLER_ADD( "ide", ide_intf, "hdd", nullptr, false )  /* FIXME */ bebox
MACHINE_CONFIG_END


MACHINE_CONFIG_START(swtpc09_state::swtpc09u)
	swtpc09(config);
	MCFG_DEVICE_MODIFY("bankdev")
	MCFG_DEVICE_PROGRAM_MAP(uniflex_dmf2_mem)
MACHINE_CONFIG_END


/* MPU09, MPID, MPS2 DMF3 */
MACHINE_CONFIG_START(swtpc09_state::swtpc09d3)
	swtpc09_base(config);
	MCFG_DEVICE_MODIFY("maincpu")
	MCFG_DEVICE_CLOCK(XTAL(8'000'000))

	MCFG_DEVICE_MODIFY("pia")
	MCFG_DEVICE_CLOCK(2000000)

	MCFG_DEVICE_MODIFY("bankdev")
	MCFG_DEVICE_PROGRAM_MAP(uniflex_dmf3_mem)

	/* video hardware */
	MCFG_DEVICE_MODIFY("acia")
	MCFG_ACIA6850_IRQ_HANDLER(INPUTLINE("maincpu", M6809_IRQ_LINE))

	MCFG_DEVICE_ADD("via", VIA6522, XTAL(4'000'000) / 4)
	MCFG_VIA6522_READPA_HANDLER(READ8(*this, swtpc09_state, dmf3_via_read_porta))
	MCFG_VIA6522_READPB_HANDLER(READ8(*this, swtpc09_state, dmf3_via_read_portb))
	MCFG_VIA6522_WRITEPA_HANDLER(WRITE8(*this, swtpc09_state, dmf3_via_write_porta))
	//MCFG_VIA6522_CA1_HANDLER(WRITELINE(*this, swtpc09_state, dmf3_via_write_ca1))
	MCFG_VIA6522_IRQ_HANDLER(WRITELINE(*this, swtpc09_state, dmf3_via_irq))
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

//    YEAR  NAME       PARENT   COMPAT  MACHINE    INPUT    CLASS          INIT            COMPANY  FULLNAME                    FLAGS
COMP( 1980, swtpc09,   0,       0,      swtpc09,   swtpc09, swtpc09_state, init_swtpc09,   "SWTPC", "swtpc S/09 Sbug",          MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09i,  swtpc09, 0,      swtpc09i,  swtpc09, swtpc09_state, init_swtpc09i,  "SWTPC", "swtpc S/09 Sbug + piaide", MACHINE_NOT_WORKING | MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09u,  swtpc09, 0,      swtpc09u,  swtpc09, swtpc09_state, init_swtpc09u,  "SWTPC", "swtpc S/09 UNIBug + DMF2", MACHINE_NO_SOUND_HW )
COMP( 1980, swtpc09d3, swtpc09, 0,      swtpc09d3, swtpc09, swtpc09_state, init_swtpc09d3, "SWTPC", "swtpc S/09 UNIBug + DMF3", MACHINE_NO_SOUND_HW )
