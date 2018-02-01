// license:BSD-3-Clause
// copyright-holders:Carl

// Dulmont Magnum
// Additional info https://www.youtube.com/watch?v=st7H_vqSaQc and
// http://www.eevblog.com/forum/blog/eevblog-949-vintage-australian-made-laptop-teardown/msg1080508/#msg1080508

#include "emu.h"
#include "cpu/i86/i186.h"
#include "machine/cdp1879.h"
#include "sound/beep.h"
#include "rendlay.h"
#include "screen.h"
#include "speaker.h"

class magnum_state : public driver_device
{
public:
	magnum_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_palette(*this, "palette")
		, m_cgrom(*this, "cgrom")
		, m_beep(*this, "beep")
	{
	}

	DECLARE_READ8_MEMBER(lcd_r);
	DECLARE_WRITE8_MEMBER(lcd_w);
	DECLARE_WRITE8_MEMBER(beep_w);
	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void magnum(machine_config &config);
	void magnum_io(address_map &map);
	void magnum_map(address_map &map);
protected:
	virtual void machine_reset() override;
	virtual void machine_start() override;

private:
	required_device<palette_device> m_palette;
	required_memory_region m_cgrom;
	required_device<beep_device> m_beep;

	struct lcd
	{
		u8 vram[640];
		u8 cmd;
		u16 cursor;
	};
	lcd m_lcd[2];
};

u32 magnum_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	u8* font = m_cgrom->base();
	u32 black = m_palette->pen(0);
	u32 white = m_palette->pen(1);
	for(int scr = 0; scr < 2; scr++)
	{
		u8* vram = m_lcd[!scr].vram;
		for(int i = 0; i < 16; i++)
		{
			for(int j = 0; j < 40; j++)
			{
				for(int k = 0; k < 9; k++)
				{
					for(int l = 0; l < 6; l++)
						bitmap.pix32((i * 9) + k, ((j + (scr * 40)) * 6) + l) = font[(vram[(i * 40) + j] * 16) + k] & (1 << (5 - l)) ? black : white;
				}
			}
		}
	}
	return 0;
}

void magnum_state::machine_start()
{
	save_item(NAME(m_lcd[0].vram));
	save_item(NAME(m_lcd[0].cmd));
	save_item(NAME(m_lcd[0].cursor));
	save_item(NAME(m_lcd[1].vram));
	save_item(NAME(m_lcd[1].cmd));
	save_item(NAME(m_lcd[1].cursor));
}

void magnum_state::machine_reset()
{
	memset(m_lcd, 0, sizeof(m_lcd));
}

READ8_MEMBER(magnum_state::lcd_r)
{
	//lcd& panel = m_lcd[BIT(offset, 1)];
	switch(BIT(offset, 0))
	{
		case 1:
			return 0; // bit 8 busy status
	}
	return 0;
}

WRITE8_MEMBER(magnum_state::lcd_w)
{
	lcd& panel = m_lcd[BIT(offset, 1)];
	switch(BIT(offset, 0))
	{
		case 0:
			switch(panel.cmd)
			{
				case 0xa:
					panel.cursor = ((panel.cursor & 0xff00) | data) % 640;
					break;
				case 0xb:
					panel.cursor = ((panel.cursor & 0x00ff) | (data << 8)) % 640;
					break;
				case 0xc:
					panel.vram[panel.cursor] = data;
					panel.cursor++;
					panel.cursor %= 640;
					break;
			}
			break;
		case 1:
			panel.cmd = data;
			break;
	}
}

WRITE8_MEMBER(magnum_state::beep_w)
{
	if (data & ~1) printf("beep_w unmapped bits %02x\n", data);
	m_beep->set_state(BIT(data, 0));
}

ADDRESS_MAP_START(magnum_state::magnum_map)
	AM_RANGE(0x00000, 0x3ffff) AM_RAM // fixed 256k for now
	AM_RANGE(0xe0000, 0xfffff) AM_ROM AM_REGION("bios", 0)
ADDRESS_MAP_END

