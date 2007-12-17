#ifndef HEADER__M7700DS
#define HEADER__M7700DS

/*

Mitsubishi 7700 CPU Emulator v0.10
By R. Belmont

Based on:
G65C816 CPU Emulator V0.92

Copyright (c) 2000 Karl Stenerud
All rights reserved.

*/

int m7700_disassemble(char* buff, unsigned int pc, unsigned int pb, const UINT8 *oprom, int m_flag, int x_flag);

#endif /* HEADER__7700DS */
