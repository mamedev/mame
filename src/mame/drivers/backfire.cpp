// license:BSD-3-Clause
// copyright-holders:David Haywood
/* Data East Backfire!

    Backfire!

    inputs are incomplete (p2 side.., alt control modes etc.)

    there may still be some problems with the 156 co-processor, but it seems to be mostly correct

    set 2 defaults to wheel controls, so until they're mapped you must change back to joystick in test mode

*/

#define DE156CPU ARM
#include "emu.h"
#include "includes/decocrpt.h"
#include "machine/eepromser.h"
#include "sound/okim6295.h"
#include "sound/ymz280b.h"
#include "cpu/arm/arm.h"
#include "video/deco16ic.h"
#include "rendlay.h"
#include "video/decospr.h"


class backfire_state : public driver_device
{
public:
	backfire_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_mainram(*this, "mainram"),
		m_left_priority(*this, "left_priority"),
		m_right_priority(*this, "right_priority"),
		m_sprgen(*this, "spritegen"),
		m_sprgen2(*this, "spritegen2"),
		m_maincpu(*this, "maincpu"),
		m_deco_tilegen1(*this, "tilegen1"),
		m_deco_tilegen2(*this, "tilegen2"),
		m_eeprom(*this, "eeprom"),
		m_io_in0(*this, "IN0"),
		m_io_in1(*this, "IN1"),
		m_io_in2(*this, "IN2"),
		m_io_in3(*this, "IN3"),
		m_palette(*this, "palette"),
		m_generic_paletteram_32(*this, "paletteram")
	{ }

	/* memory pointers */
	std::unique_ptr<UINT16[]>  m_spriteram_1;
	std::unique_ptr<UINT16[]>  m_spriteram_2;
	required_shared_ptr<UINT32> m_mainram;
	required_shared_ptr<UINT32> m_left_priority;
	required_shared_ptr<UINT32> m_right_priority;
	optional_device<decospr_device> m_sprgen;
	optional_device<decospr_device> m_sprgen2;

	/* video related */
	std::unique_ptr<bitmap_ind16>  m_left;
	std::unique_ptr<bitmap_ind16>  m_right;

	/* devices */
	required_device<cpu_device> m_maincpu;
	required_device<deco16ic_device> m_deco_tilegen1;
	required_device<deco16ic_device> m_deco_tilegen2;

	required_device<eeprom_serial_93cxx_device> m_eeprom;

	/* memory */
	UINT16    m_pf1_rowscroll[0x0800/2];
	UINT16    m_pf2_rowscroll[0x0800/2];
	UINT16    m_pf3_rowscroll[0x0800/2];
	UINT16    m_pf4_rowscroll[0x0800/2];
	DECLARE_READ32_MEMBER(backfire_control2_r);
	DECLARE_WRITE32_MEMBER(backfire_nonbuffered_palette_w);
	DECLARE_READ32_MEMBER(backfire_pf1_rowscroll_r);
	DECLARE_READ32_MEMBER(backfire_pf2_rowscroll_r);
	DECLARE_READ32_MEMBER(backfire_pf3_rowscroll_r);
	DECLARE_READ32_MEMBER(backfire_pf4_rowscroll_r);
	DECLARE_WRITE32_MEMBER(backfire_pf1_rowscroll_w);
	DECLARE_WRITE32_MEMBER(backfire_pf2_rowscroll_w);
	DECLARE_WRITE32_MEMBER(backfire_pf3_rowscroll_w);
	DECLARE_WRITE32_MEMBER(backfire_pf4_rowscroll_w);
	DECLARE_READ32_MEMBER(backfire_spriteram1_r);
	DECLARE_WRITE32_MEMBER(backfire_spriteram1_w);
	DECLARE_READ32_MEMBER(backfire_spriteram2_r);
	DECLARE_WRITE32_MEMBER(backfire_spriteram2_w);
	DECLARE_READ32_MEMBER(backfire_speedup_r);
	DECLARE_READ32_MEMBER(backfire_eeprom_r);
	DECLARE_WRITE32_MEMBER(backfire_eeprom_w);
	DECLARE_DRIVER_INIT(backfire);
	virtual void machine_start() override;
	virtual void video_start() override;
	UINT32 screen_update_backfire_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	UINT32 screen_update_backfire_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	INTERRUPT_GEN_MEMBER(deco32_vbl_interrupt);
	void descramble_sound();
	DECO16IC_BANK_CB_MEMBER(bank_callback);
	DECOSPR_PRIORITY_CB_MEMBER(pri_callback);

