/***************************************************************************

    440ops.c

    National Semiconductor COP440 Emulator.

    Copyright Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

***************************************************************************/

#include "420ops.c"

/*

    Mnemonic:           CAMT

    Hex Code:           33 3F
    Binary:

    Data Flow:          A -> T7:4
                        RAM(B) -> T3:0

    Description:        Copy A, RAM to T

*/

INSTRUCTION( camt )
{
	T = (A << 4) | RAM_R(B);
}

/*

    Mnemonic:           CTMA

    Hex Code:           33 2F
    Binary:

    Data Flow:          T7:4 -> RAM(B)
                        T3:0 -> A

    Description:        Copy T to RAM, A

*/

INSTRUCTION( ctma )
{
	RAM_W(B, T >> 4);
	A = T & 0x0f;
}

/*

    Mnemonic:           IT

    Hex Code:           33 39
    Binary:             0 0 1 1 0 0 1 1 0 0 1 1 1 0 0 1

    Description:        IDLE till Timer Overflows then Continues

*/

INSTRUCTION( it )
{
	cop400->halt = 1;
	cop400->idle = 1;
}
