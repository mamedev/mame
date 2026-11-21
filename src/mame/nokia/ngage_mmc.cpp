// license:GPL-2.0+
// copyright-holders:Andrea Bogazzi
// NEM-4 UPP native MMC host and byte-addressed card at transaction level.
// The card image supplies the sectors and optionally the original CID/CSD/OCR.

#include "emu.h"
#include "ngage_mmc.h"

#include "multibyte.h"

#include <algorithm>

namespace {

constexpr u32 SECTOR_BYTES = 512;
constexpr u64 MAX_IMAGE_BYTES = 0x40000000;
constexpr u32 OUT_OF_RANGE = 1U << 31;
constexpr u32 ADDRESS_ERROR = 1U << 30;
constexpr u32 BLOCK_LEN_ERROR = 1U << 29;
constexpr u32 WRITE_PROTECTED = 1U << 26;
constexpr u32 ILLEGAL_COMMAND = 1U << 22;

void register_field(u8 *bytes, unsigned high, unsigned low, u32 value)
{
	for (unsigned bit = low; bit <= high; ++bit)
		if (BIT(value, bit - low))
			bytes[(127 - bit) / 8] |= 1U << (bit & 7);
}

u8 register_crc(u8 const *bytes)
{
	u8 crc = 0;
	for (unsigned i = 0; i < 15; ++i)
		for (int bit = 7; bit >= 0; --bit)
			crc = ((crc << 1) ^ ((BIT(crc, 6) != BIT(bytes[i], bit)) ? 9 : 0)) & 0x7f;
	return (crc << 1) | 1;
}

} // anonymous namespace

DEFINE_DEVICE_TYPE(NGAGE_MMC, ngage_mmc_device, "ngage_mmc", "N-Gage MMC slot")

ngage_mmc_device::ngage_mmc_device(machine_config const &mconfig, char const *tag, device_t *owner, u32 clock)
	: device_t(mconfig, NGAGE_MMC, tag, owner, clock)
	, device_image_interface(mconfig, *this)
	, m_event_cb(*this)
{
}

void ngage_mmc_device::device_start()
{
	m_command_timer = timer_alloc(FUNC(ngage_mmc_device::command_complete), this);
	m_tail_timer = timer_alloc(FUNC(ngage_mmc_device::command_tail), this);
	save_item(NAME(m_reg));
	save_item(NAME(m_status));
	save_item(NAME(m_type));
	save_item(NAME(m_command));
	save_item(NAME(m_argument));
	save_item(NAME(m_response));
	save_item(NAME(m_response_words));
	save_item(NAME(m_response_pos));
	save_item(NAME(m_card_state));
	save_item(NAME(m_rca));
	save_item(NAME(m_block_length));
	save_item(NAME(m_multiblock));
	save_item(NAME(m_next_address));
	save_item(NAME(m_fifo_direction));
	save_item(NAME(m_fifo));
	save_item(NAME(m_fifo_length));
	save_item(NAME(m_fifo_pos));
}

void ngage_mmc_device::device_reset()
{
	std::fill(std::begin(m_reg), std::end(m_reg), 0);
	m_status = 0;
	m_type = m_command = m_argument = 0;
	m_response_words = m_response_pos = 0;
	m_card_state = IDLE;
	m_rca = 1;
	m_block_length = SECTOR_BYTES;
	m_multiblock = false;
	m_next_address = 0;
	m_fifo_direction = FIFO_IDLE;
	m_fifo_length = m_fifo_pos = 0;
	m_command_timer->enable(false);
	m_tail_timer->enable(false);
}

