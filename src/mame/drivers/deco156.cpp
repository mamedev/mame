// license:BSD-3-Clause
// copyright-holders:Bryan McPhail
/*
    (Some) Data East 32 bit 156 CPU ARM based games:

    Heavy Smash
    World Cup Volleyball 95

    See also deco32.c, deco_mlc.c, backfire.c

    Todo:
        complete co-processor emulation for wcvol95

    Emulation by Bryan McPhail, mish@tendril.co.uk
*/

#define DE156CPU ARM

#include "emu.h"
#include "cpu/arm/arm.h"
#include "includes/decocrpt.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "video/deco16ic.h"
#include "video/decospr.h"

class deco156_state : public driver_device
{
public:
	deco156_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
			m_maincpu(*this, "maincpu"),
			m_deco_tilegen1(*this, "tilegen1"),
			m_oki1(*this, "oki1"),
			m_oki2(*this, "oki2"),
			m_sprgen(*this, "spritegen"),
			m_palette(*this, "palette"),
			m_generic_paletteram_32(*this, "paletteram")
	{ }

	/* devices */
	required_device<arm_cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	optional_device<okim6295_device> m_oki1;
	optional_device<okim6295_device> m_oki2;
	optional_device<decospr_device> m_sprgen;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;

	/* memory */
	UINT16   m_pf1_rowscroll[0x800/2];
	UINT16   m_pf2_rowscroll[0x800/2];
	std::unique_ptr<UINT16[]> m_spriteram;
	DECLARE_WRITE32_MEMBER(hvysmsh_eeprom_w);
	DECLARE_WRITE32_MEMBER(wcvol95_nonbuffered_palette_w);
	DECLARE_WRITE32_MEMBER(deco156_nonbuffered_palette_w);
	DECLARE_READ32_MEMBER(wcvol95_pf1_rowscroll_r);
	DECLARE_READ32_MEMBER(wcvol95_pf2_rowscroll_r);
	DECLARE_READ32_MEMBER(wcvol95_spriteram_r);
	DECLARE_WRITE32_MEMBER(wcvol95_pf1_rowscroll_w);
	DECLARE_WRITE32_MEMBER(wcvol95_pf2_rowscroll_w);
	DECLARE_WRITE32_MEMBER(wcvol95_spriteram_w);
	DECLARE_WRITE32_MEMBER(hvysmsh_oki_0_bank_w);
	DECLARE_DRIVER_INIT(hvysmsh);
	DECLARE_DRIVER_INIT(wcvol95);
	virtual void video_start() override;
	UINT32 screen_update_wcvol95(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(deco32_vbl_interrupt);
	void descramble_sound( const char *tag );
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);
};


void deco156_state::video_start()
{
	m_spriteram = std::make_unique<UINT16[]>(0x2000/2);

	/* and register the allocated ram so that save states still work */
	save_item(NAME(m_pf1_rowscroll));
	save_item(NAME(m_pf2_rowscroll));
	save_pointer(NAME(m_spriteram.get()), 0x2000/2);
}


UINT32 deco156_state::screen_update_wcvol95(screen_device &screen, bitmap_rgb32 &bitmap, const rectangle &cliprect)
{
	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(1);

	screen.priority().fill(0);
	bitmap.fill(0);

	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);

	m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram.get(), 0x800);
	m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 0);
	return 0;
}

/***************************************************************************/

WRITE32_MEMBER(deco156_state::hvysmsh_eeprom_w)
{
	if (ACCESSING_BITS_0_7)
	{
		m_oki2->set_bank_base(0x40000 * (data & 0x7));
		ioport("EEPROMOUT")->write(data, 0xff);
	}
}

WRITE32_MEMBER(deco156_state::hvysmsh_oki_0_bank_w)
{
	m_oki1->set_bank_base((data & 1) * 0x40000);
}

WRITE32_MEMBER(deco156_state::wcvol95_nonbuffered_palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_palette->set_pen_color(offset,pal5bit(m_generic_paletteram_32[offset] >> 0),pal5bit(m_generic_paletteram_32[offset] >> 5),pal5bit(m_generic_paletteram_32[offset] >> 10));
}

