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

#include "emu.h"
#include "omti8621.h"
#include "image.h"
#include "imagedev/harddriv.h"
#include "formats/naslite_dsk.h"
#include "formats/apollo_dsk.h"

#define LOG_LEVEL0      (0x1U << 1)
#define LOG_LEVEL1      (0x3U << 1)
#define LOG_LEVEL2      (0x7U << 1)
#define LOG_LEVEL3      (0xfU << 1)

#define VERBOSE (LOG_LEVEL0)
#include "logmacro.h"

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
DECLARE_DEVICE_TYPE(OMTI_DISK, omti_disk_image_device)

class omti_disk_image_device : public harddisk_image_base_device
{
public:
	// construction/destruction
	omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// device_image_interface implementation
	virtual bool support_command_line_image_creation() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "awd"; }
	virtual const char *image_type_name() const noexcept override { return "winchester"; }
	virtual const char *image_brief_type_name() const noexcept override { return "disk"; }

	virtual std::pair<std::error_condition, std::string> call_create(int format_type, util::option_resolution *format_options) override;

protected:
	// device_t implementation
	virtual void device_start() override ATTR_COLD;
	virtual void device_reset() override ATTR_COLD;

	void omti_disk_config(uint16_t disk_type);

private:
	template <typename Format, typename... Params> void logerror(Format &&fmt, Params &&... args) const;

public:
	uint16_t m_type;
	uint16_t m_cylinders;
	uint16_t m_heads;
	uint16_t m_sectors;
	uint32_t m_sectorbytes;
	uint32_t m_sector_count;

	device_image_interface *m_image;

	// configuration data
	uint8_t m_config_data[10];

	// ESDI defect list data
	uint8_t m_esdi_defect_list[256];
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
#define OMTI_STATUS_CD   0x04 // Command/Data (1 = byte transferred is command or status byte)
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

std::string omti8621_device::cpu_context() const
{
	osd_ticks_t t = osd_ticks();
	int s = (t / osd_ticks_per_second()) % 3600;
	int ms = (t / (osd_ticks_per_second() / 1000)) % 1000;

	return string_format("%d.%03d %s", s, ms, machine().describe_context());
}

static void pc_hd_floppies(device_slot_interface &device)
{
	device.option_add("525hd", FLOPPY_525_HD);
	device.option_add("35hd", FLOPPY_35_HD);
	device.option_add("525dd", FLOPPY_525_DD);
	device.option_add("35dd", FLOPPY_35_DD);
}

void omti8621_device::floppy_formats(format_registration &fr)
{
	fr.add_pc_formats();
	fr.add(FLOPPY_APOLLO_FORMAT);
	fr.add(FLOPPY_NASLITE_FORMAT);
}

// this card has two EPROMs: a program for the on-board Z8 CPU,
// and a PC BIOS to make the card bootable on a PC.
// we have the Z8 program, we still need the PC BIOS.
ROM_START( omti8621 )
	ROM_REGION(0x4000, OMTI_CPU_REGION, 0)  // disassembles fine as Z8 code
	ROM_LOAD( "omti_8621_102640-b.bin", 0x000000, 0x004000, CRC(e6f20dbb) SHA1(cf1990ad72eac6b296485410f5fa3309a0d6d078) )
	ROM_REGION(0x1000, OMTI_BIOS_REGION, 0)
	ROM_LOAD_OPTIONAL("omti_bios", 0x0000, 0x1000, NO_DUMP)
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

void omti8621_device::device_add_mconfig(machine_config &config)
{
	OMTI_DISK(config, OMTI_DISK0_TAG, 0);
	OMTI_DISK(config, OMTI_DISK1_TAG, 0);

	UPD765A(config, m_fdc, 48_MHz_XTAL / 6, false, false); // clocked through FDC9239BT
	m_fdc->intrq_wr_callback().set(FUNC(omti8621_device::fdc_irq_w));
	m_fdc->drq_wr_callback().set(FUNC(omti8621_device::fdc_drq_w));
	FLOPPY_CONNECTOR(config, m_floppy[0], pc_hd_floppies, "525hd", omti8621_device::floppy_formats);
	FLOPPY_CONNECTOR(config, m_floppy[1], pc_hd_floppies, nullptr, omti8621_device::floppy_formats);
}

void omti8621_apollo_device::device_add_mconfig(machine_config &config)
{
	omti8621_device::device_add_mconfig(config);

	// Apollo workstations never have more then 1 floppy drive
	config.device_remove(OMTI_FDC_TAG":1");
}

const tiny_rom_entry *omti8621_device::device_rom_region() const
{
	return ROM_NAME( omti8621 );
}

