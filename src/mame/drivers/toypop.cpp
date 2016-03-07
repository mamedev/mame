// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/****************************************

Libble Rabble (c) 1983 Namco
Toypop        (c) 1986 Namco

Namco "Universal System 16" Hardware

Notes:
------
- Libble Rabble Easter egg:
  - enter service mode
  - turn off the service mode switch, and turn it on again quickly to remain
    on the monitor test grid
  - Enter the following sequence using the right joystick:
    9xU 2xR 9xD 2xL
  (c) 1983 NAMCO LTD. will appear on the screen.

****************************************/

#include "emu.h"
#include "cpu/m6809/m6809.h"
#include "cpu/m68000/m68000.h"

#define MASTER_CLOCK XTAL_6_144MHz

class toypop_state : public driver_device
{
public:
	toypop_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_master_cpu(*this,"maincpu"),
		m_gfxdecode(*this, "gfxdecode"),
		m_fgvram(*this, "fgvram"),
		m_fgattr(*this, "fgattr")
	{ }
	
	required_device<cpu_device> m_master_cpu;

	required_device<gfxdecode_device> m_gfxdecode;

	required_shared_ptr<UINT8> m_fgvram;
	required_shared_ptr<UINT8> m_fgattr;

	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	
	DECLARE_READ8_MEMBER(irq_enable_r);
	INTERRUPT_GEN_MEMBER(master_vblank_irq);
	DECLARE_PALETTE_INIT(toypop); 
	
protected:
	// driver_device overrides
//	virtual void machine_start() override;
	virtual void machine_reset() override;

//	virtual void video_start() override;
private:
	bool m_master_irq_enable;
	
	void legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect);

};

