// license:BSD-3-Clause
// copyright-holders:David Haywood

/*  VT1682 - NOT compatible with NES, different video system, sound CPU (4x
             main CPU clock), optional internal ROM etc.

    Internal ROM can be mapped to Main CPU, or Sound CPU at 0x3000-0x3fff if used
    can also be configured as boot device
*/

#include "emu.h"
#include "machine/m6502_vt1682.h"
#include "screen.h"

class vt_vt1682_state : public driver_device
{
public:
	vt_vt1682_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void vt_vt1682(machine_config &config);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;

private:
	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void vt_vt1682_map(address_map &map);

	DECLARE_READ8_MEMBER(vt1682_2000_r);
	DECLARE_WRITE8_MEMBER(vt1682_2000_w);

	uint8_t m_2000;

};

void vt_vt1682_state::machine_start()
{
	save_item(NAME(m_2000));
}

void vt_vt1682_state::machine_reset()
{
	m_2000 = 0;
}

/************************************************************************************************************************************
 VT1682 PPU Registers
************************************************************************************************************************************/

/*
    Address 0x2000 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - Capture
    0x08 - SLAVE
    0x04 - (unused)
    0x02 - (unused)
    0x01 - NMI_EN
*/

READ8_MEMBER(vt_vt1682_state::vt1682_2000_r)
{
	uint8_t ret = m_2000;
	logerror("%s: vt1682_2000_r offset: %02x returning: %04x\n", machine().describe_context(), offset, ret);
	return ret;
}

WRITE8_MEMBER(vt_vt1682_state::vt1682_2000_w)
{
	logerror("%s: vt1682_2000_w offset: %02x writing: %04x\n", machine().describe_context(), offset, data);
	m_2000 = data;
}

/*
    Address 0x2001 READ (MAIN CPU)

    0x80 - VBLANK
    0x40 - SP ERR
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - (unused)

    Address 0x2001 WRITE (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - EXT CLK DIV
    0x04 - EXT CLK DIV
    0x02 - SP INI
    0x01 - BK INI
*/


/*
    Address 0x2002 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - SPRAM ADDR:2
    0x02 - SPRAM ADDR:1
    0x01 - SPRAM ADDR:0
*/

/*
    Address 0x2003 r/w (MAIN CPU)

    0x80 - SPRAM ADDR:10
    0x40 - SPRAM ADDR:9
    0x20 - SPRAM ADDR:8
    0x10 - SPRAM ADDR:7
    0x08 - SPRAM ADDR:6
    0x04 - SPRAM ADDR:5
    0x02 - SPRAM ADDR:4
    0x01 - SPRAM ADDR:3
*/

/*
    Address 0x2004 r/w (MAIN CPU)

    0x80 - SPRAM DATA:7
    0x40 - SPRAM DATA:6
    0x20 - SPRAM DATA:5
    0x10 - SPRAM DATA:4
    0x08 - SPRAM DATA:3
    0x04 - SPRAM DATA:2
    0x02 - SPRAM DATA:1
    0x01 - SPRAM DATA:0
*/

/*
    Address 0x2005 r/w (MAIN CPU)

    0x80 - VRAM ADDR:7
    0x40 - VRAM ADDR:6
    0x20 - VRAM ADDR:5
    0x10 - VRAM ADDR:4
    0x08 - VRAM ADDR:3
    0x04 - VRAM ADDR:2
    0x02 - VRAM ADDR:1
    0x01 - VRAM ADDR:0
*/

/*
    Address 0x2006 r/w (MAIN CPU)

    0x80 - VRAM ADDR:15
    0x40 - VRAM ADDR:14
    0x20 - VRAM ADDR:13
    0x10 - VRAM ADDR:12
    0x08 - VRAM ADDR:11
    0x04 - VRAM ADDR:10
    0x02 - VRAM ADDR:9
    0x01 - VRAM ADDR:8
*/

/*
    Address 0x2007 r/w (MAIN CPU)

    0x80 - VRAM DATA:7
    0x40 - VRAM DATA:6
    0x20 - VRAM DATA:5
    0x10 - VRAM DATA:4
    0x08 - VRAM DATA:3
    0x04 - VRAM DATA:2
    0x02 - VRAM DATA:1
    0x01 - VRAM DATA:0
*/

/*
    Address 0x2008 r/w (MAIN CPU)

    0x80 - LCD VS DELAY
    0x40 - LCD VS DELAY
    0x20 - LCD VS DELAY
    0x10 - LCD VS DELAY
    0x08 - LCD VS DELAY
    0x04 - LCD VS DELAY
    0x02 - LCD VS DELAY
    0x01 - LCD VS DELAY
*/

/*
    Address 0x2009 r/w (MAIN CPU)

    0x80 - LCD HS DELAY
    0x40 - LCD HS DELAY
    0x20 - LCD HS DELAY
    0x10 - LCD HS DELAY
    0x08 - LCD HS DELAY
    0x04 - LCD HS DELAY
    0x02 - LCD HS DELAY
    0x01 - LCD HS DELAY
*/

/*
    Address 0x200a r/w (MAIN CPU)

    0x80 - LCD FR DELAY:7
    0x40 - LCD FR DELAY:6
    0x20 - LCD FR DELAY:5
    0x10 - LCD FR DELAY:4
    0x08 - LCD FR DELAY:3
    0x04 - LCD FR DELAY:2
    0x02 - LCD FR DELAY:1
    0x01 - LCD FR DELAY:0
*/

/*
    Address 0x200b r/w (MAIN CPU)

    0x80 - CH2 Odd Line Colour
    0x40 - CH2 Odd Line Colour
    0x20 - CH2 Even Line Colour
    0x10 - CH2 Even Line Colour
    0x08 - CH2 SEL
    0x04 - CH2 REV
    0x02 - LCD FR:8
    0x01 - LCD HS:8
*/

/*
    Address 0x200c r/w (MAIN CPU)

    0x80 - FRate
    0x40 - DotODR
    0x20 - LCD CLOCK
    0x10 - LCD CLOCK
    0x08 - UPS 052
    0x04 - Field AC
    0x02 - LCD MODE
    0x01 - LCD MODE
*/

/*
    Address 0x200d r/w (MAIN CPU)

    0x80 - LCD ENABLE
    0x40 - Dot 240
    0x20 - Reverse
    0x10 - Vcom
    0x08 - Odd Line Color
    0x04 - Odd Line Color
    0x02 - Even Line Color
    0x01 - Even Line Color
*/

/*
    Address 0x200e r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Blend2
    0x10 - Blend1
    0x08 - Palette 2 Out Sel
    0x04 - Palette 2 Out Sel
    0x02 - Palette 1 Out Sel
    0x01 - Palette 1 Out Sel
*/

/*
    Address 0x200f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Bk2 Palette Select
    0x04 - Bk2 Palette Select
    0x02 - Bk1 Palette Select
    0x01 - Bk1 Palette Select
*/

/*
    Address 0x2010 r/w (MAIN CPU)

    0x80 - BK1X:7
    0x40 - BK1X:6
    0x20 - BK1X:5
    0x10 - BK1X:4
    0x08 - BK1X:3
    0x04 - BK1X:2
    0x02 - BK1X:1
    0x01 - BK1X:0
*/

/*
    Address 0x2011 r/w (MAIN CPU)

    0x80 - BK1Y:7
    0x40 - BK1Y:6
    0x20 - BK1Y:5
    0x10 - BK1Y:4
    0x08 - BK1Y:3
    0x04 - BK1Y:2
    0x02 - BK1Y:1
    0x01 - BK1Y:0
*/

