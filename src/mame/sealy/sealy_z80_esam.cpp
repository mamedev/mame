// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
/*
    Secure module (ESAM) on Sealy Z80 boards, high-level emulation.
    See sealy_z80_esam.h for what is known about the chip and what is modelled.
*/

#include "emu.h"
#include "sealy_z80_esam.h"

#include "multibyte.h"

#include <algorithm>
#include <bit>

#define LOG_APDU (1U << 1)
#define LOG_BYTES (1U << 2)

#define VERBOSE (0)
#include "logmacro.h"


DEFINE_DEVICE_TYPE(SEALY_Z80_ESAM, sealy_z80_esam_device, "sealy_z80_esam", "Sealy Z80 board ESAM (HLE)")


namespace {

// Standard DES, adapted from the one in machine/fs8806.cpp (R. Belmont).

constexpr uint8_t IP_TBL[64] = {
	58,50,42,34,26,18,10, 2, 60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6, 64,56,48,40,32,24,16, 8,
	57,49,41,33,25,17, 9, 1, 59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5, 63,55,47,39,31,23,15, 7 };

constexpr uint8_t FP_TBL[64] = {
	40, 8,48,16,56,24,64,32, 39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30, 37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28, 35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26, 33, 1,41, 9,49,17,57,25 };

constexpr uint8_t E_TBL[48] = {
	32, 1, 2, 3, 4, 5,  4, 5, 6, 7, 8, 9,  8, 9,10,11,12,13, 12,13,14,15,16,17,
	16,17,18,19,20,21, 20,21,22,23,24,25, 24,25,26,27,28,29, 28,29,30,31,32, 1 };

constexpr uint8_t P_TBL[32] = {
	16, 7,20,21,29,12,28,17,  1,15,23,26, 5,18,31,10,
	 2, 8,24,14,32,27, 3, 9, 19,13,30, 6,22,11, 4,25 };

constexpr uint8_t PC1_TBL[56] = {
	57,49,41,33,25,17, 9,  1,58,50,42,34,26,18, 10, 2,59,51,43,35,27, 19,11, 3,60,52,44,36,
	63,55,47,39,31,23,15,  7,62,54,46,38,30,22, 14, 6,61,53,45,37,29, 21,13, 5,28,20,12, 4 };

constexpr uint8_t PC2_TBL[48] = {
	14,17,11,24, 1, 5,  3,28,15, 6,21,10, 23,19,12, 4,26, 8, 16, 7,27,20,13, 2,
	41,52,31,37,47,55, 30,40,51,45,33,48, 44,49,39,56,34,53, 46,42,50,36,29,32 };

constexpr uint8_t SHIFTS[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

constexpr uint8_t SBOX[8][64] = {
	{14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,  0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
	  4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0, 15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13},
	{15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,  3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
	  0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15, 13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9},
	{10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8, 13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
	 13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,  1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12},
	{ 7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15, 13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
	 10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,  3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14},
	{ 2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9, 14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
	  4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14, 11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3},
	{12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11, 10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
	  9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,  4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13},
	{ 4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1, 13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
	  1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,  6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12},
	{13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,  1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
	  7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,  2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11} };

uint64_t des_permute(uint64_t in, const uint8_t *tbl, int count, int inbits)
{
	uint64_t out = 0;
	for (int i = 0; i < count; i++)
		out = (out << 1) | BIT(in, inbits - tbl[i]);
	return out;
}

uint64_t des_encrypt(uint64_t block, uint64_t key)
{
	uint64_t sk[16];
	const uint64_t cd = des_permute(key, PC1_TBL, 56, 64);
	uint32_t c = uint32_t(cd >> 28) & 0x0fffffff;
	uint32_t d = uint32_t(cd) & 0x0fffffff;
	for (int i = 0; i < 16; i++)
	{
		const int s = SHIFTS[i];
		c = ((c << s) | (c >> (28 - s))) & 0x0fffffff;
		d = ((d << s) | (d >> (28 - s))) & 0x0fffffff;
		sk[i] = des_permute((uint64_t(c) << 28) | d, PC2_TBL, 48, 56);
	}

	const uint64_t ip = des_permute(block, IP_TBL, 64, 64);
	uint32_t l = uint32_t(ip >> 32), r = uint32_t(ip);
	for (int i = 0; i < 16; i++)
	{
		const uint64_t e = des_permute(r, E_TBL, 48, 32) ^ sk[i];
		uint32_t f = 0;
		for (int b = 0; b < 8; b++)
		{
			const uint8_t six = uint8_t(BIT(e, 42 - 6 * b, 6));
			const uint8_t row = (BIT(six, 5) << 1) | BIT(six, 0);
			const uint8_t col = BIT(six, 1, 4);
			f = (f << 4) | SBOX[b][row * 16 + col];
		}
		f = uint32_t(des_permute(f, P_TBL, 32, 32));
		const uint32_t nl = r;
		r = l ^ f;
		l = nl;
	}
	return des_permute((uint64_t(r) << 32) | l, FP_TBL, 64, 64);
}

} // anonymous namespace


sealy_z80_esam_device::sealy_z80_esam_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, SEALY_Z80_ESAM, tag, owner, clock),
	device_nvram_interface(mconfig, *this),
	m_default_image(*this, DEVICE_SELF)
{
}

