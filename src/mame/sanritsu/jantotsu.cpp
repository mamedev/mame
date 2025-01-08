// license:BSD-3-Clause
// copyright-holders:David Haywood, Angelo Salese
/*******************************************************************************************

4nin-uchi Mahjong Jantotsu (c) 1983 Sanritsu

driver by David Haywood and Angelo Salese

Notes:
-The 1st/2nd Player tiles on hand are actually shown on different screen sides. The Service
 Mode is for adjusting these screens (to not let the human opponent read your tiles).

TODO:
-According to the flyer, color bitplanes might be wrong on the A-N mahjong charset, might be
 a BTANB however...
-I need schematics / PCB photos (component + solder sides) to understand if the background
 color is hard-wired to the DIP-Switches or there's something else wrong.

============================================================================================
Debug cheats:

c01b-c028 player-1 tiles
c02b-c038 "right" computer tiles
c03b-c048 "up" computer tiles / player-2 tiles
c04b-c058 "left" computer tiles

============================================================================================
MSM samples table (I'm assuming that F is left-right players, M2 is up / player 2 and M1 is player 1):
start |end   |sample    |possible data triggers
------------------------|----------------------
jat-43 (0x0000-0x1fff)
0x0000|0x06ff|rii'chi M1| 0x00
0x0700|0x0bff|rii'chi M2| 0x07
0x0c00|0x0fff|nagare?   |
0x1000|0x13ff|ron M     |
0x1400|0x17ff|kore?     | 0x14
0x1800|0x1bff|kan M1    | 0x18
0x1c00|0x1fff|kan M2    |
jat-42 (0x4000-0x5fff)
0x4000|0x42ff|koi?      |
0x4300|0x4ef0|ippatsu M |
0x5000|0x52ff|pon M     |
0x5300|0x57ff|tsumo M2  | 0x53
0x5800|0x5dff|rii'chi F | 0x58
jat-41 (0x8000-0x9fff)
0x8000|0x87ff|ippatsu F |
0x8800|0x8fff|ryutoku?  | 0x88
0x9000|0x95ff|otoyo?    | 0x90
0x9600|0x9eff|yatta     | 0x96
jat-40 (0xc000-0xdfff)
0xc000|0xc8ff|odaii?    | 0xc0
0xc900|0xccff|tsumo F   | 0xc9
0xcd00|0xcfff|tsumo M1  |
0xd000|0xd3ff|ron F     | 0xd0
0xd400|0xd7ff|pon F     | 0xd4
0xd800|0xdbff|kan F     |
0xdc00|0xdfff|chi       |
------------------------------------------------


============================================================================================

4nin-uchi mahjong Jantotsu
(c)1983 Sanritsu

C2-00159_A

CPU: Z80
Sound: SN76489ANx2 MSM5205
OSC: 18.432MHz

ROMs:
JAT-00.2B (2764)
JAT-01.3B
JAT-02.4B
JAT-03.2E
JAT-04.3E
JAT-05.4E

JAT-40.6B
JAT-41.7B
JAT-42.8B
JAT-43.9B

JAT-60.10P (7051) - color PROM

This game requires special control panel.
Standard mahjong control panel + these 3 buttons.
- 9syu nagare
- Continue Yes
- Continue No

dumped by sayu

*******************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/msm5205.h"
#include "sound/sn76496.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"


namespace {

class jantotsu_state : public driver_device
{
public:
	jantotsu_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_adpcm(*this, "adpcm") ,
		m_palette(*this, "palette"),
		m_vram(*this, "vram", 0x8000, ENDIANNESS_LITTLE),
		m_vram_bank(*this, "vram_bank"),
		m_adpcm_rom(*this, "adpcm"),
		m_key(*this, { "PL1_1", "PL1_2", "PL1_3", "PL1_4", "PL2_1", "PL2_2", "PL2_3", "PL2_4" }),
		m_coins(*this, "COINS"),
		m_dsw2(*this, "DSW2")
	{ }

	void jantotsu(machine_config &config);

protected:
	virtual void machine_start() override ATTR_COLD;
	virtual void machine_reset() override ATTR_COLD;
	virtual void video_start() override ATTR_COLD;

private:
	required_device<cpu_device> m_maincpu;
	required_device<msm5205_device> m_adpcm;
	required_device<palette_device> m_palette;
	memory_share_creator<uint8_t> m_vram;
	required_memory_bank m_vram_bank;
	required_region_ptr<uint8_t> m_adpcm_rom;
	required_ioport_array<8> m_key;
	required_ioport m_coins, m_dsw2;

	// sound-related
	uint32_t   m_adpcm_pos;
	uint8_t    m_adpcm_idle;
	uint8_t    m_adpcm_data;
	uint8_t    m_adpcm_trigger;

	// misc
	uint8_t    m_mux_data;

	// video-related
	uint8_t    m_col_bank;
	uint8_t    m_display_on;
	void bankaddr_w(uint8_t data);
	uint8_t mux_r();
	void mux_w(uint8_t data);
	uint8_t dsw2_r();
	void adpcm_w(offs_t offset, uint8_t data);
	void palette(palette_device &palette) const;
	uint32_t screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	void adpcm_int(int state);
	void io_map(address_map &map) ATTR_COLD;
	void prg_map(address_map &map) ATTR_COLD;
};


/*************************************
 *
 *  Video emulation
 *
 *************************************/

