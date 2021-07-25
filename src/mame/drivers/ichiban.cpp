// license:BSD-3-Clause
// copyright-holders:Guru
/***************************************************************************

Ichi Ban Jyan
Excel, 199?

PCB Layout
----------

MJ911
|----------------------------------|
|MB3712 DSW-D DSW-C DSW-B DSW-A  SW|
|   M6378                   BATT   |
|VOL               6264         3  |
|    YM2413 MJB                    |
|M                  1           2  |
|A  YM2149  MJG  |-------|         |
|H               |ALTERA |  Z80    |
|J          MJR  |EP1810 |         |
|O               |       |  ALTERA |
|N               |-------|  EP910  |
|G                                 |
|                                  |
|      41464  41464                |
|      41464  41464       18.432MHz|
|----------------------------------|
Notes:
Z80 clock - 6.144MHz [18.432/3]
YM2149 clock - 1.536MHz [18.432/12]
YM2413 clock - 3.072MHz [18.432/6]
M6378 - OKI MSM6378A Voice Synthesis IC with 256Kbit OTP ROM (DIP16) - not populated
VSync - 60.5686Hz
HSync - 15.510kHz

Notes / TODO:
- where and how are the opcodes at 0x8000 and the data at 0x18000 banked?
***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "machine/nvram.h"
#include "sound/ay8910.h"
#include "sound/ymopl.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class ichibanjyan_state : public driver_device
{
public:
	ichibanjyan_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_maincpu(*this, "maincpu")
	{ }

	void ichibanjyan(machine_config &config);

protected:
	// driver_device overrides
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;

private:
	// devices
	required_device<cpu_device> m_maincpu;

	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void mem_map(address_map &map);
	void io_map(address_map &map);
	void opcodes_map(address_map &map);
};

void ichibanjyan_state::video_start()
{
}

uint32_t ichibanjyan_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	return 0;
}


void ichibanjyan_state::mem_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("code", 0x10000);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).ram();
}

void ichibanjyan_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ym2149_device::data_address_w));
	map(0x16, 0x17).w("ymsnd", FUNC(ym2413_device::write));
}

void ichibanjyan_state::opcodes_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("code", 0);
}

static INPUT_PORTS_START( ichibanjyan )
INPUT_PORTS_END

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	8,
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ STEP8( 0*512, 8 ) },
	{ STEP8( 0*512, 8*8 ) },
	8*8*8
};

static GFXDECODE_START( gfx_ichibanjyan )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,     0, 1 )
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,     0, 1 )
GFXDECODE_END


void ichibanjyan_state::machine_start()
{
}

void ichibanjyan_state::machine_reset()
{
}


void ichibanjyan_state::ichibanjyan(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 3);
	m_maincpu->set_addrmap(AS_PROGRAM, &ichibanjyan_state::mem_map);
	m_maincpu->set_addrmap(AS_IO, &ichibanjyan_state::io_map);
	m_maincpu->set_addrmap(AS_OPCODES, &ichibanjyan_state::opcodes_map);

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 396, 0, 320, 256, 0, 224); // dimensions guessed
	screen.set_screen_update(FUNC(ichibanjyan_state::screen_update));
	screen.set_palette("palette");

	GFXDECODE(config, "gfxdecode", "palette", gfx_ichibanjyan);

	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();
	YM2149(config, "aysnd", 18.432_MHz_XTAL / 12).add_route(ALL_OUTPUTS, "mono", 0.30);

	YM2413(config, "ymsnd", 18.432_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ichiban )
	ROM_REGION( 0x20000, "code", 0 ) // opcodes in first half are mixed with pseudo-random garbage
	ROM_LOAD( "3.u15", 0, 0x20000, CRC(76240568) SHA1(cf055d1eaae25661a49ec4722a2c7caca862e66a) )

	ROM_REGION( 0x20000, "gfx1", 0 )
	ROM_LOAD( "1.u28", 0, 0x20000, CRC(2caa4d3f) SHA1(5e5af164880140b764c097a65388c22ba5ea572b) )

	ROM_REGION( 0x20000, "gfx2", 0 )
	ROM_LOAD( "2.u14", 0, 0x20000, CRC(b4834d8e) SHA1(836ddf7586dc5440faf88f5ec50a32265e9a0ec8) )

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "mjr.u36", 0x000, 0x200, CRC(31cd7a90) SHA1(1525ad19d748561a52626e4ab13df67d9bedf3b8) )
	ROM_LOAD( "mjg.u37", 0x200, 0x200, CRC(5b3562aa) SHA1(ada60d2a5a5a657d7b209d18a23b685305d9ff7b) )
	ROM_LOAD( "mjb.u38", 0x400, 0x200, CRC(0ef881cb) SHA1(44b61a443d683f5cb2d1b1a4f74d8a8f41021de5) )
ROM_END

} // Anonymous namespace


GAME( 199?, ichiban, 0, ichibanjyan, ichibanjyan, ichibanjyan_state, empty_init, ROT0, "Excel",      "Ichi Ban Jyan", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
