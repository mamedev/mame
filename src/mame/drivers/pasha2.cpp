// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli
/********************************************************************

Pasha Pasha 2
Dong Sung, 1998

3PLAY
|--------------------------------------------------|
|      DA1311    UM53   AD-65    DREAM  9.6MHz     |
|KA22065  TL062  UM51   AD-65          RESET_SW    |
|   VOL1   VOL2                  1MHz   TL7705     |
|                                                  |
|    DSW2(8)            20MHz    GM71C18163        |
|     93C46                                   UM2  |
|           PAL                                    |
|J                                                 |
|A                                                 |
|M                      E1-16XT             AT89C52|
|M                                                 |
|A                                  U102           |
|           6116                                   |
|           6116       U3           U101           |
|                                                  |
|                                                  |
|                                             12MHz|
|                    A42MX16                       |
|    DSW1(8)                                       |
| UCN5801                                          |
| UCN5801                                          |
|         16MHz                                    |
|--------------------------------------------------|
Notes:
      U3         - 27C040 EPROM (DIP32)
      UM2/UM51/53- 29F040 EPROM (PLCC32)
      U101/102   - Each location contains a small adapter board plugged into a DIP42 socket. Each
                   adapter board holds 2x Intel E28F016S5 TSOP40 16M FlashROMs. On the PCB under the ROMs
                   it's marked '32MASK'. However, the adapter boards are not standard. If you try to read
                   the ROMs while they are _ON-THE-ADAPTER_ as a 32M DIP42 EPROM (such as 27C322), the
                   FlashROMs are damaged and the PCB no longer works :(
                   Thus, the FlashROMs must be removed and read separately!
                   The small adapter boards with their respective FlashROMs are laid out like this........

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U102
                   |                              |
                   |------------------------------|

                   |------------------------------|
                   |                              |
                   |       U2           U1        |  U101
                   |                              |
                   |------------------------------|

      A42MX16    - Actel A42MX16 FPGA (QFP160)
      AT89C52    - Atmel AT89C52 Microcontroller w/8k internal FlashROM, clock 12MHz (DIP40)
      E1-16XT    - Hyperstone E1-16XT CPU, clock 20MHz
      DREAM      - ATMEL DREAM SAM9773 Single Chip Synthesizer/MIDI with Effects and Serial Interface, clock 9.6MHz (TQFP80)
      AD-65      - Oki compatible M6295 sound chip, clock 1MHz
      5493R45    - ISSI 5493R45-001 128k x8 SRAM (SOJ32)
      GM71C18163 - Hynix 1M x16 DRAM (SOJ42)
      VSync      - 60Hz
      HSync      - 15.15kHz

 driver by Pierpaolo Prazzoli

 TODO:
 - eeprom - is it used?
 - irq2 - sound related? reads the 2 unmapped input registers.
 - irq3 - it only writes a 0 into memory and changes a registe
 - simulate music (DREAM chip)

*********************************************************************/

#include "emu.h"
#include "cpu/mcs51/mcs51.h"
#include "cpu/e132xs/e132xs.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "emupal.h"
#include "screen.h"
#include "speaker.h"

#include "pasha2.lh"


class pasha2_state : public driver_device
{
public:
	pasha2_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag)
		, m_wram(*this, "wram")
		, m_paletteram(*this, "paletteram")
		, m_mainbank(*this, "mainbank")
		, m_lamps_r(*this, "lamp_p%u_r", 1U)
		, m_lamps_g(*this, "lamp_p%u_g", 1U)
		, m_lamps_b(*this, "lamp_p%u_b", 1U)
		, m_maincpu(*this, "maincpu")
		, m_audiocpu(*this, "audiocpu")
		, m_oki(*this, "oki%u", 1U)
		, m_palette(*this, "palette")
	{ }

	void pasha2(machine_config &config);

	void init_pasha2();

