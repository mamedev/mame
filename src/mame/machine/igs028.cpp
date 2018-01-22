// license:BSD-3-Clause
// copyright-holders:David Haywood, ElSemi
/* IGS 028 */

// this seems to be very similar to the igs022 - encrypted DMA + some other ops with shared RAM
// used by
// Oriental Legend Super / Special


#include "emu.h"
#include "igs028.h"


igs028_device::igs028_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IGS028, tag, owner, clock)
{
}

void igs028_device::device_start()
{
	m_sharedprotram = nullptr;


}

void igs028_device::device_reset()
{
	//printf("igs028_device::device_reset()");


	if (!m_sharedprotram)
	{
		logerror("m_sharedprotram was not set\n");
		return;
	}

//  written by protection device
//  there seems to be an auto-dma that writes from $401000-402573?
	m_sharedprotram[0x1000/2] = 0x4749; // 'IGS.28'
	m_sharedprotram[0x1002/2] = 0x2E53;
	m_sharedprotram[0x1004/2] = 0x3832;

	m_sharedprotram[0x3064/2] = 0xB315; // crc?

}


uint32_t igs028_device::olds_prot_addr(uint16_t addr)
{
	switch (addr & 0xff)
	{
		case 0x0:
		case 0x5:
		case 0xa: return 0x402a00 + ((addr >> 8) << 2);
		case 0x2:
		case 0x8: return 0x402e00 + ((addr >> 8) << 2);
		case 0x1: return 0x40307e;
		case 0x3: return 0x403090;
		case 0x4: return 0x40309a;
		case 0x6: return 0x4030a4;
		case 0x7: return 0x403000;
		case 0x9: return 0x40306e;
		case 0xb: return 0x403044;
	}

	return 0;
}

uint32_t igs028_device::olds_read_reg(uint16_t addr)
{
	uint32_t protaddr = (olds_prot_addr(addr) - 0x400000) / 2;
	return m_sharedprotram[protaddr] << 16 | m_sharedprotram[protaddr + 1];
}

void igs028_device::olds_write_reg( uint16_t addr, uint32_t val )
{
	m_sharedprotram[((olds_prot_addr(addr) - 0x400000) / 2) + 0] = val >> 16;
	m_sharedprotram[((olds_prot_addr(addr) - 0x400000) / 2) + 1] = val & 0xffff;
}

