// license:BSD-3-Clause
// copyright-holders:Sandro Ronco
/*****************************************************************************

        Hyperscan RFID card

*****************************************************************************/

#include "emu.h"
#include "hyperscan_card.h"

#include "softlist_dev.h"

//#define LOG_OUTPUT_FUNC printf
//#define VERBOSE 1
#include "logmacro.h"


DEFINE_DEVICE_TYPE(HYPERSCAN_CARD, hyperscan_card_device, "hyperscan_card", "Hyperscan RFID card")


hyperscan_card_device::hyperscan_card_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, HYPERSCAN_CARD, tag, owner, clock)
	, device_memcard_image_interface(mconfig, *this)
{
}

const software_list_loader &hyperscan_card_device::get_software_list_loader() const
{
	return image_software_list_loader::instance();
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void hyperscan_card_device::device_start()
{
	save_item(NAME(m_state));
	save_item(NAME(m_bit_pos));
	save_item(NAME(m_data));
	save_item(NAME(m_cmd_buf));
	save_item(NAME(m_resp_buf));
	save_item(NAME(m_resp_idx));
	save_item(NAME(m_last_at));
	save_item(NAME(m_memory));
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void hyperscan_card_device::device_reset()
{
	m_state = 0;
	m_bit_pos = 0;
	m_data = 0;
	m_cmd_buf.clear();
	m_resp_buf.clear();
	m_resp_idx = 0;
	m_last_at = attotime::zero;
}

/*-------------------------------------------------
   call_load()
-------------------------------------------------*/

image_init_result hyperscan_card_device::call_load()
{
	if (fread(m_memory, sizeof(m_memory)) != sizeof(m_memory))
		return image_init_result::FAIL;

	battery_load(m_memory, sizeof(m_memory), nullptr);

	return image_init_result::PASS;
}


/*-------------------------------------------------
   call_unload()
-------------------------------------------------*/

void hyperscan_card_device::call_unload()
{
	memset(m_memory, 0, sizeof(m_memory));
}


/*-------------------------------------------------
   CRC
-------------------------------------------------*/
uint16_t hyperscan_card_device::calc_crc(std::vector<uint8_t> const &data)
{
	uint16_t crc = 0xffffU;

	for (auto b : data)
	{
		b ^= crc & 0xff;
		b ^= b << 4;

		crc = (crc >> 8) ^ ((uint16_t)b << 8) ^ ((uint16_t)b << 3) ^ ((uint16_t)b >> 4);
	}

	return ~crc;
}


void hyperscan_card_device::resp_add_byte(uint8_t data, bool add_parity)
{
	int parity = 1;
	for (int i=0; i<8; i++)
	{
		int bit = BIT(data, i);
		m_resp_buf.push_back(bit);
		parity ^= bit;
	}

	if (add_parity)
		m_resp_buf.push_back(parity);
}

void hyperscan_card_device::make_resp(std::vector<uint8_t> const &data, bool add_crc)
{
	m_resp_idx = 0;
	m_resp_buf.clear();

	resp_add_byte(0x00, false);
	resp_add_byte(0x80, false);

	for (auto b : data)
		resp_add_byte(b, true);

	if (add_crc)
	{
		uint16_t crc = calc_crc(data);
		resp_add_byte(crc, true);
		resp_add_byte(crc >> 8, true);
	}
}

void hyperscan_card_device::check_command()
{
	std::vector<uint8_t> buf;

	if (m_cmd_buf.size() == 1 && m_cmd_buf[0] == 0x26)      // REQA
	{
		LOG("RFID: REQA\n");
		buf.clear();
		buf.push_back(0x00);    // ATQA0
		buf.push_back(0x00);    // ATQA1
		m_cmd_buf.clear();
		make_resp(buf, false);
	}
	else if (m_cmd_buf.size() == 1 && m_cmd_buf[0] == 0x52) // WUPA
	{
		LOG("RFID: WUPA\n");
		buf.clear();
		buf.push_back(0x00);    // ATQA0
		buf.push_back(0x00);    // ATQA1
		m_cmd_buf.clear();
		make_resp(buf, false);
	}
	else if (m_cmd_buf.size() == 9 && m_cmd_buf[0] == 0x78) // RID
	{
		LOG("RFID: RID\n");
		buf.clear();
		buf.push_back(0x11);    // HR0
		buf.push_back(0x00);    // HR1
		for (int i=0; i<4; i++)
			buf.push_back(m_memory[i]);     // UID

		m_cmd_buf.clear();
		make_resp(buf, true);
	}
	else if (m_cmd_buf.size() == 9 && m_cmd_buf[0] == 0x01) // READ
	{
		if (m_cmd_buf[1] >= sizeof(m_memory))   m_cmd_buf[1] %= sizeof(m_memory);
		LOG("RFID: READ %x %02x\n", m_cmd_buf[1], m_memory[m_cmd_buf[1]]);
		buf.clear();
		buf.push_back(m_cmd_buf[1]);            // ADD
		buf.push_back(m_memory[m_cmd_buf[1]]);  // DATA
		m_cmd_buf.clear();
		make_resp(buf, true);
	}
	else if (m_cmd_buf.size() == 9 && m_cmd_buf[0] == 0x00) // RALL
	{
		LOG("RFID: RALL\n");
		buf.clear();
		buf.push_back(0x11);            // HR0
		buf.push_back(0x00);            // HR1
		for (int i=0; i<sizeof(m_memory); i++)
			buf.push_back(m_memory[i]);

		m_cmd_buf.clear();
		make_resp(buf, true);
	}
	else if (m_cmd_buf.size() == 9 && m_cmd_buf[0] == 0x53) // WRITE-E
	{
		LOG("RFID: WRITE-E %x %02x\n", m_cmd_buf[1], m_cmd_buf[2]);
		if (m_cmd_buf[1] >= sizeof(m_memory))   m_cmd_buf[1] %= sizeof(m_memory);

		if (m_memory[m_cmd_buf[1]] != m_cmd_buf[2])
		{
			m_memory[m_cmd_buf[1]] = m_cmd_buf[2];
			battery_save(m_memory, sizeof(m_memory));
		}

		buf.clear();
		buf.push_back(m_cmd_buf[1]);    // ADD
		buf.push_back(m_cmd_buf[2]);    // DATA
		m_cmd_buf.clear();
		make_resp(buf, true);
	}
	else if (m_cmd_buf.size() == 9 && m_cmd_buf[0] == 0x1a) // WRITE-NE
	{
		LOG("RFID: WRITE-NE %x %02x\n", m_cmd_buf[1], m_cmd_buf[2]);
		if (m_cmd_buf[1] >= sizeof(m_memory))   m_cmd_buf[1] %= sizeof(m_memory);

		if (m_memory[m_cmd_buf[1]] != (m_memory[m_cmd_buf[1]] | m_cmd_buf[2]))
		{
			m_memory[m_cmd_buf[1]] |= m_cmd_buf[2];
			battery_save(m_memory, sizeof(m_memory));
		}

		buf.clear();
		buf.push_back(m_cmd_buf[1]);    // ADD
		buf.push_back(m_cmd_buf[2]);    // DATA
		m_cmd_buf.clear();
		make_resp(buf, true);
	}
}

void hyperscan_card_device::write(int state)
{
	if (m_state == state || !is_loaded())
		return;

	m_state = state;

	// 13.56MHz
	int clk = 13560000 / (machine().time() - m_last_at).as_hz();
	m_last_at = machine().time();

	if (state)
		return;

	if (clk < 1000)
	{
		if (clk < 500)
			m_data &= ~(1 << m_bit_pos);
		else
			m_data |= 1 << m_bit_pos;

		m_bit_pos++;

		if ((m_cmd_buf.size() == 0 && m_bit_pos == 7) || m_bit_pos == 8)
		{
			if (m_cmd_buf.size() == 0)
				m_data &= 0x7f;

			m_cmd_buf.push_back(m_data);
			m_data = 0;
			m_bit_pos = 0;
			check_command();
		}
	}
	else if (clk < 5000)
	{
		m_bit_pos = 0;
		m_data = 0;
		if (m_resp_idx <= m_resp_buf.size() * 2)
			m_resp_idx++;
	}
}

int hyperscan_card_device::read()
{
	if (is_loaded() && (m_resp_idx >> 1) < m_resp_buf.size())
		return m_resp_buf[m_resp_idx >> 1] ^ (m_resp_idx & 1);

	return 0;
}
