// license:BSD-3-Clause
// copyright-holders:usernameak
/**********************************************************************

    GRiD 2102 Portable Floppy emulation

    http://deltacxx.insomnia247.nl/gridcompass/disk_info.txt for some protocol info
    http://deltacxx.insomnia247.nl/gridcompass/fdd_boards.jpg for floppy drive boards photo

**********************************************************************/

#include "emu.h"
#include "grid2102.h"

#include "harddisk.h"
#include "multibyte.h"

#include <algorithm>
#include <array>
#include <queue>
#include <string_view>
#include <vector>

#define LOG_BYTES_MASK       (LOG_GENERAL << 1)
#define LOG_GPIB_STATE_MASK  (LOG_GENERAL << 2)
#define LOG_GPIB_LINES_MASK  (LOG_GENERAL << 3)
#define LOG_DISK_OPS_MASK    (LOG_GENERAL << 4)

#define LOG_BYTES(...)       LOGMASKED(LOG_BYTES_MASK, __VA_ARGS__)
#define LOG_GPIB_STATE(...)  LOGMASKED(LOG_GPIB_STATE_MASK, __VA_ARGS__)
#define LOG_GPIB_LINES(...)  LOGMASKED(LOG_GPIB_LINES_MASK, __VA_ARGS__)
#define LOG_DISK_OPS(...)    LOGMASKED(LOG_DISK_OPS_MASK, __VA_ARGS__)

#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"


namespace {

static constexpr uint8_t GPIB_CMD_GROUP_MASK = 0xe0;
static constexpr uint8_t GPIB_CMD_ADDR_MASK = 0x1f;
static constexpr uint8_t GPIB_CMD_DCL = 0x14;
static constexpr uint8_t GPIB_CMD_SPE = 0x18;
static constexpr uint8_t GPIB_CMD_SPD = 0x19;
static constexpr uint8_t GPIB_CMD_MLA = 0x20;
static constexpr uint8_t GPIB_CMD_MTA = 0x40;
static constexpr uint8_t GPIB_CMD_UNL = 0x3f;
static constexpr uint8_t GPIB_CMD_UNT = 0x5f;

static constexpr uint8_t GPIB_STATUS_RQS = 0x40;

static constexpr int REQUEST_LENGTH = 10;
static constexpr int STATUS_RESPONSE_LENGTH = 7;

// GRiD GPIB disk devices always use 512-byte sectors, other sector sizes are not supported.
static constexpr uint16_t SECTOR_SIZE = 512;

static constexpr int GRID2101_HARDDISK_DEV_ADDR = 4;
static constexpr int GRID2102_DEV_ADDR = 6;

enum class grid210x_gpib_state
{
	idle,
	wait_dav_false,
	send_data_start,
	wait_ndac_false,
};

enum class grid210x_ignore_state
{
	none,
	wait_atn_high,
	wait_atn_low,
};

enum class grid210x_state
{
	idle,
	reading_data,
	writing_data,
	writing_data_wait,
	formatting,
};

enum class disk_command : uint8_t
{
	initialize = 0,
	get_status = 1,
	read = 4,
	write = 5,
	format = 17,
};

bool parse_disk_command(uint8_t value, disk_command &command)
{
	switch (value)
	{
	case uint8_t(disk_command::initialize):
	case uint8_t(disk_command::get_status):
	case uint8_t(disk_command::read):
	case uint8_t(disk_command::write):
	case uint8_t(disk_command::format):
		command = disk_command(value);
		return true;

	default:
		return false;
	}
}

//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> grid210x_device
class grid210x_device : public device_t,
						public device_ieee488_interface,
						public device_image_interface
{
public:
	struct disk_status
	{
		uint16_t sector_size;
		uint16_t logical_sector_size;
		uint16_t sector_count;
		uint8_t drive_status;
		uint16_t bitmap_fid;
		uint16_t superblock_fid;
		uint16_t min_dir_pages;
		uint8_t flush;
		std::string_view name;
		uint16_t bytes_per_sector;
		uint16_t sectors_per_track;
		uint16_t tracks_per_cylinder;

		std::array<uint8_t, 52> serialize() const;
	};

	// construction/destruction
	grid210x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int bus_addr, attotime read_delay = attotime::from_msec(5));

protected:
	// device-level overrides
	virtual void device_start() override ATTR_COLD;

	// device_ieee488_interface overrides
	virtual void ieee488_eoi(int state) override;
	virtual void ieee488_dav(int state) override;
	virtual void ieee488_nrfd(int state) override;
	virtual void ieee488_ndac(int state) override;
	virtual void ieee488_ifc(int state) override;
	virtual void ieee488_srq(int state) override;
	virtual void ieee488_atn(int state) override;
	virtual void ieee488_ren(int state) override;

