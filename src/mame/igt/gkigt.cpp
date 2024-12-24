// license:BSD-3-Clause
// copyright-holders:David Haywood, R. Belmont, Angelo Salese, Dirk Best
/*

IGT Gameking 960

TODO:
- complete QUART devices, and fix "QUART COUNTER NOT RUNNING" error message;
- interrupt system, wants IAC mode from i960;
\- ms3/ms72c/bmoonii acks irq0 from quart2 CIR block only;
- SENET device. Ties a i2c-like bus named netflex for comms with ticket printers, bill acceptors, touch screen ...
\- bmoonii main board has an Actel A1020B + CY7C128A static RAM;
\- CMOS never get properly initialized, tied on SENET?
- all games will eventually print "RTC device: SOFTWARE", expecting an optional device somewhere. Later boards have a RTC62423;
- bmoonii, ms5, ms14, dblheart, mystjag: crashes in i960 with unhandled 00 after RAM error.
\- Do they all need NVRAM setchips?
\- Should probably not get there, the watchdog would kick much earlier ...
- watchdog (ADM691AAN on bmoonii board, "Telltale");
\- All games hangs on soft reset;
- All games are silent;
\- bmoonii has a YM2413 on main board, tied with a XTAL(3'579'545).
\- some extra stuff routes thru QUART chips (namely SRESET comes from there)
\- ymz should require snd roms being hooked up to its internal memory map thru ROM_COPY;

gkigt4 debug hang points:
bp 8035d58,1,{r8=1;g} ; skip QUART not running (will save to NVRAM afterwards)
bp 802491c,1,{g4=1;g} ; buffer at $10000308, irq?
bp 80249f4,1,{g4=1;g} ; ^
bp 80773f4,1,{g4=0;g} ; irq 2?
bp 8177f48,1,{ip+=4;g} ; EEPROM CRC error / identification failure

workaround for init crashes:
`Fatal error: I960: <logged IP>: Unhandled 00`
bp <logged IP>,1,{ ip|=0x08000000;hardreset } ; the first time around,
                                              ; will initialize NVRAM stack to valid values

===================================================================================================

Game King board types:


Common name 038 or 3802

P/N 757-038-0x

EPROM sockets only
  Chip locations
   BASE - U8
   GME1 - U21
   GME2 - U5
   PLX1 - U20
   PLX2 - U4
   CG1  - U48
   CG2  - U47
   SND1-SND3 EPROMs on optional sound board



Common name 039 or 3902

P/N 757-039-0x

EPROM sockets and SIMM slots
  Chip locations:
   BASE  - U39
   GAME1 - U13
   GAME2 - U36
   PLX1  - U14
   PLX2  - U37 (games may use PXLF SIMMs instead of EPROMs)
   CG1   - U30
   CG2   - U53 (games may use CGF SIMM instead of EPROMs)
   PXLF Pixel Memory SIMM - SIMM slots J6, J7 & J8
   CGF CG Memory SIMM - SIMM slot J3

  J4 & J5 Two 120-pin sockets to connect MultiMedia Lite sound board
   SND1-SND4 EPROMs on optional MULTIMEDIA LITE 1 board
   SNDF SIMM on optional MULTIMEDIA LITE 2 board

MULTIMEDIA LITE boards:
 Multimedia Lite 1 - uses up to 4MB on EPROMs to store sound
 Multimedia Lite 2 - uses up to 16MB of SIMM to store sound

 Boards contain:
 Custom programmed Cypress CY37032-125JC CPLD
    32 Macrocells
    32 I/O Pins
     5 Dedicated Inputs
  labeled MML1 REV A (socketed) for EPROM type (4 32pin eprom sockets)
  labeled MML2 REV A (surface mounted) for SIMM type (1 72pin SIMM socket)
 16.9344MHz OSC
 Yamaha YMZ280-B sound chip
 1 3.5mm Audio out jack
 P4 & P5 Two 120pin connectors



Common name 044

P/N 757044

No EPROM or SIMM sockets

ONLY J6 & J7 Two 120-pin sockets to connect classic legacy or enhanced
             memory (flash) adapter boards.



GAME KING DELUXE - MEMORY 1
ASSY NO. 7682710

PCB board that connects to 044 boards via J6 & J7
    Adds the abillity to use legacy 038 EPROM based software
    or 039 EPROM + SIMM software

More chips (from eBay auction):
    2x Philips / NXT 28C94 quad UART (8 serial channels total)
    ADV476 256 color RAMDAC
*/

#include "emu.h"
#include "bus/rs232/rs232.h"
#include "cpu/i960/i960.h"
#include "machine/mc68681.h"
#include "machine/nvram.h"
#include "sound/ymz280b.h"
#include "video/ramdac.h"
#include "sound/ymopl.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class igt_gameking_state : public driver_device
{
public:
	igt_gameking_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_palette(*this, "palette")
		, m_screen(*this, "screen")
		, m_vram(*this, "vram")
		, m_bg_vram(*this, "bg_vram")
		, m_gfxdecode(*this, "gfxdecode")
		, m_quart(*this, "quart%u", 1U)
		, m_opll(*this, "opll")
	{ }

	void igt_gameking(machine_config &config);
	void igt_ms72c(machine_config &config);

private:
	virtual void video_start() override ATTR_COLD;
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void igt_gameking_map(address_map &map) ATTR_COLD;
	void ramdac_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<palette_device> m_palette;
	required_device<screen_device> m_screen;
	required_shared_ptr<uint32_t> m_vram;
	required_shared_ptr<uint32_t> m_bg_vram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device_array<sc28c94_device, 2> m_quart;
	required_device<ym2413_device> m_opll;

	tilemap_t *m_bg_tilemap = nullptr;
	TILE_GET_INFO_MEMBER(get_bg_tile_info);
	void bg_vram_w(offs_t offset, u32 data, u32 mem_mask = ~0);
};



void igt_gameking_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(igt_gameking_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS, 4, 4, 256, 128);
}

