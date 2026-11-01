// license:BSD-3-Clause
// copyright-holders:Hans Ostermeyer, R. Belmont, Chris Toshok
/*
 * ansi_disk.cpp - a Winchester disk drive on the ANSI device-level interface
 * (ANSI X3.101, "Interfaces between rigid disk drive(s) and host(s)")
 *
 * Derived from omti8621.cpp.
 *
 * The drive's command set, status bytes and attribute table follow the
 * standard; the read and write data are moved a byte at a time (not as
 * clocked NRZ data) by the callbacks to the controller.  The image holds
 * the sectors in cylinder, head, sector order, each as many bytes as the
 * host formats.  It gets the geometry of the smallest model that holds it
 * (a new image, the first model's).
 *
 *  see also:
 *  - http://bitsavers.org/pdf/ansi/X3/X3.101-1984_Interface_Between_Rigid_Disk_Drives_and_Hosts.pdf
 *  - http://bitsavers.org/pdf/priam/Priam_Device_Level_Interface_Jun83.pdf
 */

#include "emu.h"
#include "ansi_disk.h"

#define LOG_CMD     (1U << 1) // ANSI commands
#define LOG_DATA    (1U << 2) // sector reads and writes
#define LOG_SIGNALS (1U << 3) // attention and busy lines

#define VERBOSE (0)
#include "logmacro.h"

#include <vector>

#define LOGCMD(...)     LOGMASKED(LOG_CMD, __VA_ARGS__)
#define LOGDATA(...)    LOGMASKED(LOG_DATA, __VA_ARGS__)
#define LOGSIGNALS(...) LOGMASKED(LOG_SIGNALS, __VA_ARGS__)

// general status bits
#define GS_NOT_READY         0x01
#define GS_CONTROL_BUS_ERROR 0x02
#define GS_ILLEGAL_COMMAND   0x04
#define GS_ILLEGAL_PARAMETER 0x08
#define GS_SENSE_BYTE_1      0x10
#define GS_SENSE_BYTE_2      0x20
#define GS_BUSY_EXECUTING    0x40
#define GS_NORMAL_COMPLETE   0x80

// sense byte 1 bits
#define SB1_SEEK_ERROR          0x01
#define SB1_RW_FAULT            0x02
#define SB1_POWER_FAULT         0x04
#define SB1_RW_PERMIT_VIOLATION 0x08
#define SB1_SPEED_ERROR         0x10
#define SB1_COMMAND_REJECT      0x20
#define SB1_OTHER_ERRORS        0x40
#define SB1_VENDOR_ERRORS       0x80

// sense byte 2 bits
#define SB2_INITIAL_STATE                          0x01
#define SB2_READY_TRANSITION                       0x02
#define SB2_DEV_RESERVED_TO_THIS_POINT             0x04
#define SB2_FORCED_RELEASE                         0x08
#define SB2_DEV_RESERVED_TO_ALT_PORT               0x10
#define SB2_DEVICE_ATTR_TABLE_MODIFIED             0x20
#define SB2_POSITIONED_WITHIN_WRITE_PROTECTED_AREA 0x40
#define SB2_VENDOR_ATTNS                           0x80

DEFINE_DEVICE_TYPE(ANSI_DISK, ansi_disk_device, "ansi_disk", "ANSI X3.101 Winchester disk drive")

namespace {

// the 8" drives on the interface.  The model IDs are the disk type numbers
// of the Apollo DN300's software (Domain Engineering Handbook, the Dtype
// table); what the drives themselves report there is not known.
const ansi_disk_device::model listed_models[] =
{
	{ "Priam 7050",      0x105, 1049, 5, 12, 3600 }, // 70MB unformatted
	{ "Priam 3450",      0x104,  525, 5, 12, 3600 }, // 35MB unformatted
	{ "Micropolis 1203", 0x103,  580, 5, 13, 3600 }, // 38MB unformatted
};

} // anonymous namespace

ansi_disk_device::ansi_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: harddisk_image_base_device(mconfig, ANSI_DISK, tag, owner, clock)
	, m_attention(false)
	, m_busy(false)
	, cur_attention_cb()
	, cur_busy_cb()
	, cur_read_data_cb()
	, cur_index_pulse_cb()
	, cur_sector_pulse_cb()
	, m_selected(false)
	, m_sector_size(0)
	, m_extensions("img")
	, m_model_id(0)
	, m_cylinders(0)
	, m_heads(0)
	, m_sectors(0)
	, m_sector_count(0)
	, m_current_cylinder_high(0)
	, m_current_cylinder_low(0)
	, m_load_cylinder_high(0)
	, m_load_cylinder_low(0)
	, m_attribute_number(0)
	, m_test_byte(0)
	, m_selected_head(0)
	, m_general_status(0)
	, m_sense_byte_1(0)
	, m_sense_byte_2(0)
	, m_write_enabled(true)
	, m_attention_enabled(true)
	, m_attributes_initialized(false)
	, m_pulsed_sector(0)
{
}

