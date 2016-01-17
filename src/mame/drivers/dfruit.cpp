// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

    Fruit Dream (c) 1993 Nippon Data Kiki / Star Fish

    driver by Angelo Salese

    Uses a TC0091LVC, a variant of the one used on Taito L HW

    TODO:
    - title screen (PCG uploads at 0x1b400?)
    - inputs are grossly mapped;
    - lamps?
    - service mode?
    - nvram?

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/2203intf.h"
#include "machine/tc009xlvc.h"
#include "machine/i8255.h"

class dfruit_state : public driver_device
{
public:
	dfruit_state(const machine_config &mconfig, device_type type, std::string tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_vdp(*this, "tc0091lvc")
		{ }

	required_device<cpu_device> m_maincpu;
	required_device<tc0091lvc_device> m_vdp;

	virtual void video_start() override;
	UINT32 screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	void screen_eof(screen_device &screen, bool state);

	UINT8 m_ram_bank[4];
	UINT8 m_rom_bank;
	UINT8 m_irq_vector[3];
	UINT8 m_irq_enable;

	DECLARE_READ8_MEMBER(dfruit_rom_r);

	DECLARE_READ8_MEMBER(dfruit_ram_0_r);
	DECLARE_READ8_MEMBER(dfruit_ram_1_r);
	DECLARE_READ8_MEMBER(dfruit_ram_2_r);
	DECLARE_READ8_MEMBER(dfruit_ram_3_r);
	DECLARE_WRITE8_MEMBER(dfruit_ram_0_w);
	DECLARE_WRITE8_MEMBER(dfruit_ram_1_w);
	DECLARE_WRITE8_MEMBER(dfruit_ram_2_w);
	DECLARE_WRITE8_MEMBER(dfruit_ram_3_w);

	DECLARE_READ8_MEMBER(dfruit_rom_bank_r);
	DECLARE_WRITE8_MEMBER(dfruit_rom_bank_w);
	DECLARE_READ8_MEMBER(dfruit_ram_bank_r);
	DECLARE_WRITE8_MEMBER(dfruit_ram_bank_w);
	DECLARE_READ8_MEMBER(dfruit_irq_vector_r);
	DECLARE_WRITE8_MEMBER(dfruit_irq_vector_w);
	DECLARE_READ8_MEMBER(dfruit_irq_enable_r);
	DECLARE_WRITE8_MEMBER(dfruit_irq_enable_w);

	UINT8 ram_bank_r(UINT16 offset, UINT8 bank_num);
	void ram_bank_w(UINT16 offset, UINT8 data, UINT8 bank_num);
	TIMER_DEVICE_CALLBACK_MEMBER(dfruit_irq_scanline);
};

void dfruit_state::video_start()
{
}

UINT32 dfruit_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	m_vdp->screen_update(screen, bitmap, cliprect);

	return 0;
}

void dfruit_state::screen_eof(screen_device &screen, bool state)
{
	if (state)
	{
		m_vdp->screen_eof();
	}
}

READ8_MEMBER(dfruit_state::dfruit_rom_r)
{
	UINT8 *ROM = memregion("maincpu")->base();

	return ROM[offset + m_rom_bank * 0x2000];
}

READ8_MEMBER(dfruit_state::dfruit_rom_bank_r)
{
	return m_rom_bank;
}

WRITE8_MEMBER(dfruit_state::dfruit_rom_bank_w)
{
	m_rom_bank = data;
}

READ8_MEMBER(dfruit_state::dfruit_irq_vector_r)
{
	return m_irq_vector[offset];
}

WRITE8_MEMBER(dfruit_state::dfruit_irq_vector_w)
{
	m_irq_vector[offset] = data;
}

READ8_MEMBER(dfruit_state::dfruit_irq_enable_r)
{
	return m_irq_enable;
}

WRITE8_MEMBER(dfruit_state::dfruit_irq_enable_w)
{
	m_irq_enable = data;
}

READ8_MEMBER(dfruit_state::dfruit_ram_bank_r)
{
	return m_ram_bank[offset];
}

WRITE8_MEMBER(dfruit_state::dfruit_ram_bank_w)
{
	m_ram_bank[offset] = data;
}