/*
    Address 0x2012 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - BK1 HCLR
    0x08 - BK1 Scroll Enable
    0x04 - BK1 Scroll Enable
    0x02 - BK1Y:8
    0x01 - BK1X:8
*/

/*
    Address 0x2013 r/w (MAIN CPU)

    0x80 - BK1 Enable
    0x40 - BK1 Palette
    0x20 - BK1 Depth
    0x10 - BK1 Depth
    0x08 - BK1 Colour
    0x04 - BK1 Colour
    0x02 - BK1 Line
    0x01 - BK1 Size
*/

/*
    Address 0x2014 r/w (MAIN CPU)

    0x80 - BK2X:7
    0x40 - BK2X:6
    0x20 - BK2X:5
    0x10 - BK2X:4
    0x08 - BK2X:3
    0x04 - BK2X:2
    0x02 - BK2X:1
    0x01 - BK2X:0
*/

/*
    Address 0x2015 r/w (MAIN CPU)

    0x80 - BK2Y:7
    0x40 - BK2Y:6
    0x20 - BK2Y:5
    0x10 - BK2Y:4
    0x08 - BK2Y:3
    0x04 - BK2Y:2
    0x02 - BK2Y:1
    0x01 - BK2Y:0
*/

/*
    Address 0x2016 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Scroll Enable
    0x04 - BK2 Scroll Enable
    0x02 - BK2Y:8
    0x01 - BK2X:8
*/

/*
    Address 0x2017 r/w (MAIN CPU)

    0x80 - BK2 Enable
    0x40 - BK2 Palette
    0x20 - BK2 Depth
    0x10 - BK2 Depth
    0x08 - BK2 Colour
    0x04 - BK2 Colour
    0x02 - (unused)
    0x01 - BK2 Size
*/

/*
    Address 0x2018 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SPALSEL
    0x04 - SP ENABLE
    0x02 - SP SIZE
    0x01 - SP SIZE
*/

/*
    Address 0x2019 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 Gain
    0x04 - BK2 Gain
    0x02 - BK1 Gain
    0x01 - BK1 Gain
*/

/*
    Address 0x201a r/w (MAIN CPU)

    0x80 - SP SEGMENT:7
    0x40 - SP SEGMENT:6
    0x20 - SP SEGMENT:5
    0x10 - SP SEGMENT:4
    0x08 - SP SEGMENT:3
    0x04 - SP SEGMENT:2
    0x02 - SP SEGMENT:1
    0x01 - SP SEGMENT:0
*/

/*
    Address 0x201b r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - SP SEGMENT:11
    0x04 - SP SEGMENT:10
    0x02 - SP SEGMENT:9
    0x01 - SP SEGMENT:8
*/

/*
    Address 0x201c r/w (MAIN CPU)

    0x80 - BK1 SEGMENT:7
    0x40 - BK1 SEGMENT:6
    0x20 - BK1 SEGMENT:5
    0x10 - BK1 SEGMENT:4
    0x08 - BK1 SEGMENT:3
    0x04 - BK1 SEGMENT:2
    0x02 - BK1 SEGMENT:1
    0x01 - BK1 SEGMENT:0
*/

/*
    Address 0x201d r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK1 SEGMENT:11
    0x04 - BK1 SEGMENT:10
    0x02 - BK1 SEGMENT:9
    0x01 - BK1 SEGMENT:8
*/

/*
    Address 0x201e r/w (MAIN CPU)

    0x80 - BK2 SEGMENT:7
    0x40 - BK2 SEGMENT:6
    0x20 - BK2 SEGMENT:5
    0x10 - BK2 SEGMENT:4
    0x08 - BK2 SEGMENT:3
    0x04 - BK2 SEGMENT:2
    0x02 - BK2 SEGMENT:1
    0x01 - BK2 SEGMENT:0
*/

/*
    Address 0x201f r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - BK2 SEGMENT:11
    0x04 - BK2 SEGMENT:10
    0x02 - BK2 SEGMENT:9
    0x01 - BK2 SEGMENT:8
*/


/*
    Address 0x2020 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - BK2 L EN
    0x10 - BK1 L EN
    0x08 - Scroll Bank
    0x04 - Scroll Bank
    0x02 - Scroll Bank
    0x01 - Scroll Bank
*/

/*
    Address 0x2021 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Luminance_offset
    0x10 - Luminance_offset
    0x08 - Luminance_offset
    0x04 - Luminance_offset
    0x02 - Luminance_offset
    0x01 - Luminance_offset
*/

/*
    Address 0x2022 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - VCOMIO
    0x10 - RGB DAC
    0x08 - CCIR Out
    0x04 - Saturation
    0x02 - Saturation
    0x01 - Saturation
*/

/*
    Address 0x2023 r/w (MAIN CPU)

    0x80 - Light Gun Reset
    0x40 - Light Gun Reset
    0x20 - Light Gun Reset
    0x10 - Light Gun Reset
    0x08 - Light Gun Reset
    0x04 - Light Gun Reset
    0x02 - Light Gun Reset
    0x01 - Light Gun Reset
*/

/*
    Address 0x2024 r/w (MAIN CPU)

    0x80 - Light Gun 1 Y
    0x40 - Light Gun 1 Y
    0x20 - Light Gun 1 Y
    0x10 - Light Gun 1 Y
    0x08 - Light Gun 1 Y
    0x04 - Light Gun 1 Y
    0x02 - Light Gun 1 Y
    0x01 - Light Gun 1 Y
*/

/*
    Address 0x2025 r/w (MAIN CPU)

    0x80 - Light Gun 1 X
    0x40 - Light Gun 1 X
    0x20 - Light Gun 1 X
    0x10 - Light Gun 1 X
    0x08 - Light Gun 1 X
    0x04 - Light Gun 1 X
    0x02 - Light Gun 1 X
    0x01 - Light Gun 1 X
*/

/*
    Address 0x2026 r/w (MAIN CPU)

    0x80 - Light Gun 2 Y
    0x40 - Light Gun 2 Y
    0x20 - Light Gun 2 Y
    0x10 - Light Gun 2 Y
    0x08 - Light Gun 2 Y
    0x04 - Light Gun 2 Y
    0x02 - Light Gun 2 Y
    0x01 - Light Gun 2 Y
*/

/*
    Address 0x2027 r/w (MAIN CPU)

    0x80 - Light Gun 2 X
    0x40 - Light Gun 2 X
    0x20 - Light Gun 2 X
    0x10 - Light Gun 2 X
    0x08 - Light Gun 2 X
    0x04 - Light Gun 2 X
    0x02 - Light Gun 2 X
    0x01 - Light Gun 2 X
*/

/*
    Address 0x2028 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - CCIR Y
    0x10 - CCIR Y
    0x08 - CCIR Y
    0x04 - CCIR Y
    0x02 - CCIR Y
    0x01 - CCIR Y
*/

/*
    Address 0x2029 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - CCIR X
    0x08 - CCIR X
    0x04 - CCIR X
    0x02 - CCIR X
    0x01 - CCIR X
*/

/*
    Address 0x202a r/w (MAIN CPU)

    0x80 - VS Phase
    0x40 - HS Phase
    0x20 - YC Swap
    0x10 - CbCr Swap
    0x08 - SyncMod
    0x04 - YUV_RGB
    0x02 - Field O En
    0x01 - Field On
*/

/*
    Address 0x202b r/w (MAIN CPU)

    0x80 - R En
    0x40 - G En
    0x20 - B En
    0x10 - Halftone
    0x08 - B/W
    0x04 - CCIR Depth
    0x02 - CCIR Depth
    0x01 - CCIR Depth
*/

/* Address 0x202c Unused */
/* Address 0x202d Unused */

