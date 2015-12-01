// license:BSD-3-Clause
// copyright-holders:S. Smith,David Haywood


#include "emu.h"
#include "sbp_prot.h"



extern const device_type SBP_PROT = &device_creator<sbp_prot_device>;


sbp_prot_device::sbp_prot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, SBP_PROT, "NeoGeo Protection (Super Bubble Pop)", tag, owner, clock, "sbp_prot", __FILE__),
	m_mainrom(NULL)
{
}


void sbp_prot_device::device_start()
{
}

void sbp_prot_device::device_reset()
{
}



READ16_MEMBER( sbp_prot_device::sbp_lowerrom_r )
{
	UINT16* rom = (UINT16*)m_mainrom;
	UINT16 origdata = rom[(offset+(0x200/2))];
	UINT16 data =  BITSWAP16(origdata, 11,10,9,8,15,14,13,12,3,2,1,0,7,6,5,4);
	int realoffset = 0x200+(offset*2);
	logerror("sbp_lowerrom_r offset %08x data %04x\n", realoffset, data );

	// there is actually data in the rom here already, maybe we should just return it 'as is'
	if (realoffset==0xd5e) return origdata;

	return data;
}

WRITE16_MEMBER( sbp_prot_device::sbp_lowerrom_w )
{
	int realoffset = 0x200+(offset*2);

	// the actual data written is just pulled from the end of the rom, and unused space
	// maybe this is just some kind of watchdog for the protection device and it doesn't
	// matter?
	if (realoffset == 0x1080)
	{
		if (data==0x4e75)
		{
			return;
		}
		else if (data==0xffff)
		{
			return;
		}
	}

	printf("sbp_lowerrom_w offset %08x data %04x\n", realoffset, data );
}


void sbp_prot_device::sbp_install_protection(cpu_device* maincpu, UINT8* cpurom, UINT32 cpurom_size)
{
	m_mainrom = cpurom;

	// there seems to be a protection device living around here..
	// if you nibble swap the data in the rom the game will boot
	// there are also writes to 0x1080..
	//
	// other stuff going on as well tho, the main overlay is still missing, and p1 inputs don't work
	maincpu->space(AS_PROGRAM).install_read_handler(0x00200, 0x001fff, read16_delegate(FUNC(sbp_prot_device::sbp_lowerrom_r), this));
	maincpu->space(AS_PROGRAM).install_write_handler(0x00200, 0x001fff, write16_delegate(FUNC(sbp_prot_device::sbp_lowerrom_w), this));

	/* the game code clears the text overlay used ingame immediately after writing it.. why? protection? sloppy code that the hw ignores? imperfect emulation? */
	{
		UINT16* rom = (UINT16*)cpurom;

		rom[0x2a6f8 / 2] = 0x4e71;
		rom[0x2a6fa / 2] = 0x4e71;
		rom[0x2a6fc / 2] = 0x4e71;
	}
}
