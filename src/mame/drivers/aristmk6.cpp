// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/*

Aristocrat MK6 (2000)
Product numbers: 410480, 410481, 410556, 410557
                 410485 (System EPROM Board)

Hitachi SH4 7750
NEC PowerVR Neon 250 (PMX1-LC)
Altera FLEX EPF10K100E
PCI PLX9054
16C554 4xUART

Aristocrat MK6 XP (2002)
Product numbers: 410540, 410541
                 410663 (Storage Expansion Board)

same as above except:
- Altera APEX instead of FLEX
- doesn't have PCI controller

notes:
0x0001CA1E - critical error handler in BIOS, R4 - pointer to error message

*/

#include "emu.h"
#include "cpu/sh/sh4.h"
#include "machine/ins8250.h"
#include "machine/eepromser.h"
#include "emupal.h"
#include "screen.h"

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

	DECLARE_READ8_MEMBER(irqpend_r);
	DECLARE_WRITE8_MEMBER(irqen_w);
	DECLARE_READ8_MEMBER(test_r);
	DECLARE_WRITE64_MEMBER(eeprom_w);
	DECLARE_READ64_MEMBER(hwver_r);
	uint32_t screen_update_aristmk6(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	virtual void video_start() override;
	virtual void machine_reset() override;
	void aristmk6_map(address_map &map);
	void aristmk6_port(address_map &map);

#if 0
	u32 m_test_x,m_test_y,m_start_offs;
	u8 m_type;
#endif

	u8 irl0pend, irl0en;
	u8 irl1pend, irl1en;
	u8 irl2pend, irl2en;    // UARTs ?
	u8 irl3pend0, irl3en0;
	u8 irl3pend1, irl3en1;

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

uint32_t aristmk6_state::screen_update_aristmk6(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
#if 0
	int x,y,count;
	const uint8_t *blit_ram = memregion("maincpu")->base();

	if (machine().input().code_pressed(KEYCODE_U)) {
		irl2pend |= 2;
		testIrq();
	}

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
					bitmap.pix32(y, x) = r | g<<8 | b<<16;

				count+=2;
			}
			else
			{
				uint8_t color;

				color = blit_ram[count];

				if(cliprect.contains(x, y))
					bitmap.pix32(y, x) = m_palette->pen(color);

				count++;
			}
		}
	}