void jantotsu_state::video_start()
{
	save_item(NAME(m_display_on));
}

uint32_t jantotsu_state::screen_update(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	if(!m_display_on)
		return 0;

	int count = 0;
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x += 8)
		{
			for (int i = 0; i < 8; i++)
			{
				uint8_t color = m_col_bank;

				for (uint8_t pen_i = 0; pen_i < 4; pen_i++)
					color |= (((m_vram[count + pen_i * 0x2000]) >> (7 - i)) & 1) << pen_i;

				if (cliprect.contains(x + i, y))
					bitmap.pix(y, x + i) = m_palette->pen(color);
			}

			count++;
		}
	}

	return 0;
}

void jantotsu_state::bankaddr_w(uint8_t data)
{
	m_vram_bank->set_entry((data & 0xc0) >> 6);

	m_display_on = (data & 2);

	// bit 0 is unknown
	if (data & 0x3c)
		logerror("I/O port $07 write trips %02x\n", data);
}

void jantotsu_state::palette(palette_device &palette) const
{
	const uint8_t *color_prom = memregion("proms")->base();

	for (int i = 0; i < 0x20; ++i)
	{
		int bit0 = BIT(color_prom[0], 0);
		int bit1 = BIT(color_prom[0], 1);
		int bit2 = BIT(color_prom[0], 2);
		int const r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = BIT(color_prom[0], 3);
		bit1 = BIT(color_prom[0], 4);
		bit2 = BIT(color_prom[0], 5);
		int const g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;
		bit0 = 0;
		bit1 = BIT(color_prom[0], 6);
		bit2 = BIT(color_prom[0], 7);
		int const b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		palette.set_pen_color(i, rgb_t(r, g, b));
		color_prom++;
	}
}

/*************************************
 *
 *  Memory handlers
 *
 *************************************/

// Multiplexer is mapped as 6-bits reads, bits 6 & 7 are always connected to the coin mechs.
uint8_t jantotsu_state::mux_r()
{
	//  printf("%02x\n", m_mux_data);
	uint8_t res = m_coins->read();

	for (uint8_t  i = 0; i < 8; i++)
	{
		if((~m_mux_data) & (1 << i))
			res |= m_key[i]->read();
	}

	return res;
}

void jantotsu_state::mux_w(uint8_t data)
{
	m_mux_data = data;
}

/*If bits 6 & 7 don't return 0x80, the game hangs until this bit is set,
  so I'm guessing that these bits can't be read by the z80 at all but directly
  hard-wired to the video chip. However I need the schematics / PCB snaps and/or
  a side-by-side test (to know if the background colors really works) to be sure. */
uint8_t jantotsu_state::dsw2_r()
{
	return (m_dsw2->read() & 0x3f) | 0x80;
}

void jantotsu_state::adpcm_w(offs_t offset, uint8_t data)
{
	switch (offset)
	{
		case 0:
			m_adpcm_pos = (data & 0xff) * 0x100;
			m_adpcm_idle = 0;
			m_adpcm->reset_w(0);
			/* I don't think that this will ever happen, it's there just to be sure
			   (i.e. I'll probably never do a "nagare" in my entire life ;-) ) */
			if (data & 0x20)
				popmessage("ADPCM called with data = %02x, contact MAMEdev", data);
//          printf("%02x 0\n", data);
			break;
		//same write as port 2? MSM sample ack?
		case 1:
//          m_adpcm_idle = 1;
//          m_adpcm->reset_w(1);
//          printf("%02x 1\n", data);
			break;
	}
}

void jantotsu_state::adpcm_int(int state)
{
	if (m_adpcm_pos >= 0x10000 || m_adpcm_idle)
	{
		//m_adpcm_idle = 1;
		m_adpcm->reset_w(1);
		m_adpcm_trigger = 0;
	}
	else
	{
		m_adpcm_data = ((m_adpcm_trigger ? (m_adpcm_rom[m_adpcm_pos] & 0x0f) : (m_adpcm_rom[m_adpcm_pos] & 0xf0) >> 4));
		m_adpcm->data_w(m_adpcm_data & 0xf);
		m_adpcm_trigger ^= 1;
		if (m_adpcm_trigger == 0)
		{
			m_adpcm_pos++;
			if ((m_adpcm_rom[m_adpcm_pos] & 0xff) == 0x70)
				m_adpcm_idle = 1;
		}
	}
}