/* This is the same as deco32_nonbuffered_palette_w in video/deco32.c */
WRITE32_MEMBER(deco156_state::deco156_nonbuffered_palette_w)
{
	int r,g,b;

	COMBINE_DATA(&m_generic_paletteram_32[offset]);

	b = (m_generic_paletteram_32[offset] >>16) & 0xff;
	g = (m_generic_paletteram_32[offset] >> 8) & 0xff;
	r = (m_generic_paletteram_32[offset] >> 0) & 0xff;

	m_palette->set_pen_color(offset,rgb_t(r,g,b));
}

READ32_MEMBER(deco156_state::wcvol95_pf1_rowscroll_r){ return m_pf1_rowscroll[offset] ^ 0xffff0000; }
READ32_MEMBER(deco156_state::wcvol95_pf2_rowscroll_r){ return m_pf2_rowscroll[offset] ^ 0xffff0000; }
READ32_MEMBER(deco156_state::wcvol95_spriteram_r){ return m_spriteram[offset] ^ 0xffff0000; }
WRITE32_MEMBER(deco156_state::wcvol95_pf1_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf1_rowscroll[offset]); }
WRITE32_MEMBER(deco156_state::wcvol95_pf2_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf2_rowscroll[offset]); }
WRITE32_MEMBER(deco156_state::wcvol95_spriteram_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_spriteram[offset]); }


static ADDRESS_MAP_START( hvysmsh_map, AS_PROGRAM, 32, deco156_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x107fff) AM_RAM
	AM_RANGE(0x120000, 0x120003) AM_READ_PORT("INPUTS")
	AM_RANGE(0x120000, 0x120003) AM_WRITENOP // Volume control in low byte
	AM_RANGE(0x120004, 0x120007) AM_WRITE(hvysmsh_eeprom_w)
	AM_RANGE(0x120008, 0x12000b) AM_WRITENOP // IRQ ack?
	AM_RANGE(0x12000c, 0x12000f) AM_WRITE(hvysmsh_oki_0_bank_w)
	AM_RANGE(0x140000, 0x140003) AM_DEVREADWRITE8("oki1", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x160000, 0x160003) AM_DEVREADWRITE8("oki2", okim6295_device, read, write, 0x000000ff)
	AM_RANGE(0x180000, 0x18001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x190000, 0x191fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x194000, 0x195fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x1a0000, 0x1a0fff) AM_READWRITE(wcvol95_pf1_rowscroll_r, wcvol95_pf1_rowscroll_w)
	AM_RANGE(0x1a4000, 0x1a4fff) AM_READWRITE(wcvol95_pf2_rowscroll_r, wcvol95_pf2_rowscroll_w)
	AM_RANGE(0x1c0000, 0x1c0fff) AM_RAM_WRITE(deco156_nonbuffered_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x1d0010, 0x1d002f) AM_READNOP // Check for DMA complete?
	AM_RANGE(0x1e0000, 0x1e1fff) AM_READWRITE(wcvol95_spriteram_r, wcvol95_spriteram_w)
ADDRESS_MAP_END

static ADDRESS_MAP_START( wcvol95_map, AS_PROGRAM, 32, deco156_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x110000, 0x111fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x114000, 0x115fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x120000, 0x120fff) AM_READWRITE(wcvol95_pf1_rowscroll_r, wcvol95_pf1_rowscroll_w)
	AM_RANGE(0x124000, 0x124fff) AM_READWRITE(wcvol95_pf2_rowscroll_r, wcvol95_pf2_rowscroll_w)
	AM_RANGE(0x130000, 0x137fff) AM_RAM
	AM_RANGE(0x140000, 0x140003) AM_READ_PORT("INPUTS")
	AM_RANGE(0x150000, 0x150003) AM_WRITE_PORT("EEPROMOUT")
	AM_RANGE(0x160000, 0x161fff) AM_READWRITE(wcvol95_spriteram_r, wcvol95_spriteram_w)
	AM_RANGE(0x170000, 0x170003) AM_NOP // Irq ack?
	AM_RANGE(0x180000, 0x180fff) AM_RAM_WRITE(wcvol95_nonbuffered_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x1a0000, 0x1a0007) AM_DEVREADWRITE8("ymz", ymz280b_device, read, write, 0x000000ff)
