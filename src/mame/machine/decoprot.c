/****************************************************************************

    Data East protection/IO chips


    Game                        Custom chip number
    ------------------------------------------------
    Edward Randy                    60
    Mutant Fighter                  66
    Captain America                 75
    Robocop 2                       75
    Lemmings                        75
    Caveman Ninja                   104
    Wizard Fire                     104
    Pocket Gal DX                   104
    Boogie Wings                    104
    Rohga                           104
    Diet GoGo                       104
    Funky Jet                       146
    Nitro Ball                      146
    Super Shanghai Dragon's Eye     146
    Dragon Gun                      146
    Fighter's History               146 (ID scratched off)
    Stadium Hero 96                 146
    Tattoo Assassins                ?

    This series of chips is used for I/O and copy protection.  They are all
    16 bit word based chips, with 0x400 write addresses, and 0x400 read
    addresses.  The basic idea of the protection is that read & write
    addresses don't match.  So if you write to write address 0, you might
    read that data back at read address 10, and something else will be at 0.
    In addition, the data read back may be bit shifted in various ways.
    Games can be very well protected by writing variables to the chip, and
    expecting certain values back from certain read addresses.  With care,
    it can be impossible to tell from the game code what values go where,
    and what shifting goes on.  Eg, writing 10 values to the chip, of which
    7 are dummy values, then reading back 3 particular values, and using them
    in a multiplication and add calculation to get a jump address for the
    program.  Even if you can guess one of many possible legal jump addresses
    it's hard to tell what values should be bit shifted in what way.

    It's also been found some chips contain a hardwired XOR port and hardwired
    NAND port which affects only certain read values.

    The chips also handle the interface to the sound cpu via a write address,
    and game inputs via several read addresses.  The I/O input data is also
    mirrored to several locations, some with bit shifting.

    Although several games use chip 104, each seems to be different, the
    address lines leading to the chip on each game are probably arranged
    differently, to further scramble reads/writes.  From hardware tests
    chips 60 && 66 appear to be identical.

    Update January - April 2006:
        Further work on examining the 146 chip has revealed that if you
        read an address immediately after writing it, you always get
        the written value returned.  This behaviour is confirmed
        to only exist for one read/write 'tick' - any other read will
        return that location to its usual state - ie, bit shifter
        or input port.  This has been emulated for the 146 chip in
        Nitroball and Fighters History but the behaviour probably
        also exists in earlier chips as it explains the 'one-shot'
        ports in Mutant Fighter.

        The 'double buffer' feature seen in the 104 chip is also
        confirmed to exist in the 146 chip.  Again, this may well
        be present in the earlier chip too.

    Emulation by Bryan McPhail, mish@tendril.co.uk

***************************************************************************/

#include "driver.h"
#include "machine/eeprom.h"
#include "decoprot.h"

#define DECO_PORT(p) (prot_ram[p/2])

static UINT8 decoprot_buffer_ram_selected=0;
static UINT16 deco16_xor=0;
static UINT16 deco16_mask=0xffff;
static int decoprot_last_write=0, decoprot_last_write_val=0;
static UINT16 decoprot_buffer_ram[0x800];
static UINT16 decoprot_buffer_ram2[0x800];

UINT16 *deco16_prot_ram;
UINT32 *deco32_prot_ram;

/***************************************************************************/

void decoprot_reset(void)
{
	deco16_xor=0;
	deco16_mask=0xffff;
	decoprot_last_write=decoprot_last_write_val=0;
	decoprot_buffer_ram_selected=0;

	state_save_register_global(deco16_xor);
	state_save_register_global(deco16_mask);
	state_save_register_global(decoprot_last_write);
	state_save_register_global(decoprot_last_write_val);
	state_save_register_global(decoprot_buffer_ram_selected);
	state_save_register_global_array(decoprot_buffer_ram);
	state_save_register_global_array(decoprot_buffer_ram2);
}

/***************************************************************************/

WRITE16_HANDLER( deco16_104_prot_w ) /* Wizard Fire */
{
	if (offset==(0x150/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}

	if (offset!=(0x150>>1) && offset!=(0x0>>1) && offset!=(0x110>>1) && offset!=(0x280>>1)
		&& offset!=(0x290>>1) && offset!=(0x2b0>>1) && offset!=(0x370>>1) && offset!=(0x3c0>>1)
		&& offset!=(0x370>>1) && offset!=(0x3c0>>1) && offset!=(0x430>>1) && offset!=(0x460>>1)
		&& offset!=(0x5a0>>1) && offset!=(0x5b0>>1) && offset!=(0x6e0>>1) && offset!=(0x7d0>>1)
		)
		logerror("CONTROL PC %06x: warning - write protection memory address %04x %04x\n",activecpu_get_pc(),offset<<1,data);

	COMBINE_DATA(&deco16_prot_ram[offset]);
}

READ16_HANDLER( deco16_104_prot_r ) /* Wizard Fire */
{
	switch (offset<<1) {
		case 0x110: /* Player input */
			return readinputport(0);

		case 0x36c: /* Coins */
		case 0x334: /* Probably also, c6, 2c0, 2e0, 4b2, 46a, 4da, rohga is 44c */
			return readinputport(1);
		case 0x0dc:
			return readinputport(1)<<4;

		case 0x494: /* Dips */
			return readinputport(2);

		case 0x244:
			return deco16_prot_ram[0];
		case 0x7cc:
			return ((deco16_prot_ram[0]&0x000f)<<12) | ((deco16_prot_ram[0]&0x00f0)<<4) | ((deco16_prot_ram[0]&0x0f00)>>4) | ((deco16_prot_ram[0]&0xf000)>>12);
		case 0x0c0:
			return (((deco16_prot_ram[0]&0x000e)>>1) | ((deco16_prot_ram[0]&0x0001)<<3))<<12;
		case 0x188:
			return (((deco16_prot_ram[0]&0x000e)>>1) | ((deco16_prot_ram[0]&0x0001)<<3))<<12;
		case 0x65e:
			return (((deco16_prot_ram[0]&0x000c)>>2) | ((deco16_prot_ram[0]&0x0003)<<2))<<12;
		case 0x5ce:
			return ((deco16_prot_ram[0]<<8)&0xf000) | ((deco16_prot_ram[0]&0xe)<<7) | ((deco16_prot_ram[0]&0x1)<<11);
		case 0x61a:
			return (deco16_prot_ram[0]<<8)&0xff00;

		case 0x496:
			return deco16_prot_ram[0x110/2];
		case 0x40a:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<12) | ((deco16_prot_ram[0x110/2]&0x00f0)>>4) | ((deco16_prot_ram[0x110/2]&0x0f00)<<0) | ((deco16_prot_ram[0x110/2]&0xf000)>>8);
		case 0x1e8:
			return ((deco16_prot_ram[0x110/2]&0x00ff)<<8) | ((deco16_prot_ram[0x110/2]&0xff00)>>8);
		case 0x4bc:
			return ((deco16_prot_ram[0x110/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x110/2]&0x0003)<<6) | ((deco16_prot_ram[0x110/2]&0x000c)<<2);
		case 0x46e:
			return ((deco16_prot_ram[0x110/2]&0xfff0)<<0) | ((deco16_prot_ram[0x110/2]&0x0007)<<1) | ((deco16_prot_ram[0x110/2]&0x0008)>>3);
		case 0x264:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<8) | ((deco16_prot_ram[0x110/2]&0x00f0)>>0) | ((deco16_prot_ram[0x110/2]&0x0f00)<<4);
		case 0x172:
			return ((deco16_prot_ram[0x110/2]&0x000f)<<4) | ((deco16_prot_ram[0x110/2]&0x00f0)<<4) | ((deco16_prot_ram[0x110/2]&0xf000)<<0);

		case 0x214:
			return deco16_prot_ram[0x280/2];
		case 0x52e:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x07a:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x360:
			return ((deco16_prot_ram[0x280/2]&0x000f)<<8) | ((deco16_prot_ram[0x280/2]&0x00f0)>>0) | ((deco16_prot_ram[0x280/2]&0x0f00)>>8) | ((deco16_prot_ram[0x280/2]&0xf000)>>0);
		case 0x4dc:
			return ((deco16_prot_ram[0x280/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x280/2]&0x0007)<<5) | ((deco16_prot_ram[0x280/2]&0x0008)<<1);
		case 0x3a8:
			return ((deco16_prot_ram[0x280/2]&0x000e)<<3) | ((deco16_prot_ram[0x280/2]&0x0001)<<7) | ((deco16_prot_ram[0x280/2]&0x0ff0)<<4) | ((deco16_prot_ram[0x280/2]&0xf000)>>12);
		case 0x2f6:
			return ((deco16_prot_ram[0x280/2]&0xff00)>>8) | ((deco16_prot_ram[0x280/2]&0x00f0)<<8) | ((deco16_prot_ram[0x280/2]&0x000c)<<6) | ((deco16_prot_ram[0x280/2]&0x0003)<<10);

		case 0x7e4:
			return (deco16_prot_ram[0x290/2]&0x00f0)<<8;

		case 0x536:
			return ((deco16_prot_ram[0x2b0/2]&0x000f)<<8) | ((deco16_prot_ram[0x2b0/2]&0x00f0)<<0) | ((deco16_prot_ram[0x2b0/2]&0x0f00)<<4) | ((deco16_prot_ram[0x2b0/2]&0xf000)>>12);

		case 0x0be:
			return ((deco16_prot_ram[0x370/2]&0x000f)<<4) | ((deco16_prot_ram[0x370/2]&0x00f0)<<4) | ((deco16_prot_ram[0x370/2]&0x0f00)>>8) | ((deco16_prot_ram[0x370/2]&0xf000)>>0);

		case 0x490:
			return (deco16_prot_ram[0x3c0/2]&0xfff0) | ((deco16_prot_ram[0x3c0/2]&0x0007)<<1) | ((deco16_prot_ram[0x3c0/2]&0x0008)>>3);

		case 0x710:
			return (deco16_prot_ram[0x430/2]&0xfff0) | ((deco16_prot_ram[0x430/2]&0x0007)<<1) | ((deco16_prot_ram[0x430/2]&0x0008)>>3);

		case 0x22a:
			return ((deco16_prot_ram[0x5a0/2]&0xff00)>>8) | ((deco16_prot_ram[0x5a0/2]&0x00f0)<<8) | ((deco16_prot_ram[0x5a0/2]&0x0001)<<11) | ((deco16_prot_ram[0x5a0/2]&0x000e)<<7);

		case 0x626:
			return ((deco16_prot_ram[0x5b0/2]&0x000f)<<8) | ((deco16_prot_ram[0x5b0/2]&0x00f0)<<8) | ((deco16_prot_ram[0x5b0/2]&0x0f00)>>4) | ((deco16_prot_ram[0x5b0/2]&0xf000)>>12);

		case 0x444:
			return deco16_prot_ram[0x604/2]; //rohga

		case 0x5ac:
			return ((deco16_prot_ram[0x6e0/2]&0xfff0)>>4) | ((deco16_prot_ram[0x6e0/2]&0x0007)<<13) | ((deco16_prot_ram[0x6e0/2]&0x0008)<<9);

		case 0x650:
			return ((deco16_prot_ram[0x7d0/2]&0xfff0)>>4) | ((deco16_prot_ram[0x7d0/2]&0x000f)<<12);

		case 0x4ac:
			return ((deco16_prot_ram[0x460/2]&0x0007)<<13) | ((deco16_prot_ram[0x460/2]&0x0008)<<9);
	}

	logerror("Deco Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset<<1);
	return 0;
}