	// image-level overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return true; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual const char *file_extensions() const noexcept override { return "img"; }
	virtual const char *image_type_name() const noexcept override { return "floppydisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "flop"; }

	void accept_transfer();
	void start_talking_if_ready();
	void update_handshake(int atn);

	virtual disk_status get_status() = 0;

private:
	TIMER_CALLBACK_MEMBER(delay_tick);
	TIMER_CALLBACK_MEMBER(talk_tick);

	grid210x_gpib_state m_gpib_loop_state;
	grid210x_state m_floppy_loop_state;
	uint8_t m_last_recv_byte;
	int m_last_recv_eoi;
	int m_last_recv_atn;
	uint8_t m_byte_to_send;
	int m_send_eoi;
	bool listening, talking, serial_polling;
	grid210x_ignore_state m_ignore_state;
	bool has_srq;
	uint8_t serial_poll_byte;
	uint32_t floppy_sector_number;
	int bus_addr;
	std::vector<uint8_t> m_data_buffer;
	std::queue<uint8_t> m_output_data_buffer;
	uint16_t io_size;
	emu_timer *m_delay_timer;
	emu_timer *m_talk_timer;

protected:
	attotime read_delay;
};

class grid2102_device : public grid210x_device {
public:
	// construction/destruction
	grid2102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual disk_status get_status() override;
};

class grid2101_floppy_device : public grid210x_device {
public:
	// construction/destruction
	grid2101_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

protected:
	virtual disk_status get_status() override;
};

class grid2101_hdd_device : public grid210x_device {
public:
	// construction/destruction
	grid2101_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock);

	// image-level overrides
	virtual const char *image_type_name() const noexcept override { return "harddisk"; }
	virtual const char *image_brief_type_name() const noexcept override { return "hard"; }

protected:
	virtual disk_status get_status() override;
};

static const grid210x_device::disk_status fdd_status
{
	SECTOR_SIZE,
	504,
	360 * 1024 / SECTOR_SIZE,  // 360KiB, 5.25  DS/DD
	1,  // ok
	0x120,
	0x121,
	1,
	0,
	"48 TPI DS DD FLOPPY 30237-00",
	SECTOR_SIZE,
	9,
	2,
};

static const grid210x_device::disk_status hdd_status
{
	SECTOR_SIZE,
	504,
	(10 * 1024 * 1024) / SECTOR_SIZE,  // 10 MB by default
	1,  // ok
	0x2400,
	0x2420,
	10,
	0,
	"MAME HARD DISK DRIVE",
	SECTOR_SIZE,
	10,
	4,
};


std::array<uint8_t, 52> grid210x_device::disk_status::serialize() const
{
	std::array<uint8_t, 52> out{};
	put_u16le(&out[0], sector_size);
	put_u16le(&out[2], logical_sector_size);
	put_u16le(&out[4], sector_count);
	out[6] = drive_status;
	put_u16le(&out[7], bitmap_fid);
	put_u16le(&out[9], superblock_fid);
	put_u16le(&out[11], min_dir_pages);
	out[13] = flush;
	std::fill_n(std::begin(out) + 14, 32, ' ');
	std::copy_n(name.data(), std::min<size_t>(name.length(), 32), &out[14]);
	put_u16le(&out[46], bytes_per_sector);
	put_u16le(&out[48], sectors_per_track);
	put_u16le(&out[50], tracks_per_cylinder);
	return out;
}

grid210x_device::grid210x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int bus_addr, attotime read_delay) :
	device_t(mconfig, type, tag, owner, clock),
	device_ieee488_interface(mconfig, *this),
	device_image_interface(mconfig, *this),
	m_gpib_loop_state(grid210x_gpib_state::idle),
	m_floppy_loop_state(grid210x_state::idle),
	listening(false),
	talking(false),
	serial_polling(false),
	m_ignore_state(grid210x_ignore_state::none),
	has_srq(false),
	serial_poll_byte(0),
	bus_addr(bus_addr),
	read_delay(read_delay)
{

}

void grid210x_device::device_start() {
	m_bus->ndac_w(this, 1);
	m_bus->nrfd_w(this, 1);
	m_delay_timer = timer_alloc(FUNC(grid210x_device::delay_tick), this);
	m_talk_timer = timer_alloc(FUNC(grid210x_device::talk_tick), this);
}

