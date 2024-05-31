// license:LGPL-2.1+
// copyright-holders:Angelo Salese
/***************************************************************************************************************************

    Seibu CATS E-Touch Mahjong Series (c) 2001 Seibu Kaihatsu

    TODO:
    - Verify OBJ ROMs (maybe bad or wrong decryption);
    - DVD drive (Toshiba SD-B100);
    - RS-232 hookup for touchscreen;
    - RTC and NVRAM;
    - mahjong keyboard inputs (and JAMMA adapter for some games);
    - emulate YMF721-S or at least do something about MIDI sound;
    - verify interrupt table;
    - verify coin inputs;
    - Any other port lingering in the 0x400-0x7ff area?

===========================================================================================================================

    CPU and system control devices:
    - Intel i386DX (U0169; lower right corner)
    - SEI600 SB08-1513 custom DMA chip (U0154; above i386DX)
    - Xilinx XC9536 CPLDs "DVDMJ11" (U0235), "DVDMJ12" (U0236), XC9572 "DVDMJ13" (U1024)

    Graphics:
    - RISE11 custom sprite chip (U0336; upper right corner)
    - NEC UPC1830GT filter video chroma (U0935; towards lower left)
    - JRC 2246 video switch x3 (U1015, U1017, U1022; near bottom, above more batteries)
    - LM1881 Video Sync Separator (U1020)
    - There is no tilemap hardware. The SEI600 tilemap DMA channel does not appear to have been reused for anything.

    RAM:
    - RAM area A (U0230, U0231; between SEI600 and RISE11) = either 2x Toshiba TC551664BJ-15 or 2x Winbond W26010AJ-15.
    - RAM area B (U067, U062, U0326, U0327; to left of RISE11) = either 4x G-Link GLT725608-15J3 or 4x Cypress CY7C199-15VC (Ver1.3 board).
    - RAM area C (U0727, U0728) = either 2x BSI B562LV1024SC-70 or 1x Hitachi HM628512ALFP-7.
      The latter leaves U0727 unpopulated, revealing a label for a MX23C8000M mask ROM. There is no onboard ROM.

    ROM:
    - Program ROMs: MX27C40000C-12 or MBM27C4001-12Z or TMS27C040-10 x4 (U011, U015-U017 = "PRG0-PRG3" on ROM board)
    - Sprite ROMs: MX29F8100MC-12 or "MX29F1610" x4 (U0231-U0234 = "OBJ1-OBJ4" on ROM board).
      Only three ROMs appear to be populated on any game.
      This means sprites should be 6bpp, even though they could potentially have been 8bpp.

    EEPROM/NVRAM:
    - ST93C46AF Serial EEPROM (U0512; towards left center of board)
    - Toshiba TC55257DFL-70L (U0144 on ROM board) with Maxell CR2032 battery (BT011 on ROM board).
      The ROM board type used by Marumie Network lacks NVRAM and RTC;
      their locations are not populated on Pakkun Ball TV.

    RTC:
    - JRC 6355E/NJU6355 Real Time Clock (U0513, above YMF721)
    - JRC 6355E/NJU6355 Real Time Clock (U0150 on ROM board)

    Serial ports:
    - NEC uPD71051GB USART x2 (U1133, U1134; lined up with DB9 ports)
    - MAXZ32 Serial Line Driver x2 (U1138, U1141; between USARTs and DB9 ports)
    - Two DB9 ports, one marked "DVD" and the other "Touch Panel."
    The latter also uses a separate 2-pin Molex power connector (CN114).

    Sound and linear miscellany:
    - Yamaha YMF721-S General MIDI OPL4-ML2 (U0274; to right of USARTs)
    - Yamaha YMZ280B-F PCMD8 (U0722; middle of board)
    - Yamaha YAC516-M DAC x2 (U0848, U085; below first row of batteries)
    - JRC 4560 operational amplifier x2 (U0850, U087)
    - Toshiba TA7252AP power amplifier (U0847; at left edge, next to AUDIO-IN)

***************************************************************************************************************************/

