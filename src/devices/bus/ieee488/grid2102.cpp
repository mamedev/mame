// license:BSD-3-Clause
// copyright-holders:usernameak
/**********************************************************************

    GRiD 2102 Portable Floppy emulation

    http://deltacxx.insomnia247.nl/gridcompass/disk_info.txt for some protocol info
    http://deltacxx.insomnia247.nl/gridcompass/fdd_boards.jpg for floppy drive boards photo

**********************************************************************/

#include "emu.h"
#include "grid2102.h"

// device type definition
DEFINE_DEVICE_TYPE(GRID2102, grid2102_device, "grid2102", "GRID2102")
DEFINE_DEVICE_TYPE(GRID2101_FLOPPY, grid2101_floppy_device, "grid2101_floppy", "GRID2101_FLOPPY")
DEFINE_DEVICE_TYPE(GRID2101_HDD, grid2101_hdd_device, "grid2101_hdd", "GRID2101_HDD")

#define LOG_BYTES_MASK    (LOG_GENERAL << 1)
#define LOG_BYTES(...)    LOGMASKED(LOG_BYTES_MASK, __VA_ARGS__)
#define VERBOSE (LOG_GENERAL)
#include "logmacro.h"

#define GRID2102_FETCH32(Array, Offset) ((uint32_t)(\
	(Array[Offset] << 0) |\
	(Array[Offset + 1] << 8) |\
	(Array[Offset + 2] << 16) |\
	(Array[Offset + 3] << 24)\
))

#define GRID2102_FETCH16(Array, Offset) ((uint16_t)(\
	(Array[Offset] << 0) |\
	(Array[Offset + 1] << 8)\
))

#define GRID2101_HARDDISK_DEV_ADDR 4
#define GRID2102_DEV_ADDR 6

#define GRID210X_GPIB_STATE_IDLE 0
#define GRID210X_GPIB_STATE_WAIT_DAV_FALSE 1
#define GRID210X_GPIB_STATE_SEND_DATA_START 2
#define GRID210X_GPIB_STATE_WAIT_NDAC_FALSE 3

#define GRID210X_STATE_IDLE 0
#define GRID210X_STATE_READING_DATA 1
#define GRID210X_STATE_WRITING_DATA 2
#define GRID210X_STATE_WRITING_DATA_WAIT 3

uint8_t grid2102_device::identify_response[56] = {0x00, 0x02, 0xf8, 0x01, 0xD0, 0x02, 0x01, 0x20, 0x01, 0x21, 0x01, 0x01, 0x00, 0x00,
				 0x34, 0x38, 0x20, 0x54, 0x50, 0x49, 0x20, 0x44, 0x53, 0x20, 0x44, 0x44, 0x20, 0x46,
				 0x4c, 0x4f, 0x50, 0x50, 0x59, 0x20, 0x20, 0x20, 0x20, 0x33, 0x30, 0x32, 0x33, 0x37,
				 0x2d, 0x30, 0x30, 0x00, 0x02, 0x09, 0x00};

uint8_t grid2101_floppy_device::identify_response[56] = {0x00, 0x02, 0xf8, 0x01, 0xD0, 0x02, 0x01, 0x20, 0x01, 0x21, 0x01, 0x01, 0x00, 0x00,
				 0x34, 0x38, 0x20, 0x54, 0x50, 0x49, 0x20, 0x44, 0x53, 0x20, 0x44, 0x44, 0x20, 0x46,
				 0x4c, 0x4f, 0x50, 0x50, 0x59, 0x20, 0x20, 0x20, 0x20, 0x33, 0x30, 0x32, 0x33, 0x37,
				 0x2d, 0x30, 0x30, 0x00, 0x02, 0x09, 0x00};

uint8_t grid2101_hdd_device::identify_response[56] = {
	0x00, 0x02, 0xF8, 0x01, 0x8C, 0x51, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x4D, 0x41,
	0x4D, 0x45, 0x20, 0x48, 0x41, 0x52, 0x44, 0x44, 0x49, 0x53, 0x4B, 0x20, 0x44, 0x52, 0x49, 0x56,
	0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x47, 0x52, 0x49, 0x44, 0x32, 0x31, 0x30, 0x31, 0x00, 0x02,
	0x11, 0x00, 0x33, 0x01, 0x00, 0x00, 0x04, 0x00
};


grid210x_device::grid210x_device(const machine_config &mconfig, device_type type, const char *tag, device_t *owner, uint32_t clock, int bus_addr, uint8_t *identify_response, attotime read_delay)
	: device_t(mconfig, type, tag, owner, clock),
	  device_ieee488_interface(mconfig, *this),
	  device_image_interface(mconfig, *this),
	  m_gpib_loop_state(GRID210X_GPIB_STATE_IDLE),
	  m_floppy_loop_state(GRID210X_STATE_IDLE),
	  listening(false),
	  talking(false),
	  serial_polling(false),
	  has_srq(false),
	  serial_poll_byte(0),
	  bus_addr(bus_addr),
	  identify_response_ptr(identify_response),
	  read_delay(read_delay)
{

}

