// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
/****************************************************************************

    SSE HardBox emulation

****************************************************************************/

/*
    http://mikenaberezny.com/hardware/pet-cbm/sse-hardbox-corvus-interface/

    The HardBox provides a CBM DOS interface for a Corvus hard disk.  Before
    it can be used, a hard disk image must first be created and formatted.
    Use the CHDMAN utility to create a 20MB image:

    $ chdman createhd -o /path/to/corvus20mb.chd -chs 388,5,20 -ss 512

    Start the pet8032 emulator with the HardBox attached as device 9,
    with the new CHD and the utilities floppy mounted:

    $ mame pet8032 -ieee9 hardbox \
                   -hard1 /path/to/corvus20mb.chd \
                   -flop1 /path/to/hardbox-utils.d80

    Load and run the "configure" program from the floppy.  When prompted
    for the HardBox device number, enter "9".

    Select "q" for quick configure at the menu.  It will present a default
    drive size and ask if you want to alter it.  If the size is not 20,
    change it to 20.

    After accepting the drive size, it will prompt if you want to perform
    a format check.  This is optional.  If you enter "y" to proceed with
    the format check, it will always report no bad sectors.

    Enter "y" to proceed with the format.  After it has completed, the
    program will exit back to BASIC.  The drive should now be usable.
*/


#include "hardbox.h"



//**************************************************************************
//  MACROS / CONSTANTS
//**************************************************************************

#define Z80_TAG         "z80"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define CORVUS_HDC_TAG  "corvus"



//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

const device_type HARDBOX = &device_creator<hardbox_device>;


//-------------------------------------------------
//  ROM( hardbox )
//-------------------------------------------------

ROM_START( hardbox )
	ROM_REGION( 0x2000, Z80_TAG, 0 )
	ROM_DEFAULT_BIOS("v2.4")

	ROM_SYSTEM_BIOS( 0, "v2.3", "Version 2.3 (Corvus)" )
	ROMX_LOAD( "295-2.3.ic3", 0x0000, 0x1000, CRC(a3eb5fc2) SHA1(39941b45b0696db928615c41c7eae18d951d9ada), ROM_BIOS(1) )
	ROMX_LOAD( "296-2.3.ic4", 0x1000, 0x1000, CRC(fb55b058) SHA1(8f9ec313ec6beaf7b513edf39d9628e6abcc7bc3), ROM_BIOS(1) )

	ROM_SYSTEM_BIOS( 1, "v2.4", "Version 2.4 (Corvus)" )
	ROMX_LOAD( "289.ic3", 0x0000, 0x1000, CRC(c39e058f) SHA1(45b390d7125a40f84c7b411a479218baff079746), ROM_BIOS(2) )
	ROMX_LOAD( "290.ic4", 0x1000, 0x1000, CRC(62f51405) SHA1(fdfa0d7b7e8d0182f2df0aa8163c790506104dcf), ROM_BIOS(2) )

	ROM_SYSTEM_BIOS( 2, "v3.1", "Version 3.1 (Sunol)" )
	ROMX_LOAD( "295-3.1.ic3", 0x0000, 0x1000, CRC(654a5db1) SHA1(c40859526921e3d8bfd58fc28cc9cc64e59ec638), ROM_BIOS(3) )
	ROMX_LOAD( "296-3.1.ic4", 0x1000, 0x1000, CRC(4c62ddc0) SHA1(151f99dc554d3762b805fc8383cf1b3e1455784f), ROM_BIOS(3) )

	/* Note: Two sets of EPROMs were found marked only "295" and "296" but they have different contents.
	         The version numbers listed are the ROM version reported by the HardBox diagnostics program.
	         Disassembling the ROMs showed that v2.3 and v2.4 are for Corvus Systems drives but v3.1 is
	         for Sunol Systems drives.  Both types use the Corvus flat cable interface but there may be
	         some programming differences, e.g. the v3.1 firmware for Sunol does not have the park heads
	     routine in the Corvus versions.  MAME emulates a Corvus drive so we default to the last
	     known HardBox firmware for Corvus (v2.4). */
