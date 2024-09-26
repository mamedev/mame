// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Capcom Medal / Medalusion / Medalusion 2 hardware (c) 2005 Capcom

TODO:
- S29JL064H needs support the "unlock bypass" for ATA -> flash ROM reprogramming to work.
  This is currently bypassed by giving default roms instead (also because it's extremely slow for
  end user to care).
  The BAD_DUMP also denotes that we are missing the 256 bytes "secured silicon sector" data for
  those.
- Several games sets up GPU with a comparison between DrawTrap output with DrawBitmap,
  failing in the process:
  dokodemo (-nodrc only): bp c7aa342,1,{r1=r0;g}
  pingu: bp ccaea4ee,1,{r9=r1;g}
  wontame: bp cc05422c,1,{r4=r2;g}
  masmarios: bp cc98790e,1,{r9=r1;g}
  masmario2: bp cc8f41d8,1,{r9=r1;g}
  dkbanans (-nodrc only): bp c48c05e,1,{r1=r0;g}
- YMZ770B for anything but wontame;
- Proper SIO handling, particularly IRQ source;
- Identify and handle LCDC at $89xxxxx/$8axxxxx;
- Understand enough of I/O system so they don't throw error codes;

===================================================================================================

   Main boards:
    Capcom AMT-04054 "YUI2"
    - Hitachi SH-4 HD6417750S at 200MHz
    - Elpida DS1232AA-75 1M x 32-bit x 4-banks (128Mbit) SDRAM
    - Altera ACEX EP1K50TC144-3 FPGA
    - 2 x Xilinx XC9572XL CPLD (TQFP-100) stamped AMTPS005 and AMTPS011
    - M48T35Y timekeeper device
  * - 2 x Fujitsu MB86292 'Orchid' Graphics Controller
  * - 8 x Fujitsu MB81E161622-10FH 512K x 16-bit x 2-banks (16Mbit) FCRAM
  * - 2 x ADV7120 Video DAC
  * - Yamaha YMZ770B-F 'AMMSL' SPU at 16.934MHz
    components marked * might be not populated

   Capcom AMT-02008 "YUI"
    - FPGA is Altera FLEX 6000, other differences / details is unknown

   Upper boards (game specific):

   Capcom AMT-02012-01 (Medalusion 1)
    - TODO

   Capcom AMT-04041 (Alien: The Arcade, Rockman EXE Battle Chip Stadium)
    - 2 x Panasonic MN677511DE MPEG2 decoder
    - 2 x Hynix HY57V161610DTC-7 512K x 16-bit x 2-banks (16Mbit) SDRAM (TSOPII-50)
    - Hynix HY57V643220DTP-7 512K x 32-bit x 4-banks (64Mbit) SDRAM (TSOPII-86)
    - Altera ACEX EP1K100FC484-2 FPGA
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

   Capcom AMT-05057 (Donkey Kong Banana Kingdom and Medalusion 2 games)
    - Altera MAX EPM3064ATC100-10 CPLD (QFP-100) stamped PS015
    - S29JL064H 64Mbit FlashROM (TSOP-48)
    - Compact Flash connector

    Games list:
    http://web.archive.org/web/20090227151212/http://www.capcom.co.jp:80/arcade/medalgame.html

    6-8 player coin pusher machines:
 *   - Alien: The Arcade Medal Edition (c) 2005
     - Alien Danger (c) 2007
     - Chibi Maruko-chan Aim Fuji Nippon Ichi! (c) 2008
 *   - Donkey Kong Banana Kingdom (c) 2006
 *   - Super Mario Fushigi no Korokoro Party (c) 2004
 *   - Super Mario Fushigi no Korokoro Party 2 (c) 2005

    Single player medal machines:
    Medalusion:
 *   - Chibi Maruko-chan ~Minna de Sugoroku Asobi~ no Maki (c) 2003
 *   - Donkey Kong: Jungle Fever (c) 2005
 *   - Nihonzenkoku-katochan-no-kurukuru-ressya-de-Pe! (c) 2004
 *   - Rockman EXE The Medal Operation (c) 2005
 *   - Super Mario Fushigi No JanJanLand (c) 2005

    Medalusion 2:
 *!  - Doko Demo Issho Toro's Fishing (c) 2006
 *   - Pingu's Ice Block (c) 2005
 !   - Geki Makaimura (c) 2005
 *!  - Won! Tertainment Happy Channel (c) 2008 note: main board is different, uses Yamaha YMZ770C
                                                     instead of YMZ770B
    Not medal machines:
     - Rockman EXE Battle Chip Stadium (c) 2006
     - Rockman EXE Battle Chip Stadium Ver.2 (c) 2006
