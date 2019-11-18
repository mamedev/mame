// license:BSD-3-Clause
// copyright-holders:R. Belmont

/*
 * An emulation of the Digital Equipment Corporation SFB "Smart Frame Buffer" chip
 *
 * Used in:
 *
 *   Accelerated TURBOChannel video cards for DECstations and AlphaStations
 *   On-board on some DECstations
 *   On-board on many AlphaStations
 *
 * Sources:
 *
 * http://www.hpl.hp.com/techreports/Compaq-DEC/WRL-93-1.pdf
 *
 */

#include "emu.h"
#include "decsfb.h"

#define MODE_SIMPLE     0
#define MODE_OPAQUESTIPPLE  1
#define MODE_OPAQUELINE     2
#define MODE_TRANSPARENTSTIPPLE 5
#define MODE_TRANSPARENTLINE    6
#define MODE_COPY       7

#define LOG_GENERAL (1U << 0)
#define LOG_REG     (1U << 1)
#define LOG_IRQ     (1U << 2)

//#define VERBOSE (LOG_GENERAL|LOG_REG||LOG_IRQ)

#include "logmacro.h"

DEFINE_DEVICE_TYPE(DECSFB, decsfb_device, "decsfb", "Digital Equipment Corporation Smart Frame Buffer")

decsfb_device::decsfb_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, DECSFB, tag, owner, clock),
	m_int_cb(*this)
{
}

void decsfb_device::device_start()
{
	m_int_cb.resolve_safe();

	save_item(NAME(m_vram));
	save_item(NAME(m_regs));
	save_item(NAME(m_copy_src));
}

void decsfb_device::device_reset()
{
	m_copy_src = 1;
}

/*
    0x100000 copy register 0
    0x100004 copy register 1
    0x100008 copy register 2
    0x10000C copy register 3
    0x100010 copy register 4
    0x100014 copy register 5
    0x100018 copy register 6
    0x10001C copy register 7
    0x100020 foreground register
    0x100024 background register
    0x100028 plane mask
    0x10002C pixel mask
    0x100030 cxt mode
    0x100034 boolean operation
    0x100038 pixel shift
    0x10003C line address
    0x100040 bresh 1
    0x100044 bresh 2
    0x100048 bresh 3
    0x10004C bresh continue
    0x100050 deep register
    0x100054 start register
    0x100058 Clear Interrupt
    0x10005C reserved 2
    0x100060 refresh count
    0x100064 video horiz
    0x100068 video vertical
    0x10006C refresh base
    0x100070 video valid
    0x100074 Interrupt Enable
*/

READ32_MEMBER(decsfb_device::read)
{
	return m_regs[offset];
}

WRITE32_MEMBER(decsfb_device::write)
{
	COMBINE_DATA(&m_regs[offset]);

	if ((offset == (0x30/4)) && (data = 7))
	{
		m_copy_src = 1;
	}
}

READ32_MEMBER(decsfb_device::vram_r)
{
	return m_vram[offset];
}

WRITE32_MEMBER(decsfb_device::vram_w)
{
	switch (m_regs[0x30/4])
	{
		case MODE_SIMPLE:   // simple
			COMBINE_DATA(&m_vram[offset]);
			break;

		case MODE_TRANSPARENTSTIPPLE:
			{
				uint8_t *pVRAM = (uint8_t *)&m_vram[offset];
				uint8_t fgs[4];

				fgs[0] = m_regs[0x20/4] >> 24;
				fgs[1] = (m_regs[0x20/4] >> 16) & 0xff;
				fgs[2] = (m_regs[0x20/4] >> 8) & 0xff;
				fgs[3] = m_regs[0x20/4] & 0xff;
				for (int x = 0; x < 32; x++)
				{
					if (data & (1<<(31-x)))
					{
						pVRAM[x] = fgs[x & 3];
					}
				}
			}
			break;

		case MODE_COPY:
			{
				uint8_t *pVRAM = (uint8_t *)&m_vram[offset];
				uint8_t *pBuffer = (uint8_t *)&m_regs[0];    // first 8 32-bit regs are the copy buffer

				if (m_copy_src)
				{
					m_copy_src = 0;

					for (int x = 0; x < 32; x++)
					{
						if (data & (1<<(31-x)))
						{
							pBuffer[x] = pVRAM[x];
						}
					}
				}
				else
				{
					m_copy_src = 1;

					for (int x = 0; x < 32; x++)
					{
						if (data & (1<<(31-x)))
						{
							pVRAM[x] = pBuffer[x];
						}
					}
				}
			}
			break;

		default:
			logerror("SFB: Unsupported VRAM write %08x (mask %08x) at %08x in mode %x\n", data, mem_mask, offset<<2, m_regs[0x30/4]);
			break;
	}
}
