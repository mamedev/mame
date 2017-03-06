// license: BSD-3-Clause
// copyright-holders: Dirk Best
/***************************************************************************

    Popper

    © 1983 Omori Electric Co. (OEC)

    PCB connector pinout

          GND   1   2  GND
          GND   3   4  GND
           5V   5   6  5V
           5V   7   8  5V
      Speaker   9  10  Speaker
          12V  11  12  12V
     1-P Push  13  14  1-P Right
     2-P Push  15  16  2-P Right
    1-P Start  17  18  1-P Left
    2-P Start  19  20  2-P Left
       Coin A  21  22  1-P Down
       Coin B  23  24  2-P Down
      Service  25  26  1-P Up
               27  28  2-P Up
               29  30
               31  32
        Synic  33  34  Red
        Green  35  36  Blue
           5V  37  38  5V
           5V  39  40  5V
          GND  41  42  GND

    TODO:
    - According to the schematics the sub CPU ROM should be 0x2000
    - Verify screen raw parameters
    - Finish driver

***************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "sound/ay8910.h"
#include "video/resnet.h"
#include "screen.h"
#include "speaker.h"


//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

class popper_state : public driver_device
{
public:
	popper_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		m_maincpu(*this, "maincpu"),
		m_subcpu(*this, "subcpu"),
		m_screen(*this, "screen"),
		m_gfxdecode(*this, "gfxdecode"),
		m_ay{ {*this, "ay1"}, {*this, "ay2"} },
		m_ram(*this, "ram"),
		m_inputs{ {*this, "in1"}, {*this, "in0"}, {*this, "dsw2"}, {*this, "dsw1"} },
		m_scanline_timer(nullptr),
		m_tilemap(nullptr),
		m_nmi_enable(0), m_vram_page(0)
	{ }

	DECLARE_PALETTE_INIT(popper);
	uint32_t screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect);
	TILE_GET_INFO_MEMBER(tile_info);

	DECLARE_WRITE8_MEMBER(nmi_control_w);
	DECLARE_WRITE8_MEMBER(crt_direction_w);
	DECLARE_WRITE8_MEMBER(back_color_select_w);
	DECLARE_WRITE8_MEMBER(vram_page_select_w);
	DECLARE_WRITE8_MEMBER(intcycle_w);
	DECLARE_READ8_MEMBER(subcpu_nmi_r);
	DECLARE_READ8_MEMBER(subcpu_reset_r);
	DECLARE_WRITE8_MEMBER(ay1_w);
	DECLARE_READ8_MEMBER(watchdog_clear_r);
	DECLARE_READ8_MEMBER(inputs_r);

protected:
	virtual void machine_start() override;
	virtual void machine_reset() override;
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr) override;

private:
	required_device<z80_device> m_maincpu;
	required_device<z80_device> m_subcpu;
	required_device<screen_device> m_screen;
	required_device<gfxdecode_device> m_gfxdecode;
	required_device<ay8910_device> m_ay[2];
	required_shared_ptr<uint8_t> m_ram;
	required_ioport m_inputs[4];

	emu_timer *m_scanline_timer;
	tilemap_t *m_tilemap;

	int m_nmi_enable;
	int m_vram_page;
};


//**************************************************************************
//  ADDRESS MAPS
//**************************************************************************

static ADDRESS_MAP_START( main_map, AS_PROGRAM, 8, popper_state )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_NOP
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xe000, 0xe003) AM_MIRROR(0x03fc) AM_READ(inputs_r)
	AM_RANGE(0xe000, 0xe000) AM_MIRROR(0x1ff8) AM_WRITE(nmi_control_w)
	AM_RANGE(0xe001, 0xe001) AM_MIRROR(0x1ff8) AM_WRITE(crt_direction_w)
	AM_RANGE(0xe002, 0xe002) AM_MIRROR(0x1ff8) AM_WRITE(back_color_select_w)
	AM_RANGE(0xe003, 0xe003) AM_MIRROR(0x1ff8) AM_WRITE(vram_page_select_w)
	AM_RANGE(0xe004, 0xe007) AM_MIRROR(0x1ff8) AM_WRITE(intcycle_w)
	AM_RANGE(0xe400, 0xe400) AM_MIRROR(0x03ff) AM_READ(subcpu_nmi_r)
	AM_RANGE(0xe800, 0xf7ff) AM_NOP
	AM_RANGE(0xf800, 0xf800) AM_MIRROR(0x03ff) AM_READ(subcpu_reset_r)
	AM_RANGE(0xfc00, 0xfc00) AM_MIRROR(0x03ff) AM_READ(watchdog_clear_r)
ADDRESS_MAP_END

static ADDRESS_MAP_START( sub_map, AS_PROGRAM, 8, popper_state )
	AM_RANGE(0x0000, 0x1fff) AM_ROM
	AM_RANGE(0x2000, 0x7fff) AM_NOP
	AM_RANGE(0x8000, 0x8003) AM_MIRROR(0x1ffc) AM_WRITE(ay1_w)
	AM_RANGE(0xa000, 0xa003) AM_MIRROR(0x1ffc) AM_DEVWRITE("ay2", ay8910_device, write_bc1_bc2)
	AM_RANGE(0xc000, 0xdfff) AM_RAM AM_SHARE("ram")
	AM_RANGE(0xe000, 0xffff) AM_NOP
ADDRESS_MAP_END


//**************************************************************************
//  INPUT PORT DEFINITIONS
//**************************************************************************

static INPUT_PORTS_START( popper )
	PORT_START("in0")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_COIN1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)                  PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_COIN2)
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_DOWN)   PORT_COCKTAIL  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_SERVICE1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)                    PORT_8WAY
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_UNUSED)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_UP)     PORT_COCKTAIL  PORT_8WAY

	PORT_START("in1")
	PORT_BIT(0x01, IP_ACTIVE_HIGH, IPT_BUTTON1)
	PORT_BIT(0x02, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)                 PORT_8WAY
	PORT_BIT(0x04, IP_ACTIVE_HIGH, IPT_BUTTON1)         PORT_COCKTAIL
	PORT_BIT(0x08, IP_ACTIVE_HIGH, IPT_JOYSTICK_RIGHT)  PORT_COCKTAIL  PORT_8WAY
	PORT_BIT(0x10, IP_ACTIVE_HIGH, IPT_START1)
	PORT_BIT(0x20, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)                  PORT_8WAY
	PORT_BIT(0x40, IP_ACTIVE_HIGH, IPT_START2)
	PORT_BIT(0x80, IP_ACTIVE_HIGH, IPT_JOYSTICK_LEFT)   PORT_COCKTAIL  PORT_8WAY

	PORT_START("dsw1")
	PORT_DIPNAME(0x03, 0x00, DEF_STR( Coin_A ))      PORT_DIPLOCATION("DSW1:1,2")
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_1C ))
	PORT_DIPSETTING(   0x02, DEF_STR( 2C_1C ))
	PORT_DIPSETTING(   0x01, DEF_STR( 3C_1C ))
	PORT_DIPSETTING(   0x03, DEF_STR( 1C_2C ))
	PORT_DIPNAME(0x0c, 0x04, DEF_STR( Coin_B ))      PORT_DIPLOCATION("DSW1:3,4")
	PORT_DIPSETTING(   0x00, DEF_STR( 1C_3C ))
	PORT_DIPSETTING(   0x08, DEF_STR( 1C_4C ))
	PORT_DIPSETTING(   0x04, DEF_STR( 1C_5C ))
	PORT_DIPSETTING(   0x0c, DEF_STR( 1C_6C ))
	PORT_DIPNAME(0x30, 0x20, DEF_STR( Lives ))       PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(   0x00, "2")
	PORT_DIPSETTING(   0x20, "3")
	PORT_DIPSETTING(   0x10, "4")
	PORT_DIPSETTING(   0x30, "5")
	PORT_DIPNAME(0xc0, 0x00, DEF_STR( Bonus_Life ))  PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(   0x00, "20,000 Points")
	PORT_DIPSETTING(   0x80, "30,000 Points")
	PORT_DIPSETTING(   0x40, "40,000 Points")
	PORT_DIPSETTING(   0xc0, "50,000 Points")

	PORT_START("dsw2")
	PORT_DIPNAME(0x01, 0x01, DEF_STR( Demo_Sounds )) PORT_DIPLOCATION("DSW2:1")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x01, DEF_STR( On ))
	PORT_DIPUNUSED_DIPLOC(0x02, IP_ACTIVE_LOW, "DSW2:2")
	PORT_DIPNAME(0x04, 0x00, DEF_STR( Free_Play ))   PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x04, DEF_STR( On ))
	PORT_DIPNAME(0x08, 0x00, "Invulnerability")      PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x08, DEF_STR( On ))
	PORT_DIPNAME(0x10, 0x00, DEF_STR( Flip_Screen )) PORT_DIPLOCATION("DSW2:5")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x10, DEF_STR( On ))
	PORT_DIPNAME(0x20, 0x20, DEF_STR( Cabinet ))     PORT_DIPLOCATION("DSW2:6")
	PORT_DIPSETTING(   0x00, DEF_STR( Cocktail ))
	PORT_DIPSETTING(   0x20, DEF_STR( Upright ))
	PORT_DIPNAME(0x40, 0x00, "Game Repeating")       PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x40, DEF_STR( On ))
	PORT_DIPNAME(0x80, 0x00, DEF_STR( Pause ))       PORT_DIPLOCATION("DSW2:8")
	PORT_DIPSETTING(   0x00, DEF_STR( Off ))
	PORT_DIPSETTING(   0x80, DEF_STR( On ))
INPUT_PORTS_END


//**************************************************************************
//  INPUT PORT HANDLING
//**************************************************************************

READ8_MEMBER( popper_state::inputs_r )
{
	uint8_t data = 0;

	data |= BIT(m_inputs[3]->read(), offset + 4) << 7;
	data |= BIT(m_inputs[3]->read(), offset + 0) << 6;
	data |= BIT(m_inputs[2]->read(), offset + 4) << 5;
	data |= BIT(m_inputs[2]->read(), offset + 0) << 4;
	data |= BIT(m_inputs[1]->read(), offset + 4) << 3;
	data |= BIT(m_inputs[1]->read(), offset + 0) << 2;
	data |= BIT(m_inputs[0]->read(), offset + 4) << 1;
	data |= BIT(m_inputs[0]->read(), offset + 0) << 0;

	return data;
}


//**************************************************************************
//  PALETTE
//**************************************************************************

static const res_net_decode_info popper_decode_info =
{
	1,
	0,
	63,
	//   R     G     B
	{    0,    0,    0, }, // offsets
	{    0,    3,    6, }, // shifts
	{ 0x07, 0x07, 0x03, }  // masks
};

static const res_net_info popper_net_info =
{
	RES_NET_VCC_5V | RES_NET_VBIAS_5V | RES_NET_VIN_TTL_OUT,
 	{
 			{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 3, { 1000, 470, 220 } },
			{ RES_NET_AMP_NONE, 0, 0, 2, {  470, 220,   0 } }
	}
};

PALETTE_INIT_MEMBER( popper_state, popper )
{
	const uint8_t *prom = memregion("colors")->base();
	std::vector<rgb_t> rgb;

	compute_res_net_all(rgb, prom, popper_decode_info, popper_net_info);

	palette.set_pen_colors(0, rgb);
	palette.palette()->normalize_range(0, 63);
}


//**************************************************************************
//  VIDEO EMULATION
//**************************************************************************

void popper_state::device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr)
{
	int y = m_screen->vpos();

	// the maincpu gets an nmi when we enter vblank (when enabled)
	m_maincpu->set_input_line(INPUT_LINE_NMI, (m_nmi_enable && y == 240) ? ASSERT_LINE : CLEAR_LINE);

	// the subcpu gets an interrupt each 32 lines
	m_subcpu->set_input_line(INPUT_LINE_IRQ0, ((y & 31) == 0) ? ASSERT_LINE : CLEAR_LINE);

	m_scanline_timer->adjust(m_screen->time_until_pos(y + 1, 0));
}

WRITE8_MEMBER( popper_state::crt_direction_w )
{
	flip_screen_set(data);
}

WRITE8_MEMBER( popper_state::back_color_select_w )
{
	logerror("back_color_select_w: %02x\n", data);
}

WRITE8_MEMBER( popper_state::vram_page_select_w )
{
	m_vram_page = data & 1;
}

uint32_t popper_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	bitmap.fill(0, cliprect);

	// always draw all tiles
	m_tilemap->mark_all_dirty();

	// draw the character layer
	m_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	return 0;
}


//**************************************************************************
//  DRAWGFX LAYOUTS
//**************************************************************************

static const gfx_layout charlayout =
{
	8,8,
	RGN_FRAC(2,2),
	2,
	{ 0, 4 },
	{ STEP4(8,1), STEP4(0,1) },
	{ STEP8(0,16) },
	16*8
};

static const gfx_layout spritelayout =
{
	16,16,
	RGN_FRAC(1,2),
	2,
	{ RGN_FRAC(0,2), RGN_FRAC(1,2) },
	{ STEP8(8,1), STEP8(0,1) },
	{ STEP16(0,16) },
	16*2*8
};

static GFXDECODE_START( popper )
	GFXDECODE_ENTRY("tiles",   0, charlayout,   0, 16)
	GFXDECODE_ENTRY("sprites", 0, spritelayout, 0, 16)
GFXDECODE_END

TILE_GET_INFO_MEMBER( popper_state::tile_info )
{
	int code = (m_vram_page << 8) | m_ram[0x100 + tile_index];
	int attr = m_ram[0x900 + tile_index];
	int color = attr >> 4;

	SET_TILE_INFO_MEMBER(0, code, color, 0);
}


//**************************************************************************
//  SUBCPU
//**************************************************************************

READ8_MEMBER( popper_state::subcpu_nmi_r )
{
	m_subcpu->set_input_line(INPUT_LINE_NMI, ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_NMI, CLEAR_LINE);

	return 0;
}

READ8_MEMBER( popper_state::subcpu_reset_r )
{
	m_subcpu->set_input_line(INPUT_LINE_RESET, ASSERT_LINE);
	m_subcpu->set_input_line(INPUT_LINE_RESET, CLEAR_LINE);

	return 0;
}

WRITE8_MEMBER( popper_state::ay1_w )
{
	if (offset == 3)
	{
		m_ay[0]->reset();
		m_ay[1]->reset();
	}

	m_ay[0]->write_bc1_bc2(space, offset, data);
}


//**************************************************************************
//  MACHINE EMULATION
//**************************************************************************

WRITE8_MEMBER( popper_state::nmi_control_w )
{
//	logerror("nmi_control_w: %02x\n", data);
	m_nmi_enable = data & 1;
}

WRITE8_MEMBER( popper_state::intcycle_w )
{
	logerror("intcycle_w: %d = %02x\n", offset, data);
}

READ8_MEMBER( popper_state::watchdog_clear_r )
{
	logerror("watchdog_clear_r\n");
	return 0;
}

void popper_state::machine_start()
{
	// create tilemap
	m_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(FUNC(popper_state::tile_info), this), TILEMAP_SCAN_COLS, 8, 8, 48, 32);
	m_tilemap->set_transparent_pen(3);

	// allocate and start scanline timer
	m_scanline_timer = timer_alloc(0);
	m_scanline_timer->adjust(machine().first_screen()->time_until_pos(0, 0));

	// register for save states
	save_item(NAME(m_nmi_enable));
	save_item(NAME(m_vram_page));
}

void popper_state::machine_reset()
{
	m_nmi_enable = 0;
	m_vram_page = 0;
}


//**************************************************************************
//  MACHINE DEFINTIONS
//**************************************************************************

static MACHINE_CONFIG_START( popper, popper_state )
	MCFG_CPU_ADD("maincpu", Z80, XTAL_18_432MHz/3/2)
	MCFG_CPU_PROGRAM_MAP(main_map)

	MCFG_CPU_ADD("subcpu", Z80, XTAL_18_432MHz/3/2)
	MCFG_CPU_PROGRAM_MAP(sub_map)

	MCFG_QUANTUM_PERFECT_CPU("maincpu")

	// video hardware
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_RAW_PARAMS(XTAL_18_432MHz/3, 384, 48, 328, 264, 16, 240)
	MCFG_SCREEN_UPDATE_DRIVER(popper_state, screen_update)
	MCFG_SCREEN_PALETTE("palette")

	MCFG_GFXDECODE_ADD("gfxdecode", "palette", popper)

	MCFG_PALETTE_ADD("palette", 64)
	MCFG_PALETTE_INIT_OWNER(popper_state, popper)

	// audio hardware
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ay1", AY8910, XTAL_18_432MHz/3/2/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	MCFG_SOUND_ADD("ay2", AY8910, XTAL_18_432MHz/3/2/2)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)
MACHINE_CONFIG_END


//**************************************************************************
//  ROM DEFINITIONS
//**************************************************************************

ROM_START( popper )
	ROM_REGION(0x8000, "maincpu", 0)
	ROM_LOAD("p1", 0x0000, 0x2000, CRC(56881b70) SHA1(d3ade7a54a6cb8a0babf0d667a6b27f492a739dc))
	ROM_LOAD("p2", 0x2000, 0x2000, CRC(a054d9d2) SHA1(fcd86e7247b40cf07ea595a64c104b99b0e93ced))
	ROM_LOAD("p3", 0x4000, 0x2000, CRC(6201928a) SHA1(53b571b9f2c0568f10cd974641863c2e00777b46))

	ROM_REGION(0x2000, "subcpu", 0)
	ROM_LOAD("p0", 0x0000, 0x1000, CRC(ef5f7c5b) SHA1(c63a3d9ef2868ad7eaacddec810d62d2e124dc15))

	ROM_REGION(0x2000, "tiles", 0)
	ROM_LOAD("p4", 0x0000, 0x2000, CRC(86203349) SHA1(cce2dd3fa786c2fb3ca80e7b93adf94db3b46b01))

	ROM_REGION(0x4000, "sprites", 0)
	ROM_LOAD("p5", 0x0000, 0x2000, CRC(a21ac194) SHA1(2c0e3df8981a12d383b1c4619a0b95a7c2d176a7))
	ROM_LOAD("p6", 0x2000, 0x2000, CRC(d99fa790) SHA1(201271ee4fb812236a38cb5f9070ac29e8186097))

	ROM_REGION(0x40, "colors", 0)
	ROM_LOAD("p.m3", 0x00, 0x20, CRC(713217aa) SHA1(6083c3432bf94c9e983fcc79171529f519c86105))
	ROM_LOAD("p.m4", 0x20, 0x20, CRC(384de5c1) SHA1(892c89a01c11671c5708113b4e4c27b84be37ea6))
ROM_END


//**************************************************************************
//  SYSTEM DRIVERS
//**************************************************************************

//    YEAR  NAME    PARENT  MACHINE  INPUT   CLASS          INIT  ROTATION  COMPANY  FULLNAME  FLAGS
GAME( 1983, popper, 0,      popper,  popper, driver_device, 0,    ROT90,    "Omori", "Popper", MACHINE_NOT_WORKING | MACHINE_SUPPORTS_SAVE )