void sealy_z80_esam_device::device_start()
{
	m_tx_start.fill(0);
	m_tx_byte.fill(0);
	m_rx.fill(0);

	save_item(NAME(m_system));
	save_item(NAME(m_file));
	save_item(NAME(m_tx_start));
	save_item(NAME(m_tx_byte));
	save_item(NAME(m_tx_head));
	save_item(NAME(m_tx_count));
	save_item(NAME(m_tx_end));
	save_item(NAME(m_rx));
	save_item(NAME(m_rx_count));
	save_item(NAME(m_payload));
	save_item(NAME(m_rx_sample));
	save_item(NAME(m_rx_bit));
	save_item(NAME(m_rx_value));
	save_item(NAME(m_parity));
	save_item(NAME(m_receiving));
	save_item(NAME(m_parity_ok));
	save_item(NAME(m_overflow));
	save_item(NAME(m_error_until));
	save_item(NAME(m_line));
	save_item(NAME(m_clock));
	save_item(NAME(m_reset));
	save_item(NAME(m_selected));
	save_item(NAME(m_authenticated));
	save_item(NAME(m_verified));
	save_item(NAME(m_challenge_valid));
	save_item(NAME(m_challenge));
}

void sealy_z80_esam_device::device_reset()
{
	reset_session();
	m_clock = false;
	m_reset = false;
	m_line = true;
}


//-------------------------------------------------
//  NVRAM
//-------------------------------------------------

void sealy_z80_esam_device::nvram_default()
{
	if (m_default_image.found())
	{
		if (m_default_image->bytes() != SYSTEM_SIZE + FILE_SIZE)
			throw emu_fatalerror("%s: default image must be 0x%X bytes", tag(), SYSTEM_SIZE + FILE_SIZE);
		std::copy_n(m_default_image->base(), SYSTEM_SIZE, m_system.begin());
		std::copy_n(m_default_image->base() + SYSTEM_SIZE, FILE_SIZE, m_file.begin());
		return;
	}

	// blank chip: nothing provisioned yet
	m_system.fill(0);
	m_system[0] = 'S';
	m_system[1] = 'C';
	m_system[2] = 1;
	m_file.fill(0xff);
}

bool sealy_z80_esam_device::nvram_read(util::read_stream &file)
{
	auto const [err1, actual1] = util::read(file, m_system.data(), SYSTEM_SIZE);
	if (err1 || (actual1 != SYSTEM_SIZE))
		return false;
	auto const [err2, actual2] = util::read(file, m_file.data(), FILE_SIZE);
	return !err2 && (actual2 == FILE_SIZE);
}

bool sealy_z80_esam_device::nvram_write(util::write_stream &file)
{
	auto const [err1, actual1] = util::write(file, m_system.data(), SYSTEM_SIZE);
	if (err1)
		return false;
	auto const [err2, actual2] = util::write(file, m_file.data(), FILE_SIZE);
	return !err2;
}


//-------------------------------------------------
//  helpers
//-------------------------------------------------

uint64_t sealy_z80_esam_device::now() const
{
	return machine().time().as_ticks(clock());
}

bool sealy_z80_esam_device::has(uint8_t objects) const
{
	return m_system[0] == 'S' && m_system[1] == 'C' && m_system[2] == 1 && (m_system[4] & objects) == objects;
}