void grid210x_device::device_start() {
	m_bus->ndac_w(this, 1);
	m_bus->nrfd_w(this, 1);
	m_delay_timer = timer_alloc(FUNC(grid210x_device::delay_tick), this);
}

TIMER_CALLBACK_MEMBER(grid210x_device::delay_tick) {
	if (m_floppy_loop_state == GRID210X_STATE_READING_DATA) {
		std::unique_ptr<uint8_t[]> data(new uint8_t[io_size]);
		fseek(floppy_sector_number * 512, SEEK_SET);
		fread(data.get(), io_size);
		for (int i = 0; i < io_size; i++) {
			m_output_data_buffer.push(data[i]);
		}
		serial_poll_byte = 0x0F;
		has_srq = true;
		m_bus->srq_w(this, 0);
		m_floppy_loop_state = GRID210X_STATE_IDLE;
	} else if (m_floppy_loop_state == GRID210X_STATE_WRITING_DATA_WAIT) {
		// send an srq as success flag
		for (int i = 0; i < 7; i++) { // FIXME:
			m_output_data_buffer.push(0);
		}
		serial_poll_byte = 0x0F;
		has_srq = true;
		m_bus->srq_w(this, 0);
		m_floppy_loop_state = GRID210X_STATE_IDLE;
	}
}

void grid210x_device::ieee488_eoi(int state) {
	// logerror("grid210x_device eoi state set to %d\n", state);
}

void grid210x_device::accept_transfer() {
	if (m_floppy_loop_state == GRID210X_STATE_IDLE) {
		if (m_data_buffer.size() >= 0xA) {
			uint8_t command = m_data_buffer[0];
			uint32_t sector_number = GRID2102_FETCH32(m_data_buffer, 3);
			uint16_t data_size = GRID2102_FETCH16(m_data_buffer, 7);
			LOG("grid210x_device command %u, data size %u, sector no %u\n", (unsigned)command, (unsigned)data_size, (unsigned)sector_number);
			(void)(sector_number);
			if (command == 0x1) { // ddGetStatus
				for (int i = 0; i < 56 && i < data_size; i++) {
					m_output_data_buffer.push(identify_response_ptr[i]);
				}
			} else if (command == 0x4) { // ddRead
				floppy_sector_number = sector_number;
				io_size = data_size;
				m_floppy_loop_state = GRID210X_STATE_READING_DATA;
				m_delay_timer->adjust(read_delay);
			} else if (command == 0x5) {
				floppy_sector_number = sector_number;
				io_size = data_size;
				m_floppy_loop_state = GRID210X_STATE_WRITING_DATA;
			}
		} // else something is wrong, ignore
	} else if (m_floppy_loop_state == GRID210X_STATE_WRITING_DATA) {
		// write
		if (floppy_sector_number != 0xFFFFFFFF) {
			fseek(floppy_sector_number * 512, SEEK_SET);
			fwrite(m_data_buffer.data(), m_data_buffer.size());
		} else {
			// TODO: set status
		}
		// logerror("grid210x_device write sector %d\n", floppy_sector_number);
		// wait
		m_floppy_loop_state = GRID210X_STATE_WRITING_DATA_WAIT;
		m_delay_timer->adjust(read_delay);
	}
}