#else

	int count = 0;
	for (int y = 0;y < 480;y++)
	{
		for (int x = 0;x < 640/2;x++)
		{
			uint64_t pix = m_vram[count];

			uint32_t pix1;
			int col;

			pix1 = pix & 0xffffffff;
			col = 0;
			if (pix1) col = 1;
			bitmap.pix32(y, x*2) = m_palette->pen(col);

			pix1 = pix >> 32;
			col = 0;
			if (pix1) col = 1;
			bitmap.pix32(y, x*2+1) = m_palette->pen(col);


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

READ8_MEMBER(aristmk6_state::irqpend_r)
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

WRITE8_MEMBER(aristmk6_state::irqen_w)
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

READ8_MEMBER(aristmk6_state::test_r)
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

WRITE64_MEMBER(aristmk6_state::eeprom_w)
{
	m_eeprom0->di_write((data & 0x01) >> 0);
	m_eeprom0->cs_write((data & 0x04) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom0->clk_write((data & 0x02) ? ASSERT_LINE : CLEAR_LINE);
}

READ64_MEMBER(aristmk6_state::hwver_r)
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
	map(0x12000010, 0x12000017).w(FUNC(aristmk6_state::eeprom_w));
	map(0x12000078, 0x1200007f).nopw(); // watchdog ??
	map(0x12000080, 0x12000087).nopw(); // 0-1-2 written here repeatedly, diag LED or smth ?
	map(0x120000E0, 0x120000E7).r(FUNC(aristmk6_state::hwver_r));
	map(0x120000E8, 0x12000107).r(FUNC(aristmk6_state::irqpend_r));
	map(0x12000108, 0x12000127).w(FUNC(aristmk6_state::irqen_w));
	map(0x12400010, 0x12400017).rw(m_uart1, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
	map(0x12400018, 0x1240001f).rw(m_uart0, FUNC(ns16550_device::ins8250_r), FUNC(ns16550_device::ins8250_w));
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
	screen.set_screen_update(FUNC(aristmk6_state::screen_update_aristmk6));

	PALETTE(config, m_palette).set_entries(0x1000);
}

#define ROM_LOAD32_WORD_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(2) | ROM_BIOS(bios))

#define ROM_LOAD64_WORD_BIOS(bios, name, offset, length, hash) \
		ROMX_LOAD(name, offset, length, hash, ROM_GROUPWORD | ROM_SKIP(6) | ROM_BIOS(bios))

/*
BIOS/set chips are known to be in 3 locations depending on the PCB used:
"System Right" is at u83 on MK6 and MK6XP boards, u2 on MK6 System EPROM board and u20 on storage expansion board
"System Left"  is at u70 on MK6 and MK6XP boards, u3 on MK6 System EPROM board and u10 on storage expansion board
The earliest BIOSes used 4 chips, leaving room for only 4 game EPROMs on a standard MK6 board

To test whether the BIOS loading is fine, let's check the correct endianness of text string in CPU space
typically at around 0x3100-0x3200, 0x3600-0x3700 for Left/Right BIOS dumps (2 files)
*/

#define ARISTMK6_BIOS \
	ROM_REGION( 0x0400000, "maincpu", ROMREGION_ERASEFF) \
	ROM_SYSTEM_BIOS( 0, "au-nsw1",   "Aristocrat MK6 Base (02010114, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 0, "02010114.u84", 0x0000000, 0x0100000, CRC(183e3836) SHA1(4c802d0cd010bc007acb3a83e37aaa29b2d13d87) ) /* From Arabian Nights & Helen of Troy */ \
	ROM_LOAD64_WORD_BIOS( 0, "02010114.u71", 0x0000002, 0x0100000, CRC(8f83c3dd) SHA1(a5f9d80b4b515b24299d0241e1665cfd9da8bab7) ) \
	ROM_LOAD64_WORD_BIOS( 0, "02010114.u83", 0x0000004, 0x0100000, CRC(945104d7) SHA1(e372d0cf889c72b5d001b26fe4a925a28486537f) ) \
	ROM_LOAD64_WORD_BIOS( 0, "02010114.u70", 0x0000006, 0x0100000, CRC(3ba4379f) SHA1(84367f12c4c9224d2ab9cae83ae8727de338408c) ) \
	ROM_SYSTEM_BIOS( 1, "au-nsw2",   "Aristocrat MK6 Base (02010201, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 1, "02010201.u84", 0x0000000, 0x0100000, CRC(0920930f) SHA1(771b0f62442d1c75b1bb59ad82365b7ab8747173) ) /* From Money Tree & Go For Green */ \
	ROM_LOAD64_WORD_BIOS( 1, "02010201.u71", 0x0000002, 0x0100000, CRC(24d5614a) SHA1(fdcf3826dccc72b74b66379b1411cf211d5a1670) ) \
	ROM_LOAD64_WORD_BIOS( 1, "02010201.u83", 0x0000004, 0x0100000, CRC(5f64a20c) SHA1(397404ab6d2a1aa3c1fc77bb9421fef7079b65a5) ) \
	ROM_LOAD64_WORD_BIOS( 1, "02010201.u70", 0x0000006, 0x0100000, CRC(9b2db442) SHA1(d512398a2d9257bd385dc50d61c63cd1a47300ba) ) \
	ROM_SYSTEM_BIOS( 2, "au-nsw3",   "Aristocrat MK6 Base (02061013, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 2, "02061013_right.u83", 0x0000000, 0x0100000, CRC(7a8619a5) SHA1(bd03ddb68817c1660b009e102ccf69e5b603b875) ) \
	ROM_LOAD32_WORD_BIOS( 2, "02061013_left.u70",  0x0000002, 0x0100000, CRC(e70a7007) SHA1(0935f924866162d9c0fbdbb99391cbf730a04b76) ) \
	ROM_SYSTEM_BIOS( 3, "au-nsw4",   "Aristocrat MK6 Base (03010301, NSW/ACT)" ) \
	ROM_LOAD64_WORD_BIOS( 3, "03010301.u84", 0x0000000, 0x0100000, CRC(a34a9f16) SHA1(b8750e6ceb1715da8e5ac2f0183254e29a042641) ) /* From Queen of the Nile Special Edition */ \
	ROM_LOAD64_WORD_BIOS( 3, "03010301.u71", 0x0000002, 0x0100000, CRC(d793440a) SHA1(dced4c04bde13293af77a9a1f4c5c606e3758de0) ) \
	ROM_LOAD64_WORD_BIOS( 3, "03010301.u83", 0x0000004, 0x0100000, CRC(c8580554) SHA1(58b8bfff2f8d298c4e3be2b01900800c45fa7ad7) ) \
	ROM_LOAD64_WORD_BIOS( 3, "03010301.u70", 0x0000006, 0x0100000, CRC(5ae69121) SHA1(36dd3f9aaf5f7d2751d1954d67f898bc3ec71f3b) ) \
	ROM_SYSTEM_BIOS( 4, "au-nsw5",   "Aristocrat MK6 Base (04010501, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 4, "04010501_right.u83", 0x0000000, 0x0100000, CRC(3daefb7a) SHA1(411471713219f4bab5ccf5fe7a12a6c138c8c550) ) /* From Diamonds Forever */ \
	ROM_LOAD32_WORD_BIOS( 4, "04010501_left.u70",  0x0000002, 0x0100000, CRC(21182775) SHA1(7c5b7f5aba3babc85f512a8f7d4ebc0d83eb842a) ) \
	ROM_SYSTEM_BIOS( 5, "au-nsw6",   "Aristocrat MK6 Base (05010601, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 5, "05010601_right.u83", 0x0000000, 0x0100000, CRC(c12eac11) SHA1(683b9ddc323865ace7dca37d13b55de6e42759a5) ) /* From Thai Princess & Venetian Nights */ \
	ROM_LOAD32_WORD_BIOS( 5, "05010601_left.u70",  0x0000002, 0x0100000, CRC(b3e6b4a0) SHA1(3bf398c9257579f8e51ce716d6ebfa74fa510273) ) \
	ROM_SYSTEM_BIOS( 6, "au-nsw7",   "Aristocrat MK6 Base (07010801, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 6, "07010801_right.u83", 0x0000000, 0x0200000, CRC(8c148c11) SHA1(5ff3be18455b4f04675fec8d5b9d881295c65e23) ) /* From Jailbreak */ \
	ROM_LOAD32_WORD_BIOS( 6, "07010801_left.u70",  0x0000002, 0x0200000, CRC(8e92af68) SHA1(00d2bb655b7964a9652896741210ec534df0b0d2) ) \
	ROM_SYSTEM_BIOS( 7, "au-nsw8",   "Aristocrat MK6 Base (09011001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 7, "09011001_right.u83", 0x0000000, 0x0200000, CRC(8a853f80) SHA1(9a75498f7b02c81a483b4e1c158f35f0ee4c0112) ) \
	ROM_LOAD32_WORD_BIOS( 7, "09011001_left.u70",  0x0000002, 0x0200000, CRC(229c2e63) SHA1(91fd2b1acb69efe073647e93db9f11042add2feb) ) \
	ROM_SYSTEM_BIOS( 8, "au-nsw9",   "Aristocrat MK6 Base (11011501, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 8, "11011501_right.u83", 0x0000000, 0x0200000, CRC(de4c3aed) SHA1(21596a2edd20eb7de7a4ec8900a270b09c8f326f) ) \
	ROM_LOAD32_WORD_BIOS( 8, "11011501_left.u70",  0x0000002, 0x0200000, CRC(c5cc3461) SHA1(5b43c4cb6110a6ccf67cd0f3789253f6872b20c4) ) \
	ROM_SYSTEM_BIOS( 9, "au-nsw10",   "Aristocrat MK6 Base (11011901, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 9, "11011901_right.u83", 0x0000000, 0x0200000, CRC(73dcb11c) SHA1(69ae4f32a0c9141b2a82ff3935b0cd20333d2964) ) \
	ROM_LOAD32_WORD_BIOS( 9, "11011901_left.u70",  0x0000002, 0x0200000, CRC(d3dd2210) SHA1(3548f8cc39859d3f44a55f6bae48966a2d48e0eb) ) \
	ROM_SYSTEM_BIOS( 10, "au-nsw11",   "Aristocrat MK6 Base (13012001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 10, "13012001_right.u83", 0x0000000, 0x0200000, CRC(e627dbfa) SHA1(4fedbe0975ceb7dc0ebebf18a7708d78984db9b7) ) /* From Grizzly & Queen of the Nile */ \
	ROM_LOAD32_WORD_BIOS( 10, "13012001_left.u70",  0x0000002, 0x0200000, CRC(38e8f659) SHA1(88c6acba99b0aca023c6f4d27c061c231490e9e0) ) \
	ROM_SYSTEM_BIOS( 11, "au-nsw12",   "Aristocrat MK6 Base (14011913, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 11, "14011913_right.u83", 0x0000000, 0x0200000, CRC(01d13b89) SHA1(b1013366d0803dfbec5a5f90f6a5cea862de0513) ) \
	ROM_LOAD32_WORD_BIOS( 11, "14011913_left.u70",  0x0000002, 0x0200000, CRC(9a4cefdf) SHA1(6c15bc565ede8af19361d60ee1e6657a8055c92c) ) \
	ROM_SYSTEM_BIOS( 12, "au-nsw13",   "Aristocrat MK6 Base (19012801, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 12, "19012801_right.u83", 0x0000000, 0x0200000, CRC(5b20a96c) SHA1(5fd916b7cc2cdd51bf7dd212c1114f94dc9c7926) ) /* From Cactus Corral & Cashman Tonight & Heart of Vegas & Moonlight Waltz */ \
	ROM_LOAD32_WORD_BIOS( 12, "19012801_left.u70",  0x0000002, 0x0200000, CRC(b03bd17c) SHA1(f281e80f6dda5b727ed71d2deebe3b0ff548773f) ) \
	ROM_SYSTEM_BIOS( 13, "au-nsw14",   "Aristocrat MK6 Base (21012901, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 13, "21012901_right.u83", 0x0000000, 0x0200000, CRC(757618f2) SHA1(43f9a3e7d544979f8c6974945914d9e099b02abd) ) /* From Red Baron */ \
	ROM_LOAD32_WORD_BIOS( 13, "21012901_left.u70",  0x0000002, 0x0200000, CRC(0d271470) SHA1(5cd4b604bfe2fd7e9a8d08e1c7c97f17ae068479) ) \
	ROM_SYSTEM_BIOS( 14, "au-nsw15",   "Aristocrat MK6 Base (24013001, NSW/ACT)" ) \
	ROM_LOAD32_WORD_BIOS( 14, "24013001_right.u83", 0x0000000, 0x0200000, CRC(e97afedf) SHA1(10ca3b015afaff5d7812f0f5207b2535602136a5) ) /* From Wild Stallion */ \
	ROM_LOAD32_WORD_BIOS( 14, "24013001_left.u70",  0x0000002, 0x0200000, CRC(06ae7e07) SHA1(39a45575b66906d73b519988d1001c99b05c5f34) ) \
	ROM_SYSTEM_BIOS( 15, "au-sa1",   "Aristocrat MK6 Base (03030708, South Australia)" ) \
	ROM_LOAD32_WORD_BIOS( 15, "03030708_right.u83", 0x0000000, 0x0100000, CRC(b4b3c6a5) SHA1(5747f98a6eaa5c24a23d1d76a28b33a3bfbbfd1f) ) /* From Scatter Magic II SA */ \
	ROM_LOAD32_WORD_BIOS( 15, "03030708_left.u70",  0x0000002, 0x0100000, CRC(4e5ad823) SHA1(77ab1c29c6172cfdcef776222a72b2b44114d4da) ) \
	ROM_SYSTEM_BIOS( 16, "my",   "Aristocrat MK6 Base (15011025, Malaysia)" ) \
	ROM_LOAD32_WORD_BIOS( 16, "15011025_right.u83", 0x0000000, 0x0200000, CRC(bf21a975) SHA1(a251b1a7342387300689cd50fe4ce7975b903ac5) ) \
	ROM_LOAD32_WORD_BIOS( 16, "15011025_left.u70",  0x0000002, 0x0200000, CRC(c02e14b0) SHA1(6bf98927813519dfe60e582dbe5be3ccd87f7c91) ) \
	ROM_SYSTEM_BIOS( 17, "au-qld1",   "Aristocrat MK6 Base (14011605, Queensland))" ) \
	ROM_LOAD32_WORD_BIOS( 17, "14011605_right.u83", 0x0000000, 0x0200000, CRC(2bec5b74) SHA1(854733cada75e632f01f7096d4740ed4941a3d5b) ) /* From Moon Fire - Jackpot Carnival */ \
	ROM_LOAD32_WORD_BIOS( 17, "14011605_left.u70",  0x0000002, 0x0200000, CRC(cd26d4f0) SHA1(40822714abf08aeb08d827dbd8cd099f86803754) ) \
	ROM_SYSTEM_BIOS( 18, "au-qld2",   "Aristocrat MK6 Base (20012305, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 18, "20012305_right.u83", 0x0000000, 0x0200000, CRC(e436c1f5) SHA1(62ee529cc971fd76aa2ccc15778e3f0c40e3e47f) ) /* From Spring Festival */ \
	ROM_LOAD32_WORD_BIOS( 18, "20012305_left.u70",  0x0000002, 0x0200000, CRC(ea8961cc) SHA1(0ebc7c3b94a6e01ee984af4711043130d9670bd3) ) \
	ROM_SYSTEM_BIOS( 19, "au-qld3",   "Aristocrat MK6 Base (20012605, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 19, "20012605_right.u83", 0x0000000, 0x0200000, CRC(045b82ad) SHA1(b8e4f9f826970d83ae5fd2f2898de12ad1bf2d24) ) \
	ROM_LOAD32_WORD_BIOS( 19, "20012605_left.u70",  0x0000002, 0x0200000, CRC(87331111) SHA1(6cdc2d81f68de23af18a975a6f27ddec246be405) ) \
	ROM_SYSTEM_BIOS( 20, "au-qld4",   "Aristocrat MK6 Base (01040505, Queensland)" ) \
	ROM_LOAD64_WORD_BIOS( 20, "01040505.u84", 0x0000000, 0x0100000, CRC(cf5a9d1e) SHA1(0ebba478fc883831d70b0fa95f43e5f93b07ae9e) ) /* From Show Me The Money */ \
	ROM_LOAD64_WORD_BIOS( 20, "01040505.u71", 0x0000002, 0x0100000, CRC(f56ea77e) SHA1(319be1bee66a289e2c1f6beec07758f79aa0cf16) ) \
	ROM_LOAD64_WORD_BIOS( 20, "01040505.u83", 0x0000004, 0x0100000, CRC(90f32169) SHA1(228be8b4a9eb6b2acf7f7a7561bd194009936026) ) \
	ROM_LOAD64_WORD_BIOS( 20, "01040505.u70", 0x0000006, 0x0100000, CRC(b9ddea66) SHA1(f4bfdeada39a3f0094d6468b7374a34f88f5df7f) ) \
	ROM_SYSTEM_BIOS( 21, "au-qld5",   "Aristocrat MK6 Base (04041205, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 21, "04041205_right.u83", 0x0000000, 0x0100000, CRC(ca6bc86c) SHA1(69fe7fc35694e4cd7f861bff4ec3a6165a81df6e) ) \
	ROM_LOAD32_WORD_BIOS( 21, "04041205_left.u70",  0x0000002, 0x0100000, CRC(dfb9a119) SHA1(814a5a7877392aec4e4871d7f0e19d2fbd717409) ) \
	ROM_SYSTEM_BIOS( 22, "au-qld6",   "Aristocrat MK6 Base (03130334, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 22, "03130334_right.u83", 0x0000000, 0x0200000, CRC(bce3d97f) SHA1(da36377cc1465022a2434703adee63bf48c71a9c) ) \
	ROM_LOAD32_WORD_BIOS( 22, "03130334_left.u70",  0x0000002, 0x0200000, CRC(02175fde) SHA1(4e9a9e1e803a0c84b06aec99dc3147dd7a919eee) ) \
	ROM_SYSTEM_BIOS( 23, "au-qld7",   "Aristocrat MK6 Base (25012805, Queensland)" ) \
	ROM_LOAD32_WORD_BIOS( 23, "25012805_right.u83", 0x0000000, 0x0200000, CRC(2ecd8da8) SHA1(389e9668b2ba4fffed5d2721b2ce70d502fb9f67) ) \
	ROM_LOAD32_WORD_BIOS( 23, "25012805_left.u70",  0x0000002, 0x0200000, CRC(996f32ce) SHA1(cf21bef745986fcbd298167453c7b8e5945ce602) ) \
	ROM_SYSTEM_BIOS( 24, "us1",   "Aristocrat MK6 Base (01.02.08, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 24, "01.02.08_right.u2", 0x0000000, 0x0100000, CRC(aaaeac8c) SHA1(a565e5fcb4f55f31e7d36be40eec234248a66efd) ) \
	ROM_LOAD32_WORD_BIOS( 24, "01.02.08_left.u3",  0x0000002, 0x0100000, CRC(f29fd1bf) SHA1(33e043d2616e10a1c7a0936c3d208f9bcc2ca6f3) ) \
	ROM_SYSTEM_BIOS( 25, "us2",   "Aristocrat MK6 Base (01.03.03a, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 25, "01.03.03a_right.u83", 0x0000000, 0x0200000, CRC(253415f4) SHA1(50dc77ad87bc6be1932dda2fd4865602c8c49729) ) \
	ROM_LOAD32_WORD_BIOS( 25, "01.03.03a_left.u70",  0x0000002, 0x0200000, CRC(4ab5dd40) SHA1(a6812cc624e6a98ea7b0697e2797fe10ba8e303e) ) \
	ROM_SYSTEM_BIOS( 26, "us3",   "Aristocrat MK6 Base (01.03.03e, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 26, "01.03.03e_right.u83", 0x0000000, 0x0200000, CRC(2255e263) SHA1(5e9e093aaa17172f47a14c3baf7f6f0f73b19398) ) \
	ROM_LOAD32_WORD_BIOS( 26, "01.03.03e_left.u70",  0x0000002, 0x0200000, CRC(ea50729a) SHA1(14b5a71bfb91ac366ddcb5f77fb54127808f8163) ) \
	ROM_SYSTEM_BIOS( 27, "us4",   "Aristocrat MK6 Base (01.03.05, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 27, "01.03.05_right.u83", 0x0000000, 0x0200000, CRC(2c7f1ec3) SHA1(d03167f43ed6f9596080d91472695829378cef0a) ) \
	ROM_LOAD32_WORD_BIOS( 27, "01.03.05_left.u70",  0x0000002, 0x0200000, CRC(0095e3f9) SHA1(d2e8786158b1ab0a614aab21cf1d14cbc04754af) ) \
	ROM_SYSTEM_BIOS( 28, "us5",   "Aristocrat MK6 Base (01.03.06, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 28, "01.03.06_right.u83", 0x0000000, 0x0200000, CRC(bd48ca55) SHA1(8fb1576cbeb1c64c358880714740195d2e73e03e) ) /* From Diamond Eyes US */ \
	ROM_LOAD32_WORD_BIOS( 28, "01.03.06_left.u70",  0x0000002, 0x0200000, CRC(2f9d9a29) SHA1(fdebfaca9a579d7249379f19aef22fbfd66bf943) ) \
	ROM_SYSTEM_BIOS( 29, "us6",   "Aristocrat MK6 Base (01.03.07, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 29, "01.03.07_right.u83", 0x0000000, 0x0200000, CRC(2ebccc4e) SHA1(9342724e4451e9ab24ceae208284b50abd4f0be3) ) \
	ROM_LOAD32_WORD_BIOS( 29, "01.03.07_left.u70",  0x0000002, 0x0200000, CRC(a3632da4) SHA1(1c96a88e86095b81801ab88e36a4cdfa4b893265) ) \
	ROM_SYSTEM_BIOS( 30, "us7",   "Aristocrat MK6 Base (01.03.14, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 30, "01.03.14_right.u83", 0x0000000, 0x0200000, CRC(889ffd82) SHA1(9c98c9cdcf5f7d05095f11006418133029e9f0f8) ) /* From 5 Dragons US */ \
	ROM_LOAD32_WORD_BIOS( 30, "01.03.14_left.u70",  0x0000002, 0x0200000, CRC(7138fec4) SHA1(f81331d1875ac574d3e6c98be218ff25c6c7be5a) ) \
	ROM_SYSTEM_BIOS( 31, "us8",   "Aristocrat MK6 Base (01.03.17, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 31, "01.03.17_right.u83", 0x0000000, 0x0200000, CRC(1582714b) SHA1(92d0a15314ffe526159bef9a364898dd1ebdfde7) ) \
	ROM_LOAD32_WORD_BIOS( 31, "01.03.17_left.u70",  0x0000002, 0x0200000, CRC(a88193dc) SHA1(c9e1d483edaecd318d2e5fc8a54e84516c93e0ca) ) \
	ROM_SYSTEM_BIOS( 32, "us9",   "Aristocrat MK6 Base (01.04.14, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 32, "01.04.04_right.u83", 0x0000000, 0x0200000, CRC(e57ba02d) SHA1(8e29403e6b619eeab41dc171221720bc7820ccdc) ) \
	ROM_LOAD32_WORD_BIOS( 32, "01.04.04_left.u70",  0x0000002, 0x0200000, CRC(b984a92c) SHA1(90f7a61302caee40195c08565bdac856a3234c1d) ) \
	ROM_SYSTEM_BIOS( 33, "us10",   "Aristocrat MK6 Base (01.04.07, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 33, "01.04.07_right.u83", 0x0000000, 0x0200000, CRC(23c28e22) SHA1(98f24a1f86232b6c2c288a61ec7d60c867f192e5) ) \
	ROM_LOAD32_WORD_BIOS( 33, "01.04.07_left.u70",  0x0000002, 0x0200000, CRC(acfb0fe0) SHA1(b1a772d7978e6ff4406a5bb39a71cb3f89608e72) ) \
	ROM_SYSTEM_BIOS( 34, "us11",   "Aristocrat MK6 Base (01.04.08, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 34, "01.04.08_right.u83", 0x0000000, 0x0200000, CRC(95333304) SHA1(7afe49d6c5e4d6820f349778557daa88c5366a51) ) /* From Bob and Dolly, also u20 on EPROM expansion board */ \
	ROM_LOAD32_WORD_BIOS( 34, "01.04.08_left.u70",  0x0000002, 0x0200000, CRC(0dfcad10) SHA1(53798be000304aed38909f5fd8470a68bedd8229) ) /* also u10 on EPROM expansion board */ \
	ROM_SYSTEM_BIOS( 35, "us12",   "Aristocrat MK6 Base (01.04.10, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 35, "01.04.10_right.u83", 0x0000000, 0x0200000, CRC(82ce2fcc) SHA1(4c8fb3db084a67e99d1420b3f895a06ce9ef5ec2) ) \
	ROM_LOAD32_WORD_BIOS( 35, "01.04.10_left.u70",  0x0000002, 0x0200000, CRC(9d9d52c1) SHA1(b957220cdbedd516c219d1bfc28807ce466df93f) ) \
	ROM_SYSTEM_BIOS( 36, "us13",   "Aristocrat MK6 Base (01.04.11, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 36, "01.04.11_right.u83", 0x0000000, 0x0200000, CRC(2dae8ca0) SHA1(7a0fb38b4c1ac7195d15bdab6f0cfb16c78430f0) ) \
	ROM_LOAD32_WORD_BIOS( 36, "01.04.11_left.u70",  0x0000002, 0x0200000, CRC(787f2b07) SHA1(2548289e44f4b935346b759afb5383bdbac04c3e) ) \
	ROM_SYSTEM_BIOS( 37, "set-us1",   "Aristocrat MK6 Set Chips (06.02.04, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 37, "06.02.04_right.u2", 0x0000000, 0x0100000, CRC(1cf5a853) SHA1(64d17efcce702df7a0b0e151293199478e25226d) ) \
	ROM_LOAD32_WORD_BIOS( 37, "06.02.04_left.u3",  0x0000002, 0x0100000, CRC(117b75f2) SHA1(2129286853d3c50b8a943b71334d4ef6b98adc05) ) \
	ROM_SYSTEM_BIOS( 38, "set-us2",   "Aristocrat MK6 Set Chips (06.02.20, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 38, "06.02.20_right.u83", 0x0000000, 0x0100000, CRC(e4001f60) SHA1(5da34efb1ac0f7c84a48e09363d20cfecda4bcf1) ) \
	ROM_LOAD32_WORD_BIOS( 38, "06.02.20_left.u70",  0x0000002, 0x0100000, CRC(199ed3b9) SHA1(e3ee81ffd713f09e35a10c38e4f59282e2c5cd30) ) \
	ROM_SYSTEM_BIOS( 39, "set-us3",   "Aristocrat MK6 Set Chips (06.03.03, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 39, "06.03.03_right.u2", 0x0000000, 0x0100000, CRC(98763498) SHA1(246e95cc12eb34f946b2f4938c59217718f6d841) ) \
	ROM_LOAD32_WORD_BIOS( 39, "06.03.03_left.u3",  0x0000002, 0x0100000, CRC(a6924238) SHA1(b71ab39bf9c1fdbab556028138749e8c040ec83c) ) \
	ROM_SYSTEM_BIOS( 40, "set-us4",   "Aristocrat MK6 Set Chips (06.03.04, USA)" ) \
	ROM_LOAD32_WORD_BIOS( 40, "06.03.04_right.u2", 0x0000000, 0x0100000, CRC(6f5f5ef1) SHA1(70a43fba4de47ed8dcf38b25eafd5873f3428e72) ) \
	ROM_LOAD32_WORD_BIOS( 40, "06.03.04_left.u3",  0x0000002, 0x0100000, CRC(7034f26b) SHA1(7be78f23bec38d05240cdfe1186ec0c8291f5a1c) )

ROM_START( aristmk6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
ROM_END


/*
ROM labels all look basically the same (as with aristmk5.cpp boards)

    +--------------------------------------------------+
    | 20151911                                     NSW |
    | INDIAN DREAMING-LL                               |
    | 742                    4MB              6 eproms |
    |         Aristocrat      2005   P  U72            |
    +--------------------------------------------------+

"ID string" + "Region" + "Game Name" + "Combination No." + "ROM size"
+ "ROM numbers" + "Copyright" + "Location"

we reduce this to "ID string"."Location" for simplicity, also for games which we
don't have pictures for.

Decoding the game ID:
In the Australian style, the ID is split into four parts: Version number, Manufacturer, Game ID with the last two digits serving an unknown purpose (usually 11 but sometimes 21, 31 or 41 etc.)
In the case of the above game, this would be 2 (version), 0 (Aristocrat), 1519, 11.
For Jubilee games, the second digit is a J rather than a zero.

On US games, the ID format is different:
The US game ID seems to be split into three parts: Version number, manufacturer and Game ID, for example 0151178 (adonisa6).
In this case, the version number would be 01, followed by manufacturer (seems to always be 5), then 1178 for the game ID.
*/


ROM_START( 5drag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20161011.u86", 0x0000000, 0x0400000, CRC(ac83a71b) SHA1(1d6a3617e882e86100b939d62db7a7e1f86d2178) )
	ROM_LOAD32_WORD("20161011.u73", 0x0000002, 0x0400000, CRC(23cfdacd) SHA1(7efda9cc67214f68d674f1110cfb38fd9bbc4548) )
	ROM_LOAD32_WORD("20161011.u85", 0x0800000, 0x0400000, CRC(12803341) SHA1(4b640c3f270eaa77ce71e87f83e603083bcae489) )
	ROM_LOAD32_WORD("20161011.u72", 0x0800002, 0x0400000, CRC(e81d47fe) SHA1(f5730f9b5fa9a4492289ddcbc8e88d0a62771667) )
ROM_END


// we had no pics of the ROM labels for the Show Program PCBs
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


ROM_START( 5dragce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152309.u86", 0x0000000, 0x0400000, CRC(1c3fd707) SHA1(3a9fc3e865c43ae0afa31954b2372487eb729d8a) ) /* m27v322 */
	ROM_LOAD32_WORD("0152309.u73", 0x0000002, 0x0400000, CRC(7298e59b) SHA1(b104aec9be996bacad74120f461ad8cd7ef96e09) ) /* m27v322 */
	ROM_LOAD32_WORD("0152309.u85", 0x0800000, 0x0400000, CRC(4af3cae0) SHA1(79eb28b403aacf3ec7f52a5b3ff9e59ea35933c6) ) /* m27v322 */
	ROM_LOAD32_WORD("0152309.u72", 0x0800002, 0x0400000, CRC(e399df66) SHA1(d85a8011ab62b899b18873e4296c12da4291870a) ) /* m27v322 */
	ROM_LOAD32_WORD("0152309.u84", 0x1000000, 0x0400000, CRC(f37a1ce5) SHA1(8684d64b20e1fe853e3b0f1a31796cddeb8b0dee) ) /* m27v322 */
	ROM_LOAD32_WORD("0152309.u71", 0x1000002, 0x0400000, CRC(a0f5999e) SHA1(0be0d58588adbdbc7b728f164538d03509f13fe5) ) /* m27v322 */
ROM_END


ROM_START( 5koipp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10250711.u86", 0x0000000, 0x0400000, CRC(a245db85) SHA1(a03cf7978531edbc0ec1ba97e12fd59589caa90d) )
	ROM_LOAD32_WORD("10250711.u73", 0x0000002, 0x0400000, CRC(13fbfa4d) SHA1(8a9ba4fcea44f1ababa9ee89e36c8cd63b935ac5) )
	ROM_LOAD32_WORD("10250711.u85", 0x0800000, 0x0400000, CRC(2ce103f8) SHA1(eb457c0812f08e3a0b4450e44a80c5e13c131ea9) )
	ROM_LOAD32_WORD("10250711.u72", 0x0800002, 0x0400000, CRC(1a1140f2) SHA1(c097bcdcbd61edd15382f7becc32384780fb6e8e) )
	ROM_LOAD32_WORD("10250711.u84", 0x1000000, 0x0400000, CRC(413063ce) SHA1(4e933e1c0e576b028c16ab6f4630ee61575a5850) )
	ROM_LOAD32_WORD("10250711.u71", 0x1000002, 0x0400000, CRC(460e7ddb) SHA1(e6326523fb7c1c44cd0f01cd43d59d86a72c0d1d) )
ROM_END

ROM_START( 15lions )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10166211.u86", 0x0000000, 0x0400000, CRC(89107b2c) SHA1(34f9b51c62b37f2c3d43e9c50128028d3a4ce4a4) )
	ROM_LOAD32_WORD("10166211.u73", 0x0000002, 0x0400000, CRC(ecdc158f) SHA1(0e93f3a6084eaf4934cd131cd3bc24030eca1f83) )
ROM_END

ROM_START( 50lions )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10120511.u86", 0x0000000, 0x0400000, CRC(0e5c86f1) SHA1(84e329e664ace697f9ea4ace08612089e0964732) )
	ROM_LOAD32_WORD("10120511.u73", 0x0000002, 0x0400000, CRC(1c1f2297) SHA1(13fb8c83d8ce2340ef554490c21a38da7b47c666) )
ROM_END


ROM_START( 50lionsm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10156111.u86", 0x0000000, 0x0400000, CRC(c3791531) SHA1(b9c60be9624463eb591f2baf421ff90b8763449b) )
	ROM_LOAD32_WORD("10156111.u73", 0x0000002, 0x0400000, CRC(ec1b699b) SHA1(5a6ad7c7eb02443e42ee6a88525ae95a2b0a3195) )
ROM_END


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


ROM_START( 100lions )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10219211.u86", 0x0000000, 0x0400000, CRC(a1c71dd2) SHA1(9a859df876cf6a2fadcc5ae7183021881dc08887) )
	ROM_LOAD32_WORD("10219211.u73", 0x0000002, 0x0400000, CRC(da7d2ed7) SHA1(1f81cad150c013848988e6f995a45f7ea5c6d95c) )
ROM_END


ROM_START( adonisa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151178.u86", 0x0000000, 0x0400000, CRC(0eeb0ac3) SHA1(72b2b4711eb9d962bfb58fac1a5226ac81db67ea) )
	ROM_LOAD32_WORD("0151178.u73", 0x0000002, 0x0400000, CRC(eba9e84f) SHA1(bf746ea356627b1a558b4c8f8a2d8d5c54400a4a) )
ROM_END


ROM_START( aduskcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20006711.u86", 0x0000000, 0x0400000, CRC(26759ebc) SHA1(1205d74c39d13333ccc0ff4deb287ce12d6dbbf5) )
	ROM_LOAD32_WORD("20006711.u73", 0x0000002, 0x0400000, CRC(b156f918) SHA1(004ce75da1ba9b95c5d689a568d8e6502b7d00ab) )
	ROM_LOAD32_WORD("20006711.u85", 0x0800000, 0x0400000, CRC(f62830be) SHA1(13fbadf58ac2f3c15f78a40b3ac45e20d1080937) )
	ROM_LOAD32_WORD("20006711.u72", 0x0800002, 0x0400000, CRC(d963cc96) SHA1(8a855269d0ea92e1f58ceda555bb124746ae1268) )
ROM_END


ROM_START( afterhrs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251071.u86", 0x0000000, 0x0400000, CRC(099f7d16) SHA1(f4ccec1d3c194963a13894b41d35afa5a9191b75) )
	ROM_LOAD32_WORD("0251071.u73", 0x0000002, 0x0400000, CRC(972c569b) SHA1(caf4aaeaf05a08e4751a8bf06378d6f64b26883d) )
	ROM_LOAD32_WORD("0251071.u85", 0x0800000, 0x0400000, CRC(de79acdc) SHA1(d7d3d415f5261ab2b65ef1a1e93ccd3770124848) )
	ROM_LOAD32_WORD("0251071.u72", 0x0800002, 0x0400000, CRC(a4bee8ab) SHA1(cac34f91459fa5902b85e4c8150e651fda68621e) )
ROM_END


ROM_START( alchemy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009611.u86", 0x0000000, 0x0400000, CRC(7157c70b) SHA1(1740fd0e9a7dd72390146bfa4e2c4ef3b7d7ec0d) )
	ROM_LOAD32_WORD("1j009611.u73", 0x0000002, 0x0400000, CRC(bfac2656) SHA1(337f952fae2bff960c78f8808d3dc04815e74404) )
	ROM_LOAD32_WORD("1j009611.u85", 0x0800000, 0x0400000, CRC(a0a3453b) SHA1(f5f5559bbf8fdf176ca4f0ad755259120d29930c) )
	ROM_LOAD32_WORD("1j009611.u72", 0x0800002, 0x0400000, CRC(11321ce3) SHA1(83c6e7618b29fa7ea8c01e27be0b6e4d85822299) )
ROM_END


ROM_START( atkgld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152152.u86", 0x0000000, 0x0400000, CRC(dfd90fc4) SHA1(8881ae3102d87435585d04abd791f3f710cf5ed2) )
	ROM_LOAD32_WORD("0152152.u73", 0x0000002, 0x0400000, CRC(01d8e945) SHA1(ed0568a599fe514a8dcba762835831388fede679) )
	ROM_LOAD32_WORD("0152152.u85", 0x0800000, 0x0400000, CRC(81f8c412) SHA1(15ace6c1fc36cb7004e7767aea5e30fe8b9acf8b) )
	ROM_LOAD32_WORD("0152152.u72", 0x0800002, 0x0400000, CRC(0a360417) SHA1(f5224b6c03fe8cf0a341630086906c12a58321ed) )
ROM_END


ROM_START( amzqueen )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10005511.u86", 0x0000000, 0x0400000, CRC(2d5d9be4) SHA1(4eee74084279ae6e89bab55ef22711836eb11a93) )
	ROM_LOAD32_WORD("10005511.u73", 0x0000002, 0x0400000, CRC(7b64f0b4) SHA1(1af15ac0f41acde21606002add1212b9ac72225c) )
	ROM_LOAD32_WORD("10005511.u85", 0x0800000, 0x0400000, CRC(484d8848) SHA1(b86351fd828507968b44fd92c0f9b7ad51bfb9a8) )
	ROM_LOAD32_WORD("10005511.u72", 0x0800002, 0x0400000, CRC(1e039e60) SHA1(4f6c5db24c2841daa9b29af2732335498f8b8f0d) )
ROM_END


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


ROM_START( antcleo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10163211.u86", 0x0000000, 0x0400000, BAD_DUMP CRC(fc76d567) SHA1(80afc0edf22c592d6e98fbf9f094672adc1bf6f8) )
	ROM_LOAD32_WORD("10163211.u73", 0x0000002, 0x0400000, CRC(34c2b036) SHA1(ee67eafd51aa2c3975ea45ffa629e80621dd01f6) )
	ROM_LOAD32_WORD("10163211.u85", 0x0800000, 0x0400000, CRC(f5995ccf) SHA1(54a3e50171667f81d1fa21b89622c05a9d869b3e) )
	ROM_LOAD32_WORD("10163211.u72", 0x0800002, 0x0400000, CRC(f8219493) SHA1(9d954800985e2561ea1b01dd49f443583b98e5ec) )
ROM_END


ROM_START( antcleom )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177211.u86", 0x0000000, 0x0400000, CRC(4897f4ed) SHA1(0a071528b0c2cb4c42d4535bed406849a6187d9d) )
	ROM_LOAD32_WORD("10177211.u73", 0x0000002, 0x0400000, CRC(41b7d75d) SHA1(5c25e0bc65560b17b80c4430ae9d925a0f245e6c) )
	ROM_LOAD32_WORD("10177211.u85", 0x0800000, 0x0400000, CRC(909a5a6c) SHA1(abb86f82184f32fad578d5c3a6d034afaa78e3c3) )
	ROM_LOAD32_WORD("10177211.u72", 0x0800002, 0x0400000, CRC(121751b2) SHA1(caf5d7c2d3774575ef0a12146cbdd72d0a421c58) )
ROM_END


ROM_START( apocof )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151105.u86", 0x0000000, 0x0400000, CRC(d1f0a2a6) SHA1(d88fd4153b9a5f655c7949883caf3f947a50652f) )
	ROM_LOAD32_WORD("0151105.u73", 0x0000002, 0x0400000, CRC(675ee721) SHA1(3f4afe6196fc39c2544fc3731f39adef0198b9a3) )
	ROM_LOAD32_WORD("0151105.u85", 0x0800000, 0x0400000, CRC(daa8cc4f) SHA1(2a71c7dbd0691654b614fcd09dae4ff47a60b655) )
	ROM_LOAD32_WORD("0151105.u72", 0x0800002, 0x01c0000, BAD_DUMP CRC(f3733388) SHA1(cf01799770b785a0f7527aedf627f16642a60ff3) )
ROM_END


ROM_START( arabnga6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("2j008211.u86", 0x0000000, 0x0400000, CRC(45cf08c3) SHA1(a963a32b3bca5dec17a6cea4c0e5c66f14b42f65) )
	ROM_LOAD32_WORD("2j008211.u73", 0x0000002, 0x0400000, CRC(2d2629f0) SHA1(f7af60e390394b12a82a3125a9b931046a5e1fef) )
	ROM_LOAD32_WORD("2j008211.u85", 0x0800000, 0x0400000, CRC(03abddc6) SHA1(cc0a8c78d52b26977274455be8dadb21ee4d3580) )
	ROM_LOAD32_WORD("2j008211.u72", 0x0800002, 0x0400000, CRC(709de514) SHA1(3f380ef548c45fa8eaba9cba2bbabdcb59c7660b) )
ROM_END


ROM_START( arabnga6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151356.u86", 0x0000000, 0x0400000, CRC(ae1d3cd5) SHA1(e2cca9d9f3b205446e0f74292a784cfe65302bc8) )
	ROM_LOAD32_WORD("0151356.u73", 0x0000002, 0x0400000, CRC(a7835a67) SHA1(d7e2d60edeb553165b8be1b272c0d551e762ab33) )
	ROM_LOAD32_WORD("0151356.u85", 0x0800000, 0x0400000, CRC(4063edd6) SHA1(bb8f8230bbad70af683d039c041e90d50a59ceed) )
	ROM_LOAD32_WORD("0151356.u72", 0x0800002, 0x0400000, CRC(214632ed) SHA1(31cb4cbecf9b938df5b93c1260feeb6265bb2152) )
ROM_END


ROM_START( arwincm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151065.u86", 0x0000000, 0x0400000, CRC(961cca3d) SHA1(f5bd3fdb8863688a1bf96dcd74a261860a008952) ) /* m27v322 */
	ROM_LOAD32_WORD("0151065.u73", 0x0000002, 0x0400000, CRC(0c3e441c) SHA1(763879bae2b256c2c4edddc99507b3c54cbab976) ) /* m27v322 */
	ROM_LOAD32_WORD("0151065.u85", 0x0800000, 0x0400000, CRC(498b2ab7) SHA1(7f1da5ff34572c1f27f9a0735b8a9571b34ac17e) ) /* m27v322 */
	ROM_LOAD32_WORD("0151065.u72", 0x0800002, 0x0400000, CRC(3a133d96) SHA1(4c7dc422c3e0a81da1f81267d525e3a90a7d79d7) ) /* m27v322 */
ROM_END


ROM_START( arish )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151087.u86", 0x0000000, 0x0400000, CRC(ff9cf101) SHA1(1ae93eb26d2033b55ea5a28470a7c76183916ef4) )
	ROM_LOAD32_WORD("0151087.u73", 0x0000002, 0x0400000, CRC(631b26e7) SHA1(2520bce139f0bc57b70b93baf253b4ef2a52d531) )
	ROM_LOAD32_WORD("0151087.u85", 0x0800000, 0x0400000, CRC(0ea3b484) SHA1(7c9ab129a1c49bcb36f264ce30071e5b9fbb8985) )
	ROM_LOAD32_WORD("0151087.u72", 0x0800002, 0x0400000, CRC(08451014) SHA1(3dcda83503c361c93df27a0cd5edd4d68d2a735b) )
ROM_END


ROM_START( asnprinc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352502.u86", 0x0000000, 0x0400000, CRC(d0741b53) SHA1(29ede83b3c95d2e490dac66f60629238abc48bdd) )
	ROM_LOAD32_WORD("0352502.u73", 0x0000002, 0x0400000, CRC(b05400e0) SHA1(b0832ccfb1947738125f4afe557a246fffdd983c) )
	ROM_LOAD32_WORD("0352502.u85", 0x0800000, 0x0400000, CRC(c46cfe80) SHA1(294ddd29d5c13388096dc2551dc474f077e5d881) )
	ROM_LOAD32_WORD("0352502.u72", 0x0800002, 0x0400000, CRC(917421ef) SHA1(87eb28c98e1901830cb2b779f5c4d569699d1b1e) )
ROM_END


ROM_START( beavbuk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20255111.u86", 0x0000000, 0x0400000, CRC(2de9d8e5) SHA1(aea9f2668effa60a627cf62cee45919aa3b669b0) )
	ROM_LOAD32_WORD("20255111.u73", 0x0000002, 0x0400000, CRC(89c840dd) SHA1(31ab84740d866202cc5dfec17dec677186b991c5) )
ROM_END


ROM_START( betzoo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251064.u86", 0x0000000, 0x0400000, CRC(4cb680ba) SHA1(5c7e516203a2c91bb25f2a070c6a778035b1f35f) )
	ROM_LOAD32_WORD("0251064.u73", 0x0000002, 0x0400000, CRC(a9439adf) SHA1(60f2a5be0629377dd4a4adfa4ef6e283d5c60099) )
	ROM_LOAD32_WORD("0251064.u85", 0x0800000, 0x0400000, CRC(81b70c50) SHA1(145e7ce40c24635959261d6e2450cffa36f73869) )
	ROM_LOAD32_WORD("0251064.u72", 0x0800002, 0x0400000, CRC(98444462) SHA1(c633448d5d7739f609eb00c61d89c997db3626d2) )
ROM_END


ROM_START( bigben )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30126911.u86", 0x0000000, 0x0400000, CRC(21a31a76) SHA1(2290c502c492064967cf67f545351ff38ed6a1be) )
	ROM_LOAD32_WORD("30126911.u73", 0x0000002, 0x0400000, CRC(5e5d02f3) SHA1(5a8d7403186d3b10ef3e92297ca4b484c902095a) )
	ROM_LOAD32_WORD("30126911.u85", 0x0800000, 0x0400000, CRC(bc7846d4) SHA1(f754ca71c821f65a27c631f91ce254c76152ffde) )
	ROM_LOAD32_WORD("30126911.u72", 0x0800002, 0x0400000, CRC(b5b0fb0e) SHA1(a5f233c1472d7b27a85e2b116ccac8ef61d23e6c) )
ROM_END


ROM_START( bigbena )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20126911.u86", 0x0000000, 0x0400000, CRC(68c53dd6) SHA1(2c6c0a2a507b47da7675b4b62cf584b325427b26) )
	ROM_LOAD32_WORD("20126911.u73", 0x0000002, 0x0400000, CRC(5e43305f) SHA1(8e8f736510d45736068f101076d15900ba6aa1cd) )
	ROM_LOAD32_WORD("20126911.u85", 0x0800000, 0x0400000, CRC(64bb4204) SHA1(e20af2dba4c2feab07ba3344669964da79e6b9d1) )
	ROM_LOAD32_WORD("20126911.u72", 0x0800002, 0x0400000, CRC(d59c8c69) SHA1(b37a3d3061e6209a93e0a5de4a59ea073eedbbb7) )
ROM_END

ROM_START( bigbenq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20201111.u86", 0x0000000, 0x0400000, CRC(c27488bc) SHA1(b4d6e89b093fe850160baef067df9f261723c02e) )
	ROM_LOAD32_WORD("20201111.u73", 0x0000002, 0x0400000, CRC(d0429338) SHA1(c9e31a874603d7397b4720a3d88030671f0e8d27) )
	ROM_LOAD32_WORD("20201111.u85", 0x0800000, 0x0400000, CRC(8105b054) SHA1(b346ab044f311909803512e014a1af1bc8ada684) )
	ROM_LOAD32_WORD("20201111.u72", 0x0800002, 0x0400000, CRC(63f90aad) SHA1(2bbec33552cd3c134c4f66c49181a4b875d9daf0) )
ROM_END

ROM_START( bigheart )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000511.u86", 0x0000000, 0x0400000, CRC(b63fbc96) SHA1(f0a76f52441afe41e9bb11da1526a17d1ed9df6c) )
	ROM_LOAD32_WORD("10000511.u73", 0x0000002, 0x0400000, CRC(920ffac3) SHA1(be34f582712da5be84834562c758997829302c24) )
ROM_END


ROM_START( bigred )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("2j009211.u86", 0x0000000, 0x0400000, CRC(da16ed45) SHA1(febb8f960009361250ab78814d60d008b3778a57) )
	ROM_LOAD32_WORD("2j009211.u73", 0x0000002, 0x0400000, CRC(60e8fd7e) SHA1(7294a180d3ee869a545f43c2c898130f393d076a) )
	ROM_LOAD32_WORD("2j009211.u85", 0x0800000, 0x0400000, CRC(8c2f6fc9) SHA1(9dd687b242a3d0348cc20a7a20fef149f5f7834a) )
	ROM_LOAD32_WORD("2j009211.u72", 0x0800002, 0x0400000, CRC(bab7ae56) SHA1(d2de7e8544eabe1f116cc878f4ad3ccf22d4ca38) )
ROM_END


ROM_START( bigreda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009211.u86", 0x0000000, 0x0400000, CRC(e4740e2e) SHA1(e30cde7b39267b43fe21082441693df11736f63a) )
	ROM_LOAD32_WORD("1j009211.u73", 0x0000002, 0x0400000, CRC(9fd29e7f) SHA1(b1cac9ae3e0d226498346f099798bf3c0b78e908) )
	ROM_LOAD32_WORD("1j009211.u85", 0x0800000, 0x0400000, CRC(8c2f6fc9) SHA1(9dd687b242a3d0348cc20a7a20fef149f5f7834a) ) // same as parent but different label
	ROM_LOAD32_WORD("1j009211.u72", 0x0800002, 0x0400000, CRC(bab7ae56) SHA1(d2de7e8544eabe1f116cc878f4ad3ccf22d4ca38) ) // same as parent but different label
ROM_END


ROM_START( bigred2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("5j011311.u86", 0x0000000, 0x0400000, CRC(6e68c586) SHA1(f6261e92e9c22fbd59d385451d5b212bb888425b) )
	ROM_LOAD32_WORD("5j011311.u73", 0x0000002, 0x0400000, CRC(8826a403) SHA1(d0e08205c0fdc7229ca4d9a895a8456d8f07ff26) )
	ROM_LOAD32_WORD("5j011311.u85", 0x0800000, 0x0400000, CRC(e05a45db) SHA1(4fff979915952addc3251adb4385f0072b2033cd) )
	ROM_LOAD32_WORD("5j011311.u72", 0x0800002, 0x0400000, CRC(d28be7d5) SHA1(67820d280cb016ecdd8dd35cd0f89f51ab4d4cf1) )
ROM_END


ROM_START( blkbrum )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j009111.u86", 0x0000000, 0x0400000, CRC(5a1600be) SHA1(1fcdbc434f872c538997a604b70433d987e2d1ca) )
	ROM_LOAD32_WORD("1j009111.u73", 0x0000002, 0x0400000, CRC(0ad23541) SHA1(92ddaa68e212165bd2828360b9fdeafd9c821335) )
	ROM_LOAD32_WORD("1j009111.u85", 0x0800000, 0x0400000, CRC(5f29ac14) SHA1(7fae7adf85b90375732767fb9cd0e88097a61399) )
	ROM_LOAD32_WORD("1j009111.u72", 0x0800002, 0x0400000, CRC(95673ea4) SHA1(17ea6380473796042a44c42afda79ef270282892) )
ROM_END


ROM_START( blkmust )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152584.u86", 0x0000000, 0x0400000, CRC(2637a658) SHA1(7ec96c15f0fef5862f3d4a37fe96ab893e986321) )
	ROM_LOAD32_WORD("0152584.u73", 0x0000002, 0x0400000, CRC(986b58d9) SHA1(178de2a1bfaca3e859e9c948ce0626c5f31b303f) )
	ROM_LOAD32_WORD("0152584.u85", 0x0800000, 0x0400000, CRC(6a089a8b) SHA1(20d8fd974cedef0a7f642dbfea9b2fe088e26de0) )
	ROM_LOAD32_WORD("0152584.u72", 0x0800002, 0x0400000, CRC(032bf21e) SHA1(65ac1f37dbee5d965e6499d7f8a2ace27856d233) )
ROM_END


ROM_START( bluetigr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20259511.u86", 0x0000000, 0x0400000, CRC(a31f97d8) SHA1(67ec087cbb8f733c0f0314b58b4884fe3cc3ad8c) )
	ROM_LOAD32_WORD("20259511.u73", 0x0000002, 0x0400000, CRC(bc2a4fb8) SHA1(74ba00873a4a00d0839d89bb2d9dbed5fc33bff7) )
	ROM_LOAD32_WORD("20259511.u85", 0x0800000, 0x0400000, CRC(e175ef6d) SHA1(13933044f905c0fde96ff05c6afdce189160a5e2) )
	ROM_LOAD32_WORD("20259511.u72", 0x0800002, 0x0400000, CRC(f81af522) SHA1(5b7be10820f18ffa360726d20370faa1ecc1b3f8) )
ROM_END


ROM_START( bobdolly )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352298.u21", 0x0000000, 0x0400000, CRC(d3657369) SHA1(621fb20238f10e06c486dfbd6460f94047898a42) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u11", 0x0000002, 0x0400000, CRC(8e32da10) SHA1(28a49ecc3bab512e5828c96392d6fa4ff9394a0f) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u22", 0x0800000, 0x0400000, CRC(47429081) SHA1(23d996aa522c2f9beadf1fdb7c0584ca939917da) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u12", 0x0800002, 0x0400000, CRC(4302ae5e) SHA1(fe1d122093ad9234a7a5337b3a4c2de7fe046822) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u23", 0x1000000, 0x0400000, CRC(e73883db) SHA1(6382386ac6f576c739c959cbf6b6df2605496845) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u13", 0x1000002, 0x0400000, CRC(a9726ec5) SHA1(cdf78adc3ebcd50e503b2587564493a91b73f578) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u24", 0x1800000, 0x0400000, CRC(0f1a6b7c) SHA1(17686829a1a64f6d8982e015067f2d08bcb00402) ) /* 27v322 */
	ROM_LOAD32_WORD("0352298.u14", 0x1800002, 0x0400000, CRC(3646c16d) SHA1(c52fab2e11f39c126bcbbb348750bbf30ff96090) ) /* 27v322 */
ROM_END


ROM_START( brazil )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20180311.u86", 0x0000000, 0x0400000, CRC(9be9f72c) SHA1(5e86e42280c6b5b3ce484a04107e84b6aa457bd1) )
	ROM_LOAD32_WORD("20180311.u73", 0x0000002, 0x0400000, CRC(5eb15d8b) SHA1(dc313f96390eb04769c5c9249b140eed67796cd8) )
ROM_END


ROM_START( bucaneer )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173011.u86", 0x0000000, 0x0400000, CRC(9e1f8099) SHA1(28b1f19be0d6914afb604408e74b2d5260e11624) )
	ROM_LOAD32_WORD("10173011.u73", 0x0000002, 0x0400000, CRC(fa1e540e) SHA1(12d36d59fd176cbfad5e78f913a259fd06150f89) )
ROM_END


ROM_START( bucaneeru )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252523.u86", 0x0000000, 0x0400000, CRC(c93ecb04) SHA1(4dff453ff4716f0a732e07fbc2816ae4c4234377) )
	ROM_LOAD32_WORD("0252523.u73", 0x0000002, 0x0400000, CRC(c945bf45) SHA1(12156228feecc3363e61a18295edbcd8b995b62e) )
ROM_END


ROM_START( buckstop )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10294311.u86", 0x0000000, 0x0400000, CRC(b21c6704) SHA1(2181f3491f020400dfa0b79864860fb002ee5a7f) )
	ROM_LOAD32_WORD("10294311.u73", 0x0000002, 0x0400000, CRC(0a55ae9d) SHA1(25cfc1c4ddb5a3c26fd024f0be166763739e84e4) )
	ROM_LOAD32_WORD("10294311.u85", 0x0800000, 0x0400000, CRC(d2a29b6b) SHA1(4bfc7a6ea5d1251f57bbea52b25353f696f7e63e) )
	ROM_LOAD32_WORD("10294311.u72", 0x0800002, 0x0400000, CRC(de8c0100) SHA1(d27668584094c56f2840d87fc481ce03e7278dd8) )
ROM_END


ROM_START( buffalo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10217811.u86", 0x0000000, 0x0400000, CRC(9fc2bb3e) SHA1(2a0253500b52ebdebb61c0c8cf296ba9cff575da) )
	ROM_LOAD32_WORD("10217811.u73", 0x0000002, 0x0400000, CRC(bcc108b3) SHA1(937c5a3cdace65e706565af46597f46562830c80) )
	ROM_LOAD32_WORD("10217811.u85", 0x0800000, 0x0400000, CRC(fd6a18dc) SHA1(bd5d1a0d6c3c9c99e12234ac3ffca1100edf75fe) )
	ROM_LOAD32_WORD("10217811.u72", 0x0800002, 0x0400000, CRC(6e7b99c9) SHA1(fd1615e41ba3cf42c2853cf0ccdc0f434694b95e) )
ROM_END


ROM_START( buffalou )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252636.u86", 0x0000000, 0x0400000, CRC(8bee03c5) SHA1(46b39fe0b18c09764e7c7ac44838eb80df536100) )
	ROM_LOAD32_WORD("0252636.u73", 0x0000002, 0x0400000, CRC(46061814) SHA1(9a149d1edabc4a7ab8e30df9a08390a320b9b472) )
	ROM_LOAD32_WORD("0252636.u85", 0x0800000, 0x0400000, CRC(8e5ac194) SHA1(5e7a84d5211cb4e797aa04f21220e72f9d4f4694) )
	ROM_LOAD32_WORD("0252636.u72", 0x0800002, 0x0400000, CRC(2a35e7df) SHA1(112ea1872bc3b8533e1339f8eb19ef9ea0740773) )
ROM_END


ROM_START( bmbugs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351180.u86", 0x0000000, 0x0400000, CRC(f5c204f9) SHA1(592fac02d26ec5edffce862fcce67869430028dd) )
	ROM_LOAD32_WORD("0351180.u73", 0x0000002, 0x0400000, CRC(79dd907f) SHA1(06352c55ff36a82f4d0550b1723667f3287c4923) ) /* 27v322 */
ROM_END


ROM_START( bushrngr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10006111.u86", 0x0000000, 0x0400000, CRC(e2548c80) SHA1(485476a71f8910fb20a185e5883f863e0628e060) )
	ROM_LOAD32_WORD("10006111.u73", 0x0000002, 0x0400000, CRC(73206610) SHA1(9a9eb07efde20c5b079a39c96db447937f555c93) )
	ROM_LOAD32_WORD("10006111.u85", 0x0800000, 0x0400000, CRC(5b471edb) SHA1(78da72b727d71f24fbfed34feda9167dd10d96e3) )
	ROM_LOAD32_WORD("10006111.u72", 0x0800002, 0x0400000, CRC(71a0084f) SHA1(1e7b7b58b2df25ea0468d702d7eef658d7c53f47) )
ROM_END


ROM_START( cactusco )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10237711.u86", 0x0000000, 0x0400000, CRC(bd7a64ed) SHA1(0047467954eabc718b5355e61f2756fd9e29aabc) )
	ROM_LOAD32_WORD("10237711.u73", 0x0000002, 0x0400000, CRC(b2f2149d) SHA1(8f53b168555bc255d3dc9fb94d2d6dd466b7f3e4) )
	ROM_LOAD32_WORD("10237711.u85", 0x0800000, 0x0400000, CRC(e0130bf6) SHA1(84f7c2b5c10155ee92bcc02d6c3dca0acf55ed27) )
	ROM_LOAD32_WORD("10237711.u72", 0x0800002, 0x0400000, CRC(90d72135) SHA1(fb4a4ad984ff1103e2e951919132f79cb7af9ec5) )
ROM_END


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


ROM_START( calyking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197111.u86", 0x0000000, 0x0400000, CRC(09ae1f6a) SHA1(2bfc4fb9e01084c460321bb3fa29cee0600c5654) )
	ROM_LOAD32_WORD("20197111.u73", 0x0000002, 0x0400000, CRC(a6f903f6) SHA1(6101cc9115b91c6860cfd3fbe5ab2da105a41ef4) )
	ROM_LOAD32_WORD("20197111.u85", 0x0800000, 0x0400000, CRC(4e130673) SHA1(6d49558146c0e3e8a0f1659dcc2ebe60ae17782b) )
	ROM_LOAD32_WORD("20197111.u72", 0x0800002, 0x0400000, CRC(1a09ab1e) SHA1(8aca20b52d1072387e49e6d020c6e9565650aec9) )
ROM_END


ROM_START( canaryrc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10218211.u86", 0x0000000, 0x0400000, CRC(37750a22) SHA1(dcb73df1c6bd206343302a8d70966291750a5baf) )
	ROM_LOAD32_WORD("10218211.u73", 0x0000002, 0x0400000, CRC(6ae381ec) SHA1(d1103744805137dc69b77a3e1319c7168c88306f) )
ROM_END


/*
    Cashman Tonight
    4-in-1 game containing African Dusk, Arctic Wins, King Galah and Wild Cougar.
    Arctic Wins is a completely new game, it is not the same version as the original MK4 game.
    Wild Cougar is different to the original in that it has stacked symbols e.g. 3 of the same symbol (except Cougar and Wolf) can show up on the same reel.
    Cashman Tonight has a progressive jackpot unlike the original Mr. Cashman games.
*/
ROM_START( cmtonig )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30194711.u21", 0x0000000, 0x0400000, CRC(e39fdd27) SHA1(b5e391740ba695b09e2ecb169760240f39b6a27a) )
	ROM_LOAD32_WORD("30194711.u11", 0x0000002, 0x0400000, CRC(9defed89) SHA1(0496ee9b378b7117a8cea89c7b3fcca1bae20e64) )
	ROM_LOAD32_WORD("30194711.u22", 0x0800000, 0x0400000, CRC(3c93d700) SHA1(f940d3eff2225fa3470372b7cd8b630511d53843) )
	ROM_LOAD32_WORD("30194711.u12", 0x0800002, 0x0400000, CRC(2a9dd5fd) SHA1(ca484bf494b7431baaadaf7df663ffd84ea07120) )
	ROM_LOAD32_WORD("30194711.u23", 0x1000000, 0x0400000, CRC(1bce542e) SHA1(8dfb78d966ee50d866739f86c6da9b33e09cd108) )
	ROM_LOAD32_WORD("30194711.u13", 0x1000002, 0x0400000, CRC(2cbd3ef8) SHA1(c4a830c9bae7c50276ba951128056292ac09e19d) )
	ROM_LOAD32_WORD("30194711.u24", 0x1800000, 0x0400000, CRC(cb1a9281) SHA1(1de3b988336fb930d636f3bad37d3d5dc359713b) )
	ROM_LOAD32_WORD("30194711.u14", 0x1800002, 0x0400000, CRC(1dbc278e) SHA1(ac48fce5256a04eabd8271de96b300fb7c265927) )
	ROM_LOAD32_WORD("30194711.u25", 0x2000000, 0x0400000, CRC(5f7dd3f8) SHA1(23db92025455b7ab089a990b2f2cab3acb9ab3b5) )
	ROM_LOAD32_WORD("30194711.u15", 0x2000002, 0x0400000, CRC(5e28e9cb) SHA1(e45cce804fe014078d399a04ca68f1a79d2d5f41) )
	ROM_LOAD32_WORD("30194711.u26", 0x2800000, 0x0400000, CRC(6ac1430d) SHA1(a782cdcdd80e59f8f7de9a16559609759eaefb78) )
	ROM_LOAD32_WORD("30194711.u16", 0x2800002, 0x0400000, CRC(b9d91cab) SHA1(0e79c9e02068c47a9ffc7ace70adad2b579d559f) )
	ROM_LOAD32_WORD("30194711.u27", 0x3000000, 0x0400000, CRC(9b69c5ca) SHA1(f13c4355319c6c45316a05f74a561964657990b8) )
	ROM_LOAD32_WORD("30194711.u17", 0x3000002, 0x0400000, CRC(1bf2985c) SHA1(7c037b290983b70efe5f72cb8b8b30e012b53860) )
ROM_END


/*
    Chance In Hell - Lil' Lucy
    The Lil' Lucy character was originally called Lucky Devil in Australia.
*/
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


ROM_START( csd )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10104011.u86", 0x0000000, 0x0400000, CRC(674fbe7a) SHA1(5292dd3887f066e6ea150fe8c140c2d3cde142d9) )
	ROM_LOAD32_WORD("10104011.u73", 0x0000002, 0x0400000, CRC(78fb8acb) SHA1(60cde2c9fd0c3a6ac8a76099906c87393d90b8bf) )
	ROM_LOAD32_WORD("10104011.u85", 0x0800000, 0x0400000, CRC(e1b6c747) SHA1(3e0345357c4b66b30c87e6ebf540fcc636505804) )
	ROM_LOAD32_WORD("10104011.u72", 0x0800002, 0x0400000, CRC(732bc777) SHA1(bc3ced0a3e3e75151bfdf1c7d79fff2ebac15230) )
ROM_END


ROM_START( csdsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20104011.u86", 0x0000000, 0x0400000, CRC(a60deda0) SHA1(d439f3f0b267d94f88f74052068becadc96fd76e) )
	ROM_LOAD32_WORD("20104011.u73", 0x0000002, 0x0400000, CRC(00dcb4e5) SHA1(d7d32c15bdea0e28e936b87ed7b609e979aae460) )
	ROM_LOAD32_WORD("20104011.u85", 0x0800000, 0x0400000, CRC(fdeb3434) SHA1(df306a2443b46c565040307b61ed97242f4b7280) )
	ROM_LOAD32_WORD("20104011.u72", 0x0800002, 0x0400000, CRC(566792bf) SHA1(843ccbc65020ef87ef67b9d81b864c1d8674b1dd) )
ROM_END


ROM_START( csdm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20131511.u86", 0x0000000, 0x0400000, CRC(06f78c92) SHA1(e8bd3f18831dfb5c644321541fa9e75ae9e83688) )
	ROM_LOAD32_WORD("20131511.u73", 0x0000002, 0x0400000, CRC(5b2468b6) SHA1(085aa44343f11fdf5ab7cc1ca56ddb0ba5cafc36) )
	ROM_LOAD32_WORD("20131511.u85", 0x0800000, 0x0400000, CRC(6973dffd) SHA1(4350e0cdfeb9135e708f15bf2de325b8412c1434) )
	ROM_LOAD32_WORD("20131511.u72", 0x0800002, 0x0400000, CRC(1c9a77d1) SHA1(ef31215be73b09531039fda1a2f3e875045180f3) )
ROM_END


ROM_START( csdq )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10121111.u86", 0x0000000, 0x0400000, CRC(cbcabeae) SHA1(ebc951037bfda441833beb5b0ca102dc76fcc901) )
	ROM_LOAD32_WORD("10121111.u73", 0x0000002, 0x0400000, CRC(4bb97e62) SHA1(8144b6ec8a69b12fd4e4641999a988ad6a9e5160) )
	ROM_LOAD32_WORD("10121111.u85", 0x0800000, 0x0400000, CRC(8425f9d0) SHA1(81a86caf20d2ee18f6a3e68da58e69ff00bbc68a) )
	ROM_LOAD32_WORD("10121111.u72", 0x0800002, 0x0400000, CRC(f11f4f9d) SHA1(684930290f8b5c933bb72a08db4875ce9b7483da) )
ROM_END


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


ROM_START( csret )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20212211.u86", 0x0000000, 0x0400000, CRC(0136afeb) SHA1(ec8478216353100a541cf736d54fc440cd89d665) )
	ROM_LOAD32_WORD("20212211.u73", 0x0000002, 0x0400000, CRC(3dadf971) SHA1(ffab779da0160d0e8a78fdc9e1440b2f27d04d20) )
	ROM_LOAD32_WORD("20212211.u85", 0x0800000, 0x0400000, CRC(6771599d) SHA1(c3476b37b9189c3197194a9027d2f154a12aee37) )
	ROM_LOAD32_WORD("20212211.u72", 0x0800002, 0x0400000, CRC(c4278640) SHA1(bfb5c69191661625427742d24f7f2f63e59217c1) )
ROM_END


ROM_START( corrida )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30178311.u86", 0x0000000, 0x0400000, CRC(19424139) SHA1(8715c995368f5ac410b2ef6f09833b7516df0bd5) )
	ROM_LOAD32_WORD("30178311.u73", 0x0000002, 0x0400000, CRC(6df3c1f5) SHA1(5b8fcbbc650c0fefdf504fd7d48a125d2f7d22e7) )
	ROM_LOAD32_WORD("30178311.u85", 0x0800000, 0x0400000, CRC(451774c0) SHA1(e2fd78433ad9016900581720ccd29bf511a57f04) )
	ROM_LOAD32_WORD("30178311.u72", 0x0800002, 0x0400000, CRC(37d4eed5) SHA1(55aa5f4dad3193f67e8fea7506dde2230b8c6b9e) )
ROM_END


ROM_START( cryspri )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10144411.u86", 0x0000000, 0x0400000, CRC(5c0e7122) SHA1(f729e20d15f9a8d1442a9f9616cfa09ac181bc3f) )
	ROM_LOAD32_WORD("10144411.u73", 0x0000002, 0x0400000, CRC(e8ab1219) SHA1(88e792209d224fc95b4248fc1aeebcae39b06eb7) )
ROM_END


ROM_START( crysprim )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10155811.u86", 0x0000000, 0x0400000, CRC(b046ea06) SHA1(0c0310bc0afb8bac630ac0570d5b9df6a992cfdb) )
	ROM_LOAD32_WORD("10155811.u73", 0x0000002, 0x0400000, CRC(b52cac8a) SHA1(65bb5d73933df6d53a079e4efe00ea29649e3201) )
ROM_END


ROM_START( cryspriu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0352557.u86", 0x0000000, 0x0400000, CRC(dcb920fc) SHA1(71ce2a4298c9012ef7724b206ab1270b25c8bf8d) )
	ROM_LOAD32_WORD("0352557.u73", 0x0000002, 0x0400000, CRC(caa73d62) SHA1(5e41beb9ade7bd50cdc616ab3f016a6615778ea2) )
	ROM_LOAD32_WORD("0352557.u85", 0x0800000, 0x0400000, CRC(c7c542c1) SHA1(8b49969414ef3d5811c804f16dc78cba71279618) )
	ROM_LOAD32_WORD("0352557.u72", 0x0800002, 0x0400000, CRC(8864632c) SHA1(a3227831ee71bc01846a7400e14fb40e62770e7a) )
ROM_END


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


ROM_START( deepfrzu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0154081.u86", 0x0000000, 0x0400000, CRC(7f98661d) SHA1(970dcba4c2f5b40702c33751c73541a22fbcdd26) )
	ROM_LOAD32_WORD("0154081.u73", 0x0000002, 0x0400000, CRC(80fc6979) SHA1(2a08852c930f4b732830da4761010cc87ef369fc) )
	ROM_LOAD32_WORD("0154081.u85", 0x0800000, 0x0400000, CRC(ef3914cd) SHA1(e189180fb087636b6d07de5df934ed4c89e8477e) )
	ROM_LOAD32_WORD("0154081.u72", 0x0800002, 0x0400000, CRC(bcfc43a6) SHA1(cd3b25005d67dc096b13b1d35eca9701f150e461) )
ROM_END


ROM_START( deltabel )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252532.u86", 0x0000000, 0x0400000, CRC(ee2c3aad) SHA1(c3687c1447c3c324fa23f5f61d8b88139141a8d4) )
	ROM_LOAD32_WORD("0252532.u73", 0x0000002, 0x0400000, CRC(721cb449) SHA1(10be95803f3b7e86a009cc665e847dd97aeb6749) )
	ROM_LOAD32_WORD("0252532.u85", 0x0800000, 0x0400000, CRC(53eec755) SHA1(017cad59b8c25b7b24df869cb5d61867607cebb9) )
	ROM_LOAD32_WORD("0252532.u72", 0x0800002, 0x0400000, CRC(d6f6e2c9) SHA1(590c9b2000bdd888100426017250688bef9d3c1c) )
ROM_END


ROM_START( deltaqn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20185211.u86", 0x0000000, 0x0400000, CRC(93b7ca6b) SHA1(1384638d4da321ec7044f0bf10dfc626c1928215) )
	ROM_LOAD32_WORD("20185211.u73", 0x0000002, 0x0400000, CRC(56628af9) SHA1(778175ce3d0074569e434a9c3995751747352e15) )
	ROM_LOAD32_WORD("20185211.u85", 0x0800000, 0x0400000, CRC(73fe916b) SHA1(c262775bde046b5457ab998017012af12b5c036c) )
	ROM_LOAD32_WORD("20185211.u72", 0x0800002, 0x0400000, CRC(1978abb1) SHA1(7ffd1d81201bcf31936bbc5bd194b182cb9b5cdc) )
ROM_END


ROM_START( dimeye )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251312.u86", 0x0000000, 0x0400000, CRC(dd6cab00) SHA1(27934d1262e91078c0ed3d12510d1157e7d0774f) )
	ROM_LOAD32_WORD("0251312.u73", 0x0000002, 0x0400000, CRC(a0c21ad9) SHA1(689c9fb61fc7864be22b22fbdac295ced28340db) )
	ROM_LOAD32_WORD("0251312.u85", 0x0800000, 0x0400000, CRC(ca138b5c) SHA1(e0af5ac82b40f40af0438e4f6ce5d1e8d984d34d) )
	ROM_LOAD32_WORD("0251312.u72", 0x0800002, 0x0400000, CRC(f5f4d871) SHA1(e1a83ed4932602f460321bb2aaf648ef10711033) )
ROM_END


ROM_START( dimeyece )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152039.u86", 0x0000000, 0x0400000, CRC(926ce073) SHA1(12f0aa1e387f0f87be92afb9a917a30d51a63277) ) /* m27v322 */
	ROM_LOAD32_WORD("0152039.u73", 0x0000002, 0x0400000, CRC(235ba6ca) SHA1(3534b4f74531a42543383b55a2c57853b1d3f4f1) ) /* m27v322 */
	ROM_LOAD32_WORD("0152039.u85", 0x0800000, 0x0400000, CRC(f6ef5d9c) SHA1(94505a470ad65e75c6c984ec163ce92c558f491a) ) /* m27v322 */
	ROM_LOAD32_WORD("0152039.u72", 0x0800002, 0x0400000, CRC(9f6b5923) SHA1(739d3848998ddbfaf577982c12d8edd19c348548) ) /* m27v322 */
	ROM_LOAD32_WORD("0152039.u84", 0x1000000, 0x0400000, CRC(6d0dea65) SHA1(ee23c0fe2ecb5adb5e383757ade9be17880e6b45) ) /* m27v322 */
	ROM_LOAD32_WORD("0152039.u71", 0x1000002, 0x0400000, CRC(5706aa40) SHA1(d847bd0a4e210e3f065aa59129ee4654e3087909) ) /* m27v322 */
ROM_END


ROM_START( di4ever )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10105011.u86", 0x0000000, 0x0400000, CRC(d7c0fd99) SHA1(97d2b17ea89d7dd0186a13f08fe68a7f8e1336ca) )
	ROM_LOAD32_WORD("10105011.u73", 0x0000002, 0x0400000, CRC(39922c60) SHA1(19c36087cae9cf482d3dc86dc40fa94c3687ae96) )
ROM_END


ROM_START( dinosa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10118811.u86", 0x0000000, 0x0400000, CRC(74e08c66) SHA1(002a526e8e693aa68f45ae8485114c8bdc3f2453) )
	ROM_LOAD32_WORD("10118811.u73", 0x0000002, 0x0400000, CRC(17b46b56) SHA1(43d55adcc54855b6845ffedba8998b14df0447f3) )
	ROM_LOAD32_WORD("10118811.u85", 0x0800000, 0x0400000, CRC(cff22e03) SHA1(0fd6632ae9c71bc99a31e9bdc9abdfda09dfafd7) )
	ROM_LOAD32_WORD("10118811.u72", 0x0800002, 0x0400000, CRC(34181cf9) SHA1(93d014891789260fe0a0eeeff17c25c6211753db) )
ROM_END


ROM_START( dblagent )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152082.u86", 0x0000000, 0x0400000, CRC(d081a5f3) SHA1(0adfbdc36b70a26f858d353ee4381e5848d6e499) )
	ROM_LOAD32_WORD("0152082.u73", 0x0000002, 0x0400000, CRC(235e2cc8) SHA1(aaf1e71f4cdeb039e3c760031b07d7eb54467742) )
	ROM_LOAD32_WORD("0152082.u85", 0x0800000, 0x0400000, CRC(4d482f50) SHA1(8105371d582bcc5a7d0e35f110f394fc491550cf) )
	ROM_LOAD32_WORD("0152082.u72", 0x0800002, 0x0400000, CRC(fdb9a838) SHA1(83976af01273234d7eaeedba60164f9c31915e4b) )
ROM_END


ROM_START( dbldolph )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20000211.u86", 0x0000000, 0x0400000, CRC(ab98e4c1) SHA1(95ca99633a11d0087a4a702be8c075f1ad87bed4) )
	ROM_LOAD32_WORD("20000211.u73", 0x0000002, 0x0400000, CRC(493c254b) SHA1(940c5b905c9e5677db5589fb097418d08047897d) )
	ROM_LOAD32_WORD("20000211.u85", 0x0800000, 0x0400000, CRC(7aa49b2d) SHA1(15ae3f0eb6d1e44c5453e557b04f1ab3941163dc) )
	ROM_LOAD32_WORD("20000211.u72", 0x0800002, 0x0400000, CRC(cd886389) SHA1(d0044d88d7cc168ae24e3a61f7e4758317b82417) )
ROM_END


ROM_START( dbldolphq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40003211.u86", 0x0000000, 0x0400000, CRC(73fbb9df) SHA1(3f5e68f24d4d38f0b514acaac0672ca2a40f31f3) )
	ROM_LOAD32_WORD("40003211.u73", 0x0000002, 0x0400000, CRC(e69e12f2) SHA1(176af1d3177da18468673a9a52672c97a278c75a) )
	ROM_LOAD32_WORD("40003211.u85", 0x0800000, 0x0400000, CRC(7df3105d) SHA1(df287fd3f42aff0a0b3bbab5a40723de6170d488) )
	ROM_LOAD32_WORD("40003211.u72", 0x0800002, 0x0400000, CRC(7c366f7f) SHA1(9816f8ef8ae7f1d3083a5094ce27e17df478dc3a) )
ROM_END


ROM_START( drgnlord )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173511.u86", 0x0000000, 0x0400000, CRC(fc699900) SHA1(8e49672e7622ac6bd0106c25f524a7dd9e9578ab) )
	ROM_LOAD32_WORD("10173511.u73", 0x0000002, 0x0400000, CRC(dd887eb0) SHA1(7ac4f86d0cfc69acd4471cc018f8b83d98735b37) )
	ROM_LOAD32_WORD("10173511.u85", 0x0800000, 0x0400000, CRC(d17e1a0d) SHA1(955e4956656e7f08e352628cc632af0e3cb6cf0f) )
	ROM_LOAD32_WORD("10173511.u72", 0x0800002, 0x0400000, CRC(c2b42880) SHA1(7079102d576d8e9c5cb4d3067b02d131b63b8d88) )
ROM_END


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


ROM_START( drmcatch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10126721.u86", 0x0000000, 0x0400000, CRC(82645450) SHA1(17edb9f2b61f38ca6ced060d37d4294b42b934ac) )
	ROM_LOAD32_WORD("10126721.u73", 0x0000002, 0x0400000, CRC(4193c9f8) SHA1(de9747896f299f8562a8e836e28fcce991154194) )
	ROM_LOAD32_WORD("10126721.u85", 0x0800000, 0x0400000, CRC(5b639f5c) SHA1(b3a2f5797071ae57b0bc541f23d6e62df173e4cd) )
	ROM_LOAD32_WORD("10126721.u72", 0x0800002, 0x0400000, CRC(46244616) SHA1(3e16e7dd6f5afba135c3f1b9b40b7cfecc0d83e4) )
ROM_END


ROM_START( drmred )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10273811.u86", 0x0000000, 0x0400000, CRC(69e4a089) SHA1(505de7f61ff74d73da32aaf105b1ff9d5083094a) )
	ROM_LOAD32_WORD("10273811.u73", 0x0000002, 0x0400000, CRC(a38db22b) SHA1(e83485525a46741a7d97c36341c529d9643f2dd9) )
	ROM_LOAD32_WORD("10273811.u85", 0x0800000, 0x0400000, CRC(b2640b58) SHA1(79e8085ae36a2f03a7e57e21cccaeb4bbe122f8e) )
	ROM_LOAD32_WORD("10273811.u72", 0x0800002, 0x0400000, CRC(5a4e369a) SHA1(f50424edae6e912ef6d40f2e2e769c2002349c0a) )
ROM_END


ROM_START( eaglrock )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151126.u86", 0x0000000, 0x0400000, CRC(0b0cff73) SHA1(ea1bbf9e00a5308c9e853e357f9ac4b32866a195) )
	ROM_LOAD32_WORD("0151126.u73", 0x0000002, 0x0400000, CRC(ce3d4fd2) SHA1(ef784c2ae5151c7191cac66093cbc1f85155a4e9) )
	ROM_LOAD32_WORD("0151126.u85", 0x0800000, 0x0400000, CRC(a3f768e9) SHA1(1b0b96368530d51280da81479e9a3b44fb719503) )
	ROM_LOAD32_WORD("0151126.u72", 0x0800002, 0x0400000, CRC(c3ac8cfd) SHA1(ff2b45f49929dfa9aed44af2a1b204a96ed098fb) )
ROM_END


ROM_START( estrike )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40289711.u21", 0x0000000, 0x0400000, CRC(71c8121b) SHA1(1a4534b8e43dbc173809b5a621d6796a7a8655cf) )
	ROM_LOAD32_WORD("40289711.u11", 0x0000002, 0x0400000, CRC(94ff83d2) SHA1(54a02cdfb205a429d7458ecb51ba20083306011e) )
	ROM_LOAD32_WORD("40289711.u22", 0x0800000, 0x0400000, CRC(c7430ac8) SHA1(e6f7e523ef2c4aad584a08ce9c70e0e535d89890) )
	ROM_LOAD32_WORD("40289711.u12", 0x0800002, 0x0400000, CRC(4d96aef2) SHA1(40860ad6a0f82d79347b65b182f72dfbec23cd74) )
	ROM_LOAD32_WORD("40289711.u23", 0x1000000, 0x0400000, CRC(664c3d45) SHA1(26ce8a7e88eddc94ecce5ada78654e6f30a985d6) )
	ROM_LOAD32_WORD("40289711.u13", 0x1000002, 0x0400000, CRC(7311d99c) SHA1(802abed830ca3204ce50072ac69d0a3cae8f17e5) )
	ROM_LOAD32_WORD("40289711.u24", 0x1800000, 0x0400000, CRC(d70198eb) SHA1(1e4c157deec921386623091b27180c9c53551f44) )
	ROM_LOAD32_WORD("40289711.u14", 0x1800002, 0x0400000, CRC(b899e64b) SHA1(1f1d800df39de444b03ff4cc85bf4cf073f88423) )
ROM_END


ROM_START( eastland )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151117.u86", 0x0000000, 0x0400000, CRC(f24bcc2d) SHA1(519d06b85b7c3a4025f029daa394423bc6e3b4a2) )
	ROM_LOAD32_WORD("0151117.u73", 0x0000002, 0x0400000, CRC(99591f11) SHA1(9d08d339ecb19682ace66f58b8aa98b62d958bec) )
	ROM_LOAD32_WORD("0151117.u85", 0x0800000, 0x0400000, CRC(1fb075e7) SHA1(f08f43e19e84b897f0c350a0b86431939947d3b0) )
	ROM_LOAD32_WORD("0151117.u72", 0x0800002, 0x0400000, CRC(16159d21) SHA1(e3b15c84d2a6ef9b840456cecf290798b6e6961d) )
ROM_END


ROM_START( eggsrccm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351149.u86", 0x0000000, 0x0400000, CRC(9d3476d1) SHA1(983ababf1f8d65346c4cf89b7ac817156a64a636) )
	ROM_LOAD32_WORD("0351149.u73", 0x0000002, 0x0400000, CRC(d72f0fb0) SHA1(eedebb24d23899f4caffe7c79ae4ac51856df1b3) )
	ROM_LOAD32_WORD("0351149.u85", 0x0800000, 0x0400000, CRC(2cbd9e98) SHA1(dcd5df5a3c177992d13077fa434ef741e7607d89) )
	ROM_LOAD32_WORD("0351149.u72", 0x0800002, 0x0400000, CRC(883274fd) SHA1(60e65e2f7f1774eac4520f5d96dbcd149ee69b91) )
ROM_END


ROM_START( efce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151084.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0151084.u73", 0x0000002, 0x0400000, CRC(d5c600ba) SHA1(e2eb729e29d66ba5614fce1f4bd5a4883b50c499) )
	ROM_LOAD32_WORD("0151084.u85", 0x0800000, 0x00c0000, CRC(95cf5a7b) SHA1(ac5fbd83e2d73a1d6d55736d868c44396371e24e) ) // bad? this seems to contains all the necessary data: maybe it was padded up to 0x0400000 and the padding was cut?
	ROM_LOAD32_WORD("0151084.u72", 0x0800002, 0x0400000, CRC(f3ec21f4) SHA1(db9d98fe4788f27c33e84b612632feeea6d944c7) )
	ROM_END


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


ROM_START( enchantr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20000711.u86", 0x0000000, 0x0400000, CRC(17a7db59) SHA1(f948369144791d6fd761efbbe5fb37b765b637e3) )
	ROM_LOAD32_WORD("20000711.u73", 0x0000002, 0x0400000, CRC(dcaaec40) SHA1(f035e54d9c926826c7883838a65260fc5ca6c02c) )
ROM_END


ROM_START( eott )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10002011.u86", 0x0000000, 0x0400000, CRC(dae52614) SHA1(4a80d1a20f79b25e0f47c40ba05645d60b598568) )
	ROM_LOAD32_WORD("10002011.u73", 0x0000002, 0x0400000, CRC(63d43c24) SHA1(8f76d2a7f9f5c0eb89609f891255e584b3a417d5) )
	ROM_LOAD32_WORD("10002011.u85", 0x0800000, 0x0400000, CRC(d941636d) SHA1(7103599caa30d57a1fa038afd7c60c684ba9dc5a) )
	ROM_LOAD32_WORD("10002011.u72", 0x0800002, 0x0400000, CRC(0ccf36e4) SHA1(6b185113a909985730cf1a58d1179d0ffc269d40) )
ROM_END


ROM_START( firednc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20184911.u86", 0x0000000, 0x0400000, CRC(be5673c6) SHA1(68a5c6515945c8341479cae3166d8ed0915ec566) )
	ROM_LOAD32_WORD("20184911.u73", 0x0000002, 0x0400000, CRC(30b0f1a0) SHA1(9095f3848514e5a7ab5528fef701a274156d4bce) )
	ROM_LOAD32_WORD("20184911.u85", 0x0800000, 0x0400000, CRC(f05c2ebe) SHA1(7b196523a613955a3f4fec70e04c642bc1184dde) )
	ROM_LOAD32_WORD("20184911.u72", 0x0800002, 0x0400000, CRC(de394e4c) SHA1(9ec430df94de2cc0dbc8cc3a8763fde257fe3477) )
ROM_END


ROM_START( flamolce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351025.u86", 0x0000000, 0x0400000, CRC(f06f3d69) SHA1(83e1d2f72275de836e170c1c10905cb6da58b90a) )
	ROM_LOAD32_WORD("0351025.u73", 0x0000002, 0x0400000, CRC(aad37086) SHA1(76495ba46da0ae848ba290706be337c45afe6361) )
	ROM_LOAD32_WORD("0351025.u85", 0x0800000, 0x0400000, CRC(4fcd025d) SHA1(63aa8d0f5dd331e10c032cc5ac67516054db7c27) )
	ROM_LOAD32_WORD("0351025.u72", 0x0800002, 0x0400000, CRC(b4581c28) SHA1(1daa0f1218dba6e6eb96b7925ea71be531ffa787) )
ROM_END


ROM_START( flamoljc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251048.u86", 0x0000000, 0x0400000, CRC(2ea7c6d9) SHA1(cd887f0689737fd447786b966297ea3994fb8cb7) ) /* m27v322 */
	ROM_LOAD32_WORD("0251048.u73", 0x0000002, 0x0400000, CRC(c8337682) SHA1(e87c5be62528e2e73659ce73a17d1ea21ed3ccf0) ) /* m27v322 */
	ROM_LOAD32_WORD("0251048.u85", 0x0800000, 0x0400000, CRC(2661893a) SHA1(4a79f0b638f5d4fdfa7dcc7661afe3ae1823e6ce) ) /* m27v322 */
	ROM_LOAD32_WORD("0251048.u72", 0x0800002, 0x0400000, CRC(330c1988) SHA1(0d7dae49495c5b164121cd756d6e22af17a9b41b) ) /* m27v322 */
ROM_END


ROM_START( followst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20001411.u86", 0x0000000, 0x0400000, CRC(aa3d76aa) SHA1(94350d0efb49467febd1500dbb697aa865d43843) )
	ROM_LOAD32_WORD("20001411.u73", 0x0000002, 0x0400000, CRC(a1037235) SHA1(3badc901f8d1488cb630b07adc18cc280bd1c111) )
ROM_END


ROM_START( fortath )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10248611.u86", 0x0000000, 0x0400000, CRC(39274d5c) SHA1(ef01c3f010800ece44ccb50be7cf14af538e21fc) )
	ROM_LOAD32_WORD("10248611.u73", 0x0000002, 0x0400000, CRC(4f361938) SHA1(51c23d09ba7cc7c73559e10b023962e7eb7059ed) )
	ROM_LOAD32_WORD("10248611.u85", 0x0800000, 0x0400000, CRC(0b33c40b) SHA1(055ac1528a2008a4b481db201b95dabf8531d6e4) )
	ROM_LOAD32_WORD("10248611.u72", 0x0800002, 0x0400000, CRC(90e481a0) SHA1(b73045e1f8602f17e6d0f9d9f5718c7e7cd0dd81) )
ROM_END


ROM_START( frogwld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251114.u86", 0x0000000, 0x0400000, CRC(feb93395) SHA1(56c0f9b0e3616bc57a564220303c13f60fc72df8) ) /* m27v322 */
	ROM_LOAD32_WORD("0251114.u73", 0x0000002, 0x0400000, CRC(f058f8f3) SHA1(02efb3de7765e917fb6c29cbfdd90845fb3f0a7b) ) /* m27v322 */
	ROM_LOAD32_WORD("0251114.u85", 0x0800000, 0x0400000, CRC(b4165c0f) SHA1(823179a70841f49c8100d8d77620548ceea01550) ) /* m27v322 */
	ROM_LOAD32_WORD("0251114.u72", 0x0800002, 0x0400000, CRC(36f8066f) SHA1(c8a3dc25488d7d578e05bf247b2d3e0959f5c511) ) /* m27v322 */
ROM_END


ROM_START( gambjack )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197211.u86", 0x0000000, 0x0400000, CRC(d55c8c10) SHA1(fb876c0fe04785d01baef1debe5a16f4d2e1587f) )
	ROM_LOAD32_WORD("20197211.u73", 0x0000002, 0x0400000, CRC(80eeb208) SHA1(ad38d08993457a344ed5da9b6335e9a933b11a67) )
ROM_END


ROM_START( gwnn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0452501.u86", 0x0000000, 0x0400000, CRC(2b6407cf) SHA1(ac9242ebd8e669e84664e9aa0f3ec1eb15049a3e) )
	ROM_LOAD32_WORD("0452501.u73", 0x0000002, 0x0400000, CRC(3644eacc) SHA1(115b7a7ddf9ca1c82aae7c8dcf864d3c224e93be) )
	ROM_LOAD32_WORD("0452501.u85", 0x0800000, 0x0400000, CRC(3b95fa30) SHA1(09e53a4434950212346f447315b41da9216a2e24) )
	ROM_LOAD32_WORD("0452501.u72", 0x0800002, 0x0400000, CRC(68b68c92) SHA1(3c23e02ba9cc31352df19b3f641191a4e6261e7b) )
ROM_END


ROM_START( geishaa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000811.u86", 0x0000000, 0x0400000, CRC(994e2e17) SHA1(a8b40aeb98f4e80a3eb412927f23b4292a7dbf6e) )
	ROM_LOAD32_WORD("10000811.u73", 0x0000002, 0x0400000, CRC(544c2cd6) SHA1(3904bb39096dd66f3579b7f244e8872fe29f4f72) )
	ROM_LOAD32_WORD("10000811.u85", 0x0800000, 0x0400000, CRC(02c7819e) SHA1(f553adf6975c6fafff61e30605531b91ae5598af) )
	ROM_LOAD32_WORD("10000811.u72", 0x0800002, 0x0400000, CRC(1efb986d) SHA1(34a5964e0eada085ff738ee4853beb75c431aaaa) )
ROM_END


ROM_START( geishajc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351033.u86", 0x0000000, 0x0400000, CRC(620d8a40) SHA1(601a66e59a1db8be84b494eba24148ab50c674d5) )
	ROM_LOAD32_WORD("0351033.u73", 0x0000002, 0x0400000, CRC(173105e7) SHA1(7c6c082782b7a7f677e6edbba23a09ae9a922415) )
	ROM_LOAD32_WORD("0351033.u85", 0x0800000, 0x0400000, CRC(879f061c) SHA1(e8f2650a486799880e7261f3d4e3d0f6f65f3dfe) )
	ROM_LOAD32_WORD("0351033.u72", 0x0800002, 0x0400000, CRC(7ab7b3f0) SHA1(6907876ff3597e2476d105939b09e7ce88eca2a8) )
ROM_END


ROM_START( goape )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10006211.u86", 0x0000000, 0x0400000, CRC(392ed11e) SHA1(4fef477254b75c37a282a77aaf3046855118bdc0) )
	ROM_LOAD32_WORD("10006211.u73", 0x0000002, 0x0400000, CRC(d579d719) SHA1(e3679657ed2f27b52a762a7f424714e5080a6957) )
ROM_END


ROM_START( gogreen )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10003411.u86", 0x0000000, 0x0400000, CRC(17f801b4) SHA1(c2044c445694fb0755add29e4a4a0ccb9bf36442) )
	ROM_LOAD32_WORD("10003411.u73", 0x0000002, 0x0400000, CRC(279da15c) SHA1(967063011447aeb4563878c13da3cf60f3f8b456) )
	ROM_LOAD32_WORD("10003411.u85", 0x0800000, 0x0400000, CRC(57f68a9a) SHA1(8e7075e2b532a0c5ad0fd9d9f815a764cb9bb842) )
	ROM_LOAD32_WORD("10003411.u72", 0x0800002, 0x0400000, CRC(e138958c) SHA1(4416c4e00e388e8c84f9d82949c1875a5b34655f) )
ROM_END


ROM_START( goldarch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10197411.u86", 0x0000000, 0x0400000, CRC(00858fad) SHA1(8467ed9898d2234ba48e9896b8dd20afa75c5f69) )
	ROM_LOAD32_WORD("10197411.u73", 0x0000002, 0x0400000, CRC(edaaadf1) SHA1(3d68ba211c1904e7ea35d679c9f8845ef1e4ea90) )
	ROM_LOAD32_WORD("10197411.u85", 0x0800000, 0x0400000, CRC(dc2c1319) SHA1(8df96bf7b96a3c7df0a155420959d939daedf5c4) )
	ROM_LOAD32_WORD("10197411.u72", 0x0800002, 0x0400000, CRC(f6d262c6) SHA1(09e5f1e3838921482b01005e8781e83f08a9bd75) )
ROM_END


ROM_START( gldnflc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10158311.u86", 0x0000000, 0x0400000, CRC(c3d4d641) SHA1(b46d82af862c96068ddd4f530b2229634161b209) )
	ROM_LOAD32_WORD("10158311.u73", 0x0000002, 0x0400000, CRC(35e72b56) SHA1(c6013aa3b6ea243f057aedab67feeed16ee39537) )
	ROM_LOAD32_WORD("10158311.u85", 0x0800000, 0x0400000, CRC(f17c5c40) SHA1(32225a11d529c0312b27ed838badebed7d96469b) )
	ROM_LOAD32_WORD("10158311.u72", 0x0800002, 0x0400000, CRC(521e2429) SHA1(0cfadf64341574eaec6fc89ecccfa2f02c583dd1) )
ROM_END


ROM_START( gldgong )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152558.u86", 0x0000000, 0x0400000, CRC(9e825d94) SHA1(642edc067c651b127890324fcbd947124fd9a07e) )
	ROM_LOAD32_WORD("0152558.u73", 0x0000002, 0x0400000, CRC(de1a91f6) SHA1(6bc640cd5b2ffebed7ef4b43c980f10eb22ec5ba) )
	ROM_LOAD32_WORD("0152558.u85", 0x0800000, 0x0400000, CRC(29557de6) SHA1(1e7f03ca41d510561e22b944a46b98d00abfd29c) )
	ROM_LOAD32_WORD("0152558.u72", 0x0800002, 0x0400000, CRC(86ca7b2b) SHA1(938dcbc846d0f87e374d6c835ac6cb8185faf48c) )
ROM_END


ROM_START( gldgongq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10217411.u86", 0x0000000, 0x0400000, CRC(e2d0c057) SHA1(9311faeb430eaae41b6c42711cf87e3824a88933) )
	ROM_LOAD32_WORD("10217411.u73", 0x0000002, 0x0400000, CRC(4be87efb) SHA1(6775853e14f771f784ac05d1a63534cd1aedca21) )
ROM_END


ROM_START( goldinc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10174011.u86", 0x0000000, 0x0400000, CRC(c4f488f9) SHA1(0db96a71f361f80f8e7265d1873728fc3013c204) )
	ROM_LOAD32_WORD("sp__10174011.u73", 0x0000002, 0x0400000, CRC(7a88f44f) SHA1(586cbcf25eedcae538aaa3c3f8c7700778180295) )
	ROM_LOAD32_WORD("sp__10174011.u85", 0x0800000, 0x0400000, CRC(dde914b2) SHA1(f35b761dd89796f62fec40d48c0223f585247de0) )
	ROM_LOAD32_WORD("sp__10174011.u72", 0x0800002, 0x0400000, CRC(8629d154) SHA1(bb4e679188ec8f58e8e227804c36d1e61e575efe) )
ROM_END


ROM_START( goldincu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252533.u86", 0x0000000, 0x0400000, CRC(5e53d9af) SHA1(e850e1707be120b9d6a23524c65e63efb20d1cf1) )
	ROM_LOAD32_WORD("0252533.u73", 0x0000002, 0x0400000, CRC(768cecbb) SHA1(2cd2b5906c0a45aecaca2d26063b2ce62ca588eb) )
	ROM_LOAD32_WORD("0252533.u85", 0x0800000, 0x0400000, CRC(cb1268de) SHA1(dc5a18736ac76c59811c7bccd7c3a4cd468fccec) )
	ROM_LOAD32_WORD("0252533.u72", 0x0800002, 0x0400000, CRC(ce66ccd2) SHA1(71f204ecfd0e31b071cde59bbd974e489ee1f570) )
ROM_END


ROM_START( goldpym )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130811.u86", 0x0000000, 0x0400000, CRC(57b5488a) SHA1(d09915be7dee48570e79436eff55cabad58b783b) )
	ROM_LOAD32_WORD("10130811.u73", 0x0000002, 0x0400000, CRC(f20e603e) SHA1(5577934713be6996057a2fe62c8988635c842b76) )
	ROM_LOAD32_WORD("10130811.u85", 0x0800000, 0x0400000, CRC(f2e1763f) SHA1(86d8d96c016bf88c8db9eae3de1bd9c64f517189) )
	ROM_LOAD32_WORD("10130811.u72", 0x0800002, 0x0400000, CRC(be3c43ba) SHA1(23ac49737061690a814e9a54a1f32290621bdaaf) )
ROM_END


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


ROM_START( gbmoney )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351016.u86", 0x0000000, 0x0400000, CRC(04ce6b76) SHA1(b846752a739ae6c657e74bbe5e5e772a6c3d5e00) )
	ROM_LOAD32_WORD("0351016.u73", 0x0000002, 0x0400000, CRC(7bccca48) SHA1(b7a4f00eac4b91ec671555f1332f705acffd7b00) )
	ROM_LOAD32_WORD("0351016.u85", 0x0800000, 0x0400000, CRC(28ca3d73) SHA1(3d9edfffe2d7fd37e0f9c6cd292ca3c68abae15b) )
	ROM_LOAD32_WORD("0351016.u72", 0x0800002, 0x0400000, CRC(7703165a) SHA1(6d9e019796dee04a9817c4176af5bf10f7a7597d) )
ROM_END


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


ROM_START( heartgld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20169511.u86", 0x0000000, 0x0400000, CRC(c2f2111a) SHA1(9c62dcac8fe03afbb7475a0d6e89ff01c6795c6f) )
	ROM_LOAD32_WORD("20169511.u73", 0x0000002, 0x0400000, CRC(ee851155) SHA1(885928fccf3368911717b7ca5352ecae862c7ebc) )
	ROM_LOAD32_WORD("20169511.u85", 0x0800000, 0x0400000, CRC(af0db2a6) SHA1(aec212c72d8588314cdb2b558f29524b0c142bbc) )
	ROM_LOAD32_WORD("20169511.u72", 0x0800002, 0x0400000, CRC(041cf141) SHA1(793a1c1ca44f0d4c83294456f953dc4e7fd3b134) )
ROM_END


ROM_START( heartveg )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10232911.u86", 0x0000000, 0x0400000, CRC(0ef4e3cb) SHA1(0cb2c296d7ac39e0e3fe590040c232e35d1508d4) )
	ROM_LOAD32_WORD("10232911.u73", 0x0000002, 0x0400000, CRC(e33aeb25) SHA1(32c3e914fff060404ebf3fbb7a502c9081b57862) )
ROM_END


ROM_START( hlntroy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008311.u86", 0x0000000, 0x0400000, CRC(dac1d540) SHA1(8a57545604beab1f0872f6da98a3f9f5a4079ba0) )
	ROM_LOAD32_WORD("1j008311.u73", 0x0000002, 0x0400000, CRC(7e76bb49) SHA1(ae67b4fb2af4bddf2e751c7c22ac6dc416410e33) )
	ROM_LOAD32_WORD("1j008311.u85", 0x0800000, 0x0400000, CRC(21e8c508) SHA1(930843811b35da27afc8e6561046a249aa10017b) )
	ROM_LOAD32_WORD("1j008311.u72", 0x0800002, 0x0400000, CRC(74045490) SHA1(364ba8a7911f63f529ef5ff0e5bdd17f7bd5a8c1) )
ROM_END


ROM_START( hlntroyu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151336.u86", 0x0000000, 0x0400000, CRC(7100841d) SHA1(67fa5616ca0bb2b25ed2951d3decf0126e0bac82) )
	ROM_LOAD32_WORD("0151336.u73", 0x0000002, 0x0400000, CRC(aebf8e8e) SHA1(22b28883c4ea365c07b6c4ae2fe3c873be158685) )
	ROM_LOAD32_WORD("0151336.u85", 0x0800000, 0x0400000, CRC(4c82ed24) SHA1(455ac96a92b383cb7e2391e92e528c3b4e51f5cc) )
	ROM_LOAD32_WORD("0151336.u72", 0x0800002, 0x0400000, CRC(b694d927) SHA1(0151f3980a401d646a18b9956d3d247daaa48bf2) )
ROM_END


ROM_START( highnoon )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20004111.u86", 0x0000000, 0x0400000, CRC(c4f106c9) SHA1(38cde08bcdc8e053e8ea0166989a6ef40df139a3) )
	ROM_LOAD32_WORD("20004111.u73", 0x0000002, 0x0400000, CRC(a7b68492) SHA1(745220b963f9c5755ad078de9ccbca3c922d40ed) )
	ROM_LOAD32_WORD("20004111.u85", 0x0800000, 0x0400000, CRC(cde132fd) SHA1(157e1d7fde6c57e5bc4ace03e43734d30141ad09) )
	ROM_LOAD32_WORD("20004111.u72", 0x0800002, 0x0400000, CRC(fa35cdef) SHA1(060af506fee9234fb4921b788368e3e850b681c9) )
ROM_END


ROM_START( holdrma )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10100511.u86", 0x0000000, 0x0400000, CRC(9da0445f) SHA1(65e8d5f2563a5aa90dcba8b854912fe835294cb3) )
	ROM_LOAD32_WORD("10100511.u73", 0x0000002, 0x0400000, CRC(04fec2d4) SHA1(56beb6107c56674dc914f0c988311375dd281081) )
	ROM_LOAD32_WORD("10100511.u85", 0x0800000, 0x0400000, CRC(daae9640) SHA1(3d8aa7b4d55d1c3d8196be26bb89b83785666df1) )
	ROM_LOAD32_WORD("10100511.u72", 0x0800002, 0x0400000, CRC(197035d3) SHA1(18da7e44107e8e0df584bd866a0083cf85ecceee) )
ROM_END


ROM_START( househrt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10200311.u86", 0x0000000, 0x0400000, CRC(6cdc5816) SHA1(7157a12baa98ba810f5dcc5d5816a3380748032b) )
	ROM_LOAD32_WORD("10200311.u73", 0x0000002, 0x0400000, CRC(ad7c55fc) SHA1(873abc95c8118b5ca73779445023e8e0299498a6) )
ROM_END


ROM_START( incachf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20190711.u86", 0x0000000, 0x0400000, CRC(f9fa7d96) SHA1(a247c2694c0f0f78cbc431dca4d9385ab991ae80) )
	ROM_LOAD32_WORD("20190711.u73", 0x0000002, 0x0400000, CRC(45208b71) SHA1(e3ce2e9f5bd467872380ad387e77447831a0e67e) )
	ROM_LOAD32_WORD("20190711.u85", 0x0800000, 0x0400000, CRC(46816847) SHA1(aa65826fe1282b15fbdd8abe81e03f0d11d4f18f) )
	ROM_LOAD32_WORD("20190711.u72", 0x0800002, 0x0400000, CRC(95d1dfd2) SHA1(1faead5e4429e4e8a6fd53c5b16bd3fac9483c28) )
ROM_END


ROM_START( indream )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10004211.u86", 0x0000000, 0x0400000, CRC(bea8f843) SHA1(e8f345c2ff5f751e64bb945827826e481d376377) )
	ROM_LOAD32_WORD("10004211.u73", 0x0000002, 0x0400000, CRC(41279ab8) SHA1(3f275669036ef07d7a0b6ebb1ea58ca5fde963e0) )
	ROM_LOAD32_WORD("10004211.u85", 0x0800000, 0x0400000, CRC(ec18c9ea) SHA1(e3150499e7ad4a2e63aa3c070089c8267eefa5af) )
	ROM_LOAD32_WORD("10004211.u72", 0x0800002, 0x0400000, CRC(92c75004) SHA1(6baa35896dd51c17ab5818919616a217b8b2e4dd) )
ROM_END


ROM_START( indremce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20109411.u86", 0x0000000, 0x0400000, CRC(18ae6a0a) SHA1(0401b86c7053129eb7fbd27c3e5988481df67481) )
	ROM_LOAD32_WORD("20109411.u73", 0x0000002, 0x0400000, CRC(a0a99966) SHA1(6fc4e3496e7791e818cbda6d95355467ceb28e6c) )
	ROM_LOAD32_WORD("20109411.u85", 0x0800000, 0x0400000, CRC(b8b51dea) SHA1(04e6a261ab4912c7204b4a276147e1e219e02067) )
	ROM_LOAD32_WORD("20109411.u72", 0x0800002, 0x0400000, CRC(4269940b) SHA1(8f5f6d751360e0abe2a6a80551f56176c46249e5) )
ROM_END


ROM_START( indremll )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20151911.u86", 0x0000000, 0x0400000, CRC(81144634) SHA1(3352bc272da29706d3913e65e0eccfd597628239) )
	ROM_LOAD32_WORD("20151911.u73", 0x0000002, 0x0400000, CRC(ae6590eb) SHA1(d26d4c1612a466a4d67ca51cf6df2c38da88cd0c) )
	ROM_LOAD32_WORD("20151911.u85", 0x0800000, 0x0400000, CRC(d5b64886) SHA1(07b2e2c35e1a71d5c6b5687452f3debee1128632) )
	ROM_LOAD32_WORD("20151911.u72", 0x0800002, 0x0400000, CRC(ebf17295) SHA1(f057ee03fc51041a4aa4413f3cb7df7483b4dd49) )
	ROM_LOAD32_WORD("20151911.u84", 0x1000000, 0x0400000, CRC(14ca706b) SHA1(552b8c864f8b68c48688e70e55424ceb34de4757) )
	ROM_LOAD32_WORD("20151911.u71", 0x1000002, 0x0400000, CRC(81aecf10) SHA1(ca9236003e12dbe897dd942696dbac5cf059e8f1) )
ROM_END


ROM_START( indrem2c )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10150611.u86", 0x0000000, 0x0400000, CRC(23a13807) SHA1(5e0a403c3d227ef8ba49e2bd5f0a4f77acd42518) )
	ROM_LOAD32_WORD("10150611.u73", 0x0000002, 0x0400000, CRC(a23e7fcd) SHA1(d47751a88c7609118d0acbeabbcb812d195e3d48) )
	ROM_LOAD32_WORD("10150611.u85", 0x0800000, 0x0400000, CRC(d49a6d8b) SHA1(55ebb5b098565cea8305ad9998eaee9a2b84cdbd) )
	ROM_LOAD32_WORD("10150611.u72", 0x0800002, 0x0400000, CRC(7181dc26) SHA1(63b6ae4c3e7464d75762224a8c30348c6c6f0e98) )
ROM_END


ROM_START( indremmm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130711.u86", 0x0000000, 0x0400000, CRC(db13eaf5) SHA1(c2e743b72c2a280266d55642e40c3a7a740052db) )
	ROM_LOAD32_WORD("10130711.u73", 0x0000002, 0x0400000, CRC(a5e3dca5) SHA1(e585841064dc98398169bcd0cd04269bbcfaf77c) )
	ROM_LOAD32_WORD("10130711.u85", 0x0800000, 0x0400000, CRC(988f10da) SHA1(9c21cb8ebebcd603b25329331de89e9aaa36368a) )
	ROM_LOAD32_WORD("10130711.u72", 0x0800002, 0x0400000, CRC(efe15e58) SHA1(8ae7b1436664189a5d07f523d41611bca6d1936c) )
ROM_END


ROM_START( indgldcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102511.u86", 0x0000000, 0x0400000, CRC(9340f843) SHA1(e3cf3ea427535aab8027f7d2f6c6e496674d694f) )
	ROM_LOAD32_WORD("10102511.u73", 0x0000002, 0x0400000, CRC(318a4b3e) SHA1(ba16fb0a82539f01b5b8902f510b0acdfe0fdf6d) )
	ROM_LOAD32_WORD("10102511.u85", 0x0800000, 0x0400000, CRC(6b888235) SHA1(6bea4e0c547d04a25202969d6216beacd8e63b23) )
	ROM_LOAD32_WORD("10102511.u72", 0x0800002, 0x0400000, CRC(e814271e) SHA1(9aaf9a463fd08a9cdb48dcb60cb0242f3df1b759) )
ROM_END


ROM_START( indprncs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10174111.u86", 0x0000000, 0x0400000, CRC(dfad47ba) SHA1(807c0e0e4b0cccb8e4d2d34f26be26973745ca1b) )
	ROM_LOAD32_WORD("10174111.u73", 0x0000002, 0x0400000, CRC(c128c9a5) SHA1(bed6f0eba6da62a3981e5c9b6ada5209d37613e5) )
	ROM_LOAD32_WORD("10174111.u85", 0x0800000, 0x0400000, CRC(829f492e) SHA1(36abc1b1beb01814a4e5fc68f1cd424ae979ced7) )
	ROM_LOAD32_WORD("10174111.u72", 0x0800002, 0x0400000, CRC(785493ea) SHA1(e56ee84783a6033ea327a2035df58cb8deca150f) )
ROM_END


ROM_START( jailbrk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10125611.u86", 0x0000000, 0x0400000, CRC(3c6d628c) SHA1(d60ad4b04545ccaf6212a47796d16d97173d530b) )
	ROM_LOAD32_WORD("10125611.u73", 0x0000002, 0x0400000, CRC(5142c5e0) SHA1(dde1f300292b0b8250aeab1d686fb36184daabd7) )
	ROM_LOAD32_WORD("10125611.u85", 0x0800000, 0x0400000, CRC(705ddca8) SHA1(e0f8ca2e323f1dd040ed0cd4f974e419e716a0c6) )
	ROM_LOAD32_WORD("10125611.u72", 0x0800002, 0x0400000, CRC(5185ad7f) SHA1(34517ded04bf51be348200bc09e848ab74ccbeef) )
ROM_END


ROM_START( jefffox )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351068.u86", 0x0000000, 0x0400000, CRC(6b0a7eb3) SHA1(cd99fed52643afa596bfbf4a18deca42043520ef) ) /* m27v322 */
	ROM_LOAD32_WORD("0351068.u73", 0x0000002, 0x0400000, CRC(4b778a84) SHA1(d11b2aefcaa2ac22fea80faab524f82511b6344a) ) /* m27v322 */
	ROM_LOAD32_WORD("0351068.u85", 0x0800000, 0x0400000, CRC(52d5395e) SHA1(57b854e516881bc208b48ed82fdef25059e2efab) ) /* m27v322 */
	ROM_LOAD32_WORD("0351068.u72", 0x0800002, 0x0400000, CRC(ff2ff01d) SHA1(c0d17b3d9907d3876c7a1dbc19b023afcffb2bb1) ) /* m27v322 */
ROM_END


ROM_START( jewelcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151051.u86", 0x0000000, 0x0400000, CRC(68a438be) SHA1(c62fae7d0f4909ba5d3b72fc7f903a49cc25c168) )
	ROM_LOAD32_WORD("0151051.u73", 0x0000002, 0x0400000, CRC(3517c88f) SHA1(fd02618f6f4553ceda17780c7d1c5cf7a2ec44a6) )
	ROM_LOAD32_WORD("0151051.u85", 0x0800000, 0x0400000, CRC(9194156f) SHA1(b030432ae7da6b36af85a1ddd43a5657542989ad) )
	ROM_LOAD32_WORD("0151051.u72", 0x0800002, 0x0400000, CRC(6c168e3b) SHA1(db251f024881e27615049f94c6e11002e89b2c40) )
ROM_END


ROM_START( kingnep )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20189111.u86", 0x0000000, 0x0400000, CRC(750597c6) SHA1(06d3ba55b313da8973fd2b34ba888920519c51c6) )
	ROM_LOAD32_WORD("20189111.u73", 0x0000002, 0x0400000, CRC(1e815d36) SHA1(b29f5e812f52317b060feac8ee91c08a545db7f4) )
	ROM_LOAD32_WORD("20189111.u85", 0x0800000, 0x0400000, CRC(df89ae50) SHA1(fa9a5c795266bcafea0062aae34a2b7432550b53) )
	ROM_LOAD32_WORD("20189111.u72", 0x0800002, 0x0400000, CRC(4e3006a2) SHA1(6d566d2120cb612f647223e61347a1784cb69a95) )
ROM_END


ROM_START( kingnile )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30106331.u86", 0x0000000, 0x0400000, CRC(c3830908) SHA1(438fc339f8b71668e816b5909195cae4158f39c2) )
	ROM_LOAD32_WORD("30106331.u73", 0x0000002, 0x0400000, CRC(9a90354d) SHA1(5c5b0822e6a842f85e2acca0e1c8bfe0c3f68496) )
	ROM_LOAD32_WORD("30106331.u85", 0x0800000, 0x0400000, CRC(524c786e) SHA1(7e40266be5496baefd74796ce66857aec2d21437) )
	ROM_LOAD32_WORD("30106331.u72", 0x0800002, 0x0400000, CRC(ea5d2450) SHA1(fdcf6c532231e9abf3a1c035bdef9d0db7fcf148) )
ROM_END


ROM_START( kingnilea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20106321.u72", 0x0800002, 0x0400000, CRC(2600d9da) SHA1(2c78b987b8a8641b7eedda4880bd3f5846f11224) )
	ROM_LOAD32_WORD("20106321.u73", 0x0000002, 0x0400000, CRC(2875a911) SHA1(3e881c8c736f297778836c128f66150628038e49) )
	ROM_LOAD32_WORD("20106321.u85", 0x0800000, 0x0400000, CRC(a981510c) SHA1(5c018eb101193b1f4864ccd8c30fe328713c4c97) )
	ROM_LOAD32_WORD("20106321.u86", 0x0000000, 0x0400000, CRC(c4b7a58e) SHA1(f0877c9e0a285808c8117d09a33dec2d2caf7b5b) )
ROM_END


ROM_START( kingpeng )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20109021.u86", 0x0000000, 0x0400000, CRC(40c08336) SHA1(c8568c176c09239823b08c494e2e020e564c2b43) )
	ROM_LOAD32_WORD("20109021.u73", 0x0000002, 0x0400000, CRC(c51ef018) SHA1(f89ed9042c12e40350284236ca77dcaede9e38d0) )
	ROM_LOAD32_WORD("20109021.u85", 0x0800000, 0x0400000, CRC(417152bd) SHA1(4cdd6ba592bf7546226c304ade46b90774ba0875) )
	ROM_LOAD32_WORD("20109021.u72", 0x0800002, 0x0400000, CRC(06b7ce2e) SHA1(fc30a247ed2e5ac7586c4c6cba6b8cd7d8206d01) )
ROM_END


ROM_START( kingpengsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40109021.u86", 0x0000000, 0x0400000, CRC(077635b9) SHA1(3d17892df4a068701be1ac649f448e10b8c52ebd) )
	ROM_LOAD32_WORD("40109021.u73", 0x0000002, 0x0400000, CRC(ac8e7dc1) SHA1(b317107cda37c6148e84237fdc76f2c2abf4d743) )
	ROM_LOAD32_WORD("40109021.u85", 0x0800000, 0x0400000, CRC(f81ac65b) SHA1(0833960ce8b1e883c033c92a8121cc89bac6f64c) )
	ROM_LOAD32_WORD("40109021.u72", 0x0800002, 0x0400000, CRC(44854b6b) SHA1(eae87ea7a6d4ade8de5ea7a3614d7e3183ec0af3) )
ROM_END


ROM_START( koalama6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30001011.u86", 0x0000000, 0x0400000, CRC(e405c4b8) SHA1(e838767f0c224a3c96f9dd480559fed01ca7a398) )
	ROM_LOAD32_WORD("30001011.u73", 0x0000002, 0x0400000, CRC(df3d45a0) SHA1(e9cbfeba1271a66c574cfb3bd96e21fd8cdd26f2) )
	ROM_LOAD32_WORD("30001011.u85", 0x0800000, 0x0400000, CRC(1e12c01e) SHA1(21cad879ece470c465907dd970741b13774fb524) )
	ROM_LOAD32_WORD("30001011.u72", 0x0800002, 0x0400000, CRC(84c0f78d) SHA1(3f8e98261bfce1223de538d6bd53fa92d38c4b0d) )
ROM_END


ROM_START( kohinoor )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10136711.u86", 0x0000000, 0x0400000, CRC(e76e81f3) SHA1(1606c08ab6e75ad0cbbdd3f297dbb4fdf855c013) )
	ROM_LOAD32_WORD("10136711.u73", 0x0000002, 0x0400000, CRC(dfdeb535) SHA1(be6b2e84603a5b07e4561012ff9b1d45fbd4f281) )
	ROM_LOAD32_WORD("10136711.u85", 0x0800000, 0x0400000, CRC(cd8324e6) SHA1(b779d1040cd0119b778032d0e165d31d8fafd128) )
	ROM_LOAD32_WORD("10136711.u72", 0x0800002, 0x0400000, CRC(9b6f5a83) SHA1(06707953091719bf545efd2200f81504c5416e74) )
ROM_END


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


ROM_START( lemrol )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151115.u86", 0x0000000, 0x0400000, CRC(18d3513f) SHA1(5d592ce386054627c6f18e87d32317af73866207) )
	ROM_LOAD32_WORD("0151115.u73", 0x0000002, 0x0400000, CRC(57f6a4be) SHA1(c91662e666b8c02b83f61a410da93ad08b4111d2) )
	ROM_LOAD32_WORD("0151115.u85", 0x0800000, 0x0400000, CRC(b8be84bb) SHA1(1c62da796f44f23612ba605ade1835089edac3ab) )
	ROM_LOAD32_WORD("0151115.u72", 0x0800002, 0x0400000, CRC(70a40bb3) SHA1(1c3adcc36d4bb71f0099eb3663b2d67d64ba979b) )
ROM_END


ROM_START( lineking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252534.u86", 0x0000000, 0x0400000, CRC(7f739eb4) SHA1(cccba4551c7f2901b077473f202b1437da433ab4) )
	ROM_LOAD32_WORD("0252534.u73", 0x0000002, 0x0400000, CRC(1b959c89) SHA1(167d628ea3be95cfacad318288bd965823b16ea3) )
	ROM_LOAD32_WORD("0252534.u85", 0x0800000, 0x0400000, CRC(1256b8d1) SHA1(e2ab3d713c41cf00a7b3930678e0f56102fabbca) )
	ROM_LOAD32_WORD("0252534.u72", 0x0800002, 0x0400000, CRC(7440510c) SHA1(e00ee345e8811a7d72f33f30f8dbffee8a550e6a) )
ROM_END


ROM_START( lordleo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30111211.u86", 0x0000000, 0x0400000, CRC(9ac28584) SHA1(cff40762e123d3ace2fb274166eb9d98ef7c7d4c) )
	ROM_LOAD32_WORD("30111211.u73", 0x0000002, 0x0400000, CRC(7666c5f8) SHA1(cd271bc8c1042fa85506691636d22d36df23025b) )
	ROM_LOAD32_WORD("30111211.u85", 0x0800000, 0x0400000, CRC(1a921c0e) SHA1(feb59ec8372723e5babe16145596c3a9fd0938c3) )
	ROM_LOAD32_WORD("30111211.u72", 0x0800002, 0x0400000, CRC(20e3d468) SHA1(155aa4a81f18d714c6dbbe26c5c9e9d6575b858d) )
ROM_END


ROM_START( lougcm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20006911.u86", 0x0000000, 0x0400000, CRC(095161ec) SHA1(4b9ff183d244f70f59eefb2eb6b261e3bcf0c486) )
	ROM_LOAD32_WORD("20006911.u73", 0x0000002, 0x0400000, CRC(3165ca89) SHA1(73d5d32f1b144e25901afa2eeb82e2304f069a55) )
	ROM_LOAD32_WORD("20006911.u85", 0x0800000, 0x0400000, CRC(21d6d1e4) SHA1(d095389ff327940abde30afe8cb2e9f1c1e34226) )
	ROM_LOAD32_WORD("20006911.u72", 0x0800002, 0x0400000, CRC(926aefe5) SHA1(1427cae296864e6dd186614d726e243b46a9aa64) )
ROM_END


ROM_START( lovestuk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152583.u86", 0x0000000, 0x0400000, CRC(c374ee56) SHA1(18b084c32a4dad6dacd3eeaf6ba430053405dbe8) )     /* 27v322 */
	ROM_LOAD32_WORD("0152583.u73", 0x0000002, 0x0400000, BAD_DUMP CRC(421f717d) SHA1(29eaecef6dd389329a8cfdcfa4bf3f90252eb096) )    /* 27v322 */
ROM_END


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


ROM_START( magforst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151139.u86", 0x0000000, 0x0400000, CRC(e412acb3) SHA1(80c5de99b0ef53ccd3eb027aade70caf83753568) )
	ROM_LOAD32_WORD("0151139.u73", 0x0000002, 0x0400000, CRC(f9915826) SHA1(51c8455977b03fc6e2a3e898f225c8eb86ba58df) )
	ROM_LOAD32_WORD("0151139.u85", 0x0800000, 0x0400000, CRC(48b5cb37) SHA1(1b2454349ee29f8779f32830cfc405e97126c4b1) )
	ROM_LOAD32_WORD("0151139.u72", 0x0800002, 0x0400000, CRC(53d881a2) SHA1(f63c500fc0208709813d661305900de2f2edc9bf) )
ROM_END


ROM_START( magmerm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251331.u86", 0x0000000, 0x0400000, CRC(dd44042b) SHA1(130c26a2ca02003adaa729a54195a1e4832cc455) )
	ROM_LOAD32_WORD("0251331.u73", 0x0000002, 0x0400000, CRC(eff3f840) SHA1(16bec01d3aa6b103c10698cd1656ab54ee9de512) )
	ROM_LOAD32_WORD("0251331.u85", 0x0800000, 0x0400000, CRC(0c75309e) SHA1(6b745441434c585e3cda2d854dad3861d7337ab5) )
	ROM_LOAD32_WORD("0251331.u72", 0x0800002, 0x0400000, CRC(7a5399ab) SHA1(85e3ca6e00d4f1259c4899f74981ec076524e50d) )
ROM_END


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


ROM_START( metestrm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152162.u86", 0x0000000, 0x0400000, CRC(a1674822) SHA1(471d704ed2600e0b806123031c9add2b13fa42ce) )
	ROM_LOAD32_WORD("0152162.u73", 0x0000002, 0x0400000, CRC(0bc788f7) SHA1(7ce7cdbd08f7ae73a5a1b7c45e72bb9c90d91d3e) )
	ROM_LOAD32_WORD("0152162.u85", 0x0800000, 0x0400000, CRC(1d24fa72) SHA1(1ff3a912919591dcd48459369e97d62cf27d6614) )
	ROM_LOAD32_WORD("0152162.u72", 0x0800002, 0x0400000, CRC(1d225d8b) SHA1(477b39d9d2dd526dd3fe6fba6c4d36dfc845ebac) )
ROM_END


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


ROM_START( mskitty )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10216611.u86", 0x0000000, 0x0400000, CRC(36c9ef95) SHA1(71c088bd3b258ab0e8949f661f9e9e9f64949f29) )
	ROM_LOAD32_WORD("10216611.u73", 0x0000002, 0x0400000, CRC(40c1bb34) SHA1(0c54ad0fb6a5c4f096be15c1ef08cb4d246b13b6) )
	ROM_LOAD32_WORD("10216611.u85", 0x0800000, 0x0400000, CRC(c369ad11) SHA1(793b1f8d5833b06422196e2abb7eb0a34be8dbef) )
	ROM_LOAD32_WORD("10216611.u72", 0x0800002, 0x0400000, CRC(d85219a1) SHA1(db0c5f21f7af393d11465981e70ac7b1c8f197b3) )
ROM_END


ROM_START( mnytree )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001211.u86", 0x0000000, 0x0400000, CRC(bca13ff6) SHA1(84c3491816aa9a2e9a12c321b0d1144ea14ebdbe) )
	ROM_LOAD32_WORD("10001211.u73", 0x0000002, 0x0400000, CRC(2c9edccc) SHA1(05e7de1fbd13c1a62f7fb610dc3541a94d9a39b7) )
	ROM_LOAD32_WORD("10001211.u85", 0x0800000, 0x0400000, CRC(ea055c36) SHA1(e97c4992150fac9e53618ebf96ac1b2dd4e9cf65) )
	ROM_LOAD32_WORD("10001211.u72", 0x0800002, 0x0400000, CRC(9ecf041c) SHA1(543eb20385bfd79c1cf10cba61c501c495e4d94a) )
ROM_END


ROM_START( moonfire )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30164211.u86", 0x0000000, 0x0400000, CRC(9d60754d) SHA1(ab62e0f92191821b8e76dbaf3bd37303cb94187e) )
	ROM_LOAD32_WORD("30164211.u73", 0x0000002, 0x0400000, CRC(56a5ea19) SHA1(2f89527fe3dfdcab088cd3a7ab51a26816fe14dc) )
	ROM_LOAD32_WORD("30164211.u85", 0x0800000, 0x0400000, CRC(01d1f670) SHA1(efca261e4810f683084baa1cac03304cdd233ef9) )
	ROM_LOAD32_WORD("30164211.u72", 0x0800002, 0x0400000, CRC(21ebecc5) SHA1(dfd69f1ebea56b5cf49d67a437c7016410ea65c4) )
ROM_END


ROM_START( moonwalt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10227611.u86", 0x0000000, 0x0400000, CRC(cb21b264) SHA1(ecc1804c0fd399019e3496ae6ea0a6ea125562f2) )
	ROM_LOAD32_WORD("10227611.u73", 0x0000002, 0x0400000, CRC(8272e787) SHA1(b0b44fa4af0fdd3f66bd117d748ed874379ed131) )
	ROM_LOAD32_WORD("10227611.u85", 0x0800000, 0x0400000, CRC(44f56b6f) SHA1(7706301d9e40e6197aa509c972129b016d60bf34) )
	ROM_LOAD32_WORD("10227611.u72", 0x0800002, 0x0400000, CRC(80e26c8a) SHA1(5f8b01d18210d5997bbf7966d1bf4353dadc4f7d) )
ROM_END


ROM_START( mchilli )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20248711.u86", 0x0000000, 0x0400000, CRC(1f5207d7) SHA1(73c3742a94962a66df7af6eff660f90e7e22e344) )
	ROM_LOAD32_WORD("20248711.u73", 0x0000002, 0x0400000, CRC(da883738) SHA1(74f5cdb7b388dec17a0e4d8a04c2107bd07c3ad7) )
	ROM_LOAD32_WORD("20248711.u85", 0x0800000, 0x0400000, CRC(ac11dab3) SHA1(3a201a5e239b10dcdd23aed3b0d724146f2fcf68) )
	ROM_LOAD32_WORD("20248711.u72", 0x0800002, 0x0400000, CRC(85f98b61) SHA1(1ebd8caef5f163a75e185c3fa3c001517f676890) )
ROM_END


ROM_START( mchilliq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20289311.u86", 0x0000000, 0x0400000, CRC(fe0d1184) SHA1(e973b99b1a87a120183f4f6fc8c8c9bdc18bc10d) )
	ROM_LOAD32_WORD("20289311.u73", 0x0000002, 0x0400000, CRC(8fc3c3ed) SHA1(5afdfddad061a71fc17f4cc6acca65fd490c8b44) )
	ROM_LOAD32_WORD("20289311.u85", 0x0800000, 0x0400000, CRC(53dbf992) SHA1(8bda1f65d98874949c2bf5d0ac5843c80dd44228) )
	ROM_LOAD32_WORD("20289311.u72", 0x0800002, 0x0400000, CRC(29ddc2aa) SHA1(1e514ca460b5d70751846bf90d83934ff55115d4) )
ROM_END


ROM_START( mrwoo )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252599.u86", 0x0000000, 0x0400000, CRC(db5d522a) SHA1(62030c7e314a614a3d4d7386268b61f94bc1fe02) )
	ROM_LOAD32_WORD("0252599.u85", 0x0800000, 0x0400000, CRC(cb26bd37) SHA1(f20d54c5a6750362f47da5cac9b74ae516de16d4) )
	ROM_LOAD32_WORD("0252599.u86", 0x0000000, 0x0400000, CRC(db5d522a) SHA1(62030c7e314a614a3d4d7386268b61f94bc1fe02) )
	ROM_LOAD32_WORD("0252599.u73", 0x0000002, 0x0400000, CRC(59cdb6ee) SHA1(1e8be1396786fced872c8ffc09772183b98a0f96) )
	ROM_LOAD32_WORD("0252599.u85", 0x0800000, 0x0400000, CRC(cb26bd37) SHA1(f20d54c5a6750362f47da5cac9b74ae516de16d4) )
	ROM_LOAD32_WORD("0252599.u72", 0x0800002, 0x0400000, CRC(6fed3a7e) SHA1(b07354d5d927133626232c21096465426b3053bd) )
	ROM_LOAD32_WORD("0252599.u84", 0x1000000, 0x0400000, CRC(0fe46063) SHA1(10f3895b164b3077d156b7eb22e1fb1356816cb3) )
	ROM_LOAD32_WORD("0252599.u71", 0x1000002, 0x0400000, CRC(7507a271) SHA1(3bd25bf8d1ea54a7db2537fd011b12ff51e7bdd1) )
ROM_END


ROM_START( mystarr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151009.u86", 0x0000000, 0x0400000, CRC(8a1475ac) SHA1(c948f0ca991b64d9a07e42fdb346e6df450718f2) ) /* m27v322 */
	ROM_LOAD32_WORD("0151009.u73", 0x0000002, 0x0400000, CRC(7b8ed0d9) SHA1(6ea4bddedef955513e841ee14ec5e6ded23c1ea7) ) /* m27v322 */
	ROM_LOAD32_WORD("0151009.u85", 0x0800000, 0x0400000, CRC(ea82c5db) SHA1(01db2730b1eec5c9f5c011525459ec72903761cc) ) /* m27v322 */
	ROM_LOAD32_WORD("0151009.u72", 0x0800002, 0x0400000, CRC(4aa6e68b) SHA1(5f015885c15fd206053d23983a38dd1dbb8f89ce) ) /* m27v322 */
ROM_END


ROM_START( mysteycm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10008111.u86", 0x0000000, 0x0400000, CRC(3de4876b) SHA1(76eb7bdc41dc134656452f8e0d08f009c02d9205) )
	ROM_LOAD32_WORD("10008111.u73", 0x0000002, 0x0400000, CRC(fd1d56df) SHA1(2f22f83995bdea66a055a1aa3cb0e8dd2c8f87cb) )
	ROM_LOAD32_WORD("10008111.u85", 0x0800000, 0x0400000, CRC(3449d07c) SHA1(2b36edc554f157e465bcaada42d6cbfee545e146) )
	ROM_LOAD32_WORD("10008111.u72", 0x0800002, 0x0400000, CRC(ee58e1e8) SHA1(1aa57c88b97a3bed04bcaabe77f231967d092f4c) )
ROM_END


ROM_START( mysteycmu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251024.u86", 0x0000000, 0x0400000, CRC(a7bdbd03) SHA1(0e763933c082163de85c408be91d216d6545e9d7) ) /* m27v322 */
	ROM_LOAD32_WORD("0251024.u73", 0x0000002, 0x0400000, CRC(85ad1a00) SHA1(d4f1d86647eb7724c94c3cfa89d5545a20323e62) ) /* m27v322 */
	ROM_LOAD32_WORD("0251024.u85", 0x0800000, 0x0400000, CRC(ee781c0f) SHA1(7729a55e1d5ff7b8a373dda8a0798374ff60a89b) ) /* m27v322 */
	ROM_LOAD32_WORD("0251024.u72", 0x0800002, 0x0400000, CRC(380a868e) SHA1(a1ac6ec1a760f8bbf68b3b86cf5dd4623dc130c7) ) /* m27v322 */
ROM_END


ROM_START( mystmerm )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008711.u86", 0x0000000, 0x0400000, CRC(be84df3c) SHA1(b39deff7d8d0ab30e3dbc0f33c4d45329734a05a) )
	ROM_LOAD32_WORD("1j008711.u73", 0x0000002, 0x0400000, CRC(d571a0bc) SHA1(3bb3c9373fcd7c730dfd0de749d7dacb8aadaf53) )
	ROM_LOAD32_WORD("1j008711.u85", 0x0800000, 0x0400000, CRC(803023c2) SHA1(c953b3fa12347a18fa48b5ea691ad61720c2efdd) )
	ROM_LOAD32_WORD("1j008711.u72", 0x0800002, 0x0400000, CRC(a060d4b2) SHA1(5832a7f4b808017cc251a58e9cce004b2ec41e0c) )
ROM_END


ROM_START( nerdwild )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10197311.u86", 0x0000000, 0x0400000, CRC(a14e80d4) SHA1(f1729ff10991056e1880b5c0db3bdb760b1c664b) )
	ROM_LOAD32_WORD("10197311.u73", 0x0000002, 0x0400000, CRC(be64ac6d) SHA1(c0e6f68467ff248e5c619cedde74091783dafecf) )
ROM_END


ROM_START( opsesam )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351309.u86", 0x0000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0351309.u73", 0x0000002, 0x0400000, CRC(d6bc2a4b) SHA1(55daa6c9028f40ecd23c5ffdb049447ce5ce51aa) )
	ROM_LOAD32_WORD("0351309.u85", 0x0800000, 0x0400000, CRC(3bfdcf54) SHA1(4b174a3e65e0c3b874f99a1bc8947ebe35087672) )
	ROM_LOAD32_WORD("0351309.u72", 0x0800002, 0x0400000, CRC(41bf83e5) SHA1(cdd9c6a148970725360c763ed11e2e411c28d5e9) )
ROM_END


ROM_START( orbeaut )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10213511.u86", 0x0000000, 0x0400000, CRC(4d3f0586) SHA1(da38f11e48191fd53492aa5a4875fcdc33d30c24) )
	ROM_LOAD32_WORD("10213511.u73", 0x0000002, 0x0400000, CRC(8c977e22) SHA1(4de3e30995124b89354d660fed97d29574a40533) )
ROM_END


ROM_START( outbackj )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20195211.u21", 0x0000000, 0x0400000, CRC(8eea3cea) SHA1(b2353fc953f6daf03c1458e06006de31fdd7a7c3) )
	ROM_LOAD32_WORD("20195211.u11", 0x0000002, 0x0400000, CRC(972eaba2) SHA1(b927aac04e21922b995aa2587a76ef14c35cd2bd) )
	ROM_LOAD32_WORD("20195211.u22", 0x0800000, 0x0400000, CRC(a1853f27) SHA1(cd2bf8674555f6870f0689ca0adbca6890bc9b27) )
	ROM_LOAD32_WORD("20195211.u12", 0x0800002, 0x0400000, CRC(cdd28848) SHA1(0038bb422a1d650beaa0ffd63b7d6de1936a5e32) )
	ROM_LOAD32_WORD("20195211.u23", 0x1000000, 0x0400000, CRC(0cd41b5d) SHA1(a7891da788fe88d2191b2f0686cb9ca2ecfeeb9c) )
	ROM_LOAD32_WORD("20195211.u13", 0x1000002, 0x0400000, CRC(23f1f50c) SHA1(40145b839c2b5fde5c0f20e08f46f5a859b6e20a) )
	ROM_LOAD32_WORD("20195211.u24", 0x1800000, 0x0400000, CRC(79ca67a1) SHA1(68bae23df71c768dc17631f6ccfc7a720c0959ee) )
	ROM_LOAD32_WORD("20195211.u14", 0x1800002, 0x0400000, CRC(3a0ea3ca) SHA1(88513130013b1b18772a5e538da9a8db3a54633e) )
ROM_END


ROM_START( owlcap )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10008711.u86", 0x0000000, 0x0400000, CRC(66b2dc21) SHA1(1eeff14dd037c8c4c7fc95a04a8dc5946200d453) )
	ROM_LOAD32_WORD("sp__10008711.u73", 0x0000002, 0x0400000, CRC(c9197a1c) SHA1(2b4649c74f624e3414732027ca77536bcaced11f) )
	ROM_LOAD32_WORD("sp__10008711.u85", 0x0800000, 0x0400000, CRC(8de48e1e) SHA1(c52592f808f02039c7f2d9358076e3dd91f152a2) )
	ROM_LOAD32_WORD("sp__10008711.u72", 0x0800002, 0x0400000, CRC(b6be5274) SHA1(28c3a743cfef76c05567cdb7cc4ff5c60c594e1e) )
ROM_END


ROM_START( pandpays )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151109.u86", 0x0000000, 0x0400000, CRC(ba3fe38b) SHA1(11493dff5720e158ca31f72bf59dcbbbe6d87c96) )
	ROM_LOAD32_WORD("0151109.u73", 0x0000002, 0x0400000, CRC(cc63af55) SHA1(0d8d3c6392c3121d5edbeaece880041c3afd6530) )
	ROM_LOAD32_WORD("0151109.u85", 0x0800000, 0x0400000, CRC(ac7672fa) SHA1(8029ff08e29c48e8f4b2ecd03cf021cc89eab916) )
	ROM_LOAD32_WORD("0151109.u72", 0x0800002, 0x0400000, CRC(4c4b9a62) SHA1(1c287ac85346756508e2efb13e1559af4956dd9a) )
ROM_END


ROM_START( parislgt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30002611.u86", 0x0000000, 0x0400000, CRC(89508f23) SHA1(10ab87fc7c726d2d2a6c73151f2ef731b3d79979) )
	ROM_LOAD32_WORD("30002611.u73", 0x0000002, 0x0400000, CRC(05e567a5) SHA1(c605033599f5fd92b20013264c0bf9eb7076ee85) )
	ROM_LOAD32_WORD("30002611.u85", 0x0800000, 0x0400000, CRC(32eb5b34) SHA1(165c61e20ae8d52575fc0402b1cbc49bbb18d2af) )
	ROM_LOAD32_WORD("30002611.u72", 0x0800002, 0x0400000, CRC(d04e5031) SHA1(46ef9fe928409ec031afa97f67b271cb0059c603) )
ROM_END


ROM_START( parislgtu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251031.u86", 0x0000000, 0x0400000, CRC(5224e683) SHA1(8b0dbb2fcea4d1a522160d8a6634687c5bad8acd) ) /* m27v322 */
	ROM_LOAD32_WORD("0251031.u73", 0x0000002, 0x0400000, CRC(d0587548) SHA1(30d0021a78ee79c6f66fb27c8c484427a5a2db1b) ) /* m27v322 */
	ROM_LOAD32_WORD("0251031.u85", 0x0800000, 0x0400000, CRC(e7ed0bf4) SHA1(39cb9744462f10fc791d6c604631c1182da9466b) ) /* m27v322 */
	ROM_LOAD32_WORD("0251031.u72", 0x0800002, 0x0400000, CRC(8dc6a0ec) SHA1(855e8f5c6752bfc4ba25116a20ed82dfb4912c07) ) /* m27v322 */
ROM_END


ROM_START( pcockmag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008811.u86", 0x0000000, 0x0400000, CRC(5348f934) SHA1(82762d643729639a1f059a1b57b74c8ab240277c) )
	ROM_LOAD32_WORD("1j008811.u73", 0x0000002, 0x0400000, CRC(106bd1d4) SHA1(4ecfafc883da969eb1b46c00e8b67aedfdb001bf) )
	ROM_LOAD32_WORD("1j008811.u85", 0x0800000, 0x0400000, CRC(6453532c) SHA1(c9b885e42778af56df89520cb6e4856348958110) )
	ROM_LOAD32_WORD("1j008811.u72", 0x0800002, 0x0400000, CRC(0519288c) SHA1(a6e838d750f17131bf0871eac8742408cb03db6c) )
ROM_END


ROM_START( pcockmagu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151352.u86", 0x0000000, 0x0400000, CRC(53a9a249) SHA1(097e93185efe87edb8e8cf512bd07ed6de120ff2) )
	ROM_LOAD32_WORD("0151352.u73", 0x0000002, 0x0400000, CRC(8d2fac0f) SHA1(35e5ae327d0b6145b88b577ee6ba22589b2cb6dc) )
	ROM_LOAD32_WORD("0151352.u85", 0x0800000, 0x0400000, CRC(7bbc3359) SHA1(b497998475274768b831963f8df2748832772369) )
	ROM_LOAD32_WORD("0151352.u72", 0x0800002, 0x0400000, CRC(4d9912ca) SHA1(8a019b961736b30b076a8a1fdc24cbe48cfb6021) )
ROM_END


ROM_START( pelipete )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10177511.u86", 0x0000000, 0x0400000, CRC(9fb0db2d) SHA1(c4c7f5f91045778dce2229919e00aa1bdb625084) )
	ROM_LOAD32_WORD("10177511.u73", 0x0000002, 0x0400000, CRC(8626fcd5) SHA1(fb08bddf8e30ef537af7f96c632d0690f241cef3) )
	ROM_LOAD32_WORD("10177511.u85", 0x0800000, 0x0400000, CRC(99c2748b) SHA1(712c103be109839d02ace90c1498192792185733) )
	ROM_LOAD32_WORD("10177511.u72", 0x0800002, 0x0400000, CRC(6e7d67d9) SHA1(163449982405d2cd078c757d885113edbcddbd22) )
ROM_END


ROM_START( pelipetejc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10226711.u86", 0x0000000, 0x0400000, CRC(871a5bf4) SHA1(9a73b67c6e90538f889f274c36b9144e918d2d02) )
	ROM_LOAD32_WORD("10226711.u73", 0x0000002, 0x0400000, CRC(e3bd402c) SHA1(6be4b1a2bb9f8b985c4e93c1eb29832f1727ccf5) )
	ROM_LOAD32_WORD("10226711.u85", 0x0800000, 0x0400000, CRC(c5a038e8) SHA1(ecfc2a5d023d9a3a627c399337062743b288531a) )
	ROM_LOAD32_WORD("10226711.u72", 0x0800002, 0x0400000, CRC(58dda04b) SHA1(7c7219b4478d8d392f1b15588aba3cd6febe861e) )
ROM_END


ROM_START( pelipeteu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252555.u86", 0x0000000, 0x0400000, CRC(dea78964) SHA1(857e5f1deb8db3d861ea7fe78e44a8e17cc791aa) )
	ROM_LOAD32_WORD("0252555.u73", 0x0000002, 0x0400000, CRC(44d5e0af) SHA1(1693e641ddc5d4174e4e0dc07cb79e54045aaad3) )
	ROM_LOAD32_WORD("0252555.u85", 0x0800000, 0x0400000, CRC(781e076d) SHA1(0c7d2281598822ea45f37b2afd5f12d9afe2d01a) )
	ROM_LOAD32_WORD("0252555.u72", 0x0800002, 0x0400000, CRC(4ff0a40b) SHA1(b9049a35a3ebc3e1274498809f7687a36cad6f24) )
ROM_END


ROM_START( pennyar )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251169.u86", 0x0000000, 0x0400000, CRC(0fee6e4a) SHA1(b2a7ccbe81528067b29663f18b54520b5fefe6ec) )
	ROM_LOAD32_WORD("0251169.u73", 0x0000002, 0x0400000, CRC(b8b7c31c) SHA1(7a5cfd608d8c0a3db1054b5770065fc3733d5f35) )
	ROM_LOAD32_WORD("0251169.u85", 0x0800000, 0x0400000, CRC(9b9bf428) SHA1(108c722d0b56ae446961eefe27579e203eeb3b19) )
	ROM_LOAD32_WORD("0251169.u72", 0x0800002, 0x0400000, CRC(cdd0795f) SHA1(e5243c27cca69451015f232444b289abf3d95819) )
ROM_END


ROM_START( pengpyce )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151120.u86", 0x0000000, 0x0400000, CRC(b7c646a7) SHA1(e47f88347eb2354dd982f46f7ac89b5645205b12) )
	ROM_LOAD32_WORD("0151120.u73", 0x0000002, 0x0400000, CRC(36699b29) SHA1(0d867f4086c07c8114bb1f383e647dab4d8ce7b8) )
	ROM_LOAD32_WORD("0151120.u85", 0x0800000, 0x0400000, CRC(7e68ca9a) SHA1(17e96fa047b2353be53b52b0a783bb7247bc6a6f) )
	ROM_LOAD32_WORD("0151120.u72", 0x0800002, 0x0400000, CRC(1ee80a47) SHA1(c02485003d0724941ef346fb721bd246f69545b3) )
ROM_END


ROM_START( pharfor )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("40000921.u86", 0x0000000, 0x0400000, CRC(6d435495) SHA1(22644acb9ed28e737bc132698cd12048c76c4b80) )
	ROM_LOAD32_WORD("40000921.u73", 0x0000002, 0x0400000, CRC(33236ae6) SHA1(a9c41af9982c1a7035c5190732775a3eca4ebb78) )
	ROM_LOAD32_WORD("40000921.u85", 0x0800000, 0x0400000, CRC(158af672) SHA1(f653f9d400f03713baa650912c8c0ab33f218fb2) )
	ROM_LOAD32_WORD("40000921.u72", 0x0800002, 0x0400000, CRC(c5d24a99) SHA1(d6d17ab5255d153beacc39880bd58b7b647f532f) )
ROM_END


ROM_START( phxfant )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20197511.u86", 0x0000000, 0x0400000, CRC(499742c6) SHA1(7743b24e658e4ee60255e9bf2b33a4937d19e3b4) )
	ROM_LOAD32_WORD("20197511.u73", 0x0000002, 0x0400000, CRC(1854f81c) SHA1(fc97f1c55db5c66d2261de62a538cc0308988702) )
	ROM_LOAD32_WORD("20197511.u85", 0x0800000, 0x0400000, CRC(db092fc5) SHA1(c2469c131d1240faede3e6e64ee818df4ce9581d) )
	ROM_LOAD32_WORD("20197511.u72", 0x0800002, 0x0400000, CRC(3ea207d5) SHA1(ae9b032f6ab90630bb047fa508d539874bd86d18) )
ROM_END


ROM_START( pirata6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001511.u86", 0x0000000, 0x0400000, CRC(ed435c4a) SHA1(98750a2673411c3b193f25f2998755314bd60331) )
	ROM_LOAD32_WORD("10001511.u73", 0x0000002, 0x0400000, CRC(5923957d) SHA1(62d6effe9f3af0cdc301bbc6541971e91ddc573a) )
ROM_END


/*
    Player's Choice - By Demand
    4-in-1 game containing Big Red, Black Panther, Fa Fa Fa and Super Bucks IV.
*/
ROM_START( pcbydem )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10223111.u21", 0x0000000, 0x0400000, CRC(c4f95ab2) SHA1(dd0a3cdaa7b421cbcfa50b94b8bbfb69769343cd) )
	ROM_LOAD32_WORD("10223111.u11", 0x0000002, 0x0400000, CRC(4a729ca3) SHA1(52be1fbf80a06f9959d62376102d96ea8f77f272) )
	ROM_LOAD32_WORD("10223111.u22", 0x0800000, 0x0400000, CRC(a9b81cc4) SHA1(541765e9fe2d67c60f014a8d32866c7b8203766a) )
	ROM_LOAD32_WORD("10223111.u12", 0x0800002, 0x0400000, CRC(81816747) SHA1(c745bc6be35976f4784730ab8b75e07688e49b22) )
	ROM_LOAD32_WORD("10223111.u23", 0x1000000, 0x0400000, CRC(309dbbf7) SHA1(69e44494bba43d001545c4e474ab4f712d1d8015) )
	ROM_LOAD32_WORD("10223111.u13", 0x1000002, 0x0400000, CRC(c71cb0e3) SHA1(c46d017bb26fb990a403cff7659b15a11f947c19) )
	ROM_LOAD32_WORD("10223111.u24", 0x1800000, 0x0400000, CRC(f3aab3c8) SHA1(a4c08d341f6b59b5faa14aa3381be066ecdc692a) )
	ROM_LOAD32_WORD("10223111.u14", 0x1800002, 0x0400000, CRC(adfaa968) SHA1(a1a8329523cbd14c24f8e6997a5be442327f61c7) )
ROM_END


/*
    Player's Choice - Favourites
    4-in-1 game containing Black Panther, Prize Fight, Return of the Samurai and Super Bucks IV.
*/
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


/*
    Player's Choice - Gold Edition
    4-in-1 game containing Adonis, K.G. Bird, Tigress and Wild Goose.
*/
ROM_START( pcgold )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30291411.u21", 0x0000000, 0x0400000, CRC(bb839c90) SHA1(268008ffeeb867210664928a521051978c7770b3) )
	ROM_LOAD32_WORD("30291411.u11", 0x0000002, 0x0400000, CRC(6072d8d3) SHA1(37ce2a4213024b9ccd1859701c48e76e6c64e6ee) )
	ROM_LOAD32_WORD("30291411.u22", 0x0800000, 0x0400000, CRC(6208432e) SHA1(50006d010dd3d061fef8f4fd8fbbffea1b9b0f3d) )
	ROM_LOAD32_WORD("30291411.u12", 0x0800002, 0x0400000, CRC(db6e9523) SHA1(a4a272af4f5f29d2e612b52a011f9daf7c0b0cd3) )
	ROM_LOAD32_WORD("30291411.u23", 0x1000000, 0x0400000, CRC(5927770a) SHA1(faaec01280a54775e85ae796a7d05170bece3c87) )
	ROM_LOAD32_WORD("30291411.u13", 0x1000002, 0x0400000, CRC(8df4d7da) SHA1(a798424a514201d954519ec0daf0a2b1cbc00082) )
	ROM_LOAD32_WORD("30291411.u24", 0x1800000, 0x0400000, CRC(5709ba3a) SHA1(4fd85b97f434656b3bd0d26ba8573e3cd2c936ce) )
	ROM_LOAD32_WORD("30291411.u14", 0x1800002, 0x0400000, CRC(1d67f455) SHA1(4247d21757522a69184d32434cfa191a026aff19) )
ROM_END


/*
    Player's Choice - Most Wanted
    4-in-1 game containing Big Red, Black Panther, Choy Sun Doa and Spring Carnival.
*/
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


/*
    Player's Choice - Original
    4-in-1 game containing 50 Lions, Indian Dreaming II, Pyramid Power and Queen of the Nile II.
    Indian Dreaming II is an updated edition with 30 credit Reel Power bet similar to Dream Catcher, Thai Princess, Indian Dreaming (Lady Luck) etc.
    Queen of the Nile II has a similar feature to Show Me The Money, where the player can choose different free game/feature multiplier combinations.
*/
ROM_START( pcorig )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30143011.u21", 0x0000000, 0x0400000, CRC(457aa378) SHA1(248e636e33ea68a724ee074febdd7a3ed1b5d4e7) )
	ROM_LOAD32_WORD("30143011.u11", 0x0000002, 0x0400000, CRC(4caa461b) SHA1(b7b5770e9c0f4c7924f071d59eff94eea950b9c8) )
	ROM_LOAD32_WORD("30143011.u22", 0x0800000, 0x0400000, CRC(6f590385) SHA1(fc6229e1a45eaed819ec87fb1777b4ae508200a9) )
	ROM_LOAD32_WORD("30143011.u12", 0x0800002, 0x0400000, CRC(27941b7b) SHA1(5b5dbc8198b70d2965483e95f073213faa819d2e) )
	ROM_LOAD32_WORD("30143011.u23", 0x1000000, 0x0400000, CRC(17445b58) SHA1(f281b36ef9a72cf3e798993a7a80bf7fda1a6e8b) )
	ROM_LOAD32_WORD("30143011.u13", 0x1000002, 0x0400000, CRC(7eb0f277) SHA1(6a0ede9f64556223f284df4d015e200d98ec97fa) )
	ROM_LOAD32_WORD("30143011.u24", 0x1800000, 0x0400000, CRC(d4f04391) SHA1(3eb4b16518b3980505c8ee08542ae184c829f79d) )
	ROM_LOAD32_WORD("30143011.u14", 0x1800002, 0x0400000, CRC(e3b54c33) SHA1(4f1d22353b1114e476340dbec2d7d96e60d55cc2) )
ROM_END


/*
    Player's Choice - Platinum
    4-in-1 game containing Antony & Cleopatra, Miss Kitty, Reelin' N Boppin and Show Me The Money.
    This version of Miss Kitty is not a 50 line game unlike the normal version, it has a standard 5 reel, 3 position layout.
*/
ROM_START( pcplat )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20282111.u21", 0x0000000, 0x0400000, CRC(7ada7860) SHA1(4b8681f8ae3f279e38a9dba3b29585d7d0642f8c) )
	ROM_LOAD32_WORD("20282111.u11", 0x0000002, 0x0400000, CRC(44c0462c) SHA1(e6a99ddef5d48149003fa9009dcb9880c420fedf) )
	ROM_LOAD32_WORD("20282111.u22", 0x0800000, 0x0400000, CRC(51d192c5) SHA1(2b42d0ba8eb5b6ed1006ee452ca11ff295ee0a80) )
	ROM_LOAD32_WORD("20282111.u12", 0x0800002, 0x0400000, CRC(a22f17ed) SHA1(e3e6d50707873986b067a216d6dda25201224c53) )
	ROM_LOAD32_WORD("20282111.u23", 0x1000000, 0x0400000, CRC(545289a0) SHA1(af32941fb6c03c6711fc35c453546803aec0c85b) )
	ROM_LOAD32_WORD("20282111.u13", 0x1000002, 0x0400000, CRC(a66e230f) SHA1(712a08d879902d4ee6d1ee5896d0bbff5961a3f2) )
	ROM_LOAD32_WORD("20282111.u24", 0x1800000, 0x0400000, CRC(944b7425) SHA1(a87072edea72902e12dcda2e3ef7a0b35487b2f0) )
	ROM_LOAD32_WORD("20282111.u14", 0x1800002, 0x0400000, CRC(44ee59d6) SHA1(52dbc89c41e2aef844eaf64e734661c5945980d0) )
ROM_END


/*
    Player's World - Super (?)
    This game is currently unknown, the name string inside the ROM is Multigame Xtreme Hyperlink
    4-in-1 game containing 5 Dragons, Tiki Torch, Turtle Treasure, and Where's The Gold.
*/
ROM_START( pwsuper )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0154019.u21", 0x0000000, 0x0400000, CRC(48f60a2c) SHA1(0d27d544991b3403672250ff79c5bbbf35cdf175) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u11", 0x0000002, 0x0400000, CRC(678b0be2) SHA1(16f5460d67a1e897390e0c03b847511f5c14156a) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u22", 0x0800000, 0x0400000, CRC(5354cff5) SHA1(bf818e53a432aa8e7a2578dbb367be2c5f51bb16) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u12", 0x0800002, 0x0400000, CRC(9dbdefb7) SHA1(445a24233b435e3812747d8376addf969fea5cd2) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u23", 0x1000000, 0x0400000, CRC(43b44f26) SHA1(5410eea0a4911a274a794d6b27e369fb98c6edf7) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u13", 0x1000002, 0x0400000, CRC(ae04211a) SHA1(8db4fc936fa6d0e0a372b81f2e9590047522b843) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u24", 0x1800000, 0x0400000, CRC(a850af7f) SHA1(8d029de0083aad57ddd8b36153214b40cd388d56) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u14", 0x1800002, 0x0400000, CRC(dbb073f1) SHA1(e3a7f14cabab613453baffde3b592889d11578e7) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u25", 0x2000000, 0x0400000, CRC(2c7fe966) SHA1(42fc364dded0ce079ce4970b1f951c6a0ebf6c51) ) /* m27v322 */
	ROM_LOAD32_WORD("0154019.u15", 0x2000002, 0x0400000, CRC(5f9b8ef7) SHA1(f0c954d1f448f2061dbe284c78eddcfcb5afdc7c) ) /* m27v322 */
ROM_END


ROM_START( pompeia6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10002211.u86", 0x0000000, 0x0400000, CRC(0c4ef28a) SHA1(96381853ed23cb914e5e494fc90f671c2143a305) )
	ROM_LOAD32_WORD("10002211.u73", 0x0000002, 0x0400000, CRC(c56b6b9f) SHA1(3f4cb68620a0d993b1812768092b85353d7bc763) )
	ROM_LOAD32_WORD("10002211.u85", 0x0800000, 0x0400000, CRC(7569d484) SHA1(127d92b7bd62543789b25fbfce65e8429ddfc654) )
	ROM_LOAD32_WORD("10002211.u72", 0x0800002, 0x0400000, CRC(e3c02b5e) SHA1(714e83160917e83dafc058a9078ace800c4c3ebf) )
ROM_END


ROM_START( pompeia6u )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151158.u86", 0x0000000, 0x0400000, CRC(a34afbdc) SHA1(47e8ddc95ef9db933b3b6b26bb9d7f3f743d4e46) )
	ROM_LOAD32_WORD("0151158.u73", 0x0000002, 0x0400000, CRC(2a6a92ec) SHA1(98dc8506381089def51b8571038d6ece03d11faa) )
	ROM_LOAD32_WORD("0151158.u85", 0x0800000, 0x0400000, CRC(d71de7f8) SHA1(08692e432bed7e51655e0ebdcfc34b3d8f7f4682) )
	ROM_LOAD32_WORD("0151158.u72", 0x0800002, 0x0400000, CRC(da83f540) SHA1(8126e84ef6b44529632df119a30556f1ecd7c110) )
ROM_END


ROM_START( pompeia6ua )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251010.u86", 0x0000000, 0x0400000, CRC(a68aac55) SHA1(630b1d4cb08d8710b81c6d709182e7c7a52f184a) )
	ROM_LOAD32_WORD("0251010.u73", 0x0000002, 0x0400000, CRC(947d3099) SHA1(d5256fc598c2e280f9e92ef6ba0e0152c8cc5d3b) )
	ROM_LOAD32_WORD("0251010.u85", 0x0800000, 0x0400000, CRC(901f5a17) SHA1(8a7fd00a68770be23e0164907eae91b963f01091) )
	ROM_LOAD32_WORD("0251010.u72", 0x0800002, 0x0400000, CRC(a5959d1f) SHA1(fe92eaa3f1dc6bdc356bef2a680eb9f8a09c88a2) )
ROM_END


ROM_START( prideaf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10208511.u86", 0x0000000, 0x0400000, CRC(efbf0ad5) SHA1(b4d84800c60df820bbe332879189208bac8330ef) )
	ROM_LOAD32_WORD("10208511.u73", 0x0000002, 0x0400000, CRC(86c22e24) SHA1(008fb01a7690981a3bc6a7f4a06dcebaef2a892c) )
ROM_END


ROM_START( prophecy )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20173411.u86", 0x0000000, 0x0400000, CRC(e5950eef) SHA1(930582c34f3639b887d490c96f91814cb5228d8b) )
	ROM_LOAD32_WORD("20173411.u73", 0x0000002, 0x0400000, CRC(683f081f) SHA1(4d625c67cb1d5fa5b025a02e797c4fc0a03893da) )
	ROM_LOAD32_WORD("20173411.u85", 0x0800000, 0x0400000, CRC(046ae548) SHA1(246015be05294806ecfacd626449e2df7e7f569c) )
	ROM_LOAD32_WORD("20173411.u72", 0x0800002, 0x0400000, CRC(194be1d6) SHA1(8f71ff6646a34261cb003fea877b42742eea8c15) )
ROM_END


ROM_START( qnilea6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("3j011111.u86", 0x0000000, 0x0400000, CRC(76818f74) SHA1(baac1a4a8ed3f1fb5a6ca321a6efe6ece7701486) )
	ROM_LOAD32_WORD("3j011111.u73", 0x0000002, 0x0400000, CRC(2f7f783c) SHA1(1739b05628bd02cb1b73c883d9f2e113da2c29e7) )
	ROM_LOAD32_WORD("3j011111.u85", 0x0800000, 0x0400000, CRC(4e53a089) SHA1(2dd060e4ff74f7fe73850a0c3877fa4bacad36c3) )
	ROM_LOAD32_WORD("3j011111.u72", 0x0800002, 0x0400000, CRC(491f4ef2) SHA1(fbaa64d9603900de44642f95fb9f7ee15e138669) )
ROM_END


ROM_START( qnilemt2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10203811.u86", 0x0000000, 0x0400000, CRC(8822085a) SHA1(1cf43815eeab91cb3df18f4b64c6815b449184e3) )
	ROM_LOAD32_WORD("10203811.u73", 0x0000002, 0x0400000, CRC(01bfd58d) SHA1(8ba07b37624469bf802118a7778bb1d52b3649b8) )
	ROM_LOAD32_WORD("10203811.u85", 0x0800000, 0x0400000, CRC(364f2565) SHA1(f6c24c7f59f356c63c8ceda82243a5d99f4f81d8) )
	ROM_LOAD32_WORD("10203811.u72", 0x0800002, 0x0400000, CRC(af15bb42) SHA1(a5f8a7dd037fd6402db983b5cabaa156599ba9f2) )
ROM_END


ROM_START( qnilese )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20102811.u86", 0x0000000, 0x0400000, CRC(b67fbebe) SHA1(a7fff03899caa9dba714928656e7c30e709453c0) )
	ROM_LOAD32_WORD("20102811.u73", 0x0000002, 0x0400000, CRC(d46bf3e8) SHA1(fa0c67ff8b7523bc2813e1df16507cd92cabd721) )
ROM_END


ROM_START( qnilesea )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102811.u86", 0x0000000, 0x0400000, CRC(4c7bdba1) SHA1(796c9da25bbfe389aae5c03b42e15500c91a60d4) )
	ROM_LOAD32_WORD("10102811.u73", 0x0000002, 0x0400000, CRC(27391233) SHA1(e1d546cc9a57db894bdfb24592048578f5828ee5) )
ROM_END


ROM_START( qnilejc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20123911.u86", 0x0000000, 0x0400000, CRC(484fb352) SHA1(cc0c97895731dc3f333131ff5a47794fe1857e47) )
	ROM_LOAD32_WORD("20123911.u73", 0x0000002, 0x0400000, CRC(0ec5d0df) SHA1(206916986b1214a9635c901cbbcda21d21e0338a) )
	ROM_LOAD32_WORD("20123911.u85", 0x0800000, 0x0400000, CRC(a30473fd) SHA1(0af9fcee5df49d20847f98f94e09c0d394c1857a) )
	ROM_LOAD32_WORD("20123911.u72", 0x0800002, 0x0400000, CRC(a1aa9310) SHA1(a00a7dfd57fe8624421e6821c32516f018199d49) )
ROM_END


ROM_START( qnilejcsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__20123911.u86", 0x0000000, 0x0400000, CRC(8c37947b) SHA1(4d3bf1192fb3939565afc482721289eae10eab5f) )
	ROM_LOAD32_WORD("sp__20123911.u73", 0x0000002, 0x0400000, CRC(ea401551) SHA1(939216babba395baf7283601f0873897f382648a) )
	ROM_LOAD32_WORD("sp__20123911.u85", 0x0800000, 0x0400000, CRC(0be660c4) SHA1(1b3da418f907bdee21ea9fb931f1fe4438109217) )
	ROM_LOAD32_WORD("sp__20123911.u72", 0x0800002, 0x0400000, CRC(c508e9d4) SHA1(59cf85729c1936e54b6c7adedf3941c24e299d48) )
ROM_END


ROM_START( qsheba )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10133711.u86", 0x0000000, 0x0400000, CRC(e1313dff) SHA1(536014d8d4f3ff90d93befdcd55d5b3e6eb49e7c) )
	ROM_LOAD32_WORD("10133711.u73", 0x0000002, 0x0400000, CRC(fb26e09a) SHA1(e2debdcea7ee7be90e70c96e3eca593b515dfd2d) )
	ROM_LOAD32_WORD("10133711.u85", 0x0800000, 0x0400000, CRC(6f827f08) SHA1(b3777031c0a8d14557a640572c4dcfa781986b54) )
	ROM_LOAD32_WORD("10133711.u72", 0x0800002, 0x0400000, CRC(befa03be) SHA1(abd4846f29636aff70826b3086a972682c3bffe0) )
ROM_END


ROM_START( raprichs )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10194211.u86", 0x0000000, 0x0400000, CRC(8dfb94ce) SHA1(67933b9edbd9631d2558d16267559af91084172b) )
	ROM_LOAD32_WORD("10194211.u73", 0x0000002, 0x0400000, CRC(cd13d765) SHA1(f91cb232726a038c3ff1a25259622dd99d552279) )
	ROM_LOAD32_WORD("10194211.u85", 0x0800000, 0x0400000, CRC(3fbfdf7f) SHA1(bcfd939d0fa4d0acc2544910940fb09d48a399b5) )
	ROM_LOAD32_WORD("10194211.u72", 0x0800002, 0x0400000, CRC(3b4ee6dd) SHA1(83ddfdc3d8bdf02395411d8a6721a9c65c64e69c) )
ROM_END


ROM_START( redbara6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119911.u86", 0x0000000, 0x0400000, CRC(599a7170) SHA1(8e3b2730b9503baa798fd59d787052a9589e0447) )
	ROM_LOAD32_WORD("10119911.u73", 0x0000002, 0x0400000, CRC(6590c207) SHA1(8184ae0ac02f22787c631bc823bd0fbcd5d5bf05) )
	ROM_LOAD32_WORD("10119911.u85", 0x0800000, 0x0400000, CRC(e954aeac) SHA1(df8a9420df31c4ecc2cbe460cea4c0bd13958289) )
	ROM_LOAD32_WORD("10119911.u72", 0x0800002, 0x0400000, CRC(16b6207f) SHA1(9c5ac48cccba001b2e3605eceb9610c659e06e21) )
ROM_END


ROM_START( redbarfl )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10160711.u86", 0x0000000, 0x0400000, CRC(6deec801) SHA1(7a3a7ee88d116cc3f78ca77b66247c0271130682) )
	ROM_LOAD32_WORD("10160711.u73", 0x0000002, 0x0400000, CRC(8467e3a6) SHA1(4abef9510a3d71166fa0ece928f936171de784b3) )
	ROM_LOAD32_WORD("10160711.u85", 0x0800000, 0x0400000, CRC(5d77ca06) SHA1(22c6cccf6f0ddb9ee95691180299c55b0b8b5747) )
	ROM_LOAD32_WORD("10160711.u72", 0x0800002, 0x0400000, CRC(91993744) SHA1(e8ac54b233eb47c525225318a07f2e58246c5131) )
ROM_END


ROM_START( reelrkfl )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10298611.u86", 0x0000000, 0x0400000, CRC(efb8f415) SHA1(b779fda43bb63d947f3ddad84cf324d530d20728) )
	ROM_LOAD32_WORD("10298611.u73", 0x0000002, 0x0400000, CRC(2eee9ff5) SHA1(0fb78b79df4efb71d1da850b11ca11c92fb47858) )
	ROM_LOAD32_WORD("10298611.u85", 0x0800000, 0x0400000, CRC(1f1c2b5b) SHA1(c2617a8d518f58cb622f3d9859a0f7d1766d359a) )
	ROM_LOAD32_WORD("10298611.u72", 0x0800002, 0x0400000, CRC(e58d6bc4) SHA1(d6cfb78c570f50f65c27c01f804720e499d623b3) )
ROM_END


ROM_START( retsama6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10117211.u86", 0x0000000, 0x0400000, CRC(8c6d706e) SHA1(6e94e223a65757fab79164988a1fdfab4215c284) )
	ROM_LOAD32_WORD("10117211.u73", 0x0000002, 0x0400000, CRC(a2fbf139) SHA1(53d6aa13bcaf0d537384ea406bb3a34fc8c8830c) )
ROM_END


ROM_START( roadtrip )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152539.u86", 0x0000000, 0x0400000, CRC(66c2c8da) SHA1(76d338662a80ebdfd72e403388cffe8dfbd14f59) )
	ROM_LOAD32_WORD("0152539.u73", 0x0000002, 0x0400000, CRC(cb9f6f5d) SHA1(825322c6b0407a934aca632c198615d188e2f46b) )
	ROM_LOAD32_WORD("0152539.u85", 0x0800000, 0x0400000, CRC(e68205a7) SHA1(a7dc2a10de4ef229a8f25a3db19a30ea4be46cdd) )
	ROM_LOAD32_WORD("0152539.u72", 0x0800002, 0x0400000, CRC(421cfc77) SHA1(fb14ad4c06bdbdd6df4fc1299a96bb8d906d2b48) )
	ROM_LOAD32_WORD("0152539.u84", 0x1000000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0152539.u71", 0x1000002, 0x0400000, CRC(1da57ab8) SHA1(775d1f0d6021032e909c8089ba93cc63eef18586) )
ROM_END


ROM_START( ruprup )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10115911.u86", 0x0000000, 0x0400000, CRC(b6d1c4fa) SHA1(24d88882783353598e4f63edda4982f9e55b2595) )
	ROM_LOAD32_WORD("10115911.u73", 0x0000002, 0x0400000, CRC(e60d9c93) SHA1(7504f07f04be97058b50168c14e6ec41480e43ef) )
	ROM_LOAD32_WORD("10115911.u85", 0x0800000, 0x0400000, CRC(7debb885) SHA1(c46e3231d8a13cec7d398a8064ac76e78954dd82) )
	ROM_LOAD32_WORD("10115911.u72", 0x0800002, 0x0400000, CRC(7ba80d87) SHA1(c912ec8c65060060b459de0c5ed0b3ad018eff76) )
ROM_END


ROM_START( rubymag )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10125711.u86", 0x0000000, 0x0400000, CRC(717d1c53) SHA1(8f5204d4394c6e0b83f6f456c80e98e12fac1a66) )
	ROM_LOAD32_WORD("10125711.u73", 0x0000002, 0x0400000, CRC(6699d2c6) SHA1(1c96dba91a40993cf9c3bc9e8c591e971c04bedf) )
	ROM_LOAD32_WORD("10125711.u85", 0x0800000, 0x0400000, CRC(877fde17) SHA1(278b8cb71cafa0859073acc48339da9e645fdcc6) )
	ROM_LOAD32_WORD("10125711.u72", 0x0800002, 0x0400000, CRC(2cdfe857) SHA1(33b4ce8ab0a5f1e341a5c5c536c5f3619ecd7f95) )
ROM_END


ROM_START( rumbreel )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20288611.u21", 0x0000000, 0x0400000, CRC(cc48bb54) SHA1(6ed7a502deaff0e2fd6d0901d036fcbadf303859) )
	ROM_LOAD32_WORD("20288611.u11", 0x0000002, 0x0400000, CRC(06b37c77) SHA1(e4dad62667a1c1d4948abcd535332975148e952b) )
	ROM_LOAD32_WORD("20288611.u22", 0x0800000, 0x0400000, CRC(3af99e7b) SHA1(93e68056fee9b4f29a066bd6828ff320b7d3ffa4) )
	ROM_LOAD32_WORD("20288611.u12", 0x0800002, 0x0400000, CRC(739c4565) SHA1(bc2cd133dfbfe18916703eacc851f1757a06f345) )
	ROM_LOAD32_WORD("20288611.u23", 0x1000000, 0x0400000, CRC(04b2f09d) SHA1(4090876cf8ef18ed8541f13684b076d7ac1d0d72) )
	ROM_LOAD32_WORD("20288611.u13", 0x1000002, 0x0400000, CRC(2f561bd0) SHA1(9c2f659ec5cdfd335d089389616dac57fbb655df) )
	ROM_LOAD32_WORD("20288611.u24", 0x1800000, 0x0400000, CRC(a117e3ec) SHA1(119d7ac89ee4c2167b853d720eb87498cefab4b9) )
	ROM_LOAD32_WORD("20288611.u14", 0x1800002, 0x0400000, CRC(6bf0442e) SHA1(c83ed68cd0680ebac359824defa51c1942db17f1) )
	ROM_LOAD32_WORD("20288611.u25", 0x2000000, 0x0400000, CRC(2c5baf9f) SHA1(40fb5b22460b9a35ca8ebb8b0643e58665d8643f) )
	ROM_LOAD32_WORD("20288611.u15", 0x2000002, 0x0400000, CRC(3cd5171b) SHA1(fe6e17679f14ae0238c58dd5d801cb0dda3a434b) )
ROM_END


ROM_START( scatmag2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001311.u86", 0x0000000, 0x0400000, CRC(696734d8) SHA1(baffadb943fb12d0d19069023f5cdf80b1a1674f) )
	ROM_LOAD32_WORD("10001311.u73", 0x0000002, 0x0400000, CRC(aeef47b6) SHA1(63d296d60848269851e9d424908bbf3d52003f07) )
	ROM_LOAD32_WORD("10001311.u85", 0x0800000, 0x0400000, CRC(d3f7018e) SHA1(345a9f55256d9ec46182ebf9c0c6fca62ea2e275) )
	ROM_LOAD32_WORD("10001311.u72", 0x0800002, 0x0400000, CRC(e5c9dee7) SHA1(b29f7b8eb3d894e0a3dbc6cabdd2d8e59e2c2bbf) )
ROM_END


ROM_START( scatmag2sa )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10102711.u86", 0x0000000, 0x0400000, CRC(4ae076d3) SHA1(41350306d03c1211a68ae117521c2a2b9c28d9ef) )
	ROM_LOAD32_WORD("10102711.u73", 0x0000002, 0x0400000, CRC(d04eaa9a) SHA1(1a70612f5ca7fd0d9384f7986775848e3194b718) )
	ROM_LOAD32_WORD("10102711.u85", 0x0800000, 0x0400000, CRC(e7e6bf08) SHA1(657077104ec632affbeaaafe5bc9569deb955668) )
	ROM_LOAD32_WORD("10102711.u72", 0x0800002, 0x0400000, CRC(8ea86d88) SHA1(74ac4df6ec5131d3745de1fdc495fb4c9dc0af33) )
ROM_END


ROM_START( scatmag3 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0452569.u86", 0x0000000, 0x0400000, CRC(8d1d87cd) SHA1(24267218813facee476b0d7b7b3769eabe193b62) )
	ROM_LOAD32_WORD("0452569.u73", 0x0000002, 0x0400000, CRC(9b645cfd) SHA1(9309a3389021776ccc6d2d070a4a954dabcd0979) )
	ROM_LOAD32_WORD("0452569.u85", 0x0800000, 0x0400000, CRC(899f7a05) SHA1(ab24922ce670d151a45af22376042b2a9106f7d1) )
	ROM_LOAD32_WORD("0452569.u72", 0x0800002, 0x0400000, CRC(a71a9565) SHA1(9761ec846b2183c4985eeb224054544f0cf9b3ec) )
ROM_END


ROM_START( sealdeal )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251029.u86", 0x0000000, 0x0400000, CRC(ea6b666b) SHA1(ad80042770f8e66e7455010b46d81873689f63db) ) /* m27v322 */
	ROM_LOAD32_WORD("0251029.u73", 0x0000002, 0x0400000, CRC(383d55a9) SHA1(81bef0ed325ef7208403afa6d7af458c755a4de3) ) /* m27v322 */
	ROM_LOAD32_WORD("0251029.u85", 0x0800000, 0x0400000, CRC(a6327726) SHA1(2c0f02a8d361bcbfc007b9d75a4b7b685aa2b1ac) ) /* m27v322 */
	ROM_LOAD32_WORD("0251029.u72", 0x0800002, 0x0400000, CRC(43ef1218) SHA1(e1d03cd9845ac81a71b5a0471d146addc2f56e0e) ) /* m27v322 */
ROM_END


ROM_START( shamnmg )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10225911.u86", 0x0000000, 0x0400000, CRC(3edb8156) SHA1(83bb1467d892120c2bd5e85ef05527e2d02999ae) )
	ROM_LOAD32_WORD("10225911.u73", 0x0000002, 0x0400000, CRC(a84ac2a4) SHA1(743d56448135b03c25325bdc3d79781e63f0b9b6) )
ROM_END


ROM_START( shamnmgu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152647.u86", 0x0000000, 0x0400000, CRC(9c29e10b) SHA1(9f77808162d038461f77075f0565c0a0d34a708a) )
	ROM_LOAD32_WORD("0152647.u73", 0x0000002, 0x0400000, CRC(f1e9d86d) SHA1(282961a897bbfec4ab778db207f3ef20c6f4c69d) )
	ROM_LOAD32_WORD("0152647.u85", 0x0800000, 0x0400000, CRC(15bb6dd5) SHA1(96625e18b622d2a02f9a88d51ad90d5dac0289be) )
	ROM_LOAD32_WORD("0152647.u72", 0x0800002, 0x0400000, CRC(aab1dd80) SHA1(0e8fbc967c4d5c7d24f980afc78464052ca6d2a6) )
ROM_END


ROM_START( shorthng )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251088.u86", 0x0000000, 0x0400000, CRC(c3e6f083) SHA1(4ed6b1039f060cd6480a490de28619f5370de805) )
	ROM_LOAD32_WORD("0251088.u73", 0x0000002, 0x0400000, CRC(c214d960) SHA1(e5b75e5cb3af3b77f56ccec72833e9243d09c643) )
	ROM_LOAD32_WORD("0251088.u85", 0x0800000, 0x0400000, CRC(e721a879) SHA1(0632c1be6c3b671aed4f6dd67b83e966033aba19) )
	ROM_LOAD32_WORD("0251088.u72", 0x0800002, 0x0400000, CRC(e151390f) SHA1(e68f2584587687c96e4cbe0080630466fac61b17) )
ROM_END


ROM_START( smtma6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10001911.u86", 0x0000000, 0x0400000, CRC(9f436ee3) SHA1(e41a9475fd10fd7d15be1665ee2d4e9dcee2d4dd) )
	ROM_LOAD32_WORD("10001911.u73", 0x0000002, 0x0400000, CRC(ccc901f0) SHA1(4c845ccfea10a744b8c37b6b155b0fc2ce6bfede) )
	ROM_LOAD32_WORD("10001911.u85", 0x0800000, 0x0400000, CRC(7a36f3f3) SHA1(3653a6f40587c2be1810b4a9dd8f27ecca336a70) )
	ROM_LOAD32_WORD("10001911.u72", 0x0800002, 0x0400000, CRC(cb5f47d4) SHA1(70fa1e68bf1fbebc6b6663fbabb78c5f286b7805) )
ROM_END


ROM_START( smtma6q )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10015711.u86", 0x0000000, 0x0400000, CRC(f15a79e5) SHA1(82ef7f28460623bbafa704c9e26e914d4be1cb7f) )
	ROM_LOAD32_WORD("10015711.u73", 0x0000002, 0x0400000, CRC(906cbb69) SHA1(21775cc58425e828d6d77e11d07167acb6867f82) )
	ROM_LOAD32_WORD("10015711.u85", 0x0800000, 0x0400000, CRC(722ea31f) SHA1(6ace379272ca207449a4b03df480929eb1636593) )
	ROM_LOAD32_WORD("10015711.u72", 0x0800002, 0x0400000, CRC(b208c54e) SHA1(ad833271972f58bb866ca63f68d8f5dd67b275b2) )
ROM_END


ROM_START( silkrda6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152537.u86", 0x0000000, 0x0400000, CRC(5e8f8fc3) SHA1(bb586743182a3797d601db740e270e773f7e3587) )
	ROM_LOAD32_WORD("0152537.u73", 0x0000002, 0x0400000, CRC(c32608e3) SHA1(83d9749ce77317ce5631e7c7d3985c71f26d43a5) )
	ROM_LOAD32_WORD("0152537.u85", 0x0800000, 0x0400000, CRC(568f7ecd) SHA1(ccab6d872fe5bc00466071334b1af9f6799e40a0) )
	ROM_LOAD32_WORD("0152537.u72", 0x0800002, 0x0400000, CRC(1ed58be6) SHA1(114c5a800592163dbc690b5d4c4c051d358828f9) )
ROM_END


ROM_START( silgola6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20100721.u86", 0x0000000, 0x0400000, CRC(d91108fd) SHA1(cea026350f083ed0f061d5049de520d08730a1df) )
	ROM_LOAD32_WORD("20100721.u73", 0x0000002, 0x0400000, CRC(cffa82f0) SHA1(fe14e1d9bacbcb72ad1bdaf3d4a72eec67311bef) )
	ROM_LOAD32_WORD("20100721.u85", 0x0800000, 0x0400000, CRC(f66f63c4) SHA1(02e669c4f7c3d0cd39b3dff156ab4b05db3bb495) )
	ROM_LOAD32_WORD("20100721.u72", 0x0800002, 0x0400000, CRC(05861fb2) SHA1(ba3d2e3ab66fd7178289fdc95661f27e08124f32) )
ROM_END


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


ROM_START( spgcarn )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000311.u86", 0x0000000, 0x0400000, CRC(f7783fea) SHA1(4e7b86c0d2728ebc317dce14827a4873402d9bbb) )
	ROM_LOAD32_WORD("10000311.u73", 0x0000002, 0x0400000, CRC(033653f5) SHA1(40705ffb653ad4c6f3cd36c8879c48303c3845f8) )
ROM_END


ROM_START( spgcarnq )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10008011.u86", 0x0000000, 0x0400000, CRC(ad0d4455) SHA1(07ae7dd9dfce742bb77f7ecbb28072005d21be57) )
	ROM_LOAD32_WORD("10008011.u73", 0x0000002, 0x0400000, CRC(43c3600d) SHA1(ddd5dab5555e7586da4ea5ff8109c6b6b0e6ac16) )
	ROM_LOAD32_WORD("10008011.u85", 0x0800000, 0x0400000, CRC(e2a2205b) SHA1(fade79da946ff8a410b4eb05a987616e5266e00f) )
	ROM_LOAD32_WORD("10008011.u72", 0x0800002, 0x0400000, CRC(1351bf20) SHA1(0c608ae7db037691e4d8ea3b8623f7f53ba011d9) )
ROM_END


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


ROM_START( stardrft )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20150811.u86", 0x0000000, 0x0400000, CRC(7b08a2be) SHA1(2b3d7bb2317cba7b470d68c91918a5777fdb3ed1) )
	ROM_LOAD32_WORD("20150811.u73", 0x0000002, 0x0400000, CRC(580340b4) SHA1(4febb22150581a4661bc7ed1ff0662e153de5e2b) )
ROM_END


ROM_START( stardrftu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451341.u86", 0x0000000, 0x0400000, CRC(73023e64) SHA1(4d2d586a1e91ac5bac379994973dffcf9f3a3293) ) /* m27v322 */
	ROM_LOAD32_WORD("0451341.u73", 0x0000002, 0x0400000, CRC(3e59b84f) SHA1(3148d48df74c35681759a234d7aa746cbb0afb0e) ) /* m27v322 */
	ROM_LOAD32_WORD("0451341.u85", 0x0800000, 0x0400000, CRC(689ded33) SHA1(5d5b744321313c4618ecf319827c778be646245f) ) /* m27v322 */
	ROM_LOAD32_WORD("0451341.u72", 0x0800002, 0x0400000, CRC(dc7a30c1) SHA1(8f24c18efd6171cda7ca841d3b275937e35e2be8) ) /* m27v322 */
ROM_END


ROM_START( starqst )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10009511.u86", 0x0000000, 0x0400000, CRC(c21f4486) SHA1(5a6a532d73bf56f68f473bd56a7c3d9f0660d661) )
	ROM_LOAD32_WORD("10009511.u73", 0x0000002, 0x0400000, CRC(facf51b6) SHA1(73c13d507b65cd98ae43918c21b492a8499dd538) )
ROM_END


ROM_START( statsymb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251147.u86", 0x0000000, 0x0400000, CRC(fe9b9147) SHA1(5dced51e126a80191d62b88438e1098efb358e52) )
	ROM_LOAD32_WORD("0251147.u73", 0x0000002, 0x0400000, CRC(e06f2dd1) SHA1(0edadf037dee101e5fb768471d2f93474ff8268f) )
ROM_END


ROM_START( sunmoon )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0251303.u86", 0x0000000, 0x0400000, CRC(f58f4c7b) SHA1(c2e05432f7bc02a5db91a2d83b7c142ac93c1e25) )
	ROM_LOAD32_WORD("0251303.u73", 0x0000002, 0x0400000, CRC(66aa0611) SHA1(f34c28771244f21f38cadac02374243ab26065a0) )
	ROM_LOAD32_WORD("0251303.u85", 0x0800000, 0x0400000, CRC(a1032dbc) SHA1(25e560af6711259a3792285bd9279c22c21d602d) )
	ROM_LOAD32_WORD("0251303.u72", 0x0800002, 0x0400000, CRC(98602b09) SHA1(8f0048b05f069efbb96abf928dc56a6d3124be22) )
ROM_END


ROM_START( sunmoona )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151303.u86", 0x0000000, 0x0400000, CRC(012e5aeb) SHA1(3df4454ab54cc6edf62b8cc9de4c867a36e8654b) )
	ROM_LOAD32_WORD("0151303.u73", 0x0000002, 0x0400000, CRC(ce1f8d32) SHA1(7891b0e797942374d779e8df92281383dd94fe90) )
	ROM_LOAD32_WORD("0151303.u85", 0x0800000, 0x0400000, CRC(7e16ab59) SHA1(f56b3b5a252d3520e36b1d2a7643d65020af2133) )
	ROM_LOAD32_WORD("0151303.u72", 0x0800002, 0x0400000, CRC(f69ce78d) SHA1(91a433757b377406bca61b4c96f3ba3561efd75c) )
ROM_END


ROM_START( sunmonbb )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0153028.u21", 0x0000000, 0x0400000, CRC(b6a8df40) SHA1(316e3f98b75c1bdd0e8fc55aee2cf20b8a45ff4c) )
	ROM_LOAD32_WORD("0153028.u11", 0x0000002, 0x0400000, CRC(b0cc922d) SHA1(5a45d0c995c639739176a5aad42e186b122eead1) )
	ROM_LOAD32_WORD("0153028.u22", 0x0800000, 0x0400000, CRC(98cde50e) SHA1(68e60411a8c10b73482a4cc8d3f14ebdedca0ae9) )
	ROM_LOAD32_WORD("0153028.u12", 0x0800002, 0x0400000, CRC(542ec911) SHA1(48988bdefb9ad6a7e0c733eda1f220b0347cab5f) )
	ROM_LOAD32_WORD("0153028.u23", 0x1000000, 0x0400000, CRC(ac9143cc) SHA1(75aeea2ad26044bef9eb31495d67414a416ee6d4) )
	ROM_LOAD32_WORD("0153028.u13", 0x1000002, 0x0400000, BAD_DUMP CRC(0ac455b1) SHA1(3ca4f4a0172f18692c9f82a9a435ffc1ca39ddc2) )
	ROM_LOAD32_WORD("0153028.u24", 0x1800000, 0x0400000, CRC(a5552dd3) SHA1(1bbe24950e19441ca5144cb9b38320803bd5d26e) )
	ROM_LOAD32_WORD("0153028.u14", 0x1800002, 0x0400000, CRC(19e5b7e0) SHA1(cf17a84ba08a2f24725d4ccbe046710c2f3a2459) )
ROM_END


ROM_START( sunqndol )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451327.u86", 0x0000000, 0x0400000, CRC(78fd2a9f) SHA1(768db303de2ce2fb1efe6adee44d66780801e7da) )
	ROM_LOAD32_WORD("0451327.u73", 0x0000002, 0x0400000, CRC(b46080a1) SHA1(11293e7636dea28c8dfdebd107fa5aee013c832e) )
	ROM_LOAD32_WORD("0451327.u85", 0x0800000, 0x0400000, CRC(f522c85e) SHA1(2b9ba996b99745e69944451e1fd2b585ed62290f) )
	ROM_LOAD32_WORD("0451327.u72", 0x0800002, 0x0400000, CRC(a2b404ac) SHA1(409bd1ce24600fa381ad4de8b7066e5e04f7ab3d) )
ROM_END


ROM_START( sbuk5 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20203611.u86", 0x0000000, 0x0400000, CRC(cab68268) SHA1(e10e8c8ce9bfb7ae7f3920303491b24c93d5f716) )
	ROM_LOAD32_WORD("20203611.u73", 0x0000002, 0x0400000, CRC(39af5809) SHA1(048aef299de220a72bf6a57c6268949197fc738b) )
	ROM_LOAD32_WORD("20203611.u85", 0x0800000, 0x0400000, CRC(9114f9e8) SHA1(f7ec058aaaad59aeffc01085673322cb842a5ebb) )
	ROM_LOAD32_WORD("20203611.u72", 0x0800002, 0x0400000, CRC(ebcce486) SHA1(f91db1051d71c5e558a304d60157fa3cfff66c5b) )
ROM_END


ROM_START( swhr2a6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151183.u86", 0x0000000, 0x0400000, CRC(320d3052) SHA1(8b2165795e9b9d7514620b0d6a14371142de434b) )
	ROM_LOAD32_WORD("0151183.u73", 0x0000002, 0x0400000, CRC(17f0dbe2) SHA1(7f1f0936e1cf528ee794e8925c40b063d1a77729) )
ROM_END


ROM_START( swh2ld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119811.u86", 0x0000000, 0x0400000, CRC(dedd99b0) SHA1(23c4e178d00788516c16109add772dba24f98a54) )
	ROM_LOAD32_WORD("10119811.u73", 0x0000002, 0x0400000, CRC(f5c3d17f) SHA1(297e2da51fb00e0b76f9322149647c4c27ed870e) )
	ROM_LOAD32_WORD("10119811.u85", 0x0800000, 0x0400000, CRC(f0f8aa94) SHA1(c69f1c807b0e9921eb95c3351bb79897e673ec36) )
	ROM_LOAD32_WORD("10119811.u72", 0x0800002, 0x0400000, CRC(94d48b25) SHA1(a879f6d78ff6eb29e5ea2cc71ca86d097fa9d45f) )
ROM_END


ROM_START( tahitim )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10238911.u86", 0x0000000, 0x0400000, CRC(735d7642) SHA1(5f4258bf541728f7c7fe3dd65e715d68b3ade968) )
	ROM_LOAD32_WORD("10238911.u73", 0x0000002, 0x0400000, CRC(b08da2ce) SHA1(1adb4f6e134df479be0f90d2459b009d93386ac1) )
ROM_END


ROM_START( thaiprnc )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10119321.u86", 0x0000000, 0x0400000, CRC(609b3112) SHA1(77648426c7fed2e39d48d4f1ced524067bf0bb1a) )
	ROM_LOAD32_WORD("10119321.u73", 0x0000002, 0x0400000, CRC(a8346fea) SHA1(d97af71bb2c93ed517fccac605e9cfff9ec5590b) )
	ROM_LOAD32_WORD("10119321.u85", 0x0800000, 0x0400000, CRC(92fda7b5) SHA1(9a50ab3d1d18814093f5457a07a52f7e19fc97e7) )
	ROM_LOAD32_WORD("10119321.u72", 0x0800002, 0x0400000, CRC(640bf878) SHA1(910dce13181243a6baace6dd1807d391b3d9767f) )
ROM_END


ROM_START( thaiprncm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30127721.u86", 0x0000000, 0x0400000, CRC(3cb5124b) SHA1(70f6d66793c433054557da4c9f2f033bbb640bd6) )
	ROM_LOAD32_WORD("30127721.u73", 0x0000002, 0x0400000, CRC(531f05ab) SHA1(929285de219c033bdee5f8011e9a0a07b344375e) )
	ROM_LOAD32_WORD("30127721.u85", 0x0800000, 0x0400000, CRC(90c345e0) SHA1(1cf5b237eca68749a7baa147b9b15b3e139d7951) )
	ROM_LOAD32_WORD("30127721.u72", 0x0800002, 0x0100000, BAD_DUMP CRC(613cea6b) SHA1(f04a3ee53074b7cd84879d752df5dbb80437475e) ) // wrong size?!
ROM_END


ROM_START( thndrhrt )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10173811.u86", 0x0000000, 0x0400000, CRC(2b84ed7b) SHA1(9919f9526d8294eeabda40697a34878484f6cef5) )
	ROM_LOAD32_WORD("10173811.u73", 0x0000002, 0x0400000, CRC(d867ff79) SHA1(740a177a8d1ecf925583a9264b5404640a5f2289) )
ROM_END


ROM_START( tigress )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20230111.u86", 0x0000000, 0x0400000, CRC(d1ff02f7) SHA1(8e8f1c4ed3c01fc62011b1bd21f53314e345f10f) )
	ROM_LOAD32_WORD("20230111.u73", 0x0000002, 0x0400000, CRC(cc239965) SHA1(f5d5f1e3db4f0ae3cd1dbe2c88aedf65cc820d4e) )
ROM_END


ROM_START( tikitalk )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10239111.u86", 0x0000000, 0x0400000, CRC(772312e1) SHA1(5a5561d9ec70b56e558ad87ead0770ac9f18e9d1) )
	ROM_LOAD32_WORD("10239111.u73", 0x0000002, 0x0400000, CRC(983d8621) SHA1(92ad28c0a7a06035569700adb8c3449f92bb36b7) )
ROM_END


ROM_START( tiktorch )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151003.u86", 0x0000000, 0x0400000, CRC(ad10a554) SHA1(59062060ded7304193105d2b03fb06a7754a9fe6) )
	ROM_LOAD32_WORD("0151003.u73", 0x0000002, 0x0400000, CRC(f0f8e8d5) SHA1(a084befd422ce37ad19b31d3da61e7a03d5e38c4) )
	ROM_LOAD32_WORD("0151003.u85", 0x0800000, 0x0400000, CRC(32eed9c2) SHA1(8e3e8d55a4dac92ae9b543c86921126df1fb6518) )
	ROM_LOAD32_WORD("0151003.u72", 0x0800002, 0x0400000, CRC(1161133b) SHA1(760c710167e4eca84421fc1cd54ab696e16502cc) )
ROM_END


ROM_START( timbwolf )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0154013.u86", 0x0000000, 0x0400000, CRC(a3be6e57) SHA1(9e06a85b630626bf7e3e790a70b8575bc9047b3d) )
	ROM_LOAD32_WORD("0154013.u73", 0x0000002, 0x0400000, CRC(054e3956) SHA1(ccfb6ba102c22f70fbc61b3d6ba0ceadc1414e1c) )
	ROM_LOAD32_WORD("0154013.u85", 0x0800000, 0x0400000, CRC(901e72e5) SHA1(437a33ac1c73824ce81229f93eff0799b369c6b0) )
	ROM_LOAD32_WORD("0154013.u72", 0x0800002, 0x0400000, CRC(a004dd06) SHA1(c394b1703d62370ecf19c51d90554d6495f43eae) )
ROM_END


ROM_START( topshta6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30123211.u86", 0x0000000, 0x0400000, CRC(798200b2) SHA1(e753f8c8c26e30ab36705f748c01b90bcf166d82) )
	ROM_LOAD32_WORD("30123211.u73", 0x0000002, 0x0400000, CRC(486c5979) SHA1(0ece42200f31daf471b3e1e789de8b95390c4827) )
ROM_END


ROM_START( torgods )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10000411.u86", 0x0000000, 0x0400000, CRC(3c7da6e8) SHA1(ff2fb056fd26d3578216e4942b6234f4523e1d6a) )
	ROM_LOAD32_WORD("10000411.u73", 0x0000002, 0x0400000, CRC(d3f27e13) SHA1(36cd1984139855170af3b23ab32b1769430809d5) )
	ROM_LOAD32_WORD("10000411.u85", 0x0800000, 0x0400000, CRC(9f7013e0) SHA1(76f948ba08152cf623559c3c8f021861757c4872) )
	ROM_LOAD32_WORD("10000411.u72", 0x0800002, 0x0400000, CRC(2dc81027) SHA1(3d426758875e87e677ab446da12532e2a4bc4246) )
ROM_END


ROM_START( toprdice )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20007511.u86", 0x0000000, 0x0400000, CRC(2a3ae479) SHA1(30d5df8eb75775831d4b0792661168288cedbb16) )
	ROM_LOAD32_WORD("20007511.u73", 0x0000002, 0x0400000, CRC(e66f5672) SHA1(9bc92c061fcaf8803878941b9b74a7bc41beb299) )
	ROM_LOAD32_WORD("20007511.u85", 0x0800000, 0x0400000, CRC(9793d2c1) SHA1(50e9d644b6519f941781b726a8d566cae6bb262e) )
	ROM_LOAD32_WORD("20007511.u72", 0x0800002, 0x0400000, CRC(f2ccd55e) SHA1(24738035abc26459d3d06831c907da98aea9c257) )
ROM_END


ROM_START( trsking )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0152570.u86", 0x0000000, 0x0400000, CRC(8e5d3ba0) SHA1(ad3c8ba4351ba2a476aa349e2924c09be9debc20) )
	ROM_LOAD32_WORD("0152570.u73", 0x0000002, 0x0400000, CRC(70253923) SHA1(4a87a9b947404a7692404333fb4e2cd18c8de77c) )
	ROM_LOAD32_WORD("0152570.u85", 0x0800000, 0x0400000, CRC(2cece959) SHA1(c375f1fd1636898feaf1f23baf1c136e1ac09085) )
	ROM_LOAD32_WORD("0152570.u72", 0x0800002, 0x0400000, CRC(c070cb6b) SHA1(cfe6ef7723233f4325ff778679d1274159ffacab) )
ROM_END


ROM_START( turtreas )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10234711.u86", 0x0000000, 0x0400000, CRC(1b69bb51) SHA1(b510fc80616f3acbc8264ca188c5bc85bce38e5e) )
	ROM_LOAD32_WORD("10234711.u73", 0x0000002, 0x0400000, CRC(e00da684) SHA1(4d66d48644008ff998398af1cc970ed9a1504607) )
	ROM_LOAD32_WORD("10234711.u85", 0x0800000, 0x0400000, CRC(512fcb98) SHA1(ccf5bfe8cb9dcd3d2e5e4f59033816ded04f2148) )
	ROM_LOAD32_WORD("10234711.u72", 0x0800002, 0x0400000, CRC(f864fe2e) SHA1(9aa16c4cf3936841a1152418aca7a5dd3ad5a2b3) )
ROM_END


ROM_START( venetian )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("1j008911.u86", 0x0000000, 0x0400000, CRC(57a2a113) SHA1(98589e00803814fe51161d69d77b47ff2b6ac406) )
	ROM_LOAD32_WORD("1j008911.u73", 0x0000002, 0x0400000, CRC(c9cfb52a) SHA1(7df67226c69e4569216e5200364ff14fd162f0a7) )
	ROM_LOAD32_WORD("1j008911.u85", 0x0800000, 0x0400000, CRC(e845f6ee) SHA1(76a234e4b224d56d4f9ceb37d8d175413d4e6fe9) )
	ROM_LOAD32_WORD("1j008911.u72", 0x0800002, 0x0400000, CRC(7a635ef0) SHA1(d0c0746b58ae0726f18e90cc4b20d0e68f3abd86) )
ROM_END


ROM_START( venice )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151355.u86", 0x0000000, 0x0400000, CRC(a6dde7a6) SHA1(58b9bd3d80b88e9e8b40813e85b89cda69f12fb6) )
	ROM_LOAD32_WORD("0151355.u73", 0x0000002, 0x0400000, CRC(d77b73e3) SHA1(4dd4011d6371baf35fcd9cb7c878d4d2cca1e199) )
	ROM_LOAD32_WORD("0151355.u85", 0x0800000, 0x0400000, CRC(f7c3c75d) SHA1(7afc80d8d81b30c1842e4bfbf7ecd987226bdcf6) )
	ROM_LOAD32_WORD("0151355.u72", 0x0800002, 0x0400000, CRC(0570d956) SHA1(415cbaf337296527907ba36932f386aafeceec16) )
ROM_END


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


ROM_START( whalecsh )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20155711.u86", 0x0000000, 0x0400000, CRC(11bcb378) SHA1(56de7fee7631c2e468a1f1845ff9d74db56051f0) )
	ROM_LOAD32_WORD("20155711.u73", 0x0000002, 0x0400000, CRC(3b6d2292) SHA1(87e50f3ed6629c697cff59ec425b098704450993) )
	ROM_LOAD32_WORD("20155711.u85", 0x0800000, 0x0400000, CRC(50afc633) SHA1(ee237d806044bbab3f17210e4e668a8f0961ad92) )
	ROM_LOAD32_WORD("20155711.u72", 0x0800002, 0x0400000, CRC(e799e162) SHA1(aad5d6bff8afcb7f208326c4a663e8462d753f9f) )
ROM_END


ROM_START( whalecshu )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451311.u86", 0x0000000, 0x0400000, CRC(a7118016) SHA1(725a46ac861bf4abaf9cc0e7084cfa20913c2299) )
	ROM_LOAD32_WORD("0451311.u73", 0x0000002, 0x0400000, CRC(74c93c56) SHA1(34c48620b08c3a03f0d6a333acbc45c14a91d3cb) )
	ROM_LOAD32_WORD("0451311.u85", 0x0800000, 0x0400000, CRC(e90f5bb1) SHA1(28680395198a29356999a377f2f3e0013d97c7b2) )
	ROM_LOAD32_WORD("0451311.u72", 0x0800002, 0x0400000, CRC(f138de3f) SHA1(69a54457c8090f23023f197f0eba1d645a260f3f) )
ROM_END


ROM_START( whalecshua )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0451346.u86", 0x0000000, 0x0400000, CRC(8aef1004) SHA1(270379b0a3abc98adfb1a9b031a5e98050635a80) )
	ROM_LOAD32_WORD("0451346.u73", 0x0000002, 0x0400000, CRC(22785960) SHA1(2ebfd230209fd438338f981c1c133e6576a67468) )
	ROM_LOAD32_WORD("0451346.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0451346.u72", 0x0800002, 0x0400000, CRC(e9ddd951) SHA1(16863b49b5db1881afda30ea19a55f6122981eba) )
ROM_END


ROM_START( wheregld )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10124811.u86", 0x0000000, 0x0400000, CRC(d537f1f1) SHA1(49825365593238f757e3efecbe9fe02a06acd321) )
	ROM_LOAD32_WORD("10124811.u73", 0x0000002, 0x0400000, CRC(d8b5ccb3) SHA1(7974d68d594a2b283680cc440d69b106bd539819) )
	ROM_LOAD32_WORD("10124811.u85", 0x0800000, 0x0400000, CRC(f9554ab0) SHA1(c79d95ee339f139d0a62550e926acd6c1d7d534b) )
	ROM_LOAD32_WORD("10124811.u72", 0x0800002, 0x0400000, CRC(584050fc) SHA1(014889e1bf3fa38c052073c374b47684be717949) )
ROM_END


ROM_START( wheregldsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10124811.u86", 0x0000000, 0x0400000, CRC(4e6fad4a) SHA1(a2efc21cf773f9cb291ea1607d1864ba333807e9) )
	ROM_LOAD32_WORD("sp__10124811.u73", 0x0000002, 0x0400000, CRC(57253567) SHA1(41e0b1b7247ff4f0f9c6b150aef23a5eb5984e30) )
	ROM_LOAD32_WORD("sp__10124811.u85", 0x0800000, 0x0400000, CRC(86a40a48) SHA1(e0ad565e56c698df0f631e42dcfda17dfa5c8017) )
	ROM_LOAD32_WORD("sp__10124811.u72", 0x0800002, 0x0400000, CRC(bd188f12) SHA1(53433d346f23bc826fc041f1631270f8380759b6) )
ROM_END


ROM_START( wheregldm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20177111.u86", 0x0000000, 0x0400000, CRC(d09fd746) SHA1(6c805f7c535a6c08a836b94c0351d498751ac9e9) )
	ROM_LOAD32_WORD("20177111.u73", 0x0000002, 0x0400000, CRC(3007f8d1) SHA1(a76a227b8157d80b08e224807345a56da4c326fd) )
	ROM_LOAD32_WORD("20177111.u85", 0x0800000, 0x0400000, CRC(fb8c24e7) SHA1(ffa62c59f1857dcd670c733397d20c50710d9a85) )
	ROM_LOAD32_WORD("20177111.u72", 0x0800002, 0x0400000, CRC(9d0b5bdf) SHA1(0018c93df75b9a54f43d660a24e00dc801a68084) )
	ROM_LOAD32_WORD("20177111.u84", 0x1000000, 0x0400000, CRC(be31366b) SHA1(a82fb5bd41079c6727a93115fa74af5fe44ba4a9) )
	ROM_LOAD32_WORD("20177111.u71", 0x1000002, 0x0400000, CRC(6781f074) SHA1(d99a6b0a86317e7c8eb79d362338eb39b58aed54) )
ROM_END


ROM_START( wheregldq )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20184811.u86", 0x0000000, 0x0400000, CRC(95da579c) SHA1(230a18b76e72f09ea74543bd6a7a31ae51bf651e) )
	ROM_LOAD32_WORD("20184811.u73", 0x0000002, 0x0400000, CRC(c35095ec) SHA1(637c5cfbb985716140a0e123c23ba13bffc2e734) )
	ROM_LOAD32_WORD("20184811.u85", 0x0800000, 0x0400000, CRC(1520539c) SHA1(9f3503bfcadc74f9e12ea7300b3356e9efb00b5a) )
	ROM_LOAD32_WORD("20184811.u72", 0x0800002, 0x0400000, CRC(50ed1445) SHA1(6b623965b95352d7f79700a075eaf4eb2019dee8) )
ROM_END


ROM_START( whtwater )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0151075.u86", 0x0000000, 0x0400000, CRC(9e527f96) SHA1(34e46e530686ab5955ae267541e96d06a4e149a6) )
	ROM_LOAD32_WORD("0151075.u73", 0x0000002, 0x0400000, CRC(0e1fcaca) SHA1(725237cca34d86e53cb68547604d8844257cdf48) )
	ROM_LOAD32_WORD("0151075.u85", 0x0800000, 0x0400000, CRC(8802e0cb) SHA1(d11874426839a506d78440adbc49056ce21f88d3) )
	ROM_LOAD32_WORD("0151075.u72", 0x0800002, 0x0400000, CRC(64a814c6) SHA1(9415bb32675e5c3b6299967b1d10dbccd699d9ed) )
ROM_END


ROM_START( wikwin2 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10214011.u86", 0x0000000, 0x0400000, CRC(226c8d1c) SHA1(37769691d1b7e308cbd1d857e8695f1e129a6722) )
	ROM_LOAD32_WORD("10214011.u73", 0x0000002, 0x0400000, CRC(6662912d) SHA1(017e1684de64308b035b817b6fad954be538b78c) )
	ROM_END


ROM_START( wldafr )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0351083.u86", 0x0000000, 0x0400000, CRC(2193094c) SHA1(57d687301a59f36880560a3efc7040e333cb05d3) )
	ROM_LOAD32_WORD("0351083.u73", 0x0000002, 0x0400000, CRC(e1e1253e) SHA1(4c15c953b80288df8f97197d70c8076fde9a0d29) )
	ROM_LOAD32_WORD("0351083.u85", 0x0800000, 0x0400000, CRC(7ecbb5ba) SHA1(ab201ad7db848f5c03e3795c63308530b371d354) )
	ROM_LOAD32_WORD("0351083.u72", 0x0800002, 0x0400000, CRC(a55417ca) SHA1(151615f81112b30aff2a2aa26280b7c1bf0d45db) )
ROM_END


ROM_START( wldcata6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10237511.u86", 0x0000000, 0x0400000, CRC(f95e6ba6) SHA1(8d787d96d7b3cc3cde0be49a54701c2d3f70c659) )
	ROM_LOAD32_WORD("10237511.u73", 0x0000002, 0x0400000, CRC(0300f5ff) SHA1(52d640b676cd42dfab2c38108dd765c8e8092528) )
	ROM_LOAD32_WORD("10237511.u85", 0x0800000, 0x0400000, CRC(687c4bd6) SHA1(548de5a9887dfc58353b7a2861447bc9cd5f955e) )
	ROM_LOAD32_WORD("10237511.u72", 0x0800002, 0x0400000, CRC(f0707ab9) SHA1(f30d77c53dfbc43e9468f2cb91529e44ce10c59e) )
ROM_END


ROM_START( wldgoos )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20131911.u86", 0x0000000, 0x0400000, CRC(0925481f) SHA1(0dfc1bbe13752fcfd270fb3c096a3612cfa2fe9a) )
	ROM_LOAD32_WORD("20131911.u73", 0x0000002, 0x0400000, CRC(5f9ab2a5) SHA1(f55dcc32463e395f1380c4fa456b39f77c27642b) )
	ROM_LOAD32_WORD("20131911.u85", 0x0800000, 0x0400000, CRC(054d3f78) SHA1(636f0284508e77d21dc9b50d59f1d7b8ed686ed0) )
	ROM_LOAD32_WORD("20131911.u72", 0x0800002, 0x0400000, CRC(ba887abf) SHA1(3d4e529bf2fed83506ef947ade08aeff7ad78611) )
ROM_END


ROM_START( wgorilla )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20279811.u86", 0x0000000, 0x0400000, CRC(c8f15db2) SHA1(feaf68f442c96367db374772db57619ea5c01bca) )
	ROM_LOAD32_WORD("20279811.u73", 0x0000002, 0x0400000, CRC(7588c213) SHA1(b1d0ec23b8c668ae40664329a5f6a45b24f6ef2f) )
	ROM_LOAD32_WORD("20279811.u85", 0x0800000, 0x0400000, CRC(28049860) SHA1(cace39a0121127ead7c8550153372283e1610a96) )
	ROM_LOAD32_WORD("20279811.u72", 0x0800002, 0x0400000, CRC(e5a4e6f5) SHA1(f63fdc4771433f9953190b9cae4d7cbc4f6e374f) )
ROM_END


ROM_START( wldpanda )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20218411.u86", 0x0000000, 0x0400000, CRC(d3ebdc8a) SHA1(fc27c96769cc3ba02e7ad4baf6055f7902e50081) )
	ROM_LOAD32_WORD("20218411.u73", 0x0000002, 0x0400000, CRC(46e28dff) SHA1(b1fd7c0d0c814f04b2bd6f35835d1d1d41aab62c) )
	ROM_LOAD32_WORD("20218411.u85", 0x0800000, 0x0400000, CRC(0bb16a0b) SHA1(6abbbb9abac53b543e92ccdead9770271d28074e) )
	ROM_LOAD32_WORD("20218411.u72", 0x0800002, 0x0400000, CRC(11e2167f) SHA1(812ed09436c2899780791ca0ada5f3bbec4a3a88) )
ROM_END


ROM_START( wldpanth )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0651310.u86", 0x0000000, 0x0400000, CRC(c874ec84) SHA1(e4f3beaa254f5b827c7f0f5d22be7dd5b3abfd2c) )
	ROM_LOAD32_WORD("0651310.u73", 0x0000002, 0x0400000, CRC(59d83a8a) SHA1(61f6e6b85108621c2eac1bf48c27d79a72fbb2fd) )
	ROM_LOAD32_WORD("0651310.u85", 0x0800000, 0x0400000, NO_DUMP )
	ROM_LOAD32_WORD("0651310.u72", 0x0800002, 0x0400000, CRC(2969f69c) SHA1(d63f1bcbd163c067f143a55aaf7c88b3c9ac0ebe) )
ROM_END


ROM_START( wldstall )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10230811.u86", 0x0000000, 0x0400000, CRC(b3171fd7) SHA1(d3ac53ba6bf6cc9ea3dc0f83e8a3eecf86abad55) )
	ROM_LOAD32_WORD("10230811.u73", 0x0000002, 0x0400000, CRC(ceeff2d5) SHA1(04d5139c0ce256eb0a0734184ccc8460d2ac1120) )
	ROM_LOAD32_WORD("10230811.u85", 0x0800000, 0x0400000, CRC(6fee0413) SHA1(242f60d89eb6b44ef968f482070e0a81d2ad5fcf) )
	ROM_LOAD32_WORD("10230811.u72", 0x0800002, 0x0400000, CRC(0f4de65a) SHA1(232406f335786a045ab1833db50465ed476dfffe) )
ROM_END


ROM_START( wtepee )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10230211.u86", 0x0000000, 0x0400000, CRC(8c38fa9c) SHA1(0f64da87271f90309b0945403cc24fdf82c5c5a2) )
	ROM_LOAD32_WORD("10230211.u73", 0x0000002, 0x0400000, CRC(6c0a4268) SHA1(c18ad70332ec300b227edfc684db97d517031ff1) )
	ROM_LOAD32_WORD("10230211.u85", 0x0800000, 0x0400000, CRC(e5492c9b) SHA1(bd71198f24adab2841f177598d1ed8ae07aeb971) )
	ROM_LOAD32_WORD("10230211.u72", 0x0800002, 0x0400000, CRC(0c5c51c2) SHA1(6423e160355ad0380cbb46e7462f913a67e9b29d) )
ROM_END


ROM_START( wwaratah )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10215911.u86", 0x0000000, 0x0400000, CRC(42075131) SHA1(855822a44cf65f2bdd8c88f89e75cdac2f2ddf89) )
	ROM_LOAD32_WORD("10215911.u73", 0x0000002, 0x0400000, CRC(9df889d7) SHA1(3375a0dd56367c8aff6a07d54769715e7f29f6c4) )
	ROM_LOAD32_WORD("10215911.u85", 0x0800000, 0x0400000, CRC(c86d4145) SHA1(6b52d5d875ac4f786a2cb664dbb54c0a7cf68014) )
	ROM_LOAD32_WORD("10215911.u72", 0x0800002, 0x0400000, CRC(90813234) SHA1(80461a72cfcd056f8af7559f8f5e44447631ff0c) )
ROM_END


ROM_START( wwaratahsp )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("sp__10215911.u86", 0x0000000, 0x0400000, CRC(0c6014c8) SHA1(c1962fc9d9453d396cc625183d1a93f686ff176f) )
	ROM_LOAD32_WORD("sp__10215911.u73", 0x0000002, 0x0400000, CRC(fc20f733) SHA1(ca0b8e9bf9e0071e6a5012515ba0c94d8dd1d838) )
	ROM_LOAD32_WORD("sp__10215911.u85", 0x0800000, 0x0400000, CRC(54c9b20d) SHA1(290ff7380dab2f721bb7576ba4fcac8dda8c35b9) )
	ROM_LOAD32_WORD("sp__10215911.u72", 0x0800002, 0x0400000, CRC(0bf1fb4b) SHA1(009b2a0ceba6d682f9ba0d856d13177d261e0827) )
ROM_END


ROM_START( wways )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10106911.u86", 0x0000000, 0x0400000, CRC(ea4dd6db) SHA1(ee07fbcc9689cdb5e96e6259b630ff1229e2cad6) )
	ROM_LOAD32_WORD("10106911.u73", 0x0000002, 0x0400000, CRC(526ef495) SHA1(ca7df6ae32822d1205ee01e6970af428c515d940) )
	ROM_LOAD32_WORD("10106911.u85", 0x0800000, 0x0400000, CRC(81e8e5ad) SHA1(612b2fd4506f99ef946e10be2db82e50fcf6083f) )
	ROM_LOAD32_WORD("10106911.u72", 0x0800002, 0x0400000, CRC(acce8d74) SHA1(e22e9611e9a221bd5dfaad3d9feece1646bebe02) )
ROM_END


ROM_START( wwaysm )
	ARISTMK6_BIOS

	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("10130111.u86", 0x0000000, 0x0400000, CRC(2968765c) SHA1(ba2c67c4be4063d8506cc8127c31b4df2609650b) )
	ROM_LOAD32_WORD("10130111.u73", 0x0000002, 0x0400000, CRC(a1e0d77e) SHA1(df4d45d8c4dcfdb1fae4b5d5a0adfa0464c61828) )
	ROM_LOAD32_WORD("10130111.u85", 0x0800000, 0x0400000, CRC(d87426d5) SHA1(ba755f8fc426dcd8abf4f6ccee423ae0504bf6fe) )
	ROM_LOAD32_WORD("10130111.u72", 0x0800002, 0x0400000, CRC(5f2c30d1) SHA1(dfc68903e8375597477acc2fc403dcd9b838ae6c) )
ROM_END


ROM_START( wingoly )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("0252529.u86", 0x0000000, 0x0400000, CRC(c5d7d7cf) SHA1(641cdfe9b612c9cefd6a279f0c44fd535521aeef) )
	ROM_LOAD32_WORD("0252529.u73", 0x0000002, 0x0400000, CRC(5c003b1e) SHA1(be419686b89e22a1aa5d841015ab3442bffe05a6) )
	ROM_LOAD32_WORD("0252529.u85", 0x0800000, 0x0400000, CRC(63aef582) SHA1(59fa78d7d29c8d4686fab83cfa46238938a9c249) )
	ROM_LOAD32_WORD("0252529.u72", 0x0800002, 0x0400000, CRC(158877b5) SHA1(a1fc98d90a763253f07b9df44d85e8594b26d484) )
ROM_END


ROM_START( wzwaysll )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("20139111.u86", 0x0000000, 0x0400000, CRC(c8dffeef) SHA1(20d710356282c434523c5828fdb56e3da37d3371) )
	ROM_LOAD32_WORD("20139111.u73", 0x0000002, 0x0400000, CRC(5663bb95) SHA1(0df425bb4130079757fbbc6b5532c2f6a1a32920) )
	ROM_LOAD32_WORD("20139111.u85", 0x0800000, 0x0400000, CRC(6a8efdf2) SHA1(38b955755bbc2f2a6c8c987b0cae92d314d727c9) )
	ROM_LOAD32_WORD("20139111.u72", 0x0800002, 0x0400000, CRC(944dc1ec) SHA1(7dd2b14eb0af7babc9de09f247776f4a24d53e6c) )
	ROM_LOAD32_WORD("20139111.u84", 0x1000000, 0x0400000, CRC(0399bc1d) SHA1(19cd298d439dddac482c1d7f1fd7446900b39c46) )
	ROM_LOAD32_WORD("20139111.u71", 0x1000002, 0x0400000, CRC(e8f8ffb1) SHA1(3aa4f8d959e9671ae93c9946b5420d55c45b1bbc) )
ROM_END


ROM_START( zorroa6 )
	ARISTMK6_BIOS
	ROM_REGION64_LE( 0x4000000, "game_rom", ROMREGION_ERASEFF)
	ROM_LOAD32_WORD("30130611.u86", 0x0000000, 0x0400000, CRC(adf787e0) SHA1(42cfeb3ad42bf255a9be5a629a71bababbbb0147) )
	ROM_LOAD32_WORD("30130611.u73", 0x0000002, 0x0400000, CRC(0bad02dc) SHA1(dfe495b5aeb75593d52669cd86d91f7e917bc3b0) )
	ROM_LOAD32_WORD("30130611.u85", 0x0800000, 0x0400000, CRC(68da9660) SHA1(dc6ba4c9392bd6efd72bf996d016503677ea8622) )
	ROM_LOAD32_WORD("30130611.u72", 0x0800002, 0x0400000, CRC(cc5616af) SHA1(94d1d36b76e2587caa2c60ea70c9206580dc53d5) )
ROM_END


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


GAME( 2000, aristmk6,   0,        aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Aristocrat MK6 System Software",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND | MACHINE_IS_BIOS_ROOT)

// Dates listed below are for the combination (reel layout), not release dates
// Hyperlink jackpot versions e.g. Bank Buster, Cash Express, Jackpot Carnival and Maximillions are considered clones if an original set exists
GAME( 2004, 5drag,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (20161011, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    C - 14/01/04
GAME( 2004, 5dragsp,    5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons (20161011, NSW/ACT, Show Program)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744,    C - 14/01/04
GAME( 2007, 5dragce,    5drag,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Dragons - Cash Express (0152309, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US211,    A - 01/09/07
GAME( 2007, 5koipp,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "5 Koi - Power Pay (10250711, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744/2,    C - 25/10/07
GAME( 2004, 15lions,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "15 Lions (10166211, Queensland)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/3, B - 23/03/04
GAME( 2002, 50lions,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions (10120511, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717,    A - 11/10/02
GAME( 2003, 50lionsm,   50lions,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions (10156111, Malaysia)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717,    D - 20/10/03
GAME( 2012, 50lionrr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "50 Lions - Roaring Riches (20301311, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 1049,    B - 20/08/12
GAME( 2006, 100lions,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "100 Lions (10219211, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/4,    B - 13/06/06
GAME( 2005, adonisa6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Adonis (0151178, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US168,    A - 05/26/05
GAME( 2001, aduskcm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "African Dusk - Mr. Cashman (20006711, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 665/1,    B- 13/07/01
GAME( 2004, afterhrs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "After Hours (0251071, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US051,    B - 01/20/04
GAME( 2003, alchemy,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Alchemy (1J009611, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013/2,    A - 30/05/03
GAME( 2007, atkgld,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "All the King's Gold (0152152, US)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US209,    A - 01/08/07
GAME( 2001, amzqueen,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Amazon Queen (10005511, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 669,    A - 02/05/01
GAME( 2009, amztempl,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Amazon Temple (10299811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 906,    A - 29/04/09
GAME( 2004, antcleo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Antony and Cleopatra (10163211, NSW/ACT)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 754,    A - 15/01/04
GAME( 2004, antcleom,   antcleo,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Antony and Cleopatra (10177211, Malaysia)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 754,    B - 12/07/04
GAME( 2003, apocof,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Apollo Coffee (0151105, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US083,    A - 11/10/03
GAME( 2001, arabnga6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arabian Nights (2J008211, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB009/1,    E - 10/12/01
GAME( 2004, arabnga6u,  arabnga6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arabian Nights (0151356, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/21,    E - 07/06/04
GAME( 2003, arwincm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arctic Wins - Mr. Cashman (0151065, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US029,    E- 01/27/03
GAME( 2003, arish,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Arishinko (0151087, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US057,    E - 05/28/03
GAME( 2004, asnprinc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Asian Princess (0352502, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4208,    C - 09/13/04
GAME( 2008, beavbuk,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Beaver Bucks (20255111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 850,    C - 23/01/08
GAME( 2003, betzoo,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Betting Zoo - Mr. Cashman (0251064, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US037,    B - 01/28/03
GAME( 2003, bigben,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (30126911, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710,    B - 17/07/03
GAME( 2003, bigbena,    bigben,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (20126911, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710,    B - 17/07/03
GAME( 2005, bigbenq,    bigben,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Ben (20201111, Queensland)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710/3, C - 09/08/05
GAME( 1999, bigheart,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Hearted (10000511, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 637,    D - 15/11/99
GAME( 2002, bigred,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red (2J009211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB015,    D - 27/09/02
GAME( 2002, bigreda,    bigred,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red (1J009211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB015,    D - 27/09/02
GAME( 2005, bigred2,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Big Red 2 (5J011311, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB024,    B - 18/05/05
GAME( 2002, blkbrum,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Black Brumby (1J009111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB010,    C - 14/06/02
GAME( 200?, blkmust,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Black Mustang (0152584, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/21,    (no data)
GAME( 2007, bluetigr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Blue Tiger (20259511, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 855,    A - 12/12/07
GAME( 2007, bobdolly,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bob and Dolly (0352298, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US243,    A - 10/15/07
GAME( 2005, brazil,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Brazil (20180311, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 783,    B - 25/02/05
GAME( 2004, bucaneer,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buccaneer (10173011, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765,    A - 05/04/04
GAME( 2005, bucaneeru,  bucaneer, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buccaneer (0252523, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4229,    B - 01/07/05
GAME( 2010, buckstop,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Buck Stops Here (10294311, NSW/ACT)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 897/1,    A - 22/07/10
GAME( 2006, buffalo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buffalo (10217811, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 806,    E - 06/10/06
GAME( 2006, buffalou,   buffalo,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Buffalo (0252636, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4328,    (no data)
GAME( 2005, bmbugs,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bumble Bugs (0351180, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US166,    A - 05/13/05
GAME( 2001, bushrngr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Bushranger (10006111, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 673,    A - 18/04/01
GAME( 2007, cactusco,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Cactus Corral (10237711, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 818,    D - 24/07/07
GAME( 2007, callwld,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Call of the Wild (10256611, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 827,    B - 12/11/07
GAME( 2005, calyking,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Calypso King (20197111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 772/1,    B - 25/07/05
GAME( 2006, canaryrc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Canary Riches (10218211, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 809,    A - 30/03/06
GAME( 2005, cmtonig,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Cashman Tonight (30194711, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0007,    B - 25/05/05
GAME( 2004, cinhell,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Chance in Hell - Lil' Lucy (0251125, US)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US119,    A - 06/30/04
GAME( 2002, csd,        aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (10104011, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/04/02
GAME( 2002, csdsp,      csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (20104011, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/04/02
GAME( 2003, csdm,       csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (20131511, Malaysia)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 23/01/03
GAME( 2002, csdq,       csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa (10121111, Queensland)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688, C - 05/07/02
GAME( 2003, csdce,      csd,      aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Doa - Cash Express (30149511, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 688,    A - 03/02/03
GAME( 2006, csret,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Choy Sun Returns (20212211, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 744/2,    B - 15/08/06
GAME( 2005, corrida,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Corrida de Toros (30178311, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 771,    D - 22/03/05
GAME( 2003, cryspri,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (10144411, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 740,    A - 28/05/03
GAME( 2003, crysprim,   cryspri,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (10155811, Malaysia)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 740,    A - 28/05/03
GAME( 2006, cryspriu,   cryspri,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Crystal Springs (0352557, US)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4255,    C - 01/16/06
GAME( 2009, deepfrz,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Deep Freeze (10282311, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB113/2,    A - 27/02/09
GAME( 2009, deepfrzu,   deepfrz,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Deep Freeze (0154081, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB113/4,    A - 27/07/09
GAME( 2005, deltabel,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Delta Belle (0252532, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4243,    B - 04/29/05
GAME( 2004, deltaqn,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Delta Queen (20185211, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 768,    A - 13/09/04
GAME( 2004, dimeye,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamond Eyes (0251312, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB012/20,    J - 01/22/04
GAME( 2004, dimeyece,   dimeye,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamond Eyes - Cash Express (0152039, US)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB012/20,    J - 01/22/04
GAME( 2002, di4ever,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Diamonds Forever (10105011, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 679,    B - 14/03/02
GAME( 2003, dinosa6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dinosaur (10118811, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 731,    C - 08/08/03
GAME( 2005, dblagent,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Agent (0152082, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US180,    A - 12/09/05
GAME( 2000, dbldolph,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Dolphins (20000211, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647,    C - 18/04/00
GAME( 2000, dbldolphq,  dbldolph, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Double Dolphins (40003211, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647, F - 17/11/00
GAME( 2004, drgnlord,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dragon Lord (10173511, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 761,    A - 22/06/04
GAME( 2006, drgnmgic,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dragon Magic (2J015711, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB051,    E - 05/07/06
GAME( 2003, drmcatch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dream Catcher (10126721, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 704,    F - 23/06/03
GAME( 2008, drmred,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Dream of the Red Chamber (10273811, NSW/ACT)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB027/2,    A - 25/8/08
GAME( 2004, eaglrock,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eagle Rock - Cash Express (0151126, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US021,    C - 05/25/04
GAME( 2010, estrike,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eagle Strike (40289711, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 950,    A - 20/07/10
GAME( 2004, eastland,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Easter Island - Cash Express (0151117, US)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US052,    B - 02/22/04
GAME( 2005, eggsrccm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eggstreme Riches - Mr. Cashman (0351149, US)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US144,    B - 11/3/05
GAME( 200?, efce,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchanted Forest - Cash Express (0151084, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2007, encspell,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchanted Spell (10257011, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB041,    B - 18/07/07
GAME( 2000, enchantr,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Enchantress (20000711, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 633,    C- 15/01/00
GAME( 2000, eott,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Eyes of the Tiger (10002011, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 658,    B - 01/06/00
GAME( 2004, firednc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fire Dancer (20184911, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 772,    A - 01/10/04
GAME( 2002, flamolce,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Flame of Olympus - Cash Express (0351025, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/3,    A - 2/18/02
GAME( 2002, flamoljc,   flamolce, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Flame of Olympus - Jackpot Carnival (0251048, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/3,    A - 2/18/02
GAME( 2000, followst,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Follow the Stars (20001411, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 659,    D - 15/09/00
GAME( 2007, fortath,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Fortune of Athena (10248611, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 847,    C - 08/11/07
GAME( 2004, frogwld,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Frog Wild 2 (0251114, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US112,    A - 06/03/04
GAME( 2005, gambjack,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Gamblin' Jack (20197211, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 791,    D - 23/09/05
GAME( 2004, gwnn,       aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Game With No Name! (0452501, US)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4210,    C - 09/13/04
GAME( 2000, geishaa6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha (10000811, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/18,    B - 03/06/00
GAME( 2001, geishajc,   geishaa6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Geisha - Jackpot Carnival (0351033, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4091/2,    A - 10/18/01
GAME( 2001, goape,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Go Ape (10006211, NSW/ACT)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 661/2,    B - 14/06/01
GAME( 2001, gogreen,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Go For Green (10003411, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 647/1,    A - 22/02/01
GAME( 2005, goldarch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Archer (10197411, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 770,    A - 07/07/05
GAME( 2003, gldnflc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Fleece (10158311, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 680/2,    A-07/07/03
GAME( 2005, gldgong,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Gong (0152558, US)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4258,    C - 08/18/05
GAME( 2005, gldgongq,   gldgong,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Golden Gong (10217411, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 710/3,    C - 09/08/05, Identical date string to Big Ben above (the games are identical but with different graphics)
GAME( 2004, goldinc,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Incas (10174011, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 752,    A - 07/04/04
GAME( 2005, goldincu,   goldinc,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Incas (0252533, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4244,    A - 03/10/05
GAME( 2002, goldpym,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Pyramids Special Edition - Maximillions (10130811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2007, goldsam,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Golden Samurai (10247811, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 832,    A - 31/07/07
GAME( 2002, gbmoney,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Good, The Bad & The Money (0351016, US)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US023,    B - 09/11/02
GAME( 2006, grizzly,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Grizzly (10214711, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 780,    C - 12/04/06
GAME( 2003, heartgld,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Heart of Gold (20169511, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/2,    A - 15/09/03
GAME( 2006, heartveg,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Heart of Vegas (10232911, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/4,    C - 27/06/06
GAME( 2002, hlntroy,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (1J008311, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/4,    G - 21/01/02
GAME( 2004, hlntroyu,   hlntroy,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Helen of Troy (0151336, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/21,    B - 01/16/04
GAME( 2001, highnoon,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "High Noon (20004111, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 671,    B - 28/06/01
GAME( 2002, holdrma,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Hollywood Dreams (10100511, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 670/1,    A - 31/01/02
GAME( 2004, househrt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "House of Hearts (10200311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765,    C - 09/11/04
GAME( 2005, incachf,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Inca Chief (20190711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 784,    C - 21/10/05
GAME( 2001, indream,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming (10004211, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/6,    B - 24/04/01
GAME( 2002, indremce,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Cash Express (20109411, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/10,    A - 12/07/02
GAME( 2003, indremll,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Lady Luck (20151911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 742,    C - 15/10/03
GAME( 2003, indrem2c,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming 2nd Chance (10150611, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 753,    B - 08/12/03
GAME( 2003, indremmm,   indream,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Dreaming - Maximillions (10130711, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/12, A - 17/01/03
GAME( 2001, indgldcm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Gold - Mr. Cashman (10102511, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 674/1,    A- 21/11/01
GAME( 2004, indprncs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Indian Princess (10174111, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 764,    A - 07/04/04
GAME( 2002, jailbrk,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Jailbreak (10125611, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 716,    A - 14/11/02
GAME( 2003, jefffox,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "You Might Be A Redneck If... - Jeff Foxworthy (0351068, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US041,    B - 06/13/03
GAME( 2003, jewelcm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Jewel of the Enchantress - Mr. Cashman (0151051, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US039,    B- 1/24/03
GAME( 2005, kingnep,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Neptune (20189111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 789,    E - 08/08/05
GAME( 2003, kingnile,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King of the Nile (30106331, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    D - 07/01/03
GAME( 2002, kingnilea,  kingnile, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King of the Nile (20106321, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    C - 27/09/02
GAME( 2002, kingpeng,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Penguin (20109021, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 696,    D - 02/10/02
GAME( 2002, kingpengsp, kingpeng, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "King Penguin (40109021, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 696,    E- 26/11/02
GAME( 2000, koalama6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Koala Mint (30001011, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 643/6,    A- 01/09/00
GAME( 2003, kohinoor,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Kohinoor (10136711, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/21,    B - 10/02/03
GAME( 2006, lgfish,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Let's Go Fish'n (10220111, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    E - 23/05/06
GAME( 2004, lemrol,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Let 'em Roll (0151115, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US088,    B - 06/24/04
GAME( 2005, lineking,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Line King (0252534, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4245,    F - 11/10/05
GAME( 2002, lordleo,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Lord Leopard (30111211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 685,    B - 27/05/02
GAME( 2001, lougcm,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Louie's Gold - Mr. Cashman (20006911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 674,    C- 13/07/01
GAME( 200?, lovestuk,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Love Stuck (0152583, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4285,    (no data)
GAME( 2007, lucky88,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Lucky 88 (10241411, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 840,    C - 25/06/07
GAME( 2007, luigibal,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Luigi's Ball (10235411, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 822,    D  - 19/03/07
GAME( 2005, magforst,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Magic Forest (0151139, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US141,    D - 02/01/05
GAME( 2003, magmerm,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Magic Mermaid (0251331, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013/20,    F - 10/13/03
GAME( 2006, magician,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "The Magician (20210111, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 801,     C - 01/03/06
GAME( 2007, metestrm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Meteor Storm (0152162, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US217,    A - 03/19/07
GAME( 2007, milkin,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Milkin' It (10251911, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 816,    B - 13/08/07
GAME( 2005, mskitty,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Miss Kitty (10216611, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    C - 18/04/05
GAME( 2000, mnytree,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Money Tree (10001211, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657,    E - 06/11/00
GAME( 2004, moonfire,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Moon Fire - Jackpot Carnival (30164211, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/3,    E - 27/01/04
GAME( 2007, moonwalt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Moonlight Waltz (10227611, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 814/1,    B - 11/01/07
GAME( 2008, mchilli,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "More Chilli (20248711, NSW/ACT)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 844,    B - 21/01/08
GAME( 2010, mchilliq,   mchilli,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "More Chilli (20289311, Queensland)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 844/2, B - 14/12/10
GAME( 200?, mrwoo,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mr Woo (0252599, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4276,    (no data)
GAME( 2002, mystarr,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Arrow (0151009, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US005,    A - 01/07/02
GAME( 2001, mysteycm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Eyes - Mr. Cashman (10008111, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 675,    A- 09/05/01
GAME( 2002, mysteycmu,  mysteycm, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Eyes - Mr. Cashman (0251024, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US020,    C- 31/05/02
GAME( 2002, mystmerm,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Mystic Mermaid (1J008711, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB013,    D - 27/03/02
GAME( 2005, nerdwild,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Nerds Gone Wild (10197311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    C - 18/04/05
GAME( 200?, opsesam,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Open Sesame (0351309, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // ???,    ???
GAME( 2006, orbeaut,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Oriental Beauty (10213511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 808,    A - 07/02/06
GAME( 2005, outbackj,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Outback Jack (20195211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 691,    J - 02/06/05
GAME( 2001, owlcap,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Owl Capone (10008711, NSW/ACT, Show Program)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 594/9,    A - 12/06/01
GAME( 2004, pandpays,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Panda Pays - Mr. Cashman (0151109, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US092,    A - 01/27/04
GAME( 2001, parislgt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Paris Lights (30002611, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 670,    A - 03/05/01
GAME( 2002, parislgtu,  parislgt, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Paris Lights - Jackpot Carnival (0251031, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US022,    A - 07/03/02
GAME( 2002, pcockmag,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Peacock Magic (1J008811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014,    B - 09/05/02
GAME( 2004, pcockmagu,  pcockmag, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Peacock Magic (0151352, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB014/20,    C - 07/06/04
GAME( 2004, pelipete,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete (10177511, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778,    A - 22/11/04
GAME( 2006, pelipetejc, pelipete, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete - Jackpot Carnival (10226711, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 778/2, E - 23/05/06
GAME( 2005, pelipeteu,  pelipete, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pelican Pete (0252555, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4251,    C - 10/28/05
GAME( 2005, pennyar,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Penny Arcade (0251169, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US159,    A - 04/06/05
GAME( 2003, pengpyce,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Penguin Pays - Cash Express (0151120, US)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US055,    A- 4/14/03
GAME( 2000, pharfor,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pharaoh's Fortune (40000921, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 656,    D - 11/01/00
GAME( 2005, phxfant,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Phoenix Fantasy (20197511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 793,    A - 21/07/05
GAME( 2000, pirata6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pirates (10001511, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 661/1,    B - 3/11/00
GAME( 2004, pcfavor,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Favourites (20177311, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0002,    D - 04/11/04
GAME( 2006, pcbydem,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - By Demand (10223111, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0005,    D - 25/09/06
GAME( 2006, pcmostw,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Most Wanted (20234911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0010,    C - 09/11/06
GAME( 2010, pcgold,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Gold Edition (30291411, New Zealand)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0047,    A - 09/04/10
GAME( 2009, pcplat,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Platinum (20282111, New Zealand)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0031,    B - 04/02/09
GAME( 2004, pcorig,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's Choice - Original (30143011, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG001, C - 12/01/04
GAME( 200?, pwsuper,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Player's World - Super (0154019, US)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MG0016,    (no data)
GAME( 2001, pompeia6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (10002211, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/5,    A - 22/01/01
GAME( 2005, pompeia6u,  pompeia6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (0151158, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US161,    A - 04/15/05
GAME( 2002, pompeia6ua, pompeia6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pompeii (0251010, US)",                         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US008,    A - 04/03/02
GAME( 2005, prideaf,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Pride of Africa (10208511, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/9,    J - 06/12/05
GAME( 2004, prophecy,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Prophecy (20173411, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 734,    B - 10/08/04
GAME( 2005, qnilea6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile (3J011111, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB027,    C - 16/02/05
GAME( 2005, qnilemt2,   qnilea6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile - Money Train II (10203811, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/23, E - 17/11/05
GAME( 2001, qnilese,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition (20102811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683,    A - 06/12/01
GAME( 2001, qnilesea,   qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition (10102811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683,    A - 06/12/01
GAME( 2002, qnilejc,    qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition - Jackpot Carnival (20123911, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2002, qnilejcsp,  qnilese,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of the Nile Special Edition - Jackpot Carnival (20123911, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 683/1,    A - 21/10/02
GAME( 2003, qsheba,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Queen of Sheba (10133711, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 687,    F - 04/03/03
GAME( 2005, raprichs,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Rapid Riches (10194211, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 762,    D - 09/03/05
GAME( 2002, redbara6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Red Baron (10119911, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 709,    B - 21/11/02
GAME( 2003, redbarfl,   redbara6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Red Baron - Fast Lane (10160711, Queensland)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 709,    E - 04/12/03
// The following is clearly not a year 2000 game (an ID of #2986 would be from around 2010, note similar ID number region to buckstop/werewildq), the date string is most likely unchanged from the original MK5 20 line version which actually was released in 2000.
GAME( 201?, reelrkfl,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Reelin-n-Rockin - Fast Lane (10298611, Queensland)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/2, E - 20/12/00
GAME( 2002, retsama6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Return of the Samurai (10117211, NSW/ACT)",     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 628/10,    A - 26/08/02
GAME( 2005, roadtrip,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Road Trip (0152539, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4238,    C - 08/18/05
GAME( 2002, ruprup,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Roll Up! Roll Up!! (10115911, NSW/ACT)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642/5,    A - 22/08/02
GAME( 2002, rubymag,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Ruby Magic (10125711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698,    C - 12/11/02
GAME( 2010, rumbreel,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Rumble Reels (20288611, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 949,    B - 04/08/10
GAME( 2000, scatmag2,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic II (10001311, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 660,    C - 24/08/00
GAME( 2001, scatmag2sa, scatmag2, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic II (10102711, South Australia)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 660,    G - 15/05/01
GAME( 2006, scatmag3,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Scatter Magic III (0452569, US)",               MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4277,    (no data)
GAME( 2002, sealdeal,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Seal The Deal - Jackpot Carnival (0251029, US)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US024,    A - 06/20/02
GAME( 2006, shamnmg,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shaman's Magic (10225911, NSW/ACT)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 818,    C - 14/11/06
GAME( 200?, shamnmgu,   shamnmg,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shaman's Magic (0152647, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4318/1,    (no data)
GAME( 2004, shorthng,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Shore Thing (0251088, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US087,    B - 1/30/04
GAME( 2001, smtma6,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Show Me The Money (10001911, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657/1,    B - 08/01/01
GAME( 2001, smtma6q,    smtma6,   aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Show Me The Money (10015711, Queensland)",      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 657/1,    D - 31/05/01
GAME( 2006, silkrda6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Silk Road (0152537, US)",                       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4248/4,    D - 02/09/06
GAME( 2002, silgola6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Silver & Gold (20100721, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 672/1,    G - 08/07/02
GAME( 2008, skydnca6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sky Dancer (10272711, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB091/2,    A - 28/08/08
GAME( 1999, spgcarn,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Carnival (10000311, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642,    C - 15/11/99
GAME( 2001, spgcarnq,   spgcarn,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Carnival (10008011, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 642, E - 27/02/01
GAME( 2008, spgfest,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Spring Festival (20262311, Queensland)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB040, C - 03/06/08
GAME( 2003, stardrft,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Drifter (20150811, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 717/2,    A - 15/09/03
GAME( 2004, stardrftu,  stardrft, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Drifter (0451341, US)",                    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4204/1,    A - 02/16/04
GAME( 2001, starqst,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Star Quest (10009511, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 659/2,    B - 29/10/01
GAME( 2005, statsymb,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Status Symbols (0251147, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US146,    A - 02/10/05
GAME( 2003, sunmoon,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon (0251303, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173,    C - 04/01/03
GAME( 2003, sunmoona,   sunmoon,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon (0151303, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173,    C - 04/01/03
GAME( 200?, sunmonbb,   sunmoon,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun & Moon - Bank Buster (0153028, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4173/2,    (no data)
GAME( 2003, sunqndol,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sun Queen - Dollar Storm (0451327, US)",        MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV2070/1,    B - 11/24/03
GAME( 2005, sbuk5,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Super Bucks V (20203611, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 794,    A - 03/08/05
GAME( 2005, swhr2a6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sweethearts II (0151183, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US169,    A - 7/5/05
GAME( 2002, swh2ld,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Sweethearts II - Lucky Devil (10119811, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 712,    C - 16/10/02
GAME( 2006, tahitim,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tahiti Magic (10238911, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 765/2,    C - 07/12/06
GAME( 2002, thaiprnc,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thai Princess (10119321, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 707,    A - 17/09/02
GAME( 2003, thaiprncm,  thaiprnc, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thai Princess (30127721, Malaysia)",            MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 707,    E - 21/03/03
GAME( 2004, thndrhrt,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Thunder Heart (10173811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 758,    B - 11/05/04
GAME( 2007, tigress,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tigress (20230111, NSW/ACT)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 807,    C - 03/04/07
GAME( 2007, tikitalk,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tiki Talk (10239111, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 831,    D - 28/03/07
GAME( 2001, tiktorch,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Tiki Torch (0151003, US)",                      MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4130/4,    A - 08/10/01
GAME( 200?, timbwolf,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Timber Wolf (0154013, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4333/1,    (no data)
GAME( 2002, topshta6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Top Shot (30123211, NSW/ACT)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 714,    C - 16/12/02
GAME( 2000, torgods,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Torch of the Gods (10000411, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 602/19,    B - 09/08/00
GAME( 2001, toprdice,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Touch of Paradice (20007511, NSW/ACT)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 650/2,    A - 24/04/01
GAME( 200?, trsking,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Treasure King (0152570, US)",                   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4268,    (no data)
GAME( 2006, turtreas,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Turtle Treasure (10234711, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 819/1,    A - 06/10/06
GAME( 2002, venetian,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Venetian Nights (1J008911, NSW/ACT)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/1,    D - 05/08/02
GAME( 2004, venice,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Venice (0151355, US)",                          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB016/20,    F - 07/06/04
GAME( 2009, werewild,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Werewolf Wild (10288711, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 939,    A - 03/12/09
GAME( 2010, werewildq,  werewild, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Werewolf Wild (20290711, Queensland)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 939, C - 14/10/10
GAME( 2002, whalecsh,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (20155711, Malaysia)",           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 695,    A - 14/06/02
GAME( 2003, whalecshu,  whalecsh, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (0451311, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4174,    A - 03/31/03
GAME( 2005, whalecshua, whalecsh, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Whales of Cash (0451346, US)",                  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4214,    B - 01/14/05
GAME( 2004, wheregld,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (10124811, NSW/ACT)",          MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    B - 26/02/04
GAME( 2004, wheregldsp, wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (10124811, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    B - 26/02/04
GAME( 2004, wheregldm,  wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (20177111, Malaysia)",         MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    C - 17/06/04
GAME( 2005, wheregldq,  wheregld, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Where's The Gold (20184811, Queensland)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 756,    D - 21/06/05
GAME( 2003, whtwater,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "White Water (0151075, US)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // US054,    A - 4/11/03
GAME( 2006, wikwin2,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wicked Winnings II - Power Pay (10214011, NSW/ACT)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 796,    A - 28/02/06
GAME( 2002, wldafr,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Africa - Millioniser (0351083, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4076,    D - 5/08/02
GAME( 2006, wldcata6,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Cats (10237511, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    B - 20/06/06
GAME( 2003, wldgoos,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Goose (20131911, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 732,    B - 02/06/03
GAME( 2009, wgorilla,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Gorilla - Power Pay (20279811, NSW/ACT)",  MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // JB108/2,    B - 19/06/09
GAME( 2006, wldpanda,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Panda (20218411, NSW/ACT)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 800,    E - 07/08/06
GAME( 2004, wldpanth,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Panthers - Lil' Lucy (0651310, US)",       MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4187,    D- 01/15/04
GAME( 2006, wldstall,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Stallion (10230811, NSW/ACT)",             MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 805,    B - 12/09/06
GAME( 2007, wtepee,     aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Tepee - Power Pay (10230211, NSW/ACT)",    MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 820,    H - 20/04/07
GAME( 2006, wwaratah,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Waratah (10215911, NSW/ACT)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    A - 14/02/06
GAME( 2006, wwaratahsp, wwaratah, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Waratah (10215911, NSW/ACT, Show Program)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 811,    A - 14/02/06
GAME( 2002, wways,      aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Ways (10106911, NSW/ACT)",                 MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698,    B - 19/06/02
GAME( 2003, wwaysm,     wways,    aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wild Ways (10130111, Malaysia)",                MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 698/1,  A - 25/02/03
GAME( 2005, wingoly,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wings Over Olympus (0252529, US)",              MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4237/1,    A - 03/07/05
GAME( 2003, wzwaysll,   aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Wizard Ways - Lady Luck (20139111, NSW/ACT)",   MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 702,    H - 30/06/03
GAME( 2003, zorroa6,    aristmk6, aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Zorro (30130611, NSW/ACT)",                     MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // 743,    B - 5/09/03
GAME( 2005, zorroa6u,   zorroa6,  aristmk6, aristmk6, aristmk6_state, empty_init, ROT0, "Aristocrat", "Zorro (0151372, US)",                           MACHINE_NOT_WORKING | MACHINE_NO_SOUND ) // MV4212,    F - 05/13/05