/*
    Address 0x202e r/w (MAIN CPU)

    0x80 - TRC EN
    0x40 - CCIR EN
    0x20 - Bluescreen EN
    0x10 - Touch EN
    0x08 - CCIR TH
    0x04 - CCIR TH
    0x02 - CCIR TH
    0x01 - CCIR TH
*/

/* Address 0x202f Unused */

/*
    Address 0x2030 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - VDACSW
    0x20 - VDACOUT:5
    0x10 - VDACOUT:4
    0x08 - VDACOUT:3
    0x04 - VDACOUT:2
    0x02 - VDACOUT:1
    0x01 - VDACOUT:0
*/

/*
    Address 0x2031 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - R DAC SW
    0x10 - R DAC OUT:4
    0x08 - R DAC OUT:3
    0x04 - R DAC OUT:2
    0x02 - R DAC OUT:1
    0x01 - R DAC OUT:0
*/

/*
    Address 0x2032 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - G DAC SW
    0x10 - G DAC OUT:4
    0x08 - G DAC OUT:3
    0x04 - G DAC OUT:2
    0x02 - G DAC OUT:1
    0x01 - G DAC OUT:0
*/

/*
    Address 0x2033 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - B DAC SW
    0x10 - B DAC OUT:4
    0x08 - B DAC OUT:3
    0x04 - B DAC OUT:2
    0x02 - B DAC OUT:1
    0x01 - B DAC OUT:0
*/

/************************************************************************************************************************************
 VT1682 Sys Registers
************************************************************************************************************************************/

/*
    Address 0x2100 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 3
    0x04 - Program Bank 1 Register 3
    0x02 - Program Bank 1 Register 3
    0x01 - Program Bank 1 Register 3
*/

/*
    Address 0x2101 r/w (MAIN CPU)

    0x80 - Timer Preload:7
    0x40 - Timer Preload:6
    0x20 - Timer Preload:5
    0x10 - Timer Preload:4
    0x08 - Timer Preload:3
    0x04 - Timer Preload:2
    0x02 - Timer Preload:1
    0x01 - Timer Preload:0
*/

/*
    Address 0x2102 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMR_IRQ
    0x01 - TMR_EN
*/

/*
    Address 0x2103 r/w (MAIN CPU)

    0x80 - Timer IRQ Clear
    0x40 - Timer IRQ Clear
    0x20 - Timer IRQ Clear
    0x10 - Timer IRQ Clear
    0x08 - Timer IRQ Clear
    0x04 - Timer IRQ Clear
    0x02 - Timer IRQ Clear
    0x01 - Timer IRQ Clear
*/

/*
    Address 0x2104 r/w (MAIN CPU)

    0x80 - Timer Preload:15
    0x40 - Timer Preload:14
    0x20 - Timer Preload:13
    0x10 - Timer Preload:12
    0x08 - Timer Preload:11
    0x04 - Timer Preload:10
    0x02 - Timer Preload:9
    0x01 - Timer Preload:8
*/

/*
    Address 0x2105 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - COMR6
    0x20 - TV SYS SE:1
    0x10 - TV SYS SE:0
    0x08 - CCIR SEL
    0x04 - Double
    0x02 - ROM SEL
    0x01 - PRAM
*/

/*
    Address 0x2106 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - SCPURN
    0x10 - SCPU ON
    0x08 - SPI ON
    0x04 - UART ON
    0x02 - TV ON
    0x01 - LCD ON
*/

/*
    Address 0x2107 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 0
    0x40 - Program Bank 0 Register 0
    0x20 - Program Bank 0 Register 0
    0x10 - Program Bank 0 Register 0
    0x08 - Program Bank 0 Register 0
    0x04 - Program Bank 0 Register 0
    0x02 - Program Bank 0 Register 0
    0x01 - Program Bank 0 Register 0
*/

/*
    Address 0x2108 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 1
    0x40 - Program Bank 0 Register 1
    0x20 - Program Bank 0 Register 1
    0x10 - Program Bank 0 Register 1
    0x08 - Program Bank 0 Register 1
    0x04 - Program Bank 0 Register 1
    0x02 - Program Bank 0 Register 1
    0x01 - Program Bank 0 Register 1
*/

/*
    Address 0x2109 r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 2
    0x40 - Program Bank 0 Register 2
    0x20 - Program Bank 0 Register 2
    0x10 - Program Bank 0 Register 2
    0x08 - Program Bank 0 Register 2
    0x04 - Program Bank 0 Register 2
    0x02 - Program Bank 0 Register 2
    0x01 - Program Bank 0 Register 2
*/

/*
    Address 0x210a r/w (MAIN CPU)

    0x80 - Program Bank 0 Register 3
    0x40 - Program Bank 0 Register 3
    0x20 - Program Bank 0 Register 3
    0x10 - Program Bank 0 Register 3
    0x08 - Program Bank 0 Register 3
    0x04 - Program Bank 0 Register 3
    0x02 - Program Bank 0 Register 3
    0x01 - Program Bank 0 Register 3
*/

/*
    Address 0x210b r/w (MAIN CPU)

    0x80 - TSYSN Enable
    0x40 - PQ2 Enable
    0x20 - BUS Tristate
    0x10 - CS Control:1
    0x08 - CS Control:0
    0x04 - Program Bank 0 Select
    0x02 - Program Bank 0 Select
    0x01 - Program Bank 0 Select
*/

/*
    Address 0x210c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 2
    0x04 - Program Bank 1 Register 2
    0x02 - Program Bank 1 Register 2
    0x01 - Program Bank 1 Register 2
*/

/*
    Address 0x210d r/w (MAIN CPU)

    0x80 - IOD ENB
    0x40 - IOD OE
    0x20 - IOC ENB
    0x10 - IOC OE
    0x08 - IOB ENB
    0x04 - IOB OE
    0x02 - IOA ENB
    0x01 - IOA OE
*/

/*
    Address 0x210e r/w (MAIN CPU)

    0x80 - IOB:3
    0x40 - IOB:2
    0x20 - IOB:1
    0x10 - IOB:0
    0x08 - IOA:3
    0x04 - IOA:2
    0x02 - IOA:1
    0x01 - IOA:0
*/

/*
    Address 0x210f r/w (MAIN CPU)

    0x80 - IOD:3
    0x40 - IOD:2
    0x20 - IOD:1
    0x10 - IOD:0
    0x08 - IOC:3
    0x04 - IOC:2
    0x02 - IOC:1
    0x01 - IOC:0
*/

/*
    Address 0x2110 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0
*/

/*
    Address 0x2111 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1
*/

/*
    Address 0x2112 READ ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 0
    0x04 - Program Bank 1 Register 0
    0x02 - Program Bank 1 Register 0
    0x01 - Program Bank 1 Register 0
*/

/*
    Address 0x2113 READ ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - Program Bank 1 Register 1
    0x04 - Program Bank 1 Register 1
    0x02 - Program Bank 1 Register 1
    0x01 - Program Bank 1 Register 1
*/

/*
    Address 0x2114 r/w (MAIN CPU)

    0x80 - Baud Rate:7
    0x40 - Baud Rate:6
    0x20 - Baud Rate:5
    0x10 - Baud Rate:4
    0x08 - Baud Rate:3
    0x04 - Baud Rate:2
    0x02 - Baud Rate:1
    0x01 - Baud Rate:0
*/

/*
    Address 0x2115 r/w (MAIN CPU)

    0x80 - Baud Rate:15
    0x40 - Baud Rate:14
    0x20 - Baud Rate:13
    0x10 - Baud Rate:12
    0x08 - Baud Rate:11
    0x04 - Baud Rate:10
    0x02 - Baud Rate:9
    0x01 - Baud Rate:8
*/

