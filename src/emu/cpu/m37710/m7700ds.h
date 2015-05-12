// license:BSD-3-Clause
// copyright-holders:R. Belmont, Karl Stenerud
#pragma once

#ifndef __M7700DS_H__
#define __M7700DS_H__

/*

Mitsubishi 7700 CPU Emulator v0.10
By R. Belmont

Based on:
G65C816 CPU Emulator V0.92

Copyright Karl Stenerud
All rights reserved.

*/

int m7700_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag);

#endif /* __M7700DS_H__ */
