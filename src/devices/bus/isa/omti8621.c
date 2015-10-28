// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont
/*
 * omti8621.c - SMS OMTI 8621 disk controller (for Apollo DN3x00)
 *
 *  Created on: August 30, 2010
 *      Author: Hans Ostermeyer
 *
 *  Converted to ISA device by R. Belmont
 *
 *  see also:
 *  * http://www.bitsavers.org/pdf/sms/pc/OMTI_AT_Controller_Series_Jan87.pdf
 */

#define VERBOSE 0

static int verbose = VERBOSE;

#include "omti8621.h"
#include "image.h"
#include "formats/pc_dsk.h"
#include "formats/naslite_dsk.h"
#include "formats/apollo_dsk.h"

#define LOG(x)  { logerror ("%s: ", cpu_context(this)); logerror x; logerror ("\n"); }
#define LOG1(x) { if (verbose > 0) LOG(x)}
#define LOG2(x) { if (verbose > 1) LOG(x)}
#define LOG3(x) { if (verbose > 2) LOG(x)}

#define OMTI_DISK_SECTOR_SIZE 1056

#define OMTI_DISK_TYPE_155_MB 0x607 // Micropolis 1355 (170 MB Dtype = 607)
#define OMTI_DISK_TYPE_348_MB 0x604 // Maxtor EXT-4380-E (380 MB Dtype = 604)
#define OMTI_DISK_TYPE_DEFAULT OMTI_DISK_TYPE_348_MB // new disks will have this type (and size)

#define OMTI_MAX_BLOCK_COUNT 32

#define OMTI_DISK0_TAG "omti_disk0"
#define OMTI_DISK1_TAG "omti_disk1"

#define OMTI_FDC_TAG "omti_fdc"

#define OMTI_CPU_REGION "omti_cpu"
#define OMTI_BIOS_REGION "omti_bios"

// forward declaration of image class
extern const device_type OMTI_DISK;

class omti_disk_image_device :  public device_t,
								public device_image_interface
{
public:
	// construction/destruction
	omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// image-level overrides
	virtual iodevice_t image_type() const { return IO_HARDDISK; }

	virtual bool is_readable()  const { return 1; }
	virtual bool is_writeable() const { return 1; }
	virtual bool is_creatable() const { return 1; }
	virtual bool must_be_loaded() const { return 0; }
	virtual bool is_reset_on_load() const { return 0; }
	virtual const char *image_interface() const { return NULL; }
	virtual const char *file_extensions() const { return "awd"; }
	virtual const option_guide *create_option_guide() const { return NULL; }

	virtual bool call_create(int format_type, option_resolution *format_options);
protected:
	// device-level overrides
	virtual void device_config_complete();
	virtual void device_start();
	virtual void device_reset();

	void omti_disk_config(UINT16 disk_type);
public:
	UINT16 m_type;
	UINT16 m_cylinders;
	UINT16 m_heads;
	UINT16 m_sectors;
	UINT32 m_sectorbytes;
	UINT32 m_sector_count;

	device_image_interface *m_image;

	// configuration data
	UINT8 m_config_data[10];

	// ESDI defect list data
	UINT8 m_esdi_defect_list[256];
};

/*
 * I/O register offsets
 */

#define OMTI_PORT_DATA_IN    0x00    /* read, 8-bit */
#define OMTI_PORT_DATA_OUT   0x00    /* write, 8-bit */
#define OMTI_PORT_STATUS     0x01    /* read, 8-bit */
#define OMTI_PORT_RESET      0x01    /* write, 8-bit */
#define OMTI_PORT_CONFIG     0x02    /* read, 8-bit */
#define OMTI_PORT_SELECT     0x02    /* write, 8-bit */
#define OMTI_PORT_MASK       0x03    /* write only, 8-bit */

// port status

#define OMTI_STATUS_REQ  0x01 // Request (1 = request transfer of data via data in/out register)
#define OMTI_STATUS_IO   0x02 // In/Out (1 = direction of transfer is from controller to host)
#define OMTI_STATUS_CD   0x04 // Command/Data ( 1 = byte transfered is command or status byte)
#define OMTI_STATUS_BUSY 0x08 // Busy (0 = controller is idle, 1 = controller selected)
#define OMTI_STATUS_DREQ 0x10 // Data Request (0 = no DMA request, 1 = DMA cycle requested)
#define OMTI_STATUS_IREQ 0x20 // Interrupt Request (0 = no interrupt, 1 = command complete)
#define OMTI_STATUS_NU6  0x40 // not used
#define OMTI_STATUS_NU7  0x80 // not used

#define OMTI_CONFIG_W23  0x01 // jumper W23
#define OMTI_CONFIG_W22  0x02 // jumper W22
#define OMTI_CONFIG_W21  0x04 // jumper W21
#define OMTI_CONFIG_W20  0x08 // jumper W20

#define OMTI_MASK_DMAE  0x01 // DMA enable
#define OMTI_MASK_INTE  0x02 // Interrupt enable

#define OMTI_COMMAND_STATUS_ERROR 0x02 // error bit
#define OMTI_COMMAND_STATUS_LUN 0x20 // drive 0 is 0

#define OMTI_SENSE_CODE_NO_ERROR 0x00
#define OMTI_SENSE_CODE_DRIVE_NOT_READY 0x04
#define OMTI_SENSE_CODE_ADDRESS_VALID 0x80
#define OMTI_SENSE_CODE_SECTOR_NOT_FOUND 0x14
#define OMTI_SENSE_CODE_ECC_ERROR 0x11
#define OMTI_SENSE_CODE_BAD_TRACK 0x19
#define OMTI_SENSE_CODE_ALTERNATE_TRACK 0x1C
#define OMTI_SENSE_CODE_INVALID_COMMAND 0x20
#define OMTI_SENSE_CODE_ILLEGAL_ADDRESS 0x21

enum {
	OMTI_STATE_RESET,
	OMTI_STATE_IDLE,
	OMTI_STATE_SELECTION,
	OMTI_STATE_COMMAND,
	OMTI_STATE_DATA,
	OMTI_STATE_STATUS
};