/***************************************************************************/

WRITE16_HANDLER( deco16_60_prot_w ) /* Edward Randy */
{
	if (offset==(0x64/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
	}

	COMBINE_DATA(&deco16_prot_ram[offset]);

// 0 4 2c 32 34 36 3c 3e 40 54 56 58 6a 76 80 84 88 8a 8c 8e 90 92 94 96 9e a0 a2 a4 a6 a8 aa ac ae b0

if (offset!=0x32 && offset!=0x36/2 && offset!=0x9e/2 && offset!=0x76/2
	&& offset!=4/2 && offset!=0x2c/2 && offset!=0x3c/2 && offset!=0x3e/2
	&& offset!=0x80/2 && offset!=0x84/2 &&offset!=0x88/2 && offset!=0x8c/2 && offset!=0x90/2 && offset!=0x94/2
	&& offset!=0xa0/2 &&offset!=0xa2/2 && offset!=0xa4/2 && offset!=0xa6/2 && offset!=0xa8/2
	&& offset!=0xaa/2 && offset!=0xac/2 && offset!=0xae/2 && offset!=0xb0/2
	&& (offset<0xd0/2 || offset>0xe0/2)
	&& (offset<4 || offset>17)
	&& offset!=0x40/2 && offset!=0x54/2 && offset!=0x56/2 && offset!=0x58/2 && offset!=0x6a/2 && offset!=0x2c/2
	&& offset!=0 && offset!=0x34 && offset!=0x8a && offset!=0x8e && offset!=0x92 && offset!=0x96
	)
logerror("Protection PC %06x: warning - write %04x to %04x\n",activecpu_get_pc(),data,offset<<1);

}

READ16_HANDLER( deco16_60_prot_r ) /* Edward Randy */
{
	switch (offset<<1) {
		/* Video registers */
		case 0x32a: /* Moved to 0x140006 on int */
			return deco16_prot_ram[0x80/2];
		case 0x380: /* Moved to 0x140008 on int */
			return deco16_prot_ram[0x84/2];
		case 0x63a: /* Moved to 0x150002 on int */
			return deco16_prot_ram[0x88/2];
		case 0x42a: /* Moved to 0x150004 on int */
			return deco16_prot_ram[0x8c/2];
		case 0x030: /* Moved to 0x150006 on int */
			return deco16_prot_ram[0x90/2];
		case 0x6b2: /* Moved to 0x150008 on int */
			return deco16_prot_ram[0x94/2];

		case 0x6fa:
			return deco16_prot_ram[0x4/2];
		case 0xe4:
			return (deco16_prot_ram[0x4/2]&0xf000)|((deco16_prot_ram[0x4/2]&0x00ff)<<4)|((deco16_prot_ram[0x4/2]&0x0f00)>>8);

		case 0x390:
			return deco16_prot_ram[0x2c/2];
		case 0x3b2:
			return deco16_prot_ram[0x3c/2];
		case 0x440:
			return deco16_prot_ram[0x3e/2];

		case 0x6fc:
			return deco16_prot_ram[0x66/2];

		case 0x15a:
			return deco16_prot_ram[0xa0/2];
		case 0x102:
			return deco16_prot_ram[0xa2/2];
		case 0x566:
			return deco16_prot_ram[0xa4/2];
		case 0xd2:
			return deco16_prot_ram[0xa6/2];
		case 0x4a6:
			return deco16_prot_ram[0xa8/2];
		case 0x3dc:
			return deco16_prot_ram[0xaa/2];
		case 0x2a0:
			return deco16_prot_ram[0xac/2];
		case 0x392:
			return deco16_prot_ram[0xae/2];
		case 0x444:
			return deco16_prot_ram[0xb0/2];

		case 0x5ea:
			return deco16_prot_ram[0xb8/2];
		case 0x358:
			return deco16_prot_ram[0xba/2];
		case 0x342:
			return deco16_prot_ram[0xbc/2];
		case 0x3c:
			return deco16_prot_ram[0xbe/2];
		case 0x656:
			return deco16_prot_ram[0xc0/2];
		case 0x18c:
			return deco16_prot_ram[0xc2/2];
		case 0x370:
			return deco16_prot_ram[0xc4/2];
		case 0x5c6:
			return deco16_prot_ram[0xc6/2];

			/* C8 written but not read */

		case 0x248:
			return deco16_prot_ram[0xd0/2];
		case 0x1ea:
			return deco16_prot_ram[0xd2/2];
		case 0x4cc:
			return deco16_prot_ram[0xd4/2];
		case 0x724:
			return deco16_prot_ram[0xd6/2];
		case 0x578:
			return deco16_prot_ram[0xd8/2];
		case 0x63e:
			return deco16_prot_ram[0xda/2];
		case 0x4ba:
			return deco16_prot_ram[0xdc/2];
		case 0x1a:
			return deco16_prot_ram[0xde/2];
		case 0x120:
			return deco16_prot_ram[0xe0/2];
		case 0x7c2: /* (Not checked for mask/xor but seems standard) */
			return deco16_prot_ram[0x50/2];

		/* memcpy selectors, transfer occurs in interrupt */
		case 0x32e: return deco16_prot_ram[4]; /* src msb */
		case 0x6d8: return deco16_prot_ram[5]; /* src lsb */
		case 0x010: return deco16_prot_ram[6]; /* dst msb */
		case 0x07a: return deco16_prot_ram[7]; /* src lsb */

		case 0x37c: return deco16_prot_ram[8]; /* src msb */
		case 0x250: return deco16_prot_ram[9];
		case 0x04e: return deco16_prot_ram[10];
		case 0x5ba: return deco16_prot_ram[11];
		case 0x5f4: return deco16_prot_ram[12]; /* length */

		case 0x38c: return deco16_prot_ram[13]; /* src msb */
		case 0x02c: return deco16_prot_ram[14];
		case 0x1e6: return deco16_prot_ram[15];
		case 0x3e4: return deco16_prot_ram[16];
		case 0x174: return deco16_prot_ram[17]; /* length */

		/* Player 1 & 2 controls, read in IRQ then written *back* to protection device */
		case 0x50: /* written to 9e byte */
			return readinputport(0);
		case 0x6f8: /* written to 76 byte */
			return (readinputport(0)>>8)|(readinputport(0)<<8); /* byte swap IN0 */

		case 0x5c: /* After coin insert, high 0x8000 bit set starts game */
			return deco16_prot_ram[0x3b];
		case 0x3a6: /* Top byte OR'd with above, masked to 7 */
			return deco16_prot_ram[0x9e/2];
		case 0xc6:
			return ((deco16_prot_ram[0x9e/2]&0xff00)>>8) | ((deco16_prot_ram[0x9e/2]&0x00ff)<<8);

		case 0xac: /* Dip switches */
			return readinputport(2);
		case 0xc2:
			return readinputport(2) ^ deco16_prot_ram[0x2c/2];

		case 0x5d4: /* The state of the dips last frame */
			return deco16_prot_ram[0x34/2];

		case 0x7bc:
			return ((deco16_prot_ram[0x76/2]&0xff00)>>8) | ((deco16_prot_ram[0x76/2]&0x00ff)<<8);

		case 0x2f6: /* Stage clear flag */
			return (((deco16_prot_ram[0]&0xfff0)>>0) | ((deco16_prot_ram[0]&0x000c)>>2) | ((deco16_prot_ram[0]&0x0003)<<2)) & (~deco16_prot_ram[0x36/2]);

		case 0x76a: /* Coins */
			return readinputport(1);

		case 0x284: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x40/2]&0xfff0)>>0) | ((deco16_prot_ram[0x40/2]&0x0007)<<1) | ((deco16_prot_ram[0x40/2]&0x0008)>>3)) & (~deco16_prot_ram[0x36/2]);
		case 0x6c4: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x54/2]&0xf000)>>4) | ((deco16_prot_ram[0x54/2]&0x0f00)>>4) | ((deco16_prot_ram[0x54/2]&0x00f0)>>4) | ((deco16_prot_ram[0x54/2]&0x0003)<<14) | ((deco16_prot_ram[0x54/2]&0x000c)<<10)) & (~deco16_prot_ram[0x36/2]);
		case 0x33e: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x56/2]&0xff00)>>0) | ((deco16_prot_ram[0x56/2]&0x00f0)>>4) | ((deco16_prot_ram[0x56/2]&0x000f)<<4)) & (~deco16_prot_ram[0x36/2]);
		case 0x156: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x58/2]&0xfff0)>>4) | ((deco16_prot_ram[0x58/2]&0x000e)<<11) | ((deco16_prot_ram[0x58/2]&0x0001)<<15)) & (~deco16_prot_ram[0x36/2]);
		case 0x286: /* Bit shifting with inverted mask register */
			return (((deco16_prot_ram[0x6a/2]&0x00f0)<<4) | ((deco16_prot_ram[0x6a/2]&0x0f00)<<4) | ((deco16_prot_ram[0x6a/2]&0x0007)<<5) | ((deco16_prot_ram[0x6a/2]&0x0008)<<1)) & (~deco16_prot_ram[0x36/2]);

		case 0x7d6: /* XOR IN0 */
			return readinputport(0) ^ deco16_prot_ram[0x2c/2];
		case 0x4b4:
			return ((deco16_prot_ram[0x32/2]&0x00f0)<<8) | ((deco16_prot_ram[0x32/2]&0x000e)<<7) | ((deco16_prot_ram[0x32/2]&0x0001)<<11);
	}

	logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset*2);
	return 0;
}

/***************************************************************************/

static int mutantf_port_0e_hack=0, mutantf_port_6a_hack=0,mutantf_port_e8_hack=0;