void grid210x_device::ieee488_dav(int state) {
	if(state == 0 && m_gpib_loop_state == GRID210X_GPIB_STATE_IDLE) {
		// read data and wait for transfer end
		int atn = m_bus->atn_r() ^ 1;
		m_bus->nrfd_w(this, 0);
		uint8_t data = m_bus->dio_r() ^ 0xFF;
		int eoi = m_bus->eoi_r() ^ 1;
		LOG_BYTES("grid210x_device byte recv %02x atn %d eoi %d\n", data, atn, eoi);
		m_last_recv_byte = data;
		m_last_recv_atn = atn;
		m_last_recv_eoi = eoi;
		m_bus->ndac_w(this, 1);
		m_gpib_loop_state = GRID210X_GPIB_STATE_WAIT_DAV_FALSE;
	} else if (state == 1 && m_gpib_loop_state == GRID210X_GPIB_STATE_WAIT_DAV_FALSE) {
		// restore initial state
		// m_bus->ndac_w(this, 0);
		m_bus->nrfd_w(this, 1);
		m_gpib_loop_state = GRID210X_GPIB_STATE_IDLE;
		update_ndac(m_bus->atn_r() ^ 1);

		if (m_last_recv_atn) {
			if ((m_last_recv_byte & 0xE0) == 0x20) {
				if ((m_last_recv_byte & 0x1F) == bus_addr) {
					// dev-id = 5
					listening = true;
					LOG("grid210x_device now listening\n");
				} else if((m_last_recv_byte & 0x1F) == 0x1F) {
					// reset listen
					listening = false;
					LOG("grid210x_device now not listening\n");
				}
			} else if ((m_last_recv_byte & 0xE0) == 0x40) {
				if ((m_last_recv_byte & 0x1F) == bus_addr) {
					// dev-id = 5
					talking = true;
					LOG("grid210x_device now talking\n");
				} else {
					// reset talk
					talking = false;
					LOG("grid210x_device now not talking\n");
				}
			} else if (m_last_recv_byte == 0x18) {
				// serial poll enable
				serial_polling = true;
			} else if (m_last_recv_byte == 0x19) {
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

		if (talking) {
			if (serial_polling) {
				bool had_srq = has_srq;
				if (has_srq) {
					has_srq = false;
					m_bus->srq_w(this, 1);
				}
				m_byte_to_send = serial_poll_byte | (had_srq ? 0x40 : 0);
				serial_poll_byte = 0;
				m_send_eoi = 1;
				m_gpib_loop_state = GRID210X_GPIB_STATE_SEND_DATA_START;
			} else if (!m_output_data_buffer.empty()) {
				m_byte_to_send = m_output_data_buffer.front();
				m_output_data_buffer.pop();
				m_send_eoi = m_output_data_buffer.empty() ? 1 : 0;
				m_gpib_loop_state = GRID210X_GPIB_STATE_SEND_DATA_START;
			}
		}
	}
}

void grid210x_device::ieee488_nrfd(int state) {
	if (state == 1 && m_gpib_loop_state == GRID210X_GPIB_STATE_SEND_DATA_START) {
		// set dio and assert dav
		m_bus->host_dio_w(m_byte_to_send ^ 0xFF);
		m_bus->eoi_w(this, m_send_eoi ^ 1);
		m_bus->dav_w(this, 0);
		m_bus->ndac_w(this, 1);
		m_gpib_loop_state = GRID210X_GPIB_STATE_WAIT_NDAC_FALSE;
		LOG_BYTES("grid210x_device byte send %02x eoi %d\n", m_byte_to_send, m_send_eoi);
		ieee488_ndac(m_bus->ndac_r());
	}
	// logerror("grid210x_device nrfd state set to %d\n", state);
}

void grid210x_device::ieee488_ndac(int state) {
	if (state == 1 && m_gpib_loop_state == GRID210X_GPIB_STATE_WAIT_NDAC_FALSE) {
		// restore initial state
		// logerror("grid210x_device restore ndac nrfd dav eoi\n");
		m_bus->nrfd_w(this, 1);
		m_bus->dav_w(this, 1);
		m_bus->eoi_w(this, 1);
		m_gpib_loop_state = GRID210X_GPIB_STATE_IDLE;
		if (serial_polling) {
			talking = false;
		}
		update_ndac(m_bus->atn_r() ^ 1);

		if (!serial_polling && talking && !m_output_data_buffer.empty()) {
			m_byte_to_send = m_output_data_buffer.front();
			m_output_data_buffer.pop();
			m_send_eoi = m_output_data_buffer.empty() ? 1 : 0;
			m_gpib_loop_state = GRID210X_GPIB_STATE_SEND_DATA_START;
		}
	}
	// logerror("grid210x_device ndac state set to %d\n", state);
}

void grid210x_device::ieee488_ifc(int state) {
	// logerror("grid210x_device ifc state set to %d\n", state);
}

void grid210x_device::ieee488_srq(int state) {
	// logerror("grid210x_device srq state set to %d\n", state);
}

void grid210x_device::ieee488_atn(int state) {
	// logerror("grid210x_device atn state set to %d\n", state);
	update_ndac(state ^ 1);
}

void grid210x_device::update_ndac(int atn) {
	if (m_gpib_loop_state == GRID210X_GPIB_STATE_IDLE) {
		if (atn) {
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

grid2101_hdd_device::grid2101_hdd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: grid210x_device(mconfig, GRID2101_HDD, tag, owner, clock, 4, identify_response, attotime::from_usec(150))
{

}

grid2101_floppy_device::grid2101_floppy_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : grid210x_device(mconfig, GRID2101_FLOPPY, tag, owner, clock, 5, identify_response)
{

}

grid2102_device::grid2102_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) : grid210x_device(mconfig, GRID2102, tag, owner, clock, 6, identify_response)
{

}