#include "emu.h"
#include "seibuspi.h"

#include "cpu/i386/i386.h"
//#include "bus/rs232/rs232.h"
#include "machine/i8251.h"
//#include "machine/microtch.h"
//#include "machine/rtc4543.h"
#include "seibuspi_m.h"
#include "sound/ymz280b.h"
#include "screen.h"
#include "speaker.h"


namespace {

class seibucats_state : public seibuspi_state
{
public:
	seibucats_state(const machine_config &mconfig, device_type type, const char *tag)
		: seibuspi_state(mconfig, type, tag)
//        m_key(*this, "KEY.%u", 0)
	{
	}

	void seibucats(machine_config &config);

	void init_seibucats();

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// screen updates
//  u32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
//  IRQ_CALLBACK_MEMBER(spi_irq_callback);
//  INTERRUPT_GEN_MEMBER(spi_interrupt);

	u16 input_mux_r();
	void input_select_w(u16 data);
	void output_latch_w(u16 data);
	void aux_rtc_w(u16 data);

	void seibucats_map(address_map &map);

	u16 m_input_select = 0;

//  optional_ioport_array<5> m_key;
//  optional_ioport m_special;
};

void seibucats_state::video_start()
{
	m_video_dma_length = 0;
	m_video_dma_address = 0;
	m_layer_enable = 0;
	m_layer_bank = 0;
	m_rf2_layer_bank = 0;
	m_rowscroll_enable = false;
	set_layer_offsets();

	m_tilemap_ram_size = 0;
	m_palette_ram_size = 0x4000; // TODO : correct?
	m_sprite_ram_size = 0x2000; // TODO : correct?
	m_sprite_bpp = 6; // see above

	m_tilemap_ram = nullptr;
	m_palette_ram = make_unique_clear<u32[]>(m_palette_ram_size/4);
	m_sprite_ram = make_unique_clear<u16[]>(m_sprite_ram_size/2);

	m_palette->basemem().set(&m_palette_ram[0], m_palette_ram_size, 32, ENDIANNESS_LITTLE, 2);

	memset(m_alpha_table, 0, 0x2000); // TODO : no alpha blending?

	register_video_state();
}

// identical to EJ Sakura
u16 seibucats_state::input_mux_r()
{
	u16 ret = m_special->read();

	// multiplexed inputs
	for (int i = 0; i < 5; i++)
		if (BIT(m_input_select, i))
			ret &= m_key[i]->read();

	return ret;
}

void seibucats_state::input_select_w(u16 data)
{
	// Note that this is active high in ejsakura but active low here
	m_input_select = data ^ 0xffff;
}

void seibucats_state::output_latch_w(u16 data)
{
	m_eeprom->di_write(BIT(data, 15));
	m_eeprom->clk_write(BIT(data, 14) ? ASSERT_LINE : CLEAR_LINE);
	m_eeprom->cs_write(BIT(data, 13) ? ASSERT_LINE : CLEAR_LINE);
}

void seibucats_state::aux_rtc_w(u16 data)
{
}

void seibucats_state::seibucats_map(address_map &map)
{
	// TODO: map devices
	map(0x00000000, 0x0003ffff).ram().share(m_mainram);

	map(0x00000010, 0x00000010).r(FUNC(seibucats_state::spi_status_r));
	map(0x00000400, 0x00000401).w(FUNC(seibucats_state::input_select_w));
	map(0x00000404, 0x00000405).w(FUNC(seibucats_state::output_latch_w));
	map(0x00000484, 0x00000487).w(FUNC(seibucats_state::palette_dma_start_w));
	map(0x00000490, 0x00000493).w(FUNC(seibucats_state::video_dma_length_w));
	map(0x00000494, 0x00000497).w(FUNC(seibucats_state::video_dma_address_w));
	map(0x00000562, 0x00000563).w(FUNC(seibucats_state::sprite_dma_start_w));

	map(0x00000600, 0x00000607).r(FUNC(seibucats_state::input_mux_r)).umask32(0x0000ffff);

	map(0x00200000, 0x003fffff).rom().region("ipl", 0).nopw(); // emjjoshi attempts to write there?
	// following are likely to be Seibu CATS specific
	map(0x01200000, 0x01200007).rw("ymz", FUNC(ymz280b_device::read), FUNC(ymz280b_device::write)).umask32(0x000000ff);
	map(0x01200100, 0x01200107).nopw(); // YMF721-S MIDI data
	map(0x01200104, 0x01200107).nopr(); // YMF721-S MIDI status
	map(0x01200200, 0x01200207).rw("usart1", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask32(0x000000ff);
	map(0x01200300, 0x01200307).rw("usart2", FUNC(i8251_device::read), FUNC(i8251_device::write)).umask32(0x000000ff);
	map(0xa0000000, 0xa1ffffff).noprw(); // NVRAM on ROM board
	map(0xa2000000, 0xa2000001).w(FUNC(seibucats_state::aux_rtc_w));
	map(0xffe00000, 0xffffffff).rom().region("ipl", 0);
}

static INPUT_PORTS_START( spi_mahjong_keyboard )
	PORT_START("KEY.0")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.1")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.2")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0xffffffe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.3")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0xffffffc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY.4")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_SERVICE_NO_TOGGLE( 0x00000200, IP_ACTIVE_LOW)
	PORT_BIT( 0xfffffdc0, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END


static INPUT_PORTS_START( seibucats )
	PORT_INCLUDE( spi_mahjong_keyboard )

	PORT_START("SPECIAL")
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00004000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0xffffbf3f, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

static const gfx_layout sys386f_spritelayout =
{
	16,16,
	RGN_FRAC(1,4),
	8,
	{ 0, 8, RGN_FRAC(1,4)+0, RGN_FRAC(1,4)+8, RGN_FRAC(2,4)+0, RGN_FRAC(2,4)+8, RGN_FRAC(3,4)+0, RGN_FRAC(3,4)+8 },
	{ STEP8(7,-1), STEP8(8*2+7,-1) },
	{ STEP16(0,8*4) },
	16*32
};


static GFXDECODE_START( gfx_seibucats )
	GFXDECODE_ENTRY( "sprites", 0, sys386f_spritelayout, 0, 64 )
GFXDECODE_END


void seibucats_state::machine_start()
{
	save_item(NAME(m_input_select));
}

void seibucats_state::machine_reset()
{
}

#if 0
// do not remove, might be needed for the DVD stuff (unchecked)
INTERRUPT_GEN_MEMBER(seibucats_state::spi_interrupt)
{
	device.execute().set_input_line(0, HOLD_LINE); // where is ack?
}

IRQ_CALLBACK_MEMBER(seibucats_state::spi_irq_callback)
{
	return 0x20;
}
#endif

void seibucats_state::seibucats(machine_config &config)
{
	// TBD, assume same as Seibu SPI
	constexpr XTAL MAIN_CLOCK = 50_MHz_XTAL / 2;
	constexpr XTAL PIXEL_CLOCK = 28.636363_MHz_XTAL / 4;

	constexpr u16 SPI_HTOTAL  = 448;
	constexpr u16 SPI_HBEND   = 0;
	constexpr u16 SPI_HBSTART = 320;

	constexpr u16 SPI_VTOTAL  = 296;
	constexpr u16 SPI_VBEND   = 0;
	constexpr u16 SPI_VBSTART = 240; // actually 253, but visible area is 240 lines

	// basic machine hardware
	I386(config, m_maincpu, MAIN_CLOCK);
	m_maincpu->set_addrmap(AS_PROGRAM, &seibucats_state::seibucats_map);
	m_maincpu->set_vblank_int("screen", FUNC(seibuspi_state::spi_interrupt));
	m_maincpu->set_irq_acknowledge_callback(FUNC(seibuspi_state::spi_irq_callback));

	EEPROM_93C46_16BIT(config, "eeprom");

	//JRC6355E(config, m_rtc, XTAL(32'768));

	I8251(config, "usart1", 0);
	I8251(config, "usart2", 0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
//  screen.set_screen_update(FUNC(seibucats_state::screen_update));
	screen.set_screen_update(FUNC(seibuspi_state::screen_update_sys386f));
	screen.set_raw(PIXEL_CLOCK, SPI_HTOTAL, SPI_HBEND, SPI_HBSTART, SPI_VTOTAL, SPI_VBEND, SPI_VBSTART);

	PALETTE(config, m_palette, palette_device::BLACK, 8192);

	SEI25X_RISE1X(config, m_spritegen, 0, m_palette, gfx_seibucats);
	m_spritegen->set_screen("screen");
	// see above
	m_spritegen->set_pix_raw_shift(6);
	m_spritegen->set_pri_raw_shift(14);
	m_spritegen->set_transpen(63);

	// sound hardware
	SPEAKER(config, "lspeaker").front_left();
	SPEAKER(config, "rspeaker").front_right();

	ymz280b_device &ymz(YMZ280B(config, "ymz", XTAL(16'384'000)));
	ymz.add_route(0, "lspeaker", 1.0);
	ymz.add_route(1, "rspeaker", 1.0);
}


/***************************************************************************

  Machine driver(s)

***************************************************************************/

#define SEIBUCATS_OBJ_LOAD \
	ROM_REGION( 0x400000, "sprites", ROMREGION_ERASE00) \
/*  obj4.u0234 empty slot */ \
	ROM_LOAD16_WORD_SWAP("obj03.u0232", 0x100000, 0x100000, BAD_DUMP CRC(15c230cf) SHA1(7e12871d6e34e28cd4b5b23af6b0cbdff9432500)  ) \
	ROM_LOAD16_WORD_SWAP("obj02.u0233", 0x200000, 0x100000, BAD_DUMP CRC(dffd0114) SHA1(b74254061b6da5a2ce310ea89684db430b43583e)  ) \
	ROM_LOAD16_WORD_SWAP("obj01.u0231", 0x300000, 0x100000, BAD_DUMP CRC(ee5ae0fd) SHA1(0baff6ca4e8bceac4e09732da267f57578dcc280)  )


ROM_START( emjjoshi )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) // i386 program
	ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(e69bed6d) SHA1(e9626e704c5d28419cfa6a7a2c1b13b4b46f941c) )
	ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(1082ede1) SHA1(0d1a682f37ede5c9070c14d1c3491a3082ad0759) )
	ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(df85a8f7) SHA1(83226767b0c33e8cc3baee6f6bb17e4f1a6c9c27) )
	ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(6fe7fd41) SHA1(e7ea9cb83bdeed4872f9e423b8294b9ca4b29b6b) )

	SEIBUCATS_OBJ_LOAD

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "at the girls dorm sktp-10002", 0, SHA1(be47c105089d6ef4ce05a6e1ba2ec7a3101015dc) )
ROM_END


