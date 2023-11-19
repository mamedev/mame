// license:BSD-3-Clause
// copyright-holders:Brian Johnson
/*******************************************************************
 *
 * NABU PC - Network Adapter Settop Box
 *
 *******************************************************************/

#include "emu.h"
#include "adapter.h"

#include "client_http.hpp"
#include "diserial.h"
#include "emuopts.h"
#include "hashing.h"
#include "unzip.h"

#define VERBOSE 0
#include "logmacro.h"

// Undef stupid windows defines
#undef PARITY_NONE
#undef PARITY_ODD
#undef PARITY_EVEN
#undef PARITY_MARK
#undef PARITY_SPACE


namespace {
//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

/* NABU PC Network Adapter Base */
class network_adapter_base
	: public device_t
	, public device_buffered_serial_interface<16U>
	, public device_rs232_port_interface
{
public:
	virtual void input_txd(int state) override;

protected:
	// constructor/destructor
	network_adapter_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, u32 clock);
	virtual ioport_constructor device_input_ports() const override;
	virtual void device_start() override;
	virtual void device_reset() override;

	// device_buffered_serial_interface overrides
	virtual void tra_callback() override;

	virtual std::error_condition load_segment(uint32_t segment_id) = 0;

	// class to handle splitting up segments
	class segment_file {
	public:
		struct pak {
			uint16_t length;
			uint8_t segment_id[3];
			uint8_t packet_number;
			uint8_t owner;
			uint8_t tier[4];
			uint8_t mbytes[2];
			uint8_t type;
			uint8_t pak_number[2];
			uint8_t offset[2];
			uint8_t data[993]; //data + crc16 (data max size is 991)
		};
	public:
		std::error_condition generate_time_segment();
		std::error_condition parse_pak_segment(const uint8_t *data, size_t length);
		std::error_condition parse_raw_segment(uint32_t segment_id, const uint8_t *data, size_t length);

		const pak& operator[](const int index) const;
		uint32_t size() { return pak_list.size(); }

	private:
		static constexpr uint16_t crc_table[256] = {
			0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b,
			0xc18c, 0xd1ad, 0xe1ce, 0xf1ef, 0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
			0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de, 0x2462, 0x3443, 0x0420, 0x1401,
			0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
			0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738,
			0xf7df, 0xe7fe, 0xd79d, 0xc7bc, 0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
			0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b, 0x5af5, 0x4ad4, 0x7ab7, 0x6a96,
			0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
			0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd,
			0xad2a, 0xbd0b, 0x8d68, 0x9d49, 0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
			0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78, 0x9188, 0x81a9, 0xb1ca, 0xa1eb,
			0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
			0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2,
			0x4235, 0x5214, 0x6277, 0x7256, 0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
			0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405, 0xa7db, 0xb7fa, 0x8799, 0x97b8,
			0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
			0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827,
			0x18c0, 0x08e1, 0x3882, 0x28a3, 0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
			0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92, 0xfd2e, 0xed0f, 0xdd6c, 0xcd4d,
			0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
			0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74,
			0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
		};
		uint16_t update_crc(uint16_t crc, uint8_t data);

		std::vector<pak> pak_list;
	};

	enum segment_type {PAK, NABU};

	required_ioport m_config;

	uint32_t m_segment;
	uint32_t m_segment_length;
	uint8_t m_segment_type;
	std::unique_ptr<uint8_t[]> m_segment_data;

	segment_file m_pakcache;

private:
	// Serial Parameters
	static constexpr int START_BIT_COUNT = 1;
	static constexpr int DATA_BIT_COUNT = 8;
	static constexpr device_serial_interface::parity_t PARITY = device_serial_interface::PARITY_NONE;
	static constexpr device_serial_interface::stop_bits_t STOP_BITS = device_serial_interface::STOP_BITS_1;
	static constexpr int BAUD = 111'900;

	virtual void received_byte(uint8_t byte) override;

	// State Machine
	enum state {IDLE, SETSTATUS_REQUEST, GETSTATUS_REQUEST, CHANNEL_REQUEST, SEGMENT_REQUEST, SEND_SEGMENT};

