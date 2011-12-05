/***************************************************************************************

Heavy Unit
Kaneko/Taito 1988

Driver based on djboy.c / airbustr.c

This game runs on Kaneko hardware. The game is similar to R-Type.

PCB Layout
----------
M6100391A
M6100392A  880210204
KNA-001
|----------------------------------------------------|
|                                                    |
|           6116                          6116       |
|      15Mhz                              6116       |
|                                 PAL                |
|           B73_09.2P             B73_11.5P          |
|                                                    |
|                                                    |
|                                 Z80-1   DSW1 DSW2 J|
|                                                   A|
|     16MHz                                         M|
|                                                   M|
|       12MHz                       6264  MERMAID   A|
| B73_05.1H                                          |
| B73_04.1F B73_08.2F  6116              Z80-2       |
| B73_03.1D                       Z80-3  B73_12.7E   |
| B73_02.1C B73_07.2C  PANDORA    B73_10.5C  6116    |
| B73_01.1B B73_06.2B 4164 4164   6264 PAL  YM3014   |
|                     4164 4164   PAL       YM2203   |
|----------------------------------------------------|

Notes:
      Z80-1 clock  : 6.000MHz
      Z80-2 clock  : 6.000MHz
      Z80-3 clock  : 6.000MHz
      YM2203 clock : 3.000MHz
      VSync        : 58Hz
      HSync        : 15.59kHz
               \-\ : KANEKO 1988. DIP40 8751 MCU
      MERMAID    | : pin 18,19 = 6.000MHz (main clock)
                 | : pin 30 = 1.000MHz (prog/ale)
               /-/ : pin 22 = 111.48Hz (port 2 bit 1)

      PANDORA      : KANEKO PX79480FP-3 PANDORA-CHIP (C) KANEKO 1988


To Do:
------

- Fix cocktail mode
- Fix Flip Screen (currently displays an information screen and emulation hangs)


***************************************************************************************/

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/mcs51/mcs51.h"
#include "sound/2203intf.h"
#include "video/kan_pand.h"



/*************************************
 *
 *  Driver state
 *
 *************************************/

class hvyunit_state : public driver_device
{
public:
	hvyunit_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	/* Video */
	UINT8			*m_videoram;
	UINT8			*m_colorram;
	tilemap_t		*m_bg_tilemap;
	UINT16			m_scrollx;
	UINT16			m_scrolly;
	UINT16			m_port0_data;

	/* Mermaid */
	UINT8			m_data_to_mermaid;
	UINT8			m_data_to_z80;
	UINT8			m_mermaid_to_z80_full;
	UINT8			m_z80_to_mermaid_full;
	UINT8			m_mermaid_int0_l;
	UINT8			m_mermaid_p[4];

	/* Devices */
	device_t	*m_master_cpu;
	device_t	*m_slave_cpu;
	device_t	*m_sound_cpu;
	device_t	*m_mermaid;
	device_t	*m_pandora;
};


/*************************************
 *
 *  Initialisation
 *
 *************************************/

static MACHINE_START( hvyunit )
{
	hvyunit_state *state = machine.driver_data<hvyunit_state>();

	state->m_master_cpu = machine.device("master");
	state->m_slave_cpu = machine.device("slave");
	state->m_sound_cpu = machine.device("soundcpu");
	state->m_mermaid = machine.device("mermaid");
	state->m_pandora = machine.device("pandora");

	// TODO: Save state
}

static MACHINE_RESET( hvyunit )
{
	hvyunit_state *state = machine.driver_data<hvyunit_state>();

	state->m_mermaid_int0_l = 1;
	state->m_mermaid_to_z80_full = 0;
	state->m_z80_to_mermaid_full = 0;
}


/*************************************
 *
 *  Video hardware
 *
 *************************************/

static TILE_GET_INFO( get_bg_tile_info )
{
	hvyunit_state *state = machine.driver_data<hvyunit_state>();

	int attr = state->m_colorram[tile_index];
	int code = state->m_videoram[tile_index] + ((attr & 0x0f) << 8);
	int color = (attr >> 4);

	SET_TILE_INFO(1, code, color, 0);
}