void sealy_z80_esam_device::clear_serial()
{
	m_tx_head = m_tx_count = 0;
	m_rx_count = m_payload = 0;
	m_rx_sample = m_tx_end = m_error_until = 0;
	m_rx_bit = m_rx_value = m_parity = 0;
	m_receiving = m_overflow = false;
	m_parity_ok = true;
}

void sealy_z80_esam_device::reset_session()
{
	clear_serial();
	m_selected = m_authenticated = m_verified = m_challenge_valid = false;
	m_challenge = 0;
}

void sealy_z80_esam_device::enqueue(uint64_t t, const uint8_t *bytes, uint32_t count)
{
	if (!count)
		return;
	if (count > TX_CAPACITY - m_tx_count)
	{
		m_overflow = true;
		return;
	}

	uint64_t start = std::max(t + 6 * ETU, m_tx_end + 3 * ETU);
	for (uint32_t i = 0; i < count; i++)
	{
		const uint32_t slot = (m_tx_head + m_tx_count++) % TX_CAPACITY;
		m_tx_start[slot] = start;
		m_tx_byte[slot] = bytes[i];
		start += 14 * ETU;
	}
	m_tx_end = start - 3 * ETU;
}

void sealy_z80_esam_device::enqueue(uint64_t t, std::initializer_list<uint8_t> bytes)
{
	enqueue(t, bytes.begin(), bytes.size());
}

void sealy_z80_esam_device::status(uint64_t t, uint8_t sw1, uint8_t sw2)
{
	enqueue(t, { sw1, sw2 });
}

bool sealy_z80_esam_device::bytes_equal(uint32_t offset, std::initializer_list<uint8_t> expected) const
{
	return offset + expected.size() <= m_rx_count && std::equal(expected.begin(), expected.end(), m_rx.begin() + offset);
}


//-------------------------------------------------
//  APDU handling
//-------------------------------------------------

// CLA 80: provisioning.  Existing contents can only be erased after both
// authentication and PIN verification; a blank chip can be provisioned in
// dependency order.  The real access conditions are unknown, so only the exact
// descriptors used by the lucky168 host are accepted.
void sealy_z80_esam_device::management(uint64_t t)
{
	const uint8_t ins = m_rx[1];
	const uint16_t id = get_u16be(&m_rx[2]);
	const uint8_t n = m_rx[4];

	if (m_system[0] != 'S' || m_system[1] != 'C' || m_system[2] != 1)
		return status(t, 0x6f);

	if (ins == 0x0e) // ERASE DF
	{
		if (id || n)
			return status(t, 0x6a, 0x86);
		if (has(OBJ_DES_KEY | OBJ_PIN) && (!m_authenticated || !m_verified))
			return status(t, 0x69, 0x82);
		m_system.fill(0);
		m_system[0] = 'S';
		m_system[1] = 'C';
		m_system[2] = 1;
		m_file.fill(0xff);
		m_selected = m_authenticated = m_verified = m_challenge_valid = false;
		return status(t, 0x90);
	}

	if (ins == 0xe0) // CREATE FILE
	{
		uint8_t object = 0;
		if (id == 0x3f00 && n == 13 && bytes_equal(5, { 0x38, 0xff, 0xff, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }))
			object = OBJ_MF;
		else if (id == 0x0000 && n == 7 && bytes_equal(5, { 0x3f, 0x00, 0x50, 0xff, 0xf0, 0xff, 0xff }))
			object = OBJ_KEY_FILE;
		else if (id == 0x001d && n == 7 && bytes_equal(5, { 0x28, 0x01, 0x00, 0xf4, 0xf4, 0xff, 0xff }))
			object = OBJ_DATA_FILE;

		if (!object)
			return status(t, 0x6a, 0x80);
		if (has(object))
			return status(t, 0x6a, 0x89);
		if (object != OBJ_MF && !has(OBJ_MF))
			return status(t, 0x69, 0x85);
		m_system[4] |= object;
		if (object == OBJ_DATA_FILE)
			m_file.fill(0xff);
		return status(t, 0x90);
	}

	if (ins == 0xd4) // WRITE KEY
	{
		if (!has(OBJ_KEY_FILE))
			return status(t, 0x69, 0x85);

		uint8_t object = 0;
		uint8_t destination = 0;
		if (id == 0x0101 && n == 13 && bytes_equal(5, { 0x39, 0xf0, 0xfe, 0x02, 0x44 }))
		{
			object = OBJ_DES_KEY;
			destination = 0x08;
		}
		else if (id == 0x0135 && n == 13 && bytes_equal(5, { 0x3a, 0xf2, 0xfe, 0x04, 0x44 }))
		{
			object = OBJ_PIN;
			destination = 0x10;
		}

		if (!object)
			return status(t, 0x6a, 0x80);
		if (has(object))
			return status(t, 0x6a, 0x89);
		std::copy_n(m_rx.begin() + 10, 8, m_system.begin() + destination);
		m_system[4] |= object;
		m_authenticated = m_verified = m_challenge_valid = false;
		return status(t, 0x90);
	}

	status(t, 0x6d);
}