WRITE16_HANDLER( deco16_66_prot_w ) /* Mutant Fighter */
{
	if (offset==(0x64/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}

	COMBINE_DATA(&deco16_prot_ram[offset]);

	/* See below */
	if (offset==(0xe/2))
		mutantf_port_0e_hack=data;
	else
		mutantf_port_0e_hack=0x800;

	if (offset==(0x6a/2))
		mutantf_port_6a_hack=data;
	else
		mutantf_port_6a_hack=0x2866;

	if (offset==(0xe8/2))
		mutantf_port_e8_hack=data;
	else
		mutantf_port_e8_hack=0x2401;

//  2 4 c e 18 1e 22 2c 2e 34 36 38 3a 42 48 58 6a 72 7a 82 88 92 a2 a4 aa b0 b6 b8 dc e4 e8 f4 fa 1c8 308 7e8 40e
	offset=offset<<1;
	if (offset!=0x02 && offset!=0xc && offset!=0xe && offset!=0x18 && offset!=0x2c && offset!=0x2e && offset!=0x34
		&& offset!=0x42 && offset!=0x58 && offset!=0x6a && offset!=0x72 && offset!=0x7a && offset!=0xb8
		&& offset!=0xdc && offset!=0xe8 && offset!=0xf4 && offset!=0x1c8 && offset!=0x7e8
		&& offset!=0xe && offset!=0x48 && offset!=0xaa && offset!=0xb0 && offset!=0x36
		&& offset!=0xa4 && offset!=0x4 && offset!=0x82 && offset!=0x88 && offset!=0x22
		&& offset!=0xb6 && offset!=0xfa && offset!=0xe4 && offset!=0x3a && offset!=0x1e
		&& offset!=0x38 && offset!=0x92 && offset!=0xa2 && offset!=0x308 && offset!=0x40e
	)
	logerror("Protection PC %06x: warning - write %04x to %04x\n",activecpu_get_pc(),data,offset);
}

READ16_HANDLER( deco16_66_prot_r ) /* Mutant Fighter */
{
	if (offset!=0xe/2)
		mutantf_port_0e_hack=0x0800;
	if (offset!=0x6a/2)
		mutantf_port_6a_hack=0x2866;

 	switch (offset*2) {
		case 0xac: /* Dip switches */
			return readinputport(2);
		case 0xc2: /* Dip switches */
			return readinputport(2) ^ deco16_prot_ram[0x2c/2];
		case 0x46: /* Coins */
			return readinputport(1) ^ deco16_prot_ram[0x2c/2];
		case 0x50: /* Player 1 & 2 input ports */
			return readinputport(0);
		case 0x63c: /* Player 1 & 2 input ports */
			return readinputport(0) ^ deco16_prot_ram[0x2c/2];

		case 0x5f4:
			return deco16_prot_ram[0x18/2];
		case 0x7e8:
			return deco16_prot_ram[0x58/2];
		case 0x1c8:
			return deco16_prot_ram[0x6a/2];
		case 0x10:
			return deco16_prot_ram[0xc/2];
		case 0x672:
			return deco16_prot_ram[0x72/2];
		case 0x5ea:
			return deco16_prot_ram[0xb8/2];
		case 0x1e8:
			return deco16_prot_ram[0x2/2];
		case 0xf6:
			return deco16_prot_ram[0x42/2];
		case 0x692:
			return deco16_prot_ram[0x2e/2];
		case 0x63a:
			return deco16_prot_ram[0x88/2];
		case 0x7a:
			return deco16_prot_ram[0xe/2];
		case 0x40e:
			return deco16_prot_ram[0x7a/2];
		case 0x602:
			return deco16_prot_ram[0x92/2];
		case 0x5d4:
			return deco16_prot_ram[0x34/2];
		case 0x6fa:
			return deco16_prot_ram[0x4/2];
		case 0x3dc:
			return deco16_prot_ram[0xaa/2];
		case 0x444:
			return deco16_prot_ram[0xb0/2];
		case 0x102:
			return deco16_prot_ram[0xa2/2];
		case 0x458:
			return deco16_prot_ram[0xb6/2];
		case 0x2a6:
			return deco16_prot_ram[0xe8/2];
		case 0x626:
			return deco16_prot_ram[0xf4/2];
		case 0x762:
			return deco16_prot_ram[0x82/2];
		case 0x308:
			return deco16_prot_ram[0x38/2];
		case 0x1e6:
			return deco16_prot_ram[0x1e/2];
		case 0x566:
			return deco16_prot_ram[0xa4/2];
		case 0x5b6:
			return deco16_prot_ram[0xe4/2];
		case 0x77c:
			return deco16_prot_ram[0xfa/2];
		case 0x4ba:
			return deco16_prot_ram[0xdc/2];

		case 0x1e:
			return deco16_prot_ram[0xf4/2] ^ deco16_prot_ram[0x2c/2];
		case 0x18e:
			return ((deco16_prot_ram[0x1e/2]&0x000f)<<12) | ((deco16_prot_ram[0x1e/2]&0x0ff0)>>0) | ((deco16_prot_ram[0x1e/2]&0xf000)>>12);
		case 0x636:
			return ((deco16_prot_ram[0x18/2]&0x00ff)<<8) | ((deco16_prot_ram[0x18/2]&0x0f00)>>4) | ((deco16_prot_ram[0x18/2]&0xf000)>>12);
		case 0x7d4:
			return ((deco16_prot_ram[0xc/2]&0x0ff0)<<4) | ((deco16_prot_ram[0xc/2]&0x000c)<<2) | ((deco16_prot_ram[0xc/2]&0x0003)<<6);
		case 0x542:
			return ((deco16_prot_ram[0x92/2]&0x00ff)<<8) ^ deco16_prot_ram[0x2c/2];
		case 0xb0:
			return (((deco16_prot_ram[0xc/2]&0x000f)<<12) | ((deco16_prot_ram[0xc/2]&0x00f0)<<4) | ((deco16_prot_ram[0xc/2]&0xff00)>>8)) ^ deco16_prot_ram[0x2c/2];
		case 0x4:
			return (((deco16_prot_ram[0x18/2]&0x00f0)<<8) | ((deco16_prot_ram[0x18/2]&0x0003)<<10) | ((deco16_prot_ram[0x18/2]&0x000c)<<6)) & (~deco16_prot_ram[0x36/2]);

		case 0xe: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0800.  Hmm */
			{
				int ret=mutantf_port_0e_hack;
				mutantf_port_0e_hack=0x800;
				//logerror("Protection PC %06x: warning - read unknown memory address %04x\n",activecpu_get_pc(),offset<<1);
				return ret;
			}

		case 0x6a: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0x2866.  Hmm */
			{
				int ret=mutantf_port_6a_hack;
				mutantf_port_6a_hack=0x2866;
				//logerror("Protection PC %06x: warning - read unknown memory address %04x\n",activecpu_get_pc(),offset<<1);
				return ret;
			}

		case 0xe8: /* On real hardware this value only seems to persist for 1 read or write, then reverts to 0x2401.  Hmm */
			{
				int ret=mutantf_port_e8_hack;
				mutantf_port_e8_hack=0x2401;
				//logerror("Protection PC %06x: warning - read unknown memory address %04x\n",activecpu_get_pc(),offset<<1);
				return ret;
			}

		case 0xaa: /* ??? */
			//logerror("Protection PC %06x: warning - read unknown memory address %04x\n",activecpu_get_pc(),offset<<1);
			return 0xc080;

		case 0x42: /* Strange, but consistent */
			//logerror("Protection PC %06x: warning - read unknown memory address %04x\n",activecpu_get_pc(),offset<<1);
			return deco16_prot_ram[0x2c/2]^0x5302;

		case 0x48: /* Correct for test data, but I wonder if the 0x1800 is from an address, not a constant */
			//logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset<<1);
			return (0x1800) & (~deco16_prot_ram[0x36/2]);

		case 0x52:
			return (0x2188) & (~deco16_prot_ram[0x36/2]);

		case 0x82:
			return ((0x0022 ^ deco16_prot_ram[0x2c/2])) & (~deco16_prot_ram[0x36/2]);

		case 0xc:
			return 0x2000;
	}

#ifdef MAME_DEBUG
	popmessage("Deco66:  Read unmapped port %04x\n",offset*2);
#endif

	logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset<<1);
	return 0;
}

/***************************************************************************/

WRITE16_HANDLER( deco16_104_cninja_prot_w )
{
	if (offset==(0xa8/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}

	COMBINE_DATA(&deco16_prot_ram[offset]);
}

READ16_HANDLER( deco16_104_cninja_prot_r )
{
 	switch (offset<<1) {
		case 0x80: /* Master level control */
			return deco16_prot_ram[0];

		case 0xde: /* Restart position control */
			return deco16_prot_ram[1];

		case 0xe6: /* The number of credits in the system. */
			return deco16_prot_ram[2];

		case 0x86: /* End of game check.  See 0x1814 */
			return deco16_prot_ram[3];

		/* Video registers */
		case 0x5a: /* Moved to 0x140000 on int */
			return deco16_prot_ram[8];
		case 0x84: /* Moved to 0x14000a on int */
			return deco16_prot_ram[9];
		case 0x20: /* Moved to 0x14000c on int */
			return deco16_prot_ram[10];
		case 0x72: /* Moved to 0x14000e on int */
			return deco16_prot_ram[11];
		case 0xdc: /* Moved to 0x150000 on int */
			return deco16_prot_ram[12];
		case 0x6e: /* Moved to 0x15000a on int */
			return deco16_prot_ram[13]; /* Not used on bootleg */
		case 0x6c: /* Moved to 0x15000c on int */
			return deco16_prot_ram[14];
		case 0x08: /* Moved to 0x15000e on int */
			return deco16_prot_ram[15];

		case 0x36: /* Dip switches */
			return readinputport(2);

		case 0x1c8: /* Coins */
			return readinputport(1);

		case 0x22c: /* Player 1 & 2 input ports */
			return readinputport(0);
	}

	logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset);
	return 0;
}

/***************************************************************************/

WRITE16_HANDLER( deco16_146_funkyjet_prot_w )
{
	COMBINE_DATA(&deco16_prot_ram[offset]);

	if (offset == (0x10a >> 1)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}
}

READ16_HANDLER( deco16_146_funkyjet_prot_r )
{
	switch (offset)
	{
		case 0x0be >> 1:
			return deco16_prot_ram[0x106>>1];
		case 0x11e >> 1:
			return deco16_prot_ram[0x500>>1];

		case 0x148 >> 1: /* EOR mask for joysticks */
			return deco16_prot_ram[0x70e>>1];

		case 0x1da >> 1:
			return deco16_prot_ram[0x100>>1];
		case 0x21c >> 1:
			return deco16_prot_ram[0x504>>1];
		case 0x226 >> 1:
			return deco16_prot_ram[0x58c>>1];
		case 0x24c >> 1:
			return deco16_prot_ram[0x78e>>1];
		case 0x250 >> 1:
			return deco16_prot_ram[0x304>>1];
		case 0x2d4 >> 1:
			return deco16_prot_ram[0x102>>1];
		case 0x2d8 >> 1: /* EOR mask for credits */
			return deco16_prot_ram[0x502>>1];

		case 0x3a6 >> 1:
			return deco16_prot_ram[0x104>>1];
		case 0x3a8 >> 1: /* See 93e4/9376 */
			return deco16_prot_ram[0x500>>1];
		case 0x3e8 >> 1:
			return deco16_prot_ram[0x50c>>1];

		case 0x4e4 >> 1:
			return deco16_prot_ram[0x702>>1];
		case 0x562 >> 1:
			return deco16_prot_ram[0x18e>>1];
		case 0x56c >> 1:
			return deco16_prot_ram[0x50c>>1];

		case 0x688 >> 1:
			return deco16_prot_ram[0x300>>1];
		case 0x788 >> 1:
			return deco16_prot_ram[0x700>>1];

		case 0x7d4 >> 1: /* !? On the bootleg these address checks are NOP'd, so a BEQ is never taken */
			return 0x10; //deco16_prot_ram[0x7da>>1];

		case 0x27c >>1: /* From bootleg code at 0x400 */
			return ((deco16_prot_ram[0x70e>>1]>>4)&0x0fff) | ((deco16_prot_ram[0x70e>>1]&0x0001)<<15) | ((deco16_prot_ram[0x70e>>1]&0x000e)<<11);
		case 0x192 >>1: /* From bootleg code at 0x400 */
			return ((deco16_prot_ram[0x78e>>1]<<0)&0xf000);

		case 0x5be >> 1: /* Confirmed from bootleg code at 0xc07c */
			return ((deco16_prot_ram[0x70e>>1]<<4)&0xff00) | (deco16_prot_ram[0x70e>>1]&0x000f);
		case 0x5ca >> 1: /* Confirmed from bootleg code at 0xc05e */
			return ((deco16_prot_ram[0x78e>>1]>>4)&0xff00) | (deco16_prot_ram[0x78e>>1]&0x000f) | ((deco16_prot_ram[0x78e>>1]<<8)&0xf000);

		case 0x00c >> 1: /* Player 1 & Player 2 joysticks & fire buttons */
			return (readinputport(0) + (readinputport(1) << 8));
		case 0x778 >> 1: /* Credits */
			return readinputport(2);
		case 0x382 >> 1: /* DIPS */
			return (readinputport(3) + (readinputport(4) << 8));
	}

	if (activecpu_get_pc()!=0xc0ea)	logerror("CPU #0 PC %06x: warning - read unmapped control address %06x\n",activecpu_get_pc(),offset<<1);

	return 0;
}

