// license:BSD-3-Clause
// copyright-holders: David Haywood

/*
IGS games based on M68000 + IGS023 for video.
PGM-like but with different sound hardware.

TODO:
* identify sound hardware
* identify where the M6502 core is contained
*/


#include "emu.h"

#include "igs023_video.h"

#include "cpu/m6502/m6502.h"
#include "cpu/m68000/m68000.h"
#include "machine/timer.h"

#include "emupal.h"
#include "screen.h"
#include "speaker.h"
#include "tilemap.h"

namespace {

class igs_68k_023vid_state : public driver_device
{
public:
	igs_68k_023vid_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen"),
		m_video(*this, "igs023"),
		m_mainram(*this, "sram")
	{ }

	void xypmd(machine_config &config) ATTR_COLD;
	void xypmda(machine_config &config) ATTR_COLD;

private:
	u16 unknown_r() { return (machine().rand() & 0x0010) | 0xffcf; } // 0x0010 seems to be sound CPU status
	u16 unknown2_r() { return 0xffff; }

	void screen_vblank(int state);
	void screen_vblank_xypmda(int state);

	void main_program_base_map(address_map &map) ATTR_COLD;
	void main_program_xypmd_map(address_map &map) ATTR_COLD;
	void main_program_xypmda_map(address_map &map) ATTR_COLD;

	void sub_program_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;
	required_device<igs023_video_device> m_video;
	required_shared_ptr<uint16_t> m_mainram;
};

void igs_68k_023vid_state::screen_vblank(int state)
{
	// Apart from IRQ1 and IRQ2 all other IRQs point to exceptions

	// rising edge
	if (state)
	{
		m_video->get_sprites();
		m_maincpu->set_input_line(M68K_IRQ_1, HOLD_LINE); // only IRQ with useful code
	}
	else
	{
		// m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE); // does nothing
	}
}

void igs_68k_023vid_state::screen_vblank_xypmda(int state)
{
	// Apart from IRQ2 all other IRQs point to exceptions

	// rising edge
	if (state)
	{
		m_video->get_sprites();
		m_maincpu->set_input_line(M68K_IRQ_2, HOLD_LINE);
	}
	else
	{
		// no other possible IRQ vectors
	}
}

void igs_68k_023vid_state::main_program_base_map(address_map &map)
{
	map(0x000000, 0x07ffff).rom();

	map(0x800000, 0x80ffff).ram().share(m_mainram);
	map(0x900000, 0x907fff).mirror(0x0f8000).rw(m_video, FUNC(igs023_video_device::videoram_r), FUNC(igs023_video_device::videoram_w));
	map(0xa00000, 0xa011ff).ram().w("palette", FUNC(palette_device::write16)).share("palette");
	map(0xb00000, 0xb0ffff).rw(m_video, FUNC(igs023_video_device::videoregs_r), FUNC(igs023_video_device::videoregs_w));
}

void igs_68k_023vid_state::main_program_xypmd_map(address_map &map)
{
	main_program_base_map(map);

	map(0x080000, 0x08ffff).ram(); // why is this tested like it was mainram / spriteram? bug in code?

	map(0x090000, 0x090001).r(FUNC(igs_68k_023vid_state::unknown_r));
	map(0x090018, 0x090019).portr("IN1");

	// 025 or just I/O?
	map(0x0c0000, 0x0c0001).nopr().nopw();
	map(0x0c0002, 0x0c0003).nopw().portr("IN0");
}

void igs_68k_023vid_state::main_program_xypmda_map(address_map &map)
{
	main_program_base_map(map);

	// I/O / IGS025 etc. is different here
	map(0x080100, 0x080101).r(FUNC(igs_68k_023vid_state::unknown_r));
	map(0x080120, 0x080121).portr("IN1");
	map(0x080200, 0x080201).nopw();

	map(0x0a0000, 0x0a0001).r(FUNC(igs_68k_023vid_state::unknown2_r));
	map(0x0a0002, 0x0a0003).r(FUNC(igs_68k_023vid_state::unknown2_r));

	// definitely the same as 0x0c0000 on the parent
	map(0x0e0000, 0x0e0001).nopr().nopw();
	map(0x0e0002, 0x0e0003).nopw().portr("IN0");
}

void igs_68k_023vid_state::sub_program_map(address_map &map)
{
	map(0x0000, 0x03ff).ram();

	map(0x4200, 0x46ff).ram();

	map(0xc000, 0xc000).noprw();
	map(0xc001, 0xc001).nopw();

	map(0xe000, 0xffff).rom().region("subcpu", 0x0000);
}


static INPUT_PORTS_START( xypmd )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

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
INPUT_PORTS_END


void igs_68k_023vid_state::xypmd(machine_config &config)
{
	// basic machine hardware
	M68000(config, m_maincpu, 20_MHz_XTAL);
	m_maincpu->set_addrmap(AS_PROGRAM, &igs_68k_023vid_state::main_program_xypmd_map);

	m6502_device &subcpu(M6502(config, "subcpu", 8_MHz_XTAL)); // TODO: something M6502 derived (data.u13 is M6502 derived code)
	subcpu.set_addrmap(AS_PROGRAM, &igs_68k_023vid_state::sub_program_map);

	// video hardware
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER); // TODO: verify everything once emulation works
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(1000));
	m_screen->set_size(512, 256);
	m_screen->set_visarea(0, 448-1, 0, 224-1);
	m_screen->set_screen_update("igs023", FUNC(igs023_video_device::screen_update));
	m_screen->screen_vblank().set(FUNC(igs_68k_023vid_state::screen_vblank));
	m_screen->set_palette("palette");

	PALETTE(config, "palette").set_format(palette_device::xRGB_555, 0x1200 / 2);

	IGS023_VIDEO(config, m_video, 0);
	m_video->set_palette("palette");
	m_video->read_spriteram_callback().set([this](offs_t offset) { return m_mainram[offset]; });

	// sound hardware
	SPEAKER(config, "mono").front_center();

	// TODO: is sound provided by the two Novatek chips?
}

