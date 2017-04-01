// license:BSD-3-Clause
// copyright-holders:R. Belmont
/***************************************************************************

 GI Classic / GI Classic EX
 (c) 1995, 1998 Konami
 Preliminary driver by R. Belmont

 GI Classic EX Main PCB:
 Main CPU: 68020
 Photo too unclear to identify Konami customs.

 GI Classic EX Satellite PCB:
 Main CPU: 68000-12
 Video: 056832 / 058143 (GX tilemaps)
 Video: 000907 LCD Controller
 
 WANTED: main PCB and any other PCBs for GI Classic EX, plus any and all 
 PCBs for other games also believed to be on this h/w:
 - GI-Classic (1995)
 - GI-Classic Special (1996)
 - GI-Classic WINDS (1996)
 - GI-Classic WINDS EX (1998)
 
 Other "GI" games, list from http://www.konami.jp/am/g1/
 - GI-LEADING SIRE (1999)
 - GI-LEADING SIRE Ver. 2 (2000)
 - GI-LEADING SIRE Ver. 3 (2001)
 - GI-WINNING SIRE (2002)
 - GI-TURFWILD (2003)
 - GI-WINNING SIRE Ver. 2 (2003)
 - GI-TURFWILD 2 (2004)
 - GI-TURFWILD 3 (2005)
 - GI-HORSEPARK (2005)
 - GI-HORSEPARK EX (2006)
 - GI-HORSEPARK EX STD (2006)
 - GI-HORSEPARK GX STD (2009)
 - GI-HORSEPARK GX (2009)
 - GI-Turf TV (2010)
 
***************************************************************************/

#include "emu.h"
#include "cpu/m68000/m68000.h"
#include "video/k054156_k054157_k056832.h"
#include "video/konami_helper.h"
#include "screen.h"
#include "speaker.h"

class giclassic_state : public driver_device
{
public:
	giclassic_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_k056832(*this, "k056832"),
		m_palette(*this, "palette")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<k056832_device> m_k056832;
	required_device<palette_device> m_palette;

	DECLARE_PALETTE_INIT(giclassic);
	
	INTERRUPT_GEN_MEMBER(giclassic_interrupt);

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_giclassic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	K056832_CB_MEMBER(tile_callback);
	
	DECLARE_WRITE16_MEMBER(control_w);
	DECLARE_READ16_MEMBER(vrom_r);

private:
	uint8_t m_control;
};

K056832_CB_MEMBER(giclassic_state::tile_callback)
{
	*color = (*color & 0xf);
}

void giclassic_state::video_start()
{
}

uint32_t giclassic_state::screen_update_giclassic(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);
	screen.priority().fill(0, cliprect);

	m_k056832->tilemap_draw(screen, bitmap, cliprect, 3, 0, 1);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 2, 0, 2);
	m_k056832->tilemap_draw(screen, bitmap, cliprect, 1, 0, 4);
//	m_k056832->tilemap_draw(screen, bitmap, cliprect, 0, 0, 8);

	return 0;
}

INTERRUPT_GEN_MEMBER(giclassic_state::giclassic_interrupt)
{
	if (m_control & 2)
	{
		m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE);
		m_maincpu->set_input_line(M68K_IRQ_3, HOLD_LINE);
	}
}

WRITE16_MEMBER(giclassic_state::control_w)
{
	// bits:
	// 0 = ?
	// 1 = IRQ enable
	// 2 = ?
	// 3 = extra VROM readback bank
	// 4 = screen on?

	m_control = data & 0xff;
}

READ16_MEMBER(giclassic_state::vrom_r)
{
	if (m_control & 8)
	{
		return m_k056832->piratesh_rom_r(space, offset + 0x1000);
	}
	
	return m_k056832->piratesh_rom_r(space, offset);
}

static ADDRESS_MAP_START( satellite_main, AS_PROGRAM, 16, giclassic_state )
	AM_RANGE(0x000000, 0x07ffff) AM_ROM AM_REGION("maincpu", 0)
	AM_RANGE(0x100000, 0x103fff) AM_RAM
	AM_RANGE(0x200000, 0x200fff) AM_RAM_DEVWRITE("palette", palette_device, write) AM_SHARE("palette")
	AM_RANGE(0x800000, 0x801fff) AM_RAM	AM_DEVREADWRITE("k056832", k056832_device, ram_word_r, ram_word_w)
	AM_RANGE(0x900000, 0x90003f) AM_DEVREADWRITE("k056832", k056832_device, word_r, word_w)
	AM_RANGE(0xb00000, 0xb01fff) AM_READ(vrom_r)
	AM_RANGE(0xc00000, 0xc00001) AM_WRITE(control_w)
	AM_RANGE(0xd00000, 0xd0003f) AM_RAM	// these must read/write or 26S (LCD controller) fails
	AM_RANGE(0xe00000, 0xe0001f) AM_DEVWRITE8("k056832", k056832_device, b_w, 0xff00)
	AM_RANGE(0xf00000, 0xf00001) AM_NOP AM_WRITENOP	// watchdog reset
ADDRESS_MAP_END

static INPUT_PORTS_START( giclassic )
INPUT_PORTS_END

void giclassic_state::machine_start()
{
}

void giclassic_state::machine_reset()
{
}

static MACHINE_CONFIG_START( giclassic, giclassic_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M68000, XTAL_20MHz / 2) // PCB is marked "68000 12 MHz", but only visible osc is 20 MHz
	MCFG_CPU_PROGRAM_MAP(satellite_main)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", giclassic_state, giclassic_interrupt)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(59.62)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(600, 384)
	MCFG_SCREEN_VISIBLE_AREA(0, 599, 0, 383)
	MCFG_SCREEN_UPDATE_DRIVER(giclassic_state, screen_update_giclassic)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_ENABLE_SHADOWS()
	MCFG_PALETTE_FORMAT(xxxxBBBBGGGGRRRR)

	MCFG_DEVICE_ADD("k056832", K056832, 0)
	MCFG_K056832_CB(giclassic_state, tile_callback)
	MCFG_K056832_CONFIG("gfx1", K056832_BPP_4PIRATESH, 1, 0, "none")
	MCFG_K056832_PALETTE("palette")
MACHINE_CONFIG_END

ROM_START( giclasex )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* main program */
	ROM_LOAD16_WORD_SWAP( "gsgu760ae01.12t", 0x000000, 0x080000, CRC(f0f9c118) SHA1(1753d53946bc0703d329e4a09c452713b260da75) ) 

	ROM_REGION( 0x100000, "gfx1", 0 )   /* tilemaps */
	ROM_LOAD( "gsgu760ae03.14c", 0x000000, 0x080000, CRC(1663d327) SHA1(98c1a9653d38f4918f78b3a11af0c29c658201f5) ) 
	ROM_LOAD( "gsgu760ae02.14e", 0x080000, 0x080000, CRC(2b9fe163) SHA1(f60190a9689a70d6c5bb14fb46b7ac2267cf0969) ) 
ROM_END

GAME( 1998, giclasex, 0, giclassic, giclassic,  driver_device, 0, 0, "Konami", "GI-Classic EX (satellite terminal)", MACHINE_NOT_WORKING|MACHINE_NO_SOUND_HW)
