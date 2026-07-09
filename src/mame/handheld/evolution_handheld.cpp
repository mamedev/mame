// license:BSD-3-Clause
// copyright-holders:David Haywood, AJR

#include "emu.h"

#include "cpu/sonix16/sonix16.h"

#include "screen.h"
#include "speaker.h"


namespace {

class evolution_handheldgame_state : public driver_device
{
public:
	evolution_handheldgame_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_program_ram(*this, "program_ram"),
		m_dmac_params(*this, "dmac_params")
	{ }

	void evolhh(machine_config &config) ATTR_COLD;
	void smkatsum(machine_config &config) ATTR_COLD;
	void yuleyuan(machine_config &config) ATTR_COLD;
	void udrive(machine_config &config) ATTR_COLD;

	void init_yuleyuan();

private:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;

	u32 screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);

	required_device<sonix16_device> m_maincpu;
	optional_shared_ptr<u16> m_program_ram;
	optional_shared_ptr<u16> m_dmac_params;

	u16 dma_status_r();
	void dma_control_w(u16 data);

	void evolution_map(address_map &map) ATTR_COLD;
	void evolution_ram_map(address_map &map) ATTR_COLD;
	void snc7001a_map(address_map &map) ATTR_COLD;
	void smkatsum_ram_map(address_map &map) ATTR_COLD;
	void snc7648s_map(address_map &map) ATTR_COLD;
	void yuleyuan_ram_map(address_map &map) ATTR_COLD;
	void udrive_map(address_map &map) ATTR_COLD;
	void udrive_ram_map(address_map &map) ATTR_COLD;
};

void evolution_handheldgame_state::machine_start()
{
}

void evolution_handheldgame_state::machine_reset()
{
	if (m_program_ram.found())
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);
		for (unsigned i = 0; i < m_program_ram.length(); i++)
			m_program_ram[i] = space.read_word(0x400000 + i);
	}
}

u16 evolution_handheldgame_state::dma_status_r()
{
	// bit 0 = DMA busy
	return 0;
}

void evolution_handheldgame_state::dma_control_w(u16 data)
{
	if (BIT(data, 0))
	{
		address_space &space = m_maincpu->space(AS_PROGRAM);

		u32 src = u32(m_dmac_params[0]) << 16 | m_dmac_params[1];
		u32 dst = u32(m_dmac_params[2]) << 16 | m_dmac_params[3];
		u16 count = m_dmac_params[4];

		logerror("%s: DMA from %06X-%06X to %06X-%06X\n", machine().describe_context(), src, src + count, dst, dst + count);

		do
		{
			space.write_word(dst++, space.read_word(0x400000 + src++));
		} while (count-- != 0);
	}
}

static INPUT_PORTS_START( evolhh )
INPUT_PORTS_END


u32 evolution_handheldgame_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	return 0;
}

void evolution_handheldgame_state::evolution_map(address_map &map)
{
	map(0x000000, 0x00ffff).rom().region("bootstrap", 0);
	map(0x400000, 0x5fffff).rom().region("maincpu", 0);
}

void evolution_handheldgame_state::evolution_ram_map(address_map &map)
{
	map(0x000000, 0x0003ff).ram();
	map(0x00a000, 0x00bfff).ram();
	map(0x200000, 0x207fff).ram();
	map(0x400000, 0x5fffff).rom().region("maincpu", 0);
}

void evolution_handheldgame_state::snc7001a_map(address_map &map)
{
	map(0x000000, 0x007fff).ram().share(m_program_ram); // "boot from external flash, only one time after IC reset"
	map(0x200000, 0x201fff).ram(); // "boot from external flash, update anytime by user program" (tomyspt, hoppech)
	map(0x400000, 0x7fffff).rom().region("maincpu", 0);
}

void evolution_handheldgame_state::smkatsum_ram_map(address_map &map)
{
	map(0x000000, 0x003fff).ram();
	map(0x00f900, 0x00f902).noprw();
	map(0x00fe27, 0x00fe2b).writeonly().share(m_dmac_params);
	map(0x00fe2c, 0x00fe2c).rw(FUNC(evolution_handheldgame_state::dma_status_r), FUNC(evolution_handheldgame_state::dma_control_w));
}

void evolution_handheldgame_state::snc7648s_map(address_map &map)
{
	map(0x000000, 0x00bfff).ram().share(m_program_ram); // "boot from external flash, only one time after IC reset"
	map(0x200000, 0x2007ff).ram();
	map(0x400000, 0x7fffff).rom().region("maincpu", 0);
}