static VIDEO_START( hvyunit )
{
	hvyunit_state *state = machine.driver_data<hvyunit_state>();
	state->m_bg_tilemap = tilemap_create(machine, get_bg_tile_info, tilemap_scan_rows, 16, 16, 32, 32);
}

static SCREEN_UPDATE( hvyunit )
{
#define SX_POS	96
#define SY_POS	0
	hvyunit_state *state = screen->machine().driver_data<hvyunit_state>();

	tilemap_set_scrollx(state->m_bg_tilemap, 0, ((state->m_port0_data & 0x40) << 2) + state->m_scrollx + SX_POS); // TODO
	tilemap_set_scrolly(state->m_bg_tilemap, 0, ((state->m_port0_data & 0x80) << 1) + state->m_scrolly + SY_POS); // TODO
	bitmap_fill(bitmap,cliprect,get_black_pen(screen->machine()));
	tilemap_draw(bitmap, cliprect, state->m_bg_tilemap, 0, 0);
	pandora_update(state->m_pandora, bitmap, cliprect);

	return 0;
}

static SCREEN_EOF( hvyunit )
{
	hvyunit_state *state = machine.driver_data<hvyunit_state>();
	pandora_eof(state->m_pandora);
}


/*************************************
 *
 *  Master CPU handlers
 *
 *************************************/

static WRITE8_HANDLER( trigger_nmi_on_slave_cpu )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();
	device_set_input_line(state->m_slave_cpu, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( master_bankswitch_w )
{
	unsigned char *ROM = space->machine().region("master")->base();
	int bank = data & 7;
	ROM = &ROM[0x4000 * bank];
	memory_set_bankptr(space->machine(), "bank1", ROM);
}

static WRITE8_HANDLER( mermaid_data_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_data_to_mermaid = data;
	state->m_z80_to_mermaid_full = 1;
	state->m_mermaid_int0_l = 0;
	device_set_input_line(state->m_mermaid, INPUT_LINE_IRQ0, ASSERT_LINE);
}

static READ8_HANDLER( mermaid_data_r )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_mermaid_to_z80_full = 0;
	return state->m_data_to_z80;
}

static READ8_HANDLER( mermaid_status_r )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	return (!state->m_mermaid_to_z80_full << 2) | (state->m_z80_to_mermaid_full << 3);
}


/*************************************
 *
 *  Slave CPU handlers
 *
 *************************************/

static WRITE8_HANDLER( trigger_nmi_on_sound_cpu2 )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	soundlatch_w(space, 0, data);
	device_set_input_line(state->m_sound_cpu, INPUT_LINE_NMI, PULSE_LINE);
}

static WRITE8_HANDLER( hu_videoram_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_videoram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

static WRITE8_HANDLER( hu_colorram_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_colorram[offset] = data;
	tilemap_mark_tile_dirty(state->m_bg_tilemap, offset);
}

static WRITE8_HANDLER( slave_bankswitch_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	unsigned char *ROM = space->machine().region("slave")->base();
	int bank = (data & 0x03);
	state->m_port0_data = data;
	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine(), "bank2", ROM);
}

static WRITE8_HANDLER( hu_scrollx_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();
	state->m_scrollx = data;
}

static WRITE8_HANDLER( hu_scrolly_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();
	state->m_scrolly = data;
}

static WRITE8_HANDLER( coin_count_w )
{
	coin_counter_w(space->machine(), 0, data & 1);
	coin_counter_w(space->machine(), 1, data & 2);
}


/*************************************
 *
 *  Sound CPU handlers
 *
 *************************************/

static WRITE8_HANDLER( sound_bankswitch_w )
{
	unsigned char *ROM = space->machine().region("soundcpu")->base();
	int bank = data & 0x3;
	ROM = &ROM[0x4000 * bank];

	memory_set_bankptr(space->machine(), "bank3", ROM);
}


/*************************************
 *
 *  Protection MCU handlers
 *
 *************************************/

static READ8_HANDLER( mermaid_p0_r )
{
	// ?
	return 0;
}

static WRITE8_HANDLER( mermaid_p0_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	if (!BIT(state->m_mermaid_p[0], 1) && BIT(data, 1))
	{
		state->m_mermaid_to_z80_full = 1;
		state->m_data_to_z80 = state->m_mermaid_p[1];
	}

	if (BIT(data, 0) == 1)
		state->m_z80_to_mermaid_full = 0;

	state->m_mermaid_p[0] = data;
}

