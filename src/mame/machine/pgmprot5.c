/***********************************************************************
 PGM 012 + 025 PGM protection emulation

 these are simulations of the IGS 012 and 025 protection combination
 used on the following PGM games

 Dragon World 2

 ----

 IGS012 provides ROM overlay???

 IGS025 is some kind of state machine / logic device which the game
 uses for various security checks bitswap checks.

 To do:
   How is the additional xor data is calculated?
   Should the "hold" value be reset if the power is turned off? How?
   Patches are actually overlays or just hacks?

 ***********************************************************************/

#include "emu.h"
#include "includes/pgm.h"


/* Dragon World 2 */

READ16_MEMBER(pgm_state::dw2_d80000_r )
{
	UINT16 ret;
/*  UINT16 test;

    switch (dw2_asic_reg[0])
    {
        case 0x05: // Read to $80eec0
        case 0x13: // Read to $80eeb8
        case 0x1f: // Read to $80eeb8
        case 0x40: // Read to $80eeb8, increase counters
        case 0xf4: // Read to $80eeb8
        case 0xf6: // Read to $80eeb8
        case 0xf8: // Read to $80eeb8
        break;

        default:
            logerror("%06x: warning, reading with igs003_reg = %02x\n", space.device().safe_pc(), dw2_asic_reg[0]);
    }

    test = BITSWAP16(m_mainram[protection_address], 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff; // original hack
*/
	// This bitswap seems to also be common to a few IGS protection devices (igs011.c, Oriental Legend)
	ret = BITSWAP16(dw2_asic_hold, 14,11,8,6,4,3,1,0, 5,2,9,7,10,13,12,15) & 0xff;
/*
    if ((ret != test) || (dw2_asic_hold != m_mainram[protection_address])) {
        logerror ("Protection calculation error: SIMRET: %2.2x, HACKRET: %2.2x, SIMHOLD: %4.4x, REALHOLD: %4.4x\n", ret, test, dw2_asic_hold, m_mainram[protection_address]);
    }
*/
	return ret;
}

WRITE16_MEMBER(pgm_state::dw2_d80000_w )
{
	COMBINE_DATA(&dw2_asic_reg[offset]);

	if (offset == 0)
		return;

	switch (dw2_asic_reg[0])
	{
		case 0x08:
			// This reg doesn't seem to be used for anything useful but is
			// initialized. Ok to use as reset??
			// The last "hold" value used is stored in NVRAM. Otherwise we either
			// need to set the "hold" value as non-volatile or wipe NVRAM.
			dw2_asic_hold = m_mainram[protection_address]; // hack
		break;

		case 0x09: // Used only on init...
		case 0x0a:
		case 0x0b:
		case 0x0c:
		break;

		case 0x15: // ????
		case 0x17:
		break;

		case 0x20:
		case 0x21:
		case 0x22:
		case 0x23:
		case 0x24:
		case 0x25:
		case 0x26:
		case 0x27:
		{
			// computed ~$107000
			UINT16 old;

			dw2_asic_y = dw2_asic_reg[0] & 0x07;
			dw2_asic_z = data;

			// drgw2c = $80eecc, drgw2/dw2v100x = $80cb7c, drgw2j = $8091ca
			old = dw2_asic_hold;

			// drgw2c = $80eece, drgw2/dw2v100x = $80cb7e, drgw2j = $8091cc

			// roation, fixed xor, and z/y xor all also common to asic-type protection devices
			dw2_asic_hold  = old << 1;
			dw2_asic_hold |= BIT(old, 15); // rotate
			dw2_asic_hold ^= 0x2bad;
			dw2_asic_hold ^= BIT(dw2_asic_z, dw2_asic_y);

			dw2_asic_hold ^= BIT( old,  7);
			dw2_asic_hold ^= BIT( old,  3) << 11;
			dw2_asic_hold ^= BIT(~old, 13) <<  4; // inverted!

	/*
	        // additional...
	        // how is this calculated? is it ever used??
	        // drgw2c = $80eeca, drgw2/dw2v100x = $80cb7a, drgw2j = $809168
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 0) << 1;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 1) << 2;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 2) << 3;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 4) << 5;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 5) << 6;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 6) << 7;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eecb), 7) << 8;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 0) << 9;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 1) << 10;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 3) << 12;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 4) << 13;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 5) << 14;
	        dw2_asic_hold ^= BIT(space.read_byte(0x80eeca), 6) << 15;
	*/
		}
		break;

		case 0xf2: // ????
		break;

		default:
			logerror("%06x: warning, writing to igs003_reg %02x = %02x\n", space.device().safe_pc(), dw2_asic_reg[0], data);
	}
}

