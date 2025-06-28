// license:BSD-3-Clause
// copyright-holders:Wilbert Pol
/**********************************************************************************

CADR disk controller emulation

There were 2 version of the disk controller: a single board version which
connected to 1 hard disk and a 2 board version which could connect up to 8
drives.

It retrieves "CCW"s to process from main memory. Reads
and writes go directly to/from main memory.
The Command List Pointer contains the number of the physical
memory page where the CCW's are stored.

The disk controller supports at least the following disk types:
- T-80 - 80MB, 815 cylinders, 5 heads, 17 sectors per track, 1024 bytes per sector. Max transfer rate 1209KB/s, 6ms seek time; 3600 rpm.
- T-300 - 300MB, 815 cylinders, 19 heads, 17 sectors per track, 1024 bytes per sector. Max tranfer rate 1209KB/s, 6ms seek time; 3600 rpm.

Implementation based on description of the operation, not schematics.


TODO:
- Disk access times.
- THe disk controller reads/writes directly to memory, implications on cpu slowdown unknown.

**********************************************************************************/

#include "emu.h"
#include "cadr_disk.h"


//#define VERBOSE (LOG_GENERAL)
//#define LOG_OUTPUT_FUNC osd_printf_info
#include "logmacro.h"


DEFINE_DEVICE_TYPE(CADR_DISK, cadr_disk_device, "cadr_disk", "CADR disk controller")


cadr_disk_device::cadr_disk_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, CADR_DISK, tag, owner, clock)
	, m_data_space(*this, finder_base::DUMMY_TAG, -1)
	, m_harddisk(*this, "harddisk")
	, m_irq_cb(*this)
{
}


void cadr_disk_device::device_add_mconfig(machine_config &config)
{
	HARDDISK(config, m_harddisk); // T-80 (80MB) or T-300 (300MB)
}


void cadr_disk_device::device_start()
{
	save_item(NAME(m_command));
	save_item(NAME(m_status));
	save_item(NAME(m_clp));
	save_item(NAME(m_last_memory_address));

	m_disk_timer = timer_alloc(FUNC(cadr_disk_device::disk_done_callback), this);
}


void cadr_disk_device::device_reset()
{
	m_command = 0;
	m_status = 1;
	m_clp = 0;
	m_last_memory_address = 0;
}


TIMER_CALLBACK_MEMBER(cadr_disk_device::disk_done_callback)
{
	m_status |= 1;
	if (BIT(m_command, 11))
	{
		m_status |= 0x08;
		m_irq_cb(ASSERT_LINE);
	}
}


// xxxxxxxx -------- -------- -------- Block counter of unit
//          x------- -------- -------- Internal parity error
//           x------ -------- -------- Read compare difference
//            x----- -------- -------- CCW cycle
//             x---- -------- -------- Nonexistent memory error
//              x--- -------- -------- Memory parity error
//               x-- -------- -------- Header compare error
//                x- -------- -------- Header ECC error
//                 x -------- -------- ECC Hard
//                   x------- -------- ECC Soft
//                    x------ -------- Read overrun
//                     x----- -------- Write overrun
//                      x---- -------- Start block error
//                       x--- -------- Timeout error
//                        x-- -------- Selected unit seek error
//                         x- -------- Selected unit not online
//                          x -------- Selected unit not on cilinder
//                            x------- Selected unit read-only
//                             x------ Selected unit fault
//                              x----- No unit selected
//                               x---- Multiple units selected
//                                x--- Interrupt request
//                                 x-- Selected unit attention
//                                  x- Any attention
//                                   x Not active
u32 cadr_disk_device::status_r()
{
	if (!machine().side_effects_disabled())
		LOG("Disk Controller status read\n");
	return m_status;
}


u32 cadr_disk_device::command_list_r()
{
	if (!machine().side_effects_disabled())
		LOG("Disk Controller command list pointer read\n");
	return m_clp;
}


// xxxxxxxx -------- -------- -------- Not used
//          xx------ -------- -------- Disk type
//            xxxxxx xxxxxxxx xxxxxxxx Address of the last memory reference
u32 cadr_disk_device::memory_address_r()
{
	if (!machine().side_effects_disabled())
		LOG("Disk Controller memory address read\n");
	return m_last_memory_address;
}


// x------- -------- -------- -------- Not used
//  xxx---- -------- -------- -------- Unit number
//     xxxx xxxxxxxx -------- -------- Cylinder number
//                   xxxxxxxx -------- Head number
//                            xxxxxxxx Block number
u32 cadr_disk_device::disk_address_r()
{
	if (!machine().side_effects_disabled())
		LOG("Disk Controller disk address read\n");
	return (m_unit << 28) | (m_cyl << 16) | (m_head << 8) | m_block;
}


// xxxxxxxx xxxxxxxx -------- -------- Error pattern bits
//                   xxxxxxxx xxxxxxxx Error bit position + 1
u32 cadr_disk_device::error_correction_r()
{
	if (!machine().side_effects_disabled())
		LOG("Disk Controller error correction read\n");
	return 0;
}


// xxxxxxxx xxxxxxxx xxxx---- -------- Not used
//                       x--- -------- Done interrupt enable
//                        x-- -------- Attention interrupt enable
//                         x- -------- Recalibrate
//                          x -------- Fault clear
//                            x------- Data strobe early
//                             x------ Data strobe late
//                              x----- Servo offset
//                               x---- Offset forward
//                                x--- I/O Direction
//                                 xxx Command code
void cadr_disk_device::command_w(u32 data)
{
	LOG("Disk controller command write %08x\n", data);
	m_command = data;

	// Clear interrupt bit
	if (!BIT(m_command, 11))
	{
		m_status &= ~0x08;
		m_irq_cb(CLEAR_LINE);
	}
}