/***************************************************************************/

WRITE16_HANDLER( deco16_104_rohga_prot_w )
{
	/*
        Schmeizr uses a feature of this chip that other games don't seem to exploit.  There
        appear to be two banks of data ports (referred to here as front and back buffers).
        These banks are switched when certain addresses are read - also very good protection
        against trojans as it is non-obvious when data ports are switched.

        It appears to work as follows..  When the front buffer is active (the state upon
        reset) any writes to data ports push the existing front buffer word into the back
        buffer (ie, a FIFO latch type of system).  When the back buffer is active, any writes
        to data ports affect the back buffer only - the front buffer data is not affected, and
        there is no FIFO action.

        By default the read ports return a scrambled address and scrambled data from a
        front buffer port.  When the back buffer is active most read ports return scrambled
        data from the back buffer, however some ports work exclusively on front buffer data
        even if the back buffer is selected.

        There doesn't appear to be any way to detect what bank is currently selected - it
        seems game code must maintain this state (if it matters).
    */

	if (decoprot_buffer_ram_selected)
		COMBINE_DATA(&decoprot_buffer_ram[offset]);
	else
		COMBINE_DATA(&deco16_prot_ram[offset]);

	if (offset==(0xa8/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}

	// These are set regardless of bank
	if (offset==0x42/2)
		COMBINE_DATA(&deco16_xor);
	if (offset==0xee/2)
		COMBINE_DATA(&deco16_mask);

	offset=offset*2;

	//logerror("CONTROL PC %06x: warning - write protection memory address %04x %04x\n",activecpu_get_pc(),offset,data);
	if (offset==0xee || offset==0x42 || offset==0xa8)
		return;

//  logerror("CONTROL PC %06x: warning - write protection memory address %04x %04x\n",activecpu_get_pc(),offset,data);

#if 1
// 66 7c 7e 28 58 4a 9e
	if (offset==0x66 || offset==0x7c || offset==0x7e || offset==0x28 || offset==0x58 || offset==0x4a)
		return;
	if (offset==0x9e || offset==0x7c || offset==0x7e || offset==0x28 || offset==0x58 || offset==0x4a)
		return;
#endif

#if 1
	if (offset>=0x80 && offset<0xa0)
		return;
	if (offset>=0xc0 && offset<0xd0)
		return;

//  if (offset==0x3c)
//      logerror("CONTROL PC %06x: warning - write protection memory address %04x %04x\n",activecpu_get_pc(),offset,data);
// Actually read:
//  22 24 26 2c 2e 30 32 3c 40 44 46 48 60 62 66 6a 6e 76 7e 80 82 84 86 88 8a 8c 90 94 96 98 9a 9c a0 c0 c4 c6 c8 cc ce d6 dc de

// check 74 b0 b4 d4

//  24 2a 2c 2e 30 3c 44 48 66 6a 74 a0 b0 b4 d4 d6
//  32 60 7e
//  22 26 a4 a6
//  62 6e d0 dc de d2 d8 da 46
	if (offset==0x24 || offset==0x2a || offset==0x2c || offset==0x2e || offset==0x30 || offset==0x3c)
		return;
	if (offset==0x44 || offset==0x48 || offset==0x66 || offset==0x6a || offset==0x74 || offset==0xa0)
		return;
	if (offset==0xb0 || offset==0xb4 || offset==0xd4 || offset==0xd6)
		return;
	if (offset==0x32 || offset==0x60 || offset==0x7e || offset==0xa4 || offset==0xa6)
		return;
	if (offset==0x22 || offset==0x26 || offset==0x62 || offset==0xd0 || offset==0xdc)
		return;
	if (offset==0xde || offset==0xd2 || offset==0xd8 || offset==0xda || offset==0x46)
		return;
	if (offset==0x20 || offset==0x40)
		return;
	if (offset==0x6e || offset==0xaa || offset==0xac || offset==0xa2)
		return;
#endif

	logerror("CONTROL PC %06x: warning - write unmapped protection memory address %04x %04x\n",activecpu_get_pc(),offset,data);
}

READ16_HANDLER( deco16_104_rohga_prot_r )
{
	const UINT16* prot_ram=decoprot_buffer_ram_selected ? decoprot_buffer_ram : deco16_prot_ram;

//  if (offset!=0x88/2 && offset!=0x44c/2 && offset!=0x36c/2 && offset!=0x292/2)
//      logerror("Protection PC %06x: warning - read prot address %04x\n",activecpu_get_pc(),offset<<1);

	switch (offset) {
		case 0x88/2: /* Player 1 & 2 input ports */
			return readinputport(0);
		case 0x36c/2:
			return readinputport(1);
		case 0x44c/2:

			return ((readinputport(1)&0x7)<<13)|((readinputport(1)&0x8)<<9);
		case 0x292/2: /* Dips */
			return readinputport(2);

		case 0x44/2:
			return ((((DECO_PORT(0x2c)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);
		case 0x282/2:
			return ((DECO_PORT(0x26)&0x000f)<<12) & (~deco16_mask);
		case 0xd4/2:
			return ((DECO_PORT(0x6e)&0x0ff0)<<4) | ((DECO_PORT(0x6e)&0x000e)<<3) | ((DECO_PORT(0x6e)&0x0001)<<7);
		case 0x5a2/2:
			return (((DECO_PORT(0x24)&0xff00)>>4) | ((DECO_PORT(0x24)&0x000f)<<0) | ((DECO_PORT(0x24)&0x00f0)<<8)) & (~deco16_mask);
		case 0x570/2:
			return (((DECO_PORT(0x24)&0xf0f0)>>0) | ((DECO_PORT(0x24)&0x000f)<<8)) ^ deco16_xor;
		case 0x32e/2:
			return (((DECO_PORT(0x46)&0xf000)>>0) | ((DECO_PORT(0x46)&0x00ff)<<4)) & (~deco16_mask);
		case 0x4dc/2:
			return ((DECO_PORT(0x62)&0x00ff)<<8);
		case 0x1be/2:
			return ((((DECO_PORT(0xc2)&0x0ff0)<<4) | ((DECO_PORT(0xc2)&0x0003)<<6) | ((DECO_PORT(0xc2)&0x000c)<<2)) ^ deco16_xor) & (~deco16_mask);

		case 0x420/2:
			return ((DECO_PORT(0x2e)&0xf000)>>4) | ((DECO_PORT(0x2e)&0x0f00)<<4) | ((DECO_PORT(0x2e)&0x00f0)>>4) | ((DECO_PORT(0x2e)&0x000f)<<4);

		case 0x390/2:
			return DECO_PORT(0x2c);

		case 0x756/2:
			return ((DECO_PORT(0x60)&0xfff0)>>4) | ((DECO_PORT(0x60)&0x0007)<<13) | ((DECO_PORT(0x60)&0x0008)<<9);
		case 0x424/2:
			return ((DECO_PORT(0x60)&0xf000)>>4) | ((DECO_PORT(0x60)&0x0f00)<<4) | ((DECO_PORT(0x60)&0x00f0)>>0) | ((DECO_PORT(0x60)&0x000f)<<0);

		case 0x156/2:
			return (((DECO_PORT(0xde)&0xff00)<<0) | ((DECO_PORT(0xde)&0x000f)<<4) | ((DECO_PORT(0xde)&0x00f0)>>4)) & (~deco16_mask);
		case 0xa8/2:
			return (((DECO_PORT(0xde)&0xff00)>>4) | ((DECO_PORT(0xde)&0x000f)<<0) | ((DECO_PORT(0xde)&0x00f0)<<8)) & (~deco16_mask);
		case 0x64a/2:
			return (((DECO_PORT(0xde)&0xfff0)>>4) | ((DECO_PORT(0xde)&0x000c)<<10) | ((DECO_PORT(0xde)&0x0003)<<14)) & (~deco16_mask);

		case 0x16e/2:
			return DECO_PORT(0x6a);

		case 0x39c/2:
			return (DECO_PORT(0x6a)&0x00ff) | ((DECO_PORT(0x6a)&0xf000)>>4) | ((DECO_PORT(0x6a)&0x0f00)<<4);
		case 0x212/2:
			return (((DECO_PORT(0x6e)&0xff00)>>4) | ((DECO_PORT(0x6e)&0x00f0)<<8) | ((DECO_PORT(0x6e)&0x000f)<<0)) ^ deco16_xor;

		case 0x70a/2:
			return (((DECO_PORT(0xde)&0x00f0)<<8) | ((DECO_PORT(0xde)&0x0007)<<9) | ((DECO_PORT(0xde)&0x0008)<<5)) ^ deco16_xor;

		case 0x7a0/2:
			return (DECO_PORT(0x6e)&0x00ff) | ((DECO_PORT(0x6e)&0xf000)>>4) | ((DECO_PORT(0x6e)&0x0f00)<<4);
		case 0x162/2:
			return DECO_PORT(0x6e);

		case 0x384/2:
			return ((DECO_PORT(0xdc)&0xf000)>>12) | ((DECO_PORT(0xdc)&0x0ff0)<<4) | ((DECO_PORT(0xdc)&0x000c)<<2) | ((DECO_PORT(0xdc)&0x0003)<<6);

		case 0x302/2:
			return DECO_PORT(0x24);
		case 0x334/2:
			return DECO_PORT(0x30);
		case 0x34c/2:
			return DECO_PORT(0x3c);

		case 0x514/2:
			return (((DECO_PORT(0x32)&0x0ff0)<<4) | ((DECO_PORT(0x32)&0x000c)<<2) | ((DECO_PORT(0x32)&0x0003)<<6)) & (~deco16_mask);

		case 0x34e/2:
			return ((DECO_PORT(0xde)&0x0ff0)<<4) | ((DECO_PORT(0xde)&0xf000)>>8) | ((DECO_PORT(0xde)&0x000f)<<0);
		case 0x722/2:
			return (((DECO_PORT(0xdc)&0x0fff)<<4) ^ deco16_xor) & (~deco16_mask);
		case 0x574/2:
			return ((((DECO_PORT(0xdc)&0xfff0)>>0) | ((DECO_PORT(0xdc)&0x0003)<<2) | ((DECO_PORT(0xdc)&0x000c)>>2)) ^ deco16_xor) & (~deco16_mask);

		case 0x5ae/2:
			return DECO_PORT(0xdc);
		case 0x410/2:
			return DECO_PORT(0xde);
		case 0x340/2:
			return ((DECO_PORT(0x90)&0xfff0) | ((DECO_PORT(0x90)&0x7)<<1) | ((DECO_PORT(0x90)&0x8)>>3)) ^ deco16_xor;
		case 0x4a4/2:
			return (((DECO_PORT(0xce)&0x0ff0) | ((DECO_PORT(0xce)&0xf000)>>12) | ((DECO_PORT(0xce)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);
		case 0x256/2:
			return ((((DECO_PORT(0xce)&0xf000)>>12) | ((DECO_PORT(0xce)&0x0fff)<<4))) & (~deco16_mask);
		case 0x79a/2:
			return (((DECO_PORT(0xc8)&0xfff0)>>4) | ((DECO_PORT(0xc8)&0x0008)<<9) | ((DECO_PORT(0xc8)&0x0007)<<13)) ^ deco16_xor;

		case 0x65e/2:
			return DECO_PORT(0x9c);
		case 0x79c/2:
			return ((DECO_PORT(0xc6)&0xf000) | ((DECO_PORT(0xc6)&0x00ff)<<4) | ((DECO_PORT(0xc6)&0x0f00)>>8)) & (~deco16_mask);
		case 0x15e/2:
			return (((DECO_PORT(0x98)&0x0ff0)<<4) | ((DECO_PORT(0x98)&0xf000)>>12) | ((DECO_PORT(0x98)&0x0003)<<6) | ((DECO_PORT(0x98)&0x000c)<<2)) ^ deco16_xor;
		case 0x6e4/2:
			return DECO_PORT(0x98);
		case 0x1e/2:
			return ((((DECO_PORT(0xc4)&0xf000)>>4) | ((DECO_PORT(0xc4)&0x0f00)<<4) | ((DECO_PORT(0xc4)&0x00ff)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x23a/2:
			return ((((DECO_PORT(0x86)&0xfff0)>>0) | ((DECO_PORT(0x86)&0x0003)<<2) | ((DECO_PORT(0x86)&0x000c)>>2)) ^ deco16_xor);
		case 0x6e/2:
			return ((((DECO_PORT(0x96)&0xf000)>>8) | ((DECO_PORT(0x96)&0x0f0f)<<0) | ((DECO_PORT(0x96)&0x00f0)<<8)) ^ deco16_xor);
		case 0x3a2/2:
			return ((((DECO_PORT(0x94)&0xf000)>>8) | ((DECO_PORT(0x94)&0x0f00)>>8) | ((DECO_PORT(0x94)&0x00f0)<<8) | ((DECO_PORT(0x94)&0x000e)<<7) | ((DECO_PORT(0x94)&0x0001)<<11)) ^ deco16_xor);// & (~deco16_mask);
		case 0x4a6/2:
			return ((DECO_PORT(0x8c)&0xff00)>>0) | ((DECO_PORT(0x8c)&0x00f0)>>4) | ((DECO_PORT(0x8c)&0x000f)<<4);
		case 0x7b0/2:
			return DECO_PORT(0x80);
		case 0x5aa/2:
			return ((((DECO_PORT(0x98)&0x0f00)>>8) | ((DECO_PORT(0x98)&0xf000)>>8) | ((DECO_PORT(0x98)&0x00f0)<<8) | ((DECO_PORT(0x98)&0x000e)<<7) | ((DECO_PORT(0x98)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);
		case 0x662/2:
			return DECO_PORT(0x8c);
		case 0x624/2:
			return DECO_PORT(0x9a);
		case 0x2c/2:
			return (((DECO_PORT(0x82)&0x0f0f)>>0) | ((DECO_PORT(0x82)&0xf000)>>8) | ((DECO_PORT(0x82)&0x00f0)<<8)) & (~deco16_mask);

		case 0x1b4/2:
			return ((DECO_PORT(0xcc)&0x00f0)<<4) | ((DECO_PORT(0xcc)&0x000f)<<12);

		case 0x7ce/2:
			return ((DECO_PORT(0x80)&0x000e)<<11) | ((DECO_PORT(0x80)&0x0001)<<15);
		case 0x41a/2:
			return ((((DECO_PORT(0x84)&0x00f0)<<8) | ((DECO_PORT(0x84)&0xf000)>>8) | ((DECO_PORT(0x84)&0x0f00)>>8) | ((DECO_PORT(0x84)&0x0003)<<10) | ((DECO_PORT(0x84)&0x000c)<<6)) ^ deco16_xor);
		case 0x168/2:
			return ((((DECO_PORT(0x84)&0x0ff0)<<4) | ((DECO_PORT(0x84)&0x000e)<<3) | ((DECO_PORT(0x84)&0x0001)<<5))) & (~deco16_mask);
		case 0x314/2:
			return ((((DECO_PORT(0x84)&0x0ff0)<<4) | ((DECO_PORT(0x84)&0x000e)<<3) | ((DECO_PORT(0x84)&0x0001)<<5)));
		case 0x5e2/2:
			return ((((DECO_PORT(0x84)&0x00f0)<<8) | ((DECO_PORT(0x84)&0x000e)<<7) | ((DECO_PORT(0x84)&0x0001)<<9)));
		case 0x72a/2:
			return ((((DECO_PORT(0x86)&0xfff0)>>4) | ((DECO_PORT(0x86)&0x0003)<<14) | ((DECO_PORT(0x86)&0x000c)<<10)) ^ deco16_xor) & (~deco16_mask);
		case 0x178/2:
			return (((DECO_PORT(0x88)&0x00ff)<<8) | ((DECO_PORT(0x88)&0xff00)>>8)) & (~deco16_mask);
		case 0x40e/2:
			return ((((DECO_PORT(0x8a)&0xf000)>>0) | ((DECO_PORT(0x8a)&0x00ff)<<4)) ^ deco16_xor) & (~deco16_mask);
		case 0x248/2:
			return ((((DECO_PORT(0x8c)&0xff00)>>8) | ((DECO_PORT(0x8c)&0x00f0)<<4) | ((DECO_PORT(0x8c)&0x000f)<<12)) ^ deco16_xor) & (~deco16_mask);

		case 0x27e/2:
			return ((((DECO_PORT(0x94)&0x00f0)<<8)) ^ deco16_xor) & (~deco16_mask);

		case 0x22c/2:
			return ((DECO_PORT(0xc4)&0x00f0)<<8);
		case 0x77e/2:
			return ((DECO_PORT(0x62)&0xf000)>>12) | ((DECO_PORT(0x62)&0x0ff0)<<0) | ((DECO_PORT(0x62)&0x000f)<<12);
		case 0xc/2:
			return ((DECO_PORT(0xd6)&0xf000)>>12) | ((DECO_PORT(0xd6)&0x0fff)<<4);

		case 0x90/2:
			return DECO_PORT(0x44);
		case 0x246/2:
			return ((((DECO_PORT(0x48)&0xff00)>>8) | ((DECO_PORT(0x48)&0x00f0)<<8) | ((DECO_PORT(0x48)&0x0f00)>>8) | ((DECO_PORT(0x48)&0x0003)<<10) | ((DECO_PORT(0x48)&0x000c)<<6)) ^ deco16_xor);
		case 0x546/2:
			return (((DECO_PORT(0x62)&0xf0f0)>>0) | ((DECO_PORT(0x62)&0x000f)<<8)) & (~deco16_mask);
		case 0x2e2/2:
			return ((DECO_PORT(0xc6)&0x000e)<<11) | ((DECO_PORT(0xc6)&0x0001)<<15);
		case 0x3c0/2:
			return DECO_PORT(0x22);
		case 0x4b8/2:
			return (((DECO_PORT(0x46)&0xf000)>>12) | ((DECO_PORT(0x46)&0x0f00)>>4) | ((DECO_PORT(0x46)&0x00ff)<<8)) ^ deco16_xor;
		case 0x65c/2:
			return ((((DECO_PORT(0x44)&0xf000)>>12) | ((DECO_PORT(0x44)&0x0fff)<<4)) ^ deco16_xor) & (~deco16_mask);

		case 0x32a/2:
			return ((((DECO_PORT(0xc0)&0x0ff0)<<4) | ((DECO_PORT(0xc0)&0x000e)<<3) | ((DECO_PORT(0xc0)&0x0001)<<7))) & (~deco16_mask);// ^ deco16_xor;
		case 0x8/2:
			return ((((DECO_PORT(0x94)&0xfff0)<<0) | ((DECO_PORT(0x94)&0x000e)>>1) | ((DECO_PORT(0x94)&0x0001)<<3))) & (~deco16_mask);// ^ deco16_xor;
		case 0x456/2:
			return (((DECO_PORT(0x26)&0xfff0)<<0) | ((DECO_PORT(0x26)&0x0007)<<1) | ((DECO_PORT(0x26)&0x0008)>>3));// ^ deco16_xor;
		case 0x190/2:
			return ((((DECO_PORT(0x44)&0xf000)<<0) | ((DECO_PORT(0x44)&0x00ff)<<4))) & (~deco16_mask);// ^ deco16_xor;
		case 0x3f2/2:
			return ((((DECO_PORT(0x48)&0x000f)<<12) | ((DECO_PORT(0x48)&0x00f0)<<4))) & (~deco16_mask);// ^ deco16_xor;
		case 0x2be/2:
			return ((DECO_PORT(0x40)&0x00ff)<<8);

		case 0x19e/2:
			return ((((DECO_PORT(0x3c)&0xf000)>>12) | ((DECO_PORT(0x3c)&0x0f00)<<4) | ((DECO_PORT(0x3c)&0x00f0)>>0) | ((DECO_PORT(0x3c)&0x000f)<<8)) ^ deco16_xor) & (~deco16_mask);
		case 0x2a2/2:
			return ((((DECO_PORT(0x44)&0xff00)>>8) | ((DECO_PORT(0x44)&0x00f0)<<8) | ((DECO_PORT(0x44)&0x000e)<<7) | ((DECO_PORT(0x44)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);
		case 0x748/2:
			return (((DECO_PORT(0x44)&0xfff0)<<0) | ((DECO_PORT(0x44)&0x000e)>>1) | ((DECO_PORT(0x44)&0x0001)<<3));// & (~deco16_mask);
		case 0x686/2:
			return (((DECO_PORT(0x46)&0xf000)>>4) | ((DECO_PORT(0x46)&0x0f00)>>8) | ((DECO_PORT(0x46)&0x00f0)<<8) | ((DECO_PORT(0x46)&0x000f)<<4));// & (~deco16_mask);
		case 0x4c4/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12) & (~deco16_mask);
		case 0x538/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12);
		case 0x63a/2:
			return ((DECO_PORT(0x3c)&0x000f)<<12);
		case 0x348/2:
			return ((((DECO_PORT(0x44)&0xf000)>>12) | ((DECO_PORT(0x44)&0x0ff0)<<4) | ((DECO_PORT(0x44)&0x000e)<<3) | ((DECO_PORT(0x44)&0x0001)<<7))) ^ deco16_xor;// & (~deco16_mask);
		case 0x200/2:
			return (((DECO_PORT(0xa0)&0xfff0)>>4) | ((DECO_PORT(0xa0)&0x0007)<<13) | ((DECO_PORT(0xa0)&0x0008)<<9));// & (~deco16_mask);
		case 0x254/2:
			return ((((DECO_PORT(0x7e)&0x0ff0)<<4) | ((DECO_PORT(0x7e)&0x000c)<<2) | ((DECO_PORT(0x7e)&0x0003)<<6))) ^ deco16_xor;// & (~deco16_mask);
		case 0x182/2:
			return ((DECO_PORT(0x46)&0xf000)<<0) | ((DECO_PORT(0x46)&0x0f00)>>8) | ((DECO_PORT(0x46)&0x00f0)>>0) | ((DECO_PORT(0x46)&0x000f)<<8);
		case 0x58/2:
			return DECO_PORT(0x46);
		case 0x48e/2:
			return ((((DECO_PORT(0x46)&0xf000)>>12) | ((DECO_PORT(0x46)&0x0f00)>>4) | ((DECO_PORT(0x46)&0x00f0)<<4) | ((DECO_PORT(0x46)&0x000f)<<12)));// /*^ deco16_xor*/) & (~deco16_mask);

		case 0x4ba/2:
			return (((DECO_PORT(0x24)&0xf000)>>12) | ((DECO_PORT(0x24)&0x0ff0)<<4) | ((DECO_PORT(0x24)&0x000c)<<2) | ((DECO_PORT(0x24)&0x0003)<<6)) & (~deco16_mask);
		case 0x92/2:
			return (((DECO_PORT(0x3c)&0xfff0)>>0) | ((DECO_PORT(0x3c)&0x0007)<<1) | ((DECO_PORT(0x3c)&0x0008)>>3));
		case 0x1f0/2:
			return ((((DECO_PORT(0xa2)&0xf000)>>12) | ((DECO_PORT(0xa2)&0x0f00)>>4) | ((DECO_PORT(0xa2)&0x00ff)<<8)) ^ deco16_xor) & (~deco16_mask);
		case 0x24e/2:
			return ((((DECO_PORT(0x46)&0xf000)>>8) | ((DECO_PORT(0x46)&0x0f00)>>0) | ((DECO_PORT(0x46)&0x00f0)>>4) | ((DECO_PORT(0x46)&0x000f)<<12)) ^ deco16_xor);// & (~deco16_mask);
		case 0x594/2:
			return ((((DECO_PORT(0x40)&0x00f0)<<8) | ((DECO_PORT(0x40)&0x000c)<<6) | ((DECO_PORT(0x40)&0x0003)<<10)) ^ deco16_xor);// & (~deco16_mask);

		case 0x7e2/2:
			return ((((DECO_PORT(0x96)&0xf000)<<0) | ((DECO_PORT(0x96)&0x00f0)<<4) | ((DECO_PORT(0x96)&0x000f)<<4))) ^ deco16_xor;// | ((DECO_PORT(0x96)&0x0001)<<7));// ^ deco16_xor);// & (~deco16_mask);
		case 0x18c/2:
			return (((DECO_PORT(0x22)&0xfff0)>>4) | ((DECO_PORT(0x22)&0x000e)<<11) | ((DECO_PORT(0x22)&0x0001)<<15));// ^ deco16_xor);// & (~deco16_mask);
		case 0x1fa/2:
			return ((((DECO_PORT(0x26)&0xf000)>>8) | ((DECO_PORT(0x26)&0x0f00)<<0) | ((DECO_PORT(0x26)&0x00f0)>>4) | ((DECO_PORT(0x26)&0x000f)<<12))) ^ deco16_xor;// & (~deco16_mask);
		case 0x70e/2:
			return ((((DECO_PORT(0x26)&0x0ff0)<<4) | ((DECO_PORT(0x26)&0x000c)<<2) | ((DECO_PORT(0x26)&0x0003)<<6))) ^ deco16_xor;// & (~deco16_mask);
		case 0x33a/2:
			return DECO_PORT(0x60) & (~deco16_mask);
		case 0x1e2/2:
			return ((DECO_PORT(0xd0)&0xf000)>>12) | ((DECO_PORT(0xd0)&0x0f00)>>4) | ((DECO_PORT(0xd0)&0x00ff)<<8);
		case 0x3f4/2:
			return DECO_PORT(0x6e)<<4;

		case 0x2ae/2:
			return ((DECO_PORT(0x9c)&0xf000)<<0) | ((DECO_PORT(0x9c)&0x0ff0)>>4) | ((DECO_PORT(0x9c)&0x000f)<<8);// & (~deco16_mask);
		case 0x96/2:
			return ((((DECO_PORT(0x22)&0xff00)>>8) | ((DECO_PORT(0x22)&0x00f0)<<8) | ((DECO_PORT(0x22)&0x000e)<<7) | ((DECO_PORT(0x22)&0x0001)<<11)) ^ deco16_xor) & (~deco16_mask);

		case 0x33e/2:
			return (((DECO_PORT(0x0)&0xf000)>>12) | ((DECO_PORT(0x0)&0x0f00)>>4) | ((DECO_PORT(0x0)&0x00f0)<<4) | ((DECO_PORT(0x0)&0x000f)<<12)) & (~deco16_mask);

		case 0x6c4/2: /* Reads from here flip buffers */
			decoprot_buffer_ram_selected^=1;
			// Flip occurs AFTER this data has been calculated
			return ((DECO_PORT(0x66)&0xf0f0) | ((DECO_PORT(0x66)&0x000f)<<8)) & (~deco16_mask);
		case 0x700/2: /* Reads from here flip buffers */
			decoprot_buffer_ram_selected^=1;
			return (((DECO_PORT(0x66)&0xf000)>>4) | ((DECO_PORT(0x66)&0x00f0)<<8)) ^ deco16_xor;
		case 0x444/2:
			decoprot_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x00f0)<<8) | ((DECO_PORT(0x66)&0x0007)<<9)  | ((DECO_PORT(0x66)&0x0008)<<5);
		case 0x2d0/2:
			decoprot_buffer_ram_selected^=1;
			return (((DECO_PORT(0x66)&0xf000)>>4) | ((DECO_PORT(0x66)&0x00f0)<<8)) ^ deco16_xor;
		case 0x2b8/2:
			decoprot_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x00f0)<<8) ^ deco16_xor;
		case 0x294/2:
			decoprot_buffer_ram_selected^=1;
			return ((DECO_PORT(0x66)&0x000f)<<12);
		case 0x1e8/2:
			decoprot_buffer_ram_selected^=1;
			return 0; // todo

		case 0x49c/2:
			return (((DECO_PORT(0x6c)&0x00f0)<<8) ^ deco16_xor) & (~deco16_mask);

		case 0x44e/2:
			return (((DECO_PORT(0x44)&0x00f0)<<4) | ((DECO_PORT(0x44)&0x000f)<<12)) ^ deco16_xor;
		case 0x3ca/2:
			return (((DECO_PORT(0x1e)&0xfff0)>>4) | ((DECO_PORT(0x1e)&0x0003)<<14) | ((DECO_PORT(0x1e)&0x000c)<<10)) ^ deco16_xor;
		case 0x2ac/2:
			return DECO_PORT(0x1e);
		case 0x3c/2:
			return (((DECO_PORT(0x1e)&0x0003)<<14) | ((DECO_PORT(0x1e)&0x000c)<<10)) & (~deco16_mask);
		case 0x174/2:
			return (((DECO_PORT(0x1e)&0xff00)>>8) | ((DECO_PORT(0x1e)&0x00f0)<<8) | ((DECO_PORT(0x1e)&0x0007)<<9) | ((DECO_PORT(0x1e)&0x0008)<<5)) & (~deco16_mask);
		case 0x34a/2:
			return (((DECO_PORT(0x4)&0xff00)>>0) | ((DECO_PORT(0x4)&0x00f0)>>4) | ((DECO_PORT(0x4)&0x000f)<<4)) & (~deco16_mask);
		case 0x324/2:
			return (((DECO_PORT(0x6)&0xf000)>>12) | ((DECO_PORT(0x6)&0x0ff0)<<4) | ((DECO_PORT(0x6)&0x0007)<<5) | ((DECO_PORT(0x6)&0x0008)<<1));
		case 0x344/2:
			return (((DECO_PORT(0x8)&0xf000)>>8) | ((DECO_PORT(0x8)&0x0f00)>>8) | ((DECO_PORT(0x8)&0x00f0)<<4) | ((DECO_PORT(0x8)&0x000f)<<12));
		case 0x72/2:
			return ((((DECO_PORT(0xa)&0xf000)>>8) | ((DECO_PORT(0xa)&0x0ff0)<<4) | ((DECO_PORT(0xa)&0x000f)>>0))) & (~deco16_mask);
		case 0x36e/2:
			return ((((DECO_PORT(0xc)&0xf000)>>0) | ((DECO_PORT(0xc)&0x0ff0)>>4) | ((DECO_PORT(0xc)&0x000f)<<8))) & (~deco16_mask);

		case 0x590/2:
			return ((((DECO_PORT(0xe)&0xfff0)>>4) | ((DECO_PORT(0xe)&0x000e)<<11) | ((DECO_PORT(0xe)&0x0001)<<15))) ^ deco16_xor;
		case 0x7b6/2:
			return ((((DECO_PORT(0x2)&0xf000)>>8) | ((DECO_PORT(0x2)&0x0ff0)<<4) | ((DECO_PORT(0x2)&0x000f)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x588/2:
			return ((((DECO_PORT(0x4)&0xff00)>>4) | ((DECO_PORT(0x4)&0x00f0)<<8) | ((DECO_PORT(0x4)&0x000f)<<0)) ^ deco16_xor) & (~deco16_mask);
		case 0x1f6/2:
			return (((DECO_PORT(0x6)&0xf000)>>12) | ((DECO_PORT(0x6)&0x0ff0)<<4) | ((DECO_PORT(0x6)&0x0007)<<5) | ((DECO_PORT(0x6)&0x0008)<<1)) ^ deco16_xor;
		case 0x4c0/2:
			return (((DECO_PORT(0x8)&0xf000)>>4) | ((DECO_PORT(0x8)&0x0f00)<<4) | ((DECO_PORT(0x8)&0x00f0)>>4) | ((DECO_PORT(0x8)&0x000f)<<4)) & (~deco16_mask);
		case 0x63e/2:
			return ((((DECO_PORT(0xa)&0x0ff0)<<4) | ((DECO_PORT(0xa)&0xf000)>>12) | ((DECO_PORT(0xa)&0x0003)<<6) | ((DECO_PORT(0xa)&0x000c)<<2)));
		case 0x7cc/2:
			return ((((DECO_PORT(0xc)&0xfff0)>>4) | ((DECO_PORT(0xc)&0x000e)<<11) | ((DECO_PORT(0xc)&0x0001)<<15)) ^ deco16_xor) & (~deco16_mask);
		case 0x1bc/2:
			return (((DECO_PORT(0xe)&0xf000)>>12) | ((DECO_PORT(0xe)&0x0f00)>>4) | ((DECO_PORT(0xe)&0x00ff)<<8)) & (~deco16_mask);

		case 0x780/2:
			return DECO_PORT(0xb8);

		case 0x454/2:
			return (((DECO_PORT(0x82)&0xf000)>>8) | ((DECO_PORT(0x82)&0x0f00)>>0) | ((DECO_PORT(0x82)&0x00f0)>>4) | ((DECO_PORT(0x82)&0x000f)<<12)) ^ deco16_xor;
		case 0x53e/2:
			return ((DECO_PORT(0x9e)&0x0003)<<14) | ((DECO_PORT(0x9e)&0x000c)<<10);
		case 0x250/2:
			return (((DECO_PORT(0x62)&0xf0f0)<<0) | ((DECO_PORT(0x62)&0x0f00)>>8)  | ((DECO_PORT(0x62)&0x000f)<<8)) & (~deco16_mask);


		case 0x150/2: /* Shared */
			return DECO_PORT(0x7e);
		case 0x10e/2: /* Schmeizr Robo only */
			return DECO_PORT(0x7c);
		case 0x56a/2: /* Schmeizr Robo only */
			return (((DECO_PORT(0x7c)&0xfff0)>>4) | ((DECO_PORT(0x7c)&0x000e)<<11) | ((DECO_PORT(0x7c)&0x0001)<<15)) & (~deco16_mask);
		case 0x39a/2: /* Schmeizr Robo only */
			return ((((DECO_PORT(0x7e)&0xfff0)>>4) | ((DECO_PORT(0x7e)&0x000e)<<11) | ((DECO_PORT(0x7e)&0x0001)<<15)) ^ deco16_xor) & (~deco16_mask);
		case 0x188/2: /* Schmeizr Robo only */
			return (((deco16_mask&0x0003)<<6) | ((deco16_mask&0x000c)<<2) | ((deco16_mask&0x00f0)<<4) | ((deco16_mask&0x0f00)<<4)) & (~deco16_mask);
		case 0x3cc/2: /* Schmeizr Robo only */
			return deco16_mask;
		case 0x4a/2: /* Schmeizr Robo only */
			return DECO_PORT(0x9e) & (~deco16_mask);
		case 0x7e8/2: /* Schmeizr Robo only */
			return DECO_PORT(0x4a) ^ deco16_xor;
		case 0xfc/2: /* Schmeizr Robo only */
			return DECO_PORT(0x4a);
		case 0x38c/2: /* Schmeizr Robo only */
			return DECO_PORT(0x28);
		case 0x28/2: /* Schmeizr Robo only  */
			return DECO_PORT(0x58);
	}

	logerror("Protection PC %06x: warning - read unmapped protection address %04x\n",activecpu_get_pc(),offset<<1);

	return 0;
}

/**********************************************************************************/

static WRITE16_HANDLER( deco16_146_core_prot_w )
{
	const int writeport=offset;
	const int sndport=0x260;
	const int xorport=0x340;
	const int maskport=0x6c0;

	if (writeport==sndport) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}

	if (writeport==xorport)
		COMBINE_DATA(&deco16_xor);
	if (writeport==maskport)
		COMBINE_DATA(&deco16_mask);

	if (decoprot_buffer_ram_selected)
		COMBINE_DATA(&decoprot_buffer_ram2[offset>>1]);
	else
		COMBINE_DATA(&decoprot_buffer_ram[offset>>1]);

//  if (offset!=0x5e0 && offset!=0x340 && offset!=0 && offset!=0x3d0 && offset!=0x280)
//      logerror("%08x:  Write protection port %04x, data %04x (%08x)\n", activecpu_get_pc(), offset, data, mem_mask);
}

static READ16_HANDLER( deco16_146_core_prot_r )
{
//  const UINT16* prot_ram=decoprot_buffer_ram;
	UINT16 val;
	const UINT16* prot_ram=decoprot_buffer_ram_selected ? decoprot_buffer_ram2 : decoprot_buffer_ram;

	switch (offset)
	{
	case 0x582: /* Player 1 & Player 2 */
		return readinputport(0);
	case 0x04c: /* Coins/VBL */
		return readinputport(1);
	case 0x672: /* Dip switches */
		return readinputport(2);

	case 0x13a:
		return ((DECO_PORT(0x190)&0x00f0)<<8) | ((DECO_PORT(0x190)&0x0003)<<10) | ((DECO_PORT(0x190)&0x000c)<<6);

	case 0x53c:
		return ((DECO_PORT(0x30)&0x0ff0)<<4) | ((DECO_PORT(0x30)&0xf000)>>8);

	case 0x6c:
		return ((DECO_PORT(0x370)&0x00ff)<<8);

	case 0xa:
		return ((DECO_PORT(0x310)&0x0fff)<<4);

	case 0x4f6:
		return ((DECO_PORT(0x20)&0x00f0)<<8) | ((DECO_PORT(0x20)&0x0007)<<9) | ((DECO_PORT(0x20)&0x0008)<<5);

	case 0xea:
		return ((DECO_PORT(0x1c0)&0xf000)<<0) | ((DECO_PORT(0x1c0)&0x00ff)<<4);

	case 0x12e:
		return ((DECO_PORT(0x1f0)&0xf000)>>4) | ((DECO_PORT(0x1f0)&0x0f00)<<4) | ((DECO_PORT(0x1f0)&0x00f0)>>4) | ((DECO_PORT(0x1f0)&0x000f)<<4);

	case 0x316:
		return ((DECO_PORT(0x290)&0xf000)>>4) | ((DECO_PORT(0x290)&0x0f00)<<4) | ((DECO_PORT(0x290)&0x00ff)<<0);

	case 0x3c6:
		return ((DECO_PORT(0x170)&0xfff0)<<0) | ((DECO_PORT(0x170)&0x000e)>>1) | ((DECO_PORT(0x170)&0x0001)<<3);

	case 0x4d0:
		return ((DECO_PORT(0x20)&0x00f0)<<8) | ((DECO_PORT(0x20)&0x0007)<<9) | ((DECO_PORT(0x20)&0x0008)<<5);

	case 0x53a:
		return ((DECO_PORT(0x370)&0xffff)<<0);

	case 0x552:
		return ((DECO_PORT(0x240)&0xfff0)<<0) | ((DECO_PORT(0x240)&0x0007)<<1) | ((DECO_PORT(0x240)&0x0008)>>3);

	case 0x54c:
		return ((DECO_PORT(0x2f0)&0x00ff)<<8);

	case 0x5da:
		return ((DECO_PORT(0x130)&0x00f0)<<8) | ((DECO_PORT(0x130)&0x000e)<<7) | ((DECO_PORT(0x130)&0x0001)<<11);

	case 0x6be:
		return ((DECO_PORT(0x150)&0xf000)>>12) | ((DECO_PORT(0x150)&0x0ff0)<<0) | ((DECO_PORT(0x150)&0x000f)<<12);

	case 0x70a:
		return ((DECO_PORT(0x1d0)&0x0ff0)<<4) | ((DECO_PORT(0x1d0)&0x0003)<<6) | ((DECO_PORT(0x1d0)&0x000c)<<2);

	case 0x7e0:
		return ((DECO_PORT(0x2b0)&0xfff0)<<0) | ((DECO_PORT(0x2b0)&0x0003)<<2) | ((DECO_PORT(0x2b0)&0x000c)>>2);

	case 0x1de:
		return ((DECO_PORT(0x1b0)&0x0ff0)<<4) | ((DECO_PORT(0x1b0)&0x000e)<<3) | ((DECO_PORT(0x1b0)&0x0001)<<7);

	/*********************************************************************************/

//  case 0x582: return readinputport(0); /* IN0 */
//  case 0x672: return readinputport(1); /* IN1 */
//  case 0x04c: return EEPROM_read_bit();

	case 0x468:
		val=DECO_PORT(0x570);
		val=((val&0x0003)<<6) | ((val&0x000c)<<2) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>12);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1ea:
		val=DECO_PORT(0x570);
		val=((val&0x0003)<<10) | ((val&0x000c)<<6) | ((val&0x00f0)<<8) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return val ^ deco16_xor;

	case 0x7b6:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000c)>>2) | ((val&0x0003)<<2) | ((val&0xfff0)<<0);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x01c:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return val ^ deco16_xor;

	case 0x1e0:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000e)<<3) | ((val&0x0001)<<7) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>12);
		return val ^ deco16_xor;

	case 0x1d4:
		val=((DECO_PORT(0))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<4) | ((val&0x0f00)<<4) | ((val&0xf000)>>8);
		return val;

	case 0x0c0:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0x0f00)<<4) | ((val&0xf000)>>4);
		return val ^ deco16_xor;

	case 0x794:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x0007)<<1) | ((val&0xfff0)>>0) | ((val&0x0008)>>3);
		return val ^ deco16_xor;

	case 0x30:
		val=DECO_PORT(0x5e0);
		val=((val&0x0007)<<13) | ((val&0x0008)<<9); /* Bottom bits are masked out before XOR */
		return val ^ deco16_xor;

	case 0x422:
		val=((DECO_PORT(0x3d0))&0xffff);
		val=((val&0x0007)<<1) | ((val&0xfff0)>>0) | ((val&0x0008)>>3);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x558:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<8) | ((val&0x0f00)>>0) | ((val&0xf000)>>8);
		return val;

	case 0x3e:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0x0f00)<<4) | ((val&0xf000)>>4);
		return val & (~deco16_mask);

	case 0x328:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000e)<<3) | ((val&0x0001)<<7) | ((val&0x00f0)<<4) | ((val&0xf000)>>12) | ((val&0x0f00)<<4);
		return val ^ deco16_xor;

	case 0x476:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<0) | ((val&0x00f0)<<8) | ((val&0xff00)>>4);
		return val;

	case 0x50a:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)>>4) | ((val&0x0f00)<<0) | ((val&0xf000)>>8);
		return val;

	case 0x5ae:
		val=((DECO_PORT(0x210))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)>>4) | ((val&0x0f00)>>0) | ((val&0xf000)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1ae:
		val=((DECO_PORT(0x3d0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7a4:
		val=((DECO_PORT(0x620))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>0);
		return val;

	case 0x2c4:
		val=((DECO_PORT(0x410))&0xffff);
		val=((val&0x00ff)<<8) | ((val&0xff00)>>8);
		return val ^ deco16_xor;

	case 0x76: /* Bitshifted XOR, with additional inverse mask on final output */
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>8) | ((val&0xf000)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x714: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0x0003)<<14) | ((val&0x000c)<<10) | ((val&0xfff0)>>4);
		return val & (~deco16_mask);

	case 0x642:
		val=((DECO_PORT(0x2a0))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)>>8)| ((val&0x00f0)<<8) | ((val&0x000f)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x49a: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)>>4) | ((val&0xff00)>>0);
		return val & (~deco16_mask);

	case 0x49c: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000e)<<7) | ((val&0x00f0)<<8) | ((val&0x0001)<<11);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x584: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8) | ((val&0x0008)<<5) | ((val&0x0007)<<9);
		return val & (~deco16_mask);

	case 0x614: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x580))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4) | ((val&0x0f00)>>4) | ((val&0xf000)>>12);
		return val & (~deco16_mask);

	case 0x162: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0xe0))&0xffff);
		val=((val&0x0fff)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x18:
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val ^ deco16_xor;

	case 0x7f6: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x1a0: /* Bitshifting with inverse mask on final output */
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8) | ((val&0x0003)<<10) | ((val&0x000c)<<6);
		return val & (~deco16_mask);

	case 0x4f8:
		val=((DECO_PORT(0x2d0))&0xffff);
		val=((val&0x0fff)<<4);
		return val;

	case 0x1d6:
		val=((DECO_PORT(0xa0))&0xffff);
		val=((val&0x0fff)<<4);
		return val ^ deco16_xor;

	case 0x254:
		val=((DECO_PORT(0x320))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0x00f0)<<0) | ((val&0x000f)<<8);
		return val & (~deco16_mask);

	case 0x2ea:
		val=((DECO_PORT(0x320))&0xffff);
		val=((val&0x00ff)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x21e:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xfff0)<<0) | ((val&0x0007)<<1) | ((val&0x0008)>>3);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7b0:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val ^ deco16_xor;

	case 0x7da:
		val=((DECO_PORT(0x2f0))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x000f)<<12) | ((val&0x00f0)<<4);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x280:
		val=((DECO_PORT(0x2d0))&0xffff);
		val=((val&0x000f)<<8) | ((val&0x00f0)<<8) | ((val&0xf000)>>12) | ((val&0x0f00)>>4);
		return val ^ deco16_xor;

	case 0x416:
		val=((DECO_PORT(0x2e0))&0xffff);
		val=((val&0x000f)<<8) | ((val&0x00f0)>>4) | ((val&0xf000)>>0) | ((val&0x0f00)>>4);
		return val;


	case 0xac:
		val=((DECO_PORT(0x350))&0xffff);
		val=((val&0x000f)<<4) | ((val&0x00f0)<<4) | ((val&0xf000)>>0) | ((val&0x0f00)>>8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x2c2:
		val=((DECO_PORT(0x2e0))&0xffff);
		val=((val&0xf000)<<0) | ((val&0x0ff0)>>4) | ((val&0x000f)<<8);
		return val;

	case 0x450:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<4) | ((val&0x000f)<<12);
		return val;

	case 0x504:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0x000c)<<2) | ((val&0x0003)<<6)| ((val&0x0ff0)<<4);
		return val ^ deco16_xor;

	case 0xfe:
		val=((DECO_PORT(0x440))&0xffff);
		val=((val&0x0fff)<<4);
		return val;

	// 1c0 swap address
	case 0x1c0:
		decoprot_buffer_ram_selected^=1;
		return 0;

	case 0x0e2:
		decoprot_buffer_ram_selected^=1;
		val=((DECO_PORT(0x6c0))&0xffff);
		return val ^ deco16_xor;

	case 0x444:
		val=((DECO_PORT(0xa0))&0xffff);
		val=((val&0xfff0)>>4) | ((val&0x0007)<<13) | ((val&0x0008)<<9);
		return val & (~deco16_mask);

	case 0x46a:
		val=((DECO_PORT(0x10))&0xffff);
		val=((val&0xff00)>>8) | ((val&0x00f0)<<8)| ((val&0x0007)<<9) | ((val&0x0008)<<5);
		return val;

	case 0x80:
		return DECO_PORT(0xe0);

	case 0xb2:
		val=((DECO_PORT(0x280))&0xffff);
		val=((val&0x00f0)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x250:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>12) | ((val&0x0f00)<<4)| ((val&0x00f0)<<4) | ((val&0x000e)<<3) | ((val&0x0001)<<7);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x306:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0xf000)>>4);
		return (val ^ deco16_xor);

	case 0x608:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)>>4)| ((val&0x00f0)<<8) | ((val&0x000f)<<0);
		return val & (~deco16_mask);

	case 0x52e:
		val=((DECO_PORT(0x160))&0xffff);
		val=((val&0xf000)>>4) | ((val&0x0f00)<<4)| ((val&0x00f0)<<0) | ((val&0x000f)<<0);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x67a:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0xf000)>>8) | ((val&0x0ff0)<<4)| ((val&0x000f)<<0);
		return val;

	case 0x6c2:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0x000c)<<6)| ((val&0x0003)<<10);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x3d8:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0xf000)>>8) | ((val&0x0ff0)<<4)| ((val&0x000f)<<0);
		return val & (~deco16_mask);

	case 0x244:
		val=((DECO_PORT(0x760))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0x00f0)>>0)| ((val&0x000f)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x7e8:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x0f00)<<4) | ((val&0xf000)>>4)| ((val&0x00ff)>>0);
		return (val ^ deco16_xor);

	case 0x276:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0x00ff)<<8);
		return (val ^ deco16_xor) & (~deco16_mask);

	case 0x540:
		val=((DECO_PORT(0x530))&0xffff);
		val=((val&0x00f0)<<8) | ((val&0x0007)<<9) | ((val&0x0008)<<5);
		return val & (~deco16_mask);

	case 0x5c2:
		val=((DECO_PORT(0x7e0))&0xffff);
		val=((val&0xf000)>>12) | ((val&0x0ff0)<<4)| ((val&0x000c)<<2)| ((val&0x0003)<<6);
		return val;

	case 0x15c:
		val=((DECO_PORT(0x230))&0xffff);
		val=((val&0xff00)<<0) | ((val&0x000f)<<4) | ((val&0x00f0)>>4);
		return (val ^ deco16_xor);

	case 0x2c:
		val=((DECO_PORT(0x390))&0xffff);
		val=((val&0x00ff)<<8);
		return val & (~deco16_mask);
	}

	//logerror("Protection PC %06x: warning - read fully unmapped protection address %04x\n", activecpu_get_pc(), offset);

	return 0;
}