// What purpose to writes to this region serve? Written, but never read back? Must be related to the protection device?
WRITE16_MEMBER(pgm_state::dw2_unk_w)
{
//  logerror("%06x: warning, writing to address %6.6x = %4.4x\n", space.device().safe_pc(), 0xd00000+(offset*2), data);
}

void pgm_state::pgm_dw2_decrypt()
{
	int i;
	UINT16 *src = (UINT16 *) (memregion("maincpu")->base()+0x100000);

	int rom_size = 0x80000;

	for (i=0; i<rom_size/2; i++) {
		UINT16 x = src[i];

		if (((i & 0x20890) == 0) || ((i & 0x20000) == 0x20000 && (i & 0x01500) != 0x01400))
			x ^= 0x0002;

		if (((i & 0x20400) == 0 && (i & 0x02010) != 0x02010) || ((i & 0x20000) == 0x20000 && (i & 0x00148) != 0x00140))
			x ^= 0x0400;

		src[i] = x;
	}
}

void pgm_state::drgwld2_common_init()
{
	pgm_basic_init();
	pgm_dw2_decrypt();

	m_maincpu->space(AS_PROGRAM).install_read_handler(0xd80000, 0xd80003, read16_delegate(FUNC(pgm_state::dw2_d80000_r),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd80000, 0xd80003, write16_delegate(FUNC(pgm_state::dw2_d80000_w),this));
	m_maincpu->space(AS_PROGRAM).install_write_handler(0xd00000, 0xd00fff, write16_delegate(FUNC(pgm_state::dw2_unk_w),this));
}

DRIVER_INIT_MEMBER(pgm_state,drgw2)
{
	/* incomplete? */
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();

	drgwld2_common_init();

	protection_address = 0xcb7e/2; // $80cb7e;

	/* These ROM patches are not hacks, the protection device
	   overlays the normal ROM code, this has been confirmed on a real PCB
	   although some addresses may be missing */
	mem16[0x131098 / 2] = 0x4e93;
	mem16[0x13113e / 2] = 0x4e93;
	mem16[0x1311ce / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,dw2v100x)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();

	drgwld2_common_init();

	protection_address = 0xcb7e/2; // $80cb7e;

	/* These ROM patches are not hacks, the protection device
	   overlays the normal ROM code, this has been confirmed on a real PCB
	   although some addresses may be missing */
	mem16[0x131084 / 2] = 0x4e93;
	mem16[0x13112a / 2] = 0x4e93;
	mem16[0x1311ba / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,drgw2c)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();

	drgwld2_common_init();

	protection_address = 0xeece/2; // $80eece;

	/* These ROM patches are not hacks, the protection device
	   overlays the normal ROM code, this has been confirmed on a real PCB
	   although some addresses may be missing */
	mem16[0x1303bc / 2] = 0x4e93;
	mem16[0x130462 / 2] = 0x4e93;
	mem16[0x1304f2 / 2] = 0x4e93;
}

DRIVER_INIT_MEMBER(pgm_state,drgw2j)
{
	UINT16 *mem16 = (UINT16 *)memregion("maincpu")->base();

	drgwld2_common_init();

	protection_address = 0x91cc/2; // $8091cc;

	/* These ROM patches are not hacks, the protection device
	   overlays the normal ROM code, this has been confirmed on a real PCB
	   although some addresses may be missing */
	mem16[0x1302c0 / 2] = 0x4e93;
	mem16[0x130366 / 2] = 0x4e93;
	mem16[0x1303f6 / 2] = 0x4e93;
}