void evolution_handheldgame_state::yuleyuan_ram_map(address_map &map)
{
	map(0x000000, 0x001fff).ram();
	map(0x00fe27, 0x00fe2b).writeonly().share(m_dmac_params);
	map(0x00fe2c, 0x00fe2c).rw(FUNC(evolution_handheldgame_state::dma_status_r), FUNC(evolution_handheldgame_state::dma_control_w));
}

void evolution_handheldgame_state::udrive_map(address_map &map)
{
	map(0x000000, 0x000001).rom().region("maincpu", 0); // bootstrap or mirror?
	map(0x400000, 0x7fffff).rom().region("maincpu", 0);
}

void evolution_handheldgame_state::udrive_ram_map(address_map &map)
{
	map(0x000000, 0x0037ff).ram();
	map(0x005000, 0x0052ff).ram();
	map(0x005800, 0x0059ff).ram();
	map(0x006000, 0x0063ff).ram();
	map(0x007000, 0x0071ff).ram();
}


void evolution_handheldgame_state::evolhh(machine_config &config)
{
	SNL320(config, m_maincpu, XTAL(16'000'000));
	m_maincpu->set_addrmap(AS_PROGRAM, &evolution_handheldgame_state::evolution_map);
	m_maincpu->set_addrmap(AS_DATA, &evolution_handheldgame_state::evolution_ram_map);

	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(256, 256); // resolution not confirmed
	screen.set_visarea(0, 256-1, 0, 256-1);
	screen.set_screen_update(FUNC(evolution_handheldgame_state::screen_update));

	SPEAKER(config, "speaker").front_center();
}

void evolution_handheldgame_state::smkatsum(machine_config &config)
{
	evolhh(config);

	SNC7001A(config.replace(), m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &evolution_handheldgame_state::snc7001a_map);
	m_maincpu->set_addrmap(AS_DATA, &evolution_handheldgame_state::smkatsum_ram_map);
}

void evolution_handheldgame_state::yuleyuan(machine_config &config)
{
	evolhh(config);

	SNC7648S(config.replace(), m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &evolution_handheldgame_state::snc7648s_map);
	m_maincpu->set_addrmap(AS_DATA, &evolution_handheldgame_state::yuleyuan_ram_map);
}

void evolution_handheldgame_state::udrive(machine_config &config)
{
	evolhh(config);

	SNT110(config.replace(), m_maincpu, 16'000'000);
	m_maincpu->set_addrmap(AS_PROGRAM, &evolution_handheldgame_state::udrive_map);
	m_maincpu->set_addrmap(AS_DATA, &evolution_handheldgame_state::udrive_ram_map);
}

ROM_START( evolhh )
	ROM_REGION16_LE( 0x20000, "bootstrap", 0 )
	ROM_LOAD( "snl320_inter_prog.bin", 0x00000, 0x20000, NO_DUMP ) // probably the "SONIX standard code" mentioned in an application note
	ROM_FILL( 0x00000, 1, 0x40 )
	ROM_FILL( 0x00001, 1, 0xfe )
	ROM_FILL( 0x00002, 1, 0x04 )
	ROM_FILL( 0x00003, 1, 0x00 )
	ROM_FILL( 0x1537a, 1, 0x42 )
	ROM_FILL( 0x1537b, 1, 0xff )

	ROM_REGION( 0x400000, "maincpu", 0 )
	ROM_LOAD( "s29gl032m90tfir4.u4", 0x000000, 0x400000, CRC(c647ca01) SHA1(a88f512d3fe8803dadc4eb6a94b5babd40c698de) )
ROM_END

ROM_START( smkatsum )
	ROM_REGION( 0x800000, "maincpu", 0 )
	// "SNC7001A" reversed at byte offset 0x010000
	ROM_LOAD( "gpr25l64.ic4", 0x000000, 0x800000, CRC(85be7517) SHA1(f9b838e09ceff9b99f3e41f010ff12f6adfa9be1) )
ROM_END

ROM_START( buttdtct )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "25l64.ic4", 0x000000, 0x800000, CRC(a70a2b0d) SHA1(fe5517d58297b737f9b6f645f76bea2a5dae1eb6) )
ROM_END

ROM_START( pokexyqz )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// "SNC7001A" reversed at byte offset 0x010a00
	ROM_LOAD( "mx25l12835f.u3", 0x000000, 0x1000000, CRC(98e86224) SHA1(63872b7fb8a4ebb3260e3fbded03a93ae5403948) )

	ROM_REGION( 0x4200000, "nand", 0 )
	// "SNXROM" in wide characters at beginning
	ROM_LOAD( "mx23j51243tc.u5", 0x000000, 0x4200000, CRC(2f2c6c0c) SHA1(b47dbd33909306aa882e4a3f246af3150de94837) )

	ROM_REGION( 0x800, "i2cmem", 0 )
	ROM_LOAD( "24c16.u8", 0x000, 0x800, CRC(a607979b) SHA1(b06d0bac7844d571dd3cde9f2e0440b2555c3820) )

	// also has an SD card slot (was empty)
