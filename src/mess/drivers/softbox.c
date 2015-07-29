// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
/*

    SSE SoftBox

    http://mikenaberezny.com/hardware/pet-cbm/sse-softbox-z80-computer/


    Standalone vs. PET/CBM Peripheral Mode
    --------------------------------------

    The SoftBox can be used as a standalone computer with an RS-232 terminal,
    or as a PET/CBM peripheral.  This is an emulation of the standalone mode.
    For the peripheral mode, see: src/emu/bus/ieee488/softbox.c.


    Using the Corvus hard disk
    --------------------------

    The SoftBox distribution disk (softbox-distrib.d80) is configured for
    a CBM 8050 as CP/M drives A/B and a 10MB Corvus hard disk as drives C/D.

    Use the CHDMAN utility to create a 10MB hard disk image for the Corvus:

    $ chdman createhd -o /path/to/corvus10mb.chd -chs 358,3,20 -ss 512

    Start the SoftBox emulator with the floppy and hard disk images mounted:

    $ mess softbox -flop1 /path/to/softbox-distrib.d80 \
                   -hard1 /path/to/corvus10mb.chd

    Before the Corvus can be used under CP/M, it must be prepared
    by running DIAG.COM and FORMAT.COM.

    DIAG.COM

    Enter "diag" (no arguments) at the CP/M prompt to run the Corvus diagnostics
    program.  This program will perform the Corvus low-level format.

    Select option 6 (Update Controller Code) at the menu.
    Enter "corvb184.fmt" when prompted for the filename.
    Enter "y" at the confirmation prompts.
    Enter "1" for the Corvus drive number (two prompts).
    After formatting is complete, it will return to the menu.

    Select option 3 (Read Controller Code Version #) at the menu.
    Enter "1" for the Corvus drive number.
    It should report "V18.4AP" and then return to the menu.

    Select option 9 to return to CP/M.

    FORMAT.COM

    Enter "format" (no arguments) at the CP/M prompt to run the SoftBox disk
    format program.  This program will perform the CP/M filesystem format.

    Enter drive letter "c" at the prompt.
    Enter "y" to confirm the format.
    After formatting is complete, it will prompt for a drive letter again.

    Enter drive letter "d" at the prompt.
    Enter "y" to confirm the format.
    After formatting is complete, it will prompt for a drive letter again.

    Press RETURN to return to CP/M.

    STAT.COM

    After all steps are completed, drives C and D should be usable from
    CP/M.  Each drive is one half of the Corvus 10MB disk.  Running the
    command "stat c: dsk:" should show 4712 kilobyte drive capacity.
    Drive D should show the same information.


    Using other Corvus hard disk sizes
    ----------------------------------

    The SoftBox supports 5, 10, and 20 MB hard disks.  The distribution disk
    is configured for 10 MB as explained above.  To use other sizes, make
    a new image with CHDMAN.  See the top of src/emu/machine/corvushd.h
    for the parameters for the other drives.

    After the image has been created and the SoftBox emulator started with
    it mounted, the SoftBox BIOS needs to be told what size Corvus hard
    disk is attached.  Use the NEWSYS.COM utility to reconfigure the drive
    size.  When NEWSYS prompts for a source drive, enter "a" (the drive letter
    of the CP/M distribution disk).  Use option "d" (Disk drive assignment)
    to reconfigure the Corvus size.  After the change has been made, use option
    "s" (Save new system) to write the configuration to the floppy (drive A) and
    option "e" (Execute new system) to restart CP/M with the configuration.
    DIAG.COM and FORMAT.COM can then be used to format the hard disk.

*/

#include "includes/softbox.h"
#include "bus/rs232/rs232.h"


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************

//-------------------------------------------------
//  dbrg_w - baud rate selection
//-------------------------------------------------

WRITE8_MEMBER( softbox_state::dbrg_w )
{
	m_dbrg->str_w(data & 0x0f);
	m_dbrg->stt_w(data >> 4);
}



//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( softbox_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_mem, AS_PROGRAM, 8, softbox_state )
	AM_RANGE(0x0000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

static ADDRESS_MAP_START( softbox_io, AS_IO, 8, softbox_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x08, 0x08) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x09, 0x09) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x0c, 0x0c) AM_WRITE(dbrg_w)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(I8255_0_TAG, i8255_device, read, write)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(I8255_1_TAG, i8255_device, read, write)
	AM_RANGE(0x18, 0x18) AM_DEVREADWRITE(CORVUS_HDC_TAG, corvus_hdc_t, read, write)