// OMTI commands

#define OMTI_CMD_TEST_DRIVE_READY 0x00
#define OMTI_CMD_RECALIBRATE 0x01

#define OMTI_CMD_REQUEST_SENSE 0x03
#define OMTI_CMD_FORMAT_DRIVEUNIT 0x04
#define OMTI_CMD_READ_VERIFY 0x05
#define OMTI_CMD_FORMAT_TRACK 0x06
#define OMTI_CMD_FORMAT_BAD_TRACK 0x07
#define OMTI_CMD_READ 0x08

#define OMTI_CMD_WRITE 0x0a
#define OMTI_CMD_SEEK 0x0b

#define OMTI_CMD_READ_SECTOR_BUFFER 0x0e
#define OMTI_CMD_WRITE_SECTOR_BUFFER 0x0f

#define OMTI_CMD_ASSIGN_ALTERNATE_TRACK 0x11

#define OMTI_CMD_READ_DATA_TO_BUFFER 0x1e
#define OMTI_CMD_WRITE_DATA_FROM_BUFFER 0x1f
#define OMTI_CMD_COPY 0x20

#define OMTI_CMD_READ_ESDI_DEFECT_LIST 0x37

#define OMTI_CMD_RAM_DIAGNOSTICS 0xe0
#define OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC 0xe4
#define OMTI_CMD_READ_LONG 0xe5
#define OMTI_CMD_WRITE_LONG 0xe6

#define OMTI_CMD_READ_CONFIGURATION 0xec
#define OMTI_CMD_INVALID_COMMAND 0xff

/***************************************************************************
 cpu_context - return a string describing the current CPU context
 ***************************************************************************/

static const char *cpu_context(const device_t *device) {
	static char statebuf[64]; /* string buffer containing state description */

	device_t *cpu = device->machine().firstcpu;

	/* if we have an executing CPU, output data */
	if (cpu != NULL) {
		osd_ticks_t t = osd_ticks();
		int s = t / osd_ticks_per_second();
		int ms = (t % osd_ticks_per_second()) / 1000;

		sprintf(statebuf, "%d.%03d %s pc=%08x - %s", s, ms, cpu->tag(),
				cpu->safe_pcbase(), device->tag());
	} else {
		strcpy(statebuf, "(no context)");
	}
	return statebuf;
}

static SLOT_INTERFACE_START( pc_hd_floppies )
	SLOT_INTERFACE( "525hd", FLOPPY_525_HD )
	SLOT_INTERFACE( "35hd", FLOPPY_35_HD )
	SLOT_INTERFACE( "525dd", FLOPPY_525_DD )
	SLOT_INTERFACE( "35dd", FLOPPY_35_DD )
SLOT_INTERFACE_END

MACHINE_CONFIG_FRAGMENT( omti_disk )
	MCFG_DEVICE_ADD(OMTI_DISK0_TAG, OMTI_DISK, 0)
	MCFG_DEVICE_ADD(OMTI_DISK1_TAG, OMTI_DISK, 0)

	MCFG_PC_FDC_AT_ADD(OMTI_FDC_TAG)
	MCFG_PC_FDC_INTRQ_CALLBACK(WRITELINE(omti8621_device, fdc_irq_w))
	MCFG_PC_FDC_DRQ_CALLBACK(WRITELINE(omti8621_device, fdc_drq_w))
	MCFG_FLOPPY_DRIVE_ADD(OMTI_FDC_TAG":0", pc_hd_floppies, "525hd", omti8621_device::floppy_formats)
// Apollo workstations never have more then 1 floppy drive
//  MCFG_FLOPPY_DRIVE_ADD(OMTI_FDC_TAG":1", pc_hd_floppies, "525hd", omti8621_device::floppy_formats)
MACHINE_CONFIG_END

FLOPPY_FORMATS_MEMBER( omti8621_device::floppy_formats )
	FLOPPY_APOLLO_FORMAT,
	FLOPPY_PC_FORMAT,
	FLOPPY_NASLITE_FORMAT
FLOPPY_FORMATS_END

// this card has two EPROMs: a program for the on-board Z8 CPU,
// and a PC BIOS to make the card bootable on a PC.
// we have the Z8 program, we still need the PC BIOS.
ROM_START( omti8621 )
	ROM_REGION(0x4000, OMTI_CPU_REGION, 0)  // disassembles fine as Z8 code
	ROM_LOAD( "omti_8621_102640-b.bin", 0x000000, 0x004000, CRC(e6f20dbb) SHA1(cf1990ad72eac6b296485410f5fa3309a0d6d078) )

#if 1
	// OMTI 8621 boards for Apollo workstations never use a BIOS ROM
	// They don't even have a socket for the BIOS ROM
	ROM_REGION(0x1000, OMTI_BIOS_REGION, 0)
	ROM_LOAD_OPTIONAL("omti_bios", 0x0000, 0x1000, NO_DUMP)
#endif
ROM_END

static INPUT_PORTS_START( omti_port )
	PORT_START("IO_BASE")
	PORT_DIPNAME( 0x07, 0x04, "ESDI I/O base")
	PORT_DIPSETTING(    0x00, "0320h" )
	PORT_DIPSETTING(    0x01, "0324h" )
	PORT_DIPSETTING(    0x02, "0328h" )
	PORT_DIPSETTING(    0x03, "032Ch" )
	PORT_DIPSETTING(    0x04, "01A0h" )
	PORT_DIPSETTING(    0x05, "01A4h" )
	PORT_DIPSETTING(    0x06, "01A8h" )
	PORT_DIPSETTING(    0x07, "01ACh" )
	PORT_DIPNAME( 0x08, 0x00, "Floppy I/O base")
	PORT_DIPSETTING(    0x00, "03F0h" )
	PORT_DIPSETTING(    0x01, "0370h" )

	PORT_START("BIOS_OPTS")
	PORT_DIPNAME( 0x01, 0x00, "BIOS control")
	PORT_DIPSETTING(    0x00, "Disabled" )
	PORT_DIPSETTING(    0x01, "Enabled" )
	PORT_DIPNAME( 0x02, 0x00, "BIOS base")
	PORT_DIPSETTING(    0x00, "C8000h" )
	PORT_DIPSETTING(    0x01, "CA000h" )