// TODO: incomplete, several missing bit meanings
// - for banking bit 19 is trusted by gkigt4, bit 16 for ms3, 18-17 assumed to be linear
// - bits 15-12 are always 0xf?
// - bits 23-20 used, unknown purpose (at least 1 bit of extra tile bank)
TILE_GET_INFO_MEMBER(igt_gameking_state::get_bg_tile_info)
{
	const u32 entry = m_bg_vram[tile_index];
	int const tile = (entry & 0x0fff) | (entry & 0x000f'0000) >> 4;
	int const color = (entry >> 24) & 0xf;

	tileinfo.set(0, tile, color, 0);
}

void igt_gameking_state::bg_vram_w(offs_t offset, u32 data, u32 mem_mask)
{
	COMBINE_DATA(&m_bg_vram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

uint32_t igt_gameking_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	for(int y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		for(int x = cliprect.min_x; x <= cliprect.max_x; x+=4)
		{
			const u32 gfx_data = m_vram[(x + y * 1024) / 4];

			for(int xi = 0; xi < 4; xi++)
			{
				uint32_t const color = (gfx_data >> (xi*8)) & 0xff;
				if (color)
					bitmap.pix(y, x+xi) = m_palette->pen(color);
			}
		}
	}

	return 0;
}

void igt_gameking_state::igt_gameking_map(address_map &map)
{
	map(0x00000000, 0x0007ffff).flags(i960_cpu_device::BURST).rom().region("maincpu", 0);
	map(0x08000000, 0x081fffff).flags(i960_cpu_device::BURST).rom().region("game", 0);
	map(0x08200000, 0x083fffff).flags(i960_cpu_device::BURST).rom().region("pxl", 0);

//  map(0x0ac00000, 0x0ac0000f) - board identifiers?
//  map(0x0ae00000, 0x0ae0000f) /

	// TODO: fix ranges between CMOS and regular work RAM
	// - bmoonii PCB has two M48Z128Y (-> 0x20000 x2)
	// - ms3 maps CMOS ranges at 0x1002'0000-0x1003'ffff only
	map(0x10000000, 0x1001ffff).flags(i960_cpu_device::BURST).ram().share("nvram");
	map(0x10020000, 0x17ffffff).flags(i960_cpu_device::BURST).ram();

	// igtsc writes from 18000000 to 1817ffff, ms3 all the way to 181fffff.
	map(0x18000000, 0x180fffff).flags(i960_cpu_device::BURST).ram().share("vram");
	map(0x18100000, 0x1811ffff).flags(i960_cpu_device::BURST).ram().w(FUNC(igt_gameking_state::bg_vram_w)).share("bg_vram");
	map(0x18120000, 0x181fffff).ram();

//  map(0x18200000, 0x18200003) video related?
	map(0x18200000, 0x18200001).lr16(
		NAME([this] () {
			u16 res = m_screen->vpos();
			if (m_screen->vpos() >= 480)
				res |= 0x400;
			return res;
		}
	));
	map(0x18200002, 0x18200003).lr16(
		NAME([this] () {
			// TODO: unknown value required, checked at "Cold powerup machine setup" in some games
			// coming from Xilinx anyway?

			logerror("%s: version read\n", machine().describe_context());
			return 0x0038;
		}
	));

	// 28000000: MEZ2 SEL, also connected to ymz chip select?
	map(0x28000000, 0x28000003).rw(m_opll, FUNC(ym2413_device::read), FUNC(ym2413_device::write)).umask32(0x00ff00ff);
	// 28010000: first 28C94 QUART (QRT1 SEL)
	map(0x28010000, 0x2801007f).rw(m_quart[0], FUNC(sc28c94_device::read), FUNC(sc28c94_device::write)).umask32(0x00ff00ff);
	// 28020000: SENET SEL
	map(0x280201fa, 0x280201fa).lrw8(
		NAME([this] () {
			logerror("SENET $1fa read\n");
			return 0;
		}),
		NAME([this] (u8 data) {
			logerror("SENET $1fa write %02x\n", data);
		})
	);
	map(0x280201fc, 0x280201fc).lrw8(
		NAME([this] () {
			logerror("SENET $1fc read\n");
			return 0;
		}),
		NAME([this] (u8 data) {
			logerror("SENET $1fc write %02x\n", data);
		})
	);
//  map(0x28020000, 0x280205ff).flags(i960_cpu_device::BURST).ram();
	// 28030000: WCHDOG SEL
	map(0x28030000, 0x28030003).nopr();
	// 28040000: second 28C94 QUART (QRT2 SEL)
	map(0x28040000, 0x2804007f).rw(m_quart[1], FUNC(sc28c94_device::read), FUNC(sc28c94_device::write)).umask32(0x00ff00ff);
	// TODO: these overlays should come from the QUART devices
	map(0x28010038, 0x2801003b).portr("IN1").nopw();
	map(0x28040038, 0x2804003b).portr("IN2").nopw();
	// 28050000: SOUND SEL
	map(0x28050000, 0x28050003).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask32(0x00ff00ff);
	// 28060000: COLOR SEL
	map(0x28060000, 0x28060000).w("ramdac", FUNC(ramdac_device::index_w));
	map(0x28060002, 0x28060002).w("ramdac", FUNC(ramdac_device::pal_w));
	map(0x28060004, 0x28060004).w("ramdac", FUNC(ramdac_device::mask_w));
	// 28070000: OUT SEL

	map(0x3b000000, 0x3b1fffff).flags(i960_cpu_device::BURST).rom().region("snd", 0);

	map(0xa1000000, 0xa1011fff).flags(i960_cpu_device::BURST).ram(); // used by gkkey for restart IAC
}

static INPUT_PORTS_START( igt_gameking )
	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "IN1" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x010000, "IN1-1" )
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x000000, "Door M" ) // Door M
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x000000, "Door C" ) // Door C
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x000000, "Door B" ) // Door B
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0x100000, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Attendant key")
	PORT_BIT( 0x200000, IP_ACTIVE_LOW, IPT_SERVICE ) // Test switch
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x010000, "IN3-1" )
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x010000, 0x010000, "IN4-1" )
	PORT_DIPSETTING(    0x010000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x020000, 0x020000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x020000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x040000, 0x040000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x040000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x080000, 0x080000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x080000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x100000, 0x100000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x100000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x200000, 0x200000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x200000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x400000, 0x400000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x400000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_DIPNAME( 0x800000, 0x800000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x800000, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x000000, DEF_STR( On ) )
	PORT_BIT( 0xff00ff00, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

static const gfx_layout igt_gameking_layout =
{
	4,4,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{ 4, 0, 12, 8 },
	{ STEP4(0, 4*4) },
	4*4*4
};

static GFXDECODE_START( gfx_igt_gameking )
	GFXDECODE_ENTRY( "cg", 0, igt_gameking_layout,   0x0, 16  )
GFXDECODE_END

void igt_gameking_state::ramdac_map(address_map &map)
{
	map(0x000, 0x3ff).rw("ramdac", FUNC(ramdac_device::ramdac_pal_r), FUNC(ramdac_device::ramdac_rgb666_w));
}

void igt_gameking_state::machine_start()
{
}

void igt_gameking_state::machine_reset()
{
	m_quart[0]->ip2_w(1); // needs to be high
}

static DEVICE_INPUT_DEFAULTS_START( terminal )
	DEVICE_INPUT_DEFAULTS( "RS232_TXBAUD", 0xff, RS232_BAUD_38400 )
	DEVICE_INPUT_DEFAULTS( "RS232_RXBAUD", 0xff, RS232_BAUD_38400 )
	DEVICE_INPUT_DEFAULTS( "RS232_DATABITS", 0xff, RS232_DATABITS_8 )
	DEVICE_INPUT_DEFAULTS( "RS232_PARITY", 0xff, RS232_PARITY_NONE )
	DEVICE_INPUT_DEFAULTS( "RS232_STOPBITS", 0xff, RS232_STOPBITS_1 )
DEVICE_INPUT_DEFAULTS_END

void igt_gameking_state::igt_gameking(machine_config &config)
{
	/* basic machine hardware */
	I960(config, m_maincpu, XTAL(24'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &igt_gameking_state::igt_gameking_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(1024, 512);
	m_screen->set_visarea(0, 640-1, 0, 480-1);
	m_screen->set_screen_update(FUNC(igt_gameking_state::screen_update));
	m_screen->set_palette(m_palette);
	// Xilinx used as video chip XTAL(26'666'666) on board

	SC28C94(config, m_quart[0], XTAL(24'000'000) / 6);
	m_quart[0]->irq_cb().set_inputline(m_maincpu, I960_IRQ3);
	m_quart[0]->d_tx_cb().set("diag", FUNC(rs232_port_device::write_txd));

	SC28C94(config, m_quart[1], XTAL(24'000'000) / 6);
	m_quart[1]->irq_cb().set_inputline(m_maincpu, I960_IRQ0);

	rs232_port_device &diag(RS232_PORT(config, "diag", default_rs232_devices, "terminal"));
	diag.rxd_handler().set(m_quart[0], FUNC(sc28c94_device::rx_d_w));
	diag.set_option_device_input_defaults("terminal", DEVICE_INPUT_DEFAULTS_NAME(terminal));

	// TODO: SENET here
	// uses I960_IRQ2

	PALETTE(config, m_palette).set_entries(0x100);

	ramdac_device &ramdac(RAMDAC(config, "ramdac", 0, m_palette));
	ramdac.set_addrmap(0, &igt_gameking_state::ramdac_map);

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_igt_gameking);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	YM2413(config, m_opll, XTAL(3'579'545)).add_route(ALL_OUTPUTS, "mono", 1.0);

	YMZ280B(config, "ymz", XTAL(16'934'400)).add_route(ALL_OUTPUTS, "mono", 1.0); // enhanced sound on optional Media-Lite sub board

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_1);
}


// TODO: standardize ROM labels
// `${socket_name}_${printed_label}.${ROM position}`, ms14 for an example

ROM_START( ms14 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "config_1b5117ax.u8",  0x000000, 0x080000, CRC(0b533749) SHA1(54b37ddcc705dcd92932a9eed5ffb25c55fa0c49) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1_da5017ax.u21", 0x000000, 0x100000, CRC(b7f50a77) SHA1(0f860083b5e354bf904fdfd3c5545650ce5a9ad2) )
	ROM_LOAD16_BYTE( "gme2_da5017ax.u5",  0x000001, 0x100000, CRC(11974fe5) SHA1(f091458a43303ce147ad79212706ef15921f7cfa) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "cg1_1g5043ax.u48", 0x000000, 0x040000, CRC(4d81e8dd) SHA1(fd9afb1179f863536b65613e9168924247822e63) )
	ROM_LOAD16_BYTE( "cg2_1g5043ax.u47", 0x000001, 0x040000, CRC(025695be) SHA1(ed846c1a8bf0c23ae710af6281fdd2612ba08c51) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "px1_1g5043ax.u20", 0x000000, 0x100000, CRC(70829422) SHA1(78e4d40cff86f197334746a68dc08fa2af42a058) )
	ROM_LOAD16_BYTE( "px2_1g5043ax.u4",  0x000001, 0x100000, CRC(968d70e4) SHA1(a786fb12fa242fa5c3052fbadf897e8f4091c506) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "snd_1h5005ax.u6",  0x000000, 0x100000, CRC(8cee9699) SHA1(e40daebf15499abf0eface4ff277c8cbdd5f43be) )
ROM_END

ROM_START( ms3 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3b5060ax.u8",  0x000000, 0x080000, CRC(aff8d874) SHA1(1cb972759ee12c944a1cfdbe68848c9b2e64a4d3) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "ea5006ax.u21", 0x000000, 0x080000, CRC(9109b2e2) SHA1(daa1f01315caf6e08c3cf8b0e4925c86d2cd8dc3) )
	ROM_LOAD16_BYTE( "ea5006ax.u5",  0x000001, 0x080000, CRC(66c33cf6) SHA1(600f75ab112348f43b38cafd6f871559372f2807) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "1g5032ax.u48", 0x000000, 0x040000, CRC(aba6002f) SHA1(2ed51aa8bbc1e703cd63f633d745dfa4fa7f3dd0) )
	ROM_LOAD16_BYTE( "1g5032ax.u47", 0x000001, 0x040000, CRC(605a71ec) SHA1(13fe64c611c0903a7b79d8680de3ac81f3226a67) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "1g5032ax.u20", 0x000000, 0x100000, CRC(517e7478) SHA1(316a8e48ad6502f9508b06f900555d53ef40b464) )
	ROM_LOAD16_BYTE( "1g5032ax.u4",  0x000001, 0x100000, CRC(e67c878f) SHA1(b03f8d28924351e96bb9f24d32f0e4a40a51910c) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "1h5053xx.u6",  0x000000, 0x080000, CRC(6735c65a) SHA1(198cacec5441aa615c0de63a0b4e47265636bcee) )

	ROM_REGION( 0x20000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x020000, CRC(acbbc6d9) SHA1(6e86d24ad3793b41f1f23f80f9bdb22767abc3bf) )
ROM_END

ROM_START( ms5 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "config_1b5045ba.u8",  0x000000, 0x080000, CRC(c13a579f) SHA1(320c2a34f51db05e79687ffbb3ae740fb634db15) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1_da5017ax.u21", 0x000000, 0x100000, CRC(b7f50a77) SHA1(0f860083b5e354bf904fdfd3c5545650ce5a9ad2) )
	ROM_LOAD16_BYTE( "gme2_da5017ax.u5",  0x000001, 0x100000, CRC(11974fe5) SHA1(f091458a43303ce147ad79212706ef15921f7cfa) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "cg1_1g5013bx.u48", 0x000000, 0x040000, CRC(6ad179a3) SHA1(645b04873bfb38222c5a80326c8ad9bf897e75a2 ) )
	ROM_LOAD16_BYTE( "cg2_1g5013bx.u47", 0x000001, 0x040000, CRC(e3a00dc2) SHA1(0afae4ca882f39845fca679187dd9005088a39c2 ) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "px1_1g5013bx.u20", 0x000000, 0x100000, CRC(d9801616) SHA1(87e449afa454d2ed8a415be648c62a95d36b43e3) )
	ROM_LOAD16_BYTE( "px2_1g5013bx.u4",  0x000001, 0x100000, CRC(61271927) SHA1(12fd97d11a569cfe6edb476e186edddb55203344) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "snd_1h5011bx.u6",  0x000000, 0x100000, CRC(79b98d9c) SHA1(e6557eadd53f0fbaa07ced09a6edf6dceeffaa42) )
ROM_END

ROM_START( ms72c )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "3b5019fa multistar 7 2c config.u8", 0x00000, 0x80000, CRC(6c326a31) SHA1(cd8ecc814ef4f379946ab3654dddd508c24ae56c) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "da5001fa gamebase gme1.u21", 0x000000, 0x100000, CRC(4cd63b5f) SHA1(440302a6ac844b453573e358b29c64f2e8ece80e) )
	ROM_LOAD16_BYTE( "da5001fa gamebase gme2.u5",  0x000001, 0x100000, CRC(663df2fe) SHA1(d2ac3129a346450168a9f76431b0fa8b78db3b37) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "1g5019fa multistar 7 pub.u48", 0x000000, 0x80000, CRC(ac50a155) SHA1(50d07ba5ca176c97adde169fda6e6385c8ec8299) )
	ROM_LOAD16_BYTE( "1g5019fa multistar 7 pub.u47", 0x000001, 0x80000, CRC(5fee078b) SHA1(a41591d14fbc12c68d773fbd1ac340d9427d68e9) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "1g5019fa multistar 7 pub.u20", 0x000000, 0x100000, CRC(806ec7d4) SHA1(b9263f942b3d7101797bf87ad18cfddac9582791) )
	ROM_LOAD16_BYTE( "1g5019fa multistar 7 pub.u4",  0x000001, 0x100000, CRC(2e1e9c8a) SHA1(b6992f013f43debf43f4704396fc71e88449e365) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "1h5008fa multistar 7.u6", 0x000000, 0x100000, CRC(69656637) SHA1(28c2cf48856ee4f820146fdbd0f3c7e307892dc6) )

	ROM_REGION( 0x20000, "nvram", 0 )
	ROM_LOAD( "nvram",        0x000000, 0x020000, CRC(b5e42dbc) SHA1(f6afadb6877bca2cef40725b001c7918f9c99359) )
ROM_END

ROM_START( bmoonii )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "base-2b5146ax.u8", 0x00000, 0x80000, CRC(a56e625f) SHA1(f58714dad1788a28acfa0e315730516dc91d60ae) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1-ca5017ax.u21", 0x000000, 0x100000, CRC(bab3994f) SHA1(4ec2cbe92e2f2019970d6bb7fa9fbd767b3001b2) )
	ROM_LOAD16_BYTE( "gme2-ca5017ax.u5",  0x000001, 0x100000, CRC(f1898f14) SHA1(9bcd4ebe09d6f982cb12dc9a6d070cd1195e3320) )

	ROM_REGION( 0x80000, "cg", 0 )
	ROM_LOAD16_BYTE( "cg1-265069ax.u48", 0x000000, 0x40000, CRC(75188c21) SHA1(5e2ff760d68e66369d164c66a97e0fa4edeba101) ) // 1xxxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "cg2-265069ax.u47", 0x000001, 0x40000, CRC(cab7ef14) SHA1(5f904230f41ebf40c138c3d58d4dcf80e631b500) ) // 1xxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "px1-265069ax.u20", 0x000000, 0x080000, CRC(5a5191f4) SHA1(fafc1587b3186f5ecc4cb04529f0fa5d05c3a306) )
	ROM_LOAD16_BYTE( "px2-265069ax.u4",  0x000001, 0x080000, CRC(65d51a0f) SHA1(487be168f94c815dd3e0a871f1b657795ecf0186) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "sound-1h5025ax.u6", 0x000000, 0x080000, CRC(2502d6f6) SHA1(efe1177a6c02778df8ed62e52d1083c105ebe2ce) )
ROM_END

/*
GAME VERSION: G0000143 - GAME DATE: 01/08/98 - GAME TIME: 09:30:00
CONFIG VERSION: I0000114 - CONFIG DATE: 12/10/97 - CONFIG TIME: 14:35:00
PIXEL VERSION: C0000176 - PIXEL DATE: 10/15/97 - PIXEL TIME: 10:30:00
*/
ROM_START( brhino )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "i0000114 base,1-4002.bin", 0x00000, 0x80000, CRC(d8f7a957) SHA1(fb9665534e68c3d1539c50358787ce484fb38684) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000143 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(3584b0b3) SHA1(55eddb4a785fece9f86f173c1933f0c8c53bb3a8) )
	ROM_LOAD16_BYTE( "g0000143 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(64a70488) SHA1(ba683a08fa55dc09c836c4e53538d13cfc76ec8d) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000176 cg1 1 of 4,2-40.bin", 0x000000, 0x080000, CRC(2782968a) SHA1(f62295a75d81179b7314bc4a01d724d2ec38d473) )
	ROM_LOAD16_BYTE( "c0000176 cg2 2 of 4,2-40.bin", 0x000001, 0x080000, CRC(52534609) SHA1(0584965e63ecbbe229c1a7152bdad310224d9b6c) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000176 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(e0fcd660) SHA1(0c179121b8772a331f768bdac9fc58781a341adb) )
	ROM_LOAD16_BYTE( "c0000176 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(d2c0116f) SHA1(a4cd72f0d6d56f1455728af78baf5bfafbcc5b1c) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "swc00002 snd1 1 of 1,1-80.rom1", 0x000000, 0x100000, CRC(d8d41f3d) SHA1(45f3124da07d021361ac84d69d234dc1f0398476) ) // Add-on sound board @ U6
ROM_END

/*
GAME VERSION: G0000176 - GAME DATE: 04/07/98 - GAME TIME: 12:02:00
CONFIG VERSION: I0000123 - CONFIG DATE: 01/15/98 - CONFIG TIME: 15:00:00
PIXEL VERSION: C0000180 - PIXEL DATE: 12/04/97 - PIXEL TIME: 15:00:00
*/
ROM_START( wofigt )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "i0000123 base,1-4002.bin", 0x00000, 0x80000, CRC(7de6ff13) SHA1(33145364fa6df7d772b1931404c7f13a89db267f) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000176 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(b5ffe32d) SHA1(5c120e5157d81edb3b739dcad0fe49a92b4e610b) )
	ROM_LOAD16_BYTE( "g0000176 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(46279275) SHA1(a40180607893ca63de865d3023730f4929e62b67) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000180 cg1 1 of 4,2-40.bin", 0x000000, 0x080000, CRC(24ec6600) SHA1(6e3a8ef0d09b92141bc556b5c00436a0ccc7f294) )
	ROM_LOAD16_BYTE( "c0000180 cg2 2 of 4,2-40.bin", 0x000001, 0x080000, CRC(84f55a7d) SHA1(46cccc382203ec5a28dd0e5a29ba3f27541f883b) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000180 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(5d150993) SHA1(1790b5451e8a8fe8d7f05ab9c2ba4ae9e09af353) )
	ROM_LOAD16_BYTE( "c0000180 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(d86f2e7f) SHA1(8d02aec29f40393948df295f79621d086d20ef89) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "swc00003 snd1 1 of 1,1-80.rom1", 0x000000, 0x100000, CRC(2d75ae0b) SHA1(419c9517cdf17137b032c1446f0526a85e3b2aeb) ) // Add-on sound board @ U6
ROM_END

/*
GAME VERSION: G0000080 - GAME DATE: 01/08/97 - GAME TIME: 18:00:00
CONFIG VERSION: I0000007 - CONFIG DATE: 10/15/96 - CONFIG TIME: 11:40:00
PIXEL VERSION: C0000102 - PIXEL DATE: 10/15/96 - PIXEL TIME: 14:40:00
*/
ROM_START( sup8race )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "i0000007 base,1-4002.bin", 0x00000, 0x80000, CRC(2e9b9c7f) SHA1(6ed33ca50b8a01da46f7d58b774f43fde60a2ef5) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000080 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(1334b795) SHA1(25ae7ad1825f5d27eaf36b4c03245fc86c362e0f) )
	ROM_LOAD16_BYTE( "g0000080 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(58d3c394) SHA1(cff735f66da2874f3a67be13b39c2507fc695a93) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c0000102 cg1 1 of 4,2-20.bin", 0x000000, 0x040000, CRC(234507e4) SHA1(76b1931e9e8877b1fae69b830b03d19b3fea7b07) ) // 1xxxxxxxxxxxxxxxxx = 0x00
	ROM_LOAD16_BYTE( "c0000102 cg2 2 of 4,2-20.bin", 0x000001, 0x040000, CRC(54420c19) SHA1(e3a16d41177b7a209a25eac6dbab2ff396d432ea) ) // 1xxxxxxxxxxxxxxxxx = 0x00

	ROM_REGION32_LE( 0x200000, "pxl", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c0000102 pxl1 3 of 4,2-40.bin", 0x000000, 0x080000, CRC(0dadfc8c) SHA1(9bf634c0e1b57f413d750d0df96dfd491e9a5a2f) )
	ROM_LOAD16_BYTE( "c0000102 pxl2 4 of 4,2-40.bin", 0x000001, 0x080000, CRC(cf69ee9f) SHA1(5edead51c4d537ad134fb7dca0da1e33d81bd6f9) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "swc00004 snd1 1 of 1,1-40.rom1", 0x000000, 0x080000, CRC(0d40d44e) SHA1(3c28db7fc656494cb4271f55f2a5138611a51449) ) // Add-on sound board @ U6
ROM_END

ROM_START( dblheart )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "config_1b5154ax.u8",  0x000000, 0x080000, CRC(53ec9b2b) SHA1(0a2f277a59e9b61df1f38e5c9d7f53bd9d28caf3) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1_ca5017ax.u21", 0x000000, 0x100000, CRC(bab3994f) SHA1(4ec2cbe92e2f2019970d6bb7fa9fbd767b3001b2) )
	ROM_LOAD16_BYTE( "gme2_ca5017ax.u5",  0x000001, 0x100000, CRC(f1898f14) SHA1(9bcd4ebe09d6f982cb12dc9a6d070cd1195e3320) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "cg1_1g5077ax.u48", 0x000000, 0x040000, CRC(18870d22) SHA1(4ead7a84a2e2d3b2e6141ef6f99a5f586779867b) )
	ROM_LOAD16_BYTE( "cg2_1g5077ax.u47", 0x000001, 0x040000, CRC(02395605) SHA1(adbc44d8595ad86265df96d1645b18228081146f) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "px1_1g5077ax.u20", 0x000000, 0x80000, CRC(85598395) SHA1(9b6d09668ff08140a257b75eb41932c6ccca3f4d) )
	ROM_LOAD16_BYTE( "px2_1g5077ax.u4",  0x000001, 0x80000, CRC(339e2f5e) SHA1(b54076c2775e28c91c76c93ea456ebf52b3fede8) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "snd_1h5033ax.u6",  0x000000, 0x080000, CRC(daba6d27) SHA1(b6aaf6436e65a5ad7e28c40a93e2d9de5a806885) )
ROM_END

ROM_START( mystjag )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "config_1b5157ax.u8",  0x000000, 0x080000, CRC(6032a97b) SHA1(6a825d387f972252d574235ed8e461f52088e125) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1_ca5017ax.u21", 0x000000, 0x100000, CRC(bab3994f) SHA1(4ec2cbe92e2f2019970d6bb7fa9fbd767b3001b2) )
	ROM_LOAD16_BYTE( "gme2_ca5017ax.u5",  0x000001, 0x100000, CRC(f1898f14) SHA1(9bcd4ebe09d6f982cb12dc9a6d070cd1195e3320) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "cg1_2g5116xx.u48", 0x000000, 0x040000, CRC(4a1ebbc5) SHA1(f37f1b141338dd872734ecaca8fdfb2c12bcc2cf) )
	ROM_LOAD16_BYTE( "cg2_2g5116xx.u47", 0x000001, 0x040000, CRC(d227f254) SHA1(3ec6839c43fe368c1e318037d45d7ea796718dde) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "px1_2g5116xx.u20", 0x000000, 0x100000, CRC(a40abd1a) SHA1(177b7cbae5c9f7e2063fe1641e203502b4a4c927) )
	ROM_LOAD16_BYTE( "px2_2g5116xx.u4",  0x000001, 0x100000, CRC(55906b92) SHA1(f4c5d5c0e2b2407b9d4383dba7e28cc8341bafc2) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "snd_1h5152xx.u6",  0x000000, 0x100000, CRC(e459e0be) SHA1(21089fd03a1ea8e4ca3963988647fa4414390d0c) )
ROM_END


/*
  Triple Play (M0000230, Spanish)
  (c) IGT 1993-1999

  ROMs u47 and u48 are m27c4001.
  ROMs u8 and keychip are m27c4002.
  ROMS u4, u5, u20 and u21 are m27c801.

*/
ROM_START( tripplay )  // M0000230, Spanish
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "base_4002_67aa.u8",  0x000000, 0x080000, CRC(87728fb8) SHA1(4b40503bc8314883039a3a719842f7f75e04b96f) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "gme1_801_2a29.u21", 0x000000, 0x100000, CRC(0340a568) SHA1(e7dcff8d35c065d7c9e544995bac2d873ea72475) )
	ROM_LOAD16_BYTE( "gme2_801_e9bf.u5",  0x000001, 0x100000, CRC(8137e9da) SHA1(aa4405d3107be26673054eac2066337113594bc7) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "cg1_4001_0f8b.u48", 0x000000, 0x080000, CRC(1842e0bd) SHA1(625f99eece1a89a102156a7f7371c2296ea31d85) )
	ROM_LOAD16_BYTE( "cg2_4001_a0f0.u47", 0x000001, 0x080000, CRC(9b409d86) SHA1(e4a434f110484691042be608ed69017d649d6c12) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "pxl1_801_6206.u20", 0x000000, 0x100000, CRC(35e7f52e) SHA1(9517d513fd75aefdbf2a91ecad5f1f4865a6547d) )
	ROM_LOAD16_BYTE( "pxl2_801_d371.u4",  0x000001, 0x100000, CRC(809cb731) SHA1(00728be2a490d2c86c43224f54c0e9f035ab7cb9) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "snd.u6",  0x000000, 0x080000, NO_DUMP )  // no devices...
ROM_END

/*
GAME VERSION: G0000073 - GAME DATE: 12/06/96 - GAME TIME: 15:20:00
CONFIG VERSION: M0000133 - CONFIG DATE: 01/13/97 - CONFIG TIME: 16:10:00
PIXEL VERSION: C0000074 - PIXEL DATE: 06/07/96 - PIXEL TIME: 14:40:00
*/
ROM_START( igtmg133 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000133 base,1-4002.bin", 0x00000, 0x80000, CRC(292d85e8) SHA1(490a6b5cb055e12534f872c49ae1baf896fdaa02) )

	ROM_REGION32_LE( 0x200000, "game", 0 ) // outputs game CRC error so one or both could be bad dumps
	ROM_LOAD16_BYTE( "g000073 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(5b638c47) SHA1(f55d9925a7c3a4aeab0fb58f0d5a21f78a11fa16) )
	ROM_LOAD16_BYTE( "g000073 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(98e13542) SHA1(9afefad038234a0c308724478899790269325a0c) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASE00 ) // same as igtmg159
	ROM_LOAD16_BYTE( "c0000074 cg1 1 of 4,2-20.bin", 0x000000, 0x040000, CRC(1cda421b) SHA1(c4b4df2a0c60d5bf78b635679a1293003010e15d) )
	ROM_LOAD16_BYTE( "c0000074 cg2 2 of 4,2-20.bin", 0x000001, 0x040000, CRC(ebc14b9d) SHA1(37812e5de9fd1c70b700ad170290ac7e5163a7b2) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as igtmg159
	ROM_LOAD16_BYTE( "c0000074 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(8fb6f1dd) SHA1(67601c63c1b915c21e69e20f8b0734a0aa243f78) )
	ROM_LOAD16_BYTE( "c0000074 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(f1f4c70b) SHA1(1335565e4ac6830f89f7c0dabbfe7ad9fd667e64) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd", 0x000000, 0x100000, NO_DUMP )  // no sound ROMs were included (could also be 2 ROMs)
ROM_END

/*
GAME VERSION: G0000120 - GAME DATE: 05/15/97 - GAME TIME: 13:30:00
CONFIG VERSION: M0000156 - CONFIG DATE: 04/15/97 - CONFIG TIME: 09:55:00
PIXEL VERSION: C0000074 - PIXEL DATE: 06/07/96 - PIXEL TIME: 14:40:00
*/
ROM_START( igtmg156 ) // was called Game King 1 in the archive, to be verified once it boots
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000156 base,1-4002.bin", 0x00000, 0x80000, CRC(2a73d7bc) SHA1(d83053161e0d67da02c0d3f2ffa23edcf92bd5cb) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000120 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(369d3d98) SHA1(90080f3f20498f4f61487f1c9f552cfb52dd0eeb) )
	ROM_LOAD16_BYTE( "g0000120 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(8aec46ef) SHA1(e9bb853ddbeb701af7e64bab6ff6d448ee5f8416) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASE00 ) // same as igtmg133 and igtmg159
	ROM_LOAD16_BYTE( "c0000074 cg1 1 of 4,2-20.bin", 0x000000, 0x040000, CRC(1cda421b) SHA1(c4b4df2a0c60d5bf78b635679a1293003010e15d) )
	ROM_LOAD16_BYTE( "c0000074 cg2 2 of 4,2-20.bin", 0x000001, 0x040000, CRC(ebc14b9d) SHA1(37812e5de9fd1c70b700ad170290ac7e5163a7b2) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as igtmg133 and igtmg159
	ROM_LOAD16_BYTE( "c0000074 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(8fb6f1dd) SHA1(67601c63c1b915c21e69e20f8b0734a0aa243f78) )
	ROM_LOAD16_BYTE( "c0000074 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(f1f4c70b) SHA1(1335565e4ac6830f89f7c0dabbfe7ad9fd667e64) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd", 0x000000, 0x100000, NO_DUMP )  // no sound ROMs were included (could also be 2 ROMs)
ROM_END

/*
GAME VERSION: G0000139 - GAME DATE: 08/15/97 - GAME TIME: 10:40:00
CONFIG VERSION: M0000159 - CONFIG DATE: 04/23/97 - CONFIG TIME: 16:15:00
PIXEL VERSION: C0000074 - PIXEL DATE: 06/07/96 - PIXEL TIME: 14:40:00
*/
ROM_START( igtmg159 ) // was called New Multi Game in the archive, to be verified once it boots
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000159 base,1-4002.bin", 0x00000, 0x80000, CRC(4762259a) SHA1(ea101a1626172415e66bd48700aebdabb74679c3) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000139 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(3c87cadd) SHA1(3c018111f88ca37a049414426a74d9afe3215768) )
	ROM_LOAD16_BYTE( "g0000139 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(b39fe864) SHA1(3e53d848e9d8c1ed5f07c7b191f12704b58bb2ad) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASE00 )
	ROM_LOAD16_BYTE( "c0000074 cg1 1 of 4,2-20.bin", 0x000000, 0x040000, CRC(1cda421b) SHA1(c4b4df2a0c60d5bf78b635679a1293003010e15d) )
	ROM_LOAD16_BYTE( "c0000074 cg2 2 of 4,2-20.bin", 0x000001, 0x040000, CRC(ebc14b9d) SHA1(37812e5de9fd1c70b700ad170290ac7e5163a7b2) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as igtmg133 and igtmg159
	ROM_LOAD16_BYTE( "c0000074 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(8fb6f1dd) SHA1(67601c63c1b915c21e69e20f8b0734a0aa243f78) )
	ROM_LOAD16_BYTE( "c0000074 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(f1f4c70b) SHA1(1335565e4ac6830f89f7c0dabbfe7ad9fd667e64) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd", 0x000000, 0x100000, NO_DUMP )  // no sound ROMs were included (could also be 2 ROMs)
ROM_END

/*
GAME VERSION: G0000152 - GAME DATE: 11/17/97 - GAME TIME: 23:59:11
CONFIG VERSION: M0000164 - CONFIG DATE: 06/18/97 - CONFIG TIME: 23:59:11
PIXEL VERSION: C0000136 - PIXEL DATE: 06/02/97 - PIXEL TIME: 23:59:00
*/
ROM_START( igtmg164 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000164 base,1-4002.bin", 0x00000, 0x80000, CRC(79b438d5) SHA1(fef5e14f4f83f663adde33b5d6453399a712ff47) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000152 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(ff42cd6d) SHA1(22c93cada24c9a6e9b9e3b8ae07b542c4c52c34d) )
	ROM_LOAD16_BYTE( "g0000152 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(37c8d93c) SHA1(a70cc1e6cea02ef6be062889633e1e25268d643e) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000136 cg1 1 of 4,2-40.bin", 0x000000, 0x080000, CRC(9815e9bc) SHA1(d661bf807abcecff697640f485ab854cf9ed0fa6) )
	ROM_LOAD16_BYTE( "c0000136 cg2 2 of 4,2-40.bin", 0x000001, 0x080000, CRC(cd622938) SHA1(9f235b7fccda20468925cc6487212107d63d750c) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000136 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(6f20559b) SHA1(218f59f434ccb2df56e41f2371d1af0951ff48a4) ) // 2ND HALF = 00xx
	ROM_LOAD16_BYTE( "c0000136 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(2ecbc0f7) SHA1(14c0b1ee1dc2005983d64227ee85c6676c26eb7b) ) // 2ND HALF = 00xx

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd", 0x000000, 0x100000, NO_DUMP )  // no sound ROMs were included (could also be 2 ROMs)
ROM_END

/*
GAME VERSION: G0000109 - GAME DATE: 05/02/97 - GAME TIME: 16:26:00
CONFIG VERSION: M0000166 - CONFIG DATE: 05/21/97 - CONFIG TIME: 17:15:00
PIXEL VERSION: C0000074 - PIXEL DATE: 06/07/96 - PIXEL TIME: 14:40:00
*/
ROM_START( igtmg166 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000166 base,1-4002.bin", 0x00000, 0x80000, CRC(bb2abc98) SHA1(7a8da6772c9e0a8cd9568b0f04ec21e33a1de004) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000109 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(68775094) SHA1(c3c3eb747e9c78e2cdace43b0ca25ce38d649df8) )
	ROM_LOAD16_BYTE( "g0000109 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(37af5837) SHA1(d8973ecfd353e0987ad2e35da45a68e0e341dde7) )

	ROM_REGION( 0x100000, "cg", ROMREGION_ERASE00 ) // same as igtmg133 and igtmg159
	ROM_LOAD16_BYTE( "c0000074 cg1 1 of 4,2-20.bin", 0x000000, 0x040000, CRC(1cda421b) SHA1(c4b4df2a0c60d5bf78b635679a1293003010e15d) )
	ROM_LOAD16_BYTE( "c0000074 cg2 2 of 4,2-20.bin", 0x000001, 0x040000, CRC(ebc14b9d) SHA1(37812e5de9fd1c70b700ad170290ac7e5163a7b2) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as igtmg133 and igtmg159
	ROM_LOAD16_BYTE( "c0000074 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(8fb6f1dd) SHA1(67601c63c1b915c21e69e20f8b0734a0aa243f78) )
	ROM_LOAD16_BYTE( "c0000074 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(f1f4c70b) SHA1(1335565e4ac6830f89f7c0dabbfe7ad9fd667e64) )

	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASE00 )
	ROM_LOAD( "snd", 0x000000, 0x100000, NO_DUMP )  // no sound ROMs were included (could also be 2 ROMs)
ROM_END

/*
GAME VERSION: G0000177 - GAME DATE: 10/10/98 - GAME TIME: 17:38:00
CONFIG VERSION: M0000214 - CONFIG DATE: 08/19/98 - CONFIG TIME: 12:16:11
PIXEL VERSION: C0000235 - PIXEL DATE: 09/16/98 - PIXEL TIME: 11:25:00
*/
ROM_START( igtmg214 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000214 base,1-4002.bin", 0x00000, 0x80000, CRC(f832ddd3) SHA1(6f4ba8e2967091499e84d860a7a95cd2d5a1ee6d) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000177 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(57d530c7) SHA1(ea67b7b96b6d007ffa793a2cb209c1d0df0b5ae8) )
	ROM_LOAD16_BYTE( "g0000177 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(8339edd7) SHA1(f7167ce39669a3cd6ada3be351cebb51b2fd9938) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000235 cg1 1 of 4,2-40.bin", 0x000000, 0x080000, CRC(4b0e06d6) SHA1(33c158e7a857a2237fdd4a80b67abc158763dca1) )
	ROM_LOAD16_BYTE( "c0000235 cg2 2 of 4,2-40.bin", 0x000001, 0x080000, CRC(3e78e5be) SHA1(382a5d4c663d8132f1b17b24f6981a75cec2f0c4) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000235 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(29819cd7) SHA1(07243f6c6175b01bfdcbabe411ebec11d73aa289) )
	ROM_LOAD16_BYTE( "c0000235 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(60b92b5e) SHA1(1b8aa0d16b16705c1a7a1869f15d62da6f0c6800) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "swc00030 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(12af1bc9) SHA1(c0d17da6aa45e4d2a8986e8ab043673733325eda) )
	ROM_LOAD( "swc00030 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(9c68744f) SHA1(6f33b6d87ca8c6340c9ec612a6419e4e1fa6d6c3) )
ROM_END

/*
GAME VERSION: G0000397 - GAME DATE: 12/16/99 - GAME TIME: 12:44:00
CONFIG VERSION: M0000247 - CONFIG DATE: 05/03/99 - CONFIG TIME: 13:41:00
PIXEL VERSION: C0000265 - PIXEL DATE: 05/05/99 - PIXEL TIME: 17:15:00
*/
ROM_START( igtmg247 ) // key00017 was in the archive
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000247 base,1-4002.bin", 0x00000, 0x80000, CRC(03cf1431) SHA1(3e7f0a1ed192ffe353424713546c2671725f1c88) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000397 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(c9429c7b) SHA1(15d97bbbe356fa5018172d3ea1afb975b89a9840) )
	ROM_LOAD16_BYTE( "g0000397 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(e56c617e) SHA1(80189ae9f9cca88500a038e8d729ba28b6bcf842) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000265 cg1 1 of 4,2-x0.bin", 0x000000, 0x080000, NO_DUMP ) // not included for this set (x is 2==27C020, 4==270C40, 8==27C080 ROM type)
	ROM_LOAD16_BYTE( "c0000265 cg2 2 of 4,2-x0.bin", 0x000001, 0x080000, NO_DUMP ) // not included for this set

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000265 pxl1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(88ed5614) SHA1(6dde8956d344b7aa699167fcd0260d074a74287e) )
	ROM_LOAD16_BYTE( "c0000265 pxl2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(b535d175) SHA1(c253344ed54a072cb6d35d816927d80c12565ab8) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4ms and others
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

/*
GAME VERSION: G0000619 - GAME DATE: 06/30/00 - GAME TIME: 11:00:11
CONFIG VERSION: M0000394 - CONFIG DATE: 10/29/99 - CONFIG TIME: 14:00:00
PIXEL VERSION: C0000351 - PIXEL DATE: 02/16/00 - PIXEL TIME: 15:40:00
*/
ROM_START( igtmg394 ) // key00017 was in the archive
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000394 base,1-4002.u8", 0x00000, 0x80000, CRC(d6c48b14) SHA1(08b9a822a6822a94afcac843ff3740bbd4c53f08) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000619 gme1 1 of 2,2-80.u21", 0x000000, 0x100000, CRC(64048a70) SHA1(74ab372686b3a79d09f105a1e78aba86f5d728f8) )
	ROM_LOAD16_BYTE( "g0000619 gme2 2 of 2,2-80.u5",  0x000001, 0x100000, CRC(150315bc) SHA1(75e41a483b88e0b31c5e58a294261f4b484a4576) )

	ROM_REGION( 0x100000, "cg", 0 ) // same as gkigt4ms and others
	ROM_LOAD16_BYTE( "c000351 cg1 1 of 4,2-40,ms.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "c000351 cg2 2 of 4,2-40,ms.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c000351 pxl1 3 of 4,2-80,ms.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "c000351 pxl2 4 of 4,2-80,ms.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // Notes say snd1.u6 & snd2.u7 are swc0046 and "not used" but don't match known swc0046 sets
	ROM_LOAD( "snd1.u6", 0x000000, 0x100000, CRC(9c40caa2) SHA1(14c3ce95e09411325d219377dadf754a8bc9fad6) ) // The listed checksum8 didn't match checksum8 for swc0046 ROMs
	ROM_LOAD( "snd2.u7", 0x100000, 0x100000, CRC(eaf1b8df) SHA1(b336afdb0edbc8f864f873285b29e9998819c782) ) // The listed checksum8 didn't match checksum8 for swc0046 ROMs
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) ) // load in swc0046 sound ROMs because they were specifically mentioned in readme / notes
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

/*
GAME VERSION: G0001125 - GAME DATE: 11/27/01 - GAME TIME: 15:11:00
CONFIG VERSION: M0000535 - CONFIG DATE: 02/01/01 - CONFIG TIME: 10:30:00
PIXEL VERSION: C0000351 - PIXEL DATE: 02/16/00 - PIXEL TIME: 15:40:00
*/
ROM_START( igtmg535 ) // close to gkigtez?  Same graphics & sound, game ROMs G0001126 vs this games G0001125
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000535 base,1-4002.bin", 0x00000, 0x80000, CRC(4577258e) SHA1(ad3b907727f7cef71d73e58ec4e43ff6bb092129) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001125 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(d4d2e987) SHA1(4c0ffc02d7dcc8f94828763e12e61d926d30d749) )
	ROM_LOAD16_BYTE( "g0001125 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(9068fbac) SHA1(223efff9823cd92a4f83b74986c47bc2a8e4420f) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c000351 cg1 1 of 4,2-40,ms.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "c000351 cg2 2 of 4,2-40,ms.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c000351 pxl1 3 of 4,2-80,ms.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "c000351 pxl2 4 of 4,2-80,ms.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt4 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000527 base,1-4002.bin", 0x00000, 0x80000, CRC(73981260) SHA1(24b42ae2796034815d35294efe0ac3d5c33100bd) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001777 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(99d5829d) SHA1(b2ec16f35503ba6a0a41221fb3f52c5d2223ad79) )
	ROM_LOAD16_BYTE( "g0001777 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(3b7dfcc0) SHA1(2aeb35125c4320ba3198c44418c90fa6fd6270a9) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000330 cg1 1 of 4,2-40.bin", 0x000000, 0x80000, CRC(b92b8aa4) SHA1(05a1feac4012a73777eb28ab6e66e1dcadb9430f) )
	ROM_LOAD16_BYTE( "c0000330 cg2 2 of 4,2-40.bin", 0x000001, 0x80000, CRC(4e0560b5) SHA1(109f0bd47cfb0ed593fc34c5904bc639b0097d12))

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000330 plx1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(806ec7d4) SHA1(b9263f942b3d7101797bf87ad18cfddac9582791) )
	ROM_LOAD16_BYTE( "c0000330 plx2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(c4ce5dc5) SHA1(cc5d090e88551550787b87d80aafe18ee1661dd7) )

	ROM_REGION32_LE( 0x200000, "snd", 0 )
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt4ms )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m000526 base,1-4002,ms.u39", 0x00000, 0x80000, CRC(4d095df5) SHA1(bd0cdc4c1b07ef2723ba22b14abaf581b017f190) )

	ROM_REGION32_LE( 0x200000, "game", 0 ) // same as gkigt4
	ROM_LOAD16_BYTE( "g0001777 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(99d5829d) SHA1(b2ec16f35503ba6a0a41221fb3f52c5d2223ad79) )
	ROM_LOAD16_BYTE( "g0001777 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(3b7dfcc0) SHA1(2aeb35125c4320ba3198c44418c90fa6fd6270a9) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c000351 cg1 1 of 4,2-40,ms.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "c000351 cg2 2 of 4,2-40,ms.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c000351 pxl1 3 of 4,2-80,ms.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "c000351 pxl2 4 of 4,2-80,ms.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt43 )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000837 base,1-4002.bin", 0x00000, 0x80000, CRC(98841e5c) SHA1(3b04bc9bc170cfcc6145dc601a63bd1394a62897) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0002142 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(704ef406) SHA1(3f8f719342874243d479011372786a9b6b14f5b1) )
	ROM_LOAD16_BYTE( "g0002142 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(3a576a75) SHA1(d2de1b61808412fb2fe68400387dcdcb7910a770) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000793 cg1 1 of 4,2-40.bin", 0x000000, 0x80000, CRC(582137cc) SHA1(66686a2332a3844f816cf7e988a346f5f593d8f6) )
	ROM_LOAD16_BYTE( "c0000793 cg2 2 of 4,2-40.bin", 0x000001, 0x80000, CRC(5e0b6310) SHA1(4bf718dc9859e8c10c9dca967185c57738249319) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000793 plx1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(6327a76e) SHA1(01ad5747788389d3d9d71a1c37472d33db3ba5fb) )
	ROM_LOAD16_BYTE( "c0000793 plx2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(5a400e90) SHA1(c01be47d03e9ec418d0e4e1293fcf2c890301430) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt43n )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000811 base,1-4002,nj.bin", 0x00000, 0x80000,  CRC(4c659923) SHA1(4624179320cb284516980e2d3caea6fd45c3f967) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001624 gme1 1 of 2,2-80,nj.bin", 0x000000, 0x100000, CRC(4aa4139b) SHA1(c3e13c84cc13d44de90a03d0b5d45f46d4f794ce) )
	ROM_LOAD16_BYTE( "g0001624 gme2 2 of 2,2-80,nj.bin", 0x000001, 0x100000, CRC(5b3bb8bf) SHA1(271131f06944074bedab7fe7c80fce1e2136c385) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000770 cg1 1 of 4,2-40,nj.bin", 0x000000, 0x80000, CRC(35847c45) SHA1(9f6192a9cb43df1a32d13d09248f10d62cd5ad3c) )
	ROM_LOAD16_BYTE( "c0000770 cg2 2 of 4,2-40,nj.bin", 0x000001, 0x80000, CRC(2207af01) SHA1(6f59d624fbbae56af081f2a2f4eb3f7a6e6c0ec1) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000770 plx1 3 of 4,2-80,nj.bin", 0x000000, 0x100000, CRC(d1e673cd) SHA1(22d0234e3efb5238d60c9aab4ffc171f28f5abac) )
	ROM_LOAD16_BYTE( "c0000770 plx2 4 of 4,2-80,nj.bin", 0x000001, 0x100000, CRC(d99074f3) SHA1(a5829761f558f8e543a1442128c0ae3520d42318) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigtez )
	ROM_REGION( 0x80000, "maincpu", 0 ) // same as gkigt4ms
	ROM_LOAD( "m000526 base,1-4002,ms.u39", 0x00000, 0x80000, CRC(4d095df5) SHA1(bd0cdc4c1b07ef2723ba22b14abaf581b017f190) ) /* Use KEY00017 for set up */

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001126 gme1 1 of 2,2-80.u13", 0x000000, 0x100000, CRC(e9f08ea7) SHA1(701ba65aa96f857f487344478cbd1cc2fb38b73c) )
	ROM_LOAD16_BYTE( "g0001126 gme2 2 of 2,2-80.u36", 0x000001, 0x100000, CRC(0384f652) SHA1(b8b7d874a21b583b77612f3daeaa27936302aee0) )

	ROM_REGION( 0x100000, "cg", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "c000351 cg1 1 of 4,2-40,ms.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "c000351 cg2 2 of 4,2-40,ms.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "c000351 pxl1 3 of 4,2-80,ms.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "c000351 pxl2 4 of 4,2-80,ms.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigtezms )
	ROM_REGION( 0x80000, "maincpu", 0 ) // same as gkigt4ms
	ROM_LOAD( "m000526 base,1-4002,ms.u39", 0x00000, 0x80000, CRC(4d095df5) SHA1(bd0cdc4c1b07ef2723ba22b14abaf581b017f190) ) /* Use KEY00017 for set up */

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0002955 gme1 1 of 2,2-80,ms.u13", 0x000000, 0x100000, CRC(472c04a1) SHA1(00b7784d254390475c9aa1beac1700c42514cbed) )
	ROM_LOAD16_BYTE( "g0002955 gme2 2 of 2,2-80,ms.u36", 0x000001, 0x100000, CRC(16903e65) SHA1(eb01c0f88212e8e35c35f897f17e12e859255270) )

	ROM_REGION( 0x100000, "cg", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "c000351 cg1 1 of 4,2-40,ms.u30", 0x000000, 0x80000, CRC(2e841b28) SHA1(492b54e092b0d4028fd8edcb981bd1fd25dca47d) )
	ROM_LOAD16_BYTE( "c000351 cg2 2 of 4,2-40,ms.u53", 0x000001, 0x80000, CRC(673fc86c) SHA1(4d844330c5602d725253b4f78781fa9e213b8556) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 ) // same as gkigt4ms
	ROM_LOAD16_BYTE( "c000351 pxl1 3 of 4,2-80,ms.u14", 0x000000, 0x100000, CRC(438fb625) SHA1(369c860dffa323c2e9be155da1989252f6b0e694) )
	ROM_LOAD16_BYTE( "c000351 pxl2 4 of 4,2-80,ms.u37", 0x000001, 0x100000, CRC(22ec9c65) SHA1(bd944ae79faa8ceb73ed8f6f244fce6ff543ccd1) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

ROM_START( gkigt5p )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "m0000761 base,1-4002.bin", 0x00000, 0x80000, CRC(efac4e4f) SHA1(0cf5b3eead66a791701a504330d9154e8f4d657d) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001783 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(f6672841) SHA1(1f8fe98b931e7fd67e5cd56e193c44acabcb7c0a) )
	ROM_LOAD16_BYTE( "g0001783 gme1 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(639de8c0) SHA1(ad4fb79f12bf19b4b39691cda9f5e61f32fa2dd5) )

	ROM_REGION( 0x100000, "cg", 0 )
	ROM_LOAD16_BYTE( "c0000517 cg1 1 of 4,2-40.bin", 0x000000, 0x80000, CRC(26db44c9) SHA1(8afe145d1fb7535c651d78b23872b71c2c946509) )
	ROM_LOAD16_BYTE( "c0000517 cg2 2 of 4,2-40.bin", 0x000001, 0x80000, CRC(3554ba38) SHA1(6e0b8506943559dbee4cfa7c9e4b60590c6529fb) )

	ROM_REGION32_LE( 0x200000, "pxl", 0 )
	ROM_LOAD16_BYTE( "c0000517 plx1 3 of 4,2-80.bin", 0x000000, 0x100000, CRC(956ba40c) SHA1(7d8ae934ef663ea6b3f342455d1e8c70a1ca4581) )
	ROM_LOAD16_BYTE( "c0000517 plx2 4 of 4,2-80.bin", 0x000001, 0x100000, CRC(dff43975) SHA1(e1ca212e4e51175bcbab2af447863605f74ba77f) )

	ROM_REGION32_LE( 0x200000, "snd", 0 ) // same as gkigt4
	ROM_LOAD( "swc00046 snd1 1 of 2,2-80.rom1", 0x000000, 0x100000, CRC(8213aeac) SHA1(4beff02fed64e607270e0e8e322a96f112bd2093) )
	ROM_LOAD( "swc00046 snd2 2 of 2,2-80.rom2", 0x100000, 0x100000, CRC(a7ef9b46) SHA1(031373fb8e39c4ed828a58bb63a9395a205c6b6b) )
ROM_END

/*
GAME VERSION: G0000912 - GAME DATE: 02/27/01 - GAME TIME: 09:02:10
CONFIG VERSION: I0000500 - CONFIG DATE: 07/11/00 - CONFIG TIME: 17:05:30
PIXEL VERSION: C000???? - PIXEL DATE: ??/??/?? - PIXEL TIME: ??:??:??
*/
ROM_START( munsters ) // key00017 was in the archive
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "i0000500 base,1-4002.bin", 0x00000, 0x80000, CRC(d173af36) SHA1(d6b468e1aecf849deee7e37a906c16f8b1cdd721) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0000912 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(5ad6361b) SHA1(e7d45f37ecd4b725665f39d8ce0db6bd8de9ea26) )
	ROM_LOAD16_BYTE( "g0000912 gme1 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(62f65eef) SHA1(df3d777847d23b9d9e7f6b1edb048cadcbf1cb33) )

	ROM_REGION32_LE( 0x100000, "game_clrram", 0 ) // ??
	ROM_LOAD16_BYTE( "gme1clrram.040", 0x000000, 0x080000, CRC(84848907) SHA1(3474af27304b96e8946315e0f10be2608444f533) )
	ROM_LOAD16_BYTE( "gme2clrram.040", 0x000001, 0x080000, CRC(ad3af31c) SHA1(ef663dc636c7a2d9895f92c38811e75d7bea23b2) )

	ROM_REGION( 0x800000, "cg", 0 ) // not included for this set (probably SIMM like igtsc)
	ROM_LOAD( "cg", 0x000000, 0x800000, NO_DUMP )

	ROM_REGION32_LE( 0x1000000, "pxl", 0 ) // not included for this set (probably SIMM like igtsc)
	ROM_LOAD( "pxl", 0x0000000, 0x1000000, NO_DUMP )

	ROM_REGION32_LE( 0x1000000, "snd", 0 ) // not included for this set (probably SIMM like igtsc)
	ROM_LOAD( "snd", 0x0000000, 0x1000000, NO_DUMP )
ROM_END

ROM_START( igtsc )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "i0000838 base,1-4002.bin", 0x00000, 0x80000, CRC(7b66f0d5) SHA1(a13e7fa4062668ff7acb15e58025eeb401754898) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "g0001175 gme1 1 of 2,2-80.bin", 0x000000, 0x100000, CRC(674e0172) SHA1(e7bfe13781988b9193f22ad93502e303ba9427eb) )
	ROM_LOAD16_BYTE( "g0001175 gme2 2 of 2,2-80.bin", 0x000001, 0x100000, CRC(db76db22) SHA1(e389b11a05f0ef0dcee303ba91578f4cd56beba0) )

	// all these SIMM files are bad dumps, they never contain the byte value 0x0d (uploaded in ASCII mode with carriage return stripped out?)
	ROM_REGION( 0x1000000, "cg", ROMREGION_ERASE00 )
	// uses a SIMM
	ROM_LOAD( "c0000464 cgf.bin", 0x000000, 0x07ff9a3, BAD_DUMP CRC(52fcc9fd) SHA1(98089dcf550bc3670d29b7ee78e014154e672120) ) // should be 0x800000

	ROM_REGION32_LE( 0x1000000, "pxl", 0 )
	// uses a SIMM
	ROM_LOAD( "c000464 pxl3.bin", 0x000000, 0xff73bb, BAD_DUMP CRC(c6acb3cf) SHA1(0ea2d2a506be43a2a8b9d05d80f765c8351494a2) ) // should be 0x1000000

	ROM_REGION32_LE( 0x1000000, "snd", 0 )
	// uses a SIMM
	ROM_LOAD( "dss00076.simm", 0x000000, 0xfd7f81, BAD_DUMP CRC(5dd889b4) SHA1(9a6cb7599d268d110645ac8fe5d41a733cbaadc5) ) // should be 0x1000000
ROM_END

ROM_START( gkkey )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "key00005.bin",        0x00000, 0x80000, CRC(07652909) SHA1(93ad85aa1f32a16084e9261f59394c8af49c24ec) )
	ROM_LOAD( "key00016,1-4002.bin", 0x00000, 0x80000, CRC(97f43f33) SHA1(1485a343f7865f3600ba9cd80eedc489ea75ae40) )
	ROM_LOAD( "key00017,1-4002.bin", 0x00000, 0x80000, CRC(1579739f) SHA1(7b6257d17f74599a4ada3014d02a2e7c6686ab3f) ) /* non WAP keychip */
	ROM_LOAD( "key00018,1-4002.bin", 0x00000, 0x80000, CRC(b35b8888) SHA1(60adc02d17ab0a163e9a6bfbac1f46eeb4a77243) ) /* WAP keychip */
	ROM_LOAD( "key00021,1-4002.bin", 0x00000, 0x80000, CRC(4d1ef12f) SHA1(ab9eebe0ba84d8e27496864adbfe7d1639a6375e) ) /* MD3 WAP keychip & memory clear */
	ROM_LOAD( "key00022,1-4002.bin", 0x00000, 0x80000, CRC(a81c3b80) SHA1(5bda045c461f71d2780db6f238c000508c49f254) ) /* MD3 non WAP keychip & memory clear */
//  ROM_LOAD( "key00023,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* MD3 exclusive to MGM/Mirage */
//  ROM_LOAD( "key00025,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* MD3 WAP keychip & memory clear - New Jersey */
//  ROM_LOAD( "key00026,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* MD3 non WAP keychip & memory clear - New Jersey */
	ROM_LOAD( "key00028,1-4002.bin", 0x00000, 0x80000, CRC(bf06b98b) SHA1(5c46afb560bb5c0f7540b714c0dea851c6b18fe6) ) /* MD3 non WAP keychip & memory clear - 044 boards ONLY */
	ROM_LOAD( "key00029,1-4002.bin", 0x00000, 0x80000, CRC(f458afbb) SHA1(e552b3abc3407e443fdf83163ad10a0e4bb00d19) ) /* MD3 WAP keychip & memory clear - 044 boards ONLY  */
//  ROM_LOAD( "key00030,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* MD3 non WAP keychip & memory clear - New Jersey - 044 boards ONLY */
	ROM_LOAD( "key00032,1-4002.bin", 0x00000, 0x80000, CRC(eafe9167) SHA1(f44f80f7402f43f03cb16225dc944d1f1142a523) )
//  ROM_LOAD( "key00033,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* MD3 WAP keychip & memory clear - New Jersey - 044 boards ONLY */
	ROM_LOAD( "key00037,1-4002.bin", 0x00000, 0x80000, CRC(99bc6554) SHA1(f3afcbd54bd9c4d726df54f1b7aee89cdb4c24f7) ) /* Replaces KEY00017 */
	ROM_LOAD( "key00038,1-4002.bin", 0x00000, 0x80000, CRC(3f9e9e43) SHA1(06540b8e96de9bcb7c5de90d1eb408a9353f82dc) ) /* Replaces KEY00018 */
	ROM_LOAD( "key00039,1-4002.bin", 0x00000, 0x80000, CRC(da98ba31) SHA1(c87ef5638c55e9ffafc8cc53b1509aaddf23c1c2) ) /* Replaces KEY00021 */
	ROM_LOAD( "key00040,1-4002.bin", 0x00000, 0x80000, CRC(a37bda3b) SHA1(af9e0aa3817849f32649392947671cce7ae11af9) ) /* Replaces KEY00022 */
//  ROM_LOAD( "key00041,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* Replaces KEY00023 */
//  ROM_LOAD( "key00042,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* Replaces KEY00025 */
//  ROM_LOAD( "key00043,1-4002.bin", 0x00000, 0x80000, NO_DUMP ) /* Replaces KEY00026 */
	ROM_LOAD( "key00045,1-4002.bin", 0x00000, 0x80000, CRC(bc7a98f9) SHA1(d78bc2528c6ac2fddb9a2b2855a9e024e9d8df85) )
	ROM_LOAD( "cvs00077,1-4002.bin", 0x00000, 0x80000, CRC(052e7da8) SHA1(e781b198b273ecfd904168e3e30e6b453d54bd16) ) /* EZ Pay 80960 CVT Clear/Key & 80960 CVT Route/Safe */
	ROM_LOAD( "cvs00080,1-4002.bin", 0x00000, 0x80000, CRC(f58a3040) SHA1(906ed54aeafdf2cf58ee8425405498a8c64b52e1) )
	ROM_LOAD( "ivc00097,1-4002.bin", 0x00000, 0x80000, CRC(f0a59fd1) SHA1(8e980e9eb80e6899fe3bbcd21ccbd39f9fdccaca) ) /* Vision Ram/E-Square Clear (Replaces IVC00070) */
	ROM_LOAD( "set00028",            0x00000, 0x80000, CRC(2fa9485b) SHA1(46778fbeed2a7a8102ab94bdaf5e2328d5cdb6af) ) // came with the igtmg156 set

	ROM_REGION( 0x10000, "set_denom", 0 )
	ROM_LOAD( "setdenom",            0x00000, 0x10000, CRC(34aa584e) SHA1(2b28f4bfe19f539575755a26b8e3b56134fc7695) )

//  ROM_REGION( 0x80000, "miscbad", 0 )
//  these are also bad dumps, again they never contain the byte value 0x0d (uploaded in ASCII mode with carriage return stripped out?)

	ROM_REGION32_LE( 0x200000, "game", ROMREGION_ERASEFF )
	ROM_REGION( 0x100000, "cg", ROMREGION_ERASEFF )
	ROM_REGION32_LE( 0x200000, "pxl", ROMREGION_ERASEFF )
	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASEFF )
ROM_END

ROM_START( igtvp ) // IGT Vision printer? For now added it here for archiving purpose. Is the set complete?
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD( "sb100077base.402", 0x00000, 0x80000, CRC(163dfa5f) SHA1(bd88ee41c8b059a0748b4e2692d7c0adf3b291b5) )

	ROM_REGION32_LE( 0x200000, "game", 0 )
	ROM_LOAD16_BYTE( "sg000115gme1.080", 0x000000, 0x100000, CRC(4de1128f) SHA1(96867eee7d93dac8404e15a8575a453ebc4f1932) )
	ROM_LOAD16_BYTE( "sg000115gme2.080", 0x000001, 0x100000, CRC(9b1b43c1) SHA1(fcf54d65373ce3f55f00ee49f1b02146dc3b7142) )

	ROM_REGION( 0x20000, "unkrom", 0 )
	ROM_LOAD( "vs009gx0.010", 0x00000, 0x20000, CRC(a8cf1942) SHA1(3e4beb813b180fdf1d7b401b5742c1bd8b0a5a9a) )

	// for now, to avoid crashing
	ROM_REGION( 0x100000, "cg", ROMREGION_ERASEFF )
	ROM_REGION32_LE( 0x200000, "pxl", ROMREGION_ERASEFF )
	ROM_REGION32_LE( 0x200000, "snd", ROMREGION_ERASEFF )
ROM_END

} // anonymous namespace


GAME( 1997, ms14,      0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multistar 14",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, igtmg133,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000133)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, igtmg156,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000156)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, igtmg159,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000159)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, igtmg164,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000164)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, igtmg166,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000166)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1997, sup8race,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Super 8 Race",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, brhino,    0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Black Rhino (IGT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, bmoonii,   0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Blue Moon II",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, dblheart,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Double Hearts",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, igtmg214,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000214)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, ms3,       0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multistar 3",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, ms5,       0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multistar 5",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1998, wofigt,    0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Wheel of Fortune (IGT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, igtmg247,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000247)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, mystjag,   0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Mystic Jaguar",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 1999, tripplay,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Triple Play (M0000230, Spanish)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // sound is undumped
GAME( 2000, igtmg394,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000394)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2000, ms72c,     0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multistar 7 2c",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, igtmg535,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Multi Game (IGT, M0000535)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2001, igtvp,     0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Vision printer",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt4,    0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (v4.x)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt4ms,  gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (v4.x, MS)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt43,   gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (v4.3)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt43n,  gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (v4.3, NJ)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigtez,   gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (EZ Pay, v4.0)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigtezms, gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (EZ Pay, v4.0, MS)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, gkigt5p,   gkigt4, igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (Triple-Five Play)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
GAME( 2003, igtsc,     0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Super Cherry",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // SIMM dumps are bad.
GAME( 2003, munsters,  0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "The Munsters",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // GFX and sound ROMs not dumped
GAME( 2003, gkkey,     0,      igt_gameking, igt_gameking, igt_gameking_state, empty_init, ROT0, "IGT", "Game King (Set Chips)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