	required_ioport m_io_in0;
	required_ioport m_io_in1;
	required_ioport m_io_in2;
	required_ioport m_io_in3;
	required_device<palette_device> m_palette;
	required_shared_ptr<UINT32> m_generic_paletteram_32;
};

//UINT32 *backfire_180010, *backfire_188010;

/* I'm using the functions in deco16ic.c ... same chips, why duplicate code? */
void backfire_state::video_start()
{
	m_spriteram_1 = std::make_unique<UINT16[]>(0x2000/2);
	m_spriteram_2 = std::make_unique<UINT16[]>(0x2000/2);

	/* and register the allocated ram so that save states still work */
	save_item(NAME(m_pf1_rowscroll));
	save_item(NAME(m_pf2_rowscroll));
	save_item(NAME(m_pf3_rowscroll));
	save_item(NAME(m_pf4_rowscroll));

	m_left =  std::make_unique<bitmap_ind16>(80*8, 32*8);
	m_right = std::make_unique<bitmap_ind16>(80*8, 32*8);

	save_pointer(NAME(m_spriteram_1.get()), 0x2000/2);
	save_pointer(NAME(m_spriteram_2.get()), 0x2000/2);

	save_item(NAME(*m_left));
	save_item(NAME(*m_right));
}



UINT32 backfire_state::screen_update_backfire_left(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(1);

	/* screen 1 uses pf1 as the forground and pf3 as the background */
	/* screen 2 uses pf2 as the foreground and pf4 as the background */
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	screen.priority().fill(0);
	bitmap.fill(0x100, cliprect);

	if (m_left_priority[0] == 0)
	{
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram_1.get(), 0x800);
	}
	else if (m_left_priority[0] == 2)
	{
		m_deco_tilegen1->tilemap_1_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen2->tilemap_1_draw(screen, bitmap, cliprect, 0, 4);
		m_sprgen->draw_sprites(bitmap, cliprect, m_spriteram_1.get(), 0x800);
	}
	else
		popmessage( "unknown left priority %08x", m_left_priority[0]);

	return 0;
}

UINT32 backfire_state::screen_update_backfire_right(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	//FIXME: flip_screen_x should not be written!
	flip_screen_set_no_update(1);

	/* screen 1 uses pf1 as the forground and pf3 as the background */
	/* screen 2 uses pf2 as the foreground and pf4 as the background */
	m_deco_tilegen1->pf_update(m_pf1_rowscroll, m_pf2_rowscroll);
	m_deco_tilegen2->pf_update(m_pf3_rowscroll, m_pf4_rowscroll);

	screen.priority().fill(0);
	bitmap.fill(0x500, cliprect);

	if (m_right_priority[0] == 0)
	{
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 1);
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram_2.get(), 0x800);
	}
	else if (m_right_priority[0] == 2)
	{
		m_deco_tilegen1->tilemap_2_draw(screen, bitmap, cliprect, 0, 2);
		m_deco_tilegen2->tilemap_2_draw(screen, bitmap, cliprect, 0, 4);
		m_sprgen2->draw_sprites(bitmap, cliprect, m_spriteram_2.get(), 0x800);
	}
	else
		popmessage( "unknown right priority %08x", m_right_priority[0]);

	return 0;
}



READ32_MEMBER(backfire_state::backfire_eeprom_r)
{
	/* some kind of screen indicator?  checked by backfirea set before it will boot */
	int backfire_screen = machine().rand() & 1;
	return ((m_eeprom->do_read() << 24) | m_io_in0->read()
			| ((m_io_in2->read() & 0xbf) << 16)
			| ((m_io_in3->read() & 0x40) << 16)) ^ (backfire_screen << 26) ;
}

READ32_MEMBER(backfire_state::backfire_control2_r)
{
//  logerror("%08x:Read eprom %08x (%08x)\n", space.device().safe_pc(), offset << 1, mem_mask);
	return (m_eeprom->do_read() << 24) | m_io_in1->read() | (m_io_in1->read() << 16);
}

#ifdef UNUSED_FUNCTION
READ32_MEMBER(backfire_state::backfire_control3_r)
{
//  logerror("%08x:Read eprom %08x (%08x)\n", space.device().safe_pc(), offset << 1, mem_mask);
	return (m_eeprom->do_read() << 24) | m_io_in2->read() | (m_io_in2->read() << 16);
}
#endif


