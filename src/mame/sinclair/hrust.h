// license:BSD-3-Clause
// copyright-holders:HalfElf, Andrei I. Holub

#ifndef MAME_SINCLAIR_HRUST_H
#define MAME_SINCLAIR_HRUST_H

#pragma once

#include "emu.h"


class bb_stream
{
public:
	bb_stream(const u8 *src, int src_length);
	u8 get_byte();
	u8 get_bit();
	u8 get_bits(u8 n);
	bool overflow();

private:
	const u8 *m_base;
	const u8 *m_read;
	int m_offset;
	int m_length;
	bool m_eof;
	u16 m_buffer;

};


class hrust_decoder
{
public:
	hrust_decoder();

	void decode(address_space &space, const u8 *source, u16 dest, u16 size);
};

#endif // MAME_SINCLAIR_HRUST_H