UINT8 dfruit_state::ram_bank_r(UINT16 offset, UINT8 bank_num)
{
	address_space &vdp_space = machine().device<tc0091lvc_device>("tc0091lvc")->space();
	return vdp_space.read_byte(offset + (m_ram_bank[bank_num]) * 0x1000);;
}

void dfruit_state::ram_bank_w(UINT16 offset, UINT8 data, UINT8 bank_num)
{
	address_space &vdp_space = machine().device<tc0091lvc_device>("tc0091lvc")->space();
	vdp_space.write_byte(offset + (m_ram_bank[bank_num]) * 0x1000,data);;
}

READ8_MEMBER(dfruit_state::dfruit_ram_0_r) { return ram_bank_r(offset, 0); }
READ8_MEMBER(dfruit_state::dfruit_ram_1_r) { return ram_bank_r(offset, 1); }
READ8_MEMBER(dfruit_state::dfruit_ram_2_r) { return ram_bank_r(offset, 2); }
READ8_MEMBER(dfruit_state::dfruit_ram_3_r) { return ram_bank_r(offset, 3); }
WRITE8_MEMBER(dfruit_state::dfruit_ram_0_w) { ram_bank_w(offset, data, 0); }
WRITE8_MEMBER(dfruit_state::dfruit_ram_1_w) { ram_bank_w(offset, data, 1); }
WRITE8_MEMBER(dfruit_state::dfruit_ram_2_w) { ram_bank_w(offset, data, 2); }
WRITE8_MEMBER(dfruit_state::dfruit_ram_3_w) { ram_bank_w(offset, data, 3); }

static ADDRESS_MAP_START( tc0091lvc_map, AS_PROGRAM, 8, dfruit_state )
	AM_RANGE(0x0000, 0x5fff) AM_ROM
	AM_RANGE(0x6000, 0x7fff) AM_READ(dfruit_rom_r)

	AM_RANGE(0x8000, 0x9fff) AM_RAM

	AM_RANGE(0xc000, 0xcfff) AM_READWRITE(dfruit_ram_0_r,dfruit_ram_0_w)
	AM_RANGE(0xd000, 0xdfff) AM_READWRITE(dfruit_ram_1_r,dfruit_ram_1_w)
	AM_RANGE(0xe000, 0xefff) AM_READWRITE(dfruit_ram_2_r,dfruit_ram_2_w)
	AM_RANGE(0xf000, 0xfdff) AM_READWRITE(dfruit_ram_3_r,dfruit_ram_3_w)

	AM_RANGE(0xfe00, 0xfeff) AM_DEVREADWRITE("tc0091lvc", tc0091lvc_device, vregs_r, vregs_w)
	AM_RANGE(0xff00, 0xff02) AM_READWRITE(dfruit_irq_vector_r, dfruit_irq_vector_w)
	AM_RANGE(0xff03, 0xff03) AM_READWRITE(dfruit_irq_enable_r, dfruit_irq_enable_w)
	AM_RANGE(0xff04, 0xff07) AM_READWRITE(dfruit_ram_bank_r, dfruit_ram_bank_w)
	AM_RANGE(0xff08, 0xff08) AM_READWRITE(dfruit_rom_bank_r, dfruit_rom_bank_w)
ADDRESS_MAP_END


static ADDRESS_MAP_START( dfruit_map, AS_PROGRAM, 8, dfruit_state )
	AM_IMPORT_FROM(tc0091lvc_map)
	AM_RANGE(0xa000, 0xa003) AM_DEVREADWRITE("ppi8255_0", i8255_device, read, write)
	AM_RANGE(0xa004, 0xa005) AM_DEVREADWRITE("opn", ym2203_device, read, write)
	AM_RANGE(0xa008, 0xa008) AM_READNOP //watchdog
ADDRESS_MAP_END


static INPUT_PORTS_START( dfruit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_SERVICE1 ) PORT_NAME("Bookkeeping")
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_GAMBLE_KEYIN )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_TAKE )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE2 ) PORT_NAME("Alt Bookkeeping") // same as above
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_SLOT_STOP1 ) PORT_NAME("Stop Reel 1 / Double-Up")
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SLOT_STOP3 ) PORT_NAME("Stop Reel 3 / Black")
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_GAMBLE_BET )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SLOT_STOP2 ) PORT_NAME("Stop Reel 2 / Red")
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "DSWA" )
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
INPUT_PORTS_END