*    - Rockman EXE Battle Chip Stadium Ver.3.00 (c) 2006

 *  - dumped
 *! - CF card dumped, boot roms missing
 !  - Boot ROMs dumped, CF card missing

**************************************************************************************************/


#include "emu.h"

#include "cpu/sh/sh4.h"
#include "machine/intelfsh.h"
#include "machine/ram.h"
#include "machine/timekpr.h"
#include "sound/ymz770.h"
#include "video/mb86292.h"

#include "bus/ata/ataintf.h"
#include "bus/ata/hdd.h"

#include "screen.h"
#include "speaker.h"


namespace {

#define MASTER_CLOCK    XTAL(200'000'000)

class alien_state : public driver_device
{
public:
	alien_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
		, m_workram(*this, "workram")
		, m_screen(*this, "screen")
		, m_gpu(*this, "gpu")
//      , m_vram(*this, "vram%d", 0U)
		, m_vram(*this, "vram")
		, m_ata(*this, "ata")
		, m_ymz(*this, "ymz770")
		, m_ymz_flash(*this, "ymz770_flash%d", 1U)
		, m_io_in0(*this, "IN0")
		, m_io_in1(*this, "IN1")
	{ }

	void alien(machine_config &config);
	void masmario2(machine_config &config);

	void init_dkbanans();

private:
	u8 fpga_r();

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	void alien_map(address_map &map) ATTR_COLD;
	void masmario2_map(address_map &map) ATTR_COLD;

	// devices
	required_device<sh4_device> m_maincpu;
	required_shared_ptr<u64> m_workram;
	required_device<screen_device> m_screen;
	required_device<mb86292_device> m_gpu;
	required_device<ram_device> m_vram;
	required_device<ata_interface_device> m_ata;
	required_device<ymz770_device> m_ymz;
	optional_device_array<spansion_s29gl064s_device, 2> m_ymz_flash;
	required_ioport m_io_in0;
	required_ioport m_io_in1;

	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	template <unsigned N> void gpu_irq_w(int state);
	INTERRUPT_GEN_MEMBER(sio_irq_w);

	bool m_sio_irq_state = false;
};

u32 alien_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	m_gpu->screen_update(screen, bitmap, cliprect);
	return 0;
}

u8 alien_state::fpga_r()
{
	u8 fpga_type = 1; // 2 bit value
	return (fpga_type << 5) | 0x10 | (machine().rand() & 7); // status bits TODO
}

void alien_state::alien_map(address_map &map)
{
	map(0x00000000, 0x00ffffff).rom();
//  map(0x02000000, ...) accessed by dkbanana
	map(0x04000000, 0x04007fff).rw("m48t35", FUNC(timekeeper_device::read), FUNC(timekeeper_device::write));
	map(0x04800000, 0x04800000).r(FUNC(alien_state::fpga_r));
	map(0x04a00000, 0x04a00007).nopw(); // FPGA config
	map(0x08000000, 0x08000007).portr("DSW");
	// wontame at least
	map(0x08800000, 0x08800001).lr16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			return m_io_in1->read();
		})
	);
//  map(0x08900000, 0x08900001) VFD - pingu & wontame at least, protocol for a LCDC (also referenced in lamp test)
//  map(0x08a00000, 0x08a00001) DOT /

	map(0x0c000000, 0x0cffffff).ram().share("workram"); // main RAM

	map(0x10000000, 0x107fffff).rw(m_vram, FUNC(ram_device::read), FUNC(ram_device::write)); // GPU 1 VRAM
	map(0x11fc0000, 0x11ffffff).m(m_gpu, FUNC(mb86292_device::vregs_map));  // GPU 1 regs
//  map(0x12000000, 0x127fffff).ram().share(m_vram[1]); // GPU 2 VRAM
//  map(0x13fc0000, 0x13ffffff).ram().share("vregs2");  // GPU 2 regs
	// pingu accesses the mirror
	map(0x14000000, 0x1400000f).mirror(0x100000).lrw32(
		NAME([this] (offs_t offset, u32 mem_mask) {
			switch(offset)
			{
				// data in
				case 1:
					return m_io_in0->read();
				case 2:
					// must be zero otherwise inputs stops working in mariojjl (tbd)
					return (u32)0;
			}
			return 0xffffffff;
		}),
		NAME([] (offs_t offset, u32 data, u32 mem_mask) {
			// writing to 0x14000000: data out
		})
	);
