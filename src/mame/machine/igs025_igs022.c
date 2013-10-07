/* 

 IGS022 is an encrypted DMA device, most likely an MCU of some sort

 IGS025 is some kind of state machine / logic device which the game
 uses for various securit checks, and to determine the region of the
 game based on string sequences.

*/

#include "emu.h"
#include "igs025_igs022.h"


igs_025_022_device::igs_025_022_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock)
	: device_t(mconfig, IGS025022, "IGS025022", tag, owner, clock, "igs_025_022", __FILE__)
{
}

void igs_025_022_device::device_config_complete()
{
}

void igs_025_022_device::device_validity_check(validity_checker &valid) const
{
}

void igs_025_022_device::device_start()
{
	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;
	memset(m_kb_regs, 0, 0x100 * sizeof(UINT32));
	m_sharedprotram = 0;

	save_item(NAME(m_kb_prot_hold));
	save_item(NAME(m_kb_prot_hilo));
	save_item(NAME(m_kb_prot_hilo_select));
	save_item(NAME(m_kb_cmd));
	save_item(NAME(m_kb_reg));
	save_item(NAME(m_kb_ptr));
	save_item(NAME(m_kb_regs));
}

void igs_025_022_device::device_reset()
{
	if (!m_sharedprotram)
	{
		logerror("m_sharedprotram was not set\n");
		return;
	}

	IGS022_reset();

	// Reset IGS025 stuff
	m_kb_prot_hold = 0;
	m_kb_prot_hilo = 0;
	m_kb_prot_hilo_select = 0;
	m_kb_cmd = 0;
	m_kb_reg = 0;
	m_kb_ptr = 0;
	m_kb_swap = 0;
	memset(m_kb_regs, 0, 0x100 * sizeof(UINT32));
}



void igs_025_022_device::IGS022_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode)
{
	UINT16 param;
	/*
	P_SRC =0x300290 (offset from prot rom base)
	P_DST =0x300292 (words from 0x300000)
	P_SIZE=0x300294 (words)
	P_MODE=0x300296

	Mode 5 direct
	Mode 6 swap nibbles and bytes

	1,2,3 table based ops
	*/

	param = mode >> 8;
	mode &=0xf;  // what are the other bits?

	if ((mode == 0) || (mode == 1) || (mode == 2) || (mode == 3)  || (mode == 4))
	{
		/* mode3 applies a xor from a 0x100 byte table to the data being
		   transferred

		   the table is stored at the start of the protection rom.

		   the param used with the mode gives a start offset into the table

		   odd offsets cause an overflow
		*/

		int x;
		UINT16 *PROTROM = (UINT16*)memregion(":igs022data")->base();

		for (x = 0; x < size; x++)
		{
			UINT16 dat2 = PROTROM[src + x];

			UINT8 extraoffset = param&0xff;
			UINT8* dectable = (UINT8*)memregion(":igs022data")->base(); // the basic decryption table is at the start of the mcu data rom!
			UINT8 taboff = ((x*2)+extraoffset) & 0xff; // must allow for overflow in instances of odd offsets
			UINT16 extraxor = ((dectable[taboff+1]) << 8) | (dectable[taboff+0] << 0);

			if (mode==4)
			{
				extraxor = 0;
				if ((x & 0x003) == 0x000) extraxor |= 0x0049; // 'I'
				if ((x & 0x003) == 0x001) extraxor |= 0x0047; // 'G'
				if ((x & 0x003) == 0x002) extraxor |= 0x0053; // 'S'
				if ((x & 0x003) == 0x003) extraxor |= 0x0020; // ' '


				if ((x & 0x300) == 0x000) extraxor |= 0x4900; // 'I'
				if ((x & 0x300) == 0x100) extraxor |= 0x4700; // 'G'
				if ((x & 0x300) == 0x200) extraxor |= 0x5300; // 'S'
				if ((x & 0x300) == 0x300) extraxor |= 0x2000; // ' '
			}

			//  mode==0 plain
			if (mode==3) dat2 ^= extraxor;
			if (mode==2) dat2 += extraxor;
			if (mode==1) dat2 -= extraxor;

			if (mode==4)
			{
				printf("%06x | %04x (%04x)\n", (dst+x)*2, dat2, (UINT16)(dat2-extraxor));

				dat2 -= extraxor;
			}


			m_sharedprotram[dst + x] = dat2;
		}
	}
	else if (mode == 5)
	{
		/* mode 5 seems to be a straight copy, byteswap */
		int x;
		UINT16 *PROTROM = (UINT16*)memregion(":igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];

			m_sharedprotram[dst + x] = (dat << 8) | (dat >> 8);
		}
	}
	else if (mode == 6)
	{
		/* mode 6 seems to swap bytes and nibbles */
		int x;
		UINT16 *PROTROM = (UINT16*)memregion(":igs022data")->base();
		for (x = 0; x < size; x++)
		{
			UINT16 dat = PROTROM[src + x];

			dat = ((dat & 0xf000) >> 12)|
					((dat & 0x0f00) >> 4)|
					((dat & 0x00f0) << 4)|
					((dat & 0x000f) << 12);

			m_sharedprotram[dst + x] = dat;
		}
	}
	else if (mode == 7)
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		// not used by killing blade
		/* weird mode, the params get left in memory? - maybe it's a NOP? */
	}
	else
	{
		mame_printf_debug("unhandled copy mode %04x!\n", mode);
		logerror ("DMA MODE: %d, src: %4.4x, dst: %4.4x, size: %4.4x, param: %2.2x\n", mode, src, dst, size, param);
		// not used by killing blade
		/* invalid? */
	}
}

