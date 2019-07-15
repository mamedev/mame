// license:BSD-3-Clause
// copyright-holders:Ivan Vangelista

/*
TON PUU MAHJONG (Japan) by ANES

TODO:
- ROM banking;
- blitter, 8bpp with hardcoded palette & writes to ROM area!?
- inputs;

- 1x Z0840008PSC Z80 CPU
- 1x 16.000 XTAL near the Z80
- 1x YM2413 sound chip
- 1x 3.579545 XTAL near the YM2413
- 1x Xilinx XC7354 CPLD
- 2x ISSI IS61C64AH 8k x8 SRAM
- 1x HM6265LK-70
- 1x unknown 160 pin device labeled "ANES ORIGINAL SEAL NO. A199."
- 4x bank of 8 dip-switches

Sanma - 3nin-uchi Mahjong is another ANES game confirmed running on the same hardware.
*/

#include "emu.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "cpu/z80/z80.h"
#include "sound/ym2413.h"

class anes_state : public driver_device
{
public:
	anes_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
	{
	}

	void anes(machine_config &config);

private:
	DECLARE_WRITE8_MEMBER(vram_offset_w);
	DECLARE_WRITE8_MEMBER(blit_trigger_w);

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	void io_map(address_map &map);
	void prg_map(address_map &map);

	virtual void machine_start() override;

	uint8_t m_vram_offset[3];
};

uint32_t anes_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	return 0;
}

WRITE8_MEMBER(anes_state::vram_offset_w)
{
	m_vram_offset[offset] = data;
}

WRITE8_MEMBER(anes_state::blit_trigger_w)
{
	/*
	 operation is:
	 checks for bit 4 to be high in port $16
	 writes either 0 or 3 to port $04
	 writes a 0 to port $00
	 writes an offset to ports $0c / $0d / $0e
	 writes 1 to port $0b, writes to program space, writes 2 to port $0b, writes to program space
	 writes a mode to port $0b, writes to trigger port $0a
	 */

	//printf("%02x%02x%02x\n",m_vram_offset[0],m_vram_offset[1],m_vram_offset[2]);
}

void anes_state::prg_map(address_map &map)
{
	map(0x0000, 0xefff).rom();
	map(0xf000, 0xffff).ram();
}

void anes_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map.unmap_value_high();

	map(0x07, 0x07).nopw(); // mux write
	map(0x08, 0x09).w("ym", FUNC(ym2413_device::write));
	map(0x0a, 0x0a).w(FUNC(anes_state::blit_trigger_w));
//  AM_RANGE(0x0b, 0x0b) AM_WRITE(blit_mode_w)
	map(0x0c, 0x0e).w(FUNC(anes_state::vram_offset_w));
	map(0x11, 0x11).portr("DSW1");
	map(0x12, 0x12).portr("DSW2");
	map(0x13, 0x13).portr("DSW3");
	map(0x14, 0x15).nopr(); // mux read
	map(0x16, 0x16).portr("IN0").nopw();
//  AM_RANGE(0xfe, 0xfe) banking? unknown ROM range
}


static INPUT_PORTS_START( anes )
	PORT_START("IN0")
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // blitter busy status
	PORT_BIT( 0xef, IP_ACTIVE_LOW, IPT_UNKNOWN ) // used, coin?

	PORT_START("DSW1")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW1:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW1:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW1:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW1:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW1:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW1:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW1:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW1:8")

	PORT_START("DSW2")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW2:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW2:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW2:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW2:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW2:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW2:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW2:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW2:8")

	PORT_START("DSW3")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW3:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW3:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW3:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW3:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW3:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW3:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW3:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW3:8")

	PORT_START("DSW4")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "SW4:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "SW4:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "SW4:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "SW4:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "SW4:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "SW4:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "SW4:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "SW4:8")
INPUT_PORTS_END


void anes_state::machine_start()
{
}



void anes_state::anes(machine_config &config)
{
	/* basic machine hardware */
	z80_device &maincpu(Z80(config, "maincpu", XTAL(16'000'000) / 2)); // Z0840008PSC
	maincpu.set_addrmap(AS_PROGRAM, &anes_state::prg_map);
	maincpu.set_addrmap(AS_IO, &anes_state::io_map);
	maincpu.set_vblank_int("screen", FUNC(anes_state::irq0_line_hold));

	// all wrong
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(64*8, 32*8);
	screen.set_visarea_full();
	screen.set_screen_update(FUNC(anes_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette").set_entries(0x100);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	YM2413(config, "ym", XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 0.30);
}


ROM_START( tonpuu )
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD( "201.u32", 0x00000, 0x20000, CRC(ace857bb) SHA1(3f65976883c0c514abf73eeed9223ca52a2be410) ) // 27C010

	ROM_REGION(0x100000, "gfx1", 0)
	ROM_LOAD( "202.u33", 0x00000, 0x80000, CRC(4d62a358) SHA1(6edff8e031272cd5a466d9767454093870a0f90a) ) // 27C4001
	ROM_LOAD( "203.u34", 0x80000, 0x80000, CRC(a6068528) SHA1(c988bd1fc2f91befa9d0d39995ba98ef86b5d854) ) // 27C4001
ROM_END


GAME( 200?, tonpuu, 0, anes, anes, anes_state, empty_init, ROT0, "ANES", "Ton Puu Mahjong [BET] (Japan)", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_GRAPHICS )