// MJ1-1537
ROM_START( emjscanb )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) // i386 program
	ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(6e5c7c16) SHA1(19c00833357b97d0ed91a962e95d3ae2582da66c) )
	ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(a5a17fdd) SHA1(3295ecb1055cf1ab612eb915aabe8d2895aeca6a) )
	ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(b89d7693) SHA1(174b2ecfd8a3c593a81905c1c9d62728f710f5d1) )
	ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(6b38a07b) SHA1(2131ae726fc38c8054801c1de4d17eec5b55dd2d) )

	SEIBUCATS_OBJ_LOAD

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "scandal blue sktp-10008", 0, SHA1(17fe67698a9bc5dbd452c4b1afa739294ec2011c) )
ROM_END

ROM_START( emjtrapz )
	ROM_REGION32_LE( 0x200000, "ipl", 0 ) // i386 program
	ROM_LOAD32_BYTE( "prg0.u016",    0x000000, 0x080000, CRC(88e4ef2a) SHA1(110451c09983ce4720f75b89282ca49f47169a85) )
	ROM_LOAD32_BYTE( "prg1.u011",    0x000001, 0x080000, CRC(e4716996) SHA1(6abd84c1e4facf6570988db0a63968a1647144b1) )
	ROM_LOAD32_BYTE( "prg2.u017",    0x000002, 0x080000, CRC(69995273) SHA1(a7e10d21a524a286acd0a8c19c41a101eee30626) )
	ROM_LOAD32_BYTE( "prg3.u015",    0x000003, 0x080000, CRC(99f86a19) SHA1(41deb5eb78c0a675da7e1b1bbd5c440e157c7a25) )

	SEIBUCATS_OBJ_LOAD

	DISK_REGION("dvd")
	DISK_IMAGE_READONLY( "trap zone sktp-00009", 0, SHA1(b4a51f42eeaeefc329031651859caa108418a96e) )