INPUT_PORTS_END

machine_config_constructor omti8621_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( omti_disk );
}

const rom_entry *omti8621_device::device_rom_region() const
{
	return ROM_NAME( omti8621 );
}

ioport_constructor omti8621_device::device_input_ports() const
{
	return INPUT_PORTS_NAME( omti_port );
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void omti8621_device::device_start()
{
	LOG2(("device_start"));

	set_isa_device();

	m_installed = false;

	sector_buffer.resize(OMTI_DISK_SECTOR_SIZE*OMTI_MAX_BLOCK_COUNT);

	m_timer = timer_alloc(0, NULL);

	our_disks[0] = subdevice<omti_disk_image_device>(OMTI_DISK0_TAG);
	our_disks[1] = subdevice<omti_disk_image_device>(OMTI_DISK1_TAG);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void omti8621_device::device_reset()
{
	static const int io_bases[8] = { 0x320, 0x324, 0x328, 0x32c, 0x1a0, 0x1a4, 0x1a8, 0x1ac };

	LOG2(("device_reset"));

	// you can't read I/O ports in device_start() even if they're required_ioport<> in your class!
	if (!m_installed)
	{
		int esdi_base = io_bases[m_iobase->read() & 7];

		// install the ESDI ports
		m_isa->install16_device(esdi_base, esdi_base + 7, 0, 0, read16_delegate(FUNC(omti8621_device::read), this), write16_delegate(FUNC(omti8621_device::write), this));

		// and the onboard AT FDC ports
		if (m_iobase->read() & 8)
		{
			m_isa->install_device(0x0370, 0x0377, *m_fdc, &pc_fdc_interface::map);
		}
		else
		{
			m_isa->install_device(0x03f0, 0x03f7, *m_fdc, &pc_fdc_interface::map);
		}

		m_isa->set_dma_channel(2, this, TRUE);

		m_installed = true;
	}

	set_jumper(our_disks[0]->m_type);

	// should go from reset to idle after 100 us
	// state->omti_state = OMTI_STATE_RESET;
	omti_state = OMTI_STATE_IDLE;

	status_port =  OMTI_STATUS_NU6 | OMTI_STATUS_NU7;
	config_port = ~jumper;
	mask_port = 0;

	// default the sector data buffer with model and status information
	// (i.e. set sector data buffer for cmd=0x0e READ SECTOR BUFFER)

	memset(&sector_buffer[0], 0, OMTI_DISK_SECTOR_SIZE);
	memcpy(&sector_buffer[0], "8621VB.4060487xx", 0x10);
	sector_buffer[0x10] = 0; // ROM Checksum error
	sector_buffer[0x11] = 0; // Processor Register error
	sector_buffer[0x12] = 0; // Buffer RAM error
	sector_buffer[0x13] = 0; // Sequencer Register File error
	sector_buffer[0x14] = 0xc0; // 32K buffer size
	// TODO: add missing Default values for LUN 0, 1 and 3

	command_length = 0;
	command_index = 0;
	command_status = 0;

	data_index = 0;
	data_length = 0;

	clear_sense_data();

	diskaddr_ecc_error = 0;
	diskaddr_format_bad_track = 0;
	alternate_track_address[0] = 0;
	alternate_track_address[1] = 0;
}

const device_type ISA16_OMTI8621 = &device_creator<omti8621_device>;

omti8621_device::omti8621_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, ISA16_OMTI8621, "OMTI 8621 ESDI/floppy controller", tag, owner, clock, "omti8621", __FILE__),
	device_isa16_card_interface(mconfig, *this),
	m_fdc(*this, OMTI_FDC_TAG),
	m_iobase(*this, "IO_BASE"),
	m_biosopts(*this, "BIOS_OPTS")
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void omti8621_device::device_config_complete()
{
}

/*-------------------------------------------------
 set_interrupt - update the IRQ state
 -------------------------------------------------*/

void omti8621_device::set_interrupt(enum line_state line_state)
{
	LOG2(("set_interrupt: status_port=%x, line_state %d", status_port, line_state));
	m_isa->irq14_w(line_state);
}

void omti8621_device::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	set_interrupt(ASSERT_LINE);
}

/***************************************************************************
 clear_sense_data - clear the sense data
 ***************************************************************************/

void omti8621_device::clear_sense_data() {
	LOG2(("clear_sense_data"));
	memset(sense_data, 0, sizeof(sense_data));
}

/***************************************************************************
 set_sense_data - set the sense data from code and command descriptor block
 ***************************************************************************/

void omti8621_device::set_sense_data(UINT8 code, const UINT8 * cdb) {
	LOG2(("set_sense_data code=%x", code));
	sense_data[0]=code;
	sense_data[1]=cdb[1];
	sense_data[2]=cdb[2];
	sense_data[3]=cdb[3];
}

/***************************************************************************
 set_configuration_data - set the configuration data for drive lun
 ***************************************************************************/

void omti8621_device::set_configuration_data(UINT8 lun) {
	LOG2(("set_configuration_data lun=%x", lun));

	// initialize the configuration data
	omti_disk_image_device *disk = our_disks[lun];

	disk->m_config_data[0] = (disk->m_cylinders - 1) >> 8; // Number of Cylinders (MSB)
	disk->m_config_data[1] = (disk->m_cylinders - 1) & 0xff; // Number of Cylinders (LSB) (-1)
	disk->m_config_data[2] = disk->m_heads - 1; // Number of Heads (-1)
	disk->m_config_data[3] = disk->m_sectors - 1; // Number of Sectors (-1)
	disk->m_config_data[4] = 0x02; // Drive Configuration Word (MSB)
	disk->m_config_data[5] = 0x44; // Drive Configuration Word (LSB)
	disk->m_config_data[6] = 0x00; // ISG AFTER INDEX
	disk->m_config_data[7] = 0x00; // PLO SYN Field (ID)
	disk->m_config_data[8] = 0x00; // PLO SYN Field (DATA)
	disk->m_config_data[9] = 0x00; // ISG AFTER SECTOR
}