private:
	/* memory pointers */
	required_shared_ptr<uint16_t> m_wram;
	required_shared_ptr<uint16_t> m_paletteram;

	required_memory_bank m_mainbank;

	output_finder<3> m_lamps_r;
	output_finder<3> m_lamps_g;
	output_finder<3> m_lamps_b;

	/* video-related */
	int m_vbuffer;

	/* memory */
	std::unique_ptr<uint8_t[]> m_bitmap0[2];
	std::unique_ptr<uint8_t[]> m_bitmap1[2];
	DECLARE_WRITE16_MEMBER(pasha2_misc_w);
	DECLARE_WRITE16_MEMBER(pasha2_palette_w);
	DECLARE_WRITE16_MEMBER(vbuffer_set_w);
	DECLARE_WRITE16_MEMBER(vbuffer_clear_w);
	DECLARE_WRITE8_MEMBER(bitmap_0_w);
	DECLARE_WRITE8_MEMBER(bitmap_1_w);
	DECLARE_WRITE16_MEMBER(pasha2_lamps_w);
	DECLARE_READ16_MEMBER(pasha2_speedup_r);
	template<int Chip> DECLARE_WRITE16_MEMBER(oki_bank_w);
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_pasha2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_device<at89c52_device> m_audiocpu;
	required_device_array<okim6295_device, 2> m_oki;
	required_device<palette_device> m_palette;
	void pasha2_io(address_map &map);
	void pasha2_map(address_map &map);
};


WRITE16_MEMBER(pasha2_state::pasha2_misc_w)
{
	if (offset)
	{
		if (data & 0x0800)
		{
			int bank = data & 0xf000;

			switch (bank)
			{
				case 0x8000:
				case 0x9000:
				case 0xa000:
				case 0xb000:
				case 0xc000:
				case 0xd000:
					m_mainbank->set_entry((bank>>12) & 7); break;
			}
		}
	}
}

WRITE16_MEMBER(pasha2_state::pasha2_palette_w)
{
	int color;

	COMBINE_DATA(&m_paletteram[offset]);

	offset &= 0xff;

	color = (m_paletteram[offset] >> 8) | (m_paletteram[offset + 0x100] & 0xff00);
	m_palette->set_pen_color(offset * 2 + 0, pal5bit(color), pal5bit(color >> 5), pal5bit(color >> 10));

	color = (m_paletteram[offset] & 0xff) | ((m_paletteram[offset + 0x100] & 0xff) << 8);
	m_palette->set_pen_color(offset * 2 + 1, pal5bit(color), pal5bit(color >> 5), pal5bit(color >> 10));
}

WRITE16_MEMBER(pasha2_state::vbuffer_set_w)
{
	m_vbuffer = 1;
}

WRITE16_MEMBER(pasha2_state::vbuffer_clear_w)
{
	m_vbuffer = 0;
}

WRITE8_MEMBER(pasha2_state::bitmap_0_w)
{
	COMBINE_DATA(&m_bitmap0[m_vbuffer][offset]);
}

WRITE8_MEMBER(pasha2_state::bitmap_1_w)
{
	// handle overlapping pixels without writing them
	if ((data & 0xff) == 0xff)
		return;

	COMBINE_DATA(&m_bitmap1[m_vbuffer][offset]);
}

template<int Chip>
WRITE16_MEMBER(pasha2_state::oki_bank_w)
{
	if (offset)
		m_oki[Chip]->set_rom_bank(data & 1);
}

WRITE16_MEMBER(pasha2_state::pasha2_lamps_w)
{
	for (int p = 0; p < 3; p++)
	{
		m_lamps_r[p] = BIT(data, (p << 2) | 0);
		m_lamps_g[p] = BIT(data, (p << 2) | 1);
		m_lamps_b[p] = BIT(data, (p << 2) | 2);
	}
}

void pasha2_state::pasha2_map(address_map &map)
{
	map(0x00000000, 0x001fffff).ram().share("wram");
	map(0x40000000, 0x4001ffff).ram().w(FUNC(pasha2_state::bitmap_0_w));
	map(0x40020000, 0x4003ffff).ram().w(FUNC(pasha2_state::bitmap_1_w));
	map(0x40060000, 0x40060001).nopw();
	map(0x40064000, 0x40064001).nopw();
	map(0x40068000, 0x40068001).nopw();
	map(0x4006c000, 0x4006c001).nopw();
	map(0x40070000, 0x40070001).w(FUNC(pasha2_state::vbuffer_clear_w));
	map(0x40074000, 0x40074001).w(FUNC(pasha2_state::vbuffer_set_w));
	map(0x40078000, 0x40078001).nopw(); //once at startup -> to disable the eeprom?
	map(0x80000000, 0x803fffff).bankr("mainbank");
	map(0xe0000000, 0xe00003ff).ram().w(FUNC(pasha2_state::pasha2_palette_w)).share("paletteram"); //tilemap? palette?
	map(0xfff80000, 0xffffffff).rom().region("maincpu", 0);
}

