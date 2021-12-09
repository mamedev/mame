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
- code and palette banking;
- dips (are visible in the 'Analyser 2' page, just needs some time);
- seems heavily inspired by the games in ichibanjyan.cpp. Consider merging.
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
		, m_palette(*this, "palette")
		, m_videoram(*this, "videoram")
		, m_key(*this, "KEY%u", 0U)
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
	required_device<palette_device> m_palette;
	required_shared_ptr<uint8_t> m_videoram;
	required_ioport_array<10> m_key;

	uint8_t m_input_port_select;

	uint8_t player_1_port_r();
	uint8_t player_2_port_r();
	void input_port_select_w(uint8_t data);
	void control_w(uint8_t data);

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
	for (offs_t offs = 0; offs < 0x4000; offs++)
	{
		uint8_t data1 = m_videoram[offs + 0x0000];
		uint8_t data2 = m_videoram[offs + 0x4000];

		uint8_t y = (offs >> 6);
		uint8_t x = (offs << 2);

		for (int i = 0; i < 4; i++)
		{
			uint8_t pen = ((data2 >> 1) & 0x08) | ((data2 << 2) & 0x04) | ((data1 >> 3) & 0x02) | ((data1 >> 0) & 0x01);

			bitmap.pix(y, x) = m_palette->pen(pen);

			x = x + 1;
			data1 = data1 >> 1;
			data2 = data2 >> 1;
		}
	}

	return 0;
}

void ichibanjyan_state::input_port_select_w(uint8_t data)
{
	m_input_port_select = data;
}

uint8_t ichibanjyan_state::player_1_port_r()
{
	int ret = (m_key[0]->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= m_key[0]->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= m_key[1]->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= m_key[2]->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= m_key[3]->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= m_key[4]->read();

	return ret;
}

uint8_t ichibanjyan_state::player_2_port_r()
{
	int ret = (m_key[5]->read() & 0xc0) | 0x3f;

	if ((m_input_port_select & 0x01) == 0)  ret &= m_key[5]->read();
	if ((m_input_port_select & 0x02) == 0)  ret &= m_key[6]->read();
	if ((m_input_port_select & 0x04) == 0)  ret &= m_key[7]->read();
	if ((m_input_port_select & 0x08) == 0)  ret &= m_key[8]->read();
	if ((m_input_port_select & 0x10) == 0)  ret &= m_key[9]->read();

	return ret;
}

void ichibanjyan_state::control_w(uint8_t data)
{
	machine().bookkeeping().coin_counter_w(0, BIT(data, 1));

	// bit 2 always set? it's flip screen in most royalmah.cpp games
}


void ichibanjyan_state::mem_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("code", 0x10000);
	map(0x7000, 0x7fff).ram().share("nvram");
	map(0x8000, 0xffff).rom().region("code", 0x18000); // TODO: should be banked, as changing the offset to the other ROMs shows other graphics
	map(0x8000, 0xffff).writeonly().share(m_videoram);
}

void ichibanjyan_state::io_map(address_map &map) // TODO: writes to 0x12 and 0x14, probably code and palette banking
{
	map.global_mask(0xff);
	map(0x01, 0x01).r("aysnd", FUNC(ym2149_device::data_r));
	map(0x02, 0x03).w("aysnd", FUNC(ym2149_device::data_address_w));
	map(0x10, 0x10).portr("DSW-A").w(FUNC(ichibanjyan_state::control_w));
	map(0x11, 0x11).portr("SYSTEM").w(FUNC(ichibanjyan_state::input_port_select_w));
	map(0x12, 0x12).portr("DSW-B").nopw();
	map(0x13, 0x13).portr("DSW-C");
	map(0x14, 0x14).portr("DSW-D").nopw();
	map(0x16, 0x17).w("ymsnd", FUNC(ym2413_device::write));
}

void ichibanjyan_state::opcodes_map(address_map &map)
{
	map(0x0000, 0x6fff).rom().region("code", 0);
	map(0x8000, 0xffff).rom().region("code", 0x8000);
}