void ansi_disk_device::set_attention_cb(bool_cb cb)
{
	cur_attention_cb = cb;
}

void ansi_disk_device::set_busy_cb(bool_cb cb)
{
	cur_busy_cb = cb;
}

void ansi_disk_device::set_read_data_cb(read_data_cb cb)
{
	cur_read_data_cb = cb;
}

void ansi_disk_device::set_ref_clock_tick_cb(pulse_cb cb)
{
	cur_ref_clock_tick_cb = cb;
}

void ansi_disk_device::set_index_pulse_cb(pulse_cb cb)
{
	cur_index_pulse_cb = cb;
}

void ansi_disk_device::set_sector_pulse_cb(pulse_cb cb)
{
	cur_sector_pulse_cb = cb;
}

/***************************************************************************
 select_model - take a model's geometry and sector timing
 ***************************************************************************/

void ansi_disk_device::select_model(const model &m)
{
	LOG("configuring the drive as a %s\n", m.name);

	m_model_id = m.model_id;
	m_cylinders = m.cylinders;
	m_heads = m.heads;
	m_sectors = m.sectors;
	m_sector_count = m_cylinders * m_heads * m_sectors;

	// one revolution per index pulse, divided among the sector pulses
	m_sector_clock_freq = attotime::from_hz(m.rpm * m.sectors / 60.0);
	m_sector_timer->adjust(m_sector_clock_freq, 0, m_sector_clock_freq);
	m_pulsed_sector = 0;
}

/*-------------------------------------------------
	device start callback
-------------------------------------------------*/

void ansi_disk_device::device_start()
{
	if (!is_open())
		LOG("no disk\n");
	else
		LOG("disk image %s\n", basename());

	if (!m_sector_size)
		fatalerror("%s: the host did not set the sector size\n", tag());
	m_buffer = std::make_unique<uint8_t []>(m_sector_size);

	m_time_dependent_timer = timer_alloc(FUNC(ansi_disk_device::finish_time_dependent_command), this);
	m_read_timer = timer_alloc(FUNC(ansi_disk_device::read_clock_tick), this);
	m_ref_timer = timer_alloc(FUNC(ansi_disk_device::ref_clock_tick), this);

	m_sector_timer = timer_alloc(FUNC(ansi_disk_device::sector_callback), this);

	// a new image is the first model's size
	select_model(listed_models[0]);
}

/*-------------------------------------------------
	device reset callback
-------------------------------------------------*/

void ansi_disk_device::device_reset()
{
	if (exists() && !fseek(0, SEEK_END))
	{
		uint32_t disk_size = uint32_t(ftell() / m_sector_size);

		// the smallest model that holds the image, else the largest
		const model *fit = nullptr;
		const model *largest = nullptr;
		for (const model &m : listed_models)
		{
			uint32_t count = m.cylinders * m.heads * m.sectors;
			if (count >= disk_size && (!fit || count < fit->cylinders * fit->heads * fit->sectors))
				fit = &m;
			if (!largest || count > largest->cylinders * largest->heads * largest->sectors)
				largest = &m;
		}
		const model &m = fit ? *fit : *largest;
		if (m.model_id != m_model_id)
		{
			LOG("disk size=%d blocks\n", disk_size);
			select_model(m);
		}
	}
}

/*-------------------------------------------------
   disk image create callback
-------------------------------------------------*/

std::pair<std::error_condition, std::string> ansi_disk_device::call_create(int format_type, util::option_resolution *format_options)
{
	LOG("creating ANSI disk with %d blocks\n", m_sector_count);

	std::vector<uint8_t> sectordata(m_sector_size, 0x55); // empty block data

	for (int x = 0; x < m_sector_count; x++)
	{
		if (fwrite(sectordata.data(), m_sector_size) < m_sector_size)
		{
			return std::make_pair(image_error::UNSPECIFIED, std::string());
		}
	}

	return std::make_pair(std::error_condition(), std::string());
}

