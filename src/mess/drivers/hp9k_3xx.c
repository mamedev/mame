// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************
  
  hp9k3xx.c: preliminary driver for HP9000 300 Series (aka HP9000/3xx)
 
  Currently supporting:
 
  320:
      MC68020 CPU @ 16.67 MHz
      HP custom MMU
      MC68881 FPU
 
  330:
	  MC68020 CPU @ 16.67 MHz
	  MC68851 MMU
	  MC68881 FPU
 
  All models have an MC6840 PIT on IRQ6 clocked at 250 kHz.
 
  TODO:
    BBCADDR   0x420000
    RTC_DATA: 0x420001
    RTC_CMD:  0x420003
    HIL:      0x428000
    HPIB:     0x478000
    KBDNMIST: 0x478005
    DMA:      0x500000
    FRAMEBUF: 0x560000
 
    6840:     0x5F8001/3/5/7/9, IRQ 6
 
****************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "machine/6840ptm.h"

#define MAINCPU_TAG "maincpu"
#define PTM6840_TAG "ptm"

class hp9k3xx_state : public driver_device
{
public:
	hp9k3xx_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, MAINCPU_TAG),
		m_vram(*this, "vram")
		{ }

	required_device<cpu_device> m_maincpu;
	virtual void machine_reset();

	optional_shared_ptr<UINT32> m_vram;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 hp98544_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ32_MEMBER(buserror_r);
	DECLARE_WRITE32_MEMBER(buserror_w);

private:
};

UINT32 hp9k3xx_state::hp98544_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT32 pixels;
	UINT32 m_palette[2] = { 0x00000000, 0xffffffff };

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/4; x++)
		{
			pixels = m_vram[(y * 256) + x];

			*scanline++ = m_palette[(pixels>>24) & 1];
			*scanline++ = m_palette[(pixels>>16) & 1];
			*scanline++ = m_palette[(pixels>>8) & 1];
			*scanline++ = m_palette[(pixels & 1)];
		}
	}

	return 0;
}

// shared mappings for all 9000/3xx systems
static ADDRESS_MAP_START(hp9k3xx_common, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("maincpu",0) AM_WRITENOP	// writes to 1fffc are the LED

	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_SHARE("vram")	// 98544 mono framebuffer
	AM_RANGE(0x00560000, 0x00563fff) AM_ROM AM_REGION("graphics", 0x0000)	// 98544 mono ROM

	AM_RANGE(0x00510000, 0x00510003) AM_READWRITE(buserror_r, buserror_w) 	// no "Alpha display"
	AM_RANGE(0x00538000, 0x00538003) AM_READWRITE(buserror_r, buserror_w)	// no "Graphics"
	AM_RANGE(0x005c0000, 0x005c0003) AM_READWRITE(buserror_r, buserror_w)	// no add-on FP coprocessor
	AM_RANGE(0x005f8000, 0x005f800f) AM_DEVREADWRITE8(PTM6840_TAG, ptm6840_device, read, write, 0x00ff00ff)
ADDRESS_MAP_END

// 9000/320
static ADDRESS_MAP_START(hp9k320_map, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0xffe00000, 0xffefffff) AM_READWRITE(buserror_r, buserror_w)
	AM_RANGE(0xfff00000, 0xffffffff) AM_RAM

	AM_IMPORT_FROM(hp9k3xx_common)
ADDRESS_MAP_END


static ADDRESS_MAP_START(hp9k330_map, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0xffb00000, 0xffbfffff) AM_READWRITE(buserror_r, buserror_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM

	AM_IMPORT_FROM(hp9k3xx_common)
ADDRESS_MAP_END

UINT32 hp9k3xx_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

/* Input ports */
static INPUT_PORTS_START( hp9k330 )
INPUT_PORTS_END


void hp9k3xx_state::machine_reset()
{
}

READ32_MEMBER(hp9k3xx_state::buserror_r)
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
	return 0;
}

WRITE32_MEMBER(hp9k3xx_state::buserror_w)
{
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
	m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
}

static MACHINE_CONFIG_START( hp9k320, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68020, 16670000)
	MCFG_CPU_PROGRAM_MAP(hp9k320_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)	// from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k330, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68020PMMU, 16670000)
	MCFG_CPU_PROGRAM_MAP(hp9k330_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)	// from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

ROM_START( hp9k320 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "5061-6538.bin", 0x000001, 0x004000, CRC(d6aafeb1) SHA1(88c6b0b2f504303cbbac0c496c26b85458ac5d63) ) 
	ROM_LOAD16_BYTE( "5061-6539.bin", 0x000000, 0x004000, CRC(a7ff104c) SHA1(c640fe68314654716bd41b04c6a7f4e560036c7e) ) 
	ROM_LOAD16_BYTE( "5061-6540.bin", 0x008001, 0x004000, CRC(4f6796d6) SHA1(fd254897ac1afb8628f40ea93213f60a082c8d36) ) 
	ROM_LOAD16_BYTE( "5061-6541.bin", 0x008000, 0x004000, CRC(39d32998) SHA1(6de1bda75187b0878c03c074942b807cf2924f0e) ) 

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000001, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

ROM_START( hp9k330 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) ) 
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) ) 

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000001, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

/*    YEAR  NAME    PARENT  COMPAT  MACHINE   INPUT                 INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1985, hp9k320, 0,     0,      hp9k320,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/320", GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1987, hp9k330, 0,     0,      hp9k330,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/330", GAME_NOT_WORKING | GAME_NO_SOUND)