ADDRESS_MAP_END


/***************************************************************************/

static INPUT_PORTS_START( hvysmsh )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00080000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000010, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x00000020, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00000040, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END

static INPUT_PORTS_START( wcvol95 )
	PORT_START("INPUTS")
	PORT_BIT( 0x00000001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x00000010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x00000080, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x00000100, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000200, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000400, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00000800, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x00001000, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x00002000, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x00004000, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x00008000, IP_ACTIVE_LOW, IPT_START2 )

	PORT_BIT( 0x00010000, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x00020000, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x00040000, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x00080000, IP_ACTIVE_LOW )
	PORT_BIT( 0x00100000, IP_ACTIVE_LOW, IPT_CUSTOM ) PORT_VBLANK("screen")
	PORT_BIT( 0x00200000, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x00400000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x00800000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x01000000, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, do_read)
	PORT_BIT( 0x02000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x04000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(1)
	PORT_BIT( 0x08000000, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x10000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x20000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x40000000, IP_ACTIVE_LOW, IPT_UNUSED ) //PORT_PLAYER(2)
	PORT_BIT( 0x80000000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START( "EEPROMOUT" )
	PORT_BIT( 0x00000001, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, di_write)
	PORT_BIT( 0x00000002, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, clk_write)
	PORT_BIT( 0x00000004, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER("eeprom", eeprom_serial_93cxx_device, cs_write)
INPUT_PORTS_END


/**********************************************************************************/

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
	16*8    /* every char takes 8 consecutive bytes */
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 16, 0, 24, 8 },
	{ 64*8+0, 64*8+1, 64*8+2, 64*8+3, 64*8+4, 64*8+5, 64*8+6, 64*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*32, 1*32, 2*32, 3*32, 4*32, 5*32, 6*32, 7*32,
			8*32, 9*32, 10*32, 11*32, 12*32, 13*32, 14*32, 15*32 },
	128*8
};

static const gfx_layout tilelayout =
{
	16,16,
	RGN_FRAC(1,2),
	4,
	{ RGN_FRAC(1,2)+8, RGN_FRAC(1,2), 8, 0 },
	{ 32*8+0, 32*8+1, 32*8+2, 32*8+3, 32*8+4, 32*8+5, 32*8+6, 32*8+7,
		0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
			8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
	64*8
};

static GFXDECODE_START( hvysmsh )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,          0, 32 )    /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,          0, 32 )    /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, spritelayout,      512, 32 )    /* Sprites 16x16 */
GFXDECODE_END


/**********************************************************************************/

INTERRUPT_GEN_MEMBER(deco156_state::deco32_vbl_interrupt)
{
	device.execute().set_input_line(ARM_IRQ_LINE, HOLD_LINE);
}

DECO16IC_BANK_CB_MEMBER(deco156_state::bank_callback)
{
	return ((bank >> 4) & 0x7) * 0x1000;
}

DECOSPR_PRIORITY_CB_MEMBER(deco156_state::pri_callback)
{
	switch (pri & 0xc000)
	{
		case 0x0000: return 0;
		case 0x4000: return 0xf0;
		case 0x8000: return 0xf0 | 0xcc;
		case 0xc000: return 0xf0 | 0xcc;
	}

	return 0;
}

