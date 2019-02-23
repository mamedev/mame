// license:BSD-3-Clause
// copyright-holders:Angelo Salese, R. Belmont
/***************************************************************************

Centipede / Millipede / Missile Command / Let's Go Bowling
(c) 1980-2 / 2002 - Infogrames / CosmoDog

preliminary driver by Angelo Salese

Earlier revisions of this cabinet did not include the bowling game.
 Known to exist "CMM Rev 1.03" (without Let's Go Bowling)
 Let's Go Bowling is actually a completely new game by Cosmodog, not
 a port or prototype of an old Atari game.

TODO:
- flash ROM hookup
- finish video emulation;
- trackball inputs
- sound;
- NVRAM (flash ROM, as per NVRAM test at 0xC2A7).
- untangle switch-cases for inputs;
- IRQs are problematic.  Haven't yet found a reliable enable that works
  for both mainline and service mode; maybe there's no mask and the
  processor's SEI/CLI instructions are used for that?
  - If IRQs are enabled in service mode, they eventually trash all of memory.


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
#include "machine/bankdev.h"
#include "emupal.h"
#include "screen.h"

#define MAIN_CLOCK XTAL(72'576'000)

class cmmb_state : public driver_device
{
public:
	cmmb_state(const machine_config &mconfig, device_type type, const char *tag) :
		driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_flash(*this, "at29c020" ),
		m_videoram(*this, "videoram"),
		m_gfxdecode(*this, "gfxdecode"),
		m_palette(*this, "palette"),
		m_bnk2000(*this, "bnk2000")
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<at29c020_device> m_flash;
	required_shared_ptr<uint8_t> m_videoram;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<palette_device> m_palette;
	required_device<address_map_bank_device> m_bnk2000;

	uint8_t m_irq_mask;

	DECLARE_READ8_MEMBER(cmmb_charram_r);
	DECLARE_WRITE8_MEMBER(cmmb_charram_w);
	DECLARE_READ8_MEMBER(cmmb_input_r);
	DECLARE_WRITE8_MEMBER(cmmb_output_w);
	DECLARE_READ8_MEMBER(flash_r);
	DECLARE_WRITE8_MEMBER(flash_w);

	virtual void machine_reset() override;
	virtual void video_start() override;
	uint32_t screen_update_cmmb(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(vblank_irq);
	void cmmb(machine_config &config);
	void cmmb_map(address_map &map);
	void bnk2000_map(address_map &map);
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
			int tile = videoram[count] & 0x7f;
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

READ8_MEMBER(cmmb_state::flash_r)
{
	return m_flash->read(space, offset + 0x2000);
}

WRITE8_MEMBER(cmmb_state::flash_w)
{
	m_flash->write(space, offset + 0x2000, data);
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
		case 0x00:  // IRQ ack?  may also be 0x09.
			m_maincpu->set_input_line(0, CLEAR_LINE);
			//printf("IRQ ack\n");
			break;
		case 0x01:
			m_irq_mask = data;
			break;
		case 0x02:
			// bit 7 toggled - watchdog or status LED
			// toggled by code at E3DB in IRQ handler - it's on when the frame count & 0x30 is 1 and off otherwise
			// bit 6 set means accessing flash ROM at 0x2000
			m_bnk2000->set_bank((data & 0x40) ? 1 : 0);
			break;

		case 0x03:
			{
				uint8_t *ROM = memregion("maincpu")->base();
				uint32_t bankaddress;

				//bankaddress = 0x10000 + (0x4000 * ((data & 0x0f)^0xf));
				//printf("bank %02x => %x\n", data, bankaddress);
				bankaddress = 0x10000 + 0x3a000;
				membank("bank1")->set_base(&ROM[bankaddress]);
				// bit 7 sub-devCB's flash at 0x2000-0x4000?
			}
			break;

		case 0x07:
			break;

		case 0x09:
			break;
	}
}

/* overlap empty addresses */
void cmmb_state::cmmb_map(address_map &map)
{
	map(0x0000, 0x0fff).ram(); /* zero page address */
//  AM_RANGE(0x13c0, 0x13ff) AM_RAM //spriteram
	map(0x1000, 0x1fff).ram().share("videoram");
	map(0x2000, 0x9fff).m(m_bnk2000, FUNC(address_map_bank_device::amap8));
	map(0xa000, 0xafff).ram();
	map(0xb000, 0xbfff).rw(FUNC(cmmb_state::cmmb_charram_r), FUNC(cmmb_state::cmmb_charram_w));
	map(0xc000, 0xc00f).rw(FUNC(cmmb_state::cmmb_input_r), FUNC(cmmb_state::cmmb_output_w));
	map(0xc010, 0xffff).rom().region("maincpu", 0x1c010);
}