uint8_t ansi_disk_device::report_attribute()
{
	/* ensure our attributes have been initialized */
	if (!m_attributes_initialized) {
		m_attributes_initialized = true;
		memset(m_ansi_attributes, 0, sizeof(m_ansi_attributes));

		m_ansi_attributes[0x00] = 0x00;          // User ID - user defined
		m_ansi_attributes[0x01] = m_model_id >> 8;   // Model ID High - vendor defined
		m_ansi_attributes[0x02] = m_model_id & 0xff; // Model ID Low - vendor defined
		m_ansi_attributes[0x03] = 0x00;          // Revision ID - vendor defined

		m_ansi_attributes[0x0D] = 0x00; // Device Type ID - device dependent
		m_ansi_attributes[0x0E] = 0x00; // Table Modification - action dependent
		m_ansi_attributes[0x0F] = 0x00; // Table ID - vendor defined

		uint32_t bytes_per_track = m_sectors * m_sector_size;
		m_ansi_attributes[0x10] = (bytes_per_track >> 16) & 0xff;       // MSB of # of bytes per track
		m_ansi_attributes[0x11] = (bytes_per_track >> 8) & 0xff;        // MedSB of # of bytes per track
		m_ansi_attributes[0x12] = bytes_per_track & 0xff;               // LSB of # of bytes per track
		m_ansi_attributes[0x13] = (m_sector_size >> 16) & 0xff; // MSB of # of bytes per sector
		m_ansi_attributes[0x14] = (m_sector_size >> 8) & 0xff;  // MedSB of # of bytes per sector
		m_ansi_attributes[0x15] = m_sector_size & 0xff;         // LSB of # of bytes per sector
		m_ansi_attributes[0x16] = 0x00;                                 // MSB of # of sector pulses per track
		m_ansi_attributes[0x17] = m_sectors >> 8;                       // MedSB of # of sector pulses per track
		m_ansi_attributes[0x18] = m_sectors & 0xff;                     // LSB of # of sector pulses per track
		m_ansi_attributes[0x19] = 0x00;                                 // Sectoring method

		m_ansi_attributes[0x20] = m_cylinders >> 8;              // MSB of # of cylinders
		m_ansi_attributes[0x21] = m_cylinders & 0xff;            // LSB of # of cylinders
		m_ansi_attributes[0x22] = m_heads;                       // Number of heads

		m_ansi_attributes[0x30] = 0x00;                          // Encoding method #1
		m_ansi_attributes[0x31] = 0x00;                          // Preamble #1 number of bytes
		m_ansi_attributes[0x32] = 0x00;                          // Preamble #1 pattern
		m_ansi_attributes[0x33] = 0x00;                          // Sync #1 pattern
		m_ansi_attributes[0x34] = 0x00;                          // Postamble #1 number of bytes
		m_ansi_attributes[0x35] = 0x00;                          // Postamble #1 pattern
		m_ansi_attributes[0x36] = 0x00;                          // Gap #1 number of bytes
		m_ansi_attributes[0x37] = 0x00;                          // Gap #1 pattern

		m_ansi_attributes[0x40] = 0x00;                          // Encoding Method #2
		m_ansi_attributes[0x41] = 0x00;                          // Preamble #2 number of bytes
		m_ansi_attributes[0x42] = 0x00;                          // Preamble #2 pattern
		m_ansi_attributes[0x43] = 0x00;                          // Sync #2 pattern
		m_ansi_attributes[0x44] = 0x00;                          // Postamble #2 number of bytes
		m_ansi_attributes[0x45] = 0x00;                          // Postamble #2 pattern
		m_ansi_attributes[0x46] = 0x00;                          // Gap #2 number of bytes
		m_ansi_attributes[0x47] = 0x00;                          // Gap #2 pattern
	}
	return m_ansi_attributes[m_attribute_number];
}

void ansi_disk_device::load_attribute(uint8_t attribute_value)
{
	if (m_attribute_number > 0x47) {
		logerror("LOAD_ATTRIBUTE: illegal attribute number %d\n", m_attribute_number);
		return;
	}
	m_ansi_attributes[m_attribute_number] = attribute_value;
}

void ansi_disk_device::assert_read_gate()
{
	// in a perfect world we wouldn't disable this timer, because the drive is sending data
	// directly off the disk and it's still spinning at the same rate.  but we aren't clocking
	// NRZ data, we're calling a callback (in read_clock_tick) to send the data to the
	// controller, where it will result in drqs being asserted and the data being read, and I have no
	// idea how long that'll take.
	//
	// so.. we disable the timer here and enable it again when the read gate is deasserted.  This
	// happens to line up with the entire sector being read, but I'm not sure that's a hard
	// requirement?  is it possible to leave the read gate asserted and just keep getting NRZ data
	// from the disk?  will it include the postamble/preamble for the current/next sectors?  I don't
	// think I've read anything that says one way or the other.
	m_sector_timer->enable(false);

	int sector = m_pulsed_sector;
	int cylinder = (m_current_cylinder_high << 8) | m_current_cylinder_low;
	int track = cylinder * m_heads + m_selected_head;
	int track_offset = track * m_sectors;
	int sector_offset = track_offset + sector;

	LOGDATA("read sector %d on cylinder %d and head %d\n", sector, cylinder, m_selected_head);
	LOGDATA("linearized as logical sector address %d\n", sector_offset);

	// read from the image synchronously, but report the bytes back to the controller
	// on a timer.
	fseek(sector_offset * m_sector_size, SEEK_SET);
	fread(m_buffer.get(), m_sector_size);

	m_cursor = 0;
	m_read_timer->adjust(attotime::from_nsec(960), 0, attotime::from_nsec(960));
}

