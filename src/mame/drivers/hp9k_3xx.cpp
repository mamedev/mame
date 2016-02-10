// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

  hp9k3xx.c: preliminary driver for HP9000 300 Series (aka HP9000/3xx)

  Currently supporting:

  310:
      MC68010 CPU @ 10 MHz
      HP custom MMU

  320:
      MC68020 CPU @ 16.67 MHz
      HP custom MMU
      MC68881 FPU

  330:
      MC68020 CPU @ 16.67 MHz
      MC68851 MMU
      MC68881 FPU

  340:
      MC68030 CPU @ 16.67 MHz w/built-in MMU
      MC68881 FPU

  380:
    MC68040 CPU @ 25 MHz w/built-in MMU and FPU

  382:
    MC68040 CPU @ 25? MHz w/built-in MMU and FPU
    Built-in VGA compatible video

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
		m_vram16(*this, "vram16"),
		m_vram(*this, "vram")
		{ }

	required_device<cpu_device> m_maincpu;
	virtual void machine_reset() override;

	optional_shared_ptr<UINT16> m_vram16;
	optional_shared_ptr<UINT32> m_vram;

	UINT32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 hp98544_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	UINT32 hp98544_16_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	DECLARE_READ16_MEMBER(buserror16_r);
	DECLARE_WRITE16_MEMBER(buserror16_w);
	DECLARE_READ32_MEMBER(buserror_r);
	DECLARE_WRITE32_MEMBER(buserror_w);

private:
	bool m_in_buserr;
	UINT32 m_last_buserr_pc;
};

UINT32 hp9k3xx_state::hp98544_16_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	UINT32 *scanline;
	int x, y;
	UINT16 pixels;
	UINT32 m_palette[2] = { 0x00000000, 0xffffffff };

	for (y = 0; y < 768; y++)
	{
		scanline = &bitmap.pix32(y);
		for (x = 0; x < 1024/2; x++)
		{
			pixels = m_vram16[(y * 512) + x];

			*scanline++ = m_palette[(pixels>>8) & 1];
			*scanline++ = m_palette[(pixels & 1)];
		}
	}

	return 0;
}

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
	AM_RANGE(0x00000000, 0x0001ffff) AM_ROM AM_REGION("maincpu",0) AM_WRITENOP  // writes to 1fffc are the LED

	AM_RANGE(0x00500000, 0x0050000f) AM_RAM // this is sufficient to pass the DMA test for now

	AM_RANGE(0x00510000, 0x00510003) AM_READWRITE(buserror_r, buserror_w)   // no "Alpha display"
	AM_RANGE(0x00538000, 0x00538003) AM_READWRITE(buserror_r, buserror_w)   // no "Graphics"
	AM_RANGE(0x005c0000, 0x005c0003) AM_READWRITE(buserror_r, buserror_w)   // no add-on FP coprocessor
	AM_RANGE(0x005f8000, 0x005f800f) AM_DEVREADWRITE8(PTM6840_TAG, ptm6840_device, read, write, 0x00ff00ff)
ADDRESS_MAP_END

// 9000/310 - has onboard video that the graphics card used in other 3xxes conflicts with
static ADDRESS_MAP_START(hp9k310_map, AS_PROGRAM, 16, hp9k3xx_state)
	AM_RANGE(0x000000, 0x01ffff) AM_ROM AM_REGION("maincpu",0) AM_WRITENOP  // writes to 1fffc are the LED

	AM_RANGE(0x510000, 0x510003) AM_READWRITE(buserror16_r, buserror16_w)   // no "Alpha display"
	AM_RANGE(0x538000, 0x538003) AM_READWRITE(buserror16_r, buserror16_w)   // no "Graphics"
	AM_RANGE(0x5c0000, 0x5c0003) AM_READWRITE(buserror16_r, buserror16_w)   // no add-on FP coprocessor

	AM_RANGE(0x5f8000, 0x5f800f) AM_DEVREADWRITE8(PTM6840_TAG, ptm6840_device, read, write, 0x00ff)

	AM_RANGE(0x200000, 0x2fffff) AM_RAM AM_SHARE("vram16")  // 98544 mono framebuffer
	AM_RANGE(0x560000, 0x563fff) AM_ROM AM_REGION("graphics", 0x0000)   // 98544 mono ROM

	AM_RANGE(0x700000, 0x7fffff) AM_READWRITE(buserror16_r, buserror16_w)
	AM_RANGE(0x800000, 0xffffff) AM_RAM