ROM_END

void seibucats_state::init_seibucats()
{
	u16 *src = (u16 *)memregion("sprites")->base();
	u16 tmp[0x40 / 2], offset;

	// sprite_reorder() only
	for (int i = 0; i < memregion("sprites")->bytes() / 0x40; i++)
	{
		memcpy(tmp, src, 0x40);

		for (int j = 0; j < 0x40 / 2; j++)
		{
			offset = (j >> 1) | (j << 4 & 0x10);
			*src++ = tmp[offset];
		}
	}
//  seibuspi_rise11_sprite_decrypt_rfjet(memregion("sprites")->base(), 0x300000);
}

} // anonymous namespace


// Gravure Collection
// Pakkun Ball TV
/* 01 */ // Mahjong Shichau zo!
/* 02 */ GAME( 1999, emjjoshi,  0,   seibucats,  seibucats, seibucats_state, init_seibucats, ROT0, "Seibu Kaihatsu / CATS", "E-Touch Mahjong Series #2: Joshiryou de NE! (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 03 */ // Lingerie DE Ikou
/* 04 */ // Marumie Network
/* 05 */ // BINKAN Lips
/* 06 */ GAME( 2001, emjscanb,  0,   seibucats,  seibucats, seibucats_state, init_seibucats, ROT0, "Seibu Kaihatsu / CATS", "E-Touch Mahjong Series #6: Scandal Blue - Midara na Daishou (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 07 */ GAME( 2001, emjtrapz,  0,   seibucats,  seibucats, seibucats_state, init_seibucats, ROT0, "Seibu Kaihatsu / CATS", "E-Touch Mahjong Series #7: Trap Zone - Yokubou no Kaisoku Densha (Japan)", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
/* 08 */ // Poison
/* 09 */ // Nurse Call
/* 10 */ // Secret Love
/* 11 */ // Venus Shot
/* 12 */ // Platina Selection
/* 13 */ // Gal Jong
/* 14 */ // Yakin Jantou
/* 15 */ // Collector
/* 16 */ // Digicute
/* 17 */ // Gal Jong 2
/* 18 */ // Midnight Lovers
/* 19 */ // Sexual
/* 20 */ // Gekisha!
/* 21 */ // Fetish Navi
/* 22 */ // Venus On Line / Beauty On Line
/* 23 */ // Nurse Mania
/* 24 */ // Sexy Beach
/* 25 */ // Oshioki
/* 26 */ // Private Eyes
/* 27 */ // Gal Jong Kakutou Club
/* 28 */ // BINKAN Lips Plus