	TIMER_CALLBACK_MEMBER(segment_tick);

	std::error_condition parse_segment(const uint8_t *data, size_t length);

	void idle(uint8_t byte);
	void channel_request(uint8_t byte);
	void segment_request(uint8_t byte);
	void set_status(uint8_t byte);
	void get_status(uint8_t byte);
	void send_segment(uint8_t byte);


	uint16_t m_channel;
	uint8_t  m_packet;
	uint8_t  m_state;
	uint8_t  m_substate;
	uint16_t m_pak_offset;

	emu_timer *m_segment_timer;

};

/* NABU PC Network Adapter Local */
class network_adapter_local
	: public network_adapter_base
	, public device_image_interface
{
public:
	// constructor/destructor
	network_adapter_local(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);

	// image-level overrides
	virtual bool is_readable()  const noexcept override { return true; }
	virtual bool is_writeable() const noexcept override { return false; }
	virtual bool is_creatable() const noexcept override { return false; }
	virtual bool is_reset_on_load() const noexcept override { return false; }
	virtual bool core_opens_image_file() const noexcept override { return true; }
	virtual const char *file_extensions() const noexcept override { return "npz"; }
	virtual const char *image_type_name() const noexcept override { return "npz_file"; }
	virtual const char *image_brief_type_name() const noexcept override { return "npz"; }
	virtual std::pair<std::error_condition, std::string> call_load() override;
protected:
	virtual std::error_condition load_segment(uint32_t segment_id) override;
};

/* NABU PC Network Adapter Remote */
class network_adapter_remote
	: public network_adapter_base
{
public:
	// constructor/destructor
	network_adapter_remote(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock);
protected:
	virtual void device_start() override;
	virtual void device_reset() override;
	virtual ioport_constructor device_input_ports() const override;
	virtual std::error_condition load_segment(uint32_t segment_id) override;
private:
	bool m_cycle1;

	std::unique_ptr<webpp::http_client> m_httpclient;
};

//**************************************************************************
//  SEGMENT FILE LOADING
//**************************************************************************

std::error_condition network_adapter_base::segment_file::generate_time_segment()
{
	uint8_t buffer[9];
	auto now = std::chrono::system_clock::now();
	auto tim = std::chrono::system_clock::to_time_t(now);
	auto localtime = std::localtime(&tim);

	buffer[0] = 0x02;
	buffer[1] = 0x02;
	buffer[2] = localtime->tm_wday + 1;
	buffer[3] = 0x54;   // 1984 forever
	buffer[4] = localtime->tm_mon + 1;
	buffer[5] = localtime->tm_mday;
	buffer[6] = localtime->tm_hour;
	buffer[7] = localtime->tm_min;
	buffer[8] = localtime->tm_sec;
	return parse_raw_segment(0x7fffff, buffer, 9);
}


std::error_condition network_adapter_base::segment_file::parse_pak_segment(const uint8_t *data, size_t length)
{
	bool done = false;
	size_t offset = 0;
	uint16_t blk_sz;
	uint16_t crc;
	pak current;

	pak_list.clear();

	while (!done) {
		blk_sz =  (data[offset + 1] & 0xff) << 8;
		blk_sz |= (data[offset] & 0xff);
		crc = 0xffff;
		if (blk_sz > 1009) {
			return std::errc::value_too_large;
		}
		memcpy(&current, data + offset, 2 + blk_sz);
		for (int i = 2; i < blk_sz; ++i) {
			crc = update_crc(crc, ((char *)&current)[i]);
		}
		crc ^= 0xffff;
		current.data[blk_sz - 18] = (crc >> 8) & 0xff;
		current.data[blk_sz - 17] = crc & 0xff;
		pak_list.push_back(current);
		offset += blk_sz + 2;
		if (offset >= length - 1 || current.type & 0x10)
			done = true;
	}
	return std::error_condition();
}