std::pair<std::error_condition, std::string> ngage_mmc_device::call_load()
{
	u64 const size = length();
	if (size < 4 * SECTOR_BYTES || size > MAX_IMAGE_BYTES + SECTOR_BYTES || size % SECTOR_BYTES)
		return { image_error::INVALIDLENGTH, "MMC image must contain whole 512-byte sectors (2 KiB to 1 GiB)" };

	u8 footer[SECTOR_BYTES];
	if (fseek(size - SECTOR_BYTES, SEEK_SET) || fread(footer, sizeof(footer)) != sizeof(footer))
		return { image_error::INVALIDIMAGE, "Could not read the last MMC sector" };

	u64 payload = size;
	u32 ocr = 0x00ff8000;
	u8 cid[16] = { 0, 0, 0, 'M', 'A', 'M', 'E', 'M', 'C', 0x10, 0, 0, 0, 1, 0xa8, 0 };
	u8 csd[16]{};
	if (!std::memcmp(footer, "NGAGEMMC", 8))
	{
		if (footer[8] != 1)
			return { image_error::UNSUPPORTED, "Unsupported NGAGEMMC footer version" };
		payload -= SECTOR_BYTES;
		std::copy_n(&footer[16], 16, cid);
		std::copy_n(&footer[32], 16, csd);
		ocr = get_u32be(&footer[48]);
		if ((csd[0] >> 6) != 2 || (csd[5] & 15) != 9 || (ocr & 0x60000000))
			return { image_error::UNSUPPORTED, "Expected a byte-addressed MMC with 512-byte sectors" };
	}
	else
	{
		if (payload > MAX_IMAGE_BYTES)
			return { image_error::INVALIDLENGTH, "Raw MMC image exceeds 1 GiB" };
		unsigned mult = 0;
		while ((payload / SECTOR_BYTES >> (mult + 2)) > 4096)
			++mult;
		u32 const count = payload / SECTOR_BYTES >> (mult + 2);
		register_field(csd, 127, 126, 2);
		register_field(csd, 125, 122, 3);
		register_field(csd, 119, 112, 0x0f);
		register_field(csd, 103, 96, 0x2a);
		register_field(csd, 95, 84, 0x015);
		register_field(csd, 83, 80, 9);
		register_field(csd, 79, 79, 1);
		register_field(csd, 77, 77, 1);
		register_field(csd, 73, 62, count - 1);
		register_field(csd, 61, 50, 0xdb6);
		register_field(csd, 49, 47, mult);
		register_field(csd, 41, 37, 15);
		register_field(csd, 28, 26, 2);
		register_field(csd, 25, 22, 9);
	}

	u32 const c_size = ((csd[6] & 3) << 10) | (csd[7] << 2) | (csd[8] >> 6);
	u32 const mult = ((csd[9] & 3) << 1) | (csd[10] >> 7);
	u64 const capacity = u64(c_size + 1) << (mult + 11);
	if (payload > capacity && !std::memcmp(footer, "NGAGEMMC", 8))
		return { image_error::INVALIDLENGTH, "MMC payload is larger than the footer's CSD capacity" };

	m_payload_size = payload;
	m_card_capacity = capacity;
	m_ocr = ocr & 0x7fffffff;
	m_write_protected = is_readonly() || (csd[14] & 0x30) || payload < capacity;
	if (m_write_protected)
		csd[14] |= 0x10;
	cid[15] = register_crc(cid);
	csd[15] = register_crc(csd);
	std::copy_n(cid, 16, m_cid);
	std::copy_n(csd, 16, m_csd);
	return { };
}

void ngage_mmc_device::call_unload()
{
	m_payload_size = 0;
	device_reset();
}

u32 ngage_mmc_device::irq_sources() const
{
	return ((m_status & DATA_BUSY) ? 0 : (1U << 21))
		| ((m_status & CMD_BUSY) ? 0 : (1U << 23))
		| ((m_status & RESP_READY) ? (1U << 24) : 0);
}

bool ngage_mmc_device::dma_ready(bool write) const
{
	return m_fifo_direction == (write ? FIFO_TX : FIFO_RX) && m_fifo_pos < m_fifo_length;
}

u32 ngage_mmc_device::read(offs_t offset, u32 mem_mask)
{
	switch (offset)
	{
	case 0x04 / 4: return m_reg[offset] & ~1U;
	case 0x18 / 4: return m_status;
	case 0x20 / 4: return response_r();
	case 0x24 / 4: return fifo_r(mem_mask);
	default: return m_reg[offset];
	}
}

void ngage_mmc_device::write(offs_t offset, u32 data, u32 mem_mask)
{
	if (offset == 0x24 / 4)
	{
		fifo_w(data, mem_mask);
		return;
	}
	COMBINE_DATA(&m_reg[offset]);
	if (offset == 0x04 / 4 && BIT(data & mem_mask, 0))
		start(m_reg[offset]);
}