ROM_END


//-------------------------------------------------
//  rom_region - device-specific ROM region
//-------------------------------------------------

const rom_entry *hardbox_device::device_rom_region() const
{
	return ROM_NAME( hardbox );
}


//-------------------------------------------------
//  ADDRESS_MAP( hardbox_mem )
//-------------------------------------------------

static ADDRESS_MAP_START( hardbox_mem, AS_PROGRAM, 8, hardbox_device )
	AM_RANGE(0x0000, 0x3fff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_ROM AM_REGION(Z80_TAG, 0)
ADDRESS_MAP_END


//-------------------------------------------------
//  ADDRESS_MAP( hardbox_io )
//-------------------------------------------------

static ADDRESS_MAP_START( hardbox_io, AS_IO, 8, hardbox_device )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x10, 0x13) AM_DEVREADWRITE(I8255_0_TAG, i8255_device, read, write)
	AM_RANGE(0x14, 0x17) AM_DEVREADWRITE(I8255_1_TAG, i8255_device, read, write)
	AM_RANGE(0x18, 0x18) AM_DEVREADWRITE(CORVUS_HDC_TAG, corvus_hdc_t, read, write)
ADDRESS_MAP_END


//-------------------------------------------------
//  I8255A 0 interface
//-------------------------------------------------

READ8_MEMBER( hardbox_device::ppi0_pa_r )
{
	return m_bus->dio_r() ^ 0xff;
}

WRITE8_MEMBER( hardbox_device::ppi0_pb_w )
{
	m_bus->dio_w(this, data ^ 0xff);
}

READ8_MEMBER( hardbox_device::ppi0_pc_r )
{
	UINT8 data = ioport("SW1")->read();

	/* DIP switches on PC1,PC2,PC3 configure the IEEE-488 primary address.
	   We get the address from the slot instead. */
	data |= ((m_slot->get_address() - 8) << 1) ^ 0xff;

	return data;
}

//-------------------------------------------------
//  I8255A 1 interface
//-------------------------------------------------

READ8_MEMBER( hardbox_device::ppi1_pa_r )
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

	data |= !m_bus->atn_r();
	data |= !m_bus->dav_r() << 1;
	data |= !m_bus->ndac_r() << 2;
	data |= !m_bus->nrfd_r() << 3;
	data |= !m_bus->eoi_r() << 4;
	data |= !m_bus->srq_r() << 5;
	data |= !m_bus->ren_r() << 6;
	data |= !m_bus->ifc_r() << 7;

	return data;
}

WRITE8_MEMBER( hardbox_device::ppi1_pb_w )
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

	  Note: When the PCB is configured as a HardBox instead of a SoftBox,
	        IFC is read only.  Do not connect IFC for output here.

	*/

	m_bus->atn_w(this, !BIT(data, 0));
	m_bus->dav_w(this, !BIT(data, 1));
	m_bus->ndac_w(this, !BIT(data, 2));
	m_bus->nrfd_w(this, !BIT(data, 3));
	m_bus->eoi_w(this, !BIT(data, 4));
	m_bus->srq_w(this, !BIT(data, 5));
	m_bus->ren_w(this, !BIT(data, 6));
}

READ8_MEMBER( hardbox_device::ppi1_pc_r )
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

WRITE8_MEMBER( hardbox_device::ppi1_pc_w )
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

	machine().output().set_led_value(LED_A, !BIT(data, 0));
	machine().output().set_led_value(LED_B, !BIT(data, 1));
	machine().output().set_led_value(LED_READY, !BIT(data, 2));
}

//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( hardbox )
//-------------------------------------------------

