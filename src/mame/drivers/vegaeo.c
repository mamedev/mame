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
#include "cpu/e132xs/e132xs.h"
#include "machine/at28c16.h"
#include "sound/qs1000.h"
#include "includes/eolith.h"


class vegaeo_state : public eolith_state
{
public:
	vegaeo_state(const machine_config &mconfig, device_type type, const char *tag)
		: eolith_state(mconfig, type, tag) { }

	UINT32 *m_vega_vram;
	UINT8 m_vega_vbuffer;

	DECLARE_WRITE32_MEMBER(vega_vram_w);
	DECLARE_READ32_MEMBER(vega_vram_r);
	DECLARE_WRITE32_MEMBER(vega_misc_w);
	DECLARE_READ32_MEMBER(vegaeo_custom_read);
	DECLARE_WRITE32_MEMBER(soundlatch_w);
	DECLARE_READ8_MEMBER(qs1000_p1_r);
	DECLARE_WRITE8_MEMBER(qs1000_p1_w);
	DECLARE_WRITE8_MEMBER(qs1000_p2_w);
	DECLARE_WRITE8_MEMBER(qs1000_p3_w);

	DECLARE_DRIVER_INIT(vegaeo);
	DECLARE_VIDEO_START(vega);

	UINT32 screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
};

READ8_MEMBER( vegaeo_state::qs1000_p1_r )
{
	return soundlatch_byte_r(space, 0);
}

WRITE8_MEMBER( vegaeo_state::qs1000_p1_w )
{
}

WRITE8_MEMBER( vegaeo_state::qs1000_p2_w )
{
}

WRITE8_MEMBER( vegaeo_state::qs1000_p3_w )
{
	// .... .xxx - Data ROM bank (64kB)
	// ...x .... - ?
	// ..x. .... - /IRQ clear

	membank("qs1000:bank")->set_entry(data & 0x07);

	if (!BIT(data, 5))
		m_qs1000->set_irq(CLEAR_LINE);
}

WRITE32_MEMBER(vegaeo_state::vega_vram_w)
{
	switch(mem_mask)
	{
		case 0xffffffff:
			vega_vram_w(space,offset,data,0xff000000);
			vega_vram_w(space,offset,data,0x00ff0000);
			vega_vram_w(space,offset,data,0x0000ff00);
			vega_vram_w(space,offset,data,0x000000ff);
			return;

		case 0xffff0000:
			vega_vram_w(space,offset,data,0xff000000);
			vega_vram_w(space,offset,data,0x00ff0000);
			return;

		case 0x0000ffff:
			vega_vram_w(space,offset,data,0x0000ff00);
			vega_vram_w(space,offset,data,0x000000ff);
			return;

		default:
			// don't write transparent pen
			if((data & mem_mask) == mem_mask)
				return;
	}

	COMBINE_DATA(&m_vega_vram[offset + m_vega_vbuffer * (0x14000/4)]);
}

READ32_MEMBER(vegaeo_state::vega_vram_r)
{
	return m_vega_vram[offset + (0x14000/4) * m_vega_vbuffer];
}

WRITE32_MEMBER(vegaeo_state::vega_misc_w)
{
	// other bits ???

	m_vega_vbuffer = data & 1;
}


READ32_MEMBER(vegaeo_state::vegaeo_custom_read)
{
	speedup_read();
	return ioport("SYSTEM")->read();
}

WRITE32_MEMBER(vegaeo_state::soundlatch_w)
{
	soundlatch_byte_w(space, 0, data);
	m_qs1000->set_irq(ASSERT_LINE);

	machine().scheduler().boost_interleave(attotime::zero, attotime::from_usec(100));
}


static ADDRESS_MAP_START( vega_map, AS_PROGRAM, 32, vegaeo_state )
	AM_RANGE(0x00000000, 0x001fffff) AM_RAM
	AM_RANGE(0x80000000, 0x80013fff) AM_READWRITE(vega_vram_r, vega_vram_w)
	AM_RANGE(0xfc000000, 0xfc0000ff) AM_DEVREADWRITE8("at28c16", at28c16_device, read, write, 0x000000ff)
	AM_RANGE(0xfc200000, 0xfc2003ff) AM_DEVREADWRITE16("palette", palette_device, read, write, 0x0000ffff) AM_SHARE("palette")
	AM_RANGE(0xfc400000, 0xfc40005b) AM_WRITENOP // crt registers ?
	AM_RANGE(0xfc600000, 0xfc600003) AM_WRITE(soundlatch_w)
	AM_RANGE(0xfca00000, 0xfca00003) AM_WRITE(vega_misc_w)
	AM_RANGE(0xfcc00000, 0xfcc00003) AM_READ(vegaeo_custom_read)
	AM_RANGE(0xfce00000, 0xfce00003) AM_READ_PORT("P1_P2")
	AM_RANGE(0xfd000000, 0xfeffffff) AM_ROM AM_REGION("user1", 0)
	AM_RANGE(0xfff80000, 0xffffffff) AM_ROM AM_REGION("maincpu", 0)
