/***************************************************************************

  The Konami_1 CPU is a 6809 with opcodes scrambled. Here is how to
  descramble them.

***************************************************************************/

#include "driver.h"


UINT8 *konami1_decrypted;

UINT8 konami1_decodebyte( UINT8 opcode, UINT16 address )
{
/*
>
> CPU_D7 = (EPROM_D7 & ~ADDRESS_1) | (~EPROM_D7 & ADDRESS_1)  >
> CPU_D6 = EPROM_D6
>
> CPU_D5 = (EPROM_D5 & ADDRESS_1) | (~EPROM_D5 & ~ADDRESS_1) >
> CPU_D4 = EPROM_D4
>
> CPU_D3 = (EPROM_D3 & ~ADDRESS_3) | (~EPROM_D3 & ADDRESS_3) >
> CPU_D2 = EPROM_D2
>
> CPU_D1 = (EPROM_D1 & ADDRESS_3) | (~EPROM_D1 & ~ADDRESS_3) >
> CPU_D0 = EPROM_D0
>
*/
	UINT8 xormask;


	xormask = 0;
	if (address & 0x02) xormask |= 0x80;
	else xormask |= 0x20;
	if (address & 0x08) xormask |= 0x08;
	else xormask |= 0x02;

	return opcode ^ xormask;
}



static void decode(int cpu)
{
	UINT8 *rom = memory_region(REGION_CPU1+cpu);
	int size = memory_region_length(REGION_CPU1+cpu);
	int A;

	konami1_decrypted = auto_malloc(size);
	memory_set_decrypted_region(cpu, 0x0000, 0xffff, konami1_decrypted);

	for (A = 0;A < size;A++)
	{
		konami1_decrypted[A] = konami1_decodebyte(rom[A],A);
	}
}

void konami1_decode(void)
{
	decode(0);
}

void konami1_decode_cpu2(void)
{
	decode(1);
}