/*
    Address 0x2116 r/w (MAIN CPU)

    0x80 - 16bit SPI
    0x40 - SPIEN
    0x20 - SPI RST
    0x10 - M/SB
    0x08 - CLK PHASE
    0x04 - CLK POLARITY
    0x02 - CLK FREQ:1
    0x01 - CLK FREQ:0
*/

/*
    Address 0x2117 WRITE (MAIN CPU)

    0x80 - SPI TX Data
    0x40 - SPI TX Data
    0x20 - SPI TX Data
    0x10 - SPI TX Data
    0x08 - SPI TX Data
    0x04 - SPI TX Data
    0x02 - SPI TX Data
    0x01 - SPI TX Data

    Address 0x2117 READ (MAIN CPU)

    0x80 - SPI RX Data
    0x40 - SPI RX Data
    0x20 - SPI RX Data
    0x10 - SPI RX Data
    0x08 - SPI RX Data
    0x04 - SPI RX Data
    0x02 - SPI RX Data
    0x01 - SPI RX Data
*/

/*
    Address 0x2118 r/w (MAIN CPU)

    0x80 - Program Bank 1 Register 5
    0x40 - Program Bank 1 Register 5
    0x20 - Program Bank 1 Register 5
    0x10 - Program Bank 1 Register 5
    0x08 - Program Bank 1 Register 4
    0x04 - Program Bank 1 Register 4
    0x02 - Program Bank 1 Register 4
    0x01 - Program Bank 1 Register 4
*/

/*
    Address 0x2119 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - Carrier En
    0x20 - UART En
    0x10 - Tx IRQ En
    0x08 - Rx IRQ En
    0x04 - Parity En
    0x02 - Odd/Even
    0x01 - 9bit Mode
*/

/*
    Address 0x211a WRITE (MAIN CPU)

    0x80 - TX Data
    0x40 - TX Data
    0x20 - TX Data
    0x10 - TX Data
    0x08 - TX Data
    0x04 - TX Data
    0x02 - TX Data
    0x01 - TX Data

    Address 0x211a READ (MAIN CPU)

    0x80 - RX Data
    0x40 - RX Data
    0x20 - RX Data
    0x10 - RX Data
    0x08 - RX Data
    0x04 - RX Data
    0x02 - RX Data
    0x01 - RX Data
*/

/*
    Address 0x211b WRITE (MAIN CPU)

    0x80 - Carrier Freq
    0x40 - Carrier Freq
    0x20 - Carrier Freq
    0x10 - Carrier Freq
    0x08 - Carrier Freq
    0x04 - Carrier Freq
    0x02 - Carrier Freq
    0x01 - Carrier Freq

    Address 0x211b READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Rx Error
    0x10 - Tx Status
    0x08 - Rx Status
    0x04 - Parity Error
    0x02 - (unused)
    0x01 - (unused)
*/

/*
    Address 0x211c WRITE (MAIN CPU)

    0x80 - AutoWake
    0x40 - KeyWake
    0x20 - EXT2421EN
    0x10 - SCPUIRQ
    0x08 - SLEEPM
    0x04 - (unused)
    0x02 - SLEEP SEL
    0x01 - CLK SEL

    Address 0x211c READ (MAIN CPU) (maybe)

    0x80 - Clear_SCPU_IRQ
    0x40 - Clear_SCPU_IRQ
    0x20 - Clear_SCPU_IRQ
    0x10 - Clear_SCPU_IRQ
    0x08 - Clear_SCPU_IRQ
    0x04 - Clear_SCPU_IRQ
    0x02 - Clear_SCPU_IRQ
    0x01 - Clear_SCPU_IRQ
*/

/*
    Address 0x211d WRITE (MAIN CPU)

    0x80 - LVDEN
    0x40 - LVDS1
    0x20 - LVDS0
    0x10 - VDAC_EN
    0x08 - ADAC_EN
    0x04 - PLL_EN
    0x02 - LCDACEN
    0x01 - (unused)

    Address 0x211d READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - LVD
*/

/*
    Address 0x211e WRITE (MAIN CPU)

    0x80 - ADCEN
    0x40 - ADCS1
    0x20 - ADCS0
    0x10 - (unused)
    0x08 - IOFOEN3
    0x04 - IOFOEN2
    0x02 - IOFOEN1
    0x01 - IOFOEN0

    Address 0x211e READ (MAIN CPU)

    0x80 - ADC DATA:7
    0x40 - ADC DATA:6
    0x20 - ADC DATA:5
    0x10 - ADC DATA:4
    0x08 - ADC DATA:3
    0x04 - ADC DATA:2
    0x02 - ADC DATA:1
    0x01 - ADC DATA:0
*/

/*
    Address 0x211f r/w (MAIN CPU)

    0x80 - VGCEN
    0x40 - VGCA6
    0x20 - VGCA5
    0x10 - VGCA4
    0x08 - VGCA3
    0x04 - VGCA2
    0x02 - VGCA1
    0x01 - VGCA0
*/

/*
    Address 0x2120 r/w (MAIN CPU)

    0x80 - Sleep Period
    0x40 - Sleep Period
    0x20 - Sleep Period
    0x10 - Sleep Period
    0x08 - Sleep Period
    0x04 - Sleep Period
    0x02 - Sleep Period
    0x01 - Sleep Period
*/

/*
    Address 0x2121 READ (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - SPI MSK
    0x08 - UART MSK
    0x04 - SPU MSK
    0x02 - TMR MSK
    0x01 - Ext MSK

    Address 0x2121 WRITE (MAIN CPU) (maybe)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - Clear SPU
    0x02 - (unused)
    0x01 - Clear Ext
*/

/*
    Address 0x2122 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:7
    0x40 - DMA DT ADDR:6
    0x20 - DMA DT ADDR:5
    0x10 - DMA DT ADDR:4
    0x08 - DMA DT ADDR:3
    0x04 - DMA DT ADDR:2
    0x02 - DMA DT ADDR:1
    0x01 - DMA DT ADDR:0
*/

/*
    Address 0x2123 r/w (MAIN CPU)

    0x80 - DMA DT ADDR:15
    0x40 - DMA DT ADDR:14
    0x20 - DMA DT ADDR:13
    0x10 - DMA DT ADDR:12
    0x08 - DMA DT ADDR:11
    0x04 - DMA DT ADDR:10
    0x02 - DMA DT ADDR:9
    0x01 - DMA DT ADDR:8
*/

/*
    Address 0x2124 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:7
    0x40 - DMA SR ADDR:6
    0x20 - DMA SR ADDR:5
    0x10 - DMA SR ADDR:4
    0x08 - DMA SR ADDR:3
    0x04 - DMA SR ADDR:2
    0x02 - DMA SR ADDR:1
    0x01 - DMA SR ADDR:0
*/

/*
    Address 0x2125 r/w (MAIN CPU)

    0x80 - DMA SR ADDR:15
    0x40 - DMA SR ADDR:14
    0x20 - DMA SR ADDR:13
    0x10 - DMA SR ADDR:12
    0x08 - DMA SR ADDR:11
    0x04 - DMA SR ADDR:10
    0x02 - DMA SR ADDR:9
    0x01 - DMA SR ADDR:8
*/

/*
    Address 0x2126 r/w (MAIN CPU)

    0x80 - DMA SR BANK:22
    0x40 - DMA SR BANK:21
    0x20 - DMA SR BANK:20
    0x10 - DMA SR BANK:19
    0x08 - DMA SR BANK:18
    0x04 - DMA SR BANK:17
    0x02 - DMA SR BANK:16
    0x01 - DMA SR BANK:15
*/