/***************************************************************************
 get_lun - get logical unit number from a command descriptor block (in bit 5)
 ***************************************************************************/

UINT8 omti8621_device::get_lun(const UINT8 * cdb)
{
	return   (cdb[1] & 0x20) >> 5;
}

/***************************************************************************
 check_disk_address - check disk address, set sense data and return true for no error
 ***************************************************************************/

UINT8 omti8621_device::check_disk_address(const UINT8 *cdb)
{
	UINT8 sense_code = OMTI_SENSE_CODE_NO_ERROR;
	UINT8 lun = get_lun(cdb);
	UINT16 head = cdb[1] & 0x1f;
	UINT16 sector = cdb[2] & 0x3f;
	UINT32 cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	UINT8 block_count = cdb[4];
	omti_disk_image_device *disk = our_disks[lun];

	UINT32 disk_track = cylinder * disk->m_heads + head;
	UINT32 disk_addr = (disk_track * disk->m_sectors) + sector;

	if (block_count > OMTI_MAX_BLOCK_COUNT) {
		LOG(("########### check_disk_address: unexpected block count %x", block_count));
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	}

	if (lun > OMTI_MAX_LUN) {
		sense_code = OMTI_SENSE_CODE_DRIVE_NOT_READY;
	} else  if (!disk->m_image->exists()) {
		sense_code = OMTI_SENSE_CODE_DRIVE_NOT_READY;
	} else  if (sector >= OMTI_MAX_BLOCK_COUNT) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if (head >= disk->m_heads) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if (cylinder >= disk->m_cylinders) {
		sense_code = OMTI_SENSE_CODE_ILLEGAL_ADDRESS | OMTI_SENSE_CODE_ADDRESS_VALID;
	} else if ( disk_track == diskaddr_format_bad_track && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_BAD_TRACK;
	} else if (disk_addr == diskaddr_ecc_error && disk_addr != 0) {
		sense_code = OMTI_SENSE_CODE_ECC_ERROR;
	} else if (disk_track == alternate_track_address[1] && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_ALTERNATE_TRACK;
	}

	if (sense_code == OMTI_SENSE_CODE_NO_ERROR) {
		clear_sense_data();
	} else {
		command_status |= OMTI_COMMAND_STATUS_ERROR;
		set_sense_data(sense_code, cdb);
	}
	return sense_code == OMTI_SENSE_CODE_NO_ERROR;
}

/***************************************************************************
 get_disk_track - get disk track from a command descriptor block
 ***************************************************************************/

UINT32 omti8621_device::get_disk_track(const UINT8 * cdb) {
	UINT8 lun = get_lun(cdb);
	UINT16 head = cdb[1] & 0x1f;
	UINT32 cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	return cylinder * our_disks[lun]->m_heads + head;
}

/***************************************************************************
 get_disk_address - get disk address from a command descriptor block
 ***************************************************************************/

UINT32 omti8621_device::get_disk_address(const UINT8 * cdb) {
	UINT8 lun = get_lun(cdb);
	UINT16 sector = cdb[2] & 0x3f;
	return get_disk_track(cdb) * our_disks[lun]->m_sectors + sector;
}

/***************************************************************************
 set_data_transfer - setup for data transfer from/to data
 ***************************************************************************/

void omti8621_device::set_data_transfer(UINT8 *data, UINT16 length)
{
	// set controller for read data transfer
	omti_state = OMTI_STATE_DATA;
	status_port |= OMTI_STATUS_REQ | OMTI_STATUS_IO | OMTI_STATUS_BUSY;
	status_port &= ~OMTI_STATUS_CD;

	data_buffer = data;
	data_length = length;
	data_index = 0;
}

/***************************************************************************
 read_sectors_from_disk - read sectors starting at diskaddr into sector_buffer
 ***************************************************************************/

