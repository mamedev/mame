// license:BSD-3-Clause
// copyright-holders:Angelo Salese
/***************************************************************************

Centipede / Millipede / Missile Command / Let's Go Bowling
(c) 1980-2 / 2002 - Infogrames / CosmoDog

preliminary driver by Angelo Salese

Earlier revisions of this cabinet did not include the bowling game.
 Known to exist "CMM Rev 1.03" (without Let's Go Bowling)
 Let's Go Bowling is actually a completely new game by Cosmodog, not
 a port or prototype of an old Atari game.

TODO:
- flash ROM hookup (by the looks of it, at29x needs an address map?);
- finish video emulation;
- trackball inputs
- sound;
- NVRAM (flash ROM, as per NVRAM test at 0xC2A7).
- untangle switch-cases for inputs;
- One bit of the control register seems to want to bank the flash into 2000-7FFF
  which would require bankdev if verified.
- IRQ handler is super-strange, it wants to JMP ($7FFE) and JMP ($3FFE) a lot,
  which currently sends the game off into the weeds.
- In fact, the game goes off into the weeds anyway right now due to doing that.

Probably on the CPLD (CY39100V208B) - Quoted from Cosmodog's website:
 "Instead, we used a programmable chip that we could reconfigure very
 quickly while the game is running. So, during that 1/8th of a second
 or so when the screen goes black while it switches games, it's actually
 reloading the hardware with a whole new design to run the next game."

============================================================================

Centipede / Millipede / Missile Command / Let's Go Bowling.
Team Play

1.00 PCB by CosmoDog + sticker "Multipede"

U1  = WDC 65C02S8P-14
U2  = Flash ROM AT29C020 (256KB)
U4  = ISSI IS62LV256-45J (32KB)
U5  = CY39100V208B (Cypress CPLD)
U9  = CY37128-P100 (Cypress CPLD)
U10 = CYC1399 (?)

OSC @ 72.576MHz

***************************************************************************/

#include "emu.h"
#include "cpu/m6502/m65sc02.h"
#include "machine/at29x.h"
#include "screen.h"

#define MAIN_CLOCK XTAL(72'576'000)

class cmmb_state : public driver_device
{
public:
	cmmb_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "at29c020" ),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<at29c020_device> m_flash;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;

	uint8_t m_irq_mask;

	DECLARE_READ8_MEMBER(cmmb_charram_r);
	DECLARE_WRITE8_MEMBER(cmmb_charram_w);
	DECLARE_READ8_MEMBER(cmmb_input_r);
	DECLARE_WRITE8_MEMBER(cmmb_output_w);
	DECLARE_WRITE8_MEMBER(flash_dbg_0_w);
	DECLARE_WRITE8_MEMBER(flash_dbg_1_w);

	DECLARE_WRITE8_MEMBER(irq_ack_w)
	{
		m_maincpu->set_input_line(0, CLEAR_LINE);
	}

	DECLARE_WRITE8_MEMBER(irq_enable_w)
	{
		m_irq_mask = data & 0x80;
	}

	//DECLARE_READ8_MEMBER(kludge_r);
	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cmmb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void cmmb(machine_config &config);
	void cmmb_map(address_map &map);
};


void cmmb_state::video_start()
{
}

uint32_t cmmb_state::screen_update_cmmb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	uint8_t *videoram = m_videoram;
	gfx_element *gfx = m_gfxdecode->gfx(0);
	int count = 0x00000;

	int y,x;

	for (y=0;y<32;y++)
	{
		for (x=0;x<32;x++)
		{
			int tile = videoram[count] & 0x3f;
			int colour = (videoram[count] & 0xc0)>>6;
			gfx->opaque(bitmap,cliprect,tile,colour,0,0,x*8,y*8);

			count++;
		}
	}

	return 0;
}

READ8_MEMBER(cmmb_state::cmmb_charram_r)
{
	uint8_t *GFX = memregion("gfx")->base();

	return GFX[offset];
}