WRITE32_MEMBER(backfire_state::backfire_eeprom_w)
{
	logerror("%s:write eprom %08x (%08x) %08x\n",machine().describe_context(),offset<<1,mem_mask,data);
	if (ACCESSING_BITS_0_7)
	{
		m_eeprom->clk_write(BIT(data, 1) ? ASSERT_LINE : CLEAR_LINE);
		m_eeprom->di_write(BIT(data, 0));
		m_eeprom->cs_write(BIT(data, 2) ? ASSERT_LINE : CLEAR_LINE);
	}
}


WRITE32_MEMBER(backfire_state::backfire_nonbuffered_palette_w)
{
	COMBINE_DATA(&m_generic_paletteram_32[offset]);
	m_palette->set_pen_color(offset,pal5bit(m_generic_paletteram_32[offset] >> 0),pal5bit(m_generic_paletteram_32[offset] >> 5),pal5bit(m_generic_paletteram_32[offset] >> 10));
}

/* map 32-bit writes to 16-bit */

READ32_MEMBER(backfire_state::backfire_pf1_rowscroll_r){ return m_pf1_rowscroll[offset] ^ 0xffff0000; }
READ32_MEMBER(backfire_state::backfire_pf2_rowscroll_r){ return m_pf2_rowscroll[offset] ^ 0xffff0000; }
READ32_MEMBER(backfire_state::backfire_pf3_rowscroll_r){ return m_pf3_rowscroll[offset] ^ 0xffff0000; }
READ32_MEMBER(backfire_state::backfire_pf4_rowscroll_r){ return m_pf4_rowscroll[offset] ^ 0xffff0000; }
WRITE32_MEMBER(backfire_state::backfire_pf1_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf1_rowscroll[offset]); }
WRITE32_MEMBER(backfire_state::backfire_pf2_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf2_rowscroll[offset]); }
WRITE32_MEMBER(backfire_state::backfire_pf3_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf3_rowscroll[offset]); }
WRITE32_MEMBER(backfire_state::backfire_pf4_rowscroll_w){ data &= 0x0000ffff; mem_mask &= 0x0000ffff; COMBINE_DATA(&m_pf4_rowscroll[offset]); }


#ifdef UNUSED_FUNCTION
READ32_MEMBER(backfire_state::backfire_unknown_wheel_r)
{
	return ioport("PADDLE0")->read();
}

READ32_MEMBER(backfire_state::backfire_wheel1_r)
{
	return machine().rand();
}

READ32_MEMBER(backfire_state::backfire_wheel2_r)
{
	return machine().rand();
}
#endif


READ32_MEMBER(backfire_state::backfire_spriteram1_r)
{
	return m_spriteram_1[offset] ^ 0xffff0000;
}

WRITE32_MEMBER(backfire_state::backfire_spriteram1_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_spriteram_1[offset]);
}

READ32_MEMBER(backfire_state::backfire_spriteram2_r)
{
	return m_spriteram_2[offset] ^ 0xffff0000;
}

WRITE32_MEMBER(backfire_state::backfire_spriteram2_w)
{
	data &= 0x0000ffff;
	mem_mask &= 0x0000ffff;

	COMBINE_DATA(&m_spriteram_2[offset]);
}



static ADDRESS_MAP_START( backfire_map, AS_PROGRAM, 32, backfire_state )
	AM_RANGE(0x000000, 0x0fffff) AM_ROM
	AM_RANGE(0x100000, 0x10001f) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x110000, 0x111fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x114000, 0x115fff) AM_DEVREADWRITE("tilegen1", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x120000, 0x120fff) AM_READWRITE(backfire_pf1_rowscroll_r, backfire_pf1_rowscroll_w)
	AM_RANGE(0x124000, 0x124fff) AM_READWRITE(backfire_pf2_rowscroll_r, backfire_pf2_rowscroll_w)
	AM_RANGE(0x130000, 0x13001f) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf_control_dword_r, pf_control_dword_w)
	AM_RANGE(0x140000, 0x141fff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf1_data_dword_r, pf1_data_dword_w)
	AM_RANGE(0x144000, 0x145fff) AM_DEVREADWRITE("tilegen2", deco16ic_device, pf2_data_dword_r, pf2_data_dword_w)
	AM_RANGE(0x150000, 0x150fff) AM_READWRITE(backfire_pf3_rowscroll_r, backfire_pf3_rowscroll_w)
	AM_RANGE(0x154000, 0x154fff) AM_READWRITE(backfire_pf4_rowscroll_r, backfire_pf4_rowscroll_w)
	AM_RANGE(0x160000, 0x161fff) AM_WRITE(backfire_nonbuffered_palette_w) AM_SHARE("paletteram")
	AM_RANGE(0x170000, 0x177fff) AM_RAM AM_SHARE("mainram")// main ram