void ngage_mmc_device::start(u32 control)
{
	m_type = (control >> 2) & 15;
	m_command = m_reg[0x1c / 4] & 63;
	m_argument = m_reg[0x20 / 4];
	m_response_words = m_response_pos = 0;
	m_fifo_direction = FIFO_IDLE;
	m_fifo_pos = m_fifo_length = 0;
	m_status = 0;
	m_command_timer->enable(false);
	m_tail_timer->enable(false);

	bool const continuation = m_type == 6 || m_type == 9;
	bool const data_first = m_type == 7 || m_type == 10;
	if (!continuation)
		m_status |= CMD_BUSY;
	if (m_type >= 5 && m_type <= 10)
		m_status |= DATA_BUSY;
	if (continuation || data_first)
		begin_data(m_type <= 7);
	else
		m_command_timer->adjust(clocks_to_attotime(48 + (m_type == 3 ? 136 : 48)));
	m_event_cb(1);
}

TIMER_CALLBACK_MEMBER(ngage_mmc_device::command_complete)
{
	card_command();
	if (m_response_words != 4)
		m_tail_timer->adjust(clocks_to_attotime(8));
	m_event_cb(1);
}

TIMER_CALLBACK_MEMBER(ngage_mmc_device::command_tail)
{
	m_status &= ~CMD_BUSY;
	m_event_cb(1);
}

void ngage_mmc_device::response(u32 value)
{
	if (m_type)
	{
		m_response[0] = value;
		m_response_words = 1;
		m_status |= RESP_READY;
	}
}

void ngage_mmc_device::response(u8 const *bytes)
{
	for (unsigned word = 0; word < 4; ++word)
		m_response[word] = get_u32be(bytes + word * 4);
	m_response_words = 4;
	m_status |= RESP_READY;
}

u32 ngage_mmc_device::response_r()
{
	if (m_response_pos == m_response_words)
		return 0;
	u32 const data = m_response[m_response_pos];
	if (!machine().side_effects_disabled())
	{
		++m_response_pos;
		// NEM-4 consumes three R2 words through CmdRdy, then waits for
		// CmdIdle before reading the fourth word.
		if (m_response_words == 4 && m_response_pos == 3)
			m_status &= ~CMD_BUSY;
		if (m_response_pos == m_response_words)
			m_status &= ~RESP_READY;
		m_event_cb(1);
	}
	return data;
}

bool ngage_mmc_device::in_range(u64 offset, u32 length) const
{
	return offset + length <= std::min(m_card_capacity, m_payload_size);
}

void ngage_mmc_device::card_command()
{
	if (!mounted())
	{
		if (m_type)
			m_status |= RESP_TIMEOUT;
		m_status &= ~DATA_BUSY;
		return;
	}

	u32 const r1 = (u32(m_card_state) << 9) | 0x100;
	bool const addressed = u16(m_argument >> 16) == m_rca;
	switch (m_command)
	{
	case 0:
		m_card_state = IDLE;
		m_rca = 1;
		m_block_length = SECTOR_BYTES;
		m_multiblock = false;
		return;
	case 1:
		if (m_card_state == IDLE)
		{
			response(m_ocr | 0x80000000);
			if (m_argument)
				m_card_state = READY;
			return;
		}
		break;
	case 2:
		if (m_card_state == READY)
		{
			response(m_cid);
			m_card_state = IDENT;
			return;
		}
		break;
	case 3:
		if (m_card_state == IDENT)
		{
			response(r1);
			m_rca = m_argument >> 16;
			m_card_state = STANDBY;
			return;
		}
		break;
	case 7:
		if (addressed && (m_card_state == STANDBY || m_card_state == TRANSFER))
		{
			response(r1);
			m_card_state = TRANSFER;
		}
		else
		{
			if (m_card_state >= TRANSFER)
				m_card_state = STANDBY;
			if (m_type)
				m_status |= RESP_TIMEOUT;
		}
		return;
	case 9:
	case 10:
		if (addressed && m_card_state == STANDBY)
		{
			response(m_command == 9 ? m_csd : m_cid);
			return;
		}
		break;
	case 12:
		if (m_card_state == DATA || m_card_state == RECEIVE || m_card_state == TRANSFER)
		{
			response(r1);
			m_card_state = TRANSFER;
			m_multiblock = false;
			return;
		}
		break;
	case 13:
		if (addressed && m_card_state >= STANDBY && m_card_state != INACTIVE)
		{
			response(r1);
			return;
		}
		break;
	case 15:
		if (addressed)
			m_card_state = INACTIVE;
		return;
	case 16:
		if (m_card_state == TRANSFER)
		{
			if (m_argument && m_argument <= SECTOR_BYTES && (BIT(m_csd[6], 7) || m_argument == SECTOR_BYTES))
			{
				m_block_length = m_argument;
				response(r1);
			}
			else
				response(r1 | BLOCK_LEN_ERROR);
			return;
		}
		break;
	case 17: case 18: case 24: case 25:
	{
		bool const write = m_command >= 24;
		unsigned const card_class = write ? 4 : 2;
		u32 const classes = (m_csd[4] << 4) | (m_csd[5] >> 4);
		if (m_card_state != TRANSFER || !BIT(classes, card_class))
			break;

		u32 error = 0;
		if (write && m_write_protected)
			error = WRITE_PROTECTED;
		else if (write && m_block_length != SECTOR_BYTES)
			error = BLOCK_LEN_ERROR;
		else if ((write || !BIT(m_csd[6], 5)) && m_argument % m_block_length)
			error = ADDRESS_ERROR;
		else if (!in_range(m_argument, m_block_length))
			error = OUT_OF_RANGE;
		response(r1 | error);
		if (error)
		{
			m_status = (m_status & ~DATA_BUSY) | DATA_TIMEOUT;
			return;
		}
		m_card_state = write ? RECEIVE : DATA;
		m_next_address = m_argument;
		m_multiblock = m_command == 18 || m_command == 25;
		if (m_status & DATA_BUSY)
			begin_data(write);
		return;
	}
	default:
		break;
	}
	if (m_type == 3)
		m_status |= RESP_TIMEOUT;
	else
		response(r1 | ILLEGAL_COMMAND);
	if (m_status & DATA_BUSY)
		m_status = (m_status & ~DATA_BUSY) | DATA_TIMEOUT;
}

