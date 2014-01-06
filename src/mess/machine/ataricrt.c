/***************************************************************************

    MESS specific Atari init and Cartridge code for Atari 8 bit systems

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "includes/atari.h"
#include "machine/ram.h"
#include "hashfile.h"

#define LEFT_CARTSLOT_MOUNTED  1
#define RIGHT_CARTSLOT_MOUNTED 2

/* PCB */
enum
{
	A800_UNKNOWN = 0,
	A800_4K, A800_8K, A800_12K, A800_16K,
	A800_RIGHT_4K, A800_RIGHT_8K,
	OSS_034M, OSS_M091, PHOENIX_8K, XEGS_32K,
	BBSB, DIAMOND_64K, WILLIAMS_64K, EXPRESS_64,
	SPARTADOS_X
};


#if 0
static int bbsb_bankl = 0;
static int bbsb_bankh = 0;

static WRITE8_HANDLER( bbsb_bankl_w )
{
	bbsb_bankl = offset; // 0,1,2,3
}

static WRITE8_HANDLER( bbsb_bankh_w )
{
	bbsb_bankh = offset; // 4,5,6,7
}

static READ8_HANDLER( bbsb_bankl_r )
{
	// return data from the selected bank (0,1,2,3)
	UINT8 *mem = space.machine().root_device().memregion("lslot")->base();
	return &mem[0x0000 + bbsb_bankl * 0x1000];
}

static READ8_HANDLER( bbsb_bankh_r )
{
	// return data from the selected bank (4,5,6,7)
	UINT8 *mem = space.machine().root_device().memregion("lslot")->base();
	return &mem[0x4000 + bbsb_bankh * 0x1000];
}
#endif