//  map(0x14100010, 0x14100070) masmario2/dkbanans satellite terminal comms?
//  map(0x18000000, 0x1800000f).r(FUNC(alien_state::test_r)).nopw(); // Alien CF ATA, other games have it other way

	// pingu ATA i/f, similar to konami/konamigs.cpp
	map(0x18800000, 0x18800007).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) {
			offset *= 2;
			u16 data = 0;
			if (ACCESSING_BITS_0_7)
				data |= m_ata->cs0_r(offset, 0xff) & 0xff;
			if (ACCESSING_BITS_8_15)
				data |= (m_ata->cs0_r(offset + 1, 0xff) << 8);
			return data;
		}),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			offset *= 2;
			if (ACCESSING_BITS_0_7)
				m_ata->cs0_w(offset, data & 0xff, 0xff);
			if (ACCESSING_BITS_8_15)
				m_ata->cs0_w(offset + 1, data >> 8, 0xff);
		})
	);
	map(0x18800400, 0x18800403).lrw16(
		NAME([this] (offs_t offset, u16 mem_mask) { return m_ata->cs0_r(0, mem_mask); }),
		NAME([this] (offs_t offset, u16 data, u16 mem_mask) {
			m_ata->cs0_w(0, data, mem_mask);
		})
	);
	map(0x1b000000, 0x1b7fffff).rw(m_ymz_flash[0], FUNC(spansion_s29gl064s_device::read), FUNC(spansion_s29gl064s_device::write));
}

void alien_state::masmario2_map(address_map &map)
{
	alien_map(map);
	map(0x1b800000, 0x1bffffff).rw(m_ymz_flash[1], FUNC(spansion_s29gl064s_device::read), FUNC(spansion_s29gl064s_device::write));
}


static INPUT_PORTS_START( alien )
	PORT_START("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin SW "L"
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin SW "L-Err"
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_COIN2 ) // Coin SW "R"
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin SW "R-Err"
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Push SW L")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Push SW R")
	PORT_BIT( 0x000000c0, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Test SW
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Enter SW")
	PORT_CONFNAME( 0x00001800, 0x00001800, "Mini JP SW" )
	PORT_CONFSETTING( 0x00000000, "0/0" )
	PORT_CONFSETTING( 0x00001000, "0/1" )
	PORT_CONFSETTING( 0x00000800, "1/0" )
	PORT_CONFSETTING( 0x00001800, "1/1" )
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x7fff0000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_START1 ) // "Star SW" (sic)

	PORT_START("IN1")
	PORT_BIT( 0xffffffff, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW")
	// "DIP 8"
	PORT_DIPUNKNOWN_DIPLOC( 0x00010000, 0x00010000, "SW1:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00020000, 0x00020000, "SW1:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00040000, 0x00040000, "SW1:3" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00080000, 0x00080000, "SW1:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00100000, 0x00100000, "SW1:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x00200000, 0x00200000, "SW1:6" )
	PORT_SERVICE_DIPLOC( 0x00400000, IP_ACTIVE_LOW, "SW1:7" )
	PORT_DIPNAME( 0x00800000, 0x00000000, "RAM Tests" ) PORT_DIPLOCATION("SW1:8") // disable tests to make things faster
	PORT_DIPSETTING( 0x00000000, DEF_STR( Off ) )
	PORT_DIPSETTING( 0x00800000, DEF_STR( On ) )
	// "DIP 9"
	PORT_DIPUNKNOWN_DIPLOC( 0x01000000, 0x01000000, "SW2:1" )
	PORT_DIPUNKNOWN_DIPLOC( 0x02000000, 0x02000000, "SW2:2" )
	PORT_DIPUNKNOWN_DIPLOC( 0x04000000, 0x04000000, "SW2:3" ) // additional test mode in wontame (needs both this and SW1:7 to be enabled)
	PORT_DIPUNKNOWN_DIPLOC( 0x08000000, 0x08000000, "SW2:4" )
	PORT_DIPUNKNOWN_DIPLOC( 0x10000000, 0x10000000, "SW2:5" )
	PORT_DIPUNKNOWN_DIPLOC( 0x20000000, 0x20000000, "SW2:6" )
	PORT_DIPUNKNOWN_DIPLOC( 0x40000000, 0x40000000, "SW2:7" )
	PORT_DIPUNKNOWN_DIPLOC( 0x80000000, 0x80000000, "SW2:8" )
INPUT_PORTS_END

static INPUT_PORTS_START( wontame )
	PORT_INCLUDE( alien )

	PORT_MODIFY("IN0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin SW "L-Err"
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_COIN1 ) // Coin SW "L"
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_UNKNOWN ) // Coin SW "R-Err"
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_COIN2 ) // Coin SW "R"
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_NAME("Push SW L")
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Action SW L")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_NAME("Push SW R")
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_NAME("Action SW R")
	// related to solenoid test?
	PORT_CONFNAME( 0x00000300, 0x00000300, "Mini JP SW" )
	PORT_CONFSETTING( 0x00000000, "0/0" )
	PORT_CONFSETTING( 0x00000100, "1/0" )
	PORT_CONFSETTING( 0x00000200, "0/1" )
	PORT_CONFSETTING( 0x00000300, "1/1" )