void ngage_mmc_device::begin_data(bool write)
{
	u32 const programmed = m_reg[0x08 / 4];
	u32 const size = programmed ? std::min<u32>(programmed, sizeof(m_fifo)) : m_block_length;
	if (m_card_state != (write ? RECEIVE : DATA) || !in_range(m_next_address, size)
			|| (!m_multiblock && size > m_block_length) || (write && size % SECTOR_BYTES))
	{
		m_status |= DATA_TIMEOUT;
		end_data();
		return;
	}
	if (!write && (fseek(m_next_address, SEEK_SET) || fread(m_fifo, size) != size))
	{
		m_status |= DATA_ERROR;
		end_data();
		return;
	}
	m_fifo_length = size;
	m_fifo_pos = 0;
	m_fifo_direction = write ? FIFO_TX : FIFO_RX;
}

void ngage_mmc_device::end_data()
{
	if (m_fifo_direction == FIFO_TX
			&& (fseek(m_next_address, SEEK_SET) || fwrite(m_fifo, m_fifo_length) != m_fifo_length))
		m_status |= DATA_ERROR;
	m_next_address += m_fifo_length;
	m_fifo_direction = FIFO_IDLE;
	m_status &= ~DATA_BUSY;
	if (!m_multiblock && (m_card_state == RECEIVE || m_card_state == DATA))
		m_card_state = TRANSFER;
	if (m_type == 7 || m_type == 10)
		m_command_timer->adjust(clocks_to_attotime(96));
	m_event_cb(1);
}

u32 ngage_mmc_device::fifo_r(u32 mask)
{
	if (m_fifo_direction != FIFO_RX)
		return 0;

	u32 data = 0;
	unsigned idx = m_fifo_pos;
	for (unsigned lane = 0; lane < 32 && idx < m_fifo_length; lane += 8)
		if (mask & (0xffU << lane))
			data |= u32(m_fifo[idx++]) << lane;
	if (!machine().side_effects_disabled())
	{
		m_fifo_pos = idx;
		if (m_fifo_pos == m_fifo_length)
			end_data();
	}
	return data;
}

void ngage_mmc_device::fifo_w(u32 data, u32 mask)
{
	if (m_fifo_direction != FIFO_TX)
		return;

	for (unsigned lane = 0; lane < 32 && m_fifo_pos < m_fifo_length; lane += 8)
		if (mask & (0xffU << lane))
			m_fifo[m_fifo_pos++] = data >> lane;
	if (m_fifo_pos == m_fifo_length)
		end_data();
}