ROM_END

ROM_START( pokesmqz )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// "SNC7001A" reversed at byte offset 0x010a00
	ROM_LOAD( "mx25l12845e.u3", 0x000000, 0x1000000, CRC(c9b79adf) SHA1(fd0180529166ed6daf73ae6734183031c42257a5) )

	ROM_REGION( 0x800, "i2cmem", 0 )
	ROM_LOAD( "24c16.u4", 0x000, 0x800, CRC(1b4058f2) SHA1(813961f0afd1b36d78563074ddc796cf8826c6ff) )
ROM_END

ROM_START( yuleyuan )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// "SNC7648S" reversed at byte offset 0x018000 (after decryption)
	// "SNXROM" in wide characters at byte offset 0x060000 (after decryption)
	ROM_LOAD( "25l128.bin", 0x0000000, 0x1000000, CRC(51ab49e2) SHA1(ecad532d27efea55031ffd31ac4479c9c4eceae6) )
ROM_END

ROM_START( tomyspt )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// "SNC7001A" reversed at byte offset 0x011200
	ROM_LOAD( "mx25l12845e.u3", 0x000000, 0x1000000, CRC(3c8685ed) SHA1(289948c3d9a06db184397bc6a31ea594c404449d) )

	// there was also a FT24C16.u4, blank on dumped unit
ROM_END

ROM_START( hoppech )
	ROM_REGION( 0x1000000, "maincpu", 0 )
	// "SNC7001A" reversed at byte offset 0x010a00
	ROM_LOAD( "25l128.u3", 0x000000, 0x1000000, CRC(4a983ab2) SHA1(d5571cf0f3fcf872826a2ff8b45be69336b117dd) )
ROM_END

ROM_START( udrive )
	ROM_REGION( 0x800000, "maincpu", 0 )
	ROM_LOAD( "udrive.u3", 0x000000, 0x800000, CRC(c7fd123f) SHA1(8b315e594f7bf99544f323e517ccdebf2b1ac8a7) )
ROM_END


void evolution_handheldgame_state::init_yuleyuan()
{
	u16 *spi = &memregion("maincpu")->as_u16();
	for (u32 offset = 0; offset < 0x1000000 / 2; offset++)
		spi[offset] ^= 0x4890;
}

} // anonymous namespace


CONS( 2006, evolhh,      0,       0,      evolhh, evolhh, evolution_handheldgame_state, empty_init, "Kidz Delight", "Evolution Max", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // from a pink 'for girls' unit, exists in other colours, software likely the same

CONS( 2018, smkatsum,    0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "San-X / Tomy", "Sumikko Gurashi - Sumikko Atsume (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// おしりたんてい ププッとかいけつゲーム
CONS( 2020, buttdtct,    0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "Tomy", "Oshiri Tantei - Puputto Kaiketsu Game (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 2015, pokexyqz,    0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "Takara Tomy", "Pokemon Encyclopedia Z Pokemon XY Quiz Game Rotom (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// ロトム図鑑 サン＆ムーン ポケモン クイズ
CONS( 2015, pokesmqz,    0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "Takara Tomy", "Rotom Zukan Sun & Moon Pokemon Quiz (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

CONS( 201?, tomyspt,     0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "Takara Tomy", "Pretty Rhythm Smart Pod Touch (Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// ほっぺちゃん スイ☆コレ　ホワイト
CONS( 201?, hoppech,     0,       0,      smkatsum, evolhh, evolution_handheldgame_state, empty_init, "Takara Tomy", "Hoppe-chan SuiColle (white, Japan)", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )

// 星座电子宠物机 (virtual pet by 育乐元)
CONS( 2022, yuleyuan,    0,       0,      yuleyuan, evolhh, evolution_handheldgame_state, init_yuleyuan, "Yule Yuan", "Xingzuo Dianzi Chongwu Ji", MACHINE_NO_SOUND | MACHINE_NOT_WORKING ) // dumped from yellow model

// this uses TV output, rather than being a handheld
// SONIX SNT110FG SoC, test mode shows '4941' as checksum
CONS( 201?, udrive,     0,       0,       udrive, evolhh, evolution_handheldgame_state, empty_init, "MGA", "Little Tikes Cozy Coupe U-Drive", MACHINE_NO_SOUND | MACHINE_NOT_WORKING )