static READ8_HANDLER( mermaid_p1_r )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	if (BIT(state->m_mermaid_p[0], 0) == 0)
		return state->m_data_to_mermaid;
	else
		return 0; // ?
}

static WRITE8_HANDLER( mermaid_p1_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	if (data == 0xff)
	{
		state->m_mermaid_int0_l = 1;
		device_set_input_line(state->m_mermaid, INPUT_LINE_IRQ0, CLEAR_LINE);
	}

	state->m_mermaid_p[1] = data;
}

static READ8_HANDLER( mermaid_p2_r )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	switch ((state->m_mermaid_p[0] >> 2) & 3)
	{
		case 0: return input_port_read(space->machine(), "IN1");
		case 1: return input_port_read(space->machine(), "IN2");
		case 2: return input_port_read(space->machine(), "IN0");
		default: return 0xff;
	}
}

static WRITE8_HANDLER( mermaid_p2_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_mermaid_p[2] = data;
}

static READ8_HANDLER( mermaid_p3_r )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	UINT8 dsw = 0;
	UINT8 dsw1 = input_port_read(space->machine(), "DSW1");
	UINT8 dsw2 = input_port_read(space->machine(), "DSW2");

	switch ((state->m_mermaid_p[0] >> 5) & 3)
	{
		case 0: dsw = (BIT(dsw2, 4) << 3) | (BIT(dsw2, 0) << 2) | (BIT(dsw1, 4) << 1) | BIT(dsw1, 0); break;
		case 1: dsw = (BIT(dsw2, 5) << 3) | (BIT(dsw2, 1) << 2) | (BIT(dsw1, 5) << 1) | BIT(dsw1, 1); break;
		case 2: dsw = (BIT(dsw2, 6) << 3) | (BIT(dsw2, 2) << 2) | (BIT(dsw1, 6) << 1) | BIT(dsw1, 2); break;
		case 3: dsw = (BIT(dsw2, 7) << 3) | (BIT(dsw2, 3) << 2) | (BIT(dsw1, 7) << 1) | BIT(dsw1, 3); break;
	}

	return (dsw << 4) | (state->m_mermaid_int0_l << 2) | (state->m_mermaid_to_z80_full << 3);
}

static WRITE8_HANDLER( mermaid_p3_w )
{
	hvyunit_state *state = space->machine().driver_data<hvyunit_state>();

	state->m_mermaid_p[3] = data;
	device_set_input_line(state->m_slave_cpu, INPUT_LINE_RESET, data & 2 ? CLEAR_LINE : ASSERT_LINE);
}


/*************************************
 *
 *  Memory maps
 *
 *************************************/

