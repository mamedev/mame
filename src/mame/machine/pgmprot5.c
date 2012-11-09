/***********************************************************************
 PGM 012 + 025 PGM protection emulation

 these are simulations of the IGS 012 and 025 protection combination
 used on the following PGM games

 Dragon World 2

 ----

 IGS012 provides ROM overlay???

 IGS025 is some kind of state machine / logic device which the game
 uses for various security checks bitswap checks.

 Simulation is incomplete for some regions

 ***********************************************************************/


#include "emu.h"
#include "includes/pgm.h"


/* Dragon World 2 */

#define DW2BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))
//Use this handler for reading from 0xd80000-0xd80002
READ16_HANDLER( dw2_d80000_r )
{
// addr&=0xff;
// if(dw2reg<0x20) //NOT SURE!!
	{
		//The value at 0x80EECE is computed in the routine at 0x107c18
		pgm_state *state = space.machine().driver_data<pgm_state>();
		UINT16 d = state->m_mainram[0xEECE/2];
		UINT16 d2 = 0;
		d = (d >> 8) | (d << 8);
		DW2BITSWAP(d, d2, 7,  0);
		DW2BITSWAP(d, d2, 4,  1);
		DW2BITSWAP(d, d2, 5,  2);
		DW2BITSWAP(d, d2, 2,  3);
		DW2BITSWAP(d, d2, 15, 4);
		DW2BITSWAP(d, d2, 1,  5);
		DW2BITSWAP(d, d2, 10, 6);
		DW2BITSWAP(d, d2, 13, 7);
		// ... missing bitswaps here (8-15) there is not enough data to know them
		// the code only checks the lowest 8 bytes
		return d2;
	}
}

static void pgm_dw2_decrypt(running_machine &machine)
{

	int i;
	UINT16 *src = (UINT16 *) (machine.root_device().memregion("maincpu")->base()+0x100000);

	int rom_size = 0x80000;

	for(i=0; i<rom_size/2; i++) {
		UINT16 x = src[i];

		if(((i & 0x020890) == 0x000000)
		   || ((i & 0x020000) == 0x020000 && (i & 0x001500) != 0x001400))
			x ^= 0x0002;

		if(((i & 0x020400) == 0x000000 && (i & 0x002010) != 0x002010)
		   || ((i & 0x020000) == 0x020000 && (i & 0x000148) != 0x000140))
			x ^= 0x0400;

		src[i] = x;
	}
}

static void drgwld2_common_init(running_machine &machine)
{
	pgm_basic_init(machine);
	pgm_dw2_decrypt(machine);
	/*
    Info from Elsemi
    Here is how to "bypass" the dw2 hang protection, it fixes the mode
    select and after failing in the 2nd stage (probably there are other checks
    out there).
    */
	machine.device("maincpu")->memory().space(AS_PROGRAM).install_legacy_read_handler(0xd80000, 0xd80003, FUNC(dw2_d80000_r));
}

DRIVER_INIT_MEMBER(pgm_state,drgw2)
{	/* incomplete? */
	UINT16 *mem16 = (UINT16 *)machine().root_device().memregion("maincpu")->base();
	drgwld2_common_init(machine());
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x131098 / 2] = 0x4e93;
	mem16[0x13113e / 2] = 0x4e93;
	mem16[0x1311ce / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,dw2v100x)
{
	UINT16 *mem16 = (UINT16 *)machine().root_device().memregion("maincpu")->base();
	drgwld2_common_init(machine());

	mem16[0x131084 / 2] = 0x4e93;
	mem16[(0x131084+0xa6) / 2] = 0x4e93;
	mem16[(0x131084+0x136) / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,drgw2c)
{
	UINT16 *mem16 = (UINT16 *)machine().root_device().memregion("maincpu")->base();
	drgwld2_common_init(machine());
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1303bc / 2] = 0x4e93;
	mem16[0x130462 / 2] = 0x4e93;
	mem16[0x1304f2 / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,drgw2j)
{
	UINT16 *mem16 = (UINT16 *)machine().root_device().memregion("maincpu")->base();
	drgwld2_common_init(machine());
	/* These ROM patches are not hacks, the protection device
       overlays the normal ROM code, this has been confirmed on a real PCB
       although some addresses may be missing */
	mem16[0x1302c0 / 2] = 0x4e93;
	mem16[0x130366 / 2] = 0x4e93;
	mem16[0x1303f6 / 2] = 0x4e93;
}