void igs_68k_023vid_state::xypmda(machine_config &config)
{
	xypmd(config);

	m_maincpu->set_addrmap(AS_PROGRAM, &igs_68k_023vid_state::main_program_xypmda_map);

	m_screen->screen_vblank().set(FUNC(igs_68k_023vid_state::screen_vblank_xypmda));
}
/*
Xing Yun Pao Ma Di Super, IGS, 2003
Hardware Info By Guru
---------------------

IGS PCB NO-0255-1
  |----------------------------------------------|
  | IDC34    SOCKET.U11     |------|  TL082  VOL |
|-|                         |NOVATEK      OM8383S|
|1  TLP521(x23)        8MHz |NT3570F     TL082   |
|8           DATA.U13       |------|             |
|W                                               |
|A                          |------|      7805   |
|Y           24257          |NOVATEK             |
|-|                         |NT3580F             |
  |          24257 24257    |------|        T518B|
  | TEXT.U5  |---------|                         |
|-|          |  IGS023 |  PAL V-401CN.U26        |
|1  CG.U6    |         |  PAL     68000   24258  |
|0           |         |  PAL                    |
|W  CG.U7    |         | |-----|          24258  |
|A           |---------| |IGS025  20MHz      BATT|
|Y         DSW1     50MHz| S8  |             SW3 |
|-|ULN2004 DSW2          |-----|   TLP521(x17)   |
  | TLP521(x6)  |--|       JAMMA          |--|   |
  |-------------|  |----------------------|  |---|
Notes:
      68000 - Motorola MC68000FN20 CPU. Clock Input 20MHz
              Note: This is a 20MHz part without clock divider and runs at 20MHz.
      24257 - Winbond W24257AJ-8N 32kB x8-bit SRAM (VRAM)
      24258 - Winbond W24258S-55LL 32kB x8-bit SRAM (Work RAM)
     IGS023 - Custom IGS023 Graphics Chip (also used on IGS PGM)
     IGS025 - Custom IGS025 Chip with Sticker 'S8'
    V-401CN - 27C4096 EPROM (main program)
    TEXT.U5 - 27C160 EPROM
      CG.U6 - 27C4096 EPROM
      CG.U7 - 27C160 EPROM
       DATA - 27512 Mask ROM
 SOCKET.U11 - Empty Socket. ROM is likely to be missing as the socket is clean. PCB printed 'MUSIC1' and '27C080'.
              There is space for another unpopulated ROM below this one marked 'MUSIC2' and '27C080'.
      IDC34 - IDC34 34-Pin Flat Cable Connector for I/O and Control Expansion (same connector seen on PGM)
    NT3580F - Novatek NT3580F-AM019 QFP100 Sound CPU with 6502 Core. Clock Input 8.000MHz. DATA ROM connects to this chip.
    NT3570F - Novatek NT3570F QFP80 Sound Chip. Clock Input 8.000MHz. This chip connects to the opamps so this is the sound chip.
              ROM U11 connects to this chip. Both Novatek chips are dated 1999.
      T518B - Mitsumi T518B Reset Chip
       7805 - LM7805 5V Linear Voltage Regulator
     TLP521 - Toshiba TLP521 Opto-Coupler
       BATT - 3.6V Ni-Cad Battery (both W24258 RAMs are battery-backed)
        SW3 - Toggle Switch for NVRAM Clear and Reset
     DSW1/2 - 8-Position DIP Switch
    ULN2003 - ULN2003 7-Channel Darlington Transistor Array
      TL082 - Texas Instruments TL082 JFET Input Dual Operational Amplifier
    OM8383S - Philips OM8383S Audio Power Amplifier
*/