//  PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_NAME("Enter SW")
	PORT_BIT( 0x00000400, IP_ACTIVE_HIGH, IPT_TILT )
	PORT_BIT( 0x00000800, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "pusher"
	PORT_BIT( 0x0000f000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "kurukuru" (4 SW)
	PORT_BIT( 0x00010000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "chacker sw left-top"
	PORT_BIT( 0x00020000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "chacker sw right-top"
	PORT_BIT( 0x00040000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "chacker sw center-bottom"
	PORT_BIT( 0x00080000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "chacker sw left-bottom"
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "chacker sw right-bottom"
	PORT_BIT( 0x00200000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 1"
	PORT_BIT( 0x00400000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 2"
	PORT_BIT( 0x00800000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 3"
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 4"
	PORT_BIT( 0x02000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 5"
	PORT_BIT( 0x04000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 6"
	PORT_BIT( 0x08000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 7"
	PORT_BIT( 0x10000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "v chacker 8"
	PORT_BIT( 0x20000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "left hopper"
	PORT_BIT( 0x40000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "center hopper"
	PORT_BIT( 0x80000000, IP_ACTIVE_HIGH, IPT_UNKNOWN ) // "right hopper"

	PORT_MODIFY("IN1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_SERVICE1 ) // Test SW
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_NAME("Enter SW")
	PORT_BIT( 0xfffffffc, IP_ACTIVE_LOW, IPT_UNKNOWN )
INPUT_PORTS_END

template <unsigned N> void alien_state::gpu_irq_w(int state)
{
	// TODO: configured by FPGA
	const unsigned level = N == 1 ? 4 : 2;
	m_maincpu->sh4_set_irln_input(state ? level : 15);
}

// TODO: source and ack
INTERRUPT_GEN_MEMBER(alien_state::sio_irq_w)
{
	m_maincpu->sh4_set_irln_input(m_sio_irq_state ? 8 : 15);
	m_sio_irq_state ^= 1;
}

void medalusion_devices(device_slot_interface &device)
{
	device.option_add("cfcard", ATA_CF);
}

void alien_state::machine_start()
{
	m_maincpu->sh2drc_set_options(SH2DRC_STRICT_VERIFY);
	m_maincpu->sh2drc_add_fastram(0x00000000, 0x00ffffff, 0, memregion("maincpu")->base());
	m_maincpu->sh2drc_add_fastram(0x0c000000, 0x0cffffff, 0, &m_workram[0]);
}

void alien_state::machine_reset()
{
	//m_maincpu->set_input_line(INPUT_LINE_HALT, ASSERT_LINE);
	m_sio_irq_state = false;
}

void alien_state::alien(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, MASTER_CLOCK);    /* 200MHz */
	m_maincpu->set_addrmap(AS_PROGRAM, &alien_state::alien_map);
	m_maincpu->set_periodic_int(FUNC(alien_state::sio_irq_w), attotime::from_hz(60));
//  m_maincpu->set_force_no_drc(true);

	// Configured as FCRAM 16MBit with 8MB / 64-bit data bus thru MMR register
	RAM(config, m_vram);
	m_vram->set_default_size("8M");
	m_vram->set_default_value(0);

	SPANSION_S29GL064S(config, m_ymz_flash[0]);

	ATA_INTERFACE(config, m_ata).options(medalusion_devices, "cfcard", nullptr, true);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	// configured by main GPU
	m_screen->set_raw(MASTER_CLOCK / 20, 608, 0, 480, 262, 0, 234);
	m_screen->set_screen_update(FUNC(alien_state::screen_update));

	MB86292(config, m_gpu, 0);
	m_gpu->set_screen(m_screen);
	m_gpu->set_vram(m_vram);
	m_gpu->set_xint_cb().set(FUNC(alien_state::gpu_irq_w<0>));

	/* sound hardware */
	SPEAKER(config, "mono").front_center();
	// YMZ770B on all games but wontame (YMZ770C), TBD
	YMZ770(config, m_ymz, 16.384_MHz_XTAL).add_route(ALL_OUTPUTS, "mono", 1.0);

	M48T35(config, "m48t35");
}

void alien_state::masmario2(machine_config &config)
{
	alien(config);
	m_maincpu->set_addrmap(AS_PROGRAM, &alien_state::masmario2_map);

	SPANSION_S29GL064S(config, m_ymz_flash[1]);
}

void alien_state::init_dkbanans()
{
	uint8_t *rom = memregion("maincpu")->base();
	rom[2] = 0x02;
	rom[3] = 0x60;
}

/*************************
*        Rom Load        *
*************************/

////////////////////////////
// Custom multi-unit games
////////////////////////////

ROM_START( alien )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "aln_s04.4.ic30", 0x000000, 0x400000, CRC(11777d3f) SHA1(8cc9fcae7911e6be273b4532d89b44a309687ead) )
	ROM_LOAD32_WORD( "aln_s05.5.ic33", 0x000002, 0x400000, CRC(71d2f22c) SHA1(16b25aa34f8b0d988565e7ab7cecc4df62ee8cf3) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "s29jl064hxxtfi00.u35", 0x000000, 0x800100, CRC(01890c61) SHA1(4fad321f42eab835351c6d5f73539bdbed80affe) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "alien", 0, SHA1(0328f12765db41a9ef5c8bfb88d4983345093072) )
ROM_END

// Host unit board, GPUs and YMZ770B not populated.
ROM_START( dkbanana )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_m04.ic30", 0x000000, 0x400000, CRC(a294f17c) SHA1(7e0f865342f63f93a9a31ad7e6d3b70c59f3fa1b) )
	ROM_LOAD32_WORD( "dnk_m05.ic33", 0x000002, 0x400000, CRC(22f5db87) SHA1(bdca65d39e94d88979218c8c586c6f20bb00e5ce) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "29lj064.ic10", 0x000000, 0x800100, CRC(67cec133) SHA1(1412287fe977eb422a3cca6a0da1523859c2562e) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// Satellite unit board
ROM_START( dkbanans )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "dnk_s04.ic30", 0x000000, 0x400000, CRC(eed7d46f) SHA1(43edb15ff72952f7c9825e5735faa238edfd934d) )
	ROM_LOAD32_WORD( "dnk_s05.ic33", 0x000002, 0x400000, BAD_DUMP CRC(2fc88385) SHA1(03393bdb1fa526c70d766469c37b453f0e1eb8a3) ) // 2 first bytes is bad/wrong or (unlikely) supplied by protection, see driver init

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "flash1", 0x000000, 0x800000, BAD_DUMP CRC(97e43944) SHA1(dc5c4e0f9db6497638bb80c5050acd4ec865d0e4) )

	ROM_REGION( 0x800100, "ymz770_flash2", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "flash2", 0x000000, 0x800000, BAD_DUMP CRC(8c04f3a0) SHA1(b66ef92530c5cc581470c4eea3a7c350288f1f88) )

	ROM_REGION( 0x1000000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0,        0x800000 )
	ROM_COPY( "ymz770_flash2", 0, 0x800000, 0x800000 )

	// contain host.abs and sate.abs ELF executables and game assets
	// same card was used in both Host and Satellite units
	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "dkbanana", 0, SHA1(c6b50486f2a6382a7eb36167712342212f87c189) )
ROM_END

// 'Center' unit, GPUs and YMZ770B not populated.
ROM_START( masmario )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "mpf_m04f.ic30", 0x000000, 0x200000, CRC(f83ffb1a) SHA1(fa0ec83c21d81288b69e23ee46db359a3902648e) )
	ROM_LOAD32_WORD( "mpf_m05f.ic33", 0x000002, 0x200000, CRC(fe19dfb7) SHA1(2fdc2feb86840448eb9e47f7bd4dcc9adfc36bdf) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF ) // not populated

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

// Satellite unit
ROM_START( masmarios )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS and game code/data
	ROM_LOAD32_WORD( "mpf_s04j.ic30", 0x000000, 0x400000, CRC(73d8b6bd) SHA1(8353df96107303427fc146da29b6a6a5d303c4ee) )
	ROM_LOAD32_WORD( "mpf_s05j.ic33", 0x000002, 0x400000, CRC(1b1e5429) SHA1(4b25fc83172c7422bfe3f2aed9d2ee6c8a2c537f) )
	ROM_LOAD32_WORD( "mpf_s06j.ic39", 0x800000, 0x400000, CRC(dc20e3cc) SHA1(5b9bd0fc4a6abdda16781727b01014b0a68ef8df) )
	ROM_LOAD32_WORD( "mpf_s07j.ic42", 0x800002, 0x400000, CRC(cb08dc74) SHA1(31e658f8bd03fea3dffa5f32dc7ac2e73930b383) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "mpf_s01.ic31", 0x000000, 0x400000, CRC(99688b6d) SHA1(2052471e2a742c05c2bbd6bcb24deca681df41c3) )
	ROM_LOAD16_WORD_SWAP( "mpf_s02.ic38", 0x400000, 0x400000, CRC(251f7111) SHA1(4d6e4111d76e7f56e9aeff19686dd84717ccb78a) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

ROM_START( masmario2 )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "mpv_s04.ic30", 0x000000, 0x400000, CRC(1217f923) SHA1(5b0c2331ce6bec6d5239c0a04c22e67ec16383c4) )
	ROM_LOAD32_WORD( "mpv_s05.ic33", 0x000002, 0x400000, CRC(63d59b97) SHA1(8d2ef35c0776b296cc7d3b37df5a996f6fb33bc1) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "flash1", 0x000000, 0x800000, BAD_DUMP CRC(30a7c77e) SHA1(3764a096ae22ecad4fba37ec62dc39c5381c5825) )

	ROM_REGION( 0x800100, "ymz770_flash2", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "flash2", 0x000000, 0x800000, BAD_DUMP CRC(35d015cf) SHA1(564313bca5bc85abc0d0d84b132981bfce386bc5) )

	ROM_REGION( 0x1000000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0,        0x800000 )
	ROM_COPY( "ymz770_flash2", 0, 0x800000, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "massmario2", 0, SHA1(9632c91bf2e4983ee29f417e3122e9380baee25b) )
ROM_END

////////////////////////
// Medalusion 1 platform
////////////////////////

// uses main board with 1 GPU populated and AMT-02012-01 I/O board

ROM_START( dkjfever )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS and game code/data
	ROM_LOAD32_WORD( "spd_04.ic30", 0x000000, 0x400000, CRC(76c91374) SHA1(4d1157fbbe46becd4182f948203816809ae792b6) )
	ROM_LOAD32_WORD( "spd_05.ic33", 0x000002, 0x400000, CRC(94722aca) SHA1(cf2e1d43b490a96ba4ca69d9865c6ea94ca6962d) )
	ROM_LOAD32_WORD( "spd_06.ic39", 0x800000, 0x400000, CRC(4db2f154) SHA1(e1bdefc95af70f7f697cc938e235d102b7afa18d) )
	ROM_LOAD32_WORD( "spd_07.ic42", 0x800002, 0x400000, CRC(074ccd84) SHA1(6a2d59d21de4d73febd29d4bfb5ba71f6015b40e) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "spd_01.ic31", 0x000000, 0x400000, CRC(672d5db8) SHA1(b57f0c4826b03bc6c5bcc7875a3807e7ce58b564) )
	ROM_LOAD16_WORD_SWAP( "spd_02.ic38", 0x400000, 0x400000, CRC(ea79aebf) SHA1(c7e607c205f6364798932f982d067b245a5355a0) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

ROM_START( mariojjl )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 )
	ROM_LOAD32_WORD( "spm_04c.ic30", 0x000000, 0x400000, CRC(159e912d) SHA1(5db1434d34e52f9c35d71e05675dd035765d2e6f) )
	ROM_LOAD32_WORD( "spm_05c.ic33", 0x000002, 0x400000, CRC(482d2b32) SHA1(01fb4b5f2441dc8c0f07943f190429c19c60b9d6) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "spm_01.ic31", 0x000000, 0x400000, CRC(141761a7) SHA1(ab1029c9277b3932d43308a7b4c106cd526a82c7) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

ROM_START( mmaruchan )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 )
	ROM_LOAD32_WORD( "spt_04b.ic30", 0x000000, 0x400000, CRC(9899f171) SHA1(d114c1ef0608c0740b7d58561c9f39c13b453e3a) )
	ROM_LOAD32_WORD( "spt_05b.ic33", 0x000002, 0x400000, CRC(108efb71) SHA1(3f9e1c59f7af60976d140bf68b75c270a364f3a2) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "spt_01.ic31", 0x000000, 0x400000, CRC(790b4bed) SHA1(3df68610f8b81dd5f74dca0f05da47a539b45163) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

ROM_START( mmaruchana )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 )
	ROM_LOAD32_WORD( "spt_04a.ic30", 0x000000, 0x400000, CRC(9c919870) SHA1(376a5f51e09ea0f32f511994d6a492bccdbbe0e2) )
	ROM_LOAD32_WORD( "spt_05a.ic33", 0x000002, 0x400000, CRC(f8794160) SHA1(86a44ce678f38413fc40bdc0bd5633fa00af8ddb) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "spt_01.ic31", 0x000000, 0x400000, CRC(790b4bed) SHA1(3df68610f8b81dd5f74dca0f05da47a539b45163) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

ROM_START( rockmanmdl )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS and game code/data
	ROM_LOAD32_WORD( "spr_04a.ic30", 0x000000, 0x400000, CRC(d402c580) SHA1(2bd1987be94f2118d30e264f7eeec012f724089f) )
	ROM_LOAD32_WORD( "spr_05a.ic33", 0x000002, 0x400000, CRC(96a51e8b) SHA1(1c5ee0512ccac1202693250e38763eb2c8fa4696) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "spr_01.ic31", 0x000000, 0x400000, CRC(91c313f9) SHA1(b593013f8a60499965db1c359f0e44e39a4d31b1) )
	ROM_LOAD16_WORD_SWAP( "spr_02.ic38", 0x400000, 0x400000, CRC(2de87492) SHA1(e125753eeb3d4a0e54dfad53449243834d57d374) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )
ROM_END

////////////////////////
// Medalusion 2 platform
////////////////////////

// CF card only dumped, boot ROMs is missing
ROM_START( dokodemo )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(74687757) SHA1(96b6e3725bcf16e92c6966f9b9ce93cfdd7ba641) ) // needs verification if this game really use same boot ROMs as pingu/gekimaka
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(ba2e6716) SHA1(49c5abb9d96e3f4a78ed4dced7a9f052a96b186d) ) //

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD16_WORD_SWAP( "flash1", 0x000000, 0x800000, BAD_DUMP CRC(dda4879f) SHA1(4aa06247ca674e86be6c111db7f6abf1ed6e121d) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "dokodemo", 0, SHA1(0c786b6857a29b26971578abe1c8439fe43d94b5) )
ROM_END

// only boot loader ROMs dumped, actual game CF card is missing
ROM_START( gekimaka )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "stm_04.ic30", 0x000000, 0x400000, CRC(74687757) SHA1(96b6e3725bcf16e92c6966f9b9ce93cfdd7ba641) )
	ROM_LOAD32_WORD( "stm_05.ic33", 0x000002, 0x400000, CRC(ba2e6716) SHA1(49c5abb9d96e3f4a78ed4dced7a9f052a96b186d) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	ROM_REGION( 0x8000, "m48t35", 0 )
	ROM_LOAD( "ic26", 0x000000, 0x008000, CRC(da921265) SHA1(c4a2d79d834c0333218c7a4853a8e4167036955a) )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "gekimaka", 0, NO_DUMP )
ROM_END

ROM_START( pingu )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "stp_04.ic30", 0x000000, 0x400000, CRC(74687757) SHA1(96b6e3725bcf16e92c6966f9b9ce93cfdd7ba641) )
	ROM_LOAD32_WORD( "stp_05.ic33", 0x000002, 0x400000, CRC(ba2e6716) SHA1(49c5abb9d96e3f4a78ed4dced7a9f052a96b186d) )

	ROM_REGION( 0x800100, "ymz770_flash1", 0 ) //sound samples flash rom, not really needed, programmed by boot loader
	ROM_LOAD( "ic10", 0x000000, 0x800100, CRC(04cf9722) SHA1(854e056a03d6f7ac9b438ba9ce8a0499a79bdec8) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "pingu", 0, SHA1(9c74e30906f229eba4bff8262c98e556d3ea1c23) )
ROM_END

// CF card only dumped, boot ROMs is missing
ROM_START( wontame )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "ic30", 0x000000, 0x400000, BAD_DUMP CRC(74687757) SHA1(96b6e3725bcf16e92c6966f9b9ce93cfdd7ba641) ) // needs verification if this game really use same boot ROMs as pingu/gekimaka
	ROM_LOAD32_WORD( "ic33", 0x000002, 0x400000, BAD_DUMP CRC(ba2e6716) SHA1(49c5abb9d96e3f4a78ed4dced7a9f052a96b186d) ) //

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )
	ROM_LOAD16_WORD_SWAP( "flash1", 0x000000, 0x800000, BAD_DUMP CRC(056e982e) SHA1(4b29218b00d7023c0b748aae13752f0c12539750) )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "wontame", 0, SHA1(eb4fe73d5f723b3af08d96c6d3061c9bbc7b2488) )
ROM_END

ROM_START( rockmanbc )
	ROM_REGION32_LE( 0x1000000, "maincpu", 0 ) // BIOS code
	ROM_LOAD32_WORD( "rbc_04.ic30", 0x000000, 0x400000, CRC(69c5e49f) SHA1(ea16e69c9fcb30541ccf72b663af6fe7c8c9dd82) )
	ROM_LOAD32_WORD( "rbc_05.ic33", 0x000002, 0x400000, CRC(35b6a112) SHA1(8f1669c6257e6759bf0df31e876dc337dff31dc8) )

	ROM_REGION( 0x800100, "ymz770_flash1", ROMREGION_ERASEFF )

	ROM_REGION( 0x800000, "ymz770", 0 )
	ROM_COPY( "ymz770_flash1", 0, 0, 0x800000 )

	DISK_REGION( "ata:0:cfcard" )
	DISK_IMAGE( "rockmanbc", 0, BAD_DUMP SHA1(685aca95ba133d84bba76f3292912903a531f058) ) // BAD_DUMP note: original IDENT dump missing, use hand crafted for now, media dump is OK
ROM_END

} // anonymous namespace


// Custom
GAME( 2005, alien,     0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Alien: The Arcade Medal Edition (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanana,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Donkey Kong Banana Kingdom (host)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2007, dkbanans,  dkbanana, masmario2, alien,   alien_state, init_dkbanans, ROT0, "Capcom",               "Donkey Kong Banana Kingdom (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, masmario,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party (center)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2004, masmarios, 0,        alien,     alien,   alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party (satellite)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, masmario2, 0,        masmario2, alien,   alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no Korokoro Party 2", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// Medalusion 1
GAME( 2005, dkjfever,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Donkey Kong Jungle Fever (Ver.1.000, 2005/08/03)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // ドンキーコングジャングルフィーバー
GAME( 2006, mariojjl,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Nintendo / Capcom",    "Super Mario Fushigi no JanJanLand (Ver.1.00C, 2006/08/29)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, mmaruchan, 0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Chibi Maruko-chan ~Minna de Sugoroku Asobi~ no Maki (Ver.1.00B, 2005/06/22)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // ちびまる子ちゃん「みんなですごろく遊び」の巻
GAME( 2004, mmaruchana,mmaruchan,alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Chibi Maruko-chan ~Minna de Sugoroku Asobi~ no Maki (Ver.1.00A, 2004/04/20)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // ちびまる子ちゃん「みんなですごろく遊び」の巻
GAME( 2005, rockmanmdl,0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Rockman EXE The Medal Operation", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// Medalusion 2
GAME( 2006, dokodemo,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Sony / Capcom",        "Doko Demo Issho: Toro's Fishing (Ver.1.00, 2006/06/07)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2005, gekimaka,  0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Geki Makaimura", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2006, pingu,     0,        alien,     alien,   alien_state, empty_init,    ROT0, "Pygos Group / Capcom", "Pingu's Ice Block (Ver.1.00 2006/01/27)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
GAME( 2008, wontame,   0,        alien,     wontame, alien_state, empty_init,    ROT0, "Capcom / Tomy",        "Won! Tertainment Happy Channel (Ver.1.00E 2008/02/21)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
// not medal machines
GAME( 2006, rockmanbc, 0,        alien,     alien,   alien_state, empty_init,    ROT0, "Capcom",               "Rockman EXE Battle Chip Stadium Ver.3.00", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