/**********************************************************************************/

WRITE32_HANDLER( deco16_146_fghthist_prot_w )
{
	UINT16 addr=BITSWAP16(offset<<1, 0, 0, 0, 0, 0, 10, 1, 9, 2, 8, 3, 7, 4, 6, 5, 0);

	decoprot_last_write=addr;
	decoprot_last_write_val=data>>16;

	deco16_146_core_prot_w(addr, data>>16, mem_mask>>16);
}

READ32_HANDLER( deco16_146_fghthist_prot_r )
{
	UINT16 addr=BITSWAP16(offset<<1, 0, 0, 0, 0, 0, 10, 1, 9, 2, 8, 3, 7, 4, 6, 5, 0);
	UINT16 val;

	/* Special case inputs, because this is the only game with an eprom */
	switch (addr)
	{
	case 0x582: return (readinputport(0)<<16) | 0xffff; /* IN0 */
	case 0x672: return (readinputport(1)<<16) | 0xffff; /* IN1 */
	case 0x04c: return (EEPROM_read_bit()<<16) | 0xffff;
	}

	/* Handle 'one shots' - writing data to an address, then immediately reading it back */
	if (decoprot_last_write==addr)
	{
		//logerror("Hit one shot for %04x (return %04x)\n", addr, decoprot_last_write_val);
		decoprot_last_write=-1;
		return (decoprot_last_write_val<<16)|0xffff;
	}
	decoprot_last_write=-1;

	val=deco16_146_core_prot_r(addr, mem_mask>>16);

	if (addr!=0x7b6 && addr!=0x1c && addr!=0x1e0 && addr!=0x1d4
		&& addr!=0x2c4 && addr!=0x7a4 && addr!=0x30 // confirmed
		&& addr!=0x49a && addr!=0x49c && addr!=0x584 // confirmed
		&& addr!=0x162 // confirmed
		&& addr!=0x1a0 && addr!=0x7f6 && addr!=0x18 // confirmed
		&& addr!=0x422 && addr!=0x794 // confirmed
		&& addr!=0xc0 && addr!=0x280 && addr!=0x1c0 && addr!=0xe2 // confirmed
		&& addr!=0x6c0  // not confirmed butnot read
		&& addr!=0x1ae && addr!=0x1d6 && addr!=0x4f8 && addr!=0x614 // cnofirmed
		&& addr!=0x5ae && addr!=0x50a && addr!=0x476 && addr!=0x328 && addr!=0x3e && addr!=0x558 // dbl check these later
		&& addr!=0x444 && addr!=0x46a // confirmed
		&& activecpu_get_pc()!=0x16448 // hmm
 		&& addr!=0x67a
		&& addr!=0x6c2 && addr!=0xac && addr!=0x416 && addr!=0x2c2 // confirmed
		&& addr!=0x3d8
		&& addr!=0x250 && addr!=0x306 && addr!=0x608 && addr!=0x52e // confirmed
		&& addr!=0x21e && addr!=0x7b0 && addr!=0x7da
		&& addr!=0xfe && addr!=0x504 && addr!=0x450  && addr!=0x276 // confirmed
		&& addr!=0x76 && addr!=0x714 && addr!=0x642 && addr!=0x7e8 && addr!=0x244 // confirmed
		&& addr!=0x2ea && addr!=0x254
		&& addr!=0x540 && addr!=0x5c2 // confirmed
		&& addr!=0x15c // confirmed
		&& addr!=0x80 && addr!=0xb2
		&& addr!=0x2c

		&& addr!=0x2e0  && addr!=0x350  && addr!=0x244  && addr!=0x2c4  && addr!=0xac  && addr!=0x416 // not handled at all

		// These addresses are read but the value is never used, and there are no side effects from reading
		// these addresses - seems to purely be some code obfustication
		&& addr!=0x400 && addr!=0x640 && addr!= 0x4c0 && addr!= 0x660 && addr!=0x4e0 && addr!=0x6e0 && addr!=0x448 && addr!=0x648 && addr!=0x4c8 && addr!=0x6c8 && addr!=0x468 && addr!=0x668 && addr!=0x4e8 && addr!=0x6e8 && addr!=0x442 && addr!=0x4c2 && addr!=0x462 && addr!=0x662
		&& addr!=0x4e2 && addr!=0x6e2 && addr!=0x44a && addr!=0x64a && addr!=0x4ca && addr!=0x6ca && addr!=0x66a && addr!=0x4ea && addr!=0x6ea
		&& addr!=0x440 && addr!=0x460
		)
	{
		logerror("Protection PC %06x: warning - read unmapped protection address %04x (ret %04x)\n", activecpu_get_pc(), addr, val);
		popmessage("Read protection port %04x", addr);
	}
	//  logerror("Protection PC %06x: warning - read unmapped protection address %04x (ret %04x)\n", activecpu_get_pc(), addr, val);

	return (val<<16)|0xffff;
}