static const gfx_layout bg2_layout =
{
	8, 8,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};

#define O 8*8*4
#define O2 2*O
static const gfx_layout sp2_layout =
{
	16, 16,
	RGN_FRAC(1,1),
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16, O+3, O+2, O+1, O+0, O+19, O+18, O+17, O+16 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32, O2+0*32, O2+1*32, O2+2*32, O2+3*32, O2+4*32, O2+5*32, O2+6*32, O2+7*32 },
	8*8*4*4
};
#undef O
#undef O2

#if 0
static const gfx_layout char_layout =
{
	8, 8,
	1024,
	4,
	{ 8, 12, 0, 4 },
	{ 3, 2, 1, 0, 19, 18, 17, 16},
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32 },
	8*8*4
};
#endif

static GFXDECODE_START( dfruit )
	GFXDECODE_ENTRY( "gfx1", 0, bg2_layout, 0, 16 )
	GFXDECODE_ENTRY( "gfx1", 0, sp2_layout, 0, 16 )
	//GFXDECODE_ENTRY( NULL,           0, char_layout,  0, 16 )  // Ram-based
GFXDECODE_END

TIMER_DEVICE_CALLBACK_MEMBER(dfruit_state::dfruit_irq_scanline)
{
	int scanline = param;

	if (scanline == 240 && (m_irq_enable & 4))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_vector[2]);
	}

	if (scanline == 0 && (m_irq_enable & 2))
	{
		m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_vector[1]);
	}

	if (scanline == 196 && (m_irq_enable & 1))
	{
		//m_maincpu->set_input_line_and_vector(0, HOLD_LINE, m_irq_vector[0]);
	}
}

#define MASTER_CLOCK XTAL_14MHz

static MACHINE_CONFIG_START( dfruit, dfruit_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",Z80,MASTER_CLOCK/2) //!!! TC0091LVC !!!
	MCFG_CPU_PROGRAM_MAP(dfruit_map)
	MCFG_TIMER_DRIVER_ADD_SCANLINE("scantimer", dfruit_state, dfruit_irq_scanline, "screen", 0, 1)

	//MCFG_MACHINE_START_OVERRIDE(dfruit_state,4enraya)
	//MCFG_MACHINE_RESET_OVERRIDE(dfruit_state,4enraya)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_SIZE(64*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 2*8, 30*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(dfruit_state, screen_update)
	MCFG_SCREEN_VBLANK_DRIVER(dfruit_state, screen_eof)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", dfruit )
	MCFG_PALETTE_ADD("palette", 0x100)

	MCFG_DEVICE_ADD("tc0091lvc", TC0091LVC, 0)
	MCFG_TC0091LVC_GFXDECODE("gfxdecode")
	MCFG_TC0091LVC_PALETTE("palette")

	MCFG_DEVICE_ADD("ppi8255_0", I8255A, 0)
	MCFG_I8255_IN_PORTA_CB(IOPORT("IN0"))
	MCFG_I8255_IN_PORTB_CB(IOPORT("IN1"))
	MCFG_I8255_IN_PORTC_CB(IOPORT("IN2"))

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD("opn", YM2203, MASTER_CLOCK/4)
	MCFG_AY8910_PORT_A_READ_CB(IOPORT("IN4"))
	MCFG_AY8910_PORT_B_READ_CB(IOPORT("IN5"))
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END

/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( dfruit )
	ROM_REGION( 0x40000, "maincpu", 0 )
	ROM_LOAD( "n-3800ii_ver.1.20.ic2", 0x00000, 0x40000, CRC(4e7c3700) SHA1(17bc731a91460d8f67c2b2b6e038641d57cf93be) )

	ROM_REGION( 0x80000, "gfx1", 0 )
	ROM_LOAD( "c2.ic10", 0x00000, 0x80000, CRC(d869ab24) SHA1(382e874a846855a7f6f8811625aaa30d9dfa1ce2) )
ROM_END

GAME( 1993, dfruit,  0,   dfruit, dfruit, driver_device,  0, ROT0, "Nippon Data Kiki / Star Fish", "Fruit Dream (Japan)", MACHINE_IMPERFECT_GRAPHICS )