static ADDRESS_MAP_START( master_memory, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank1")
	AM_RANGE(0xc000, 0xcfff) AM_DEVREADWRITE("pandora", pandora_spriteram_r, pandora_spriteram_w)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( master_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(master_bankswitch_w)
	AM_RANGE(0x01, 0x01) AM_WRITE(master_bankswitch_w) // correct?
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_slave_cpu)
ADDRESS_MAP_END


static ADDRESS_MAP_START( slave_memory, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank2")
	AM_RANGE(0xc000, 0xc3ff) AM_RAM_WRITE(hu_videoram_w) AM_BASE_MEMBER(hvyunit_state, m_videoram)
	AM_RANGE(0xc400, 0xc7ff) AM_RAM_WRITE(hu_colorram_w) AM_BASE_MEMBER(hvyunit_state, m_colorram)
	AM_RANGE(0xd000, 0xd1ff) AM_RAM_WRITE(paletteram_xxxxRRRRGGGGBBBB_split2_w) AM_BASE_GENERIC(paletteram2)
	AM_RANGE(0xd800, 0xd9ff) AM_RAM_WRITE(paletteram_xxxxRRRRGGGGBBBB_split1_w) AM_BASE_GENERIC(paletteram)
	AM_RANGE(0xd000, 0xdfff) AM_RAM
	AM_RANGE(0xe000, 0xffff) AM_RAM AM_SHARE("share1")
ADDRESS_MAP_END

static ADDRESS_MAP_START( slave_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(slave_bankswitch_w)
	AM_RANGE(0x02, 0x02) AM_WRITE(trigger_nmi_on_sound_cpu2)
	AM_RANGE(0x04, 0x04) AM_READWRITE(mermaid_data_r, mermaid_data_w)
	AM_RANGE(0x06, 0x06) AM_WRITE(hu_scrolly_w)
	AM_RANGE(0x08, 0x08) AM_WRITE(hu_scrollx_w)
	AM_RANGE(0x0c, 0x0c) AM_READ(mermaid_status_r)
	AM_RANGE(0x0e, 0x0e) AM_WRITE(coin_count_w)

//  AM_RANGE(0x22, 0x22) AM_READ(hu_scrolly_hi_reset) //22/a2 taken from ram $f065
//  AM_RANGE(0xa2, 0xa2) AM_READ(hu_scrolly_hi_set)
ADDRESS_MAP_END


static ADDRESS_MAP_START( sound_memory, AS_PROGRAM, 8 )
	AM_RANGE(0x0000, 0x7fff) AM_ROM
	AM_RANGE(0x8000, 0xbfff) AM_ROMBANK("bank3")
	AM_RANGE(0xc000, 0xc7ff) AM_RAM
ADDRESS_MAP_END

static ADDRESS_MAP_START( sound_io, AS_IO, 8 )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x00, 0x00) AM_WRITE(sound_bankswitch_w)
	AM_RANGE(0x02, 0x03) AM_DEVREADWRITE("ymsnd", ym2203_r, ym2203_w)
	AM_RANGE(0x04, 0x04) AM_READ(soundlatch_r)
ADDRESS_MAP_END


static ADDRESS_MAP_START( mcu_io, AS_IO, 8 )
	AM_RANGE(MCS51_PORT_P0, MCS51_PORT_P0) AM_READWRITE(mermaid_p0_r, mermaid_p0_w)
	AM_RANGE(MCS51_PORT_P1, MCS51_PORT_P1) AM_READWRITE(mermaid_p1_r, mermaid_p1_w)
	AM_RANGE(MCS51_PORT_P2, MCS51_PORT_P2) AM_READWRITE(mermaid_p2_r, mermaid_p2_w)
	AM_RANGE(MCS51_PORT_P3, MCS51_PORT_P3) AM_READWRITE(mermaid_p3_r, mermaid_p3_w)
ADDRESS_MAP_END


/*************************************
 *
 *  Port definitions
 *
 *************************************/

static INPUT_PORTS_START( hvyunit )
	PORT_START("IN0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_START1 )
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_START2 )
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_COIN1 )
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_COIN2 )
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_SERVICE )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 )
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 )
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("IN2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_JOYSTICK_UP ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_JOYSTICK_DOWN ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_JOYSTICK_LEFT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_JOYSTICK_RIGHT ) PORT_8WAY PORT_PLAYER(2)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_BUTTON1 ) PORT_PLAYER(2)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_BUTTON2 ) PORT_PLAYER(2)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNKNOWN )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNKNOWN )

	PORT_START("DSW1")
	PORT_DIPNAME( 0x01, 0x00, DEF_STR( Cabinet ) )			PORT_DIPLOCATION("DSW1:1")
	PORT_DIPSETTING(    0x00, DEF_STR( Upright ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Cocktail ) )
	PORT_DIPNAME( 0x02, 0x02, DEF_STR( Flip_Screen ) )		PORT_DIPLOCATION("DSW1:2")
	PORT_DIPSETTING(    0x02, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Service_Mode ) )		PORT_DIPLOCATION("DSW1:3")
	PORT_DIPSETTING(    0x04, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x08, ( "Coin Mode" ) )			PORT_DIPLOCATION("DSW1:4")
	PORT_DIPSETTING(    0x08, ( "Mode 1" ) )
	PORT_DIPSETTING(    0x00, ( "Mode 2" ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x20, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x10, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_2C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_6C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x08)
	PORT_DIPSETTING(    0x80, DEF_STR( 2C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x40, DEF_STR( 1C_3C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)
	PORT_DIPSETTING(    0x00, DEF_STR( 1C_4C ) ) PORT_CONDITION("DSW1", 0x08, PORTCOND_EQUALS, 0x00)

	PORT_START("DSW2")
	PORT_DIPNAME( 0x03, 0x03, DEF_STR( Difficulty ) )		PORT_DIPLOCATION("DSW2:1,2")
	PORT_DIPSETTING(    0x02, DEF_STR( Easy ) )
	PORT_DIPSETTING(    0x03, DEF_STR( Normal ) )
	PORT_DIPSETTING(    0x01, DEF_STR( Hard ) )
	PORT_DIPSETTING(    0x00, DEF_STR( Hardest ) )
	PORT_DIPNAME( 0x04, 0x04, DEF_STR( Allow_Continue ) )		PORT_DIPLOCATION("DSW2:3")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x04, DEF_STR( On ) )
	PORT_DIPNAME( 0x08, 0x00, "Bonus" )				PORT_DIPLOCATION("DSW2:4")
	PORT_DIPSETTING(    0x08, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x00, DEF_STR( On ) )
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Lives ) )			PORT_DIPLOCATION("DSW2:5,6")
	PORT_DIPSETTING(    0x30, "3" )
	PORT_DIPSETTING(    0x20, "4" )
	PORT_DIPSETTING(    0x10, "5" )
	PORT_DIPSETTING(    0x00, "7" )
	PORT_DIPNAME( 0x40, 0x40, DEF_STR( Demo_Sounds ) )		PORT_DIPLOCATION("DSW2:7")
	PORT_DIPSETTING(    0x00, DEF_STR( Off ) )
	PORT_DIPSETTING(    0x40, DEF_STR( On ) )
	PORT_DIPUNUSED_DIPLOC( 0x80, 0x80, "DSW2:8")
