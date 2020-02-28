// license:BSD-3-Clause
// copyright-holders:R. Belmont, Olivier Galibert

// HD44780/LCD image combo used in the yamaha mus, vl70m, fs1r and
// probably others

#include "emu.h"
#include "mulcd.h"

#include "../machine/mulcd.hxx"

DEFINE_DEVICE_TYPE(MULCD, mulcd_device, "mulcd", "Yamaha MU/VL70/FS1R common LCD")

ROM_START( mulcd )
	ROM_REGION( 0x1000, "hd44780", 0)
	// Hand made, 3 characters unused
	ROM_LOAD( "mu100-font.bin", 0x0000, 0x1000, BAD_DUMP CRC(a7d6c1d6) SHA1(9f0398d678bdf607cb34d83ee535f3b7fcc97c41) )
ROM_END

const tiny_rom_entry *mulcd_device::device_rom_region() const
{
	return ROM_NAME(mulcd);
}

mulcd_device::mulcd_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock) :
	device_t(mconfig, MULCD, tag, owner, clock),
	m_lcd(*this, "hd44780")
{
}

void mulcd_device::device_start()
{
}

void mulcd_device::device_reset()
{
	m_contrast = 1.0;
	m_leds = 0;
}

float mulcd_device::lightlevel(const u8 *src, const u8 *render)
{
	u8 l = *src;
	if(l == 0)
		return 1.0;
	int slot = (src[1] << 8) | src[2];
	if(slot >= 0xff00)
		return (255-l)/255.0;

	int bit = slot & 7;
	int adr = (slot >> 3);
	if(render[adr] & (1 << bit))
		return 1-(1-(255-l)/255.0f)*m_contrast;
	return 0.95f;
}

u32 mulcd_device::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	const u8 *render = m_lcd->render();
	const u8 *src = mulcd_bkg + 15;

	for(int y=0; y<241; y++) {
		u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y));
		for(int x=0; x<800; x++) {
			float light = lightlevel(src, render);
			u32 col = (int(0xef*light) << 16) | (int(0xf5*light) << 8);
			*pix++ = col;
			src += 3;
		}
		for(int x=800; x<900; x++)
			*pix++ = 0;
	}

	for(int i=0; i<6; i++)
		if(m_leds & (1 << i)) {
			int x = 830 + 40*(i & 1);
			int y = 55 + 65*(i >> 1);
			for(int yy=-9; yy <= 9; yy++) {
				int dx = int(sqrt((float)(99-yy*yy)));
				u32 *pix = reinterpret_cast<u32 *>(bitmap.raw_pixptr(y+yy)) + (x-dx);
				for(int xx=0; xx<2*dx+1; xx++)
					*pix++ = 0x00ff00;
			}
		}
	return 0;
}

void mulcd_device::device_add_mconfig(machine_config &config)
{
	HD44780(config, m_lcd);
	m_lcd->set_lcd_size(4, 20);

	auto &screen = SCREEN(config, "screen", SCREEN_TYPE_LCD);
	screen.set_refresh_hz(50);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); /* not accurate, asynchronous updating anyway */
	screen.set_screen_update(FUNC(mulcd_device::screen_update));
	screen.set_size(900, 241);
	screen.set_visarea(0, 899, 0, 240);
}