void sealy_z80_esam_device::command(uint64_t t)
{
	LOGMASKED(LOG_APDU, "APDU %02x %02x %02x %02x %02x (%u bytes)\n", m_rx[0], m_rx[1], m_rx[2], m_rx[3], m_rx[4], m_rx_count);

	const uint8_t ins = m_rx[1];
	const uint8_t n = m_rx[4];
	const uint16_t p1p2 = get_u16be(&m_rx[2]);

	if (m_rx[0] == 0x80)
	{
		management(t);
	}
	else if (ins == 0x84) // GET CHALLENGE
	{
		if (p1p2)
			status(t, 0x6a, 0x86);
		else if (n != 8)
			status(t, 0x67);
		else
		{
			uint8_t answer[11];
			answer[0] = ins;
			for (int i = 0; i < 8; i++)
				answer[1 + i] = machine().rand() & 0xff;
			m_challenge = get_u64be(&answer[1]);
			answer[9] = 0x90;
			answer[10] = 0x00;
			enqueue(t, answer, sizeof(answer));
			m_challenge_valid = true;
		}
	}
	else if (ins == 0x82) // EXTERNAL AUTHENTICATE, key 01
	{
		if (p1p2 != 0x0001)
			status(t, 0x6a, 0x86);
		else if (n != 8)
			status(t, 0x67);
		else
		{
			const uint64_t key = get_u64be(&m_system[0x08]);
			const uint64_t response = get_u64be(&m_rx[5]);
			m_authenticated = has(OBJ_DES_KEY) && m_challenge_valid && (response == des_encrypt(m_challenge, key));
			m_challenge_valid = false;
			status(t, m_authenticated ? 0x90 : 0x63);
		}
	}
	else if (ins == 0x20) // VERIFY, PIN 35
	{
		if (p1p2 != 0x0035)
			status(t, 0x6a, 0x86);
		else if (n != 8)
			status(t, 0x67);
		else
		{
			m_verified = has(OBJ_PIN) && std::equal(m_rx.begin() + 5, m_rx.begin() + 13, m_system.begin() + 0x10);
			status(t, m_verified ? 0x90 : 0x63);
		}
	}
	else if (ins == 0xa4) // SELECT
	{
		if (p1p2)
			status(t, 0x6a, 0x86);
		else if (n != 2)
			status(t, 0x67);
		else
		{
			m_selected = has(OBJ_DATA_FILE) && m_rx[5] == 0x00 && m_rx[6] == 0x1d;
			status(t, m_selected ? 0x90 : 0x6a, m_selected ? 0x00 : 0x82);
		}
	}
	else if (ins == 0xb0 || ins == 0xd6) // READ BINARY / UPDATE BINARY
	{
		const uint32_t length = n ? n : 256; // Le = 00 means 256; a zero Lc is rejected earlier
		if (!m_selected || !has(OBJ_DATA_FILE))
			status(t, 0x69, 0x86);
		else if (p1p2 + length > FILE_SIZE)
			status(t, 0x6b);
		else if (!m_authenticated || !m_verified)
			status(t, 0x69, 0x82);
		else if (ins == 0xb0)
		{
			enqueue(t, { ins });
			enqueue(t, &m_file[p1p2], length);
			enqueue(t, { 0x90, 0x00 });
		}
		else
		{
			std::copy_n(m_rx.begin() + 5, length, m_file.begin() + p1p2);
			status(t, 0x90);
		}
	}
	else if (ins == 0xc0) // GET RESPONSE: SELECT answered 9000, nothing is pending
	{
		status(t, 0x6a, 0x86);
	}
	else
	{
		status(t, 0x6d);
	}

	m_rx_count = m_payload = 0;
}