INPUT_PORTS_END

static INPUT_PORTS_START( hvyunitj )
	PORT_INCLUDE(hvyunit)

	PORT_MODIFY("DSW1")
	PORT_DIPUNUSED_DIPLOC( 0x08, 0x08, "DSW1:4")
	PORT_DIPNAME( 0x30, 0x30, DEF_STR( Coin_A ) )			PORT_DIPLOCATION("DSW1:5,6")
	PORT_DIPSETTING(    0x10, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0x30, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x20, DEF_STR( 1C_2C ) )
	PORT_DIPNAME( 0xc0, 0xc0, DEF_STR( Coin_B ) )			PORT_DIPLOCATION("DSW1:7,8")
	PORT_DIPSETTING(    0x40, DEF_STR( 2C_1C ) )
	PORT_DIPSETTING(    0xc0, DEF_STR( 1C_1C ) )
	PORT_DIPSETTING(    0x00, DEF_STR( 2C_3C ) )
	PORT_DIPSETTING(    0x80, DEF_STR( 1C_2C ) )
INPUT_PORTS_END


/*************************************
 *
 *  Graphics definitions
 *
 *************************************/

static const gfx_layout tile_layout =
{
	16,16,
	RGN_FRAC(1,1),
	4,
	{ 0, 1, 2, 3 },
	{
		0*4,1*4,2*4,3*4,4*4,5*4,6*4,7*4,
		8*32+0*4,8*32+1*4,8*32+2*4,8*32+3*4,8*32+4*4,8*32+5*4,8*32+6*4,8*32+7*4
	},
	{
		0*32,1*32,2*32,3*32,4*32,5*32,6*32,7*32,
		16*32+0*32,16*32+1*32,16*32+2*32,16*32+3*32,16*32+4*32,16*32+5*32,16*32+6*32,16*32+7*32
	},
	4*8*32
};

static GFXDECODE_START( hvyunit )
	GFXDECODE_ENTRY( "gfx1", 0, tile_layout, 0x100, 16 ) /* sprite bank */
	GFXDECODE_ENTRY( "gfx2", 0, tile_layout, 0x000, 16 ) /* background tiles */
GFXDECODE_END


/*************************************
 *
 *  Machine driver
 *
 *************************************/

