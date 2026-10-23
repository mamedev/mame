// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    fs8806.cpp

    FameG (福华先进微电子, Shanghai) FS8806 authentication and secure-counter chip
    Emulation by R. Belmont

    I2C dongle (SPI mode not emulated).  Every message is one or more DES-ECB
    blocks under a shared 8-byte key, each block byte-reversed into and out of
    the cipher, followed by a 16-bit CRC.  The chip counts the data messages
    it accepts and the host periodically compares that count with its own.

    Message: [ command, sequence, ... ][ payload from byte 8 ][ crc-lo, crc-hi ]

    Commands, as used by the Happy Fish 302-in-1 Linux kernel:

    0xe1, 0xf1  prefix sent ahead of a reset or init, no reply
    0x95        reset          -> 0xa5 (0xa6 on failure), zeroes the counter
    0x94        init           -> 0xa3 (0xa4 on failure)
    0x81        bulk data, 8 to 0x40 payload bytes, no reply, counts
    0x82        8 payload bytes, no reply, counts
    0x91        report counter -> 0xa1 with the count in byte 1
    0xb1        present an 8-byte challenge, no reply
    0x92        read challenge -> 0xb2 with the complement of the challenge
    0xb3        write EEPROM, byte 6 address, byte 7 length, counts
    0x98        verify write   -> 0xb4 with the complement of the data
    0xb5        read EEPROM, byte 6 address, byte 7 length -> 0xb6 with data
    0x71        write EEPROM without the verify step, counts
    0x96        status         -> 0xa7

***************************************************************************/

#include "emu.h"
#include "fs8806.h"

#include "multibyte.h"

#include <algorithm>
#include <bit>
#include <iterator>

#define LOG_COMMAND (1U << 1)
#define LOG_PACKET  (1U << 2)

#define VERBOSE (0)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(FS8806, fs8806_device, "fs8806", "FameG FS8806 authentication chip")