void omti8621_device::read_sectors_from_disk(INT32 diskaddr, UINT8 count, UINT8 lun)
{
	UINT8 *data_buffer = &sector_buffer[0];
	device_image_interface *image = our_disks[lun]->m_image;

	while (count-- > 0) {
		LOG2(("read_sectors_from_disk lun=%d diskaddr=%x", lun, diskaddr));

		image->fseek( diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread( data_buffer, OMTI_DISK_SECTOR_SIZE);

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 write_sectors_to_disk - write sectors starting at diskaddr from sector_buffer
 ***************************************************************************/

void omti8621_device::write_sectors_to_disk(INT32 diskaddr, UINT8 count, UINT8 lun)
{
	UINT8 *data_buffer = &sector_buffer[0];
	device_image_interface *image = our_disks[lun]->m_image;

	while (count-- > 0) {
		LOG2(("write_sectors_to_disk lun=%d diskaddr=%x", lun, diskaddr));

		image->fseek( diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite( data_buffer, OMTI_DISK_SECTOR_SIZE);

		if (diskaddr == diskaddr_ecc_error) {
			// reset previous ECC error
			diskaddr_ecc_error = 0;
		}

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 copy_sectors - copy sectors
 ***************************************************************************/

void omti8621_device::copy_sectors(INT32 dst_addr, INT32 src_addr, UINT8 count, UINT8 lun)
{
	device_image_interface *image = our_disks[lun]->m_image;

	LOG2(("copy_sectors lun=%d src_addr=%x dst_addr=%x count=%x", lun, src_addr, dst_addr, count));

	while (count-- > 0) {
		image->fseek( src_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread( &sector_buffer[0], OMTI_DISK_SECTOR_SIZE);

		image->fseek( dst_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite( &sector_buffer[0], OMTI_DISK_SECTOR_SIZE);

		if (dst_addr == diskaddr_ecc_error) {
			// reset previous ECC error
			diskaddr_ecc_error = 0;
		}

		src_addr++;
		dst_addr++;
	}
}

/***************************************************************************
 format track - format a track
 ***************************************************************************/

void omti8621_device::format_track(const UINT8 * cdb)
{
	UINT8 lun = get_lun(cdb);
	UINT32 disk_addr = get_disk_address(cdb);
	UINT32 disk_track = get_disk_track(cdb);

	if (diskaddr_ecc_error == disk_addr) {
		// reset previous ECC error
		diskaddr_ecc_error = 0;
	}

	if (diskaddr_format_bad_track == disk_track) {
		// reset previous bad track formatting
		diskaddr_format_bad_track = 0;
	}

	if (alternate_track_address[0] == disk_track) {
		// reset source of alternate track address
		alternate_track_address[0] = 0;
	}

	if (alternate_track_address[1] == disk_track) {
		// reset alternate track address
		alternate_track_address[1] = 0;
	}

	if (check_disk_address(cdb) ) {
		if ((cdb[5] & 0x40) == 0) {
			memset(&sector_buffer[0], 0x6C, OMTI_DISK_SECTOR_SIZE * our_disks[lun]->m_sectors);
		}
		write_sectors_to_disk(disk_addr, our_disks[lun]->m_sectors, lun);
	}

}

/***************************************************************************
 set_esdi_defect_list - setup the (emty) ESDI defect list
 ***************************************************************************/

void omti8621_device::set_esdi_defect_list(UINT8 lun, UINT8 head)
{
	omti_disk_image_device *disk = our_disks[lun];

	memset(disk->m_esdi_defect_list, 0, sizeof(disk->m_esdi_defect_list));
	disk->m_esdi_defect_list[0] = 1; // month
	disk->m_esdi_defect_list[1] = 1; // day
	disk->m_esdi_defect_list[2] = 90; // year
	disk->m_esdi_defect_list[3] = head;
	memset(disk->m_esdi_defect_list+6, 0xff, 5); // end of defect list
}

/***************************************************************************
 log_command - log command from a command descriptor block
 ***************************************************************************/

void omti8621_device::log_command(const UINT8 cdb[], const UINT16 cdb_length)
{
	if (verbose > 0) {
		int i;
		logerror("%s: OMTI command ", cpu_context(this));
		switch (cdb[0]) {
		case OMTI_CMD_TEST_DRIVE_READY: // 0x00
			logerror("Test Drive Ready");
			break;
		case OMTI_CMD_RECALIBRATE: // 0x01
			logerror("Recalibrate");
			break;
		case OMTI_CMD_REQUEST_SENSE: // 0x03
			logerror("Request Sense");
			break;
		case OMTI_CMD_READ_VERIFY: // 0x05
			logerror("Read Verify");
			break;
		case OMTI_CMD_FORMAT_TRACK: // 0x06
			logerror("Format Track");
			break;
		case OMTI_CMD_FORMAT_BAD_TRACK: // 0x07
			logerror("Format Bad Track");
			break;
		case OMTI_CMD_READ: // 0x08
			logerror("Read");
			break;
		case OMTI_CMD_WRITE: // 0x0A
			logerror("Write");
			break;
		case OMTI_CMD_SEEK: // 0x0B
			logerror("Seek");
			break;
		case OMTI_CMD_READ_SECTOR_BUFFER: // 0x0E
			logerror("Read Sector Buffer");
			break;
		case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
			logerror("Write Sector Buffer");
			break;
		case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
			logerror("Assign Alternate Track");
			break;
		case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
			logerror("Read Data to Buffer");
			break;
		case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
			logerror("Write Data from Buffer");
			break;
		case OMTI_CMD_COPY: // 0x20
			logerror("Copy");
			break;
		case OMTI_CMD_READ_ESDI_DEFECT_LIST: // 0x37
			logerror("Read ESDI Defect List");
			break;
		case OMTI_CMD_RAM_DIAGNOSTICS: // 0xE0
			logerror("RAM. Diagnostic");
			break;
		case OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC: // 0xE4
			logerror("Controller Int. Diagnostic");
			break;
		case OMTI_CMD_READ_LONG: // 0xE5
			logerror("Read Long");
			break;
		case OMTI_CMD_WRITE_LONG: // 0xE6
			logerror("Write Long");
			break;
		case OMTI_CMD_READ_CONFIGURATION: // 0xEC
			logerror("Read Configuration");
			break;
		case OMTI_CMD_INVALID_COMMAND: // 0xFF
			logerror("Invalid Command");
			break;
		default:
			logerror("!!! Unexpected Command !!!");
		}
//      logerror(" (%02x, length=%02x)", cdb[0], cdb_length);
		for (i = 0; i < cdb_length; i++) {
			logerror(" %02x", cdb[i]);
		}

		switch (cdb[0]) {
		case OMTI_CMD_READ_VERIFY: // 0x05
		case OMTI_CMD_READ: // 0x08
		case OMTI_CMD_WRITE: // 0x0a
		case OMTI_CMD_SEEK: // 0x0b
		case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
		case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
		case OMTI_CMD_COPY: // 0x20
			logerror(" (diskaddr=%x count=%x)", get_disk_address(cdb), cdb[4]);
			break;
		}
		logerror("\n");
	}
}

/***************************************************************************
 log_data - log data in the common data buffer
 ***************************************************************************/

void omti8621_device::log_data()
{
	if (verbose > 0) {
		int i;
		logerror("%s: OMTI data (length=%02x)", cpu_context(this),
				data_length);
		for (i = 0; i < data_length && i < OMTI_DISK_SECTOR_SIZE; i++) {
			logerror(" %02x", data_buffer[i]);
		}

		if (i < data_length) {
			logerror(" ...");
		}
		logerror("\n");
	}
}

/***************************************************************************
 do_command
 ***************************************************************************/

void omti8621_device::do_command(const UINT8 cdb[], const UINT16 cdb_length)
{
	UINT8 lun = get_lun(cdb);
	omti_disk_image_device *disk = our_disks[lun];
	int command_duration = 0; // ms

	log_command( cdb, cdb_length);

	// default to read status and status is successful completion
	omti_state = OMTI_STATE_STATUS;
	status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
	command_status = lun ? OMTI_COMMAND_STATUS_LUN : 0;

	if (mask_port & OMTI_MASK_INTE) {
		set_interrupt(CLEAR_LINE);
	}

	if (!disk->m_image->exists()) {
		command_status |= OMTI_COMMAND_STATUS_ERROR; // no such drive
	}

	switch (cdb[0]) {
	case OMTI_CMD_TEST_DRIVE_READY: // 0x00
		if (!disk->m_image->exists())
		{
			set_sense_data(OMTI_SENSE_CODE_DRIVE_NOT_READY, cdb);
		}
		break;

	case OMTI_CMD_RECALIBRATE: // 0x01
		break;

	case OMTI_CMD_REQUEST_SENSE: // 0x03
		set_data_transfer(sense_data, sizeof(sense_data));
		break;

	case OMTI_CMD_READ_VERIFY: // 0x05
		check_disk_address(cdb);
		break;

	case OMTI_CMD_FORMAT_TRACK: // 0x06
		format_track(cdb);
		break;

	case OMTI_CMD_FORMAT_BAD_TRACK: // 0x07
		diskaddr_format_bad_track = get_disk_address(cdb);
		break;

	case OMTI_CMD_READ: // 0x08
		if (check_disk_address(cdb)) {
			// read data from controller
			read_sectors_from_disk(get_disk_address(cdb), cdb[4], lun);
			set_data_transfer(&sector_buffer[0],  OMTI_DISK_SECTOR_SIZE*cdb[4]);
		}
		break;

	case OMTI_CMD_WRITE: // 0x0A
		log_data();
		if (check_disk_address(cdb)) {
			write_sectors_to_disk(get_disk_address(cdb), cdb[4], lun);
		}
		break;

	case OMTI_CMD_SEEK: // 0x0B
		check_disk_address(cdb);
		break;

	case OMTI_CMD_READ_SECTOR_BUFFER: // 0x0E
		set_data_transfer(&sector_buffer[0], OMTI_DISK_SECTOR_SIZE*cdb[4]);
		break;

	case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
		log_data();
		break;

	case OMTI_CMD_COPY: // 0x20
		if (check_disk_address(cdb) && check_disk_address(cdb+4)) {
			// copy sectors
			copy_sectors (get_disk_address(cdb+4), get_disk_address(cdb), cdb[4], lun);
		}
		break;

	case OMTI_CMD_READ_ESDI_DEFECT_LIST: // 0x37
		set_esdi_defect_list(get_lun(cdb), cdb[1] & 0x1f);
		set_data_transfer(disk->m_esdi_defect_list, sizeof(disk->m_esdi_defect_list));
		break;

#if 0   // this command seems unused by Domain/OS, and it's unclear what the intent of the code is (it makes some versions of GCC quite unhappy)
	case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
		log_data();
		alternate_track_address[0] = get_disk_track(cdb);
		alternate_track_address[1] = get_disk_track(alternate_track_buffer-1);
		break;
#endif

	case OMTI_CMD_READ_DATA_TO_BUFFER: // 0x1E
		if (check_disk_address(cdb)) {
			// read data from controller
			read_sectors_from_disk (get_disk_address(cdb), cdb[4], lun);
			// Domain/OS doesn't expect zero access time
			command_duration += 1; // 1 ms is enough, average time would be 30 ms)
		}
		break;

	case OMTI_CMD_WRITE_DATA_FROM_BUFFER: // 0x1F
		log_data();
		if (check_disk_address(cdb)) {
			write_sectors_to_disk(get_disk_address(cdb), cdb[4], lun);
		}
		break;

	case  OMTI_CMD_RAM_DIAGNOSTICS: // 0xE0
		break;

	case OMTI_CMD_CONTROLLER_INT_DIAGNOSTIC: // 0xE4
		break;

	case OMTI_CMD_READ_LONG: // 0xE5
		if (check_disk_address(cdb)) {
			// read data from controller
			read_sectors_from_disk(get_disk_address(cdb), cdb[4], lun);
			set_data_transfer(&sector_buffer[0], OMTI_DISK_SECTOR_SIZE+6);
		}
		break;

	case OMTI_CMD_WRITE_LONG: // 0xE6
		log_data();
		if (check_disk_address(cdb)) {
			UINT32 diskaddr =  get_disk_address(cdb);
			write_sectors_to_disk(diskaddr, cdb[4], lun);
			// this will spoil the ECC code
			diskaddr_ecc_error = diskaddr;
		}
		break;

	case OMTI_CMD_READ_CONFIGURATION: // 0xEC
		set_configuration_data(get_lun(cdb));
		set_data_transfer(disk->m_config_data, sizeof(disk->m_config_data));
		break;

	case OMTI_CMD_INVALID_COMMAND: // 0xFF
		set_sense_data(OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;

	default:
		LOG(("do_command: UNEXPECTED command %02x",cdb[0]));
		set_sense_data(OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;
	}

	if (mask_port & OMTI_MASK_INTE) {
//      if (omti_state != OMTI_STATE_STATUS) {
//          LOG(("do_command: UNEXPECTED omti_state %02x",omti_state));
//      }
		status_port |= OMTI_STATUS_IREQ;
		if (command_duration == 0)
		{
			set_interrupt(ASSERT_LINE);
		}
		else
		{
			// FIXME: should delay omti_state and status_port as well
			m_timer->adjust(attotime::from_msec(command_duration), 0);
		}
	}
}

/***************************************************************************
 get_command_length
 ***************************************************************************/

UINT8 omti8621_device::get_command_length(UINT8 command_byte)
{
	return command_byte == OMTI_CMD_COPY ? 10 : 6;
}

/***************************************************************************
 get_data
 ***************************************************************************/

UINT16 omti8621_device::get_data()
{
	UINT16 data = 0xff;
	if (data_index < data_length) {
		data = data_buffer[data_index++];
		data |= data_buffer[data_index++] << 8;
		if (data_index >= data_length) {
			omti_state = OMTI_STATE_STATUS;
			status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
			log_data();
		}
	} else {
		LOG(("UNEXPECTED reading OMTI 8621 data (buffer length exceeded)"));
	}
	return data;
}

/***************************************************************************
 set_data
 ***************************************************************************/

void omti8621_device::set_data(UINT16 data)
{
	if (data_index < data_length) {
		data_buffer[data_index++] = data & 0xff;
		data_buffer[data_index++] = data >> 8;
		if (data_index >= data_length) {
			do_command(command_buffer, command_index);
		}
	} else {
		LOG(("UNEXPECTED writing OMTI 8621 data (buffer length exceeded)"));
	}
}

/***************************************************************************
 OMTI8621 Disk Controller-AT Registers
***************************************************************************/

WRITE16_MEMBER(omti8621_device::write)
{
	switch (mem_mask)
	{
		case 0x00ff:
			write8(space, offset*2, data, mem_mask);
			break;

		case 0xff00:
			write8(space, offset*2+1, data>>8, mem_mask>>8);
			break;

		default:
			set_data(data);
			break;
	}
}

WRITE8_MEMBER(omti8621_device::write8)
{
	switch (offset)
	{
	case OMTI_PORT_DATA_OUT: //  0x00
		switch (omti_state) {
		case OMTI_STATE_COMMAND:
			LOG2(("writing OMTI 8621 Command Register at offset %02x = %02x", offset, data));
			if (command_index == 0) {
				command_length = get_command_length(data);
			}

			if (command_index < command_length) {
				command_buffer[command_index++] = data;
			} else {
				LOG(("UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (command length exceeded)", offset, data));
			}

			if (command_index == command_length) {
				switch (command_buffer[0]) {
				case OMTI_CMD_WRITE: // 0x0A
					// TODO: check diskaddr
					// Fall through
				case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
					set_data_transfer(&sector_buffer[0],
							OMTI_DISK_SECTOR_SIZE * command_buffer[4]);
					status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
					set_data_transfer(alternate_track_buffer, sizeof(alternate_track_buffer));
					status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_WRITE_LONG: // 0xE6
					// TODO: check diskaddr
					set_data_transfer(&sector_buffer[0],
							(OMTI_DISK_SECTOR_SIZE +6) * command_buffer[4]);
					status_port &= ~OMTI_STATUS_IO;
					break;

				default:
					do_command(command_buffer, command_index);
					break;
				}
			}
			break;

		case OMTI_STATE_DATA:
			LOG(("UNEXPECTED: writing OMTI 8621 Data Register at offset %02x = %02x", offset, data));
			break;

		default:
			LOG(("UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (omti state = %02x)", offset, data, omti_state));
			break;
		}
		break;

	case OMTI_PORT_RESET: // 0x01
		LOG2(("writing OMTI 8621 Reset Register at offset %02x = %02x", offset, data));
		device_reset();
		break;

	case OMTI_PORT_SELECT: // 0x02
		LOG2(("writing OMTI 8621 Select Register at offset %02x = %02x (omti state = %02x)", offset, data, omti_state));
		omti_state = OMTI_STATE_COMMAND;

		status_port |= OMTI_STATUS_BUSY | OMTI_STATUS_REQ | OMTI_STATUS_CD;
		status_port &= ~OMTI_STATUS_IO;

		command_status = 0;
		command_index = 0;
		break;

	case OMTI_PORT_MASK: // 0x03
		LOG2(("writing OMTI 8621 Mask Register at offset %02x = %02x", offset, data));
		mask_port = data;

		if ((data & OMTI_MASK_INTE) == 0) {
			status_port &= ~OMTI_STATUS_IREQ;
			set_interrupt(CLEAR_LINE);
		}

		if ((data & OMTI_MASK_DMAE) == 0) {
			status_port &= ~OMTI_STATUS_DREQ;
		}
		break;

	default:
		LOG(("UNEXPECTED writing OMTI 8621 Register at offset %02x = %02x", offset, data));
		break;
	}
}

READ16_MEMBER(omti8621_device::read)
{
	switch (mem_mask)
	{
		case 0x00ff:
			return read8(space, offset*2, mem_mask);
		case 0xff00:
			return read8(space, offset*2+1, mem_mask >> 8) << 8;
		default:
			return get_data();
	}
}

READ8_MEMBER(omti8621_device::read8)
{
	UINT8 data = 0xff;
	static UINT8 last_data = 0xff;

	switch (offset) {
	case OMTI_PORT_DATA_IN: // 0x00
		if (status_port & OMTI_STATUS_CD)
		{
			data = command_status;
			switch (omti_state)
			{
			case OMTI_STATE_COMMAND:
				LOG2(("reading OMTI 8621 Data Status Register 1 at offset %02x = %02x (omti state = %02x)", offset, data, omti_state));
				break;
			case OMTI_STATE_STATUS:
				omti_state = OMTI_STATE_IDLE;
				status_port &= ~(OMTI_STATUS_BUSY | OMTI_STATUS_CD  | OMTI_STATUS_IO | OMTI_STATUS_REQ);
				LOG2(("reading OMTI 8621 Data Status Register 2 at offset %02x = %02x", offset, data));
				break;
			default:
				LOG(("UNEXPECTED reading OMTI 8621 Data Status Register 3 at offset %02x = %02x (omti state = %02x)", offset, data, omti_state));
				break;
			}
		}
		else
		{
			LOG(("UNEXPECTED reading OMTI 8621 Data Register 4 at offset %02x = %02x (status bit C/D = 0)", offset, data));
		}
		break;

	case OMTI_PORT_STATUS: // 0x01
		data = status_port;
		// omit excessive logging
		if (data != last_data)
		{
			LOG2(("reading OMTI 8621 Status Register 5 at offset %02x = %02x", offset, data));
//          last_data = data;
		}
		break;

	case OMTI_PORT_CONFIG: // 0x02
		data = config_port;
		LOG2(("reading OMTI 8621 Configuration Register at offset %02x = %02x", offset, data));
		break;

	case OMTI_PORT_MASK: // 0x03
		data = mask_port ;
		// win.dex will update the mask register with read-modify-write
		// LOG2(("reading OMTI 8621 Mask Register at offset %02x = %02x (UNEXPECTED!)", offset, data));
		break;

	default:
		LOG(("UNEXPECTED reading OMTI 8621 Register at offset %02x = %02x", offset, data));
		break;
	}

	return data;
}

void omti8621_device::set_verbose(int on_off)
{
	verbose = on_off == 0 ? 0 : VERBOSE > 1 ? VERBOSE : 1;
}

/***************************************************************************
 get_sector - get sector diskaddr of logical unit lun into data_buffer
 ***************************************************************************/

// FIXME: this will work, but is not supported by MESS
#if 0 // APOLLO_XXL
UINT32 omti8621_device::get_sector(INT32 diskaddr, UINT8 *data_buffer, UINT32 length, UINT8 lun)
{
	omti_disk_image_device *disk = omti8621_device_1->our_disks[lun];

	if (disk == NULL || disk->m_image == NULL || !disk->m_image->exists())
	{
		return 0;
	}
	else
	{
//      LOG1(("omti8621_get_sector %x on lun %d", diskaddr, lun));

		// restrict length to size of 1 sector (i.e. 1024 Byte)
		length = length < OMTI_DISK_SECTOR_SIZE ? length  : OMTI_DISK_SECTOR_SIZE;

		disk->m_image->fseek(diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		disk->m_image->fread(data_buffer, length);

		return length;
	}
}
#endif

/***************************************************************************
 omti_set_jumper - set OMI jumpers
 ***************************************************************************/

void omti8621_device::set_jumper(UINT16 disk_type)
{
	LOG1(("set_jumper: disk type=%x", disk_type));

	switch (disk_type)
	{
	case OMTI_DISK_TYPE_348_MB: // Maxtor 380 MB (348-MB FA formatted)
		jumper = OMTI_CONFIG_W22 | OMTI_CONFIG_W23;
		break;

	case OMTI_DISK_TYPE_155_MB: // Micropolis 170 MB (155-MB formatted)
	default:
		jumper = OMTI_CONFIG_W20;
		break;
	}
}

// FDC uses the standard IRQ 6 / DMA 2, doesn't appear to be configurable
WRITE_LINE_MEMBER( omti8621_device::fdc_irq_w )
{
	m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

WRITE_LINE_MEMBER( omti8621_device::fdc_drq_w )
{
	m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

UINT8 omti8621_device::dack_r(int line)
{
	return m_fdc->dma_r();
}

void omti8621_device::dack_w(int line, UINT8 data)
{
	return m_fdc->dma_w(data);
}

void omti8621_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}

//##########################################################################

// device type definition
const device_type OMTI_DISK = &device_creator<omti_disk_image_device>;

omti_disk_image_device::omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, OMTI_DISK, "OMTI8621 ESDI disk", tag, owner, clock, "omti_disk_image", __FILE__),
		device_image_interface(mconfig, *this)
{
}

void omti_disk_image_device::device_config_complete()
{
	update_names(OMTI_DISK, "disk", "disk");
}


/***************************************************************************
 omti_disk_config - configure disk parameters
 ***************************************************************************/

void omti_disk_image_device::omti_disk_config(UINT16 disk_type)
{
	LOG1(("omti_disk_config: configuring disk with type %x", disk_type));

	switch (disk_type)
	{
	case OMTI_DISK_TYPE_348_MB: // Maxtor 380 MB (348-MB FA formatted)
		m_cylinders = 1223;
		m_heads = 15;
		m_sectors = 18;
		break;

	case OMTI_DISK_TYPE_155_MB: // Micropolis 170 MB (155-MB formatted)
	default:
		m_cylinders = 1023;
		m_heads = 8;
		m_sectors = 18;
		break;
	}

	m_type = disk_type;
	m_sectorbytes = OMTI_DISK_SECTOR_SIZE;
	m_sector_count = m_cylinders * m_heads * m_sectors;
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void omti_disk_image_device::device_start()
{
	m_image = this;

	if (m_image->image_core_file() == NULL)
	{
		LOG1(("device_start_omti_disk: no disk"));
	}
	else
	{
		LOG1(("device_start_omti_disk: with disk image %s",m_image->basename() ));
	}

	// default disk type
	omti_disk_config(OMTI_DISK_TYPE_DEFAULT);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void omti_disk_image_device::device_reset()
{
	LOG1(("device_reset_omti_disk"));

	if (exists() && fseek(0, SEEK_END) == 0)
	{
		UINT32 disk_size = (UINT32)(ftell() / OMTI_DISK_SECTOR_SIZE);
		UINT16 disk_type = disk_size >= 300000 ? OMTI_DISK_TYPE_348_MB : OMTI_DISK_TYPE_155_MB;
		if (disk_type != m_type) {
			LOG1(("device_reset_omti_disk: disk size=%d blocks, disk type=%x", disk_size, disk_type ));
			omti_disk_config(disk_type);
		}
	}
}

/*-------------------------------------------------
   disk image create callback
-------------------------------------------------*/

bool omti_disk_image_device::call_create(int format_type, option_resolution *format_options)
{
	LOG(("device_create_omti_disk: creating OMTI Disk with %d blocks", m_sector_count));

	int x;
	unsigned char sectordata[OMTI_DISK_SECTOR_SIZE]; // empty block data


	memset(sectordata, 0x55, sizeof(sectordata));
	for (x = 0; x < m_sector_count; x++)
	{
		if (fwrite(sectordata, OMTI_DISK_SECTOR_SIZE)
				< OMTI_DISK_SECTOR_SIZE)
		{
			return IMAGE_INIT_FAIL;
		}
	}
	return IMAGE_INIT_PASS;
}