void igs028_device::IGS028_do_dma(uint16_t src, uint16_t dst, uint16_t size, uint16_t mode)
{
	uint16_t param = mode >> 8;
	uint16_t *PROTROM = (uint16_t*)memregion(":user1")->base();

//  logerror ("mode: %2.2x, src: %4.4x, dst: %4.4x, size: %4.4x, data: %4.4x\n", (mode &0xf), src, dst, size, mode);

	mode &= 0x0f;

	switch (mode & 0x7)
	{
		// igs022 has an 'IGS ' encryption mode, a plain copy, and a NOP, these aren't covered at the moment..

		case 0x00: // -= encryption
		case 0x01: // swap nibbles
		case 0x02: // ^= encryption

		case 0x03: // unused?
		case 0x04: // unused?

		case 0x05: // swap bytes
		case 0x06: // += encryption (correct?)

		case 0x07: // unused?
		{
			uint8_t extraoffset = param & 0xff;
			uint8_t *dectable = (uint8_t *)(PROTROM + (0x100 / 2));

			for (int32_t x = 0; x < size; x++)
			{
				uint16_t dat2 = PROTROM[src + x];

				int taboff = ((x*2)+extraoffset) & 0xff; // must allow for overflow in instances of odd offsets
				unsigned short extraxor = ((dectable[taboff + 0]) << 0) | (dectable[taboff + 1] << 8);

				if (mode==0) dat2 -= extraxor;
				else if (mode==1) dat2  = ((dat2 & 0xf0f0) >> 4)|((dat2 & 0x0f0f) << 4);
				else if (mode==2) dat2 ^= extraxor;
				else if (mode==5) dat2  = ((dat2 &0x00ff) << 8) | ((dat2 &0xff00) >> 8);
				else if (mode==6) dat2 += extraxor;
				else
				{
					// if other modes are used we need to know about them
					uint16_t extraxor2 = 0;
					if ((x & 0x003) == 0x000) extraxor2 |= 0x0049; // 'I'
					if ((x & 0x003) == 0x001) extraxor2 |= 0x0047; // 'G'
					if ((x & 0x003) == 0x002) extraxor2 |= 0x0053; // 'S'
					if ((x & 0x003) == 0x003) extraxor2 |= 0x0020; // ' '


					if ((x & 0x300) == 0x000) extraxor2 |= 0x4900; // 'I'
					if ((x & 0x300) == 0x100) extraxor2 |= 0x4700; // 'G'
					if ((x & 0x300) == 0x200) extraxor2 |= 0x5300; // 'S'
					if ((x & 0x300) == 0x300) extraxor2 |= 0x2000; // ' '


					printf("mode %d - %04x (%04x %04x %04x - %04x %04x %04x - %04x %04x \n", mode, dat2, (uint16_t)(dat2-extraxor), (uint16_t)(dat2+extraxor), (uint16_t)(dat2^extraxor), (uint16_t)(dat2-extraxor2), (uint16_t)(dat2+extraxor2), (uint16_t)(dat2^extraxor2), ((dat2 & 0xf0f0) >> 4)|((dat2 & 0x0f0f) << 4), ((dat2 &0x00ff) << 8) | ((dat2 &0xff00) >> 8) );
					dat2 = 0x4e75; // hack
				}

				m_sharedprotram[dst + x] = dat2;
			}
		}
		break;

		default: // >=8
			printf ("DMA mode unknown!!!\nsrc:%4.4x, dst: %4.4x, size: %4.4x, mode: %4.4x\n", src, dst, size, mode);
	}
}

void igs028_device::IGS028_handle()
{
	uint16_t cmd = m_sharedprotram[0x3026 / 2];

	//  logerror ("command: %x\n", cmd);

	switch (cmd)
	{
		case 0x12:
		{
			uint16_t mode = m_sharedprotram[0x303e / 2];  // ?
			uint16_t src  = m_sharedprotram[0x306a / 2] >> 1; // ?
			uint16_t dst  = m_sharedprotram[0x3084 / 2] & 0x1fff;
			uint16_t size = m_sharedprotram[0x30a2 / 2] & 0x1fff;

			IGS028_do_dma(src, dst, size, mode);
		}
		break;

		case 0x64: // incomplete?
		{
				uint16_t p1 = m_sharedprotram[0x3050 / 2];
				uint16_t p2 = m_sharedprotram[0x3082 / 2];
				uint16_t p3 = m_sharedprotram[0x3054 / 2];
				uint16_t p4 = m_sharedprotram[0x3088 / 2];

				if (p2  == 0x02)
						olds_write_reg(p1, olds_read_reg(p1) + 0x10000);

				switch (p4)
				{
						case 0xd:
								olds_write_reg(p1,olds_read_reg(p3));
								break;
						case 0x0:
								olds_write_reg(p3,(olds_read_reg(p2))^(olds_read_reg(p1)));
								break;
						case 0xe:
								olds_write_reg(p3,olds_read_reg(p3)+0x10000);
								break;
						case 0x2:
								olds_write_reg(p1,(olds_read_reg(p2))+(olds_read_reg(p3)));
								break;
						case 0x6:
								olds_write_reg(p3,(olds_read_reg(p2))&(olds_read_reg(p1)));
								break;
						case 0x1:
								olds_write_reg(p2,olds_read_reg(p1)+0x10000);
								break;
						case 0x7:
								olds_write_reg(p3,olds_read_reg(p1));
								break;
						default:
								break;
				}
		}
		break;

	//  default:
	//      logerror ("unemulated command!\n");
	}
}


DEFINE_DEVICE_TYPE(IGS028, igs028_device, "igs028", "IGS028")