//  AM_RANGE(0x180010, 0x180013) AM_RAM AM_SHARE("backfire_180010") // always 180010 ?
//  AM_RANGE(0x188010, 0x188013) AM_RAM AM_SHARE("backfire_188010") // always 188010 ?

	AM_RANGE(0x184000, 0x185fff) AM_READWRITE(backfire_spriteram1_r, backfire_spriteram1_w)
	AM_RANGE(0x18c000, 0x18dfff) AM_READWRITE(backfire_spriteram2_r, backfire_spriteram2_w)
	AM_RANGE(0x190000, 0x190003) AM_READ(backfire_eeprom_r)
	AM_RANGE(0x194000, 0x194003) AM_READ(backfire_control2_r)
	AM_RANGE(0x1a4000, 0x1a4003) AM_WRITE(backfire_eeprom_w)

	AM_RANGE(0x1a8000, 0x1a8003) AM_RAM AM_SHARE("left_priority")
	AM_RANGE(0x1ac000, 0x1ac003) AM_RAM AM_SHARE("right_priority")
//  AM_RANGE(0x1b0000, 0x1b0003) AM_WRITENOP // always 1b0000

	/* when set to pentometer in test mode */
//  AM_RANGE(0x1e4000, 0x1e4003) AM_READ(backfire_unknown_wheel_r)
//  AM_RANGE(0x1e8000, 0x1e8003) AM_READ(backfire_wheel1_r)
//  AM_RANGE(0x1e8004, 0x1e8007) AM_READ(backfire_wheel2_r)

	AM_RANGE(0x1c0000, 0x1c0007) AM_DEVREADWRITE8("ymz", ymz280b_device, read, write, 0x000000ff)
ADDRESS_MAP_END


