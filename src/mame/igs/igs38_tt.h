// license:BSD-3-Clause
// copyright-holders:Peter Wilhelmsen
// IGS38 SD TTable transformation.

#ifndef MAME_IGS_IGS38_TT_H
#define MAME_IGS_IGS38_TT_H

#pragma once
#include <cstdint>

inline std::uint8_t igs38_sd_tt_decrypt(
		std::uint32_t address, std::uint8_t data, unsigned table_mode, unsigned key_mode, std::uint8_t table, std::uint8_t key)
{
	auto a = [address](unsigned n)
	{
		return (address >> n) & 1U;
	};
	auto t = [table](unsigned n)
	{
		return (table >> n) & 1U;
	};
	auto k = [key](unsigned n)
	{
		return (key >> n) & 1U;
	};
	unsigned gt = 0, gk = 0;
	switch (table_mode & 3)
	{
	case 0:
		gt |= (((t(0) & (a(8))) | (~a(0))) & 1U) << 0;
		gt |= (((t(2) | (~a(9))) & (~a(1))) & 1U) << 1;
		gt |= (((t(4) & (~a(10))) | (~a(2))) & 1U) << 2;
		gt |= (((t(6) & (a(11))) & (~a(3))) & 1U) << 3;
		gt |= (((t(7) | (a(12))) & (a(4))) & 1U) << 4;
		gt |= (((t(5) & (~a(13))) | (a(5))) & 1U) << 5;
		gt |= (((t(3) | (~a(14))) & (~a(6))) & 1U) << 6;
		gt |= (((t(1) | (~a(15))) | (~a(7))) & 1U) << 7;
		break;
	case 1:
		gt |= (((t(0) | (~a(0))) | (a(15))) & 1U) << 0;
		gt |= (((t(2) & (a(1))) & (~a(14))) & 1U) << 1;
		gt |= (((t(4) & (a(2))) | (a(13))) & 1U) << 2;
		gt |= (((t(6) | (a(3))) | (~a(12))) & 1U) << 3;
		gt |= (((t(1) | (~a(4))) & (a(11))) & 1U) << 4;
		gt |= (((t(3) & (~a(5))) | (~a(10))) & 1U) << 5;
		gt |= (((t(5) | (~a(6))) | (a(9))) & 1U) << 6;
		gt |= (((t(7) | (~a(7))) & (~a(8))) & 1U) << 7;
		break;
	case 2:
		gt |= (((t(0) & (~a(0))) & (a(15))) & 1U) << 0;
		gt |= (((t(1) | (~a(2))) | (~a(13))) & 1U) << 1;
		gt |= (((t(2) & (a(4))) & (a(11))) & 1U) << 2;
		gt |= (((t(3) | (~a(6))) | (~a(9))) & 1U) << 3;
		gt |= (((t(4) & (a(8))) & (a(7))) & 1U) << 4;
		gt |= (((t(5) | (a(10))) | (~a(5))) & 1U) << 5;
		gt |= (((t(6) & (~a(12))) & (a(3))) & 1U) << 6;
		gt |= (((t(7) | (a(14))) | (~a(1))) & 1U) << 7;
		break;
	case 3:
		gt |= (((t(0) & (a(14))) | (a(1))) & 1U) << 0;
		gt |= (((t(1) | (~a(12))) | (~a(3))) & 1U) << 1;
		gt |= (((t(2) & (a(10))) & (~a(5))) & 1U) << 2;
		gt |= (((t(3) | (a(8))) & (a(7))) & 1U) << 3;
		gt |= (((t(4) & (~a(6))) | (~a(9))) & 1U) << 4;
		gt |= (((t(5) | (a(4))) | (a(11))) & 1U) << 5;
		gt |= (((t(6) & (~a(2))) & (~a(13))) & 1U) << 6;
		gt |= (((t(7) | (a(0))) | (a(15))) & 1U) << 7;
		break;
	}
	switch (key_mode & 3)
	{
	case 0:
		gk |= (((k(7) & (a(0))) & (~a(21))) & 1U) << 0;
		gk |= (((k(5) & (~a(17))) | (a(4))) & 1U) << 1;
		gk |= (((k(3) | (~a(2))) & (~a(23))) & 1U) << 2;
		gk |= (((k(1) & (a(19))) | (~a(6))) & 1U) << 3;
		gk |= (((k(0) | (~a(20))) & (a(7))) & 1U) << 4;
		gk |= (((k(2) & (~a(3))) | (~a(16))) & 1U) << 5;
		gk |= (((k(4) | (a(18))) & (a(5))) & 1U) << 6;
		gk |= (((k(6) & (~a(1))) | (~a(22))) & 1U) << 7;
		break;
	case 1:
		gk |= (((k(0) | (a(6))) & (a(3))) & 1U) << 0;
		gk |= (((k(2) & (~a(16))) | (~a(7))) & 1U) << 1;
		gk |= (((k(4) | (~a(2))) | (a(4))) & 1U) << 2;
		gk |= (((k(6) & (a(17))) | (a(1))) & 1U) << 3;
		gk |= (((k(1) | (~a(5))) & (a(0))) & 1U) << 4;
		gk |= (((k(3) & (~a(21))) | (~a(20))) & 1U) << 5;
		gk |= (((k(5) | (~a(19))) & (~a(22))) & 1U) << 6;
		gk |= (((k(7) | (~a(23))) | (~a(18))) & 1U) << 7;
		break;
	case 2:
		gk |= (((k(0) | (a(21))) & (a(17))) & 1U) << 0;
		gk |= (((k(1) & (~a(16))) | (~a(22))) & 1U) << 1;
		gk |= (((k(2) | (~a(3))) & (a(7))) & 1U) << 2;
		gk |= (((k(3) | (a(6))) | (~a(4))) & 1U) << 3;
		gk |= (((k(4) & (~a(5))) & (~a(23))) & 1U) << 4;
		gk |= (((k(5) & (a(18))) | (a(2))) & 1U) << 5;
		gk |= (((k(6) | (~a(1))) & (~a(19))) & 1U) << 6;
		gk |= (((k(7) | (~a(20))) | (a(0))) & 1U) << 7;
		break;
	case 3:
		gk |= (((k(0) | (a(16))) & (a(0))) & 1U) << 0;
		gk |= (((k(1) & (~a(20))) & (~a(22))) & 1U) << 1;
		gk |= (((k(2) | (a(23))) & (~a(17))) & 1U) << 2;
		gk |= (((k(3) & (a(3))) | (a(21))) & 1U) << 3;
		gk |= (((k(4) | (~a(1))) & (a(18))) & 1U) << 4;
		gk |= (((k(5) | (a(4))) & (~a(7))) & 1U) << 5;
		gk |= (((k(6) & (~a(19))) | (~a(6))) & 1U) << 6;
		gk |= (((k(7) | (a(2))) | (a(5))) & 1U) << 7;
		break;
	}
	unsigned rot = (gt & 1) | ((gt >> 1) & 2) | ((gt >> 2) & 4);
	unsigned value = data ^ gk;
	value = (gt & 0x40) ? ((value >> rot) | (value << (8 - rot))) : ((value << rot) | (value >> (8 - rot)));
	unsigned permuted = ((value & 1) << 7) | ((value & 0x80) >> 1) | ((value & 2) << 4) | ((value & 0x40) >> 2) | ((value & 4) << 1) |
			((value & 0x20) >> 3) | ((value & 8) >> 2) | ((value & 0x10) >> 4);
	return std::uint8_t(permuted ^ gt);
}
#endif
