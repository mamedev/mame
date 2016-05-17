// license:BSD-3-Clause
// copyright-holders:David Haywood
/*


*/

#include "emu.h"
#include "cpu/z80/z80.h"

#include "audio/t5182.h"

class metlfrzr_state : public driver_device
{
public:
	metlfrzr_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_decrypted_opcodes(*this, "decrypted_opcodes"),
		m_vram(*this, "vram"),
		m_palette(*this, "palette"),
		m_gfxdecode(*this, "gfxdecode")
		{ }

	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void video_start() override;
	void legacy_fg_draw(bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	required_device<cpu_device> m_maincpu;
	required_shared_ptr<UINT8> m_decrypted_opcodes;
	required_shared_ptr<UINT8> m_vram;
	required_device<palette_device> m_palette;
	required_device<gfxdecode_device> m_gfxdecode;

	DECLARE_DRIVER_INIT(metlfrzr);
	DECLARE_WRITE8_MEMBER(output_w);
	TIMER_DEVICE_CALLBACK_MEMBER(scanline);
};


void metlfrzr_state::video_start()
{
}

void metlfrzr_state::legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_0 = m_gfxdecode->gfx(0);
	int count;

	for (count=0;count<32*32;count++)
	{
		int y = (count % 32);
		int x = count / 32;

		UINT16 tile = m_vram[count*2+0] + ((m_vram[count*2+1] & 0xf0) << 4);
		UINT8 color = 0;//(m_fgattr[count] & 0x3f) + (m_pal_bank<<6);

		gfx_0->transpen(bitmap,cliprect,tile,color,0,0,x*8,y*8,0xf);
	}

}

UINT32 metlfrzr_state::screen_update_metlfrzr(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(m_palette->black_pen(), cliprect);

	legacy_fg_draw(bitmap,cliprect);
	return 0;
}

WRITE8_MEMBER(metlfrzr_state::output_w)
{
	// bit 7: flip screen
	if(data & 0x7f)
		printf("%02x\n",data);
}

static ADDRESS_MAP_START( metlfrzr_map, AS_PROGRAM, 8, metlfrzr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM AM_SHARE("vram")
	AM_RANGE(0xd000, 0xd1ff) AM_RAM_DEVWRITE("palette", palette_device, write_indirect) AM_SHARE("palette")
	AM_RANGE(0xd200, 0xd3ff) AM_RAM_DEVWRITE("palette", palette_device, write_indirect_ext) AM_SHARE("palette_ext")

	AM_RANGE(0xd600, 0xd600) AM_READ_PORT("P1")
	AM_RANGE(0xd601, 0xd601) AM_READ_PORT("P2")
	AM_RANGE(0xd602, 0xd602) AM_READ_PORT("START")
	AM_RANGE(0xd603, 0xd603) AM_READ_PORT("DSW1")
	AM_RANGE(0xd604, 0xd604) AM_READ_PORT("DSW2")
	
	AM_RANGE(0xd700, 0xd700) AM_WRITE(output_w)
	AM_RANGE(0xd800, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xefff) AM_RAM
	AM_RANGE(0xf000, 0xffff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( decrypted_opcodes_map, AS_DECRYPTED_OPCODES, 8, metlfrzr_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM AM_SHARE("decrypted_opcodes")
ADDRESS_MAP_END


static INPUT_PORTS_START( metlfrzr )
	PORT_START("P1")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 )
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("P2")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_COCKTAIL
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_BUTTON1 ) PORT_COCKTAIL
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_BUTTON2 ) PORT_COCKTAIL
	PORT_BIT( 0xc0, IP_ACTIVE_HIGH, IPT_UNKNOWN )

	PORT_START("START")
	PORT_DIPNAME( 0x01, 0x01, "2-0" )
	PORT_DIPSETTING(    0x01, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x02, 0x02, "2-1" )
	PORT_DIPSETTING(    0x02, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x04, 0x04, "2-2" )
	PORT_DIPSETTING(    0x04, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x08, 0x08, "2-2" )
	PORT_DIPSETTING(    0x08, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_START1 )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_START2 )
	PORT_DIPNAME( 0x40, 0x40, "2-6" )
	PORT_DIPSETTING(    0x40, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	PORT_DIPNAME( 0x80, 0x80, "2-7" )
	PORT_DIPSETTING(    0x80, DEF_STR( No ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Yes ) )
	
	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, "SYSA" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_SERVICE_DIPLOC( 0x02, IP_ACTIVE_LOW, "SW1:2" )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Flip_Screen ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
	

	PORT_START("DSW2")
	PORT_DIPNAME( 0x01, 0x00, "SYSB" )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x01, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x02, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x08, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x10, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x20, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x00, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x80, DEF_STR( On ) )