namespace {

// standard DES tables
constexpr u8 IP_TBL[64] = {
	58,50,42,34,26,18,10, 2, 60,52,44,36,28,20,12, 4,
	62,54,46,38,30,22,14, 6, 64,56,48,40,32,24,16, 8,
	57,49,41,33,25,17, 9, 1, 59,51,43,35,27,19,11, 3,
	61,53,45,37,29,21,13, 5, 63,55,47,39,31,23,15, 7 };

constexpr u8 FP_TBL[64] = {
	40, 8,48,16,56,24,64,32, 39, 7,47,15,55,23,63,31,
	38, 6,46,14,54,22,62,30, 37, 5,45,13,53,21,61,29,
	36, 4,44,12,52,20,60,28, 35, 3,43,11,51,19,59,27,
	34, 2,42,10,50,18,58,26, 33, 1,41, 9,49,17,57,25 };

constexpr u8 E_TBL[48] = {
	32, 1, 2, 3, 4, 5,  4, 5, 6, 7, 8, 9,  8, 9,10,11,12,13, 12,13,14,15,16,17,
	16,17,18,19,20,21, 20,21,22,23,24,25, 24,25,26,27,28,29, 28,29,30,31,32, 1 };

constexpr u8 P_TBL[32] = {
	16, 7,20,21,29,12,28,17,  1,15,23,26, 5,18,31,10,
	 2, 8,24,14,32,27, 3, 9, 19,13,30, 6,22,11, 4,25 };

constexpr u8 PC1_TBL[56] = {
	57,49,41,33,25,17, 9,  1,58,50,42,34,26,18, 10, 2,59,51,43,35,27, 19,11, 3,60,52,44,36,
	63,55,47,39,31,23,15,  7,62,54,46,38,30,22, 14, 6,61,53,45,37,29, 21,13, 5,28,20,12, 4 };

constexpr u8 PC2_TBL[48] = {
	14,17,11,24, 1, 5,  3,28,15, 6,21,10, 23,19,12, 4,26, 8, 16, 7,27,20,13, 2,
	41,52,31,37,47,55, 30,40,51,45,33,48, 44,49,39,56,34,53, 46,42,50,36,29,32 };

constexpr u8 SHIFTS[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

constexpr u8 SBOX[8][64] = {
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

u64 permute(u64 in, const u8 *tbl, int count, int inbits)
{
	u64 out = 0;
	for (int i = 0; i < count; i++)
	{
		out = (out << 1) | ((in >> (inbits - tbl[i])) & 1);
	}
	return out;
}

void des_keysched(u64 key, u64 sk[16])
{
	const u64 cd = permute(key, PC1_TBL, 56, 64);
	u32 c = u32(cd >> 28) & 0x0fffffff;
	u32 d = u32(cd) & 0x0fffffff;
	for (int i = 0; i < 16; i++)
	{
		const int s = SHIFTS[i];
		c = ((c << s) | (c >> (28 - s))) & 0x0fffffff;
		d = ((d << s) | (d >> (28 - s))) & 0x0fffffff;
		sk[i] = permute((u64(c) << 28) | d, PC2_TBL, 48, 56);
	}
}

u64 des_block(u64 block, const u64 sk[16], bool decrypt)
{
	const u64 ip = permute(block, IP_TBL, 64, 64);
	u32 l = u32(ip >> 32), r = u32(ip);
	for (int i = 0; i < 16; i++)
	{
		const u64 e = permute(r, E_TBL, 48, 32) ^ sk[decrypt ? (15 - i) : i];
		u32 f = 0;
		for (int b = 0; b < 8; b++)
		{
			const u8 six = u8((e >> (42 - 6 * b)) & 0x3f);
			const u8 row = ((six >> 4) & 2) | (six & 1);
			const u8 col = (six >> 1) & 0x0f;
			f = (f << 4) | SBOX[b][row * 16 + col];
		}
		f = u32(permute(f, P_TBL, 32, 32));
		const u32 nl = r;
		r = l ^ f;
		l = nl;
	}
	return permute((u64(r) << 32) | l, FP_TBL, 64, 64);
}

} // anonymous namespace


fs8806_device::fs8806_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock) :
	device_t(mconfig, FS8806, tag, owner, clock),
	i2c_hle_interface(mconfig, *this, 0x62),
	m_key(0),
	m_sk{},
	m_rx{},
	m_rx_len(0),
	m_tx{},
	m_tx_len(0),
	m_challenge{},
	m_verify{},
	m_counter(0)
{
}

void fs8806_device::device_start()
{
	des_keysched(m_key, m_sk);
	std::fill(std::begin(m_eeprom), std::end(m_eeprom), 0xff);

	save_item(NAME(m_rx));
	save_item(NAME(m_rx_len));
	save_item(NAME(m_tx));
	save_item(NAME(m_tx_len));
	save_item(NAME(m_eeprom));
	save_item(NAME(m_challenge));
	save_item(NAME(m_verify));
	save_item(NAME(m_counter));
}

void fs8806_device::device_reset()
{
	m_rx_len = 0;
	m_tx_len = 0;
	m_counter = 0;
}

/***************************************************************************
    CIPHER AND CHECKSUM
***************************************************************************/

// each 8-byte block is byte-reversed, run through DES-ECB, and reversed again
void fs8806_device::crypt(u8 *dst, const u8 *src, int length, bool encrypt) const
{
	for (int i = 0; i < length; i += 8)
	{
		u64 v = 0;
		for (int j = 0; j < 8; j++)
		{
			v = (v << 8) | src[i + 7 - j];
		}

		v = des_block(v, m_sk, !encrypt);

		for (int j = 0; j < 8; j++)
		{
			dst[i + j] = u8(v >> (8 * j));
		}
	}
}

// a 16-bit CRC kept with its halves swapped and its bits reversed, so it matches no named variant
u16 fs8806_device::crc(const u8 *data, int length)
{
	u16 state = 0xffff;

	for (int i = 0; i < length; i++)
	{
		const u8 t = data[i] ^ bitswap<8>(u8(state >> 8), 0, 1, 2, 3, 4, 5, 6, 7);
		const u8 parity = BIT(std::popcount(t), 0);

		u16 next = 0;
		next |= u16(parity ^ BIT(state, 7)) << 15;
		next |= u16(BIT(state, 6)) << 14;
		next |= u16(BIT(state, 5)) << 13;
		next |= u16(BIT(state, 4)) << 12;
		next |= u16(BIT(state, 3)) << 11;
		next |= u16(BIT(state, 2)) << 10;
		next |= u16(BIT(t, 0) ^ BIT(state, 1)) << 9;
		next |= u16(BIT(t, 0) ^ BIT(t, 1) ^ BIT(state, 0)) << 8;
		next |= u16(BIT(t, 1) ^ BIT(t, 2)) << 7;
		next |= u16(BIT(t, 2) ^ BIT(t, 3)) << 6;
		next |= u16(BIT(t, 3) ^ BIT(t, 4)) << 5;
		next |= u16(BIT(t, 4) ^ BIT(t, 5)) << 4;
		next |= u16(BIT(t, 5) ^ BIT(t, 6)) << 3;
		next |= u16(BIT(t, 6) ^ BIT(t, 7)) << 2;
		next |= u16(BIT(std::popcount(u8(t & 0x7f)), 0)) << 1;
		next |= parity;
		state = next;
	}

	return bitswap<16>(u16(~state), 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);
}

/***************************************************************************
    I2C INTERFACE
***************************************************************************/

void fs8806_device::write_data(u16 offset, u8 data)
{
	if (offset < std::size(m_rx))
	{
		m_rx[offset] = data;
		m_rx_len = u8(offset) + 1;
	}
	else
	{
		logerror("overlong packet, dropping byte %02x at %d\n", data, offset);
	}
}

u8 fs8806_device::read_data(u16 offset)
{
	return (offset < m_tx_len) ? m_tx[offset] : 0xff;
}

void fs8806_device::i2c_stop()
{
	if (m_rx_len != 0)
	{
		handle_packet();
		m_rx_len = 0;
	}
}

/***************************************************************************
    PROTOCOL
***************************************************************************/

// encrypt m_tx in place and append the CRC, leaving a full reply for the host to read
void fs8806_device::make_response(u8 length)
{
	crypt(m_tx, m_tx, length, true);
	const u16 sum = crc(m_tx, length);
	put_u16le(&m_tx[length], sum);
	m_tx_len = length + 2;

	if (VERBOSE & LOG_PACKET)
	{
		std::string dump;
		for (int i = 0; i < m_tx_len; i++)
		{
			dump += util::string_format(" %02x", m_tx[i]);
		}
		LOGMASKED(LOG_PACKET, "reply%s\n", dump);
	}
}

void fs8806_device::handle_packet()
{
	if ((m_rx_len < 10) || ((m_rx_len - 2) & 7))
	{
		logerror("bad packet length %d\n", m_rx_len);
		return;
	}

	const int payload = m_rx_len - 2;
	const u16 expected = get_u16le(&m_rx[payload]);
	const u16 actual = crc(m_rx, payload);

	// the resync packet the host sends before every command carries no CRC
	const bool crc_ok = (expected == actual);

	u8 packet[80];
	crypt(packet, m_rx, payload, false);

	const u8 command = packet[0];
	const u8 sequence = packet[1];

	if (!crc_ok)
	{
		LOGMASKED(LOG_COMMAND, "ignoring command %02x with bad CRC (%04x, expected %04x)\n", command, expected, actual);
		return;
	}

	LOGMASKED(LOG_COMMAND, "command %02x sequence %02x (counter %02x) payload %d\n",
			command, sequence, m_counter & 0xff, payload - 8);

	// every message that carries data advances the counter the host is checking
	auto count = [this, sequence] ()
	{
		if (sequence != (m_counter & 0xff))
		{
			logerror("host sequence %02x doesn't match counter %02x\n", sequence, m_counter & 0xff);
		}
		m_counter++;
	};

	m_tx_len = 0;
	std::fill(std::begin(m_tx), std::end(m_tx), 0);

	switch (command)
	{
		case 0xe1:  // sent ahead of a reset
		case 0xf1:  // sent ahead of an init
			break;

		case 0x95:  // reset
			m_counter = 0;
			m_tx[0] = 0xa5;
			make_response(8);
			break;

		case 0x94:  // init
			m_tx[0] = 0xa3;
			make_response(8);
			break;

		case 0x96:  // status
			m_tx[0] = 0xa7;
			make_response(8);
			break;

		case 0x81:  // bulk data
		case 0x82:  // one block of data
			count();
			break;

		case 0x91:  // report the counter
			m_tx[0] = 0xa1;
			m_tx[1] = u8(m_counter);
			make_response(8);
			break;

		case 0xb1:  // present a challenge
			std::copy_n(&packet[8], 8, m_challenge);
			break;

		case 0x92:  // answer the challenge with its complement
			m_tx[0] = 0xb2;
			for (int i = 0; i < 8; i++)
			{
				m_tx[8 + i] = m_challenge[i] ^ 0xff;
			}
			make_response(16);
			break;

		case 0x71:  // write EEPROM
		case 0xb3:  // write EEPROM, to be confirmed with 0x98
		{
			const u8 address = packet[6];
			const u8 length = std::min<u8>(packet[7], 8);
			for (int i = 0; i < length; i++)
			{
				if ((address + i) < std::size(m_eeprom))
				{
					m_eeprom[address + i] = packet[8 + i];
				}
			}
			std::copy_n(&packet[8], 8, m_verify);
			count();
			break;
		}

		case 0x98:  // confirm the last write with the complement of its data
			m_tx[0] = 0xb4;
			for (int i = 0; i < 8; i++)
			{
				m_tx[8 + i] = m_verify[i] ^ 0xff;
			}
			make_response(16);
			break;

		case 0xb5:  // read EEPROM
		{
			const u8 address = packet[6];
			const u8 length = std::min<u8>(packet[7], 8);
			m_tx[0] = 0xb6;
			for (int i = 0; i < length; i++)
			{
				m_tx[8 + i] = ((address + i) < std::size(m_eeprom)) ? m_eeprom[address + i] : 0xff;
			}
			make_response(16);
			break;
		}

		default:
			// the host sends a deliberately invalid command to resynchronise
			LOGMASKED(LOG_COMMAND, "resync command %02x\n", command);
			break;
	}
}