WRITE8_MEMBER(cmmb_state::cmmb_charram_w)
{
	uint8_t *GFX = memregion("gfx")->base();

	GFX[offset] = data;

	offset&=0xfff;

	/* dirty char */
	m_gfxdecode->gfx(0)->mark_dirty(offset >> 4);
	m_gfxdecode->gfx(1)->mark_dirty(offset >> 5);
}

READ8_MEMBER(cmmb_state::cmmb_input_r)
{
	//printf("%02x R\n",offset);
	switch(offset)
	{
		case 0x00: return ioport("IN2")->read();
		case 0x03: return 4; // incorrect image U9 otherwise (???)
		case 0x0e: return ioport("IN0")->read();
		case 0x0f: return ioport("IN1")->read();
	}

	return 0xff;
}

WRITE8_MEMBER(cmmb_state::cmmb_output_w)
{
	//printf("%02x -> [%02x] W\n",data,offset);
	switch(offset)
	{
		case 0x01:
//          m_irq_mask = data & 0x80;
			break;
		case 0x02:
			// bit 7 toggled - watchdog or status LED
			// toggled by code at E3DB in IRQ handler - it's on when the frame count & 0x30 is 1 and off otherwise
			// bit 6 set means accessing flash ROM, possibly that entire 2000-7FFF banks over to the flash?
			break;

		case 0x03:
			{
				uint8_t *ROM = memregion("maincpu")->base();
				uint32_t bankaddress;

				bankaddress = 0x10000 + (0x4000 * (data & 0x0f));
				membank("bank1")->set_base(&ROM[bankaddress]);
			}
			break;

		case 0x07:
			break;
	}
}

WRITE8_MEMBER(cmmb_state::flash_dbg_0_w)
{
	m_flash->write(space,0x2aaa,data);
}

WRITE8_MEMBER(cmmb_state::flash_dbg_1_w)
{
	m_flash->write(space,0x5555,data);
}

/* overlap empty addresses */
ADDRESS_MAP_START(cmmb_state::cmmb_map)
	ADDRESS_MAP_GLOBAL_MASK(0xffff)
	AM_RANGE(0x0000, 0x0fff) AM_RAM /* zero page address */
//  AM_RANGE(0x13c0, 0x13ff) AM_RAM //spriteram
	AM_RANGE(0x1000, 0x13ff) AM_RAM AM_SHARE("videoram")
	AM_RANGE(0x2000, 0x2000) AM_READ_PORT("IN3")
	AM_RANGE(0x2001, 0x2001) AM_READ_PORT("IN4")
	AM_RANGE(0x2011, 0x2011) AM_READ_PORT("IN5")
	AM_RANGE(0x2480, 0x249f) AM_RAM_DEVWRITE("palette", palette_device, write8) AM_SHARE("palette")
	AM_RANGE(0x2505, 0x2505) AM_WRITE(irq_enable_w)
	AM_RANGE(0x2600, 0x2600) AM_WRITE(irq_ack_w)
	//AM_RANGE(0x4000, 0x400f) AM_READWRITE(cmmb_input_r,cmmb_output_w)
	//AM_RANGE(0x4900, 0x4900) AM_READ(kludge_r)
	AM_RANGE(0x4000, 0x7fff) AM_ROMBANK("bank1")
	AM_RANGE(0x8000, 0x9fff) AM_ROM AM_REGION("maincpu", 0x18000)
	AM_RANGE(0xa000, 0xafff) AM_RAM
	AM_RANGE(0xb000, 0xbfff) AM_READWRITE(cmmb_charram_r,cmmb_charram_w)
	AM_RANGE(0xc000, 0xc00f) AM_READWRITE(cmmb_input_r,cmmb_output_w)
	// debugging, to be removed
//  AM_RANGE(0x2aaa, 0x2aaa) AM_WRITE(flash_dbg_0_w)
//  AM_RANGE(0x5555, 0x5555) AM_WRITE(flash_dbg_1_w)
	AM_RANGE(0xc010, 0xffff) AM_ROM AM_REGION("maincpu", 0x1c010)