void pasha2_state::pasha2_io(address_map &map)
{
	map(0x08, 0x0b).nopr(); //sound status?
	map(0x18, 0x1b).nopr(); //sound status?
	map(0x20, 0x23).w(FUNC(pasha2_state::pasha2_lamps_w));
	map(0x40, 0x43).portr("COINS");
	map(0x60, 0x63).portr("DSW");
	map(0x80, 0x83).portr("INPUTS");
	map(0xa0, 0xa3).nopw(); //soundlatch?
	map(0xc0, 0xc3).w(FUNC(pasha2_state::pasha2_misc_w));
	map(0xe3, 0xe3).rw(m_oki[0], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe7, 0xe7).rw(m_oki[1], FUNC(okim6295_device::read), FUNC(okim6295_device::write));
	map(0xe8, 0xeb).w(FUNC(pasha2_state::oki_bank_w<0>));
	map(0xec, 0xef).w(FUNC(pasha2_state::oki_bank_w<1>));
}

static INPUT_PORTS_START( pasha2 )
	PORT_START("COINS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_COIN3 )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNKNOWN )

	// 2 physical dip-switches
	PORT_START("DSW")
	PORT_DIPNAME( 0x0001, 0x0001, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0001, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0002, 0x0002, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0002, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0004, 0x0004, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0004, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0018, 0x0008, DEF_STR( Lives ) )
	PORT_DIPSETTING(      0x0018, "1" )
	PORT_DIPSETTING(      0x0010, "2" )
	PORT_DIPSETTING(      0x0008, "3" )
	PORT_DIPSETTING(      0x0000, "5" )
	PORT_DIPNAME( 0x0060, 0x0060, DEF_STR( Coinage ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( 5C_1C ) )
	PORT_DIPSETTING(      0x0020, DEF_STR( 3C_1C ) )
	PORT_DIPSETTING(      0x0040, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(      0x0060, DEF_STR( 1C_1C ) )
	PORT_DIPNAME( 0x0080, 0x0080, DEF_STR( Free_Play ) )
	PORT_DIPSETTING(      0x0080, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0100, 0x0100, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0100, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0200, 0x0200, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0200, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0400, 0x0400, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0400, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x0800, 0x0800, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x0800, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x1000, 0x1000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x1000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x2000, 0x2000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x2000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x4000, 0x4000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x4000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )
	PORT_DIPNAME( 0x8000, 0x8000, DEF_STR( Unknown ) )
	PORT_DIPSETTING(      0x8000, DEF_STR( Off ) )
	PORT_DIPSETTING(      0x0000, DEF_STR( On ) )

	PORT_START("INPUTS")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(3)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(3)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(3)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_START3 )
INPUT_PORTS_END

void pasha2_state::video_start()
{
	for (int i = 0; i < 2; i++)
	{
		m_bitmap0[i] = make_unique_clear<uint8_t[]>(0x20000);
		m_bitmap1[i] = make_unique_clear<uint8_t[]>(0x20000);
		save_pointer(NAME(m_bitmap0[i]), 0x20000, i);
		save_pointer(NAME(m_bitmap1[i]), 0x20000, i);
	}
}

uint32_t pasha2_state::screen_update_pasha2(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x, y, count;
	int color;

	/* 2 512x256 bitmaps */

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		count = cliprect.min_x | (y << 9);
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			bitmap.pix16(y, x) = m_bitmap0[(m_vbuffer ^ 1)][count++] | 0x100;
		}
	}

	for (y = cliprect.min_y; y <= cliprect.max_y; y++)
	{
		count = cliprect.min_x | (y << 9);
		for (x = cliprect.min_x; x <= cliprect.max_x; x++)
		{
			color = m_bitmap1[(m_vbuffer ^ 1)][count++];
			if (color != 0)
				bitmap.pix16(y, x) = color;

		}
	}

	return 0;
}

void pasha2_state::machine_start()
{
	m_lamps_r.resolve();
	m_lamps_g.resolve();
	m_lamps_b.resolve();
	save_item(NAME(m_vbuffer));
}

void pasha2_state::machine_reset()
{
	m_vbuffer = 0;
}