std::error_condition network_adapter_base::segment_file::parse_raw_segment(uint32_t segment_id, const uint8_t *data, size_t length)
{
	util::core_file::ptr fd;
	pak current;
	uint64_t offset = 0;
	size_t actual;
	uint16_t crc;
	uint8_t npak = 0;
	std::error_condition err;

	pak_list.clear();

	err = util::core_file::open_ram(data, length, OPEN_FLAG_READ, fd);
	if (err)
		return err;

	memset(current.data, 0, 993);
	current.segment_id[0] = (segment_id & 0xff0000) >> 16;
	current.segment_id[1] = (segment_id & 0x00ff00) >> 8;
	current.segment_id[2] = (segment_id & 0x0000ff);
	current.owner         = 0x01;
	current.tier[0]       = 0x7f;
	current.tier[1]       = 0xff;
	current.tier[2]       = 0xff;
	current.tier[3]       = 0xff;
	current.mbytes[0]     = 0x7f;
	current.mbytes[1]     = 0x80;
	err = fd->read_at(offset, current.data, 991, actual);
	do {
		crc = 0xffff;
		if (err) {
			return err;
		}
		if (actual > 0) {
			current.length = actual + 18;
			current.packet_number = npak;
			current.pak_number[0] = npak;
			current.pak_number[1] = 0;
			current.type = 0x20;
			if (offset == 0)
				current.type |= 0x81;
			if (actual < 991)
				current.type |= 0x10;
			current.offset[0] = ((offset) >> 8) & 0xff;
			current.offset[1] = offset & 0xff;
			for (int i = 2; i < actual + 18; ++i) {
				crc = update_crc(crc, ((char *)&current)[i]);
			}
			crc ^= 0xffff;
			current.data[actual] = (crc >> 8) & 0xff;
			current.data[actual + 1] = crc & 0xff;
			pak_list.push_back(current);
			offset = (++npak * 991);
			memset(current.data, 0, 991);
			err = fd->read_at(offset, current.data, 991, actual);
		}
	} while(actual > 0);

	return err;
}

const network_adapter_base::segment_file::pak& network_adapter_base::segment_file::operator[](const int index) const
{
	assert(index >= 0 && index < size());

	return pak_list[index];
}

// crc16 calculation
uint16_t network_adapter_base::segment_file::update_crc(uint16_t crc, uint8_t data)
{
	uint8_t bc;

	bc = (crc >> 8) ^ data;

	crc <<= 8;
	crc ^= crc_table[bc];

	return crc;
}

//**************************************************************************
//  INPUT PORTS
//**************************************************************************

// CONFIG
static INPUT_PORTS_START( nabu_network_adapter_base )
	PORT_START("CONFIG")
	PORT_CONFNAME(0x01, 0x00, "Prompt for channel?")
	PORT_CONFSETTING(0x01, "Yes")
	PORT_CONFSETTING(0x00, "No")
INPUT_PORTS_END

//**************************************************************************
//  DEVICE INITIALIZATION - Base
//**************************************************************************

network_adapter_base::network_adapter_base(machine_config const &mconfig, device_type type, char const *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, type, tag, owner, clock)
	, device_buffered_serial_interface(mconfig, *this)
	, device_rs232_port_interface(mconfig, *this)
	, m_config(*this, "CONFIG")
	, m_segment(0)
	, m_segment_length(0)
	, m_segment_type(segment_type::PAK)
	, m_channel(0)
	, m_packet(0)
	, m_state(state::IDLE)
	, m_substate(0)
	, m_pak_offset(0)
	, m_segment_timer(nullptr)
{
}

void network_adapter_base::device_start()
{
	m_segment_timer = timer_alloc(FUNC(network_adapter_base::segment_tick), this);

	m_segment_data = std::make_unique<uint8_t[]>(0x10000);;

	save_item(NAME(m_state));
	save_item(NAME(m_substate));
	save_item(NAME(m_channel));
	save_item(NAME(m_packet));
	save_item(NAME(m_segment));
	save_item(NAME(m_segment_length));
	save_item(NAME(m_segment_type));
	save_item(NAME(m_pak_offset));
	save_pointer(NAME(m_segment_data), 0x10000);
}