static INPUT_PORTS_START( ichibanjyan )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("Payout") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL )
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_BET ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0xe0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_LAST_CHANCE ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_SCORE ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_DOUBLE_UP ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_FLIP_FLOP ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_BIG ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_SMALL ) PORT_PLAYER(2)
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("SYSTEM")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN2 ) // "Note" ("Paper Money") = 10 Credits
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MEMORY_RESET )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SERVICE1 )  // Analizer (Statistics). This plus service mode give access to dip page
	PORT_SERVICE( 0x08, IP_ACTIVE_LOW )
	PORT_BIT( 0xf0, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("DSW-A")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW-A:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW-A:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW-A:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW-A:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW-A:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW-A:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW-A:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW-A:8")

	PORT_START("DSW-B")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW-B:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW-B:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW-B:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW-B:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW-B:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW-B:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW-B:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW-B:8")

	PORT_START("DSW-C")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW-C:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW-C:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW-C:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW-C:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW-C:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW-C:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW-C:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW-C:8")

	PORT_START("DSW-D")
	PORT_DIPUNKNOWN_DIPLOC(0x01, 0x01, "DSW-D:1")
	PORT_DIPUNKNOWN_DIPLOC(0x02, 0x02, "DSW-D:2")
	PORT_DIPUNKNOWN_DIPLOC(0x04, 0x04, "DSW-D:3")
	PORT_DIPUNKNOWN_DIPLOC(0x08, 0x08, "DSW-D:4")
	PORT_DIPUNKNOWN_DIPLOC(0x10, 0x10, "DSW-D:5")
	PORT_DIPUNKNOWN_DIPLOC(0x20, 0x20, "DSW-D:6")
	PORT_DIPUNKNOWN_DIPLOC(0x40, 0x40, "DSW-D:7")
	PORT_DIPUNKNOWN_DIPLOC(0x80, 0x80, "DSW-D:8")
INPUT_PORTS_END

void ichibanjyan_state::machine_start()
{
	m_input_port_select = 0;

	save_item(NAME(m_input_port_select));
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
	m_maincpu->set_vblank_int("screen", FUNC(ichibanjyan_state::irq0_line_hold));

	NVRAM(config, "nvram", nvram_device::DEFAULT_ALL_0);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_raw(18.432_MHz_XTAL / 3, 256, 0, 255, 256, 0, 247); // dimensions guessed
	screen.set_screen_update(FUNC(ichibanjyan_state::screen_update));
	screen.set_palette("palette");

	PALETTE(config, "palette", palette_device::RGB_444_PROMS, "proms", 512);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	ym2149_device &ay(YM2149(config, "aysnd", 18.432_MHz_XTAL / 12));
	ay.port_a_read_callback().set(FUNC(ichibanjyan_state::player_1_port_r));
	ay.port_b_read_callback().set(FUNC(ichibanjyan_state::player_2_port_r));
	ay.add_route(ALL_OUTPUTS, "mono", 0.30);

	YM2413(config, "ymsnd", 18.432_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.5);
}


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( ichiban )
	ROM_REGION( 0x60000, "code", 0 ) // opcodes in first half are mixed with pseudo-random garbage
	ROM_LOAD( "3.u15", 0x00000, 0x20000, CRC(76240568) SHA1(cf055d1eaae25661a49ec4722a2c7caca862e66a) )
	ROM_LOAD( "2.u14", 0x20000, 0x20000, CRC(b4834d8e) SHA1(836ddf7586dc5440faf88f5ec50a32265e9a0ec8) )
	ROM_LOAD( "1.u28", 0x40000, 0x20000, CRC(2caa4d3f) SHA1(5e5af164880140b764c097a65388c22ba5ea572b) ) // ?

	ROM_REGION( 0x600, "proms", 0 )
	ROM_LOAD( "mjr.u36", 0x000, 0x200, CRC(31cd7a90) SHA1(1525ad19d748561a52626e4ab13df67d9bedf3b8) )
	ROM_LOAD( "mjg.u37", 0x200, 0x200, CRC(5b3562aa) SHA1(ada60d2a5a5a657d7b209d18a23b685305d9ff7b) )
	ROM_LOAD( "mjb.u38", 0x400, 0x200, CRC(0ef881cb) SHA1(44b61a443d683f5cb2d1b1a4f74d8a8f41021de5) )
ROM_END

} // Anonymous namespace


GAME( 199?, ichiban, 0, ichibanjyan, ichibanjyan, ichibanjyan_state, empty_init, ROT180, "Excel",      "Ichi Ban Jyan", MACHINE_NOT_WORKING | MACHINE_IMPERFECT_SOUND ) // should just need correct banking