TIMER_CALLBACK_MEMBER(grid210x_device::delay_tick) {
	if (m_floppy_loop_state == grid210x_state::reading_data) {
		std::unique_ptr<uint8_t[]> data(new uint8_t[io_size]);
		fseek(floppy_sector_number * SECTOR_SIZE, SEEK_SET);
		fread(data.get(), io_size);
		for (int i = 0; i < io_size; i++) {
			m_output_data_buffer.push(data[i]);
		}
	} else if (m_floppy_loop_state == grid210x_state::formatting) {
		const uint32_t sector_total = get_status().sector_count;

		uint8_t buf[SECTOR_SIZE];
		std::fill(std::begin(buf), std::end(buf), 0xe5);
		std::fill_n(std::begin(buf), 8, 0xff);

		for (uint32_t sec = 0; sec < sector_total; sec++) {
			fseek(s64(sec) * SECTOR_SIZE, SEEK_SET);
			fwrite(buf, SECTOR_SIZE);
		}

		for (int i = 0; i < STATUS_RESPONSE_LENGTH; i++) {
			m_output_data_buffer.push(0);
		}
	} else if (m_floppy_loop_state == grid210x_state::writing_data_wait) {
		for (int i = 0; i < STATUS_RESPONSE_LENGTH; i++) {
			m_output_data_buffer.push(0);
		}
	} else {
		return;
	}

	serial_poll_byte = 0x0f;
	has_srq = true;
	m_bus->srq_w(this, 0);
	m_floppy_loop_state = grid210x_state::idle;
}

TIMER_CALLBACK_MEMBER(grid210x_device::talk_tick) {
	if (m_gpib_loop_state == grid210x_gpib_state::send_data_start) {
		ieee488_nrfd(m_bus->nrfd_r());
	}
}

void grid210x_device::ieee488_eoi(int state) {
	LOG_GPIB_LINES("grid210x_device eoi state set to %d\n", state);
}

void grid210x_device::accept_transfer() {
	if (m_floppy_loop_state == grid210x_state::idle) {
		if (m_data_buffer.size() == REQUEST_LENGTH) {
			const uint8_t command_byte = m_data_buffer[0];
			disk_command command;
			uint32_t sector_number = get_u32le(&m_data_buffer[3]);
			uint16_t data_size = get_u16le(&m_data_buffer[7]);
			LOG("grid210x_device command %u, data size %u, sector no %u\n", (unsigned)command_byte, (unsigned)data_size, (unsigned)sector_number);
			if (!parse_disk_command(command_byte, command)) {
				LOG("grid210x_device ignoring unsupported command %u, data size %u, sector no %u\n", (unsigned)command_byte, (unsigned)data_size, (unsigned)sector_number);
				return;
			}

			(void)(sector_number);
			switch (command) {
			case disk_command::initialize:
				for (int i = 0; i < STATUS_RESPONSE_LENGTH; i++) { // just OK
					m_output_data_buffer.push(0);
				}
				break;

			case disk_command::get_status:
				for (uint8_t b : get_status().serialize()) {
					m_output_data_buffer.push(b);
				}
				break;

			case disk_command::read:
				floppy_sector_number = sector_number;
				io_size = data_size;
				m_floppy_loop_state = grid210x_state::reading_data;
				m_delay_timer->adjust(read_delay);
				break;

			case disk_command::write:
				floppy_sector_number = sector_number;
				io_size = data_size;
				m_floppy_loop_state = grid210x_state::writing_data;
				break;

			case disk_command::format:
				m_floppy_loop_state = grid210x_state::formatting;
				m_delay_timer->adjust(read_delay);
				break;

			default:
				LOG("grid210x_device unhandled command %u, data size %u, sector no %u\n", (unsigned)command_byte, (unsigned)data_size, (unsigned)sector_number);
				break;
			}
		} // else something is wrong, ignore
	} else if (m_floppy_loop_state == grid210x_state::writing_data) {
		// write
		if (floppy_sector_number < 0xfffF) {
			fseek(floppy_sector_number * SECTOR_SIZE, SEEK_SET);
			fwrite(m_data_buffer.data(), m_data_buffer.size());
		} else {
			// TODO: set status
		}
		LOG_DISK_OPS("grid210x_device write sector %u\n", floppy_sector_number);
		// wait
		m_floppy_loop_state = grid210x_state::writing_data_wait;
		m_delay_timer->adjust(read_delay);
	}
}