/* Main Z80 uses IM2 */
static TIMER_DEVICE_CALLBACK( hvyunit_scanline )
{
	hvyunit_state *state = timer.machine().driver_data<hvyunit_state>();
	int scanline = param;

	if(scanline == 240) // vblank-out irq
		device_set_input_line_and_vector(state->m_master_cpu, 0, HOLD_LINE, 0xfd);

	/* Pandora "sprite end dma" irq? TODO: timing is likely off */
	if(scanline == 64)
		device_set_input_line_and_vector(state->m_master_cpu, 0, HOLD_LINE, 0xff);
}

static const kaneko_pandora_interface hvyunit_pandora_config =
{
	"screen",	/* screen tag */
	0,			/* gfx_region */
	0, 0		/* x_offs, y_offs */
};


/*************************************
 *
 *  Machine driver
 *
 *************************************/

static MACHINE_CONFIG_START( hvyunit, hvyunit_state )

	MCFG_CPU_ADD("master", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(master_memory)
	MCFG_CPU_IO_MAP(master_io)
	MCFG_TIMER_ADD_SCANLINE("scantimer", hvyunit_scanline, "screen", 0, 1)

	MCFG_CPU_ADD("slave", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(slave_memory)
	MCFG_CPU_IO_MAP(slave_io)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("soundcpu", Z80, 6000000)
	MCFG_CPU_PROGRAM_MAP(sound_memory)
	MCFG_CPU_IO_MAP(sound_io)
	MCFG_CPU_VBLANK_INT("screen", irq0_line_hold)

	MCFG_CPU_ADD("mermaid", I80C51, 6000000)
	MCFG_CPU_IO_MAP(mcu_io)

	MCFG_QUANTUM_TIME(attotime::from_hz(6000))

	MCFG_MACHINE_START(hvyunit)
	MCFG_MACHINE_RESET(hvyunit)

	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(58)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(0))
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(256, 256)
	MCFG_SCREEN_VISIBLE_AREA(0, 256-1, 16, 240-1)
	MCFG_SCREEN_UPDATE(hvyunit)
	MCFG_SCREEN_EOF(hvyunit)

	MCFG_GFXDECODE(hvyunit)
	MCFG_PALETTE_LENGTH(0x800)

	MCFG_KANEKO_PANDORA_ADD("pandora", hvyunit_pandora_config)

	MCFG_VIDEO_START(hvyunit)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")

	MCFG_SOUND_ADD("ymsnd", YM2203, 3000000)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.80)
MACHINE_CONFIG_END



/*************************************
 *
 *  ROM definition(s)
 *
 *************************************/

ROM_START( hvyunit )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_10.5c",  0x00000, 0x20000, CRC(ca52210f) SHA1(346951962aa5bbad641117dbd66f035dddc7c0bf) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_11.5p",  0x00000, 0x10000, CRC(cb451695) SHA1(116fd59f96a54c22fae65eea9ee5e58cb9ce5074) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	/*                      0x190000, 0x010000  no data */
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	/*                      0x1b0000, 0x010000  no data */
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitj )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_30.5c",  0x00000, 0x20000, CRC(600af545) SHA1(c52b9be2bae28848ad0818c296f000a1bda4fa4f) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "gfx1", 0 ) /* note, the rom ordering on this is different to the other Japan set */
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x110000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73_01.1b",  0x120000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73_02.1c",  0x130000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73_03.1d",  0x140000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	ROM_LOAD( "b73_04.1f",  0x150000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	ROM_LOAD( "b73_05.1h",  0x160000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitjo )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_13.5c",  0x00000, 0x20000, CRC(e2874601) SHA1(7f7f3287113b8622eb365d04135d2d9c35d70554) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_14.5p",  0x00000, 0x10000, CRC(0dfb51d4) SHA1(0e6f3b3d4558f12fe1b1620f57a0f4ac2065fd1a) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_07.2c",  0x100000, 0x010000, CRC(5cffa42c) SHA1(687e047345039479b35d5099e56dbc1d57284ed9) )
	ROM_LOAD( "b73_06.2b",  0x120000, 0x010000, CRC(a98e4aea) SHA1(560fef03ad818894c9c7578c6282d55b646e8129) )
	ROM_LOAD( "b73_01.1b",  0x140000, 0x010000, CRC(3a8a4489) SHA1(a01d7300015f90ce6dd571ad93e7a58270a99e47) )
	ROM_LOAD( "b73_02.1c",  0x160000, 0x010000, CRC(025c536c) SHA1(075e95cc39e792049ae656404e7f7440df064391) )
	ROM_LOAD( "b73_03.1d",  0x180000, 0x010000, CRC(ec6020cf) SHA1(2973aa2dc3deb2f27c9f1bad07a7664bad95b3f2) )
	/*                      0x190000, 0x010000  no data */
	ROM_LOAD( "b73_04.1f",  0x1a0000, 0x010000, CRC(f7badbb2) SHA1(d824ab4aba94d7ca02401f4f6f34213143c282ec) )
	/*                      0x1b0000, 0x010000  no data */
	ROM_LOAD( "b73_05.1h",  0x1c0000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) )

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END