/*
    Address 0x2127 WRITE (MAIN CPU)

    0x80 - DMA Number
    0x40 - DMA Number
    0x20 - DMA Number
    0x10 - DMA Number
    0x08 - DMA Number
    0x04 - DMA Number
    0x02 - DMA Number
    0x01 - DMA Number

    Address 0x2127 READ (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - (unused)
    0x01 - DMA Status
*/

/*
    Address 0x2128 r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - DMA SR BANK:24
    0x01 - DMA SR BANK:23
*/

/*
    Address 0x2129 READ (MAIN CPU)

    0x80 - UIOA DATA IN / Send Joy CLK
    0x40 - UIOA DATA IN / Send Joy CLK
    0x20 - UIOA DATA IN / Send Joy CLK
    0x10 - UIOA DATA IN / Send Joy CLK
    0x08 - UIOA DATA IN / Send Joy CLK
    0x04 - UIOA DATA IN / Send Joy CLK
    0x02 - UIOA DATA IN / Send Joy CLK
    0x01 - UIOA DATA IN / Send Joy CLK

    Address 0x2129 WRITE (MAIN CPU)

    0x80 - UIOA DATA OUT
    0x40 - UIOA DATA OUT
    0x20 - UIOA DATA OUT
    0x10 - UIOA DATA OUT
    0x08 - UIOA DATA OUT
    0x04 - UIOA DATA OUT
    0x02 - UIOA DATA OUT
    0x01 - UIOA DATA OUT

*/

/*
    Address 0x212a READ (MAIN CPU)

    0x80 - Send Joy CLK 2
    0x40 - Send Joy CLK 2
    0x20 - Send Joy CLK 2
    0x10 - Send Joy CLK 2
    0x08 - Send Joy CLK 2
    0x04 - Send Joy CLK 2
    0x02 - Send Joy CLK 2
    0x01 - Send Joy CLK 2

    Address 0x212a WRITE (MAIN CPU)

    0x80 - UIOA DIRECTION
    0x40 - UIOA DIRECTION
    0x20 - UIOA DIRECTION
    0x10 - UIOA DIRECTION
    0x08 - UIOA DIRECTION
    0x04 - UIOA DIRECTION
    0x02 - UIOA DIRECTION
    0x01 - UIOA DIRECTION
*/

/*
    Address 0x212b r/w (MAIN CPU)

    0x80 - UIOA ATTRIBUTE
    0x40 - UIOA ATTRIBUTE
    0x20 - UIOA ATTRIBUTE
    0x10 - UIOA ATTRIBUTE
    0x08 - UIOA ATTRIBUTE
    0x04 - UIOA ATTRIBUTE
    0x02 - UIOA ATTRIBUTE
    0x01 - UIOA ATTRIBUTE
*/

/*
    Address 0x212c READ (MAIN CPU)

    0x80 - Pseudo Random Number
    0x40 - Pseudo Random Number
    0x20 - Pseudo Random Number
    0x10 - Pseudo Random Number
    0x08 - Pseudo Random Number
    0x04 - Pseudo Random Number
    0x02 - Pseudo Random Number
    0x01 - Pseudo Random Number

    Address 0x212c WRITE (MAIN CPU)

    0x80 - Pseudo Random Number Seed
    0x40 - Pseudo Random Number Seed
    0x20 - Pseudo Random Number Seed
    0x10 - Pseudo Random Number Seed
    0x08 - Pseudo Random Number Seed
    0x04 - Pseudo Random Number Seed
    0x02 - Pseudo Random Number Seed
    0x01 - Pseudo Random Number Seed
*/

/*
    Address 0x212d WRITE ONLY (MAIN CPU)

    0x80 - PLL B
    0x40 - PLL B
    0x20 - PLL B
    0x10 - PLL B
    0x08 - PLL M
    0x04 - PLL A
    0x02 - PLL A
    0x01 - PLL A
*/

/* Address 0x212e Unused */
/* Address 0x212f Unused */

/*
    Address 0x2130 WRITE (MAIN CPU)

    0x80 - ALU Oprand 1
    0x40 - ALU Oprand 1
    0x20 - ALU Oprand 1
    0x10 - ALU Oprand 1
    0x08 - ALU Oprand 1
    0x04 - ALU Oprand 1
    0x02 - ALU Oprand 1
    0x01 - ALU Oprand 1

    Address 0x2130 READ (MAIN CPU)

    0x80 - ALU Output 1
    0x40 - ALU Output 1
    0x20 - ALU Output 1
    0x10 - ALU Output 1
    0x08 - ALU Output 1
    0x04 - ALU Output 1
    0x02 - ALU Output 1
    0x01 - ALU Output 1
*/

/*
    Address 0x2131 WRITE (MAIN CPU)

    0x80 - ALU Oprand 2
    0x40 - ALU Oprand 2
    0x20 - ALU Oprand 2
    0x10 - ALU Oprand 2
    0x08 - ALU Oprand 2
    0x04 - ALU Oprand 2
    0x02 - ALU Oprand 2
    0x01 - ALU Oprand 2

    Address 0x2131 READ (MAIN CPU)

    0x80 - ALU Output 2
    0x40 - ALU Output 2
    0x20 - ALU Output 2
    0x10 - ALU Output 2
    0x08 - ALU Output 2
    0x04 - ALU Output 2
    0x02 - ALU Output 2
    0x01 - ALU Output 2
*/

/*
    Address 0x2132 WRITE (MAIN CPU)

    0x80 - ALU Oprand 3
    0x40 - ALU Oprand 3
    0x20 - ALU Oprand 3
    0x10 - ALU Oprand 3
    0x08 - ALU Oprand 3
    0x04 - ALU Oprand 3
    0x02 - ALU Oprand 3
    0x01 - ALU Oprand 3

    Address 0x2132 READ (MAIN CPU)

    0x80 - ALU Output 3
    0x40 - ALU Output 3
    0x20 - ALU Output 3
    0x10 - ALU Output 3
    0x08 - ALU Output 3
    0x04 - ALU Output 3
    0x02 - ALU Output 3
    0x01 - ALU Output 3
*/

/*
    Address 0x2133 WRITE (MAIN CPU)

    0x80 - ALU Oprand 4
    0x40 - ALU Oprand 4
    0x20 - ALU Oprand 4
    0x10 - ALU Oprand 4
    0x08 - ALU Oprand 4
    0x04 - ALU Oprand 4
    0x02 - ALU Oprand 4
    0x01 - ALU Oprand 4

    Address 0x2133 READ (MAIN CPU)

    0x80 - ALU Output 4
    0x40 - ALU Output 4
    0x20 - ALU Output 4
    0x10 - ALU Output 4
    0x08 - ALU Output 4
    0x04 - ALU Output 4
    0x02 - ALU Output 4
    0x01 - ALU Output 4
*/

/*
    Address 0x2134 WRITE (MAIN CPU)

    0x80 - ALU Mul Oprand 5
    0x40 - ALU Mul Oprand 5
    0x20 - ALU Mul Oprand 5
    0x10 - ALU Mul Oprand 5
    0x08 - ALU Mul Oprand 5
    0x04 - ALU Mul Oprand 5
    0x02 - ALU Mul Oprand 5
    0x01 - ALU Mul Oprand 5

    Address 0x2134 READ (MAIN CPU)

    0x80 - ALU Output 5
    0x40 - ALU Output 5
    0x20 - ALU Output 5
    0x10 - ALU Output 5
    0x08 - ALU Output 5
    0x04 - ALU Output 5
    0x02 - ALU Output 5
    0x01 - ALU Output 5
*/

