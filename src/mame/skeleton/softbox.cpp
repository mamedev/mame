// license:BSD-3-Clause
// copyright-holders:Curt Coder, Mike Naberezny
/*

    SSE SoftBox

    http://mikenaberezny.com/hardware/pet-cbm/sse-softbox-z80-computer/


    Standalone vs. PET/CBM Peripheral Mode
    --------------------------------------

    The SoftBox can be used as a standalone computer with an RS-232 terminal,
    or as a PET/CBM peripheral.  This is an emulation of the standalone mode.
    For the peripheral mode, see: src/devices/bus/ieee488/softbox.c.


    Using the Corvus hard disk
    --------------------------

    The SoftBox distribution disk (softbox-distrib.d80) is configured for
    a CBM 8050 as CP/M drives A/B and a 10MB Corvus hard disk as drives C/D.

    Use the CHDMAN utility to create a 10MB hard disk image for the Corvus:

    $ chdman createhd -o /path/to/corvus10mb.chd -chs 358,3,20 -ss 512

    Start the SoftBox emulator with the floppy and hard disk images mounted:

    $ mame softbox -flop1 /path/to/softbox-distrib.d80 \
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
    a new image with CHDMAN.  See the top of src/devices/machine/corvushd.h
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

#include "emu.h"

#include "bus/ieee488/ieee488.h"
#include "bus/imi7000/imi7000.h"
#include "bus/rs232/rs232.h"
#include "cpu/z80/z80.h"
#include "imagedev/harddriv.h"
#include "machine/corvushd.h"
#include "machine/com8116.h"
#include "machine/i8251.h"
#include "machine/i8255.h"
#include "softlist_dev.h"

namespace {

#define Z80_TAG         "z80"
#define I8251_TAG       "ic15"
#define I8255_0_TAG     "ic17"
#define I8255_1_TAG     "ic16"
#define COM8116_TAG     "ic14"
#define RS232_TAG       "rs232"
#define CORVUS_HDC_TAG  "corvus"

class softbox_state : public driver_device
{
public:
	softbox_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, Z80_TAG)
		, m_ieee(*this, IEEE488_TAG)
		, m_hdc(*this, CORVUS_HDC_TAG)
		, m_leds(*this, "led%u", 0U)
	{ }

	void softbox(machine_config &config);

private:
	// device_ieee488_interface overrides
	virtual void ieee488_ifc(int state);

	uint8_t ppi0_pa_r();
	void ppi0_pb_w(uint8_t data);

	uint8_t ppi1_pa_r();
	void ppi1_pb_w(uint8_t data);
	uint8_t ppi1_pc_r();
	void ppi1_pc_w(uint8_t data);

	enum
	{
		LED_A = 0,
		LED_B,
		LED_READY
	};

	void softbox_io(address_map &map) ATTR_COLD;
	void softbox_mem(address_map &map) ATTR_COLD;
	int m_ifc = 0;  // Tracks previous state of IEEE-488 IFC line

	virtual void machine_start() override ATTR_COLD;
	virtual void device_reset_after_children() override;

	required_device<cpu_device> m_maincpu;
	required_device<ieee488_device> m_ieee;
	required_device<corvus_hdc_device> m_hdc;
	output_finder<3> m_leds;
};


//**************************************************************************
//  READ/WRITE HANDLERS
//**************************************************************************


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

//-------------------------------------------------
//  ADDRESS_MAP( softbox_mem )
//-------------------------------------------------

void softbox_state::softbox_mem(address_map &map)
{
	map(0x0000, 0xefff).ram();
	map(0xf000, 0xffff).rom().region(Z80_TAG, 0);
}


//-------------------------------------------------
//  ADDRESS_MAP( softbox_io )
//-------------------------------------------------

void softbox_state::softbox_io(address_map &map)
{
	map.global_mask(0xff);
	map(0x08, 0x09).rw(I8251_TAG, FUNC(i8251_device::read), FUNC(i8251_device::write));
	map(0x0c, 0x0c).w(COM8116_TAG, FUNC(com8116_device::stt_str_w));
	map(0x10, 0x13).rw(I8255_0_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x14, 0x17).rw(I8255_1_TAG, FUNC(i8255_device::read), FUNC(i8255_device::write));
	map(0x18, 0x18).rw(m_hdc, FUNC(corvus_hdc_device::read), FUNC(corvus_hdc_device::write));
}



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

uint8_t softbox_state::ppi0_pa_r()
{
	return m_ieee->dio_r() ^ 0xff;
}

void softbox_state::ppi0_pb_w(uint8_t data)
{
	m_ieee->host_dio_w(data ^ 0xff);
}

//-------------------------------------------------
//  I8255A 1 Interface
//-------------------------------------------------

uint8_t softbox_state::ppi1_pa_r()
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

	uint8_t data = 0;

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

void softbox_state::ppi1_pb_w(uint8_t data)
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

	m_ieee->host_atn_w(!BIT(data, 0));
	m_ieee->host_dav_w(!BIT(data, 1));
	m_ieee->host_ndac_w(!BIT(data, 2));
	m_ieee->host_nrfd_w(!BIT(data, 3));
	m_ieee->host_eoi_w(!BIT(data, 4));
	m_ieee->host_srq_w(!BIT(data, 5));
	m_ieee->host_ren_w(!BIT(data, 6));
	m_ieee->host_ifc_w(!BIT(data, 7));
}

uint8_t softbox_state::ppi1_pc_r()
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

	uint8_t status = m_hdc->status_r();
	uint8_t data = 0;

	data |= (status & corvus_hdc_device::CONTROLLER_BUSY) ? 0 : 0x10;
	data |= (status & corvus_hdc_device::CONTROLLER_DIRECTION) ? 0 : 0x20;

	return data;
}

void softbox_state::ppi1_pc_w(uint8_t data)
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

	m_leds[LED_A] = BIT(~data, 0);
	m_leds[LED_B] = BIT(~data, 1);
	m_leds[LED_READY] = BIT(~data, 2);
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_9600 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_9600 )
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
	m_leds.resolve();
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

void softbox_state::softbox(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, XTAL(8'000'000)/2);
	m_maincpu->set_addrmap(AS_PROGRAM, &softbox_state::softbox_mem);
	m_maincpu->set_addrmap(AS_IO, &softbox_state::softbox_io);

	// devices
	i8251_device &i8251(I8251(config, I8251_TAG, 0));
	i8251.txd_handler().set(RS232_TAG, FUNC(rs232_port_device::write_txd));
	i8251.dtr_handler().set(RS232_TAG, FUNC(rs232_port_device::write_dtr));
	i8251.rts_handler().set(RS232_TAG, FUNC(rs232_port_device::write_rts));

	rs232_port_device &rs232(RS232_PORT(config, RS232_TAG, default_rs232_devices, "terminal"));
	rs232.rxd_handler().set(I8251_TAG, FUNC(i8251_device::write_rxd));
	rs232.dsr_handler().set(I8251_TAG, FUNC(i8251_device::write_dsr));
	rs232.cts_handler().set(I8251_TAG, FUNC(i8251_device::write_cts));
	rs232.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	i8255_device &ppi0(I8255A(config, I8255_0_TAG));
	ppi0.in_pa_callback().set(FUNC(softbox_state::ppi0_pa_r));
	ppi0.out_pb_callback().set(FUNC(softbox_state::ppi0_pb_w));
	ppi0.in_pc_callback().set_ioport("SW1");

	i8255_device &ppi1(I8255A(config, I8255_1_TAG));
	ppi1.in_pa_callback().set(FUNC(softbox_state::ppi1_pa_r));
	ppi1.out_pb_callback().set(FUNC(softbox_state::ppi1_pb_w));
	ppi1.in_pc_callback().set(FUNC(softbox_state::ppi1_pc_r));
	ppi1.out_pc_callback().set(FUNC(softbox_state::ppi1_pc_w));

	com8116_device &dbrg(COM8116(config, COM8116_TAG, 5.0688_MHz_XTAL));
	dbrg.fr_handler().set(I8251_TAG, FUNC(i8251_device::write_rxc));
	dbrg.ft_handler().set(I8251_TAG, FUNC(i8251_device::write_txc));

	ieee488_device::add_cbm_devices(config, "c8050");

	CORVUS_HDC(config, m_hdc, 0);
	HARDDISK(config, "harddisk1", "corvus_hdd");
	HARDDISK(config, "harddisk2", "corvus_hdd");
	HARDDISK(config, "harddisk3", "corvus_hdd");
	HARDDISK(config, "harddisk4", "corvus_hdd");

	IMI7000_BUS(config, "imi7000").set_slot_default_options("imi5000h", nullptr, nullptr, nullptr);

	// software lists
	SOFTWARE_LIST(config, "flop_list").set_original("softbox");
}



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
	ROMX_LOAD( "375.ic3", 0x000, 0x800, CRC(177580e7) SHA1(af6a97495de825b80cdc9fbf72329d5440826177), ROM_BIOS(0) )
	ROMX_LOAD( "376.ic4", 0x800, 0x800, CRC(edfee5be) SHA1(5662e9071cc622a1c071d89b00272fc6ba122b9a), ROM_BIOS(0) )
	ROM_SYSTEM_BIOS( 1, "19811027", "27-Oct-81" )
	ROMX_LOAD( "379.ic3", 0x000, 0x800, CRC(7b5a737c) SHA1(2348590884b026b7647f6864af8c9ba1c6f8746b), ROM_BIOS(1) )
	ROMX_LOAD( "380.ic4", 0x800, 0x800, CRC(65a13029) SHA1(46de02e6f04be298047efeb412e00a5714dc21b3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 2, "19830609", "09-June-1983" )
	ROMX_LOAD( "389.ic3", 0x000, 0x800, CRC(d66e581a) SHA1(2403e25c140c41b0e6d6975d39c9cd9d6f335048), ROM_BIOS(2) )
	ROMX_LOAD( "390.ic4", 0x800, 0x800, CRC(abe6cb30) SHA1(4b26d5db36f828e01268f718799f145d09b449ad), ROM_BIOS(2) )
ROM_END

} // anonymous namespace

//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME     PARENT  COMPAT  MACHINE  INPUT    CLASS          INIT        COMPANY                      FULLNAME   FLAGS
COMP( 1981, softbox, 0,      0,      softbox, softbox, softbox_state, empty_init, "Small Systems Engineering", "SoftBox", MACHINE_NO_SOUND_HW )