INPUT_PORTS_END



void metlfrzr_state::machine_start()
{
	membank("bank1")->configure_entries(0, 4, memregion("maincpu")->base() + 0x10000, 0x4000);
}

void metlfrzr_state::machine_reset()
{
	membank("bank1")->set_entry(0);

}


static const gfx_layout tiles8x8_layout =
{
	8,8,
	RGN_FRAC(1,1),
	4,
	{ 0, 4, 8, 12 },
	{ 19, 18,17,16,3,2,1,0 }, 
//	{ 19, 18, 17, 16, 3, 2, 1, 0 }, // maybe display is flipped?
//	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	32*8
};

static const gfx_layout tiles16x16_layout =
{
	16, 16,
		RGN_FRAC(1, 1),
		4,
		{ 0, 4, 8, 12 },
		//	{ 0, 1, 2, 3, 16, 17, 18, 19, 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+16, 64*8+17, 64*8+18, 64*8+19 },
		{ 64 * 8 + 19, 64 * 8 + 18, 64 * 8 + 17, 64 * 8 + 16, 64 * 8 + 3, 64 * 8 + 2, 64 * 8 + 1, 64 * 8 + 0, 19, 18, 17, 16, 3, 2, 1, 0 },
		//	{ 0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32, 8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
		{ 15 * 32,14 * 32,13 * 32,12 * 32,11 * 32,10 * 32,9 * 32,8 * 32, 7 * 32, 6 * 32, 5 * 32, 4 * 32, 3 * 32, 2 * 32, 1 * 32, 0 * 32 },
		128 * 8
};


static GFXDECODE_START(metlfrzr)
	GFXDECODE_ENTRY("gfx1", 0, tiles8x8_layout, 0, 16)
	GFXDECODE_ENTRY("gfx2", 0, tiles8x8_layout, 0, 16)
	GFXDECODE_ENTRY("gfx3", 0, tiles16x16_layout, 0, 16)
	GFXDECODE_ENTRY("gfx4", 0, tiles16x16_layout, 0, 16)
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(metlfrzr_state::scanline)
{
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0x10); /* RST 10h */

	if(scanline == 0) // vblank-in irq
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE,0x08); /* RST 08h */
}