void cadr_disk_device::command_list_w(u32 data)
{
	LOG("Disk controller command list pointer write %08x\n", data);
	m_clp = data;
}


// x------- -------- -------- -------- Not used
//  xxx---- -------- -------- -------- Unit number
//     xxxx xxxxxxxx -------- -------- Cylinder number
//                   xxxxxxxx -------- Head number
//                            xxxxxxxx Block number
void cadr_disk_device::disk_address_w(u32 data)
{
	LOG("Disk controller disk address write %08x\n", data);
	m_unit = (data >> 28) & 0x07;
	m_cyl = (data >> 16) & 0xfff;
	m_head = (data >> 8) & 0xff;
	m_block = data & 0xff;
}


void cadr_disk_device::start_w(u32 data)
{
	LOG("Disk controller start command\n");

	m_status = m_status & ~5;

	switch (m_command & 0x3ff)
	{
	case 0x000: // 0000 - Read
		if (m_harddisk->exists())
		{
			u32 max_ccws = 0x10000;
			const auto &info = m_harddisk->get_info();
			u32 sector = (m_cyl * info.heads * info.sectors) + (m_head * info.sectors) + m_block;
			u8 buffer[1024];
			do
			{
				const u32 ccw = m_data_space->read_dword(m_clp);
				const u16 page = (ccw >> 8) & 0x7fff;
				LOG("Start read sector %d\n", sector);
				m_harddisk->read(sector, &buffer[0]);
				// Move data to ram
				LOG("Write sector data to %08x\n", page << 8);
				for (int i = 0; i < 1024; i += 4)
				{
					u32 data = (buffer[i + 3] << 24) | (buffer[i + 2] << 16) | (buffer[i + 1] << 8) | buffer[i];
					m_data_space->write_dword((page * 256) + (i / 4), data);
				}
				if (BIT(ccw, 0) && max_ccws)
				{
					sector++;
					m_clp++;
					max_ccws--;
				}
				else
				{
					max_ccws = 0;
				}
				m_last_memory_address = ccw & 0xffffff00;
			} while (max_ccws);
			m_block = sector % info.sectors;
			sector /= info.sectors;
			m_head = sector % info.heads;
			m_cyl = sector / info.heads;
			m_status |= 1;
			m_disk_timer->adjust(attotime::from_msec(6));
		}
		else
		{
			m_status |= 0x8200; // The timeout bit is checked for a unit that is not present?
			m_status |= 1;
			m_disk_timer->adjust(attotime::from_msec(6));
		}
		break;

	case 0x005: // 0005 - At ease
		m_status &= ~0x04;
		m_status |= 1;
		break;

	case 0x009: // 0011 - Write
		LOG("Start write\n");
		if (m_harddisk->exists())
		{
			u32 max_ccws = 0x10000;
			const auto &info = m_harddisk->get_info();
			u32 sector = (m_cyl * info.heads * info.sectors) + (m_head * info.sectors) + m_block;
			u8 buffer[1024];
			do
			{
				const u32 ccw = m_data_space->read_dword(m_clp);
				const u16 page = (ccw >> 8) & 0x7fff;
				LOG("Start write sector %d\n", sector);
				LOG("Read sector data from %08x\n", page << 8);
				for (int i = 0; i < 256; i++)
				{
					u32 data = m_data_space->read_dword((page * 256) + i);
					buffer[(i * 4) + 3] = (data >> 24) & 0xff;
					buffer[(i * 4) + 2] = (data >> 16) & 0xff;
					buffer[(i * 4) + 1] = (data >> 8) & 0xff;
					buffer[i * 4] = data & 0xff;
				}
				m_harddisk->write(sector, &buffer[0]);
				if (BIT(ccw, 0) && max_ccws)
				{
					sector++;
					m_clp++;
					max_ccws--;
				}
				else
				{
					max_ccws = 0;
				}
				m_last_memory_address = ccw & 0xffffff00;
			} while (max_ccws);
			m_block = sector % info.sectors;
			sector /= info.sectors;
			m_head = sector % info.heads;
			m_cyl = sector / info.heads;
			m_status |= 1;
			m_disk_timer->adjust(attotime::from_msec(6));
		}
		break;

	case 0x105: // 0405 - Fault clear
		LOG("Start fault clear\n");
		m_status |= 1;
		break;

	case 0x205: // 1005 - Recalibrate
		LOG("Start recalibrate\n");
		m_status |= 5;
		break;

	case 0x008: // 0010 - Read and compare
	case 0x004: // 0004 - Seek
	default:
		fatalerror("Unknown disk controller command %03x initiated\n", m_command & 0x3ff);
		break;
	}
}


u32 cadr_disk_device::read(offs_t offset)
{
	switch (offset)
	{
	case 0x04:
		return status_r();
	case 0x05:
		return command_list_r();
	case 0x06:
		return disk_address_r();
	case 0x07:
		return error_correction_r();
	}
	return 0xffffffff;
}


void cadr_disk_device::write(offs_t offset, u32 data)
{
	switch (offset)
	{
	case 0x04:
		command_w(data);
		break;
	case 0x05:
		command_list_w(data);
		break;
	case 0x06:
		disk_address_w(data);
		break;
	case 0x07:
		start_w(data);
		break;
	}
}