// the internal MCU boot code automatically does this DMA
// and puts the version # of the data rom in ram
void igs_025_022_device::IGS022_reset()
{
	int i;
	UINT16 *PROTROM = (UINT16*)memregion(":igs022data")->base();

	// fill ram with A5 patern
	for (i = 0; i < 0x4000/2; i++)
		m_sharedprotram[i] = 0xa55a;

	// the auto-dma
	UINT16 src  = PROTROM[0x100 / 2];
	UINT32 dst  = PROTROM[0x102 / 2];
	UINT16 size = PROTROM[0x104 / 2];
	UINT16 mode = PROTROM[0x106 / 2];

	mode &= 0xff;

	src >>= 1;

	IGS022_do_dma(src,dst,size,mode);

	// there is also a version ID? (or is it some kind of checksum) that is stored in the data rom, and gets copied..
	// Dragon World 3 checks it
	// Setting $3002a0 to #3 causes Dragon World 3 to skip this check
	m_sharedprotram[0x2a2/2] = PROTROM[0x114/2];
}

void igs_025_022_device::IGS022_handle_command()
{
	UINT16 cmd = m_sharedprotram[0x200/2];

	if (cmd == 0x6d)    // Store values to asic ram
	{
		UINT32 p1 = (m_sharedprotram[0x298/2] << 16) | m_sharedprotram[0x29a/2];
		UINT32 p2 = (m_sharedprotram[0x29c/2] << 16) | m_sharedprotram[0x29e/2];

		if ((p2 & 0xffff) == 0x9)   // Set value
		{
			int reg = (p2 >> 16) & 0xffff;

			if (reg & 0x300) { // 300?? killbld expects 0x200, drgw3 expects 0x100?
				m_kb_regs[reg & 0xff] = p1;
			}
		}

		if ((p2 & 0xffff) == 0x6)   // Add value
		{
			int src1 = (p1 >> 16) & 0xff;
			int src2 = (p1 >> 0) & 0xff;
			int dst = (p2 >> 16) & 0xff;

			m_kb_regs[dst] = m_kb_regs[src2] - m_kb_regs[src1];
		}

		if ((p2 & 0xffff) == 0x1)   // Add Imm?
		{
			int reg = (p2 >> 16) & 0xff;
			int imm = (p1 >> 0) & 0xffff;

			m_kb_regs[reg] += imm;
		}

		if ((p2 & 0xffff) == 0xa)   // Get value
		{
			int reg = (p1 >> 16) & 0xFF;

			m_sharedprotram[0x29c/2] = (m_kb_regs[reg] >> 16) & 0xffff;
			m_sharedprotram[0x29e/2] = m_kb_regs[reg] & 0xffff;
		}

		m_sharedprotram[0x202 / 2] = 0x7c;	// this mode complete?
	}

	// Is this actually what this is suppose to do? Complete guess.
	if (cmd == 0x12) // copy??
	{
		m_sharedprotram[0x28c / 2] = m_sharedprotram[0x288 / 2];
		m_sharedprotram[0x28e / 2] = m_sharedprotram[0x28a / 2];

		m_sharedprotram[0x202 / 2] = 0x23; 	// this mode complete?
	}

	// what do these do? write the completion byte for now...
	if (cmd == 0x45) m_sharedprotram[0x202 / 2] = 0x56;
	if (cmd == 0x5a) m_sharedprotram[0x202 / 2] = 0x4b;
	if (cmd == 0x2d) m_sharedprotram[0x202 / 2] = 0x3c;

	if (cmd == 0x4f) // memcpy with encryption / scrambling
	{
		UINT16 src  = m_sharedprotram[0x290 / 2] >> 1; // External mcu data is 8 bit and addressed as such
		UINT32 dst  = m_sharedprotram[0x292 / 2];
		UINT16 size = m_sharedprotram[0x294 / 2];
		UINT16 mode = m_sharedprotram[0x296 / 2];

		IGS022_do_dma(src,dst,size,mode);

		m_sharedprotram[0x202 / 2] = 0x5e;	// this mode complete?
	}
}