ADDRESS_MAP_END



//**************************************************************************
//  INPUT PORTS
//**************************************************************************

//-------------------------------------------------
//  INPUT_PORTS( softbox )
//-------------------------------------------------

static INPUT_PORTS_START( softbox )
	/* An 8-position DIP switch may be installed at SW1.  Some
	   SoftBox units have it and some do not.  The switches are
	   not used by the SoftBox BIOS. */
	PORT_START("SW1")
	PORT_DIPUNUSED_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPUNUSED_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPUNUSED_DIPLOC( 0x04, IP_ACTIVE_LOW, "SW1:3" )
	PORT_DIPUNUSED_DIPLOC( 0x08, IP_ACTIVE_LOW, "SW1:4" )
	PORT_DIPUNUSED_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNUSED_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNUSED_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNUSED_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END



//**************************************************************************
//  DEVICE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  I8255A 0 Interface
//-------------------------------------------------

READ8_MEMBER( softbox_state::ppi0_pa_r )
{
	return m_ieee->dio_r() ^ 0xff;
}

WRITE8_MEMBER( softbox_state::ppi0_pb_w )
{
	m_ieee->dio_w(data ^ 0xff);
}

//-------------------------------------------------
//  I8255A 1 Interface
//-------------------------------------------------

READ8_MEMBER( softbox_state::ppi1_pa_r )
{
	/*

	  bit     description

	  PA0     ATN
	  PA1     DAV
	  PA2     NDAC
	  PA3     NRFD
	  PA4     EOI
	  PA5     SRQ
	  PA6     REN
	  PA7     IFC

	*/

	UINT8 data = 0;

	data |= !m_ieee->atn_r();
	data |= !m_ieee->dav_r() << 1;
	data |= !m_ieee->ndac_r() << 2;
	data |= !m_ieee->nrfd_r() << 3;
	data |= !m_ieee->eoi_r() << 4;
	data |= !m_ieee->srq_r() << 5;
	data |= !m_ieee->ren_r() << 6;
	data |= !m_ieee->ifc_r() << 7;

	return data;
}

WRITE8_MEMBER( softbox_state::ppi1_pb_w )
{
	/*

	  bit     description

	  PB0     ATN
	  PB1     DAV
	  PB2     NDAC
	  PB3     NRFD
	  PB4     EOI
	  PB5     SRQ
	  PB6     REN
	  PB7     IFC

	*/

	m_ieee->atn_w(!BIT(data, 0));
	m_ieee->dav_w(!BIT(data, 1));
	m_ieee->ndac_w(!BIT(data, 2));
	m_ieee->nrfd_w(!BIT(data, 3));
	m_ieee->eoi_w(!BIT(data, 4));
	m_ieee->srq_w(!BIT(data, 5));
	m_ieee->ren_w(!BIT(data, 6));
	m_ieee->ifc_w(!BIT(data, 7));
}

READ8_MEMBER( softbox_state::ppi1_pc_r )
{
	/*

	  bit     description

	  PC0
	  PC1
	  PC2
	  PC3
	  PC4     Corvus READY
	  PC5     Corvus DIRC
	  PC6
	  PC7

	*/

	UINT8 status = m_hdc->status_r(space, 0);
	UINT8 data = 0;

	data |= (status & CONTROLLER_BUSY) ? 0 : 0x10;
	data |= (status & CONTROLLER_DIRECTION) ? 0 : 0x20;

	return data;
}