static MACHINE_CONFIG_FRAGMENT( hardbox )
	// basic machine hardware
	MCFG_CPU_ADD(Z80_TAG, Z80, XTAL_8MHz/2)
	MCFG_CPU_PROGRAM_MAP(hardbox_mem)
	MCFG_CPU_IO_MAP(hardbox_io)

	// devices
	MCFG_DEVICE_ADD(I8255_0_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(hardbox_device, ppi0_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(hardbox_device, ppi0_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(hardbox_device, ppi0_pc_r))

	MCFG_DEVICE_ADD(I8255_1_TAG, I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(READ8(hardbox_device, ppi1_pa_r))
	MCFG_I8255_OUT_PORTB_CB(WRITE8(hardbox_device, ppi1_pb_w))
	MCFG_I8255_IN_PORTC_CB(READ8(hardbox_device, ppi1_pc_r))
	MCFG_I8255_OUT_PORTC_CB(WRITE8(hardbox_device, ppi1_pc_w))

	MCFG_DEVICE_ADD(CORVUS_HDC_TAG, CORVUS_HDC, 0)
	MCFG_HARDDISK_ADD("harddisk1")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk2")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk3")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
	MCFG_HARDDISK_ADD("harddisk4")
	MCFG_HARDDISK_INTERFACE("corvus_hdd")
MACHINE_CONFIG_END


//-------------------------------------------------
//  machine_config_additions - device-specific
//  machine configurations
//-------------------------------------------------

machine_config_constructor hardbox_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( hardbox );
}


//-------------------------------------------------
//  INPUT_PORTS( hardbox )
//-------------------------------------------------

INPUT_PORTS_START( hardbox )
	PORT_START("SW1")
	PORT_DIPUNKNOWN_DIPLOC( 0x01, IP_ACTIVE_LOW, "SW1:1" )
	PORT_DIPNAME( 0x0e, 0x0c, "Device Address" ) PORT_DIPLOCATION("SW1:2,3,4")
	PORT_DIPSETTING(    0x0e, "8" )
	PORT_DIPSETTING(    0x0c, "9" )
	PORT_DIPSETTING(    0x0a, "10" )
	PORT_DIPSETTING(    0x08, "11" )
	PORT_DIPSETTING(    0x06, "12" )
	PORT_DIPSETTING(    0x04, "13" )
	PORT_DIPSETTING(    0x02, "14" )
	PORT_DIPSETTING(    0x00, "15" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10, IP_ACTIVE_LOW, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20, IP_ACTIVE_LOW, "SW1:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80, IP_ACTIVE_LOW, "SW1:8" )
INPUT_PORTS_END


//-------------------------------------------------
//  input_ports - device-specific input ports
//-------------------------------------------------

ioport_constructor hardbox_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( hardbox );
}



//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  hardbox_device - constructor
//-------------------------------------------------

hardbox_device::hardbox_device(const machine_config &mconfig, std::string tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, HARDBOX, "HardBox", tag, owner, clock, "hardbox", __FILE__),
		device_ieee488_interface(mconfig, *this),
		m_maincpu(*this, Z80_TAG),
		m_hdc(*this, CORVUS_HDC_TAG), m_ifc(0)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hardbox_device::device_start()
{
}


//-------------------------------------------------
//  device_reset_after_children - device-specific
//    reset that must happen after child devices
//    have performed their resets
//-------------------------------------------------

void hardbox_device::device_reset_after_children()
{
	/* The Z80 starts at address 0x0000 but the HardBox has RAM there and
	   needs to start from the BIOS at 0xe000.  The PCB has logic and a
	   74S287 PROM that temporarily changes the memory map so that the
	   IC3 EPROM at 0xe000 is mapped to 0x0000 for the first instruction
	   fetch only.  The instruction normally at 0xe000 is an absolute jump
	   into the ROM.  On reset, the Z80 will fetch it from 0x0000 and set
	   its PC, then the normal map will be restored before the next
	   instruction fetch.  Here we just set the PC to 0xe000 after the Z80
	   resets, which has the same effect. */

	m_maincpu->set_state_int(Z80_PC, 0xe000);
}


//-------------------------------------------------
//  ieee488_ifc - interface clear (reset)
//-------------------------------------------------

void hardbox_device::ieee488_ifc(int state)
{
	if (!m_ifc && state)
	{
		device_reset();
	}

	m_ifc = state;
}