void network_adapter_base::device_reset()
{
	m_state = state::IDLE;
	m_substate = 0;
	// initialise state
	clear_fifo();

	// configure device_buffered_serial_interface
	set_data_frame(START_BIT_COUNT, DATA_BIT_COUNT, PARITY, STOP_BITS);
	set_rate(BAUD);
	receive_register_reset();
	transmit_register_reset();
}

//**************************************************************************
//  DEVICE CONFIGURATION - Base
//**************************************************************************

ioport_constructor network_adapter_base::device_input_ports() const
{
	return INPUT_PORTS_NAME( nabu_network_adapter_base );
}

//**************************************************************************
//  SERIAL PROTOCOL - Base
//**************************************************************************

void network_adapter_base::input_txd(int state)
{
	device_buffered_serial_interface::rx_w(state);
}

void network_adapter_base::tra_callback()
{
	output_rxd(transmit_register_get_data_bit());
}

void network_adapter_base::received_byte(uint8_t byte)
{
	LOG("Received Byte 0x%02X\n", byte);
	switch (m_state) {
	case state::IDLE:
		idle(byte);
		break;
	case state::CHANNEL_REQUEST:
		channel_request(byte);
		break;
	case state::SEGMENT_REQUEST:
		segment_request(byte);
		break;
	case state::SETSTATUS_REQUEST:
		set_status(byte);
		break;
	case state::GETSTATUS_REQUEST:
		get_status(byte);
		break;
	case state::SEND_SEGMENT:
		send_segment(byte);
		break;
	}
}

//**************************************************************************
//  STATE MACHINE - Base
//**************************************************************************

void network_adapter_base::idle(uint8_t byte)
{
	m_substate = 0;
	switch (byte) {
	case 0x85:
		transmit_byte(0x10);
		transmit_byte(0x06);
		m_state = state::CHANNEL_REQUEST;
		break;
	case 0x84:
		transmit_byte(0x10);
		transmit_byte(0x06);
		m_state = state::SEGMENT_REQUEST;
		break;
	case 0x83:
		transmit_byte(0x10);
		transmit_byte(0x06);
		transmit_byte(0xe4);
		break;
	case 0x82:
		transmit_byte(0x10);
		transmit_byte(0x06);
		m_state = state::GETSTATUS_REQUEST;
		break;
	case 0x81:
		transmit_byte(0x10);
		transmit_byte(0x06);
		m_state = state::SETSTATUS_REQUEST;
		break;
	}

}

void network_adapter_base::set_status(uint8_t byte)
{
	if (m_substate == 1) {
		transmit_byte(0xe4);
		m_state = state::IDLE;
	}
	++m_substate;

}

void network_adapter_base::get_status(uint8_t byte)
{
	if (byte == 0x01) {
		transmit_byte(bool(m_config->read() & 1) ? 0x9f : 0x1f);
	}
	transmit_byte(0x10);
	transmit_byte(0xe1);
	m_state = state::IDLE;
}

void network_adapter_base::channel_request(uint8_t byte)
{
	if (m_substate == 0) {
		m_channel = (m_channel & 0xff00) | (byte);
	} else if (m_substate == 1) {
		m_channel = (m_channel & 0xff) | (byte << 8);
		transmit_byte(0xe4);
		LOG("Channel: 0x%04X\n", m_channel);
		m_state = state::IDLE;
	}
	++m_substate;
}

void network_adapter_base::segment_request(uint8_t byte)
{
	static uint32_t segment_id = 0;

	if (m_substate == 0) {
		m_packet = byte;
	} else if (m_substate == 1) {
		segment_id = (segment_id & 0xffff00) | (byte);
	} else if (m_substate == 2) {
		segment_id = (segment_id & 0xff00ff) | (byte << 8);
	} else if (m_substate == 3) {
		segment_id = (segment_id & 0xffff) | (byte << 16);
		transmit_byte(0xe4);
		if (!load_segment(segment_id)) {
			transmit_byte(0x91);
			m_state = state::SEND_SEGMENT;
			m_substate = 0;
			m_segment = segment_id;
			LOG("Segment: 0x%06X, Packet: 0x%02X\n", m_segment, m_packet);
			return;
		} else {
			transmit_byte(0x90);
			m_state = state::IDLE;
			LOG("Segment 0x%06X not found\n", segment_id);
			m_segment = 0;
		}
	}
	++m_substate;
}