TIMER_CALLBACK_MEMBER(ansi_disk_device::read_clock_tick)
{
	if (m_cursor >= m_sector_size) {
		LOGDATA("done with read and the controller didn't finish the op\n");
		m_read_timer->adjust(attotime::never);
		return;
	}

	cur_read_data_cb(this, m_buffer[m_cursor++]);
}

void ansi_disk_device::deassert_read_gate()
{
	LOGDATA("read finished\n");
	m_read_timer->reset();
	// see the comment up in assert_read_gate.
	m_sector_timer->enable();
}

void ansi_disk_device::assert_write_gate()
{
	// similar to the read gate, we disable the sector pulse while doing the write so we
	// don't get confusing events happening.
	m_sector_timer->enable(false);

	// write to the buffer until we've accumulated everything, then do a single write
	// to the image.
	m_cursor = 0;
	m_ref_timer->adjust(attotime::from_usec(1), 0, attotime::from_usec(1));
}

TIMER_CALLBACK_MEMBER(ansi_disk_device::ref_clock_tick)
{
	cur_ref_clock_tick_cb(this);
}

void ansi_disk_device::write_sector_next_byte(uint8_t data)
{
	if (m_cursor >= m_sector_size) {
		LOGDATA("done with write and the controller didn't finish the op.  ignoring.\n");
		return;
	}

	m_buffer[m_cursor++] = data;
}

void ansi_disk_device::deassert_write_gate()
{
	int sector = m_pulsed_sector;
	int cylinder = (m_current_cylinder_high << 8) | m_current_cylinder_low;
	int track = cylinder * m_heads + m_selected_head;
	int track_offset = track * m_sectors;
	int sector_offset = track_offset + sector;

	LOGDATA("write sector %d on cylinder %d and head %d\n", sector, cylinder, m_selected_head);
	LOGDATA("linearized as logical sector address %d\n", sector_offset);

	fseek(sector_offset * m_sector_size, SEEK_SET);
	fwrite(m_buffer.get(), m_sector_size);

	LOGDATA("write finished\n");

	m_ref_timer->reset();
	// relight the sector timer
	m_sector_timer->enable();
}

void ansi_disk_device::select()
{
	LOGCMD("select\n");
	m_selected = true;
}

