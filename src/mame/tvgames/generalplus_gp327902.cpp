// license:BSD-3-Clause
// copyright-holders:David Haywood

#include "emu.h"

#include "cpu/arm7/arm7.h"
#include "machine/timer.h"

#include "screen.h"
#include "speaker.h"

#include "multibyte.h"


namespace {

class generalplus_gp327902_game_state : public driver_device
{
public:
	generalplus_gp327902_game_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_screen(*this, "screen")
	{ }

	void gp327902(machine_config &config) ATTR_COLD;

	void init_spi() ATTR_COLD;

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	void arm_map(address_map &map) ATTR_COLD;

	required_device<cpu_device> m_maincpu;
	required_device<screen_device> m_screen;

	uint32_t screen_update_gp327902(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

private:
	TIMER_DEVICE_CALLBACK_MEMBER(timer);

	uint32_t c0020070_unk_r() { return machine().rand(); }
	void c0060000_unk_w(uint32_t data);
	uint32_t c008000c_unk_r() { return machine().rand(); }
	uint32_t d000003c_unk_r() { return 0xffffffff; }

	int m_copybase;
	int m_copylength;
	int m_copydest;
};


void generalplus_gp327902_game_state::c0060000_unk_w(uint32_t data)
{
	/*

	this is some kind of debug serial output, it currently outputs (if copy_lowest_block is set to false) the following sequence

	adc_init
	DAC Task Create[0]
	DAC BG Task Create[0]
	Audio Task Create[4]
	Audio BG Task Create[6]
	FileServ Task Create[8]
	Image Task Create[10]
	audio_init()
	watch-dog enable
	power on

	with copy_lowest_block as true you get

	GP DV BootLoader v2.2 Entry @ 0 MHz
	GPDV  chip detect

	*/
	//printf("%c", data & 0xff);
}

void generalplus_gp327902_game_state::arm_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram(); // 16M-bit internal SDRAM

	map(0xc0020070, 0xc0020073).r(FUNC(generalplus_gp327902_game_state::c0020070_unk_r));
	map(0xc0060000, 0xc0060003).w(FUNC(generalplus_gp327902_game_state::c0060000_unk_w));
	map(0xc008000c, 0xc008000f).r(FUNC(generalplus_gp327902_game_state::c008000c_unk_r));
	map(0xd000003c, 0xd000003f).r(FUNC(generalplus_gp327902_game_state::d000003c_unk_r));

