// license:BSD-3-Clause
// copyright-holders:Alex W. Jackson
/*
Namco Custom 116, used in:
System 1
System 2
NB-1/NB-2
System FL

C116 controls palette RAM, blanking/clipping, and generates raster IRQs.
It is paired with one of two priority mixer chips, depending on the board.
On System 1, its partner is Custom 120; on System 2, NB-1, NB-2, and System
FL, its partner is Custom 156. Based on schematics, C156 has more color input
pins than C120 in order to support sprites with higher bit depth; other
differences between the two mixer chips are unknown.

The mixer (C120 or C156) outputs a 13-bit address corresponding to the color
index of the highest-priority input pixel to three 6264 SRAMs, one for each of
red, green and blue. The data from the RAMs is output to C116, which either
sends it to the DAC or clips it to black if the beam position is outside the
visible area programmed via its registers.

When accessing the palette RAM from the CPU, data lines D0-D7 and address lines
A11 and A12 go to C116; these two address lines select between the three RAMs
and the C116 internal registers. A0-A10, A13 and A14 go to C120 or C156, which
simply pass them through to the RAMs (A13 becoming A11 and A14 becoming A12).
Thus, the palette RAM and the C116 registers are laid out like this from the
perspective of the CPU:

0000-07FF: Red   (first 2048 pens)
0800-0FFF: Green ("")
1000-17FF: Blue  ("")
1800-1FFF: C116 registers
2000-27FF: Red   (second 2048 pens)
2800-2FFF: Green ("")
3000-37FF: Blue  ("")
3800-3FFF: C116 registers (mirror)
4000-47FF: Red   (third 2048 pens)
4800-4FFF: Green ("")
5000-57FF: Blue  ("")
5800-5FFF: C116 registers (mirror)
6000-67FF: Red (last 2048 pens)
6800-6FFF: Green ("")
7000-77FF: Blue  ("")
7800-7FFF: C116 registers (mirror)

C116 has six (or eight?) 16-bit registers:

00-01: left clip
02-03: right clip
04-05: top clip
06-07: bottom clip
08-09: raster IRQ horizontal position?
0A-0B: raster IRQ scanline
0C-0D: unknown, some games write 0 here, but probably unused
0E-0F: ""

The registers are mirrored every 0x10 bytes within the address ranges
indicated above.

Although the registers are logically 16-bit, the chip's external interface
is 8-bit, so the registers are written a byte at a time and in a big-endian
fashion (even addresses access the MSB and odd addresses access the LSB)
regardless of the endianness of the CPU. Thus System FL, which has an Intel
i960 CPU, needs to write its clip and raster values byteswapped.
*/

#include "emu.h"
#include "video/namco_c116.h"

#include <algorithm>

DEFINE_DEVICE_TYPE(NAMCO_C116, namco_c116_device, "namco_c116", "Namco C116 Video Controller")

//-------------------------------------------------
//  namco_c116_device -- constructor
//-------------------------------------------------

namco_c116_device::namco_c116_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, NAMCO_C116, tag, owner, clock)
	, device_palette_interface(mconfig, *this)
	, device_video_interface(mconfig, *this)
	, m_enable_shadows(false)
{
}


//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void namco_c116_device::device_start()
{
	m_ram_r.resize(0x2000);
	m_ram_g.resize(0x2000);
	m_ram_b.resize(0x2000);
	std::fill(std::begin(m_regs), std::end(m_regs), 0);

	save_item(NAME(m_ram_r));
	save_item(NAME(m_ram_g));
	save_item(NAME(m_ram_b));
	save_item(NAME(m_regs));
}


READ8_MEMBER(namco_c116_device::read)
{
	uint8_t *RAM;

	switch (offset & 0x1800)
	{
		case 0x0000:
			RAM = &m_ram_r[0];
			break;
		case 0x0800:
			RAM = &m_ram_g[0];
			break;
		case 0x1000:
			RAM = &m_ram_b[0];
			break;
		default: // case 0x1800 (internal registers)
		{
			int reg = (offset & 0xf) >> 1;
			if (offset & 1)
				return m_regs[reg] & 0xff;
			else
				return m_regs[reg] >> 8;
		/* registers 6,7: unmapped? */
		//if (reg > 0x6) return 0xff;
		}
	}

	return RAM[((offset & 0x6000) >> 2) | (offset & 0x7ff)];
}


WRITE8_MEMBER(namco_c116_device::write)
{
	uint8_t *RAM;

	switch (offset & 0x1800)
	{
		case 0x0000:
			RAM = &m_ram_r[0];
			break;
		case 0x0800:
			RAM = &m_ram_g[0];
			break;
		case 0x1000:
			RAM = &m_ram_b[0];
			break;
		default: // case 0x1800 (internal registers)
		{   /* notes from namcos2.cpp */
			/* registers 0-3: clipping */

			/* register 4: ? */
			/* sets using it:
			assault:    $0020
			burnforc:   $0130 after titlescreen
			dirtfoxj:   $0108 at game start
			finalap1/2/3:   $00C0
			finehour:   $0168 after titlescreen
			fourtrax:   $00E8 and $00F0
			luckywld:   $00E8 at titlescreen, $00A0 in game and $0118 if in tunnel
			suzuka8h1/2:    $00E8 and $00A0 */

			/* register 5: POSIRQ scanline (only 8 bits used) */

			/* registers 6,7: nothing? */
			int reg = (offset & 0xf) >> 1;
			if (offset & 1)
				m_regs[reg] = (m_regs[reg] & 0xff00) | data;
			else
				m_regs[reg] = (m_regs[reg] & 0x00ff) | (data << 8);
			//logerror("reg%d = %d\n", reg, m_regs[reg]);
			return;
		}
	}
	int color = ((offset & 0x6000) >> 2) | (offset & 0x7ff);
	RAM[color] = data;
	set_pen_color(color,m_ram_r[color],m_ram_g[color],m_ram_b[color]);
}