static MACHINE_CONFIG_START(metlfrzr, metlfrzr_state)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", Z80, XTAL_12MHz / 2)
	MCFG_CPU_PROGRAM_MAP(metlfrzr_map)
	MCFG_CPU_DECRYPTED_OPCODES_MAP(decrypted_opcodes_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", metlfrzr_state, scanline, "screen", 0, 1)

	MCFG_DEVICE_ADD("t5182", T5182, 0)

	MCFG_PALETTE_ADD("palette", 0x200)
	MCFG_PALETTE_INDIRECT_ENTRIES(256*2)
	MCFG_PALETTE_FORMAT(xxxxRRRRGGGGBBBB)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", metlfrzr)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256 - 1, 16, 256 - 16 - 1)
	MCFG_SCREEN_UPDATE_DRIVER(metlfrzr_state, screen_update_metlfrzr)
	MCFG_SCREEN_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_YM2151_ADD("ymsnd", XTAL_14_31818MHz / 4)    /* 3.579545 MHz */
	MCFG_YM2151_IRQ_HANDLER(DEVWRITELINE("t5182", t5182_device, ym2151_irq_handler))
	MCFG_SOUND_ROUTE(0, "mono", 1.0)
	MCFG_SOUND_ROUTE(1, "mono", 1.0)

MACHINE_CONFIG_END



ROM_START(metlfrzr)
	ROM_REGION(0x20000, "maincpu", 0)
	ROM_LOAD("1.15j", 0x00000, 0x08000, CRC(f59b5fa2) SHA1(6033967dad5e64f45afbcb1b45c8eb79e0787afb))
	ROM_LOAD("2.14j", 0x10000, 0x10000, CRC(21ecc248) SHA1(2fccf7db73890faf7c489bfc43c88ded54d5052d))

	ROM_REGION(0x8000, "t5182_z80", 0) /* Toshiba T5182 external data ROM */
	ROM_LOAD("3.4h", 0x0000, 0x8000, CRC(36f88e54) SHA1(5cbea56c7e547c353ae2f9256caaceb20e5e8503))

	ROM_REGION(0x20000, "gfx1", 0)
	ROM_LOAD16_BYTE("10.5a", 0x00001, 0x10000, CRC(3313e74a) SHA1(8622dfb5c013173d5bb037254f4c23b1282404e1))
	ROM_LOAD16_BYTE("12.7a", 0x00000, 0x10000, CRC(6da5fda9) SHA1(9d7b0b26598f31da589fece3535a4d1405b03fc2))

	ROM_REGION(0x20000, "gfx2", 0)
	ROM_LOAD16_BYTE("11.6a", 0x00001, 0x10000, CRC(fa6490b8) SHA1(9a4c1e09b9e8fb256fec0a5ed120fece8a12e1c8))
	ROM_LOAD16_BYTE("13.9a", 0x00000, 0x10000, CRC(a4f689ec) SHA1(e58bfede3fabf4cfca76c20aafb3e9fb604777c9))

	ROM_REGION(0x20000, "gfx3", 0)
	ROM_LOAD16_BYTE("14.13a", 0x00001, 0x10000, CRC(a9cd5225) SHA1(f3d5e29ee08fb563fdc1af3c64128f2cd2feb987))
	ROM_LOAD16_BYTE("16.11a", 0x00000, 0x10000, CRC(92f2cb49) SHA1(498021d94b0fde216207076491702af2324a2dcc))
	
	ROM_REGION(0x20000, "gfx4", 0)
	ROM_LOAD16_BYTE("15.12a", 0x00001, 0x10000, CRC(ce5c4c8b) SHA1(2351d66ba51e80097ce53bfd448ac24901844cda))
	ROM_LOAD16_BYTE("17.10a", 0x00000, 0x10000, CRC(3fec33f7) SHA1(af086ba30fc4521a0114da2824f5baa04d225a89))

	ROM_REGION(0x20000, "proms", 0)
	ROM_LOAD("n8s129a.7f",  0x000, 0x100, CRC(c849d60b) SHA1(0022fb71b3d777cadac7005e6156725df9bcaf90))
	ROM_LOAD("n82s135n.9c", 0x000, 0x100, CRC(7bbd52db) SHA1(b9bab5fb515579d0270aea8b992a16eeb878f242))

	ROM_REGION(0x20000, "plds", 0)
	ROM_LOAD("pld3.14h.bin", 0x000, 0x149, CRC(8183f7f0) SHA1(3cec53838120064374ecf4ebee048409c6f34081))
	ROM_LOAD("pld8.4d.bin",  0x000, 0x149, CRC(f1e35034) SHA1(527faddbf2ac905fa59ebda8ea327e6e6a7c1fb6))
ROM_END



DRIVER_INIT_MEMBER(metlfrzr_state, metlfrzr)
{
	// same as cshooter.cpp
	UINT8 *rom = memregion("maincpu")->base();

	for (int A = 0x0000;A < 0x8000;A++)
	{
		/* decode the opcodes */
		m_decrypted_opcodes[A] = rom[A];

		if (BIT(A,5) && !BIT(A,3))
			m_decrypted_opcodes[A] ^= 0x40;

		if (BIT(A,10) && !BIT(A,9) && BIT(A,3))
			m_decrypted_opcodes[A] ^= 0x20;

		if ((BIT(A,10) ^ BIT(A,9)) && BIT(A,1))
			m_decrypted_opcodes[A] ^= 0x02;

		if (BIT(A,9) || !BIT(A,5) || BIT(A,3))
			m_decrypted_opcodes[A] = BITSWAP8(m_decrypted_opcodes[A],7,6,1,4,3,2,5,0);

		/* decode the data */
		if (BIT(A,5))
			rom[A] ^= 0x40;

		if (BIT(A,9) || !BIT(A,5))
			rom[A] = BITSWAP8(rom[A],7,6,1,4,3,2,5,0);
	}
}



GAME( 1989, metlfrzr,  0,    metlfrzr, metlfrzr, metlfrzr_state,  metlfrzr, ROT270, "Seibu", "Metal Freezer", MACHINE_NOT_WORKING | MACHINE_NO_SOUND )