static MACHINE_CONFIG_START( hvysmsh, deco156_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM, 28000000) /* Unconfirmed */
	MCFG_CPU_PROGRAM_MAP(hvysmsh_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", deco156_state,  deco32_vbl_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(deco156_state, screen_update_wcvol95)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hvysmsh)
	MCFG_PALETTE_ADD("palette", 1024)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(deco156_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(deco156_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_PRIORITY_CB(deco156_state, pri_callback)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_OKIM6295_ADD("oki1", 28000000/28, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 1.0)

	MCFG_OKIM6295_ADD("oki2", 28000000/14, OKIM6295_PIN7_HIGH)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "lspeaker", 0.35)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "rspeaker", 0.35)
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( wcvol95, deco156_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM, 28000000) /* Unconfirmed */
	MCFG_CPU_PROGRAM_MAP(wcvol95_map)
	MCFG_CPU_VBLANK_INT_DRIVER("screen", deco156_state,  deco32_vbl_interrupt)

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(529))
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(deco156_state, screen_update_wcvol95)

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", hvysmsh)
	MCFG_PALETTE_ADD("palette", 1024)

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x10)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(deco156_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(deco156_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")
	MCFG_DECO16IC_PALETTE("palette")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_DECO_SPRITE_GFX_REGION(2)
	MCFG_DECO_SPRITE_PRIORITY_CB(deco156_state, pri_callback)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")
	MCFG_DECO_SPRITE_PALETTE("palette")

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, 28000000 / 2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/**********************************************************************************/

/*

Heavy Smash
Data East, 1993

PCB Layout

DE-0385-2  DEC-22VO
|----------------------------------------------|
|                      28MHz  DE52             |
|           MBG-04.13J               MBG-02.11A|
|  M6295(1) MBG-03.10K  VL-02        MBG-01.10A|
|  M6295(2)                   DE153  MBG-00.9A |
|                                              |
|                                              |
|J                                             |
|A               93C46.8K                      |
|M                                             |
|M                            DE141            |
|A                                VL-01 VL-00  |
|                  6264                        |
|      DE153       6264                        |
|                                              |
|                                              |
|TEST_SW                                 DE156 |
|           LP01-2.3J  6264   6264             |
|           LP00-2.2J  6264   6264             |
|                                              |
|----------------------------------------------|

Notes:
      - CPU DE156 is a custom encrypted ARM7-based chip. The clock input is 7.000MHz on pin 90
        The package is a Quad Flat Pack and has 100 pins.
      - OKI M6295(1) clock: 1.000MHz (28 / 28), sample rate = 1000000 / 132
      - OKI M6295(2) clock: 2.000MHz (28 / 14), sample rate = 2000000 / 132
      - VSync: 58Hz
      - VL-00 (PAL16R8), VL-01 (PAL16L8), VL-02 (PAL16R6)
      - On the Data East boards of this type (using DE156) that use an EEPROM, the EEPROM contains the
        country/region code also. It has been proven by comparing the dumps of Osman and Cannon Dancer....
        they were identical and there are no region jumper pads on the PCB. Therefore the EEPROM must
        hold the region code.

      ROMs
      ----
      - MBG-00, MBG-01, MBG-02  - 16M MASK  Graphics
      - LP00, LP01              - 27C4096   Main program
      - MBG-03                  - 4M MASK   Sound (samples, linked to M6295(1)
      - MBG-04                  - 16M MASK  Sound (samples, linked to M6295(2)
      - 93C46                   - 128 bytes EEPROM (Note! this chip has identical halves and fixed
                                                    bits, but the dump is correct!)

*/

ROM_START( hvysmsh ) /* Europe -2  1993/06/30 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "lt00-2.2j", 0x000002, 0x080000, CRC(f6e10fc0) SHA1(76189260ca0a79500d62c4aa8e3aed6cfca3e102) )
	ROM_LOAD32_WORD( "lt01-2.3j", 0x000000, 0x080000, CRC(ce2a75e2) SHA1(4119a3175d7c394041197f01523a6eaa3d9ba398) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbg-00.9a",  0x000000, 0x080000, CRC(7d94eb16) SHA1(31cf5302eba37e935865822aebd76c700bc51eaf) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbg-01.10a", 0x000000, 0x200000, CRC(bcd7fb29) SHA1(a54a813b5adcb4df0bfdd58285b1f8e17fbbb7a2) )
	ROM_LOAD16_BYTE( "mbg-02.11a", 0x000001, 0x200000, CRC(0cc16440) SHA1(1cbf620a9d875ec87dd28a97a256584b6ef277cd) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-03.10k", 0x00000, 0x80000,  CRC(4b809420) SHA1(ad0278745002320804a31af0b772f9ab5f075027) )

	ROM_REGION( 0x200000, "oki2", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-04.11k", 0x00000, 0x200000, CRC(2281c758) SHA1(934691b4002ecd6bc9a09b8970ff18a09451d492) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.8k", 0x00, 0x80, CRC(d31fbd5b) SHA1(bf044408c637f6b39afd30ccb86af183ec0acc02) )
ROM_END

ROM_START( hvysmshj ) /* Japan -2  1993/06/30 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "lp00-2.2j", 0x000002, 0x080000, CRC(3f8fd724) SHA1(8efb27b96dbdc58715eb44c7846f30d485e1ded4) )
	ROM_LOAD32_WORD( "lp01-2.3j", 0x000000, 0x080000, CRC(a6fe282a) SHA1(10295b740ced35b3bb1f48ca3af2e985912405ec) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbg-00.9a",  0x000000, 0x080000, CRC(7d94eb16) SHA1(31cf5302eba37e935865822aebd76c700bc51eaf) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbg-01.10a", 0x000000, 0x200000, CRC(bcd7fb29) SHA1(a54a813b5adcb4df0bfdd58285b1f8e17fbbb7a2) )
	ROM_LOAD16_BYTE( "mbg-02.11a", 0x000001, 0x200000, CRC(0cc16440) SHA1(1cbf620a9d875ec87dd28a97a256584b6ef277cd) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-03.10k", 0x00000, 0x80000,  CRC(4b809420) SHA1(ad0278745002320804a31af0b772f9ab5f075027) )

	ROM_REGION( 0x200000, "oki2", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-04.11k", 0x00000, 0x200000, CRC(2281c758) SHA1(934691b4002ecd6bc9a09b8970ff18a09451d492) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.8k", 0x00, 0x80, CRC(d31fbd5b) SHA1(bf044408c637f6b39afd30ccb86af183ec0acc02) )
ROM_END

ROM_START( hvysmsha ) /* Asia -4  1993/09/06 */
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "xx00-4.2j", 0x000002, 0x080000, CRC(333a92c1) SHA1(b7e174ea081febb765298aa1c6533b2f9f162bce) ) /* "xx" is NOT the correct region code, this needs */
	ROM_LOAD32_WORD( "xx01-4.3j", 0x000000, 0x080000, CRC(8c24c5ed) SHA1(ab9689530f4f4a6015ce0a6f8e0d796b0618cd79) ) /* to be verified and corrected at some point */

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "mbg-00.9a",  0x000000, 0x080000, CRC(7d94eb16) SHA1(31cf5302eba37e935865822aebd76c700bc51eaf) )
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x080000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)

	ROM_REGION( 0x800000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbg-01.10a", 0x000000, 0x200000, CRC(bcd7fb29) SHA1(a54a813b5adcb4df0bfdd58285b1f8e17fbbb7a2) )
	ROM_LOAD16_BYTE( "mbg-02.11a", 0x000001, 0x200000, CRC(0cc16440) SHA1(1cbf620a9d875ec87dd28a97a256584b6ef277cd) )

	ROM_REGION( 0x80000, "oki1", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-03.10k", 0x00000, 0x80000,  CRC(4b809420) SHA1(ad0278745002320804a31af0b772f9ab5f075027) )

	ROM_REGION( 0x200000, "oki2", 0 ) /* Oki samples */
	ROM_LOAD( "mbg-04.11k", 0x00000, 0x200000, CRC(2281c758) SHA1(934691b4002ecd6bc9a09b8970ff18a09451d492) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.8k", 0x00, 0x80, CRC(d31fbd5b) SHA1(bf044408c637f6b39afd30ccb86af183ec0acc02) )
ROM_END

/*
World Cup Volley '95
Data East, 1995

PCB Layout

DE-0430-2
|----------------------------------------------|
|          MBX-03.13J                MBX-02.13A|
|       LC7881         28MHz   DE52            |
|             YMZ280B-F              MBX-01.12A|
|                         CY7C185              |
|                         CY7C185    MBX-00.9A |
|             WE-02                            |
|J                                             |
|A                                             |
|M                 6264                        |
|M                             DE141           |
|A     DE223       6264                        |
|                                              |
|                                       WE-00  |
|                              WE-01           |
|                                              |
|TEST_SW           PN01-0.4F   6264            |
|         93C46.3K             6264            |
|                  PN00-0.2F   6264     DE156  |
|                              6264            |
|----------------------------------------------|

Notes:
      - CPU DE156 is a custom encrypted ARM7-based chip. The clock input is 7.000MHz on pin 90
        The package is a Quad Flat Pack and has 100 pins.
      - YMZ280B-F clock: 14.000MHz (28 / 2)
        SANYO LC7881 clock: 2.3333MHz (28 / 12)
      - VSync: 58Hz
      - WE-00, WE-01 and WE-02 are PALs type GAL16V8

*/

ROM_START( wcvol95 )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "pn00-0.2f",    0x000002, 0x080000, CRC(c9ed2006) SHA1(cee93eafc42c4de7a1453c85e7d6bca8d62cdc7b) )
	ROM_LOAD32_WORD( "pn01-0.4f",    0x000000, 0x080000, CRC(1c3641c3) SHA1(60dddc3585e4dedb485f7505fee03495f615c0c0) )

	ROM_REGION( 0x080000, "gfx1", 0 )
	ROM_LOAD( "mbx-00.9a",    0x000000, 0x080000, CRC(a0b24204) SHA1(cec8089c6c635f23b3a4aeeef2c43f519568ad70) )

	ROM_REGION( 0x200000, "gfx2", 0 )
	ROM_LOAD16_BYTE( "mbx-01.12a",    0x000000, 0x100000, CRC(73deb3f1) SHA1(c0cabecfd88695afe0f27c5bb115b4973907207d) )
	ROM_LOAD16_BYTE( "mbx-02.13a",    0x000001, 0x100000, CRC(3204d324) SHA1(44102f71bae44bf3a9bd2de7e5791d959a2c9bdd) )

	ROM_REGION( 0x200000, "ymz", 0 ) /* YMZ280B-F samples */
	ROM_LOAD( "mbx-03.13j",    0x00000, 0x200000,  CRC(061632bc) SHA1(7900ac56e59f4a4e5768ce72f4a4b7c5875f5ae8) )

//  ROM_REGION( 0x80, "user1", 0 ) /* eeprom */
//  ROM_LOAD( "93c46.3k",    0x00, 0x80, CRC(88f8e270) SHA1(cb82203ad38e0c12ea998562b7b785979726afe5) )
ROM_END

/**********************************************************************************/

void deco156_state::descramble_sound( const char *tag )
{
	UINT8 *rom = memregion(tag)->base();
	int length = memregion(tag)->bytes();
	dynamic_buffer buf1(length);
	UINT32 x;

	for (x = 0; x < length; x++)
	{
		UINT32 addr;

		addr = BITSWAP24 (x,23,22,21,0, 20,
							19,18,17,16,
							15,14,13,12,
							11,10,9, 8,
							7, 6, 5, 4,
							3, 2, 1 );

		buf1[addr] = rom[x];
	}

	memcpy(rom,&buf1[0],length);
}

DRIVER_INIT_MEMBER(deco156_state,hvysmsh)
{
	deco56_decrypt_gfx(machine(), "gfx1"); /* 141 */
	deco156_decrypt(machine());
	descramble_sound("oki2");
}

DRIVER_INIT_MEMBER(deco156_state,wcvol95)
{
	deco56_decrypt_gfx(machine(), "gfx1"); /* 141 */
	deco156_decrypt(machine());
	descramble_sound("ymz");
}


/**********************************************************************************/

GAME( 1993, hvysmsh,  0,       hvysmsh, hvysmsh, deco156_state, hvysmsh,  ROT0, "Data East Corporation", "Heavy Smash (Europe version -2)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, hvysmsha, hvysmsh, hvysmsh, hvysmsh, deco156_state, hvysmsh,  ROT0, "Data East Corporation", "Heavy Smash (Asia version -4)", MACHINE_SUPPORTS_SAVE )
GAME( 1993, hvysmshj, hvysmsh, hvysmsh, hvysmsh, deco156_state, hvysmsh,  ROT0, "Data East Corporation", "Heavy Smash (Japan version -2)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, wcvol95,  0,       wcvol95, wcvol95, deco156_state, wcvol95,  ROT0, "Data East Corporation", "World Cup Volley '95 (Japan v1.0)", MACHINE_SUPPORTS_SAVE )