/*
    Address 0x2135 WRITE (MAIN CPU)

    0x80 - ALU Mul Oprand 6
    0x40 - ALU Mul Oprand 6
    0x20 - ALU Mul Oprand 6
    0x10 - ALU Mul Oprand 6
    0x08 - ALU Mul Oprand 6
    0x04 - ALU Mul Oprand 6
    0x02 - ALU Mul Oprand 6
    0x01 - ALU Mul Oprand 6

    Address 0x2135 READ (MAIN CPU)

    0x80 - ALU Output 6
    0x40 - ALU Output 6
    0x20 - ALU Output 6
    0x10 - ALU Output 6
    0x08 - ALU Output 6
    0x04 - ALU Output 6
    0x02 - ALU Output 6
    0x01 - ALU Output 6
*/

/*
    Address 0x2136 WRITE ONLY (MAIN CPU)

    0x80 - ALU Div Oprand 5
    0x40 - ALU Div Oprand 5
    0x20 - ALU Div Oprand 5
    0x10 - ALU Div Oprand 5
    0x08 - ALU Div Oprand 5
    0x04 - ALU Div Oprand 5
    0x02 - ALU Div Oprand 5
    0x01 - ALU Div Oprand 5

*/

/*
    Address 0x2137 WRITE ONLY (MAIN CPU)

    0x80 - ALU Div Oprand 6
    0x40 - ALU Div Oprand 6
    0x20 - ALU Div Oprand 6
    0x10 - ALU Div Oprand 6
    0x08 - ALU Div Oprand 6
    0x04 - ALU Div Oprand 6
    0x02 - ALU Div Oprand 6
    0x01 - ALU Div Oprand 6
*/

/* Address 0x2138 Unused */
/* Address 0x2139 Unused */
/* Address 0x213a Unused */
/* Address 0x213b Unused */
/* Address 0x213c Unused */
/* Address 0x213d Unused */
/* Address 0x213e Unused */
/* Address 0x213f Unused */

/*
    Address 0x2140 r/w (MAIN CPU)

    0x80 - I2C ID
    0x40 - I2C ID
    0x20 - I2C ID
    0x10 - I2C ID
    0x08 - I2C ID
    0x04 - I2C ID
    0x02 - I2C ID
    0x01 - I2C ID
*/

/*
    Address 0x2141 r/w (MAIN CPU)

    0x80 - I2C ADDR
    0x40 - I2C ADDR
    0x20 - I2C ADDR
    0x10 - I2C ADDR
    0x08 - I2C ADDR
    0x04 - I2C ADDR
    0x02 - I2C ADDR
    0x01 - I2C ADDR
*/

/*
    Address 0x2142 r/w (MAIN CPU)

    0x80 - I2C DATA
    0x40 - I2C DATA
    0x20 - I2C DATA
    0x10 - I2C DATA
    0x08 - I2C DATA
    0x04 - I2C DATA
    0x02 - I2C DATA
    0x01 - I2C DATA
*/

/*
    Address 0x2143 WRITE ONLY (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - I2C CLK SELECT
    0x01 - I2C CLK SELECT
*/

/* Address 0x2144 Unused */
/* Address 0x2145 Unused */
/* Address 0x2146 Unused */
/* Address 0x2147 Unused */

/*
    Address 0x2148 WRITE ONLY (MAIN CPU)

    0x80 - UIOB SEL:7
    0x40 - UIOB SEL:6
    0x20 - UIOB SEL:5
    0x10 - UIOB SEL:4
    0x08 - UIOB SEL:3
    0x04 - (unused)
    0x02 - UIOA MODE
    0x01 - UIOA MODE
*/

/*
    Address 0x2149 WRITE (MAIN CPU)

    0x80 - UIOB DATA OUT
    0x40 - UIOB DATA OUT
    0x20 - UIOB DATA OUT
    0x10 - UIOB DATA OUT
    0x08 - UIOB DATA OUT
    0x04 - UIOB DATA OUT
    0x02 - UIOB DATA OUT
    0x01 - UIOB DATA OUT

    Address 0x2149 READ (MAIN CPU)

    0x80 - UIOB DATA IN
    0x40 - UIOB DATA IN
    0x20 - UIOB DATA IN
    0x10 - UIOB DATA IN
    0x08 - UIOB DATA IN
    0x04 - UIOB DATA IN
    0x02 - UIOB DATA IN
    0x01 - UIOB DATA IN
*/

/*
    Address 0x214a r/w (MAIN CPU)

    0x80 - UIOB DIRECTION
    0x40 - UIOB DIRECTION
    0x20 - UIOB DIRECTION
    0x10 - UIOB DIRECTION
    0x08 - UIOB DIRECTION
    0x04 - UIOB DIRECTION
    0x02 - UIOB DIRECTION
    0x01 - UIOB DIRECTION
*/

/*
    Address 0x214b r/w (MAIN CPU)

    0x80 - UIOB ATTRIBUTE
    0x40 - UIOB ATTRIBUTE
    0x20 - UIOB ATTRIBUTE
    0x10 - UIOB ATTRIBUTE
    0x08 - UIOB ATTRIBUTE
    0x04 - UIOB ATTRIBUTE
    0x02 - UIOB ATTRIBUTE
    0x01 - UIOB ATTRIBUTE
*/

/*
    Address 0x214c r/w (MAIN CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - Keychange Enable
    0x10 - Keychange Enable
    0x08 - IOFEN
    0x04 - (unused)
    0x02 - (unused)
    0x01 - IOEOEN
*/

/*
    Address 0x214d r/w (MAIN CPU)

    0x80 - IOF:3
    0x40 - IOF:2
    0x20 - IOF:1
    0x10 - IOF:0
    0x08 - IOE:3
    0x04 - IOE:2
    0x02 - IOE:1
    0x01 - IOE:0
*/


/************************************************************************************************************************************
 VT1682 Sound CPU Registers
************************************************************************************************************************************/

/*
    Address 0x2100 r/w (SOUND CPU)

    0x80 - Timer A Preload:7
    0x40 - Timer A Preload:6
    0x20 - Timer A Preload:5
    0x10 - Timer A Preload:4
    0x08 - Timer A Preload:3
    0x04 - Timer A Preload:2
    0x02 - Timer A Preload:1
    0x01 - Timer A Preload:0
*/

/*
    Address 0x2101 r/w (SOUND CPU)

    0x80 - Timer A Preload:15
    0x40 - Timer A Preload:14
    0x20 - Timer A Preload:13
    0x10 - Timer A Preload:12
    0x08 - Timer A Preload:11
    0x04 - Timer A Preload:10
    0x02 - Timer A Preload:9
    0x01 - Timer A Preload:8
*/

/*
    Address 0x2102 r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMRA IRQ
    0x01 - TMRA EN
*/

/*
    Address 0x2103 r/w (SOUND CPU)

    0x80 - Timer A IRQ Clear
    0x40 - Timer A IRQ Clear
    0x20 - Timer A IRQ Clear
    0x10 - Timer A IRQ Clear
    0x08 - Timer A IRQ Clear
    0x04 - Timer A IRQ Clear
    0x02 - Timer A IRQ Clear
    0x01 - Timer A IRQ Clear
*/

/* Address 0x2104 Unused (SOUND CPU) */
/* Address 0x2105 Unused (SOUND CPU) */
/* Address 0x2106 Unused (SOUND CPU) */
/* Address 0x2107 Unused (SOUND CPU) */
/* Address 0x2108 Unused (SOUND CPU) */
/* Address 0x2109 Unused (SOUND CPU) */
/* Address 0x210a Unused (SOUND CPU) */
/* Address 0x210b Unused (SOUND CPU) */
/* Address 0x210c Unused (SOUND CPU) */
/* Address 0x210d Unused (SOUND CPU) */
/* Address 0x210e Unused (SOUND CPU) */
/* Address 0x210f Unused (SOUND CPU) */


