// license:BSD-3-Clause
// copyright-holders:Darren Olafson, Quench
/***************************************************************************

Toaplan Video controller

named as 'SCU' at outzone schematics (do not confused with
'SCU' sprite generator (toaplan/toaplan_scu.cpp))

Functions are driven with NEC Gate array. (part name differs
per hardware)

Features:
- Palette RAM address generator (Up to 8Kwords, 4Kwords per layer)
- Input 2 pixel data and mix them
- Video interrupt generator
- Video timing/sync generator (not emulated yet)

Used at:
- toaplan/toaplan1.cpp
- toaplan/rallybik.cpp
- toaplan/fireshrk.cpp

Input pixel format:

Bit                 Description
fedc ba98 7654 3210
xxxx ---- ---- ---- Priority*
---- xxxx xxxx xxxx Color**

* 0 = invisible, 1...15 = backmost to frontmost
** (10 bit only in currently emulated hardwares)

Palette RAM address format:

Bit                 Description
fedc ba98 7654 3210
---x ---- ---- ---- Layer*
---- xxxx xxxx xxxx Color**

0 = Background, 1 = Foreground
** (10 bit only in currently emulated hardwares)

Video timing register format:

Word Bit                 Description
     fedc ba98 7654 3210
0    ---- ---- xxxx xxxx Horizontal total (Added by 1 then multiplied by 2)
1    ---- ---- ---- ----
2    ---- ---- xxxx xxxx Vertical total (Added by 1 then multiplied by 2)
3    ---- ---- ---- ----

* Unmarked bits are used but unknown

TODO:
- Verify video timing parameter values
- Support dynamic refresh rate

***************************************************************************/


#include "emu.h"
#include "toaplan_video_controller.h"
#include "screen.h"

#define LOG_REGS (1 << 1)

#define VERBOSE (0)

#include "logmacro.h"

#define LOGREGS(...) LOGMASKED(LOG_REGS, __VA_ARGS__)

// host interface
void toaplan_video_controller_device::host_map(address_map &map)
{
	map(0x0000, 0x0001).lr16(NAME([this]() { return m_vblank_cb(); }));
//  map(0x0000, 0x0001).w(?? video frame related ??)
	map(0x0003, 0x0003).w(FUNC(toaplan_video_controller_device::intenable_w));
	map(0x0008, 0x000f).w(FUNC(toaplan_video_controller_device::vtiming_w)).share(m_vtiming);
	map(0x4000, 0x5fff).rw(FUNC(toaplan_video_controller_device::palette_r<0>), FUNC(toaplan_video_controller_device::palette_w<0>));
	map(0x6000, 0x7fff).rw(FUNC(toaplan_video_controller_device::palette_r<1>), FUNC(toaplan_video_controller_device::palette_w<1>));
}

DEFINE_DEVICE_TYPE(TOAPLAN_VIDEO_CONTROLLER, toaplan_video_controller_device, "toaplan_video_controller", "Toaplan Video Controller")

toaplan_video_controller_device::toaplan_video_controller_device(const machine_config &mconfig, const char *tag, device_t *owner, u32 clock)
	: device_t(mconfig, TOAPLAN_VIDEO_CONTROLLER, tag, owner, clock)
	, device_video_interface(mconfig, *this)
	, m_palette(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_paletteram(*this, {finder_base::DUMMY_TAG, finder_base::DUMMY_TAG})
	, m_vtiming(*this, "vtiming")
	, m_vblank_cb(*this, 0)
	, m_byte_per_color{2}
	, m_intenable(0)
{
}

void toaplan_video_controller_device::device_start()
{
	for (int i = 0; i < 2; i++)
	{
		assert(!(m_paletteram[i].length() & (m_paletteram[i].length() - 1)));

		m_palette[i]->basemem().set(&m_paletteram[i][0], m_paletteram[i].bytes(), 16, ENDIANNESS_BIG, m_byte_per_color[i]);
	}

	save_item(NAME(m_intenable));
}

void toaplan_video_controller_device::device_reset()
{
	m_intenable = 0;
}


void toaplan_video_controller_device::intenable_w(offs_t offset, u8 data)
{
	m_intenable = data;
}

void toaplan_video_controller_device::vtiming_w(offs_t offset, u16 data, u16 mem_mask)
{
	LOGREGS("%s: Video timing register:%02x now = %04x & %04x\n", machine().describe_context(), offset, data, mem_mask);
	COMBINE_DATA(&m_vtiming[offset]);
}

template <unsigned Layer>
u16 toaplan_video_controller_device::palette_r(offs_t offset)
{
	return m_paletteram[Layer][offset & (m_paletteram[Layer].length() - 1)];
}

template <unsigned Layer>
void toaplan_video_controller_device::palette_w(offs_t offset, u16 data, u16 mem_mask)
{
	data = COMBINE_DATA(&m_paletteram[Layer][offset & (m_paletteram[Layer].length() - 1)]);
	m_palette[Layer]->write16(offset, data);
}