ADDRESS_MAP_END

// 9000/320
static ADDRESS_MAP_START(hp9k320_map, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_SHARE("vram")    // 98544 mono framebuffer
	AM_RANGE(0x00560000, 0x00563fff) AM_ROM AM_REGION("graphics", 0x0000)   // 98544 mono ROM

	AM_RANGE(0xffe00000, 0xffefffff) AM_READWRITE(buserror_r, buserror_w)
	AM_RANGE(0xfff00000, 0xffffffff) AM_RAM

	AM_IMPORT_FROM(hp9k3xx_common)
ADDRESS_MAP_END

// 9000/330 and 9000/340
static ADDRESS_MAP_START(hp9k330_map, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_SHARE("vram")    // 98544 mono framebuffer
	AM_RANGE(0x00560000, 0x00563fff) AM_ROM AM_REGION("graphics", 0x0000)   // 98544 mono ROM

	AM_RANGE(0xffb00000, 0xffbfffff) AM_READWRITE(buserror_r, buserror_w)
	AM_RANGE(0xffc00000, 0xffffffff) AM_RAM

	AM_IMPORT_FROM(hp9k3xx_common)
ADDRESS_MAP_END

// 9000/370 - 8 MB RAM standard
static ADDRESS_MAP_START(hp9k370_map, AS_PROGRAM, 32, hp9k3xx_state)
	AM_RANGE(0x00200000, 0x002fffff) AM_RAM AM_SHARE("vram")    // 98544 mono framebuffer
	AM_RANGE(0x00560000, 0x00563fff) AM_ROM AM_REGION("graphics", 0x0000)   // 98544 mono ROM

	AM_RANGE(0xff700000, 0xff7fffff) AM_READWRITE(buserror_r, buserror_w)
	AM_RANGE(0xff800000, 0xffffffff) AM_RAM

	AM_IMPORT_FROM(hp9k3xx_common)
ADDRESS_MAP_END

// 9000/382 - onboard VGA compatible video (where?)
static ADDRESS_MAP_START(hp9k382_map, AS_PROGRAM, 32, hp9k3xx_state)
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
	m_in_buserr = false;
	m_last_buserr_pc = 0;
}

READ16_MEMBER(hp9k3xx_state::buserror16_r)
{
	if (m_last_buserr_pc == space.device().safe_pc())
	{
		return 0;
	}

	if (!m_in_buserr)
	{
		m_in_buserr = true;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_in_buserr = false;
		m_last_buserr_pc = space.device().safe_pc();
	}
	return 0;
}

WRITE16_MEMBER(hp9k3xx_state::buserror16_w)
{
	if (m_last_buserr_pc == space.device().safe_pc())
	{
		return;
	}

	if (!m_in_buserr)
	{
		m_in_buserr = true;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_in_buserr = false;
		m_last_buserr_pc = space.device().safe_pc();
	}
}

READ32_MEMBER(hp9k3xx_state::buserror_r)
{
	if (m_last_buserr_pc == space.device().safe_pc())
	{
		return 0;
	}

	if (!m_in_buserr)
	{
		m_in_buserr = true;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_in_buserr = false;
		m_last_buserr_pc = space.device().safe_pc();
	}
	return 0;
}

WRITE32_MEMBER(hp9k3xx_state::buserror_w)
{
	if (m_last_buserr_pc == space.device().safe_pc())
	{
		return;
	}

	if (!m_in_buserr)
	{
		m_in_buserr = true;
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, ASSERT_LINE);
		m_maincpu->set_input_line(M68K_LINE_BUSERROR, CLEAR_LINE);
		m_in_buserr = false;
		m_last_buserr_pc = space.device().safe_pc();
	}
}