WRITE8_MEMBER( softbox_state::ppi1_pc_w )
{
	/*

	  bit     description

	  PC0     LED "A"
	  PC1     LED "B"
	  PC2     LED "READY"
	  PC3
	  PC4
	  PC5
	  PC6
	  PC7

	*/

	output_set_led_value(LED_A, !BIT(data, 0));
	output_set_led_value(LED_B, !BIT(data, 1));
	output_set_led_value(LED_READY, !BIT(data, 2));
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_STARTBITS", 0xff, RS232_STARTBITS_1 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_7 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_EVEN )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END



//**************************************************************************
//  MACHINE INITIALIZATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_START( softbox )
//-------------------------------------------------

void softbox_state::machine_start()
{
}


//-------------------------------------------------
//  device_reset_after_children - device-specific
//    reset that must happen after child devices
//    have performed their resets
//-------------------------------------------------

void softbox_state::device_reset_after_children()
{
	/* The Z80 starts at address 0x0000 but the SoftBox has RAM there and
	   needs to start from the BIOS at 0xf000.  The PCB has logic and a
	   74S287 PROM that temporarily changes the memory map so that the
	   IC3 EPROM at 0xf000 is mapped to 0x0000 for the first instruction
	   fetch only.  The instruction normally at 0xf000 is an absolute jump
	   into the BIOS.  On reset, the Z80 will fetch it from 0x0000 and set
	   its PC, then the normal map will be restored before the next
	   instruction fetch.  Here we just set the PC to 0xf000 after the Z80
	   resets, which has the same effect. */

	m_maincpu->set_state_int(Z80_PC, 0xf000);
}


//-------------------------------------------------
//  ieee488_ifc - interface clear (reset)
//-------------------------------------------------

void softbox_state::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}



//**************************************************************************
//  MACHINE CONFIGURATION
//**************************************************************************

//-------------------------------------------------
//  MACHINE_CONFIG( softbox )
//-------------------------------------------------

static MACHINE_CONFIG_START( softbox, softbox_state )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(softbox_mem)
	MCFG_CPU_IO_MAP(softbox_io)

	// devices
	MCFG_DEVICE_ADD(I8251_TAG, I8251, 0)
	MCFG_I8251_TXD_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_txd))
	MCFG_I8251_DTR_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_dtr))
	MCFG_I8251_RTS_HANDLER(DEVWRITELINE(RS232_TAG, rs232_port_device, write_rts))

	MCFG_RS232_PORT_ADD(RS232_TAG, default_rs232_devices, "terminal")
	MCFG_RS232_RXD_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxd))
	MCFG_RS232_DSR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_dsr))
	MCFG_DEVICE_CARD_DEVICE_INPUT_DEFAULTS("terminal", terminal)

	MCFG_DEVICE_ADD(I8255_0_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(softbox_state, ppi0_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(softbox_state, ppi0_pb_w))
	MCFG_I8255_IN_PORTC_CB(IOPORT("SW1"))

	MCFG_DEVICE_ADD(I8255_1_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(softbox_state, ppi1_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(softbox_state, ppi1_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(softbox_state, ppi1_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(softbox_state, ppi1_pc_w))

	MCFG_DEVICE_ADD(COM8116_TAG, COM8116, XTAL_5_0688MHz)
	MCFG_COM8116_FR_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_rxc))
	MCFG_COM8116_FT_HANDLER(DEVWRITELINE(I8251_TAG, i8251_device, write_txc))

	MCFG_CBM_IEEE488_ADD("c8050")

	MCFG_DEVICE_ADD(CORVUS_HDC_TAG, CORVUS_HDC, 0)
	MCFG_HARDDISK_ADD("harddisk1")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk2")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk3")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk4")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")

	MCFG_IMI7000_BUS_ADD("imi5000h", NULL, NULL, NULL)

	// software lists
	MCFG_SOFTWARE_LIST_ADD("flop_list", "softbox")
MACHINE_CONFIG_END



//**************************************************************************
//  ROMS
//**************************************************************************

//-------------------------------------------------
//  ROM( softbox )
//-------------------------------------------------

ROM_START( softbox )
	ROM_REGION( 0x1000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("19830609")
	ROM_SYSTEM_BIOS( 0, "19810908", "8/9/81" )
	ROMX_LOAD( "375.ic3", 0x000, 0x800, CRC(177580e7) SHA1(af6a97495de825b80cdc9fbf72329d5440826177), ROM_BIOS(1) )
	ROMX_LOAD( "376.ic4", 0x800, 0x800, CRC(edfee5be) SHA1(5662e9071cc622a1c071d89b00272fc6ba122b9a), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "19811027", "27-Oct-81" )
	ROMX_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b), ROM_BIOS(2) )
	ROMX_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "19830609", "09-June-1983" )
	ROMX_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048), ROM_BIOS(3) )
	ROMX_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad), ROM_BIOS(3) )
ROM_END



//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT    COMPANY   FULLNAME       FLAGS
COMP( 1981, softbox,    0,      0,      softbox,        softbox, driver_device, 0,      "Small Systems Engineering",  "SoftBox",  MACHINE_NO_SOUND_HW )
