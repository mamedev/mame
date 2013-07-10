
#include "emu.h"
#include "k054000.h"


#define VERBOSE 0
#define LOG(x) do { if (VERBOSE) logerror x; } while (0)

/***************************************************************************/
/*                                                                         */
/*                                 054000                                  */
/*                                                                         */
/***************************************************************************/

static UINT8 K054000_ram[0x20];

static WRITE8_HANDLER( K054000_w )
{
//logerror("%04x: write %02x to 054000 address %02x\n",space.device().safe_pc(),data,offset);

	K054000_ram[offset] = data;
}

static READ8_HANDLER( K054000_r )
{
	int Acx,Acy,Aax,Aay;
	int Bcx,Bcy,Bax,Bay;

//logerror("%04x: read 054000 address %02x\n",space.device().safe_pc(),offset);

	if (offset != 0x18) return 0;

	Acx = (K054000_ram[0x01] << 16) | (K054000_ram[0x02] << 8) | K054000_ram[0x03];
	Acy = (K054000_ram[0x09] << 16) | (K054000_ram[0x0a] << 8) | K054000_ram[0x0b];
/* TODO: this is a hack to make thndrx2 pass the startup check. It is certainly wrong. */
if (K054000_ram[0x04] == 0xff) Acx+=3;
if (K054000_ram[0x0c] == 0xff) Acy+=3;
	Aax = K054000_ram[0x06] + 1;
	Aay = K054000_ram[0x07] + 1;

	Bcx = (K054000_ram[0x15] << 16) | (K054000_ram[0x16] << 8) | K054000_ram[0x17];
	Bcy = (K054000_ram[0x11] << 16) | (K054000_ram[0x12] << 8) | K054000_ram[0x13];
	Bax = K054000_ram[0x0e] + 1;
	Bay = K054000_ram[0x0f] + 1;

	if (Acx + Aax < Bcx - Bax)
		return 1;

	if (Bcx + Bax < Acx - Aax)
		return 1;

	if (Acy + Aay < Bcy - Bay)
		return 1;

	if (Bcy + Bay < Acy - Aay)
		return 1;

	return 0;
}

READ16_HANDLER( K054000_lsb_r )
{
	return K054000_r(space, offset, mem_mask & 0xff);
}

WRITE16_HANDLER( K054000_lsb_w )
{
	if (ACCESSING_BITS_0_7)
		K054000_w(space, offset, data & 0xff, mem_mask & 0xff);
}