/*************************************
 *
 *  Address maps
 *
 *************************************/

void jantotsu_state::prg_map(address_map &map)
{
	map(0x0000, 0xbfff).rom();
	map(0xc000, 0xc7ff).ram();
	map(0xe000, 0xffff).bankrw("vram_bank");
}

void jantotsu_state::io_map(address_map &map)
{
	map.global_mask(0xff);
	map(0x00, 0x00).portr("DSW1").w("sn1", FUNC(sn76489a_device::write));
	map(0x01, 0x01).r(FUNC(jantotsu_state::dsw2_r)).w("sn2", FUNC(sn76489a_device::write));
	map(0x02, 0x03).w(FUNC(jantotsu_state::adpcm_w));
	map(0x04, 0x04).rw(FUNC(jantotsu_state::mux_r), FUNC(jantotsu_state::mux_w));
	map(0x07, 0x07).w(FUNC(jantotsu_state::bankaddr_w));
}


/*************************************
 *
 *  Input ports
 *
 *************************************/

static INPUT_PORTS_START( jantotsu )
	PORT_START("COINS")
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_COIN2 )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Coinage ) ) PORT_DIPLOCATION("SW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x01, DEF_STR( 1C_2C ) )
	PORT_DIPUNUSED_DIPLOC( 0x02, 0x00, "SW1:2" )
	PORT_DIPNAME( 0x1c, 0x10, "Player vs. CPU timer decrement speed") PORT_DIPLOCATION("SW1:3,4,5") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "30")
	PORT_DIPSETTING(    0x08, "50" )
	PORT_DIPSETTING(    0x04, "70")
	PORT_DIPSETTING(    0x0c, "90" )
	PORT_DIPSETTING(    0x10, "110" )
	PORT_DIPSETTING(    0x18, "130" )
	PORT_DIPSETTING(    0x14, "150" )
	PORT_DIPSETTING(    0x1c, "170" )
	PORT_DIPNAME( 0xe0, 0x80, "Player vs. Player timer decrement speed" ) PORT_DIPLOCATION("SW1:6,7,8") //in msecs I suppose
	PORT_DIPSETTING(    0x00, "100" )
	PORT_DIPSETTING(    0x40, "120" )
	PORT_DIPSETTING(    0x20, "140" )
	PORT_DIPSETTING(    0x60, "160" )
	PORT_DIPSETTING(    0x80, "180" )
	PORT_DIPSETTING(    0xc0, "200" )
	PORT_DIPSETTING(    0xa0, "220" )
	PORT_DIPSETTING(    0xe0, "240" )

	PORT_START("DSW2")
	PORT_DIPNAME( 0x07, 0x07, DEF_STR( Difficulty ) ) PORT_DIPLOCATION("SW2:1,2,3")
	PORT_DIPSETTING(    0x07, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hard ) )
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x00, "SW2:4" )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Service_Mode ) ) PORT_DIPLOCATION("SW2:5")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, "Play BGM") PORT_DIPLOCATION("SW2:6")
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0xc0, 0x40, "Background Color" ) PORT_DIPLOCATION("SW2:7,8")
	PORT_DIPSETTING(    0xc0, "Purple" )
	PORT_DIPSETTING(    0x80, "Green" )
	PORT_DIPSETTING(    0x40, "Blue" )
	PORT_DIPSETTING(    0x00, "Black" )

	PORT_START("PL1_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Mahjong Nagare") PORT_CODE(KEYCODE_S)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button Yes") PORT_CODE(KEYCODE_Y)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P1 Continue Button No") PORT_CODE(KEYCODE_O)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("PL1_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH )

	PORT_START("PL1_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L )

	PORT_START("PL1_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F )

	PORT_START("PL2_1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_RON ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Mahjong Nagare") PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button Yes") PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_OTHER ) PORT_NAME("P2 Continue Button No") PORT_PLAYER(2)
	PORT_BIT( 0x30, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PL2_2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_M ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_N ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_PON ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_KAN ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_CHI ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_REACH ) PORT_PLAYER(2)

	PORT_START("PL2_3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_G ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_H ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_I ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_J ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_K ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_L ) PORT_PLAYER(2)

	PORT_START("PL2_4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_MAHJONG_A ) PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_MAHJONG_B ) PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_MAHJONG_C ) PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_MAHJONG_D ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_MAHJONG_E ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_MAHJONG_F ) PORT_PLAYER(2)
INPUT_PORTS_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

void jantotsu_state::machine_start()
{
	m_vram_bank->configure_entries(0, 4, m_vram, 0x2000);
	m_vram_bank->set_entry(0);

	save_item(NAME(m_mux_data));
	save_item(NAME(m_adpcm_pos));
	save_item(NAME(m_adpcm_idle));
	save_item(NAME(m_adpcm_data));
	save_item(NAME(m_adpcm_trigger));
}

void jantotsu_state::machine_reset()
{
	// Load hard-wired background color.
	m_col_bank = (m_dsw2->read() & 0xc0) >> 3;

	m_mux_data = 0;
	m_adpcm_pos = 0;
	m_adpcm_idle = 1;
	m_adpcm_data = 0;
	m_adpcm_trigger = 0;
}

void jantotsu_state::jantotsu(machine_config &config)
{
	// basic machine hardware
	Z80(config, m_maincpu, 18.432_MHz_XTAL / 4);
	m_maincpu->set_addrmap(AS_PROGRAM, &jantotsu_state::prg_map);
	m_maincpu->set_addrmap(AS_IO, &jantotsu_state::io_map);

	// video hardware
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(2500)); //not accurate
	screen.set_size(256, 256);
	screen.set_visarea(0, 256-1, 16, 240-1);
	screen.set_screen_update(FUNC(jantotsu_state::screen_update));
	screen.screen_vblank().set_inputline(m_maincpu, INPUT_LINE_NMI);

	PALETTE(config, m_palette, FUNC(jantotsu_state::palette), 0x20);

	// sound hardware
	SPEAKER(config, "mono").front_center();

	SN76489A(config, "sn1", 18.432_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.50);
	SN76489A(config, "sn2", 18.432_MHz_XTAL / 6).add_route(ALL_OUTPUTS, "mono", 0.50);

	MSM5205(config, m_adpcm, 384_kHz_XTAL);
	m_adpcm->vck_legacy_callback().set(FUNC(jantotsu_state::adpcm_int));  // interrupt function
	m_adpcm->set_prescaler_selector(msm5205_device::S64_4B);  // 6 KHz
	m_adpcm->add_route(ALL_OUTPUTS, "mono", 1.00);
}


/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( jantotsu )
	ROM_REGION( 0x10000, "maincpu", 0 )
	ROM_LOAD( "jat-00.2b", 0x00000, 0x02000, CRC(bed10f86) SHA1(c5ac845b32fa295b0ff205b1401bcc071d604d6e) )
	ROM_LOAD( "jat-01.3b", 0x02000, 0x02000, CRC(cc9312e9) SHA1(b08d38cc58d92378305c015c5a001b4da072188b) )
	ROM_LOAD( "jat-02.4b", 0x04000, 0x02000, CRC(292969f1) SHA1(e7cb93b67296e84ea012fcceb72df712c7e0f135) )
	ROM_LOAD( "jat-03.2e", 0x06000, 0x02000, CRC(7452ff63) SHA1(b258674e77eee5548a065419a3092877957dec66) )
	ROM_LOAD( "jat-04.3e", 0x08000, 0x02000, CRC(734e029f) SHA1(75aa13397847b4db32c41aaa6ff2ac82f16bd7a2) )
	ROM_LOAD( "jat-05.4e", 0x0a000, 0x02000, CRC(1a725e1a) SHA1(1d39d607850f47b9389f41147d4570da8814f639) )

	ROM_REGION( 0x10000, "adpcm", ROMREGION_ERASE00 )
	ROM_LOAD( "jat-43.9b", 0x00000, 0x02000, CRC(3c1d843c) SHA1(7a836e66cad4e94916f0d80a439efde49306a0e1) )
	ROM_LOAD( "jat-42.8b", 0x04000, 0x02000, CRC(3ac3efbf) SHA1(846faea7c7c01fb7500aa33a70d4b54e878c0e41) )
	ROM_LOAD( "jat-41.7b", 0x08000, 0x02000, CRC(ce08ed71) SHA1(8554e5e7ec178f57bed5fbdd5937e3a35f72c454) )
	ROM_LOAD( "jat-40.6b", 0x0c000, 0x02000, CRC(2275253e) SHA1(64e9415faf2775c6b9ab497dce7fda8c4775192e) )

	ROM_REGION( 0x20, "proms", 0 )
	ROM_LOAD( "jat-60.10p", 0x00, 0x20,  CRC(65528ae0) SHA1(6e3bf27d10ec14e3c6a494667b03b68726fcff14) )
ROM_END

} // Anonymous namespace


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1983, jantotsu, 0, jantotsu, jantotsu, jantotsu_state, empty_init, ROT270, "Sanritsu", "4nin-uchi Mahjong Jantotsu", MACHINE_SUPPORTS_SAVE )