PALETTE_INIT_MEMBER(toypop_state, toypop)
{
	const UINT8 *color_prom = memregion("proms")->base();

	for (int i = 0;i < 256;i++)
	{
		int bit0,bit1,bit2,bit3,r,g,b;

		// red component
		bit0 = (color_prom[i] >> 0) & 0x01;
		bit1 = (color_prom[i] >> 1) & 0x01;
		bit2 = (color_prom[i] >> 2) & 0x01;
		bit3 = (color_prom[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// green component
		bit0 = (color_prom[i+0x100] >> 0) & 0x01;
		bit1 = (color_prom[i+0x100] >> 1) & 0x01;
		bit2 = (color_prom[i+0x100] >> 2) & 0x01;
		bit3 = (color_prom[i+0x100] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		// blue component
		bit0 = (color_prom[i+0x200] >> 0) & 0x01;
		bit1 = (color_prom[i+0x200] >> 1) & 0x01;
		bit2 = (color_prom[i+0x200] >> 2) & 0x01;
		bit3 = (color_prom[i+0x200] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		palette.set_indirect_color(i, rgb_t(r,g,b));
	}

	for (int i = 0;i < 256;i++)
	{
		UINT8 entry;

		// characters
		palette.set_pen_indirect(i + 0*256, (color_prom[i + 0x300] & 0x0f) | 0x70);
		palette.set_pen_indirect(i + 1*256, (color_prom[i + 0x300] & 0x0f) | 0xf0);
		// sprites
		entry = color_prom[i + 0x500];
		palette.set_pen_indirect(i + 2*256, entry);
	}
	for (int i = 0;i < 16;i++)
	{
		// background
		palette.set_pen_indirect(i + 3*256 + 0*16, 0x60 + i);
		palette.set_pen_indirect(i + 3*256 + 1*16, 0xe0 + i);
	}
}

void toypop_state::legacy_fg_draw(bitmap_ind16 &bitmap,const rectangle &cliprect)
{
	gfx_element *gfx_0 = m_gfxdecode->gfx(0);
	int count;
	
	for (count=0;count<32*32;count++)
	{
		int x;// = (count % 32);
		int y; //= count / 32;
		
		if(count < 64)
		{
			x = 34 + (count / 32);
			y = (count % 32) - 2;
		}
		else if(count >= 32*30)
		{
			x = (count / 32) - 30;
			y = (count % 32) - 2;
		}
		else
		{
			x = 2 + (count % 32);
			y = (count / 32) - 2;
		}
		
		UINT16 tile = m_fgvram[count];
		UINT8 color = m_fgattr[count] & 0x3f;

			//if((color & 0x30) != 0x30)
		gfx_0->opaque(bitmap,cliprect,tile,color,0,0,x*8,y*8);

	}
}

UINT32 toypop_state::screen_update( screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect )
{
	legacy_fg_draw(bitmap,cliprect);
	return 0;
}

READ8_MEMBER(toypop_state::irq_enable_r)
{
	m_master_irq_enable = true;
	return 0;
}

static ADDRESS_MAP_START( master_map, AS_PROGRAM, 8, toypop_state )
	AM_RANGE(0x0000, 0x03ff) AM_RAM AM_SHARE("fgvram")
	AM_RANGE(0x0400, 0x07ff) AM_RAM AM_SHARE("fgattr")
	AM_RANGE(0x0800, 0x1fff) AM_RAM
	AM_RANGE(0x2800, 0x2fff) AM_RAM
	// 0x6000 - 0x603f namco i/o
	AM_RANGE(0x6800, 0x6bff) AM_RAM // shared ram device
	AM_RANGE(0x7000, 0x7000) AM_READ(irq_enable_r)
	// 0x8000 m68k irq
	// 0x9000 sound irq
	// 0xa000 paletteram
	AM_RANGE(0x8000, 0xffff) AM_ROM AM_REGION("master_rom",0)
ADDRESS_MAP_END

static INPUT_PORTS_START( liblrabl )
INPUT_PORTS_END

static INPUT_PORTS_START( toypop )
INPUT_PORTS_END


static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 8*8+0, 8*8+1, 8*8+2, 8*8+3, 0, 1, 2, 3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	2,
	{ 0, 4 },
	{ 0, 1, 2, 3, 8*8, 8*8+1, 8*8+2, 8*8+3, 16*8+0, 16*8+1, 16*8+2, 16*8+3,
	24*8+0, 24*8+1, 24*8+2, 24*8+3 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
	32 * 8, 33 * 8, 34 * 8, 35 * 8, 36 * 8, 37 * 8, 38 * 8, 39 * 8 },
	64*8
};

static GFXDECODE_START( toypop )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,       0, 128 )
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout, 128*4,  64 )
GFXDECODE_END

void toypop_state::machine_reset()
{
	m_master_irq_enable = false;
}

INTERRUPT_GEN_MEMBER(toypop_state::master_vblank_irq)
{
	if(m_master_irq_enable == true)
		device.execute().set_input_line(M6809_IRQ_LINE,HOLD_LINE);
}

static MACHINE_CONFIG_START( liblrabl, toypop_state )
 	MCFG_CPU_ADD("maincpu", M6809, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(master_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", toypop_state,  master_vblank_irq)

#if 0
	MCFG_CPU_ADD("slave", M68000, MASTER_CLOCK)
	MCFG_CPU_PROGRAM_MAP(slave_map) 

 	MCFG_CPU_ADD("audiocpu", M6809, MASTER_CLOCK/4)
	MCFG_CPU_PROGRAM_MAP(sound_map)

	
	
#endif

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60.606060)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(36*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 36*8-1, 0*8, 32*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(toypop_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", toypop)
	MCFG_PALETTE_ADD("palette", 128*4+64*4+16*2)
	MCFG_PALETTE_INDIRECT_ENTRIES(256)
	MCFG_PALETTE_INIT_OWNER(toypop_state, toypop)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( toypop, liblrabl )
MACHINE_CONFIG_END


ROM_START( liblrabl )
	ROM_REGION( 0x8000, "master_rom", 0 )
	ROM_LOAD( "5b.rom",   0x0000, 0x4000, CRC(da7a93c2) SHA1(fe4a02cdab66722eb7b8cf58825f899b1949a6a2) )
	ROM_LOAD( "5c.rom",   0x4000, 0x4000, CRC(6cae25dc) SHA1(de74317a7d5de1865d096c377923a764be5e6879) )

	ROM_REGION( 0x2000, "sound_rom", 0 )
	ROM_LOAD( "2c.rom",   0x0000, 0x2000, CRC(7c09e50a) SHA1(5f004d60bbb7355e008a9cda137b28bc2192b8ef) )

	ROM_REGION( 0x8000, "slave_rom", 0 )
	ROM_LOAD16_BYTE( "8c.rom",   0x0000, 0x4000, CRC(a00cd959) SHA1(cc5621103c31cfbc65941615cab391db0f74e6ce) )
	ROM_LOAD16_BYTE("10c.rom",   0x0001, 0x4000, CRC(09ce209b) SHA1(2ed46d6592f8227bac8ab54963d9a300706ade47) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5p.rom",   0x0000, 0x2000, CRC(3b4937f0) SHA1(06d9de576f1c2262c34aeb91054e68c9298af688) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "9t.rom",   0x0000, 0x4000, CRC(a88e24ca) SHA1(eada133579f19de09255084dcdc386311606a335) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "lr1-3.1r", 0x0000, 0x0100, CRC(f3ec0d07) SHA1(b0aad1fb6df79f202889600f486853995352f9c2) )
	ROM_LOAD( "lr1-2.1s", 0x0100, 0x0100, CRC(2ae4f702) SHA1(838fdca9e91fea4f64a59880ac47c48973bb8fbf) )
	ROM_LOAD( "lr1-1.1t", 0x0200, 0x0100, CRC(7601f208) SHA1(572d070ca387b780030ed5de38a8970b7cc14349) )
	ROM_LOAD( "lr1-5.5l", 0x0300, 0x0100, CRC(940f5397) SHA1(825a7bd78a8a08d30bad2e4890ae6e9ad88b36b8) )
	ROM_LOAD( "lr1-6.2p", 0x0400, 0x0200, CRC(a6b7f850) SHA1(7cfde16dfd5c4d5b876b4fbe4f924f1385932a93) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "lr1-4.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

ROM_START( toypop )
	ROM_REGION( 0x8000, "master_rom", 0 )
	ROM_LOAD( "tp1-2.5b", 0x0000, 0x4000, CRC(87469620) SHA1(2ee257486c9c044386ac7d0cd4a90583eaeb3e97) )
	ROM_LOAD( "tp1-1.5c", 0x4000, 0x4000, CRC(dee2fd6e) SHA1(b2c12008d6d3e7544ba3c12a52a6abf9181842c8) )

	ROM_REGION( 0x2000, "sound_rom", 0 )
	ROM_LOAD( "tp1-3.2c", 0x0000, 0x2000, CRC(5f3bf6e2) SHA1(d1b3335661b9b23cb10001416c515b77b5e783e9) )

	ROM_REGION( 0x8000, "slave_rom", 0 )
	ROM_LOAD16_BYTE( "tp1-4.8c", 0x0000, 0x4000, CRC(76997db3) SHA1(5023a2f20a5f2c9baff130f6832583493c71f883) )
	ROM_LOAD16_BYTE("tp1-5.10c", 0x0001, 0x4000, CRC(37de8786) SHA1(710365e34c05d01815844c414518f93234b6160b) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "tp1-7.5p", 0x0000, 0x2000, CRC(95076f9e) SHA1(1e3d32b21f6d46591ec3921aba51f672d64a9023) )

	ROM_REGION( 0x4000, "gfx2", 0 )
	ROM_LOAD( "tp1-6.9t", 0x0000, 0x4000, CRC(481ffeaf) SHA1(c51735ad3a1dbb46ad414408b54554e9223b2219) )

	ROM_REGION( 0x0600, "proms", 0 )
	ROM_LOAD( "tp1-3.1r", 0x0000, 0x0100, CRC(cfce2fa5) SHA1(b42aa0f34d885389d2650bf7a0531b95703b8a28) )
	ROM_LOAD( "tp1-2.1s", 0x0100, 0x0100, CRC(aeaf039d) SHA1(574560526100d38635aecd71eb73499c4f57d586) )
	ROM_LOAD( "tp1-1.1t", 0x0200, 0x0100, CRC(08e7cde3) SHA1(5261aca6834d635d17f8afaa8e35848930030ba4) )
	ROM_LOAD( "tp1-4.5l", 0x0300, 0x0100, CRC(74138973) SHA1(2e21dbb1b19dd089da52e70fcb0ca91336e004e6) )
	ROM_LOAD( "tp1-5.2p", 0x0400, 0x0200, CRC(4d77fa5a) SHA1(2438910314b23ecafb553230244f3931861ad2da) )

	ROM_REGION( 0x0100, "namco", 0 )
	ROM_LOAD( "tp1-6.3d", 0x0000, 0x0100, CRC(16a9166a) SHA1(847cbaf7c88616576c410177e066ae1d792ac0ba) )
ROM_END

GAME( 1983, liblrabl, 0,     liblrabl, liblrabl, driver_device, 0,   ROT0,   "Namco", "Libble Rabble", MACHINE_IS_SKELETON )
GAME( 1986, toypop,   0,     toypop,   toypop, driver_device,   0,   ROT0,   "Namco", "Toypop", MACHINE_IS_SKELETON )