static INPUT_PORTS_START( backfire )
	PORT_START("IN0")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0008, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(1)
	PORT_BIT( 0x0010, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(1)
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(1)
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(1)
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_START1 )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_BUTTON3 ) PORT_PLAYER(2)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_START2 )

	PORT_START("IN2")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x0002, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x0004, IP_ACTIVE_LOW, IPT_SERVICE1 )
	PORT_SERVICE_NO_TOGGLE( 0x0008, IP_ACTIVE_LOW )
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("lscreen")
	PORT_BIT( 0x0020, IP_ACTIVE_LOW, IPT_UNUSED ) /* 'soundmask' */
	PORT_BIT( 0x0040, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x0100, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(1)
	PORT_BIT( 0x0200, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(1)
	PORT_BIT( 0x0400, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(1)
	PORT_BIT( 0x0800, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x1000, IP_ACTIVE_LOW, IPT_BUTTON4 ) PORT_PLAYER(2)
	PORT_BIT( 0x2000, IP_ACTIVE_LOW, IPT_BUTTON5 ) PORT_PLAYER(2)
	PORT_BIT( 0x4000, IP_ACTIVE_LOW, IPT_BUTTON6 ) PORT_PLAYER(2)
	PORT_BIT( 0x8000, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("IN3")
	PORT_BIT( 0x003f, IP_ACTIVE_LOW, IPT_UNUSED ) /* all other bits like low IN2 */
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_CUSTOM ) PORT_VBLANK("lscreen")
	PORT_BIT( 0x0080, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("PADDLE0")
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START("PADDLE1")
	PORT_BIT ( 0x00ff, 0x0080, IPT_PADDLE ) PORT_SENSITIVITY(30) PORT_KEYDELTA(1)

	PORT_START("UNK")
	/* ?? */
INPUT_PORTS_END


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


static GFXDECODE_START( backfire )
	GFXDECODE_ENTRY( "gfx1", 0, charlayout,      0, 128 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx1", 0, tilelayout,      0, 128 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx2", 0, charlayout,      0, 128 )   /* Characters 8x8 */
	GFXDECODE_ENTRY( "gfx2", 0, tilelayout,      0, 128 )   /* Tiles 16x16 */
	GFXDECODE_ENTRY( "gfx3", 0, spritelayout,    0x200, 32 )    /* Sprites 16x16 (screen 1) */
	GFXDECODE_ENTRY( "gfx4", 0, spritelayout,    0x600, 32 )    /* Sprites 16x16 (screen 2) */
GFXDECODE_END


INTERRUPT_GEN_MEMBER(backfire_state::deco32_vbl_interrupt)
{
	device.execute().set_input_line(ARM_IRQ_LINE, HOLD_LINE);
}


DECO16IC_BANK_CB_MEMBER(backfire_state::bank_callback)
{
	//  osd_printf_debug("bank callback %04x\n",bank); // bit 1 gets set too?
	bank = bank >> 4;
	bank = (bank & 1) | ((bank & 4) >> 1) | ((bank & 2) << 1);

	return bank * 0x1000;
}

DECOSPR_PRIORITY_CB_MEMBER(backfire_state::pri_callback)
{
	switch (pri & 0xc000)
	{
		case 0x0000: return 0;    // numbers, people, cars when in the air, status display..
		case 0x4000: return 0xf0; // cars most of the time
		case 0x8000: return 0;    // car wheels during jump?
		case 0xc000: return 0xf0; // car wheels in race?
	}
	return 0;
}

void backfire_state::machine_start()
{
}

static MACHINE_CONFIG_START( backfire, backfire_state )

	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu", ARM, 28000000/4) /* Unconfirmed */
	MCFG_CPU_PROGRAM_MAP(backfire_map)
	MCFG_CPU_VBLANK_INT_DRIVER("lscreen", backfire_state,  deco32_vbl_interrupt)    /* or is it "rscreen?" */

	MCFG_EEPROM_SERIAL_93C46_ADD("eeprom")


	/* video hardware */
	MCFG_PALETTE_ADD("palette", 2048)
	MCFG_GFXDECODE_ADD("gfxdecode", "palette", backfire)
	MCFG_DEFAULT_LAYOUT(layout_dualhsxs)

	MCFG_SCREEN_ADD("lscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(backfire_state, screen_update_backfire_left)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_SCREEN_ADD("rscreen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500) /* not accurate */)
	MCFG_SCREEN_SIZE(40*8, 32*8)
	MCFG_SCREEN_VISIBLE_AREA(0*8, 40*8-1, 1*8, 31*8-1)
	MCFG_SCREEN_UPDATE_DRIVER(backfire_state, screen_update_backfire_right)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_DEVICE_ADD("tilegen1", DECO16IC, 0)
	MCFG_DECO16IC_SET_SCREEN("lscreen")
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x00)
	MCFG_DECO16IC_PF2_COL_BANK(0x40)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(backfire_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(backfire_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(0)
	MCFG_DECO16IC_PF12_16X16_BANK(1)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("tilegen2", DECO16IC, 0)
	MCFG_DECO16IC_SET_SCREEN("lscreen")
	MCFG_DECO16IC_SPLIT(0)
	MCFG_DECO16IC_WIDTH12(1)
	MCFG_DECO16IC_PF1_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF2_TRANS_MASK(0x0f)
	MCFG_DECO16IC_PF1_COL_BANK(0x10)
	MCFG_DECO16IC_PF2_COL_BANK(0x50)
	MCFG_DECO16IC_PF1_COL_MASK(0x0f)
	MCFG_DECO16IC_PF2_COL_MASK(0x0f)
	MCFG_DECO16IC_BANK1_CB(backfire_state, bank_callback)
	MCFG_DECO16IC_BANK2_CB(backfire_state, bank_callback)
	MCFG_DECO16IC_PF12_8X8_BANK(2)
	MCFG_DECO16IC_PF12_16X16_BANK(3)
	MCFG_DECO16IC_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen", DECO_SPRITE, 0)
	MCFG_VIDEO_SET_SCREEN("lscreen")
	MCFG_DECO_SPRITE_GFX_REGION(4)
	MCFG_DECO_SPRITE_PRIORITY_CB(backfire_state, pri_callback)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")

	MCFG_DEVICE_ADD("spritegen2", DECO_SPRITE, 0)
	MCFG_VIDEO_SET_SCREEN("rscreen")
	MCFG_DECO_SPRITE_GFX_REGION(5)
	MCFG_DECO_SPRITE_PRIORITY_CB(backfire_state, pri_callback)
	MCFG_DECO_SPRITE_GFXDECODE("gfxdecode")


	/* sound hardware */
	MCFG_SPEAKER_STANDARD_STEREO("lspeaker", "rspeaker")

	MCFG_SOUND_ADD("ymz", YMZ280B, 28000000 / 2)
	MCFG_SOUND_ROUTE(0, "lspeaker", 1.0)
	MCFG_SOUND_ROUTE(1, "rspeaker", 1.0)
MACHINE_CONFIG_END


/*

Backfire!
Data East, 1995

This game is similar to World Rally, Blomby Car, Drift Out'94 etc


PCB Layout
----------


DE-0432-2
---------------------------------------------------------------------
|              MBZ-06.19L     28.000MHz                MBZ-04.19A * |
|                                           52                      |
|                              153                     MBZ-03.18A + |
|              MBZ-05.17L                                           |
|                                                                   |
--|        LC7881  YMZ280B-F   153          52         MBZ-04.16A * |
  |                                                                 |
--|                                                    MBZ-03.15A + |
|                     CY7C185 (x2)                                  |
|J                                     141                          |
|                                                      MBZ-02.12A   |
|A                                                                  |
|                                                      MBZ-01.10A   |
|M       223                                                        |
|                                                      MBZ-00.9A    |
|M         93C45.8M   CY7C185 (x2)     141                          |
|                                                                   |
|A                                                                  |
|                                                                   |
--|                                                                 |
  |                                                                 |
--|        TSW1                                                     |
|                                          CY7C185 (x4)             |
|                                                           156     |
|                 ADC0808       RA01-0.3J                           |
|                               RA00-0.2J                           |
|CONN2      CONN1    D4701                                          |
|                                                                   |
---------------------------------------------------------------------


Notes:
CONN1 & CONN2: For connection of potentiometer or opto steering wheel.
               Joystick (via JAMMA) can also be used for controls.
TSW1: Push Button TEST switch to access options menu (coins/lives etc).
*   : These ROMs have identical contents AND identical halves.
+   : These ROMs have identical contents AND identical halves.

*/

ROM_START( backfire )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "ra00-0.2j",    0x000002, 0x080000, CRC(790da069) SHA1(84fd90fb1833b97459cb337fdb92f7b6e93b5936) )
	ROM_LOAD32_WORD( "ra01-0.3j",    0x000000, 0x080000, CRC(447cb57b) SHA1(1d503b9cf1cadd3fdd7c9d6d59d4c40a59fa25ab))

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Tiles 1 */
	ROM_LOAD( "mbz-00.9a",    0x000000, 0x080000, CRC(1098d504) SHA1(1fecd26b92faffce0b59a8a9646bfd457c17c87c) )
	ROM_CONTINUE( 0x200000, 0x080000)
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x300000, 0x080000)
	ROM_LOAD( "mbz-01.10a",    0x080000, 0x080000, CRC(19b81e5c) SHA1(4c8204a6a4ad30b23fbfdd79c6e39581e23de6ae) )
	ROM_CONTINUE( 0x280000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)
	ROM_CONTINUE( 0x380000, 0x080000)

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Tiles 2 */
	ROM_LOAD( "mbz-02.12a",    0x000000, 0x100000, CRC(2bd2b0a1) SHA1(8fcb37728f3248ad55e48f2d398b014b36c9ec05) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites 1 */
	ROM_LOAD16_BYTE( "mbz-03.15a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.16a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* Sprites 2 */
	ROM_LOAD16_BYTE( "mbz-03.18a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.19a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASEFF ) /* samples */
	ROM_LOAD( "mbz-05.17l",    0x000000, 0x200000,  CRC(947c1da6) SHA1(ac36006e04dc5e3990f76539763cc76facd08376) )
	ROM_LOAD( "mbz-06.19l",    0x200000, 0x080000,  CRC(4a38c635) SHA1(7f0fb6a7a4aa6774c04fa38e53ceff8744fe1e9f) )

	ROM_REGION( 0x0600, "plds", 0 )
	ROM_LOAD( "gal16v8b.6b",  0x0000, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8b.6d",  0x0200, 0x0117, NO_DUMP ) /* PAL is read protected */
	ROM_LOAD( "gal16v8b.12n", 0x0400, 0x0117, NO_DUMP ) /* PAL is read protected */
ROM_END

ROM_START( backfirea )
	ROM_REGION( 0x100000, "maincpu", 0 ) /* DE156 code (encrypted) */
	ROM_LOAD32_WORD( "rb-00h.h2",    0x000002, 0x080000, CRC(60973046) SHA1(e70d9be9cb172920da2a2ac9d317768b1438c59d) )
	ROM_LOAD32_WORD( "rb-01l.h3",    0x000000, 0x080000, CRC(27472f60) SHA1(d73b1e68dc51e28b1148db39ce22bd2e93f6fd0a) )

	ROM_REGION( 0x400000, "gfx1", 0 ) /* Tiles 1 */
	ROM_LOAD( "mbz-00.9a",    0x000000, 0x080000, CRC(1098d504) SHA1(1fecd26b92faffce0b59a8a9646bfd457c17c87c) )
	ROM_CONTINUE( 0x200000, 0x080000)
	ROM_CONTINUE( 0x100000, 0x080000)
	ROM_CONTINUE( 0x300000, 0x080000)
	ROM_LOAD( "mbz-01.10a",    0x080000, 0x080000, CRC(19b81e5c) SHA1(4c8204a6a4ad30b23fbfdd79c6e39581e23de6ae) )
	ROM_CONTINUE( 0x280000, 0x080000)
	ROM_CONTINUE( 0x180000, 0x080000)
	ROM_CONTINUE( 0x380000, 0x080000)

	ROM_REGION( 0x100000, "gfx2", 0 ) /* Tiles 2 */
	ROM_LOAD( "mbz-02.12a",    0x000000, 0x100000, CRC(2bd2b0a1) SHA1(8fcb37728f3248ad55e48f2d398b014b36c9ec05) )

	ROM_REGION( 0x400000, "gfx3", 0 ) /* Sprites 1 */
	ROM_LOAD16_BYTE( "mbz-03.15a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.16a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, "gfx4", 0 ) /* Sprites 2 */
	ROM_LOAD16_BYTE( "mbz-03.18a",    0x000001, 0x200000, CRC(2e818569) SHA1(457c1cad25d9b21459262be8b5788969f566a996) )
	ROM_LOAD16_BYTE( "mbz-04.19a",    0x000000, 0x200000, CRC(67bdafb1) SHA1(9729c18f3153e4bba703a6f46ad0b886c52d84e2) )

	ROM_REGION( 0x400000, "ymz", ROMREGION_ERASEFF ) /* samples */
	ROM_LOAD( "mbz-05.17l",    0x000000, 0x200000,  CRC(947c1da6) SHA1(ac36006e04dc5e3990f76539763cc76facd08376) )
	ROM_LOAD( "mbz-06.19l",    0x200000, 0x080000,  CRC(4a38c635) SHA1(7f0fb6a7a4aa6774c04fa38e53ceff8744fe1e9f) )
ROM_END

void backfire_state::descramble_sound()
{
	UINT8 *rom = memregion("ymz")->base();
	int length = 0x200000; // only the first rom is swapped on backfire!
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

	memcpy(rom, &buf1[0], length);
}

READ32_MEMBER(backfire_state::backfire_speedup_r)
{
	//osd_printf_debug( "%08x\n",space.device().safe_pc());

	if (space.device() .safe_pc()== 0xce44)  space.device().execute().spin_until_time(attotime::from_usec(400)); // backfire
	if (space.device().safe_pc() == 0xcee4)  space.device().execute().spin_until_time(attotime::from_usec(400)); // backfirea

	return m_mainram[0x18/4];
}


DRIVER_INIT_MEMBER(backfire_state,backfire)
{
	deco56_decrypt_gfx(machine(), "gfx1"); /* 141 */
	deco56_decrypt_gfx(machine(), "gfx2"); /* 141 */
	deco156_decrypt(machine());
	m_maincpu->set_clock_scale(4.0f); /* core timings aren't accurate */
	descramble_sound();
	m_maincpu->space(AS_PROGRAM).install_read_handler(0x0170018, 0x017001b, read32_delegate(FUNC(backfire_state::backfire_speedup_r), this));
}

GAME( 1995, backfire,  0,        backfire,   backfire, backfire_state, backfire, ROT0, "Data East Corporation", "Backfire! (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1995, backfirea, backfire, backfire,   backfire, backfire_state, backfire, ROT0, "Data East Corporation", "Backfire! (set 2)", MACHINE_SUPPORTS_SAVE ) // defaults to wheel controls, must change to joystick to play