ADDRESS_MAP_END

static INPUT_PORTS_START( crazywar )
	PORT_START("SYSTEM")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00000020, IP_ACTIVE_LOW )
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_SPECIAL ) PORT_CUSTOM_MEMBER(DEVICE_SELF, vegaeo_state, eolith_speedup_getvblank, NULL)
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


VIDEO_START_MEMBER(vegaeo_state,vega)
{
	m_vega_vram = auto_alloc_array(machine(), UINT32, 0x14000*2/4);
	save_pointer(NAME(m_vega_vram), 0x14000*2/4);
	save_item(NAME(m_vega_vbuffer));
}

UINT32 vegaeo_state::screen_update_vega(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	int x,y,count;
	int color;

	count = 0;
	for (y=0;y < 240;y++)
	{
		for (x=0;x < 320/4;x++)
		{
			color = m_vega_vram[count + (0x14000/4) * (m_vega_vbuffer ^ 1)] & 0xff;
			bitmap.pix16(y, x*4 + 3) = color;

			color = (m_vega_vram[count + (0x14000/4) * (m_vega_vbuffer ^ 1)] & 0xff00) >> 8;
			bitmap.pix16(y, x*4 + 2) = color;

			color = (m_vega_vram[count + (0x14000/4) * (m_vega_vbuffer ^ 1)] & 0xff0000) >> 16;
			bitmap.pix16(y, x*4 + 1) = color;

			color = (m_vega_vram[count + (0x14000/4) * (m_vega_vbuffer ^ 1)] & 0xff000000) >> 24;
			bitmap.pix16(y, x*4 + 0) = color;

			count++;
		}
	}
	return 0;
}


static MACHINE_CONFIG_START( vega, vegaeo_state )
	MCFG_CPU_ADD("maincpu", GMS30C2132, XTAL_55MHz)
	MCFG_CPU_PROGRAM_MAP(vega_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", vegaeo_state, eolith_speedup, "screen", 0, 1)

	MCFG_AT28C16_ADD("at28c16", NULL)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(512, 262)
	MCFG_SCREEN_VISIBLE_AREA(0, 319, 0, 239)
	MCFG_SCREEN_UPDATE_DRIVER(vegaeo_state, screen_update_vega)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_PALETTE_ADD("palette", 256)
	MCFG_PALETTE_FORMAT(xRRRRRGGGGGBBBBB)
	MCFG_PALETTE_MEMBITS(16)

	MCFG_VIDEO_START_OVERRIDE(vegaeo_state,vega)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("qs1000", QS1000, XTAL_24MHz)
	MCFG_QS1000_EXTERNAL_ROM(true)
	MCFG_QS1000_IN_P1_CB(READ8(vegaeo_state, qs1000_p1_r))
	MCFG_QS1000_OUT_P1_CB(WRITE8(vegaeo_state, qs1000_p1_w))
	MCFG_QS1000_OUT_P2_CB(WRITE8(vegaeo_state, qs1000_p2_w))
	MCFG_QS1000_OUT_P3_CB(WRITE8(vegaeo_state, qs1000_p3_w))
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END

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

	ROM_REGION32_BE( 0x2000000, "user1", ROMREGION_ERASE00 ) /* Game Data - banked ROM, swapping necessary */
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

DRIVER_INIT_MEMBER(vegaeo_state,vegaeo)
{
	// Set up the QS1000 program ROM banking, taking care not to overlap the internal RAM
	machine().device("qs1000:cpu")->memory().space(AS_IO).install_read_bank(0x0100, 0xffff, "bank");
	membank("qs1000:bank")->configure_entries(0, 8, memregion("qs1000:cpu")->base()+0x100, 0x10000);

	init_speedup();
}

GAME( 2002, crazywar, 0, vega, crazywar, vegaeo_state, vegaeo, ROT0, "Eolith", "Crazy War", MACHINE_IMPERFECT_SOUND | MACHINE_SUPPORTS_SAVE )