void sealy_z80_esam_device::byte_received(uint64_t t, uint8_t value)
{
	LOGMASKED(LOG_BYTES, "RX %02x\n", value);

	if (m_rx_count >= RX_CAPACITY)
	{
		m_overflow = true;
		m_rx_count = m_payload = 0;
		return;
	}

	m_rx[m_rx_count++] = value;
	if (m_rx_count == 5)
	{
		const uint8_t ins = m_rx[1];
		const bool standard = m_rx[0] == 0x00 && (ins == 0x84 || ins == 0x82 || ins == 0x20 || ins == 0xa4 || ins == 0xb0 || ins == 0xd6 || ins == 0xc0);
		const bool proprietary = m_rx[0] == 0x80 && (ins == 0x0e || ins == 0xe0 || ins == 0xd4);
		if (!standard && !proprietary)
		{
			status(t, (m_rx[0] == 0x00 || m_rx[0] == 0x80) ? 0x6d : 0x6e);
			m_rx_count = m_payload = 0;
			return;
		}

		const bool incoming = ins == 0x82 || ins == 0x20 || ins == 0xa4 || ins == 0xd6 || ins == 0xe0 || ins == 0xd4;
		if (incoming && !m_rx[4])
		{
			status(t, 0x67);
			m_rx_count = m_payload = 0;
			return;
		}

		m_payload = incoming ? m_rx[4] : 0;
		if (m_payload)
			enqueue(t, { ins }); // procedure byte: send the data
		else
			command(t);
	}
	else if (m_payload && m_rx_count == 5 + m_payload)
	{
		command(t);
	}
}

// Bring the receiver and the transmit queue up to time t.
void sealy_z80_esam_device::advance(uint64_t t)
{
	while (m_receiving && m_rx_sample <= t)
	{
		if (m_rx_bit < 8)
		{
			m_rx_value |= uint8_t(m_line) << m_rx_bit;
			m_parity ^= m_line;
		}
		else if (m_rx_bit == 8)
		{
			m_parity_ok = m_parity == m_line;
			if (!m_parity_ok)
				m_error_until = m_rx_sample + 2 * ETU;
		}
		else
		{
			m_receiving = false;
			if (m_parity_ok && m_line)
				byte_received(m_rx_sample, m_rx_value);
		}
		m_rx_bit++;
		m_rx_sample += ETU;
	}

	while (m_tx_count && t >= m_tx_start[m_tx_head] + 11 * ETU)
	{
		m_tx_head = (m_tx_head + 1) % TX_CAPACITY;
		m_tx_count--;
	}
}


//-------------------------------------------------
//  line interface
//-------------------------------------------------

void sealy_z80_esam_device::clk_w(int state)
{
	const uint64_t t = now();
	advance(t);
	if (m_clock && !state)
		clear_serial();
	m_clock = state;
}

void sealy_z80_esam_device::rst_w(int state)
{
	const uint64_t t = now();
	advance(t);
	if (!state)
		reset_session();
	if (state && !m_reset && m_clock)
		enqueue(t, { 0x3b, 0x00 }); // minimal ATR
	m_reset = state;
}

void sealy_z80_esam_device::io_w(int state)
{
	const uint64_t t = now();
	advance(t);
	if (m_reset && m_clock && !m_receiving && m_line && !state && t >= m_error_until)
	{
		// start bit from the host
		m_receiving = true;
		m_rx_sample = t + ETU * 3 / 2;
		m_rx_bit = m_rx_value = m_parity = 0;
		m_parity_ok = true;
	}
	m_line = state;
}

int sealy_z80_esam_device::line_state(uint64_t t) const
{
	if (!m_reset || !m_clock)
		return m_line; // inactive: the line is just what the host drives
	if (!m_line || t < m_error_until)
		return 0;

	uint32_t head = m_tx_head;
	uint32_t count = m_tx_count;
	while (count && t >= m_tx_start[head] + 11 * ETU)
	{
		head = (head + 1) % TX_CAPACITY;
		count--;
	}
	if (!count || t < m_tx_start[head])
		return 1;

	const uint64_t bit = (t - m_tx_start[head]) / ETU;
	if (!bit)
		return 0; // start bit
	if (bit <= 8)
		return BIT(m_tx_byte[head], bit - 1);
	if (bit == 9)
		return std::popcount(m_tx_byte[head]) & 1; // even parity
	return 1;
}

int sealy_z80_esam_device::io_r()
{
	const uint64_t t = now();
	if (!machine().side_effects_disabled())
		advance(t);
	return line_state(t);
}
