// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

Notes:
- most BIOSes needs SH-4 MMU
- exception is with earlier games/BIOSes, for example:
  mamefoo scatmag2sa -bios au-sa1
  will run the program up to "vidgam.c: could not set the desired video mode"
  bp 800795a, 1, {r0=0;g}
- "v_scene.c: Could not initialize Display Manager" & "spin.c: cannot load assets",
  both related to PLX controller not DMA-ing?
- 0x0001CA1E - critical error handler in BIOS, R4 - pointer to error message

TODO:
- Identify irq sources;
- Irq acknowledge doesn't work as intended;
- Emulate PLX 9054 controller & PCI root;
- PMX1 likely runs under PCI bus too;
- Eventually gets to "flyback" errors, pings $12000130-$3f for strobe and/or irq enable.
  Flyback is an alias for vblank, cfr. Acorn Archimedes.

===================================================================================================

Aristocrat MK6 (2000)
Product numbers: 410480, 410481, 410556, 410557

Hitachi SH-4 7750
NEC PowerVR 250 (PMX1-LC)
Altera FLEX EPF10K100E
PLX PCI9054
16C554 4xUART
4x 1MB 42-pin EPROM sockets for system software
4x 4MB 42-pin EPROM sockets for game software


Aristocrat MK6-XP (2002)
Product numbers: 410540, 410541

same as above except:
- Altera APEX instead of FLEX
- doesn't have PCI controller
2x 2MB 42-pin EPROM sockets for system software
6x 4MB 42-pin EPROM sockets for game software


MK6 XP CCB (Communication Configuration Board)
Product numbers: 410748, 410749
72-pin SIMM, same form factor as PC EDO DRAM (but not RAM)

2x NXP MC145406D L0A3S202 UnG020F


MK6 System EPROM Board
Product numbers: 410484, 410485

2x 2MB 42-pin EPROM sockets for system software


MK6 Storage Expansion Board
Product number: 410663

2x 2MB 42-pin EPROM sockets for system software
14x 4MB 42-pin EPROM sockets for game software


PMC Video Board (used for dual-screen software)
Product number: 410535

NEC PowerVR 250 (PMX1-LC)
4x Samsung K4S643232H-TC60


MKV LAB Comms Board
Product numbers: 410174, 410175
Note: NSW/ACT games only - MK5 and MK6 hardware use the same board

U1: Unknown 20-pin socketed PLCC marked 563072-A
U3: TL16C452FN


MKV VLC Comms Inlet Board
Product numbers: 410275, 410276
Note: South Australian and Victorian games only - MK5 and MK6 hardware use the same board

U3: SIPEX SP232ACP


SPC 2.0 Board (Serial Protocol Converter)

Intel i386EX KU80386EX33
66.6666 MHz XTAL
Analog Devices ADM691AR
Lattice GAL16V8D
Samsung K6T4016C38-TB55
CR2032 battery


SPC 2.5 Board
Product numbers: 704R6, 432762, 432769


SPC 2.5 Type II Board
Product number: 432825


Comms protocols:

ASP games do not have a specific region, the region code is specified by the BIOS (base) used.
ASP stands for Aristocrat Serial Protocol.
Currently two ASP bases are dumped, one for Africa and one unknown region, possibly Malaysia.
These games use the ASP 1000 protocol.
The reason the latter's region is currently called Malaysia is that there seems to be code for RM (ringgit) denominations in the 15011025 system software.

Queensland and New Zealand games use the QCOM protocol.

The South Australian system set and game (only one dumped so far) uses the VLC (Video Lottery Consultants) protocol.

Victorian games (none are dumped) use either VLC or QCOM depending on the game and/or the operator the machines were designed for.
Victoria (excluding Crown Melbourne) originally had two rival gaming operators during the MK6's lifetime, Tabcorp, and Tattersall's, trading as Tabaret and Tatts Pokies respectively.
Crown Melbourne uses regular ASP software.

**************************************************************************************************/

#include "emu.h"
#include "cpu/sh/sh4.h"
#include "machine/eepromser.h"
#include "machine/ins8250.h"
#include "emupal.h"
#include "screen.h"

namespace {

class aristmk6_state : public driver_device
{
public:
	aristmk6_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_uart0(*this, "uart0"),
		m_uart1(*this, "uart1"),
		m_eeprom0(*this, "eeprom0"),
		m_palette(*this, "palette"),
		m_vram(*this, "vram")
	{ }

	void aristmk6(machine_config &config);

private:
	void testIrq();

	uint8_t irqpend_r(offs_t offset);
	void irqen_w(offs_t offset, uint8_t data);
	uint8_t test_r(offs_t offset);
	void eeprom_w(uint64_t data);
	uint64_t hwver_r();
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void video_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	void aristmk6_map(address_map &map) ATTR_COLD;
	void aristmk6_port(address_map &map) ATTR_COLD;

	u8 irl0pend = 0, irl0en = 0;
	u8 irl1pend = 0, irl1en = 0;
	u8 irl2pend = 0, irl2en = 0;    // UARTs ?
	u8 irl3pend0 = 0, irl3en0 = 0;
	u8 irl3pend1 = 0, irl3en1 = 0;

	required_device<sh4_device> m_maincpu;
	required_device<ns16550_device> m_uart0;
	required_device<ns16550_device> m_uart1;
	required_device<eeprom_serial_93cxx_device> m_eeprom0;
	required_device<palette_device> m_palette;
	required_shared_ptr<uint64_t> m_vram;
};


void aristmk6_state::video_start()
{
}
void aristmk6_state::machine_reset()
{
	irl0pend = irl0en = irl1pend = irl1en = irl2pend = irl2en = irl3pend0 = irl3en0 = irl3pend1 = irl3en1 = 0;
}

uint32_t aristmk6_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#if 0
	int x,y,count;
	const uint8_t *blit_ram = memregion("maincpu")->base();
	static u32 m_test_x = 0, m_test_y = 0, m_start_offs = 0;
	static u8 m_type = 0;

	if(machine().input().code_pressed(KEYCODE_Z))
		m_test_x++;

	if(machine().input().code_pressed(KEYCODE_X))
		m_test_x--;

	if(machine().input().code_pressed(KEYCODE_A))
		m_test_y++;

	if(machine().input().code_pressed(KEYCODE_S))
		m_test_y--;

	if(machine().input().code_pressed(KEYCODE_Q))
		m_start_offs+=0x2000;

	if(machine().input().code_pressed(KEYCODE_W))
		m_start_offs-=0x2000;

	if(machine().input().code_pressed(KEYCODE_E))
		m_start_offs++;

	if(machine().input().code_pressed(KEYCODE_R))
		m_start_offs--;

	if(machine().input().code_pressed_once(KEYCODE_L))
		m_type^=1;

	popmessage("%d %d %04x %d",m_test_x,m_test_y,m_start_offs,m_type);

	bitmap.fill(m_palette->black_pen(), cliprect);

	count = (m_start_offs);

	for(y=0;y<m_test_y;y++)
	{
		for(x=0;x<m_test_x;x++)
		{
			if(m_type)
			{
				uint16_t vram;
				int r,g,b;

				vram = blit_ram[count+0] | blit_ram[count+1]<<8;

				r = (vram & 0x001f)>>0;
				g = (vram & 0x07e0)>>5;
				b = (vram & 0xf800)>>11;

				r = (r << 3) | (r & 0x7);
				g = (g << 2) | (g & 3);
				b = (b << 3) | (b & 0x7);

				if(cliprect.contains(x, y))
					bitmap.pix(y, x) = r | g<<8 | b<<16;

				count+=2;
			}
			else
			{
				uint8_t color;

				color = blit_ram[count];

				if(cliprect.contains(x, y))
					bitmap.pix(y, x) = m_palette->pen(color);

				count++;
			}
		}
	}
#else

/*
    if (machine().input().code_pressed_once(KEYCODE_U))
    {
        irl2pend |= 2;
        testIrq();
        machine().debug_break();
    }
*/

	// TODO: honor cliprect, remove count variable, RGB565 format is unconfirmed, likely double buffered
	int count = 0;
	for (int y = 0; y < 480; y++)
	{
		for (int x = 0; x < 640 / 2; x++)
		{
			uint64_t pix = m_vram[count];

			uint32_t pix1;
			u8 r, g, b;

			pix1 = pix & 0xffffffff;
			r = pal5bit((pix1 & 0x001f) >> 0);
			g = pal6bit((pix1 & 0x07e0) >> 5);
			b = pal5bit((pix1 & 0xf800) >> 11);
			bitmap.pix(y, x * 2) = r | g << 8 | b << 16;

			pix1 = pix >> 32;
			r = pal5bit((pix1 & 0x001f) >> 0);
			g = pal6bit((pix1 & 0x07e0) >> 5);
			b = pal5bit((pix1 & 0xf800) >> 11);
			bitmap.pix(y, x * 2 + 1) = r | g << 8 | b << 16;

			count++;
		}
	}
#endif

	return 0;
}

void aristmk6_state::testIrq()
{
	m_maincpu->set_input_line(SH4_IRL0, (irl0pend & irl0en) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(SH4_IRL1, (irl1pend & irl1en) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(SH4_IRL2, (irl2pend & irl2en) ? ASSERT_LINE : CLEAR_LINE);
	m_maincpu->set_input_line(SH4_IRL3, ((irl3pend0 & irl3en0) || (irl3pend1 & irl3en1)) ? ASSERT_LINE : CLEAR_LINE);
}

uint8_t aristmk6_state::irqpend_r(offs_t offset)
{
	switch (offset)
	{
	case 0x00e8 - 0x00e8:
		return irl0pend;
	case 0x00f0 - 0x00e8:
		return irl1pend;
	case 0x00f8 - 0x00e8:
		return irl2pend;
	case 0x0100 - 0x00e8:
		return irl3pend0;
	case 0x0101 - 0x00e8:
		return irl3pend1;
	default:
		logerror("Unhandled read %08x\n", 0x120000e8 + offset);
		return 0;
	}
}

void aristmk6_state::irqen_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
	case 0x0108 - 0x0108:
		irl0en = data;
		irl0pend &= ~data;
		testIrq();
		break;
	case 0x0110 - 0x0108:
		irl1en = data;
		irl1pend &= ~data;
		testIrq();
		break;
	case 0x0118 - 0x0108:
		irl2en = data;
		irl2pend &= ~data;
		testIrq();
		break;
	case 0x0120 - 0x0108:
		irl3en0 = data;
		irl3pend0 &= ~data;
		testIrq();
		break;
	case 0x0121 - 0x0108:
		irl3en1 = data;
		irl3pend1 &= ~data;
		testIrq();
		break;
	default:
		logerror("Unhandled write %08x %02x\n", 0x12000108 + offset, data);
	}
}

uint8_t aristmk6_state::test_r(offs_t offset)
{
	static int flip;

	switch (offset)
	{
	case 0:
		flip ^= 2;
		// bit 1 read in various places, status for something...
		return flip;
	case 1:
		return (m_eeprom0->do_read() << 5);
	default:
		logerror("Unmapped read %08x\n", 0x13800000 + offset);
	}

	return 0;
}

void aristmk6_state::eeprom_w(uint64_t data)
{
	m_eeprom0->di_write((data & 0x01) >> 0);
	m_eeprom0->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom0->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

uint64_t aristmk6_state::hwver_r()
{
	// hardware version/revison register
	// bit 0-3: acceptable values 0-2 (deadloop otherwise), if > 0 - add 4 to unk B3800000 registers offsets
	// bit 4:   1 - to B24xxxxxx UARTs registers offsets will be added E0
	// bit 5-8: unk, checked for (bit4 == 1 && bit5-8 == 1), if true set flag at C406E20
	return 0;
}

void aristmk6_state::aristmk6_map(address_map &map)
{
	map(0x00000000, 0x003fffff).rom().region("maincpu", 0);
	map(0x04000000, 0x05ffffff).ram().share("vram"); // VRAM 32MB
	map(0x08000000, 0x0bffffff).rom().region("game_rom", 0);
	map(0x0c000000, 0x0cffffff).ram(); // Main RAM 16MB
	map(0x10800000, 0x1087ffff).ram(); // SRAM0 512KB
	map(0x11000000, 0x1107ffff).ram(); // SRAM1 512KB
	map(0x11800000, 0x1187ffff).ram(); // SRAM2 512KB
// 12000xxx main control registers area
//  map(0x12000000, another eeprom or SPI bus
	map(0x12000010, 0x12000017).w(FUNC(aristmk6_state::eeprom_w));
	map(0x12000078, 0x1200007f).nopw(); // watchdog ??
	map(0x12000080, 0x12000087).nopw(); // 0-1-2 written here repeatedly, diag LED or smth ?
	map(0x120000e0, 0x120000e7).r(FUNC(aristmk6_state::hwver_r));
	map(0x120000e8, 0x12000107).r(FUNC(aristmk6_state::irqpend_r));
	map(0x12000108, 0x12000127).w(FUNC(aristmk6_state::irqen_w));
	map(0x12400010, 0x12400017).rw(m_uart1, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x12400018, 0x1240001f).rw(m_uart0, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
//  map(0x13000000 MMIO space for PLX9054?
	map(0x13800000, 0x13800007).r(FUNC(aristmk6_state::test_r));
}

void aristmk6_state::aristmk6_port(address_map &map)
{
}

static INPUT_PORTS_START( aristmk6 )
INPUT_PORTS_END

// ?
static constexpr XTAL ARISTMK6_CPU_CLOCK = 200_MHz_XTAL;
// ?

void aristmk6_state::aristmk6(machine_config &config)
{
	/* basic machine hardware */
	SH4LE(config, m_maincpu, ARISTMK6_CPU_CLOCK);
	m_maincpu->set_md(0, 1);
	m_maincpu->set_md(1, 0);
	m_maincpu->set_md(2, 1);
	m_maincpu->set_md(3, 0);
	m_maincpu->set_md(4, 0);
	m_maincpu->set_md(5, 1);
	m_maincpu->set_md(6, 0);
	m_maincpu->set_md(7, 1);
	m_maincpu->set_md(8, 0);
	m_maincpu->set_sh4_clock(ARISTMK6_CPU_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &aristmk6_state::aristmk6_map);
	m_maincpu->set_addrmap(AS_IO, &aristmk6_state::aristmk6_port);
	m_maincpu->set_mmu_hacktype(2);
	m_maincpu->set_force_no_drc(true);
//  m_maincpu->set_disable();

	NS16550(config, "uart0", 8_MHz_XTAL);
	NS16550(config, "uart1", 8_MHz_XTAL);

	EEPROM_93C56_16BIT(config, m_eeprom0).default_value(0xff);

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500));  /* not accurate */
	screen.set_size(640, 480);
	screen.set_visarea(0, 640-1, 0, 480-1);
	screen.set_screen_update(FUNC(aristmk6_state::screen_update));

	PALETTE(config, m_palette).set_entries(0x1000);
}

#define ROM_LOAD32_WORD_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(bios))

#define ROM_LOAD64_WORD_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6) | ROM_BIOS(bios))

/*
EPROM labels all look basically the same between games

Standard platform:

    +--------------------------------------------------+
    | 20151911                                 NSW     |
    | INDIAN DREAMING- LL                              |
    | 742                  4MB              6 eproms   |
    |             Aristocrat © 2005        P     U72   |
    +--------------------------------------------------+

"ID string" + "Region" + "Game Name" + "Combination No." + "ROM size"
+ "Number of EPROMs" + "Copyright" + "Board location"

USA platform:

    +--------------------------------------------------+
    | 0251312-U86   Series:01   M27V322   Mk6USA       |
    | Diamond Eyes                                     |
    | E6A2D2DA                                         |
    | 4B5B-B5BF / 1E6C64C8 / 6FC8B9B6                  |
    | Copyright © 2003 Aristocrat Technologies, Inc.   |
    +--------------------------------------------------+

"ID string" + "Board location" + "Series" + "EPROM type" + "Platform"
+ "Game Name" + "Checksums" + "Copyright"

We reduce this to "ID string"."Location" in the filenames for simplicity, also for games which we don't have pictures for.

Decoding the game ID:
On the standard or "Australian" platform, the ID is split into four parts: Version number, Manufacturer, Game ID with the last two digits serving an unknown purpose (usually 11 but sometimes 21, 31 or 41 etc.)
In the case of the above game, this would be 2 (version), 0 (Aristocrat), 1519, 11.
For Jubilee games, the second digit is a J rather than a zero.

Note that in the Machine Identification menu, the last two digits of the ID are replaced with AV (standard software) or AD (demo software aka show program).

On the USA platform, the ID format is different:
The game ID seems to be split into three parts: Version number, manufacturer and Game ID, for example 0151178 (adonisa6).
In this case, the version number would be 01, followed by manufacturer (seems to always be 5), then 1178 for the game ID.
US games also show the ROM checksum on the label.

Game EPROMs are M27V322 (4MB), system/setchip EPROMs are M27V800 (1MB) or M27V160 (2MB). All three types have 42 pins.

Currently there are no dumps of the SPC (serial protocol converter) boards.
On the SPC2 PCB there is a 32-pin EPROM located at U3.
On SPC2.5 boards (both original and type II), the chip is a 32-pin PLCC rather than DIP, and is located at U19.

SPC2:

    +------------------------------------------------+
    | GAMBS2M 01.06.17    U3                         |
    | V. 01.06.17                                    |
    | 8H37 / 3B68 / 00DF3B68                         |
    |                                                |
    | Copyright © 2011 Aristocrat Technologies, Inc. |
    +------------------------------------------------+

Also:
    GAMBS2M 01.06.12   U3   V. 01.06.12   P244 / BB6D / 00DFBB6D   Copyright © 2011 Aristocrat Technologies Inc.
    GAMBS2M 01.06.03   U3   V. 01.06.03   35HH / AD8D / 00DDAD8D   Copyright © 2009 Aristocrat Technologies Inc.

SPC2.5:
U19 (program chip)
    +--------------+
    |              |
    |GAMBS2.5M     |
    |V. 02.06.28   |
    |69P1          |
    |              |
    +--------------+

U14 (configuration chip)
    +----------+
    | U14      |
    | SPC2.5   |
    | 1.3.3(0) |
    +----------+

Also:
    U19: GAMBS2.5M   V. 02.06.25   40P5
    U14: U14   SPC2.5 1.3.3(0) (note: same as above)

    U19: GAMBS2.5M   V. 02.06.19   1FH6
    U14: U14   SPC2.5 1.3.2(0)

    U19: GAMBS2.5M   V. 02.06.03   U8A6
    U14: SPC2.5   1.3.1(0)

BIOS/set chips are known to be in 3 locations depending on the PCB used:
"System Right" is at u83 on MK6 and MK6-XP boards, u2 on MK6 System EPROM board and u10 on Storage Expansion board
"System Left"  is at u70 on MK6 and MK6-XP boards, u3 on MK6 System EPROM board and u20 on Storage Expansion board

The earliest BIOSes, designed for the first-generation (non-XP) MK6 boards, use four 1MB chips, leaving room for only four game EPROM sockets on the board.
Later BIOSes, designed for the later MK6-XP system, use two 2MB chips, and MK6-XP boards have six game EPROM sockets instead of four.

Earlier BIOSes are usually not compatible with later games.

To test whether the BIOS loading is fine, let's check the correct endianness of text string in CPU space
typically at around 0x3100-0x3200, 0x3600-0x3700 for Left/Right BIOS dumps (2 files)
*/

#define ARISTMK6_BIOS \
	ROM_REGION( 0x0400000, "maincpu", ROMREGION_ERASEFF) \
	ROM_SYSTEM_BIOS( 0, "au-nsw1",   "Aristocrat MK6 Base (24013001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 0, "24013001_right.u83", 0x0000000, 0x0200000, CRC(e97afedf) SHA1(10ca3b015afaff5d7812f0f5207b2535602136a5) ) /* From mchilli & wldstall */ \
	ROM_LOAD32_WORD_BIOS( 0, "24013001_left.u70",  0x0000002, 0x0200000, CRC(06ae7e07) SHA1(39a45575b66906d73b519988d1001c99b05c5f34) ) \
	ROM_SYSTEM_BIOS( 1, "au-nsw2",   "Aristocrat MK6 Base (21012901, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 1, "21012901_right.u83", 0x0000000, 0x0200000, CRC(757618f2) SHA1(43f9a3e7d544979f8c6974945914d9e099b02abd) ) /* From lucky88 & redbara6 */ \
	ROM_LOAD32_WORD_BIOS( 1, "21012901_left.u70",  0x0000002, 0x0200000, CRC(0d271470) SHA1(5cd4b604bfe2fd7e9a8d08e1c7c97f17ae068479) ) \
	ROM_SYSTEM_BIOS( 2, "au-nsw3",   "Aristocrat MK6 Base (19012801, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 2, "19012801_right.u83", 0x0000000, 0x0200000, CRC(5b20a96c) SHA1(5fd916b7cc2cdd51bf7dd212c1114f94dc9c7926) ) /* From cactusco & cmtonig & heartveg & moonwalt */ \
	ROM_LOAD32_WORD_BIOS( 2, "19012801_left.u70",  0x0000002, 0x0200000, CRC(b03bd17c) SHA1(f281e80f6dda5b727ed71d2deebe3b0ff548773f) ) \
	ROM_SYSTEM_BIOS( 3, "au-nsw4",   "Aristocrat MK6 Base (13012001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 3, "13012001_right.u83", 0x0000000, 0x0200000, CRC(e627dbfa) SHA1(4fedbe0975ceb7dc0ebebf18a7708d78984db9b7) ) /* From grizzly & qnilea6 */ \
	ROM_LOAD32_WORD_BIOS( 3, "13012001_left.u70",  0x0000002, 0x0200000, CRC(38e8f659) SHA1(88c6acba99b0aca023c6f4d27c061c231490e9e0) ) \
	ROM_SYSTEM_BIOS( 4, "au-nsw5",   "Aristocrat MK6 Base (11011901, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 4, "11011901_right.u83", 0x0000000, 0x0200000, CRC(73dcb11c) SHA1(69ae4f32a0c9141b2a82ff3935b0cd20333d2964) ) \
	ROM_LOAD32_WORD_BIOS( 4, "11011901_left.u70",  0x0000002, 0x0200000, CRC(d3dd2210) SHA1(3548f8cc39859d3f44a55f6bae48966a2d48e0eb) ) \
	ROM_SYSTEM_BIOS( 5, "au-nsw6",   "Aristocrat MK6 Base (11011501, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 5, "11011501_right.u83", 0x0000000, 0x0200000, CRC(de4c3aed) SHA1(21596a2edd20eb7de7a4ec8900a270b09c8f326f) ) \
	ROM_LOAD32_WORD_BIOS( 5, "11011501_left.u70",  0x0000002, 0x0200000, CRC(c5cc3461) SHA1(5b43c4cb6110a6ccf67cd0f3789253f6872b20c4) ) \
	ROM_SYSTEM_BIOS( 6, "au-nsw7",   "Aristocrat MK6 Base (09011001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 6, "09011001_right.u83", 0x0000000, 0x0200000, CRC(8a853f80) SHA1(9a75498f7b02c81a483b4e1c158f35f0ee4c0112) ) \
	ROM_LOAD32_WORD_BIOS( 6, "09011001_left.u70",  0x0000002, 0x0200000, CRC(229c2e63) SHA1(91fd2b1acb69efe073647e93db9f11042add2feb) ) \
	ROM_SYSTEM_BIOS( 7, "au-nsw8",   "Aristocrat MK6 Base (07010801, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 7, "07010801_right.u83", 0x0000000, 0x0200000, CRC(8c148c11) SHA1(5ff3be18455b4f04675fec8d5b9d881295c65e23) ) /* From jailbrk & wzwaysll */ \
	ROM_LOAD32_WORD_BIOS( 7, "07010801_left.u70",  0x0000002, 0x0200000, CRC(8e92af68) SHA1(00d2bb655b7964a9652896741210ec534df0b0d2) ) \
	ROM_SYSTEM_BIOS( 8, "au-nsw9",   "Aristocrat MK6 Base (05010601, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 8, "05010601_right.u83", 0x0000000, 0x0100000, CRC(c12eac11) SHA1(683b9ddc323865ace7dca37d13b55de6e42759a5) ) /* From thaiprnc & venetian */ \
	ROM_LOAD32_WORD_BIOS( 8, "05010601_left.u70",  0x0000002, 0x0100000, CRC(b3e6b4a0) SHA1(3bf398c9257579f8e51ce716d6ebfa74fa510273) ) \
	ROM_SYSTEM_BIOS( 9, "au-nsw10",   "Aristocrat MK6 Base (04010501, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 9, "04010501_right.u83", 0x0000000, 0x0100000, CRC(3daefb7a) SHA1(411471713219f4bab5ccf5fe7a12a6c138c8c550) ) /* From di4ever */ \
	ROM_LOAD32_WORD_BIOS( 9, "04010501_left.u70",  0x0000002, 0x0100000, CRC(21182775) SHA1(7c5b7f5aba3babc85f512a8f7d4ebc0d83eb842a) ) \
	ROM_SYSTEM_BIOS( 10, "au-nsw11",   "Aristocrat MK6 Base (03010301, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 10, "03010301.u84", 0x0000000, 0x0100000, CRC(a34a9f16) SHA1(b8750e6ceb1715da8e5ac2f0183254e29a042641) ) /* From qnilese */ \
	ROM_LOAD64_WORD_BIOS( 10, "03010301.u71", 0x0000002, 0x0100000, CRC(d793440a) SHA1(dced4c04bde13293af77a9a1f4c5c606e3758de0) ) \
	ROM_LOAD64_WORD_BIOS( 10, "03010301.u83", 0x0000004, 0x0100000, CRC(c8580554) SHA1(58b8bfff2f8d298c4e3be2b01900800c45fa7ad7) ) \
	ROM_LOAD64_WORD_BIOS( 10, "03010301.u70", 0x0000006, 0x0100000, CRC(5ae69121) SHA1(36dd3f9aaf5f7d2751d1954d67f898bc3ec71f3b) ) \
	ROM_SYSTEM_BIOS( 11, "au-nsw12",   "Aristocrat MK6 Base (02010201, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 11, "02010201.u84", 0x0000000, 0x0100000, CRC(0920930f) SHA1(771b0f62442d1c75b1bb59ad82365b7ab8747173) ) /* From mnytree & gogreen */ \
	ROM_LOAD64_WORD_BIOS( 11, "02010201.u71", 0x0000002, 0x0100000, CRC(24d5614a) SHA1(fdcf3826dccc72b74b66379b1411cf211d5a1670) ) \
	ROM_LOAD64_WORD_BIOS( 11, "02010201.u83", 0x0000004, 0x0100000, CRC(5f64a20c) SHA1(397404ab6d2a1aa3c1fc77bb9421fef7079b65a5) ) \
	ROM_LOAD64_WORD_BIOS( 11, "02010201.u70", 0x0000006, 0x0100000, CRC(9b2db442) SHA1(d512398a2d9257bd385dc50d61c63cd1a47300ba) ) \
	ROM_SYSTEM_BIOS( 12, "au-nsw13",   "Aristocrat MK6 Base (02010114, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 12, "02010114.u84", 0x0000000, 0x0100000, CRC(183e3836) SHA1(4c802d0cd010bc007acb3a83e37aaa29b2d13d87) ) /* From arabnga6 & hlntroy */ \
	ROM_LOAD64_WORD_BIOS( 12, "02010114.u71", 0x0000002, 0x0100000, CRC(8f83c3dd) SHA1(a5f9d80b4b515b24299d0241e1665cfd9da8bab7) ) \
	ROM_LOAD64_WORD_BIOS( 12, "02010114.u83", 0x0000004, 0x0100000, CRC(945104d7) SHA1(e372d0cf889c72b5d001b26fe4a925a28486537f) ) \
	ROM_LOAD64_WORD_BIOS( 12, "02010114.u70", 0x0000006, 0x0100000, CRC(3ba4379f) SHA1(84367f12c4c9224d2ab9cae83ae8727de338408c) ) \
	ROM_SYSTEM_BIOS( 13, "au-qld1",   "Aristocrat MK6 Base (25012805, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 13, "25012805_right.u83", 0x0000000, 0x0200000, CRC(2ecd8da8) SHA1(389e9668b2ba4fffed5d2721b2ce70d502fb9f67) ) \
	ROM_LOAD32_WORD_BIOS( 13, "25012805_left.u70",  0x0000002, 0x0200000, CRC(996f32ce) SHA1(cf21bef745986fcbd298167453c7b8e5945ce602) ) \
	ROM_SYSTEM_BIOS( 14, "au-qld2",   "Aristocrat MK6 Base (20012605, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 14, "20012605_right.u83", 0x0000000, 0x0200000, CRC(045b82ad) SHA1(b8e4f9f826970d83ae5fd2f2898de12ad1bf2d24) ) \
	ROM_LOAD32_WORD_BIOS( 14, "20012605_left.u70",  0x0000002, 0x0200000, CRC(87331111) SHA1(6cdc2d81f68de23af18a975a6f27ddec246be405) ) \
	ROM_SYSTEM_BIOS( 15, "au-qld3",   "Aristocrat MK6 Base (20012305, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 15, "20012305_right.u83", 0x0000000, 0x0200000, CRC(e436c1f5) SHA1(62ee529cc971fd76aa2ccc15778e3f0c40e3e47f) ) /* From spgfest */ \
	ROM_LOAD32_WORD_BIOS( 15, "20012305_left.u70",  0x0000002, 0x0200000, CRC(ea8961cc) SHA1(0ebc7c3b94a6e01ee984af4711043130d9670bd3) ) \
	ROM_SYSTEM_BIOS( 16, "au-qld4",   "Aristocrat MK6 Base (14011605, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 16, "14011605_right.u83", 0x0000000, 0x0200000, CRC(2bec5b74) SHA1(854733cada75e632f01f7096d4740ed4941a3d5b) ) /* From moonfire */ \
	ROM_LOAD32_WORD_BIOS( 16, "14011605_left.u70",  0x0000002, 0x0200000, CRC(cd26d4f0) SHA1(40822714abf08aeb08d827dbd8cd099f86803754) ) \
	ROM_SYSTEM_BIOS( 17, "au-qld5",   "Aristocrat MK6 Base (04041205, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 17, "04041205_right.u83", 0x0000000, 0x0100000, CRC(ca6bc86c) SHA1(69fe7fc35694e4cd7f861bff4ec3a6165a81df6e) ) /* From csdjc & spgcarnjc */ \
	ROM_LOAD32_WORD_BIOS( 17, "04041205_left.u70",  0x0000002, 0x0100000, CRC(dfb9a119) SHA1(814a5a7877392aec4e4871d7f0e19d2fbd717409) ) \
	ROM_SYSTEM_BIOS( 18, "au-qld6",   "Aristocrat MK6 Base (03130334, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 18, "03130334_right.u83", 0x0000000, 0x0200000, CRC(bce3d97f) SHA1(da36377cc1465022a2434703adee63bf48c71a9c) ) \
	ROM_LOAD32_WORD_BIOS( 18, "03130334_left.u70",  0x0000002, 0x0200000, CRC(02175fde) SHA1(4e9a9e1e803a0c84b06aec99dc3147dd7a919eee) ) \
	ROM_SYSTEM_BIOS( 19, "au-qld7",   "Aristocrat MK6 Base (01040505, Queensland)" ) \
	ROM_LOAD64_WORD_BIOS( 19, "01040505.u84", 0x0000000, 0x0100000, CRC(cf5a9d1e) SHA1(0ebba478fc883831d70b0fa95f43e5f93b07ae9e) ) /* From smtma6q */ \
	ROM_LOAD64_WORD_BIOS( 19, "01040505.u71", 0x0000002, 0x0100000, CRC(f56ea77e) SHA1(319be1bee66a289e2c1f6beec07758f79aa0cf16) ) \
	ROM_LOAD64_WORD_BIOS( 19, "01040505.u83", 0x0000004, 0x0100000, CRC(90f32169) SHA1(228be8b4a9eb6b2acf7f7a7561bd194009936026) ) \
	ROM_LOAD64_WORD_BIOS( 19, "01040505.u70", 0x0000006, 0x0100000, CRC(b9ddea66) SHA1(f4bfdeada39a3f0094d6468b7374a34f88f5df7f) ) \
	ROM_SYSTEM_BIOS( 20, "au-sa1",   "Aristocrat MK6 Base (03030708, South Australia)" ) \
	ROM_LOAD32_WORD_BIOS( 20, "03030708_right.u83", 0x0000000, 0x0100000, CRC(b4b3c6a5) SHA1(5747f98a6eaa5c24a23d1d76a28b33a3bfbbfd1f) ) /* From scatmag2sa */ \
	ROM_LOAD32_WORD_BIOS( 20, "03030708_left.u70",  0x0000002, 0x0100000, CRC(4e5ad823) SHA1(77ab1c29c6172cfdcef776222a72b2b44114d4da) ) \
	ROM_SYSTEM_BIOS( 21, "nz1",   "Aristocrat MK6 Base (14011913, New Zealand)" ) \
	ROM_LOAD32_WORD_BIOS( 21, "14011913_right.u83", 0x0000000, 0x0200000, CRC(01d13b89) SHA1(b1013366d0803dfbec5a5f90f6a5cea862de0513) ) /* From doltra6nz & spgfestnz */ \
	ROM_LOAD32_WORD_BIOS( 21, "14011913_left.u70",  0x0000002, 0x0200000, CRC(9a4cefdf) SHA1(6c15bc565ede8af19361d60ee1e6657a8055c92c) ) \
	ROM_SYSTEM_BIOS( 22, "nz2",   "Aristocrat MK6 Base (14010152, New Zealand)" ) \
	ROM_LOAD32_WORD_BIOS( 22, "14010152_right.u83", 0x0000000, 0x0200000, CRC(7e3f61f6) SHA1(1e27d72c35b0c633187159ef434f22398df28882) ) /* From tiktorchnz */ \
	ROM_LOAD32_WORD_BIOS( 22, "14010152_left.u70",  0x0000002, 0x0200000, CRC(2716e1ef) SHA1(81fe1ae4f9cd1bcb24795ce85913ee22ed0fabcd) ) \
	ROM_SYSTEM_BIOS( 23, "nz3",   "Aristocrat MK6 Base (02061013, New Zealand)" ) \
	ROM_LOAD32_WORD_BIOS( 23, "02061013_right.u83", 0x0000000, 0x0100000, CRC(7a8619a5) SHA1(bd03ddb68817c1660b009e102ccf69e5b603b875) ) \
	ROM_LOAD32_WORD_BIOS( 23, "02061013_left.u70",  0x0000002, 0x0100000, CRC(e70a7007) SHA1(0935f924866162d9c0fbdbb99391cbf730a04b76) ) \
	ROM_SYSTEM_BIOS( 24, "nz4",   "Aristocrat MK6 Base (02060913, New Zealand)" ) \
	ROM_LOAD32_WORD_BIOS( 24, "02060913_right.u83", 0x0000000, 0x0100000, CRC(31068c41) SHA1(962da0079495a64f7ffb34be643892c272017cc9) ) /* From 5dragnz */ \
	ROM_LOAD32_WORD_BIOS( 24, "02060913_left.u70",  0x0000002, 0x0100000, CRC(d6a6713c) SHA1(0f3bb2746f1a6fa6a587fd50827299408a3b28d2) ) \
	ROM_SYSTEM_BIOS( 25, "my",   "Aristocrat MK6 Base (15011025, Malaysia)" ) \
	ROM_LOAD32_WORD_BIOS( 25, "15011025_right.u83", 0x0000000, 0x0200000, CRC(bf21a975) SHA1(a251b1a7342387300689cd50fe4ce7975b903ac5) ) /* ASP software */ \
	ROM_LOAD32_WORD_BIOS( 25, "15011025_left.u70",  0x0000002, 0x0200000, CRC(c02e14b0) SHA1(6bf98927813519dfe60e582dbe5be3ccd87f7c91) ) \
	ROM_SYSTEM_BIOS( 26, "afr",   "Aristocrat MK6 Base (24010467, Africa)" ) \
	ROM_LOAD32_WORD_BIOS( 26, "24010467_right.u83", 0x0000000, 0x0200000, CRC(eddeff13) SHA1(77ccbcf40aeb7305eb13d6d24efafd09955f1eac) ) /* ASP software */ \
	ROM_LOAD32_WORD_BIOS( 26, "24010467_left.u70",  0x0000002, 0x0200000, CRC(9093d820) SHA1(05bb14895e3077d277a1d0822036d08f359c0307) ) \
	ROM_SYSTEM_BIOS( 27, "us1",   "Aristocrat MK6 Base (01.04.11, US)" ) \
	ROM_LOAD32_WORD_BIOS( 27, "01.04.11_right.u83", 0x0000000, 0x0200000, CRC(2dae8ca0) SHA1(7a0fb38b4c1ac7195d15bdab6f0cfb16c78430f0) ) /* From centrion */ \
	ROM_LOAD32_WORD_BIOS( 27, "01.04.11_left.u70",  0x0000002, 0x0200000, CRC(787f2b07) SHA1(2548289e44f4b935346b759afb5383bdbac04c3e) ) \
	ROM_SYSTEM_BIOS( 28, "us2",   "Aristocrat MK6 Base (01.04.10, US)" ) \
	ROM_LOAD32_WORD_BIOS( 28, "01.04.10_right.u83", 0x0000000, 0x0200000, CRC(82ce2fcc) SHA1(4c8fb3db084a67e99d1420b3f895a06ce9ef5ec2) ) \
	ROM_LOAD32_WORD_BIOS( 28, "01.04.10_left.u70",  0x0000002, 0x0200000, CRC(9d9d52c1) SHA1(b957220cdbedd516c219d1bfc28807ce466df93f) ) \
	ROM_SYSTEM_BIOS( 29, "us3",   "Aristocrat MK6 Base (01.04.08, US)" ) \
	ROM_LOAD32_WORD_BIOS( 29, "01.04.08_right.u83", 0x0000000, 0x0200000, CRC(95333304) SHA1(7afe49d6c5e4d6820f349778557daa88c5366a51) ) /* From bobdolly, also u10 on EPROM expansion board */ \
	ROM_LOAD32_WORD_BIOS( 29, "01.04.08_left.u70",  0x0000002, 0x0200000, CRC(0dfcad10) SHA1(53798be000304aed38909f5fd8470a68bedd8229) ) /* also u20 on EPROM expansion board */ \
	ROM_SYSTEM_BIOS( 30, "us4",   "Aristocrat MK6 Base (01.04.07, US)" ) \
	ROM_LOAD32_WORD_BIOS( 30, "01.04.07_right.u83", 0x0000000, 0x0200000, CRC(23c28e22) SHA1(98f24a1f86232b6c2c288a61ec7d60c867f192e5) ) \
	ROM_LOAD32_WORD_BIOS( 30, "01.04.07_left.u70",  0x0000002, 0x0200000, CRC(acfb0fe0) SHA1(b1a772d7978e6ff4406a5bb39a71cb3f89608e72) ) \
	ROM_SYSTEM_BIOS( 31, "us5",   "Aristocrat MK6 Base (01.04.04, US)" ) \
	ROM_LOAD32_WORD_BIOS( 31, "01.04.04_right.u83", 0x0000000, 0x0200000, CRC(e57ba02d) SHA1(8e29403e6b619eeab41dc171221720bc7820ccdc) ) \
	ROM_LOAD32_WORD_BIOS( 31, "01.04.04_left.u70",  0x0000002, 0x0200000, CRC(b984a92c) SHA1(90f7a61302caee40195c08565bdac856a3234c1d) ) \
	ROM_SYSTEM_BIOS( 32, "us6",   "Aristocrat MK6 Base (01.03.17, US)" ) \
	ROM_LOAD32_WORD_BIOS( 32, "01.03.17_right.u83", 0x0000000, 0x0200000, CRC(1582714b) SHA1(92d0a15314ffe526159bef9a364898dd1ebdfde7) ) \
	ROM_LOAD32_WORD_BIOS( 32, "01.03.17_left.u70",  0x0000002, 0x0200000, CRC(a88193dc) SHA1(c9e1d483edaecd318d2e5fc8a54e84516c93e0ca) ) \
	ROM_SYSTEM_BIOS( 33, "us7",   "Aristocrat MK6 Base (01.03.14, US)" ) \
	ROM_LOAD32_WORD_BIOS( 33, "01.03.14_right.u83", 0x0000000, 0x0200000, CRC(889ffd82) SHA1(9c98c9cdcf5f7d05095f11006418133029e9f0f8) ) /* From 5dragce */ \
	ROM_LOAD32_WORD_BIOS( 33, "01.03.14_left.u70",  0x0000002, 0x0200000, CRC(7138fec4) SHA1(f81331d1875ac574d3e6c98be218ff25c6c7be5a) ) \
	ROM_SYSTEM_BIOS( 34, "us8",   "Aristocrat MK6 Base (01.03.07, US)" ) \
	ROM_LOAD32_WORD_BIOS( 34, "01.03.07_right.u83", 0x0000000, 0x0200000, CRC(2ebccc4e) SHA1(9342724e4451e9ab24ceae208284b50abd4f0be3) ) /* From pompeia6u */ \
	ROM_LOAD32_WORD_BIOS( 34, "01.03.07_left.u70",  0x0000002, 0x0200000, CRC(a3632da4) SHA1(1c96a88e86095b81801ab88e36a4cdfa4b893265) ) \
	ROM_SYSTEM_BIOS( 35, "us9",   "Aristocrat MK6 Base (01.03.06, US)" ) \
	ROM_LOAD32_WORD_BIOS( 35, "01.03.06_right.u83", 0x0000000, 0x0200000, CRC(bd48ca55) SHA1(8fb1576cbeb1c64c358880714740195d2e73e03e) ) /* From dimeyeu */ \
	ROM_LOAD32_WORD_BIOS( 35, "01.03.06_left.u70",  0x0000002, 0x0200000, CRC(2f9d9a29) SHA1(fdebfaca9a579d7249379f19aef22fbfd66bf943) ) \
	ROM_SYSTEM_BIOS( 36, "us10",   "Aristocrat MK6 Base (01.03.05, US)" ) \
	ROM_LOAD32_WORD_BIOS( 36, "01.03.05_right.u83", 0x0000000, 0x0200000, CRC(2c7f1ec3) SHA1(d03167f43ed6f9596080d91472695829378cef0a) ) \
	ROM_LOAD32_WORD_BIOS( 36, "01.03.05_left.u70",  0x0000002, 0x0200000, CRC(0095e3f9) SHA1(d2e8786158b1ab0a614aab21cf1d14cbc04754af) ) \
	ROM_SYSTEM_BIOS( 37, "us11",   "Aristocrat MK6 Base (01.03.03e, US)" ) \
	ROM_LOAD32_WORD_BIOS( 37, "01.03.03e_right.u83", 0x0000000, 0x0200000, CRC(2255e263) SHA1(5e9e093aaa17172f47a14c3baf7f6f0f73b19398) ) \
	ROM_LOAD32_WORD_BIOS( 37, "01.03.03e_left.u70",  0x0000002, 0x0200000, CRC(ea50729a) SHA1(14b5a71bfb91ac366ddcb5f77fb54127808f8163) ) \
	ROM_SYSTEM_BIOS( 38, "us12",   "Aristocrat MK6 Base (01.03.03a, US)" ) \
	ROM_LOAD32_WORD_BIOS( 38, "01.03.03a_right.u83", 0x0000000, 0x0200000, CRC(253415f4) SHA1(50dc77ad87bc6be1932dda2fd4865602c8c49729) ) \
	ROM_LOAD32_WORD_BIOS( 38, "01.03.03a_left.u70",  0x0000002, 0x0200000, CRC(4ab5dd40) SHA1(a6812cc624e6a98ea7b0697e2797fe10ba8e303e) ) \
	ROM_SYSTEM_BIOS( 39, "us13",   "Aristocrat MK6 Base (01.02.08, US)" ) \
	ROM_LOAD32_WORD_BIOS( 39, "01.02.08_right.u2", 0x0000000, 0x0100000, CRC(aaaeac8c) SHA1(a565e5fcb4f55f31e7d36be40eec234248a66efd) ) \
	ROM_LOAD32_WORD_BIOS( 39, "01.02.08_left.u3",  0x0000002, 0x0100000, CRC(f29fd1bf) SHA1(33e043d2616e10a1c7a0936c3d208f9bcc2ca6f3) ) \
	ROM_SYSTEM_BIOS( 40, "set-us1",   "Aristocrat MK6 Set Chips (06.03.04, US)" ) \
	ROM_LOAD32_WORD_BIOS( 40, "06.03.04_right.u2", 0x0000000, 0x0100000, CRC(6f5f5ef1) SHA1(70a43fba4de47ed8dcf38b25eafd5873f3428e72) ) \
	ROM_LOAD32_WORD_BIOS( 40, "06.03.04_left.u3",  0x0000002, 0x0100000, CRC(7034f26b) SHA1(7be78f23bec38d05240cdfe1186ec0c8291f5a1c) ) \
	ROM_SYSTEM_BIOS( 41, "set-us2",   "Aristocrat MK6 Set Chips (06.03.03, US)" ) \
	ROM_LOAD32_WORD_BIOS( 41, "06.03.03_right.u2", 0x0000000, 0x0100000, CRC(98763498) SHA1(246e95cc12eb34f946b2f4938c59217718f6d841) ) \
	ROM_LOAD32_WORD_BIOS( 41, "06.03.03_left.u3",  0x0000002, 0x0100000, CRC(a6924238) SHA1(b71ab39bf9c1fdbab556028138749e8c040ec83c) ) \
	ROM_SYSTEM_BIOS( 42, "set-us3",   "Aristocrat MK6 Set Chips (06.02.20, US)" ) \
	ROM_LOAD32_WORD_BIOS( 42, "06.02.20_right.u83", 0x0000000, 0x0100000, CRC(e4001f60) SHA1(5da34efb1ac0f7c84a48e09363d20cfecda4bcf1) ) \
	ROM_LOAD32_WORD_BIOS( 42, "06.02.20_left.u70",  0x0000002, 0x0100000, CRC(199ed3b9) SHA1(e3ee81ffd713f09e35a10c38e4f59282e2c5cd30) ) \
	ROM_SYSTEM_BIOS( 43, "set-us4",   "Aristocrat MK6 Set Chips (06.02.04, US)" ) \
	ROM_LOAD32_WORD_BIOS( 43, "06.02.04_right.u2", 0x0000000, 0x0100000, CRC(1cf5a853) SHA1(64d17efcce702df7a0b0e151293199478e25226d) ) \
	ROM_LOAD32_WORD_BIOS( 43, "06.02.04_left.u3",  0x0000002, 0x0100000, CRC(117b75f2) SHA1(2129286853d3c50b8a943b71334d4ef6b98adc05) )


ROM_START( aristmk6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
ROM_END


// 20161011 / 744
// 5 Dragons / Local / C - 14/01/04
// Variations (% and NO):
//    88.55% 99
//    87.88% 01
//    86.88% 02
ROM_START( 5drag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20161011.u86", 0x0000000, 0x0400000, CRC(ac83a71b) SHA1(1d6a3617e882e86100b939d62db7a7e1f86d2178) )
	ROM_LOAD32_WORD("20161011.u73", 0x0000002, 0x0400000, CRC(23cfdacd) SHA1(7efda9cc67214f68d674f1110cfb38fd9bbc4548) )
	ROM_LOAD32_WORD("20161011.u85", 0x0800000, 0x0400000, CRC(12803341) SHA1(4b640c3f270eaa77ce71e87f83e603083bcae489) )
	ROM_LOAD32_WORD("20161011.u72", 0x0800002, 0x0400000, CRC(e81d47fe) SHA1(f5730f9b5fa9a4492289ddcbc8e88d0a62771667) )
ROM_END


// 20161011 / 744
// 5 Dragons / Local / C - 14/01/04
// SHOW PROGRAM
// Variations (% and NO):
//    88.55% 99
// Identified as 201610AD in the Machine Identification menu, rather than 201610AV from the parent set
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( 5dragsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__20161011.u86", 0x0000000, 0x0400000, CRC(726f45a0) SHA1(b48227f4438b52857f43d26baa216a3b1d1fd27f) )
	ROM_LOAD32_WORD("sp__20161011.u73", 0x0000002, 0x0400000, CRC(6188048d) SHA1(d03d2ef8ce744f6523bc5a3775013764386acdf5) )
	ROM_LOAD32_WORD("sp__20161011.u85", 0x0800000, 0x0400000, CRC(9840fdcb) SHA1(ee87ab3e93fafd4a262451e906cbae2028cc2f61) )
	ROM_LOAD32_WORD("sp__20161011.u72", 0x0800002, 0x0400000, CRC(a534f262) SHA1(27201f8fd9a6c55e41e30d73e2868961961df827) )
ROM_END


// 10176611 / 744
// 5 Dragons / ASP / G - 15/07/04
// Variations (% and NO):
//    88.54% 99
ROM_START( 5draga )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10176611.u86", 0x0000000, 0x0400000, CRC(a4d91f15) SHA1(e1d12de5bbd6bed803fc26bf0580179993a453e7) )
	ROM_LOAD32_WORD("10176611.u73", 0x0000002, 0x0400000, CRC(1107e9b4) SHA1(8087e36cd4acada42ec10e40db3e40ada4286100) )
	ROM_LOAD32_WORD("10176611.u85", 0x0800000, 0x0400000, CRC(a6e4b2a9) SHA1(bc1153059da756f42bf9e4ed269b54552af48207) )
	ROM_LOAD32_WORD("10176611.u72", 0x0800002, 0x0400000, CRC(89ba2892) SHA1(96379c292fb86ea4a1d91084c57e4b86f2fbd4eb) )
ROM_END


// 0152309 / US211
// 5 Dragons / Local / A - 01/09/07
// Cash Express Hyperlink game
// Variations (% and NO):
//    87.899% 99
//    89.935% 01
//    91.848% 02
//    94.886% 03
//    97.086% 04
//    82.180% 05 (Hyperlink)
ROM_START( 5dragce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152309.u86", 0x0000000, 0x0400000, CRC(1c3fd707) SHA1(3a9fc3e865c43ae0afa31954b2372487eb729d8a) )
	ROM_LOAD32_WORD("0152309.u73", 0x0000002, 0x0400000, CRC(7298e59b) SHA1(b104aec9be996bacad74120f461ad8cd7ef96e09) )
	ROM_LOAD32_WORD("0152309.u85", 0x0800000, 0x0400000, CRC(4af3cae0) SHA1(79eb28b403aacf3ec7f52a5b3ff9e59ea35933c6) )
	ROM_LOAD32_WORD("0152309.u72", 0x0800002, 0x0400000, CRC(e399df66) SHA1(d85a8011ab62b899b18873e4296c12da4291870a) )
	ROM_LOAD32_WORD("0152309.u84", 0x1000000, 0x0400000, CRC(f37a1ce5) SHA1(8684d64b20e1fe853e3b0f1a31796cddeb8b0dee) )
	ROM_LOAD32_WORD("0152309.u71", 0x1000002, 0x0400000, CRC(a0f5999e) SHA1(0be0d58588adbdbc7b728f164538d03509f13fe5) )
ROM_END


// 10178611 / MV4186
// 5 Dragons / NZ - QCOM / B - 21/08/03
// Variations (% and NO):
//    87.79% 99
ROM_START( 5dragnz )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	10178611   NZ   5 DRAGONS   MV4186   4MB   4 eproms   Aristocrat © 2005   P   U86
	10178611   NZ   5 DRAGONS   MV4186   4MB   4 eproms   Aristocrat © 2005   P   U73
	10178611   NZ   5 DRAGONS   MV4186   4MB   4 eproms   Aristocrat © 2005   P   U85
	10178611   NZ   5 DRAGONS   MV4186   4MB   4 eproms   Aristocrat © 2005   P   U72
	*/
	ROM_LOAD32_WORD("10178611.u86", 0x0000000, 0x0400000, CRC(60c26bb2) SHA1(078252462e8cf435f359f3e6cd07b7785f97ac16) )
	ROM_LOAD32_WORD("10178611.u73", 0x0000002, 0x0400000, CRC(149ff293) SHA1(96ac51e62f12d4dcda79a0336023f9f4dde3130d) )
	ROM_LOAD32_WORD("10178611.u85", 0x0800000, 0x0400000, CRC(246ec8d6) SHA1(9f1c41101d7e27aea929e0d2b91a9ab08745d837) )
	ROM_LOAD32_WORD("10178611.u72", 0x0800002, 0x0400000, CRC(a92ad05e) SHA1(dbd355f53f64af2690e1db08343b2233d59de890) )
ROM_END


// 10250711 / 744/2
// 5 Koi / C - 25/10/07
// Variations (% and NO):
//    88.54% 99
//    87.88% 01
//    86.87% 02
ROM_START( 5koi )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10250711.u86", 0x0000000, 0x0400000, CRC(a245db85) SHA1(a03cf7978531edbc0ec1ba97e12fd59589caa90d) )
	ROM_LOAD32_WORD("10250711.u73", 0x0000002, 0x0400000, CRC(13fbfa4d) SHA1(8a9ba4fcea44f1ababa9ee89e36c8cd63b935ac5) )
	ROM_LOAD32_WORD("10250711.u85", 0x0800000, 0x0400000, CRC(2ce103f8) SHA1(eb457c0812f08e3a0b4450e44a80c5e13c131ea9) )
	ROM_LOAD32_WORD("10250711.u72", 0x0800002, 0x0400000, CRC(1a1140f2) SHA1(c097bcdcbd61edd15382f7becc32384780fb6e8e) )
	ROM_LOAD32_WORD("10250711.u84", 0x1000000, 0x0400000, CRC(413063ce) SHA1(4e933e1c0e576b028c16ab6f4630ee61575a5850) )
	ROM_LOAD32_WORD("10250711.u71", 0x1000002, 0x0400000, CRC(460e7ddb) SHA1(e6326523fb7c1c44cd0f01cd43d59d86a72c0d1d) )
ROM_END


// 1J016211 / JB067
// 5 Koi / ASP / A - 23/02/07
// Variations (% and NO):
//    88.54% 99
ROM_START( 5koia )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j016211.u86", 0x0000000, 0x0400000, CRC(48c39d21) SHA1(2112667dd6a6e360fe1909ee329e6b8c0f0bc2ee) )
	ROM_LOAD32_WORD("1j016211.u73", 0x0000002, 0x0400000, CRC(1d52e0ad) SHA1(7ab1722a86288d761fab89ae839560324a83ef0a) )
	ROM_LOAD32_WORD("1j016211.u85", 0x0800000, 0x0400000, CRC(0d851d28) SHA1(1e956c39a9312913227d4fd5a22e4fb3d8dccd3d) )
	ROM_LOAD32_WORD("1j016211.u72", 0x0800002, 0x0400000, CRC(e0984ee9) SHA1(db8c55b2a6a6df7fe840c7fa5ecde1830cdd1e75) )
	ROM_LOAD32_WORD("1j016211.u84", 0x1000000, 0x0400000, CRC(7691b681) SHA1(de8a97575eac52b70034055bd2d938c606798dc7) )
	ROM_LOAD32_WORD("1j016211.u71", 0x1000002, 0x0400000, CRC(bbb2ef37) SHA1(977f558816188b86012c61adeddfad9077885e37) )
ROM_END


// 10166211 / 717/3
// 15 Lions / QCOM / B - 23/03/04
// Variations (% and NO):
//    87.67% 99
ROM_START( 15lions )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10166211.u86", 0x0000000, 0x0400000, CRC(89107b2c) SHA1(34f9b51c62b37f2c3d43e9c50128028d3a4ce4a4) )
	ROM_LOAD32_WORD("10166211.u73", 0x0000002, 0x0400000, CRC(ecdc158f) SHA1(0e93f3a6084eaf4934cd131cd3bc24030eca1f83) )
ROM_END


// 10120511 / 717
// 50 Lions / Local / A - 11/10/02
// Variations (% and NO):
//    87.58% 99
//    90.03% 01
//    92.11% 02
ROM_START( 50lions )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10120511.u86", 0x0000000, 0x0400000, CRC(0e5c86f1) SHA1(84e329e664ace697f9ea4ace08612089e0964732) )
	ROM_LOAD32_WORD("10120511.u73", 0x0000002, 0x0400000, CRC(1c1f2297) SHA1(13fb8c83d8ce2340ef554490c21a38da7b47c666) )
ROM_END


// 10156111 / 717
// 50 Lions / ASP / D - 20/10/03
// Variations (% and NO):
//    87.58% 99
ROM_START( 50lionsa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10156111.u86", 0x0000000, 0x0400000, CRC(c3791531) SHA1(b9c60be9624463eb591f2baf421ff90b8763449b) )
	ROM_LOAD32_WORD("10156111.u73", 0x0000002, 0x0400000, CRC(ec1b699b) SHA1(5a6ad7c7eb02443e42ee6a88525ae95a2b0a3195) )
ROM_END


// 0152077 / US128
// Fifty Lions PLG / Local / C - 11/09/05
// '50 Lions' on artwork, 'Fifty Lions' in ROM
// Pele's Legendary Goals Hyperlink game? Doesn't seem to have the PLG program code/assets aside from the 82% option
// Variations (% and NO):
//    87.581% 99
//    90.033% 01
//    92.110% 02
//    94.711% 03
//    97.031% 04
//    82.008% 05 (Hyperlink)
ROM_START( 50lionsu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152077.u86", 0x0000000, 0x0400000, CRC(28b9c1d7) SHA1(10ea4db0d563e6f6431afa83fb79053cfb8ec8ac) )
	ROM_LOAD32_WORD("0152077.u73", 0x0000002, 0x0400000, CRC(f1b2c769) SHA1(0ae594093fc475c98fe7b512f839d1f59ebc778b) )
ROM_END


// 20301311 / 1049
// 50 Lions - Roaring Riches / B - 20/08/12
// Variations (% and NO):
//    85.03% 05
//    87.57% 06
ROM_START( 50lionrr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20301311.u86", 0x0000000, 0x0400000, CRC(dacbe4ec) SHA1(af6118c1843f675155aa1b8d50f605ac60168370) )
	ROM_LOAD32_WORD("20301311.u73", 0x0000002, 0x0400000, CRC(eee56944) SHA1(b7eb97f7a59076997195edfe8676edbb712a6376) )
	ROM_LOAD32_WORD("20301311.u85", 0x0800000, 0x0400000, CRC(809c54c0) SHA1(8a45601248c450f323e39a7397387cf0caf8beed) )
	ROM_LOAD32_WORD("20301311.u72", 0x0800002, 0x0400000, CRC(73c2795c) SHA1(bce64fedf84ab5374b07487f6b887c06abe1213a) )
	ROM_LOAD32_WORD("20301311.u84", 0x1000000, 0x0400000, CRC(c6bd1e46) SHA1(2cf52574ce3f3726c723fc6fbf098c3177c46a37) )
	ROM_LOAD32_WORD("20301311.u71", 0x1000002, 0x0400000, CRC(412524b3) SHA1(ef724755616aad7fd484b5d5fcb6f6773e92b9b9) )
ROM_END


// 10219211 / 717/4
// 100 Lions / Local / B - 13/06/06
// Variations (% and NO):
//    87.90% 99
//    90.07% 01
//    91.99% 02
ROM_START( 100lions )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10219211.u86", 0x0000000, 0x0400000, CRC(a1c71dd2) SHA1(9a859df876cf6a2fadcc5ae7183021881dc08887) )
	ROM_LOAD32_WORD("10219211.u73", 0x0000002, 0x0400000, CRC(da7d2ed7) SHA1(1f81cad150c013848988e6f995a45f7ea5c6d95c) )
ROM_END


// 30223811 / 717/5
// 100 Lions / ASP / A - 03/07/06
// ROM has unchanged 10219211 ID (NSW part number) in header with "302238" further down
// Variations (% and NO):
//    87.90% 99
ROM_START( 100lionsa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30223811.u86", 0x0000000, 0x0400000, CRC(735285e1) SHA1(964dd5ceeb6604620bc1293559d51b2abd9afd87) )
	ROM_LOAD32_WORD("30223811.u73", 0x0000002, 0x0400000, CRC(8e34e360) SHA1(84c287973a61f4ca39b9c367a6b547d4e8210e4e) )
ROM_END


// 0151178 / US168
// Adonis / USA / A - 05/26/05
// Variations (% and NO):
//    87.928% 99
//    90.511% 01
//    92.161% 02
//    94.941% 03
//    97.325% 04
ROM_START( adonisa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151178-U86 Series:01 M27V322 Mk6USA   Adonis   L05-2380-01   36B879EF   1F64-FB99 / 1DDCF6CB / 0442AEB8   Copyright © 1998-2005 Aristocrat Technologies, Inc.
	0151178-U73 Series:01 M27V322 Mk6USA   Adonis   L05-2380-02   C59A5623   FDAB-4D8F / 1D7BA8E0 / F497D9AD   Copyright © 1998-2005 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151178.u86", 0x0000000, 0x0400000, CRC(0eeb0ac3) SHA1(72b2b4711eb9d962bfb58fac1a5226ac81db67ea) )
	ROM_LOAD32_WORD("0151178.u73", 0x0000002, 0x0400000, CRC(eba9e84f) SHA1(bf746ea356627b1a558b4c8f8a2d8d5c54400a4a) )
ROM_END


// 20006711 / 665/1
// AFRICAN DUSK (MR. CASHMAN) / Local / B- 13/07/01
// Variations (% and NO):
//    87.84% 99
ROM_START( aduskcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20006711.u86", 0x0000000, 0x0400000, CRC(26759ebc) SHA1(1205d74c39d13333ccc0ff4deb287ce12d6dbbf5) )
	ROM_LOAD32_WORD("20006711.u73", 0x0000002, 0x0400000, CRC(b156f918) SHA1(004ce75da1ba9b95c5d689a568d8e6502b7d00ab) )
	ROM_LOAD32_WORD("20006711.u85", 0x0800000, 0x0400000, CRC(f62830be) SHA1(13fbadf58ac2f3c15f78a40b3ac45e20d1080937) )
	ROM_LOAD32_WORD("20006711.u72", 0x0800002, 0x0400000, CRC(d963cc96) SHA1(8a855269d0ea92e1f58ceda555bb124746ae1268) )
ROM_END


// 0251071 / US051
// After Hours / USA / B - 01/20/04
// Variations (% and NO):
//    87.493% 99 (9 line)    87.645% 99 (20 line)
//    90.663% 01 (9 line)    90.640% 01 (20 line)
//    92.625% 02 (9 line)    92.548% 02 (20 line)
//    95.538% 03 (9 line)    95.630% 03 (20 line)
//    97.469% 04 (9 line)    97.494% 04 (20 line)
ROM_START( afterhrs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251071.u86", 0x0000000, 0x0400000, CRC(099f7d16) SHA1(f4ccec1d3c194963a13894b41d35afa5a9191b75) )
	ROM_LOAD32_WORD("0251071.u73", 0x0000002, 0x0400000, CRC(972c569b) SHA1(caf4aaeaf05a08e4751a8bf06378d6f64b26883d) )
	ROM_LOAD32_WORD("0251071.u85", 0x0800000, 0x0400000, CRC(de79acdc) SHA1(d7d3d415f5261ab2b65ef1a1e93ccd3770124848) )
	ROM_LOAD32_WORD("0251071.u72", 0x0800002, 0x0400000, CRC(a4bee8ab) SHA1(cac34f91459fa5902b85e4c8150e651fda68621e) )
ROM_END


// 1J009611 / JB013/2
// Alchemy / NSW/ACT / A - 30/05/03
// Variations (% and NO):
//    87.99% 99
//    90.10% 01
//    92.18% 02
ROM_START( alchemy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009611.u86", 0x0000000, 0x0400000, CRC(7157c70b) SHA1(1740fd0e9a7dd72390146bfa4e2c4ef3b7d7ec0d) )
	ROM_LOAD32_WORD("1j009611.u73", 0x0000002, 0x0400000, CRC(bfac2656) SHA1(337f952fae2bff960c78f8808d3dc04815e74404) )
	ROM_LOAD32_WORD("1j009611.u85", 0x0800000, 0x0400000, CRC(a0a3453b) SHA1(f5f5559bbf8fdf176ca4f0ad755259120d29930c) )
	ROM_LOAD32_WORD("1j009611.u72", 0x0800002, 0x0400000, CRC(11321ce3) SHA1(83c6e7618b29fa7ea8c01e27be0b6e4d85822299) )
ROM_END


// 0152152 / US209
// All The King's Gold / Local / A - 01/08/07
// Variations (% and NO):
//    87.938% 99
//    90.088% 01
//    92.194% 02
//    94.961% 03
//    97.015% 04
ROM_START( atkgld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152152-U86 Series 01 M27v322 MK6USA   ALL THE KINGS GOLD   BP00AFAC   B329-B00D-/ 1E63786A / 3F861679   Copyright © 2001-2007 Aristocrat Technologies Australia Pty Ltd
	0152152-U73 Series 01 M27v322 MK6USA   ALL THE KINGS GOLD   8860DCF6   0622-ECC9 / 1E09FBAB / 2D898187   Copyright © 2001-2007 Aristocrat Technologies Australia Pty Ltd
	0152152-U85 Series 01 M27v322 MK6USA   ALL THE KINGS GOLD   17D91490   540B-6C9F / 003E11CB / 1DEF2CE1   Copyright © 2001-2007 Aristocrat Technologies Australia Pty Ltd
	0152152-U72 Series 01 M27v322 MK6USA   ALL THE KINGS GOLD   EC2CF383   952C-3725 / 00378188 / 1ABC1654   Copyright © 2001-2007 Aristocrat Technologies Australia Pty Ltd
	*/
	ROM_LOAD32_WORD("0152152.u86", 0x0000000, 0x0400000, CRC(dfd90fc4) SHA1(8881ae3102d87435585d04abd791f3f710cf5ed2) )
	ROM_LOAD32_WORD("0152152.u73", 0x0000002, 0x0400000, CRC(01d8e945) SHA1(ed0568a599fe514a8dcba762835831388fede679) )
	ROM_LOAD32_WORD("0152152.u85", 0x0800000, 0x0400000, CRC(81f8c412) SHA1(15ace6c1fc36cb7004e7767aea5e30fe8b9acf8b) )
	ROM_LOAD32_WORD("0152152.u72", 0x0800002, 0x0400000, CRC(0a360417) SHA1(f5224b6c03fe8cf0a341630086906c12a58321ed) )
ROM_END


// 1005511 / 669
// Amazon Queen / Local / A - 02/05/01
// Variations (% and NO):
//    87.51% 99
//    89.98% 01
//    92.03% 02
ROM_START( amzqueen )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10005511.u86", 0x0000000, 0x0400000, CRC(2d5d9be4) SHA1(4eee74084279ae6e89bab55ef22711836eb11a93) )
	ROM_LOAD32_WORD("10005511.u73", 0x0000002, 0x0400000, CRC(7b64f0b4) SHA1(1af15ac0f41acde21606002add1212b9ac72225c) )
	ROM_LOAD32_WORD("10005511.u85", 0x0800000, 0x0400000, CRC(484d8848) SHA1(b86351fd828507968b44fd92c0f9b7ad51bfb9a8) )
	ROM_LOAD32_WORD("10005511.u72", 0x0800002, 0x0400000, CRC(1e039e60) SHA1(4f6c5db24c2841daa9b29af2732335498f8b8f0d) )
ROM_END


// 10299811 / 906
// Amazon Temple / A - 29/04/09
// Variations (% and NO):
//    87.83% 99
//    90.09% 01
//    91.89% 02
//    88.64% 07
ROM_START( amztempl )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10299811.u86", 0x0000000, 0x0400000, CRC(c001c1e7) SHA1(bf8958325f90d2a04a6c86075df664b29b7073ca) )
	ROM_LOAD32_WORD("10299811.u73", 0x0000002, 0x0400000, CRC(b11fb5a0) SHA1(ebd78bcf1a06878c8a50fe078379f590ec6bc3e2) )
	ROM_LOAD32_WORD("10299811.u85", 0x0800000, 0x0400000, CRC(887449d7) SHA1(a1b00d8205f9eb45ad2b3ec73422bc06673d68e0) )
	ROM_LOAD32_WORD("10299811.u72", 0x0800002, 0x0400000, CRC(ca0b23ac) SHA1(4514e691f5926608a1624a91751f50469e234ed4) )
	ROM_LOAD32_WORD("10299811.u84", 0x1000000, 0x0400000, CRC(aa689460) SHA1(111130f65ad4bb235e8722befb41b947d7d47c4e) )
	ROM_LOAD32_WORD("10299811.u71", 0x1000002, 0x0400000, CRC(8b6ef913) SHA1(7cc8688cb9c6096a81ad21cdc046db5bd0134356) )
ROM_END


// 10163211 / 754
// Antony and Cleopatra / Local / A - 15/01/04
// Variations (% and NO):
//    87.93% 99
//    90.12% 01
//    91.93% 02
ROM_START( antcleo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10163211.u86", 0x0000000, 0x0400000, BAD_DUMP CRC(fc76d567) SHA1(80afc0edf22c592d6e98fbf9f094672adc1bf6f8) )
	ROM_LOAD32_WORD("10163211.u73", 0x0000002, 0x0400000, CRC(34c2b036) SHA1(ee67eafd51aa2c3975ea45ffa629e80621dd01f6) )
	ROM_LOAD32_WORD("10163211.u85", 0x0800000, 0x0400000, CRC(f5995ccf) SHA1(54a3e50171667f81d1fa21b89622c05a9d869b3e) )
	ROM_LOAD32_WORD("10163211.u72", 0x0800002, 0x0400000, CRC(f8219493) SHA1(9d954800985e2561ea1b01dd49f443583b98e5ec) )
ROM_END


// 10177211 / 754
// Antony and Cleopatra / ASP / B - 12/07/04
// Variations (% and NO):
//    87.93% 99
ROM_START( antcleoa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177211.u86", 0x0000000, 0x0400000, CRC(4897f4ed) SHA1(0a071528b0c2cb4c42d4535bed406849a6187d9d) )
	ROM_LOAD32_WORD("10177211.u73", 0x0000002, 0x0400000, CRC(41b7d75d) SHA1(5c25e0bc65560b17b80c4430ae9d925a0f245e6c) )
	ROM_LOAD32_WORD("10177211.u85", 0x0800000, 0x0400000, CRC(909a5a6c) SHA1(abb86f82184f32fad578d5c3a6d034afaa78e3c3) )
	ROM_LOAD32_WORD("10177211.u72", 0x0800002, 0x0400000, CRC(121751b2) SHA1(caf5d7c2d3774575ef0a12146cbdd72d0a421c58) )
ROM_END


// 0151105 / US083
// Apollo Coffee / USA / A - 11/10/03
// 'Apollo Coffee Company' on artwork, 'Apollo Coffee' in ROM
// Variations (% and NO):
//    87.500% 99
//    90.202% 01
//    92.604% 02
//    94.712% 03
//    97.414% 04
ROM_START( apocof )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151105-U86 Series:01 M27V322 Mk6USA   Apollo Coffee   E9D0E942   C6CA-7785 / 1E8C95B2 / 64CA9321   Copyright © 2003 Aristocrat Technologies, Inc.
	0151105-U73 Series:01 M27V322 Mk6USA   Apollo Coffee   345P0U49   9B3A-FC7F / 1E6FB09E / 5FB1A319   Copyright © 2003 Aristocrat Technologies, Inc.
	0151105-U85 Series:01 M27V322 Mk6USA   Apollo Coffee   5U0C9346   8AE9-7461 / 1FD1AC34 / D1358929   Copyright © 2003 Aristocrat Technologies, Inc.
	0151105-U72 Series:01 M27V322 Mk6USA   Apollo Coffee   72299F3U   41DC-471B / 0F804101 / B6F374AB   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151105.u86", 0x0000000, 0x0400000, CRC(d1f0a2a6) SHA1(d88fd4153b9a5f655c7949883caf3f947a50652f) )
	ROM_LOAD32_WORD("0151105.u73", 0x0000002, 0x0400000, CRC(675ee721) SHA1(3f4afe6196fc39c2544fc3731f39adef0198b9a3) )
	ROM_LOAD32_WORD("0151105.u85", 0x0800000, 0x0400000, CRC(daa8cc4f) SHA1(2a71c7dbd0691654b614fcd09dae4ff47a60b655) )
	ROM_LOAD32_WORD("0151105.u72", 0x0800002, 0x01c0000, BAD_DUMP CRC(f3733388) SHA1(cf01799770b785a0f7527aedf627f16642a60ff3) )
ROM_END


// 2J008211 / JB009/1
// Arabian Nights / NSW/ACT / E - 10/12/01
// Variations (% and NO):
//    87.99% 99
//    90.04% 01
//    92.17% 02
ROM_START( arabnga6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("2j008211.u86", 0x0000000, 0x0400000, CRC(45cf08c3) SHA1(a963a32b3bca5dec17a6cea4c0e5c66f14b42f65) )
	ROM_LOAD32_WORD("2j008211.u73", 0x0000002, 0x0400000, CRC(2d2629f0) SHA1(f7af60e390394b12a82a3125a9b931046a5e1fef) )
	ROM_LOAD32_WORD("2j008211.u85", 0x0800000, 0x0400000, CRC(03abddc6) SHA1(cc0a8c78d52b26977274455be8dadb21ee4d3580) )
	ROM_LOAD32_WORD("2j008211.u72", 0x0800002, 0x0400000, CRC(709de514) SHA1(3f380ef548c45fa8eaba9cba2bbabdcb59c7660b) )
ROM_END


// 10122611 / JB009/1
// Arabian Nights / ASP / E - 10/12/01
// Same date string as parent
// Variations (% and NO):
//    87.99% 99
ROM_START( arabnga6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122611.u86", 0x0000000, 0x0400000, CRC(b292b1b3) SHA1(230d2591aa07555dc4dfda00e0a25ba5f9b91272) )
	ROM_LOAD32_WORD("10122611.u73", 0x0000002, 0x0400000, CRC(b6971377) SHA1(0a431921ceb71f4ffa2179d293e50c9ea78479a0) )
	ROM_LOAD32_WORD("10122611.u85", 0x0800000, 0x0400000, CRC(cec8dbec) SHA1(7a6403ab66bad3cb3a28251114b2d2d041cd7dc0) )
	ROM_LOAD32_WORD("10122611.u72", 0x0800002, 0x0400000, CRC(42cba9b9) SHA1(a6ca20b130b3deec080e0efbd198412d4ff8a7cf) )
ROM_END


// 0151356 / JB016/21
// Arabian Nights / Local / E - 07/06/04
// Variations (% and NO):
//    87.938% 99
//    90.088% 01
//    92.194% 02
//    94.961% 03
//    97.015% 04
ROM_START( arabnga6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151356-U86 Series:01 M27V322 Mk6USA   Arabian Nights   L04-2023-01   C449D97F   B5AB-4BAB / 1E5CC3CF / 50330576   Copyright © 2004 Aristocrat Technologies, Inc.
	0151356-U73 Series:01 M27V322 Mk6USA   Arabian Nights   L04-2023-02   371A609U   E0EC-4045 / 1DF4D3CE / 353EC76A   Copyright © 2004 Aristocrat Technologies, Inc.
	0151356-U85 Series:01 M27V322 Mk6USA   Arabian Nights   L04-2023-03   CUE7246A   A337-EABB / 06AD8CC5 / 5512514A   Copyright © 2004 Aristocrat Technologies, Inc.
	0151356-U72 Series:01 M27V322 Mk6USA   Arabian Nights   L04-2023-04   5A35548H   D3FB-EB5E / 069EA983 / 52745BAF   Copyright © 2004 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151356.u86", 0x0000000, 0x0400000, CRC(ae1d3cd5) SHA1(e2cca9d9f3b205446e0f74292a784cfe65302bc8) )
	ROM_LOAD32_WORD("0151356.u73", 0x0000002, 0x0400000, CRC(a7835a67) SHA1(d7e2d60edeb553165b8be1b272c0d551e762ab33) )
	ROM_LOAD32_WORD("0151356.u85", 0x0800000, 0x0400000, CRC(4063edd6) SHA1(bb8f8230bbad70af683d039c041e90d50a59ceed) )
	ROM_LOAD32_WORD("0151356.u72", 0x0800002, 0x0400000, CRC(214632ed) SHA1(31cb4cbecf9b938df5b93c1260feeb6265bb2152) )
ROM_END


// 0151065 / US029
// ARCTIC WINS CASHMAN / Local / E- 01/27/03
// Variations (% and NO):
//    92.378% 99
ROM_START( arwincm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151065-U86 Series: 01 M27V322 Mk6USA   Arctic Wins- Mr. Cashman   L03-0510-01   4U16F54H   3EED-529C / 1D6D96C5 / N/A   Copyright © 2002 Aristocrat Technologies, Inc
	0151065-U73 Series: 01 M27V322 Mk6USA   Arctic Wins- Mr. Cashman   L03-0510-02   134PECC2   A559-0C27 / 1D40BB15 / N/A   Copyright © 2002 Aristocrat Technologies, Inc
	0151065-U85 Series: 01 M27V322 Mk6USA   Arctic Wins- Mr. Cashman   L03-0510-03   5PA3BU44   9AFB-291C / 1B39BF80 / N/A   Copyright © 2002 Aristocrat Technologies, Inc
	0151065-U72 Series: 01 M27V322 Mk6USA   Arctic Wins- Mr. Cashman   L03-0510-04   81AA1F64   56A3-8C58 / 1AFB0341 / N/A   Copyright © 2002 Aristocrat Technologies, Inc
	*/
	ROM_LOAD32_WORD("0151065.u86", 0x0000000, 0x0400000, CRC(961cca3d) SHA1(f5bd3fdb8863688a1bf96dcd74a261860a008952) )
	ROM_LOAD32_WORD("0151065.u73", 0x0000002, 0x0400000, CRC(0c3e441c) SHA1(763879bae2b256c2c4edddc99507b3c54cbab976) )
	ROM_LOAD32_WORD("0151065.u85", 0x0800000, 0x0400000, CRC(498b2ab7) SHA1(7f1da5ff34572c1f27f9a0735b8a9571b34ac17e) )
	ROM_LOAD32_WORD("0151065.u72", 0x0800002, 0x0400000, CRC(3a133d96) SHA1(4c7dc422c3e0a81da1f81267d525e3a90a7d79d7) )
ROM_END


// 0151087 / US057
// Arishinko / Local / E - 05/28/03
// Variations (% and NO):
//    87.455% 99
//    90.085% 01
//    92.507% 02
//    95.433% 03
ROM_START( arish )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151087.u86", 0x0000000, 0x0400000, CRC(ff9cf101) SHA1(1ae93eb26d2033b55ea5a28470a7c76183916ef4) )
	ROM_LOAD32_WORD("0151087.u73", 0x0000002, 0x0400000, CRC(631b26e7) SHA1(2520bce139f0bc57b70b93baf253b4ef2a52d531) )
	ROM_LOAD32_WORD("0151087.u85", 0x0800000, 0x0400000, CRC(0ea3b484) SHA1(7c9ab129a1c49bcb36f264ce30071e5b9fbb8985) )
	ROM_LOAD32_WORD("0151087.u72", 0x0800002, 0x0400000, CRC(08451014) SHA1(3dcda83503c361c93df27a0cd5edd4d68d2a735b) )
ROM_END


// 0352502 / MV4208
// Asian Princess / Local / C - 09/13/04
// Variations (% and NO):
//    87.536% 99
//    90.007% 01
//    92.000% 02
//    94.797% 03
ROM_START( asnprinc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352502.u86", 0x0000000, 0x0400000, CRC(d0741b53) SHA1(29ede83b3c95d2e490dac66f60629238abc48bdd) )
	ROM_LOAD32_WORD("0352502.u73", 0x0000002, 0x0400000, CRC(b05400e0) SHA1(b0832ccfb1947738125f4afe557a246fffdd983c) )
	ROM_LOAD32_WORD("0352502.u85", 0x0800000, 0x0400000, CRC(c46cfe80) SHA1(294ddd29d5c13388096dc2551dc474f077e5d881) )
	ROM_LOAD32_WORD("0352502.u72", 0x0800002, 0x0400000, CRC(917421ef) SHA1(87eb28c98e1901830cb2b779f5c4d569699d1b1e) )
ROM_END


// 20255111 / 850
// Beaver Bucks / C - 23/01/08
// Variations (% and NO):
//    87.74% 99
//    90.03% 01
ROM_START( beavbuk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20255111.u86", 0x0000000, 0x0400000, CRC(2de9d8e5) SHA1(aea9f2668effa60a627cf62cee45919aa3b669b0) )
	ROM_LOAD32_WORD("20255111.u73", 0x0000002, 0x0400000, CRC(89c840dd) SHA1(31ab84740d866202cc5dfec17dec677186b991c5) )
ROM_END


// 0251064 / US037
// BETTING ZOO CM / Local / B- 1/28/03
// Variations (% and NO):
//    92.411% 99
ROM_START( betzoo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251064.u86", 0x0000000, 0x0400000, CRC(4cb680ba) SHA1(5c7e516203a2c91bb25f2a070c6a778035b1f35f) )
	ROM_LOAD32_WORD("0251064.u73", 0x0000002, 0x0400000, CRC(a9439adf) SHA1(60f2a5be0629377dd4a4adfa4ef6e283d5c60099) )
	ROM_LOAD32_WORD("0251064.u85", 0x0800000, 0x0400000, CRC(81b70c50) SHA1(145e7ce40c24635959261d6e2450cffa36f73869) )
	ROM_LOAD32_WORD("0251064.u72", 0x0800002, 0x0400000, CRC(98444462) SHA1(c633448d5d7739f609eb00c61d89c997db3626d2) )
ROM_END


// 30126911 / 710
// Big Ben / Local / B - 17/07/03
// Variations (% and NO):
//    87.99% 99
//    90.09% 01
//    91.90% 02
ROM_START( bigben )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30126911.u86", 0x0000000, 0x0400000, CRC(21a31a76) SHA1(2290c502c492064967cf67f545351ff38ed6a1be) )
	ROM_LOAD32_WORD("30126911.u73", 0x0000002, 0x0400000, CRC(5e5d02f3) SHA1(5a8d7403186d3b10ef3e92297ca4b484c902095a) )
	ROM_LOAD32_WORD("30126911.u85", 0x0800000, 0x0400000, CRC(bc7846d4) SHA1(f754ca71c821f65a27c631f91ce254c76152ffde) )
	ROM_LOAD32_WORD("30126911.u72", 0x0800002, 0x0400000, CRC(b5b0fb0e) SHA1(a5f233c1472d7b27a85e2b116ccac8ef61d23e6c) )
ROM_END


// 20126911 / 710
// Big Ben / Local / B - 17/07/03
// Same date string as parent
// Early version, has 'Underground' symbol instead of 'Light'
// Variations (% and NO):
//    87.99% 99
//    90.09% 01
//    91.90% 02
ROM_START( bigbena )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20126911.u86", 0x0000000, 0x0400000, CRC(68c53dd6) SHA1(2c6c0a2a507b47da7675b4b62cf584b325427b26) )
	ROM_LOAD32_WORD("20126911.u73", 0x0000002, 0x0400000, CRC(5e43305f) SHA1(8e8f736510d45736068f101076d15900ba6aa1cd) )
	ROM_LOAD32_WORD("20126911.u85", 0x0800000, 0x0400000, CRC(64bb4204) SHA1(e20af2dba4c2feab07ba3344669964da79e6b9d1) )
	ROM_LOAD32_WORD("20126911.u72", 0x0800002, 0x0400000, CRC(d59c8c69) SHA1(b37a3d3061e6209a93e0a5de4a59ea073eedbbb7) )
ROM_END


// 10169611 / MV4213
// Big Ben / ASP / A - 01/04/04
// Variations (% and NO):
//    87.99% 99
ROM_START( bigbenb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10169611.u86", 0x0000000, 0x0400000, CRC(e1f29a35) SHA1(b77339672d385f84ef16fe1c33ae74a8c285d48e) )
	ROM_LOAD32_WORD("10169611.u73", 0x0000002, 0x0400000, CRC(6c8aebc2) SHA1(32116ac3cc3a7c6f1903658c4ee788093f715fae) )
	ROM_LOAD32_WORD("10169611.u85", 0x0800000, 0x0400000, CRC(ef9e5e55) SHA1(9d7fea168c2aad493a81f947fdab3b009afcb1e1) )
	ROM_LOAD32_WORD("10169611.u72", 0x0800002, 0x0400000, CRC(96b07586) SHA1(5f813a55b57a58d36a31e11bf872db99c79ab914) )
ROM_END


// 20201111 / 710/3
// Big Ben / QCOM / C - 09/08/05
// Variations (% and NO):
//    87.91% 99
ROM_START( bigbenq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20201111.u86", 0x0000000, 0x0400000, CRC(c27488bc) SHA1(b4d6e89b093fe850160baef067df9f261723c02e) )
	ROM_LOAD32_WORD("20201111.u73", 0x0000002, 0x0400000, CRC(d0429338) SHA1(c9e31a874603d7397b4720a3d88030671f0e8d27) )
	ROM_LOAD32_WORD("20201111.u85", 0x0800000, 0x0400000, CRC(8105b054) SHA1(b346ab044f311909803512e014a1af1bc8ada684) )
	ROM_LOAD32_WORD("20201111.u72", 0x0800002, 0x0400000, CRC(63f90aad) SHA1(2bbec33552cd3c134c4f66c49181a4b875d9daf0) )
ROM_END


// 10000511 / 637
// Big Hearted / Local / D - 15/11/99
// Variations (% and NO):
//    87.70% 99
//    90.09% 01
//    92.08% 02
ROM_START( bigheart )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000511.u86", 0x0000000, 0x0400000, CRC(b63fbc96) SHA1(f0a76f52441afe41e9bb11da1526a17d1ed9df6c) )
	ROM_LOAD32_WORD("10000511.u73", 0x0000002, 0x0400000, CRC(920ffac3) SHA1(be34f582712da5be84834562c758997829302c24) )
ROM_END


// 2J009211 / JB015
// Big Red / NSW/ACT / D - 27/09/02
// Variations (% and NO):
//    90.64% 99 (3 line)    90.65% 99 (5 line)
//    92.44% 01 (3 line)    92.38% 01 (5 line)
//    93.54% 02 (3 line)    93.56% 02 (5 line)
//    94.95% 03 (3 line)    94.94% 03 (5 line)
//    87.92% 05 (3 line)    87.96% 05 (5 line)
ROM_START( bigred )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("2j009211.u86", 0x0000000, 0x0400000, CRC(da16ed45) SHA1(febb8f960009361250ab78814d60d008b3778a57) )
	ROM_LOAD32_WORD("2j009211.u73", 0x0000002, 0x0400000, CRC(60e8fd7e) SHA1(7294a180d3ee869a545f43c2c898130f393d076a) )
	ROM_LOAD32_WORD("2j009211.u85", 0x0800000, 0x0400000, CRC(8c2f6fc9) SHA1(9dd687b242a3d0348cc20a7a20fef149f5f7834a) )
	ROM_LOAD32_WORD("2j009211.u72", 0x0800002, 0x0400000, CRC(bab7ae56) SHA1(d2de7e8544eabe1f116cc878f4ad3ccf22d4ca38) )
ROM_END


// 1J009211 / JB015
// Big Red / NSW/ACT / D - 27/09/02
// Same date string as parent
// Variations (% and NO):
//    90.64% 99 (3 line)    90.65% 99 (5 line)
//    92.44% 01 (3 line)    92.38% 01 (5 line)
//    93.54% 02 (3 line)    93.56% 02 (5 line)
//    94.95% 03 (3 line)    94.94% 03 (5 line)
//    87.92% 05 (3 line)    87.96% 05 (5 line)
ROM_START( bigreda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009211.u86", 0x0000000, 0x0400000, CRC(e4740e2e) SHA1(e30cde7b39267b43fe21082441693df11736f63a) )
	ROM_LOAD32_WORD("1j009211.u73", 0x0000002, 0x0400000, CRC(9fd29e7f) SHA1(b1cac9ae3e0d226498346f099798bf3c0b78e908) )
	ROM_LOAD32_WORD("1j009211.u85", 0x0800000, 0x0400000, CRC(8c2f6fc9) SHA1(9dd687b242a3d0348cc20a7a20fef149f5f7834a) ) // same as parent but different label
	ROM_LOAD32_WORD("1j009211.u72", 0x0800002, 0x0400000, CRC(bab7ae56) SHA1(d2de7e8544eabe1f116cc878f4ad3ccf22d4ca38) ) // same as parent but different label
ROM_END


// 5J011311 / JB024
// Big Red II / NSW/ACT / B - 18/05/05
// 'Big Red 2' on artwork, 'Big Red II' in ROM
// Variations (% and NO):
//    90.67% 99
//    92.36% 01
//    93.66% 02
//    94.97% 03
//    88.00% 05
ROM_START( bigred2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("5j011311.u86", 0x0000000, 0x0400000, CRC(6e68c586) SHA1(f6261e92e9c22fbd59d385451d5b212bb888425b) )
	ROM_LOAD32_WORD("5j011311.u73", 0x0000002, 0x0400000, CRC(8826a403) SHA1(d0e08205c0fdc7229ca4d9a895a8456d8f07ff26) )
	ROM_LOAD32_WORD("5j011311.u85", 0x0800000, 0x0400000, CRC(e05a45db) SHA1(4fff979915952addc3251adb4385f0072b2033cd) )
	ROM_LOAD32_WORD("5j011311.u72", 0x0800002, 0x0400000, CRC(d28be7d5) SHA1(67820d280cb016ecdd8dd35cd0f89f51ab4d4cf1) )
ROM_END


// 1J009111 / JB024
// Black Brumby / NSW/ACT / C - 14/06/02
// Variations (% and NO):
//    88.00% 99 (10 line)    88.00% 99 (25 line)
//    90.16% 01 (10 line)    90.11% 01 (25 line)
//    92.13% 02 (10 line)    92.11% 01 (25 line)
ROM_START( blkbrum )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009111.u86", 0x0000000, 0x0400000, CRC(5a1600be) SHA1(1fcdbc434f872c538997a604b70433d987e2d1ca) )
	ROM_LOAD32_WORD("1j009111.u73", 0x0000002, 0x0400000, CRC(0ad23541) SHA1(92ddaa68e212165bd2828360b9fdeafd9c821335) )
	ROM_LOAD32_WORD("1j009111.u85", 0x0800000, 0x0400000, CRC(5f29ac14) SHA1(7fae7adf85b90375732767fb9cd0e88097a61399) )
	ROM_LOAD32_WORD("1j009111.u72", 0x0800002, 0x0400000, CRC(95673ea4) SHA1(17ea6380473796042a44c42afda79ef270282892) )
ROM_END


// 0152584 / JB024/21
// Black Mustang / Local / N/A
// Variations (% and NO):
//    87.982% 99
//    90.178% 01
//    92.197% 02
//    94.967% 03
ROM_START( blkmust )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152584.u86", 0x0000000, 0x0400000, CRC(2637a658) SHA1(7ec96c15f0fef5862f3d4a37fe96ab893e986321) )
	ROM_LOAD32_WORD("0152584.u73", 0x0000002, 0x0400000, CRC(986b58d9) SHA1(178de2a1bfaca3e859e9c948ce0626c5f31b303f) )
	ROM_LOAD32_WORD("0152584.u85", 0x0800000, 0x0400000, CRC(6a089a8b) SHA1(20d8fd974cedef0a7f642dbfea9b2fe088e26de0) )
	ROM_LOAD32_WORD("0152584.u72", 0x0800002, 0x0400000, CRC(032bf21e) SHA1(65ac1f37dbee5d965e6499d7f8a2ace27856d233) )
ROM_END


// 20259511 / 855
// Blue Tiger / A - 12/12/07
// Variations (% and NO):
//    87.91% 99
//    90.05% 01
ROM_START( bluetigr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20259511.u86", 0x0000000, 0x0400000, CRC(a31f97d8) SHA1(67ec087cbb8f733c0f0314b58b4884fe3cc3ad8c) )
	ROM_LOAD32_WORD("20259511.u73", 0x0000002, 0x0400000, CRC(bc2a4fb8) SHA1(74ba00873a4a00d0839d89bb2d9dbed5fc33bff7) )
	ROM_LOAD32_WORD("20259511.u85", 0x0800000, 0x0400000, CRC(e175ef6d) SHA1(13933044f905c0fde96ff05c6afdce189160a5e2) )
	ROM_LOAD32_WORD("20259511.u72", 0x0800002, 0x0400000, CRC(f81af522) SHA1(5b7be10820f18ffa360726d20370faa1ecc1b3f8) )
ROM_END


// 0352298 / US 243
// Bob and Dolly / Local / A - 10/15/07
// US241 / A - 09/18/07 (alternate strings)
// Variations (% and NO):
//    82.559% 05 (Reel 1)    82.559% 05 (Reels 1-2)    82.663% 05 (Reels 1-3)    82.726% 05 (Reels 1-4)    82.846% 05 (Reels 1-5)
ROM_START( bobdolly )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352298.u11", 0x0000000, 0x0400000, CRC(d3657369) SHA1(621fb20238f10e06c486dfbd6460f94047898a42) )
	ROM_LOAD32_WORD("0352298.u21", 0x0000002, 0x0400000, CRC(8e32da10) SHA1(28a49ecc3bab512e5828c96392d6fa4ff9394a0f) )
	ROM_LOAD32_WORD("0352298.u12", 0x0800000, 0x0400000, CRC(47429081) SHA1(23d996aa522c2f9beadf1fdb7c0584ca939917da) )
	ROM_LOAD32_WORD("0352298.u22", 0x0800002, 0x0400000, CRC(4302ae5e) SHA1(fe1d122093ad9234a7a5337b3a4c2de7fe046822) )
	ROM_LOAD32_WORD("0352298.u13", 0x1000000, 0x0400000, CRC(e73883db) SHA1(6382386ac6f576c739c959cbf6b6df2605496845) )
	ROM_LOAD32_WORD("0352298.u23", 0x1000002, 0x0400000, CRC(a9726ec5) SHA1(cdf78adc3ebcd50e503b2587564493a91b73f578) )
	ROM_LOAD32_WORD("0352298.u14", 0x1800000, 0x0400000, CRC(0f1a6b7c) SHA1(17686829a1a64f6d8982e015067f2d08bcb00402) )
	ROM_LOAD32_WORD("0352298.u24", 0x1800002, 0x0400000, CRC(3646c16d) SHA1(c52fab2e11f39c126bcbbb348750bbf30ff96090) )
ROM_END


// 20180311 / 783/1 (9 line) / 783 (20 line)
// Brazil / Local / B - 25/02/05
// Variations (% and NO):
//    87.68% 99
//    90.00% 99
ROM_START( brazil )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20180311.u86", 0x0000000, 0x0400000, CRC(9be9f72c) SHA1(5e86e42280c6b5b3ce484a04107e84b6aa457bd1) )
	ROM_LOAD32_WORD("20180311.u73", 0x0000002, 0x0400000, CRC(5eb15d8b) SHA1(dc313f96390eb04769c5c9249b140eed67796cd8) )
ROM_END


// 10218511 / 783
// Brazil / ASP / E - 06/04/06
// Alternate string: 783/1 / D - 02/05/06
// Variations (% and NO):
//    87.68% 99
ROM_START( brazila )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10218511.u86", 0x0000000, 0x0400000, CRC(47b2e221) SHA1(6d760c259b78a7ae2477fbd5e0ebf45f4af92bc0) )
	ROM_LOAD32_WORD("10218511.u73", 0x0000002, 0x0400000, CRC(19d51833) SHA1(8c2fd783734c744e42e9db2767156c3408950d39) )
ROM_END


// 10173011 / 765
// Buccaneer / Local / A - 05/04/04
// Variations (% and NO):
//    87.87% 99
//    90.14% 01
ROM_START( bucaneer )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173011.u86", 0x0000000, 0x0400000, CRC(9e1f8099) SHA1(28b1f19be0d6914afb604408e74b2d5260e11624) )
	ROM_LOAD32_WORD("10173011.u73", 0x0000002, 0x0400000, CRC(fa1e540e) SHA1(12d36d59fd176cbfad5e78f913a259fd06150f89) )
ROM_END


// 10181911 / 765/1
// Buccaneer / Local / A - 29/07/04
// Variations (% and NO):
//    87.87% 99
ROM_START( bucaneera )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10181911.u86", 0x0000000, 0x0400000, CRC(19ccfc0a) SHA1(c296a207db55a34f532267e7077643ce80a01bf3) )
	ROM_LOAD32_WORD("10181911.u73", 0x0000002, 0x0400000, CRC(32c854f7) SHA1(5e8ef6f3fbf5054d4781ae19e403a19779e06e36) )
ROM_END


// 0252523 / MV4229
// Buccaneer / Local / B - 01/07/05
// EPROM labels marked 2002 despite being a 2005 game
// Variations (% and NO):
//    87.869% 99
//    90.140% 01
//    92.154% 02
//    94.953% 03
ROM_START( bucaneeru )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0252523-U86 Series:02 M27V322 Mk6USA   Buccaneer   L05-1174-01   A46U9CF8   DB23-9FE8 / 1B3F1CE6 / AB74A38C   Copyright © 2002 Aristocrat Technologies, Inc.
	0252523-U73 Series:02 M27V322 Mk6USA   Buccaneer   L05-1174-02   E329D743   4F58-477A / 1AD75EC3 / 967B5E96   Copyright © 2002 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0252523.u86", 0x0000000, 0x0400000, CRC(c93ecb04) SHA1(4dff453ff4716f0a732e07fbc2816ae4c4234377) )
	ROM_LOAD32_WORD("0252523.u73", 0x0000002, 0x0400000, CRC(c945bf45) SHA1(12156228feecc3363e61a18295edbcd8b995b62e) )
ROM_END


// 10294311 / 897/1
// The Buck Stops Here / A - 22/07/10
// Variations (% and NO):
//    87.87% 99
//    90.09% 01
//    88.70% 07
//    87.09% 09
ROM_START( buckstop )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10294311.u86", 0x0000000, 0x0400000, CRC(b21c6704) SHA1(2181f3491f020400dfa0b79864860fb002ee5a7f) )
	ROM_LOAD32_WORD("10294311.u73", 0x0000002, 0x0400000, CRC(0a55ae9d) SHA1(25cfc1c4ddb5a3c26fd024f0be166763739e84e4) )
	ROM_LOAD32_WORD("10294311.u85", 0x0800000, 0x0400000, CRC(d2a29b6b) SHA1(4bfc7a6ea5d1251f57bbea52b25353f696f7e63e) )
	ROM_LOAD32_WORD("10294311.u72", 0x0800002, 0x0400000, CRC(de8c0100) SHA1(d27668584094c56f2840d87fc481ce03e7278dd8) )
ROM_END


// 10217811 / 806
// Buffalo / Local / E - 06/10/06
// Variations (% and NO):
//    87.75% 99
//    90.10% 01
//    91.90% 02
ROM_START( buffalo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10217811.u86", 0x0000000, 0x0400000, CRC(9fc2bb3e) SHA1(2a0253500b52ebdebb61c0c8cf296ba9cff575da) )
	ROM_LOAD32_WORD("10217811.u73", 0x0000002, 0x0400000, CRC(bcc108b3) SHA1(937c5a3cdace65e706565af46597f46562830c80) )
	ROM_LOAD32_WORD("10217811.u85", 0x0800000, 0x0400000, CRC(fd6a18dc) SHA1(bd5d1a0d6c3c9c99e12234ac3ffca1100edf75fe) )
	ROM_LOAD32_WORD("10217811.u72", 0x0800002, 0x0400000, CRC(6e7b99c9) SHA1(fd1615e41ba3cf42c2853cf0ccdc0f434694b95e) )
ROM_END


// 20232611 / 806
// Buffalo / ASP / E - 06/10/06
// Same date string as parent
// Variations (% and NO):
//    87.75% 99
ROM_START( buffaloa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20232611.u86", 0x0000000, 0x0400000, CRC(14831da1) SHA1(ec9c31e20226ed8304118da63e781d5e332d91fc) )
	ROM_LOAD32_WORD("20232611.u73", 0x0000002, 0x0400000, CRC(0394013c) SHA1(32ff3539b7ddc6c375cbcbd02484b7478c8b3d42) )
	ROM_LOAD32_WORD("20232611.u85", 0x0800000, 0x0400000, CRC(2b457709) SHA1(52c4494f73ee0d1f121de1392b2ad60b0182f695) )
	ROM_LOAD32_WORD("20232611.u72", 0x0800002, 0x0400000, CRC(5bf5f587) SHA1(0089f7f0a0d3980857950b3a4890869f4f26f2d7) )
ROM_END


// 0252636 / MV4328
// Buffalo / Local / N/A
// Variations (% and NO):
//    87.748% 99
//    90.094% 01
//    91.897% 02
//    94.898% 03
ROM_START( buffalou )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0252636-U86 Series 01 M27v322 MK6USA   BUFFALO   0683419H   FED1-6365 / 1D474640 / BEC3EA6F   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd
	0252636-U73 Series 01 M27v322 MK6USA   BUFFALO   71320606   8049-205B / 1CCC026C / 96C434B9   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd
	0252636-U85 Series 01 M27v322 MK6USA   BUFFALO   983H080C   BEDD-5E41 / 03B967EE / DDFB2CCC   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd
	0252636-U72 Series 01 M27v322 MK6USA   BUFFALO   FAF0AHBC   A399-E6B6 / 03B777DC / DCD03030   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd
	*/
	ROM_LOAD32_WORD("0252636.u86", 0x0000000, 0x0400000, CRC(8bee03c5) SHA1(46b39fe0b18c09764e7c7ac44838eb80df536100) )
	ROM_LOAD32_WORD("0252636.u73", 0x0000002, 0x0400000, CRC(46061814) SHA1(9a149d1edabc4a7ab8e30df9a08390a320b9b472) )
	ROM_LOAD32_WORD("0252636.u85", 0x0800000, 0x0400000, CRC(8e5ac194) SHA1(5e7a84d5211cb4e797aa04f21220e72f9d4f4694) )
	ROM_LOAD32_WORD("0252636.u72", 0x0800002, 0x0400000, CRC(2a35e7df) SHA1(112ea1872bc3b8533e1339f8eb19ef9ea0740773) )
ROM_END


// 0351180 / US166
// Bumble Bugs / USA / A - 05/13/05
// Variations (% and NO):
//    87.006% 99
//    89.962% 01
//    92.691% 02
//    94.623% 03
//    97.020% 04
ROM_START( bmbugs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0351180-U86 Series:01 M27V322 Mk6USA   Bumble Bugs   L05-3303-01   60154FA5   8191-302C / 1512D246 / B52F61EF   Copyright © 2005 Aristocrat Technologies, Inc.
	0351180-U73 Series:01 M27V322 Mk6USA   Bumble Bugs   L05-3303-02   14BH22D7   9C9B-C8E1 / 14B3730D / A812F68C   Copyright © 2005 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0351180.u86", 0x0000000, 0x0400000, CRC(f5c204f9) SHA1(592fac02d26ec5edffce862fcce67869430028dd) )
	ROM_LOAD32_WORD("0351180.u73", 0x0000002, 0x0400000, CRC(79dd907f) SHA1(06352c55ff36a82f4d0550b1723667f3287c4923) )
ROM_END


// 10006111 / 673
// Bush Ranger / Local / A - 18/04/01
// 'Bushranger' on artwork, 'Bush Ranger' in ROM
// Variations (% and NO):
//    87.91% 99
//    90.13% 01
//    92.08% 02
ROM_START( bushrngr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10006111.u86", 0x0000000, 0x0400000, CRC(e2548c80) SHA1(485476a71f8910fb20a185e5883f863e0628e060) )
	ROM_LOAD32_WORD("10006111.u73", 0x0000002, 0x0400000, CRC(73206610) SHA1(9a9eb07efde20c5b079a39c96db447937f555c93) )
	ROM_LOAD32_WORD("10006111.u85", 0x0800000, 0x0400000, CRC(5b471edb) SHA1(78da72b727d71f24fbfed34feda9167dd10d96e3) )
	ROM_LOAD32_WORD("10006111.u72", 0x0800002, 0x0400000, CRC(71a0084f) SHA1(1e7b7b58b2df25ea0468d702d7eef658d7c53f47) )
ROM_END


// 10237711 / 818
// Cactus Corral / D - 24/07/07
// Variations (% and NO):
//    87.81% 99
//    90.03% 01
ROM_START( cactusco )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10237711.u86", 0x0000000, 0x0400000, CRC(bd7a64ed) SHA1(0047467954eabc718b5355e61f2756fd9e29aabc) )
	ROM_LOAD32_WORD("10237711.u73", 0x0000002, 0x0400000, CRC(b2f2149d) SHA1(8f53b168555bc255d3dc9fb94d2d6dd466b7f3e4) )
	ROM_LOAD32_WORD("10237711.u85", 0x0800000, 0x0400000, CRC(e0130bf6) SHA1(84f7c2b5c10155ee92bcc02d6c3dca0acf55ed27) )
	ROM_LOAD32_WORD("10237711.u72", 0x0800002, 0x0400000, CRC(90d72135) SHA1(fb4a4ad984ff1103e2e951919132f79cb7af9ec5) )
ROM_END


// 10256611 / 827
// Call Of The Wild / B - 12/11/07
// Variations (% and NO):
//    87.81% 99
//    90.01% 01
ROM_START( callwld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10256611.u86", 0x0000000, 0x0400000, CRC(80b28091) SHA1(c27f57c7bfe797463ddd203304d9be58918aa586) )
	ROM_LOAD32_WORD("10256611.u73", 0x0000002, 0x0400000, CRC(36231cad) SHA1(9230ef7bb8433c611754b4625a64bef3931aba0d) )
	ROM_LOAD32_WORD("10256611.u85", 0x0800000, 0x0400000, CRC(725dad93) SHA1(7228e3637331450667ed872e48a6693846a5b87a) )
	ROM_LOAD32_WORD("10256611.u72", 0x0800002, 0x0400000, CRC(8ee4f3e7) SHA1(553487a4c2915e9621330d6a8cd621dd6b4e38d3) )
	ROM_LOAD32_WORD("10256611.u84", 0x1000000, 0x0400000, CRC(c6d27e00) SHA1(4f81922c0f3f82f48a8354d1a81283f06aaa9fb6) )
	ROM_LOAD32_WORD("10256611.u71", 0x1000002, 0x0400000, CRC(ed7dc53a) SHA1(686326580fe1a58685341d82ae5697fd216b88b6) )
ROM_END


// 20197111 / 772/1
// Calypso King / Local / B - 25/07/05
// Variations (% and NO):
//    87.81% 99
//    90.08% 01
//    91.84% 02
ROM_START( calyking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197111.u86", 0x0000000, 0x0400000, CRC(09ae1f6a) SHA1(2bfc4fb9e01084c460321bb3fa29cee0600c5654) )
	ROM_LOAD32_WORD("20197111.u73", 0x0000002, 0x0400000, CRC(a6f903f6) SHA1(6101cc9115b91c6860cfd3fbe5ab2da105a41ef4) )
	ROM_LOAD32_WORD("20197111.u85", 0x0800000, 0x0400000, CRC(4e130673) SHA1(6d49558146c0e3e8a0f1659dcc2ebe60ae17782b) )
	ROM_LOAD32_WORD("20197111.u72", 0x0800002, 0x0400000, CRC(1a09ab1e) SHA1(8aca20b52d1072387e49e6d020c6e9565650aec9) )
ROM_END


// 10218211 / 809
// Canary Riches / Local / A - 30/03/06
// Variations (% and NO):
//    87.68% 99
//    90.06% 01
//    91.87% 02
ROM_START( canaryrc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10218211.u86", 0x0000000, 0x0400000, CRC(37750a22) SHA1(dcb73df1c6bd206343302a8d70966291750a5baf) )
	ROM_LOAD32_WORD("10218211.u73", 0x0000002, 0x0400000, CRC(6ae381ec) SHA1(d1103744805137dc69b77a3e1319c7168c88306f) )
ROM_END


// 0152314 / US248
// Centurion / Local / A - 11/27/07
// Variations (% and NO):
//    88.002% 99 (Reel 1)    88.002% 99 (Reels 1-2)    88.002% 99 (Reels 1-3)    88.009% 99 (Reels 1-4)    88.060% 99 (Reels 1-5)
//    90.115% 01 (Reel 1)    90.115% 01 (Reels 1-2)    90.115% 01 (Reels 1-3)    90.160% 01 (Reels 1-4)    90.167% 01 (Reels 1-5)
//    92.086% 02 (Reel 1)    92.086% 02 (Reels 1-2)    92.086% 02 (Reels 1-3)    92.108% 02 (Reels 1-4)    92.124% 02 (Reels 1-5)
//    94.857% 03 (Reel 1)    94.857% 03 (Reels 1-2)    94.857% 03 (Reels 1-3)    94.850% 03 (Reels 1-4)    94.859% 03 (Reels 1-5)
ROM_START( centrion )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152314-U86 Series 01 M27v322 Mk6USA   CENTURION   1U18E4FC S2008-1113-04   D02E-C7AE / 1DBBC97A / 815E6FBB   Copyright © 2004-2007 Aristocrat Technologies Australia Pty Ltd.
	0152314-U73 Series 01 M27v322 Mk6USA   CENTURION   5AC05PD1 S2008-1113-02   25C7-C0A3 / 1DB7DD52 / 8501620A   Copyright © 2004-2007 Aristocrat Technologies Australia Pty Ltd.
	0152314-U85 Series 01 M27v322 Mk6USA   CENTURION   6315BC51 S2008-1113-03   E825-8175 / 1BBE24D3 / 0DF79B23   Copyright © 2004-2007 Aristocrat Technologies Australia Pty Ltd.
	0152314-U72 Series 01 M27v322 Mk6USA   CENTURION   6996EA9H S2008-1113-01   A5D5-434A / 1B96E0FA / FA022A59   Copyright © 2004-2007 Aristocrat Technologies Australia Pty Ltd.
	*/
	ROM_LOAD32_WORD("0152314.u86", 0x0000000, 0x0400000, CRC(fd72e607) SHA1(3af0c7b9f30e061d80de1d2f29e0e356354ee4b4) )
	ROM_LOAD32_WORD("0152314.u73", 0x0000002, 0x0400000, CRC(c55629d1) SHA1(c154bb63cb75b9fa0aba857f6bc175a27b7ca625) )
	ROM_LOAD32_WORD("0152314.u85", 0x0800000, 0x0400000, CRC(7d57fe03) SHA1(1a6c94cce462ea6bb67785cfa87f10ffc6ca2f19) )
	ROM_LOAD32_WORD("0152314.u72", 0x0800002, 0x0400000, CRC(b038e439) SHA1(697f41ed1694c6211c0b0174ff5ab206bd3d18ad) )
ROM_END


// 30194711 / MG0007
// Cashman Tonight / B - 25/05/05
// 4-in-1 game containing African Dusk, Arctic Wins, King Galah and Wild Cougar
// Variations (% and NO):
//    88/93% 99
//    90/95% 01
// African Dusk - 1c / 785 / C - 19/05/05
// African Dusk - 2c / 785/1 / C - 19/05/05
// Variations (% and NO):
//    88.13% 99
//    90.17% 01
// Arctic Wins - 1c / 786 / C - 19/05/05
// Arctic Wins - 2c / 786/1 / C - 19/05/05
// Arctic Wins is not the same game as the original MK4 title
// Variations (% and NO):
//    88.11% 99
//    90.14% 01
// King Galah - 1c / 787 / C - 19/05/05
// King Galah - 2c / 787/1 / C - 19/05/05
// Variations (% and NO):
//    87.93% 99
//    90.23% 01
// Wild Cougar - 1c / 788 / C - 19/05/05
// Wild Cougar - 2c / 788/1 / C - 19/05/05
// Wild Cougar is different to the original in that it has stacked symbols e.g. 3 of the same symbol (except Cougar and Wolf) can show up on the same reel
// The base game is similar to Mr Woo Moonlight Market
// Variations (% and NO):
//    88.13% 99
//    90.17% 01
ROM_START( cmtonig )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30194711.u11", 0x0000000, 0x0400000, CRC(e39fdd27) SHA1(b5e391740ba695b09e2ecb169760240f39b6a27a) )
	ROM_LOAD32_WORD("30194711.u21", 0x0000002, 0x0400000, CRC(9defed89) SHA1(0496ee9b378b7117a8cea89c7b3fcca1bae20e64) )
	ROM_LOAD32_WORD("30194711.u12", 0x0800000, 0x0400000, CRC(3c93d700) SHA1(f940d3eff2225fa3470372b7cd8b630511d53843) )
	ROM_LOAD32_WORD("30194711.u22", 0x0800002, 0x0400000, CRC(2a9dd5fd) SHA1(ca484bf494b7431baaadaf7df663ffd84ea07120) )
	ROM_LOAD32_WORD("30194711.u13", 0x1000000, 0x0400000, CRC(1bce542e) SHA1(8dfb78d966ee50d866739f86c6da9b33e09cd108) )
	ROM_LOAD32_WORD("30194711.u23", 0x1000002, 0x0400000, CRC(2cbd3ef8) SHA1(c4a830c9bae7c50276ba951128056292ac09e19d) )
	ROM_LOAD32_WORD("30194711.u14", 0x1800000, 0x0400000, CRC(cb1a9281) SHA1(1de3b988336fb930d636f3bad37d3d5dc359713b) )
	ROM_LOAD32_WORD("30194711.u24", 0x1800002, 0x0400000, CRC(1dbc278e) SHA1(ac48fce5256a04eabd8271de96b300fb7c265927) )
	ROM_LOAD32_WORD("30194711.u15", 0x2000000, 0x0400000, CRC(5f7dd3f8) SHA1(23db92025455b7ab089a990b2f2cab3acb9ab3b5) )
	ROM_LOAD32_WORD("30194711.u25", 0x2000002, 0x0400000, CRC(5e28e9cb) SHA1(e45cce804fe014078d399a04ca68f1a79d2d5f41) )
	ROM_LOAD32_WORD("30194711.u16", 0x2800000, 0x0400000, CRC(6ac1430d) SHA1(a782cdcdd80e59f8f7de9a16559609759eaefb78) )
	ROM_LOAD32_WORD("30194711.u26", 0x2800002, 0x0400000, CRC(b9d91cab) SHA1(0e79c9e02068c47a9ffc7ace70adad2b579d559f) )
	ROM_LOAD32_WORD("30194711.u17", 0x3000000, 0x0400000, CRC(9b69c5ca) SHA1(f13c4355319c6c45316a05f74a561964657990b8) )
	ROM_LOAD32_WORD("30194711.u27", 0x3000002, 0x0400000, CRC(1bf2985c) SHA1(7c037b290983b70efe5f72cb8b8b30e012b53860) )
ROM_END


// 0251125 / US119
// Chance In Hell / Local / A- 06/30/04
// Variations (% and NO):
//    92.475% 99
ROM_START( cinhell )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251125.u86", 0x0000000, 0x0400000, CRC(c064a4a6) SHA1(c3cc4326b92e690ee2df3ab2d58a809435d897c2) )
	ROM_LOAD32_WORD("0251125.u73", 0x0000002, 0x0400000, CRC(d0a3c39d) SHA1(eff6a03042d175e6088dd7bada25791c7b404687) )
	ROM_LOAD32_WORD("0251125.u85", 0x0800000, 0x0400000, CRC(44234acd) SHA1(384676e2fdeb1a216078826e0e4634d67e0d1be3) )
	ROM_LOAD32_WORD("0251125.u72", 0x0800002, 0x0400000, CRC(a6c1937d) SHA1(436bab41fabc9b991e4d8128abc5d7b1c627331e) )
	ROM_LOAD32_WORD("0251125.u84", 0x1000000, 0x0400000, CRC(60427f98) SHA1(ff59b0e8bca48d29c08d1c7e672dc6318fcf9a10) )
	ROM_LOAD32_WORD("0251125.u71", 0x1000002, 0x0400000, CRC(f096865e) SHA1(c8091028dfaf43f7017c40a488d039bd6e1b0177) )
ROM_END


// 10104011 / 688
// Choy Sun Doa / Local / A - 23/04/02
// Variations (% and NO):
//    87.98% 99
//    89.95% 01
//    92.00% 02
ROM_START( csd )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10104011.u86", 0x0000000, 0x0400000, CRC(674fbe7a) SHA1(5292dd3887f066e6ea150fe8c140c2d3cde142d9) )
	ROM_LOAD32_WORD("10104011.u73", 0x0000002, 0x0400000, CRC(78fb8acb) SHA1(60cde2c9fd0c3a6ac8a76099906c87393d90b8bf) )
	ROM_LOAD32_WORD("10104011.u85", 0x0800000, 0x0400000, CRC(e1b6c747) SHA1(3e0345357c4b66b30c87e6ebf540fcc636505804) )
	ROM_LOAD32_WORD("10104011.u72", 0x0800002, 0x0400000, CRC(732bc777) SHA1(bc3ced0a3e3e75151bfdf1c7d79fff2ebac15230) )
ROM_END


// 20104011 / 688
// Choy Sun Doa / Local / A - 23/04/02
// SHOW PROGRAM
// Variations (% and NO):
//    87.98% 99
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( csdsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__20104011.u86", 0x0000000, 0x0400000, CRC(a60deda0) SHA1(d439f3f0b267d94f88f74052068becadc96fd76e) )
	ROM_LOAD32_WORD("sp__20104011.u73", 0x0000002, 0x0400000, CRC(00dcb4e5) SHA1(d7d32c15bdea0e28e936b87ed7b609e979aae460) )
	ROM_LOAD32_WORD("sp__20104011.u85", 0x0800000, 0x0400000, CRC(fdeb3434) SHA1(df306a2443b46c565040307b61ed97242f4b7280) )
	ROM_LOAD32_WORD("sp__20104011.u72", 0x0800002, 0x0400000, CRC(566792bf) SHA1(843ccbc65020ef87ef67b9d81b864c1d8674b1dd) )
ROM_END


// 20131511 / 688
// Choy Sun Doa / ASP / D - 23/01/03
// Variations (% and NO):
//    87.98% 99
ROM_START( csda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20131511.u86", 0x0000000, 0x0400000, CRC(06f78c92) SHA1(e8bd3f18831dfb5c644321541fa9e75ae9e83688) )
	ROM_LOAD32_WORD("20131511.u73", 0x0000002, 0x0400000, CRC(5b2468b6) SHA1(085aa44343f11fdf5ab7cc1ca56ddb0ba5cafc36) )
	ROM_LOAD32_WORD("20131511.u85", 0x0800000, 0x0400000, CRC(6973dffd) SHA1(4350e0cdfeb9135e708f15bf2de325b8412c1434) )
	ROM_LOAD32_WORD("20131511.u72", 0x0800002, 0x0400000, CRC(1c9a77d1) SHA1(ef31215be73b09531039fda1a2f3e875045180f3) )
ROM_END


// 10121111 / 688
// Choy Sun Doa / QCOM / C - 05/07/02
// Variations (% and NO):
//    87.98% 99
ROM_START( csdq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10121111.u86", 0x0000000, 0x0400000, CRC(cbcabeae) SHA1(ebc951037bfda441833beb5b0ca102dc76fcc901) )
	ROM_LOAD32_WORD("10121111.u73", 0x0000002, 0x0400000, CRC(4bb97e62) SHA1(8144b6ec8a69b12fd4e4641999a988ad6a9e5160) )
	ROM_LOAD32_WORD("10121111.u85", 0x0800000, 0x0400000, CRC(8425f9d0) SHA1(81a86caf20d2ee18f6a3e68da58e69ff00bbc68a) )
	ROM_LOAD32_WORD("10121111.u72", 0x0800002, 0x0400000, CRC(f11f4f9d) SHA1(684930290f8b5c933bb72a08db4875ce9b7483da) )
ROM_END


// 30149511 / 688
// Choy Sun Doa Cash Express / Local / E - 03/02/03
// Cash Express Hyperlink game
// Variations (% and NO):
//    82.00% 05 (Hyperlink)
ROM_START( csdce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30149511.u86", 0x0000000, 0x0400000, CRC(cdf64cac) SHA1(3673f1cce8ce92192754ba2afa24be14dbfd760f) )
	ROM_LOAD32_WORD("30149511.u73", 0x0000002, 0x0400000, CRC(d38ffd1b) SHA1(3d39c93478faafbadd9caf6f2a9b3d40ac4e51ce) )
	ROM_LOAD32_WORD("30149511.u85", 0x0800000, 0x0400000, CRC(c58ea0e5) SHA1(545dc5d0e6d18d5f22ec3e216d9204efc1d69633) )
	ROM_LOAD32_WORD("30149511.u72", 0x0800002, 0x0400000, CRC(f74e7f25) SHA1(5b43b54c10141b845d0de1d4f3dd54b833bd2d2a) )
	ROM_LOAD32_WORD("30149511.u84", 0x1000000, 0x0400000, CRC(15a5fb71) SHA1(bb2025e4a85ba77daf8bb6b55e38a8b8250f90f3) )
	ROM_LOAD32_WORD("30149511.u71", 0x1000002, 0x0400000, CRC(32fcccb0) SHA1(a0fa260dadf4b6c659eaa60f67a3e552dbb1807b) )
ROM_END


// 20156611 / 688
// Choy Sun Doa - Jackpot Carnival / QCOM / G - 04/12/03
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.00% 05 (Hyperlink)
ROM_START( csdjc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U86
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U73
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U85
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U72
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U84
	20156611   QCOM   CHOY SUN DOA - J/C HYP   688   4MB   6 eproms   Aristocrat © 2004   P   U71
	*/
	ROM_LOAD32_WORD("20156611.u86", 0x0000000, 0x0400000, CRC(85f8338a) SHA1(c32cfcbc67fd753f9fa7bcb3e53a44f6315d297d) )
	ROM_LOAD32_WORD("20156611.u73", 0x0000002, 0x0400000, CRC(354fb92d) SHA1(6974de53cb9ec87d3ac3147a315ffaa79b65f058) )
	ROM_LOAD32_WORD("20156611.u85", 0x0800000, 0x0400000, CRC(37c104b6) SHA1(0616eabb5191fd089671ab7183cf24880dcaadfd) )
	ROM_LOAD32_WORD("20156611.u72", 0x0800002, 0x0400000, CRC(07ac6bda) SHA1(563ae31d713d57c04906fc756e77310326b4c839) )
	ROM_LOAD32_WORD("20156611.u84", 0x1000000, 0x0400000, CRC(3f1a5d66) SHA1(e80044f988dec6052972187956560f365fae5f9b) )
	ROM_LOAD32_WORD("20156611.u71", 0x1000002, 0x0400000, CRC(c77ba62d) SHA1(2bae686d491ffb55078fd44384a8ed6ead282139) )
ROM_END


// 20212211 / 744/2
// Choy Sun Returns / Local / B - 15/08/06
// Variations (% and NO):
//    88.54% 99
//    87.88% 01
//    86.87% 02
ROM_START( csret )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20212211.u86", 0x0000000, 0x0400000, CRC(0136afeb) SHA1(ec8478216353100a541cf736d54fc440cd89d665) )
	ROM_LOAD32_WORD("20212211.u73", 0x0000002, 0x0400000, CRC(3dadf971) SHA1(ffab779da0160d0e8a78fdc9e1440b2f27d04d20) )
	ROM_LOAD32_WORD("20212211.u85", 0x0800000, 0x0400000, CRC(6771599d) SHA1(c3476b37b9189c3197194a9027d2f154a12aee37) )
	ROM_LOAD32_WORD("20212211.u72", 0x0800002, 0x0400000, CRC(c4278640) SHA1(bfb5c69191661625427742d24f7f2f63e59217c1) )
ROM_END


// 30178311 / 771
// Corrida De Toros / Local / D - 22/03/05
// Variations (% and NO):
//    90.15% 99
//    92.14% 01
//    94.12% 02
ROM_START( corrida )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30178311.u86", 0x0000000, 0x0400000, CRC(19424139) SHA1(8715c995368f5ac410b2ef6f09833b7516df0bd5) )
	ROM_LOAD32_WORD("30178311.u73", 0x0000002, 0x0400000, CRC(6df3c1f5) SHA1(5b8fcbbc650c0fefdf504fd7d48a125d2f7d22e7) )
	ROM_LOAD32_WORD("30178311.u85", 0x0800000, 0x0400000, CRC(451774c0) SHA1(e2fd78433ad9016900581720ccd29bf511a57f04) )
	ROM_LOAD32_WORD("30178311.u72", 0x0800002, 0x0400000, CRC(37d4eed5) SHA1(55aa5f4dad3193f67e8fea7506dde2230b8c6b9e) )
ROM_END


// 10144411 / 740
// Crystal Springs / Local / A - 28/5/03
// Variations (% and NO):
//    87.98% 99
//    90.02% 01
//    91.86% 02
ROM_START( cryspri )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10144411.u86", 0x0000000, 0x0400000, CRC(5c0e7122) SHA1(f729e20d15f9a8d1442a9f9616cfa09ac181bc3f) )
	ROM_LOAD32_WORD("10144411.u73", 0x0000002, 0x0400000, CRC(e8ab1219) SHA1(88e792209d224fc95b4248fc1aeebcae39b06eb7) )
ROM_END


// 10155811 / 740
// Crystal Springs / ASP / A - 28/5/03
// Same date string as parent
// Variations (% and NO):
//    87.98% 99
ROM_START( cryspria )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10155811.u86", 0x0000000, 0x0400000, CRC(b046ea06) SHA1(0c0310bc0afb8bac630ac0570d5b9df6a992cfdb) )
	ROM_LOAD32_WORD("10155811.u73", 0x0000002, 0x0400000, CRC(b52cac8a) SHA1(65bb5d73933df6d53a079e4efe00ea29649e3201) )
ROM_END


// 0352557 / MV4255
// Crystal Springs / Local / C - 01/16/06
// Variations (% and NO):
//    87.987% 99
//    90.027% 01
//    91.864% 02
//    94.957% 03
ROM_START( cryspriu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352557.u86", 0x0000000, 0x0400000, CRC(dcb920fc) SHA1(71ce2a4298c9012ef7724b206ab1270b25c8bf8d) )
	ROM_LOAD32_WORD("0352557.u73", 0x0000002, 0x0400000, CRC(caa73d62) SHA1(5e41beb9ade7bd50cdc616ab3f016a6615778ea2) )
	ROM_LOAD32_WORD("0352557.u85", 0x0800000, 0x0400000, CRC(c7c542c1) SHA1(8b49969414ef3d5811c804f16dc78cba71279618) )
	ROM_LOAD32_WORD("0352557.u72", 0x0800002, 0x0400000, CRC(8864632c) SHA1(a3227831ee71bc01846a7400e14fb40e62770e7a) )
ROM_END


// 10282311 / JB113/2
// Deep Freeze / A - 27/02/09
// Variations (% and NO):
//    87.92% 99
//    90.17% 01
//    88.54% 07
ROM_START( deepfrz )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10282311.u86", 0x0000000, 0x0400000, CRC(82187e66) SHA1(bb8bb2c50ab722d0e8d7e2e0d6b4be095fa1d3b3) )
	ROM_LOAD32_WORD("10282311.u73", 0x0000002, 0x0400000, CRC(de7c52eb) SHA1(6291a9f032c164d000a75b24a19dc9e8fb9184a1) )
	ROM_LOAD32_WORD("10282311.u85", 0x0800000, 0x0400000, CRC(30040885) SHA1(0f0f4cc3cd7ee70c03099bdeec60313c16a21c70) )
	ROM_LOAD32_WORD("10282311.u72", 0x0800002, 0x0400000, CRC(a024e1e0) SHA1(5d4d6ab51aef1668e05b9296c7204c0b24c5f473) )
	ROM_LOAD32_WORD("10282311.u84", 0x1000000, 0x0400000, CRC(9bc78342) SHA1(91f99679eda7d0b8bfcb8cc741bf5d9e9ed6c2d4) )
	ROM_LOAD32_WORD("10282311.u71", 0x1000002, 0x0400000, CRC(7fde4cb0) SHA1(6ecfdaab82d2b2f30e0d122dee08b7e0848e0cea) )
ROM_END


// 0154081 / JB113/4
// Deep Freeze / A - 27/07/09
// Variations (% and NO):
//    87.922% 99
//    90.169% 01
//    91.863% 02
//    94.919% 03
ROM_START( deepfrzu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0154081-U86 Series 01 M27v322 MK6USA   DEEP FREEZE   938U69FZ S2010-1502-4   AF8E-BBE1 / 1FD0EF43 / 6A57CC83   Copyright © 2010 Aristocrat Technologies Australia Pty Ltd.
	0154081-U73 Series 01 M27v322 MK6USA   DEEP FREEZE   DFAC6583 S2010-1502-2   D57E-D0EE / 1FC1556E / 63A52169   Copyright © 2010 Aristocrat Technologies Australia Pty Ltd.
	0154081-U85 Series 01 M27v322 MK6USA   DEEP FREEZE   17B0BCD3 S2010-1502-3   D763-E8D7 / 1A1D9881 / A2BB6D93   Copyright © 2010 Aristocrat Technologies Australia Pty Ltd.
	0154081-U72 Series 01 M27v322 MK6USA   DEEP FREEZE   266C6835 S2010-1502-1   4B96-2EF0 / 1C773922 / EA27A12D   Copyright © 2010 Aristocrat Technologies Australia Pty Ltd.
	*/
	ROM_LOAD32_WORD("0154081.u86", 0x0000000, 0x0400000, CRC(7f98661d) SHA1(970dcba4c2f5b40702c33751c73541a22fbcdd26) )
	ROM_LOAD32_WORD("0154081.u73", 0x0000002, 0x0400000, CRC(80fc6979) SHA1(2a08852c930f4b732830da4761010cc87ef369fc) )
	ROM_LOAD32_WORD("0154081.u85", 0x0800000, 0x0400000, CRC(ef3914cd) SHA1(e189180fb087636b6d07de5df934ed4c89e8477e) )
	ROM_LOAD32_WORD("0154081.u72", 0x0800002, 0x0400000, CRC(bcfc43a6) SHA1(cd3b25005d67dc096b13b1d35eca9701f150e461) )
ROM_END


// 0252532 / MV4243
// Delta Belle / Local / B - 04/29/05
// Variations (% and NO):
//    87.812% 99
//    90.026% 01
//    91.920% 02
//    94.365% 03
//    97.431% 04
ROM_START( deltabel )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252532.u86", 0x0000000, 0x0400000, CRC(ee2c3aad) SHA1(c3687c1447c3c324fa23f5f61d8b88139141a8d4) )
	ROM_LOAD32_WORD("0252532.u73", 0x0000002, 0x0400000, CRC(721cb449) SHA1(10be95803f3b7e86a009cc665e847dd97aeb6749) )
	ROM_LOAD32_WORD("0252532.u85", 0x0800000, 0x0400000, CRC(53eec755) SHA1(017cad59b8c25b7b24df869cb5d61867607cebb9) )
	ROM_LOAD32_WORD("0252532.u72", 0x0800002, 0x0400000, CRC(d6f6e2c9) SHA1(590c9b2000bdd888100426017250688bef9d3c1c) )
ROM_END


// 20185211 / 768
// Delta Queen / Local / A - 13/09/04
// Variations (% and NO):
//    87.80% 99
//    90.05% 01
//    91.81% 02
ROM_START( deltaqn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20185211.u86", 0x0000000, 0x0400000, CRC(93b7ca6b) SHA1(1384638d4da321ec7044f0bf10dfc626c1928215) )
	ROM_LOAD32_WORD("20185211.u73", 0x0000002, 0x0400000, CRC(56628af9) SHA1(778175ce3d0074569e434a9c3995751747352e15) )
	ROM_LOAD32_WORD("20185211.u85", 0x0800000, 0x0400000, CRC(73fe916b) SHA1(c262775bde046b5457ab998017012af12b5c036c) )
	ROM_LOAD32_WORD("20185211.u72", 0x0800002, 0x0400000, CRC(1978abb1) SHA1(7ffd1d81201bcf31936bbc5bd194b182cb9b5cdc) )
ROM_END


// 20202311 / 748
// Desert Gold / Local / D - 31/05/05
// Variations (% and NO):
//    87.93% 99
ROM_START( dstgold )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20202311.u86", 0x0000000, 0x0400000, CRC(0ad7582b) SHA1(65ed266635e176ce495f128ec9ee5c7a1824e249) )
	ROM_LOAD32_WORD("20202311.u73", 0x0000002, 0x0400000, CRC(59744017) SHA1(a701514b4b48a526ddbd4fa4d788a9acad8ceea4) )
	ROM_LOAD32_WORD("20202311.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20202311.u72", 0x0800002, 0x0400000, CRC(2f8938ec) SHA1(9b4d77d9c12d9da1b17785c840dbc86ea9978807) )
ROM_END


// 10229311 / 821
// Diamonds and Hearts - Power Pay / Local / D - 22/02/07
// 'Diamonds & Hearts' on artwork, 'Diamonds and Hearts' in ROM
// Variations (% and NO):
//    87.51% 99
//    85.54% 01
ROM_START( diamhrts )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10229311.u86", 0x0000000, 0x0400000, CRC(6bffd690) SHA1(35d4f3ef58955bef91aef7b2d3a70af264d9d18d) )
	ROM_LOAD32_WORD("10229311.u73", 0x0000002, 0x0400000, CRC(70c2c9de) SHA1(ef361bc58eb50b47602d96f0655ad08d3049a449) )
ROM_END


// 10129211 / JB012
// Diamond Eyes / ASP / G - 25/03/02
// Variations (% and NO):
//    87.90% 99
ROM_START( dimeye )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10129211.u86", 0x0000000, 0x0400000, CRC(b0ba42e4) SHA1(1c885ca75bdea92457333f267b879cb49513025d) )
	ROM_LOAD32_WORD("10129211.u73", 0x0000002, 0x0400000, CRC(36dd733f) SHA1(d121e5f0ad6ffc302c80dd089ba1078fc22bf889) )
	ROM_LOAD32_WORD("10129211.u85", 0x0800000, 0x0400000, CRC(3624de8e) SHA1(7972f918e0244bb186242ca053be5f128e19a789) )
	ROM_LOAD32_WORD("10129211.u72", 0x0800002, 0x0400000, CRC(5dfd442b) SHA1(3f4f6ad91adca586d4d33dc8886ede7071d04100) )
ROM_END


// 0251312 / JB012/20
// Diamond Eyes / Local / J - 01/22/04
// EPROM labels marked 2003 despite being a 2004 game
// Variations (% and NO):
//    87.893% 99
//    90.139% 01
//    92.168% 02
//    94.957% 03
//    96.939% 04
ROM_START( dimeyeu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251312-U86 Series:01 M27V322 Mk6USA   Diamond Eyes   E6A2D2DA   4B5B-B5BF / 1E6C64C8 / 6FC8B9B6   Copyright © 2003 Aristocrat Technologies, Inc.
	0251312-U73 Series:01 M27V322 Mk6USA   Diamond Eyes   9F7C25E6   2130-F7C0 / 1DF3CEC2 / 4E417192   Copyright © 2003 Aristocrat Technologies, Inc.
	0251312-U85 Series:01 M27V322 Mk6USA   Diamond Eyes   F1DHFH2H   EBC0-3E3F / 13A4E259 / 32F9E1DB   Copyright © 2003 Aristocrat Technologies, Inc.
	0251312-U72 Series:01 M27V322 Mk6USA   Diamond Eyes   A578F0D2   84C2-4D3F / 13A0484A / 30D3B780   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0251312.u86", 0x0000000, 0x0400000, CRC(dd6cab00) SHA1(27934d1262e91078c0ed3d12510d1157e7d0774f) )
	ROM_LOAD32_WORD("0251312.u73", 0x0000002, 0x0400000, CRC(a0c21ad9) SHA1(689c9fb61fc7864be22b22fbdac295ced28340db) )
	ROM_LOAD32_WORD("0251312.u85", 0x0800000, 0x0400000, CRC(ca138b5c) SHA1(e0af5ac82b40f40af0438e4f6ce5d1e8d984d34d) )
	ROM_LOAD32_WORD("0251312.u72", 0x0800002, 0x0400000, CRC(f5f4d871) SHA1(e1a83ed4932602f460321bb2aaf648ef10711033) )
ROM_END


// 0152039 / JB012/20
// Diamond Eyes / Local / J - 01/22/04
// Same date string as dimeyeu
// Cash Express Hyperlink game
// EPROM labels marked 2003 despite being a 2004 game
// Variations (% and NO):
//    87.893% 99
//    90.139% 01
//    92.168% 02
//    94.957% 03
//    96.939% 04
//    82.160% 05 (Hyperlink)
ROM_START( dimeyece )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152039-U86 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-01   CAC9128C   4C92-FA1D / 16EB68E5 / C1D94B5E   Copyright © 2003 Aristocrat Technologies, Inc.
	0152039-U73 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-02   B0186229   41DD-E6DF / 16D608F9 / B9455A8A   Copyright © 2003 Aristocrat Technologies, Inc.
	0152039-U85 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-03   951C3U47   67B6-6BE8 / 1EBF176F / 93611749   Copyright © 2003 Aristocrat Technologies, Inc.
	0152039-U72 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-04   2FC1678H   BED6-0C2B / 1E8D19BE / 7ED536E9   Copyright © 2003 Aristocrat Technologies, Inc.
	0152039-U84 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-05   1FC52033   4CA0-FFBC / 0AF918AD / B42348A4   Copyright © 2003 Aristocrat Technologies, Inc.
	0152039-U71 Series:01 M27V322 Mk6USA   Diamond Eyes – CE   L06-0525-06   75462433   EC20-BA87 / 0AECF0A9 / AFBF1A03   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152039.u86", 0x0000000, 0x0400000, CRC(926ce073) SHA1(12f0aa1e387f0f87be92afb9a917a30d51a63277) )
	ROM_LOAD32_WORD("0152039.u73", 0x0000002, 0x0400000, CRC(235ba6ca) SHA1(3534b4f74531a42543383b55a2c57853b1d3f4f1) )
	ROM_LOAD32_WORD("0152039.u85", 0x0800000, 0x0400000, CRC(f6ef5d9c) SHA1(94505a470ad65e75c6c984ec163ce92c558f491a) )
	ROM_LOAD32_WORD("0152039.u72", 0x0800002, 0x0400000, CRC(9f6b5923) SHA1(739d3848998ddbfaf577982c12d8edd19c348548) )
	ROM_LOAD32_WORD("0152039.u84", 0x1000000, 0x0400000, CRC(6d0dea65) SHA1(ee23c0fe2ecb5adb5e383757ade9be17880e6b45) )
	ROM_LOAD32_WORD("0152039.u71", 0x1000002, 0x0400000, CRC(5706aa40) SHA1(d847bd0a4e210e3f065aa59129ee4654e3087909) )
ROM_END


// 10105011 / 679
// DIAMONDS FOREVER / Local / B - 14/03/02
// Variations (% and NO):
//    87.63% 99
//    89.88% 01
//    92.06% 02
ROM_START( di4ever )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10105011.u86", 0x0000000, 0x0400000, CRC(d7c0fd99) SHA1(97d2b17ea89d7dd0186a13f08fe68a7f8e1336ca) )
	ROM_LOAD32_WORD("10105011.u73", 0x0000002, 0x0400000, CRC(39922c60) SHA1(19c36087cae9cf482d3dc86dc40fa94c3687ae96) )
ROM_END


// 10118811 / 731
// Dinosaur / Local / C - 08/08/03
// Variations (% and NO):
//    97.55% 99
//    95.10% 01
//    93.62% 02
ROM_START( dinosa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10118811.u86", 0x0000000, 0x0400000, CRC(74e08c66) SHA1(002a526e8e693aa68f45ae8485114c8bdc3f2453) )
	ROM_LOAD32_WORD("10118811.u73", 0x0000002, 0x0400000, CRC(17b46b56) SHA1(43d55adcc54855b6845ffedba8998b14df0447f3) )
	ROM_LOAD32_WORD("10118811.u85", 0x0800000, 0x0400000, CRC(cff22e03) SHA1(0fd6632ae9c71bc99a31e9bdc9abdfda09dfafd7) )
	ROM_LOAD32_WORD("10118811.u72", 0x0800002, 0x0400000, CRC(34181cf9) SHA1(93d014891789260fe0a0eeeff17c25c6211753db) )
ROM_END


// 10177911 / 602/1
// Dolphin Treasure / Local / I - 19/07/04
// Variations (% and NO):
//    87.87% 99
ROM_START( doltra6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177911.u86", 0x0000000, 0x0400000, CRC(0ec06b74) SHA1(8f16751b721af3941c737652ebcdde2599c14497) )
	ROM_LOAD32_WORD("10177911.u73", 0x0000002, 0x0400000, CRC(99dc7748) SHA1(b78d1373a88b4693a689374ed52626fc9a063d2d) )
ROM_END


// 20265311 / MV4047/1
// Dolphin Treasure / NZ - QCOM / A - 21/09/04
// Variations (% and NO):
//    87.63% 99
ROM_START( doltra6nz )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20265311   NZ   DOLPHIN TREASURE   MV4047/1   4MB   2 eproms   Aristocrat © 2008   P   U86
	20265311   NZ   DOLPHIN TREASURE   MV4047/1   4MB   2 eproms   Aristocrat © 2008   P   U73
	*/
	ROM_LOAD32_WORD("20265311.u86", 0x0000000, 0x0400000, CRC(85b64233) SHA1(3055cc8b6ab78e6216becfbb4ffea18a00b1718e) )
	ROM_LOAD32_WORD("20265311.u73", 0x0000002, 0x0400000, CRC(2fc1cf23) SHA1(3acdd397540c9ace36ebb3d4d23e4eee0b510a94) )
ROM_END


// 0152082 / US180
// Double Agent / Local / A - 12/09/05
// Variations (% and NO):
//    87.659% 99
//    90.245% 01
//    92.723% 02
//    94.639% 03
//    97.838% 04
ROM_START( dblagent )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152082-U86 Series:01 M27V322 Mk6USA   Double Agent   L06-1336-04   35CPF15F   2C48-003A / 179518B0 / 6C7A8BFA   Copyright © 2005 Aristocrat Technologies, Inc.
	0152082-U73 Series:01 M27V322 Mk6USA   Double Agent   L06-1336-02   FF1526DP   BC04-1A32 / 1E3CDC90 / 5176A947   Copyright © 2005 Aristocrat Technologies, Inc.
	0152082-U85 Series:01 M27V322 Mk6USA   Double Agent   L06-1336-03   A3848U83   E867-ACF2 / 0058BA18 / 2B15FDEC   Copyright © 2005 Aristocrat Technologies, Inc.
	0152082-U72 Series:01 M27V322 Mk6USA   Double Agent   L06-1336-01   DU643UF8   1D98-3854 / 00514957 / 278684BF   Copyright © 2005 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152082.u86", 0x0000000, 0x0400000, CRC(d081a5f3) SHA1(0adfbdc36b70a26f858d353ee4381e5848d6e499) )
	ROM_LOAD32_WORD("0152082.u73", 0x0000002, 0x0400000, CRC(235e2cc8) SHA1(aaf1e71f4cdeb039e3c760031b07d7eb54467742) )
	ROM_LOAD32_WORD("0152082.u85", 0x0800000, 0x0400000, CRC(4d482f50) SHA1(8105371d582bcc5a7d0e35f110f394fc491550cf) )
	ROM_LOAD32_WORD("0152082.u72", 0x0800002, 0x0400000, CRC(fdb9a838) SHA1(83976af01273234d7eaeedba60164f9c31915e4b) )
ROM_END


// 20000211 / 647
// Double Dolphins / Local / C - 18/04/00
// Variations (% and NO):
//    87.97% 99
//    90.12% 01
//    92.10% 02
ROM_START( dbldolph )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20000211.u86", 0x0000000, 0x0400000, CRC(ab98e4c1) SHA1(95ca99633a11d0087a4a702be8c075f1ad87bed4) )
	ROM_LOAD32_WORD("20000211.u73", 0x0000002, 0x0400000, CRC(493c254b) SHA1(940c5b905c9e5677db5589fb097418d08047897d) )
	ROM_LOAD32_WORD("20000211.u85", 0x0800000, 0x0400000, CRC(7aa49b2d) SHA1(15ae3f0eb6d1e44c5453e557b04f1ab3941163dc) )
	ROM_LOAD32_WORD("20000211.u72", 0x0800002, 0x0400000, CRC(cd886389) SHA1(d0044d88d7cc168ae24e3a61f7e4758317b82417) )
ROM_END


// 40003211 / 647
// Double Dolphins / Local / F - 17/11/00
// Variations (% and NO):
//    87.97% 99
ROM_START( dbldolphq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40003211.u86", 0x0000000, 0x0400000, CRC(73fbb9df) SHA1(3f5e68f24d4d38f0b514acaac0672ca2a40f31f3) )
	ROM_LOAD32_WORD("40003211.u73", 0x0000002, 0x0400000, CRC(e69e12f2) SHA1(176af1d3177da18468673a9a52672c97a278c75a) )
	ROM_LOAD32_WORD("40003211.u85", 0x0800000, 0x0400000, CRC(7df3105d) SHA1(df287fd3f42aff0a0b3bbab5a40723de6170d488) )
	ROM_LOAD32_WORD("40003211.u72", 0x0800002, 0x0400000, CRC(7c366f7f) SHA1(9816f8ef8ae7f1d3083a5094ce27e17df478dc3a) )
ROM_END


// 10173511 / 761
// Dragon Lord / Local / A - 22/06/04
// Variations (% and NO):
//    87.95% 99
//    90.13% 01
//    91.95% 02
ROM_START( drgnlord )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173511.u86", 0x0000000, 0x0400000, CRC(fc699900) SHA1(8e49672e7622ac6bd0106c25f524a7dd9e9578ab) )
	ROM_LOAD32_WORD("10173511.u73", 0x0000002, 0x0400000, CRC(dd887eb0) SHA1(7ac4f86d0cfc69acd4471cc018f8b83d98735b37) )
	ROM_LOAD32_WORD("10173511.u85", 0x0800000, 0x0400000, CRC(d17e1a0d) SHA1(955e4956656e7f08e352628cc632af0e3cb6cf0f) )
	ROM_LOAD32_WORD("10173511.u72", 0x0800002, 0x0400000, CRC(c2b42880) SHA1(7079102d576d8e9c5cb4d3067b02d131b63b8d88) )
ROM_END


// 2J015711 / JB051
// Dragon Magic / Local / E - 05/07/06
// Variations (% and NO):
//    87.91% 99
//    90.08% 01
//    92.00% 02
//    94.87% 03
ROM_START( drgnmgic )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("2j015711.u86", 0x0000000, 0x0400000, CRC(9fb775a7) SHA1(636d7959a8aeb5f2b5d46fc369721451fc3e4693) )
	ROM_LOAD32_WORD("2j015711.u73", 0x0000002, 0x0400000, CRC(aa321624) SHA1(8c0e7e84c0dc7a2a61eb8d4777e5e937c1622730) )
	ROM_LOAD32_WORD("2j015711.u85", 0x0800000, 0x0400000, CRC(aa2ebf24) SHA1(fc4bc1d9b201defa04cdcfaf12c79e67496c233c) )
	ROM_LOAD32_WORD("2j015711.u72", 0x0800002, 0x0400000, CRC(cc84661f) SHA1(85fca4517b6fbbe83c4f5a5159af95c7e1089849) )
	ROM_LOAD32_WORD("2j015711.u84", 0x1000000, 0x0400000, CRC(cc84cd2f) SHA1(1939b7a40de44de5d026f2e4252f9d2b33d44012) )
	ROM_LOAD32_WORD("2j015711.u71", 0x1000002, 0x0400000, CRC(768ff8fb) SHA1(a99a750ee4bb761a21c264133ce7ea7ef344578f) )
ROM_END


// 10126721 / 704
// Dream Catcher / Local / F - 23/06/03
// Variations (% and NO):
//    87.51% 99
//    89.86% 01
ROM_START( drmcatch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10126721.u86", 0x0000000, 0x0400000, CRC(82645450) SHA1(17edb9f2b61f38ca6ced060d37d4294b42b934ac) )
	ROM_LOAD32_WORD("10126721.u73", 0x0000002, 0x0400000, CRC(4193c9f8) SHA1(de9747896f299f8562a8e836e28fcce991154194) )
	ROM_LOAD32_WORD("10126721.u85", 0x0800000, 0x0400000, CRC(5b639f5c) SHA1(b3a2f5797071ae57b0bc541f23d6e62df173e4cd) )
	ROM_LOAD32_WORD("10126721.u72", 0x0800002, 0x0400000, CRC(46244616) SHA1(3e16e7dd6f5afba135c3f1b9b40b7cfecc0d83e4) )
ROM_END


// 10172921 / 704
// Dream Catcher / ASP / H - 04/03/04
// Variations (% and NO):
//    87.51% 99
ROM_START( drmcatcha )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10172921.u86", 0x0000000, 0x0400000, CRC(a7371abd) SHA1(bfdb09eb8316cde47ec917aff0aedba82f5e95ab) )
	ROM_LOAD32_WORD("10172921.u73", 0x0000002, 0x0400000, CRC(449d9b85) SHA1(631826ed0966b466ffd2b8fbaa08f2bb59e6483b) )
	ROM_LOAD32_WORD("10172921.u85", 0x0800000, 0x0400000, CRC(080bd46c) SHA1(808733d86a237ffb5fc530d9cd46e4a1eef76d61) )
	ROM_LOAD32_WORD("10172921.u72", 0x0800002, 0x0400000, CRC(f406541f) SHA1(6954e3b89bbb801948d6ef54b78622e2c9e1ab18) )
ROM_END


// 10273811 / JB027/2
// Dream of the Red Chamber / A - 25/8/08
// Variations (% and NO):
//    87.87% 99
//    90.31% 01
//    92.26% 02
ROM_START( drmred )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10273811.u86", 0x0000000, 0x0400000, CRC(69e4a089) SHA1(505de7f61ff74d73da32aaf105b1ff9d5083094a) )
	ROM_LOAD32_WORD("10273811.u73", 0x0000002, 0x0400000, CRC(a38db22b) SHA1(e83485525a46741a7d97c36341c529d9643f2dd9) )
	ROM_LOAD32_WORD("10273811.u85", 0x0800000, 0x0400000, CRC(b2640b58) SHA1(79e8085ae36a2f03a7e57e21cccaeb4bbe122f8e) )
	ROM_LOAD32_WORD("10273811.u72", 0x0800002, 0x0400000, CRC(5a4e369a) SHA1(f50424edae6e912ef6d40f2e2e769c2002349c0a) )
ROM_END


// 0151126 / US021
// Eagle Rock CE / Local / C - 5/25/04
// Cash Express Hyperlink game
// Variations (% and NO):
//    87.813% 99
//    90.115% 01
//    92.730% 02
//    94.720% 03
//    97.302% 04
//    82.137% 05 (Hyperlink)
ROM_START( eaglrock )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151126.u86", 0x0000000, 0x0400000, CRC(0b0cff73) SHA1(ea1bbf9e00a5308c9e853e357f9ac4b32866a195) )
	ROM_LOAD32_WORD("0151126.u73", 0x0000002, 0x0400000, CRC(ce3d4fd2) SHA1(ef784c2ae5151c7191cac66093cbc1f85155a4e9) )
	ROM_LOAD32_WORD("0151126.u85", 0x0800000, 0x0400000, CRC(a3f768e9) SHA1(1b0b96368530d51280da81479e9a3b44fb719503) )
	ROM_LOAD32_WORD("0151126.u72", 0x0800002, 0x0400000, CRC(c3ac8cfd) SHA1(ff2b45f49929dfa9aed44af2a1b204a96ed098fb) )
ROM_END


// 40289711 / 950
// Eagle Strike / A - 20/07/10
// Variations (% and NO):
//    85.18% 99
//    87.68% 01
//    89.18% 02
ROM_START( estrike )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40289711.u11", 0x0000000, 0x0400000, CRC(71c8121b) SHA1(1a4534b8e43dbc173809b5a621d6796a7a8655cf) )
	ROM_LOAD32_WORD("40289711.u21", 0x0000002, 0x0400000, CRC(94ff83d2) SHA1(54a02cdfb205a429d7458ecb51ba20083306011e) )
	ROM_LOAD32_WORD("40289711.u12", 0x0800000, 0x0400000, CRC(c7430ac8) SHA1(e6f7e523ef2c4aad584a08ce9c70e0e535d89890) )
	ROM_LOAD32_WORD("40289711.u22", 0x0800002, 0x0400000, CRC(4d96aef2) SHA1(40860ad6a0f82d79347b65b182f72dfbec23cd74) )
	ROM_LOAD32_WORD("40289711.u13", 0x1000000, 0x0400000, CRC(664c3d45) SHA1(26ce8a7e88eddc94ecce5ada78654e6f30a985d6) )
	ROM_LOAD32_WORD("40289711.u23", 0x1000002, 0x0400000, CRC(7311d99c) SHA1(802abed830ca3204ce50072ac69d0a3cae8f17e5) )
	ROM_LOAD32_WORD("40289711.u14", 0x1800000, 0x0400000, CRC(d70198eb) SHA1(1e4c157deec921386623091b27180c9c53551f44) )
	ROM_LOAD32_WORD("40289711.u24", 0x1800002, 0x0400000, CRC(b899e64b) SHA1(1f1d800df39de444b03ff4cc85bf4cf073f88423) )
ROM_END


// 0151117 / US052
// EASTER ISLAND / Local / B - 02/22/04
// Cash Express Hyperlink game
// Variations (% and NO):
//    87.773% 99
//    90.693% 01
//    92.699% 02
//    94.839% 03
//    97.349% 04
//    82.627% 05 (Hyperlink)
ROM_START( eastland )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151117.u86", 0x0000000, 0x0400000, CRC(f24bcc2d) SHA1(519d06b85b7c3a4025f029daa394423bc6e3b4a2) )
	ROM_LOAD32_WORD("0151117.u73", 0x0000002, 0x0400000, CRC(99591f11) SHA1(9d08d339ecb19682ace66f58b8aa98b62d958bec) )
	ROM_LOAD32_WORD("0151117.u85", 0x0800000, 0x0400000, CRC(1fb075e7) SHA1(f08f43e19e84b897f0c350a0b86431939947d3b0) )
	ROM_LOAD32_WORD("0151117.u72", 0x0800002, 0x0400000, CRC(16159d21) SHA1(e3b15c84d2a6ef9b840456cecf290798b6e6961d) )
ROM_END


// 0351149 / US144
// Eggstreme Riches CM / Local / B - 11/3/05
// Variations (% and NO):
//    92.378% 99
//    92.344% 01
ROM_START( eggsrccm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351149.u86", 0x0000000, 0x0400000, CRC(9d3476d1) SHA1(983ababf1f8d65346c4cf89b7ac817156a64a636) )
	ROM_LOAD32_WORD("0351149.u73", 0x0000002, 0x0400000, CRC(d72f0fb0) SHA1(eedebb24d23899f4caffe7c79ae4ac51856df1b3) )
	ROM_LOAD32_WORD("0351149.u85", 0x0800000, 0x0400000, CRC(2cbd9e98) SHA1(dcd5df5a3c177992d13077fa434ef741e7607d89) )
	ROM_LOAD32_WORD("0351149.u72", 0x0800002, 0x0400000, CRC(883274fd) SHA1(60e65e2f7f1774eac4520f5d96dbcd149ee69b91) )
ROM_END


// 0151084
// Enchanted Forest CE
// Cash Express Hyperlink game
ROM_START( efce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151084.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0151084.u73", 0x0000002, 0x0400000, CRC(d5c600ba) SHA1(e2eb729e29d66ba5614fce1f4bd5a4883b50c499) )
	ROM_LOAD32_WORD("0151084.u85", 0x0800000, 0x00c0000, BAD_DUMP CRC(95cf5a7b) SHA1(ac5fbd83e2d73a1d6d55736d868c44396371e24e) ) // bad? this seems to contains all the necessary data: maybe it was padded up to 0x0400000 and the padding was cut?
	ROM_LOAD32_WORD("0151084.u72", 0x0800002, 0x0400000, CRC(f3ec21f4) SHA1(db9d98fe4788f27c33e84b612632feeea6d944c7) )
ROM_END


// 10257011 / JB041
// Enchanted Spell / F - 01/05/07
// B - 18/07/07 (alternate string)
// Variations (% and NO):
//    88.00% 99
//    90.13% 01
//    92.29% 02
ROM_START( encspell )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10257011.u86", 0x0000000, 0x0400000, CRC(1b65e345) SHA1(2eba34cbbdbf939bb2241c95e610ee4d97ef6258) )
	ROM_LOAD32_WORD("10257011.u73", 0x0000002, 0x0400000, CRC(969cf792) SHA1(0c44ed0ab3a5c11a171e7615ced9234bbbd9f2c9) )
	ROM_LOAD32_WORD("10257011.u85", 0x0800000, 0x0400000, CRC(a6a4bcf2) SHA1(5ee43d090d7505db2be59064bb1fdbcf3b29b16b) )
	ROM_LOAD32_WORD("10257011.u72", 0x0800002, 0x0400000, CRC(651cb916) SHA1(caae7e9122b51352eb0befbc422c20866caeb92c) )
	ROM_LOAD32_WORD("10257011.u84", 0x1000000, 0x0400000, CRC(5d351e99) SHA1(ad492bc56d804a298c05f67746f3e1664084a0f2) )
	ROM_LOAD32_WORD("10257011.u71", 0x1000002, 0x0400000, CRC(7f91f73f) SHA1(5b70ffe30047d68bf521ee773ff3c323d1678e83) )
ROM_END


// 20000711 / 633
// Enchantress / Local / C- 15/01/00
// A- 15/01/00 (alternate string)
// Variations (% and NO):
//    87.18% 99
//    90.00% 01
//    92.20% 02
ROM_START( enchantr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20000711.u86", 0x0000000, 0x0400000, CRC(17a7db59) SHA1(f948369144791d6fd761efbbe5fb37b765b637e3) )
	ROM_LOAD32_WORD("20000711.u73", 0x0000002, 0x0400000, CRC(dcaaec40) SHA1(f035e54d9c926826c7883838a65260fc5ca6c02c) )
ROM_END


// 10002011 / 658
// Eyes of the Tiger / Local / B - 01/06/00
// Variations (% and NO):
//    87.85% 99 (9 line)    87.85% 99 (20 line)
//    89.81% 01 (9 line)    90.04% 01 (20 line)
//    92.02% 02 (9 line)    92.01% 02 (20 line)
ROM_START( eott )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10002011.u86", 0x0000000, 0x0400000, CRC(dae52614) SHA1(4a80d1a20f79b25e0f47c40ba05645d60b598568) )
	ROM_LOAD32_WORD("10002011.u73", 0x0000002, 0x0400000, CRC(63d43c24) SHA1(8f76d2a7f9f5c0eb89609f891255e584b3a417d5) )
	ROM_LOAD32_WORD("10002011.u85", 0x0800000, 0x0400000, CRC(d941636d) SHA1(7103599caa30d57a1fa038afd7c60c684ba9dc5a) )
	ROM_LOAD32_WORD("10002011.u72", 0x0800002, 0x0400000, CRC(0ccf36e4) SHA1(6b185113a909985730cf1a58d1179d0ffc269d40) )
ROM_END


// 20184911 / 772
// Fire Dancer / Local / A - 01/10/04
// Variations (% and NO):
//    87.81% 99
//    90.08% 01
ROM_START( firednc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20184911.u86", 0x0000000, 0x0400000, CRC(be5673c6) SHA1(68a5c6515945c8341479cae3166d8ed0915ec566) )
	ROM_LOAD32_WORD("20184911.u73", 0x0000002, 0x0400000, CRC(30b0f1a0) SHA1(9095f3848514e5a7ab5528fef701a274156d4bce) )
	ROM_LOAD32_WORD("20184911.u85", 0x0800000, 0x0400000, CRC(f05c2ebe) SHA1(7b196523a613955a3f4fec70e04c642bc1184dde) )
	ROM_LOAD32_WORD("20184911.u72", 0x0800002, 0x0400000, CRC(de394e4c) SHA1(9ec430df94de2cc0dbc8cc3a8763fde257fe3477) )
ROM_END


// 10191311 / 772
// Fire Dancer / ASP / B - 01/03/05
// Variations (% and NO):
//    87.81% 99
ROM_START( firednca )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10191311.u86", 0x0000000, 0x0400000, CRC(b9d203ad) SHA1(19ecd5c41cab6ef50e4fb08a66c02c20812cd30c) )
	ROM_LOAD32_WORD("10191311.u73", 0x0000002, 0x0400000, CRC(3938ef29) SHA1(893c265a56cfad61d21c66fd6c885fa726cf4419) )
	ROM_LOAD32_WORD("10191311.u85", 0x0800000, 0x0400000, CRC(aa7e7675) SHA1(fcb41ebf365aa1d24388404d52b036e136ac5967) )
	ROM_LOAD32_WORD("10191311.u72", 0x0800002, 0x0400000, CRC(a412f587) SHA1(4400a9ff3bf7b92d830c33652b7f3a37b39c377b) )
ROM_END


// 0351025 / MV4091/3
// Flame of Olympus / USA / A - 2/18/02
// Cash Express Hyperlink game
// Variations (% and NO):
//    87.928% 99
//    90.511% 01
//    92.161% 02
//    94.941% 03
//    97.325% 04
//    82.070% 05 (Hyperlink)
ROM_START( flamolce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0351025-U86 Series:01 M27V322 Mk6USA   Flame of Olympus CE   72E9294A   ACD9-6A8C / 1E2A1492 / 2CBC6195   Copyright © 2002 Aristocrat Technologies, Inc.
	0351025-U73 Series:01 M27V322 Mk6USA   Flame of Olympus CE   2C2969B4   3F77-21C4 / 1DCC4068 / 128F13CE   Copyright © 2002 Aristocrat Technologies, Inc.
	0351025-U85 Series:01 M27V322 Mk6USA   Flame of Olympus CE   E8A85U5H   5916-199F / 09EFD2C9 / 1F74BC40   Copyright © 2002 Aristocrat Technologies, Inc.
	0351025-U72 Series:01 M27V322 Mk6USA   Flame of Olympus CE   07D39U7P   F52E-2598 / 09ECD2D8 / 1EC86D48   Copyright © 2002 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0351025.u86", 0x0000000, 0x0400000, CRC(f06f3d69) SHA1(83e1d2f72275de836e170c1c10905cb6da58b90a) )
	ROM_LOAD32_WORD("0351025.u73", 0x0000002, 0x0400000, CRC(aad37086) SHA1(76495ba46da0ae848ba290706be337c45afe6361) )
	ROM_LOAD32_WORD("0351025.u85", 0x0800000, 0x0400000, CRC(4fcd025d) SHA1(63aa8d0f5dd331e10c032cc5ac67516054db7c27) )
	ROM_LOAD32_WORD("0351025.u72", 0x0800002, 0x0400000, CRC(b4581c28) SHA1(1daa0f1218dba6e6eb96b7925ea71be531ffa787) )
ROM_END


// 0251048 / MV4091/3
// Flame of Olympus / USA / A - 2/18/02
// Same date string as flamolce
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    87.928% 99
//    90.511% 01
//    92.161% 02
//    94.941% 03
//    97.325% 04
//    82.070% 05 (Hyperlink)
ROM_START( flamoljc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251048.u86", 0x0000000, 0x0400000, CRC(2ea7c6d9) SHA1(cd887f0689737fd447786b966297ea3994fb8cb7) )
	ROM_LOAD32_WORD("0251048.u73", 0x0000002, 0x0400000, CRC(c8337682) SHA1(e87c5be62528e2e73659ce73a17d1ea21ed3ccf0) )
	ROM_LOAD32_WORD("0251048.u85", 0x0800000, 0x0400000, CRC(2661893a) SHA1(4a79f0b638f5d4fdfa7dcc7661afe3ae1823e6ce) )
	ROM_LOAD32_WORD("0251048.u72", 0x0800002, 0x0400000, CRC(330c1988) SHA1(0d7dae49495c5b164121cd756d6e22af17a9b41b) )
ROM_END


// 20001411 / 659
// Follow the Stars / Local / D - 15/09/00
// Variations (% and NO):
//    87.20% 99
//    90.11% 01
//    92.15% 02
ROM_START( followst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20001411.u86", 0x0000000, 0x0400000, CRC(aa3d76aa) SHA1(94350d0efb49467febd1500dbb697aa865d43843) )
	ROM_LOAD32_WORD("20001411.u73", 0x0000002, 0x0400000, CRC(a1037235) SHA1(3badc901f8d1488cb630b07adc18cc280bd1c111) )
ROM_END


// 10230911 / 744
// Fortune King / ASP / I - 26/09/06
// Variations (% and NO):
//    88.54% 99
ROM_START( fortking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U86
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U73
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U85
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U72
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U84
	10230911   ASP   FORTUNE KING   744   4MB   4 eproms   Aristocrat © 2006   P  U71
	*/
	ROM_LOAD32_WORD("10230911.u86", 0x0000000, 0x0400000, CRC(7cb86c8d) SHA1(781f09c9026f7241efc9aee0d262677bab09fe04) )
	ROM_LOAD32_WORD("10230911.u73", 0x0000002, 0x0400000, CRC(46f6427f) SHA1(2c9d87298cb35d36ddb8226132742ae4776cd665) )
	ROM_LOAD32_WORD("10230911.u85", 0x0800000, 0x0400000, CRC(cd08b3e1) SHA1(2d2189f335376cc882319b583512240e7c3316f8) )
	ROM_LOAD32_WORD("10230911.u72", 0x0800002, 0x0400000, CRC(d8b87f17) SHA1(2487ec62a4a9644b9e30cfd3d5877cf03b29ca5b) )
ROM_END


// 10248611 / 847
// Fortune of Athena / C - 08/11/07
// Variations (% and NO):
//    87.94% 99
//    90.18% 01
ROM_START( fortath )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10248611.u86", 0x0000000, 0x0400000, CRC(39274d5c) SHA1(ef01c3f010800ece44ccb50be7cf14af538e21fc) )
	ROM_LOAD32_WORD("10248611.u73", 0x0000002, 0x0400000, CRC(4f361938) SHA1(51c23d09ba7cc7c73559e10b023962e7eb7059ed) )
	ROM_LOAD32_WORD("10248611.u85", 0x0800000, 0x0400000, CRC(0b33c40b) SHA1(055ac1528a2008a4b481db201b95dabf8531d6e4) )
	ROM_LOAD32_WORD("10248611.u72", 0x0800002, 0x0400000, CRC(90e481a0) SHA1(b73045e1f8602f17e6d0f9d9f5718c7e7cd0dd81) )
ROM_END


// 0251114 / US112
// Frog Wild / USA / A - 06/03/04
// 'Frog Wild 2' on artwork, 'Frog Wild' in ROM
// Variations (% and NO):
//    87.654% 99
//    90.520% 01
//    92.233% 02
//    95.261% 03
//    97.041% 04
ROM_START( frogwld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251114.u86", 0x0000000, 0x0400000, CRC(feb93395) SHA1(56c0f9b0e3616bc57a564220303c13f60fc72df8) )
	ROM_LOAD32_WORD("0251114.u73", 0x0000002, 0x0400000, CRC(f058f8f3) SHA1(02efb3de7765e917fb6c29cbfdd90845fb3f0a7b) )
	ROM_LOAD32_WORD("0251114.u85", 0x0800000, 0x0400000, CRC(b4165c0f) SHA1(823179a70841f49c8100d8d77620548ceea01550) )
	ROM_LOAD32_WORD("0251114.u72", 0x0800002, 0x0400000, CRC(36f8066f) SHA1(c8a3dc25488d7d578e05bf247b2d3e0959f5c511) )
ROM_END


// 20197211 / 791
// Gamblin' Jack / Local / D - 23/09/05
// Variations (% and NO):
//    87.99% 99
//    90.10% 01
ROM_START( gambjack )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197211.u86", 0x0000000, 0x0400000, CRC(d55c8c10) SHA1(fb876c0fe04785d01baef1debe5a16f4d2e1587f) )
	ROM_LOAD32_WORD("20197211.u73", 0x0000002, 0x0400000, CRC(80eeb208) SHA1(ad38d08993457a344ed5da9b6335e9a933b11a67) )
ROM_END


// 0452501 / MV4210
// The Game With No Name / Local / C - 09/13/04
// Variations (% and NO):
//    87.834% 99
//    90.000% 01
//    91.816% 02
//    94.584% 03
ROM_START( gwnn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0452501.u86", 0x0000000, 0x0400000, CRC(2b6407cf) SHA1(ac9242ebd8e669e84664e9aa0f3ec1eb15049a3e) )
	ROM_LOAD32_WORD("0452501.u73", 0x0000002, 0x0400000, CRC(3644eacc) SHA1(115b7a7ddf9ca1c82aae7c8dcf864d3c224e93be) )
	ROM_LOAD32_WORD("0452501.u85", 0x0800000, 0x0400000, CRC(3b95fa30) SHA1(09e53a4434950212346f447315b41da9216a2e24) )
	ROM_LOAD32_WORD("0452501.u72", 0x0800002, 0x0400000, CRC(68b68c92) SHA1(3c23e02ba9cc31352df19b3f641191a4e6261e7b) )
ROM_END


// 10000811 / 602/18
// Geisha / Local / B- 13/06/00
// Variations (% and NO):
//    87.87% 99
//    90.31% 01
//    92.26% 02
ROM_START( geishaa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000811.u86", 0x0000000, 0x0400000, CRC(994e2e17) SHA1(a8b40aeb98f4e80a3eb412927f23b4292a7dbf6e) )
	ROM_LOAD32_WORD("10000811.u73", 0x0000002, 0x0400000, CRC(544c2cd6) SHA1(3904bb39096dd66f3579b7f244e8872fe29f4f72) )
	ROM_LOAD32_WORD("10000811.u85", 0x0800000, 0x0400000, CRC(02c7819e) SHA1(f553adf6975c6fafff61e30605531b91ae5598af) )
	ROM_LOAD32_WORD("10000811.u72", 0x0800002, 0x0400000, CRC(1efb986d) SHA1(34a5964e0eada085ff738ee4853beb75c431aaaa) )
ROM_END


// 10122011 / 602/18
// Geisha / ASP Game / D- 14/03/01
// Variations (% and NO):
//    87.870% 99
ROM_START( geishaa6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122011.u86", 0x0000000, 0x0400000, CRC(2d662f53) SHA1(75aacf41b0fc297511995a207fb31298a64bdaa0) )
	ROM_LOAD32_WORD("10122011.u73", 0x0000002, 0x0400000, CRC(e197c69b) SHA1(c92bd95b4df1c35be92176e8777e5efe9d8adc14) )
	ROM_LOAD32_WORD("10122011.u85", 0x0800000, 0x0400000, CRC(eda126db) SHA1(acc77bbbdb54a9229d342580f388e9db5cce47cf) )
	ROM_LOAD32_WORD("10122011.u72", 0x0800002, 0x0400000, CRC(440c56e6) SHA1(1328b129a291df105be6df9ec0002f6929919e97) )
ROM_END


// 10112411 / 602/18
// Geisha / ASP Game / D- 14/03/01
// Same date string as geishaa6a
// Variations (% and NO):
//    87.870% 99
ROM_START( geishaa6b )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10112411.u86", 0x0000000, 0x0400000, CRC(6b107e0b) SHA1(818ce3a1cba49293a12c6d904552b0945b35e58d) )
	ROM_LOAD32_WORD("10112411.u73", 0x0000002, 0x0400000, CRC(4976ca1c) SHA1(27d17dae233df3f67dbcf62a2ed57304cf93db97) )
	ROM_LOAD32_WORD("10112411.u85", 0x0800000, 0x0400000, CRC(e5404d66) SHA1(39fd9669427aa126da7f97c0b800c0786c5681ac) )
	ROM_LOAD32_WORD("10112411.u72", 0x0800002, 0x0400000, CRC(c59106fe) SHA1(655fe31254dc2b72beafde3365708f966c8bcd40) )
ROM_END


// 0351033 / MV4091/2
// Geisha / USA / B - 10/18/01
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    87.928% 99
//    90.511% 01
//    92.161% 02
//    94.941% 03
//    97.325% 04
//    82.070% 05 (Hyperlink)
ROM_START( geishajc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0351033-U86 Series:01 M27V322 Mk6USA   Geisha - Jackpot Carnival   L02-2312-01   0U312F90   5A6A-4F05 / 1E64E4D0 / 4A040BCF   Copyright © 2002 Aristocrat Technologies, Inc.
	0351033-U73 Series:01 M27V322 Mk6USA   Geisha - Jackpot Carnival   L02-2312-02   572HC27U   11EC-480E / 1E064DFF / 2FF8A594   Copyright © 2002 Aristocrat Technologies, Inc.
	0351033-U85 Series:01 M27V322 Mk6USA   Geisha - Jackpot Carnival   L02-2312-03   06AP4257   9804-7CC4 / 114ABE71 / EEE402AD   Copyright © 2002 Aristocrat Technologies, Inc.
	0351033-U72 Series:01 M27V322 Mk6USA   Geisha - Jackpot Carnival   L02-2312-04   D7BA0F9U   69C4-5083 / 1140D1D5 / EB03857C   Copyright © 2002 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0351033.u86", 0x0000000, 0x0400000, CRC(620d8a40) SHA1(601a66e59a1db8be84b494eba24148ab50c674d5) )
	ROM_LOAD32_WORD("0351033.u73", 0x0000002, 0x0400000, CRC(173105e7) SHA1(7c6c082782b7a7f677e6edbba23a09ae9a922415) )
	ROM_LOAD32_WORD("0351033.u85", 0x0800000, 0x0400000, CRC(879f061c) SHA1(e8f2650a486799880e7261f3d4e3d0f6f65f3dfe) )
	ROM_LOAD32_WORD("0351033.u72", 0x0800002, 0x0400000, CRC(7ab7b3f0) SHA1(6907876ff3597e2476d105939b09e7ce88eca2a8) )
ROM_END


// 10006211 / 661/2
// Go Ape / Local / B - 14/06/01
// Variations (% and NO):
//    87.16% 99
//    90.08% 01
//    92.07% 02
ROM_START( goape )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10006211.u86", 0x0000000, 0x0400000, CRC(392ed11e) SHA1(4fef477254b75c37a282a77aaf3046855118bdc0) )
	ROM_LOAD32_WORD("10006211.u73", 0x0000002, 0x0400000, CRC(d579d719) SHA1(e3679657ed2f27b52a762a7f424714e5080a6957) )
ROM_END


// 10003411 / 647/1
// Go For Green / Local / A - 22/02/01
// Variations (% and NO):
//    87.97% 99
//    90.12% 01
//    92.10% 02
ROM_START( gogreen )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10003411.u86", 0x0000000, 0x0400000, CRC(17f801b4) SHA1(c2044c445694fb0755add29e4a4a0ccb9bf36442) )
	ROM_LOAD32_WORD("10003411.u73", 0x0000002, 0x0400000, CRC(279da15c) SHA1(967063011447aeb4563878c13da3cf60f3f8b456) )
	ROM_LOAD32_WORD("10003411.u85", 0x0800000, 0x0400000, CRC(57f68a9a) SHA1(8e7075e2b532a0c5ad0fd9d9f815a764cb9bb842) )
	ROM_LOAD32_WORD("10003411.u72", 0x0800002, 0x0400000, CRC(e138958c) SHA1(4416c4e00e388e8c84f9d82949c1875a5b34655f) )
ROM_END


// 10122111
// Go For Green / ASP
ROM_START( gogreena )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122111.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10122111.u73", 0x0000002, 0x0400000, CRC(72bc7d58) SHA1(5d65ec7ae7705575b352782d017d2ed62b80286f) )
	ROM_LOAD32_WORD("10122111.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10122111.u72", 0x0800002, 0x0400000, NO_DUMP )
ROM_END


// 10197411 / 770
// Golden Archer / Local / A - 07/07/05
// Variations (% and NO):
//    87.90% 99
//    90.10% 01
//    91.91% 02
ROM_START( goldarch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10197411.u86", 0x0000000, 0x0400000, CRC(00858fad) SHA1(8467ed9898d2234ba48e9896b8dd20afa75c5f69) )
	ROM_LOAD32_WORD("10197411.u73", 0x0000002, 0x0400000, CRC(edaaadf1) SHA1(3d68ba211c1904e7ea35d679c9f8845ef1e4ea90) )
	ROM_LOAD32_WORD("10197411.u85", 0x0800000, 0x0400000, CRC(dc2c1319) SHA1(8df96bf7b96a3c7df0a155420959d939daedf5c4) )
	ROM_LOAD32_WORD("10197411.u72", 0x0800002, 0x0400000, CRC(f6d262c6) SHA1(09e5f1e3838921482b01005e8781e83f08a9bd75) )
ROM_END


// 10158311 / 680/2
// Golden Fleece / Local / A- 07/07/03
// 'The Golden Fleece' on artwork, 'Golden Fleece' in ROM
// Variations (% and NO):
//    87.70% 99
//    90.08% 01
//    92.08% 02
ROM_START( gldnflc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10158311.u86", 0x0000000, 0x0400000, CRC(c3d4d641) SHA1(b46d82af862c96068ddd4f530b2229634161b209) )
	ROM_LOAD32_WORD("10158311.u73", 0x0000002, 0x0400000, CRC(35e72b56) SHA1(c6013aa3b6ea243f057aedab67feeed16ee39537) )
	ROM_LOAD32_WORD("10158311.u85", 0x0800000, 0x0400000, CRC(f17c5c40) SHA1(32225a11d529c0312b27ed838badebed7d96469b) )
	ROM_LOAD32_WORD("10158311.u72", 0x0800002, 0x0400000, CRC(521e2429) SHA1(0cfadf64341574eaec6fc89ecccfa2f02c583dd1) )
ROM_END


// 20196011 / 710/1
// The Golden Gong / ASP / ? - 12/05/05
// Variations (% and NO):
//    87.99% 99
ROM_START( gldgong )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20196011.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20196011.u73", 0x0000002, 0x0400000, CRC(9786e94b) SHA1(32c5039dc7170c65b451209cd6fec184b5288d84) )
	ROM_LOAD32_WORD("20196011.u85", 0x0800000, 0x0400000, CRC(835a1638) SHA1(a87dde1fad742919213045ffc4ffd529fe6bf0f1) )
	ROM_LOAD32_WORD("20196011.u72", 0x0800002, 0x0400000, CRC(71647499) SHA1(59693285494498fd13761c4fd0c488e29af5b4ed) )
ROM_END


// 10217411 / 710/3
// The Golden Gong / QCOM / C - 09/08/05
// Variations (% and NO):
//    87.91% 99
ROM_START( gldgongq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10217411.u86", 0x0000000, 0x0400000, CRC(e2d0c057) SHA1(9311faeb430eaae41b6c42711cf87e3824a88933) )
	ROM_LOAD32_WORD("10217411.u73", 0x0000002, 0x0400000, CRC(4be87efb) SHA1(6775853e14f771f784ac05d1a63534cd1aedca21) )
ROM_END


// 0152558 / MV4258
// The Golden Gong / Local / C - 08/18/05
// Variations (% and NO):
//    87.979% 99
//    90.160% 01
//    91.967% 02
//    94.930% 03
//    97.022% 04
ROM_START( gldgongu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152558-U86 Series 01 M27V322 Mk6USA   Golden Gong SSAP   L05-3302-01   375C61EF   CEEF-F17D / 1E15296C / 302C73E9   Copyright © 2005 Aristocrat Technologies, Inc.
	0152558-U73 Series 01 M27V322 Mk6USA   Golden Gong SSAP   L05-3302-02   6A44CAF0   851C-78CB / 1DEBB4F0 / 2498EFF2   Copyright © 2005 Aristocrat Technologies, Inc.
	0152558-U85 Series 01 M27V322 Mk6USA   Golden Gong SSAP   L05-3302-04   CC466205   4AAF-2365 / 05FDAD90 / EB4C0205   Copyright © 2005 Aristocrat Technologies, Inc.
	0152558-U72 Series 01 M27V322 Mk6USA   The Golden Gong  SSAP   L05-3302-03   F4CPDP81   FB56-8A43 / 05CB4CC9 / DE29D408   Copyright © 2005 Aristocrat Technologies, Inc.

	Mismatched labels? Last one has a "The" prefix, the others don't
	*/
	ROM_LOAD32_WORD("0152558.u86", 0x0000000, 0x0400000, CRC(9e825d94) SHA1(642edc067c651b127890324fcbd947124fd9a07e) )
	ROM_LOAD32_WORD("0152558.u73", 0x0000002, 0x0400000, CRC(de1a91f6) SHA1(6bc640cd5b2ffebed7ef4b43c980f10eb22ec5ba) )
	ROM_LOAD32_WORD("0152558.u85", 0x0800000, 0x0400000, CRC(29557de6) SHA1(1e7f03ca41d510561e22b944a46b98d00abfd29c) )
	ROM_LOAD32_WORD("0152558.u72", 0x0800002, 0x0400000, CRC(86ca7b2b) SHA1(938dcbc846d0f87e374d6c835ac6cb8185faf48c) )
ROM_END


// 10174011 / 752
// Golden Incas / Local / A - 07/04/04
// SHOW PROGRAM
// Variations (% and NO):
//    87.82% 99
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( goldinc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10174011.u86", 0x0000000, 0x0400000, CRC(c4f488f9) SHA1(0db96a71f361f80f8e7265d1873728fc3013c204) )
	ROM_LOAD32_WORD("sp__10174011.u73", 0x0000002, 0x0400000, CRC(7a88f44f) SHA1(586cbcf25eedcae538aaa3c3f8c7700778180295) )
	ROM_LOAD32_WORD("sp__10174011.u85", 0x0800000, 0x0400000, CRC(dde914b2) SHA1(f35b761dd89796f62fec40d48c0223f585247de0) )
	ROM_LOAD32_WORD("sp__10174011.u72", 0x0800002, 0x0400000, CRC(8629d154) SHA1(bb4e679188ec8f58e8e227804c36d1e61e575efe) )
ROM_END


// 0252533 / MV4244
// Golden Incas / Local / A - 03/10/05
// Variations (% and NO):
//    87.862% 99
//    90.052% 01
//    91.827% 02
//    94.659% 03
//    97.228% 04
ROM_START( goldincu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252533.u86", 0x0000000, 0x0400000, CRC(5e53d9af) SHA1(e850e1707be120b9d6a23524c65e63efb20d1cf1) )
	ROM_LOAD32_WORD("0252533.u73", 0x0000002, 0x0400000, CRC(768cecbb) SHA1(2cd2b5906c0a45aecaca2d26063b2ce62ca588eb) )
	ROM_LOAD32_WORD("0252533.u85", 0x0800000, 0x0400000, CRC(cb1268de) SHA1(dc5a18736ac76c59811c7bccd7c3a4cd468fccec) )
	ROM_LOAD32_WORD("0252533.u72", 0x0800002, 0x0400000, CRC(ce66ccd2) SHA1(71f204ecfd0e31b071cde59bbd974e489ee1f570) )
ROM_END


// 10196511 / 602/23
// Golden Pyramids / ASP / C - 23/11/04
// Variations (% and NO):
//    87.87% 99
ROM_START( goldpya6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10196511.u86", 0x0000000, 0x0400000, CRC(b506b9a5) SHA1(fa00b5a8f3a6937b04e201955566ef9db0415c3f) )
	ROM_LOAD32_WORD("10196511.u73", 0x0000002, 0x0400000, CRC(0a7c47c6) SHA1(5b44bfcbe4cf747f5b412c7970a7a28e94343720) )
ROM_END


// 10130811 / 683/1
// Golden Pyramids Special Edition / Local / A - 21/10/02
// Maximillions Hyperlink game
// Variations (% and NO):
//    82.05% 05 (Hyperlink)
ROM_START( goldpym )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130811.u86", 0x0000000, 0x0400000, CRC(57b5488a) SHA1(d09915be7dee48570e79436eff55cabad58b783b) )
	ROM_LOAD32_WORD("10130811.u73", 0x0000002, 0x0400000, CRC(f20e603e) SHA1(5577934713be6996057a2fe62c8988635c842b76) )
	ROM_LOAD32_WORD("10130811.u85", 0x0800000, 0x0400000, CRC(f2e1763f) SHA1(86d8d96c016bf88c8db9eae3de1bd9c64f517189) )
	ROM_LOAD32_WORD("10130811.u72", 0x0800002, 0x0400000, CRC(be3c43ba) SHA1(23ac49737061690a814e9a54a1f32290621bdaaf) )
ROM_END


// 10247811 / 832
// Golden Samurai / A - 31/07/07
// Variations (% and NO):
//    90.67% 99
//    92.10% 01
//    93.46% 02
ROM_START( goldsam )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10247811.u86", 0x0000000, 0x0400000, CRC(b888e6ab) SHA1(a3df344fa1a1419ef4f4225e808189d4acffdc86) )
	ROM_LOAD32_WORD("10247811.u73", 0x0000002, 0x0400000, CRC(dc0c374b) SHA1(5e4b1800aa69863aa537aec72285e2ffaccf0aee) )
	ROM_LOAD32_WORD("10247811.u85", 0x0800000, 0x0400000, CRC(d1f22168) SHA1(a63e6856f73fc2642edd59b18282ad69e32b3310) )
	ROM_LOAD32_WORD("10247811.u72", 0x0800002, 0x0400000, CRC(33967413) SHA1(ceb6aa9937dc5087298dfad8f3b2edc5cc8dd8a1) )
	ROM_LOAD32_WORD("10247811.u84", 0x1000000, 0x0400000, CRC(7c9b075a) SHA1(9d0095d2219747610c060b99b612d3a1353ed561) )
	ROM_LOAD32_WORD("10247811.u71", 0x1000002, 0x0400000, CRC(b5d9eb6f) SHA1(c614073463e60cc3fe379b81215324fa9e150948) )
ROM_END


// 0351016 / US023
// Good, Bad, Money / USA / A - 31/07/07
// 'The Good, The Bad and The Money' on artwork, 'Good, Bad, Money' in ROM
// Variations (% and NO):
//    87.545% 99
//    90.497% 01
//    92.545% 02
//    94.617% 03
//    97.388% 04
ROM_START( gbmoney )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351016.u86", 0x0000000, 0x0400000, CRC(04ce6b76) SHA1(b846752a739ae6c657e74bbe5e5e772a6c3d5e00) )
	ROM_LOAD32_WORD("0351016.u73", 0x0000002, 0x0400000, CRC(7bccca48) SHA1(b7a4f00eac4b91ec671555f1332f705acffd7b00) )
	ROM_LOAD32_WORD("0351016.u85", 0x0800000, 0x0400000, CRC(28ca3d73) SHA1(3d9edfffe2d7fd37e0f9c6cd292ca3c68abae15b) )
	ROM_LOAD32_WORD("0351016.u72", 0x0800002, 0x0400000, CRC(7703165a) SHA1(6d9e019796dee04a9817c4176af5bf10f7a7597d) )
ROM_END


// 10214711 / 780
// Grizzly / Local / C - 12/04/06
// Variations (% and NO):
//    87.93% 99
//    90.11% 01
//    91.83% 02
ROM_START( grizzly )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10214711.u86", 0x0000000, 0x0400000, CRC(4d071a11) SHA1(1f1557d6ad403ab13bc505076f26969fd81dd2e7) )
	ROM_LOAD32_WORD("10214711.u73", 0x0000002, 0x0400000, CRC(c45f8405) SHA1(2a5ef6932736e65ac62bbeee54e62689dd912dac) )
	ROM_LOAD32_WORD("10214711.u85", 0x0800000, 0x0400000, CRC(381765db) SHA1(8dc8901455f3c1f8b6302c6a996afe15280cf44b) )
	ROM_LOAD32_WORD("10214711.u72", 0x0800002, 0x0400000, CRC(135ebde3) SHA1(baa20971ab9617742382088b9e956eeede11879b) )
	ROM_LOAD32_WORD("10214711.u84", 0x1000000, 0x0400000, CRC(d78ce4b8) SHA1(ab0a10d583b386975b0a53fccd9870b82a3d91a5) )
	ROM_LOAD32_WORD("10214711.u71", 0x1000002, 0x0400000, CRC(f1bb1351) SHA1(59b5f6ee3a3e05e5d975b22de35d0c7bd87f82d3) )
ROM_END


// 20169511 / 717/2
// Heart of Gold / Local / A - 15/09/03
// Variations (% and NO):
//    87.58% 99
//    90.03% 01
//    92.11% 02
ROM_START( heartgld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20169511.u86", 0x0000000, 0x0400000, CRC(c2f2111a) SHA1(9c62dcac8fe03afbb7475a0d6e89ff01c6795c6f) )
	ROM_LOAD32_WORD("20169511.u73", 0x0000002, 0x0400000, CRC(ee851155) SHA1(885928fccf3368911717b7ca5352ecae862c7ebc) )
	ROM_LOAD32_WORD("20169511.u85", 0x0800000, 0x0400000, CRC(af0db2a6) SHA1(aec212c72d8588314cdb2b558f29524b0c142bbc) )
	ROM_LOAD32_WORD("20169511.u72", 0x0800002, 0x0400000, CRC(041cf141) SHA1(793a1c1ca44f0d4c83294456f953dc4e7fd3b134) )
ROM_END


// 10184211 / 717/2
// Heart of Gold / ASP / B - 10/03/04
// Variations (% and NO):
//    87.58% 99
ROM_START( heartglda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10184211.u86", 0x0000000, 0x0400000, CRC(abf52db8) SHA1(4f04360ed4c13d9fb986fbee6c8a658db3e79564) )
	ROM_LOAD32_WORD("10184211.u73", 0x0000002, 0x0400000, CRC(7fac3ddc) SHA1(4c88a91afd2c569077fe9b967761130538507b52) )
	ROM_LOAD32_WORD("10184211.u85", 0x0800000, 0x0400000, CRC(93e85193) SHA1(38059f2ac94405931f6a1cfcbf0b8fc65f5e00f2) )
	ROM_LOAD32_WORD("10184211.u72", 0x0800002, 0x0400000, CRC(dfdb5269) SHA1(9c49579d9503c6929c5fb26d6413c8e1ca645610) )
ROM_END


// 10232911 / 717/4
// Heart of Vegas / Local / C - 27/06/06
// Variations (% and NO):
//    87.90% 99
//    90.07% 01
ROM_START( heartveg )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10232911.u86", 0x0000000, 0x0400000, CRC(0ef4e3cb) SHA1(0cb2c296d7ac39e0e3fe590040c232e35d1508d4) )
	ROM_LOAD32_WORD("10232911.u73", 0x0000002, 0x0400000, CRC(e33aeb25) SHA1(32c3e914fff060404ebf3fbb7a502c9081b57862) )
ROM_END


// 1J008311 / JB001/4
// Helen of Troy / NSW/ACT / G - 21/01/02
// Variations (% and NO):
//    87.93% 99
//    90.05% 01
//    92.23% 02
ROM_START( hlntroy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008311.u86", 0x0000000, 0x0400000, CRC(dac1d540) SHA1(8a57545604beab1f0872f6da98a3f9f5a4079ba0) )
	ROM_LOAD32_WORD("1j008311.u73", 0x0000002, 0x0400000, CRC(7e76bb49) SHA1(ae67b4fb2af4bddf2e751c7c22ac6dc416410e33) )
	ROM_LOAD32_WORD("1j008311.u85", 0x0800000, 0x0400000, CRC(21e8c508) SHA1(930843811b35da27afc8e6561046a249aa10017b) )
	ROM_LOAD32_WORD("1j008311.u72", 0x0800002, 0x0400000, CRC(74045490) SHA1(364ba8a7911f63f529ef5ff0e5bdd17f7bd5a8c1) )
ROM_END


// 10129121 / JB001/4
// Helen of Troy / ASP / G - 21/01/02
// Same date string as parent
// Variations (% and NO):
//    87.93% 99
ROM_START( hlntroya )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10129121.u86", 0x0000000, 0x0400000, CRC(5c23080e) SHA1(bb11f51893e5e970ee59d7395bc529c6cbebfd23) )
	ROM_LOAD32_WORD("10129121.u73", 0x0000002, 0x0400000, CRC(7ae229ca) SHA1(7960701a1febb51d3ccfccc6f85d938fdcb4ad68) )
	ROM_LOAD32_WORD("10129121.u85", 0x0800000, 0x0400000, CRC(33ab6751) SHA1(4e5a844be882f1899b6c76f812ef0606996d7c16) )
	ROM_LOAD32_WORD("10129121.u72", 0x0800002, 0x0400000, CRC(c4c9ee7b) SHA1(018ad844502b9427454b88dc5f7cdd0cb402a363) )
ROM_END


// 10122711 / JB001/4
// Helen of Troy / ASP / G - 21/01/02
// Same date string as parent
// Variations (% and NO):
//    87.93% 99
ROM_START( hlntroyb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122711.u86", 0x0000000, 0x0400000, CRC(859db0b0) SHA1(70e675c914f8eb959ff92cb3e2e114affb2e6b88) )
	ROM_LOAD32_WORD("10122711.u73", 0x0000002, 0x0400000, CRC(0211b4b4) SHA1(aaf1d36af3965094f3f207e5fc58ae875dafd2ec) )
	ROM_LOAD32_WORD("10122711.u85", 0x0800000, 0x0400000, CRC(8d350f92) SHA1(748c9692a419cd53ed8dea265edbce3a1a0d5e54) )
	ROM_LOAD32_WORD("10122711.u72", 0x0800002, 0x0400000, CRC(56170a27) SHA1(83ebabe9ee31772c0b5a20187dfcd9859bc82822) )
ROM_END


// 10116411 / JB001/4
// Helen of Troy / ASP
ROM_START( hlntroyc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	10116411   EXP   mk6   Helen Of Troy   jb001/4   MUL   1/4   © 2003 Aristocrat   P   U86
	10116411   EXP   mk6   Helen Of Troy   jb001/4   MUL   2/4   © 2003 Aristocrat   P   U73
	10116411   EXP   mk6   Helen Of Troy   jb001/4   MUL   3/4   © 2003 Aristocrat   P   U85
	10116411   EXP   mk6   Helen Of Troy   jb001/4   MUL   4/4   © 2003 Aristocrat   P   U72
	*/
	ROM_LOAD32_WORD("10116411.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10116411.u73", 0x0000002, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10116411.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10116411.u72", 0x0800002, 0x0400000, CRC(df3fe910) SHA1(35fd2324278fabeaafd36881656623addbf415d8) )
ROM_END


// 0151336 / JB014/21
// Helen of Troy / Local / B - 01/16/04
// Variations (% and NO):
//    87.982% 99
//    90.178% 01
//    92.197% 02
//    94.967% 03
//    96.957% 04
ROM_START( hlntroyu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151336-U86 Series:01 M27V322 Mk6USA   Helen Of Troy   L04-1236-01   C2818H02   1A6B-E271 / 1E5A6BCE / 4E4C29DF   Copyright © 2003 Aristocrat Technologies, Inc.
	0151336-U73 Series:01 M27V322 Mk6USA   Helen Of Troy   L04-1236-02   F8DPDAFU   74DC-01AA / 1DEE7249 / 34321C36   Copyright © 2003 Aristocrat Technologies, Inc.
	0151336-U85 Series:01 M27V322 Mk6USA   Helen Of Troy   L04-1236-03   B361D4F0   EB5C-6E4E / 083BEB5D / 1C75EC0A   Copyright © 2003 Aristocrat Technologies, Inc.
	0151336-U72 Series:01 M27V322 Mk6USA   Helen Of Troy   L04-1236-04   16DH7PB7   2924-7CB1 / 08295E8C / 16A62338   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151336.u86", 0x0000000, 0x0400000, CRC(7100841d) SHA1(67fa5616ca0bb2b25ed2951d3decf0126e0bac82) )
	ROM_LOAD32_WORD("0151336.u73", 0x0000002, 0x0400000, CRC(aebf8e8e) SHA1(22b28883c4ea365c07b6c4ae2fe3c873be158685) )
	ROM_LOAD32_WORD("0151336.u85", 0x0800000, 0x0400000, CRC(4c82ed24) SHA1(455ac96a92b383cb7e2391e92e528c3b4e51f5cc) )
	ROM_LOAD32_WORD("0151336.u72", 0x0800002, 0x0400000, CRC(b694d927) SHA1(0151f3980a401d646a18b9956d3d247daaa48bf2) )
ROM_END


// 20004111 / 671
// High Noon / Local / B - 28/06/01
// Variations (% and NO):
//    87.20% 99
//    90.00% 01
//    92.05% 02
ROM_START( highnoon )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20004111.u86", 0x0000000, 0x0400000, CRC(c4f106c9) SHA1(38cde08bcdc8e053e8ea0166989a6ef40df139a3) )
	ROM_LOAD32_WORD("20004111.u73", 0x0000002, 0x0400000, CRC(a7b68492) SHA1(745220b963f9c5755ad078de9ccbca3c922d40ed) )
	ROM_LOAD32_WORD("20004111.u85", 0x0800000, 0x0400000, CRC(cde132fd) SHA1(157e1d7fde6c57e5bc4ace03e43734d30141ad09) )
	ROM_LOAD32_WORD("20004111.u72", 0x0800002, 0x0400000, CRC(fa35cdef) SHA1(060af506fee9234fb4921b788368e3e850b681c9) )
ROM_END


// 10100511 / 670/1
// Hollywood Dreams / Local / A - 31/01/02
// Variations (% and NO):
//    87.89% 99
//    90.14% 01
//    92.15% 02
ROM_START( holdrma6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10100511.u86", 0x0000000, 0x0400000, CRC(9da0445f) SHA1(65e8d5f2563a5aa90dcba8b854912fe835294cb3) )
	ROM_LOAD32_WORD("10100511.u73", 0x0000002, 0x0400000, CRC(04fec2d4) SHA1(56beb6107c56674dc914f0c988311375dd281081) )
	ROM_LOAD32_WORD("10100511.u85", 0x0800000, 0x0400000, CRC(daae9640) SHA1(3d8aa7b4d55d1c3d8196be26bb89b83785666df1) )
	ROM_LOAD32_WORD("10100511.u72", 0x0800002, 0x0400000, CRC(197035d3) SHA1(18da7e44107e8e0df584bd866a0083cf85ecceee) )
ROM_END


// 10122811
// Hollywood Dreams / ASP
ROM_START( holdrma6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122811.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10122811.u73", 0x0000002, 0x0400000, CRC(d823df5d) SHA1(9f4cdbd31af1794a1f48b5a3694ea55fc8bd5206) )
	ROM_LOAD32_WORD("10122811.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10122811.u72", 0x0800002, 0x0400000, CRC(185485b0) SHA1(495d1dfe1ecbde358a10a4d8be5717080be773c7) )
ROM_END


// 10200311 / 765
// HOUSE OF HEARTS / Local / C - 09/11/04
// Variations (% and NO):
//    87.87% 99
//    90.14% 01
ROM_START( househrt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10200311.u86", 0x0000000, 0x0400000, CRC(6cdc5816) SHA1(7157a12baa98ba810f5dcc5d5816a3380748032b) )
	ROM_LOAD32_WORD("10200311.u73", 0x0000002, 0x0400000, CRC(ad7c55fc) SHA1(873abc95c8118b5ca73779445023e8e0299498a6) )
ROM_END


// 10208411 / 765/1
// House of Hearts / ASP / C - 25/10/05
// Variations (% and NO):
//    87.87% 99
ROM_START( househrta )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10208411.u86", 0x0000000, 0x0400000, CRC(88651d66) SHA1(99d3f485bd40524a079824b82ba32485d1b939fd) )
	ROM_LOAD32_WORD("10208411.u73", 0x0000002, 0x0400000, CRC(5ed7fc1d) SHA1(a3085d90cc56705b0936ab75336703d4d4f8e2d1) )
ROM_END


// 20190711 / 784
// Inca Chief / Local / C - 21/10/05
// Variations (% and NO):
//    87.82% 99
//    90.11% 01
//    91.84% 02
ROM_START( incachf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20190711.u86", 0x0000000, 0x0400000, CRC(f9fa7d96) SHA1(a247c2694c0f0f78cbc431dca4d9385ab991ae80) )
	ROM_LOAD32_WORD("20190711.u73", 0x0000002, 0x0400000, CRC(45208b71) SHA1(e3ce2e9f5bd467872380ad387e77447831a0e67e) )
	ROM_LOAD32_WORD("20190711.u85", 0x0800000, 0x0400000, CRC(46816847) SHA1(aa65826fe1282b15fbdd8abe81e03f0d11d4f18f) )
	ROM_LOAD32_WORD("20190711.u72", 0x0800002, 0x0400000, CRC(95d1dfd2) SHA1(1faead5e4429e4e8a6fd53c5b16bd3fac9483c28) )
ROM_END


// 10004211 / 628/6
// Indian Dreaming / Local / B - 24/04/01
// Variations (% and NO):
//    87.15% 99
//    90.14% 02
//    92.32% 03
ROM_START( indream )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10004211.u86", 0x0000000, 0x0400000, CRC(bea8f843) SHA1(e8f345c2ff5f751e64bb945827826e481d376377) )
	ROM_LOAD32_WORD("10004211.u73", 0x0000002, 0x0400000, CRC(41279ab8) SHA1(3f275669036ef07d7a0b6ebb1ea58ca5fde963e0) )
	ROM_LOAD32_WORD("10004211.u85", 0x0800000, 0x0400000, CRC(ec18c9ea) SHA1(e3150499e7ad4a2e63aa3c070089c8267eefa5af) )
	ROM_LOAD32_WORD("10004211.u72", 0x0800002, 0x0400000, CRC(92c75004) SHA1(6baa35896dd51c17ab5818919616a217b8b2e4dd) )
ROM_END


// 10192211 / 628/6
// Indian Dreaming / ASP / C - 20/09/04
// Variations (% and NO):
//    87.15% 99
ROM_START( indreama )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10192211.u86", 0x0000000, 0x0400000, CRC(909365b8) SHA1(3d8b985319dbf1f1f161fab87c31160086552ece) )
	ROM_LOAD32_WORD("10192211.u73", 0x0000002, 0x0400000, CRC(685f34ba) SHA1(89e024da779173830a02412502ff0f1ff6e549ee) )
ROM_END


// 20109411 / 628/10
// Indian Dreaming / Local / A - 12/07/02
// Cash Express Hyperlink game
// Variations (% and NO):
//    82.11% 99 (Hyperlink)
ROM_START( indremce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20109411.u86", 0x0000000, 0x0400000, CRC(18ae6a0a) SHA1(0401b86c7053129eb7fbd27c3e5988481df67481) )
	ROM_LOAD32_WORD("20109411.u73", 0x0000002, 0x0400000, CRC(a0a99966) SHA1(6fc4e3496e7791e818cbda6d95355467ceb28e6c) )
	ROM_LOAD32_WORD("20109411.u85", 0x0800000, 0x0400000, CRC(b8b51dea) SHA1(04e6a261ab4912c7204b4a276147e1e219e02067) )
	ROM_LOAD32_WORD("20109411.u72", 0x0800002, 0x0400000, CRC(4269940b) SHA1(8f5f6d751360e0abe2a6a80551f56176c46249e5) )
ROM_END


// 20151911 / 742
// Indian Dreaming (Lady Luck) / Local / C - 15/10/03
// Variations (% and NO):
//    88.61% 99
//    87.56% 01
//    86.70% 02
//    86.03% 03
ROM_START( indremll )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U86
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U73
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U85
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U72
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U84
	20151911   NSW   INDIAN DREAMING- LL   742   4MB   6 eproms   Aristocrat © 2005   P   U71
	*/
	ROM_LOAD32_WORD("20151911.u86", 0x0000000, 0x0400000, CRC(81144634) SHA1(3352bc272da29706d3913e65e0eccfd597628239) )
	ROM_LOAD32_WORD("20151911.u73", 0x0000002, 0x0400000, CRC(ae6590eb) SHA1(d26d4c1612a466a4d67ca51cf6df2c38da88cd0c) )
	ROM_LOAD32_WORD("20151911.u85", 0x0800000, 0x0400000, CRC(d5b64886) SHA1(07b2e2c35e1a71d5c6b5687452f3debee1128632) )
	ROM_LOAD32_WORD("20151911.u72", 0x0800002, 0x0400000, CRC(ebf17295) SHA1(f057ee03fc51041a4aa4413f3cb7df7483b4dd49) )
	ROM_LOAD32_WORD("20151911.u84", 0x1000000, 0x0400000, CRC(14ca706b) SHA1(552b8c864f8b68c48688e70e55424ceb34de4757) )
	ROM_LOAD32_WORD("20151911.u71", 0x1000002, 0x0400000, CRC(81aecf10) SHA1(ca9236003e12dbe897dd942696dbac5cf059e8f1) )
ROM_END


// 10130711 / 628/12
// Indian Dreaming / Local / A - 17/01/03
// Maximillions Hyperlink game
// Variations (% and NO):
//    82.11% 99 (Hyperlink)
ROM_START( indremmm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130711.u86", 0x0000000, 0x0400000, CRC(db13eaf5) SHA1(c2e743b72c2a280266d55642e40c3a7a740052db) )
	ROM_LOAD32_WORD("10130711.u73", 0x0000002, 0x0400000, CRC(a5e3dca5) SHA1(e585841064dc98398169bcd0cd04269bbcfaf77c) )
	ROM_LOAD32_WORD("10130711.u85", 0x0800000, 0x0400000, CRC(988f10da) SHA1(9c21cb8ebebcd603b25329331de89e9aaa36368a) )
	ROM_LOAD32_WORD("10130711.u72", 0x0800002, 0x0400000, CRC(efe15e58) SHA1(8ae7b1436664189a5d07f523d41611bca6d1936c) )
ROM_END


// 10150611 / 753
// Indian Dreaming 2nd Chance / Local / B - 08/12/03
// Variations (% and NO):
//    93.84% 99
//    91.81% 01
//    89.82% 02
ROM_START( indrem2c )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10150611.u86", 0x0000000, 0x0400000, CRC(23a13807) SHA1(5e0a403c3d227ef8ba49e2bd5f0a4f77acd42518) )
	ROM_LOAD32_WORD("10150611.u73", 0x0000002, 0x0400000, CRC(a23e7fcd) SHA1(d47751a88c7609118d0acbeabbcb812d195e3d48) )
	ROM_LOAD32_WORD("10150611.u85", 0x0800000, 0x0400000, CRC(d49a6d8b) SHA1(55ebb5b098565cea8305ad9998eaee9a2b84cdbd) )
	ROM_LOAD32_WORD("10150611.u72", 0x0800002, 0x0400000, CRC(7181dc26) SHA1(63b6ae4c3e7464d75762224a8c30348c6c6f0e98) )
ROM_END


// 10102511 / 674/1
// INDIAN GOLD (MR. CASHMAN) / Local / A- 21/11/01
// Variations (% and NO):
//    87.20% 99
ROM_START( indgldcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102511.u86", 0x0000000, 0x0400000, CRC(9340f843) SHA1(e3cf3ea427535aab8027f7d2f6c6e496674d694f) )
	ROM_LOAD32_WORD("10102511.u73", 0x0000002, 0x0400000, CRC(318a4b3e) SHA1(ba16fb0a82539f01b5b8902f510b0acdfe0fdf6d) )
	ROM_LOAD32_WORD("10102511.u85", 0x0800000, 0x0400000, CRC(6b888235) SHA1(6bea4e0c547d04a25202969d6216beacd8e63b23) )
	ROM_LOAD32_WORD("10102511.u72", 0x0800002, 0x0400000, CRC(e814271e) SHA1(9aaf9a463fd08a9cdb48dcb60cb0242f3df1b759) )
ROM_END


// 10174111 / 764
// Indian Princess / Local / A - 07/04/04
// Variations (% and NO):
//    87.94% 99
//    90.17% 01
//    91.98% 02
ROM_START( indprncs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10174111.u86", 0x0000000, 0x0400000, CRC(dfad47ba) SHA1(807c0e0e4b0cccb8e4d2d34f26be26973745ca1b) )
	ROM_LOAD32_WORD("10174111.u73", 0x0000002, 0x0400000, CRC(c128c9a5) SHA1(bed6f0eba6da62a3981e5c9b6ada5209d37613e5) )
	ROM_LOAD32_WORD("10174111.u85", 0x0800000, 0x0400000, CRC(829f492e) SHA1(36abc1b1beb01814a4e5fc68f1cd424ae979ced7) )
	ROM_LOAD32_WORD("10174111.u72", 0x0800002, 0x0400000, CRC(785493ea) SHA1(e56ee84783a6033ea327a2035df58cb8deca150f) )
ROM_END


// 10125611 / 716
// Jailbreak / Local / A - 14/11/02
// Variations (% and NO):
//    87.86% 99
//    89.84% 01
//    92.05% 02
ROM_START( jailbrk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10125611.u86", 0x0000000, 0x0400000, CRC(3c6d628c) SHA1(d60ad4b04545ccaf6212a47796d16d97173d530b) )
	ROM_LOAD32_WORD("10125611.u73", 0x0000002, 0x0400000, CRC(5142c5e0) SHA1(dde1f300292b0b8250aeab1d686fb36184daabd7) )
	ROM_LOAD32_WORD("10125611.u85", 0x0800000, 0x0400000, CRC(705ddca8) SHA1(e0f8ca2e323f1dd040ed0cd4f974e419e716a0c6) )
	ROM_LOAD32_WORD("10125611.u72", 0x0800002, 0x0400000, CRC(5185ad7f) SHA1(34517ded04bf51be348200bc09e848ab74ccbeef) )
ROM_END


// 0351068 / US041
// Foxworthy Redneck / Local / B - 06/13/03
// 'You Might Be A Redneck If...' on artwork, 'Foxworthy Recneck' in ROM
// Variations (% and NO):
//    92.395% 99
ROM_START( jefffox )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351068.u86", 0x0000000, 0x0400000, CRC(6b0a7eb3) SHA1(cd99fed52643afa596bfbf4a18deca42043520ef) )
	ROM_LOAD32_WORD("0351068.u73", 0x0000002, 0x0400000, CRC(4b778a84) SHA1(d11b2aefcaa2ac22fea80faab524f82511b6344a) )
	ROM_LOAD32_WORD("0351068.u85", 0x0800000, 0x0400000, CRC(52d5395e) SHA1(57b854e516881bc208b48ed82fdef25059e2efab) )
	ROM_LOAD32_WORD("0351068.u72", 0x0800002, 0x0400000, CRC(ff2ff01d) SHA1(c0d17b3d9907d3876c7a1dbc19b023afcffb2bb1) )
ROM_END


// 0151051 / US039
// Jewel Enchant CM / Local / B- 1/24/03
// 'Jewel of the Enchantress' on artwork, 'Jewel Enchant CM' in ROM
// Variations (% and NO):
//    92.395% 99
ROM_START( jewelcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151051.u86", 0x0000000, 0x0400000, CRC(68a438be) SHA1(c62fae7d0f4909ba5d3b72fc7f903a49cc25c168) )
	ROM_LOAD32_WORD("0151051.u73", 0x0000002, 0x0400000, CRC(3517c88f) SHA1(fd02618f6f4553ceda17780c7d1c5cf7a2ec44a6) )
	ROM_LOAD32_WORD("0151051.u85", 0x0800000, 0x0400000, CRC(9194156f) SHA1(b030432ae7da6b36af85a1ddd43a5657542989ad) )
	ROM_LOAD32_WORD("0151051.u72", 0x0800002, 0x0400000, CRC(6c168e3b) SHA1(db251f024881e27615049f94c6e11002e89b2c40) )
ROM_END


// 20189111 / 789
// King Neptune / Local / E - 08/08/05
// Variations (% and NO):
//    87.99% 99
//    90.20% 01
//    91.99% 02
ROM_START( kingnep )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20189111.u86", 0x0000000, 0x0400000, CRC(750597c6) SHA1(06d3ba55b313da8973fd2b34ba888920519c51c6) )
	ROM_LOAD32_WORD("20189111.u73", 0x0000002, 0x0400000, CRC(1e815d36) SHA1(b29f5e812f52317b060feac8ee91c08a545db7f4) )
	ROM_LOAD32_WORD("20189111.u85", 0x0800000, 0x0400000, CRC(df89ae50) SHA1(fa9a5c795266bcafea0062aae34a2b7432550b53) )
	ROM_LOAD32_WORD("20189111.u72", 0x0800002, 0x0400000, CRC(4e3006a2) SHA1(6d566d2120cb612f647223e61347a1784cb69a95) )
ROM_END


// 30106331 / 687
// King of The Nile / Local / D - 07/01/03
// Variations (% and NO):
//    99.38% 99
//    97.13% 01
//    94.36% 02
ROM_START( kingnile )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30106331.u86", 0x0000000, 0x0400000, CRC(c3830908) SHA1(438fc339f8b71668e816b5909195cae4158f39c2) )
	ROM_LOAD32_WORD("30106331.u73", 0x0000002, 0x0400000, CRC(9a90354d) SHA1(5c5b0822e6a842f85e2acca0e1c8bfe0c3f68496) )
	ROM_LOAD32_WORD("30106331.u85", 0x0800000, 0x0400000, CRC(524c786e) SHA1(7e40266be5496baefd74796ce66857aec2d21437) )
	ROM_LOAD32_WORD("30106331.u72", 0x0800002, 0x0400000, CRC(ea5d2450) SHA1(fdcf6c532231e9abf3a1c035bdef9d0db7fcf148) )
ROM_END


// 20106321 / 687
// King of The Nile / Local / C - 27/09/02
// Variations (% and NO):
//    87.53% 99
ROM_START( kingnilea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20106321.u72", 0x0800002, 0x0400000, CRC(2600d9da) SHA1(2c78b987b8a8641b7eedda4880bd3f5846f11224) )
	ROM_LOAD32_WORD("20106321.u73", 0x0000002, 0x0400000, CRC(2875a911) SHA1(3e881c8c736f297778836c128f66150628038e49) )
	ROM_LOAD32_WORD("20106321.u85", 0x0800000, 0x0400000, CRC(a981510c) SHA1(5c018eb101193b1f4864ccd8c30fe328713c4c97) )
	ROM_LOAD32_WORD("20106321.u86", 0x0000000, 0x0400000, CRC(c4b7a58e) SHA1(f0877c9e0a285808c8117d09a33dec2d2caf7b5b) )
ROM_END


// 10127511 / 687
// King of The Nile / ASP / G - 30/01/03
// Variations (% and NO):
//    99.38% 99
ROM_START( kingnileb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10127511.u86", 0x0000000, 0x0400000, CRC(8357933a) SHA1(34c665cc37ccfb78719027c9f6dc82c9756da7d3) )
	ROM_LOAD32_WORD("10127511.u73", 0x0000002, 0x0400000, CRC(bac43224) SHA1(bd67b3a7cc684dc3ec8224bde431ad5706bf1711) )
	ROM_LOAD32_WORD("10127511.u85", 0x0800000, 0x0400000, CRC(c328b2eb) SHA1(2e0a870e74eb0735a4bf187e501e39e208d1aee7) )
	ROM_LOAD32_WORD("10127511.u72", 0x0800002, 0x0400000, CRC(bfae9161) SHA1(7be63f2ee264d5d9fa19a6baba60cf8924574cd5) )
ROM_END


// 20109021 / 696
// King Penguin / Local / D- 02/10/02
// Variations (% and NO):
//    87.54% 99
ROM_START( kingpeng )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20109021.u86", 0x0000000, 0x0400000, CRC(40c08336) SHA1(c8568c176c09239823b08c494e2e020e564c2b43) )
	ROM_LOAD32_WORD("20109021.u73", 0x0000002, 0x0400000, CRC(c51ef018) SHA1(f89ed9042c12e40350284236ca77dcaede9e38d0) )
	ROM_LOAD32_WORD("20109021.u85", 0x0800000, 0x0400000, CRC(417152bd) SHA1(4cdd6ba592bf7546226c304ade46b90774ba0875) )
	ROM_LOAD32_WORD("20109021.u72", 0x0800002, 0x0400000, CRC(06b7ce2e) SHA1(fc30a247ed2e5ac7586c4c6cba6b8cd7d8206d01) )
ROM_END


// 40109021 / 696
// King Penguin / Local / E- 26/11/02
// SHOW PROGRAM
// Variations (% and NO):
//    87.54% 99
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( kingpengsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__40109021.u86", 0x0000000, 0x0400000, CRC(077635b9) SHA1(3d17892df4a068701be1ac649f448e10b8c52ebd) )
	ROM_LOAD32_WORD("sp__40109021.u73", 0x0000002, 0x0400000, CRC(ac8e7dc1) SHA1(b317107cda37c6148e84237fdc76f2c2abf4d743) )
	ROM_LOAD32_WORD("sp__40109021.u85", 0x0800000, 0x0400000, CRC(f81ac65b) SHA1(0833960ce8b1e883c033c92a8121cc89bac6f64c) )
	ROM_LOAD32_WORD("sp__40109021.u72", 0x0800002, 0x0400000, CRC(44854b6b) SHA1(eae87ea7a6d4ade8de5ea7a3614d7e3183ec0af3) )
ROM_END


// 30001011 / 643/6
// Koala Mint / Local / A- 01/09/00
// Variations (% and NO):
//    90.15% 99
//    87.71% 01
//    92.07% 02
//    94.95% 03
//    93.56% 04
ROM_START( koalama6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30001011.u86", 0x0000000, 0x0400000, CRC(e405c4b8) SHA1(e838767f0c224a3c96f9dd480559fed01ca7a398) )
	ROM_LOAD32_WORD("30001011.u73", 0x0000002, 0x0400000, CRC(df3d45a0) SHA1(e9cbfeba1271a66c574cfb3bd96e21fd8cdd26f2) )
	ROM_LOAD32_WORD("30001011.u85", 0x0800000, 0x0400000, CRC(1e12c01e) SHA1(21cad879ece470c465907dd970741b13774fb524) )
	ROM_LOAD32_WORD("30001011.u72", 0x0800002, 0x0400000, CRC(84c0f78d) SHA1(3f8e98261bfce1223de538d6bd53fa92d38c4b0d) )
ROM_END


// 10136711 / 602/21
// Kohinoor / Local / B - 10/02/03
// Variations (% and NO):
//    87.93% 99
//    90.08% 01
//    92.09% 02
ROM_START( kohinoor )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10136711.u86", 0x0000000, 0x0400000, CRC(e76e81f3) SHA1(1606c08ab6e75ad0cbbdd3f297dbb4fdf855c013) )
	ROM_LOAD32_WORD("10136711.u73", 0x0000002, 0x0400000, CRC(dfdeb535) SHA1(be6b2e84603a5b07e4561012ff9b1d45fbd4f281) )
	ROM_LOAD32_WORD("10136711.u85", 0x0800000, 0x0400000, CRC(cd8324e6) SHA1(b779d1040cd0119b778032d0e165d31d8fafd128) )
	ROM_LOAD32_WORD("10136711.u72", 0x0800002, 0x0400000, CRC(9b6f5a83) SHA1(06707953091719bf545efd2200f81504c5416e74) )
ROM_END


// 10220111 / 756
// Let's Go Fish'n / Local / E - 23/05/06
// Variations (% and NO):
//    87.95% 99
//    90.08% 01
ROM_START( lgfish )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10220111.u86", 0x0000000, 0x0400000, CRC(4c491910) SHA1(1f9013621a4c9140c447637bc967ba583da318eb) )
	ROM_LOAD32_WORD("10220111.u73", 0x0000002, 0x0400000, CRC(7cfed17e) SHA1(fc93e71d67228b3b1f3a34a3f3c34c2648100d2b) )
	ROM_LOAD32_WORD("10220111.u85", 0x0800000, 0x0400000, CRC(c386320a) SHA1(c02b4b1cdd45b9b579b21d239cf45f7f3510c14a) )
	ROM_LOAD32_WORD("10220111.u72", 0x0800002, 0x0400000, CRC(9ff206fe) SHA1(4c8023a3b2e4d658a859d19d54be8cfe53cf4a6f) )
	ROM_LOAD32_WORD("10220111.u84", 0x1000000, 0x0400000, CRC(471f61c6) SHA1(67828d3cf06e32b087fb7c4ade212580207dc2cf) )
	ROM_LOAD32_WORD("10220111.u71", 0x1000002, 0x0400000, CRC(75d5bf1c) SHA1(6249b3ed929a65efed770615f2fe691612e8ac72) )
ROM_END


// 10223911 / 756
// Let's Go Fish'n / ASP / E - 23/05/06
// Same date string as parent
// ROM has unchanged 10220111 ID (NSW part number) in header with "102239" string further down
// Variations (% and NO):
//    87.95% 99
ROM_START( lgfisha )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10223911.u86", 0x0000000, 0x0400000, CRC(8dd20eaf) SHA1(f6485685da6fed1e5ddde59bf0375f61b47c7dcf) )
	ROM_LOAD32_WORD("10223911.u73", 0x0000002, 0x0400000, CRC(4a11007a) SHA1(8cebbe5103b252ca062ea0c95ab16b4b4e1a63f0) )
	ROM_LOAD32_WORD("10223911.u85", 0x0800000, 0x0400000, CRC(9dcc9cc2) SHA1(821df1fb77f3135c37a96962df5075b724429218) )
	ROM_LOAD32_WORD("10223911.u72", 0x0800002, 0x0400000, CRC(fe85bfdf) SHA1(2c846d68012539fe0a8986699dcd382edc39c691) )
	ROM_LOAD32_WORD("10223911.u84", 0x1000000, 0x0400000, CRC(06f7c9c3) SHA1(40bfadd3e8cbdbe59ed714c4040469312c3fe84d) )
	ROM_LOAD32_WORD("10223911.u71", 0x1000002, 0x0400000, CRC(af635e3d) SHA1(15430458787a1f3128a22159f3feefc46c876d8e) )
ROM_END


// 0151115 / US088
// Let Em Roll / Local / B - 06/24/04
// 'Let 'em Roll' on artwork, 'Let Em Roll' in ROM
// Variations (% and NO):
//    87.581% 99 (9 line)    87.660% 99 (20 line)
//    90.540% 01 (9 line)    90.494% 01 (20 line)
//    92.424% 02 (9 line)    92.468% 02 (20 line)
//    95.368% 03 (9 line)    95.317% 03 (20 line)
//    97.127% 04 (9 line)    97.300% 04 (20 line)
ROM_START( lemrol )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151115.u86", 0x0000000, 0x0400000, CRC(18d3513f) SHA1(5d592ce386054627c6f18e87d32317af73866207) )
	ROM_LOAD32_WORD("0151115.u73", 0x0000002, 0x0400000, CRC(57f6a4be) SHA1(c91662e666b8c02b83f61a410da93ad08b4111d2) )
	ROM_LOAD32_WORD("0151115.u85", 0x0800000, 0x0400000, CRC(b8be84bb) SHA1(1c62da796f44f23612ba605ade1835089edac3ab) )
	ROM_LOAD32_WORD("0151115.u72", 0x0800002, 0x0400000, CRC(70a40bb3) SHA1(1c3adcc36d4bb71f0099eb3663b2d67d64ba979b) )
ROM_END


// 0252534 / MV4245
// Line King / Local / F - 11/10/05
// MV4245/1 / E - 11/10/05 (alternate strings)
// Variations (% and NO):
//    87.857% 99
//    90.510% 01
//    92.110% 02
//    94.711% 03
//    97.031% 04
ROM_START( lineking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252534.u86", 0x0000000, 0x0400000, CRC(7f739eb4) SHA1(cccba4551c7f2901b077473f202b1437da433ab4) )
	ROM_LOAD32_WORD("0252534.u73", 0x0000002, 0x0400000, CRC(1b959c89) SHA1(167d628ea3be95cfacad318288bd965823b16ea3) )
	ROM_LOAD32_WORD("0252534.u85", 0x0800000, 0x0400000, CRC(1256b8d1) SHA1(e2ab3d713c41cf00a7b3930678e0f56102fabbca) )
	ROM_LOAD32_WORD("0252534.u72", 0x0800002, 0x0400000, CRC(7440510c) SHA1(e00ee345e8811a7d72f33f30f8dbffee8a550e6a) )
ROM_END


// 30111211 / 685
// Lord Leopard / Local / B - 27/05/02
// Variations (% and NO):
//    87.78% 99
//    90.05% 01
//    92.05% 02
ROM_START( lordleo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30111211.u86", 0x0000000, 0x0400000, CRC(9ac28584) SHA1(cff40762e123d3ace2fb274166eb9d98ef7c7d4c) )
	ROM_LOAD32_WORD("30111211.u73", 0x0000002, 0x0400000, CRC(7666c5f8) SHA1(cd271bc8c1042fa85506691636d22d36df23025b) )
	ROM_LOAD32_WORD("30111211.u85", 0x0800000, 0x0400000, CRC(1a921c0e) SHA1(feb59ec8372723e5babe16145596c3a9fd0938c3) )
	ROM_LOAD32_WORD("30111211.u72", 0x0800002, 0x0400000, CRC(20e3d468) SHA1(155aa4a81f18d714c6dbbe26c5c9e9d6575b858d) )
ROM_END


// 20006911 / 674
// LOUIE'S GOLD (MR. CASHMAN) / Local / C- 13/07/01
// Variations (% and NO):
//    87.20% 99
ROM_START( lougcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20006911.u86", 0x0000000, 0x0400000, CRC(095161ec) SHA1(4b9ff183d244f70f59eefb2eb6b261e3bcf0c486) )
	ROM_LOAD32_WORD("20006911.u73", 0x0000002, 0x0400000, CRC(3165ca89) SHA1(73d5d32f1b144e25901afa2eeb82e2304f069a55) )
	ROM_LOAD32_WORD("20006911.u85", 0x0800000, 0x0400000, CRC(21d6d1e4) SHA1(d095389ff327940abde30afe8cb2e9f1c1e34226) )
	ROM_LOAD32_WORD("20006911.u72", 0x0800002, 0x0400000, CRC(926aefe5) SHA1(1427cae296864e6dd186614d726e243b46a9aa64) )
ROM_END


// 0152583 / MV4285
// Love Stuck / Local / N/A
// Variations (% and NO):
//    87.666% 99
//    90.185% 01
//    91.970% 02
//    94.972% 03
ROM_START( lovestuk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152583.u86", 0x0000000, 0x0400000, CRC(c374ee56) SHA1(18b084c32a4dad6dacd3eeaf6ba430053405dbe8) )
	ROM_LOAD32_WORD("0152583.u73", 0x0000002, 0x0400000, BAD_DUMP CRC(421f717d) SHA1(29eaecef6dd389329a8cfdcfa4bf3f90252eb096) )
ROM_END


// 10241411 / 840
// Lucky88 / C - 25/06/07
// 'Lucky 88' on artwork, 'Lucky88' in ROM
// Variations (% and NO):
//    87.01% 99
//    87.98% 01
//    86.04% 02
ROM_START( lucky88 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10241411.u86", 0x0000000, 0x0400000, CRC(ae09bfd7) SHA1(65f068e947a26b7b56db6b3a94691a50a002970d) )
	ROM_LOAD32_WORD("10241411.u73", 0x0000002, 0x0400000, CRC(94b7d07e) SHA1(c12cf2da50791262ef85d0a08e897f87185c359d) )
	ROM_LOAD32_WORD("10241411.u85", 0x0800000, 0x0400000, CRC(6c08de75) SHA1(ffadb5181bd82239bf616c1773ccb10dc06549a5) )
	ROM_LOAD32_WORD("10241411.u72", 0x0800002, 0x0400000, CRC(e3d58454) SHA1(a6bfcdb0a047712e77957d2e472bb300a758f12b) )
	ROM_LOAD32_WORD("10241411.u84", 0x1000000, 0x0400000, CRC(eb338e37) SHA1(d795d24c38e654695b8122262bcdd0641f720e58) )
	ROM_LOAD32_WORD("10241411.u71", 0x1000002, 0x0400000, CRC(f0e88142) SHA1(52271aa7dbdc99cff2a8e5230210758f4534f3bd) )
ROM_END


// 10235411 / 822
// Luigi's Ball / NSW / D  - 19/03/07
// Variations (% and NO):
//    87.85% 99
//    90.01% 01
//    91.98% 02
ROM_START( luigibal )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10235411.u86", 0x0000000, 0x0400000, CRC(b44aeadc) SHA1(abee3ea6886973877002c3283bb52c624abd148c) )
	ROM_LOAD32_WORD("10235411.u73", 0x0000002, 0x0400000, CRC(d82cca04) SHA1(ecd126404ab147cba3fedb127772b4c9ccb849bd) )
	ROM_LOAD32_WORD("10235411.u85", 0x0800000, 0x0400000, CRC(9513eded) SHA1(296974e1c4d6c61ce90c6fafc3986aa48a4344ac) )
	ROM_LOAD32_WORD("10235411.u72", 0x0800002, 0x0400000, CRC(5f3a62b0) SHA1(c4adec9f13473246333ece978ea35d35a1527638) )
	ROM_LOAD32_WORD("10235411.u84", 0x1000000, 0x0400000, CRC(c8c80acf) SHA1(958310f0c31642960f90476a9d384fcbefeada75) )
	ROM_LOAD32_WORD("10235411.u71", 0x1000002, 0x0400000, CRC(4f289aea) SHA1(0c77020a30178644a02dac5c9e8ab5543b63409e) )
ROM_END


// 0151139 / US141
// Magic Forest / Local / D - 02/01/05
// Variations (% and NO):
//    87.660% 99
//    90.170% 01
//    91.960% 02
//    94.850% 03
//    97.500% 04
ROM_START( magforst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151139-U86 Series:01 M27V322 Mk6USA   Magic Forest   L05-0684-01   C842BC9H   7B60-29DB / 1F3AAD23 / AAC82F78   Copyright © 2005 Aristocrat Technologies, Inc.
	0151139-U73 Series:01 M27V322 Mk6USA   Magic Forest   L05-0684-02   8CA564F4   E75A-BD24 / 1F27DE9F / A64015B9   Copyright © 2005 Aristocrat Technologies, Inc.
	0151139-U85 Series:01 M27V322 Mk6USA   Magic Forest   L05-0684-03   C68C05DF   0208-3B40 / 0D9E2B83 / E2092543   Copyright © 2005 Aristocrat Technologies, Inc.
	0151139-U72 Series:01 M27V322 Mk6USA   Magic Forest   L05-0684-04   D8F0834U   68CD-008E / 0D6195EA / C5280EEC   Copyright © 2005 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151139.u86", 0x0000000, 0x0400000, CRC(e412acb3) SHA1(80c5de99b0ef53ccd3eb027aade70caf83753568) )
	ROM_LOAD32_WORD("0151139.u73", 0x0000002, 0x0400000, CRC(f9915826) SHA1(51c8455977b03fc6e2a3e898f225c8eb86ba58df) )
	ROM_LOAD32_WORD("0151139.u85", 0x0800000, 0x0400000, CRC(48b5cb37) SHA1(1b2454349ee29f8779f32830cfc405e97126c4b1) )
	ROM_LOAD32_WORD("0151139.u72", 0x0800002, 0x0400000, CRC(53d881a2) SHA1(f63c500fc0208709813d661305900de2f2edc9bf) )
ROM_END


// 0251331 / JB013/20
// Magic Mermaid  / Local / F - 10/13/03
// JB013/21 / B - 02/04/04 (alternate strings)
// Variations (% and NO):
//    87.994% 99
//    90.099% 01
//    92.185% 02
//    94.956% 03
//    97.001% 04
ROM_START( magmerm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251331.u86", 0x0000000, 0x0400000, CRC(dd44042b) SHA1(130c26a2ca02003adaa729a54195a1e4832cc455) )
	ROM_LOAD32_WORD("0251331.u73", 0x0000002, 0x0400000, CRC(eff3f840) SHA1(16bec01d3aa6b103c10698cd1656ab54ee9de512) )
	ROM_LOAD32_WORD("0251331.u85", 0x0800000, 0x0400000, CRC(0c75309e) SHA1(6b745441434c585e3cda2d854dad3861d7337ab5) )
	ROM_LOAD32_WORD("0251331.u72", 0x0800002, 0x0400000, CRC(7a5399ab) SHA1(85e3ca6e00d4f1259c4899f74981ec076524e50d) )
ROM_END


// 20210111 / 801
// Magician / Local / C - 01/03/06
// 'The Magician' on artwork, 'Magician' in ROM
// Variations (% and NO):
//    88.20% 99
//    90.16% 01
//    92.14% 02
ROM_START( magician )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20210111.u86", 0x0000000, 0x0400000, CRC(d5763bfa) SHA1(c8363f855bc1962588e35acf11c847e1c3c1775b) )
	ROM_LOAD32_WORD("20210111.u73", 0x0000002, 0x0400000, CRC(6198b73e) SHA1(1459212376c25df9f3120dc4e3567dfd30b8dd96) )
	ROM_LOAD32_WORD("20210111.u85", 0x0800000, 0x0400000, CRC(ae216d29) SHA1(d40ee5d9a6f96d7cc6b38ccbe1dc9b2f543a35f3) )
	ROM_LOAD32_WORD("20210111.u72", 0x0800002, 0x0400000, CRC(7fb305d3) SHA1(c79bae5a62bb3aa9090fd6cac91d35a950300146) )
	ROM_LOAD32_WORD("20210111.u84", 0x1000000, 0x0400000, CRC(dbb4f1ee) SHA1(f4064ba792a5705209b3221ceb2f3d828c73df1c) )
	ROM_LOAD32_WORD("20210111.u71", 0x1000002, 0x0400000, CRC(67a0906d) SHA1(3f0f8f17dfcc5d1fe38e0737cd623594076bbd2d) )
ROM_END


// 0152162 / US217
// Meteor Storm / USA / A - 03/19/07
// Variations (% and NO):
//    87.977% 99
//    90.041% 01
//    92.090% 02
//    94.905% 03
ROM_START( metestrm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152162-U86 Series 01 M27v322 MK6USA   METEOR STORM   63B9FA13 S2007-3802-04   F79D-29C3 / 1F0FB8FC / AD296499   Copyright © 2007 Aristocrat Technologies, Inc.
	0152162-U73 Series 01 M27v322 MK6USA   METEOR STORM   BUCHE3C0 S2007-3802-02   8345-365D / 1EF47965 / AA22F91C   Copyright © 2007 Aristocrat Technologies, Inc.
	0152162-U85 Series 01 M27v322 MK6USA   METEOR STORM   AFC4F903 S2007-3802-03   81CE-A87F / 08B6D98F / 4B7C1943   Copyright © 2007 Aristocrat Technologies, Inc.
	0152162-U72 Series 01 M27v322 MK6USA   METEOR STORM   98DHA1D0 S2007-3802-01   8FA8-F1A0 / 08723651 / 30BC4DC3   Copyright © 2007 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152162.u86", 0x0000000, 0x0400000, CRC(a1674822) SHA1(471d704ed2600e0b806123031c9add2b13fa42ce) )
	ROM_LOAD32_WORD("0152162.u73", 0x0000002, 0x0400000, CRC(0bc788f7) SHA1(7ce7cdbd08f7ae73a5a1b7c45e72bb9c90d91d3e) )
	ROM_LOAD32_WORD("0152162.u85", 0x0800000, 0x0400000, CRC(1d24fa72) SHA1(1ff3a912919591dcd48459369e97d62cf27d6614) )
	ROM_LOAD32_WORD("0152162.u72", 0x0800002, 0x0400000, CRC(1d225d8b) SHA1(477b39d9d2dd526dd3fe6fba6c4d36dfc845ebac) )
ROM_END


// 10251911 / 816
// Milkin' It / B - 13/08/07
// Variations (% and NO):
//    87.95% 99
//    90.17% 01
ROM_START( milkin )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10251911.u86", 0x0000000, 0x0400000, CRC(eb66636c) SHA1(48216f1f72fc6714cc1876d71489bccaa1624b2e) )
	ROM_LOAD32_WORD("10251911.u73", 0x0000002, 0x0400000, CRC(a20e4928) SHA1(a8fb46a250fe11652c5ccd2253be9aff9e6cae8b) )
	ROM_LOAD32_WORD("10251911.u85", 0x0800000, 0x0400000, CRC(00186fab) SHA1(7c84808ed6f20118a79eebe1730f2fa0ef989d97) )
	ROM_LOAD32_WORD("10251911.u72", 0x0800002, 0x0400000, CRC(e855f155) SHA1(5a065eb205832781b9a565b9f87f7638d52a1a4b) )
	ROM_LOAD32_WORD("10251911.u84", 0x1000000, 0x0400000, CRC(fc0a3315) SHA1(0871a0c6a87d0197d9dfd631006426b6522d61b8) )
	ROM_LOAD32_WORD("10251911.u71", 0x1000002, 0x0400000, CRC(a5c83ff6) SHA1(b5eb551ab93e57c4978243909893d872b0192e08) )
ROM_END


// 10216611 / 778
// Miss Kitty / NSW / C - 18/04/05
// Variations (% and NO):
//    87.57% 99
//    90.19% 01
ROM_START( mskitty )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10216611.u86", 0x0000000, 0x0400000, CRC(36c9ef95) SHA1(71c088bd3b258ab0e8949f661f9e9e9f64949f29) )
	ROM_LOAD32_WORD("10216611.u73", 0x0000002, 0x0400000, CRC(40c1bb34) SHA1(0c54ad0fb6a5c4f096be15c1ef08cb4d246b13b6) )
	ROM_LOAD32_WORD("10216611.u85", 0x0800000, 0x0400000, CRC(c369ad11) SHA1(793b1f8d5833b06422196e2abb7eb0a34be8dbef) )
	ROM_LOAD32_WORD("10216611.u72", 0x0800002, 0x0400000, CRC(d85219a1) SHA1(db0c5f21f7af393d11465981e70ac7b1c8f197b3) )
ROM_END


// 10001211 / 657
// Money Tree / Local / E - 06/11/00
// Variations (% and NO):
//    87.88% 99
//    90.01% 01
//    92.45% 02
ROM_START( mnytree )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001211.u86", 0x0000000, 0x0400000, CRC(bca13ff6) SHA1(84c3491816aa9a2e9a12c321b0d1144ea14ebdbe) )
	ROM_LOAD32_WORD("10001211.u73", 0x0000002, 0x0400000, CRC(2c9edccc) SHA1(05e7de1fbd13c1a62f7fb610dc3541a94d9a39b7) )
	ROM_LOAD32_WORD("10001211.u85", 0x0800000, 0x0400000, CRC(ea055c36) SHA1(e97c4992150fac9e53618ebf96ac1b2dd4e9cf65) )
	ROM_LOAD32_WORD("10001211.u72", 0x0800002, 0x0400000, CRC(9ecf041c) SHA1(543eb20385bfd79c1cf10cba61c501c495e4d94a) )
ROM_END


// 10122211 / 657
// Money Tree / ASP Game / F - 14/03/01
// Variations (% and NO):
//    87.88% 99
ROM_START( mnytreea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122211.u86", 0x0000000, 0x0400000, CRC(0800b976) SHA1(670c82bf2e7f8b0c8628dc7109ffc78403cd15b4) )
	ROM_LOAD32_WORD("10122211.u73", 0x0000002, 0x0400000, CRC(ed576ead) SHA1(cc1eef6ee3bcdb893f974b14f78e51f3812f3f93) )
	ROM_LOAD32_WORD("10122211.u85", 0x0800000, 0x0400000, CRC(71ff3af6) SHA1(e2a8e3acac9ca59230fe266dd41aa1af7f6fed39) )
	ROM_LOAD32_WORD("10122211.u72", 0x0800002, 0x0400000, CRC(beeeb62e) SHA1(f03fd85611a925311321364f127d27f476947382) )
ROM_END


// 30164211 / 628/3
// Moon Fire Jackpot Carnival / QCOM / E - 27/01/04
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.05% 99 (Hyperlink)
ROM_START( moonfire )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	30164211   QCOM   MOON FIRE -J/C HYP   628/3   4MB   4 eproms   Aristocrat © 2014   P   U86
	30164211   QCOM   MOON FIRE -J/C HYP   628/3   4MB   4 eproms   Aristocrat © 2014   P   U73
	30164211   QCOM   MOON FIRE -J/C HYP   628/3   4MB   4 eproms   Aristocrat © 2014   P   U85
	30164211   QCOM   MOON FIRE -J/C HYP   628/3   4MB   4 eproms   Aristocrat © 2014   P   U72
	    Labels marked 2014 despite being a 2004 game
	*/
	ROM_LOAD32_WORD("30164211.u86", 0x0000000, 0x0400000, CRC(9d60754d) SHA1(ab62e0f92191821b8e76dbaf3bd37303cb94187e) )
	ROM_LOAD32_WORD("30164211.u73", 0x0000002, 0x0400000, CRC(56a5ea19) SHA1(2f89527fe3dfdcab088cd3a7ab51a26816fe14dc) )
	ROM_LOAD32_WORD("30164211.u85", 0x0800000, 0x0400000, CRC(01d1f670) SHA1(efca261e4810f683084baa1cac03304cdd233ef9) )
	ROM_LOAD32_WORD("30164211.u72", 0x0800002, 0x0400000, CRC(21ebecc5) SHA1(dfd69f1ebea56b5cf49d67a437c7016410ea65c4) )
ROM_END


// 10227611 / 814/1
// Moonlight Waltz / B - 11/01/07
// Variations (% and NO):
//    87.99% 99
//    90.03% 01
ROM_START( moonwalt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10227611.u86", 0x0000000, 0x0400000, CRC(cb21b264) SHA1(ecc1804c0fd399019e3496ae6ea0a6ea125562f2) )
	ROM_LOAD32_WORD("10227611.u73", 0x0000002, 0x0400000, CRC(8272e787) SHA1(b0b44fa4af0fdd3f66bd117d748ed874379ed131) )
	ROM_LOAD32_WORD("10227611.u85", 0x0800000, 0x0400000, CRC(44f56b6f) SHA1(7706301d9e40e6197aa509c972129b016d60bf34) )
	ROM_LOAD32_WORD("10227611.u72", 0x0800002, 0x0400000, CRC(80e26c8a) SHA1(5f8b01d18210d5997bbf7966d1bf4353dadc4f7d) )
ROM_END


// 20248711 / 844
// More Chilli / A - 17/07/07
// Variations (% and NO):
//    86.96% 99
//    87.89% 01
//    85.95% 02
ROM_START( mchilli )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20248711   NSW   MORE CHILLI   844   4MB   4 eproms   Aristocrat © 2007   P   U86
	20248711   NSW   MORE CHILLI   844   4MB   4 eproms   Aristocrat © 2007   P   U73
	20248711   NSW   MORE CHILLI   844   4MB   4 eproms   Aristocrat © 2007   P   U85
	20248711   NSW   MORE CHILLI   844   4MB   4 eproms   Aristocrat © 2007   P   U72
	*/
	ROM_LOAD32_WORD("20248711.u86", 0x0000000, 0x0400000, CRC(1f5207d7) SHA1(73c3742a94962a66df7af6eff660f90e7e22e344) )
	ROM_LOAD32_WORD("20248711.u73", 0x0000002, 0x0400000, CRC(da883738) SHA1(74f5cdb7b388dec17a0e4d8a04c2107bd07c3ad7) )
	ROM_LOAD32_WORD("20248711.u85", 0x0800000, 0x0400000, CRC(ac11dab3) SHA1(3a201a5e239b10dcdd23aed3b0d724146f2fcf68) )
	ROM_LOAD32_WORD("20248711.u72", 0x0800002, 0x0400000, CRC(85f98b61) SHA1(1ebd8caef5f163a75e185c3fa3c001517f676890) )
ROM_END


// 20289311 / 844/2
// More Chilli / B - 14/12/10
// Variations (% and NO):
//    86.00% 99
ROM_START( mchilliq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20289311.u86", 0x0000000, 0x0400000, CRC(fe0d1184) SHA1(e973b99b1a87a120183f4f6fc8c8c9bdc18bc10d) )
	ROM_LOAD32_WORD("20289311.u73", 0x0000002, 0x0400000, CRC(8fc3c3ed) SHA1(5afdfddad061a71fc17f4cc6acca65fd490c8b44) )
	ROM_LOAD32_WORD("20289311.u85", 0x0800000, 0x0400000, CRC(53dbf992) SHA1(8bda1f65d98874949c2bf5d0ac5843c80dd44228) )
	ROM_LOAD32_WORD("20289311.u72", 0x0800002, 0x0400000, CRC(29ddc2aa) SHA1(1e514ca460b5d70751846bf90d83934ff55115d4) )
ROM_END


// 0252599 / MV4276
// MR WOO / Local / N/A
// Variations (% and NO):
//    80.314% 99
ROM_START( mrwoo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0252599-U86 Series 01 M27v322 MK6USA   MR WOO   9PB2CF81   2C63-275D / 1ECA81AA / 6FBF6076   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	0252599-U73 Series 01 M27v322 MK6USA   MR WOO   885619B8   0B5B-4014 / 1EB4272D / 7057D778   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	0252599-U85 Series 01 M27v322 MK6USA   MR WOO   C986E266   8A0B-D3FD / 1EB06503 / 7A0B5949   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	0252599-U72 Series 01 M27v322 MK6USA   MR WOO   00D4BUC0   4455-743F / 1E6C3D72 / 558D390F   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	0252599-U84 Series 01 M27v322 MK6USA   MR WOO   3F150U79   BD85-B259 / 0E95772A / 519661F4   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	0252599-U71 Series 01 M27v322 MK6USA   MR WOO   76112F88   CC58-7443 / 0E908355 / 4EA06918   Copyright © 2006 Aristocrat Technologies Australia Pty Ltd.
	*/
	ROM_LOAD32_WORD("0252599.u86", 0x0000000, 0x0400000, CRC(db5d522a) SHA1(62030c7e314a614a3d4d7386268b61f94bc1fe02) )
	ROM_LOAD32_WORD("0252599.u73", 0x0000002, 0x0400000, CRC(59cdb6ee) SHA1(1e8be1396786fced872c8ffc09772183b98a0f96) )
	ROM_LOAD32_WORD("0252599.u85", 0x0800000, 0x0400000, CRC(cb26bd37) SHA1(f20d54c5a6750362f47da5cac9b74ae516de16d4) )
	ROM_LOAD32_WORD("0252599.u72", 0x0800002, 0x0400000, CRC(6fed3a7e) SHA1(b07354d5d927133626232c21096465426b3053bd) )
	ROM_LOAD32_WORD("0252599.u84", 0x1000000, 0x0400000, CRC(0fe46063) SHA1(10f3895b164b3077d156b7eb22e1fb1356816cb3) )
	ROM_LOAD32_WORD("0252599.u71", 0x1000002, 0x0400000, CRC(7507a271) SHA1(3bd25bf8d1ea54a7db2537fd011b12ff51e7bdd1) )
ROM_END


// 0151009 / US005
// Mystic Arrow / Local / A - 01/07/02
// Variations (% and NO):
//    87.085% 99 (untested, variations above 04 may not be valid)
//    90.041% 01
//    92.081% 02
//    94.918% 03
//    97.022% 04
//    82.070% 05
//    85.070% 06
//    88.480% 07
//    90.510% 08
ROM_START( mystarr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151009.u86", 0x0000000, 0x0400000, CRC(8a1475ac) SHA1(c948f0ca991b64d9a07e42fdb346e6df450718f2) )
	ROM_LOAD32_WORD("0151009.u73", 0x0000002, 0x0400000, CRC(7b8ed0d9) SHA1(6ea4bddedef955513e841ee14ec5e6ded23c1ea7) )
	ROM_LOAD32_WORD("0151009.u85", 0x0800000, 0x0400000, CRC(ea82c5db) SHA1(01db2730b1eec5c9f5c011525459ec72903761cc) )
	ROM_LOAD32_WORD("0151009.u72", 0x0800002, 0x0400000, CRC(4aa6e68b) SHA1(5f015885c15fd206053d23983a38dd1dbb8f89ce) )
ROM_END


// 10008111 / 675
// MYSTIC EYES (MR. CASHMAN) / Local / A- 09/05/01
// Variations (% and NO):
//    87.56% 99
ROM_START( mysteycm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10008111.u86", 0x0000000, 0x0400000, CRC(3de4876b) SHA1(76eb7bdc41dc134656452f8e0d08f009c02d9205) )
	ROM_LOAD32_WORD("10008111.u73", 0x0000002, 0x0400000, CRC(fd1d56df) SHA1(2f22f83995bdea66a055a1aa3cb0e8dd2c8f87cb) )
	ROM_LOAD32_WORD("10008111.u85", 0x0800000, 0x0400000, CRC(3449d07c) SHA1(2b36edc554f157e465bcaada42d6cbfee545e146) )
	ROM_LOAD32_WORD("10008111.u72", 0x0800002, 0x0400000, CRC(ee58e1e8) SHA1(1aa57c88b97a3bed04bcaabe77f231967d092f4c) )
ROM_END


// 0251024 / US020
// MYSTIC EYES CM / Local / C- 31/05/02
// Variations (% and NO):
//    92.395% 99
ROM_START( mysteycmu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251024.u86", 0x0000000, 0x0400000, CRC(a7bdbd03) SHA1(0e763933c082163de85c408be91d216d6545e9d7) )
	ROM_LOAD32_WORD("0251024.u73", 0x0000002, 0x0400000, CRC(85ad1a00) SHA1(d4f1d86647eb7724c94c3cfa89d5545a20323e62) )
	ROM_LOAD32_WORD("0251024.u85", 0x0800000, 0x0400000, CRC(ee781c0f) SHA1(7729a55e1d5ff7b8a373dda8a0798374ff60a89b) )
	ROM_LOAD32_WORD("0251024.u72", 0x0800002, 0x0400000, CRC(380a868e) SHA1(a1ac6ec1a760f8bbf68b3b86cf5dd4623dc130c7) )
ROM_END


// 1J008711 / JB013
// Mystic Mermaid / NSW/ACT / D - 27/03/02
// Variations (% and NO):
//    87.99% 99
//    90.10% 01
//    92.18% 02
ROM_START( mystmerm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008711.u86", 0x0000000, 0x0400000, CRC(be84df3c) SHA1(b39deff7d8d0ab30e3dbc0f33c4d45329734a05a) )
	ROM_LOAD32_WORD("1j008711.u73", 0x0000002, 0x0400000, CRC(d571a0bc) SHA1(3bb3c9373fcd7c730dfd0de749d7dacb8aadaf53) )
	ROM_LOAD32_WORD("1j008711.u85", 0x0800000, 0x0400000, CRC(803023c2) SHA1(c953b3fa12347a18fa48b5ea691ad61720c2efdd) )
	ROM_LOAD32_WORD("1j008711.u72", 0x0800002, 0x0400000, CRC(a060d4b2) SHA1(5832a7f4b808017cc251a58e9cce004b2ec41e0c) )
ROM_END


// 10197311 / 778
// Nerds Gone Wild / Local / C - 18/04/05
// Variations (% and NO):
//    87.57% 99
//    90.19% 01
ROM_START( nerdwild )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10197311.u86", 0x0000000, 0x0400000, CRC(a14e80d4) SHA1(f1729ff10991056e1880b5c0db3bdb760b1c664b) )
	ROM_LOAD32_WORD("10197311.u73", 0x0000002, 0x0400000, CRC(be64ac6d) SHA1(c0e6f68467ff248e5c619cedde74091783dafecf) )
ROM_END


// 0351309
// Open Sesame
ROM_START( opsesam )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351309.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0351309.u73", 0x0000002, 0x0400000, CRC(d6bc2a4b) SHA1(55daa6c9028f40ecd23c5ffdb049447ce5ce51aa) )
	ROM_LOAD32_WORD("0351309.u85", 0x0800000, 0x0400000, CRC(3bfdcf54) SHA1(4b174a3e65e0c3b874f99a1bc8947ebe35087672) )
	ROM_LOAD32_WORD("0351309.u72", 0x0800002, 0x0400000, CRC(41bf83e5) SHA1(cdd9c6a148970725360c763ed11e2e411c28d5e9) )
ROM_END


// 10213511 / 808
// Oriental Beauty / Local / A - 07/02/06
// Variations (% and NO):
//    87.71% 99
//    90.17% 01
//    91.96% 02
ROM_START( orbeaut )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10213511.u86", 0x0000000, 0x0400000, CRC(4d3f0586) SHA1(da38f11e48191fd53492aa5a4875fcdc33d30c24) )
	ROM_LOAD32_WORD("10213511.u73", 0x0000002, 0x0400000, CRC(8c977e22) SHA1(4de3e30995124b89354d660fed97d29574a40533) )
ROM_END


// 20195211 / 691
// Outback Jack / Local / J - 02/06/05
// Variations (% and NO):
//    87.79% 99
//    85.72% 01
ROM_START( outbackj )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20195211.u11", 0x0000000, 0x0400000, CRC(8eea3cea) SHA1(b2353fc953f6daf03c1458e06006de31fdd7a7c3) )
	ROM_LOAD32_WORD("20195211.u21", 0x0000002, 0x0400000, CRC(972eaba2) SHA1(b927aac04e21922b995aa2587a76ef14c35cd2bd) )
	ROM_LOAD32_WORD("20195211.u12", 0x0800000, 0x0400000, CRC(a1853f27) SHA1(cd2bf8674555f6870f0689ca0adbca6890bc9b27) )
	ROM_LOAD32_WORD("20195211.u22", 0x0800002, 0x0400000, CRC(cdd28848) SHA1(0038bb422a1d650beaa0ffd63b7d6de1936a5e32) )
	ROM_LOAD32_WORD("20195211.u13", 0x1000000, 0x0400000, CRC(0cd41b5d) SHA1(a7891da788fe88d2191b2f0686cb9ca2ecfeeb9c) )
	ROM_LOAD32_WORD("20195211.u23", 0x1000002, 0x0400000, CRC(23f1f50c) SHA1(40145b839c2b5fde5c0f20e08f46f5a859b6e20a) )
	ROM_LOAD32_WORD("20195211.u14", 0x1800000, 0x0400000, CRC(79ca67a1) SHA1(68bae23df71c768dc17631f6ccfc7a720c0959ee) )
	ROM_LOAD32_WORD("20195211.u24", 0x1800002, 0x0400000, CRC(3a0ea3ca) SHA1(88513130013b1b18772a5e538da9a8db3a54633e) )
ROM_END


// 10008711 / 594/9
// OWL CAPONE / Local / A - 12/06/01
// SHOW PROGRAM
// Variations (% and NO):
//    90.10% 99
//    92.04% 01
//    95.03% 02
//    87.77% 05
//    93.45% 09
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( owlcap )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10008711.u86", 0x0000000, 0x0400000, CRC(66b2dc21) SHA1(1eeff14dd037c8c4c7fc95a04a8dc5946200d453) )
	ROM_LOAD32_WORD("sp__10008711.u73", 0x0000002, 0x0400000, CRC(c9197a1c) SHA1(2b4649c74f624e3414732027ca77536bcaced11f) )
	ROM_LOAD32_WORD("sp__10008711.u85", 0x0800000, 0x0400000, CRC(8de48e1e) SHA1(c52592f808f02039c7f2d9358076e3dd91f152a2) )
	ROM_LOAD32_WORD("sp__10008711.u72", 0x0800002, 0x0400000, CRC(b6be5274) SHA1(28c3a743cfef76c05567cdb7cc4ff5c60c594e1e) )
ROM_END


// 0151109 / US092
// PANDA PAYS CM / Local / A - 01/27/04
// Variations (% and NO):
//    92.397% 99
ROM_START( pandpays )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151109.u86", 0x0000000, 0x0400000, CRC(ba3fe38b) SHA1(11493dff5720e158ca31f72bf59dcbbbe6d87c96) )
	ROM_LOAD32_WORD("0151109.u73", 0x0000002, 0x0400000, CRC(cc63af55) SHA1(0d8d3c6392c3121d5edbeaece880041c3afd6530) )
	ROM_LOAD32_WORD("0151109.u85", 0x0800000, 0x0400000, CRC(ac7672fa) SHA1(8029ff08e29c48e8f4b2ecd03cf021cc89eab916) )
	ROM_LOAD32_WORD("0151109.u72", 0x0800002, 0x0400000, CRC(4c4b9a62) SHA1(1c287ac85346756508e2efb13e1559af4956dd9a) )
ROM_END


// 30002611 / 670
// Paris Lights / Local / A - 03/05/01
// Variations (% and NO):
//    87.89% 99
//    90.14% 01
//    92.15% 02
ROM_START( parislgt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30002611.u86", 0x0000000, 0x0400000, CRC(89508f23) SHA1(10ab87fc7c726d2d2a6c73151f2ef731b3d79979) )
	ROM_LOAD32_WORD("30002611.u73", 0x0000002, 0x0400000, CRC(05e567a5) SHA1(c605033599f5fd92b20013264c0bf9eb7076ee85) )
	ROM_LOAD32_WORD("30002611.u85", 0x0800000, 0x0400000, CRC(32eb5b34) SHA1(165c61e20ae8d52575fc0402b1cbc49bbb18d2af) )
	ROM_LOAD32_WORD("30002611.u72", 0x0800002, 0x0400000, CRC(d04e5031) SHA1(46ef9fe928409ec031afa97f67b271cb0059c603) )
ROM_END


// 10139011
// Paris Lights / ASP
ROM_START( parislgta )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10139011.u86", 0x0000000, 0x0400000, CRC(792c925d) SHA1(08264f7e182e0f6001ceb7058c3d11d6ca106819) )
	ROM_LOAD32_WORD("10139011.u73", 0x0000002, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10139011.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10139011.u72", 0x0800002, 0x0400000, NO_DUMP )
ROM_END


// 0251031 / US022
// Paris Lights JC / Local / A - 07/03/02
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    87.893% 99
//    90.143% 01
//    92.151% 02
//    94.832% 03
//    97.180% 04
//    82.703% 05 (Hyperlink)
ROM_START( parislgtu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251031.u86", 0x0000000, 0x0400000, CRC(5224e683) SHA1(8b0dbb2fcea4d1a522160d8a6634687c5bad8acd) )
	ROM_LOAD32_WORD("0251031.u73", 0x0000002, 0x0400000, CRC(d0587548) SHA1(30d0021a78ee79c6f66fb27c8c484427a5a2db1b) )
	ROM_LOAD32_WORD("0251031.u85", 0x0800000, 0x0400000, CRC(e7ed0bf4) SHA1(39cb9744462f10fc791d6c604631c1182da9466b) )
	ROM_LOAD32_WORD("0251031.u72", 0x0800002, 0x0400000, CRC(8dc6a0ec) SHA1(855e8f5c6752bfc4ba25116a20ed82dfb4912c07) )
ROM_END


// 1J008811 / JB014
// Peacock Magic / NSW/ACT / B - 09/05/02
// Variations (% and NO):
//    88.01% 99
//    90.04% 01
//    92.18% 02
ROM_START( pcockmag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008811.u86", 0x0000000, 0x0400000, CRC(5348f934) SHA1(82762d643729639a1f059a1b57b74c8ab240277c) )
	ROM_LOAD32_WORD("1j008811.u73", 0x0000002, 0x0400000, CRC(106bd1d4) SHA1(4ecfafc883da969eb1b46c00e8b67aedfdb001bf) )
	ROM_LOAD32_WORD("1j008811.u85", 0x0800000, 0x0400000, CRC(6453532c) SHA1(c9b885e42778af56df89520cb6e4856348958110) )
	ROM_LOAD32_WORD("1j008811.u72", 0x0800002, 0x0400000, CRC(0519288c) SHA1(a6e838d750f17131bf0871eac8742408cb03db6c) )
ROM_END


// 10134311 / JB014
// Peacock Magic / ASP / C - 27/05/02
// Variations (% and NO):
//    88.01% 99
ROM_START( pcockmaga )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10134311.u86", 0x0000000, 0x0400000, CRC(aedc9e65) SHA1(33e993f10a6287a770032f166edafbf3f4564d9f) )
	ROM_LOAD32_WORD("10134311.u73", 0x0000002, 0x0400000, CRC(75d5333b) SHA1(523d2983e31acd18cef18e2b8db3bd84fd726ee2) )
	ROM_LOAD32_WORD("10134311.u85", 0x0800000, 0x0400000, CRC(83542cfb) SHA1(56b1dd319d9eb8287db8cbb80b12031dab0e0c74) )
	ROM_LOAD32_WORD("10134311.u72", 0x0800002, 0x0400000, CRC(41aeadf2) SHA1(37cfe89021e4c9f2f99a1727f00b7ac0ee984620) )
ROM_END


// 0151352 / JB014/20
// Peacock Magic / Local / C - 07/06/04
// Variations (% and NO):
//    87.982% 99
//    90.178% 01
//    92.197% 02
//    94.967% 03
//    96.957% 04
ROM_START( pcockmagu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151352-U86 Series:01 M27V322 Mk6USA   Peacock Magic   L04-1953-01   070AD89H   4247-0268 / 1FDF7221 / 064D220D   Copyright © 2003 Aristocrat Technologies, Inc.
	0151352-U73 Series:01 M27V322 Mk6USA   Peacock Magic   L04-1953-02   9P91A705   D2D9-D9C4 / 1F689986 / E8C34AA1   Copyright © 2003 Aristocrat Technologies, Inc.
	0151352-U85 Series:01 M27V322 Mk6USA   Peacock Magic   L04-1953-03   E3972A45   0A6A-2BCA / 0D962D1C / C0D272E0   Copyright © 2003 Aristocrat Technologies, Inc.
	0151352-U72 Series:01 M27V322 Mk6USA   Peacock Magic   L04-1953-04   6HB4245H   34D1-A37C / 0D800D59 / B6DD3821   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151352.u86", 0x0000000, 0x0400000, CRC(53a9a249) SHA1(097e93185efe87edb8e8cf512bd07ed6de120ff2) )
	ROM_LOAD32_WORD("0151352.u73", 0x0000002, 0x0400000, CRC(8d2fac0f) SHA1(35e5ae327d0b6145b88b577ee6ba22589b2cb6dc) )
	ROM_LOAD32_WORD("0151352.u85", 0x0800000, 0x0400000, CRC(7bbc3359) SHA1(b497998475274768b831963f8df2748832772369) )
	ROM_LOAD32_WORD("0151352.u72", 0x0800002, 0x0400000, CRC(4d9912ca) SHA1(8a019b961736b30b076a8a1fdc24cbe48cfb6021) )
ROM_END


// 10177511 / 778
// Pelican Pete / Local / A - 22/11/04
// Variations (% and NO):
//    87.57% 99
//    90.19% 01
//    91.83% 02
ROM_START( pelipete )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177511.u86", 0x0000000, 0x0400000, CRC(9fb0db2d) SHA1(c4c7f5f91045778dce2229919e00aa1bdb625084) )
	ROM_LOAD32_WORD("10177511.u73", 0x0000002, 0x0400000, CRC(8626fcd5) SHA1(fb08bddf8e30ef537af7f96c632d0690f241cef3) )
	ROM_LOAD32_WORD("10177511.u85", 0x0800000, 0x0400000, CRC(99c2748b) SHA1(712c103be109839d02ace90c1498192792185733) )
	ROM_LOAD32_WORD("10177511.u72", 0x0800002, 0x0400000, CRC(6e7d67d9) SHA1(163449982405d2cd078c757d885113edbcddbd22) )
ROM_END


// 10196211 / 778/1
// Pelican Pete / Local / B - 26/07/05
// Variations (% and NO):
//    87.69% 99
ROM_START( pelipetea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10196211.u86", 0x0000000, 0x0400000, CRC(853bd004) SHA1(6572abfc50a3e2c48a0a509426be340388611523) )
	ROM_LOAD32_WORD("10196211.u73", 0x0000002, 0x0400000, CRC(b5f8f1bb) SHA1(8dd28655a1a39f091ffa23cc3c467ce01d3480b8) )
	ROM_LOAD32_WORD("10196211.u85", 0x0800000, 0x0400000, CRC(b39c2b82) SHA1(f501687c47657cff5f2838483177972d1f840166) )
	ROM_LOAD32_WORD("10196211.u72", 0x0800002, 0x0400000, CRC(cc835e2b) SHA1(d36dd10ab1e1c9d236ca656d18e395b4deac2281) )
ROM_END


// 10226711 / 778/2
// Pelican Pete - Jackpot Carnival / QCOM / E - 23/05/06
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.08% 05 (Hyperlink)
ROM_START( pelipetejc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10226711.u86", 0x0000000, 0x0400000, CRC(871a5bf4) SHA1(9a73b67c6e90538f889f274c36b9144e918d2d02) )
	ROM_LOAD32_WORD("10226711.u73", 0x0000002, 0x0400000, CRC(e3bd402c) SHA1(6be4b1a2bb9f8b985c4e93c1eb29832f1727ccf5) )
	ROM_LOAD32_WORD("10226711.u85", 0x0800000, 0x0400000, CRC(c5a038e8) SHA1(ecfc2a5d023d9a3a627c399337062743b288531a) )
	ROM_LOAD32_WORD("10226711.u72", 0x0800002, 0x0400000, CRC(58dda04b) SHA1(7c7219b4478d8d392f1b15588aba3cd6febe861e) )
ROM_END


// 0252555 / MV4251
// Pelican Pete / Local / C - 11/07/05
// Variations (% and NO):
//    87.569% 99 (1 credit buys 2 lines)    87.689% 99 (1 credit buys 1 line)
//    90.192% 01 (1 credit buys 2 lines)    89.913% 01 (1 credit buys 1 line)
//    91.832% 02 (1 credit buys 2 lines)    91.657% 02 (1 credit buys 1 line)
//    95.416% 03 (1 credit buys 2 lines)    94.765% 03 (1 credit buys 1 line)
//                      96.645% 04 (1 credit buys 1 line)
// Untested variations, verification needed
ROM_START( pelipeteu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0252555-U86 Series 01 M27v322 MK6USA   PELICAN PETE   4P254783   147F-9E71 / 1F093B1F / 8E6FCEA6   Copyright © 2004-2005 Aristocrat Technologies Inc
	0252555-U73 Series 01 M27v322 MK6USA   PELICAN PETE   462HAA2P   5871-985A / 1EF0B748 / 89FDD7A0   Copyright © 2004-2005 Aristocrat Technologies Inc
	0252555-U85 Series 01 M27v322 MK6USA   PELICAN PETE   25F5BC28   C4EC-940A / 12EACB48 / 80709B7B   Copyright © 2004-2005 Aristocrat Technologies Inc
	0252555-U72 Series 01 M27v322 MK6USA   PELICAN PETE   55AF426A   5053-B8C9 / 12C0F94B / 74E69221   Copyright © 2004-2005 Aristocrat Technologies Inc
	*/
	ROM_LOAD32_WORD("0252555.u86", 0x0000000, 0x0400000, CRC(dea78964) SHA1(857e5f1deb8db3d861ea7fe78e44a8e17cc791aa) )
	ROM_LOAD32_WORD("0252555.u73", 0x0000002, 0x0400000, CRC(44d5e0af) SHA1(1693e641ddc5d4174e4e0dc07cb79e54045aaad3) )
	ROM_LOAD32_WORD("0252555.u85", 0x0800000, 0x0400000, CRC(781e076d) SHA1(0c7d2281598822ea45f37b2afd5f12d9afe2d01a) )
	ROM_LOAD32_WORD("0252555.u72", 0x0800002, 0x0400000, CRC(4ff0a40b) SHA1(b9049a35a3ebc3e1274498809f7687a36cad6f24) )
ROM_END


// 0251169 / US159
// Penny Arcade (NGCB) / Local / A - 04/06/05
// Variations (% and NO):
//    87.269% 99 (Reel 1)    87.269% 99 (Reels 1-2)    87.804% 99 (Reels 1-3)    87.814% 99 (Reels 1-4)    87.966% 99 (Reels 1-5)
//    90.059% 01 (Reel 1)    90.141% 01 (Reels 1-2)    90.271% 01 (Reels 1-3)    90.331% 01 (Reels 1-4)    90.441% 01 (Reels 1-5)
//    92.428% 02 (Reel 1)    92.511% 02 (Reels 1-2)    92.641% 02 (Reels 1-3)    92.846% 02 (Reels 1-4)    92.874% 02 (Reels 1-5)
//    95.060% 03 (Reel 1)    95.142% 03 (Reels 1-2)    95.560% 03 (Reels 1-3)    95.585% 03 (Reels 1-4)    95.678% 03 (Reels 1-5)
//    97.200% 04 (Reel 1)    97.282% 04 (Reels 1-2)    97.550% 04 (Reels 1-3)    97.593% 04 (Reels 1-4)    97.676% 04 (Reels 1-5)
ROM_START( pennyar )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251169.u86", 0x0000000, 0x0400000, CRC(0fee6e4a) SHA1(b2a7ccbe81528067b29663f18b54520b5fefe6ec) )
	ROM_LOAD32_WORD("0251169.u73", 0x0000002, 0x0400000, CRC(b8b7c31c) SHA1(7a5cfd608d8c0a3db1054b5770065fc3733d5f35) )
	ROM_LOAD32_WORD("0251169.u85", 0x0800000, 0x0400000, CRC(9b9bf428) SHA1(108c722d0b56ae446961eefe27579e203eeb3b19) )
	ROM_LOAD32_WORD("0251169.u72", 0x0800002, 0x0400000, CRC(cdd0795f) SHA1(e5243c27cca69451015f232444b289abf3d95819) )
ROM_END


// 0151120 / US055
// Penquin Pays / Local / A- 4/14/03
// Penguin Pays title is misspelled in the ROM
// Cash Express Hyperlink game
// Variations (% and NO):
//    87.634% 99
//    90.445% 01
//    92.130% 02
//    94.640% 03
//    97.001% 04
//    81.980% 05 (Hyperlink)
ROM_START( pengpyce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151120.u86", 0x0000000, 0x0400000, CRC(b7c646a7) SHA1(e47f88347eb2354dd982f46f7ac89b5645205b12) )
	ROM_LOAD32_WORD("0151120.u73", 0x0000002, 0x0400000, CRC(36699b29) SHA1(0d867f4086c07c8114bb1f383e647dab4d8ce7b8) )
	ROM_LOAD32_WORD("0151120.u85", 0x0800000, 0x0400000, CRC(7e68ca9a) SHA1(17e96fa047b2353be53b52b0a783bb7247bc6a6f) )
	ROM_LOAD32_WORD("0151120.u72", 0x0800002, 0x0400000, CRC(1ee80a47) SHA1(c02485003d0724941ef346fb721bd246f69545b3) )
ROM_END


// 40000921 / 656
// Pharaoh's Fortune / Local / D - 11/01/00
// Variations (% and NO):
//    87.93% 99
//    90.16% 01
//    92.10% 02
ROM_START( pharfor )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40000921.u86", 0x0000000, 0x0400000, CRC(6d435495) SHA1(22644acb9ed28e737bc132698cd12048c76c4b80) )
	ROM_LOAD32_WORD("40000921.u73", 0x0000002, 0x0400000, CRC(33236ae6) SHA1(a9c41af9982c1a7035c5190732775a3eca4ebb78) )
	ROM_LOAD32_WORD("40000921.u85", 0x0800000, 0x0400000, CRC(158af672) SHA1(f653f9d400f03713baa650912c8c0ab33f218fb2) )
	ROM_LOAD32_WORD("40000921.u72", 0x0800002, 0x0400000, CRC(c5d24a99) SHA1(d6d17ab5255d153beacc39880bd58b7b647f532f) )
ROM_END


// 20197511 / 793
// Phoenix Fantasy / Local / A - 21/07/05
// Variations (% and NO):
//    87.55% 99
//    90.02% 01
//    91.88% 02
ROM_START( phxfant )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197511.u86", 0x0000000, 0x0400000, CRC(499742c6) SHA1(7743b24e658e4ee60255e9bf2b33a4937d19e3b4) )
	ROM_LOAD32_WORD("20197511.u73", 0x0000002, 0x0400000, CRC(1854f81c) SHA1(fc97f1c55db5c66d2261de62a538cc0308988702) )
	ROM_LOAD32_WORD("20197511.u85", 0x0800000, 0x0400000, CRC(db092fc5) SHA1(c2469c131d1240faede3e6e64ee818df4ce9581d) )
	ROM_LOAD32_WORD("20197511.u72", 0x0800002, 0x0400000, CRC(3ea207d5) SHA1(ae9b032f6ab90630bb047fa508d539874bd86d18) )
ROM_END


// 10001511 / 661/1
// Pirates / Local / B - 3/11/00
// Variations (% and NO):
//    87.16% 99
//    90.08% 01
//    92.07% 02
ROM_START( pirata6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001511.u86", 0x0000000, 0x0400000, CRC(ed435c4a) SHA1(98750a2673411c3b193f25f2998755314bd60331) )
	ROM_LOAD32_WORD("10001511.u73", 0x0000002, 0x0400000, CRC(5923957d) SHA1(62d6effe9f3af0cdc301bbc6541971e91ddc573a) )
ROM_END


// 10122311 / 661/1
// Pirates / ASP Game / C - 30/03/01
// Variations (% and NO):
//    87.16% 99
ROM_START( pirata6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122311.u86", 0x0000000, 0x0400000, CRC(7d3a11c6) SHA1(b1ea0b1f225aca4e869e3e5d3fd7695ae5681726) )
	ROM_LOAD32_WORD("10122311.u73", 0x0000002, 0x0400000, CRC(0da8dc38) SHA1(d1d83ed9090771a26895e339116490234e95c7fe) )
	ROM_LOAD32_WORD("10122311.u85", 0x0800000, 0x0400000, CRC(6cd03c0a) SHA1(07e67ecfabd68197b9e8c5d4979b3b7b0675b286) )
	ROM_LOAD32_WORD("10122311.u72", 0x0800002, 0x0400000, CRC(96bcb407) SHA1(22a108b9a90e4b6e1e2383f9a2e3f8c365b99868) )
ROM_END


// 10223111 / MG0005
// Player's Choice - By Demand / D - 25/09/06
// 4-in-1 game containing Big Red, Black Panther, Fa Fa Fa and Super Bucks IV
// Variations (% and NO):
//    90% 99
//    88% 01
//    92% 02
//    95% 03
//    93.5% 04
// Big Red / 803 / E - 18/10/06
// Variations (% and NO):
//    90.07% 99
//    87.83% 01
//    91.93% 02
//    94.78% 03
//    93.61% 04
// Black Panther / 736/1 / B - 28/07/06
// Variations (% and NO):
//    90.10% 99
//    87.77% 01
//    92.04% 02
//    94.80% 03
//    93.54% 04
// Fa Fa Fa / 802 / B - 26/6/06
// Variations (% and NO):
//    90.20% 99
//    87.95% 01
//    91.86% 02
//    94.85% 03
//    93.69% 04
// Super Bucks IV / 749 / E - 15/09/06
// Variations (% and NO):
//    90.06% 99
//    87.94% 01
//    92.17% 02
//    94.89% 03
//    93.41% 04
ROM_START( pcbydem )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10223111.u11", 0x0000000, 0x0400000, CRC(c4f95ab2) SHA1(dd0a3cdaa7b421cbcfa50b94b8bbfb69769343cd) )
	ROM_LOAD32_WORD("10223111.u21", 0x0000002, 0x0400000, CRC(4a729ca3) SHA1(52be1fbf80a06f9959d62376102d96ea8f77f272) )
	ROM_LOAD32_WORD("10223111.u12", 0x0800000, 0x0400000, CRC(a9b81cc4) SHA1(541765e9fe2d67c60f014a8d32866c7b8203766a) )
	ROM_LOAD32_WORD("10223111.u22", 0x0800002, 0x0400000, CRC(81816747) SHA1(c745bc6be35976f4784730ab8b75e07688e49b22) )
	ROM_LOAD32_WORD("10223111.u13", 0x1000000, 0x0400000, CRC(309dbbf7) SHA1(69e44494bba43d001545c4e474ab4f712d1d8015) )
	ROM_LOAD32_WORD("10223111.u23", 0x1000002, 0x0400000, CRC(c71cb0e3) SHA1(c46d017bb26fb990a403cff7659b15a11f947c19) )
	ROM_LOAD32_WORD("10223111.u14", 0x1800000, 0x0400000, CRC(f3aab3c8) SHA1(a4c08d341f6b59b5faa14aa3381be066ecdc692a) )
	ROM_LOAD32_WORD("10223111.u24", 0x1800002, 0x0400000, CRC(adfaa968) SHA1(a1a8329523cbd14c24f8e6997a5be442327f61c7) )
ROM_END


// 20177311 / MG0002
// Player's Choice-Favourites / D - 04/11/04
// 4-in-1 game containing Prize Fight, Return of the Samurai, Black Panther and Super Bucks IV
// Variations (% and NO):
//    90% 99
//    88% 01
//    92% 02
//    95% 03
//    93.5% 04
// Prize Fight / 759 / B - 08/10/04
// Variations (% and NO):
//    90.08% 99
//    87.91% 01
//    91.87% 02
//    94.96% 03
//    93.45% 04
// Return Of The Samurai / 608/11 / A - 04/11/04
// Variations (% and NO):
//    90.11% 99
//    87.54% 01
//    92.38% 02
//    94.79% 03
//    93.66% 04
// Black Panther / 736 / B - 12/02/04
// Variations (% and NO):
//    90.10% 99
//    87.77% 01
//    92.04% 02
//    94.80% 03
//    93.54% 04
// Super Bucks IV / 749 / D - 14/04/04
// Variations (% and NO):
//    90.06% 99
//    87.94% 01
//    92.17% 02
//    94.89% 03
//    93.41% 04
ROM_START( pcfavor )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20177311.u86", 0x0000000, 0x0400000, CRC(f06d4fbb) SHA1(aad9c37da2ee9f7a5252267f39a9d1e8169a2472) )
	ROM_LOAD32_WORD("20177311.u73", 0x0000002, 0x0400000, CRC(72db5268) SHA1(de7f3619162b27bab4825f801eb1a04cce8721a3) )
	ROM_LOAD32_WORD("20177311.u85", 0x0800000, 0x0400000, CRC(385e8636) SHA1(5e4bf33f91481080c80264b915a11606b03f8686) )
	ROM_LOAD32_WORD("20177311.u72", 0x0800002, 0x0400000, CRC(9b0fb8b6) SHA1(bd5a2734068e031ca7ab0f19749f28f84288125f) )
	ROM_LOAD32_WORD("20177311.u84", 0x1000000, 0x0400000, CRC(7944e972) SHA1(23d45a2e948a737d3186de5f0447ec7dd51ddc7b) )
	ROM_LOAD32_WORD("20177311.u71", 0x1000002, 0x0400000, CRC(116bb514) SHA1(cd3dcca730c8e9d3fe5b0c70638f3d81f8eb8f98) )
ROM_END


// 30291411 / MG0047
// Player's Choice - Gold Edition / A - 09/04/10
// 4-in-1 game containing Adonis, K.G. Bird, Tigress and Wild Goose
// Variations (% and NO):
//    88% 99
//    90% 01
//    92% 02
//    95% 03
//    89% 04
// Adonis / 823 / D - 23/10/07
// Variations (% and NO):
//    87.87% 99
//    90.19% 01
//    91.83% 02
//    94.88% 03
//    88.30% 04
// KG Bird / 834 / E - 14/04/10
// Variations (% and NO):
//    87.84% 99
//    90.17% 01
//    92.18% 02
//    94.98% 03
//    88.48% 04
// Tigress / 807 / G - 17/03/09
// Variations (% and NO):
//    87.93% 99
//    90.05% 01
//    91.96% 02
//    94.92% 03
//    88.51% 04
// Wild Goose / 732 / E - 13/09/04
// Variations (% and NO):
//    87.62% 99
//    90.09% 01
//    92.15% 02
//    94.73% 03
//    88.72% 04
ROM_START( pcgold )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30291411.u11", 0x0000000, 0x0400000, CRC(bb839c90) SHA1(268008ffeeb867210664928a521051978c7770b3) )
	ROM_LOAD32_WORD("30291411.u21", 0x0000002, 0x0400000, CRC(6072d8d3) SHA1(37ce2a4213024b9ccd1859701c48e76e6c64e6ee) )
	ROM_LOAD32_WORD("30291411.u12", 0x0800000, 0x0400000, CRC(6208432e) SHA1(50006d010dd3d061fef8f4fd8fbbffea1b9b0f3d) )
	ROM_LOAD32_WORD("30291411.u22", 0x0800002, 0x0400000, CRC(db6e9523) SHA1(a4a272af4f5f29d2e612b52a011f9daf7c0b0cd3) )
	ROM_LOAD32_WORD("30291411.u13", 0x1000000, 0x0400000, CRC(5927770a) SHA1(faaec01280a54775e85ae796a7d05170bece3c87) )
	ROM_LOAD32_WORD("30291411.u23", 0x1000002, 0x0400000, CRC(8df4d7da) SHA1(a798424a514201d954519ec0daf0a2b1cbc00082) )
	ROM_LOAD32_WORD("30291411.u14", 0x1800000, 0x0400000, CRC(5709ba3a) SHA1(4fd85b97f434656b3bd0d26ba8573e3cd2c936ce) )
	ROM_LOAD32_WORD("30291411.u24", 0x1800002, 0x0400000, CRC(1d67f455) SHA1(4247d21757522a69184d32434cfa191a026aff19) )
ROM_END


// 20234911 / MG0010
// Player's Choice - Most Wanted
// 4-in-1 game containing Spring Carnival, Choy Sun Doa, Black Panther and Big Red
// Variations (% and NO):
//    89.10% 99
//    90.10% 01
// Spring Carnival / 642/5 / A - 22/08/02
// Variations (% and NO):
//    87.92% 99
//    90.17% 01
// Choy Sun Doa / 688/3 / C - 9/11/06
// Variations (% and NO):
//    87.96% 99
//    89.95% 01
// Black Panther / 736 / E - 21/1/08
// Variations (% and NO):
//    90.10% 99
//    87.77% 01
// Big Red / 803 / E - 08/10/06
// Variations (% and NO):
//    90.07% 99
//    87.83% 01
ROM_START( pcmostw )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20234911.u86", 0x0000000, 0x0400000, CRC(7f460e25) SHA1(d97150221a3c0cac3b46d147ae96937a871ecdb3) )
	ROM_LOAD32_WORD("20234911.u73", 0x0000002, 0x0400000, CRC(a26a224e) SHA1(222e3b067f4bde826ae19276bb48136896cbd853) )
	ROM_LOAD32_WORD("20234911.u85", 0x0800000, 0x0400000, CRC(214a647f) SHA1(51b250ecf49776ab9dfe80cb2efdc685109f0c19) )
	ROM_LOAD32_WORD("20234911.u72", 0x0800002, 0x0400000, CRC(d6f86683) SHA1(0e5ca980010bf40bba01d7222bc4d241b9d86667) )
	ROM_LOAD32_WORD("20234911.u84", 0x1000000, 0x0400000, CRC(da762c17) SHA1(44c5bfee805d0ee8209e7b37161921d652b4d83b) )
	ROM_LOAD32_WORD("20234911.u71", 0x1000002, 0x0400000, CRC(6b67d71e) SHA1(902f832c79f160b22c02222eefa5ec31285b7610) )
ROM_END


// 30143011 / MG0001
// Player's Choice/Original / A - 09/04/10
// 4-in-1 game containing 50 Lions, Indian Dreaming II, Pyramid Power and Queen of the Nile II
// Indian Dreaming II is an updated edition with 30 credit Reel Power bet similar to Thai Princess
// Queen of the Nile II has a similar feature to Show Me The Money, where the player can choose different free game/feature multiplier combinations
// Variations (% and NO):
//    88% 99
//    90% 01
//    92% 02
// 50 Lions / 717 / D - 20/10/03
// Variations (% and NO):
//    87.58% 99
//    90.03% 01
//    92.11% 02
// Indian Dreaming / 737 / B - 02/12/03
// Called 'Indian Dreaming' in ROM
// Variations (% and NO):
//    87.51% 99
//    89.86% 01
//    91.80% 02
// Pyramid Power / 662/2 / F  23/04/04
// Variations (% and NO):
//    87.60% 99
//    90.15% 01
//    92.06% 02
// Queen of the Nile II / 735 / B - 02/12/03
// Variations (% and NO):
//    88.07% 99
//    90.09% 01
//    91.78% 02
ROM_START( pcorig )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30143011.u11", 0x0000000, 0x0400000, CRC(457aa378) SHA1(248e636e33ea68a724ee074febdd7a3ed1b5d4e7) )
	ROM_LOAD32_WORD("30143011.u21", 0x0000002, 0x0400000, CRC(4caa461b) SHA1(b7b5770e9c0f4c7924f071d59eff94eea950b9c8) )
	ROM_LOAD32_WORD("30143011.u12", 0x0800000, 0x0400000, CRC(6f590385) SHA1(fc6229e1a45eaed819ec87fb1777b4ae508200a9) )
	ROM_LOAD32_WORD("30143011.u22", 0x0800002, 0x0400000, CRC(27941b7b) SHA1(5b5dbc8198b70d2965483e95f073213faa819d2e) )
	ROM_LOAD32_WORD("30143011.u13", 0x1000000, 0x0400000, CRC(17445b58) SHA1(f281b36ef9a72cf3e798993a7a80bf7fda1a6e8b) )
	ROM_LOAD32_WORD("30143011.u23", 0x1000002, 0x0400000, CRC(7eb0f277) SHA1(6a0ede9f64556223f284df4d015e200d98ec97fa) )
	ROM_LOAD32_WORD("30143011.u14", 0x1800000, 0x0400000, CRC(d4f04391) SHA1(3eb4b16518b3980505c8ee08542ae184c829f79d) )
	ROM_LOAD32_WORD("30143011.u24", 0x1800002, 0x0400000, CRC(e3b54c33) SHA1(4f1d22353b1114e476340dbec2d7d96e60d55cc2) )
ROM_END


// 20282111 / MG0031
// PLAYER'S CHOICE PLATINUM / B - 04/02/09
// 4-in-1 game containing Antony & Cleopatra, Miss Kitty, Reelin' N Boppin and Show Me The Money
// This version of Miss Kitty is not a 50 line game, it is a standard 5 reel, 3 position layout
// Variations (% and NO):
//    88% 99
//    90% 01
//    92% 02
// Antony & Cleopatra / MV4249 / B - 11/05/05
// Variations (% and NO):
//    87.93% 99
//    90.11% 01
//    91.92% 02
// Miss Kitty / 778/2 / F - 24/07/07
// Variations (% and NO):
//    87.57% 99
//    90.12% 01
//    91.92% 02
// Reelin' N Boppin / MV4121/2 / C - 24/09/08
// Variations (% and NO):
//    87.03% 99
//    90.22% 01
//    91.95% 02
// Show Me The Money / MV4126/1 / B - 15/04/02
// Variations (% and NO):
//    87.96% 99
//    90.04% 01
//    91.73% 02
ROM_START( pcplat )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U86
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U73
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U85
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U72
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U84
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U71
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U83
	20282111   NZCB   PLAVER'S CHOICE PLATINUM   MG0031   4MB   8 eproms   8/10/09   Aristocrat © 2008   P   U70
	'Player' is misspelled 'Plaver' on the labels
	*/
	ROM_LOAD32_WORD("20282111.u11", 0x0000000, 0x0400000, CRC(7ada7860) SHA1(4b8681f8ae3f279e38a9dba3b29585d7d0642f8c) )
	ROM_LOAD32_WORD("20282111.u21", 0x0000002, 0x0400000, CRC(44c0462c) SHA1(e6a99ddef5d48149003fa9009dcb9880c420fedf) )
	ROM_LOAD32_WORD("20282111.u12", 0x0800000, 0x0400000, CRC(51d192c5) SHA1(2b42d0ba8eb5b6ed1006ee452ca11ff295ee0a80) )
	ROM_LOAD32_WORD("20282111.u22", 0x0800002, 0x0400000, CRC(a22f17ed) SHA1(e3e6d50707873986b067a216d6dda25201224c53) )
	ROM_LOAD32_WORD("20282111.u13", 0x1000000, 0x0400000, CRC(545289a0) SHA1(af32941fb6c03c6711fc35c453546803aec0c85b) )
	ROM_LOAD32_WORD("20282111.u23", 0x1000002, 0x0400000, CRC(a66e230f) SHA1(712a08d879902d4ee6d1ee5896d0bbff5961a3f2) )
	ROM_LOAD32_WORD("20282111.u14", 0x1800000, 0x0400000, CRC(944b7425) SHA1(a87072edea72902e12dcda2e3ef7a0b35487b2f0) )
	ROM_LOAD32_WORD("20282111.u24", 0x1800002, 0x0400000, CRC(44ee59d6) SHA1(52dbc89c41e2aef844eaf64e734661c5945980d0) )
ROM_END


// 0154019 / MG0016
// Multigame XTREME HYPERLINK
// Conflicting title, 'Player's World - Super', but ROM says 'Multigame XTREME HYPERLINK'
// No date strings whatsoever
// 4-in-1 Cash Express Hyperlink game containing 5 Dragons, Tiki Torch, Turtle Treasure, and Where's The Gold
// Variations (% and NO):
//    87.194% 05 (Hyperlink)
// 5 Dragons / MV4280/4
// Variations (% and NO):
//    82.007% 05 (Hyperlink)
// Tiki Torch / MV4313/2
// Variations (% and NO):
//    87.57% 05 (Hyperlink)
// Turtle Treasure / MV4326/2
// Variations (% and NO):
//    82.080% 05 (Hyperlink)
// Where's The Gold / MV4220/2
// Variations (% and NO):
//    82.194% 05 (Hyperlink)
ROM_START( pwsuper )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0154019.u11", 0x0000000, 0x0400000, CRC(48f60a2c) SHA1(0d27d544991b3403672250ff79c5bbbf35cdf175) )
	ROM_LOAD32_WORD("0154019.u21", 0x0000002, 0x0400000, CRC(678b0be2) SHA1(16f5460d67a1e897390e0c03b847511f5c14156a) )
	ROM_LOAD32_WORD("0154019.u12", 0x0800000, 0x0400000, CRC(5354cff5) SHA1(bf818e53a432aa8e7a2578dbb367be2c5f51bb16) )
	ROM_LOAD32_WORD("0154019.u22", 0x0800002, 0x0400000, CRC(9dbdefb7) SHA1(445a24233b435e3812747d8376addf969fea5cd2) )
	ROM_LOAD32_WORD("0154019.u13", 0x1000000, 0x0400000, CRC(43b44f26) SHA1(5410eea0a4911a274a794d6b27e369fb98c6edf7) )
	ROM_LOAD32_WORD("0154019.u23", 0x1000002, 0x0400000, CRC(ae04211a) SHA1(8db4fc936fa6d0e0a372b81f2e9590047522b843) )
	ROM_LOAD32_WORD("0154019.u14", 0x1800000, 0x0400000, CRC(a850af7f) SHA1(8d029de0083aad57ddd8b36153214b40cd388d56) )
	ROM_LOAD32_WORD("0154019.u24", 0x1800002, 0x0400000, CRC(dbb073f1) SHA1(e3a7f14cabab613453baffde3b592889d11578e7) )
	ROM_LOAD32_WORD("0154019.u15", 0x2000000, 0x0400000, CRC(2c7fe966) SHA1(42fc364dded0ce079ce4970b1f951c6a0ebf6c51) )
	ROM_LOAD32_WORD("0154019.u25", 0x2000002, 0x0400000, CRC(5f9b8ef7) SHA1(f0c954d1f448f2061dbe284c78eddcfcb5afdc7c) )
ROM_END


// 10002211 / 628/5
// Pompeii / Local / A - 22/01/01
// Variations (% and NO):
//    87.15% 99
//    90.14% 01
//    92.32% 02
ROM_START( pompeia6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10002211.u86", 0x0000000, 0x0400000, CRC(0c4ef28a) SHA1(96381853ed23cb914e5e494fc90f671c2143a305) )
	ROM_LOAD32_WORD("10002211.u73", 0x0000002, 0x0400000, CRC(c56b6b9f) SHA1(3f4cb68620a0d993b1812768092b85353d7bc763) )
	ROM_LOAD32_WORD("10002211.u85", 0x0800000, 0x0400000, CRC(7569d484) SHA1(127d92b7bd62543789b25fbfce65e8429ddfc654) )
	ROM_LOAD32_WORD("10002211.u72", 0x0800002, 0x0400000, CRC(e3c02b5e) SHA1(714e83160917e83dafc058a9078ace800c4c3ebf) )
ROM_END


// 10122411 / 628/9
// Pompeii / ASP / C - 09/01/02
// Variations (% and NO):
//    87.05% 99
ROM_START( pompeia6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122411.u86", 0x0000000, 0x0400000, CRC(b2bcdcb8) SHA1(ae13d153265e8d847ecfa14f13c0999a66cfe9e1) )
	ROM_LOAD32_WORD("10122411.u73", 0x0000002, 0x0400000, CRC(e486f2df) SHA1(7a4fe1bb4a02f79ad64bb39faea97ee34bd63b05) )
	ROM_LOAD32_WORD("10122411.u85", 0x0800000, 0x0400000, CRC(cb9e0bf8) SHA1(e84a29d09512b83280cf9065960c191ed3892a24) )
	ROM_LOAD32_WORD("10122411.u72", 0x0800002, 0x0400000, CRC(8f8825e8) SHA1(dc3bfad9ce1bfd41669f52d9f181f728fd7cb062) )
ROM_END


// 0151158 / US008
// Pompeii / Local / A - 04/03/02
// Variations (% and NO):
//    87.136% 99
//    90.013% 01
//    92.171% 02
//    94.516% 03
//    96.999% 04
ROM_START( pompeia6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151158-U86 Series 01 M27V322 MK6USA   POMPEII   82C5A980   E4C8-1AFB / 1E21C0CD / 33A536AF   Copyright © 2005 Aristocrat Technologies Inc
	0151158-U73 Series 01 M27V322 MK6USA   POMPEII   40D46H66   AA1B-E7A2 / 1E03FF50 / 2FE01C36   Copyright © 2005 Aristocrat Technologies Inc
	0151158-U85 Series 01 M27V322 MK6USA   POMPEII   8P6536B8   CE04-9DBA / 0E3FD8F2 / 1EAC56F0   Copyright © 2005 Aristocrat Technologies Inc
	0151158-U72 Series 01 M27V322 MK6USA   POMPEII   F0392U11   038D-DE25 / 0E0459A1 / 0F12687C   Copyright © 2005 Aristocrat Technologies Inc
	*/
	ROM_LOAD32_WORD("0151158.u86", 0x0000000, 0x0400000, CRC(a34afbdc) SHA1(47e8ddc95ef9db933b3b6b26bb9d7f3f743d4e46) )
	ROM_LOAD32_WORD("0151158.u73", 0x0000002, 0x0400000, CRC(2a6a92ec) SHA1(98dc8506381089def51b8571038d6ece03d11faa) )
	ROM_LOAD32_WORD("0151158.u85", 0x0800000, 0x0400000, CRC(d71de7f8) SHA1(08692e432bed7e51655e0ebdcfc34b3d8f7f4682) )
	ROM_LOAD32_WORD("0151158.u72", 0x0800002, 0x0400000, CRC(da83f540) SHA1(8126e84ef6b44529632df119a30556f1ecd7c110) )
ROM_END


// 0251010 / US161
// Pompeii / Local / A - 04/15/05
// Variations (% and NO):
//    87.040% 99 (Reel 1)    87.040% 99 (Reels 1-2)    87.363% 99 (Reels 1-3)    87.876% 99 (Reels 1-4)    87.881% 99 (Reels 1-5)
//    90.063% 01 (Reel 1)    90.063% 01 (Reels 1-2)    90.155% 01 (Reels 1-3)    90.804% 01 (Reels 1-4)    90.872% 01 (Reels 1-5)
//    92.123% 02 (Reel 1)    92.123% 02 (Reels 1-2)    92.165% 02 (Reels 1-3)    92.942% 02 (Reels 1-4)    93.047% 02 (Reels 1-5)
//    95.689% 03 (Reel 1)    95.689% 03 (Reels 1-2)    95.719% 03 (Reels 1-3)    95.965% 03 (Reels 1-4)    95.978% 03 (Reels 1-5)
//    96.759% 04 (Reel 1)    96.759% 04 (Reels 1-2)    96.851% 04 (Reels 1-3)    97.482% 04 (Reels 1-4)    97.616% 04 (Reels 1-5)
ROM_START( pompeia6ua )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251010-U86 Series:01 27V322 Mk6USA   Pompeii   D2882087   94F3-B50D / 1DE40B32 / 1DD3CE70   Copyright © 2002 Aristocrat Technologies, Inc
	0251010-U73 Series:01 27V322 Mk6USA   Pompeii   FU44BU83   9795-2DCE / 1D999368 / 0DE1EDC6   Copyright © 2002 Aristocrat Technologies, Inc
	0251010-U85 Series:01 27V322 Mk6USA   Pompeii   A9E18HB4   F990-9B49 / 064566E9 / 19B7A225   Copyright © 2002 Aristocrat Technologies, Inc
	0251010-U72 Series:01 27V322 Mk6USA   Pompeii   E9E554E7   2163-9F0F / 063A751A / 16DB02D8   Copyright © 2002 Aristocrat Technologies, Inc
	*/
	ROM_LOAD32_WORD("0251010.u86", 0x0000000, 0x0400000, CRC(a68aac55) SHA1(630b1d4cb08d8710b81c6d709182e7c7a52f184a) )
	ROM_LOAD32_WORD("0251010.u73", 0x0000002, 0x0400000, CRC(947d3099) SHA1(d5256fc598c2e280f9e92ef6ba0e0152c8cc5d3b) )
	ROM_LOAD32_WORD("0251010.u85", 0x0800000, 0x0400000, CRC(901f5a17) SHA1(8a7fd00a68770be23e0164907eae91b963f01091) )
	ROM_LOAD32_WORD("0251010.u72", 0x0800002, 0x0400000, CRC(a5959d1f) SHA1(fe92eaa3f1dc6bdc356bef2a680eb9f8a09c88a2) )
ROM_END


// 10208511 / 628/9
// Pride of Africa / NSW / J - 06/12/05
// Variations (% and NO):
//    87.05% 99
//    90.04% 01
//    92.01% 02
ROM_START( prideaf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10208511.u86", 0x0000000, 0x0400000, CRC(efbf0ad5) SHA1(b4d84800c60df820bbe332879189208bac8330ef) )
	ROM_LOAD32_WORD("10208511.u73", 0x0000002, 0x0400000, CRC(86c22e24) SHA1(008fb01a7690981a3bc6a7f4a06dcebaef2a892c) )
ROM_END


// 20173411 / 734
// Prophecy / Local / B - 10/08/04
// Variations (% and NO):
//    87.98% 99
//    90.03% 01
//    91.90% 02
ROM_START( prophecy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20173411.u86", 0x0000000, 0x0400000, CRC(e5950eef) SHA1(930582c34f3639b887d490c96f91814cb5228d8b) )
	ROM_LOAD32_WORD("20173411.u73", 0x0000002, 0x0400000, CRC(683f081f) SHA1(4d625c67cb1d5fa5b025a02e797c4fc0a03893da) )
	ROM_LOAD32_WORD("20173411.u85", 0x0800000, 0x0400000, CRC(046ae548) SHA1(246015be05294806ecfacd626449e2df7e7f569c) )
	ROM_LOAD32_WORD("20173411.u72", 0x0800002, 0x0400000, CRC(194be1d6) SHA1(8f71ff6646a34261cb003fea877b42742eea8c15) )
ROM_END


// 10133721 / 687
// Queen of Sheba / Local / F - 04/03/03
// Variations (% and NO):
//    99.38% 99
//    97.13% 01
//    94.36% 02
//    92.92% 03
ROM_START( qsheba )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10133721.u86", 0x0000000, 0x0400000, CRC(e1313dff) SHA1(536014d8d4f3ff90d93befdcd55d5b3e6eb49e7c) )
	ROM_LOAD32_WORD("10133721.u73", 0x0000002, 0x0400000, CRC(fb26e09a) SHA1(e2debdcea7ee7be90e70c96e3eca593b515dfd2d) )
	ROM_LOAD32_WORD("10133721.u85", 0x0800000, 0x0400000, CRC(6f827f08) SHA1(b3777031c0a8d14557a640572c4dcfa781986b54) )
	ROM_LOAD32_WORD("10133721.u72", 0x0800002, 0x0400000, CRC(befa03be) SHA1(abd4846f29636aff70826b3086a972682c3bffe0) )
ROM_END


// 30146921 / 687
// Queen of Sheba / ASP / G - 29/07/03
// Variations (% and NO):
//    99.38% 99
ROM_START( qshebaa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30146921.u86", 0x0000000, 0x0400000, CRC(cba707bf) SHA1(861168f118cd3dbd309f2150b78e18b4a4621966) )
	ROM_LOAD32_WORD("30146921.u73", 0x0000002, 0x0400000, CRC(40aeb0c0) SHA1(1e80f9ece78b3df050ed849033c96e9cb3f37a5d) )
	ROM_LOAD32_WORD("30146921.u85", 0x0800000, 0x0400000, CRC(e205d34c) SHA1(bbfedcffc423888a6d1aed733c0016d2543967ae) )
	ROM_LOAD32_WORD("30146921.u72", 0x0800002, 0x0400000, CRC(6e34062a) SHA1(29515a8ab40aa6e005e96a077fbe86185aa476b1) )
ROM_END


// 3J011111 / JB027
// Queen of the Nile / Local / C - 16/02/05
// Variations (% and NO):
//    87.87% 99
//    90.31% 01
//    92.26% 02
ROM_START( qnilea6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("3j011111.u86", 0x0000000, 0x0400000, CRC(76818f74) SHA1(baac1a4a8ed3f1fb5a6ca321a6efe6ece7701486) )
	ROM_LOAD32_WORD("3j011111.u73", 0x0000002, 0x0400000, CRC(2f7f783c) SHA1(1739b05628bd02cb1b73c883d9f2e113da2c29e7) )
	ROM_LOAD32_WORD("3j011111.u85", 0x0800000, 0x0400000, CRC(4e53a089) SHA1(2dd060e4ff74f7fe73850a0c3877fa4bacad36c3) )
	ROM_LOAD32_WORD("3j011111.u72", 0x0800002, 0x0400000, CRC(491f4ef2) SHA1(fbaa64d9603900de44642f95fb9f7ee15e138669) )
ROM_END


// 10204311 / 602/4
// Queen of the Nile / ASP / D - 18/07/05
// Alternate string: H - 22/02/05
// Variations (% and NO):
//    87.87% 99
ROM_START( qnilea6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10204311.u86", 0x0000000, 0x0400000, CRC(62ccc5ed) SHA1(3e58a3ad5a69643e814442670cd5be4757effb7a) )
	ROM_LOAD32_WORD("10204311.u73", 0x0000002, 0x0400000, CRC(efaf8b80) SHA1(1bb95a33c13c9aec5faf47de540c566835029881) )
ROM_END


// 10192311 / 602/4
// Queen of the Nile / ASP / H - 22/02/05
// Variations (% and NO):
//    87.87% 99
ROM_START( qnilea6b )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10192311.u86", 0x0000000, 0x0400000, CRC(0f14b76b) SHA1(f6b3f4450d7731fe2fa4ab75497262510a0f0788) )
	ROM_LOAD32_WORD("10192311.u73", 0x0000002, 0x0400000, CRC(faffddf4) SHA1(ef359ad6c3b8d13f9bcf9e0bd1516e44392ebcd0) )
ROM_END


// 10203811 / 602/23
// Queen of The Nile Money Train II / QCOM / E - 17/11/05
// Money Train II Hyperlink game
// Variations (% and NO):
//    82.06% 05 (Hyperlink)
ROM_START( qnilemt2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10203811.u86", 0x0000000, 0x0400000, CRC(8822085a) SHA1(1cf43815eeab91cb3df18f4b64c6815b449184e3) )
	ROM_LOAD32_WORD("10203811.u73", 0x0000002, 0x0400000, CRC(01bfd58d) SHA1(8ba07b37624469bf802118a7778bb1d52b3649b8) )
	ROM_LOAD32_WORD("10203811.u85", 0x0800000, 0x0400000, CRC(364f2565) SHA1(f6c24c7f59f356c63c8ceda82243a5d99f4f81d8) )
	ROM_LOAD32_WORD("10203811.u72", 0x0800002, 0x0400000, CRC(af15bb42) SHA1(a5f8a7dd037fd6402db983b5cabaa156599ba9f2) )
ROM_END


// 20102811 / 683
// Queen of The Nile Special Edition / Local / A - 06/12/01
// Variations (% and NO):
//    87.87% 99
//    90.18% 01
//    92.15% 02
ROM_START( qnilese )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20102811.u86", 0x0000000, 0x0400000, CRC(b67fbebe) SHA1(a7fff03899caa9dba714928656e7c30e709453c0) )
	ROM_LOAD32_WORD("20102811.u73", 0x0000002, 0x0400000, CRC(d46bf3e8) SHA1(fa0c67ff8b7523bc2813e1df16507cd92cabd721) )
ROM_END


// 10102811 / 683
// Queen of The Nile Special Edition / Local / A - 06/12/01
// Same date string as parent
// Variations (% and NO):
//    87.87% 99
//    90.18% 01
//    92.15% 02
ROM_START( qnilesea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102811.u86", 0x0000000, 0x0400000, CRC(4c7bdba1) SHA1(796c9da25bbfe389aae5c03b42e15500c91a60d4) )
	ROM_LOAD32_WORD("10102811.u73", 0x0000002, 0x0400000, CRC(27391233) SHA1(e1d546cc9a57db894bdfb24592048578f5828ee5) )
ROM_END


// 10127411 / 683
// Queen of The Nile Special Edition / Local / A - 06/12/01
// Same date string as parent
// Variations (% and NO):
//    87.87% 99
ROM_START( qnileseb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10127411.u86", 0x0000000, 0x0400000, CRC(a83ac949) SHA1(7602f230a1138a00757403037f7535c1cba549e1) )
	ROM_LOAD32_WORD("10127411.u73", 0x0000002, 0x0400000, CRC(42062eee) SHA1(0ba373ccde2f83846aa8cceb689b00c3d3beb920) )
ROM_END


// 20123911 / 683/1
// Queen of The Nile Special Edition / Local / A - 21/10/02
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.05% 05 (Hyperlink)
ROM_START( qnilejc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20123911.u86", 0x0000000, 0x0400000, CRC(484fb352) SHA1(cc0c97895731dc3f333131ff5a47794fe1857e47) )
	ROM_LOAD32_WORD("20123911.u73", 0x0000002, 0x0400000, CRC(0ec5d0df) SHA1(206916986b1214a9635c901cbbcda21d21e0338a) )
	ROM_LOAD32_WORD("20123911.u85", 0x0800000, 0x0400000, CRC(a30473fd) SHA1(0af9fcee5df49d20847f98f94e09c0d394c1857a) )
	ROM_LOAD32_WORD("20123911.u72", 0x0800002, 0x0400000, CRC(a1aa9310) SHA1(a00a7dfd57fe8624421e6821c32516f018199d49) )
ROM_END


// 20123911 / 683/1
// Queen of The Nile Special Edition / Local / A - 21/10/02
// SHOW PROGRAM
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.05% 05 (Hyperlink)
// Identified as 201239AD in the Machine Identification menu, rather than 201239AV from the qnilejc set
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( qnilejcsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__20123911.u86", 0x0000000, 0x0400000, CRC(8c37947b) SHA1(4d3bf1192fb3939565afc482721289eae10eab5f) )
	ROM_LOAD32_WORD("sp__20123911.u73", 0x0000002, 0x0400000, CRC(ea401551) SHA1(939216babba395baf7283601f0873897f382648a) )
	ROM_LOAD32_WORD("sp__20123911.u85", 0x0800000, 0x0400000, CRC(0be660c4) SHA1(1b3da418f907bdee21ea9fb931f1fe4438109217) )
	ROM_LOAD32_WORD("sp__20123911.u72", 0x0800002, 0x0400000, CRC(c508e9d4) SHA1(59cf85729c1936e54b6c7adedf3941c24e299d48) )
ROM_END


// 10194211 / 762
// Rapid Riches / Local / D - 09/03/05
// Variations (% and NO):
//    87.81% 99
//    90.12% 01
//    91.82% 02
ROM_START( raprichs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10194211.u86", 0x0000000, 0x0400000, CRC(8dfb94ce) SHA1(67933b9edbd9631d2558d16267559af91084172b) )
	ROM_LOAD32_WORD("10194211.u73", 0x0000002, 0x0400000, CRC(cd13d765) SHA1(f91cb232726a038c3ff1a25259622dd99d552279) )
	ROM_LOAD32_WORD("10194211.u85", 0x0800000, 0x0400000, CRC(3fbfdf7f) SHA1(bcfd939d0fa4d0acc2544910940fb09d48a399b5) )
	ROM_LOAD32_WORD("10194211.u72", 0x0800002, 0x0400000, CRC(3b4ee6dd) SHA1(83ddfdc3d8bdf02395411d8a6721a9c65c64e69c) )
ROM_END


// 10119911 / 709
// Red Baron / Local / B - 21/11/02
// Variations (% and NO):
//    87.85% 99
//    90.13% 01
//    92.11% 02
ROM_START( redbara6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119911.u86", 0x0000000, 0x0400000, CRC(599a7170) SHA1(8e3b2730b9503baa798fd59d787052a9589e0447) )
	ROM_LOAD32_WORD("10119911.u73", 0x0000002, 0x0400000, CRC(6590c207) SHA1(8184ae0ac02f22787c631bc823bd0fbcd5d5bf05) )
	ROM_LOAD32_WORD("10119911.u85", 0x0800000, 0x0400000, CRC(e954aeac) SHA1(df8a9420df31c4ecc2cbe460cea4c0bd13958289) )
	ROM_LOAD32_WORD("10119911.u72", 0x0800002, 0x0400000, CRC(16b6207f) SHA1(9c5ac48cccba001b2e3605eceb9610c659e06e21) )
ROM_END


// 10160711 / 709
// Red Baron / QCOM / E - 04/12/03
// Fast Lane Hyperlink game
// Variations (% and NO):
//    82.10% 05 (Hyperlink)
ROM_START( redbarfl )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10160711.u86", 0x0000000, 0x0400000, CRC(6deec801) SHA1(7a3a7ee88d116cc3f78ca77b66247c0271130682) )
	ROM_LOAD32_WORD("10160711.u73", 0x0000002, 0x0400000, CRC(8467e3a6) SHA1(4abef9510a3d71166fa0ece928f936171de784b3) )
	ROM_LOAD32_WORD("10160711.u85", 0x0800000, 0x0400000, CRC(5d77ca06) SHA1(22c6cccf6f0ddb9ee95691180299c55b0b8b5747) )
	ROM_LOAD32_WORD("10160711.u72", 0x0800002, 0x0400000, CRC(91993744) SHA1(e8ac54b233eb47c525225318a07f2e58246c5131) )
ROM_END


// 10160911 / 709
// Red Baron / QCOM / E - 04/12/03
// Same date string as redbarfl
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.10% 05 (Hyperlink)
ROM_START( redbarjc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10160911.u86", 0x0000000, 0x0400000, CRC(fbeda427) SHA1(ef16afa9e672eedd0008239c08d622a8fb7f5926) )
	ROM_LOAD32_WORD("10160911.u73", 0x0000002, 0x0400000, CRC(fee10f9a) SHA1(74f07bcf50fb9c009556e8e8387874cca11bdd2b) )
	ROM_LOAD32_WORD("10160911.u85", 0x0800000, 0x0400000, CRC(74a2d581) SHA1(eaee3ff5423aab0fbc965eacc2be7c3a7d6d6b45) )
	ROM_LOAD32_WORD("10160911.u72", 0x0800002, 0x0400000, CRC(d071b4d4) SHA1(b81e1ba000bd27a43582cc126ee7ab21c68b6924) )
	ROM_LOAD32_WORD("10160911.u84", 0x1000000, 0x0400000, CRC(c34e320b) SHA1(8b4ffbd99270b89903385751816c54e5b5e71ce3) )
	ROM_LOAD32_WORD("10160911.u71", 0x1000002, 0x0400000, CRC(1d46694e) SHA1(331826afb6c3baa5703ceb55f7850809baba9418) )
ROM_END


// 10298611 / 628/2
// Reelin-n-Rockin Fast Lane / QCOM / E - 20/12/00
// Date string unchanged from MK5 version
// Fast Lane Hyperlink game
// Variations (% and NO):
//    82.05% 05 (Hyperlink)
ROM_START( reelrkfl )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10298611.u86", 0x0000000, 0x0400000, CRC(efb8f415) SHA1(b779fda43bb63d947f3ddad84cf324d530d20728) )
	ROM_LOAD32_WORD("10298611.u73", 0x0000002, 0x0400000, CRC(2eee9ff5) SHA1(0fb78b79df4efb71d1da850b11ca11c92fb47858) )
	ROM_LOAD32_WORD("10298611.u85", 0x0800000, 0x0400000, CRC(1f1c2b5b) SHA1(c2617a8d518f58cb622f3d9859a0f7d1766d359a) )
	ROM_LOAD32_WORD("10298611.u72", 0x0800002, 0x0400000, CRC(e58d6bc4) SHA1(d6cfb78c570f50f65c27c01f804720e499d623b3) )
ROM_END


// 10117211 / 608/10
// Return of the Samurai / Local / A - 26/08/02
// Variations (% and NO):
//    90.15% 99
//    87.57% 01
//    92.42% 02
//    94.84% 03
//    93.69% 04
ROM_START( retsama6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10117211.u86", 0x0000000, 0x0400000, CRC(8c6d706e) SHA1(6e94e223a65757fab79164988a1fdfab4215c284) )
	ROM_LOAD32_WORD("10117211.u73", 0x0000002, 0x0400000, CRC(a2fbf139) SHA1(53d6aa13bcaf0d537384ea406bb3a34fc8c8830c) )
ROM_END


// 0152539 / MV4238
// Road Trip / Local / C - 08/18/05
// Variations (% and NO):
//    87.926% 99
//    90.064% 01
//    92.165% 02
//    94.898% 03
ROM_START( roadtrip )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152539-U86 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-01   1F4PA3E0   E916-2888 / 1D21ACA4 / B38C68D8   Copyright © 2005 Aristocrat Technologies, Inc.
	0152539-U73 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-02   08BA756P   4590-81D7 / 1D0EF23D / B1F7C9D9   Copyright © 2005 Aristocrat Technologies, Inc.
	0152539-U85 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-03   8AAU6A72   5710-E6B2 / 1EBD2278 / 787BDEB2   Copyright © 2005 Aristocrat Technologies, Inc.
	0152539-U72 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-04   2514A96H   5ABE-D654 / 1E935047 / 773F8AF8   Copyright © 2005 Aristocrat Technologies, Inc.
	0152539-U84 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-05   54E651C7   7A2A-3E7E / 13C31DB8 / EA9E1AFF   Copyright © 2005 Aristocrat Technologies, Inc.
	0152539-U71 Series:01 M27V322 Mk6USA   Road Trip   L06-0453-06   F1C52929   29AB-EE54 / 134FA438 / D7478F87   Copyright © 2005 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152539.u86", 0x0000000, 0x0400000, CRC(66c2c8da) SHA1(76d338662a80ebdfd72e403388cffe8dfbd14f59) )
	ROM_LOAD32_WORD("0152539.u73", 0x0000002, 0x0400000, CRC(cb9f6f5d) SHA1(825322c6b0407a934aca632c198615d188e2f46b) )
	ROM_LOAD32_WORD("0152539.u85", 0x0800000, 0x0400000, CRC(e68205a7) SHA1(a7dc2a10de4ef229a8f25a3db19a30ea4be46cdd) )
	ROM_LOAD32_WORD("0152539.u72", 0x0800002, 0x0400000, CRC(421cfc77) SHA1(fb14ad4c06bdbdd6df4fc1299a96bb8d906d2b48) )
	ROM_LOAD32_WORD("0152539.u84", 0x1000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0152539.u71", 0x1000002, 0x0400000, CRC(1da57ab8) SHA1(775d1f0d6021032e909c8089ba93cc63eef18586) )
ROM_END


// 10115911 / 642/5
// Roll Up! Roll Up!! / Local / A - 22/08/02
// Variations (% and NO):
//    87.97% 99 (9 line)    87.92% 99 (20 line)
//    90.19% 01 (9 line)    90.17% 01 (20 line)
//    92.18% 02 (9 line)    92.19% 02 (20 line)
ROM_START( ruprup )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10115911.u86", 0x0000000, 0x0400000, CRC(b6d1c4fa) SHA1(24d88882783353598e4f63edda4982f9e55b2595) )
	ROM_LOAD32_WORD("10115911.u73", 0x0000002, 0x0400000, CRC(e60d9c93) SHA1(7504f07f04be97058b50168c14e6ec41480e43ef) )
	ROM_LOAD32_WORD("10115911.u85", 0x0800000, 0x0400000, CRC(7debb885) SHA1(c46e3231d8a13cec7d398a8064ac76e78954dd82) )
	ROM_LOAD32_WORD("10115911.u72", 0x0800002, 0x0400000, CRC(7ba80d87) SHA1(c912ec8c65060060b459de0c5ed0b3ad018eff76) )
ROM_END


// 10125711 / 698
// Ruby Magic / Local / C - 12/11/02
// Variations (% and NO):
//    87.52% 99
//    90.02% 01
//    92.11% 02
ROM_START( rubymag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10125711.u86", 0x0000000, 0x0400000, CRC(717d1c53) SHA1(8f5204d4394c6e0b83f6f456c80e98e12fac1a66) )
	ROM_LOAD32_WORD("10125711.u73", 0x0000002, 0x0400000, CRC(6699d2c6) SHA1(1c96dba91a40993cf9c3bc9e8c591e971c04bedf) )
	ROM_LOAD32_WORD("10125711.u85", 0x0800000, 0x0400000, CRC(877fde17) SHA1(278b8cb71cafa0859073acc48339da9e645fdcc6) )
	ROM_LOAD32_WORD("10125711.u72", 0x0800002, 0x0400000, CRC(2cdfe857) SHA1(33b4ce8ab0a5f1e341a5c5c536c5f3619ecd7f95) )
ROM_END


// 10148811 / 698/1
// Ruby Magic / Local / B - 14/07/03
// Variations (% and NO):
//    90.02% 99
ROM_START( rubymaga )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10148811.u86", 0x0000000, 0x0400000, CRC(a9d31936) SHA1(3738c1402fbd2363102f3f9e8e03880fc7d8cdb9) )
	ROM_LOAD32_WORD("10148811.u73", 0x0000002, 0x0400000, CRC(13fb122c) SHA1(07edf53863596258a1126eca559cc801e00a606b) )
	ROM_LOAD32_WORD("10148811.u85", 0x0800000, 0x0400000, CRC(723a80f1) SHA1(3323089eba0f289ea0509e534bd1c378f39a58e1) )
	ROM_LOAD32_WORD("10148811.u72", 0x0800002, 0x0400000, CRC(ac89dcc5) SHA1(fa14b15df7bf72176fa122274ec77dd83ac3204f) )
ROM_END


// 20288611 / 949
// Rumble Reels / B - 04/08/10
// Variations (% and NO):
//    87.99% 99
//    90.19% 01
//    88.74% 07
ROM_START( rumbreel )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20288611.u11", 0x0000000, 0x0400000, CRC(cc48bb54) SHA1(6ed7a502deaff0e2fd6d0901d036fcbadf303859) )
	ROM_LOAD32_WORD("20288611.u21", 0x0000002, 0x0400000, CRC(06b37c77) SHA1(e4dad62667a1c1d4948abcd535332975148e952b) )
	ROM_LOAD32_WORD("20288611.u12", 0x0800000, 0x0400000, CRC(3af99e7b) SHA1(93e68056fee9b4f29a066bd6828ff320b7d3ffa4) )
	ROM_LOAD32_WORD("20288611.u22", 0x0800002, 0x0400000, CRC(739c4565) SHA1(bc2cd133dfbfe18916703eacc851f1757a06f345) )
	ROM_LOAD32_WORD("20288611.u13", 0x1000000, 0x0400000, CRC(04b2f09d) SHA1(4090876cf8ef18ed8541f13684b076d7ac1d0d72) )
	ROM_LOAD32_WORD("20288611.u23", 0x1000002, 0x0400000, CRC(2f561bd0) SHA1(9c2f659ec5cdfd335d089389616dac57fbb655df) )
	ROM_LOAD32_WORD("20288611.u14", 0x1800000, 0x0400000, CRC(a117e3ec) SHA1(119d7ac89ee4c2167b853d720eb87498cefab4b9) )
	ROM_LOAD32_WORD("20288611.u24", 0x1800002, 0x0400000, CRC(6bf0442e) SHA1(c83ed68cd0680ebac359824defa51c1942db17f1) )
	ROM_LOAD32_WORD("20288611.u15", 0x2000000, 0x0400000, CRC(2c5baf9f) SHA1(40fb5b22460b9a35ca8ebb8b0643e58665d8643f) )
	ROM_LOAD32_WORD("20288611.u25", 0x2000002, 0x0400000, CRC(3cd5171b) SHA1(fe6e17679f14ae0238c58dd5d801cb0dda3a434b) )
ROM_END


// 10001311 / 660
// Scatter Magic 2 / Local / C- 24/08/00
// 'Scatter Magic II' on artwork, 'Scatter Magic 2' in ROM
// Variations (% and NO):
//    87.85% 99 (9 line)    87.80% 99 (20 line)
//    90.11% 01 (9 line)    89.98% 01 (20 line)
//    92.03% 02 (9 line)    92.03% 02 (20 line)
ROM_START( scatmag2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001311.u86", 0x0000000, 0x0400000, CRC(696734d8) SHA1(baffadb943fb12d0d19069023f5cdf80b1a1674f) )
	ROM_LOAD32_WORD("10001311.u73", 0x0000002, 0x0400000, CRC(aeef47b6) SHA1(63d296d60848269851e9d424908bbf3d52003f07) )
	ROM_LOAD32_WORD("10001311.u85", 0x0800000, 0x0400000, CRC(d3f7018e) SHA1(345a9f55256d9ec46182ebf9c0c6fca62ea2e275) )
	ROM_LOAD32_WORD("10001311.u72", 0x0800002, 0x0400000, CRC(e5c9dee7) SHA1(b29f7b8eb3d894e0a3dbc6cabdd2d8e59e2c2bbf) )
ROM_END


// 10122511 / 660
// Scatter Magic 2 / ASP Game / G- 15/05/01
// 'Scatter Magic II' on artwork, 'Scatter Magic 2' in ROM
// Variations (% and NO):
//    87.85% 99 (9 line)    87.80% 99 (20 line)
ROM_START( scatmag2a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10122511.u86", 0x0000000, 0x0400000, CRC(4238094b) SHA1(72cfe7d63c2463fd0d5ad08c2df7ee6a7efdc33f) )
	ROM_LOAD32_WORD("10122511.u73", 0x0000002, 0x0400000, CRC(76f49e2a) SHA1(e25126d99169204d4a3c344a2af2652cb69e25ba) )
	ROM_LOAD32_WORD("10122511.u85", 0x0800000, 0x0400000, CRC(f3ba7977) SHA1(3f97345f84137b6a029e67eb2b5659cf6bb13925) )
	ROM_LOAD32_WORD("10122511.u72", 0x0800002, 0x0400000, CRC(691170ab) SHA1(cf6059a5e00269c0c4eebc3d3a05c3ed3bd8c76e) )
ROM_END


// 10102711 / 660
// Scatter Magic 2 / Local / G - 15/05/01
// 'Scatter Magic II' on artwork, 'Scatter Magic 2' in ROM
// Same date string as scatmag2a
// Variations (% and NO):
//    87.85% 99 (9 line)    87.80% 99 (20 line)
ROM_START( scatmag2sa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102711.u86", 0x0000000, 0x0400000, CRC(4ae076d3) SHA1(41350306d03c1211a68ae117521c2a2b9c28d9ef) )
	ROM_LOAD32_WORD("10102711.u73", 0x0000002, 0x0400000, CRC(d04eaa9a) SHA1(1a70612f5ca7fd0d9384f7986775848e3194b718) )
	ROM_LOAD32_WORD("10102711.u85", 0x0800000, 0x0400000, CRC(e7e6bf08) SHA1(657077104ec632affbeaaafe5bc9569deb955668) )
	ROM_LOAD32_WORD("10102711.u72", 0x0800002, 0x0400000, CRC(8ea86d88) SHA1(74ac4df6ec5131d3745de1fdc495fb4c9dc0af33) )
ROM_END


// 0452569 / MV4277
// Scatter Magic III / Local / N/A
// Variations (% and NO):
//    87.863% 99
//    90.074% 01
//    91.898% 02
//    94.815% 03
//    97.291% 04
ROM_START( scatmag3 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0452569-U86 Series:01 M27V322 Mk6USA   Scatter Magic III   L06-2129-01   B93C76A4   FA1C-289C / 1D2606EC / A1701204   Copyright © 2001-2006 Aristocrat Technologies, Inc.
	0452569-U73 Series:01 M27V322 Mk6USA   Scatter Magic III   L06-2129-02   A6777858   47DD-1F63 / 1CCD98D6 / 8F3D0A74   Copyright © 2001-2006 Aristocrat Technologies, Inc.
	0452569-U85 Series:01 M27V322 Mk6USA   Scatter Magic III   L06-2129-03   30083C7U   D55A-1D84 / 00CC6748 / 60776143   Copyright © 2001-2006 Aristocrat Technologies, Inc.
	0452569-U72 Series:01 M27V322 Mk6USA   Scatter Magic III   L06-2129-04   1C53C0E0   36F0-D4DC / 00CCDFBD / 614E2793   Copyright © 2001-2006 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0452569.u86", 0x0000000, 0x0400000, CRC(8d1d87cd) SHA1(24267218813facee476b0d7b7b3769eabe193b62) )
	ROM_LOAD32_WORD("0452569.u73", 0x0000002, 0x0400000, CRC(9b645cfd) SHA1(9309a3389021776ccc6d2d070a4a954dabcd0979) )
	ROM_LOAD32_WORD("0452569.u85", 0x0800000, 0x0400000, CRC(899f7a05) SHA1(ab24922ce670d151a45af22376042b2a9106f7d1) )
	ROM_LOAD32_WORD("0452569.u72", 0x0800002, 0x0400000, CRC(a71a9565) SHA1(9761ec846b2183c4985eeb224054544f0cf9b3ec) )
ROM_END


// 0251029 / US024
// Seal The Deal / USA / A - 06/20/02
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    87.508% 99
//    90.363% 01
//    92.570% 02
//    94.623% 03
//    97.887% 04
//    82.888% 05 (Hyperlink)
ROM_START( sealdeal )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251029-U86 Series:01 M27V322 Mk6USA   Seal the Deal - Jackpot Carnival   L02-2211-01   5911990C   8528-DD4E / 1E5BB152 / 3B7E3381   Copyright © 2002 Aristocrat Technologies, Inc.
	0251029-U73 Series:01 M27V322 Mk6USA   Seal the Deal - Jackpot Carnival   L02-2211-02   DC65137F   CF82-4C1D / 1DDB76A6 / 16AF75CB   Copyright © 2002 Aristocrat Technologies, Inc.
	0251029-U85 Series:01 M27V322 Mk6USA   Seal the Deal - Jackpot Carnival   L02-2211-03   8348E3B7   B992-AFE2 / 106629A8 / 69F97666   Copyright © 2002 Aristocrat Technologies, Inc.
	0251029-U72 Series:01 M27V322 Mk6USA   Seal the Deal - Jackpot Carnival   L02-2211-04   B823824U   B5C2-BA0F / 1064D2CD / 68FEF8AC   Copyright © 2002 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0251029.u86", 0x0000000, 0x0400000, CRC(ea6b666b) SHA1(ad80042770f8e66e7455010b46d81873689f63db) )
	ROM_LOAD32_WORD("0251029.u73", 0x0000002, 0x0400000, CRC(383d55a9) SHA1(81bef0ed325ef7208403afa6d7af458c755a4de3) )
	ROM_LOAD32_WORD("0251029.u85", 0x0800000, 0x0400000, CRC(a6327726) SHA1(2c0f02a8d361bcbfc007b9d75a4b7b685aa2b1ac) )
	ROM_LOAD32_WORD("0251029.u72", 0x0800002, 0x0400000, CRC(43ef1218) SHA1(e1d03cd9845ac81a71b5a0471d146addc2f56e0e) )
ROM_END


// 10225911 / 818
// Shaman's Magic / Local / C - 14/11/06
// Variations (% and NO):
//    87.81% 99
//    90.03% 01
ROM_START( shamnmg )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10225911.u86", 0x0000000, 0x0400000, CRC(3edb8156) SHA1(83bb1467d892120c2bd5e85ef05527e2d02999ae) )
	ROM_LOAD32_WORD("10225911.u73", 0x0000002, 0x0400000, CRC(a84ac2a4) SHA1(743d56448135b03c25325bdc3d79781e63f0b9b6) )
ROM_END


// 0152647 / MV4318/1
// Shaman's Magic / Local / N/A
// Variations (% and NO):
//    87.090% 99
//    89.887% 01
//    91.336% 02
//    93.814% 03
ROM_START( shamnmgu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152647.u86", 0x0000000, 0x0400000, CRC(9c29e10b) SHA1(9f77808162d038461f77075f0565c0a0d34a708a) )
	ROM_LOAD32_WORD("0152647.u73", 0x0000002, 0x0400000, CRC(f1e9d86d) SHA1(282961a897bbfec4ab778db207f3ef20c6f4c69d) )
	ROM_LOAD32_WORD("0152647.u85", 0x0800000, 0x0400000, CRC(15bb6dd5) SHA1(96625e18b622d2a02f9a88d51ad90d5dac0289be) )
	ROM_LOAD32_WORD("0152647.u72", 0x0800002, 0x0400000, CRC(aab1dd80) SHA1(0e8fbc967c4d5c7d24f980afc78464052ca6d2a6) )
ROM_END


// 0251088 / US087
// Shore Thing / Local / B - 1/30/04
// Variations (% and NO):
//    86.419% 99 (9 line)    86.475% 99 (20 line)
//    89.362% 01 (9 line)    89.200% 01 (20 line)
//    91.497% 02 (9 line)    91.358% 02 (20 line)
//    94.317% 03 (9 line)    94.091% 03 (20 line)
//    95.311% 04 (9 line)    95.921% 04 (20 line)
ROM_START( shorthng )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251088-U86 Series:01 M27V322 Mk6USA   Shore Thing   0HC5F86U   3A3A-E163 / 1E05CAD8 / 1BEF812B   Copyright © 2004 Aristocrat Technologies, Inc.
	0251088-U73 Series:01 M27V322 Mk6USA   Shore Thing   6A5P332F   0627-40DA / 1DAD5A97 / 02D32FA8   Copyright © 2004 Aristocrat Technologies, Inc.
	0251088-U85 Series:01 M27V322 Mk6USA   Shore Thing   4956FUBH   E293-A34F / 09143E44 / 8AA285E8   Copyright © 2004 Aristocrat Technologies, Inc.
	0251088-U72 Series:01 M27V322 Mk6USA   Shore Thing   C53PF655   DE45-0434 / 090B92D7 / 86643B54   Copyright © 2004 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0251088.u86", 0x0000000, 0x0400000, CRC(c3e6f083) SHA1(4ed6b1039f060cd6480a490de28619f5370de805) )
	ROM_LOAD32_WORD("0251088.u73", 0x0000002, 0x0400000, CRC(c214d960) SHA1(e5b75e5cb3af3b77f56ccec72833e9243d09c643) )
	ROM_LOAD32_WORD("0251088.u85", 0x0800000, 0x0400000, CRC(e721a879) SHA1(0632c1be6c3b671aed4f6dd67b83e966033aba19) )
	ROM_LOAD32_WORD("0251088.u72", 0x0800002, 0x0400000, CRC(e151390f) SHA1(e68f2584587687c96e4cbe0080630466fac61b17) )
ROM_END


// 10001911 / 657/1
// Show Me The Money / Local / B - 08/01/01
// Variations (% and NO):
//    87.88% 99
//    90.01% 01
//    92.45% 02
ROM_START( smtma6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001911.u86", 0x0000000, 0x0400000, CRC(9f436ee3) SHA1(e41a9475fd10fd7d15be1665ee2d4e9dcee2d4dd) )
	ROM_LOAD32_WORD("10001911.u73", 0x0000002, 0x0400000, CRC(ccc901f0) SHA1(4c845ccfea10a744b8c37b6b155b0fc2ce6bfede) )
	ROM_LOAD32_WORD("10001911.u85", 0x0800000, 0x0400000, CRC(7a36f3f3) SHA1(3653a6f40587c2be1810b4a9dd8f27ecca336a70) )
	ROM_LOAD32_WORD("10001911.u72", 0x0800002, 0x0400000, CRC(cb5f47d4) SHA1(70fa1e68bf1fbebc6b6663fbabb78c5f286b7805) )
ROM_END


// 10015711 / 657/1
// Show Me the Money / Qld Clubs and Hotels / D - 31/05/01
// Variations (% and NO):
//    87.88% 99
ROM_START( smtma6q )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10015711.u86", 0x0000000, 0x0400000, CRC(f15a79e5) SHA1(82ef7f28460623bbafa704c9e26e914d4be1cb7f) )
	ROM_LOAD32_WORD("10015711.u73", 0x0000002, 0x0400000, CRC(906cbb69) SHA1(21775cc58425e828d6d77e11d07167acb6867f82) )
	ROM_LOAD32_WORD("10015711.u85", 0x0800000, 0x0400000, CRC(722ea31f) SHA1(6ace379272ca207449a4b03df480929eb1636593) )
	ROM_LOAD32_WORD("10015711.u72", 0x0800002, 0x0400000, CRC(b208c54e) SHA1(ad833271972f58bb866ca63f68d8f5dd67b275b2) )
ROM_END


// 10176811 / JB021
// Silk Road / ASP / J - 1/06/04
// Variations (% and NO):
//    87.86% 99
ROM_START( silkrda6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10176811.u86", 0x0000000, 0x0400000, CRC(ff264c60) SHA1(2ab13845d359617f12010ba37033918d761f9230) )
	ROM_LOAD32_WORD("10176811.u73", 0x0000002, 0x0400000, CRC(34b27f3e) SHA1(ec9e694363fdd1f68e3d171385877a1e8914b3b7) )
	ROM_LOAD32_WORD("10176811.u85", 0x0800000, 0x0400000, CRC(98f66ca6) SHA1(b7562eaf1d28d7886d43f9e9a37a5c27d4485b60) )
	ROM_LOAD32_WORD("10176811.u72", 0x0800002, 0x0400000, CRC(6cda859c) SHA1(471312fd851aef881506730d38135eae80acb3b7) )
ROM_END


// 0152537 / MV4248/4
// Silk Road / Local / D - 02/09/06
// EPROM labels marked 2004 despite being a 2006 game
// Variations (% and NO):
//    87.040% 99 (Reel 1)    87.040% 99 (Reels 1-2)    87.363% 99 (Reels 1-3)    87.876% 99 (Reels 1-4)    87.881% 99 (Reels 1-5)
//    90.063% 01 (Reel 1)    90.063% 01 (Reels 1-2)    90.155% 01 (Reels 1-3)    90.804% 01 (Reels 1-4)    90.872% 01 (Reels 1-5)
//    92.123% 02 (Reel 1)    92.123% 02 (Reels 1-2)    92.165% 02 (Reels 1-3)    92.942% 02 (Reels 1-4)    93.047% 02 (Reels 1-5)
//    95.689% 03 (Reel 1)    95.689% 03 (Reels 1-2)    95.719% 03 (Reels 1-3)    95.965% 03 (Reels 1-4)    95.978% 03 (Reels 1-5)
//    96.759% 04 (Reel 1)    96.759% 04 (Reels 1-2)    96.851% 04 (Reels 1-3)    97.482% 04 (Reels 1-4)    97.616% 04 (Reels 1-5)
ROM_START( silkrda6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251029-U86 Series:01 M27V322 Mk6USA   Silk Road   L06-0853-01   C1811FC5   E2E2-5479 / 1DAB930F / E0D24960   Copyright © 2004 Aristocrat Technologies, Inc.
	0251029-U73 Series:01 M27V322 Mk6USA   Silk Road   L06-0853-02   403AEFAH   1F8A-550F / 1D85621F / D421C25D   Copyright © 2004 Aristocrat Technologies, Inc.
	0251029-U85 Series:01 M27V322 Mk6USA   Silk Road   L06-0853-03   C878F74C   F506-B584 / 11CBF976 / ED87CA05   Copyright © 2004 Aristocrat Technologies, Inc.
	0251029-U72 Series:01 M27V322 Mk6USA   Silk Road   L06-0853-04   66F27363   CA83-64A6 / 1184E2F7 / CF37164B   Copyright © 2004 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152537.u86", 0x0000000, 0x0400000, CRC(5e8f8fc3) SHA1(bb586743182a3797d601db740e270e773f7e3587) )
	ROM_LOAD32_WORD("0152537.u73", 0x0000002, 0x0400000, CRC(c32608e3) SHA1(83d9749ce77317ce5631e7c7d3985c71f26d43a5) )
	ROM_LOAD32_WORD("0152537.u85", 0x0800000, 0x0400000, CRC(568f7ecd) SHA1(ccab6d872fe5bc00466071334b1af9f6799e40a0) )
	ROM_LOAD32_WORD("0152537.u72", 0x0800002, 0x0400000, CRC(1ed58be6) SHA1(114c5a800592163dbc690b5d4c4c051d358828f9) )
ROM_END


// 20100721 / 672/1
// SILVER & GOLD / Local / G - 08/07/02
// Variations (% and NO):
//    87.91% 99
//    89.91% 01
//    92.10% 02
ROM_START( silgola6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20100721.u86", 0x0000000, 0x0400000, CRC(d91108fd) SHA1(cea026350f083ed0f061d5049de520d08730a1df) )
	ROM_LOAD32_WORD("20100721.u73", 0x0000002, 0x0400000, CRC(cffa82f0) SHA1(fe14e1d9bacbcb72ad1bdaf3d4a72eec67311bef) )
	ROM_LOAD32_WORD("20100721.u85", 0x0800000, 0x0400000, CRC(f66f63c4) SHA1(02e669c4f7c3d0cd39b3dff156ab4b05db3bb495) )
	ROM_LOAD32_WORD("20100721.u72", 0x0800002, 0x0400000, CRC(05861fb2) SHA1(ba3d2e3ab66fd7178289fdc95661f27e08124f32) )
ROM_END


// 10272711 / JB091/2
// Sky Dancer / A - 28/08/08
// Variations (% and NO):
//    87.80% 99
//    90.09% 01
//    92.05% 02
ROM_START( skydnca6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10272711.u86", 0x0000000, 0x0400000, CRC(81c0b79d) SHA1(ec84ded928637f8888d0caa935ab8d64c647a536) )
	ROM_LOAD32_WORD("10272711.u73", 0x0000002, 0x0400000, CRC(f90fc640) SHA1(07b523839ac36b07d3dbc4d24e6d987c1f644095) )
	ROM_LOAD32_WORD("10272711.u85", 0x0800000, 0x0400000, CRC(4a9a7c33) SHA1(e3243fa32b6736140992ad9fcf04a420bd417846) )
	ROM_LOAD32_WORD("10272711.u72", 0x0800002, 0x0400000, CRC(12d02b67) SHA1(aed879daca62575def363872650d33b7af7d71bb) )
	ROM_LOAD32_WORD("10272711.u84", 0x1000000, 0x0400000, CRC(df6885af) SHA1(e3302defbc496c1b71da6be8ed49dd4d3556d7d4) )
	ROM_LOAD32_WORD("10272711.u71", 0x1000002, 0x0400000, CRC(d30c3d6f) SHA1(581a198d98f00c1b7a432ab85b5f65760e364131) )
ROM_END


// 20115211
// Snap Shot / ASP
ROM_START( snapshot )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20115211.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20115211.u73", 0x0000002, 0x0400000, CRC(f9190188) SHA1(4a8ac12700e4edeeaafd22a5574e4fad8394bb01) )
	ROM_LOAD32_WORD("20115211.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20115211.u72", 0x0800002, 0x0400000, NO_DUMP )
ROM_END


// 10000311 / 642
// Spring Carnival / Local / C - 15/11/99
// Variations (% and NO):
//    87.93% 99 (9 line)    87.96% 99 (20 line)
//    90.19% 01 (9 line)    90.17% 01 (20 line)
//    92.14% 02 (9 line)    92.17% 02 (20 line)
ROM_START( spgcarn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000311.u86", 0x0000000, 0x0400000, CRC(f7783fea) SHA1(4e7b86c0d2728ebc317dce14827a4873402d9bbb) )
	ROM_LOAD32_WORD("10000311.u73", 0x0000002, 0x0400000, CRC(033653f5) SHA1(40705ffb653ad4c6f3cd36c8879c48303c3845f8) )
ROM_END


// 10032721 / 642
// Spring Carnival / QLD QCOM / F - 21/09/01
// Jackpot Carnival Hyperlink game
// Variations (% and NO):
//    82.19% 05 (Hyperlink) (9 line)    82.09% 05 (Hyperlink) (20 line)
ROM_START( spgcarnjc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	10032721   QCOM   SPRING CARNIVAL - J/P HYP   642   4MB   4 eproms   Aristocrat © 2003   P   U86
	10032721   QCOM   SPRING CARNIVAL - J/P HYP   642   4MB   4 eproms   Aristocrat © 2003   P   U73
	10032721   QCOM   SPRING CARNIVAL - J/P HYP   642   4MB   4 eproms   Aristocrat © 2003   P   U85
	10032721   QCOM   SPRING CARNIVAL - J/P HYP   642   4MB   4 eproms   Aristocrat © 2003   P   U72
	*/
	ROM_LOAD32_WORD("10032721.u86", 0x0000000, 0x0400000, CRC(5027111e) SHA1(aca962df77ea19a386cc7a88397d7629d53e2e20) )
	ROM_LOAD32_WORD("10032721.u73", 0x0000002, 0x0400000, CRC(e1b78c98) SHA1(78dfd4aa21169077d796e84c69b2f769e614a03c) )
	ROM_LOAD32_WORD("10032721.u85", 0x0800000, 0x0400000, CRC(671e70e1) SHA1(42879e248895d819af8f7968933ee09122cb694c) )
	ROM_LOAD32_WORD("10032721.u72", 0x0800002, 0x0400000, CRC(5689fe18) SHA1(5801ca7bb9a896bdaa42834055967638c5ae62ea) )
ROM_END


// 10008011 / 642
// Spring Carnival / QLD QCOM / E - 27/02/01
// Variations (% and NO):
//    87.93% 99 (9 line)    87.96% 99 (20 line)
ROM_START( spgcarnq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10008011.u86", 0x0000000, 0x0400000, CRC(ad0d4455) SHA1(07ae7dd9dfce742bb77f7ecbb28072005d21be57) )
	ROM_LOAD32_WORD("10008011.u73", 0x0000002, 0x0400000, CRC(43c3600d) SHA1(ddd5dab5555e7586da4ea5ff8109c6b6b0e6ac16) )
	ROM_LOAD32_WORD("10008011.u85", 0x0800000, 0x0400000, CRC(e2a2205b) SHA1(fade79da946ff8a410b4eb05a987616e5266e00f) )
	ROM_LOAD32_WORD("10008011.u72", 0x0800002, 0x0400000, CRC(1351bf20) SHA1(0c608ae7db037691e4d8ea3b8623f7f53ba011d9) )
ROM_END


// 20262311 / JB040
// Spring Festival / Local / C - 03/06/08
// Variations (% and NO):
//    87.88% 99
ROM_START( spgfest )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20262311.u86", 0x0000000, 0x0400000, CRC(fba59959) SHA1(28fd5f54a627ba097d1841fff17b3be5b4e08619) )
	ROM_LOAD32_WORD("20262311.u73", 0x0000002, 0x0400000, CRC(ea2b0902) SHA1(d4ff35750c473b7816e9bd8db3ab88f235a96041) )
	ROM_LOAD32_WORD("20262311.u85", 0x0800000, 0x0400000, CRC(692260eb) SHA1(410b038ab2254d207608033986f9ce90866f0c71) )
	ROM_LOAD32_WORD("20262311.u72", 0x0800002, 0x0400000, CRC(1d645199) SHA1(31f51022c7533b52b7933fc3baec374b1df4f35c) )
	ROM_LOAD32_WORD("20262311.u84", 0x1000000, 0x0400000, CRC(b7c8700e) SHA1(72123d279931b758d08b5760c5f4c38e77611790) )
	ROM_LOAD32_WORD("20262311.u71", 0x1000002, 0x0400000, CRC(3c2baee2) SHA1(fc03ca7e96d0e877a4f8a8f267f874c374e018d8) )
ROM_END


// 20267211 / JB040/2
// Spring Festival / NZ - QCOM / D - 24/10/08
// Variations (% and NO):
//    88.99% 99
ROM_START( spgfestnz )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U86
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U73
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U85
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U72
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U84
	20267211   NZCB   SPRING FESTIVAL   JB040/2   4MB   6 eproms   Aristocrat © 2008   P   U71
	*/
	ROM_LOAD32_WORD("20267211.u86", 0x0000000, 0x0400000, CRC(67e3ed7e) SHA1(9c79b029bb39d53b5e11ea7b997a33b9a3957c2a) )
	ROM_LOAD32_WORD("20267211.u73", 0x0000002, 0x0400000, CRC(99275d9c) SHA1(d5f167ed224d933b77631f3952a2c16622d3f3b1) )
	ROM_LOAD32_WORD("20267211.u85", 0x0800000, 0x0400000, CRC(b9b502ef) SHA1(207bcfe7d52d4785c0d7a6028b940f36a68aff9a) )
	ROM_LOAD32_WORD("20267211.u72", 0x0800002, 0x0400000, CRC(a2a015dc) SHA1(84a7db7a79b1cc54782b6ab813d6ad46abc0f182) )
	ROM_LOAD32_WORD("20267211.u84", 0x1000000, 0x0400000, CRC(b1326bd8) SHA1(a997b1b0ae477a9883231ed439e9d4e83724ba32) )
	ROM_LOAD32_WORD("20267211.u71", 0x1000002, 0x0400000, CRC(44c603d8) SHA1(f1fedbb8931095b563a3bcd17adfc6d839b62baf) )
ROM_END


// 20150811 / 717/2
// Star Drifter / Local / A - 15/09/03
// Variations (% and NO):
//    87.58% 99
//    90.03% 01
//    92.11% 02
ROM_START( stardrft )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20150811.u86", 0x0000000, 0x0400000, CRC(7b08a2be) SHA1(2b3d7bb2317cba7b470d68c91918a5777fdb3ed1) )
	ROM_LOAD32_WORD("20150811.u73", 0x0000002, 0x0400000, CRC(580340b4) SHA1(4febb22150581a4661bc7ed1ff0662e153de5e2b) )
ROM_END


// 0451341 / MV4204/1
// Star Drifter / Local / A - 02/16/04
// Variations (% and NO):
//    87.581% 99
//    90.033% 01
//    92.110% 02
//    94.711% 03
//    97.031% 04
ROM_START( stardrftu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451341.u86", 0x0000000, 0x0400000, CRC(73023e64) SHA1(4d2d586a1e91ac5bac379994973dffcf9f3a3293) )
	ROM_LOAD32_WORD("0451341.u73", 0x0000002, 0x0400000, CRC(3e59b84f) SHA1(3148d48df74c35681759a234d7aa746cbb0afb0e) )
	ROM_LOAD32_WORD("0451341.u85", 0x0800000, 0x0400000, CRC(689ded33) SHA1(5d5b744321313c4618ecf319827c778be646245f) )
	ROM_LOAD32_WORD("0451341.u72", 0x0800002, 0x0400000, CRC(dc7a30c1) SHA1(8f24c18efd6171cda7ca841d3b275937e35e2be8) )
ROM_END


// 10009511 / 659/2
// Star Quest / Local / B - 29/10/01
// Variations (% and NO):
//    90.08% 99
//    87.72% 01
//    92.07% 02
//    94.81% 03
//    94.05% 04
ROM_START( starqst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10009511.u86", 0x0000000, 0x0400000, CRC(c21f4486) SHA1(5a6a532d73bf56f68f473bd56a7c3d9f0660d661) )
	ROM_LOAD32_WORD("10009511.u73", 0x0000002, 0x0400000, CRC(facf51b6) SHA1(73c13d507b65cd98ae43918c21b492a8499dd538) )
ROM_END


// 0251147 / US146
// Status Symbols / Local / A - 02/10/05
// Variations (% and NO):
//    87.546% 99
//    90.017% 01
//    92.375% 02
//    94.984% 03
//    97.061% 04
ROM_START( statsymb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251147.u86", 0x0000000, 0x0400000, CRC(fe9b9147) SHA1(5dced51e126a80191d62b88438e1098efb358e52) )
	ROM_LOAD32_WORD("0251147.u73", 0x0000002, 0x0400000, CRC(e06f2dd1) SHA1(0edadf037dee101e5fb768471d2f93474ff8268f) )
ROM_END


// 0251303 / MV4173
// Sun & Moon / Local / C - 04/01/03
// Variations (% and NO):
//    87.974% 99
//    89.962% 01
//    92.071% 02
//    94.981% 03
//    94.407% 04
ROM_START( sunmoon )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0251303-U86 Series: 01 M27V322 Mk6USA   Sun and Moon   L05-2845-01   A6BH94BP   3557-E982 / 1BDB0709 / 01BA99A3   Copyright © 2003 Aristocrat Technologies, Inc.
	0251303-U73 Series: 01 M27V322 Mk6USA   Sun and Moon   L05-2845-02   1U868288   9D17-ABCC / 1BB6E9F2 / F2A1C943   Copyright © 2003 Aristocrat Technologies, Inc.
	0251303-U85 Series: 01 M27V322 Mk6USA   Sun and Moon   L05-2845-03   3031DCDA   78E7-D857 / 0CD0B0B8 / 66025D7A   Copyright © 2003 Aristocrat Technologies, Inc.
	0251303-U72 Series: 01 M27V322 Mk6USA   Sun and Moon   L05-2845-04   53234640   F609-F72C / 0C8FCE79 / 55AB01DB   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0251303.u86", 0x0000000, 0x0400000, CRC(f58f4c7b) SHA1(c2e05432f7bc02a5db91a2d83b7c142ac93c1e25) )
	ROM_LOAD32_WORD("0251303.u73", 0x0000002, 0x0400000, CRC(66aa0611) SHA1(f34c28771244f21f38cadac02374243ab26065a0) )
	ROM_LOAD32_WORD("0251303.u85", 0x0800000, 0x0400000, CRC(a1032dbc) SHA1(25e560af6711259a3792285bd9279c22c21d602d) )
	ROM_LOAD32_WORD("0251303.u72", 0x0800002, 0x0400000, CRC(98602b09) SHA1(8f0048b05f069efbb96abf928dc56a6d3124be22) )
ROM_END


// 0151303 / MV4173
// Sun & Moon / Local / C - 04/01/03
// Same date string as parent
// Variations (% and NO):
//    87.974% 99
//    89.962% 01
//    92.071% 02
//    94.981% 03
//    94.407% 04
ROM_START( sunmoona )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151303.u86", 0x0000000, 0x0400000, CRC(012e5aeb) SHA1(3df4454ab54cc6edf62b8cc9de4c867a36e8654b) )
	ROM_LOAD32_WORD("0151303.u73", 0x0000002, 0x0400000, CRC(ce1f8d32) SHA1(7891b0e797942374d779e8df92281383dd94fe90) )
	ROM_LOAD32_WORD("0151303.u85", 0x0800000, 0x0400000, CRC(7e16ab59) SHA1(f56b3b5a252d3520e36b1d2a7643d65020af2133) )
	ROM_LOAD32_WORD("0151303.u72", 0x0800002, 0x0400000, CRC(f69ce78d) SHA1(91a433757b377406bca61b4c96f3ba3561efd75c) )
ROM_END


// 0153028 / MV4173/2
// Sun & Moon (Bank Buster) / Local / N/A
// Variations (% and NO):
//    72.868% 99
//    73.862% 01
//    74.805% 02
//    72.868% 03 (same %, different variations)
//    73.862% 04
//    74.805% 05
ROM_START( sunmonbb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0153028.u11", 0x0000000, 0x0400000, CRC(b6a8df40) SHA1(316e3f98b75c1bdd0e8fc55aee2cf20b8a45ff4c) )
	ROM_LOAD32_WORD("0153028.u21", 0x0000002, 0x0400000, CRC(b0cc922d) SHA1(5a45d0c995c639739176a5aad42e186b122eead1) )
	ROM_LOAD32_WORD("0153028.u12", 0x0800000, 0x0400000, CRC(98cde50e) SHA1(68e60411a8c10b73482a4cc8d3f14ebdedca0ae9) )
	ROM_LOAD32_WORD("0153028.u22", 0x0800002, 0x0400000, CRC(542ec911) SHA1(48988bdefb9ad6a7e0c733eda1f220b0347cab5f) )
	ROM_LOAD32_WORD("0153028.u13", 0x1000000, 0x0400000, CRC(ac9143cc) SHA1(75aeea2ad26044bef9eb31495d67414a416ee6d4) )
	ROM_LOAD32_WORD("0153028.u23", 0x1000002, 0x0400000, BAD_DUMP CRC(0ac455b1) SHA1(3ca4f4a0172f18692c9f82a9a435ffc1ca39ddc2) )
	ROM_LOAD32_WORD("0153028.u14", 0x1800000, 0x0400000, CRC(a5552dd3) SHA1(1bbe24950e19441ca5144cb9b38320803bd5d26e) )
	ROM_LOAD32_WORD("0153028.u24", 0x1800002, 0x0400000, CRC(19e5b7e0) SHA1(cf17a84ba08a2f24725d4ccbe046710c2f3a2459) )
ROM_END


// 0451327 / MV2070/1
// Sun Queen / Local / B - 11/24/03
// Variations (% and NO):
//    89.993% 99
//    89.995% 01
//    91.973% 02
//    91.995% 03
//    94.973% 04
//    94.995% 05
//    96.723% 06
//    96.995% 07
ROM_START( sunqndol )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451327.u86", 0x0000000, 0x0400000, CRC(78fd2a9f) SHA1(768db303de2ce2fb1efe6adee44d66780801e7da) )
	ROM_LOAD32_WORD("0451327.u73", 0x0000002, 0x0400000, CRC(b46080a1) SHA1(11293e7636dea28c8dfdebd107fa5aee013c832e) )
	ROM_LOAD32_WORD("0451327.u85", 0x0800000, 0x0400000, CRC(f522c85e) SHA1(2b9ba996b99745e69944451e1fd2b585ed62290f) )
	ROM_LOAD32_WORD("0451327.u72", 0x0800002, 0x0400000, CRC(a2b404ac) SHA1(409bd1ce24600fa381ad4de8b7066e5e04f7ab3d) )
ROM_END


// 20203611 / 794
// Super Bucks V / Local / A - 03/08/05
// Variations (% and NO):
//    90.11% 99
//    87.94% 01
//    91.89% 02
//    94.86% 03
ROM_START( sbuk5 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20203611.u86", 0x0000000, 0x0400000, CRC(cab68268) SHA1(e10e8c8ce9bfb7ae7f3920303491b24c93d5f716) )
	ROM_LOAD32_WORD("20203611.u73", 0x0000002, 0x0400000, CRC(39af5809) SHA1(048aef299de220a72bf6a57c6268949197fc738b) )
	ROM_LOAD32_WORD("20203611.u85", 0x0800000, 0x0400000, CRC(9114f9e8) SHA1(f7ec058aaaad59aeffc01085673322cb842a5ebb) )
	ROM_LOAD32_WORD("20203611.u72", 0x0800002, 0x0400000, CRC(ebcce486) SHA1(f91db1051d71c5e558a304d60157fa3cfff66c5b) )
ROM_END


// 0151183 / US169
// Sweethearts II / Local / A - 7/5/05
// Variations (% and NO):
//    87.181% 99
//    90.151% 01
//    92.252% 02
//    94.972% 03
//    97.055% 04
ROM_START( swhr2a6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151183.u86", 0x0000000, 0x0400000, CRC(320d3052) SHA1(8b2165795e9b9d7514620b0d6a14371142de434b) )
	ROM_LOAD32_WORD("0151183.u73", 0x0000002, 0x0400000, CRC(17f0dbe2) SHA1(7f1f0936e1cf528ee794e8925c40b063d1a77729) )
ROM_END


// 10119811 / 712
// Sweethearts II (Lucky Devil) / Local / C - 16/10/02
// Variations (% and NO):
//    87.84% 99
//    89.81% 01
//    86.12% 02
ROM_START( swh2ld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119811.u86", 0x0000000, 0x0400000, CRC(dedd99b0) SHA1(23c4e178d00788516c16109add772dba24f98a54) )
	ROM_LOAD32_WORD("10119811.u73", 0x0000002, 0x0400000, CRC(f5c3d17f) SHA1(297e2da51fb00e0b76f9322149647c4c27ed870e) )
	ROM_LOAD32_WORD("10119811.u85", 0x0800000, 0x0400000, CRC(f0f8aa94) SHA1(c69f1c807b0e9921eb95c3351bb79897e673ec36) )
	ROM_LOAD32_WORD("10119811.u72", 0x0800002, 0x0400000, CRC(94d48b25) SHA1(a879f6d78ff6eb29e5ea2cc71ca86d097fa9d45f) )
ROM_END


// 10238911 / 765/2
// Tahiti Magic / Local / C - 07/12/06
// Variations (% and NO):
//    87.90% 99
//    90.18% 01
ROM_START( tahitim )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10238911.u86", 0x0000000, 0x0400000, CRC(735d7642) SHA1(5f4258bf541728f7c7fe3dd65e715d68b3ade968) )
	ROM_LOAD32_WORD("10238911.u73", 0x0000002, 0x0400000, CRC(b08da2ce) SHA1(1adb4f6e134df479be0f90d2459b009d93386ac1) )
ROM_END


// 10119321 / 707
// Thai Princess / Local / A - 17/09/02
// Variations (% and NO):
//    87.51% 99
//    89.80% 01
//    92.02% 02
ROM_START( thaiprnc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119321.u86", 0x0000000, 0x0400000, CRC(609b3112) SHA1(77648426c7fed2e39d48d4f1ced524067bf0bb1a) )
	ROM_LOAD32_WORD("10119321.u73", 0x0000002, 0x0400000, CRC(a8346fea) SHA1(d97af71bb2c93ed517fccac605e9cfff9ec5590b) )
	ROM_LOAD32_WORD("10119321.u85", 0x0800000, 0x0400000, CRC(92fda7b5) SHA1(9a50ab3d1d18814093f5457a07a52f7e19fc97e7) )
	ROM_LOAD32_WORD("10119321.u72", 0x0800002, 0x0400000, CRC(640bf878) SHA1(910dce13181243a6baace6dd1807d391b3d9767f) )
ROM_END


// 30127721 / 707
// Thai Princess / Crown / E - 21/03/03
// Region says Crown but game is universal ASP
// Variations (% and NO):
//    87.51% 99
//    89.80% 01
//    92.01% 02
//    88.23% 03
//    87.03% 04
ROM_START( thaiprnca )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30127721.u86", 0x0000000, 0x0400000, CRC(3cb5124b) SHA1(70f6d66793c433054557da4c9f2f033bbb640bd6) )
	ROM_LOAD32_WORD("30127721.u73", 0x0000002, 0x0400000, CRC(531f05ab) SHA1(929285de219c033bdee5f8011e9a0a07b344375e) )
	ROM_LOAD32_WORD("30127721.u85", 0x0800000, 0x0400000, CRC(90c345e0) SHA1(1cf5b237eca68749a7baa147b9b15b3e139d7951) )
	ROM_LOAD32_WORD("30127721.u72", 0x0800002, 0x0100000, BAD_DUMP CRC(613cea6b) SHA1(f04a3ee53074b7cd84879d752df5dbb80437475e) ) // wrong size!
ROM_END


// 10173811 / 758
// Thunder Heart / Local / B - 11/05/04
// 'Thunderheart' on artwork, 'Thunder Heart' in ROM
// Variations (% and NO):
//    90.18% 99
//    87.85% 01
//    92.07% 02
//    94.91% 03
ROM_START( thndrhrt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173811.u86", 0x0000000, 0x0400000, CRC(2b84ed7b) SHA1(9919f9526d8294eeabda40697a34878484f6cef5) )
	ROM_LOAD32_WORD("10173811.u73", 0x0000002, 0x0400000, CRC(d867ff79) SHA1(740a177a8d1ecf925583a9264b5404640a5f2289) )
ROM_END


// 20230111 / 807
// Tigress / Local / C - 03/04/07
// Variations (% and NO):
//    87.93% 99
//    90.05% 01
//    91.96% 02
ROM_START( tigress )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20230111.u86", 0x0000000, 0x0400000, CRC(d1ff02f7) SHA1(8e8f1c4ed3c01fc62011b1bd21f53314e345f10f) )
	ROM_LOAD32_WORD("20230111.u73", 0x0000002, 0x0400000, CRC(cc239965) SHA1(f5d5f1e3db4f0ae3cd1dbe2c88aedf65cc820d4e) )
ROM_END


// 20243811
// Tigress / ASP
ROM_START( tigressa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20243811.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20243811.u73", 0x0000002, 0x0400000, CRC(525c76a1) SHA1(a4d01cd01fe185259ff9f010129a8fcc91418542) )
ROM_END


// 10239111 / 831
// Tiki Talk / D - 28/03/07
// Variations (% and NO):
//    87.95% 99
//    90.04% 01
ROM_START( tikitalk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10239111.u86", 0x0000000, 0x0400000, CRC(772312e1) SHA1(5a5561d9ec70b56e558ad87ead0770ac9f18e9d1) )
	ROM_LOAD32_WORD("10239111.u73", 0x0000002, 0x0400000, CRC(983d8621) SHA1(92ad28c0a7a06035569700adb8c3449f92bb36b7) )
ROM_END


// 0151003 / MV4130/4
// Tiki Torch / Local / A - 08/10/01
// MV4130/5 (alternate string)
// Variations (% and NO):
//    87.598% 99 (9 line)    87.546% 99 (20 line)
//    90.059% 01 (9 line)    90.017% 01 (20 line)
//    92.087% 02 (9 line)    92.375% 02 (20 line)
//    94.895% 03 (9 line)    94.984% 03 (20 line)
//    97.119% 04 (9 line)    97.061% 04 (20 line)
ROM_START( tiktorch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151003-U86 Series: 01 M27V322 Mk6USA   Tiki Torch   L02-2236-01   6HDUD3AU   D9C6-3D4B / 1D6E9D79 / D86D1??2   Copyright © 2003 Aristocrat Technologies, Inc. // Last checksum unreadable due to damaged label
	0151003-U73 Series: 01 M27V322 Mk6USA   Tiki Torch   L02-2236-02   B65F9UD7   B01E-27A5 / 1D5A47F3 / D78983BF   Copyright © 2003 Aristocrat Technologies, Inc.
	0151003-U85 Series: 01 M27V322 Mk6USA   Tiki Torch   L02-2236-03   9CC5A864   940E-CDF9 / 12676C1D / 3C97A77E   Copyright © 2003 Aristocrat Technologies, Inc.
	0151003-U72 Series: 01 M27V322 Mk6USA   Tiki Torch   L02-2236-04   8A7U8PCC   83F3-D776 / 121BE449 / 213F41B0   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151003.u86", 0x0000000, 0x0400000, CRC(ad10a554) SHA1(59062060ded7304193105d2b03fb06a7754a9fe6) )
	ROM_LOAD32_WORD("0151003.u73", 0x0000002, 0x0400000, CRC(f0f8e8d5) SHA1(a084befd422ce37ad19b31d3da61e7a03d5e38c4) )
	ROM_LOAD32_WORD("0151003.u85", 0x0800000, 0x0400000, CRC(32eed9c2) SHA1(8e3e8d55a4dac92ae9b543c86921126df1fb6518) )
	ROM_LOAD32_WORD("0151003.u72", 0x0800002, 0x0400000, CRC(1161133b) SHA1(760c710167e4eca84421fc1cd54ab696e16502cc) )
ROM_END


// 10124011 / MV4164
// Tiki Torch / NZ Pub/Club / A - 06/09/02
// Variations (% and NO):
//    87.55% 99
ROM_START( tiktorchnz )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10124011.u86", 0x0000000, 0x0400000, CRC(3791d66c) SHA1(0cf62a57c0f18c694f83344bf8c99220e80b0a5f) )
	ROM_LOAD32_WORD("10124011.u73", 0x0000002, 0x0400000, BAD_DUMP CRC(4daf6865) SHA1(11ce07ba18fa0d6b5075a613cfdfba68fa3c350b) )
	ROM_LOAD32_WORD("10124011.u85", 0x0800000, 0x0400000, CRC(920eecff) SHA1(577a7732bccde7c8f88d37d88065c230326f504b) )
	ROM_LOAD32_WORD("10124011.u72", 0x0800002, 0x0400000, CRC(8485163b) SHA1(74466f1734b5a6cec66ed2441a11ef5531c5a8fd) )
ROM_END


// 0154013 / MV4333/1
// Timber Wolf / N/A
// Variations (% and NO):
//    87.432% 99
//    90.142% 01
//    91.108% 02
//    94.427% 03
ROM_START( timbwolf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0154013.u86", 0x0000000, 0x0400000, CRC(a3be6e57) SHA1(9e06a85b630626bf7e3e790a70b8575bc9047b3d) )
	ROM_LOAD32_WORD("0154013.u73", 0x0000002, 0x0400000, CRC(054e3956) SHA1(ccfb6ba102c22f70fbc61b3d6ba0ceadc1414e1c) )
	ROM_LOAD32_WORD("0154013.u85", 0x0800000, 0x0400000, CRC(901e72e5) SHA1(437a33ac1c73824ce81229f93eff0799b369c6b0) )
	ROM_LOAD32_WORD("0154013.u72", 0x0800002, 0x0400000, CRC(a004dd06) SHA1(c394b1703d62370ecf19c51d90554d6495f43eae) )
ROM_END


// 30123211 / 714
// Top Shot / Local / C - 16/12/02
// Variations (% and NO):
//    90.01% 99
//    87.50% 01
//    92.02% 02
//    94.53% 03
//    93.51% 04
ROM_START( topshta6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30123211.u86", 0x0000000, 0x0400000, CRC(798200b2) SHA1(e753f8c8c26e30ab36705f748c01b90bcf166d82) )
	ROM_LOAD32_WORD("30123211.u73", 0x0000002, 0x0400000, CRC(486c5979) SHA1(0ece42200f31daf471b3e1e789de8b95390c4827) )
ROM_END


// 10000411 / 602/19
// Torch of the Gods / Local / B - 09/08/00
// Variations (% and NO):
//    87.870% 99
//    90.310% 01
//    92.260% 02
ROM_START( torgods )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000411.u86", 0x0000000, 0x0400000, CRC(3c7da6e8) SHA1(ff2fb056fd26d3578216e4942b6234f4523e1d6a) )
	ROM_LOAD32_WORD("10000411.u73", 0x0000002, 0x0400000, CRC(d3f27e13) SHA1(36cd1984139855170af3b23ab32b1769430809d5) )
	ROM_LOAD32_WORD("10000411.u85", 0x0800000, 0x0400000, CRC(9f7013e0) SHA1(76f948ba08152cf623559c3c8f021861757c4872) )
	ROM_LOAD32_WORD("10000411.u72", 0x0800002, 0x0400000, CRC(2dc81027) SHA1(3d426758875e87e677ab446da12532e2a4bc4246) )
ROM_END


// 20210211 / 602/19
// Torch of the Gods / ASP / C - 23/12/04
// Variations (% and NO):
//    87.87% 99
ROM_START( torgodsa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20210211.u86", 0x0000000, 0x0400000, CRC(235e4d5f) SHA1(54e854eb2002c0db13ae1415be38c543f7581814) )
	ROM_LOAD32_WORD("20210211.u73", 0x0000002, 0x0400000, CRC(57f1e9b8) SHA1(8c54f9214c794eb63c8358ad0f656bdef9f3d807) )
	ROM_LOAD32_WORD("20210211.u85", 0x0800000, 0x0400000, CRC(7ad23f53) SHA1(c383f3350c5e42ba3a159e23579c269abab92d5c) )
	ROM_LOAD32_WORD("20210211.u72", 0x0800002, 0x0400000, CRC(35cb3f32) SHA1(0de24bc1164ad90274750b7d1db804d97f46e73e) )
ROM_END


// 20007521 / 650/2
// Touch of Paradice / Local / A - 24/04/01
// Variations (% and NO):
//    87.97% 99
//    90.00% 01
//    92.06% 02
ROM_START( toprdice )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20007521.u86", 0x0000000, 0x0400000, CRC(2a3ae479) SHA1(30d5df8eb75775831d4b0792661168288cedbb16) )
	ROM_LOAD32_WORD("20007521.u73", 0x0000002, 0x0400000, CRC(e66f5672) SHA1(9bc92c061fcaf8803878941b9b74a7bc41beb299) )
	ROM_LOAD32_WORD("20007521.u85", 0x0800000, 0x0400000, CRC(9793d2c1) SHA1(50e9d644b6519f941781b726a8d566cae6bb262e) )
	ROM_LOAD32_WORD("20007521.u72", 0x0800002, 0x0400000, CRC(f2ccd55e) SHA1(24738035abc26459d3d06831c907da98aea9c257) )
ROM_END


// 0152570 / MV4268
// Treasure King / Local / NA
// Variations (% and NO):
//    87.928% 99
//    90.511% 01
//    92.161% 02
//    94.941% 03
//    97.325% 04
ROM_START( trsking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0152570-U86 Series:01 M27V322 Mk6USA   Treasure King   L06-1313-01   25EHCH7U   368A-A0F2 / 1DA075CC / DFD7FA3F   Copyright © 2004-2006 Aristocrat Technologies, Inc.
	0152570-U73 Series:01 M27V322 Mk6USA   Treasure King   L06-1313-02   C1D642FH   378F-D80A / 1D48677F / CE8A8361   Copyright © 2004-2006 Aristocrat Technologies, Inc.
	0152570-U85 Series:01 M27V322 Mk6USA   Treasure King   L06-1313-03   FP5CA663   E55C-617F / 041EDD92 / 11EAE4AF   Copyright © 2004-2006 Aristocrat Technologies, Inc.
	0152570-U72 Series:01 M27V322 Mk6USA   Treasure King   L06-1313-04   2H294U12   B0F4-BBBE / 041E5C03 / 11ACEAD6   Copyright © 2004-2006 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0152570.u86", 0x0000000, 0x0400000, CRC(8e5d3ba0) SHA1(ad3c8ba4351ba2a476aa349e2924c09be9debc20) )
	ROM_LOAD32_WORD("0152570.u73", 0x0000002, 0x0400000, CRC(70253923) SHA1(4a87a9b947404a7692404333fb4e2cd18c8de77c) )
	ROM_LOAD32_WORD("0152570.u85", 0x0800000, 0x0400000, CRC(2cece959) SHA1(c375f1fd1636898feaf1f23baf1c136e1ac09085) )
	ROM_LOAD32_WORD("0152570.u72", 0x0800002, 0x0400000, CRC(c070cb6b) SHA1(cfe6ef7723233f4325ff778679d1274159ffacab) )
ROM_END


// 10234711 / 819/1
// Turtle Treasure / Local / A - 06/10/2006
// Variations (% and NO):
//    87.91% 99
//    90.15% 01
//    92.20% 02
ROM_START( turtreas )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10234711.u86", 0x0000000, 0x0400000, CRC(1b69bb51) SHA1(b510fc80616f3acbc8264ca188c5bc85bce38e5e) )
	ROM_LOAD32_WORD("10234711.u73", 0x0000002, 0x0400000, CRC(e00da684) SHA1(4d66d48644008ff998398af1cc970ed9a1504607) )
	ROM_LOAD32_WORD("10234711.u85", 0x0800000, 0x0400000, CRC(512fcb98) SHA1(ccf5bfe8cb9dcd3d2e5e4f59033816ded04f2148) )
	ROM_LOAD32_WORD("10234711.u72", 0x0800002, 0x0400000, CRC(f864fe2e) SHA1(9aa16c4cf3936841a1152418aca7a5dd3ad5a2b3) )
ROM_END


// 10239811 / 819
// Turtle Treasure / Local / A - 21/08/2006
// Variations (% and NO):
//    87.82% 99
ROM_START( turtreasa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10239811.u86", 0x0000000, 0x0400000, CRC(c705c596) SHA1(c4733e1052fc30e21254a3583abcea6205595416) )
	ROM_LOAD32_WORD("10239811.u73", 0x0000002, 0x0400000, CRC(8da83882) SHA1(e07727cdc80757d2d8053635119ff6b8bc4a6a15) )
	ROM_LOAD32_WORD("10239811.u85", 0x0800000, 0x0400000, CRC(f1b198a9) SHA1(98f12b35b4bc6de974113e2f4bb01bd54b48d1d3) )
	ROM_LOAD32_WORD("10239811.u72", 0x0800002, 0x0400000, CRC(cc5454e5) SHA1(5369a1ace60d2813b6761bcbb2a00a52aee3e90d) )
ROM_END


// 1J008911 / JB016/1
// Venetian Nights / NSW/ACT / D - 05/08/02
// Variations (% and NO):
//    87.99% 99
//    90.11% 01
//    92.15% 02
ROM_START( venetian )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008911.u86", 0x0000000, 0x0400000, CRC(57a2a113) SHA1(98589e00803814fe51161d69d77b47ff2b6ac406) )
	ROM_LOAD32_WORD("1j008911.u73", 0x0000002, 0x0400000, CRC(c9cfb52a) SHA1(7df67226c69e4569216e5200364ff14fd162f0a7) )
	ROM_LOAD32_WORD("1j008911.u85", 0x0800000, 0x0400000, CRC(e845f6ee) SHA1(76a234e4b224d56d4f9ceb37d8d175413d4e6fe9) )
	ROM_LOAD32_WORD("1j008911.u72", 0x0800002, 0x0400000, CRC(7a635ef0) SHA1(d0c0746b58ae0726f18e90cc4b20d0e68f3abd86) )
ROM_END


// 0151355 / JB016/20
// Venice / Local / F - 07/06/04
// Variations (% and NO):
//    87.938% 99
//    90.088% 01
//    92.194% 02
//    94.961% 03
//    97.015% 04
ROM_START( venice )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151355.u86", 0x0000000, 0x0400000, CRC(a6dde7a6) SHA1(58b9bd3d80b88e9e8b40813e85b89cda69f12fb6) )
	ROM_LOAD32_WORD("0151355.u73", 0x0000002, 0x0400000, CRC(d77b73e3) SHA1(4dd4011d6371baf35fcd9cb7c878d4d2cca1e199) )
	ROM_LOAD32_WORD("0151355.u85", 0x0800000, 0x0400000, CRC(f7c3c75d) SHA1(7afc80d8d81b30c1842e4bfbf7ecd987226bdcf6) )
	ROM_LOAD32_WORD("0151355.u72", 0x0800002, 0x0400000, CRC(0570d956) SHA1(415cbaf337296527907ba36932f386aafeceec16) )
ROM_END


// 10288711 / 939
// Werewolf Wild / A - 03/12/09
// Variations (% and NO):
//    87.76% 99
//    90.13% 01
//    88.42% 07
//    87.12% 09
ROM_START( werewild )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10288711.u86", 0x0000000, 0x0400000, CRC(c8a6321d) SHA1(60278ac3b055d94e67da4041efa76d4b66431cc0) )
	ROM_LOAD32_WORD("10288711.u73", 0x0000002, 0x0400000, CRC(2d653677) SHA1(653de0cb045772c1fe533bfba6188b697b5d40bb) )
	ROM_LOAD32_WORD("10288711.u85", 0x0800000, 0x0400000, CRC(0f9af5cf) SHA1(da58075cbb9104b656c5ece827980ac42185bb6a) )
	ROM_LOAD32_WORD("10288711.u72", 0x0800002, 0x0400000, CRC(08e7d250) SHA1(e57a5da1fe9b1a2e0b775a881cf52f89a287b340) )
	ROM_LOAD32_WORD("10288711.u84", 0x1000000, 0x0400000, CRC(42cece50) SHA1(3da725f4711e2eddab3fbd1b365cd79049ccaefa) )
	ROM_LOAD32_WORD("10288711.u71", 0x1000002, 0x0400000, CRC(cc1a8ecc) SHA1(857ec89411720e7d451c76dfe4821c721f84ad06) )
ROM_END


// 20290711 / 939
// Werewolf Wild / C - 14/10/10
// Variations (% and NO):
//    87.76% 99
//    90.13% 01
//    91.82% 02
//    94.58% 03
//    85.53% 04
ROM_START( werewildq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20290711.u86", 0x0000000, 0x0400000, CRC(34bd976c) SHA1(9377ad7d3c4caeea6b13826821fd4dc1d3401fd3) )
	ROM_LOAD32_WORD("20290711.u73", 0x0000002, 0x0400000, CRC(f5c677f7) SHA1(fd5502145523c4025941e59ff4351f0dbf40a280) )
	ROM_LOAD32_WORD("20290711.u85", 0x0800000, 0x0400000, CRC(22510d63) SHA1(843785966509c4a2009392bb0db253853a8a3331) )
	ROM_LOAD32_WORD("20290711.u72", 0x0800002, 0x0400000, CRC(0edf0860) SHA1(2fa327b39b9f5d75a98d6435e9abb1030bf9943e) )
	ROM_LOAD32_WORD("20290711.u84", 0x1000000, 0x0400000, CRC(c70866df) SHA1(874b978c67de3f1620d48ba8a95b889df42bbf8d) )
	ROM_LOAD32_WORD("20290711.u71", 0x1000002, 0x0400000, CRC(f6efe70f) SHA1(c00ace27516443728ad46b87c4360f28b516fd47) )
ROM_END


// 20155711 / 695
// Whales of Cash / ASP / A - 14/06/02
// Variations (% and NO):
//    87.59% 99
ROM_START( whalecsh )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20155711.u86", 0x0000000, 0x0400000, CRC(11bcb378) SHA1(56de7fee7631c2e468a1f1845ff9d74db56051f0) )
	ROM_LOAD32_WORD("20155711.u73", 0x0000002, 0x0400000, CRC(3b6d2292) SHA1(87e50f3ed6629c697cff59ec425b098704450993) )
	ROM_LOAD32_WORD("20155711.u85", 0x0800000, 0x0400000, CRC(50afc633) SHA1(ee237d806044bbab3f17210e4e668a8f0961ad92) )
	ROM_LOAD32_WORD("20155711.u72", 0x0800002, 0x0400000, CRC(e799e162) SHA1(aad5d6bff8afcb7f208326c4a663e8462d753f9f) )
ROM_END


// 0451311 / MV4174
// Whales of Cash / Local / A - 03/31/03
// Variations (% and NO):
//    87.345% 99
//    89.923% 01
//    92.011% 02
//    94.550% 03
//    97.010% 04
ROM_START( whalecshu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451311.u86", 0x0000000, 0x0400000, CRC(a7118016) SHA1(725a46ac861bf4abaf9cc0e7084cfa20913c2299) )
	ROM_LOAD32_WORD("0451311.u73", 0x0000002, 0x0400000, CRC(74c93c56) SHA1(34c48620b08c3a03f0d6a333acbc45c14a91d3cb) )
	ROM_LOAD32_WORD("0451311.u85", 0x0800000, 0x0400000, CRC(e90f5bb1) SHA1(28680395198a29356999a377f2f3e0013d97c7b2) )
	ROM_LOAD32_WORD("0451311.u72", 0x0800002, 0x0400000, CRC(f138de3f) SHA1(69a54457c8090f23023f197f0eba1d645a260f3f) )
ROM_END


// 0451346 / MV4214
// Whales of Cash / Local / B - 01/14/05
// Variations (% and NO):
//    87.269% 99
//    90.059% 01
//    92.428% 02
//    95.060% 03
//    97.200% 04
ROM_START( whalecshua )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451346.u86", 0x0000000, 0x0400000, CRC(8aef1004) SHA1(270379b0a3abc98adfb1a9b031a5e98050635a80) )
	ROM_LOAD32_WORD("0451346.u73", 0x0000002, 0x0400000, CRC(22785960) SHA1(2ebfd230209fd438338f981c1c133e6576a67468) )
	ROM_LOAD32_WORD("0451346.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0451346.u72", 0x0800002, 0x0400000, CRC(e9ddd951) SHA1(16863b49b5db1881afda30ea19a55f6122981eba) )
ROM_END


// 10124811 / 756
// Where's The Gold / Local / B - 26/02/04
// Variations (% and NO):
//    87.95% 99
//    90.08% 01
ROM_START( wheregld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10124811.u86", 0x0000000, 0x0400000, CRC(d537f1f1) SHA1(49825365593238f757e3efecbe9fe02a06acd321) )
	ROM_LOAD32_WORD("10124811.u73", 0x0000002, 0x0400000, CRC(d8b5ccb3) SHA1(7974d68d594a2b283680cc440d69b106bd539819) )
	ROM_LOAD32_WORD("10124811.u85", 0x0800000, 0x0400000, CRC(f9554ab0) SHA1(c79d95ee339f139d0a62550e926acd6c1d7d534b) )
	ROM_LOAD32_WORD("10124811.u72", 0x0800002, 0x0400000, CRC(584050fc) SHA1(014889e1bf3fa38c052073c374b47684be717949) )
ROM_END


// 10124811 / 756
// Where's The Gold / Local / B - 26/02/04
// SHOW PROGRAM
// Variations (% and NO):
//    87.95% 99
// Identified as 101248AD in the Machine Identification menu, rather than 101248AV from the parent set
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( wheregldsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10124811.u86", 0x0000000, 0x0400000, CRC(4e6fad4a) SHA1(a2efc21cf773f9cb291ea1607d1864ba333807e9) )
	ROM_LOAD32_WORD("sp__10124811.u73", 0x0000002, 0x0400000, CRC(57253567) SHA1(41e0b1b7247ff4f0f9c6b150aef23a5eb5984e30) )
	ROM_LOAD32_WORD("sp__10124811.u85", 0x0800000, 0x0400000, CRC(86a40a48) SHA1(e0ad565e56c698df0f631e42dcfda17dfa5c8017) )
	ROM_LOAD32_WORD("sp__10124811.u72", 0x0800002, 0x0400000, CRC(bd188f12) SHA1(53433d346f23bc826fc041f1631270f8380759b6) )
ROM_END


// 20177111 / 756
// Where's The Gold / Local / C - 17/06/04
// Variations (% and NO):
//    87.95% 99
ROM_START( whereglda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20177111.u86", 0x0000000, 0x0400000, CRC(d09fd746) SHA1(6c805f7c535a6c08a836b94c0351d498751ac9e9) )
	ROM_LOAD32_WORD("20177111.u73", 0x0000002, 0x0400000, CRC(3007f8d1) SHA1(a76a227b8157d80b08e224807345a56da4c326fd) )
	ROM_LOAD32_WORD("20177111.u85", 0x0800000, 0x0400000, CRC(fb8c24e7) SHA1(ffa62c59f1857dcd670c733397d20c50710d9a85) )
	ROM_LOAD32_WORD("20177111.u72", 0x0800002, 0x0400000, CRC(9d0b5bdf) SHA1(0018c93df75b9a54f43d660a24e00dc801a68084) )
	ROM_LOAD32_WORD("20177111.u84", 0x1000000, 0x0400000, CRC(be31366b) SHA1(a82fb5bd41079c6727a93115fa74af5fe44ba4a9) )
	ROM_LOAD32_WORD("20177111.u71", 0x1000002, 0x0400000, CRC(6781f074) SHA1(d99a6b0a86317e7c8eb79d362338eb39b58aed54) )
ROM_END


// 10177111 / 756
// Where's The Gold / Local / C - 17/06/04
// Same date string as whereglda
// Variations (% and NO):
//    87.95% 99
ROM_START( wheregldb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177111.u86", 0x0000000, 0x0400000, CRC(337857b7) SHA1(5ae2e37e3f0b78dc9556579b64152e6b4e996adf) )
	ROM_LOAD32_WORD("10177111.u73", 0x0000002, 0x0400000, CRC(206bb9fc) SHA1(c1c941d35e58260a593a28470909797b967e911b) )
	ROM_LOAD32_WORD("10177111.u85", 0x0800000, 0x0400000, CRC(14b0dea3) SHA1(8143330b84e964aa9dc7904dbf22214d4b19d4e5) )
	ROM_LOAD32_WORD("10177111.u72", 0x0800002, 0x0400000, CRC(6902151b) SHA1(a13ba25cfa7527a23b0321aa24ce81d06c757cd9) )
	ROM_LOAD32_WORD("10177111.u84", 0x1000000, 0x0400000, CRC(0e04938c) SHA1(8bc6ffcbf72d98c5b8a511b2871c6943a00fda9b) )
	ROM_LOAD32_WORD("10177111.u71", 0x1000002, 0x0400000, CRC(05c4a2ab) SHA1(479e08f569c6b88b9bc2d76329477849f9014ce7) )
ROM_END


// 20184811 / 756
// Where's The Gold / QCOM / D - 21/06/05
// Variations (% and NO):
//    87.95% 99
ROM_START( wheregldq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20184811.u86", 0x0000000, 0x0400000, CRC(95da579c) SHA1(230a18b76e72f09ea74543bd6a7a31ae51bf651e) )
	ROM_LOAD32_WORD("20184811.u73", 0x0000002, 0x0400000, CRC(c35095ec) SHA1(637c5cfbb985716140a0e123c23ba13bffc2e734) )
	ROM_LOAD32_WORD("20184811.u85", 0x0800000, 0x0400000, CRC(1520539c) SHA1(9f3503bfcadc74f9e12ea7300b3356e9efb00b5a) )
	ROM_LOAD32_WORD("20184811.u72", 0x0800002, 0x0400000, CRC(50ed1445) SHA1(6b623965b95352d7f79700a075eaf4eb2019dee8) )
ROM_END


// 0151075 / US054
// WhiteWater / Local / A - 4/11/03
// 'White Water' on artwork, 'WhiteWater' in ROM
// Variations (% and NO):
//    87.813% 99
//    90.115% 01
//    92.730% 02
//    94.720% 03
//    97.302% 04
ROM_START( whtwater )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	0151075-U86 Series:01 M27V322 Mk6USA   White Water   57EPCF99   3854-2D8F / 1E63D95B / 4C57D72C   Copyright © 2003 Aristocrat Technologies, Inc.
	0151075-U73 Series:01 M27V322 Mk6USA   White Water   2C9CA446   0A68-11A5 / 1DFB10EC / 39C6BB4B   Copyright © 2003 Aristocrat Technologies, Inc.
	0151075-U85 Series:01 M27V322 Mk6USA   White Water   54?????8   A0??-??7F / 025317F1 / 2ABC5323   Copyright © 2003 Aristocrat Technologies, Inc. // First checksums unreadable due to sticker covering label
	0151075-U72 Series:01 M27V322 Mk6USA   White Water   FF97E1E4   17E2-F5F9 / 0252A2FD / 2A7582D1   Copyright © 2003 Aristocrat Technologies, Inc.
	*/
	ROM_LOAD32_WORD("0151075.u86", 0x0000000, 0x0400000, CRC(9e527f96) SHA1(34e46e530686ab5955ae267541e96d06a4e149a6) )
	ROM_LOAD32_WORD("0151075.u73", 0x0000002, 0x0400000, CRC(0e1fcaca) SHA1(725237cca34d86e53cb68547604d8844257cdf48) )
	ROM_LOAD32_WORD("0151075.u85", 0x0800000, 0x0400000, CRC(8802e0cb) SHA1(d11874426839a506d78440adbc49056ce21f88d3) )
	ROM_LOAD32_WORD("0151075.u72", 0x0800002, 0x0400000, CRC(64a814c6) SHA1(9415bb32675e5c3b6299967b1d10dbccd699d9ed) )
ROM_END


// 10214011 / 796
// Wicked Winnings II - Power Pay / Local / A - 28/02/06
// Variations (% and NO):
//    85.23% 99
//    87.50% 01
ROM_START( wikwin2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10214011.u86", 0x0000000, 0x0400000, CRC(226c8d1c) SHA1(37769691d1b7e308cbd1d857e8695f1e129a6722) )
	ROM_LOAD32_WORD("10214011.u73", 0x0000002, 0x0400000, CRC(6662912d) SHA1(017e1684de64308b035b817b6fad954be538b78c) )
ROM_END


// 0351083 / MV4076
// Wild Africa ML / Local / D - 5/08/02
// Millioniser Hyperlink game
// Variations (% and NO):
//    82.306% 05 (Hyperlink)
//    82.306% 07 (same RTP, different variation)
ROM_START( wldafr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351083.u86", 0x0000000, 0x0400000, CRC(2193094c) SHA1(57d687301a59f36880560a3efc7040e333cb05d3) )
	ROM_LOAD32_WORD("0351083.u73", 0x0000002, 0x0400000, CRC(e1e1253e) SHA1(4c15c953b80288df8f97197d70c8076fde9a0d29) )
	ROM_LOAD32_WORD("0351083.u85", 0x0800000, 0x0400000, CRC(ca451b30) SHA1(e201c06c45ee376a68bb4260cd99f0575bfac400) )
	ROM_LOAD32_WORD("0351083.u72", 0x0800002, 0x0400000, CRC(130c5385) SHA1(523b0d57b533b9b3e0fd8c0305c6099219bf2785) )
ROM_END


// 10237511 / 811
// Wild Cats / B - 20/6/06
// Variations (% and NO):
//    87.82% 99
//    90.06% 01
ROM_START( wldcata6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10237511.u86", 0x0000000, 0x0400000, CRC(f95e6ba6) SHA1(8d787d96d7b3cc3cde0be49a54701c2d3f70c659) )
	ROM_LOAD32_WORD("10237511.u73", 0x0000002, 0x0400000, CRC(0300f5ff) SHA1(52d640b676cd42dfab2c38108dd765c8e8092528) )
	ROM_LOAD32_WORD("10237511.u85", 0x0800000, 0x0400000, CRC(687c4bd6) SHA1(548de5a9887dfc58353b7a2861447bc9cd5f955e) )
	ROM_LOAD32_WORD("10237511.u72", 0x0800002, 0x0400000, CRC(f0707ab9) SHA1(f30d77c53dfbc43e9468f2cb91529e44ce10c59e) )
ROM_END


// 20258111 / 811/1
// Wild Cats / B - 14/2/07
// Variations (% and NO):
//    90.05% 01
ROM_START( wldcata6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20258111.u86", 0x0000000, 0x0400000, CRC(bc10394e) SHA1(0e21d8ca60b09ba13ac04f26d33e37bd9bda10c3) )
	ROM_LOAD32_WORD("20258111.u73", 0x0000002, 0x0400000, CRC(5652cfc3) SHA1(2fb2d9acb880dbe8e98914fae3b9d339dd53613a) )
	ROM_LOAD32_WORD("20258111.u85", 0x0800000, 0x0400000, CRC(676559a4) SHA1(db9190ec7b578fa0b862abbbf93f7c1e8bfe15c6) )
	ROM_LOAD32_WORD("20258111.u72", 0x0800002, 0x0400000, CRC(3e9b8172) SHA1(03d621680feecf8f003bd3544fa3962ec93e1733) )
ROM_END


// 30214211 / 798
// Wild Cougar - Power Pay / ASP / D - 06/06/06
// Variations (% and NO):
//    87.05% 99
ROM_START( wcougpp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30214211.u86", 0x0000000, 0x0400000, CRC(0ec06b74) SHA1(8f16751b721af3941c737652ebcdde2599c14497) )
	ROM_LOAD32_WORD("30214211.u73", 0x0000002, 0x0400000, CRC(99dc7748) SHA1(b78d1373a88b4693a689374ed52626fc9a063d2d) )
ROM_END


// 20131911 / 732
// Wild Goose / Local / B - 02/06/03
// Variations (% and NO):
//    87.62% 99
//    90.09% 01
ROM_START( wldgoos )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20131911.u86", 0x0000000, 0x0400000, CRC(0925481f) SHA1(0dfc1bbe13752fcfd270fb3c096a3612cfa2fe9a) )
	ROM_LOAD32_WORD("20131911.u73", 0x0000002, 0x0400000, CRC(5f9ab2a5) SHA1(f55dcc32463e395f1380c4fa456b39f77c27642b) )
	ROM_LOAD32_WORD("20131911.u85", 0x0800000, 0x0400000, CRC(054d3f78) SHA1(636f0284508e77d21dc9b50d59f1d7b8ed686ed0) )
	ROM_LOAD32_WORD("20131911.u72", 0x0800002, 0x0400000, CRC(ba887abf) SHA1(3d4e529bf2fed83506ef947ade08aeff7ad78611) )
ROM_END


// 10155911 / 732
// Wild Goose / ASP / C - 02/10/03
// Variations (% and NO):
//    87.62% 99
ROM_START( wldgoosa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10155911.u86", 0x0000000, 0x0400000, CRC(85e3d6af) SHA1(dd9d0f0b9f04127fb8cf2c13208ae285b5e55cfe) )
	ROM_LOAD32_WORD("10155911.u73", 0x0000002, 0x0400000, CRC(80857af5) SHA1(55995f2c9e68a765e7c75eb803136a056400960c) )
	ROM_LOAD32_WORD("10155911.u85", 0x0800000, 0x0400000, CRC(3122047d) SHA1(f9b26bd5684592447606f9dabd8bc3be3a648e33) )
	ROM_LOAD32_WORD("10155911.u72", 0x0800002, 0x0400000, CRC(38297ad3) SHA1(b3daff4b22a5a0e156473e3c3f1e886d66b9b9a1) )
ROM_END


// 20279811 / JB108/2
// Wild Gorilla / B - 19/06/09
// Variations (% and NO):
//    85.17% 99
//    87.57% 01
ROM_START( wgorilla )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20279811.u86", 0x0000000, 0x0400000, CRC(c8f15db2) SHA1(feaf68f442c96367db374772db57619ea5c01bca) )
	ROM_LOAD32_WORD("20279811.u73", 0x0000002, 0x0400000, CRC(7588c213) SHA1(b1d0ec23b8c668ae40664329a5f6a45b24f6ef2f) )
	ROM_LOAD32_WORD("20279811.u85", 0x0800000, 0x0400000, CRC(28049860) SHA1(cace39a0121127ead7c8550153372283e1610a96) )
	ROM_LOAD32_WORD("20279811.u72", 0x0800002, 0x0400000, CRC(e5a4e6f5) SHA1(f63fdc4771433f9953190b9cae4d7cbc4f6e374f) )
ROM_END


// 20218411 / 800
// Wild Panda / Local / E - 07/08/06
// Variations (% and NO):
//    87.98% 99
//    90.10% 01
//    91.99% 02
ROM_START( wldpanda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20218411.u86", 0x0000000, 0x0400000, CRC(d3ebdc8a) SHA1(fc27c96769cc3ba02e7ad4baf6055f7902e50081) )
	ROM_LOAD32_WORD("20218411.u73", 0x0000002, 0x0400000, CRC(46e28dff) SHA1(b1fd7c0d0c814f04b2bd6f35835d1d1d41aab62c) )
	ROM_LOAD32_WORD("20218411.u85", 0x0800000, 0x0400000, CRC(0bb16a0b) SHA1(6abbbb9abac53b543e92ccdead9770271d28074e) )
	ROM_LOAD32_WORD("20218411.u72", 0x0800002, 0x0400000, CRC(11e2167f) SHA1(812ed09436c2899780791ca0ada5f3bbec4a3a88) )
ROM_END


// 20225011 / 800/1
// Wild Panda / ASP / A - 04/07/06
// ROM has unchanged 10218411 ID (NSW part number) in header with "202250" string further down
// Variations (% and NO):
//    87.98% 99
ROM_START( wldpandaa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20225011.u86", 0x0000000, 0x0400000, CRC(1c0a5227) SHA1(2443fe22d1ceaf23f1048535ce3a201925da9505) )
	ROM_LOAD32_WORD("20225011.u73", 0x0000002, 0x0400000, CRC(7d8747b7) SHA1(44aebefa60d5c45876fab042a5bd74f18114b533) )
	ROM_LOAD32_WORD("20225011.u85", 0x0800000, 0x0400000, CRC(4e9ef4ef) SHA1(f9c175d3b818aba06f08c39aaeabda368e84b2c5) )
	ROM_LOAD32_WORD("20225011.u72", 0x0800002, 0x0400000, CRC(1abbfd7a) SHA1(84bdef385f98de10a3c22e2a746afcefdbb5b17b) )
ROM_END


// 0651310 / MV4187
// WILD PANTHERS / Local / D- 01/15/04
// Variations (% and NO):
//    92.475% 99
ROM_START( wldpanth )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0651310.u86", 0x0000000, 0x0400000, CRC(c874ec84) SHA1(e4f3beaa254f5b827c7f0f5d22be7dd5b3abfd2c) )
	ROM_LOAD32_WORD("0651310.u73", 0x0000002, 0x0400000, CRC(59d83a8a) SHA1(61f6e6b85108621c2eac1bf48c27d79a72fbb2fd) )
	ROM_LOAD32_WORD("0651310.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0651310.u72", 0x0800002, 0x0400000, CRC(2969f69c) SHA1(d63f1bcbd163c067f143a55aaf7c88b3c9ac0ebe) )
ROM_END


// 10230811 / 805
// Wild Stallion / Local / B - 12/09/2006
// Variations (% and NO):
//    90.20% 99
//    87.90% 01
//    91.84% 02
//    94.87% 03
ROM_START( wldstall )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10230811.u86", 0x0000000, 0x0400000, CRC(b3171fd7) SHA1(d3ac53ba6bf6cc9ea3dc0f83e8a3eecf86abad55) )
	ROM_LOAD32_WORD("10230811.u73", 0x0000002, 0x0400000, CRC(ceeff2d5) SHA1(04d5139c0ce256eb0a0734184ccc8460d2ac1120) )
	ROM_LOAD32_WORD("10230811.u85", 0x0800000, 0x0400000, CRC(6fee0413) SHA1(242f60d89eb6b44ef968f482070e0a81d2ad5fcf) )
	ROM_LOAD32_WORD("10230811.u72", 0x0800002, 0x0400000, CRC(0f4de65a) SHA1(232406f335786a045ab1833db50465ed476dfffe) )
ROM_END


// 20230211 / US054
// Wild Tepee - Power Pay / Local / H - 20/04/07
// Variations (% and NO):
//    87.05% 99
//    85.01% 01
ROM_START( wtepee )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20230211.u86", 0x0000000, 0x0400000, CRC(8c38fa9c) SHA1(0f64da87271f90309b0945403cc24fdf82c5c5a2) )
	ROM_LOAD32_WORD("20230211.u73", 0x0000002, 0x0400000, CRC(6c0a4268) SHA1(c18ad70332ec300b227edfc684db97d517031ff1) )
	ROM_LOAD32_WORD("20230211.u85", 0x0800000, 0x0400000, CRC(e5492c9b) SHA1(bd71198f24adab2841f177598d1ed8ae07aeb971) )
	ROM_LOAD32_WORD("20230211.u72", 0x0800002, 0x0400000, CRC(0c5c51c2) SHA1(6423e160355ad0380cbb46e7462f913a67e9b29d) )
ROM_END


// 10215911 / 811
// WILD WARATAH / Local / A - 14/02/06
// Variations (% and NO):
//    87.82% 99
//    90.06% 01
//    91.83% 02
ROM_START( wwaratah )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10215911.u86", 0x0000000, 0x0400000, CRC(42075131) SHA1(855822a44cf65f2bdd8c88f89e75cdac2f2ddf89) )
	ROM_LOAD32_WORD("10215911.u73", 0x0000002, 0x0400000, CRC(9df889d7) SHA1(3375a0dd56367c8aff6a07d54769715e7f29f6c4) )
	ROM_LOAD32_WORD("10215911.u85", 0x0800000, 0x0400000, CRC(c86d4145) SHA1(6b52d5d875ac4f786a2cb664dbb54c0a7cf68014) )
	ROM_LOAD32_WORD("10215911.u72", 0x0800002, 0x0400000, CRC(90813234) SHA1(80461a72cfcd056f8af7559f8f5e44447631ff0c) )
ROM_END


// 10215911 / 811
// WILD WARATAH / Local / A - 14/02/06
// SHOW PROGRAM
// Variations (% and NO):
//    87.82% 99
// Identified as 102159AD in the Machine Identification menu, rather than 102159AV from the parent set
// We had no pics of the ROM labels for the Show Program games
// they probably have some different string in the label but
// we just append a "sp__" prefix for the moment
ROM_START( wwaratahsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10215911.u86", 0x0000000, 0x0400000, CRC(0c6014c8) SHA1(c1962fc9d9453d396cc625183d1a93f686ff176f) )
	ROM_LOAD32_WORD("sp__10215911.u73", 0x0000002, 0x0400000, CRC(fc20f733) SHA1(ca0b8e9bf9e0071e6a5012515ba0c94d8dd1d838) )
	ROM_LOAD32_WORD("sp__10215911.u85", 0x0800000, 0x0400000, CRC(54c9b20d) SHA1(290ff7380dab2f721bb7576ba4fcac8dda8c35b9) )
	ROM_LOAD32_WORD("sp__10215911.u72", 0x0800002, 0x0400000, CRC(0bf1fb4b) SHA1(009b2a0ceba6d682f9ba0d856d13177d261e0827) )
ROM_END


// 10106911 / 698
// Wild Ways / Local / B - 19/06/02
// Variations (% and NO):
//    87.52% 99
//    90.02% 01
//    92.11% 02
ROM_START( wways )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10106911.u86", 0x0000000, 0x0400000, CRC(ea4dd6db) SHA1(ee07fbcc9689cdb5e96e6259b630ff1229e2cad6) )
	ROM_LOAD32_WORD("10106911.u73", 0x0000002, 0x0400000, CRC(526ef495) SHA1(ca7df6ae32822d1205ee01e6970af428c515d940) )
	ROM_LOAD32_WORD("10106911.u85", 0x0800000, 0x0400000, CRC(81e8e5ad) SHA1(612b2fd4506f99ef946e10be2db82e50fcf6083f) )
	ROM_LOAD32_WORD("10106911.u72", 0x0800002, 0x0400000, CRC(acce8d74) SHA1(e22e9611e9a221bd5dfaad3d9feece1646bebe02) )
ROM_END


// 10130111 / 698/1
// Wild Ways / Local / A - 25/02/03
// Variations (% and NO):
//    87.06% 99
ROM_START( wwaysa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130111.u86", 0x0000000, 0x0400000, CRC(2968765c) SHA1(ba2c67c4be4063d8506cc8127c31b4df2609650b) )
	ROM_LOAD32_WORD("10130111.u73", 0x0000002, 0x0400000, CRC(a1e0d77e) SHA1(df4d45d8c4dcfdb1fae4b5d5a0adfa0464c61828) )
	ROM_LOAD32_WORD("10130111.u85", 0x0800000, 0x0400000, CRC(d87426d5) SHA1(ba755f8fc426dcd8abf4f6ccee423ae0504bf6fe) )
	ROM_LOAD32_WORD("10130111.u72", 0x0800002, 0x0400000, CRC(5f2c30d1) SHA1(dfc68903e8375597477acc2fc403dcd9b838ae6c) )
ROM_END


// 10176511 / 732
// Wings Over Olympus / ASP / C - 27/01/04
ROM_START( wingoly )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10176511.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("10176511.u73", 0x0000002, 0x0400000, CRC(02792d85) SHA1(453ea853ad58525a3418e9deaccc3875a8aa7021) )
	ROM_LOAD32_WORD("10176511.u85", 0x0800000, 0x0400000, CRC(f18b5a2e) SHA1(d4d01ef3485a9133e505cfd94937944b5a957a30) )
	ROM_LOAD32_WORD("10176511.u72", 0x0800002, 0x0400000, CRC(3894de72) SHA1(c694bcb96877a5c67f0390f3834ab83c00984bdf) )
ROM_END


// 0252529 / MV4237/1
// Wings Over Olympus / Local / A - 03/07/05
// Variations (% and NO):
//    87.920% 99
//    90.128% 01
//    91.921% 02
//    94.916% 03
//    97.058% 04
ROM_START( wingolyu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252529.u86", 0x0000000, 0x0400000, CRC(c5d7d7cf) SHA1(641cdfe9b612c9cefd6a279f0c44fd535521aeef) )
	ROM_LOAD32_WORD("0252529.u73", 0x0000002, 0x0400000, CRC(5c003b1e) SHA1(be419686b89e22a1aa5d841015ab3442bffe05a6) )
	ROM_LOAD32_WORD("0252529.u85", 0x0800000, 0x0400000, CRC(63aef582) SHA1(59fa78d7d29c8d4686fab83cfa46238938a9c249) )
	ROM_LOAD32_WORD("0252529.u72", 0x0800002, 0x0400000, CRC(158877b5) SHA1(a1fc98d90a763253f07b9df44d85e8594b26d484) )
ROM_END


// 20139121 / 698
// Wizard Ways (Lady Luck) / Local / H - 30/06/03
// Variations (% and NO):
//    88.60% 02
//    87.64% 03
//    86.91% 04
//    86.05% 05
ROM_START( wzwaysll )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U86
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U73
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U85
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U72
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U84
	20139121   NSW   WIZARD WAYS   702   4MB   6 eproms   Aristocrat © 2003   P   U71
	*/
	ROM_LOAD32_WORD("20139121.u86", 0x0000000, 0x0400000, CRC(c8dffeef) SHA1(20d710356282c434523c5828fdb56e3da37d3371) )
	ROM_LOAD32_WORD("20139121.u73", 0x0000002, 0x0400000, CRC(5663bb95) SHA1(0df425bb4130079757fbbc6b5532c2f6a1a32920) )
	ROM_LOAD32_WORD("20139121.u85", 0x0800000, 0x0400000, CRC(6a8efdf2) SHA1(38b955755bbc2f2a6c8c987b0cae92d314d727c9) )
	ROM_LOAD32_WORD("20139121.u72", 0x0800002, 0x0400000, CRC(944dc1ec) SHA1(7dd2b14eb0af7babc9de09f247776f4a24d53e6c) )
	ROM_LOAD32_WORD("20139121.u84", 0x1000000, 0x0400000, CRC(0399bc1d) SHA1(19cd298d439dddac482c1d7f1fd7446900b39c46) )
	ROM_LOAD32_WORD("20139121.u71", 0x1000002, 0x0400000, CRC(e8f8ffb1) SHA1(3aa4f8d959e9671ae93c9946b5420d55c45b1bbc) )
ROM_END


// 30130611 / 743
// ZORRO / Local / B - 5/09/2003
// Variations (% and NO):
//    87.86% 99
//    85.11% 01
//    89.41% 02
ROM_START( zorroa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30130611.u86", 0x0000000, 0x0400000, CRC(adf787e0) SHA1(42cfeb3ad42bf255a9be5a629a71bababbbb0147) )
	ROM_LOAD32_WORD("30130611.u73", 0x0000002, 0x0400000, CRC(0bad02dc) SHA1(dfe495b5aeb75593d52669cd86d91f7e917bc3b0) )
	ROM_LOAD32_WORD("30130611.u85", 0x0800000, 0x0400000, CRC(68da9660) SHA1(dc6ba4c9392bd6efd72bf996d016503677ea8622) )
	ROM_LOAD32_WORD("30130611.u72", 0x0800002, 0x0400000, CRC(cc5616af) SHA1(94d1d36b76e2587caa2c60ea70c9206580dc53d5) )
ROM_END


// 20167511 / 743/1
// ZORRO / ASP
ROM_START( zorroa6a )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	/*
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U86
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U73
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U85
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U72
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U84
	20167511   ASP   ZORRO   743/1   4MB   6 eproms   Aristocrat © 2006   P   U71
	*/
	ROM_LOAD32_WORD("20167511.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20167511.u73", 0x0000002, 0x0400000, CRC(76dcf27b) SHA1(f3a33a6fd656fdc30393ac91b6aaade6967d4a99) )
	ROM_LOAD32_WORD("20167511.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20167511.u72", 0x0800002, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20167511.u84", 0x1000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("20167511.u71", 0x1000002, 0x0400000, NO_DUMP )
ROM_END


// 0151372 / MV4212
// ZORRO / Local / F - 05/13/05
// Variations (% and NO):
//    88.045% 02
//    88.044% 03
//    88.114% 04
//    88.114% 05
//    90.045% 06
//    90.044% 07
//    90.114% 08
//    90.114% 09
ROM_START( zorroa6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151372.u86", 0x0000000, 0x0400000, CRC(8d38c20c) SHA1(a0a38844aa18da72f1933571047ca36715911fb6) )
	ROM_LOAD32_WORD("0151372.u73", 0x0000002, 0x0400000, CRC(b030bdd1) SHA1(b3138ed4181342dfe1adbcb784f4e3366cb6e103) )
	ROM_LOAD32_WORD("0151372.u85", 0x0800000, 0x0400000, CRC(dead9ca1) SHA1(13b1b44a537227b6ada45ed99277d650b11aefd1) )
	ROM_LOAD32_WORD("0151372.u72", 0x0800002, 0x0400000, CRC(9e572513) SHA1(76f5578d957ec9a3a48fa9d532e6c440855d1cb6) )
	ROM_LOAD32_WORD("0151372.u84", 0x1000000, 0x0400000, CRC(e636ae58) SHA1(70e941118ae19165651157cb5219d38f58f196af) )
	ROM_LOAD32_WORD("0151372.u71", 0x1000002, 0x0400000, CRC(ff5a8592) SHA1(03d452133d30d780312066bada8e21566255209f) )
ROM_END

} // anonymous namespace


GAME( 2000, aristmk6,   0,        aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "MK6 System Software/Setchips",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_BIOS_ROOT)

// Dates listed below are for the combination (reel layout), not release dates
// Hyperlink jackpot versions e.g. Cash Express, Fast Lane, Jackpot Carnival and Maximillions are considered clones if an original set exists
GAME( 2004, 5drag,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (20161011, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    C - 14/01/04
GAME( 2004, 5draga,     5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (10176611, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    G - 15/07/04
GAME( 2005, 5dragnz,    5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (10178611, New Zealand)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4186,    B - 21/08/03
GAME( 2004, 5dragsp,    5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (20161011, NSW/ACT, Show Program)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    C - 14/01/04
GAME( 2007, 5dragce,    5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons - Cash Express (0152309, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US211,    A - 01/09/07
GAME( 2007, 5koi,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Koi - Power Pay (10250711, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744/2,    C - 25/10/07
GAME( 2007, 5koia,      5koi,     aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Koi - Power Pay (1J016211, ASP)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB067,    A - 23/02/07
GAME( 2004, 15lions,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "15 Lions (10166211, Queensland)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/3,    B - 23/03/04
GAME( 2002, 50lions,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions (10120511, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717,    A - 11/10/02
GAME( 2003, 50lionsa,   50lions,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions (10156111, ASP)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717,    D - 20/10/03
GAME( 2005, 50lionsu,   50lions,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions (0152077, US)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US128,    C - 11/09/05
GAME( 2012, 50lionrr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions - Roaring Riches (20301311, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 1049,    B - 20/08/12
GAME( 2006, 100lions,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "100 Lions (10219211, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/4,    B - 13/06/06
GAME( 2006, 100lionsa,  100lions, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "100 Lions (30223811, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/5,    A - 03/07/06
GAME( 2005, adonisa6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Adonis (0151178, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US168,    A - 05/26/05
GAME( 2001, aduskcm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "African Dusk - Mr. Cashman (20006711, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 665/1,    B- 13/07/01
GAME( 2004, afterhrs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "After Hours (0251071, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US051,    B - 01/20/04
GAME( 2003, alchemy,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Alchemy (1J009611, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013/2,    A - 30/05/03
GAME( 2007, atkgld,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "All the King's Gold (0152152, US)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US209,    A - 01/08/07
GAME( 2001, amzqueen,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Amazon Queen (10005511, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 669,    A - 02/05/01
GAME( 2009, amztempl,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Amazon Temple (10299811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 906,    A - 29/04/09
GAME( 2004, antcleoa,   antcleo,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Antony and Cleopatra (10177211, ASP)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 754,    B - 12/07/04
GAME( 2001, arabnga6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arabian Nights (2J008211, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB009/1,    E - 10/12/01
GAME( 2001, arabnga6a,  arabnga6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arabian Nights (10122611, ASP)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB009/1,    E - 10/12/01
GAME( 2004, arabnga6u,  arabnga6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arabian Nights (0151356, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/21,    E - 07/06/04
GAME( 2003, arwincm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arctic Wins - Mr. Cashman (0151065, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US029,    E- 01/27/03
GAME( 2003, arish,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arishinko (0151087, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US057,    E - 05/28/03
GAME( 2004, asnprinc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Asian Princess (0352502, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4208,    C - 09/13/04
GAME( 2008, beavbuk,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Beaver Bucks (20255111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 850,    C - 23/01/08
GAME( 2003, betzoo,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Betting Zoo - Mr. Cashman (0251064, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US037,    B - 01/28/03
GAME( 2003, bigben,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (30126911, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710,    B - 17/07/03
GAME( 2003, bigbena,    bigben,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (20126911, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710,    B - 17/07/03
GAME( 2004, bigbenb,    bigben,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (10169611, ASP)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4213,    A - 01/04/04
GAME( 2005, bigbenq,    bigben,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (20201111, Queensland)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710/3,    C - 09/08/05
GAME( 1999, bigheart,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Hearted (10000511, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 637,    D - 15/11/99
GAME( 2002, bigred,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red (2J009211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB015,    D - 27/09/02
GAME( 2002, bigreda,    bigred,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red (1J009211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB015,    D - 27/09/02
GAME( 2005, bigred2,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red 2 (5J011311, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB024,    B - 18/05/05
GAME( 2002, blkbrum,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Black Brumby (1J009111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB010,    C - 14/06/02
GAME( 2006, blkmust,    blkbrum,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Black Mustang (0152584, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/21,    (no data)
GAME( 2007, bluetigr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Blue Tiger (20259511, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 855,    A - 12/12/07
GAME( 2007, bobdolly,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bob and Dolly (0352298, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US243,    A - 10/15/07
GAME( 2005, brazil,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Brazil (20180311, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 783,    B - 25/02/05
GAME( 2006, brazila,    brazil,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Brazil (10218511, ASP)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 783,    D - 02/05/06
GAME( 2004, bucaneer,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buccaneer (Aristocrat, 10173011, NSW/ACT)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765,    A - 05/04/04
GAME( 2004, bucaneera,  bucaneer, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buccaneer (Aristocrat, 10181911, ASP)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765/1,    A - 29/07/04
GAME( 2005, bucaneeru,  bucaneer, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buccaneer (Aristocrat, 0252523, US)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4229,    B - 01/07/05
GAME( 2010, buckstop,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Buck Stops Here (10294311, NSW/ACT)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 897/1,    A - 22/07/10
GAME( 2006, buffalo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buffalo (10217811, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 806,    E - 06/10/06
GAME( 2006, buffaloa,   buffalo,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buffalo (20232611, ASP)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 806,    E - 06/10/06
GAME( 2006, buffalou,   buffalo,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buffalo (0252636, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4328,    (no data)
GAME( 2005, bmbugs,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bumble Bugs (0351180, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US166,    A - 05/13/05
GAME( 2001, bushrngr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bushranger (10006111, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 673,    A - 18/04/01
GAME( 2007, cactusco,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Cactus Corral (10237711, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 818,    D - 24/07/07
GAME( 2007, callwld,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Call of the Wild (10256611, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 827,    B - 12/11/07
GAME( 2005, calyking,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Calypso King (20197111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 772/1,    B - 25/07/05
GAME( 2006, canaryrc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Canary Riches (10218211, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 809,    A - 30/03/06
GAME( 2005, cmtonig,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Cashman Tonight (30194711, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0007,    B - 25/05/05
GAME( 2007, centrion,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Centurion (0152314, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US248,    A - 11/27/07
GAME( 2004, cinhell,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Chance in Hell - Lil' Lucy (0251125, US)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US119,    A - 06/30/04
GAME( 2002, csd,        aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (10104011, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/04/02
GAME( 2002, csdsp,      csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (20104011, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/04/02
GAME( 2003, csda,       csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (20131511, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/01/03
GAME( 2002, csdq,       csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (10121111, Queensland)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    C - 05/07/02
GAME( 2003, csdce,      csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa - Cash Express (30149511, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 03/02/03
GAME( 2003, csdjc,      csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa - Jackpot Carnival (20156611, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    G - 04/12/03
GAME( 2006, csret,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Returns (20212211, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744/2,    B - 15/08/06
GAME( 2005, corrida,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Corrida de Toros (30178311, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 771,    D - 22/03/05
GAME( 2003, cryspri,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (10144411, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 740,    A - 28/05/03
GAME( 2003, cryspria,   cryspri,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (10155811, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 740,    A - 28/05/03
GAME( 2006, cryspriu,   cryspri,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (0352557, US)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4255,    C - 01/16/06
GAME( 2009, deepfrz,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Deep Freeze (10282311, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB113/2,    A - 27/02/09
GAME( 2010, deepfrzu,   deepfrz,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Deep Freeze (0154081, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB113/4,    A - 27/07/09
GAME( 2005, deltabel,   deltaqn,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Delta Belle (0252532, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4243,    B - 04/29/05
GAME( 2004, deltaqn,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Delta Queen (20185211, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 768,    A - 13/09/04
GAME( 2002, dimeye,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamond Eyes (10129211, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB012,    G - 25/03/02
GAME( 2004, dimeyeu,    dimeye,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamond Eyes (0251312, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB012/20,    J - 01/22/04
GAME( 2004, dimeyece,   dimeye,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamond Eyes - Cash Express (0152039, US)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB012/20,    J - 01/22/04
GAME( 2002, di4ever,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamonds Forever (10105011, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 679,    B - 14/03/02
GAME( 2007, diamhrts,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamonds & Hearts - Power Pay (10229311, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 821,    D - 22/02/07
GAME( 2003, dinosa6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dinosaur (10118811, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 731,    C - 08/08/03
GAME( 2005, dblagent,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Agent (0152082, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US180,    A - 12/09/05
GAME( 2000, dbldolph,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Dolphins (20000211, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647,    C - 18/04/00
GAME( 2000, dbldolphq,  dbldolph, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Dolphins (40003211, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647,    F - 17/11/00
GAME( 2004, doltra6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dolphin Treasure (10177911, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/1,    I - 19/07/04
GAME( 2004, doltra6nz,  doltra6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dolphin Treasure (20265311, New Zealand)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4047/1,    A - 21/09/04
GAME( 2004, drgnlord,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dragon Lord (10173511, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 761,    A - 22/06/04
GAME( 2006, drgnmgic,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dragon Magic (2J015711, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB051,    E - 05/07/06
GAME( 2003, drmcatch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dream Catcher (10126721, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 704,    F - 23/06/03
GAME( 2004, drmcatcha,  drmcatch, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dream Catcher (10172921, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 704,    H - 04/03/04
GAME( 2008, drmred,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dream of the Red Chamber (10273811, NSW/ACT)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB027/2,    A - 25/8/08
GAME( 2004, eaglrock,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eagle Rock - Cash Express (0151126, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US021,    C - 05/25/04
GAME( 2010, estrike,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eagle Strike (40289711, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 950,    A - 20/07/10
GAME( 2004, eastland,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Easter Island - Cash Express (0151117, US)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US052,    B - 02/22/04
GAME( 2005, eggsrccm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eggstreme Riches - Mr. Cashman (0351149, US)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US144,    B - 11/3/05
GAME( 2007, encspell,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchanted Spell (10257011, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB041,    B - 18/07/07
GAME( 2000, enchantr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchantress (20000711, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 633,    C- 15/01/00
GAME( 2000, eott,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eyes of the Tiger (10002011, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 658,    B - 01/06/00
GAME( 2004, firednc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fire Dancer (20184911, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 772,    A - 01/10/04
GAME( 2005, firednca,   firednc,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fire Dancer (10191311, ASP)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 772,    A - 01/10/04
GAME( 2002, flamolce,   adonisa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Flame of Olympus - Cash Express (0351025, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/3,    A - 2/18/02
GAME( 2002, flamoljc,   adonisa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Flame of Olympus - Jackpot Carnival (0251048, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/3,    A - 2/18/02
GAME( 2000, followst,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Follow the Stars (20001411, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 659,    D - 15/09/00
GAME( 2007, fortath,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fortune of Athena (10248611, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 847,    C - 08/11/07
GAME( 2006, fortking,   csret,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fortune King (10230911, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    I - 26/09/06
GAME( 2004, frogwld,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Frog Wild 2 (0251114, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US112,    A - 06/03/04
GAME( 2005, gambjack,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Gamblin' Jack (20197211, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 791,    D - 23/09/05
GAME( 2004, gwnn,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Game With No Name! (0452501, US)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4210,    C - 09/13/04
GAME( 2000, geishaa6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha (10000811, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/18,    B - 03/06/00
GAME( 2001, geishaa6a,  geishaa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha (10122011, ASP)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/18,    D- 14/03/01
GAME( 2001, geishaa6b,  geishaa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha (10112411, ASP)",                        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/18,    D- 14/03/01
GAME( 2001, geishajc,   geishaa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha - Jackpot Carnival (0351033, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/2,    A - 10/18/01
GAME( 2001, goape,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Go Ape (10006211, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 661/2,    B - 14/06/01
GAME( 2001, gogreen,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Go For Green (10003411, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647/1,    A - 22/02/01
GAME( 2005, goldarch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Archer (10197411, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 770,    A - 07/07/05
GAME( 2003, gldnflc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Fleece (10158311, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 680/2,    A-07/07/03
GAME( 2005, gldgongu,   gldgong,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Gong (0152558, US)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4258,    C - 08/18/05
GAME( 2005, gldgongq,   gldgong,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Gong (10217411, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710/3,    C - 09/08/05, same date string as bigben (the games are identical but with different graphics)
GAME( 2004, goldinc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Incas (10174011, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 752,    A - 07/04/04
GAME( 2005, goldincu,   goldinc,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Incas (0252533, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4244,    A - 03/10/05
GAME( 2004, goldpya6,   qnilea6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Pyramids (10196511, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/23,    C - 23/11/04
GAME( 2002, goldpym,    qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Pyramids Special Edition - Maximillions (10130811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2007, goldsam,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Samurai (10247811, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 832,    A - 31/07/07
GAME( 2002, gbmoney,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Good, The Bad & The Money (0351016, US)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US023,    B - 09/11/02
GAME( 2006, grizzly,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Grizzly (10214711, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 780,    C - 12/04/06
GAME( 2003, heartgld,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Heart of Gold (20169511, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/2,    A - 15/09/03
GAME( 2004, heartglda,  heartgld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Heart of Gold (10184211, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/2,    B - 10/03/04
GAME( 2006, heartveg,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Heart of Vegas (10232911, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/4,    C - 27/06/06
GAME( 2002, hlntroy,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (1J008311, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB001/4,    G - 21/01/02
GAME( 2002, hlntroya,   hlntroy,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (10129121, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB001/4,    G - 21/01/02
GAME( 2002, hlntroyb,   hlntroy,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (10122711, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB001/4,    G - 21/01/02
GAME( 2004, hlntroyu,   hlntroy,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (0151336, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/21,    B - 01/16/04
GAME( 2001, highnoon,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "High Noon (20004111, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 671,    B - 28/06/01
GAME( 2002, holdrma6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Hollywood Dreams (10100511, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 670/1,    A - 31/01/02
GAME( 2004, househrt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "House of Hearts (10200311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765,    C - 09/11/04
GAME( 2005, househrta,  househrt, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "House of Hearts (10208411, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765/1,    C - 25/10/05
GAME( 2005, incachf,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Inca Chief (20190711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 784,    C - 21/10/05
GAME( 2001, indream,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming (10004211, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/6,    B - 24/04/01
GAME( 2004, indreama,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming (10192211, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/6,    C - 20/09/04
GAME( 2003, indrem2c,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming 2nd Chance (10150611, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 753,    B - 08/12/03
GAME( 2002, indremce,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Cash Express (20109411, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/10,    A - 12/07/02
GAME( 2003, indremll,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Lady Luck (20151911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 742,    C - 15/10/03
GAME( 2003, indremmm,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Maximillions (10130711, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/12,    A - 17/01/03
GAME( 2001, indgldcm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Gold - Mr. Cashman (10102511, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 674/1,    A- 21/11/01
GAME( 2004, indprncs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Princess (10174111, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 764,    A - 07/04/04
GAME( 2002, jailbrk,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Jailbreak (10125611, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 716,    A - 14/11/02
GAME( 2003, jefffox,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "You Might Be A Redneck If... - Jeff Foxworthy (0351068, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US041,    B - 06/13/03
GAME( 2003, jewelcm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Jewel of the Enchantress - Mr. Cashman (0151051, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US039,    B- 1/24/03
GAME( 2005, kingnep,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Neptune (20189111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 789,    E - 08/08/05
GAME( 2003, kingnile,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King of the Nile (30106331, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    D - 07/01/03
GAME( 2002, kingnilea,  kingnile, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King of the Nile (20106321, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    C - 27/09/02
GAME( 2003, kingnileb,  kingnile, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King of the Nile (10127511, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    G - 30/01/03
GAME( 2002, kingpeng,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Penguin (20109021, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 696,    D - 02/10/02
GAME( 2002, kingpengsp, kingpeng, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Penguin (40109021, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 696,    E- 26/11/02
GAME( 2000, koalama6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Koala Mint (30001011, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 643/6,    A- 01/09/00
GAME( 2003, kohinoor,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Kohinoor (10136711, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/21,    B - 10/02/03
GAME( 2006, lgfish,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Let's Go Fish'n (10220111, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    E - 23/05/06
GAME( 2006, lgfisha,    lgfish,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Let's Go Fish'n (10223911, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    E - 23/05/06
GAME( 2004, lemrol,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Let 'em Roll (0151115, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US088,    B - 06/24/04
GAME( 2005, lineking,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Line King (0252534, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4245,    F - 11/10/05
GAME( 2002, lordleo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Lord Leopard (30111211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 685,    B - 27/05/02
GAME( 2001, lougcm,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Louie's Gold - Mr. Cashman (20006911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 674,    C- 13/07/01
GAME( 2007, lucky88,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Lucky 88 (10241411, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 840,    C - 25/06/07
GAME( 2007, luigibal,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Luigi's Ball (10235411, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 822,    D  - 19/03/07
GAME( 2005, magforst,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Magic Forest (0151139, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US141,    D - 02/01/05
GAME( 2003, magmerm,    mystmerm, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Magic Mermaid (0251331, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013/20,    F - 10/13/03
GAME( 2006, magician,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Magician (20210111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 801,     C - 01/03/06
GAME( 2008, mchilli,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "More Chilli (20248711, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 844,    B - 21/01/08
GAME( 2010, mchilliq,   mchilli,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "More Chilli (20289311, Queensland)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 844/2,    B - 14/12/10
GAME( 2007, metestrm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Meteor Storm (0152162, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US217,    A - 03/19/07
GAME( 2007, milkin,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Milkin' It (10251911, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 816,    B - 13/08/07
GAME( 2000, mnytree,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Money Tree (10001211, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657,    E - 06/11/00
GAME( 2001, mnytreea,   mnytree,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Money Tree (10122211, ASP)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657,    F - 14/03/01
GAME( 2004, moonfire,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Moon Fire - Jackpot Carnival (30164211, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/3,    E - 27/01/04
GAME( 2007, moonwalt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Moonlight Waltz (10227611, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 814/1,    B - 11/01/07
GAME( 200?, mrwoo,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mr Woo (0252599, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4276,    (no data)
GAME( 2005, mskitty,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Miss Kitty (10216611, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    C - 18/04/05
GAME( 2002, mystarr,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Arrow (0151009, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US005,    A - 01/07/02
GAME( 2001, mysteycm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Eyes - Mr. Cashman (10008111, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 675,    A- 09/05/01
GAME( 2002, mysteycmu,  mysteycm, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Eyes - Mr. Cashman (0251024, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US020,    C- 31/05/02
GAME( 2002, mystmerm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Mermaid (1J008711, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013,    D - 27/03/02
GAME( 2005, nerdwild,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Nerds Gone Wild (10197311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    C - 18/04/05
GAME( 2006, orbeaut,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Oriental Beauty (10213511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 808,    A - 07/02/06
GAME( 2005, outbackj,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Outback Jack (20195211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 691,    J - 02/06/05
GAME( 2001, owlcap,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Owl Capone (10008711, NSW/ACT, Show Program)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 594/9,    A - 12/06/01
GAME( 2004, pandpays,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Panda Pays - Mr. Cashman (0151109, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US092,    A - 01/27/04
GAME( 2001, parislgt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Paris Lights (30002611, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 670,    A - 03/05/01
GAME( 2002, parislgtu,  parislgt, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Paris Lights - Jackpot Carnival (0251031, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US022,    A - 07/03/02
GAME( 2002, pcockmag,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Peacock Magic (1J008811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014,    B - 09/05/02
GAME( 2002, pcockmaga,  pcockmag, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Peacock Magic (10134311, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014,    C - 27/05/02
GAME( 2004, pcockmagu,  pcockmag, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Peacock Magic (0151352, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/20,    C - 07/06/04
GAME( 2004, pelipete,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete (10177511, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    A - 22/11/04
GAME( 2005, pelipetea,  pelipete, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete (10196211, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778/1,    B - 26/07/05
GAME( 2006, pelipetejc, pelipete, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete - Jackpot Carnival (10226711, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778/2, E - 23/05/06
GAME( 2005, pelipeteu,  pelipete, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete (0252555, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4251,    C - 10/28/05
GAME( 2003, pengpyce,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Penguin Pays - Cash Express (0151120, US)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US055,    A- 4/14/03
GAME( 2005, pennyar,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Penny Arcade (0251169, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US159,    A - 04/06/05
GAME( 2000, pharfor,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pharaoh's Fortune (40000921, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 656,    D - 11/01/00
GAME( 2005, phxfant,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Phoenix Fantasy (20197511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 793,    A - 21/07/05
GAME( 2000, pirata6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pirates (10001511, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 661/1,    B - 3/11/00
GAME( 2001, pirata6a,   pirata6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pirates (10122311, ASP)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 661/1,    C - 30/03/01
GAME( 2004, pcfavor,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Favourites (20177311, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0002,    D - 04/11/04
GAME( 2006, pcbydem,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - By Demand (10223111, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0005,    D - 25/09/06
GAME( 2008, pcmostw,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Most Wanted (20234911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0010,    C - 09/11/06
GAME( 2010, pcgold,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Gold Edition (30291411, New Zealand)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0047,    A - 09/04/10
GAME( 2009, pcplat,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Platinum (20282111, New Zealand)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0031,    B - 04/02/09
GAME( 2004, pcorig,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Original (30143011, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG001,    C - 12/01/04
GAME( 200?, pwsuper,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's World - Super (0154019, US)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0016,    (no data)
GAME( 2001, pompeia6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (10002211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/5,    A - 22/01/01
GAME( 2002, pompeia6a,  pompeia6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (10122411, ASP)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/9,    C - 09/01/02
GAME( 2005, pompeia6u,  pompeia6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (0151158, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US161,    A - 04/15/05
GAME( 2002, pompeia6ua, pompeia6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (0251010, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US008,    A - 04/03/02
GAME( 2005, prideaf,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pride of Africa (10208511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/9,    J - 06/12/05
GAME( 2004, prophecy,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Prophecy (20173411, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 734,    B - 10/08/04
GAME( 2005, qnilea6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile (3J011111, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB027,    C - 16/02/05
GAME( 2005, qnilea6a,   qnilea6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile (10204311, ASP)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/4,    D - 18/07/05
GAME( 2005, qnilea6b,   qnilea6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile (10192311, ASP)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/4,    H - 22/02/05
GAME( 2005, qnilemt2,   qnilea6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile - Money Train II (10203811, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/23,    E - 17/11/05
GAME( 2001, qnilese,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition (20102811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683,    A - 06/12/01
GAME( 2001, qnilesea,   qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition (10102811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683,    A - 06/12/01
GAME( 2001, qnileseb,   qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition (10127411, ASP)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683,    A - 06/12/01
GAME( 2002, qnilejc,    qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition - Jackpot Carnival (20123911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2002, qnilejcsp,  qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition - Jackpot Carnival (20123911, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2003, qsheba,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of Sheba (10133721, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    F - 04/03/03
GAME( 2003, qshebaa,    qsheba,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of Sheba (30146921, ASP)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    G - 29/07/03
GAME( 2005, raprichs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Rapid Riches (10194211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 762,    D - 09/03/05
GAME( 2002, redbara6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Red Baron (10119911, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 709,    B - 21/11/02
GAME( 2003, redbarfl,   redbara6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Red Baron - Fast Lane (10160711, Queensland)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 709,    E - 04/12/03
GAME( 2003, redbarjc,   redbara6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Red Baron - Jackpot Carnival (10160911, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 709,    E - 04/12/03, same date string as above
// The following set is clearly not a year 2000 game (an ID of #2986 would be from around 2010, note similar ID number region to buckstop/werewildq), the date string is unchanged from the original MK5 20 line version which actually was released in 2000.
GAME( 201?, reelrkfl,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Reelin-n-Rockin - Fast Lane (10298611, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/2,    E - 20/12/00
GAME( 2002, retsama6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Return of the Samurai (10117211, NSW/ACT)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/10,    A - 26/08/02
GAME( 2002, rubymag,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Ruby Magic (10125711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698,    C - 12/11/02
GAME( 2003, rubymaga,   rubymag,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Ruby Magic (10148811, ASP)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698/1,    B - 14/07/03
GAME( 2002, ruprup,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Roll Up! Roll Up!! (10115911, NSW/ACT)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642/5,    A - 22/08/02
GAME( 2010, rumbreel,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Rumble Reels (20288611, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 949,    B - 04/08/10
GAME( 2000, scatmag2,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic II (10001311, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 660,    C - 24/08/00
GAME( 2001, scatmag2a,  scatmag2, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic II (10122511, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 660,    G- 15/05/01
GAME( 2001, scatmag2sa, scatmag2, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic II (10102711, South Australia)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 660,    G - 15/05/01
GAME( 2006, scatmag3,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic III (0452569, US)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4277,    (no data)
GAME( 2002, sealdeal,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Seal The Deal - Jackpot Carnival (0251029, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US024,    A - 06/20/02
GAME( 2006, shamnmg,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shaman's Magic (10225911, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 818,    C - 14/11/06
GAME( 200?, shamnmgu,   shamnmg,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shaman's Magic (0152647, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4318/1,    (no data)
GAME( 2004, shorthng,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shore Thing (0251088, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US087,    B - 1/30/04
GAME( 2001, smtma6,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Show Me The Money (10001911, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657/1,    B - 08/01/01
GAME( 2001, smtma6q,    smtma6,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Show Me The Money (10015711, Queensland)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657/1,    D - 31/05/01
GAME( 2004, silkrda6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Silk Road (10176811, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB021,    J - 1/06/04
GAME( 2006, silkrda6u,  silkrda6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Silk Road (0152537, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4248/4,    D - 02/09/06
GAME( 2002, silgola6,   sunmoon,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Silver & Gold (20100721, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 672/1,    G - 08/07/02
GAME( 2008, skydnca6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sky Dancer (10272711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB091/2,    A - 28/08/08
GAME( 1999, spgcarn,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Carnival (10000311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642,    C - 15/11/99
GAME( 2001, spgcarnq,   spgcarn,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Carnival (10008011, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642,    E - 27/02/01
GAME( 2003, spgcarnjc,  spgcarn,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Carnival - Jackpot Carnival (10032721, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642,    F - 21/09/01
GAME( 2008, spgfest,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Festival (20262311, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB040, C - 03/06/08
GAME( 2008, spgfestnz,  spgfest,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Festival (20267211, New Zealand)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB040/2, D - 24/10/08
GAME( 2003, stardrft,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Drifter (20150811, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/2,    A - 15/09/03
GAME( 2004, stardrftu,  stardrft, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Drifter (0451341, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4204/1,    A - 02/16/04
GAME( 2001, starqst,    followst, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Quest (10009511, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 659/2,    B - 29/10/01
GAME( 2005, statsymb,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Status Symbols (0251147, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US146,    A - 02/10/05
GAME( 2003, sunmoon,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon (0251303, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173,    C - 04/01/03
GAME( 2003, sunmoona,   sunmoon,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon (0151303, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173,    C - 04/01/03
GAME( 2003, sunqndol,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun Queen - Dollar Storm (0451327, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV2070/1,    B - 11/24/03
GAME( 2005, sbuk5,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Super Bucks V (20203611, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 794,    A - 03/08/05
GAME( 2005, swhr2a6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sweethearts II (0151183, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US169,    A - 7/5/05
GAME( 2002, swh2ld,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sweethearts II - Lucky Devil (10119811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 712,    C - 16/10/02
GAME( 2006, tahitim,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tahiti Magic (10238911, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765/2,    C - 07/12/06
GAME( 2002, thaiprnc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thai Princess (10119321, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 707,    A - 17/09/02
GAME( 2004, thndrhrt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thunder Heart (10173811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 758,    B - 11/05/04
GAME( 2007, tigress,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tigress (20230111, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 807,    C - 03/04/07
GAME( 2007, tikitalk,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tiki Talk (10239111, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 831,    D - 28/03/07
GAME( 2003, tiktorch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tiki Torch (0151003, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4130/4,    A - 08/10/01
GAME( 200?, timbwolf,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Timber Wolf (0154013, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4333/1,    (no data)
GAME( 2002, topshta6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Top Shot (30123211, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 714,    C - 16/12/02
GAME( 2000, torgods,    adonisa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Torch of the Gods (10000411, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/19,    B - 09/08/00
GAME( 2004, torgodsa,   adonisa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Torch of the Gods (20210211, ASP)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/19,    C - 23/12/04
GAME( 2001, toprdice,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Touch of Paradice (20007511, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 650/2,    A - 24/04/01
GAME( 2006, trsking,    kingnep,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Treasure King (0152570, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4268,    (no data)
GAME( 2006, turtreas,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Turtle Treasure (10234711, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 819/1,    A - 06/10/06
GAME( 2006, turtreasa,  turtreas, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Turtle Treasure (10239811, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 819/1,    A - 21/08/2006
GAME( 2002, venetian,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Venetian Nights (1J008911, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/1,    D - 05/08/02
GAME( 2004, venice,     venetian, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Venice (0151355, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/20,    F - 07/06/04
GAME( 2006, wcougpp,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Cougar - Power Pay (30214211, ASP)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 798,    D - 06/06/06
GAME( 2009, werewild,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Werewolf Wild (10288711, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 939,    A - 03/12/09
GAME( 2010, werewildq,  werewild, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Werewolf Wild (20290711, Queensland)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 939,    C - 14/10/10
GAME( 2002, whalecsh,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (20155711, ASP)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 695,    A - 14/06/02
GAME( 2003, whalecshu,  whalecsh, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (0451311, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4174,    A - 03/31/03
GAME( 2004, wheregld,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (10124811, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    B - 26/02/04
GAME( 2004, wheregldsp, wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (10124811, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    B - 26/02/04
GAME( 2004, whereglda,  wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (20177111, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    C - 17/06/04
GAME( 2004, wheregldb,  wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (10177111, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    C - 17/06/04
GAME( 2005, wheregldq,  wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (20184811, Queensland)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    D - 21/06/05
GAME( 2003, whtwater,   raprichs, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "White Water (0151075, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US054,    A - 4/11/03
GAME( 2006, wikwin2,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wicked Winnings II - Power Pay (10214011, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 796,    A - 28/02/06
GAME( 2002, wldafr,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Africa - Millioniser (0351083, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4076,    D - 5/08/02
GAME( 2006, wldcata6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Cats (10237511, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    B - 20/06/06
GAME( 2007, wldcata6a,  wldcata6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Cats (20258111, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811/1,    B - 14/2/07
GAME( 2003, wldgoos,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Goose (20131911, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 732,    B - 02/06/03
GAME( 2003, wldgoosa,   wldgoos,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Goose (10155911, ASP)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 732,    C - 02/10/03
GAME( 2009, wgorilla,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Gorilla - Power Pay (20279811, NSW/ACT)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB108/2,    B - 19/06/09
GAME( 2006, wldpanda,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Panda (20218411, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 800,    E - 07/08/06
GAME( 2006, wldpandaa,  wldpanda, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Panda (20225011, ASP)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 800,    A - 04/07/06
GAME( 2006, wldstall,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Stallion (10230811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 805,    B - 12/09/06
GAME( 2007, wtepee,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Tepee - Power Pay (20230211, NSW/ACT)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 820,    H - 20/04/07
GAME( 2006, wwaratah,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Waratah (10215911, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    A - 14/02/06
GAME( 2006, wwaratahsp, wwaratah, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Waratah (10215911, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    A - 14/02/06
GAME( 2002, wways,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Ways (10106911, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698,    B - 19/06/02
GAME( 2003, wwaysa,     wways,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Ways (10130111, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698/1,  A - 25/02/03
GAME( 2005, wingolyu,   wingoly,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wings over Olympus (0252529, US)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4237/1,    A - 03/07/05
GAME( 2003, wzwaysll,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wizard Ways - Lady Luck (20139121, NSW/ACT)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 702,    H - 30/06/03
GAME( 2003, zorroa6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Zorro (30130611, NSW/ACT)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 743,    B - 5/09/03
GAME( 2005, zorroa6u,   zorroa6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Zorro (0151372, US)",                           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4212,    F - 05/13/05

// The following sets are bad dumps

GAME( 2004, antcleo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Antony and Cleopatra (10163211, NSW/ACT)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 754,    A - 15/01/04
GAME( 2003, apocof,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Apollo Coffee (0151105, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US083,    A - 11/10/03
GAME( 2005, dstgold,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Desert Gold (20202311, ASP)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 748,    D - 31/05/05
GAME( 200?, efce,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchanted Forest - Cash Express (0151084, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2001, gogreena,   gogreen,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Go For Green (10122111, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2005, gldgong,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Gong (20196011, ASP)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710/1,    ? - 12/05/05
GAME( 2002, hlntroyc,   hlntroy,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (10116411, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB001/4,    ???
GAME( 2002, holdrma6a,  holdrma6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Hollywood Dreams (10122811, ASP)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 200?, lovestuk,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Love Stuck (0152583, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4285,    (no data)
GAME( 200?, opsesam,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Open Sesame (0351309, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2001, parislgta,  parislgt, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Paris Lights (10139011, ASP)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2005, roadtrip,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Road Trip (0152539, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4238,    C - 08/18/05
GAME( 200?, snapshot,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Snap Shot (20115211, ASP)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 200?, sunmonbb,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon - Bank Buster (0153028, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173/2,    (no data)
GAME( 2003, thaiprnca,  thaiprnc, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thai Princess (30127721, ASP)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 707,    E - 21/03/03
GAME( 2007, tigressa,   tigress,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tigress (20243811, ASP)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2002, tiktorchnz, tiktorch, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tiki Torch (10124011, New Zealand)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4164,    A - 06/09/02
GAME( 2005, whalecshua, whalecsh, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (0451346, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4214,    B - 01/14/05
GAME( 2004, wingoly,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wings over Olympus (10176511, ASP)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2004, wldpanth,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Panthers - Lil' Lucy (0651310, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4187,    D- 01/15/04
GAME( 2006, zorroa6a,   zorroa6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Zorro (20167511, ASP)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 743/1,    ???