void grid210x_device::start_talking_if_ready() {
	if (m_gpib_loop_state != grid210x_gpib_state::idle || m_ignore_state != grid210x_ignore_state::none || !talking || !m_bus->atn_r()) {
		return;
	}

	if (serial_polling) {
		bool had_srq = has_srq;
		if (has_srq) {
			has_srq = false;
			m_bus->srq_w(this, 1);
		}
		m_byte_to_send = serial_poll_byte | (had_srq ? GPIB_STATUS_RQS : 0);
		serial_poll_byte = 0;
		m_send_eoi = 1;
		m_gpib_loop_state = grid210x_gpib_state::send_data_start;
		m_talk_timer->adjust(attotime::zero);
	} else if (!m_output_data_buffer.empty()) {
		m_byte_to_send = m_output_data_buffer.front();
		m_output_data_buffer.pop();
		m_send_eoi = m_output_data_buffer.empty() ? 1 : 0;
		m_gpib_loop_state = grid210x_gpib_state::send_data_start;
		m_talk_timer->adjust(attotime::zero);
	}
}

void grid210x_device::ieee488_dav(int state) {
	if (m_ignore_state != grid210x_ignore_state::none) {
		m_bus->nrfd_w(this, 1);
		m_bus->ndac_w(this, 1);
		return;
	}

	if(state == 0 && m_gpib_loop_state == grid210x_gpib_state::idle) {
		// read data and wait for transfer end
		int atn = m_bus->atn_r() ^ 1;
		m_bus->nrfd_w(this, 0);
		uint8_t data = m_bus->dio_r() ^ 0xff;
		int eoi = m_bus->eoi_r() ^ 1;
		LOG_BYTES("grid210x_device byte recv %02x atn %d eoi %d\n", data, atn, eoi);
		m_last_recv_byte = data;
		m_last_recv_atn = atn;
		m_last_recv_eoi = eoi;
		m_bus->ndac_w(this, 1);
		m_gpib_loop_state = grid210x_gpib_state::wait_dav_false;
	} else if (state == 1 && m_gpib_loop_state == grid210x_gpib_state::wait_dav_false) {
		// restore initial state
		// m_bus->ndac_w(this, 0);
		m_bus->nrfd_w(this, 1);
		m_gpib_loop_state = grid210x_gpib_state::idle;

		if (m_last_recv_atn) {
			const uint8_t command_group = m_last_recv_byte & GPIB_CMD_GROUP_MASK;
			const uint8_t command_addr = m_last_recv_byte & GPIB_CMD_ADDR_MASK;

			if (command_group == GPIB_CMD_MLA) {
				m_data_buffer.clear();
				listening = false;
				talking = false;
				if (command_addr == bus_addr) {
					listening = true;
					m_ignore_state = grid210x_ignore_state::none;
					LOG_GPIB_STATE("grid210x_device now listening\n");
				} else if (m_last_recv_byte == GPIB_CMD_UNL) {
					LOG_GPIB_STATE("grid210x_device now not listening\n");
				} else {
					m_ignore_state = grid210x_ignore_state::wait_atn_high;
					LOG_GPIB_STATE("grid210x_device ignoring transfer for listener %u\n", (unsigned)command_addr);
				}
			} else if (command_group == GPIB_CMD_MTA) {
				m_data_buffer.clear();
				talking = false;
				listening = false;
				if (command_addr == bus_addr) {
					talking = true;
					m_ignore_state = grid210x_ignore_state::none;
					LOG_GPIB_STATE("grid210x_device now talking\n");
				} else if (m_last_recv_byte == GPIB_CMD_UNT) {
					LOG_GPIB_STATE("grid210x_device now not talking\n");
				} else {
					m_ignore_state = grid210x_ignore_state::wait_atn_high;
					LOG_GPIB_STATE("grid210x_device ignoring transfer for talker %u\n", (unsigned)command_addr);
				}
			} else if (m_last_recv_byte == GPIB_CMD_SPE) {
				// serial poll enable
				serial_polling = true;
			} else if (m_last_recv_byte == GPIB_CMD_SPD) {
				// serial poll disable
				serial_polling = false;
			}
		} else if (listening) {
			m_data_buffer.push_back(m_last_recv_byte);
			if (m_last_recv_eoi) {
				accept_transfer();
				m_data_buffer.clear();
			}
		}

		update_handshake(m_bus->atn_r() ^ 1);
		start_talking_if_ready();
	}
}