WRITE16_HANDLER( deco16_146_nitroball_prot_w )
{
	UINT16 addr=BITSWAP16(offset<<1, 0, 0, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	deco16_146_core_prot_w(addr, data, mem_mask);
}

READ16_HANDLER( deco16_146_nitroball_prot_r )
{
	UINT16 addr=BITSWAP16(offset<<1, 0, 0, 0, 0, 0, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);

	return deco16_146_core_prot_r(addr, mem_mask);
}

/**********************************************************************************/

READ16_HANDLER( dietgo_104_prot_r )
{
	switch (offset * 2)
	{
	case 0x298: return input_port_0_word_r(0, 0);
	case 0x342: return input_port_1_word_r(0, 0);
	case 0x506: return input_port_2_word_r(0, 0);
	}

	logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset<<1);

	return 0;
}

WRITE16_HANDLER( dietgo_104_prot_w )
{
	if (offset==(0x380/2)) {
		soundlatch_w(0,data&0xff);
		cpunum_set_input_line(1,0,HOLD_LINE);
		return;
	}
	logerror("Protection PC %06x: warning - write unmapped memory address %04x %04x\n",activecpu_get_pc(),offset<<1,data);
}

/**********************************************************************************/

READ16_HANDLER( deco16_104_pktgaldx_prot_r )
{
	const UINT16* prot_ram=deco16_prot_ram;

	switch (offset * 2)
	{
	case 0x5b2: return input_port_0_word_r(0, 0);
	case 0x44c: return input_port_1_word_r(0, 0);
	case 0x042: return input_port_2_word_r(0, 0);

	case 0x510: return DECO_PORT(0);
	case 0x51a: return DECO_PORT(2);
	}

	logerror("Protection PC %06x: warning - read unmapped memory address %04x\n",activecpu_get_pc(),offset<<1);

	return 0;
}

WRITE16_HANDLER( deco16_104_pktgaldx_prot_w )
{
	COMBINE_DATA(&deco16_prot_ram[offset]);
//  logerror("Protection PC %06x: warning - write unmapped memory address %04x %04x\n",activecpu_get_pc(),offset<<1,data);
}

/**********************************************************************************/

