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
	Address 0x2001 r (MAIN CPU)

	0x80 - VBLANK
	0x40 - SP ERR
	0x20 - (unused)
	0x10 - (unused)
	0x08 - (unused)
	0x04 - (unused)
	0x02 - (unused)
	0x01 - (unused)
*/

/*
	Address 0x2001 w (MAIN CPU)

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

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x200d r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x200e r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x200f r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2010 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2011 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2012 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2013 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2014 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2015 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2016 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2017 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2018 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2019 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201a r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201b r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201c r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201d r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201e r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x201f r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/


/*
	Address 0x2020 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2021 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2022 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2023 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2024 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2025 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2026 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2027 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2028 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2029 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202a r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202b r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202c r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202d r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202e r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x202f r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2030 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2031 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2032 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
*/

/*
	Address 0x2033 r/w (MAIN CPU)

	0x80 - 
	0x40 - 
	0x20 - 
	0x10 - 
	0x08 - 
	0x04 - 
	0x02 - 
	0x01 - 
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
