// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************
 Eolith 32 bits hardware: Vega system

 driver by Pierpaolo Prazzoli

 Games dumped
 - Crazy War

 TODO:
 - where are mapped the unused dip switches?

 *********************************************************************/

#include "emu.h"
#include "eolith_speedup.h"

#include "cpu/e132xs/e132xs.h"
#include "machine/at28c16.h"
#include "machine/gen_latch.h"
#include "sound/qs1000.h"

#include "speaker.h"


namespace {

class vegaeo_state : public eolith_e1_speedup_state_base
{
public:
	vegaeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: eolith_e1_speedup_state_base(mconfig, type, tag)
		, m_soundlatch(*this, "soundlatch")
		, m_qs1000(*this, "qs1000")
		, m_system_io(*this, "SYSTEM")
		, m_qs1000_bank(*this, "qs1000_bank")
	{
	}

	void vega(machine_config &config) ATTR_COLD;

	void init_vegaeo() ATTR_COLD;

protected:
	virtual void video_start() override ATTR_COLD;

private:
	required_device<generic_latch_8_device> m_soundlatch;
	required_device<qs1000_device> m_qs1000;
	required_ioport m_system_io;
	memory_bank_creator m_qs1000_bank;

	std::unique_ptr<uint8_t[]> m_vram;
	int m_vbuffer;

	void vram_w(offs_t offset, uint8_t data, uint8_t mem_mask = ~0);
	uint8_t vram_r(offs_t offset);
	void vega_misc_w(uint32_t data);
	uint32_t vegaeo_custom_read();
	void qs1000_p1_w(uint8_t data);
	void qs1000_p2_w(uint8_t data);
	void qs1000_p3_w(uint8_t data);

	uint32_t screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void vega_map(address_map &map) ATTR_COLD;
};

void vegaeo_state::qs1000_p1_w(uint8_t data)
{
}

void vegaeo_state::qs1000_p2_w(uint8_t data)
{
}

void vegaeo_state::qs1000_p3_w(uint8_t data)
{
	// .... .xxx - Data ROM bank (64kB)
	// ...x .... - ?
	// ..x. .... - /IRQ clear

	m_qs1000_bank->set_entry(data & 0x07);

	if (!BIT(data, 5))
		m_soundlatch->acknowledge_w();
}

void vegaeo_state::vram_w(offs_t offset, uint8_t data, uint8_t mem_mask)
{
	// don't write transparent pen
	if (data != 0xff)
		COMBINE_DATA(&m_vram[offset + m_vbuffer * 0x14000]);
}

uint8_t vegaeo_state::vram_r(offs_t offset)
{
	return m_vram[offset + 0x14000 * m_vbuffer];
}

void vegaeo_state::vega_misc_w(uint32_t data)
{
	// other bits ???

	m_vbuffer = data & 1;
}


uint32_t vegaeo_state::vegaeo_custom_read()
{
	speedup_read();
	return m_system_io->read();
}


void vegaeo_state::vega_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram();
	map(0x80000000, 0x80013fff).rw(FUNC(vegaeo_state::vram_r), FUNC(vegaeo_state::vram_w));
	map(0xfc000000, 0xfc0000ff).rw("at28c16", FUNC(at28c16_device::read), FUNC(at28c16_device::write)).umask32(0x000000ff);
	map(0xfc200000, 0xfc2003ff).rw("palette", FUNC(palette_device::read16), FUNC(palette_device::write16)).umask32(0x0000ffff).share("palette");
	map(0xfc400000, 0xfc40005b).nopw(); // crt registers ?
	map(0xfc600000, 0xfc600003).w(m_soundlatch, FUNC(generic_latch_8_device::write)).umask32(0x000000ff).cswidth(32);
	map(0xfca00000, 0xfca00003).w(FUNC(vegaeo_state::vega_misc_w));
	map(0xfcc00000, 0xfcc00003).r(FUNC(vegaeo_state::vegaeo_custom_read));
	map(0xfce00000, 0xfce00003).portr("P1_P2");
	map(0xfd000000, 0xfeffffff).rom().region("maindata", 0);
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0);
}