static MACHINE_CONFIG_START( hp9k310, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68010, 10000000)
	MCFG_CPU_PROGRAM_MAP(hp9k310_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_16_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k320, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68020FPU, 16670000)
	MCFG_CPU_PROGRAM_MAP(hp9k320_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
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
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k340, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68030, 16670000)
	MCFG_CPU_PROGRAM_MAP(hp9k330_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k370, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68030, 33000000)
	MCFG_CPU_PROGRAM_MAP(hp9k370_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k380, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68040, 25000000) // 25 MHz?  33?
	MCFG_CPU_PROGRAM_MAP(hp9k330_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, hp98544_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( hp9k382, hp9k3xx_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(MAINCPU_TAG, M68040, 25000000) // 25 MHz?  33?
	MCFG_CPU_PROGRAM_MAP(hp9k382_map)

	MCFG_DEVICE_ADD(PTM6840_TAG, PTM6840, 0)
	MCFG_PTM6840_INTERNAL_CLOCK(250000.0f)  // from oscillator module next to the 6840
	MCFG_PTM6840_EXTERNAL_CLOCKS(250000.0f, 250000.0f, 250000.0f)

	MCFG_SCREEN_ADD( "screen", RASTER)
	MCFG_SCREEN_UPDATE_DRIVER(hp9k3xx_state, screen_update)
	MCFG_SCREEN_SIZE(1024,768)
	MCFG_SCREEN_VISIBLE_AREA(0, 1024-1, 0, 768-1)
	MCFG_SCREEN_REFRESH_RATE(70)
MACHINE_CONFIG_END

ROM_START( hp9k310 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-3771.bin", 0x000001, 0x008000, CRC(b9e4e3ad) SHA1(ed6f1fad94a15d95362701dbe124b52877fc3ec4) )
	ROM_LOAD16_BYTE( "1818-3772.bin", 0x000000, 0x008000, CRC(a3665919) SHA1(ec1bc7e5b7990a1b09af947a06401e8ed3cb0516) )

	ROM_REGION( 0x800, "mcu", 0 )
	ROM_LOAD( "1820-4784_1.bin", 0x000000, 0x000800, CRC(e929044a) SHA1(90849a10bdb8c6e38e73ce027c9c0ad8b3956b1b) )
	ROM_LOAD( "1820-4784_2.bin", 0x000000, 0x000800, CRC(8defcf50) SHA1(d3abfea468a43db7c2369500a3e390e77a8e22e6) )

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000000, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

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

ROM_START( hp9k340 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) )
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) )

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000001, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

ROM_START( hp9k370 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_BYTE( "1818-4416.bin", 0x000000, 0x010000, CRC(cd71e85e) SHA1(3e83a80682f733417fdc3720410e45a2cfdcf869) )
	ROM_LOAD16_BYTE( "1818-4417.bin", 0x000001, 0x010000, CRC(374d49db) SHA1(a12cbf6c151e2f421da4571000b5dffa3ef403b3) )

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000001, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

ROM_START( hp9k380 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_WORD_SWAP( "1818-5062_98754_9000-380_27c210.bin", 0x000000, 0x020000, CRC(500a0797) SHA1(4c0a3929e45202a2689e353657e5c4b58ff9a1fd) )

	ROM_REGION( 0x4000, "graphics", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD16_BYTE( "98544_1818-1999.bin", 0x000001, 0x002000, CRC(8c7d6480) SHA1(d2bcfd39452c38bc652df39f84c7041cfdf6bd51) )
ROM_END

ROM_START( hp9k382 )
	ROM_REGION( 0x20000, MAINCPU_TAG, 0 )
	ROM_LOAD16_WORD_SWAP( "1818-5468_27c1024.bin", 0x000000, 0x020000, CRC(d1d9ef13) SHA1(6bbb17b9adad402fbc516dc2f3143e9c38ceef8e) )

	ROM_REGION( 0x2000, "unknown", ROMREGION_ERASEFF | ROMREGION_BE | ROMREGION_32BIT )
	ROM_LOAD( "1818-5282_8ce61e951207_28c64.bin", 0x000000, 0x002000, CRC(740442f3) SHA1(ab65bd4eec1024afb97fc2dd3bd3f017e90f49ae) )
ROM_END

/*    YEAR  NAME    PARENT   COMPAT  MACHINE   INPUT                 INIT    COMPANY          FULLNAME       FLAGS */
COMP( 1985, hp9k310, 0,      0,      hp9k310,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/310", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1985, hp9k320, 0,      0,      hp9k320,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/320", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1987, hp9k330, 0,      0,      hp9k330,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/330", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1989, hp9k340, hp9k330,0,      hp9k340,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/340", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1988, hp9k370, hp9k330,0,      hp9k370,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/370", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1991, hp9k380, 0,      0,      hp9k380,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/380", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
COMP( 1991, hp9k382, 0,      0,      hp9k382,  hp9k330, driver_device, 0, "Hewlett-Packard", "HP9000/382", MACHINE_NOT_WORKING | MACHINE_NO_SOUND)