ROM_START( xypmd )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "v-401cn.u26", 0x00000, 0x80000, CRC(84f4f46c) SHA1(c04d3aaf531caba6cdf8296570ce24964bd7a077) ) // version string at 0x360dc

	ROM_REGION( 0x10000, "subcpu", 0 ) // TODO: sound related? almost empty
	ROM_LOAD( "data.u13", 0x00000, 0x10000, CRC(7c0d8c8f) SHA1(d36ae4749fd248c399741f2024f2f44cf22536b8) ) // 111xxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x200000, "igs023",  0 )
	ROM_LOAD( "text.u5", 0x000000, 0x200000, CRC(253b8517) SHA1(3f583410ab7083d5f45a5e23f73bddd18b000260) )

	ROM_REGION16_LE( 0x200000, "igs023:sprcol", 0 )
	ROM_LOAD( "cg.u7", 0x000000, 0x200000, CRC(1c6764f2) SHA1(ed1efcab927bdc439247d422df5dedc72fce5682) ) // 1xxxxxxxxxxxxxxxxxxxx = 0xFF

	ROM_REGION16_LE( 0x80000, "igs023:sprmask", 0 )
	ROM_LOAD( "cg.u6", 0x00000, 0x80000, CRC(20ff0cb3) SHA1(4562996675fe62563f393817f40395c8bce37c5f) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "u11", 0x000000, 0x100000, NO_DUMP ) // probably removed from this PCB, possibly or even probably the same as xypmda
ROM_END

/*
Unknown IGS
Hardware Info By Guru
---------------------

IGS PCB NO-0198-2
Basically the same PCB as IGS PCB No-0255-1 with parts shuffled.
IGS did **MANY** board re-designs with no additional functionality.

Notes:
      68000 - Clock 20MHz
   2x 61256 - 32kB x8-bit SRAM (Main Work RAM)
   3x 61256 - 32kB x8-bit SRAM (VRAM)
     IGS023 - Custom IGS023 Graphics Chip (also used on IGS PGM)
     IGS025 - Custom IGS025 Chip with Sticker 'T2'
    CG/TEXT - vs 0255-1 PCB, this board has EPROMs replaced with SOP40 and SOP44 mask ROMs
*/

ROM_START( xypmda )
	ROM_REGION( 0x80000, "maincpu", 0 )
	ROM_LOAD16_WORD_SWAP( "prg.u36", 0x00000, 0x80000, CRC(99d6c58c) SHA1(c8db8689c18ef05ad52ef0476033b62f778e6abf) ) // no version string

	ROM_REGION( 0x10000, "subcpu", 0 ) // TODO: sound related? almost empty
	ROM_LOAD( "data.u33", 0x00000, 0x10000, CRC(5e3e3558) SHA1(ca9cdb4e8d124b4a7341ef6597c7ccdbbe124138) ) // 111xxxxxxxxxxxxx = 0x00

	ROM_REGION( 0x200000, "igs023",  0 )
	ROM_LOAD( "igs_t1801.u30", 0x000000, 0x200000, CRC(253b8517) SHA1(3f583410ab7083d5f45a5e23f73bddd18b000260) )

	ROM_REGION16_LE( 0x100000, "igs023:sprcol", 0 )
	// Data is NOT identical to cg.u7 (size difference is irrelevant, cg.u7 just has blank data in 2nd half)
	// Unusual however, as the sprcol and sprmask ROMs are essentially a pair. Check if one is bad or if
	// some colours have been changed intentionally
	ROM_LOAD( "igs_a1802.u40", 0x000000, 0x100000, CRC(5bf791cc) SHA1(df23c8a25a26410ec4021948403bb4111810d7af) )

	ROM_REGION16_LE( 0x80000, "igs023:sprmask", 0 )
	ROM_LOAD( "igs_a1803.u39", 0x00000, 0x80000, CRC(20ff0cb3) SHA1(4562996675fe62563f393817f40395c8bce37c5f) )

	ROM_REGION( 0x100000, "samples", 0 )
	ROM_LOAD( "igs_s1804_speech_v100.u32", 0x000000, 0x100000, CRC(d95220ee) SHA1(72259856bc2a12059ff481f7aab5ecc3118edd18) )
ROM_END

} // anonymous namespace



GAME( 2003, xypmd,  0,     xypmd,  xypmd, igs_68k_023vid_state, empty_init, ROT0, "IGS", "Xing Yun Pao Ma Di Super (V401CN)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 1999, xypmda, xypmd, xypmda, xypmd, igs_68k_023vid_state, empty_init, ROT0, "IGS", "Xing Yun Pao Ma Di (unknown ver)",  MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