static INPUT_PORTS_START( crazywar )
	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_READ_LINE_MEMBER(FUNC(vegaeo_state::speedup_vblank_r))
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffffff00, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("P1_P2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP    ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT  ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0xffff0000, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


void vegaeo_state::video_start()
{
	eolith_e1_speedup_state_base::video_start();

	m_vram = std::make_unique<uint8_t[]>(0x14000*2);
	save_pointer(NAME(m_vram), 0x14000*2);
	save_item(NAME(m_vbuffer));
}

uint32_t vegaeo_state::screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	for (int y = cliprect.top(); y <= std::min(cliprect.bottom(), 239); y++)
	{
		auto *pix = &bitmap.pix(y);
		for (int x = 0; x < 320; x++)
			*pix++ = m_vram[0x14000 * (m_vbuffer ^ 1) + (y * 320) + x] & 0xff;
	}

	return 0;
}

void vegaeo_state::vega(machine_config &config)
{
	GMS30C2132(config, m_maincpu, XTAL(55'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &vegaeo_state::vega_map);
	TIMER(config, "scantimer").configure_scanline(FUNC(vegaeo_state::eolith_speedup), "screen", 0, 1);

	AT28C16(config, "at28c16", 0);

	/* video hardware */
	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_vblank_time(ATTOSECONDS_IN_USEC(0));
	m_screen->set_size(512, 262);
	m_screen->set_visarea(0, 319, 0, 239);
	m_screen->set_screen_update(FUNC(vegaeo_state::screen_update_vega));
	m_screen->set_palette(m_palette);

	PALETTE(config, m_palette).set_format(palette_device::xRGB_555, 256);
	m_palette->set_membits(16);

	/* sound hardware */
	SPEAKER(config, "speaker", 2).front();

	GENERIC_LATCH_8(config, m_soundlatch);
	m_soundlatch->data_pending_callback().set("qs1000", FUNC(qs1000_device::set_irq));
	m_soundlatch->set_separate_acknowledge(true);

	QS1000(config, m_qs1000, XTAL(24'000'000));
	m_qs1000->set_external_rom(true);
	m_qs1000->p1_in().set("soundlatch", FUNC(generic_latch_8_device::read));
	m_qs1000->p1_out().set(FUNC(vegaeo_state::qs1000_p1_w));
	m_qs1000->p2_out().set(FUNC(vegaeo_state::qs1000_p2_w));
	m_qs1000->p3_out().set(FUNC(vegaeo_state::qs1000_p3_w));
	m_qs1000->add_route(0, "speaker", 1.0, 0);
	m_qs1000->add_route(1, "speaker", 1.0, 1);
}

/*
Crazy Wars
Eolith

This game runs on Eolith Vega II V1.2 hardware

PCB Layout
----------

VEGA II V1.2
|-------------------------------------------------------|
|   VOL_L    QDSP      BGM.U84                          |
|   VOL_R    QS1000              61C256         61C256  |
|                                                       |
|   DA1311                       61C256         61C256  |
|                    24MHz                              |
|           EFFECT.U85           61C256         61C256  |
|                  14.31818MHz                          |
|J   QS-1001A.U86                61C256         61C256  |
|A                    EOLITH                            |
|M                    EV0514-001                        |
|M                                                  PAL |
|A   SERVICE_SW                                RESET_SW |
|                                GMS30C2132             |
|    TEST_SW    KT76C28K-10              41C16256       |
|               KT76C28K-10              41C16256  55MHz|
|                   |----------------------------|      |
|          28C16.U29|  06 04 02 00 14 12 10 08   |      |
|                   |                            |      |
|DSW2(4)            |                            |      |
|DSW1(4)            |  07 05 03 01 15 13 11 09   |      |
|                   |                            |   U7 |
|-------------------|----------------------------|------|
Notes:
      GMS30C2132 - Hyperstone CPU @ 55.0MHz
      61C256     - 32kx8 SRAM
      41C16256   - ISSI 256kx16 DRAM
      K76C28K-10 - SRAM, probably 2kx8 or 8kx8
      00 - 15    - Macronix MX29F1610M 16MBit SOP44 FlashROMs
      U7         - 27C040 EPROM
      U85        - 27C801 EPROM
      U84        - 27C4001 EPROM
      U29        - 2kx8 EEPROM
*/


ROM_START( crazywar )
	ROM_REGION( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "u7",         0x00000, 0x80000, CRC(697c2505) SHA1(c787007f05d2ddf1706e15e9d9ef9b2479708f12) )

	ROM_REGION32_BE( 0x2000000, "maindata", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
	ROM_LOAD32_WORD_SWAP( "00", 0x0000000, 0x200000, CRC(fbb917ae) SHA1(1fd975cda06b3cb748503b7c8009e6184b46af3f) )
	ROM_LOAD32_WORD_SWAP( "01", 0x0000002, 0x200000, CRC(59308556) SHA1(bc8c28531fca009be5b7b3b1a4a9b3ebcc9d3c3a) )
	ROM_LOAD32_WORD_SWAP( "02", 0x0400000, 0x200000, CRC(34813167) SHA1(d04c71164b36af78425dcd637e60aee45c39a1ba) )
	ROM_LOAD32_WORD_SWAP( "03", 0x0400002, 0x200000, CRC(7fcb0a53) SHA1(f74e0512b5d4854d0c4b04bf8c917f8dccb4dc0f) )
	ROM_LOAD32_WORD_SWAP( "04", 0x0800000, 0x200000, CRC(f8eb8ce5) SHA1(a631f6979a9df2fda622483256ea569c6b4d1586) )
	ROM_LOAD32_WORD_SWAP( "05", 0x0800002, 0x200000, CRC(14d854df) SHA1(5527fb1a12193e27a3fad5ca7f4e3027f462ee50) )
	ROM_LOAD32_WORD_SWAP( "06", 0x0c00000, 0x200000, CRC(31c67f0a) SHA1(7a587bb86bc6450c66016c82efe047f2d350d586) )
	ROM_LOAD32_WORD_SWAP( "07", 0x0c00002, 0x200000, CRC(dddf93d2) SHA1(c982f18c4bd242885a6150252c9c2fa4a07ebf4b) )
	ROM_LOAD32_WORD_SWAP( "08", 0x1000000, 0x200000, CRC(dc37bcb9) SHA1(144050056905e3dce08795d1a4ac17a45f2a1fec) )
	ROM_LOAD32_WORD_SWAP( "09", 0x1000002, 0x200000, CRC(86ba59cc) SHA1(566cc6527188e24a6eae4a64131deca7e2140ada) )
	ROM_LOAD32_WORD_SWAP( "10", 0x1400000, 0x200000, CRC(524bf126) SHA1(85a27a74ba4caaf3ab1e1f0e8e8b516bb0182ae7) )
	ROM_LOAD32_WORD_SWAP( "11", 0x1400002, 0x200000, CRC(613b2764) SHA1(7a7c85c8cf1cba74e2e98a3b77d5ea44bb76a563) )
	ROM_LOAD32_WORD_SWAP( "12", 0x1800000, 0x200000, CRC(3c81d117) SHA1(76d6728f8c55e68c84d68ff2f242684bde30f4dd) )
	ROM_LOAD32_WORD_SWAP( "13", 0x1800002, 0x200000, CRC(b86545a0) SHA1(4aaa23c37d776647f3288ba541cefa79ddbd962d) )
	ROM_LOAD32_WORD_SWAP( "14", 0x1c00000, 0x200000, CRC(38ede322) SHA1(9496685a1280885a61a568047c4a8c2cd70d1b83) )
	ROM_LOAD32_WORD_SWAP( "15", 0x1c00002, 0x200000, CRC(d35e630a) SHA1(8c220f1baddd39cc978e3e5a874cc58e78b74c62) )

	ROM_REGION( 0x080000, "qs1000:cpu", 0 )  /* QDSP (8052) Code */
	ROM_LOAD( "bgm.u84",      0x000000, 0x080000, CRC(13aa7778) SHA1(131f74e1b73dd7a7038864593dc7ca24af0ffc30) )

	ROM_REGION( 0x1000000, "qs1000", 0 )
	ROM_LOAD( "effect.u85",   0x000000, 0x100000, CRC(9159fcc6) SHA1(2be9a197a51303a0da9484dced12a3f6d3b0d867) )
	ROM_LOAD( "qs1001a.u86",  0x200000, 0x080000, CRC(d13c6407) SHA1(57b14f97c7d4f9b5d9745d3571a0b7115fbe3176) )
ROM_END

void vegaeo_state::init_vegaeo()
{
	// Set up the QS1000 program ROM banking, taking care not to overlap the internal RAM
	m_qs1000->cpu().space(AS_IO).install_read_bank(0x0100, 0xffff, m_qs1000_bank);
	m_qs1000_bank->configure_entries(0, 8, memregion("qs1000:cpu")->base()+0x100, 0x10000);

	init_speedup();
}

} // anonymous namespace


GAME( 2002, crazywar, 0, vega, crazywar, vegaeo_state, init_vegaeo, ROT0, "Eolith", "Crazy War", MACHINE_SUPPORTS_SAVE )
