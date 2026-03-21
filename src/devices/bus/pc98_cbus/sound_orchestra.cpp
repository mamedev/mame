// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/**************************************************************************************************

SNE Sound Orchestra

Built on top of -26, with extra YM3812 as ADPCM engine

https://j02.nobody.jp/jto98/n_desk_sound/sso.htm

TODO:
- Better PnP (has separate ADPCM control)
- valis2 has no sound from the ADPCM channel (initializes the registers tho?)
- add SNE Sound Orchestra VS (replaces OPL2 with Y8950)

===================================================================================================

- Known SW with Sound Orchestra support
  albatr2
  valis2
  zan
  みゅあっぷ98/iv (V6.41A) ~ muap98/iv (VS only?)
  SNE Sound Musician / Great Musician / Little Musician / SOS1 / SOS2 (all undumped?)

**************************************************************************************************/

#include "emu.h"
#include "sound_orchestra.h"

#include "speaker.h"

DEFINE_DEVICE_TYPE(SOUND_ORCHESTRA, sound_orchestra_device, "sound_orchestra", "SNE Sound Orchestra sound card")

sound_orchestra_device::sound_orchestra_device(const machine_config &mconfig, const char *tag, device_t *owner, uint32_t clock)
	: pc9801_26_device(mconfig, SOUND_ORCHESTRA, tag, owner, clock)
	, m_opl2(*this, "opl2")
{
}

void sound_orchestra_device::device_add_mconfig(machine_config &config)
{
	pc9801_26_device::device_add_mconfig(config);
	// YM2203C
	m_opn->reset_routes();

	// pseudo-stereo
	config.device_remove("mono");
	SPEAKER(config, "ssg_left").front_left();
	SPEAKER(config, "fm_right").front_right();

	// TODO: mixing unconfirmed
	m_opn->add_route(0, "fm_right", 0.25);
	m_opn->add_route(1, "fm_right", 0.25);
	m_opn->add_route(2, "fm_right", 0.25);
	m_opn->add_route(3, "fm_right", 0.50);

	YM3812(config, m_opl2, 15.9744_MHz_XTAL / 4);
	m_opl2->add_route(ALL_OUTPUTS, "ssg_left", 1.00);

}

ROM_START( sound_orchestra )
	ROM_REGION( 0x4000, "bios", ROMREGION_ERASEFF )
	// baddump: should require own BIOS to accomodate the ADPCM in BASIC
	ROM_LOAD16_BYTE( "26k_wyka01_00.bin", 0x0000, 0x2000, BAD_DUMP CRC(f071bf69) SHA1(f3cdef94e9fee116cf4a9b54881e77c6cd903815) )
	ROM_LOAD16_BYTE( "26k_wyka02_00.bin", 0x0001, 0x2000, BAD_DUMP CRC(eaa01052) SHA1(5d47edae49aad591f139d5599fe04b61aefd5ecd) )
ROM_END

const tiny_rom_entry *sound_orchestra_device::device_rom_region() const
{
	return ROM_NAME( sound_orchestra );
}

void sound_orchestra_device::io_map(address_map &map)
{
	const u16 io_base = read_io_base();
	map(0x0088 | io_base, 0x008b | io_base).rw(m_opn, FUNC(ym2203_device::read), FUNC(ym2203_device::write)).umask16(0x00ff);
	map(0x008c | io_base, 0x008f | io_base).rw(m_opl2, FUNC(ym3812_device::read), FUNC(ym3812_device::write)).umask16(0x00ff);
}