ADDRESS_MAP_START(magnum_state::magnum_io)
	ADDRESS_MAP_UNMAP_HIGH
	//AM_RANGE(0x000a, 0x000b) cdp1854 1
	//AM_RANGE(0x000e, 0x000f) cpd1854 2
	AM_RANGE(0x0018, 0x001f) AM_READWRITE8(lcd_r, lcd_w, 0x00ff)
	AM_RANGE(0x0056, 0x0057) AM_WRITE8(beep_w, 0x00ff)
	AM_RANGE(0x0080, 0x008f) AM_DEVREADWRITE8("rtc", cdp1879_device, read, write, 0x00ff)
ADDRESS_MAP_END

MACHINE_CONFIG_START(magnum_state::magnum)
	MCFG_CPU_ADD("maincpu", I80186, XTAL(12'000'000) / 2)
	MCFG_CPU_PROGRAM_MAP(magnum_map)
	MCFG_CPU_IO_MAP(magnum_io)

	MCFG_DEVICE_ADD("rtc", CDP1879, XTAL(32'768))

	MCFG_SCREEN_ADD("screen", LCD)
	MCFG_SCREEN_UPDATE_DRIVER(magnum_state, screen_update)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_SIZE(6*80, 9*16)
	MCFG_SCREEN_VISIBLE_AREA(0, 6*80-1, 0, 9*16-1)

	MCFG_DEFAULT_LAYOUT(layout_lcd)

	MCFG_PALETTE_ADD_MONOCHROME("palette")

	MCFG_SPEAKER_STANDARD_MONO("speaker")
	MCFG_SOUND_ADD("beep", BEEP, 500) /// frequency is guessed
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "speaker", 0.50)
MACHINE_CONFIG_END

ROM_START( magnum )
	ROM_REGION(0x20000, "bios", 0)
	ROM_LOAD16_BYTE("a1.7.88.bin", 0x00000, 0x4000, CRC(57882427) SHA1(97637b65ca43eb9d3bba546fb8ca701ba25ade8d))
	ROM_LOAD16_BYTE("a1.7.81.bin", 0x00001, 0x4000, CRC(949f53a8) SHA1(b339f1495d9af7dfff0c3a2c24789631f9d1265b))
	ROM_LOAD16_BYTE("a1.7.87.bin", 0x08000, 0x4000, CRC(25036dda) SHA1(20bc3782a66855b20cb0abe1051fa2eb50c7a860))
	ROM_LOAD16_BYTE("a1.7.82.bin", 0x08001, 0x4000, CRC(ecf387d8) SHA1(8b42f6ab030afb51f21f4a56c62e5acf7d074066))
	ROM_LOAD16_BYTE("a1.7.86.bin", 0x10000, 0x4000, CRC(c80b3a6b) SHA1(0f0d2cb653bbeff8f3bab6d20dc30c220a67a315))
	ROM_LOAD16_BYTE("a1.7.83.bin", 0x10001, 0x4000, CRC(51f56d78) SHA1(df717eada5e6439b1c01d91bd0ea009cd0f8ddfa))
	ROM_LOAD16_BYTE("a1.7.85.bin", 0x18000, 0x4000, CRC(f5dd5407) SHA1(af2edf7a658bcf648acb8be9f13849f838d96214))
	ROM_LOAD16_BYTE("a1.7.84.bin", 0x18001, 0x4000, CRC(b3434bb0) SHA1(8000a7aca8fc505b136a618d9eb210c50393eff1))

	ROM_REGION(0x1000, "char", 0)
	ROM_LOAD("dulmontcharrom.bin", 0x0000, 0x1000, CRC(9dff89bf) SHA1(d359aeba7f0b0c81accf3bca25e7da636c033721))

	ROM_REGION(0x1000, "cgrom", 0) // borrow this rom for the lcd screen as it looks the same, the above is for the crt output
	ROM_LOAD("hd44780_a00.bin", 0x0000, 0x1000,  BAD_DUMP CRC(01d108e2) SHA1(bc0cdf0c9ba895f22e183c7bd35a3f655f2ca96f))
ROM_END

COMP( 1983, magnum, 0, 0, magnum, 0, magnum_state, 0, "Dulmont", "Magnum", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND)