ADDRESS_MAP_END


static INPUT_PORTS_START( cmmb )
	PORT_START("IN3")
	PORT_DIPNAME( 0x01, 0x01, "IN3" )
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

	PORT_START("IN4")
	PORT_DIPNAME( 0x01, 0x01, "IN4" )
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

	// TODO: pin-point writes for trackball
	// TODO: trackball might be muxed for 1p & 2p sides
	PORT_START("IN5")
	PORT_DIPNAME( 0x01, 0x01, "IN5" ) // trackball V clk
	PORT_DIPSETTING(    0x01, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Unknown ) ) // trackball V dir
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Unknown ) ) // trackball H clk
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, DEF_STR( Unknown ) ) // trackball H dir
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_DIPNAME( 0x20, 0x20, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x20, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_DIPNAME( 0x40, 0x40, "IN0" )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_VOLUME_DOWN )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_VOLUME_UP )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_START1 )
	PORT_DIPNAME( 0x10, 0x10, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x10, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Unknown ) )
	PORT_DIPSETTING(    0x40, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x80, 0x80, DEF_STR( Service_Mode ) )
	PORT_DIPSETTING(    0x80, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_START("IN2")
	PORT_DIPNAME( 0x01, 0x01, "IN2" )
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

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	8*16
};

static const gfx_layout spritelayout =
{
	8,16,
	RGN_FRAC(1,1),
	2,
	{ 1, 0 },
	{ 6, 4, 2, 0, 14, 12, 10, 8 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,8*16,9*16,10*16,11*16,12*16,13*16,14*16,15*16 },
	8*32
};


static GFXDECODE_START( cmmb )
	GFXDECODE_ENTRY( "gfx", 0, charlayout,     0x00, 4 )
	GFXDECODE_ENTRY( "gfx", 0, spritelayout,   0x10, 4 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cmmb_state::vblank_irq)
{
	#if 0
	if (m_irq_mask & 0x80)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
	#endif
}

void cmmb_state::machine_reset()
{
	m_irq_mask = 0;
}


MACHINE_CONFIG_START(cmmb_state::cmmb)

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", M65SC02, MAIN_CLOCK/5) // Unknown clock, but chip rated for 14MHz
	MCFG_CPU_PROGRAM_MAP(cmmb_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", cmmb_state, vblank_irq)

	MCFG_AT29C020_ADD("at29c020")

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/12, 384, 0, 256, 264, 0, 240) // TBD, not real measurements
	MCFG_SCREEN_UPDATE_DRIVER(cmmb_state, screen_update_cmmb)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", cmmb)

	MCFG_PALETTE_ADD("palette", 512)
	MCFG_PALETTE_FORMAT(RRRGGGBB_inverted)

	/* sound hardware */
//  MCFG_SPEAKER_STANDARD_MONO("mono")
//  MCFG_SOUND_ADD("aysnd", AY8910, 8000000/4)
//  MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.30)
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cmmb162 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cmmb162.u2",   0x10000, 0x40000, CRC(71a5a75d) SHA1(0ad7b97580082cda98cb1e8aab8efcf491d0ed25) )
	ROM_COPY( "maincpu",      0x18000, 0x08000, 0x08000 )
	ROM_FILL( 0x1c124, 2, 0xea ) // temporary patch to avoid waiting on IRQs
	ROM_FILL( 0x1e3c3, 3, 0xea ) // patch out weird IRQ handler code that causes reboot each IRQ

	ROM_REGION( 0x1000, "gfx", ROMREGION_ERASE00 )
ROM_END

GAME( 2002, cmmb162,  0,       cmmb,  cmmb, cmmb_state,  0, ROT270, "Cosmodog / Team Play (Licensed from Infogrames via Midway Games West)", "Centipede / Millipede / Missile Command / Let's Go Bowling (rev 1.62)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