// excepts both the command and a parameter out byte.  returns what should be the parameter in byte.
// if the command doesn't require a parameter in byte, return m_general_status.
uint8_t ansi_disk_device::execute_command(uint8_t command, uint8_t parameter)
{
	if (m_busy) {
		LOGCMD("busy, ignoring command\n");
		return 0;
	}

	switch (command) {
		case CMD_REPORT_ILLEGAL_COMMAND:
			LOGCMD("REPORT_ILLEGAL_COMMAND\n");
			// This command shall force the Illegal Command Bit to be set in the
			// General Status Byte (see Section 4.4). The General Status Byte,
			// with the Illegal Command Bit equal to one, is returned to the host
			// by the Parameter Byte of the command sequence.
			m_general_status |= GS_ILLEGAL_COMMAND;
			return m_general_status;

		case CMD_CLEAR_FAULT:
			LOGCMD("CLEAR_FAULT\n");
			// This command shall cause all fault status bits of the selected
			// device to be reset, provided the fault condition has passed. If
			// the fault condition persits the appropriate status bit shall
			// continue to be equal to one. The General Status Byte, cleared of
			// previous fault status, shall be returned by the Parameter Byte of
			// the command sequence.
			// The Clear Fault Command shall also reset the Attention Condition
			// caused by the fault condition, again only if the fault condition no
			// longer exists.

			m_general_status &= ~(
				GS_CONTROL_BUS_ERROR |
				GS_ILLEGAL_COMMAND |
				GS_ILLEGAL_PARAMETER
			);

			clear_sb1(SB1_SEEK_ERROR | SB1_RW_FAULT | SB1_POWER_FAULT | SB1_COMMAND_REJECT);

			set_attention_line(false);

			return m_general_status;

		case CMD_CLEAR_ATTENTION:
			LOGCMD("CLEAR_ATTENTION\n");
			// This command shall cause the Attention Condition to be reset in the
			// selected device. The General Status Byte shall be returned by the
			// Parameter Byte of the command sequence.
			//
			// If the error or other condition that caused the Attention Condition
			// persists, the Attention Condition shall not be set again. If,
			// however, the condition is reset and the error reoccurs, the
			// Attention Condition shall be set again.
			m_sense_byte_2 &= ~(
				SB2_INITIAL_STATE |
				SB2_READY_TRANSITION |
				SB2_DEVICE_ATTR_TABLE_MODIFIED
			);

			m_general_status &= ~GS_NORMAL_COMPLETE;

			set_attention_line(false);

			return m_general_status;

		case CMD_SEEK:
			LOGCMD("SEEK\n");
			// This command shall cause the selected device to seek to the
			// cylinder identified as the target cylinder by the Load Cylinder
			// Address Commands (see Sections 4.1.3 and 4.1.4). The General
			// Status Byte shall be returned to the host by the Parameter Byte of
			// the command sequence with the Busy Executing bit set (see Section
			// 4.4.1.7).
			// The Seek Command shall set the Attention Condition and the Illegal
			// Parameter Bit in the General Status Byte if the target cylinder
			// address is outside the cylinder address range of the device.
			// Upon completion of any seek (including a zero length seek) the
			// device shall clear the Busy Executing bit in the General Status
			// Byte and set the Attention Condition.

			m_current_cylinder_high = m_load_cylinder_high;
			m_current_cylinder_low = m_load_cylinder_low;

			start_time_dependent_command(attotime::from_msec(5)); // look up this timing...
			return m_general_status;

		case CMD_REZERO:
			LOGCMD("REZERO\n");
			// This command shall cause the selected device to position the moving
			// head(s) over cylinder zero. The General Status byte shall be
			// returned to the host by the Parameter Byte of the command sequence
			// with the Busy Executing bit set (see Section 4.4.1.7).
			//
			// Upon the completion of the positioning of the moving head(s) over
			// cylinder zero the device shall clear the Busy Executing bit in the
			// General Status byte and set the Attention Condition.
			m_current_cylinder_high = 0;
			m_current_cylinder_low = 0;

			start_time_dependent_command(attotime::from_msec(5)); // look up this timing...
			return m_general_status;

		case CMD_REPORT_SENSE_BYTE_2:
			LOGCMD("REPORT_SENSE_BYTE_2\n");
			// The command shall cause the selected device to return Sense Byte 2
			// by the Parameter Byte of the command sequence. No other action
			// shall be taken in the device.
			return m_sense_byte_2;

		case CMD_REPORT_SENSE_BYTE_1:
			LOGCMD("REPORT_SENSE_BYTE_1\n");
			// This command shall cause the selected device to return Sense Byte 1
			// by the Parameter Byte of the command sequence. No other action
			// shall be taken in the device.
			return m_sense_byte_1;

		case CMD_REPORT_GENERAL_STATUS:
			LOGCMD("REPORT_GENERAL_STATUS\n");
			// This command shall cause the selected device to return the general
			// Status Byte by the Parameter Byte of the command sequence. This
			// command shall not perform any other function in the device and acts
			// as a "no-op" in order to allow the host to monitor the device's
			// General Status Byte without changing any device condition.
			return m_general_status;

		case CMD_REPORT_ATTRIBUTE:
			// This command shall cause the selected device to return a byte of
			// information that is the Device Attribute whose number was defined
			// in the Load Attribute Number Command (see Section 4.1.6). The
			// contents of the byte is defined by Table 4-3 and Section 4.3.
			LOGCMD("REPORT_ATTRIBUTE %02x\n", m_attribute_number);
			return report_attribute();

		case CMD_SET_ATTENTION:
			LOGCMD("SET_ATTENTION\n");
			// This command shall cause the selected device to set the Attention
			// Condition. No other action shall be caused.
			// The General Status Byte shall be transferred to the host by the
			// Parameter Byte of the command sequence.

			start_time_dependent_command(attotime::from_msec(5)); // look up this timing...
			return m_general_status; // XXX is this supposed to be returned with the BUSY_EXECUTING bit or not?

		case CMD_SELECTIVE_RESET:
			// This command shall cause the selected device to reach Initial State
			// (see Section 3.2.1). This is a time dependent command and as such
			// shall set the Busy Executing bit prior to the assertion of the
			// acknowledge to parameter request and shall be reflected in the
			// returned General Status Byte. Upon completion of.the parameter
			// byte transfer the device shall go to the initial state and all
			// resetable parameter. attentions. errors. etc •• shall be reset.
			// When the initial state is reached bit 0 of Sense Byte 2 will be set
			// and bit 6 of the General Status Byte shall be cleared. (This
			// causes the setting of the Attention Condition).
			//
			// TODO
			logerror("SELECTIVE_RESET unimplemented\n");
			start_time_dependent_command(attotime::from_msec(5)); // look up this timing...
			return parameter;

		case CMD_REFORMAT_TRACK:
			// This command shall cause the selected device to reconfigure the
			// arrangement of Sector Pulse generation according to parameters
			// received via the Load Sector Pulses Per Track Commands (see
			// Sections 4.1.9 to 4.1.11). The General Status Byte shall be
			// returned to the host by the Parameter Byte of the command
			// sequence.
			// The Partition Track Command is a Time Dependent Command and as such
			// shall set the Busy Executing bit in the General Status Byte
			// returned by this command (see Section 4.4.1.7) and it is to remain
			// set while this command execution is in process. Also. the device
			// shall exercise appropriate control over the Busy signal at the
			// interface (see Section 3.2.7).
			// Upon the completion of execution of this command the Bytes Per
			// Sector and the Sector Per Track will be updated in the Attribute
			// Table and also bit 6 of Attribute byte OE Hex will be cleared and
			// this shall set the Attention Condition.
			// The Partition Track Command shall set the Attention Condition and
			// the Illegal Parameter Bit in the General Status Byte if the Sector
			// Pulses Per Track create a set that is outside the range of the
			// device.
			// activating Read Gate or Write Gate while this command is executing
			// is a violation of protocol.
			//
			// TODO
			logerror("REFORMAT_TRACK unimplemented\n");
			start_time_dependent_command(attotime::from_msec(5)); // look up this timing...
			return m_general_status; // XXX is this supposed to be returned with the BUSY_EXECUTING bit or not?

		case CMD_REPORT_CYL_ADDR_HIGH:
			// This command shall cause the selected device to return a byte of
			// information that is the most significant byte of a 16 bit .number
			// that, indicates the cylinder address of the current position of the
			// moving heads. This number shall not reflect the most recent
			// cylinder address set by the Set Cylinder Address Commands (see
			// Sections 4.1.3 and 4.1.4) ,unless there has been an intervening Seek
			// Command completed (see Section 4.2.4).
			// If executed during a seek operation, the information returned shall
			// be ascertained by the vendor specification.
			// The information shall be transferred by the Parameter Byte of the
			// command sequence.
			LOGCMD("REPORT_CYL_ADDR_HIGH %02x\n", m_current_cylinder_high);
			return m_current_cylinder_high;

		case CMD_REPORT_CYL_ADDR_LOW:
			// This command shall cause the selected device to return a byte of
			// information that is the least significant byte of a 16 bit number
			// that indicates the cylinder address of the current position of the
			// moving heads. This number shall not reflect the most recent
			// cylinder address set by the Set Cylinder Address Commands (see
			// Sections 4.1.3 and 4.1.4) unless there has been an intervening Seek
			// Command completed (see Section 4.2.4).
			// If executed during a seek operation, the information returned shall
			// be ascertained by the vendor specification.
			// The information shall be transferred by the Parameter Byte of the
			// command sequence.
			//
			LOGCMD("REPORT_CYL_ADDR_LOW %02x\n", m_current_cylinder_low);
			return m_current_cylinder_low;

		case CMD_REPORT_TEST_BYTE:
			// This command shall cause the selected device to return a copy of
			// the Test Byte transferred to the device via the Load 'Test Byte
			// Command. (See Section 4.1.12.)
			// The Test Byte shall be transferred by the Parameter Byte of the
			// command sequence.
			//
			// TODO
			LOGCMD("REPORT_TEST_BYTE %02x\n", m_test_byte);
			return m_test_byte;

		case CMD_ATTENTION_CONTROL:
			LOGCMD("ATTENTION_CONTROL %02x\n", parameter);
			// This command shall condition the selected device to enable or
			// disable its attention circuitry based on the value of the Parameter
			// Byte as shown below.
			// 7 6 5 4 3 2 1 0
			// | o o o o o o o
			// |
			// o - Enable Attention
			// 1 - Disable Attention
			//
			// This command allows the host to selectively ignore attention
			// requests from certain devices on the interface. This might be done
			// in response to a device that generates spurious attention requests
			// due to a malfuntion.
			// The Enable Attention Command shall cause the selected device to
			// gate its internal Attention Condition onto the party line ("wired
			// OR") Attention Signal. The Disable Attention Command shall cause
			// the selected device to disable the gating of the internal Attention
			// Condition onto the party line Attention Signal. This command shall
			// have no impact on the function of the radial status returned with
			// the Attention In Strobe Signal (see Signal 3.2.3.2).
			// Devices shall be initilized with the Attention circuitry enabled.
			//
			m_attention_enabled = (parameter & 0x80) ? false : true;
			return parameter;

		case CMD_WRITE_CONTROL:
			LOGCMD("WRITE_CONTROL %02x\n", parameter);
			// This command shall condition the selected device to enable or
			// disable its write circuitry based on the value of the parameter
			// Byte as shown below:
			// 7 6 5 4 3 2 1 0
			// | o o o o o o o
			// |
			// 1 - Write Enable
			// o - Write Disable
			//
			// This command is used in conjunction with the Write Gate Signal and
			// therefore merely enables the write circuitry while the Write Gate
			// Signal activates the circuitry at the proper time. An active Write
			// Gate Signal while the device's write circuitry is disabled shall
			// result in no data being recorded.
			// Devices shall be initialized with the write circuitry disabled.
			// A Write Control Command execute during a write operation is a
			// violation of protocol.
			//
			m_write_enabled = (parameter & 0x80) != 0;
			return parameter;

		case CMD_LOAD_CYL_ADDR_HIGH:
			// This command shall condition the selected device to accept the
			// Parameter Byte as the most significant Byte of a cylinder address.
			// This command is used in conjunction with the Seek Command (see
			// Section 4.2.4) and therefore is a means of supplying the most
			// significant byte of a target cylinder address.
			// This command shall not cause any head motion. Loading a cylinder
			// address outside the range of a device shall not cause an error
			// unless a subsequent Seek Command is issued to that illegal
			// cylinder.
			// Devices shall be initialized with the target cylinder address equal
			// to zero.
			//
			LOGCMD("LOAD_CYL_ADDR_HIGH %02x\n", parameter);
			m_load_cylinder_high = parameter;
			return parameter;

		case CMD_LOAD_CYL_ADDR_LOW:
			// This command shall condition the selected device to accept the
			// Parameter Byte as the least signficant byte of a cylinder address.
			// This command is used in conjunction with the Seek Command (see
			// Section 4.2.4) and therefore is a means of supplying the least
			// significant byte of a target cylinder address.
			// This command shall not cause any head motion. Loading a cylinder
			// address outside the range of a device shall not cause an error
			// unless a subsequent seek command is issued to that illegal
			// cylinder.
			// Devices shall be initialized with the target cylinder address equal
			// to zero.
			//
			LOGCMD("LOAD_CYL_ADDR_LOW %02x\n", parameter);
			m_load_cylinder_low = parameter;
			return parameter;

		case CMD_SELECT_HEAD:
			LOGCMD("SELECT_HEAD %d\n", parameter);
			// This command shall condition the selected device to. accept the
			// Parameter Byte as the binary address of the head selected for read
			// or write operations. This command shall enable the moving heads
			// and shall disable the fixed heads.
			// A Select Moving Head Command issued during a read or write
			// operation is a violation of protocol.
			// The device shall set the Attention Condition and the Illegal
			// Parameter Bit in the General Status Byte upon receipt of a head
			// address outside the head address range of the device.
			// Devices shall be initialized with moving head zero selected.
			m_selected_head = parameter;
			return parameter;

		case CMD_LOAD_ATTRIBUTE_NUMBER:
			LOGCMD("LOAD_ATTRIBUTE_NUMBER %02x\n", parameter);
			// This command shall condition the selected device to accept the
			// Parameter Byte as the number of a Device Attribute as defined in
			// Table 4-3. This command prepares the device for a subsequent Load
			// Device Attribute Command or Report Device Attribute Command (see
			// Sections 4.1.7 and 4.2.9). This command may be issued at any time.
			m_attribute_number = parameter;
			return parameter;

		case CMD_LOAD_ATTRIBUTE:
			// This command shall condition the selected device to accept the
			// Parameter Byte as the new value of a Device Attribute. The number
			// of the Device Attribute must have been previously defined by the
			// Load Attribute Number Command (see Section 4.1.6).
			LOGCMD("LOAD_ATTRIBUTE %02x = %02x\n", m_attribute_number, parameter);
			load_attribute(parameter);
			return parameter;

		case CMD_SPIN_CONTROL:
			LOGCMD("SPIN_CONTROL %02x\n", parameter);
			// This command shall condition the seleted device to enter a spin up
			// or spin down cycle based on the value of the Parameter Byte as
			// shown below.
			// 7 6 5 4 3 2 1 0
			// | o o o o o o o
			// |
			// 1 - Spin Up
			// o - Spin Down
			// A spin up cycle shall consist of starting the rotation of the
			// spindle. A spin down cycle shall consist of stopping the rotation
			// of the spindle.
			// Upon completion of a spin control cycle the device shall set the
			// Attention Condition. Issuing a spin up command to a device whose
			// spindle is already at full speed or issuing a spin down command to
			// a device whose spindle has already stopped shall also set the
			// Attention Condition.
			// The Spin Control Command is a Time Dependent Command and as such
			// shall set the Busy Executing bit in the General Status Byte (see
			// Section 4.4.1.7) while command execution is in process. Also, the
			// device shall exercise appropriate control over the Busy signal at
			// the interface (see Section 3.2.7).
			// A spin down cycle shall cause the repositioning of the moving
			// head(s) over the landing zone and stop the rotation of the
			// spindle. If the device detects that it cannot successfully seek to
			// the landing zone it shall set the Attention Condition and set bit 0
			// of Sense Byte 1.
			// See vendor specification for initial state of the Spin Control.

			start_time_dependent_command(attotime::from_msec(10));
			return m_general_status;

		case CMD_LOAD_SECT_PER_TRACK_HIGH:
			logerror("LOAD_SECT_PER_TRACK_HIGH unimplemented\n");
			return parameter;

		case CMD_LOAD_SECT_PER_TRACK_MEDIUM:
			logerror("LOAD_SECT_PER_TRACK_MEDIUM unimplemented\n");
			return parameter;

		case CMD_LOAD_SECT_PER_TRACK_LOW:
			logerror("LOAD_SECT_PER_TRACK_LOW unimplemented\n");
			return parameter;

		case CMD_LOAD_BYTES_PER_SECT_HIGH:
			logerror("LOAD_BYTES_PER_SECT_HIGH unimplemented\n");
			return parameter;

		case CMD_LOAD_BYTES_PER_SECT_MEDIUM:
			logerror("LOAD_BYTES_PER_SECT_MEDIUM unimplemented\n");
			return parameter;

		case CMD_LOAD_BYTES_PER_SECT_LOW:
			logerror("LOAD_BYTES_PER_SECT_LOW unimplemented\n");
			return parameter;

		case CMD_LOAD_READ_PERMIT_HIGH:
			logerror("LOAD_READ_PERMIT_HIGH unimplemented\n");
			return parameter;

		case CMD_LOAD_READ_PERMIT_LOW:
			logerror("LOAD_READ_PERMIT_LOW unimplemented\n");
			return parameter;

		case CMD_LOAD_WRITE_PERMIT_HIGH:
			logerror("LOAD_WRITE_PERMIT_HIGH unimplemented\n");
			return parameter;

		case CMD_LOAD_WRITE_PERMIT_LOW:
			logerror("LOAD_WRITE_PERMIT_LOW unimplemented\n");
			return parameter;

		case CMD_LOAD_TEST_BYTE:
			LOGCMD("LOAD_TEST_BYTE %02x\n", parameter);
			m_test_byte = parameter;
			return m_general_status;

		default:
			logerror("unknown ANSI command %02x\n", command);
			return parameter;
	}
}