	map(0xf8000000, 0xf80003ff).ram(); // writes pointers used by exceptions (including IRQs) here
}

TIMER_DEVICE_CALLBACK_MEMBER( generalplus_gp327902_game_state::timer )
{
	m_maincpu->set_input_line(arm7_cpu_device::ARM7_FIRQ_LINE, HOLD_LINE);
}

uint32_t generalplus_gp327902_game_state::screen_update_gp327902(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void generalplus_gp327902_game_state::machine_start()
{
}

void generalplus_gp327902_game_state::machine_reset()
{
	// perform some kind of bootstrap likely done by an internal ROM
	uint8_t *spirom = memregion("spi")->base();
	address_space &mem = m_maincpu->space(AS_PROGRAM);

	for (int i = 0; i < m_copylength / 2; i++)
	{
		uint16_t word = get_u16le(&spirom[m_copybase + (i * 2)]);
		mem.write_word(m_copydest + (i * 2), word);
	}

	m_maincpu->set_state_int(arm7_cpu_device::ARM7_R15, m_copydest);
}

static INPUT_PORTS_START( gp327902 )
INPUT_PORTS_END

void generalplus_gp327902_game_state::gp327902(machine_config &config)
{
	ARM9(config, m_maincpu, 240'000'000); // unknown core / frequency, but ARM based
	m_maincpu->set_addrmap(AS_PROGRAM, &generalplus_gp327902_game_state::arm_map);

	SCREEN(config, m_screen, SCREEN_TYPE_RASTER);
	m_screen->set_refresh_hz(60);
	m_screen->set_size(320, 262);
	m_screen->set_visarea(0, 320-1, 0, 240-1);
	m_screen->set_screen_update(FUNC(generalplus_gp327902_game_state::screen_update_gp327902));

	SPEAKER(config, "speaker", 2).front();

	TIMER(config, "timer").configure_periodic(FUNC(generalplus_gp327902_game_state::timer), attotime::from_hz(1000));
}

void generalplus_gp327902_game_state::init_spi()
{
	const bool copy_lowest_block = true; // change this to false to copy a later part of the bootstrap sequence on sanpetx?

	// these likely come from the header at the start of the ROM
	if (copy_lowest_block)
	{
		// all dumped sets have a block here
		m_copybase = 0x800;
		m_copydest = 0x1f8000;
		m_copylength = 0x2800;
	}
	else
	{
		// smksagas has the main block here, which is likely copied to RAM by previous parts of the bootloader above
		m_copybase = 0x16000;
		m_copydest = 0x0;
		m_copylength = 0x200000;
	}
}

ROM_START( smksagas )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "25l64.u1", 0x0000, 0x800000, CRC(f28b9fd3) SHA1(8ed4668f271cbe01065bc0836e49ce70faf10834) )
ROM_END

ROM_START( smksagasa ) // code is the same, some data area differs, could be different factory defaults, or user data, remove later if redundant
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l64.u1", 0x0000, 0x800000, CRC(5dee73ea) SHA1(ff0302a479f0a1a0a6dc605e18f6389f6244922f) )
ROM_END

ROM_START( smkmikke ) // DX version
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l6403f.u1", 0x0000, 0x800000, CRC(cb5dc7b6) SHA1(425c4d01b56784278b77824a354d9efa46e1a74e) )
ROM_END

ROM_START( smkmikkea ) // different code revision (non-DX)
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l64.u1", 0x0000, 0x800000, CRC(af900ab7) SHA1(c2cb0d37acf94edd150e6ef4f987f66b2306b97e) )
ROM_END


ROM_START( tomyegg )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "gpr25l6403f.u1", 0x0000, 0x800000, CRC(2acd6752) SHA1(85e59546a1af4618c75c275cead7ef0f5e3faa44) )
ROM_END

ROM_START( chikawac )
	ROM_REGION(  0x800000, "spi", ROMREGION_ERASE00 )
	ROM_LOAD( "xm25qh64c.u1", 0x0000, 0x800000, CRC(88c984aa) SHA1(6e176960b64fc3576efaa40dfe2ff0a6dcea3c3f) )
ROM_END

} // anonymous namespace

// Tomy / San-X devices

// dates for each of these taken from back of case, are the DX versions different software or just different accessories?

// 2018 version is a square device - Sumikko Gurashi - Sumikko Atsume (すみっコぐらし すみっコあつめ)
// see evolution_handheld.cpp

// 2019 version is house shaped device - すみっコぐらし すみっコさがし
CONS( 2019, smksagas,         0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Sagashi (Japan, set 1)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // from a standard unit
CONS( 2019, smksagasa,        smksagas, 0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Sagashi (Japan, set 2)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // from a 'Deluxe' unit
// or Sumikko Gurashi - Sumikko Sagashi DX (すみっコぐらし すみっコさがしDX "Sumikko Gurashi the movie" alt version)

// 2020 version - Sumikko Gurashi - Sumikko Catch, see generalplus_gpl16250_spi_direct.cpp

// 2021 version is a square device with a tiny 'mole' figure on top - すみっコぐらし すみっコみっけDX
// or Sumikko Gurashi - Sumikko Mikke (すみっコぐらし すみっコみっけ)
CONS( 2021, smkmikke,        0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Mikke DX (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // from a 'Deluxe' unit
CONS( 2021, smkmikkea,       smkmikke, 0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "San-X / Tomy",        "Sumikko Gurashi - Sumikko Mikke (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING) // from a standard unit

// other devices on the same Soc

// キラッとプリ☆チャン プリたまGO ミスティパープル
CONS( 2019, tomyegg,         0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "Tomy",        "Kiratto Pri-Chan - PritamaGO: Misty Purple (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
// these also exist, are they the same software or different versions?
// Powder Pink (パウダーピンク)
// Mint Blue (ミントブルー).

CONS( 2021, chikawac,        0,        0,      gp327902, gp327902, generalplus_gp327902_game_state, init_spi,  "Tomy",        "Chiikawa Camera De Ya-! (Japan)",   MACHINE_NO_SOUND | MACHINE_NOT_WORKING)