/*
    Address 0x2110 r/w (SOUND CPU)

    0x80 - Timer B Preload:7
    0x40 - Timer B Preload:6
    0x20 - Timer B Preload:5
    0x10 - Timer B Preload:4
    0x08 - Timer B Preload:3
    0x04 - Timer B Preload:2
    0x02 - Timer B Preload:1
    0x01 - Timer B Preload:0
*/

/*
    Address 0x2111 r/w (SOUND CPU)

    0x80 - Timer B Preload:15
    0x40 - Timer B Preload:14
    0x20 - Timer B Preload:13
    0x10 - Timer B Preload:12
    0x08 - Timer B Preload:11
    0x04 - Timer B Preload:10
    0x02 - Timer B Preload:9
    0x01 - Timer B Preload:8
*/

/*
    Address 0x2112 r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - TMRB IRQ
    0x01 - TMRB EN
*/

/*
    Address 0x2113 r/w (SOUND CPU)

    0x80 - Timer B IRQ Clear
    0x40 - Timer B IRQ Clear
    0x20 - Timer B IRQ Clear
    0x10 - Timer B IRQ Clear
    0x08 - Timer B IRQ Clear
    0x04 - Timer B IRQ Clear
    0x02 - Timer B IRQ Clear
    0x01 - Timer B IRQ Clear
*/

/* Address 0x2114 Unused (SOUND CPU) */
/* Address 0x2115 Unused (SOUND CPU) */
/* Address 0x2116 Unused (SOUND CPU) */
/* Address 0x2117 Unused (SOUND CPU) */

/*
    Address 0x2118 r/w (SOUND CPU)

    0x80 - Audio DAC Left:7
    0x40 - Audio DAC Left:6
    0x20 - Audio DAC Left:5
    0x10 - Audio DAC Left:4
    0x08 - Audio DAC Left:3
    0x04 - Audio DAC Left:2
    0x02 - Audio DAC Left:1
    0x01 - Audio DAC Left:0
*/

/*
    Address 0x2119 r/w (SOUND CPU)

    0x80 - Audio DAC Left:15
    0x40 - Audio DAC Left:14
    0x20 - Audio DAC Left:13
    0x10 - Audio DAC Left:12
    0x08 - Audio DAC Left:11
    0x04 - Audio DAC Left:10
    0x02 - Audio DAC Left:9
    0x01 - Audio DAC Left:8
*/

/*
    Address 0x211a r/w (SOUND CPU)

    0x80 - Audio DAC Right:7
    0x40 - Audio DAC Right:6
    0x20 - Audio DAC Right:5
    0x10 - Audio DAC Right:4
    0x08 - Audio DAC Right:3
    0x04 - Audio DAC Right:2
    0x02 - Audio DAC Right:1
    0x01 - Audio DAC Right:0
*/

/*
    Address 0x211b r/w (SOUND CPU)

    0x80 - Audio DAC Right:15
    0x40 - Audio DAC Right:14
    0x20 - Audio DAC Right:13
    0x10 - Audio DAC Right:12
    0x08 - Audio DAC Right:11
    0x04 - Audio DAC Right:10
    0x02 - Audio DAC Right:9
    0x01 - Audio DAC Right:8
*/

/*
    Address 0x211c WRITE (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - IRQ_OUT
    0x08 - SLEEP
    0x04 - ExtIRQSel
    0x02 - NMI_EN
    0x01 - ExtMask

    Address 0x211c READ (SOUND CPU)

    0x80 - Clear_CPU_IRQ
    0x40 - Clear_CPU_IRQ
    0x20 - Clear_CPU_IRQ
    0x10 - Clear_CPU_IRQ
    0x08 - Clear_CPU_IRQ
    0x04 - Clear_CPU_IRQ
    0x02 - Clear_CPU_IRQ
    0x01 - Clear_CPU_IRQ
*/

/*
    Address 0x211d r/w (SOUND CPU)

    0x80 - (unused)
    0x40 - (unused)
    0x20 - (unused)
    0x10 - (unused)
    0x08 - (unused)
    0x04 - (unused)
    0x02 - IIS Mode
    0x01 - IIS EN
*/

/* Address 0x211E Unused? (maybe) (SOUND CPU) */
/* Address 0x211F Unused (SOUND CPU) */
/* Address 0x2120 Unused (SOUND CPU) */
/* Address 0x2121 Unused (SOUND CPU) */
/* Address 0x2122 Unused (SOUND CPU) */
/* Address 0x2123 Unused (SOUND CPU) */
/* Address 0x2124 Unused (SOUND CPU) */
/* Address 0x2125 Unused (SOUND CPU) */
/* Address 0x2126 Unused (SOUND CPU) */
/* Address 0x2127 Unused (SOUND CPU) */
/* Address 0x2128 Unused (SOUND CPU) */
/* Address 0x2129 Unused (SOUND CPU) */
/* Address 0x212a Unused (SOUND CPU) */
/* Address 0x212b Unused (SOUND CPU) */
/* Address 0x212c Unused (SOUND CPU) */
/* Address 0x212d Unused (SOUND CPU) */
/* Address 0x212e Unused (SOUND CPU) */
/* Address 0x212f Unused (SOUND CPU) */

/*
    Address 0x2130 WRITE (SOUND CPU)

    0x80 - ALU Oprand 1
    0x40 - ALU Oprand 1
    0x20 - ALU Oprand 1
    0x10 - ALU Oprand 1
    0x08 - ALU Oprand 1
    0x04 - ALU Oprand 1
    0x02 - ALU Oprand 1
    0x01 - ALU Oprand 1

    Address 0x2130 READ (SOUND CPU)

    0x80 - ALU Output 1
    0x40 - ALU Output 1
    0x20 - ALU Output 1
    0x10 - ALU Output 1
    0x08 - ALU Output 1
    0x04 - ALU Output 1
    0x02 - ALU Output 1
    0x01 - ALU Output 1
*/

/*
    Address 0x2131 WRITE (SOUND CPU)

    0x80 - ALU Oprand 2
    0x40 - ALU Oprand 2
    0x20 - ALU Oprand 2
    0x10 - ALU Oprand 2
    0x08 - ALU Oprand 2
    0x04 - ALU Oprand 2
    0x02 - ALU Oprand 2
    0x01 - ALU Oprand 2

    Address 0x2131 READ (SOUND CPU)

    0x80 - ALU Output 2
    0x40 - ALU Output 2
    0x20 - ALU Output 2
    0x10 - ALU Output 2
    0x08 - ALU Output 2
    0x04 - ALU Output 2
    0x02 - ALU Output 2
    0x01 - ALU Output 2
*/

/*
    Address 0x2132 WRITE (SOUND CPU)

    0x80 - ALU Oprand 3
    0x40 - ALU Oprand 3
    0x20 - ALU Oprand 3
    0x10 - ALU Oprand 3
    0x08 - ALU Oprand 3
    0x04 - ALU Oprand 3
    0x02 - ALU Oprand 3
    0x01 - ALU Oprand 3

    Address 0x2132 READ (SOUND CPU)

    0x80 - ALU Output 3
    0x40 - ALU Output 3
    0x20 - ALU Output 3
    0x10 - ALU Output 3
    0x08 - ALU Output 3
    0x04 - ALU Output 3
    0x02 - ALU Output 3
    0x01 - ALU Output 3
*/