void ansi_disk_device::clear_sb1(uint8_t value) {
	m_sense_byte_1 &= ~value;
}

void ansi_disk_device::start_time_dependent_command(attotime duration) {
	set_busy_line(true);
	m_general_status |= GS_BUSY_EXECUTING;
	m_time_dependent_timer->adjust(duration, 0);
}

void ansi_disk_device::set_attention_line(bool state) {
	LOGSIGNALS("set_attention_line(%d, cur=%d)\n", state, m_attention);
	if (m_attention != state) {
		m_attention = state;
		cur_attention_cb(this, state);
	}
}

void ansi_disk_device::set_busy_line(bool state) {
	LOGSIGNALS("set_busy_line(%d, cur=%d)\n", state, m_busy);
	if (m_busy != state) {
		m_busy = state;
		cur_busy_cb(this, state);

		if (!m_busy && m_attention_enabled) {
			set_attention_line(true);
		}
	}
}

TIMER_CALLBACK_MEMBER(ansi_disk_device::finish_time_dependent_command) {
	LOGSIGNALS("time dependent command finished\n");
	m_general_status |= GS_NORMAL_COMPLETE;
	m_general_status &= ~GS_BUSY_EXECUTING;
	set_busy_line(false);
}

TIMER_CALLBACK_MEMBER(ansi_disk_device::sector_callback) {
	m_pulsed_sector = (m_pulsed_sector+1) % m_sectors;

	// if we aren't selected we shouldn't be generating pulses, but the platters are still a'spinnin'
	if (!m_selected) {
		return;
	}

	if (m_pulsed_sector == 0) {
		cur_index_pulse_cb(this);
	} else {
		cur_sector_pulse_cb(this);
	}
}