void igs_025_022_device::killbld_protection_calculate_hold(int y, int z)
{
	unsigned short old = m_kb_prot_hold;

	m_kb_prot_hold = ((old << 1) | (old >> 15));

	m_kb_prot_hold ^= 0x2bad;
	m_kb_prot_hold ^= BIT(z, y);
	m_kb_prot_hold ^= BIT( old,  7) <<  0;
	m_kb_prot_hold ^= BIT(~old, 13) <<  4;
	m_kb_prot_hold ^= BIT( old,  3) << 11;

	m_kb_prot_hold ^= (m_kb_prot_hilo & ~0x0408) << 1;
}

void igs_025_022_device::killbld_protection_calculate_hilo()
{
	UINT8 source;

	m_kb_prot_hilo_select++;

	if (m_kb_prot_hilo_select > 0xeb) {
		m_kb_prot_hilo_select = 0;
	}

	source = m_kb_source_data[(ioport(":Region")->read() - m_kb_source_data_offset)][m_kb_prot_hilo_select];

	if (m_kb_prot_hilo_select & 1)
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0x00ff) | (source << 8);
	}
	else
	{
		m_kb_prot_hilo = (m_kb_prot_hilo & 0xff00) | (source << 0);
	}
}

WRITE16_MEMBER(igs_025_022_device::killbld_igs025_prot_w )
{
	if (offset == 0)
	{
		m_kb_cmd = data;
	}
	else
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				m_kb_reg = data;
			break;

			case 0x01: // drgw3
			{
				if (data == 0x0002) { // Execute command
					IGS022_handle_command();		
				}
			}
			break;

			case 0x02: // killbld
			{
				if (data == 0x0001) { // Execute command
					IGS022_handle_command();
					m_kb_reg++;
				}
			}
			break;

			case 0x03:
				m_kb_swap = data;
			break;

			case 0x04:
		//		m_kb_ptr = data; // Suspect. Not good for drgw3
			break;

			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x26:
			case 0x27:
				m_kb_ptr++;
				killbld_protection_calculate_hold(m_kb_cmd & 0x0f, data & 0xff);
			break;

		//	default:
		//		logerror("%06X: ASIC25 W CMD %X  VAL %X\n", space.device().safe_pc(), m_kb_cmd, data);
		}
	}
}

READ16_MEMBER(igs_025_022_device::killbld_igs025_prot_r )
{
	if (offset)
	{
		switch (m_kb_cmd)
		{
			case 0x00:
				return BITSWAP8((m_kb_swap+1) & 0x7f, 0,1,2,3,4,5,6,7); // drgw3

			case 0x01:
				return m_kb_reg & 0x7f;

			case 0x05:
			{
				switch (m_kb_ptr)
				{
					case 1:
						return 0x3f00 | ioport(":Region")->read();

					case 2:
						return 0x3f00 | ((m_kb_game_id >> 8) & 0xff);

					case 3:
						return 0x3f00 | ((m_kb_game_id >> 16) & 0xff);

					case 4:
						return 0x3f00 | ((m_kb_game_id >> 24) & 0xff);

					default: // >= 5
						return 0x3f00 | BITSWAP8(m_kb_prot_hold, 5,2,9,7,10,13,12,15);
				}

				return 0;
			}

			case 0x40:
				killbld_protection_calculate_hilo();
				return 0; // Read and then discarded

		//	default:
		//		logerror("%06X: ASIC25 R CMD %X\n", space.device().safe_pc(), m_kb_cmd);
		}
	}

	return 0;
}




const device_type IGS025022 = &device_creator<igs_025_022_device>;



