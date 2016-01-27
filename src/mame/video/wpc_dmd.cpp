// license:BSD-3-Clause
// copyright-holders:Olivier Galibert
#include "emu.h"
#include "wpc_dmd.h"
#include "rendlay.h"

const device_type WPC_DMD = &device_creator<wpc_dmd_device>;

DEVICE_ADDRESS_MAP_START( registers, 8, wpc_dmd_device )
	AM_RANGE(0, 0) AM_WRITE(bank2_w)
	AM_RANGE(1, 1) AM_WRITE(bank0_w)
	AM_RANGE(2, 2) AM_WRITE(bank6_w)
	AM_RANGE(3, 3) AM_WRITE(bank4_w)
	AM_RANGE(4, 4) AM_WRITE(banka_w)
	AM_RANGE(5, 5) AM_WRITE(firq_scanline_w)
	AM_RANGE(6, 6) AM_WRITE(bank8_w)
	AM_RANGE(7, 7) AM_WRITE(visible_page_w)
ADDRESS_MAP_END

static MACHINE_CONFIG_FRAGMENT( wpc_dmd )
	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500))
	MCFG_SCREEN_UPDATE_DEVICE(DEVICE_SELF, wpc_dmd_device, screen_update)
	MCFG_SCREEN_SIZE(128*4, 32*4)
	MCFG_SCREEN_VISIBLE_AREA(0, 128*4-1, 0, 32*4-1)
	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_TIMER_DRIVER_ADD_PERIODIC("scanline", wpc_dmd_device, scanline_timer, attotime::from_hz(60*4*32))
MACHINE_CONFIG_END


wpc_dmd_device::wpc_dmd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
	device_t(mconfig, WPC_DMD, "Williams Pinball Controller Dot Matrix Display", tag, owner, clock, "wpc_dmd", __FILE__),
	scanline_cb(*this),
	dmd0(*this, ":dmd0"),
	dmd2(*this, ":dmd2"),
	dmd4(*this, ":dmd4"),
	dmd6(*this, ":dmd6"),
	dmd8(*this, ":dmd8"),
	dmda(*this, ":dmda")
{
}

wpc_dmd_device::~wpc_dmd_device()
{
}

machine_config_constructor wpc_dmd_device::device_mconfig_additions() const
{
	return MACHINE_CONFIG_NAME( wpc_dmd );
}

void wpc_dmd_device::device_start()
{
	scanline_cb.resolve_safe();

	ram.resize(0x2000);
	screen_buffer.resize(128*32);
	bitcounts.resize(256);

	dmd0->configure_entries(0, 0x10, &ram[0], 0x200);
	dmd2->configure_entries(0, 0x10, &ram[0], 0x200);
	dmd4->configure_entries(0, 0x10, &ram[0], 0x200);
	dmd6->configure_entries(0, 0x10, &ram[0], 0x200);
	dmd8->configure_entries(0, 0x10, &ram[0], 0x200);
	dmda->configure_entries(0, 0x10, &ram[0], 0x200);

	memset(&ram[0], 0x00, 0x2000);

	for(int i=0; i<256; i++) {
		int bc = i;
		bc = ((bc & 0xaa) >> 1) + (bc & 0x55);
		bc = ((bc & 0xcc) >> 2) + (bc & 0x33);
		bc = ((bc & 0xf0) >> 4) + (bc & 0x0f);
		bitcounts[i] = bc;
	}

	save_item(NAME(visible_page));
	save_item(NAME(cur_scanline));
	save_item(NAME(firq_scanline));
	save_item(NAME(ram));
	save_item(NAME(screen_buffer));
	save_item(NAME(bitcounts));
}

void wpc_dmd_device::device_reset()
{
	dmd0->set_entry(0);
	dmd2->set_entry(1);
	dmd4->set_entry(2);
	dmd6->set_entry(3);
	dmd8->set_entry(4);
	dmda->set_entry(5);

	memset(&screen_buffer[0], 0x00, 128*32);
	visible_page = 0;
	firq_scanline = 0;
	cur_scanline = 0;
}

UINT32 wpc_dmd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const UINT8 *src = &screen_buffer[0];
	for(int y=0; y<32; y++) {
		UINT32 *pix0 = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y*4));
		UINT32 *pix1 = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y*4+1));
		UINT32 *pix2 = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y*4+2));
		UINT32 *pix3 = reinterpret_cast<UINT32 *>(bitmap.raw_pixptr(y*4+3));
		for(int x=0; x<128; x++) {
			UINT8 v = bitcounts[*src++ & 0x3f];
			UINT8 v0 = v < 2 ? 0 : v-2;
			UINT8 v1 = v < 1 ? 0 : v-1;
			UINT8 v2 = v > 5 ? 5 : v;
			v0 = 255*v0/5;
			v1 = 255*v1/5;
			v2 = 255*v2/5;

			UINT32 xv0 = (v0 << 16) | (v0 << 8);
			UINT32 xv1 = (v1 << 16) | (v1 << 8);
			UINT32 xv2 = (v2 << 16) | (v2 << 8);
			*pix0++ = xv0;
			*pix0++ = xv1;
			*pix0++ = xv1;
			*pix0++ = xv0;

			*pix1++ = xv1;
			*pix1++ = xv2;
			*pix1++ = xv2;
			*pix1++ = xv1;

			*pix2++ = xv1;
			*pix2++ = xv2;
			*pix2++ = xv2;
			*pix2++ = xv1;

			*pix3++ = xv0;
			*pix3++ = xv1;
			*pix3++ = xv1;
			*pix3++ = xv0;
		}
	}
	return 0;
}


TIMER_DEVICE_CALLBACK_MEMBER(wpc_dmd_device::scanline_timer)
{
	const UINT8 *src = &ram[0x200*(visible_page & 0xf) + 16*cur_scanline];
	UINT8 *base = &screen_buffer[128*cur_scanline];

	for(int x1=0; x1<16; x1++) {
		UINT8 v = *src++;
		for(int x2=0; x2<8; x2++) {
			*base = (*base << 1) | ((v & (0x01 << x2)) ? 1 : 0);
			base++;
		}
	}

	cur_scanline = (cur_scanline+1) & 0x1f;
	scanline_cb(cur_scanline == (firq_scanline & 0x1f));
}

WRITE8_MEMBER(wpc_dmd_device::firq_scanline_w)
{
	firq_scanline = data;
}

WRITE8_MEMBER(wpc_dmd_device::bank0_w)
{
	dmd0->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::bank2_w)
{
	dmd2->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::bank4_w)
{
	dmd4->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::bank6_w)
{
	dmd6->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::bank8_w)
{
	dmd8->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::banka_w)
{
	dmda->set_entry(data & 0xf);
}

WRITE8_MEMBER(wpc_dmd_device::visible_page_w)
{
	visible_page = data;
}