ROM_START( hvyunitu )
	ROM_REGION( 0x20000, "master", 0 )
	ROM_LOAD( "b73_34.5c",  0x00000, 0x20000, CRC(05c30a90) SHA1(97cc0ded2896e0945d790247c284e5058c28c735) )

	ROM_REGION( 0x10000, "slave", 0 )
	ROM_LOAD( "b73_35.6p",  0x00000, 0x10000, CRC(aed1669d) SHA1(d0539261d6128fa2d58b529e8383b6d1f3ccac77) )

	ROM_REGION( 0x10000, "soundcpu", 0 )
	ROM_LOAD( "b73_12.7e",  0x000000, 0x010000, CRC(d1d24fab) SHA1(ed0312535d0b136d79aa885b9e6eea19ebde6409) )

	ROM_REGION( 0x1000, "mermaid", 0 )
	ROM_LOAD( "mermaid.bin",  0x0000, 0x0e00, CRC(88c5dd27) SHA1(5043fed7fd192891be7e4096f2c5daaae1538bc4) )

	ROM_REGION( 0x200000, "gfx1", 0 )
	ROM_LOAD( "b73_08.2f",  0x000000, 0x080000, CRC(f83dd808) SHA1(09d5f1e86fad3a0d2d3ac1845103d3f2833c6793) )
	ROM_LOAD( "b73_28.2c",  0x100000, 0x020000, CRC(a02e08d6) SHA1(72764d4e8474aaac0674fd1c20278a706da7ade2) )
	ROM_LOAD( "b73_27.2b",  0x120000, 0x020000, CRC(8708f97c) SHA1(ccddc7f2fa53c5e35345c2db0520f515c512b723) )
	ROM_LOAD( "b73_25.0b",  0x140000, 0x020000, CRC(2f13f81e) SHA1(9d9c1869bf582a0bc0581cdf5b65237124b9e456) ) /* the data in first half of this actually differs slightly to the other sets, a 0x22 fill is replaced by 0xff on empty tiles */
	ROM_LOAD( "b73_26.0c",  0x160000, 0x010000, CRC(b8e829d2) SHA1(31102358500d7b58173d4f18647decf5db744416) ) /* == b73_05.1h, despite the different label */

	ROM_REGION( 0x80000, "gfx2", 0 )
	ROM_LOAD( "b73_09.2p",  0x000000, 0x080000, CRC(537c647f) SHA1(941c0f4e251bc68e53d62e70b033a3a6c145bb7e) )
ROM_END


/*************************************
 *
 *  Game driver(s)
 *
 *************************************/

GAME( 1988, hvyunit, 0,        hvyunit, hvyunit,  0, ROT0, "Kaneko / Taito", "Heavy Unit (World)", GAME_NO_COCKTAIL )
GAME( 1988, hvyunitj, hvyunit, hvyunit, hvyunitj, 0, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, Newer)", GAME_NO_COCKTAIL )
GAME( 1988, hvyunitjo,hvyunit, hvyunit, hvyunitj, 0, ROT0, "Kaneko / Taito", "Heavy Unit (Japan, Older)", GAME_NO_COCKTAIL )
GAME( 1988, hvyunitu, hvyunit, hvyunit, hvyunitj, 0, ROT0, "Kaneko / Taito", "Heavy Unit -U.S.A. Version- (US)", GAME_NO_COCKTAIL )
