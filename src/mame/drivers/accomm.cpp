// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

    Acorn Communicator

    Skeleton driver by R. Belmont

	Main CPU: 65C816
	Other chips: 6850 UART, 6522 VIA, SAA5240(video?), AM7910 modem, PCF0335(?)

****************************************************************************/

#include "emu.h"
#include "cpu/g65816/g65816.h"
#include "machine/nvram.h"
#include "machine/bankdev.h"
#include "bus/rs232/rs232.h"
#include "screen.h"
#include "speaker.h"

class accomm_state : public driver_device
{
public:
	accomm_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_bank0dev(*this, "bank0dev"),
			m_vram(*this, "vram")
	{ }

	virtual void machine_reset() override;
	virtual void machine_start() override;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	
	DECLARE_WRITE8_MEMBER(b0_rom_disable_w);

protected:

	// devices
	required_device<g65816_device> m_maincpu;
	required_device<address_map_bank_device> m_bank0dev;
	required_shared_ptr<uint8_t> m_vram;

	// driver_device overrides
	virtual void video_start() override;
};

void accomm_state::machine_reset()
{
}

void accomm_state::machine_start()
{
}

void accomm_state::video_start()
{
}

WRITE8_MEMBER(accomm_state::b0_rom_disable_w)
{
	m_bank0dev->set_bank(1);
}

uint32_t accomm_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	uint32_t *scanline;
	int x, y;
	uint8_t pixels;
	static const uint32_t palette[2] = { 0, 0xffffff };
	uint8_t *vram = (uint8_t *)m_vram.target();
	uint32_t ula_addr = 0;

	vram += 0x3000;

	for (y = 0; y < 256; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 80; x++)
		{
			pixels = vram[ula_addr + (x << 3)];

			*scanline++ = palette[(pixels>>7)&1];
			*scanline++ = palette[(pixels>>6)&1];
			*scanline++ = palette[(pixels>>5)&1];
			*scanline++ = palette[(pixels>>4)&1];
			*scanline++ = palette[(pixels>>3)&1];
			*scanline++ = palette[(pixels>>2)&1];
			*scanline++ = palette[(pixels>>1)&1];
			*scanline++ = palette[(pixels&1)];
		}
		ula_addr++;
		
		if ((y & 7) == 7)
		{
			ula_addr += 0x278;
		}
	}

	return 0;
}

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, accomm_state )
	AM_RANGE(0x000000, 0x007fff) AM_RAM
	AM_RANGE(0x008000, 0x00ffff) AM_DEVICE("bank0dev", address_map_bank_device, amap8)
	AM_RANGE(0x010000, 0x08ffff) AM_RAM	// "576K RAM"
	// VIA probably at 420000
	AM_RANGE(0x440000, 0x440000) AM_WRITE(b0_rom_disable_w)
	AM_RANGE(0x450000, 0x467fff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xfc0000, 0xffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static ADDRESS_MAP_START( b0dev_map, AS_PROGRAM, 8, accomm_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_REGION("maincpu", 0x38000)
	AM_RANGE(0x8000, 0xffff) AM_RAM
ADDRESS_MAP_END

static INPUT_PORTS_START( accomm )
INPUT_PORTS_END

static MACHINE_CONFIG_START( accomm )
	MCFG_CPU_ADD("maincpu", G65816, XTAL_2MHz)
	MCFG_CPU_PROGRAM_MAP(main_map)
//	MCFG_CPU_VBLANK_INT_DRIVER("screen", accomm_state,  irq0_line_hold)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE( 50.08 )
	MCFG_SCREEN_SIZE( 640, 312 )
	MCFG_SCREEN_VISIBLE_AREA( 0, 640-1, 0, 256-1 )
	MCFG_SCREEN_UPDATE_DRIVER(accomm_state, screen_update)
	
	MCFG_DEVICE_ADD("bank0dev", ADDRESS_MAP_BANK, 0)
	MCFG_DEVICE_PROGRAM_MAP(b0dev_map)
	MCFG_ADDRESS_MAP_BANK_ENDIANNESS(ENDIANNESS_LITTLE)
	MCFG_ADDRESS_MAP_BANK_DATABUS_WIDTH(8)
	MCFG_ADDRESS_MAP_BANK_STRIDE(0x8000)
	
	MCFG_SPEAKER_STANDARD_MONO("mono")
MACHINE_CONFIG_END

ROM_START(accomm)
	ROM_REGION(0x40000, "maincpu", 0)       /* C68 / M37450 program ROM */
	ROM_LOAD( "romv100-3.rom", 0x000000, 0x010000, CRC(bd87a157) SHA1(b9b9ed1aab9ffef2de988b2cfeac293afa11448a) ) 
	ROM_LOAD( "romv100-2.rom", 0x010000, 0x010000, CRC(3438adee) SHA1(cd9d5522d9430cb2e1936210b77d2edd280f9419) ) 
	ROM_LOAD( "romv100-1.rom", 0x020000, 0x010000, CRC(adc6a073) SHA1(3e87f21fafc1d69f33c5b541a20a98e82aacbfab) ) 
	ROM_LOAD( "romv100-0.rom", 0x030000, 0x010000, CRC(6d22950d) SHA1(d4cbdccf8d2bc836fb81182b2ed344d7134fe5c9) ) 
ROM_END

GAME( 1986,  accomm,  0,  accomm,  accomm, accomm_state,  0,  ROT0,  "Acorn", "Acorn Communicator", MACHINE_NOT_WORKING )