void cmmb_state::bnk2000_map(address_map &map)
{
	map(0x0000, 0x5fff).bankr("bank1");
	map(0x0000, 0x0000).portr("IN3");
	map(0x0001, 0x0001).portr("IN4");
	map(0x0011, 0x0011).portr("IN5");
	map(0x0480, 0x049f).ram().w(m_palette, FUNC(palette_device::write8)).share("palette");
//  map(0x0505, 0x0505).w(FUNC(cmmb_state::irq_enable_w));
//  map(0x0600, 0x0600).w(FUNC(cmmb_state::irq_ack_w));
	map(0x0680, 0x0680).nopw();
	map(0x6000, 0x7fff).rom().region("maincpu", 0x18000);

	map(0x8000, 0xffff).rw(FUNC(cmmb_state::flash_r), FUNC(cmmb_state::flash_w));
}

static INPUT_PORTS_START( cmmb )
	PORT_START("IN3")
	#if 1
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_SERVICE2 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_PLAYER(1)
	PORT_BIT( 0xcf, IP_ACTIVE_LOW, IPT_UNKNOWN )
	#else
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
	#endif

	PORT_START("IN4")
	#if 1
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_PLAYER(1)
	PORT_BIT( 0xdf, IP_ACTIVE_LOW, IPT_UNKNOWN)
	#else
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
	#endif
	// TODO: pin-point writes for trackball
	// TODO: bit 7 of 0x2507 selects trackball 1 and 2
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
	PORT_DIPNAME( 0x40, 0x40, "Magic" ) // makes part of the IRQ handler skip if ON
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
	PORT_BIT( 0xc0, IP_ACTIVE_LOW, IPT_SERVICE2 )   // if this is lit up, coins will auto-insert each frame until they hit 99 (?!)
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


static GFXDECODE_START( gfx_cmmb )
	GFXDECODE_ENTRY( "gfx", 0, charlayout,     0x00, 4 )
	GFXDECODE_ENTRY( "gfx", 0, spritelayout,   0x10, 4 )
GFXDECODE_END

INTERRUPT_GEN_MEMBER(cmmb_state::vblank_irq)
{
	if (m_irq_mask & 0x08)
	{
		m_maincpu->set_input_line(0, ASSERT_LINE);
	}
}

void cmmb_state::machine_reset()
{
	m_irq_mask = 0;
}


MACHINE_CONFIG_START(cmmb_state::cmmb)

	/* basic machine hardware */
	MCFG_DEVICE_ADD("maincpu", M65SC02, MAIN_CLOCK/5) // Unknown clock, but chip rated for 14MHz
	MCFG_DEVICE_PROGRAM_MAP(cmmb_map)
	MCFG_DEVICE_VBLANK_INT_DRIVER("screen", cmmb_state, vblank_irq)

	AT29C020(config, "at29c020");

	ADDRESS_MAP_BANK(config, "bnk2000").set_map(&cmmb_state::bnk2000_map).set_options(ENDIANNESS_LITTLE, 8, 32, 0x8000);

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(MAIN_CLOCK/12, 384, 0, 256, 264, 0, 240) // TBD, not real measurements
	MCFG_SCREEN_UPDATE_DRIVER(cmmb_state, screen_update_cmmb)
	MCFG_SCREEN_PALETTE(m_palette)

	GFXDECODE(config, m_gfxdecode, m_palette, gfx_cmmb);

	PALETTE(config, m_palette).set_format(palette_device::RGB_332_inverted, 512);

	/* sound hardware */
//  SPEAKER(config, "mono").front_center();
//  AY8910(config, "aysnd", 8000000/4).add_route(ALL_OUTPUTS, "mono", 0.30);
MACHINE_CONFIG_END


/***************************************************************************

  Game driver(s)

***************************************************************************/

ROM_START( cmmb162 )
	ROM_REGION( 0x50000, "maincpu", 0 )
	ROM_LOAD( "cmmb162.u2",   0x10000, 0x40000, CRC(71a5a75d) SHA1(0ad7b97580082cda98cb1e8aab8efcf491d0ed25) )
	ROM_COPY( "maincpu",      0x18000, 0x08000, 0x08000 )
	ROM_FILL( 0x1c124, 2, 0xea ) // temporary patch to avoid waiting on IRQs

	ROM_REGION( 0x1000, "gfx", ROMREGION_ERASE00 )
ROM_END

GAME( 2002, cmmb162, 0, cmmb, cmmb, cmmb_state, empty_init, ROT270, "Cosmodog / Team Play (Licensed from Infogrames via Midway Games West)", "Centipede / Millipede / Missile Command / Let's Go Bowling (rev 1.62)", MACHINE_NO_SOUND|MACHINE_NOT_WORKING )