const tiny_rom_entry *omti8621_apollo_device::device_rom_region() const
{
	// OMTI 8621 boards for Apollo workstations never use a BIOS ROM
	// They don't even have a socket for the BIOS ROM
	return nullptr;
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
	LOGMASKED(LOG_LEVEL2, "device_start");

	set_isa_device();

	m_installed = false;

	m_sector_buffer.resize(OMTI_DISK_SECTOR_SIZE*OMTI_MAX_BLOCK_COUNT);

	m_timer = timer_alloc(FUNC(omti8621_device::trigger_interrupt), this);

	our_disks[0] = subdevice<omti_disk_image_device>(OMTI_DISK0_TAG);
	our_disks[1] = subdevice<omti_disk_image_device>(OMTI_DISK1_TAG);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void omti8621_device::device_reset()
{
	static const int io_bases[8] = { 0x320, 0x324, 0x328, 0x32c, 0x1a0, 0x1a4, 0x1a8, 0x1ac };

	LOGMASKED(LOG_LEVEL2, "device_reset");

	// you can't read I/O ports in device_start() even if they're required_ioport<> in your class!
	if (!m_installed)
	{
		int esdi_base = io_bases[m_iobase->read() & 7];

		// install the ESDI ports
		m_isa->install16_device(esdi_base, esdi_base + 7, read16s_delegate(*this, FUNC(omti8621_device::read)), write16s_delegate(*this, FUNC(omti8621_device::write)));

		// and the onboard AT FDC ports
		if (m_iobase->read() & 8)
		{
			m_isa->install_device(0x0370, 0x0377, *this, &omti8621_device::fdc_map);
		}
		else
		{
			m_isa->install_device(0x03f0, 0x03f7, *this, &omti8621_device::fdc_map);
		}

		m_isa->set_dma_channel(2, this, true);

		m_installed = true;
	}

	set_jumper(our_disks[0]->m_type);

	// should go from reset to idle after 100 us
	// m_omti_state = OMTI_STATE_RESET;
	m_omti_state = OMTI_STATE_IDLE;

	m_status_port = OMTI_STATUS_NU6 | OMTI_STATUS_NU7;
	m_config_port = ~m_jumper;
	m_mask_port = 0;

	// default the sector data buffer with model and status information
	// (i.e. set sector data buffer for cmd=0x0e READ SECTOR BUFFER)

	memset(&m_sector_buffer[0], 0, OMTI_DISK_SECTOR_SIZE);
	memcpy(&m_sector_buffer[0], "8621VB.4060487xx", 0x10);
	m_sector_buffer[0x10] = 0; // ROM Checksum error
	m_sector_buffer[0x11] = 0; // Processor Register error
	m_sector_buffer[0x12] = 0; // Buffer RAM error
	m_sector_buffer[0x13] = 0; // Sequencer Register File error
	m_sector_buffer[0x14] = 0xc0; // 32K buffer size
	// TODO: add missing Default values for LUN 0, 1 and 3

	m_command_length = 0;
	m_command_index = 0;
	m_command_status = 0;

	m_data_index = 0;
	m_data_length = 0;

	clear_sense_data();

	m_diskaddr_ecc_error = 0;
	m_diskaddr_format_bad_track = 0;
	m_alternate_track_address[0] = 0;
	m_alternate_track_address[1] = 0;

	fd_moten_w(0);
	fd_rate_w(0);
	fd_extra_w(0);
}

DEFINE_DEVICE_TYPE(ISA16_OMTI8621, omti8621_pc_device, "omti8621isa", "OMTI 8621 ESDI/floppy controller (ISA)")

omti8621_pc_device::omti8621_pc_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: omti8621_device(mconfig, ISA16_OMTI8621, tag, owner, clock)
{
}

DEFINE_DEVICE_TYPE(ISA16_OMTI8621_APOLLO, omti8621_apollo_device, "omti8621ap", "OMTI 8621 ESDI/floppy controller (Apollo)")

omti8621_apollo_device::omti8621_apollo_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: omti8621_device(mconfig, ISA16_OMTI8621_APOLLO, tag, owner, clock)
{
}

omti8621_device::omti8621_device(
		const machine_config &mconfig,
		device_type type,
		const char *tag,
		device_t *owner,
		uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_isa16_card_interface(mconfig, *this)
	, m_fdc(*this, OMTI_FDC_TAG)
	, m_floppy(*this, OMTI_FDC_TAG":%u", 0U)
	, m_iobase(*this, "IO_BASE")
	, m_biosopts(*this, "BIOS_OPTS")
	, m_jumper(0)
	, m_omti_state(0)
	, m_status_port(0)
	, m_config_port(0)
	, m_mask_port(0)
	, m_command_length(0)
	, m_command_index(0)
	, m_command_status(0)
	, m_data_buffer(nullptr)
	, m_data_length(0)
	, m_data_index(0)
	, m_diskaddr_ecc_error(0)
	, m_diskaddr_format_bad_track(0)
	, m_timer(nullptr)
	, m_moten(0)
	, m_installed(false)
{
}

/*-------------------------------------------------
 set_interrupt - update the IRQ state
 -------------------------------------------------*/

void omti8621_device::set_interrupt(line_state state)
{
	LOGMASKED(LOG_LEVEL2, "%s: set_interrupt: status_port=%x, line_state %d", cpu_context(), m_status_port, state);
	m_isa->irq14_w(state);
}

TIMER_CALLBACK_MEMBER(omti8621_device::trigger_interrupt)
{
	set_interrupt(ASSERT_LINE);
}

/***************************************************************************
 clear_sense_data - clear the sense data
 ***************************************************************************/

void omti8621_device::clear_sense_data() {
	LOGMASKED(LOG_LEVEL2, "%s: clear_sense_data", cpu_context());
	std::fill_n(m_sense_data, std::size(m_sense_data), 0);
}

/***************************************************************************
 set_sense_data - set the sense data from code and command descriptor block
 ***************************************************************************/

void omti8621_device::set_sense_data(uint8_t code, const uint8_t * cdb) {
	LOGMASKED(LOG_LEVEL2, "%s: set_sense_data code=%x", cpu_context(), code);
	m_sense_data[0] = code;
	m_sense_data[1] = cdb[1];
	m_sense_data[2] = cdb[2];
	m_sense_data[3] = cdb[3];
}

/***************************************************************************
 set_configuration_data - set the configuration data for drive lun
 ***************************************************************************/

void omti8621_device::set_configuration_data(uint8_t lun) {
	LOGMASKED(LOG_LEVEL2, "%s: set_configuration_data lun=%x", cpu_context(), lun);

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

uint8_t omti8621_device::get_lun(const uint8_t * cdb)
{
	return   (cdb[1] & 0x20) >> 5;
}

/***************************************************************************
 check_disk_address - check disk address, set sense data and return true for no error
 ***************************************************************************/

uint8_t omti8621_device::check_disk_address(const uint8_t *cdb)
{
	uint8_t sense_code = OMTI_SENSE_CODE_NO_ERROR;
	uint8_t lun = get_lun(cdb);
	uint16_t head = cdb[1] & 0x1f;
	uint16_t sector = cdb[2] & 0x3f;
	uint32_t cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	uint8_t block_count = cdb[4];
	omti_disk_image_device *disk = our_disks[lun];

	uint32_t disk_track = cylinder * disk->m_heads + head;
	uint32_t disk_addr = (disk_track * disk->m_sectors) + sector;

	if (block_count > OMTI_MAX_BLOCK_COUNT) {
		LOGMASKED(LOG_LEVEL0, "%s: ########### check_disk_address: unexpected block count %x", cpu_context(), block_count);
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
	} else if (disk_track == m_diskaddr_format_bad_track && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_BAD_TRACK;
	} else if (disk_addr == m_diskaddr_ecc_error && disk_addr != 0) {
		sense_code = OMTI_SENSE_CODE_ECC_ERROR;
	} else if (disk_track == m_alternate_track_address[1] && disk_track != 0) {
		sense_code = OMTI_SENSE_CODE_ALTERNATE_TRACK;
	}

	if (sense_code == OMTI_SENSE_CODE_NO_ERROR) {
		clear_sense_data();
	} else {
		m_command_status |= OMTI_COMMAND_STATUS_ERROR;
		set_sense_data(sense_code, cdb);
	}
	return sense_code == OMTI_SENSE_CODE_NO_ERROR;
}

/***************************************************************************
 get_disk_track - get disk track from a command descriptor block
 ***************************************************************************/

uint32_t omti8621_device::get_disk_track(const uint8_t * cdb) {
	uint8_t lun = get_lun(cdb);
	uint16_t head = cdb[1] & 0x1f;
	uint32_t cylinder = cdb[3] + ((cdb[2] & 0xc0) << 2) + ((cdb[1] & 0x80) << 3);
	return cylinder * our_disks[lun]->m_heads + head;
}

/***************************************************************************
 get_disk_address - get disk address from a command descriptor block
 ***************************************************************************/

uint32_t omti8621_device::get_disk_address(const uint8_t * cdb) {
	uint8_t lun = get_lun(cdb);
	uint16_t sector = cdb[2] & 0x3f;
	return get_disk_track(cdb) * our_disks[lun]->m_sectors + sector;
}

/***************************************************************************
 set_data_transfer - setup for data transfer from/to data
 ***************************************************************************/

void omti8621_device::set_data_transfer(uint8_t *data, uint16_t length)
{
	// set controller for read data transfer
	m_omti_state = OMTI_STATE_DATA;
	m_status_port |= OMTI_STATUS_REQ | OMTI_STATUS_IO | OMTI_STATUS_BUSY;
	m_status_port &= ~OMTI_STATUS_CD;

	m_data_buffer = data;
	m_data_length = length;
	m_data_index = 0;
}

/***************************************************************************
 read_sectors_from_disk - read sectors starting at diskaddr into m_sector_buffer
 ***************************************************************************/

void omti8621_device::read_sectors_from_disk(int32_t diskaddr, uint8_t count, uint8_t lun)
{
	uint8_t *data_buffer = &m_sector_buffer[0];
	device_image_interface *image = our_disks[lun]->m_image;

	while (count-- > 0) {
		LOGMASKED(LOG_LEVEL2, "%s: read_sectors_from_disk lun=%d diskaddr=%x", cpu_context(), lun, diskaddr);

		image->fseek(diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread(data_buffer, OMTI_DISK_SECTOR_SIZE);

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 write_sectors_to_disk - write sectors starting at diskaddr from m_sector_buffer
 ***************************************************************************/

void omti8621_device::write_sectors_to_disk(int32_t diskaddr, uint8_t count, uint8_t lun)
{
	uint8_t *data_buffer = &m_sector_buffer[0];
	device_image_interface *image = our_disks[lun]->m_image;

	while (count-- > 0) {
		LOGMASKED(LOG_LEVEL2, "%s: write_sectors_to_disk lun=%d diskaddr=%x", cpu_context(), lun, diskaddr);

		image->fseek(diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite(data_buffer, OMTI_DISK_SECTOR_SIZE);

		if (diskaddr == m_diskaddr_ecc_error) {
			// reset previous ECC error
			m_diskaddr_ecc_error = 0;
		}

		diskaddr++;
		data_buffer += OMTI_DISK_SECTOR_SIZE;
	}
}

/***************************************************************************
 copy_sectors - copy sectors
 ***************************************************************************/

void omti8621_device::copy_sectors(int32_t dst_addr, int32_t src_addr, uint8_t count, uint8_t lun)
{
	device_image_interface *image = our_disks[lun]->m_image;

	LOGMASKED(LOG_LEVEL2, "%s: copy_sectors lun=%d src_addr=%x dst_addr=%x count=%x", cpu_context(), lun, src_addr, dst_addr, count);

	while (count-- > 0) {
		image->fseek(src_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fread(&m_sector_buffer[0], OMTI_DISK_SECTOR_SIZE);

		image->fseek(dst_addr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		image->fwrite(&m_sector_buffer[0], OMTI_DISK_SECTOR_SIZE);

		if (dst_addr == m_diskaddr_ecc_error) {
			// reset previous ECC error
			m_diskaddr_ecc_error = 0;
		}

		src_addr++;
		dst_addr++;
	}
}

/***************************************************************************
 format track - format a track
 ***************************************************************************/

void omti8621_device::format_track(const uint8_t * cdb)
{
	uint8_t lun = get_lun(cdb);
	uint32_t disk_addr = get_disk_address(cdb);
	uint32_t disk_track = get_disk_track(cdb);

	if (m_diskaddr_ecc_error == disk_addr) {
		// reset previous ECC error
		m_diskaddr_ecc_error = 0;
	}

	if (m_diskaddr_format_bad_track == disk_track) {
		// reset previous bad track formatting
		m_diskaddr_format_bad_track = 0;
	}

	if (m_alternate_track_address[0] == disk_track) {
		// reset source of alternate track address
		m_alternate_track_address[0] = 0;
	}

	if (m_alternate_track_address[1] == disk_track) {
		// reset alternate track address
		m_alternate_track_address[1] = 0;
	}

	if (check_disk_address(cdb) ) {
		if ((cdb[5] & 0x40) == 0) {
			memset(&m_sector_buffer[0], 0x6C, OMTI_DISK_SECTOR_SIZE * our_disks[lun]->m_sectors);
		}
		write_sectors_to_disk(disk_addr, our_disks[lun]->m_sectors, lun);
	}

}

/***************************************************************************
 set_esdi_defect_list - setup the (empty) ESDI defect list
 ***************************************************************************/

void omti8621_device::set_esdi_defect_list(uint8_t lun, uint8_t head)
{
	omti_disk_image_device *disk = our_disks[lun];

	memset(disk->m_esdi_defect_list, 0, sizeof(disk->m_esdi_defect_list));
	disk->m_esdi_defect_list[0] = 1; // month
	disk->m_esdi_defect_list[1] = 1; // day
	disk->m_esdi_defect_list[2] = 90; // year
	disk->m_esdi_defect_list[3] = head;
	memset(disk->m_esdi_defect_list+6, 0xff, 5); // end of defect list
}

/*-------------------------------------------------
 logerror - log an error message (w/o device tags)
 -------------------------------------------------*/

template <typename Format, typename... Params>
void omti8621_device::logerror(Format &&fmt, Params &&... args) const
{
	machine().logerror(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

/***************************************************************************
 log_command - log command from a command descriptor block
 ***************************************************************************/

void omti8621_device::log_command(const uint8_t cdb[], const uint16_t cdb_length)
{
	if (VERBOSE & (LOG_LEVEL1 | LOG_LEVEL2 | LOG_LEVEL3)) {
		logerror("%s: OMTI command ", cpu_context());
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
		for (int i = 0; i < cdb_length; i++) {
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
	if (VERBOSE & LOG_LEVEL1) {
		logerror("%s: OMTI data (length=%02x)", cpu_context(), m_data_length);
		uint16_t i;
		for (i = 0; i < m_data_length && i < OMTI_DISK_SECTOR_SIZE; i++) {
			logerror(" %02x", m_data_buffer[i]);
		}

		if (i < m_data_length) {
			logerror(" ...");
		}
		logerror("\n");
	}
}

/***************************************************************************
 do_command
 ***************************************************************************/

void omti8621_device::do_command(const uint8_t cdb[], const uint16_t cdb_length)
{
	uint8_t lun = get_lun(cdb);
	omti_disk_image_device *disk = our_disks[lun];
	int command_duration = 0; // ms

	log_command(cdb, cdb_length);

	// default to read status and status is successful completion
	m_omti_state = OMTI_STATE_STATUS;
	m_status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
	m_command_status = lun ? OMTI_COMMAND_STATUS_LUN : 0;

	if (m_mask_port & OMTI_MASK_INTE) {
		set_interrupt(CLEAR_LINE);
	}

	if (!disk->m_image->exists()) {
		m_command_status |= OMTI_COMMAND_STATUS_ERROR; // no such drive
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
		set_data_transfer(m_sense_data, sizeof(m_sense_data));
		break;

	case OMTI_CMD_READ_VERIFY: // 0x05
		check_disk_address(cdb);
		break;

	case OMTI_CMD_FORMAT_TRACK: // 0x06
		format_track(cdb);
		break;

	case OMTI_CMD_FORMAT_BAD_TRACK: // 0x07
		m_diskaddr_format_bad_track = get_disk_address(cdb);
		break;

	case OMTI_CMD_READ: // 0x08
		if (check_disk_address(cdb)) {
			// read data from controller
			read_sectors_from_disk(get_disk_address(cdb), cdb[4], lun);
			set_data_transfer(&m_sector_buffer[0],  OMTI_DISK_SECTOR_SIZE*cdb[4]);
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
		set_data_transfer(&m_sector_buffer[0], OMTI_DISK_SECTOR_SIZE*cdb[4]);
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
		m_alternate_track_address[0] = get_disk_track(cdb);
		m_alternate_track_address[1] = get_disk_track(m_alternate_track_buffer - 1);
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
			set_data_transfer(&m_sector_buffer[0], OMTI_DISK_SECTOR_SIZE+6);
		}
		break;

	case OMTI_CMD_WRITE_LONG: // 0xE6
		log_data();
		if (check_disk_address(cdb)) {
			uint32_t diskaddr =  get_disk_address(cdb);
			write_sectors_to_disk(diskaddr, cdb[4], lun);
			// this will spoil the ECC code
			m_diskaddr_ecc_error = diskaddr;
		}
		break;

	case OMTI_CMD_READ_CONFIGURATION: // 0xEC
		set_configuration_data(get_lun(cdb));
		set_data_transfer(disk->m_config_data, sizeof(disk->m_config_data));
		break;

	case OMTI_CMD_INVALID_COMMAND: // 0xFF
		set_sense_data(OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		m_command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;

	default:
		LOGMASKED(LOG_LEVEL0, "%s: do_command: UNEXPECTED command %02x", cpu_context(), cdb[0]);
		set_sense_data(OMTI_SENSE_CODE_INVALID_COMMAND, cdb);
		m_command_status |= OMTI_COMMAND_STATUS_ERROR;
		break;
	}

	if (m_mask_port & OMTI_MASK_INTE) {
//      if (m_omti_state != OMTI_STATE_STATUS) {
//          LOGMASKED(LOG_LEVEL0, "%s: do_command: UNEXPECTED omti_state %02x", cpu_context(), m_omti_state));
//      }
		m_status_port |= OMTI_STATUS_IREQ;
		if (command_duration == 0)
		{
			set_interrupt(ASSERT_LINE);
		}
		else
		{
			// FIXME: should delay m_omti_state and m_status_port as well
			m_timer->adjust(attotime::from_msec(command_duration), 0);
		}
	}
}

/***************************************************************************
 get_command_length
 ***************************************************************************/

uint8_t omti8621_device::get_command_length(uint8_t command_byte)
{
	return command_byte == OMTI_CMD_COPY ? 10 : 6;
}

/***************************************************************************
 get_data
 ***************************************************************************/

uint16_t omti8621_device::get_data()
{
	uint16_t data = 0xff;
	if (m_data_index < m_data_length) {
		data = m_data_buffer[m_data_index++];
		data |= m_data_buffer[m_data_index++] << 8;
		if (m_data_index >= m_data_length) {
			m_omti_state = OMTI_STATE_STATUS;
			m_status_port |= OMTI_STATUS_IO | OMTI_STATUS_CD;
			log_data();
		}
	} else {
		LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED reading OMTI 8621 data (buffer length exceeded)", cpu_context());
	}
	return data;
}

/***************************************************************************
 set_data
 ***************************************************************************/

void omti8621_device::set_data(uint16_t data)
{
	if (m_data_index < m_data_length) {
		m_data_buffer[m_data_index++] = data & 0xff;
		m_data_buffer[m_data_index++] = data >> 8;
		if (m_data_index >= m_data_length) {
			do_command(m_command_buffer, m_command_index);
		}
	} else {
		LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED writing OMTI 8621 data (buffer length exceeded)", cpu_context());
	}
}

/***************************************************************************
 OMTI8621 Disk Controller-AT Registers
***************************************************************************/

void omti8621_device::write(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	switch (mem_mask)
	{
		case 0x00ff:
			write8(offset*2, data);
			break;

		case 0xff00:
			write8(offset*2+1, data>>8);
			break;

		default:
			set_data(data);
			break;
	}
}

void omti8621_device::write8(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case OMTI_PORT_DATA_OUT: //  0x00
		switch (m_omti_state) {
		case OMTI_STATE_COMMAND:
			LOGMASKED(LOG_LEVEL2, "%s: writing OMTI 8621 Command Register at offset %02x = %02x", cpu_context(), offset, data);
			if (m_command_index == 0) {
				m_command_length = get_command_length(data);
			}

			if (m_command_index < m_command_length) {
				m_command_buffer[m_command_index++] = data;
			} else {
				LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (command length exceeded)", cpu_context(), offset, data);
			}

			if (m_command_index == m_command_length) {
				switch (m_command_buffer[0]) {
				case OMTI_CMD_WRITE: // 0x0A
					// TODO: check diskaddr
					[[fallthrough]];
				case OMTI_CMD_WRITE_SECTOR_BUFFER: // 0x0F
					set_data_transfer(&m_sector_buffer[0], OMTI_DISK_SECTOR_SIZE * m_command_buffer[4]);
					m_status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_ASSIGN_ALTERNATE_TRACK: // 0x11
					set_data_transfer(m_alternate_track_buffer, sizeof(m_alternate_track_buffer));
					m_status_port &= ~OMTI_STATUS_IO;
					break;

				case OMTI_CMD_WRITE_LONG: // 0xE6
					// TODO: check diskaddr
					set_data_transfer(&m_sector_buffer[0], (OMTI_DISK_SECTOR_SIZE +6) * m_command_buffer[4]);
					m_status_port &= ~OMTI_STATUS_IO;
					break;

				default:
					do_command(m_command_buffer, m_command_index);
					break;
				}
			}
			break;

		case OMTI_STATE_DATA:
			LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED: writing OMTI 8621 Data Register at offset %02x = %02x", cpu_context(), offset, data);
			break;

		default:
			LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED writing OMTI 8621 Data Register at offset %02x = %02x (omti state = %02x)", cpu_context(), offset, data, m_omti_state);
			break;
		}
		break;

	case OMTI_PORT_RESET: // 0x01
		LOGMASKED(LOG_LEVEL2, "%s: writing OMTI 8621 Reset Register at offset %02x = %02x", cpu_context(), offset, data);
		device_reset();
		break;

	case OMTI_PORT_SELECT: // 0x02
		LOGMASKED(LOG_LEVEL2, "%s: writing OMTI 8621 Select Register at offset %02x = %02x (omti state = %02x)", cpu_context(), offset, data, m_omti_state);
		m_omti_state = OMTI_STATE_COMMAND;

		m_status_port |= OMTI_STATUS_BUSY | OMTI_STATUS_REQ | OMTI_STATUS_CD;
		m_status_port &= ~OMTI_STATUS_IO;

		m_command_status = 0;
		m_command_index = 0;
		break;

	case OMTI_PORT_MASK: // 0x03
		LOGMASKED(LOG_LEVEL2, "%s: writing OMTI 8621 Mask Register at offset %02x = %02x", cpu_context(), offset, data);
		m_mask_port = data;

		if ((data & OMTI_MASK_INTE) == 0) {
			m_status_port &= ~OMTI_STATUS_IREQ;
			set_interrupt(CLEAR_LINE);
		}

		if ((data & OMTI_MASK_DMAE) == 0) {
			m_status_port &= ~OMTI_STATUS_DREQ;
		}
		break;

	default:
		LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED writing OMTI 8621 Register at offset %02x = %02x", cpu_context(), offset, data);
		break;
	}
}

uint16_t omti8621_device::read(offs_t offset, uint16_t mem_mask)
{
	switch (mem_mask)
	{
		case 0x00ff:
			return read8(offset*2);
		case 0xff00:
			return read8(offset*2+1) << 8;
		default:
			return get_data();
	}
}

uint8_t omti8621_device::read8(offs_t offset)
{
	uint8_t data = 0xff;
	static uint8_t last_data = 0xff;

	switch (offset) {
	case OMTI_PORT_DATA_IN: // 0x00
		if (m_status_port & OMTI_STATUS_CD)
		{
			data = m_command_status;
			switch (m_omti_state)
			{
			case OMTI_STATE_COMMAND:
				LOGMASKED(LOG_LEVEL2, "%s: reading OMTI 8621 Data Status Register 1 at offset %02x = %02x (omti state = %02x)", cpu_context(), offset, data, m_omti_state);
				break;
			case OMTI_STATE_STATUS:
				m_omti_state = OMTI_STATE_IDLE;
				m_status_port &= ~(OMTI_STATUS_BUSY | OMTI_STATUS_CD  | OMTI_STATUS_IO | OMTI_STATUS_REQ);
				LOGMASKED(LOG_LEVEL2, "%s: reading OMTI 8621 Data Status Register 2 at offset %02x = %02x", cpu_context(), offset, data);
				break;
			default:
				LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED reading OMTI 8621 Data Status Register 3 at offset %02x = %02x (omti state = %02x)", cpu_context(), offset, data, m_omti_state);
				break;
			}
		}
		else
		{
			LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED reading OMTI 8621 Data Register 4 at offset %02x = %02x (status bit C/D = 0)", cpu_context(), offset, data);
		}
		break;

	case OMTI_PORT_STATUS: // 0x01
		data = m_status_port;
		// omit excessive logging
		if (data != last_data)
		{
			LOGMASKED(LOG_LEVEL2, "%s: reading OMTI 8621 Status Register 5 at offset %02x = %02x", cpu_context(), offset, data);
//          last_data = data;
		}
		break;

	case OMTI_PORT_CONFIG: // 0x02
		data = m_config_port;
		LOGMASKED(LOG_LEVEL2, "%s: reading OMTI 8621 Configuration Register at offset %02x = %02x", cpu_context(), offset, data);
		break;

	case OMTI_PORT_MASK: // 0x03
		data = m_mask_port;
		// win.dex will update the mask register with read-modify-write
		// LOGMASKED(LOG_LEVEL2, "%s: reading OMTI 8621 Mask Register at offset %02x = %02x (UNEXPECTED!)", cpu_context(), offset, data);
		break;

	default:
		LOGMASKED(LOG_LEVEL0, "%s: UNEXPECTED reading OMTI 8621 Register at offset %02x = %02x", cpu_context(), offset, data);
		break;
	}

	return data;
}

/***************************************************************************
 get_sector - get sector diskaddr of logical unit lun into data_buffer
 ***************************************************************************/

uint32_t omti8621_apollo_device::get_sector(int32_t diskaddr, uint8_t *buffer, uint32_t length, uint8_t lun)
{
	omti_disk_image_device *disk = our_disks[lun];

	if (disk == nullptr || disk->m_image == nullptr || !disk->m_image->exists())
	{
		return 0;
	}
	else
	{
//      LOGMASKED(LOG_LEVEL1, "%s: omti8621_get_sector %x on lun %d", cpu_context(), diskaddr, lun);

		// restrict length to size of 1 sector (i.e. 1024 Byte)
		length = length < OMTI_DISK_SECTOR_SIZE ? length  : OMTI_DISK_SECTOR_SIZE;

		disk->m_image->fseek(diskaddr * OMTI_DISK_SECTOR_SIZE, SEEK_SET);
		disk->m_image->fread(buffer, length);

		return length;
	}
}

/***************************************************************************
 omti_set_jumper - set OMTI jumpers
 ***************************************************************************/

void omti8621_device::set_jumper(uint16_t disk_type)
{
	LOGMASKED(LOG_LEVEL1, "%s: set_jumper: disk type=%x", cpu_context(), disk_type);

	switch (disk_type)
	{
	case OMTI_DISK_TYPE_348_MB: // Maxtor 380 MB (348-MB FA formatted)
		m_jumper = OMTI_CONFIG_W22 | OMTI_CONFIG_W23;
		break;

	case OMTI_DISK_TYPE_155_MB: // Micropolis 170 MB (155-MB formatted)
	default:
		m_jumper = OMTI_CONFIG_W20;
		break;
	}
}

// FDC uses the standard IRQ 6 / DMA 2, doesn't appear to be configurable
void omti8621_device::fdc_irq_w(int state)
{
	if (BIT(m_moten, 3))
		m_isa->irq6_w(state ? ASSERT_LINE : CLEAR_LINE);
}

void omti8621_device::fdc_drq_w(int state)
{
	if (BIT(m_moten, 3))
		m_isa->drq2_w(state ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t omti8621_device::dack_r(int line)
{
	return m_fdc->dma_r();
}

void omti8621_device::dack_w(int line, uint8_t data)
{
	return m_fdc->dma_w(data);
}

void omti8621_device::dack_line_w(int line, int state)
{
	//m_fdc->dack_w(state);
}

void omti8621_device::eop_w(int state)
{
	m_fdc->tc_w(state == ASSERT_LINE);
}

void omti8621_device::fdc_map(address_map &map)
{
	map(2, 2).w(FUNC(omti8621_device::fd_moten_w));
	map(4, 5).m(m_fdc, FUNC(upd765a_device::map));
	map(6, 6).w(FUNC(omti8621_device::fd_extra_w));
	map(7, 7).rw(FUNC(omti8621_device::fd_disk_chg_r), FUNC(omti8621_device::fd_rate_w));
}

void omti8621_device::fd_moten_w(uint8_t data)
{
	if (BIT(data, 3) && !BIT(m_moten, 3))
	{
		m_isa->irq6_w(m_fdc->get_irq() ? ASSERT_LINE : CLEAR_LINE);
		m_isa->drq2_w(m_fdc->get_drq() ? ASSERT_LINE : CLEAR_LINE);
	}
	else if (!BIT(data, 3) && BIT(m_moten, 3))
	{
		m_isa->irq6_w(CLEAR_LINE);
		m_isa->drq2_w(CLEAR_LINE);
	}

	m_moten = data;

	m_fdc->reset_w(!BIT(data, 2));

	for (int i = 0; i < 2; i++)
	{
		floppy_image_device *floppy = m_floppy[i].found() ? m_floppy[i]->get_device() : nullptr;
		if (floppy != nullptr)
			floppy->mon_w(!BIT(data, i + 4));

		if (i == (data & 0x01))
			m_fdc->set_floppy(BIT(data, i + 4) ? floppy : nullptr);
	}
}

void omti8621_device::fd_rate_w(uint8_t data)
{
	// Bit 1 = FD_MINI (connects to pin 3 of FDC9239)
	// Bit 0 = FD_RATE (inverted output connects to pin 4 of 74F163)
	u32 fdc_clk = (48_MHz_XTAL / (BIT(data, 0) ? 5 : 3) / (BIT(data, 1) ? 4 : 2)).value();
	m_fdc->set_unscaled_clock(fdc_clk);
	m_fdc->set_rate(fdc_clk / 16);
}

void omti8621_device::fd_extra_w(uint8_t data)
{
	// Bit 7 = FD_EXTRA-2 (NC)
	// Bit 6 = FD_EXTRA-1 (NC)
	// Bit 5 = FD_PIN_6 (TODO)
	// Bit 4 = FD_IUSE_HLD (TODO)
	// Bit 3 = FD_DEN_SEL (TODO)
	// Bit 2 = FD_PRE-2 (TODO)
	// Bit 1 = FD_PRE-1 (TODO)
	// Bit 0 = FD_PRE-0 (TODO)
}

uint8_t omti8621_device::fd_disk_chg_r()
{
	floppy_image_device *floppy = m_floppy[m_moten & 0x01].found() ? m_floppy[m_moten & 0x01]->get_device() : nullptr;
	if (floppy == nullptr || floppy->dskchg_r())
		return 0x00;
	else
		return 0x80;
}

//##########################################################################

// device type definition
DEFINE_DEVICE_TYPE(OMTI_DISK, omti_disk_image_device, "omti_disk_image", "OMTI 8621 ESDI disk")

omti_disk_image_device::omti_disk_image_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: harddisk_image_base_device(mconfig, OMTI_DISK, tag, owner, clock)
	, m_type(0), m_cylinders(0), m_heads(0), m_sectors(0), m_sectorbytes(0), m_sector_count(0), m_image(nullptr)
{
}


/***************************************************************************
 omti_disk_config - configure disk parameters
 ***************************************************************************/

void omti_disk_image_device::omti_disk_config(uint16_t disk_type)
{
	logerror("omti_disk_config: configuring disk with type %x\n", disk_type);

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
 logerror - log an error message (w/o device tags)
 -------------------------------------------------*/

template <typename Format, typename... Params>
void omti_disk_image_device::logerror(Format &&fmt, Params &&... args) const
{
	machine().logerror(std::forward<Format>(fmt), std::forward<Params>(args)...);
}

/*-------------------------------------------------
    device start callback
-------------------------------------------------*/

void omti_disk_image_device::device_start()
{
	m_image = this;

	if (!m_image->is_open())
	{
		logerror("device_start_omti_disk: no disk\n");
	}
	else
	{
		logerror("device_start_omti_disk: with disk image %s\n", m_image->basename());
	}

	// default disk type
	omti_disk_config(OMTI_DISK_TYPE_DEFAULT);
}

/*-------------------------------------------------
    device reset callback
-------------------------------------------------*/

void omti_disk_image_device::device_reset()
{
	logerror("device_reset_omti_disk\n");

	if (exists() && !fseek(0, SEEK_END))
	{
		uint32_t disk_size = uint32_t(ftell() / OMTI_DISK_SECTOR_SIZE);
		uint16_t disk_type = disk_size >= 300000 ? OMTI_DISK_TYPE_348_MB : OMTI_DISK_TYPE_155_MB;
		if (disk_type != m_type) {
			logerror("device_reset_omti_disk: disk size=%d blocks, disk type=%x\n", disk_size, disk_type);
			omti_disk_config(disk_type);
		}
	}
}

/*-------------------------------------------------
   disk image create callback
-------------------------------------------------*/

std::pair<std::error_condition, std::string> omti_disk_image_device::call_create(int format_type, util::option_resolution *format_options)
{
	logerror("device_create_omti_disk: creating OMTI Disk with %d blocks\n", m_sector_count);

	unsigned char sectordata[OMTI_DISK_SECTOR_SIZE]; // empty block data
	memset(sectordata, 0x55, sizeof(sectordata));

	for (int x = 0; x < m_sector_count; x++)
	{
		if (fwrite(sectordata, OMTI_DISK_SECTOR_SIZE) < OMTI_DISK_SECTOR_SIZE)
			return std::make_pair(image_error::UNSPECIFIED, std::string());
	}

	return std::make_pair(std::error_condition(), std::string());
}