void network_adapter_base::send_segment(uint8_t byte)
{
	if (m_substate == 0) {
		if (byte != 0x10) {
			LOG("Expecting byte 0x10 got %02X, restarting.\n", byte);
			m_state = state::IDLE;
			m_substate = 0;
			return;
		}
	} else if (m_substate == 1) {
		if (byte != 0x06) {
			LOG("Expecting byte 0x06 got %02X, restarting.\n", byte);
			m_state = state::IDLE;
			m_substate = 0;
			return;
		}
		m_pak_offset = 0;
		if (!parse_segment(m_segment_data.get(), m_segment_length)) {
			m_segment_timer->adjust(attotime::zero, 0, attotime::from_hz(7'500));
			LOG("Segment sending, returning to idle state\n");
		} else {
			LOG("Failed to find segment: %06d, restarting\n", m_segment);
			transmit_byte(0x10);
			transmit_byte(0x06);
			transmit_byte(0xe4);
		}
		m_state = state::IDLE;
	}

	++m_substate;
}

TIMER_CALLBACK_MEMBER(network_adapter_base::segment_tick)
{
	char * data = (char*)&m_pakcache[m_packet];
	data += 2;
	if (data[m_pak_offset] == 0x10) {
		transmit_byte(data[m_pak_offset]);
	}
	transmit_byte(data[m_pak_offset++]);
	if (m_pak_offset >= m_pakcache[m_packet].length) {
		transmit_byte(0x10);
		transmit_byte(0xe1);
		m_segment_timer->reset();
	}
}

std::error_condition network_adapter_base::parse_segment(const uint8_t *data, size_t length)
{
	if (m_segment == 0x7fffff) {
		return m_pakcache.generate_time_segment();
	}

	if (m_segment_type == segment_type::NABU) {
		return m_pakcache.parse_raw_segment(m_segment, data, length);
	}

	return m_pakcache.parse_pak_segment(data, length);
}


//**************************************************************************
//  DEVICE INITIALIZATION - Local
//**************************************************************************

network_adapter_local::network_adapter_local(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: network_adapter_base(mconfig, NABUPC_NETWORK_LOCAL_ADAPTER, tag, owner, clock)
	, device_image_interface(mconfig, *this)
{
}

std::pair<std::error_condition, std::string> network_adapter_local::call_load()
{
	if (is_filetype("npz")) {
		return std::make_pair(std::error_condition(), std::string());
	}
	return std::make_pair(image_error::INVALIDIMAGE, "Invalid image");
}

// Load Segment from local npz file
std::error_condition network_adapter_local::load_segment(uint32_t segment_id)
{
	segment_id &= 0xffffff;

	util::core_file::ptr proxy;
	std::error_condition err;
	util::archive_file::ptr zipfile;
	std::string segment_filename;

	if ((m_segment_length != 0 && (segment_id == m_segment)) || segment_id == 0x7fffff) {
		return std::error_condition();
	}

	err = util::core_file::open_proxy(image_core_file(), proxy);
	if (err) {
		m_segment_length = 0;
		return err;
	}

	err = util::archive_file::open_zip(std::move(proxy), zipfile);
	if (err) {
		m_segment_length = 0;
		return err;
	}

	segment_filename = util::string_format("%06X.pak", segment_id);
	m_segment_type = segment_type::PAK;

	if (zipfile->search(segment_filename, false) < 0) {
		segment_filename = util::string_format("%06X.nabu", segment_id);
		m_segment_type = segment_type::NABU;
		if (zipfile->search(segment_filename, false) < 0) {
			m_segment_length = 0;
			return std::errc::no_such_file_or_directory;
		}
	}

	// determine the uncompressed length
	uint64_t uncompressed_length_uint64 = zipfile->current_uncompressed_length();
	size_t uncompressed_length = (size_t)uncompressed_length_uint64;
	if (uncompressed_length != uncompressed_length_uint64) {
		m_segment_length = 0;
		return std::errc::not_enough_memory;
	}

	if (uncompressed_length > 0x10000) {
		m_segment_length = 0;
		return std::errc::file_too_large;
	}

	// prepare a buffer for the segment file
	m_segment_length = uncompressed_length;;

	err = zipfile->decompress(&m_segment_data[0], m_segment_length);
	if (err) {
		m_segment_length = 0;
	}

	return err;
}


//**************************************************************************
//  DEVICE INITIALIZATION - Remote
//**************************************************************************
static INPUT_PORTS_START( nabu_network_adapter_remote )
	PORT_INCLUDE( nabu_network_adapter_base )
	PORT_MODIFY("CONFIG")
	PORT_CONFNAME(0x02, 0x02, "Network Cycle")
	PORT_CONFSETTING(0x02, "Cycle 1")
	PORT_CONFSETTING(0x00, "Cycle 2")
INPUT_PORTS_END

network_adapter_remote::network_adapter_remote(machine_config const &mconfig, char const *tag, device_t *owner, uint32_t clock)
	: network_adapter_base(mconfig, NABUPC_NETWORK_REMOTE_ADAPTER, tag, owner, clock)
{
}

void network_adapter_remote::device_start()
{
	network_adapter_base::device_start();

	m_httpclient = std::make_unique<webpp::http_client>("cloud.nabu.ca");

	save_item(NAME(m_cycle1));
}

void network_adapter_remote::device_reset()
{
	network_adapter_base::device_reset();

	m_cycle1 = bool(m_config->read() & 2);
}

ioport_constructor network_adapter_remote::device_input_ports() const
{
	return INPUT_PORTS_NAME( nabu_network_adapter_remote );
}

// Load segment from Remote Server (cloud.nabu.ca)
std::error_condition network_adapter_remote::load_segment(uint32_t segment_id)
{
	uint32_t content_length = 0;
	std::string url;
	std::shared_ptr<webpp::http_client::Response> resp;

	segment_id &= 0xffffff;

	if ((m_segment_length != 0 && (segment_id == m_segment)) || segment_id == 0x7fffff) {
		return std::error_condition();
	}

	std::string segment_filename = util::string_format("%06X.pak", segment_id);

	if (m_cycle1) {
		url = util::string_format("/cycle%%201%%20raw/%s", segment_filename);
	} else {
		url = util::string_format("/cycle%%202%%20raw/%s", segment_filename);
	}

	resp = m_httpclient->request("GET", url);
	if (resp->status_code != "200 OK") {
		m_segment_length = 0;
		return std::errc::no_such_file_or_directory;
	}

	auto header_it = resp->header.find("Content-Length");
	if (header_it != resp->header.end()) {
		content_length = stoull(header_it->second);
	}
	if (content_length == 0 || content_length > 0x10000) {
		m_segment_length = 0;
		return std::errc::file_too_large;
	}

	// prepare a buffer for the segment file
	m_segment_length = content_length;;

	resp->content.read(reinterpret_cast<char *>(&m_segment_data[0]), m_segment_length);
	if (!resp->content) {
		m_segment_length = 0;
		return std::errc::io_error;
	}

	return std::error_condition();
}

} // anonymous namespace

//**************************************************************************
//  DEVICE DEFINITIONS
//**************************************************************************

DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_NETWORK_LOCAL_ADAPTER, device_rs232_port_interface, network_adapter_local, "nabu_net_local_adapter", "NABU Network Adapter - Local")
DEFINE_DEVICE_TYPE_PRIVATE(NABUPC_NETWORK_REMOTE_ADAPTER, device_rs232_port_interface, network_adapter_remote, "nabu_net_remote_adapter", "NABU Network Adapter - Remote")