void grid210x_device::ieee488_nrfd(int state) {
	if (state == 1 && m_gpib_loop_state == grid210x_gpib_state::send_data_start && m_bus->atn_r()) {
		// set dio and assert dav
		m_bus->host_dio_w(m_byte_to_send ^ 0xff);
		m_bus->eoi_w(this, m_send_eoi ^ 1);
		m_bus->dav_w(this, 0);
		m_bus->ndac_w(this, 1);
		m_gpib_loop_state = grid210x_gpib_state::wait_ndac_false;
		LOG_BYTES("grid210x_device byte send %02x eoi %d\n", m_byte_to_send, m_send_eoi);
		ieee488_ndac(m_bus->ndac_r());
	}
	LOG_GPIB_LINES("grid210x_device nrfd state set to %d\n", state);
}

void grid210x_device::ieee488_ndac(int state) {
	if (state == 1 && m_gpib_loop_state == grid210x_gpib_state::wait_ndac_false) {
		// restore initial state
		LOG_GPIB_LINES("grid210x_device restore ndac nrfd dav eoi\n");
		m_bus->nrfd_w(this, 1);
		m_bus->dav_w(this, 1);
		m_bus->eoi_w(this, 1);
		m_gpib_loop_state = grid210x_gpib_state::idle;
		if (serial_polling) {
			talking = false;
		}
		update_handshake(m_bus->atn_r() ^ 1);
		start_talking_if_ready();
	}
	LOG_GPIB_LINES("grid210x_device ndac state set to %d\n", state);
}

void grid210x_device::ieee488_ifc(int state) {
	LOG_GPIB_LINES("grid210x_device ifc state set to %d\n", state);
}

void grid210x_device::ieee488_srq(int state) {
	LOG_GPIB_LINES("grid210x_device srq state set to %d\n", state);
}

void grid210x_device::ieee488_atn(int state) {
	LOG_GPIB_LINES("grid210x_device atn state set to %d\n", state);
	if (m_ignore_state == grid210x_ignore_state::wait_atn_high) {
		if (state) {
			m_ignore_state = grid210x_ignore_state::wait_atn_low;
		}
	} else if (m_ignore_state == grid210x_ignore_state::wait_atn_low) {
		if (!state) {
			m_ignore_state = grid210x_ignore_state::none;
		}
	}
	update_handshake(state ^ 1);
	if (state) {
		start_talking_if_ready();
	}
}

void grid210x_device::update_handshake(int atn) {
	if (m_gpib_loop_state == grid210x_gpib_state::idle) {
		if (m_ignore_state != grid210x_ignore_state::none) {
			m_bus->nrfd_w(this, 1);
			m_bus->ndac_w(this, 1);
		} else if (atn) {
			// pull NDAC low
			m_bus->ndac_w(this, 0);
		} else {
			// pull NDAC high if not listener and low if listener
			m_bus->ndac_w(this, listening ? 0 : 1);
		}
	}
}

void grid210x_device::ieee488_ren(int state) {
	LOG("grid210x_device ren state set to %d\n", state);
}

grid2101_hdd_device::grid2101_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	grid210x_device(mconfig, GPIB_GRID2101_HDD, tag, owner, clock, 4, attotime::from_usec(150))
{

}

grid2101_floppy_device::grid2101_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	grid210x_device(mconfig, GPIB_GRID2101_FLOPPY, tag, owner, clock, 5)
{

}

grid2102_device::grid2102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	grid210x_device(mconfig, GPIB_GRID2102, tag, owner, clock, 6)
{

}

grid210x_device::disk_status grid2101_hdd_device::get_status()
{
	grid210x_device::disk_status status = hdd_status;

	if (is_open()) {
		hard_disk_file hd(image_core_file(), 0);
		const hard_disk_file::info &info = hd.get_info();

		status.sector_count = u16(std::min<u64>(length() / u64(status.sector_size), 0xffffU));
		status.sectors_per_track = u16(std::min<u32>(info.sectors, 0xffffU));
		status.tracks_per_cylinder = u16(std::min<u32>(info.heads, 0xffffU));
	}

	return status;
}

grid210x_device::disk_status grid2101_floppy_device::get_status()
{
	return fdd_status;
}

grid210x_device::disk_status grid2102_device::get_status()
{
	return fdd_status;
}

} // anonymous namespace


// device type definition
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_GRID2102, device_ieee488_interface, grid2102_device, "grid2102", "GRID2102")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_GRID2101_FLOPPY, device_ieee488_interface, grid2101_floppy_device, "grid2101_floppy", "GRID2101_FLOPPY")
DEFINE_DEVICE_TYPE_PRIVATE(GPIB_GRID2101_HDD, device_ieee488_interface, grid2101_hdd_device, "grid2101_hdd", "GRID2101_HDD")
