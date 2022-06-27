// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

IREM "M72" sound hardware

All games have a YM2151 for music, and most of them also samples. Samples
are not handled consistently by all the games, some use a high frequency NMI
handler to push them through a DAC, others use external hardware.
In the following table, the NMI column indicates with a No the games whose
NMI handler only consists of RETN. R-Type is an exception, it doesn't have
a valid NMI handler at all.

Game                                    Year  ID string     NMI
--------------------------------------  ----  ------------  ---
R-Type                                  1987  - (earlier version, no samples)
Battle Chopper / Mr. Heli               1987  Rev 2.20      Yes
Vigilante                               1988  Rev 2.20      Yes
Ninja Spirit                            1988  Rev 2.20      Yes
Image Fight                             1988  Rev 2.20      Yes
Legend of Hero Tonma                    1989  Rev 2.20      Yes
X Multiply                              1989  Rev 2.20      Yes
Dragon Breed                            1989  Rev 2.20      Yes
Kickle Cubicle                          1988  Rev 2.21      Yes
Shisensho                               1989  Rev 2.21      Yes
R-Type II                               1989  Rev 2.21      Yes
Major Title                             1990  Rev 2.21      Yes
Air Duel                                1990  Rev 3.14 M72   No
Daiku no Gensan                         1990  Rev 3.14 M81  Yes
Daiku no Gensan (M72)                   1990  Rev 3.15 M72   No
Hammerin' Harry                         1990  Rev 3.15 M81  Yes
Ken-Go                                  1991  Rev 3.15 M81  Yes
Pound for Pound                         1990  Rev 3.15 M83   No
Cosmic Cop                              1991  Rev 3.15 M81  Yes
Gallop - Armed Police Unit              1991  Rev 3.15 M72   No
Hasamu                                  1991  Rev 3.15 M81  Yes
Bomber Man                              1991  Rev 3.15 M81  Yes
Bomber Man World (Japan)                1992  Rev 3.31 M81  Yes
Bomber Man World (World) / Atomic Punk  1992  Rev 3.31 M99   No
Quiz F-1 1,2finish                      1992  Rev 3.33 M81  Yes
Risky Challenge                         1993  Rev 3.34 M81  Yes
Shisensho II                            1993  Rev 3.34 M81  Yes

***************************************************************************/

#include "emu.h"
#include "m72_a.h"


DEFINE_DEVICE_TYPE(IREM_M72_AUDIO, m72_audio_device, "m72_audio", "Irem M72 Audio")

m72_audio_device::m72_audio_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: device_t(mconfig, IREM_M72_AUDIO, tag, owner, clock)
	, device_rom_interface(mconfig, *this)
	, m_sample_addr(0)
	, m_dac(*this, finder_base::DUMMY_TAG)
{
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void m72_audio_device::device_start()
{
	save_item(NAME(m_sample_addr));
}

//-------------------------------------------------
//  rom_bank_updated - the rom bank has changed
//-------------------------------------------------

void m72_audio_device::rom_bank_updated()
{
}

void m72_audio_device::set_sample_start(int start)
{
	m_sample_addr = start;
}

void m72_audio_device::vigilant_sample_addr_w(offs_t offset, u8 data)
{
	if (offset == 1)
		m_sample_addr = (m_sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		m_sample_addr = (m_sample_addr & 0xff00) | ((data << 0) & 0x00ff);
}

void m72_audio_device::shisen_sample_addr_w(offs_t offset, u8 data)
{
	m_sample_addr >>= 2;

	if (offset == 1)
		m_sample_addr = (m_sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		m_sample_addr = (m_sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	m_sample_addr <<= 2;
}

void m72_audio_device::rtype2_sample_addr_w(offs_t offset, u8 data)
{
	m_sample_addr >>= 5;

	if (offset == 1)
		m_sample_addr = (m_sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		m_sample_addr = (m_sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	m_sample_addr <<= 5;
}

void m72_audio_device::poundfor_sample_addr_w(offs_t offset, u8 data)
{
	/* poundfor writes both sample start and sample END - a first for Irem...
	   we don't handle the end written here, 00 marks the sample end as usual. */
	if (offset > 1) return;

	m_sample_addr >>= 4;

	if (offset == 1)
		m_sample_addr = (m_sample_addr & 0x00ff) | ((data << 8) & 0xff00);
	else
		m_sample_addr = (m_sample_addr & 0xff00) | ((data << 0) & 0x00ff);

	m_sample_addr <<= 4;
}

u8 m72_audio_device::sample_r()
{
	return read_byte(m_sample_addr);
}

void m72_audio_device::sample_w(u8 data)
{
	m_dac->write(data);
	m_sample_addr++;
}