/*
    Address 0x2133 WRITE (SOUND CPU)

    0x80 - ALU Oprand 4
    0x40 - ALU Oprand 4
    0x20 - ALU Oprand 4
    0x10 - ALU Oprand 4
    0x08 - ALU Oprand 4
    0x04 - ALU Oprand 4
    0x02 - ALU Oprand 4
    0x01 - ALU Oprand 4

    Address 0x2133 READ (SOUND CPU)

    0x80 - ALU Output 4
    0x40 - ALU Output 4
    0x20 - ALU Output 4
    0x10 - ALU Output 4
    0x08 - ALU Output 4
    0x04 - ALU Output 4
    0x02 - ALU Output 4
    0x01 - ALU Output 4
*/

/*
    Address 0x2134 WRITE (SOUND CPU)

    0x80 - ALU Mul Oprand 5
    0x40 - ALU Mul Oprand 5
    0x20 - ALU Mul Oprand 5
    0x10 - ALU Mul Oprand 5
    0x08 - ALU Mul Oprand 5
    0x04 - ALU Mul Oprand 5
    0x02 - ALU Mul Oprand 5
    0x01 - ALU Mul Oprand 5

    Address 0x2134 READ (SOUND CPU)

    0x80 - ALU Output 5
    0x40 - ALU Output 5
    0x20 - ALU Output 5
    0x10 - ALU Output 5
    0x08 - ALU Output 5
    0x04 - ALU Output 5
    0x02 - ALU Output 5
    0x01 - ALU Output 5
*/

/*
    Address 0x2135 WRITE (SOUND CPU)

    0x80 - ALU Mul Oprand 6
    0x40 - ALU Mul Oprand 6
    0x20 - ALU Mul Oprand 6
    0x10 - ALU Mul Oprand 6
    0x08 - ALU Mul Oprand 6
    0x04 - ALU Mul Oprand 6
    0x02 - ALU Mul Oprand 6
    0x01 - ALU Mul Oprand 6

    Address 0x2135 READ (SOUND CPU)

    0x80 - ALU Output 6
    0x40 - ALU Output 6
    0x20 - ALU Output 6
    0x10 - ALU Output 6
    0x08 - ALU Output 6
    0x04 - ALU Output 6
    0x02 - ALU Output 6
    0x01 - ALU Output 6
*/

/*
    Address 0x2136 WRITE ONLY (SOUND CPU)

    0x80 - ALU Div Oprand 5
    0x40 - ALU Div Oprand 5
    0x20 - ALU Div Oprand 5
    0x10 - ALU Div Oprand 5
    0x08 - ALU Div Oprand 5
    0x04 - ALU Div Oprand 5
    0x02 - ALU Div Oprand 5
    0x01 - ALU Div Oprand 5

*/

/*
    Address 0x2137 WRITE ONLY (SOUND CPU)

    0x80 - ALU Div Oprand 6
    0x40 - ALU Div Oprand 6
    0x20 - ALU Div Oprand 6
    0x10 - ALU Div Oprand 6
    0x08 - ALU Div Oprand 6
    0x04 - ALU Div Oprand 6
    0x02 - ALU Div Oprand 6
    0x01 - ALU Div Oprand 6
*/

/* Address 0x2138 Unused (SOUND CPU) */
/* Address 0x2139 Unused (SOUND CPU) */
/* Address 0x213a Unused (SOUND CPU) */
/* Address 0x213b Unused (SOUND CPU) */
/* Address 0x213c Unused (SOUND CPU) */
/* Address 0x213d Unused (SOUND CPU) */
/* Address 0x213e Unused (SOUND CPU) */
/* Address 0x213f Unused (SOUND CPU) */

/*
    Address 0x2140 r/w (SOUND CPU)

    0x80 - IOA DATA
    0x40 - IOA DATA
    0x20 - IOA DATA
    0x10 - IOA DATA
    0x08 - IOA DATA
    0x04 - IOA DATA
    0x02 - IOA DATA
    0x01 - IOA DATA
*/

/*
    Address 0x2141 r/w (SOUND CPU)

    0x80 - IOA DIR
    0x40 - IOA DIR
    0x20 - IOA DIR
    0x10 - IOA DIR
    0x08 - IOA DIR
    0x04 - IOA DIR
    0x02 - IOA DIR
    0x01 - IOA DIR
*/

/*
    Address 0x2142 r/w (SOUND CPU)

    0x80 - IOA PLH
    0x40 - IOA PLH
    0x20 - IOA PLH
    0x10 - IOA PLH
    0x08 - IOA PLH
    0x04 - IOA PLH
    0x02 - IOA PLH
    0x01 - IOA PLH
*/

/* Address 0x2143 Unused (SOUND CPU) */

/*
    Address 0x2144 r/w (SOUND CPU)

    0x80 - IOB DATA
    0x40 - IOB DATA
    0x20 - IOB DATA
    0x10 - IOB DATA
    0x08 - IOB DATA
    0x04 - IOB DATA
    0x02 - IOB DATA
    0x01 - IOB DATA
*/

/*
    Address 0x2145 r/w (SOUND CPU)

    0x80 - IOB DIR
    0x40 - IOB DIR
    0x20 - IOB DIR
    0x10 - IOB DIR
    0x08 - IOB DIR
    0x04 - IOB DIR
    0x02 - IOB DIR
    0x01 - IOB DIR
*/

/*
    Address 0x2146 r/w (SOUND CPU)

    0x80 - IOB PLH
    0x40 - IOB PLH
    0x20 - IOB PLH
    0x10 - IOB PLH
    0x08 - IOB PLH
    0x04 - IOB PLH
    0x02 - IOB PLH
    0x01 - IOB PLH
*/


uint32_t vt_vt1682_state::screen_update(screen_device& screen, bitmap_rgb32& bitmap, const rectangle& cliprect)
{
	return 0;
}

void vt_vt1682_state::vt_vt1682_map(address_map &map)
{
	map(0x0000, 0x1fff).ram();

	map(0x2000, 0x2000).rw(FUNC(vt_vt1682_state::vt1682_2000_r), FUNC(vt_vt1682_state::vt1682_2000_w));

	// 3000-3fff internal ROM if enabled
	map(0x4000, 0xffff).rom().region("mainrom", 0x74000);
}

static INPUT_PORTS_START( intec )
INPUT_PORTS_END

void vt_vt1682_state::vt_vt1682(machine_config &config)
{
	/* basic machine hardware */
	M6502_VT1682(config, m_maincpu, 5000000);
	m_maincpu->set_addrmap(AS_PROGRAM, &vt_vt1682_state::vt_vt1682_map);

	// 6502 sound CPU running at 20mhz

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(256, 256);
	m_screen->set_visarea(0, 256-1, 0, 256-1);
	m_screen->set_screen_update(FUNC(vt_vt1682_state::screen_update));
}

ROM_START( ii8in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii8in1.bin", 0x00000, 0x2000000, CRC(7aee7464) SHA1(7a9cf7f54a350f0853a17459f2dcbef34f4f7c30) ) // 2ND HALF EMPTY

	// possible undumped 0x1000 bytes of Internal ROM
ROM_END

ROM_START( ii32in1 )
	ROM_REGION( 0x2000000, "mainrom", 0 )
	ROM_LOAD( "ii32in1.bin", 0x00000, 0x2000000, CRC(ddee4eac) SHA1(828c0c18a66bb4872299f9a43d5e3647482c5925) )

	// possible undumped 0x1000 bytes of Internal ROM
ROM_END

CONS( 200?, ii8in1,    0,  0,  vt_vt1682,    intec, vt_vt1682_state, empty_init, "Intec", "InterAct 8-in-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
CONS( 200?, ii32in1,   0,  0,  vt_vt1682,    intec, vt_vt1682_state, empty_init, "Intec", "InterAct 32-in-1", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