void pasha2_state::pasha2(machine_config &config)
{
	/* basic machine hardware */
	E116XT(config, m_maincpu, 20000000*4);     /* 4x internal multiplier */
	m_maincpu->set_addrmap(AS_PROGRAM, &pasha2_state::pasha2_map);
	m_maincpu->set_addrmap(AS_IO, &pasha2_state::pasha2_io);
	m_maincpu->set_vblank_int("screen", FUNC(pasha2_state::irq0_line_hold));

	AT89C52(config, m_audiocpu, 12000000);     /* clock from docs */
	/* TODO : ports are unimplemented; P0,P1,P2,P3 and Serial Port Used */

	EEPROM_93C46_16BIT(config, "eeprom");

	/* video hardware */
	screen_device &screen(SCREEN(config, "screen", SCREEN_TYPE_RASTER));
	screen.set_refresh_hz(60);
	screen.set_vblank_time(ATTOSECONDS_IN_USEC(0));
	screen.set_size(512, 512);
	screen.set_visarea(0, 383, 0, 239);
	screen.set_screen_update(FUNC(pasha2_state::screen_update_pasha2));
	screen.set_palette(m_palette);

	PALETTE(config, m_palette).set_entries(0x200);

	/* sound hardware */
	SPEAKER(config, "mono").front_center();

	OKIM6295(config, m_oki[0], 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	OKIM6295(config, m_oki[1], 1000000, okim6295_device::PIN7_HIGH).add_route(ALL_OUTPUTS, "mono", 1.0);

	//and ATMEL DREAM SAM9773
}

ROM_START( pasha2 )
	ROM_REGION16_BE( 0x80000, "maincpu", 0 ) /* Hyperstone CPU Code */
	ROM_LOAD( "pp2.u3",       0x00000, 0x80000, CRC(1c701273) SHA1(f465323a1d3f2fd752c51c178fafe4cc866e28d6) )

	ROM_REGION16_BE( 0x400000*6, "bankeddata", ROMREGION_ERASEFF ) /* data roms */
	ROM_LOAD16_BYTE( "pp2-u2.u101",  0x000000, 0x200000, CRC(85c4a2d0) SHA1(452b24b74bd0b65d2d6852486e2917f94e21ecc8) )
	ROM_LOAD16_BYTE( "pp2-u1.u101",  0x000001, 0x200000, CRC(96cbd04e) SHA1(a4e7dd61194584b3c4217674d78ab2fd96b7b2e0) )
	ROM_LOAD16_BYTE( "pp2-u2.u102",  0x400000, 0x200000, CRC(2097d88c) SHA1(7597578e6ddca00909feac35d9d7331f783b2bd6) )
	ROM_LOAD16_BYTE( "pp2-u1.u102",  0x400001, 0x200000, CRC(7a3492fb) SHA1(de72c4d10e17eaf2b7531f637b42cbb3d07819b5) )
	// empty space, but no empty sockets on the pcb

	ROM_REGION( 0x2000, "audiocpu", 0 ) /* AT89C52 */
	ROM_LOAD( "89c52.bin",  0x0000, 0x2000, CRC(9ce43ce4) SHA1(8027a3549b38e9a2e7bb8f518a0defcaf9743371) ) // music play 1.0

	ROM_REGION( 0x80000, "sam9773", 0 ) /* SAM9773 sound data */
	ROM_LOAD( "pp2.um2",      0x00000, 0x80000, CRC(86814b37) SHA1(70f8a94410e362669570c39e00492c0d69de6b17) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki Samples */
	ROM_LOAD( "pp2.um51",     0x00000, 0x80000, CRC(3b1b1a30) SHA1(1ea1266d280a2b96ac4ef9fe8ee7b1a5f7861672) )

	ROM_REGION( 0x80000, "oki2", 0 ) /* Oki Samples */
	ROM_LOAD( "pp2.um53",     0x00000, 0x80000, CRC(8a29ad03) SHA1(3e9b0c86d8e3bb0b7691f68ad45431f6f9e8edbd) )
ROM_END

READ16_MEMBER(pasha2_state::pasha2_speedup_r)
{
	if(m_maincpu->pc() == 0x8302)
		m_maincpu->spin_until_interrupt();

	return m_wram[(0x95744 / 2) + offset];
}

void pasha2_state::init_pasha2()
{
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x95744, 0x95747, read16_delegate(FUNC(pasha2_state::pasha2_speedup_r), this));

	m_mainbank->configure_entries(0, 6, memregion("bankeddata")->base(), 0x400000);
	m_mainbank->set_entry(0);
}

GAMEL( 1998, pasha2, 0, pasha2, pasha2, pasha2_state, init_pasha2, ROT0, "Dong Sung", "Pasha Pasha 2", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE, layout_pasha2 )
